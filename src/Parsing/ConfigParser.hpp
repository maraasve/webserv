#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include "../Server/Server.hpp"
#include "./ConfigTokenizer.hpp"

class ConfigParser
{
private:
	std::vector<Server> servers;
	ConfigTokenizer Tokenizer;

public:
	ConfigParser(const std::string &filename);

	std::string loadFileAsString(std::ifstream &file);
	void parseServer(std::vector<Token> tokens);
	// void parseLocation(std::ifstream &file, Location &location);

	// u_long convertHost(const std::string &host);

	const std::vector<Server> &getServers() const;
	void error_check(const std::string &msg) const;
};

#endif