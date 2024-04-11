#ifndef CONVERTER_HPP_
#define CONVERTER_HPP_

#include <string>

#include "file.hpp"

class Converter {
	public:
		Converter();

	public:
		bool parse_json(std::string);
		bool write_bat();
		void print_fmt();
		void out_to_file(bool);
		void force_overwrite(bool);
		bool is_overwritable(char *);

	public:
		File *file;
		Json::Reader reader;

	private:
		bool writeToFile;
		bool forceOW;
};

#endif /* CONVERTER_HPP_ */
