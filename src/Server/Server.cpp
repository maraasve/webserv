#include "./Server.hpp"

void	Server::handleIncoming() {
	int client_fd = _serverSocket->acceptConnection();
	if (client_fd > 0 && onClientAccepted) {
		onClientAccepted(client_fd);
	}
}

void	Server::handleOutgoing() {}

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
	_server_names.insert(_server_names.end(), server_names.begin(), server_names.end());
}

void	Server::setSocket(const std::shared_ptr<Socket>& socket)
{
	_serverSocket = socket;
}

bool Server::getAutoIndex() const {
	return _auto_index;
}

std::pair<std::string, std::string> Server::getErrorPage() {
	return _error_page;
}

// Socket& Server::getServerSocket() {
// 	return _server_socket;
// }

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

int	Server::getSocketFd() {
	return _serverSocket->getSocketFd();
}