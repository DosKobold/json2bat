#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <jsoncpp/json/json.h>

#include "converter.hpp"

Converter::Converter()
{
	file        = new File{};
	forceOW     = false;
	writeToFile = true;
}

Converter::~Converter()
{
	delete file;
}

void
Converter::clear_file()
{
	delete file;
	file = new File{};
}

bool
Converter::parse_json(std::string inFile)
{
	Json::Value object{};
	std::ifstream input(inFile);

	if (!reader.parse(input, object)) {
		std::cerr << "ERROR: [" << inFile  << \
		    "] Could not parse file into objects! Following message is " \
		    "provided by the parser: " << std::endl;
		std::cerr << reader.getFormattedErrorMessages();
		return false;
	}

	if (!file->fill(object, inFile, std::cerr)) {
		return false;
	}
	return true;
}

bool
Converter::write_bat()
{
	std::streambuf *sbuf;
	std::ofstream tmp;

	/* In case the filename already exists, ask if overwriting is okay. */
	if (std::filesystem::exists(file->outfile()) && !forceOW && writeToFile) {
		char yn;
		do {
			std::cerr << "Overwrite existing file [" << file->outfile() \
			    << "]? (y/n): ";
			std::cin >> yn;
		} while (yn != 'y' && yn != 'n');
		if (yn == 'n')
			return true;
	}

	/*
	 * By default std::cout can't be assigned to std::ofstream.
	 * This limitation would make it impossible to use a single
	 * objects for files and stdout. To avoid this, take the
	 * underlying buffer out of the file and move it to
	 * a new objects with a datatype that can represent both.
	 */
	if (writeToFile) {
		tmp.open(file->outfile());
		sbuf = tmp.rdbuf();
	} else {
		sbuf = std::cout.rdbuf();
	}
	std::ostream output(sbuf);

	/* Basic setup */
	output << "@ECHO OFF\r\n" << "C:\\Windows\\System32\\cmd.exe ";

	if (file->hideshell() == true) {
		output << "/c \"";
	} else {
		output << "/k \"";
	}

	/* Take care of EXE instructions */
	file->iterate_commands(output, " && ", true);

	/* Take care of ENV instructions */
	bool b = (file->commands_size() == 0 && file->env_size() > 0);
	if (b) {
		output << "set ";
	}
	file->iterate_env(output, " && set ", b);

	/* Take care of PATH instructions */
	if (file->commands_size() > 0 && file->env_size() > 0) {
		output << " && ";
	}
	if (file->paths_size() != 0)
		output << "set path=";
	file->iterate_paths(output, ";", "");
	if (file->paths_size() != 0)
		output << "\%path\%";

	/* Take care of application */
	if (!file->application().empty()) {
		output << " && start \"" << file->title() \
		    << "\" " << file->application();
	}
	output << "\"\r\n@ECHO ON\r\n";

	return true;
}

void
Converter::out_to_file(bool b)
{
	writeToFile = b;
}

void
Converter::force_overwrite(bool b)
{
	forceOW = b;
}

void
Converter::print_fmt()
{
	std::cout << "Target name: " << file->outfile() << std::endl;
	std::cout << "Hideshell:   " << std::boolalpha << file->hideshell();

	file->iterate_commands(std::cout, "\nEXE   | ", false);
	file->iterate_env(std::cout, "\nENV   | ", false);
	file->iterate_paths(std::cout, "\nPATH  | ", "");
	std::cout << std::endl;
}

bool
Converter::overwrite(char *arg)
{
	std::string val;
	char tmp;
	char *eq;

	if (!*arg && *arg != '=')
		return false;

	eq = strchr(arg, '=');
	if (!eq || !eq[1])
		return false;
	tmp = *eq;
	*eq++ = '\0';

	val.assign(eq);
	if (!strcmp(arg, "hideshell")) {
		file->overwrite_hideshell((val == "true") ? true : false);
	} else if (!strcmp(arg, "outputfile")) {
		file->overwrite_outfile(val);
	} else if (!strcmp(arg, "application")) {
		file->overwrite_application(val);
	} else {
		return false;
	}
	*--eq = tmp;

	return true;
}
