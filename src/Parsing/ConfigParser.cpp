#include "ConfigParser.hpp"

static void printLocationDetails(Location &loc)
{
	std::cout << "    Location:" << "\n";
	std::cout << "      path:            " << loc.getPath() << "\n";
	std::cout << "      root:            " << loc.getRoot() << "\n";
	std::cout << "      index:           " << loc.getIndex() << "\n";
	std::cout << "      auto_index:      " << (loc.getAutoIndex() ? "on" : "off") << "\n";
	std::cout << "      client_max_body: " << loc.getClientMaxBody() << "\n";
	std::cout << "      upload_dir:      " << loc.getUploadDir() << "\n";

	std::cout << "      allowed_methods:";
	for (const std::string &m : loc.getAllowedMethods())
	{
		std::cout << " " << m;
	}
	std::cout << "\n";

	std::cout << "      error_pages:\n";
	for (const auto &kv : loc.getErrorPage())
	{
		std::cout << "        " << kv.first
				  << " -> " << kv.second << "\n";
	}

	const std::pair<std::string, std::string> &redir = loc.getRedirection();
	if (!redir.first.empty())
	{
		std::cout << "      redirection:     "
				  << redir.first << " -> " << redir.second << "\n";
	}
	std::cout << "\n";
}

void ConfigParser::printServerDetails(Server &s)
{
	std::cout << "=== Server Details ===\n";

	std::cout << "  port:              " << s.getPort() << "\n";
	std::cout << "  host_string:       " << s.getHost_string() << "\n";
	std::cout << "  host_u_long:       " << s.getHost_u_long() << "\n";

	std::cout << "  server_names:";
	for (const std::string &name : s.getServerNames())
	{
		std::cout << " " << name;
	}
	std::cout << "\n";

	std::cout << "  root:              " << s.getRoot() << "\n";
	std::cout << "  index:             " << s.getIndex() << "\n";
	std::cout << "  auto_index:        " << (s.getAutoIndex() ? "on" : "off") << "\n";
	std::cout << "  client_max_body:   " << s.getClientMaxBody() << "\n";

	std::cout << "  error_pages:\n";
	for (const auto &kv : s.getErrorPage())
	{
		std::cout << "    " << kv.first
				  << " -> " << kv.second << "\n";
	}

	std::vector<Location> &locs = s.getLocations();
	if (!locs.empty())
	{
		std::cout << "\n  locations:\n";
		for (Location &loc : locs)
		{
			printLocationDetails(loc);
		}
	}

	std::cout << "=== End of Server ===\n\n";
}

ConfigParser::ConfigParser(const std::string &filename, std::vector<Server> &webservers) : open_braces(0), servers(webservers)
{
	initParsers();
	auto tokens = loadTokensFromFile(filename);
	parseConfigFile(tokens);
	for (auto &server : servers)
	{
		printServerDetails(server);
	}
}

std::string ConfigParser::printTokenType(int i)
{
	if (i < 0 || i >= static_cast<int>(TokenNames.size()))
	{
		return {};
	}
	return TokenNames[i];
}

void ConfigParser::initParsers()
{
	_serverParsers["root"] = wrapParser(&ConfigParser::parseRoot<Server>);
	_serverParsers["index"] = wrapParser(&ConfigParser::parseIndex<Server>);
	_serverParsers["auto_index"] = wrapParser(&ConfigParser::parseAutoIndex<Server>);
	_serverParsers["client_max_body"] = wrapParser(&ConfigParser::parseClientMaxBody<Server>);
	_serverParsers["error_page"] = wrapParser(&ConfigParser::parseErrorPage<Server>);
	_serverParsers["listen"] = wrapParser(&ConfigParser::parseListen<Server>);
	_serverParsers["host"] = wrapParser(&ConfigParser::parseHost<Server>);
	_serverParsers["server_name"] = wrapParser(&ConfigParser::parseServerNames<Server>);
	_serverParsers["location"] = [this](Server &s, TokenIt &it, TokenIt &end)
	{ parseLocationBlock(s, it, end); };

	_locationParsers["root"] = wrapParser(&ConfigParser::parseRoot<Location>);
	_locationParsers["index"] = wrapParser(&ConfigParser::parseIndex<Location>);
	_locationParsers["auto_index"] = wrapParser(&ConfigParser::parseAutoIndex<Location>);
	_locationParsers["client_max_body"] = wrapParser(&ConfigParser::parseClientMaxBody<Location>);
	_locationParsers["error_page"] = wrapParser(&ConfigParser::parseErrorPage<Location>);
	_locationParsers["allowed_methods"] = wrapParser(&ConfigParser::parseAllowedMethods<Location>);
	_locationParsers["return"] = wrapParser(&ConfigParser::parseReturn<Location>);
	_locationParsers["upload_dir"] = wrapParser(&ConfigParser::parseUploadDir<Location>);
}

