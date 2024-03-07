#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <list>
#include <string>
#include <jsoncpp/json/json.h>

#include "converter.h"

Converter::Converter()
{
	toFile = true;
}

int8_t
Converter::parse_json(std::string inFile)
{
	this->inFile = inFile;
	std::ifstream input(inFile);

	if (!reader.parse(input, object)) {
		std::cerr << "ERROR: Could not parse file into object!" << std::endl;
		return 1;
	}

	outFile   = object["outputfile"];
	hideshell = object["hideshell"];
	entries   = object["entries"];

	for (auto entry : entries) {
		if (entry["type"].asString() == "EXE") {
			commands.push_back(entry["command"].asString());
		} else if (entry["type"].asString() == "ENV") {
			keys.push_back(entry["key"].asString());
			values.push_back(entry["value"].asString());
		} else if (entry["type"].asString() == "PATH") {
			paths.push_back(entry["path"].asString());
		}
	}

	return 0;
}

int8_t
Converter::write_bat()
{
	std::streambuf *sbuf;
	std::ofstream tmp;

	/*
	 * By default std::cout can't be assigned to std::ofstream.
	 * This limitation would make it impossible to use a single
	 * object for files and stdout. To avoid this, take the
	 * underlying buffer out of the file and move it to
	 * a new object with a datatype that can represent both.
	 */
	if (toFile) {
		tmp.open(outFile.asString());
		sbuf = tmp.rdbuf();
	} else {
		sbuf = std::cout.rdbuf();
	}
	std::ostream output(sbuf);

	/* Basic setup */
	output << "@ECHO OFF " << "C:\\Windows\\System32\\cmd.exe ";
	if (hideshell.asString() == "true") {
		output << "/c \"";
	} else {
		output << "/k \"";
	}

	bool first = true;
	/* Take care of EXE instructions */
	for (auto command : commands) {
		if (!first)
			output << " && ";
		output << command;
		first = false;
	}

	/* Take care of ENV instructions */
	std::list<std::string>::iterator key = keys.begin();
	std::list<std::string>::iterator value = values.begin();
	for (; key!=keys.end() && value!=values.end(); ++key, ++value) {
		output << " && set " << *key << '=' << *value;
	}

	/* Take care of PATH instructions */
	output << " && set path=";
	for (auto path : paths) {
		output << path << ';';
	}
	output << "\%path\%\"" << " @ECHO ON";

	return 0;
}

void
Converter::to_file(bool b)
{
	toFile = b;
}

void
Converter::outfmt()
{
	std::cout << "Content of the file \"" << inFile << "\": " << std::endl;
	std::cout << "Target name: " << outFile.asString() << std::endl;
	std::cout << "Hideshell:   " << hideshell.asString() << std::endl;

	for (auto command : commands) {
		std::cout << "EXE  | " << command << std::endl;
	}
	std::list<std::string>::iterator key = keys.begin();
	std::list<std::string>::iterator value = values.begin();
	for (; key!=keys.end() && value!=values.end(); ++key, ++value) {
		std::cout << "ENV  | " << *key << ' ' << *value << std::endl;
	}
	for (auto path : paths) {
		std::cout << "PATH | " << path << std::endl;
	}
}
