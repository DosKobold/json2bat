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

std::size_t
Converter::get_lineno(std::ifstream& in, std::string inval) const
{
	std::size_t lineno;
	std::string line{};

	/* Make sure no random substring is accidentally matched.*/
	inval.append("\",").insert(0, "\"");

	/* Reset input stream so the whole file can be read. */
	in.clear();
	in.seekg(0, std::ios::beg);

	for (lineno = 1; std::getline(in, line); ++lineno) {
		if (line.find(inval) != std::string::npos) {
			break;
		}
	}
	return lineno;
}

bool
Converter::check_error(std::ifstream& in, const Json::Value& obj, std::list<std::string> valid) const
{
	std::string unequal{};

	/**
	 * Check array entries for errors.
	 */
	if (obj.isArray()) {
		for (auto arr : obj) {
			unequal.clear();
			/* All entries have to start with "type". */
			if (arr["type"].asString() == "") {
				std::cout << "ERROR: entries has to start with \"types\"" << \
				    std::endl;
				return true;
			}
			for (auto str : valid) {
				/* Name matches and is thus valid. */
				if (str == arr["type"].asString()) {
					unequal = str;
					break;
				}
			}
			/* No match indicates an error. */
			if (unequal.empty()) {
				std::cout << "ERROR: [" << arr["type"].asString() << \
				    "] invalid entry at line " << \
				    get_lineno(in, arr["type"].asString()) << std::endl;;
				return true;
			}
		}
		return false;
	}

	/**
	 * Check normal entries for errors.
	 */
	for (auto name : obj.getMemberNames()) {
		unequal.clear();
		for (auto str : valid) {
			/* Name matches and is thus valid. */
			if (str == name) {
				unequal = str;
				break;
			}
		}
		/* No match indicates an error, abort iteration. */
		if (unequal.empty()) {
			std::cout << "ERROR: [" << name << "] invalid entry at line " << \
			    get_lineno(in, name) << std::endl;
			return true;
		}
	}

	/* Zero indicates success. */
	return unequal.empty();
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

	/*
	 * These lists contain all valid entries that are allowed in the JSON file.
	 * This way, it's possible to add more fields and functionality without
	 * needing to change the source in a complicated manner.
	 */
	std::list<std::string> root = {"outputfile", "hideshell", "entries", \
	    "application"};
	std::list<std::string> entries = {"EXE", "ENV", "PATH"};

	/* Fail on the first invalid entry. */
	if (check_error(input, object, root)) {
		return false;
	}
	if (check_error(input, object["entries"], entries)) {
		return false;
	}
	if (!file->initialize(object, inFile, std::cerr)) {
		return false;
	}
	return true;
}

bool
Converter::write_bat() const
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
			return false;
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
	if (file->paths_size() != 0) {
		if (file->commands_size() > 0 || file->env_size() > 0) {
			output << " && ";
		}
		output << "set path=";
		file->iterate_paths(output, ";", true);
		output << ";%path%";
	}

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
Converter::print_fmt() const
{
	std::cout << "Target name: " << file->outfile() << std::endl;
	std::cout << "Hideshell:   " << std::boolalpha << file->hideshell() << std::endl;
	std::cout << "Application: " << file->application();

	file->iterate_commands(std::cout, "\nEXE   | ", false);
	file->iterate_env(std::cout, "\nENV   | ", false);
	file->iterate_paths(std::cout, "\nPATH  | ", false);
	std::cout << std::endl;
}

bool
Converter::overwrite(char *arg) const
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
