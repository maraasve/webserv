#include "./WebServer.hpp"

WebServer::WebServer(const std::string& config_file) {
	ConfigParser parser(config_file, _servers);
}

void WebServer::setupServerSockets(Epoll& epoll) {
	for (auto& server : _servers) {
		Socket &server_socket = server.getServerSocket();
		int server_fd = server_socket.getSocketFd();
		server_socket.bindSocket(server.getPort(), server.getHost_u_long());
		server_socket.listenSocket();
		epoll.addFd(server_fd, EPOLLIN);
		_socketToServer[server_fd] = &server;
		_socketToServer.emplace(server_fd, &server);

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
			auto it_server = _socketToServer.find(event_fd); //finding which socket belongs to which server, what happens if I have to servers listening to the same port and IP, what should I do?
			if (it_server != _socketToServer.end()) {
				Server* server = it_server->second;
				int client_fd = server->getServerSocket().acceptConnection();
				_clients.emplace(client_fd, Client(client_fd, epoll));
				epoll.addFd(client_fd, EPOLLIN);
			}
			auto it_client = _clients.find(event_fd);
			if (it_client != _clients.end()) {
					Client &client = it_client->second;
			if (event_fd & EPOLLIN) {
					// client.handleRequest();
			}
			if (event_fd & EPOLLOUT) {
				if (!client.handleResponse())
					client.closeConnection();
					_clients.erase(it_client);
			}
		}
	}
}
	cleanServersResources(epoll);
}

void WebServer::cleanServersResources(Epoll& epoll) {
	for (auto& server : _servers) {
		epoll.deleteFd(server.getServerSocket().getSocketFd());
	}
	exit(1);
}

WebServer::~WebServer() {
}