#include "Socket.hpp"

Socket::Socket(int domain, int service, int protocol, u_long interface, int port = 8080) 
: socketfd(socket(domain, service, protocol)) {
    error_check(socketfd, "Socket Creation");
    server_addr.sin_family = domain;
    server_addr.sin_addr.s_addr = htonl(interface);
    server_addr.sin_port = htons(port);
    addrlen = sizeof(server_addr);
    std::cout << "Socket " << socketfd << " is created" << std::endl;
}

Socket::~Socket() {
    close(socketfd); //close can fail you need to protect that
    std::cout << "Socket " << socketfd << "is destroyed" << std::endl;
}

void Socket::bindSocket(int fd, int port) {
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
    std::cout << "New Socket non-listening " << new_socketfd << " is accepted" << std::endl;
    return new_socketfd;
}

void Socket::error_check(int val, const std::string& msg) const {
    if (val < 0) {
        throw std::runtime_error("Socket error: " + msg);
    }
}
