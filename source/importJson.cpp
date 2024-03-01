//-ljsoncpp --std=c++20

#include <iostream>
#include <string>
#include <list>
#include <ranges>
#include <tuple>
#include <getopt.h>
#include <fstream>
#include <filesystem>
#include <jsoncpp/json/json.h>

//----- Class -----

class jsonFile {
	public:
		int convertJson(std::string inFile);
		void verbose();
	private:
		Json::Reader reader;
		Json::Value object;
		
		Json::Value outFile;
		Json::Value hideshell;
		Json::Value entries;

		std::list <std::string> commands;
		std::list <std::string> keys;
		std::list <std::string> values;
		std::list <std::string> paths;
};

//----- Functions of class -----

int jsonFile::convertJson(std::string inFile) {
	std::ifstream input(inFile);
	if(!reader.parse(input, object)) {
		std::cout << "ERROR: Could not parse file into object!" << std::endl;
		return 1;
	}

	outFile = object["outputfile"];
	hideshell = object["hideshell"];
	entries = object["entries"];

	for(auto entry : entries) {
		if(entry["type"].asString() == "EXE") {
			commands.push_back(entry["command"].asString());
		}
		else if(entry["type"].asString() == "ENV") {
			keys.push_back(entry["key"].asString());
			values.push_back(entry["value"].asString());
		}
		else if(entry["type"].asString() == "PATH") {
			paths.push_back(entry["path"].asString());
		}
	}

	std::ofstream output(outFile.asString());
	output << "@ECHO OFF " << "C:\\Windows\\System32\\cmd.exe ";
	if(hideshell.asString() == "true") {
		output << "/c \"";
	}
	else {
		output << "/k \"";
	}
	for(auto command : commands) {
		output << command << " && "; //&& is dangerous, when there is only ONE element
	}
	/*for(auto key : keys) {
		output << "set " << key << '=' << " && "; //<< value;
	}*/
	for(std::tuple<std::string&, std::string&> i : std::ranges::view::zip(keys, values)){
		//std::cout << key << value;
	}
	output << "set path=";
	for(auto path : paths) {
		output << path << ';';
	}
	output << "\%path\%\"" << " @ECHO ON";
	

	return 0;
}

//Complete function has to be changed, cause of putting data into lists
void jsonFile::verbose() {
	std::cout << "Content of the file:" << std::endl;
	std::cout << outFile.asString() << std::endl;
	std::cout << hideshell.asString() << std::endl;
	for(auto entry : entries) {
		std::cout << entry["type"].asString();
		if(entry["type"] == "EXE") {
			std::cout << " | " << entry["command"].asString();
		}
		else if(entry["type"] == "ENV") {
			std::cout << " | " << entry["key"].asString() << " | " << entry["value"].asString();
		}
		else if(entry["type"] == "PATH") {
			std::cout << " | " << entry["path"].asString();
		}
		else {
			std::cout << std::endl << "WARNING: There is unknown data in the json";
		}
		std::cout << std::endl;
	}
}

//----- Main -----

int main(int argc, char *argv[]) {
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{0, required_argument, 0, 0},
	};

	int opt;
	int option_index = 0;

	while((opt = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
		switch(opt) {
			case 'h':
				std::cout << "Placeholder: Helptext" << std::endl;
				return 0;
			default:
				std::cout << "Placeholder: Default on Error" << std::endl;
				return 1;
		}
	}

	for(int index = optind; index < argc; index++) {
		jsonFile convertation;

		if(std::filesystem::exists(argv[index])) {
			if(convertation.convertJson(argv[index])) {
				std::cout << "ERROR: Could not convert json to batch!" << std::endl;
				return 1;
			}
		}
		else {
			std::cout << "ERROR: The given file does not exist!" << std::endl;
			return 1;
		}
	}
	return 0;
}
