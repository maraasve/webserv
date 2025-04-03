#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "./Server.hpp"
#include "./Client.hpp"
#include "../Networks/Epoll.hpp"
#include "../Parsing/ConfigParser.hpp"

#include <vector>
#include <iostream>
#include <unordered_map>

class WebServer {
private:
	std::vector<Server>								_servers;
	std::unordered_map<int, Server*>	_socketToServer;
	std::unordered_map<int, Client>		_clients;

	void	setupServerSockets(Epoll& epoll);
	void	cleanServersResources(Epoll& epoll);

public:
	WebServer(const std::string& config_file);
	~WebServer();

	void	run();
};

#endif