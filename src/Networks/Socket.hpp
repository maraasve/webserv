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
    Socket();
    ~Socket();
    
    void bindSocket(int port, u_long host);
    void listenSocket(int backlog = BACKLOG);
    int acceptConnection();
    int getSocketFd() const;
};

#endif
