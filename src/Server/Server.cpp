#include "./Server.hpp"

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