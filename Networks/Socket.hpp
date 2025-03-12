#ifndef SOCKET_HPP
#define SOCKET_HPP

#define BACKLOG 10

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <iostream>
#include <unistd.h>

class Socket : public std::exception {
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
};

#endif
