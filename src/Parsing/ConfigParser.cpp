#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string &filename) {
	std::ifstream file(filename);
	if (!file) {
		error_check("Failed to Open File: " + filename);
	}
	std::string config_in_one_line = parseToLine(file);
	if (config_in_one_line.empty()){
		error_check("Config File " + filename + " is empty");
	}
	Tokenizer(config_in_one_line);
}

std::string ConfigParser::parseToLine(std::ifstream &file)
{
	std::stringstream ss;
	ss << file.rdbuf();
	if (file.fail()) {
		error_check("Config File Failed to Read");
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
	// close or quit do something here
}