#include "./Server.hpp"

void Server::handleReadRequest(int client_fd, Epoll& epoll) {
	try {
		//Handle read through the request, remember that you cannot read everything in one chunk.
		//parse information from the socket and save it in the request.
		Request request(client_fd);
		Response response(request); //create the response based on the request
		if (request._method = "GET") {
			request.handleGET();
		}
		else if (request.method == "POST") {
			request.handlePOST();
		}
		else if (request.method == "DELETE") {
			request.handleDELETE();
		}
		//saving the response to that client for later.
		_clientsResponse[client_fd] = response.buffer;
		epoll.modifyFd(client_fd, EPOLLOUT);
	} catch (...) {
		//idk
	}
}


void Server::handleWriteRequest(int client_fd, Epoll& epoll) {
	std::string& response = _clientsResponse[client_fd];
	ssize_t bytes = send(client_fd, response.c_str(), response.size(), 0);
	if (bytes <= 0) {
		std::cerr << "Error: sending data to client " + client_fd << std::endl;
		closeClientConnection(client_fd);
		return ;
	}
	respone.erase(0, bytes);
	if (response.emtpy()) {
		closeClientConnection(client_fd);
	}
}

void Server::closeClientConnection(int client_fd) {
	_clientsResponse.erase(client_fd);
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