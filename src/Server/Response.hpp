#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./Request.hpp"

#include <string>
#include <sstream>
#include <fstream>


class Response {
private:

public:
    Response() = default;
	std::string createResponseStr(Request& request);
    // std::string	loadErrorPage(int status_code);
};

#endif