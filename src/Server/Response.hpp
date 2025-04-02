#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./Request.hpp"

#include <string>
#include <sstream>
#include <fstream>


class Response {
private:

public:
    Response(Request request);
    std::string	loadErrorPage(int status_code);
	// void		sendErrorResponse(int client_fd, int status_code, const std::string& msg);
};

#endif