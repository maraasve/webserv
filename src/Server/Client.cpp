#include "./Client.hpp"

Client::Client(int fd, Epoll& epoll): _fd(fd), _server_ptr(nullptr), _epoll(epoll) {
	std::cout << "Client socket(" << _fd << ") is created" << std::endl;
}


// void Server::handleRequest(int client_fd, Epoll& epoll) {
// 	std::string& request_string = _clientsRequestString[client_fd];
// 	Request& request_object = *_clientsRequestObject[client_fd]; //when does this get initialized?

// 	char	buffer[BUFSIZ];
// 	ssize_t bytes = recv(client_fd, buffer, BUFSIZ, MSG_DONTWAIT);
// 	if (bytes < 0) {
// 		std::cerr << "Error: reading data from client " + client_fd << std::endl;
// 		closeClientConnection(client_fd, epoll);
// 		return ;
// 	}
// 	request_string.append(buffer, bytes);
// 	request_object.parseRequest(request_string);
// 	if (request_object.getMethod() == "POST" && !request_object._bytesToRead) {
// 		size_t header_end = request_string.find("\r\n\r\n");
// 		if (header_end != std::string::npos) {
// 			auto headers = request_object.getHeaders();
// 			auto it = headers.find("Content-Length");
// 			if (it != headers.end()) {
// 				try {
// 					request_object._bytesToRead = std::stoll(headers["Content-Length"]);
// 					request_object._bytesRead = request_string.size() - (header_end + 4);
// 				} catch (...) {
// 					//set an error message so the response catches it
// 					request_object.setErrorCode("400"); //depending on what the problem is
// 					//we need to make sure to read everything from the client even if we know it is wrong
// 					request_object._bytesToRead = 0;
// 				}
// 		}
// 	}
// 	if (request_object._bytesToRead > 0) {
// 			request_object._bytesRead += bytes;
// 			if (request_object._bytesToRead == request_object._bytesRead) {
// 				epoll.modifyFd(client_fd, EPOLLOUT);
// 			}
// 	}
// 	//when should I modify the epoll because:
// 	//POST is waiting until request_object.bytesToRead and reques_object.bytesRead are equal but 
// 		epoll.modifyFd(client_fd, EPOLLOUT);
// }

void					handleRequest() {

}

bool Client::handleResponse() {
	std::string& response = _responseString; //_responseString is initialized in a Response object that checks the request 
	ssize_t bytes = send(_fd, response.c_str(), response.size(), MSG_DONTWAIT);
	if (bytes < 0) {
		std::cerr << "Error: sending data to client " + _fd << std::endl;		
		return false;
	}
	response.erase(0, bytes);
	return !response.empty();
}

void Client::closeConnection() {
	std::cout << "Closing connection for client socket(" << _fd << ")" << std::endl;
	_epoll.deleteFd(_fd);
	close(_fd);
}

void	Client::setRequestStr(std::string request) {
	_requestString = request;
}
void	Client::setResopnseStr(std::string response) {
	_responseString = response;
}

void	Client::setServer(Server* server) {
	_server_ptr = server;
}

int	Client::getFd(){
	return _fd;
}

std::string&	Client::getRequestStr(){
	return _requestString;
}

std::string&	Client::getResponseStr(){
	return _responseString;
}

Server*	Client::getServer(){
	return _server_ptr;
}
