#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

class Request {
private:
	std::string method;
	std::string path;
	std::string http_version;
	
	void sendResponse();

public:
	Request(int client_fd);

	void handleGET();
	void handlePOST();
	void handleDELETE();

};

#endif