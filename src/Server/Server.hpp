#ifndef SERVER_HPP
#define SERVER_HPP

#include "./Request.hpp"
#include "./Response.hpp"
#include "../Networks/Socket.hpp"
#include "../Networks/Epoll.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <stdbool.h>
#include <unordered_map>

struct Location {
	std::string _path;
	std::string _root;
	std::string _index;
	bool _auto_index = false;
	unsigned long long _client_max_body = 0;
	std::vector<std::string> _allowed_methods;
	std::pair<std::string, std::string> _error_page;
	// std::pair<std::string, std::string> HTTP_redirection; //301 (code: int) http://new_webstie (redirection: string)
};

class Server {
private:
	unsigned int _port = 8080;
	u_long _host_u_long = INADDR_ANY;
	std::string _host_string = "0.0.0.0";
	unsigned long long _client_max_body = 1048576;
	std::vector<std::string> _server_names {""};
	std::string _root;
	std::string _index;
	bool _auto_index = false;
	std::pair<std::string, std::string> _error_page;
	std::vector<Location> _locations;

	Socket _server_socket;

public:
	Server() = default;
	~Server() = default;

	void handleRequest(int client_fd, Epoll& epoll);
	void handleResponse(int client_fd, Epoll& epoll);

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

	void closeClientConnection(int client_fd, Epoll& epoll);

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
