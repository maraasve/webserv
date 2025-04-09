#include "./WebServer.hpp"

WebServer::WebServer(const std::string& config_file) {
	ConfigParser parser(config_file, _servers); //_servers is not set in Webserver
	_servers = parser.getServers(); //Thats why im doing this here now, but it's still passed to parser
}

// void WebServer::setupServerSockets(Epoll& epoll) {
// 	std::unordered_map <std::pair<u_long, unsigned int>, int, hashPair> addressToFd;

// 	for (auto& server : _servers) {
// 		Socket &server_socket = server.getServerSocket();
// 		int server_fd = server_socket.getSocketFd();
// 		server_socket.bindSocket(server.getPort(), server.getHost_u_long());
// 		server_socket.listenSocket();
// 		epoll.addFd(server_fd, EPOLLIN);
// 		_socketToServer[server_fd] = &server;
// 		_socketToServer.emplace(server_fd, &server);

// 	}
// }

void WebServer::setupServerSockets(Epoll& epoll) {
	std::unordered_map <std::pair<u_long, unsigned int>, int, hashPair> addressToFd;

	for (auto& server : _servers) {
		u_long			host = server.getHost_u_long();
		unsigned int	port = server.getPort();
		std::pair<u_long,unsigned int> key = {host, port};

		int	socketFd;
		if (addressToFd.find(key) == addressToFd.end()) {
			Socket*	serverSocket = new Socket;
			socketFd = serverSocket->getSocketFd();
			serverSocket->bindSocket(port, host);
			serverSocket->listenSocket();
			epoll.addFd(socketFd, EPOLLIN);
			addressToFd[key] = socketFd;
			_sockets.push_back(serverSocket);
			_socketFdToServer[socketFd].push_back(&server);
		}
		else {
			socketFd = addressToFd[key];
			_socketFdToServer[socketFd].push_back(&server);
		}
	}
}

void WebServer::run() {
	Epoll epoll;
	setupServerSockets(epoll);
	// while(true) {
	// 	int	ready_fds = epoll.getReadyFd();
	// 	struct epoll_event *ready_events = epoll.getEvents(); 
	// 	for (int i = 0; i < ready_fds; ++i) {
	// 		struct epoll_event event = ready_events[i];
	// 		int event_fd = event.data.fd;
	// 		auto it_server = _socketFdToServer.find(event_fd); //finding which socket belongs to which server, what happens if I have to servers listening to the same port and IP, what should I do?
	// 		if (it_server != _socketFdToServer.end()) {
	// 			Server* server = it_server->second;
	// 			int client_fd = server->getServerSocket().acceptConnection();
	// 			_clients.emplace(client_fd, Client(client_fd, epoll));
	// 			epoll.addFd(client_fd, EPOLLIN);
	// 		}
	// 		auto it_client = _clients.find(event_fd);
	// 		if (it_client != _clients.end()) {
	// 				Client &client = it_client->second;
	// 		}
	// 		if (event_fd & EPOLLIN) {
	// 			if (!client.readRequest()) {
	// 				client.closeConnection();
	// 				_clients.erase(it_client);
	// 			}
	// 		}
	// 		if (event_fd & EPOLLOUT) {
	// 			//make the response so that then it can be sent
	// 			if (client.getResponseStr().empty()) {
	// 				client.setServer(_servers);
	// 				client.setResponseStr(client.getRequest());
	// 			}
	// 			if (!client.sendResponse()) {
	// 				client.closeConnection();
	// 				_clients.erase(it_client);
	// 			}
	// 		}
	// 	}
	// }
	// cleanServersResources(epoll);
}


void WebServer::cleanServersResources(Epoll& epoll) {
	for (auto& it : _sockets) {
		epoll.deleteFd(it->getSocketFd());
	}
	exit(1);
}

WebServer::~WebServer() {
	for (Socket *socket : _sockets) {
		delete socket;
	}
	_sockets.clear();
}