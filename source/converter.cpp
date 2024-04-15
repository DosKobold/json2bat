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

bool
Converter::parse_json(std::string inFile)
{
	file->inFile = inFile;
	std::ifstream input(inFile);

	if (!reader.parse(input, file->object)) {
		std::cerr << "ERROR: [" << file->inFile  << \
		    "] Could not parse file into object! Following message is " \
		    "provided by the parser: " << std::endl;
		std::cerr << reader.getFormattedErrorMessages();
		return false;
	}

	/* Make sure overwriting works. */
	if (!file->outFile) {
		if (file->object["outputfile"] && file->object["outputfile"] != "") {
			file->outFile = file->object["outputfile"];
		} else {
			std::cerr << "ERROR: [" << file->inFile << "] Object \"outputfile\"" \
			    " does not exist or is empty!" << std::endl;
			return false;
		}
	}

	if (!file->hideshell) {
		if (file->object["hideshell"] && (file->object["hideshell"] == false || \
		    file->object["hideshell"] == true)) {
			file->hideshell = file->object["hideshell"];
		} else {
			std::cerr << "ERROR: [" << file->inFile << "] Object \"hideshell\"" \
			    " does not exist or is no boolean!" << std::endl;
			return false;
		}
	}

	if (!file->application) {
		file->application = file->object["application"];
	}
	file->entries = file->object["entries"];

	for (auto entry : file->entries) {
		if (entry["type"].asString() == "EXE") {
			file->add_command(entry["command"].asString());
		} else if (entry["type"].asString() == "ENV") {
			file->add_pair(entry["key"].asString(), entry["value"].asString());
		} else if (entry["type"].asString() == "PATH") {
			file->add_path(entry["path"].asString());
		}
	}

	return true;
}

bool
Converter::write_bat()
{
	std::streambuf *sbuf;
	std::ofstream tmp;

	/* In case the filename already exists, ask if overwriting is okay. */
	if (std::filesystem::exists(file->outFile.asString()) && !forceOW) {
		char yn;
		do {
			std::cerr << "Overwrite existing file [" << file->outFile.asString() \
			    << "]? (y/n): ";
			std::cin >> yn;
		} while (yn != 'y' && yn != 'n');
		if (yn == 'n')
			return true;
	}

	/*
	 * By default std::cout can't be assigned to std::ofstream.
	 * This limitation would make it impossible to use a single
	 * object for files and stdout. To avoid this, take the
	 * underlying buffer out of the file and move it to
	 * a new object with a datatype that can represent both.
	 */
	if (writeToFile) {
		tmp.open(file->outFile.asString());
		sbuf = tmp.rdbuf();
	} else {
		sbuf = std::cout.rdbuf();
	}
	std::ostream output(sbuf);

	/* Basic setup */
	output << "@ECHO OFF\r\n" << "C:\\Windows\\System32\\cmd.exe ";

	if (file->hideshell.asString() == "true") {
		output << "/c \"";
	} else {
		output << "/k \"";
	}

	bool first = true;
	/* Take care of EXE instructions */
	for (auto command : file->get_commands()) {
		if (!first)
			output << " && ";
		output << command;
		first = false;
	}

	/* Take care of ENV instructions */
	std::list<std::string>::iterator key = file->get_keys()->begin();
	std::list<std::string>::iterator value = file->get_values()->begin();
	auto keyEnd = file->get_keys()->end();
	auto valEnd = file->get_values()->end();

	first = true;
	for (; key!=keyEnd && value!=valEnd; ++key, ++value) {
		if (first && (file->get_commands()).empty()) {
			output << "set ";
		} else {
			output << " && set ";
		}
		output  << *key << '=' << *value;
		first = false;
	}

	/* Take care of PATH instructions */
	if (!(file->get_paths()).empty())
		output << " && set path=";
	for (auto path : file->get_paths()) {
		output << path << ';';
	}
	if (!(file->get_paths()).empty())
		output << "\%path\%";

	/* Take care of application */
	if (file->application) {
		/* Extract title from outputfile */
		size_t dot = file->outFile.asString().find_last_of(".");
		output << " && start \"" << file->outFile.asString().substr(0, dot) \
		    << "\" " << file->application.asString();
	}
	output << "\"\r\n@ECHO ON\r\n";

	/* Empty the old File so no values clutter a potential new one. */
	delete file;
	file = new File{};

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
	std::cout << "Content of the file \"" << file->inFile << "\": " << std::endl;
	std::cout << "Target name: " << file->outFile.asString() << std::endl;
	std::cout << "Hideshell:   " << file->hideshell.asString() << std::endl;

	for (auto command : file->get_commands()) {
		std::cout << "EXE  | " << command << std::endl;
	}
	std::list<std::string>::iterator key = file->get_keys()->begin();
	std::list<std::string>::iterator value = file->get_values()->begin();
	for (; key!=file->get_keys()->end() && value!=file->get_values()->end(); ++key, ++value) {
		std::cout << "ENV  | " << *key << ' ' << *value << std::endl;
	}
	for (auto path : file->get_paths()) {
		std::cout << "PATH | " << path << std::endl;
	}
}

bool
Converter::is_overwritable(char *arg)
{
	char *eq;

	if (!*arg && *arg != '=')
		return false;

	eq = strchr(arg, '=');
	if (!eq || !eq[1])
		return false;
	*eq++ = '\0';

	if (arg == "hideshell") {
		file->hideshell = (eq == "true") ? "true" : "false";
	} else if (arg == "outputfile") {
		file->outFile = eq;
	} else if (arg == "application") {
		file->application = eq;
	}

	return true;
}
