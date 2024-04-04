#ifndef JSONFILE_HXX_
#define JSONFILE_HXX_

#include <string>

class Converter {
	public:
		Converter();

		int8_t parse_json(std::string);
		int8_t write_bat();
		void outfmt();
		void to_file(bool);
		bool is_overwritable(char *);
	private:
		bool toFile;

		std::string inFile;

		Json::Reader reader;
		Json::Value object;

		Json::Value outFile;
		Json::Value hideshell;
		Json::Value application;
		Json::Value entries;

		std::list<std::string> commands;
		std::list<std::string> keys;
		std::list<std::string> values;
		std::list<std::string> paths;
};

#endif /* JSONFILE_HXX_ */
