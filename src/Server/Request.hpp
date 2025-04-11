#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../Parsing/RequestParser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <sys/socket.h>
#include <sys/types.h>
#include <unordered_map>

class Request : public RequestParser{
public:
	Request() = default;
	~Request();
};

#endif
