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

/*
	server {
	server {

*/
	while (it != tokens.end()) {
		if (it->value == "server" && !inside_server_block) {
			++it;
			if (it == tokens.end() || it->token_type != BRACE_OPEN) {
				error_check("Missing opening braces for server block");
			}
			inside_server_block = true;
			++open_braces;
			++i; //for server[i] of the vector of servers 
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
		if (check_duplicates[directive] == true) {
			error_check("Duplicate directive inside server block: " + directive);
		}
		std::string value = it->value;
		++it;
		if (directive == "listen") {

		} else if (directive == "server_name") {

		} else if (directive == "root") {

		} else if (directive == "index") {

		} else if (directive == "auto_index") {

		} else if (directive == "client_max_body") {

		} else if (directive == "error_page") {
			check_duplicates[directive] = true;
		} else if (directive == "location") {
			if (!isValidPath(value)) {
				error_check("Invalid path for location directive: " + value);
			}
			//add the path to location
			++it;
			if (it != tokens.end() && it->token_type == BRACE_OPEN) {
				++open_braces;
				parseLocation(it, tokens.end());
			} else {
				error_check("Missing opening braces for location directive");
			}
			continue;
		}
		if (it == tokens.end() || it->token_type != SEMI_COLON) {
			error_check("Missing semicolon for directive: " + directive);
		} else if (it->token_type == BRACE_CLOSE) {
			--open_braces;
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

void ConfigParser::parseLocation(std::vector<Token>::iterator& it, std::vector<Token>::iterator end) {
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
			check_duplicates[directive] = true;
		} else if (directive == "client_max_body") {
			if (!isValidClientBodySize(value)) {
				error_check("Invalid client's body size: " + value);
			}
			//fill in client_max_body
			check_duplicates[directive] = true;
		} else if (directive == "root") {
			if (!isValidPath(value)) {
				error_check("Invalid root path: " + value);
			}
			//fill in root
			check_duplicates[directive] = true;
		} else if (directive == "index") {
			if (!isValidIndex(value)) {
				error_check("Invalid index file: " + value);
			}
			//save index
			check_duplicates[directive] = true;
		} else if (directive == "auto_index") {
			if (value != "on" && value != "off") {
				error_check("Invalid auto_index value: " + value);
			}
			//save auto_index
			check_duplicates[directive] = true;
		}
		if (it == end || it->token_type != SEMI_COLON) {
			error_check("Missing semicolon for directive: " + directive);
		}
		++it;
	}
	error_check("Missing closing braces");
}

bool ConfigParser::isValidIndex(std::string& index) {
	const std::regex index_regex("R()");
	return std::regex_match(index, index_regex);
}

bool ConfigParser::isValidPath(std::string& path) {
	const std::regex path_regex(R"(^\/[a-zA-Z0-9_\-\.]+\/?)*$)");
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

// u_long ConfigParser::convertHost(const std::string &host)
// {

// 	std::vector<int> bytes;
// 	u_long ip{0};
// 	std::stringstream ss(host);
// 	std::string ip_segment;

// 	while (std::getline(ss, ip_segment, '.'))
// 	{
// 		try
// 		{
// 			int num = std::stoi(ip_segment);
// 			if (num < 0 || num > 255)
// 			{
// 				error_check("Invalid IP Number: " + ip_segment);
// 			}
// 			bytes.push_back(num);
// 		}
// 		catch (const std::exception &e)
// 		{
// 			error_check(e.what());
// 		}
// 	}
// 	if (bytes.size() != 4)
// 	{
// 		error_check("Invalid IP Length: " + host);
// 	}
// 	ip = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
// 	return ip;
// }


const std::vector<Server> &ConfigParser::getServers() const {
	return this->servers;
}

void ConfigParser::error_check(const std::string &msg) const {
	std::cerr << "Error in Parser : " + msg << std::endl;
}