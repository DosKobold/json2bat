#ifndef FILE_HPP_
#define FILE_HPP_

#include <list>

class File {
	public:
		void add_command(std::string);
		void add_pair(std::string, std::string);
		void add_path(std::string);
		std::list<std::string> get_commands();
		std::list<std::string> get_paths();
		std::list<std::string> *get_keys();
		std::list<std::string> *get_values();

	public:
		std::string inFile;
		Json::Value object;
		Json::Value outFile;
		Json::Value hideshell;
		Json::Value application;
		Json::Value entries;
	private:
		std::list<std::string> commands;
		std::list<std::string> keys;
		std::list<std::string> values;
		std::list<std::string> paths;
};

#endif /* FILE_HPP_ */
