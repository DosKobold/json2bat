#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <list>
#include <string>
#include <jsoncpp/json/json.h>

#include "converter.hpp"

static void
help()
{
	std::cerr << "Usage: json2bat [options] [files...]\n"
	    "Options:\n"
	    "\t-h, --help\tprint this message and exit\n"
	    "\t-c, --console\tprint the ouput only to the console\n"
	    "\t-f, --force\tforce overwriting an existing file\n"
	    "\t-s, --stdout\tprint the input information formated to the console\n"
	    "\t\t\tbut still create the batch file\n"
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
	bool sflag = false;
	int opt;
	int ret = 0;
	int option_index = 0;
	char *ow = nullptr;
	Converter converter;

	struct option long_options[] = {
		{"help",      no_argument,       0, 'h'},
		{"console",   no_argument,       0, 'c'},
		{"force",     no_argument,       0, 'f'},
		{"stdout",    no_argument,       0, 's'},
		{"overwrite", required_argument, 0, 'o'},
		{0,         0,                   0,  0 },
	};

	while ((opt = getopt_long(argc, argv, "hcfso:", long_options, &option_index)) != -1) {
		switch (opt) {
			case 'c':
				converter.out_to_file(false);
				break;
			case 'f':
				converter.force_overwrite(true);
				break;
			case 's':
				sflag = true;
				break;
			case 'o':
				ow = optarg;
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
		converter.clear_file();
		if (std::filesystem::exists(*argv)) {
			if (!converter.parse_json(*argv)) {
				ret = 1;
				continue;
			}
			if (ow && !converter.overwrite(ow)) {
				std::cerr << "ERROR: [" << *argv << "] Invalid value for overwrite" << std::endl;
				return 1;
			}
			if (!converter.write_bat()) {
				ret = 1;
				continue;
			}
			if (sflag) {
				converter.print_fmt();
			}
		} else {
			std::cerr << "ERROR: [" << *argv << "] The given file does not exist!" << std::endl;
			ret = 1;
			continue;
		}
	}
	return ret;
}
