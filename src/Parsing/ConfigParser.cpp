#include "ConfigParser.hpp"

static constexpr std::array<const char*, 5> TokenNames = {{ 
	"KEYWORD", "BRACE_OPEN", "BRACE_CLOSE", "COLON", "SEMI_COLON"
}};

std::unordered_map<std::string, bool> seenDirective;

std::string ConfigParser::printTokenType(int i) {
	if (i < 0 || i >= (int)TokenNames.size()) {
		return {};
	}
	return TokenNames[i];
}

template<typename T>
std::function<void(T&, TokenIt&, TokenIt&)> wrapParser(void (ConfigParser::*fn)(T&, TokenIt&, TokenIt&)) {
	return [this, fn](T& target, TokenIt& it, TokenIt& end) {
		(this->*fn)(target, it, end);
	};
}

void ConfigParser::initParsers() {
	_serverParsers["root"] = wrapParser(&ConfigParser::parseRoot<Server>);
	_serverParsers["index"] = wrapParser(&ConfigParser::parseIndex<Server>);
	_serverParsers["listen"] = wrapParser(&ConfigParser::parseListen<Server>);
	_serverParsers["host"] = wrapParser(&ConfigParser::parseHost<Server>);
	_serverParsers["server_name"] = wrapParser(&ConfigParser::parseServerName<Server>);
	_serverParsers["auto_index"] = wrapParser(&ConfigParser::parseAutoIndex<Server>);
	_serverParsers["error_page"] = wrapParser(&ConfigParser::parseErrorPage<Server>);
	_serverParsers["client_max_body"] = wrapParser(&ConfigParser::parseClientMaxBody<Server>);
	_serverParsers["location"] = [this](Server &s, TokenIt &it, TokenIt &end){ return parseLocation(s, it, end); }},

	_locationParsers[""] = wrapParser(&ConfigParser::parseRoot<Location>);
}

void ConfigParser::expectTokenType(TokenName expected_type, TokenIt& it, TokenIt& end) {
	if (it == end) {
		error("Unexpected end of tokens: expected token \"" 
			+ printTokenType(expected_type) + "\"");
	}
	if (it->token_type != expected_type) {
		std::string got = printTokenType(it->token_type);
		std::string want = printTokenType(expected_type);
		error("Unexpected token type: expected \"" 
			+ want + "\", got \"" + got + "\"");
	}
}

void ConfigParser::assertNotDuplicate(const std::string& directive) {
	if (seenDirective[directive] && RepeatableDirectives.count(directive) == 0) {
		error("Unexpected duplicate directive: " + directive);
	}
	seenDirective[directive] = true;
}

void ConfigParser::parseConfigFile(const std::vector<Token>& tokens) {
	auto it = tokens.begin();
	auto it = tokens.end();
	bool insideServerBlock = false;
	while (it != end) {
		if (it->value != "server" || insideServerBlock) {
			error("Server: missing server directive or nested server block");
		}
		servers.emplace_back();
		insideServerBlock = true;
		++it;
		expectTokenType(BRACE_OPEN, it, end);
		++open_braces;
		++it;
		parseServerBlock(servers.back(), it, end);
		expectTokenType(BRACE_CLOSE, it, end);
		--open_braces;
		if (!required_directives.has_index || !required_directives.has_root) {
			error("Missing required directives in server block");
		}
		if (open_braces == 0) {
			insideServerBlock = false;
		}
		++it;
	}
	if (open_braces != 0) {
		error("Mismatched braces in configuration");
	}
}

void ConfigParser::parseServerBlock(Server &s, TokenIt &it, TokenIt &end) {
	while (it != end || it->token_type == BRACE_CLOSE) {
		std::string directive = it->value;
		auto dpIt = _serverParsers.find(directive);
		if (dpIt == _serverParsers.end()) {
			error("Unknown directive: " + directive);
		}
		assertNotDuplicate(directive);
		++it;
		expectTokenType(KEYWORD, it, end);
		dpIt->second(servers.back(), it, end);
		expectTokenType(SEMI_COLON, it, end);
		++it;
	}
}

template<typename T>
void ConfigParser::parseListen(T &t, TokenIt &it, TokenIt &end) {
	std::string port = it->value;
	if (!isValidPort(port)) {
		error("Listen directive: invalid port number \"" + port + "\"");
	}
	t.setPort(std::stoi(port));
	++it;
}

