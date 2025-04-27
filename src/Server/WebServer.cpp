#include "./WebServer.hpp"
#include "./Client.hpp"

WebServer::WebServer(const std::string& config_file) {
	ConfigParser parser(config_file, _servers);
	_servers = parser.getServers();
}

void WebServer::run() {
	setupServerSockets(_epoll);

	while(true) {
		int	ready_fds = _epoll.getReadyFd();
		struct epoll_event *ready_events = _epoll.getEvents();
		for (int i = 0; i < ready_fds; ++i) {
			struct epoll_event event = ready_events[i];
			int event_fd = event.data.fd;
			auto handler = _eventHandlers.find(event_fd);
			if (handler != _eventHandlers.end()) {
				if (event.events & EPOLLIN) {
					handler->second->handleIncoming();
				}
				if (event.events & EPOLLOUT) {
					handler->second->handleOutgoing();
				}
			}
		}
	}
	//clean up resources
}

void	WebServer::handleNewClient(int client_fd, Server &server) {
	auto newClient = std::make_shared<Client>(client_fd, _epoll, server.getSocketFd());
	_eventHandlers[client_fd] = newClient;
	_epoll.addFd(client_fd, EPOLLIN);
	newClient->assignServer = [this](Client& client) {
		this->assignServer(client);
	};
	newClient->onCgiAccepted = [this, newClient](int cgiFd, int event_type) {
		_epoll.addFd(cgiFd, event_type);
		auto newCgi = newClient->getCgi();
		_eventHandlers[cgiFd] = newCgi;
		newCgi->onCgiPipeDone = [this](int cgiFd) {
			_epoll.deleteFd(cgiFd);
			_eventHandlers.erase(cgiFd);
		};
	};
	newClient->closeClientConnection = [this, newClient]() {
		std::cout << "Calling Close Connection successful" << std::endl;
		int client_fd = newClient->getFd();
		std::cout << "We get the client fd: " << client_fd << std::endl;
		_epoll.deleteFd(client_fd);
		std::cout << "We delete the client fd: " << client_fd << std::endl;
		close(client_fd);
		std::cout << "We close the client fd: " << client_fd << std::endl;
		_eventHandlers.erase(client_fd);
		std::cout << "We erase the client fd from epoll: " << client_fd << std::endl;
	};
}
void	WebServer::assignServer(Client &client) {
	int			fd = client.getSocketFd();
	std::string	host = client.getRequestParser().getRequest().getHost();
	Server		*fallback = nullptr;
	for (Server& server : _servers) {
		if (fd == server.getSocketFd()) {
			for (std::string serverName : server.getServerNames()) {
				if (strcasecmp(host.c_str(), serverName.c_str()) == 0) {
					client.setServer(server); //if socket matches, but not servername NGINX sets a fallback server
					return ;
				}
			}
			if (!fallback) {
				fallback = &server;
			}
		}
	}
	if (fallback) {
		client.setServer(*fallback);
	}
	else {
		client.getRequest().setErrorCode("400");
	}
}

void WebServer::setupServerSockets(Epoll& epoll) {
	std::unordered_map <std::pair<u_long, unsigned int>, std::shared_ptr<Socket>, hashPair> addressToFd;

	for (auto& server : _servers) {
		u_long			host = server.getHost_u_long();
		unsigned int	port = server.getPort();
		std::pair<u_long,unsigned int> key = {host, port};

		int	socketFd;
		if (addressToFd.find(key) == addressToFd.end()) {
			auto serverSocket = std::make_shared<Socket>();
			socketFd = serverSocket->getSocketFd();
			serverSocket->bindSocket(port, host);
			serverSocket->listenSocket();
			epoll.addFd(socketFd, EPOLLIN);
			addressToFd[key] = serverSocket;
			server.setSocket(serverSocket);
			server.onClientAccepted = [this, &server](int client_fd) {
				this->handleNewClient(client_fd, server);
			};
			_eventHandlers[socketFd] = std::make_shared<Server>(server);
		}
		else {
			server.setSocket(addressToFd[key]);
		}
	}
}

