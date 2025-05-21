#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "./Server.hpp"
#include "../Networks/Epoll.hpp"
#include "../Parsing/ConfigParser.hpp"
#include "EventHandler.hpp"

#include <vector>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <csignal>

class Client;

class WebServer
{
	private:
		std::vector<Server>										_servers;
		std::unordered_map<int, std::shared_ptr<EventHandler>>	_eventHandlers;
		Epoll													_epoll;

		void	checkTimeouts();
		void	setupServerSockets(Epoll &epoll);
		void	handleNewClient(int client_fd, Server &server);
		void	assignServer(Client &client);
		void	cleanUpResources();

	public:
		WebServer(const std::string &config_file);
		~WebServer() = default;

		void run();
};

struct hashPair
{
	template <typename T, typename U>
	std::size_t operator()(const std::pair<T, U> &p) const
	{
		auto h1 = std::hash<T>{}(p.first);
		auto h2 = std::hash<U>{}(p.second);
		return h1 ^ (h2 << 1);
	}
};

#endif