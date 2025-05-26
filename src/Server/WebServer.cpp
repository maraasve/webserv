#include "./WebServer.hpp"
#include "./Client.hpp"

extern volatile sig_atomic_t shutdownRequested;

WebServer::WebServer(const std::string &config_file)
{
	ConfigParser parser(config_file, _servers);
	_servers = parser.getServers();
}

void WebServer::cleanUpResources()
{
	for (auto it : _eventHandlers)
	{
		close(it.first);
	}
}

void WebServer::run()
{
	setupServerSockets(_epoll);

	while (!shutdownRequested)
	{
		checkTimeouts();
		int ready_fds = _epoll.getReadyFd();
		struct epoll_event *ready_events = _epoll.getEvents();
		for (int i = 0; i < ready_fds; ++i)
		{
			struct epoll_event event = ready_events[i];
			int event_fd = event.data.fd;
			auto handler = _eventHandlers.find(event_fd);
			if (handler != _eventHandlers.end())
			{
				if (event.events & EPOLLIN || event.events & EPOLLHUP)
				{
					handler->second->handleIncoming();
				}
				if (event.events & EPOLLOUT)
				{
					handler->second->handleOutgoing();
				}
			}
		}
	}
	cleanUpResources();
}

void	WebServer::checkTimeouts()
{
	for (auto& [fd, handler] : _eventHandlers)
	{
		auto now = std::chrono::steady_clock::now();
		if (std::shared_ptr<Cgi> cgi = std::dynamic_pointer_cast<Cgi>(handler))
		{
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - cgi->getStartTime()).count();
			if (elapsed >= TIMEOUT_CGI)
			{
				cgi->setState(cgiState::TIMEOUT);
				cgi->getClient().handleIncoming();
				return;
			}
		}
		if (std::shared_ptr<Client> client = std::dynamic_pointer_cast<Client>(handler))
		{
			auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - client->getStartTime()).count();
			if (elapsed >= TIMEOUT_CLIENT)
			{
				client->setState(clientState::ERROR);
				client->handleIncoming();
				return;
			}
		}
	}
}

void WebServer::handleNewClient(int client_fd, Server &server)
{
	auto newClient = std::make_shared<Client>(client_fd, _epoll, server.getSocketFd());
	_eventHandlers[client_fd] = newClient;
	_epoll.addFd(client_fd, EPOLLIN);

	std::weak_ptr<Client> weakClient = newClient;
	newClient->assignServer = [this](Client &client)
	{
		this->assignServer(client);
	};
	newClient->onCgiAccepted = [this, weakClient](int cgiFd, int event_type)
	{
		if (auto client = weakClient.lock())
		{
			_epoll.addFd(cgiFd, event_type);
			auto newCgi = client->getCgi();
			_eventHandlers[cgiFd] = newCgi;
			newCgi->onCgiPipeDone = [this](int cgiFd)
			{
				_epoll.deleteFd(cgiFd);
				_eventHandlers.erase(cgiFd);
			};
			newCgi->closeInheritedFds = [this]()
			{
				for (auto it : _eventHandlers)
				{
					close(it.first);
				}
				close(_epoll.getEpollFd());
			};
		}
	};
	newClient->closeClientConnection = [this](int client_fd)
	{
		_epoll.deleteFd(client_fd);
		_eventHandlers.erase(client_fd);
		close(client_fd);
	};
}

void WebServer::assignServer(Client &client)
{
	int fd = client.getSocketFd();
	std::string host = client.getRequestParser().getRequest().getHost();
	Server *fallback = nullptr;
	for (Server &server : _servers)
	{
		if (fd == server.getSocketFd())
		{
			for (std::string serverName : server.getServerNames())
			{
				if (strcasecmp(host.c_str(), serverName.c_str()) == 0)
				{
					client.setServer(server);
					return;
				}
			}
			if (!fallback)
			{
				fallback = &server;
			}
		}
	}
	if (fallback)
	{
		client.setServer(*fallback);
	}
	else
	{
		client.getRequest().setErrorCode("400");
	}
}

void WebServer::setupServerSockets(Epoll &epoll)
{
	std::unordered_map<std::pair<u_long, unsigned int>, std::shared_ptr<Socket>, hashPair> addressToFd;

	for (auto &server : _servers)
	{
		u_long host = server.getHost_u_long();
		unsigned int port = server.getPort();
		std::pair<u_long, unsigned int> key = {host, port};

		int socketFd;
		if (addressToFd.find(key) == addressToFd.end())
		{
			auto serverSocket = std::make_shared<Socket>();
			socketFd = serverSocket->getSocketFd();
			serverSocket->bindSocket(port, host);
			serverSocket->listenSocket();
			epoll.addFd(socketFd, EPOLLIN);
			addressToFd[key] = serverSocket;
			server.setSocket(serverSocket);
			server.onClientAccepted = [this, &server](int client_fd)
			{
				this->handleNewClient(client_fd, server);
			};
			_eventHandlers[socketFd] = std::make_shared<Server>(server);
		}
		else
		{
			server.setSocket(addressToFd[key]);
		}
	}
}
