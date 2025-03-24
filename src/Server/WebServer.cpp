#include "./WebServer.hpp"

WebServer::WebServer(const std::string& config_file) {
    ConfigParser parser(config_file);
    servers = parser.getServers(); //how is this initialized is the question
    for (auto& server : servers) {
        server.getServerSocket().bindSocket();
        server.getServerSocket().listenSocket();
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
            for (auto& server : servers) { //this loop connects clients to the their specific server
                if (event.data.fd == server.getServerSocket().getSocketFd()) {
                    int client_fd = server.getServerSocket().acceptConnection();
                    clients[client_fd] = &server;
                    epoll.addFd(client_fd, EPOLLIN | EPOLLOUT);
                } 
            }
            //check if the event comes from a client --> handleclient event
            auto it = clients.find(event.data.fd); //after we process this, this instance needs to be deleted
            if (it != clients.end()) {
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