void ConfigParser::expectTokenType(TokenType expected_type, TokenIt &it, TokenIt &end)
{
	if (it == end)
	{
		error("Unexpected end of tokens: expected token \"" + printTokenType(expected_type) + "\"");
	}
	if (it->token_type != expected_type)
	{
		error("Unexpected token type: expected \"" + printTokenType(expected_type) + "\", got \"" + printTokenType(it->token_type) + "\"");
	}
}

void ConfigParser::assertNotDuplicate(
	const std::string &directive,
	std::unordered_map<std::string, bool> &seenDirectives,
	const std::set<std::string> &allowDuplicates)
{
	if (seenDirectives[directive] && allowDuplicates.find(directive) == allowDuplicates.end())
	{
		error("Unexpected duplicate directive: " + directive);
	}
	seenDirectives[directive] = true;
}

void ConfigParser::resetServerRequirements()
{
	required_directives.has_index = false;
	required_directives.has_root = false;
	for (auto &pair : seenDirectiveServer)
	{
		pair.second = false;
	}
}

void ConfigParser::resetLocationRequirements()
{
	for (auto &pair : seenDirectiveLocation)
	{
		pair.second = false;
	}
}

void ConfigParser::parseConfigFile(std::vector<Token> &tokens)
{
	auto it = tokens.begin();
	auto end = tokens.end();
	bool insideServerBlock = false;
	while (it != end)
	{
		if (it->value != "server" || insideServerBlock)
		{
			error("Server: missing server directive or nested server block");
		}
		servers.emplace_back();
		resetServerRequirements();
		insideServerBlock = true;
		++it;
		expectTokenType(BRACE_OPEN, it, end);
		++open_braces;
		++it;
		parseServerBlock(servers.back(), it, end);
		expectTokenType(BRACE_CLOSE, it, end);
		--open_braces;
		if (!required_directives.has_index || !required_directives.has_root)
		{
			error("Missing required directives in server block");
		}
		if (open_braces == 0)
		{
			insideServerBlock = false;
		}
		++it;
	}
	if (open_braces != 0)
	{
		error("Mismatched braces in configuration");
	}
}

void ConfigParser::parseServerBlock(Server &s, TokenIt &it, TokenIt &end)
{
	while (it != end && it->token_type != BRACE_CLOSE)
	{
		std::string directive = it->value;
		auto parser_it = _serverParsers.find(directive);
		if (parser_it == _serverParsers.end())
		{
			error("Unknown directive: " + directive);
		}
		assertNotDuplicate(directive, seenDirectiveServer, {"location", "error_page"});
		++it;
		expectTokenType(KEYWORD, it, end);
		parser_it->second(s, it, end);
		if (directive == "location")
		{
			continue;
		}
		expectTokenType(SEMI_COLON, it, end);
		++it;
	}
}

void ConfigParser::parseLocationBlock(Server &s, TokenIt &it, TokenIt &end)
{
	std::string loc_path = it->value;
	if (!isValidPath(loc_path))
	{
		error("Location directive: Invalid path \"" + loc_path + "\"");
	}
	++it;
	expectTokenType(BRACE_OPEN, it, end);
	++it;
	++open_braces;
	expectTokenType(KEYWORD, it, end);
	resetLocationRequirements();
	s.getLocations().emplace_back();
	Location &loc = s.getLocations().back();
	loc.setPath(loc_path);
	while (it != end && it->token_type != BRACE_CLOSE)
	{
		std::string directive = it->value;
		auto parser_it = _locationParsers.find(directive);
		if (parser_it == _locationParsers.end())
		{
			error("Unknown location-directive: " + directive);
		}
		assertNotDuplicate(directive, seenDirectiveLocation, {"error_page"});
		++it;
		parser_it->second(loc, it, end);
		expectTokenType(SEMI_COLON, it, end);
		++it;
	}
	expectTokenType(BRACE_CLOSE, it, end);
	open_braces--;
	++it;
}

