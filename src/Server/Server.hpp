#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stdbool.h>
#include <unordered_map>
#include "../Networks/Socket.hpp"

struct Location {
	std::string path;
	std::string root;
	std::string index;
	bool directory_listing = false;
	std::vector<std::string> methods; //GET POST
	std::pair<int, std::string> HTTP_redirection; //301 (code: int) http://new_webstie (redirection: string)
};

class Server {
private:
	Socket server_socket;
	int port = 8080;
	u_long host = INADDR_ANY;
	unsigned long long client_max_body = 1048576; //max is 4GB (4294967296)
	std::vector<std::string> server_names {};
	std::unordered_map<std::string, Location> locations {};

public:
	Server(int port, u_long host);
	~Server() = default;

	void handleRequest(int client_fd, const std::string& request);
	void handleGET(int client_fd, const std::string& path);
	void handlePOST(int client_fd, const std::string& path);
	void handleDELETE(int client_fd, const std::string& path);

	void setPort(int port);
	void setHost(u_long host);
	void setClientMaxBody(int client_max_body);
	void setServerNames(std::vector<std::string>& server_names);
	void setLocations();

	Socket& getServerSocket();
	int getPort() const;
	u_long getHost() const;
	int getClientMaxBody() const;
	std::vector<std::string> getServerNames() const;
	std::unordered_map<std::string, Location> getLocations() const;

};

#endif
