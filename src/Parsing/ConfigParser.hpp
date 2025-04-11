#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <set>
#include <regex>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <sstream>
#include "../Server/Server.hpp"
#include "./ConfigTokenizer.hpp"

struct ServerValidationState {
	bool has_root = false;
	bool has_index = false;
};

class ConfigParser {
private:
	int open_braces;
	std::vector<Server> servers;
	ConfigTokenizer Tokenizer;
	ServerValidationState required_directives;

public:
	ConfigParser(const std::string &filename, std::vector<Server>& webservers);

	std::string loadFileAsString(std::ifstream &file);
	void parseServer(std::vector<Token> tokens);
	void parseLocation(Location& location, std::vector<Token>::iterator& it, std::vector<Token>::iterator end);
	void parseDirectiveListen(Server& server, std::string& value, std::vector<Token>::iterator& it, std::vector<Token>::iterator end);
	std::vector<std::string> parseServerName(std::string value, std::vector<Token>::iterator& it, std::vector<Token>::iterator end);

	bool isValidPath(std::string& path);
	bool isValidServerName(std::string& server_name);
	bool isValidPort(std::string& port);
	bool isValidIndex(std::string& index);
	unsigned long long isValidClientBodySize(std::string& client_body_size);
	bool isValidErrorCode(std::string& error_code);

	u_long convertHost(const std::string &host);
	void error_check(const std::string &msg) const;

	std::string printEnum(int i);
	void printServerDetails(Server& server);

	const std::vector<Server> &getServers() const;
};

#endif