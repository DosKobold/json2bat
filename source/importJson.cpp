#include <iostream>
#include <string>
#include <fstream>
#include <jsoncpp/json/json.h>

//----- Class -----

class jsonFile {
	public:
		int convertJson(std::string inFile);
	private:
		Json::Reader reader;
		Json::Value object;
		
		Json::Value outFile;
		Json::Value hideshell;
		Json::Value entries;
};

//----- Functions of class -----

int jsonFile::convertJson(std::string inFile) {
	std::ifstream file(inFile);
	if(!reader.parse(file, object)) {
		std::cout << "ERROR: Could not parse file into object!" << std::endl;
		return 1;
	}

	outFile = object["outputfile"];
	hideshell = object["hideshell"];
	entries = object["entries"];

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

	return 0;
}

//----- Main -----

int main() {
	jsonFile convertation;

	if(convertation.convertJson("MinGW.json")) {
		std::cout << "ERROR: Could not convert json to batch!" << std::endl;
	}
	return 0;
}
