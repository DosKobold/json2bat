#ifndef JSONFILE_HXX_
#define JSONFILE_HXX_

#include <string>

class Converter {
	public:
		Converter();

		bool parse_json(std::string);
		bool write_bat();
		void outfmt();
		void set_toFile(bool);
		void set_forceOW(bool);
		bool is_overwritable(char *);
	private:
		bool toFile;
		bool forceOW;

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
