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
	    "Options:\n"
	    "\t-h, --help\tprint this message and exit\n"
	    "\t-c, --console\tprint the ouput only to the console\n"
	    "\t-f, --format\tprint the input information to the console\n"
	    "\t-o, --overwrite\tchange the value of either 'hideshell',\n"
	    "\t\t\t'outputfile' or 'application' in the generated file.\n"
	    "\t\t\tOnly a single value can be overwritten at a time.\n"
	    "\t\t\tExample: -o hideshell=true\n"
	    "Authors:\n"
	    "\tTom Schwindl\t<schwindl@posteo.de>\n"
	    "\tPaul Stoeckle\t<paul.stoeckle@t-online.de>\n"
	    "\tElias Schnick\t<eliasschnick@t-online.de>\n"
	    "\tBen Oeckl\t<ben@oeckl.com>\n"
	<< std::endl;
}

int
main(int argc, char *argv[])
{
	bool fflag = false;
	int opt;
	int option_index = 0;
	Converter converter;

	struct option long_options[] = {
		{"help",      no_argument,       0, 'h'},
		{"console",   no_argument,       0, 'c'},
		{"format",    no_argument,       0, 'f'},
		{"overwrite", required_argument, 0, 'o'},
		{0,         0,                   0,  0 },
	};

	while ((opt = getopt_long(argc, argv, "hcfo:", long_options, &option_index)) != -1) {
		switch (opt) {
			case 'c':
				converter.to_file(false);
				break;
			case 'f':
				fflag = true;
				break;
			case 'o':
				if (!converter.overwrite(optarg)) {
					std::cerr << "ERROR: Invalid value for overwrite" << std::endl;
					return 1;
				}
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
		std::cerr << "ERROR: No file given!" << std::endl;
		return 1;
	}

	for (; *argv; ++argv) {
		if (std::filesystem::exists(*argv)) {
			if (converter.parse_json(*argv) || converter.write_bat()) {
				std::cerr << "ERROR: Could not convert json to batch!" << std::endl;
				continue;
			}
		} else {
			std::cerr << "ERROR: The given file does not exist!" << std::endl;
			continue;
		}
		if (fflag) {
			converter.outfmt();
		}
	}

	return 0;
}
