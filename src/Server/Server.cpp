#include "./Server.hpp"

void Server::handleRequest(int client_fd, Epoll& epoll) {
	std::string& request_string = _clientsRequestString[client_fd];
	Request& request_object = *_clientsRequestObject[client_fd]; //when does this get initialized?

	char	buffer[BUFSIZ];
	ssize_t bytes = recv(client_fd, buffer, BUFSIZ, MSG_DONTWAIT);
	if (bytes < 0) {
		std::cerr << "Error: reading data from client " + client_fd << std::endl;
		closeClientConnection(client_fd, epoll);
		return ;
	}
	request_string.append(buffer, bytes);
	request_object.parseRequest(request_string);
	if (request_object.getMethod() == "POST" && !request_object._bytesToRead) {
		size_t header_end = request_string.find("\r\n\r\n");
		if (header_end != std::string::npos) {
			auto headers = request_object.getHeaders();
			auto it = headers.find("Content-Length");
			if (it != headers.end()) {
				try {
					request_object._bytesToRead = std::stoll(headers["Content-Length"]);
					request_object._bytesRead = request_string.size() - (header_end + 4);
				} catch (...) {
					//set an error message so the response catches it
					request_object.setErrorCode("400"); //depending on what the problem is
					//we need to make sure to read everything from the client even if we know it is wrong
					request_object._bytesToRead = 0;
				}
		}
	}
	if (request_object._bytesToRead > 0) {
			request_object._bytesRead += bytes;
			if (request_object._bytesToRead == request_object._bytesRead) {
				epoll.modifyFd(client_fd, EPOLLOUT);
			}
	}
	//when should I modify the epoll because:
	//POST is waiting until request_object.bytesToRead and reques_object.bytesRead are equal but 
		epoll.modifyFd(client_fd, EPOLLOUT);
}




void Server::handleResponse(int client_fd, Epoll& epoll) {
	std::string& response = _clientsResponseString[client_fd];
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

void Server::closeClientConnection(int client_fd, Epoll& epoll) {
	_clientsResponseString.erase(client_fd); //what if there i
	epoll.deleteFd(client_fd);
	close(client_fd);
}

void Server::setErrorPage(std::string error_code, std::string path) {
	_error_page.first = error_code;
	_error_page.second = path;
}

void Server::setPort(int port) {
	_port = port;
}

void Server::setAutoIndex(bool auto_index) {
	_auto_index = auto_index;
}

void Server::setIndex(std::string index) {
	_index = index;
}

void Server::setHost_u_long(u_long host) {
	_host_u_long = host;
}

void Server::setHost_string(std::string host) {
	_host_string = host;
}

void Server::setClientMaxBody(unsigned long long client_max_body) {
	_client_max_body = client_max_body;
}

void Server::setRoot(std::string root) {
	_root = root;
}

void Server::setServerNames(std::vector<std::string> server_names) {
	for (auto it = _server_names.begin(); it != _server_names.end(); ++it) {
		_server_names.push_back(*it);
	}
}

bool Server::getAutoIndex() const {
	return _auto_index;
}

std::pair<std::string, std::string> Server::getErrorPage() const {
	return _error_page;
}

Socket& Server::getServerSocket() {
	return _server_socket;
}

std::string Server::getIndex() const {
	return _index;
}

std::string Server::getRoot() const {
	return _root;
}

int Server::getPort() const {
	return _port;
}

u_long Server::getHost_u_long() const {
	return _host_u_long;
}

std::string Server::getHost_string() const {
	return _host_string;
}

unsigned long long Server::getClientMaxBody() const {
	return _client_max_body;
}

std::vector<std::string> Server::getServerNames() const {
	return _server_names;
}

std::vector<Location>& Server::getLocations() {
	return _locations;
}