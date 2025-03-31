#include "./WebServer.hpp"

WebServer::WebServer(const std::string& config_file) {
    ConfigParser parser(config_file, servers);
}

void WebServer::setupServerSockets(Epoll& epoll) {
    for (auto& server : servers) {
        server.getServerSocket().bindSocket(server.getPort(), server.getHost_u_long());
        server.getServerSocket().listenSocket();
        epoll.addFd(server.getServerSocket().getSocketFd(), EPOLLIN);
        socketToServer[server.getServerSocket().getSocketFd()] = &server;
    }
}

void WebServer::run() {
    Epoll epoll;
    setupServerSockets(epoll);
    while(true) {
        int ready_fds = epoll.getReadyFd();
        for (int i = 0; i < ready_fds; ++i) {
            struct epoll_event event = epoll.getEvents()[i];
            auto it_server = socketToServer.find(event.data.fd);
            if (it_server != socketToServer.end()) {
                Server* server = it_server->second;
                int client_fd = server->getServerSocket().acceptConnection();
                clientsToServer[client_fd] = server;
                epoll.addFd(client_fd, EPOLLIN | EPOLLOUT);
            }
            auto it_client = clientsToServer.find(event.data.fd);
            if (it_client != clientsToServer.end()) {
                Server* client_server = it_client->second;
                if (!client_server) {
                    throw std::runtime_error("Error: client server is null");
                }
                client_server->handleRequest(event.data.fd);
            }
        }
    }
    cleanServersResources(epoll);
}

void WebServer::cleanServersResources(Epoll& epoll) {
    for (auto& server : servers) {
        epoll.deleteFd(server.getServerSocket().getSocketFd());
    }
    exit(1);
}

WebServer::~WebServer() {
}