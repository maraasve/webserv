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
			_fdToSocket[socketFd] = serverSocket;
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
	while(true) {
		int	ready_fds = epoll.getReadyFd();
		struct epoll_event *ready_events = epoll.getEvents(); 
		for (int i = 0; i < ready_fds; ++i) {
			struct epoll_event event = ready_events[i];
			int event_fd = event.data.fd;
			auto it_socket = _fdToSocket.find(event_fd);
			if (it_socket != _fdToSocket.end()) {
				Socket* socket = it_socket->second;
				int client_fd = socket->acceptConnection();
				_clients.emplace(client_fd, Client(client_fd, epoll, socket->getSocketFd()));
				epoll.addFd(client_fd, EPOLLIN);
			}


			ssize_t bytes = client.readIncomingData();
			auto it_client = _clients.find(event_fd);
			if (it_client != _clients.end()) {
				Client &client = it_client->second;
			if (event_fd & EPOLLIN) {
				if (_clients.count(event_fd)) {
					handleClient();
					//when we identify we need to handle cgi through client, client sets itself to the cgi instance
					//if not cgi then -->epollout
					//only if POST we set CGI to epollout
				}
				else {

					readCGI();
					//cgi reade --> epollout and then send response html to client
				}
			}
			if (event_fd & EPOLLOUT) {
				if (_clients.count(event_fd)) {
					handleClient(); // --> send response
				}
				else
					writeCGI(); // --> start CGI & add fds to epoll
			}




			auto it_client = _clients.find(event_fd);
			if (it_client != _clients.end()) {
				Client &client = it_client->second;
				if (event_fd & EPOLLIN) {
					ssize_t bytes = client.readIncomingData();
					if (bytes == -1) {
						client.closeConnection();
						_clients.erase(it_client);
					}
					client.parseRequest(bytes);
				}
				if (event_fd & EPOLLOUT && client.getRequest().getRequestReady()) {
					if (client.getResponseStr().empty()) {
						client.setServer(_socketFdToServer);
						client.setResponseStr(client.getRequest());
						if (cgi?) {
							continue;
						}
					}
					//have boolean
					if (client.sendResponse()) {
						client.closeConnection();
						_clients.erase(it_client);
					}
				}
				}
			}
		}
	}
	cleanServersResources(epoll);
}


void WebServer::cleanServersResources(Epoll& epoll) {
	for (auto& it : _fdToSocket) {
		epoll.deleteFd(it.first);
	}
	exit(1);
}

WebServer::~WebServer() {
	for (auto it : _fdToSocket) {
		delete it.second;
	}
	_fdToSocket.clear();
}