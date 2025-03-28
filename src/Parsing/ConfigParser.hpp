#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <regex>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <sstream>
#include "../Server/Server.hpp"
#include "./ConfigTokenizer.hpp"

class ConfigParser
{
private:
	int open_braces;
	std::vector<Server> servers;
	ConfigTokenizer Tokenizer;

public:
	ConfigParser(const std::string &filename);

	std::string loadFileAsString(std::ifstream &file);
	void parseServer(std::vector<Token> tokens);
	void parseLocation(std::vector<Token>::iterator& it, std::vector<Token>::iterator end);

	bool isValidPath(std::string& path);

	// u_long convertHost(const std::string &host);

	const std::vector<Server> &getServers() const;
	void error_check(const std::string &msg) const;
};

#endif