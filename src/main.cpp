#include "./Networks/Epoll.hpp"
#include "./Networks/Socket.hpp"
#include "./Parsing/ConfigParser.hpp"
#include "./Server/WebServer.hpp"
#include <iostream>
#include <csignal>

#define PORT 8080

volatile sig_atomic_t shutdownRequested = 0;

void sigHandler(int signal)
{
	(void)signal;
	shutdownRequested = 1;
}

int main(int argc, char *argv[])
{
	std::string configuration_file;
	if (argc != 2)
	{
		configuration_file = "./configuration_files/default.conf";
	}
	else
	{
		configuration_file = argv[1];
	}
	signal(SIGINT, sigHandler);
	signal(SIGPIPE, SIG_IGN);
	try
	{
		WebServer webserver(configuration_file);
		webserver.run();
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
