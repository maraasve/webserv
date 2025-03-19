#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		//we need to check if there is no config file then we act accordingly
		error_check("Unable to open " + filename);
	}
	int server_blocks = this->countServerBlocks(file);
	file.clear();
	file.seekg(0, std::ios_base::beg);
	for (int i = 0; i < server_blocks; ++i) {
		try {
			this->parseServer(file, servers[i]);
			this->parseLocation(file, servers[i].getLocations());
		} catch (...) {
			//if something goes wrong with another server do not fill in that server so do:
			--i;
			continue ;
		}
	}
}

int ConfigParser::countServerBlocks(std::ifstream& file) {
	int server_blocks = 0;
	std::string line;
	while(std::getline(file, line)) {
		if (line.find("server {") != std::string::npos) {
			++server_blocks;
		}
	}
	return server_blocks;
}

void parseServer(std::ifstream& file, Server& server) {
	std::string line
	while
}

void parseLocation(std::ifstream& file, Location& location){

}

u_long ConfigParser::convertHost(const std::string& host) {

	std::vector<int> bytes;
	u_long ip {0};
	std::stringstream ss(host);
	std::string ip_segment;

	while(std::getline(ss, ip_segment, '.')) {
		try {
			int num = std::stoi(ip_segment);
			if (num < 0 || num > 255) {
				error_check("Invalid IP Number: " + ip_segment);
			}
			bytes.push_back(num);
		} catch (const std::exception& e) {
			error_check(e.what());
		}	
	}
	if (bytes.size() != 4) {
		error_check("Invalid IP Length: " + host);
	}
	ip = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	return ip;
}

const std::vector<Server>& ConfigParser::getServers() const {
	return this->servers;
}




void ConfigParser::error_check(const std::string& msg) const {
	std::cerr << "Error in Parser : " + msg << std::endl;
	//close or quit do something here
}