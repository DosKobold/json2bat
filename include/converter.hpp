#ifndef CONVERTER_HPP_
#define CONVERTER_HPP_

#include <string>

#include "file.hpp"

class Converter {
public:
	Converter();
	~Converter();

	/* Methods for controlling the JSON to BATCH conversion process. */
	bool parse_json(std::string);
	bool write_bat();
	void print_fmt();
	void clear_file();

	/* Methods for configuring the output behaviour. */
	void out_to_file(bool);
	void force_overwrite(bool);
	bool overwrite(char *);

private:
	Json::Reader reader;
	File *file;
	bool writeToFile;
	bool forceOW;
};

#endif /* CONVERTER_HPP_ */
