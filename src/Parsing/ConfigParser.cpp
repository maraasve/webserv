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


ConfigParser::ConfigParser(const std::string &filename) {
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
	int open_braces {0}, closing_braces {0};

	for (auto token : tokens) {
		
		if (token.token_type == BRACE_OPEN) {
			open_braces++;
		} else if (token.token_type == BRACE_CLOSE) {
			closing_braces++;
		}
	}
	if (open_braces != closing_braces) {
		error_check("Uneven number of Opening/Closing Braces");
	}
}

//ideally make this function in a place where you only need to loop once rather than having to loop again and again
bool ConfigParser::validateBraces(std::vector<Token> tokens) {
	int open_braces {0};
	int closing_braces {0};

	for (auto token : tokens) {
		if (token.token_type == BRACE_OPEN) {
			open_braces++;
		} else if (token.token_type == BRACE_CLOSE) {
			closing_braces++;
		}
	}
	if (open_braces != closing_braces) {
		return false;
	}
	return true;
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