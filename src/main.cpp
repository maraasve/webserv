#include "./Networks/Epoll.hpp"
#include "./Networks/Socket.hpp"
#include <iostream>
#include "./Parsing/ConfigParser.hpp"
#include "./Server/WebServer.hpp"	

#define PORT 8080

int main(int argc, char *argv[]) {
	std::string configuration_file;
	if (argc != 2) {
		configuration_file = "./configuration_files/default.conf";
	} else {
		configuration_file = argv[1];
	}
	try {
		WebServer webserver(configuration_file);
		webserver.run();
	} catch (const std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
