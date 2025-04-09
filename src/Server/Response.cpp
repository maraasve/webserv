#include "./Response.hpp"

// Response::Response(Request& request) {

// }

Response::~Response() {

}

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