std::vector<Token> ConfigParser::loadTokensFromFile(const std::string &filename)
{
	std::ifstream in(filename);
	if (!in)
	{
		error("Could not open configuration file: " + filename);
	}
	std::string text{
		std::istreambuf_iterator<char>(in),
		std::istreambuf_iterator<char>()};
	if (text.empty())
	{
		error("Configuration file is empty: " + filename);
	}
	auto tokens = ConfigTokenizer(text).getTokens();
	if (tokens.empty())
	{
		error("No tokens found in configuration file: " + filename);
	}
	return tokens;
}

bool ConfigParser::isValidPath(std::string &path)
{
	if (path == "/")
	{
		return true;
	}
	const std::regex path_regex(R"(^\/([a-zA-Z0-9_\-\.]+\/?)*$)");
	return std::regex_match(path, path_regex);
}

bool ConfigParser::isValidServerName(std::string &server_name)
{
	if (server_name == "localhost")
	{
		return true;
	}
	const std::regex domain_name(R"(^(?!-)([a-zA-Z0-9-]{1,63}\.)+[a-zA-Z0-9-]{1,63}$)");
	return std::regex_match(server_name, domain_name);
}

bool ConfigParser::isValidPort(std::string &port)
{
	const std::regex port_regex(R"(^([1-9][0-9]*)$)");
	if (!std::regex_match(port, port_regex))
	{
		return false;
	}
	try
	{
		int num = std::stoi(port);
		if (num <= 0 || num > 65535)
		{
			return false;
		}
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool ConfigParser::isValidIndex(std::string &index)
{
	const std::regex index_regex(R"(^[0-9a-zA-Z][a-zA-Z0-9_\-]*\.(?:html?|php?|py?|jpg)$)");
	return std::regex_match(index, index_regex);
}

unsigned long long ConfigParser::isValidClientBodySize(std::string &client_body_size)
{
	static const std::regex pattern(R"(^([1-9][0-9]*)([mMkKgG])$)");
	static const std::unordered_map<char, unsigned long long> multipliers = {
		{'k', 1024ULL},
		{'m', 1024ULL * 1024},
		{'g', 1024ULL * 1024 * 1024}};
	std::smatch match;
	if (std::regex_match(client_body_size, match, pattern))
	{
		try
		{
			unsigned long long num = std::stoull(match[1].str());
			char unit = std::tolower(match[2].str()[0]);
			auto it = multipliers.find(unit);
			if (it == multipliers.end())
			{
				return 0;
			}
			num *= it->second;
			if (num > 4294967296ULL)
			{
				return 0;
			}
			return num;
		}
		catch (...)
		{
			return 0;
		}
	}
	return 0;
}

bool ConfigParser::isValidErrorCode(std::string &error_code)
{
	const std::regex error_code_regex(R"(^(?:400|403|404|405|408|413|500|502|503|504|505)$)");
	return std::regex_match(error_code, error_code_regex);
}

u_long ConfigParser::convertHost(const std::string &host)
{

	std::vector<int> bytes;
	u_long ip{0};
	std::stringstream ss(host);
	std::string ip_segment;

	if (host == "localhost")
	{
		ip = INADDR_ANY;
		return ip;
	}
	while (std::getline(ss, ip_segment, '.'))
	{
		try
		{
			int num = std::stoi(ip_segment);
			if (num < 0 || num > 255)
			{
				error("Invalid IP Number: " + ip_segment);
			}
			bytes.push_back(num);
		}
		catch (...)
		{
			error("Invalid IP Number: " + ip_segment);
		}
	}
	if (bytes.size() != 4)
	{
		error("Invalid IP Length: " + host);
	}
	ip = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	return ip;
}

const std::vector<Server> &ConfigParser::getServers() const
{
	return this->servers;
}

void ConfigParser::error(const std::string &msg) const
{
	throw std::runtime_error("Error: " + msg);
}
