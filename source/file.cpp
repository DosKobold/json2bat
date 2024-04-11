#include <string>
#include <iostream>
#include <jsoncpp/json/json.h>

#include "file.hpp"

void
File::add_command(std::string cmd)
{
	this->commands.push_back(cmd);
}

void
File::add_pair(std::string key, std::string value)
{
	this->keys.push_back(key);
	this->values.push_back(value);
}

void
File::add_path(std::string path)
{
	this->paths.push_back(path);
}

std::list<std::string>
File::get_commands()
{
	return this->commands;
}

std::list<std::string> *
File::get_keys()
{
	return &this->keys;
}

std::list<std::string> *
File::get_values()
{
	return &this->values;
}

std::list<std::string>
File::get_paths()
{
	return this->paths;
}
