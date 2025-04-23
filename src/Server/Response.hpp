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
#include <filesystem>

class Response {
private:
    const std::unordered_map<std::string, std::string> _errorsTexts;

    std::string _rooted_uri;
    std::string _error_code;
    std::string _error_text;
    std::string _body;
    std::unordered_map<std::string, std::string> _headers;
    Location _location;

    std::string formatStatusLine();
    std::string formatHeaders();

    void serveFile(const std::string& file_path);
    void serveDirectoryListing(const std::string& dir_path);

    void createHeaders(const std::string& content_type, const std::string& content_length);
    void createErrorPage();

    void setErrorCodeText(std::string error_code);
    std::string setContentType(const std::string& path);

    void uploadFile(const Request& request);
	std::string findFileName(const Request& request);

    void handleRequest(const Request& request);


public:
    Response();
    ~Response() = default;

	std::string    createResponseStr(const Request& request);
};

#endif