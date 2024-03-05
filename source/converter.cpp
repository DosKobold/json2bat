#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <list>
#include <string>
#include <json/json.h>

#include "converter.h"

Converter::Converter()
{
	verbose = true;
}

int8_t
Converter::parse_json(std::string inFile)
{
	std::ifstream input(inFile);

	if (!reader.parse(input, object)) {
		std::cout << "ERROR: Could not parse file into object!" << std::endl;
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
	bool first = true;
	std::ofstream output(outFile.asString());

	/* Basic setup */
	output << "@ECHO OFF " << "C:\\Windows\\System32\\cmd.exe ";
	if (hideshell.asString() == "true") {
		output << "/c \"";
	} else {
		output << "/k \"";
	}

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
Converter::set_verbose(bool v)
{
	verbose = v;
}

//Complete function has to be changed, cause of putting data into lists + is shit because of 2nd iteration!!!
void
Converter::fverbose()
{
	std::cout << "Content of the file:" << std::endl;
	std::cout << outFile.asString() << std::endl;
	std::cout << hideshell.asString() << std::endl;

	for (auto entry : entries) {
		std::cout << entry["type"].asString();
		if (entry["type"] == "EXE") {
			std::cout << " | " << entry["command"].asString();
		} else if (entry["type"] == "ENV") {
			std::cout << " | " << entry["key"].asString() << " | " << entry["value"].asString();
		} else if (entry["type"] == "PATH") {
			std::cout << " | " << entry["path"].asString();
		} else {
			std::cout << std::endl << "WARNING: There is unknown data in the json";
		}
		std::cout << std::endl;
	}
}
