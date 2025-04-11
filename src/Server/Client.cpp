/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:06:22 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/07 18:19:57 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Client.hpp"

Client::Client(int fd, Epoll& epoll): _fd(fd), _serverPtr(nullptr), _epoll(epoll) {
	std::cout << "Client socket(" << _fd << ") is created" << std::endl;
}

bool	Client::readRequest() {
	char buffer[BUFSIZ];

	ssize_t bytes = recv(_fd, buffer, BUFSIZ, MSG_DONTWAIT);
	if (bytes < 0) {
		return false;
	}
	_requestString.append(buffer, bytes);
	if (!_request._header_ready) {

		_request.parseHeader(_requestString);
	}
	else if (!_request._request_ready) {
		_request.parseBody(_requestString, bytes);
	}
	if (_request._request_ready) {
		_epoll.modifyFd(_fd, EPOLLOUT);
	}
	return true;
}

bool Client::sendResponse() {
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

void	Client::setResponseStr(Request& request) {
	_responseString = _response.createResponseStr(request, _serverPtr);
}

void	Client::setServer(std::unordered_map<int, std::vector<Server*>>	_socketFdToServer) {
	auto it = _socketFdToServer.find(_fd);
	if (it != _socketFdToServer.end()) {
		std::vector<Server*>& serverVector = it->second;
		if (serverVector.size() == 1) {
			_serverPtr = serverVector.at(0);
		}
		else {
			for (Server *server : serverVector) {
				for (std::string serverName : server->getServerNames()) {
					if (_request.getHost() == serverName) {
						_serverPtr = server;
					}
				} 
			}
		}
	}
	else {
		_request.setErrorCode("400");
	}
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
	return _serverPtr;
}

Request&		Client::getRequest() {
	return _request;
}