template<typename T>
void ConfigParser::parseHost(T &t, TokenIt &it, TokenIt &end) {
	std::string host = it->value;
	t.setHost_u_long(convertHost(host));
	t.setHost_string(host);
	++it;
}

template<typename T>
void ConfigParser::parseRoot(T &t, TokenIt &it, TokenIt &end) {
	std::string root = it->value;
	if (!isValidPath(root)) {
		error("Root directive: invalid root path \"" + root + "\"");
	}
	t.setRoot(root);
	required_directives.has_root = true;
	++it;
}

template<typename T>
void ConfigParser::parseIndex(T &t, TokenIt &it, TokenIt &end) {
	std::string index = it->value;
	if (!isValidIndex(index)) {
		error("Index directive: invalid file name \"" + index + "\"");
	}
	t.setIndex(index);
	required_directives.has_index = true; //maybe there is a better way to do this?
	++it;
}

template<typename T>
void ConfigParser::parseServerNames(T &t, TokenIt &it, TokenIt &end) {
	std::vector<std::string> server_names;
	while (it != end) {
		std::string name = it->value;
		if (!isValidServerName(name)) {
			error("Server Names directive: invalid server name \"" 
				+ server_name + "\"");
		}
		server_names.push_back(name);
		++it;
		if (it->token_type == SEMI_COLON) {
			t.setServerNames(server_names);
			return;
		}
	}
	error("Server Names directive: missing semi-colon");
	
}

template<typename T>
void ConfigParser::parseAutoIndex(T &t, TokenIt &it, TokenIt &end) {
	std::string value = it->value;
	if (value != "on" && value != "off") {
		error("Auto Index directive: Invalid value \"" + value + "\"");
	}
	t.setAutoIndex(value == "on");
	++it;
}

template<typename T>
void ConfigParser::parseClientMaxBody(T &t, TokenIt &it, TokenIt &end) {
	auto bytes = isValidClientBodySize(it->value);
	if (!bytes) {
		error("Client Max Body directive: Invalid value \"" + value + "\"");
	}
	t.setClientMaxBody(bytes);
	++it;
}

template<typename T>
void ConfigParser::parseErrorPage(T &t, TokenIt &it, TokenIt &end) {
	std::string error_code = it->value;
	if (!isValidErrorCode(error_code)){
		error("Error Page: Invalid code \"" + error_code + "\"");
	}
	++it;
	expectTokenType(KEYWORD, it, end);
	std::string error_page_path = it->value;
	if (!isValidPath(error_page_path)) {
		error_check("Error Page: Invalid error page path \"" + error_page_path + "\"");
	}
	t.setErrorPage(error_code, error_page_path);
	++it;
}

Location& ConfigParser::addLocation(Server &s, const std::string& location_path) {
	auto locs = s.getLocations();
	locs.emplace_back();
	locs.back()._path = location_path;
	return locs.back();
}

void ConfigParser::parseLocationBlock(Server &s, TokenIt &it, TokenIt &end) {
	const std::string loc_path = it->value;
	if (!isValidPath(loc_path)) {
		error_check("Location directive: Invalid path \"" + loc_path + "\"");
	}
	++it;
	expectTokenType(BRACE_OPEN, it, end);
	++it;
	++open_braces; //is there a better way to do this?
	expectTokenType(KEYWORD, it, end);
	Location& loc = addLocation(s.getLocations(), loc_path);
	while (it != end && it->token_type != BRACE_CLOSE) {
		std::string directive = it->value;
		auto dp = location_parsers.find(directive);
		if (dp == location_parsers.end()) {
			error("Unknown location-directive: " + directive);
		}
		dp->second(loc, it, end);
	}
	expectTokenType(it, end, BRACE_CLOSE);
	open_braces--;
	++it;
}


/*
	What is location parsing?
	error_page
	allowed_methods
	client_max_body
	root
	index
	auto_index
	return
*/



