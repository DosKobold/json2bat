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
		std::cerr << "ERROR: Could not parse file into object! Following message is provided by the parser: " << std::endl;
		std::cerr << reader.getFormattedErrorMessages();
		return 1;
	}

	/* Make sure overwriting works. */
	if (!outFile) {
		if (object["outputfile"] && object["outputfile"] != "") {
			outFile = object["outputfile"];
		} else {
			std::cerr << "ERROR: Object \"outputfile\" does not exist or is empty!" << std::endl;
			return 1;
		}
	}

	if (!hideshell) {
		if (object["hideshell"] && (object["hideshell"] == false || object["hideshell"] == true)) {
			hideshell = object["hideshell"];
		} else {
			std::cerr << "ERROR: Object \"hideshell\" does not exist or is no boolean!" << std::endl;
			return 1;
		}
	}

	if (!application) {
		application = object["application"];
	}
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
	output << "@ECHO " << (hideshell == true ? "OFF" : "ON") << \
	    " C:\\Windows\\System32\\cmd.exe ";

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
	output << "\%path\%";

	/* Take care of application */ //WARNING: It it is allowed to be NULL (ReqFunc25)
	size_t dot = outFile.asString().find_last_of(".");
	output << " && start \"" << outFile.asString().substr(0, dot) << "\" " \
	    << application.asString();
	output << "\" @ECHO ON\r\n";

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

	if (arg == "hidshell") {
		this->hideshell = (eq == "true") ? "true" : "false";
	} else if (arg == "outputfile") {
		this->outFile = eq;
	} else if (arg == "application") {
		this->application = eq;
	}

	return true;
}
