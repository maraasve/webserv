#include "./Server.hpp"

// Server::Server(): server_socket() {

// }

void Server::setPort(int port) {
	this->port = port;
}

void Server::setHost(u_long host) {
	this->host = host;
}

void Server::setClientMaxBody(int client_max_body) {
	this->client_max_body = client_max_body;
}

void Server::setServerNames(std::vector<std::string>& server_names) {
	for (auto it = server_names.begin(); it != server_names.end(); ++it) {
		this->server_names.push_back(*it);
	}
}

int Server::getPort() const {
	return this->port;
}

u_long Server::getHost() const {
	return this->host;
}

int Server::getClientMaxBody() const {
	return this->client_max_body;
}

std::vector<std::string> Server::getServerNames() const {
	return this->server_names;
}

std::unordered_map<std::string, Location> Server::getLocations() const {
	return this->locations;
}