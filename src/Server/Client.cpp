#include "./Client.hpp"

Client::Client(int fd): _fd(fd), _server_ptr(nullptr){
	std::cout << "Client socket(" << _fd << ") is created" << std::endl;
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
