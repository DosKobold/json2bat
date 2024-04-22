#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
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
Converter::get_lineno(std::ifstream& in, const std::string& inval) const
{
	std::size_t lineno;
	std::string line{};

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
Converter::check_error(std::string inFile, std::ifstream& in, const Json::Value& obj, \
     std::map<std::string, std::string>& valid) const
{
	bool unequal;
	std::string type{};
	std::string pattern{};

	/**
	 * Check array entries for errors.
	 */
	if (obj.isArray()) {
		for (auto& entry : obj) {
			/* Make sure the type is valid [ENV|EXE|PATH]. */
			type = entry["type"].asString();
			if (!type.empty() && valid[type] == "") {
				std::cout << "ERROR: [" << inFile << "] Invalid entry at " \
				    << "line " << get_lineno(in, type) << ": " << type << std::endl;
				return true;
			}
			for (auto& c : entry.getMemberNames()) {
				/*
				 * In case an entry doesn't contain a "type" field, it's
				 * invalid. To detect a misspelled type-field, insert
				 * the current key into 'entry' and the resulting value into
				 * the map holding all valid entry combinations.
				 * In case the type-field is found, 'entry[c]' results in
				 * either ENV, EXE or PATH which are valid keys for the
				 * 'valid' map. Thus resulting in a return value unequal to
				 * empty string and meeting the condition.
				 */
				if (type.empty() && valid[entry[c].asString()] != "") {
					/* Make sure no random substring is matched.*/
					pattern.append("\"").append(c).append("\": \"") \
					    .append(entry[c].asString()).append("\"");
					std::cout << "ERROR: [" << inFile << "] Expected \"" << type << "\" but got \"" \
					    << c << "\" at line " << get_lineno(in, pattern) \
					    << std::endl;
					return true;
				}
				/* Make sure each type is only followed by valid data. */
				if (!type.empty()) {
					// TODO: special case, integrate this into the maps.
					if (valid[type] == "ENV" && c == "value") {
						break;
					}
					if (valid[type] != c) {
						pattern.append("\"").append(type).append("\", \"") \
						    .append(c).append("\":");
						std::cout << "ERROR: [" << inFile <<"] Expected \"" << valid[type] << "\"" \
						    << " but got \"" << c << "\" at line " \
						    << get_lineno(in, pattern) << std::endl;
						return true;
					}
					break;
				}
			}
		}
		return false;
	}

	/**
	 * Check normal entries for errors.
	 */
	for (auto& name : obj.getMemberNames()) {
		unequal = true;
		for (auto& str : valid) {
			/* Name matches and is thus valid. */
			if (str.first == name) {
				unequal = false;
				break;
			}
		}
		/* No match indicates an error, abort iteration. */
		if (unequal) {
			std::cout << "ERROR: [" << inFile << "] Invalid entry at line " << \
			    get_lineno(in, name) << ": " << name  << std::endl;
			return true;
		}
	}
	return false;
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
	 * These maps contain all valid entries that are allowed in the JSON file.
	 * They are split into the "normal" entries and "array" entries since both
	 * need slightly different treatment to function properly.
	 * This way, it's possible to add more fields and functionality without
	 * needing to change the source in a complicated manner.
	 */
	std::map<std::string, std::string> root {
	        {"outputfile", ""}, {"hideshell", ""}, {"entries", ""},
	        {"application", ""}
	    };
	std::map<std::string, std::string> entries {
	        {"EXE", "command"}, {"ENV", "key"}, {"PATH", "path"}
	    };

	/* Fail on the first invalid entry. */
	if (check_error(inFile, input, object, root)) {
		return false;
	}
	if (check_error(inFile, input, object["entries"], entries)) {
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
	bool b;
	char yn;
	std::streambuf *sbuf;
	std::ofstream tmp;

	/* In case the filename already exists, ask if overwriting is okay. */
	if (std::filesystem::exists(file->outfile()) && !forceOW && writeToFile) {
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
	b = (file->commands_size() == 0 && file->env_size() > 0);
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
	std::string val{};
	char tmp;
	char *eq;

	if (!*arg && *arg != '=') {
		return false;
	}

	eq = strchr(arg, '=');
	if (!eq || !eq[1]) {
		return false;
	}
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
