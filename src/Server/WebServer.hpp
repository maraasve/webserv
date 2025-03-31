#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <iostream>
#include <unordered_map>
#include "./Server.hpp"
#include "../Networks/Epoll.hpp"
#include "../Parsing/ConfigParser.hpp"

class WebServer {
private:
    std::vector<Server> servers;
    std::unordered_map<int, Server*> socketToServer;
    std::unordered_map<int, Server*> clientsToServer;
    void setupServerSockets(Epoll& epoll);
    void cleanServersResources(Epoll& epoll);

public:
    WebServer(const std::string& config_file);
    ~WebServer();

    void run();
};

#endif