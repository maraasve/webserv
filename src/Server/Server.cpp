#include "./Server.hpp"

// Server::Server(int port, u_long host): port(port), host(host), server_socket(port, host) {
// }

void Server::setErrorPage(std::string error_code, std::string path) {
	this->error_page.first = error_code;
	this->error_page.second = path;
}

void Server::setPort(int port) {
	this->port = port;
}

void Server::setAutoIndex(bool auto_index) {
	this->auto_index = auto_index;
}

void Server::setIndex(std::string index) {
	this->index = index;
}

void Server::setHost_u_long(u_long host) {
	this->host_u_long = host;
}

void Server::setHost_string(std::string host) {
	this->host_string = host;
}

void Server::setClientMaxBody(unsigned long long client_max_body) {
	this->client_max_body = client_max_body;
}

void Server::setRoot(std::string root) {
	this->root = root;
}

void Server::setServerNames(std::vector<std::string>& server_names) {
	for (auto it = server_names.begin(); it != server_names.end(); ++it) {
		this->server_names.push_back(*it);
	}
}

bool Server::getAutoIndex() const {
	return this->auto_index;
}

std::pair<std::string, std::string> Server::getErrorPage() const {
	return this->error_page;
}

Socket& Server::getServerSocket() {
	return this->server_socket;
}

std::string Server::getIndex() const {
	return this->index;
}

std::string Server::getRoot() const {
	return this->root;
}

int Server::getPort() const {
	return this->port;
}

u_long Server::getHost_u_long() const {
	return this->host_u_long;
}

std::string Server::getHost_string() const {
	return this->host_string;
}

unsigned long long Server::getClientMaxBody() const {
	return this->client_max_body;
}

std::vector<std::string> Server::getServerNames() const {
	return this->server_names;
}

std::vector<Location>& Server::getLocations() {
	return this->locations;
}