ConfigParser::ConfigParser(const std::string &filename, std::vector<Server> &webservers)
	: open_braces(0)
	, servers(webservers)
	, _serverParsers({
		{"listen",			[this](Server &s, TokenIt &it, TokenIt &end){ return parseListen(s, it, end); }},
		{"host",			[this](Server &s, TokenIt &it, TokenIt &end){ return parseHost(s, it, end); }},
		{"root",			[this](Server &s, TokenIt &it, TokenIt &end){ return parseRoot(s, it, end); }},
		{"index",			[this](Server &s, TokenIt &it, TokenIt &end){ return parseIndex(s, it, end); }},
		{"auto_index",		[this](Server &s, TokenIt &it, TokenIt &end){ return parseAutoIndex(s, it, end); }},
		{"server_name",		[this](Server &s, TokenIt &it, TokenIt &end){ return parseServerName(s, it, end); }},
		{"location",		[this](Server &s, TokenIt &it, TokenIt &end){ return parseLocation(s, it, end); }},
		{"client_max_body", [this](Server &s, TokenIt &it, TokenIt &end){ return parseClientMaxBody(s, it, end); }},
		{"error_page",		[this](Server &s, TokenIt &it, TokenIt &end){ return parseErrorPage(s, it, end); }},
		{},


	})
	, _locationParsers({
		{"root", [this](Location &l, TokenIt &it, TokenIt &end){ return parseRoot(l, it, end); }},
		{"index", },
		{"auto_index", },
		{"return", },
		{"client_max_body", },
		{"allowed_methods", },
		{"error_page", },
		{},
		{},
		{},
		{},
		{},
		{},
		{},
		{},

	}) {
	initParsers();
	auto tokens = loadTokensFromFile(filename);
	// for (auto tokens : Tokenizer.getTokens()) {
	// 	std::cout << printEnum(tokens.token_type) << " --> " << tokens.value << std::endl;
	// }
	parseServer(tokens);
	// for(auto& server : servers) {
	// 	printServerDetails(server);
	// }		
}

std::vector<Token> ConfigParser::loadTokensFromFile(const std::string& filename) {
	std::ifstream in(filename);
	if (!in) {
		error("Could not open configuration file: " + filename);
	}
	std::string text{
		std::istreambuf_iterator<char>(in),
		std::istreambuf_iterator<char>()
	};
	if (text.empty()) {
		error("Configuration file is empty: " + filename);
	}
	auto tokens = ConfigTokenizer(text).getTokens();
	if (tokens.empty()) {
		error("No tokens found in configuration file: " + filename);
	}
	return tokens;
}




























void ConfigParser::parseLocation(Location &location, std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	std::unordered_set<std::string> valid_directives = {
			"error_page", "client_max_body", "auto_index", "root", "index", "allowed_methods", "return"};
	std::unordered_map<std::string, bool> check_duplicates = {
			{"client_max_body", false}, {"auto_index", false}, {"root", false}, {"index", false}, {"allowed_methods", false}, {"return", false}};
	while (it != end)
	{
		if (it->token_type == BRACE_CLOSE)
		{
			--open_braces;
			++it;
			return;
		}
		if (valid_directives.find(it->value) == valid_directives.end())
		{
			error_check("Unexpected directive inside location block: " + it->value);
		}
		std::string directive = it->value;
		++it;
		if (it == end || it->token_type != KEYWORD)
		{
			error_check("Missing value for directive: " + directive);
		}
		if (check_duplicates[directive] == true && directive != "error_page")
		{
			error_check("Duplicate directive inside location block: " + directive);
		}
		check_duplicates[directive] = true;
		std::string value = it->value;
		++it;
		if (directive == "error_page")
		{
			if (!isValidErrorCode(value))
			{
				error_check("Invalid error_page code: " + value);
			}
			if (it == end || it->token_type != KEYWORD || !isValidPath(it->value))
			{
				error_check("Invalid error_page path");
			}
			location._error_page.emplace(value, it->value);
			++it;
		}
		else if (directive == "allowed_methods")
		{
			std::set<std::string> unique_methods;
			for (int i = 0; i < 3; i++)
			{
				if (value != "GET" && value != "POST" && value != "DELETE")
				{
					error_check("Invalid allowed_method method: " + value);
				}
				if (!unique_methods.insert(value).second)
				{
					error_check("Duplicated method in allowed_method directive: " + value);
				}
				location._allowed_methods.push_back(value);
				if (it->token_type != KEYWORD)
				{
					break;
				}
				value = it->value;
				++it;
			}
		}
		else if (directive == "client_max_body")
		{
			unsigned long long bytes = isValidClientBodySize(value);
			if (!bytes)
			{
				error_check("Invalid client's body size: " + value);
			}
			location._client_max_body = bytes;
		}
		else if (directive == "root")
		{
			if (!isValidPath(value))
			{
				error_check("Invalid root path: " + value);
			}
			location._root = value;
			required_directives.has_root = true;
		}
		else if (directive == "index")
		{
			if (!isValidIndex(value))
			{
				error_check("Invalid index file: " + value);
			}
			location._index = value;
			required_directives.has_index = true;
		}
		else if (directive == "auto_index")
		{
			if (value != "on" && value != "off")
			{
				error_check("Invalid auto_index value: " + value);
			}
			location._auto_index = (value == "on");
		}
		else if (directive == "return") 
		{
			if (value != "301" && value != "302")
			{
				error_check("Invalid redirection value: " + value);
			}
			if (it->token_type == KEYWORD && (it->value == "http" || it->value == "https")) {
				std::string uri_redirection;
				uri_redirection.append(it->value);
				++it;
				if(it->token_type != COLON) {
					break;
				}
				uri_redirection.append(it->value);
				++it;
				if (it->token_type != KEYWORD) {
					break;
				}
				uri_redirection.append(it->value);
				location._redirection = std::make_pair(value, uri_redirection);
				++it;
			}
			else if (it->token_type == KEYWORD && it->value[0] == '/') {
				location._redirection = std::make_pair(value, it->value);
				++it;
			} else {
				error_check("Wrongly formated redirection directive return");
			}
		}
		if (it == end || it->token_type != SEMI_COLON)
		{
			error_check("Missing semicolon for directive: " + directive);
		}
		++it;
	}
	error_check("Missing closing braces");
}


