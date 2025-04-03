#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <sys/socket.h>
#include <sys/types.h>
#include <unordered_map>

class Request {
private:
	std::string 									_method;
	std::string 									_uri;
	std::string										_path;
	std::string 									_query;
	std::string 									_http_version;
	std::string 									_body;
	std::unordered_map<std::string, std::string>	_headers;
	std::string _error_code;
	
	void sendResponse();
	
	public:
	ssize_t												_bytesToRead {0};
	ssize_t												_bytesRead{0};
 
	Request(std::string& request);

	void	parseRequest(std::string &request);
	void	parseRequestLine(std::istringstream &request);
	void	parseHeaders(std::istringstream &request);
	void	parseBody();

	void	splitUri();

	bool	checkPath() const;
	bool	checkQuery() const;
	bool	checkMethod() const;
	bool	checkHTTP() const;

	std::string	trim(std::string str);

	void setErrorCode(std::string error_code);

	std::string	getMethod();
	std::string getURI();
	std::string getQueryString();
	std::string getHTTPVersion();
	std::string getBody();
	std::unordered_map<std::string, std::string> getHeaders();
	std::string getErrorCode();

};

#endif
