#include "./Request.hpp"

Request::Request(int client_fd) {
	std::string request = readRequest(client_fd);
	if (request.empty()) {
		
	}
}

std::string Request::readRequest(int client_fd) {
	char buffer[4096];
	std::string request;
	ssize_t bytes{0};
	while(true) {
		bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
		if (!bytes) {
			break ;
		}
		buffer[bytes] = '\0';
		request += buffer;
		if (request.find("\r\n\r\n") != std::string::npos) {
			break;
		}
	}
	return request;
}