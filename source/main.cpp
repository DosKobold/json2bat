#include <iostream>
#include <string>
#include <list>
#include <getopt.h>
#include <fstream>
#include <filesystem>
#include <jsoncpp/json/json.h>

#include "converter.h"

static void
help()
{
	std::cerr << "Usage: json2bat [options] [files...]\n"
	    "options:\n\t-h, --help print this message and exit\n"
	    "Authoren:\n\tTom Schwindl\t<a@a.com>\n"
	    "\tPaul Stoeckle\t<a@a.com>\n"
	    "\tElias Schnick\t<a@a.com>\n"
	    "\tBen Oeckl\t<a@a.com>\n"
	<< std::endl;
}

int
main(int argc, char *argv[])
{
	std::cout << "json2bat-converter prototype v0" << std::endl;
	int opt;
	int option_index = 0;
	Converter converter;

	struct option long_options[] = {
		{"help",   no_argument,       0, 'h'},
		{"silent", no_argument,       0, 's'},
		{0,        required_argument, 0,  0 },
	};

	while ((opt = getopt_long(argc, argv, "hs", long_options, &option_index)) != -1) {
		switch (opt) {
			case 's':
				converter.set_verbose(false);
				break;
			case 'h':
				help();
				return 0;
			default:
				return 1;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0) {
		std::cerr << "missing file" << std::endl;
		return 1;
	}

	for (; *argv; ++argv) {
		if (std::filesystem::exists(*argv)) {
			if (converter.parse_json(*argv) || converter.write_bat()) {
				std::cout << "ERROR: Could not convert json to batch!" << std::endl;
				continue;
			}
			//TESTING
			else {
				converter.fverbose();
			}
		} else {
			std::cout << "ERROR: The given file does not exist!" << std::endl;
			continue;
		}
	}

	return 0;
}
