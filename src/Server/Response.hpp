#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./Request.hpp"
#include "./Server.hpp"

#include <string>
#include <sstream>
#include <fstream>


class Response {
private:
    Server*     _server = nullptr;
    const std::unordered_map<std::string, std::string> _errorsTexts;

    std::string createStatusLine(std::string& error_code);
    std::string createHeaders();
    std::string createBody();

    void setErrorCode(std::string& error_code);
    void setServer(Server* server);
public:
    Response();
    ~Response() = default;

	std::string createResponseStr(Request& request, Server* server);
    
};

#endif