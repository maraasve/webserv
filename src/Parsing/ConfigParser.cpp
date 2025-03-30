#include "ConfigParser.hpp"

std::string printEnum(int i) {
	switch (i) {
		case 0:
			return ("KEYWORD");
		case 1:
			return ("BRACE_OPEN");
		case 2:
			return ("BRACE_CLOSE");
		case 3:
			return ("COLON");
		case 4:
			return ("SEMI_COLON");
	}
	return ("");
}


ConfigParser::ConfigParser(const std::string &filename): open_braces(0){
	std::ifstream file(filename);
	if (!file) {
		error_check("Failed to Open File: " + filename);
	}
	Tokenizer = ConfigTokenizer(loadFileAsString(file));
	// for (auto tokens : Tokenizer.getTokens()) {
	// 	std::cout << printEnum(tokens.token_type) << " --> " << tokens.value << std::endl;
	// }
	parseServer(Tokenizer.getTokens());
}

void ConfigParser::parseServer(std::vector<Token> tokens) {
	std::unordered_set<std::string> valid_directives = {
		"listen", "server_name", "root", "index", "auto_index", "client_max_body",
		"error_page", "location"
	};
	std::unordered_map<std::string, bool> check_duplicates = {
		{"listen", false}, {"server_name", false}, {"root", false}, {"index", false}, 
		{"auto_index", false}, {"client_max_body", false}, {"error_page", false}
	};
	auto it = tokens.begin();
	int i = -1;
	bool inside_server_block = false;

	while (it != tokens.end()) {
		if (it->value != "server" && i == -1) {
			error_check("Server block should start with directive server: " + it->value);
		}
		if (it->value == "server" && !inside_server_block) {
			++it;
			if (it == tokens.end() || it->token_type != BRACE_OPEN) {
				error_check("Missing opening braces for server block");
			}
			inside_server_block = true;
			++open_braces;
			++i; 
			++it;
			for (auto& entry : check_duplicates) {
				entry.second = false;
			}
			continue;
		}
		if (valid_directives.find(it->value) == valid_directives.end()) {
			error_check("Unexpected directive inside server block: " + it->value);
		}
		std::string directive = it->value;
		++it;
		if (it == tokens.end() || it->token_type != KEYWORD) {
			error_check("Missing value for directive: " + directive);
		}
		if (check_duplicates[directive] == true && directive != "location") {
			error_check("Duplicate directive inside server block: " + directive);
		}
		check_duplicates[directive] = true;
		std::string value = it->value;
		++it;
		if (directive == "listen") {
			parseDirectiveListen(servers[i], value, it, tokens.end());
			required_directives.has_listen = true;
		} else if (directive == "server_name") {
			servers[i].setServerNames(parseServerName(value, it, tokens.end()));
		} else if (directive == "root") {
			if (!isValidPath(value)) {
				error_check("Invalid path for root directive: " + value);
			}
			servers[i].setRoot(value);
			required_directives.has_root = true;
		} else if (directive == "index") {
			if (!isValidIndex(value)) {
				error_check("Invalid file for index directive: " + value);
			}
			servers[i].setIndex(value);
			required_directives.has_index = true;
		} else if (directive == "auto_index") {
			if (value != "on" && value != "off") {
				error_check("Invalid auto_index value: " + value);
			}
			servers[i].setAutoIndex(value == "on");
		} else if (directive == "client_max_body") {
			unsigned long long bytes = isValidClientBodySize(value);
			if (!bytes) {
				error_check("Invalid client's body size: " + value);
			}
			servers[i].setClientMaxBody(bytes);
		} else if (directive == "error_page") {
			if (!isValidErrorCode(value)) {
				error_check("Invalid error_page code: " + value);
			}
			++it;
			if (it == tokens.end() || it->token_type != KEYWORD || !isValidPath(it->value)) {
				error_check("Invalid error_page path");
			}
			servers[i].setErrorPage(value, it->value);
		} else if (directive == "location") {
			if (!isValidPath(value)) {
				error_check("Invalid path for location directive: " + value);
			}
			++it;
			if (it != tokens.end() && it->token_type == BRACE_OPEN) {
				++open_braces;
				parseLocation(servers[i].getLocations()[i], it, tokens.end());
			} else {
				error_check("Missing opening braces for location directive");
			}
			continue;
		}
		if (it == tokens.end() || it->token_type != SEMI_COLON) {
			error_check("Missing semicolon for directive: " + directive);
		} else if (it->token_type == BRACE_CLOSE) {
			--open_braces;
			if (!required_directives.has_index || !required_directives.has_listen || !required_directives.has_root) {
				error_check("Missing directives");
			}
			if (open_braces == 0) {
				inside_server_block = false;
			}
			++it;
		}
	}
	if (open_braces != 0) {
		error_check("Uneven number of Opening/Closing Braces");
	}
}

