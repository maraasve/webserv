#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./Request.hpp"
#include "./Server.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

class Response {
private:
    Server*     _server = nullptr;
    const std::unordered_map<std::string, std::string> _errorsTexts;

    std::string _rooted_uri;
    std::string _error_code;
    std::string _error_text;
    std::string _body;
    std::unordered_map<std::string, std::string> _headers;
    Location _location;

    std::string formatStatusLine();
    std::string formatHeaders();

    void serverDirectoryListing(const std::string& dir_path, const std::string& uri_path);
    void serveFile(const std::string& file_path);

    void createHeaders(const std::string& content_type, const std::string& content_length);
    std::string createDirectoryListing(const std::string& dir_path, const std::string& uri_path);
    std::string createErrorPage(const std::string& error_code);
    
    void handleGET(Request& request);
    void handlePOST(Request& request);
    void handleDELETE(Request& request);

    void setErrorText(const std::string& error_code);
    void setErrorResponse(const std::string& error_code);
    void setServer(Server* server);

    std::string checkRequestURI(const std::string& rooted_uri, int mode);
    bool checkEnabledAutoIndex();
    bool checkMatchURI(const std::string& uri);
    bool checkAllowedMethods(const std::string& method);
    bool checkPostTooBig(const std::unordered_map<std::string, std::string> request_headers);

    void handleRequest(Request& request);


public:
    Response();
    ~Response() = default;

	std::string createResponseStr(Request& request, Server* server);
    
};

#endif