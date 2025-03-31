#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stdbool.h>
#include <unordered_map>
#include "./Request.hpp"
#include "../Networks/Socket.hpp"

struct Location {
	std::string path;
	std::string root;
	std::string index;
	bool auto_index = false;
	unsigned long long client_max_body = 0;
	std::vector<std::string> allowed_methods;
	std::pair<std::string, std::string> error_page;
	// std::pair<std::string, std::string> HTTP_redirection; //301 (code: int) http://new_webstie (redirection: string)
};

class Server {
private:
	unsigned int port = 8080;
	u_long host_u_long = INADDR_ANY;
	std::string host_string = "0.0.0.0";
	unsigned long long client_max_body = 1048576;
	std::vector<std::string> server_names {""};
	std::string root;
	std::string index;
	bool auto_index = false;
	std::pair<std::string, std::string> error_page;
	std::vector<Location> locations;
	Socket server_socket;

public:
	Server() = default;
	~Server() = default;

	void handleRequest(int client_fd);

	void setPort(int port);
	void setAutoIndex(bool auto_index);
	void setErrorPage(std::string error_code, std::string path);
	void setHost_u_long(u_long host);
	void setHost_string(std::string host);
	void setClientMaxBody(unsigned long long client_max_body);
	void setServerNames(std::vector<std::string> server_names);
	void setIndex(std::string index);
	void setRoot(std::string root);
	void setLocations();

	Socket& getServerSocket();
	std::pair<std::string, std::string> getErrorPage() const;
	std::string getRoot() const;
	std::string getIndex() const;
	bool getAutoIndex() const;
	int getPort() const;
	u_long getHost_u_long() const;
	std::string getHost_string() const;
	unsigned long long getClientMaxBody() const;
	std::vector<std::string> getServerNames() const;
	std::vector<Location>& getLocations();

};

#endif
