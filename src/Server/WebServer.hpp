#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "./Server.hpp"
#include "./Client.hpp"
#include "../Networks/Epoll.hpp"
#include "../Parsing/ConfigParser.hpp"

#include <vector>
#include <iostream>
#include <unordered_map>
#include <functional>

class WebServer {
private:
	std::vector<Server>								_servers;
	std::vector<Socket*>							_sockets;
	std::unordered_map<int, Client>					_clients;
	std::unordered_map<int, std::vector<Server*>>	_socketFdToServer;

	void	setupServerSockets(Epoll& epoll);
	void	cleanServersResources(Epoll& epoll);

public:
	WebServer(const std::string& config_file);
	~WebServer();

	void	run();
};

struct hashPair {
	template <typename T, typename U>
	std::size_t operator()(const std::pair<T, U>& p) const {
		auto h1 = std::hash<T>{}(p.first);  // Hash the first element (std::string)
		auto h2 = std::hash<U>{}(p.second); // Hash the second element (int)
		return h1 ^ (h2 << 1);// Combine both hashes using XOR and bit shifting
	}
};

#endif