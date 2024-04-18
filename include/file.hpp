#ifndef FILE_HPP_
#define FILE_HPP_

#include <list>
#include <string>
#include <jsoncpp/json/json.h>

class File {
public:
	bool fill(Json::Value&, std::string, std::ostream&);

	/* Formatted output. */
	void iterate_env(std::ostream&, const std::string&, bool) const;
	void iterate_commands(std::ostream&, const std::string&, bool) const;
	void iterate_paths(std::ostream&, const std::string&, bool) const;
	std::string title() const;

	/* Manupulate fields after initial File::fill(). */
	void overwrite_hideshell(bool);
	void overwrite_application(std::string);
	void overwrite_outfile(std::string);

	/* Receive information from outside. */
	bool hideshell() const;
	const std::string& application() const;
	const std::string& outfile() const;
	std::size_t env_size() const;
	std::size_t paths_size() const;
	std::size_t commands_size() const;

private:
	std::list<std::string> m_commands;
	std::list<std::string> m_keys;
	std::list<std::string> m_values;
	std::list<std::string> m_paths;
	Json::Value m_entries;
	std::string m_inFile;
	std::string m_outFile;
	std::string m_application;
	bool m_hideshell;
};

#endif /* FILE_HPP_ */
