#include "./WebServer.hpp"

WebServer::WebServer(const std::string& config_file) {
    ConfigParser parser(config_file);
    servers = parser.getServers(); //how is this initialized is the question
    for (auto& server : servers) {
        server.getServerSocket().bindSocket();
        server.getServerSocket().listenSocket();
        server_sockets[server.getServerSocket().getSocketFd()] = &server;
        epoll.addFd(server.getServerSocket().getSocketFd(), EPOLLIN);
    }
}

void WebServer::handleClient(Server& server, int client_fd) {

}

void WebServer::run() {
    while(true) {
        int ready_fds = epoll.getReadyFd();
        for (int i = 0; i < ready_fds; ++i) {
            struct epoll_event event = epoll.getEvents()[i];
            //this is not okay because it loops through the servers even if they have no clients to accept
            if (server_sockets.find(event.data.fd) != std::nposstd:) //this loop connects clients to the their specific server
                if (event.data.fd == server.getServerSocket().getSocketFd()) {
                    int client_fd = server.getServerSocket().acceptConnection();
                    clients_to_servers[client_fd] = &server;
                    epoll.addFd(client_fd, EPOLLIN | EPOLLOUT);
                } 
            //check if the event comes from a client --> handleclient event
            auto it = clients_to_servers.find(event.data.fd); //after we process this, this instance needs to be deleted
            if (it != clients_to_servers.end()) {
                Server* correct_server = it->second;
                if (correct_server) {
                    this->handleClient(*correct_server, event.data.fd);
                }
            }
        }
    }
}

WebServer::~WebServer() {
    for (auto& server : servers) {
        epoll.deleteFd(server.getServerSocket().getSocketFd());
    }
}