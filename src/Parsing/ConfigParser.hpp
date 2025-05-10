#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <set>
#include <regex>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <array>
#include <functional>
#include <sstream>
#include "../Server/Server.hpp"
#include "./ConfigTokenizer.hpp"

static constexpr std::array<const char*, 5> TokenNames = {{ 
	"KEYWORD", "BRACE_OPEN", "BRACE_CLOSE", "COLON", "SEMI_COLON"
}};

struct ServerValidationState {
	bool has_root = false;
	bool has_index = false;
};

class ConfigParser {
private:
	using TokenIt = std::vector<Token>::iterator;
	using ServerHandler = std::function<void(Server&, TokenIt&, TokenIt&)>;
	using LocationHandler = std::function<void(Location&, TokenIt&, TokenIt&)>;
	
	std::unordered_map<std::string, ServerHandler> _serverParsers;
	std::unordered_map<std::string, LocationHandler> _locationParsers;
	std::unordered_map<std::string, bool> seenDirective;

	int open_braces;
	std::vector<Server> servers;
	ServerValidationState required_directives;

public:
	ConfigParser(const std::string &filename, std::vector<Server>& webservers);

	void	initParsers();
	void	expectTokenType(TokenType expected_type, TokenIt& it, TokenIt& end);
	void	parseConfigFile(const std::vector<Token>& tokens);
	void	parseServerBlock(Server &s, TokenIt &it, TokenIt &end);
	void	parseLocationBlock(Server &s, TokenIt &it, TokenIt &end);
	void	assertNotDuplicate(const std::string& directive);
	Location&	addLocation(Server &s, const std::string& location_path);
	std::vector<Token>	loadTokensFromFile(const std::string& filename);

	bool isValidPath(std::string& path);
	bool isValidServerName(std::string& server_name);
	bool isValidPort(std::string& port);
	bool isValidIndex(std::string& index);
	unsigned long long isValidClientBodySize(std::string& client_body_size);
	bool isValidErrorCode(std::string& error_code);

	u_long convertHost(const std::string &host);
	void error(const std::string &msg) const;

	std::string printEnum(int i);
	void printServerDetails(Server& server);
	std::string printTokenType(int i);
	const std::vector<Server> &getServers() const;

	template<typename T>
	std::function<void(T&, TokenIt&, TokenIt&)> wrapParser(void (ConfigParser::*fn)(T&, TokenIt&, TokenIt&));

	template<typename T>
	void parseReturn(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseAllowedMethods(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseListen(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseHost(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseRoot(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseIndex(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseServerNames(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseAutoIndex(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseClientMaxBody(T &t, TokenIt &it, TokenIt &end);

	template<typename T>
	void parseErrorPage(T &t, TokenIt &it, TokenIt &end);
};

#include "./ConfigParser.tpp"

#endif