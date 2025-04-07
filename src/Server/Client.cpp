#include "./Client.hpp"

Client::Client(int fd, Epoll& epoll): _fd(fd), _server_ptr(nullptr), _epoll(epoll) {
	std::cout << "Client socket(" << _fd << ") is created" << std::endl;
}

void	Client::readRequest() {
	char	buffer[BUFSIZ];
	int		bytes_read;

	while (true) {
		bytes_read = recv(_fd, buffer, BUFSIZ, MSG_DONTWAIT);
		if (bytes_read < 0) {
			std::cerr << "recv() error on client " << _fd << std::endl;
			// erase client, maybe make a function for this?? - error handling
			return ;
		}
		if (bytes_read == 0)
			return ;
		_requestString.append(buffer, bytes_read);
	}
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
