#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <list>
#include <string>
#include <json/json.h>

#include "jsonFile.h"

int
jsonFile::convertJson(std::string inFile)
{
	std::ifstream input(inFile);
	if (!reader.parse(input, object)) {
		std::cout << "ERROR: Could not parse file into object!" << std::endl;
		return 1;
	}

	outFile = object["outputfile"];
	hideshell = object["hideshell"];
	entries = object["entries"];

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

	std::ofstream output(outFile.asString());
	output << "@ECHO OFF " << "C:\\Windows\\System32\\cmd.exe ";
	if (hideshell.asString() == "true") {
		output << "/c \"";
	} else {
		output << "/k \"";
	}
	for (auto command : commands) {
		output << command << " && "; //&& is dangerous, when there is only ONE element
	}
	std::list<std::string>::iterator key = keys.begin();
	std::list<std::string>::iterator value = values.begin();
	for (; key!=keys.end() && value!=values.end(); ++key, ++value) {
		output << "set " << *key << '=' << *value << " && ";
	}
	output << "set path=";
	for (auto path : paths) {
		output << path << ';';
	}
	output << "\%path\%\"" << " @ECHO ON";

	return 0;
}

//Complete function has to be changed, cause of putting data into lists + is shit because of 2nd iteration!!!
void
jsonFile::verbose()
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
