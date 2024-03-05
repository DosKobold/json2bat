#ifndef JSONFILE_HXX_
#define JSONFILE_HXX_

#include <string>

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

		std::list<std::string> commands;
		std::list<std::string> keys;
		std::list<std::string> values;
		std::list<std::string> paths;
};

int convertJson(std::string);
void verbose();

#endif /* JSONFILE_HXX_ */
