#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../Parsing/ConfigParser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <sys/socket.h>
#include <sys/types.h>

class Request {
private:
	std::string 									_method;
	std::string 									_uri;
	std::string 									_query_string;
	std::string 									_http_version;
	std::string 									_body;
	std::unordered_map<std::string, std::string>	_headers;
	std::string _error_code;
	
	void sendResponse();

public:
	Request(std::string& request);

	// std::string readRequest(int client_fd);
	void	parseRequest(std::string &request);
	void	parseHeaders(std::string &request);

	bool	checkUri();
	bool	checkMethod() const;
	bool	checkHTTP() const;

	void handleGET();
	void handlePOST();
	void handleDELETE();
	std::string	getMethod();

};

#endif
