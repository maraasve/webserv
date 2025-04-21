/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:06:22 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/16 18:18:29 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Client.hpp"

Client::Client(int fd, Epoll& epoll, int socket_fd): _fd(fd), _serverPtr(nullptr), _epoll(epoll), _socketFd(socket_fd) {
	std::cout << "Client socket(" << _fd << ") is created" << std::endl;
}

void	Client::handleIncoming() {
	ssize_t	bytes;

	switch (_state) {
		case READING_HEADERS:
			handleHeaderState();
			break;
		case READING_BODY:
			handleBodyState();
			break; // do we need to break everywhere?
		case PARSING_CHECKS:
			handleParsingCheckState();
		case CGI:
			handleCGIState();
		case RESPONDING:
			handleResponseState();
		case ERROR:
			handleErrorState();
		case COMPLETE:
			handleCompleteState();
			break ;
	}
}

void	Client::handleHeaderState() {
	ssize_t	bytes = readIncomingData(_requestString, _fd);
	if (bytes != -1) {
		if (_requestParser.parseHeader(_requestString)) {
			if (_requestParser.getErrorCode() != "200" ) {
				_state = RESPONDING ;
				return ;
			}
			assignServer();
			if (_requestParser.getState() == PARSING_BODY) {
				_state = READING_BODY;
				return ;
			}
			else if (_requestParser.getState() == COMPLETE) {
				_state = PARSING_CHECKS;
				return ;
			}
		}
	}
	_state = ERROR;
}

void	Client::handleBodyState() {
	ssize_t	bytes = readIncomingData(_requestString, _fd);
	if (bytes != -1) {
		if (_requestParser.parseBody(_requestString, bytes)) {
			_state = PARSING_CHECKS;
			return ;
		}
		if (_requestParser.getErrorCode() != "200") {
			_state = RESPONDING;
			return ;
		}
	}
	_state = ERROR;
}

void	Client::handleParsingCheckState() {
	assignServer(); //I don't think i need to pass the client instance, but not sure
	if (_requestParser.getErrorCode() != "200") {
		_state = RESPONDING;
		return ;
	}
	if (!resolveLocation(_requestParser.getRequest().getURI())) {
		_state = RESPONDING;
		return ;
	}
	_requestParser.checkServerDependentHeaders(*_serverPtr, _location);
	_request = _requestParser.getRequest();
	if (_request->getErrorCode() != "200") {
		_state = RESPONDING;
		return ;
	}
	_state = _CGI.shouldRunCgi(_request->getRootedURI()) ? CGI : RESPONDING;
}

bool	Client::resolveLocation(std::string uri) {
	size_t bestMatchLength = 0;
    for (const auto& location : _serverPtr->getLocations()) {
        if (uri.compare(0, location._path.size(), location._path) == 0) {
            if (location._path.size() < bestMatchLength) {
                continue;
            }
            bestMatchLength = location._path.size();
            _location = location;
		}
	}
	if (bestMatchLength == 0) {
		return false;
	}
	return true;
}

void	Client::handleCGIState() {
	if (!_request->getBody().empty()) {
		_CGI.setBody(_request->getBody());
	}
	_CGI.startCGI();
}

void	Client::handleResponseState() {
	if (!_request) { //in case of parse error and _request wasn't assigned yet
		_request = _requestParser.getRequest();
	}
}

// void	Client::parseRequest(std::unordered_map<int, std::vector<Server*>>	socketFdToServer) {
// 	ssize_t bytes = readIncomingData(_requestString, _fd); //think about where to put this
// 	if (bytes == -1) {
// 		closeConnection();
// 		//should we send client error?
// 	}
// 	if (_state == PARSE_HEADER) {
// 		_state = _request.parseHeader(_requestString);
// 		setServer(socketFdToServer);
// 		_state = _request.checkHeader();
// 	}
// 	else if (_state == PARSE_BODY) {
// 		_state = _request.parseBody(_requestString, bytes);
// 	}
// 	if (_state == READY) {
// 		_epoll.modifyFd(_fd, EPOLLOUT);
// 	}
// }

bool Client::sendResponse() {
	std::string& response = _responseString; //_responseString is initialized in a Response object that checks the request 
	ssize_t bytes = send(_fd, response.c_str(), response.size(), MSG_DONTWAIT);
	if (bytes < 0) {
		std::cerr << "Error: sending data to client " << _fd << std::endl;
		return true;
	}
	response.erase(0, bytes);
	return response.empty();
}

void Client::closeConnection() {
	std::cout << "Closing connection for client socket(" << _fd << ")" << std::endl;
	_epoll.deleteFd(_fd);
	close(_fd);
	//make callback function so WebServer can remove client from eventHandlers
}

void	Client::setRequestStr(std::string request) {
	_requestString = request;
}

void	Client::setResponseStr(Request& request) {
	_responseString = _response.createResponseStr(request, _serverPtr);
}

void	Client::setServer(Server& server) {
	_serverPtr = &server;
}

void	Client::setServerError(std::string error) {
	_requestParser.getRequest().setErrorCode(error);
}

// void	Client::setServer(std::unordered_map<int, std::vector<Server*>> socketFdToServer) {
// 	auto it = socketFdToServer.find(_socketFd);
// 	if (it != socketFdToServer.end()) {
// 		std::vector<Server*>& serverVector = it->second;
// 		if (serverVector.size() == 1) {
// 			_serverPtr = serverVector.at(0);
// 		}
// 		else {
// 			for (Server *server : serverVector) {
// 				for (std::string serverName : server->getServerNames()) {
// 					if (_request.getHost() == serverName) {
// 						_serverPtr = server;
// 					}
// 				} 
// 			}
// 		}
// 	}
// 	else {
// 		_request.setErrorCode("400");
// 	}
// }



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

// Request&		Client::getRequest() {
// 	return _request; //need to find another way because of the optional _request
// }
