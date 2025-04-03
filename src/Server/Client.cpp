#include "./Client.hpp"

Client::Client(int fd): _fd(fd), _server_ptr(nullptr){
	std::cout << "Client socket(" << _fd << ") is created" << std::endl;
}

void Client::handleResponse(Epoll& epoll) {
	// std::string& response = _clientsResponseString[client_fd];
	std::string& response = _responseString; //_responseString we did something to it before 
	ssize_t bytes = send(client_fd, response.c_str(), response.size(), MSG_DONTWAIT);
	if (bytes < 0) {
		std::cerr << "Error: sending data to client " + client_fd << std::endl;
		closeClientConnection(client_fd);
		return ;
	}
	//what happebns when bytes == 0??
	respone.erase(0, bytes); 
	if (response.emtpy()) {
		closeClientConnection(client_fd, epoll);
	}
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
