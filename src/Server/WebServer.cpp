#include "./WebServer.hpp"
#include "./Client.hpp"

extern volatile sig_atomic_t shutdownRequested;

WebServer::WebServer(const std::string& config_file) {
	ConfigParser parser(config_file, _servers);
	_servers = parser.getServers();
}

void WebServer::run() {
	setupServerSockets(_epoll);

	while(!shutdownRequested) {
		int	ready_fds = _epoll.getReadyFd();
		struct epoll_event *ready_events = _epoll.getEvents();
		for (int i = 0; i < ready_fds; ++i) {
			struct epoll_event event = ready_events[i];
			int event_fd = event.data.fd;
			auto handler = _eventHandlers.find(event_fd);
			if (handler != _eventHandlers.end()) {
				if (event.events & EPOLLIN || event.events & EPOLLHUP) {
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

	std::weak_ptr<Client> weakClient = newClient;
	newClient->assignServer = [this](Client& client) {
		//client has a default server?
		this->assignServer(client);
	};
	newClient->onCgiAccepted = [this, weakClient](int cgiFd, int event_type) {
		if (auto client = weakClient.lock()) {
			_epoll.addFd(cgiFd, event_type);
			auto newCgi = client->getCgi();
			_eventHandlers[cgiFd] = newCgi;
			newCgi->onCgiPipeDone = [this](int cgiFd) {
				_epoll.deleteFd(cgiFd);
				_eventHandlers.erase(cgiFd);
			};
			newCgi->closeInheritedFds = [this]() {
				for (auto it : _eventHandlers) {
					close(it.first);
				}
				close(_epoll.getEpollFd());
			};
		}
	};
	newClient->closeClientConnection = [this](int client_fd) {
		_epoll.deleteFd(client_fd);
		_eventHandlers.erase(client_fd);
		close(client_fd);
	};
}
//make sure that you are getting a 400 page and not a segmentation fault
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
			std::cout << server.getHost_string() << " " << port << std::endl;
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

