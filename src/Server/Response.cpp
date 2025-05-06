/*
Status Line = HTTP/1.1 201 Created
Headers: General | Response | Entity
Empty Line: /r/n/r/n
Body:

if _error_code is empty then that means we did not set up an error before that
*/

/*
    Errors handled by the request parser:
    501 (Not Implemented) --> when the server does not recognize the method

    505 (HTTP Version Not Supported) --> The response SHOULD contain
   an entity describing why that version is not supported and what other
   protocols are supported by that server.

   400 (Bad Request)

   Errors that need to be handled by the response:

   403 (Forbidden) --> Directory listening is off and there is no index.html, a file exists but it has
   no read permissions for your server. A directory exists but it has autoindex off and there is no index file

   404 (Not Found) --> Nothing matches the Request-URI

   405 (Method Not Allowed) --> Method specified in the request line is not allowed
   for the resource identified by the Request-URI. The response MUST include an Allow header
   containing a list of valid methods for the requested resource.

   408 (Request Timeout) --> The client did not produce a request within the time the server was prepared to wait
   Do we need this??

   411 (Length Required) --> The server refused to accept the request without a defined Content-Length header
   
   412??
   
   413 (Request Entity Too Large)
   
   500 (Internal Server Error) --> The server encountered an unexpected condition which prevented it from fulfilling the reques
   
   502 (Bad Gateway) --> The server while acting as a gateway (CGI?) received an invalid response
   from the upstream server it accessed in attempting to fulfill the request
   
   
   void printRequestObject(Request& request) {
       std::cout << "---REQUES FROM CLIENT---" << std::endl;
       std::cout << request.getMethod() << std::endl;
       std::cout << request.getURI() << std::endl;
       std::cout << request.getHTTPVersion() << std::endl;
       std::cout << request.getHost() << std::endl;
       for (auto it = request.getHeaders().begin() ; it != request.getHeaders().end() ; ++it) {
           std::cout << it->first << ": " << it->second << std::endl;
       }
       std::cout << request.getBody().substr(0, 700) << std::endl;
   }
   
   */
  

#include "./Response.hpp"

Response::Response() : _errorsTexts({
    {"200", "OK"},
    {"201", "Created"},
    {"301", "Moved Permanently"},
    {"302", "Found"},
    {"400", "Bad Request"},
    {"403", "Forbidden"},
    {"404", "Not Found"},
    {"405", "Method Not Allowed"},
    {"408", "Request Timeout"},
    {"413", "Request Entity Too Large"},
    {"500", "Internal Server Error"},
    {"501", "Not Implemented"},
    {"502", "Bad Gateway"},
    {"505", "HTTP Version Not Supported"}
}) {}

void Response::setErrorCodeText(std::string error_code) {
    _error_code = error_code;
    auto it = _errorsTexts.find(_error_code);
    if (it != _errorsTexts.end()) {
        _error_text = it->second;
    } else {
        _error_text = "Unknown";
    }
}

void Response::createErrorPage(std::string filename) {
    // std::string filename = "./variables/errors/" + _error_code + ".html";
    std::ifstream file(filename);
    if (!file) {
        _body = "<h1>" + _error_code + "-" + _error_text + "</h1>";
    } else {
        std::stringstream buffer;
        buffer << file.rdbuf();
        _body = buffer.str();
    }
    createHeaders("text/html", std::to_string(_body.size()));
}

void Response::createHeaders(const std::string& content_type, const std::string& content_length) {
    _headers["Content-Type"] = content_type;
    _headers["Content-Length"] = content_length;
    _headers["Connection"] = "close";
}

std::string Response::formatStatusLine() {
    return "HTTP/1.1 " + _error_code + " " + _error_text + "\r\n";
}

std::string Response::formatHeaders() {
    std::stringstream ss;
    for (auto it = _headers.begin(); it != _headers.end(); ++it) {
        ss << it->first << ":" << it->second << "\r\n";
    }
    return ss.str();
}

std::string Response::setContentType(const std::string& path) {
    size_t pos = path.find_last_of(".");
    if (pos == std::string::npos) {
        return "application/octet-stream";
    }
    std::string extension = path.substr(pos + 1);
    if ((extension == "html") | (extension == "htm")) {
        return "text/html";
    } else if ((extension == "jpg") | (extension == "jpeg")) {
        return "image/jpeg";
    } else if (extension == "txt") {
        return "text/plain";
    } else if (extension == "png") {
        return "image/png";
    } else {
        return "application/octet-stream";
    }
}

void    Response::serveFile(Request& request) {
    std::ifstream file(request.getRootedURI());
    if (!file) {
        setErrorCodeText("500");
        createErrorPage("./variables/errors/" + _error_code + ".html"); //the only one is not working now is 500 if someone redifines that
        createHeaders("text/html", std::to_string(_body.size()));
        return ;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    _body = buffer.str();
    createHeaders(setContentType(request.getRootedURI()), std::to_string(_body.size()));
    file.close();
}

void Response::serveDirectoryListing(Request& request) {
    std::stringstream dirc_listing;


    dirc_listing << "<html><head><title>Index of " << request.getRootedURI() << "</title></head>";
    dirc_listing << "<body><h1>Index of " << request.getRootedURI() << "</h1><ul>";

    std::filesystem::path path(request.getRootedURI());
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        setErrorCodeText("500");
        createErrorPage("./variables/errors/" + _error_code + ".html"); //The only one that is not handled now is 500 if we redifined it
        createHeaders("text/html", std::to_string(_body.size()));
        return ;
    }
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string name = entry.path().filename().string();
        std::string href = name + (std::filesystem::is_directory(entry.path()) ? "/" : "");
        dirc_listing << "<li><a href=\"" << href << "\">" << href << "</a></li>";
    }
    dirc_listing << "</ul></body></html>";
    _body = dirc_listing.str();
    createHeaders("text/html", std::to_string(_body.size()));
}


void Response::handleRequest(Request& request) {
    if (request.getFileType() == CGI_FILE) {
        _body = request.getBody();
        createHeaders("text/html", std::to_string(_body.size()));
        return;
    }
    std::string method = request.getMethod();
     if (method == "GET") {
        if (request.getFileType() != AUTOINDEX) {
            serveFile(request);
        } else if (request.getFileType() == AUTOINDEX) {
            serveDirectoryListing(request);
        }
    }
}

std::string Response::createResponseStr(Request& request) {
    setErrorCodeText(request.getErrorCode());
    if (_error_code == "301" || _error_code == "302") {
        _headers["Location"] = request.getRedirectionURI();
        createHeaders("text/html", "0");
    }
    else if (_error_code != "200") {
        if (request.getErrorPagePath().empty()) {
            createErrorPage("./variables/errors/" + _error_code + ".html");
        } else {
            createErrorPage(request.getBaseRoot() + request.getErrorPagePath());
        }
    } else {
        handleRequest(request);
    }
    std::stringstream response;
    response << formatStatusLine();
    response << formatHeaders();
    response << "\r\n";
    response << _body;
    return response.str();
}


