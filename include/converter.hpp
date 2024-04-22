#ifndef CONVERTER_HPP_
#define CONVERTER_HPP_

#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>

#include "file.hpp"

class Converter {
public:
	Converter();
	~Converter();

	/* Methods for controlling the JSON to BATCH conversion process. */
	bool parse_json(std::string);
	bool write_bat() const;
	void print_fmt() const;
	void clear_file();

	/* Methods for configuring the output behaviour. */
	void out_to_file(bool);
	void force_overwrite(bool);
	bool overwrite(char *) const;

private:
	bool check_error(std::ifstream&, const Json::Value&, std::list<std::string>) const;
	std::size_t get_lineno(std::ifstream&, std::string) const;

private:
	Json::Reader reader;
	File *file;
	bool writeToFile;
	bool forceOW;
};

#endif /* CONVERTER_HPP_ */
