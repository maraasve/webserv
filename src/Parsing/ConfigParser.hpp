#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include "../Server/Server.hpp"

class ConfigParser {
private:
	std::vector<Server> servers;

public:
	ConfigParser(const std::string& filename);

	int countServerBlocks(std::ifstream& file);
	void parseServer(std::ifstream& file, Server& server);
	void parseLocation(std::ifstream& file, Location& location);

	u_long convertHost(const std::string& host);

	const std::vector<Server>& getServers() const;
	void error_check(const std::string& msg) const;
};

#endif