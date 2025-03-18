#include "./Networks/Epoll.hpp"
#include "./Networks/Socket.hpp"
#include <iostream>

#define PORT 8080

//we need to make sure that it only works with HTTP/1.1
void handle_client(int clientfd, Epoll& epoll) {
	char buffer[1024] {'\0'};
	ssize_t bytes = recv(clientfd, buffer, sizeof(buffer), 0);
	std::cout << "---Client Request---" << std::endl;
	std::cout << buffer << std::endl;
	if (bytes > 0) {
		std::string request(buffer, bytes);
		if (request.find("GET") == 0) {
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
			response += "<html><body><h1>Hello, World!</h1></body></html>";
			send(clientfd, response.c_str(), response.size(), 0);
		} else if (request.find("POST")) {
			//handle Post, probably uploading a file
		} else {
			std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n";
			response += "<html><body><h1>400 BAD REQUEST</h1></body></html>";
			send(clientfd, response.c_str(), response.size(), 0);
		}
	} else {
		std::cout << "Client not sending data" << std::endl;
	}
	epoll.deleteFd(clientfd);
	close(clientfd);
}

int main() {
	try {
		Socket server_socket(AF_INET, SOCK_STREAM, 0, INADDR_ANY);
		server_socket.bindSocket();
		server_socket.listenSocket();

		Epoll epoll;
		epoll.addFd(server_socket.getSocketFd(), EPOLLIN);

		while(true) {
			int ready_fds = epoll.getReadyFd();
			for (int i = 0; i < ready_fds; ++i) {
				struct epoll_event event = epoll.getEvents()[i];
				if (event.data.fd == server_socket.getSocketFd()) {
					int client_fd = server_socket.acceptConnection();
					epoll.addFd(client_fd, EPOLLIN);
				} else if (event.events & EPOLLIN) {
					handle_client(event.data.fd, epoll);
				}
			}
		}
	} catch (const std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}