void ConfigParser::parseLocation(Location& location, std::vector<Token>::iterator& it, std::vector<Token>::iterator end) {
	std::unordered_set<std::string> valid_directives = {
		"error_page", "client_max_body", "auto_index", "limit_except", "root", "index"
	};
	std::unordered_map<std::string, bool> check_duplicates = {
		{"error_page", false}, {"client_max_body", false}, {"auto_index", false}, 
		{"limit_except", false}, {"root", false}, {"index", false}
	};
	while (it != end) {
		if (it->token_type == BRACE_CLOSE) {
			--open_braces;
			++it;
			return;
		}
		if (valid_directives.find(it->value) == valid_directives.end()) {
			error_check("Unexpected directive inside location block: " + it->value);
		}
		std::string directive = it->value;
		++it;
		if (it == end || it->token_type != KEYWORD) {
			error_check("Missing value for directive: " + directive);
		}
		if (check_duplicates[directive] == true) {
			error_check("Duplicate directive inside location block: " + directive);
		}
		check_duplicates[directive] = true;
		std::string value = it->value;
		++it;
		if (directive == "error_page") {
			if (!isValidErrorCode(value)) {
				error_check("Invalid error_page code: " + value);
			}
			++it;
			if (it == end || it->token_type != KEYWORD || !isValidPath(it->value)) {
				error_check("Invalid error_page path");
			}
			//save error code (value) and path (it->value)
		} else if (directive == "client_max_body") {
			unsigned long long bytes = isValidClientBodySize(value);
			if (!bytes) {
				error_check("Invalid client's body size: " + value);
			}
			location.client_max_body = bytes;
		} else if (directive == "root") {
			if (!isValidPath(value)) {
				error_check("Invalid root path: " + value);
			}
			location.root = value;
			required_directives.has_root = true;
		} else if (directive == "index") {
			if (!isValidIndex(value)) {
				error_check("Invalid index file: " + value);
			}
			location.index = value;
			required_directives.has_index = true;
		} else if (directive == "auto_index") {
			if (value != "on" && value != "off") {
				error_check("Invalid auto_index value: " + value);
			}
			location.auto_index = (value == "on");
		}
		if (it == end || it->token_type != SEMI_COLON) {
			error_check("Missing semicolon for directive: " + directive);
		}
		++it;
	}
	error_check("Missing closing braces");
}

void ConfigParser::parseDirectiveListen(Server& server, std::string& value, std::vector<Token>::iterator& it, std::vector<Token>::iterator& end) {
	++it;
	if (it != end && it->token_type == COLON) {
		server.setHost_u_long(convertHost(value));
		server.setHost_string(value);
		++it;
		if (it == end || it->token_type != KEYWORD || !isValidPort(it->value)) {
			error_check("Invalid argument in listen directive, expecting a valid port number");
		}
		server.setPort(std::stoi(it->value));
		++it;
	} else if (it != end && it->token_type == SEMI_COLON) {
		if (!isValidPort(value)) {
			error_check("Invalid port number in listen directive: " + value);
		}
		server.setPort(std::stoi(it->value));
		++it;
	}
}

std::vector<std::string> ConfigParser::parseServerName(std::string value, std::vector<Token>::iterator& it, std::vector<Token>::iterator& end) {
	std::vector<std::string> server_names;
	int i = -1;
	while(it != end){
		if (it->token_type == SEMI_COLON) {
			return server_names;
		}
		if (!isValidServerName(value)) {
			error_check("Invalid name for server_name directive: " + value);
		}
		server_names[++i] = value;
		value = it->value;
		it++;
	}
	error_check("Missing semi-colon in server_name directive");

}

bool ConfigParser::isValidServerName(std::string& server_name) {
	if (server_name == "localhost") {
		return true;
	}
	const std::regex domain_name(R"(^(?!-)([a-zA-Z0-9-]{1,63}\.)+[a-zA-Z0-9-]{1,63}$)");
	return std::regex_match(server_name, domain_name);
}

bool ConfigParser::isValidPort(std::string& port) {
	try {
		int num = std::stoi(port);
		if (num < 0 || num > 65535) {
			return false;
		}
		return true;
	} catch(...) {
		return false;
	}
}

bool ConfigParser::isValidErrorCode(std::string& error_code) {
	const std::regex error_code_regex(R"(^(?:403|404|405|408|413|500|502|503|504|505)$)");
	return std::regex_match(error_code, error_code_regex);
}

unsigned long long ConfigParser::isValidClientBodySize(std::string& client_body_size) {
	const std::regex client_body_regex(R"(^([1-9][0-9]*)([mMkKgG])$)");
	std::smatch match;
	if (std::regex_match(client_body_size, match, client_body_regex)) {
		try {
			char unit = match[2].str()[0];
			unsigned long long num = std::stoull(match[1]);
			switch (unit) {
				case 'k': case 'K': num *= 1024; break;
				case 'm': case 'M': num *= 1024 * 1024; break;
				case 'g': case 'G': num *= 1024 * 1024 * 1024; break;
				default: return 0;
			}
			if (num > 4294967296) {
				return 0;
			}
			return num;
		} catch(...) {
			return 0;
		}
	}
	return 0;
}

bool ConfigParser::isValidIndex(std::string& index) {
	const std::regex index_regex(R"(^[a-zA-Z][a-zA-Z0-9_\-]*\.(?:html?|php)$)");
	return std::regex_match(index, index_regex);
}

bool ConfigParser::isValidPath(std::string& path) {
	if (path == "/") {
		return true;
	}
	const std::regex path_regex(R"(^\/([a-zA-Z0-9_\-\.]+\/?)*$)");
	return std::regex_match(path, path_regex);
}

std::string ConfigParser::loadFileAsString(std::ifstream &file) {
	std::stringstream ss;
	ss << file.rdbuf();
	if (file.fail()) {
		error_check("Config File Failed to Read");
	}
	if (ss.str().empty()) {
		error_check("Config File is empty");
	}
	return ss.str();
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
				error_check("Invalid IP Number: " + ip_segment);
			}
			bytes.push_back(num);
		} catch (...) {
			error_check("Invalid IP" + ip_segment);
		}
	}
	if (bytes.size() != 4) {
		error_check("Invalid IP Length: " + host);
	}
	ip = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	return ip;
}


const std::vector<Server> &ConfigParser::getServers() const {
	return this->servers;
}

void ConfigParser::error_check(const std::string &msg) const {
	std::cerr << "Error in Parser : " + msg << std::endl;
}