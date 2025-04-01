#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./Request.hpp"

#include <string>
#include <sstream>
#include <fstream>


class Response {
private:

public:
    Response();
    std::string loadErrorPage(int status_code);
};

#endif