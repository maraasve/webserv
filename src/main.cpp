#include "./Networks/Epoll.hpp"
#include "./Networks/Socket.hpp"
#include <iostream>
#include "./Parsing/ConfigParser.hpp"
#include "./Server/WebServer.hpp"	

#define PORT 8080

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "Wrong Argument Count" << std::endl;
		//we need to make it work with a default configuration file
		return 1;
	}
	try {
		WebServer webserver(argv[1]);
		webserver.run();
	} catch (const std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
