#ifndef CONVERTER_HPP_
#define CONVERTER_HPP_

#include <string>

#include "file.hpp"

class Converter {
	public:
		Converter();
		~Converter();

		bool parse_json(std::string);
		bool write_bat();
		void print_fmt();
		void clear_file();
		void out_to_file(bool);
		void force_overwrite(bool);
		bool overwrite(char *);

	private:
		File *file;
		Json::Reader reader;
		bool writeToFile;
		bool forceOW;
};

#endif /* CONVERTER_HPP_ */
