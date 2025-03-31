#include "Socket.hpp"

Socket::Socket() 
: socketfd(socket(AF_INET, SOCK_STREAM, 0)) {
    error_check(socketfd, "Socket Creation");
    std::cout << "Socket " << socketfd << " is created" << std::endl;
}

Socket::~Socket() {
    if (socketfd >= 0) {
        close(socketfd);
        std::cout << "Socket " << socketfd << " is destroyed" << std::endl;
    }
}

void Socket::bindSocket(int port, u_long host) {
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(host);
    server_addr.sin_port = htons(port);
    addrlen = sizeof(server_addr);
    
    int val = bind(socketfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr));
    error_check(val, "Binding Socket");
    std::cout << "Socket " << socketfd << " is binded to an IP & Port" << std::endl;
}

void Socket::listenSocket(int backlog) {
    int val = listen(socketfd, backlog);
    error_check(val, "Listening Socket");
    std::cout << "Socket " << socketfd << " is listening" << std::endl;
}

int Socket::acceptConnection() {
    int new_socketfd = accept(socketfd, reinterpret_cast<struct sockaddr *>(&server_addr), reinterpret_cast<socklen_t*>(&addrlen));
    error_check(new_socketfd, "Accepting Socket");
    std::cout << "New client socket " << new_socketfd << " is created" << std::endl;
    return new_socketfd;
}

void Socket::error_check(int val, const std::string& msg) const {
    if (val < 0) {
        throw std::runtime_error(msg + ": " + strerror(errno));
    }
}

int Socket::getSocketFd() {
    return socketfd;
}
