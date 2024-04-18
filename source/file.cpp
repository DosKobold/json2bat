#include <iostream>
#include <list>
#include <string>
#include <jsoncpp/json/json.h>

#include "file.hpp"

bool
File::fill(Json::Value& objects, std::string in, std::ostream& errout)
{
	m_inFile = in;

	if (objects["outputfile"] && objects["outputfile"] != "") {
		m_outFile = objects["outputfile"].asString();
	} else {
		errout << "ERROR: [" << m_inFile << "] Object \"outputfile\"" \
		    " does not exist or is empty!" << std::endl;
		return false;
	}

	if (objects["hideshell"] && objects["hideshell"].isBool()) {
		m_hideshell = objects["hideshell"].asBool();
	} else {
		errout << "ERROR: [" << m_inFile << "] Object \"hideshell\"" \
		    " does not exist or is no boolean!" << std::endl;
		return false;
	}


	/* application isn't mandatory and can either be NULL or omitted. */
	m_application = objects["application"].asString();

	/* Same for 'entries'. */
	m_entries = objects["entries"];
	for (auto entry : m_entries) {
		if (entry["type"].asString() == "EXE") {
			m_commands.push_back(entry["command"].asString());
		} else if (entry["type"].asString() == "ENV") {
			m_keys.push_back(entry["key"].asString());
			m_values.push_back( entry["value"].asString());
		} else if (entry["type"].asString() == "PATH") {
			m_paths.push_back(entry["path"].asString());
		}
	}
	return true;
}

void
File::iterate_env(std::ostream& out) const
{
	bool first  = true;
	auto key    = this->m_keys.begin();
	auto keyEnd = this->m_keys.end();
	auto val    = this->m_values.begin();
	auto valEnd = this->m_values.end();

	for (; key!=keyEnd && val!=valEnd; ++key, ++val) {
		if (first && m_commands.empty()) {
			out << "set ";
		} else {
			out << " && set ";
		}
		out << *key << "=" << *val;
		first = false;
	}
}

void
File::iterate_env(std::ostream& out, const std::string& k, const std::string& v) const
{
	auto key    = this->m_keys.begin();
	auto keyEnd = this->m_keys.end();
	auto val    = this->m_values.begin();
	auto valEnd = this->m_values.end();

	for (; key!=keyEnd && val!=valEnd; ++key, ++val) {
		out << k << *key << v << *val << std::endl;
	}
}

void
File::iterate_commands(std::ostream& out) const
{
	bool first = true;

	for (auto cmd : this->m_commands) {
		if (!first) {
			out << " && ";
		}
		out << cmd;
		first = false;
	}
}

void
File::iterate_commands(std::ostream& out, const std::string& sep) const
{
	for (auto cmd : this->m_commands) {
		out << sep << cmd << std::endl;
	}
}

void
File::iterate_paths(std::ostream& out) const
{
	if (this->m_paths.empty()) {
		return;
	}

	out << " && set path=";
	for (auto path : this->m_paths) {
		out << path << ";";
	}
	out << "\%path\%";
}

void
File::iterate_paths(std::ostream& out, const std::string& sep) const
{
	for (auto path : this->m_paths) {
		out << sep << path << std::endl;
	}
}

std::string
File::title() const
{
	return this->m_outFile.substr(0, this->m_outFile.find_last_of("."));
}

void
File::overwrite_hideshell(bool hideshell)
{
	this->m_hideshell = hideshell;
}

void
File::overwrite_application(std::string application)
{
	this->m_application = application;
}

void
File::overwrite_outfile(std::string outFile)
{
	this->m_outFile = outFile;
}

bool
File::hideshell() const
{
	return this->m_hideshell;
}

const std::string&
File::application() const
{
	return this->m_application;
}

const std::string&
File::outfile() const
{
	return this->m_outFile;
}
