#ifndef SOCKET_HPP
#define SOCKET_HPP

#define BACKLOG 10

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

class Socket {
private:
    int socketfd;
    int addrlen;
    struct sockaddr_in server_addr;

    void error_check(int val, const std::string& msg) const;
    
public:
    Socket(int domain, int service, int protocol, u_long interface, int port);
    ~Socket();
    
    void bindSocket(int fd, int port);
    void listenSocket(int backlog);
    int acceptConnection();
    void setNonBlockingSocket();
};

#endif