bool ConfigParser::isValidServerName(std::string &server_name) {
	if (server_name == "localhost") {
		return true;
	}
	const std::regex domain_name(R"(^(?!-)([a-zA-Z0-9-]{1,63}\.)+[a-zA-Z0-9-]{1,63}$)");
	return std::regex_match(server_name, domain_name);
}

bool ConfigParser::isValidPort(std::string &port) {
	const std::regex port_regex(R"(^([1-9][0-9]*)$)");
	if (!std::regex_match(port, port_regex)) {
		return false;
	}
	try {
		int num = std::stoi(port);
		if (num <= 0 || num > 65535) {
			return false;
		}
		return true;
	}
	catch (...) {
		return false;
	}
}

bool ConfigParser::isValidErrorCode(std::string &error_code) {
	const std::regex error_code_regex(R"(^(?:400|403|404|405|408|413|500|502|503|504|505)$)");
	return std::regex_match(error_code, error_code_regex);
}

unsigned long long ConfigParser::isValidClientBodySize(std::string &client_body_size) {
	const std::regex client_body_regex(R"(^([1-9][0-9]*)([mMkKgG])$)");
	std::smatch match;
	if (std::regex_match(client_body_size, match, client_body_regex)) {
		try {
			char unit = match[2].str()[0];
			unsigned long long num = std::stoull(match[1]); // check this
			switch (unit) {
			case 'k':
			case 'K':
				num *= 1024;
				break;
			case 'm':
			case 'M':
				num *= 1024 * 1024;
				break;
			case 'g':
			case 'G':
				num *= 1024 * 1024 * 1024;
				break;
			default:
				return 0;
			}
			if (num > 4294967296){
				return 0;
			}
			return num;
		}
		catch (...) {
			return 0;
		}
	}
	return 0;
}

bool ConfigParser::isValidIndex(std::string &index) {
	const std::regex index_regex(R"(^[0-9a-zA-Z][a-zA-Z0-9_\-]*\.(?:html?|php?|py?|jpg)$)");
	return std::regex_match(index, index_regex);
}

bool ConfigParser::isValidPath(std::string &path) {
	if (path == "/") {
		return true;
	}
	const std::regex path_regex(R"(^\/([a-zA-Z0-9_\-\.]+\/?)*$)");
	return std::regex_match(path, path_regex);
}

u_long ConfigParser::convertHost(const std::string &host) {

	std::vector<int> bytes;
	u_long ip{0};
	std::stringstream ss(host);
	std::string ip_segment;

	if (host == "localhost") {
		ip = INADDR_ANY;
		return ip;
	}
	while (std::getline(ss, ip_segment, '.')) {
		try {
			int num = std::stoi(ip_segment);
			if (num < 0 || num > 255) {
				error("Invalid IP Number: " + ip_segment);
			}
			bytes.push_back(num);
		}
		catch (...) {
			error("Invalid IP Number: " + ip_segment);
		}
	}
	if (bytes.size() != 4) {
		error("Invalid IP Length: " + host);
	}
	ip = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	return ip;
}

const std::vector<Server> &ConfigParser::getServers() const {
	return this->servers;
}

void ConfigParser::error(const std::string &msg) const {
	throw std::runtime_error("Error: " + msg);
}
