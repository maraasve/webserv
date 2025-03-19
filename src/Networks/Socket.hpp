#ifndef SOCKET_HPP
#define SOCKET_HPP

#define BACKLOG 10

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstring>

class Socket {
private:
    int socketfd;
    int addrlen;
    struct sockaddr_in server_addr;

    void error_check(int val, const std::string& msg) const;
    
public:
    Socket(int port, u_long host);
    ~Socket();
    
    void bindSocket();
    void listenSocket(int backlog = BACKLOG);
    int acceptConnection();
    int getSocketFd();
};

#endif
