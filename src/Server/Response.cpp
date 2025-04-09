#include "./Response.hpp"

Response::Response() : _errorsTexts({
    {"200", "OK"},
    {"201", "Created"},
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

std::string Response::loadErrorPage(int status_code) {
    std::string filename = "/var/errors/" + std::to_string(status_code) + ".html";
    std::ifstream file(filename);
    if (!file) {
        //handle error
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// void Response::sendErrorResponse(int client_fd, int status_code, const std::string& msg) {
//     //I need to check if I can send the client anything with EPOLLOUT right?
//     std::string body = loadErrorPage(status_code);
//     std::stringstream response;
//     response << "HTTP/1.1 " << status_code << " " << msg << "\r\n";
//     //response << "Server: " << *OUR SERVER NAME*
//     response << "Content-Type: text/html; charset=UTF-8/\r\n";
//     response << "Content-Length: " << body.size() << "\r\n";
//     response << "Connection: close\r\n\r\n";
//     response << body;
//     //need to check if I can send the thing to the client right?
// }

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

   403 (Forbidden) --> The method understood the request but is refusing to fulfill it

   404 (Not Found) --> Nothing matches the Request-URI

   405 (Method Not Allowed) --> Method specified in the request line is not allowed
   for the resource identified by the Request-URI. The response MUST include an Allow header
   containing a list of valid methods for the requested resource.

   408 (Request Timeout) --> The client did not produce a request within the time the server was prepared to wait
   Do we need this??

   411??
   412??

   413 (Request Entity Too Large)

   500 (Internal Server Error) --> The server encountered an unexpected condition which prevented it from fulfilling the reques

   502 (Bad Gateway) --> The server while acting as a gateway (CGI?) received an invalid response
   from the upstream server it accessed in attempting to fulfill the request



*/
void Response::setErrorCode(std::string& error_code) {
    //to be able to identify we need to go to the server and check for several things
    //413 --> We need to check if the method is Post then we need to see how big the body is based on Content Length
    //then we check if there is a location to which the URI matches?? then we check if client_size_max is bigger than zero
    //then we compare it to 


    //There is a URI, you need to check if the URI matches any location path by 
    

}

std::string Response::createStatusLine(std::string& error_code) {
    std::stringstream status_line;
    std::string       error_phrase = "Unkown";
    if (error_code.empty()) {
        setErrorCode(error_code);
    }
    auto it = _errorsTexts.find(error_code);
    if (it != _errorsTexts.end()) {
        error_phrase = it->second;
    }
    status_line << "HTTP/1.1 " << error_code << " " << error_phrase << "/r/n";
}

std::string Response::createHeaders() {

}

std::string Response::createBody() {

}

void Response::setServer(Server* server) {
    _server = server;
}

std::string Response::createResponseStr(Request& request, Server* server) {
    std::string responseStr;
    
    setServer(server);
    responseStr += createStatusLine(request.getErrorCode());
    responseStr += createHeaders();
    responseStr += "/r/n/r/n";
    responseStr += createBody();
}
