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
	switch (_state) {
		case clientState::READING_HEADERS:
			handleHeaderState();
			break;
		case clientState::READING_BODY:
			handleBodyState();
			break;
		case clientState::PARSING_CHECKS:
			handleParsingCheckState();
			break;
		case clientState::CGI:
			handleCgiState();
			break;
		case clientState::RESPONDING:
			handleResponseState();
			break;
		case clientState::ERROR:
			handleErrorState();
			break;
	}
}

void Client::handleOutgoing() {
	ssize_t bytes = send(_fd, _responseString.c_str(), _responseString.size(), MSG_DONTWAIT);
	if (bytes > 0) {
		_responseString.erase(0, bytes);
	} else if (_responseString.empty() && closeClientConnection) {
		closeClientConnection();
	}
}

void	Client::handleHeaderState() {
	ssize_t	bytes = readIncomingData(_requestString, _fd);
	if (bytes != -1) {
		if (_requestParser.parseHeader(_requestString)) {
			if (_requestParser.getErrorCode() != "200" ) {
				_state = clientState::RESPONDING ;
				//how can we return here if we do not assign a server first?? THIS REQUIRES ATTENTION
				return ;
			}
			std::cout << "I am calling the function here\n" << std::endl;
			assignServer();
			if (_requestParser.getState() == requestState::PARSING_BODY) {
				_state = clientState::READING_BODY;
				return ;
			}
			else if (_requestParser.getState() == requestState::COMPLETE) {
				_state = clientState::PARSING_CHECKS;
				return ;
			}
		}
	} 
	_state = clientState::ERROR;

}

void	Client::handleBodyState() {
	ssize_t	bytes = readIncomingData(_requestString, _fd);
	if (bytes != -1) {
		if (_requestParser.parseBody(_requestString, bytes)) {
			_state = clientState::PARSING_CHECKS;
			return ;
		}
		if (_requestParser.getErrorCode() != "200") {
			_state = clientState::RESPONDING;
			return ;
		}
	}
	_state = clientState::ERROR;
}

void	Client::handleParsingCheckState() {
	if (_requestParser.getErrorCode() != "200") {
		_request = std::move(_requestParser).getRequest();
		_state = clientState::RESPONDING;
		return ;
	}
	if (!resolveLocation(_requestParser.getRequest().getURI())) {
		_request = std::move(_requestParser).getRequest();
		_state = clientState::RESPONDING;
		return ;
	}
	_requestParser.checkServerDependentHeaders(*_serverPtr, _location);
	_request = std::move(_requestParser).getRequest();
	if (_request.getErrorCode() != "200") {
		_state = clientState::RESPONDING;
		return ;
	}
	_state = shouldRunCgi() ? clientState::CGI : clientState::RESPONDING;
}

bool Client::shouldRunCgi() const {
	static const std::set<std::string> cgiExtensions = {"py", "php"};
	const std::string& uri = _request.getRootedURI();
	size_t pos = uri.find_last_of('.');
	if (pos == std::string::npos || pos == uri.size() - 1) {
		return false;
	}
	std::string ext = uri.substr(pos + 1);
	return cgiExtensions.count(ext) > 0;
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

void	Client::handleErrorState() {
	_request.setErrorCode("500");
	_state = clientState::RESPONDING;
}

void	Client::handleCgiState() {
	if (!_Cgi) {
		std::filesystem::path path(_request.getRootedURI());
		if (!std::filesystem::exists(path)) {
			_state = clientState::ERROR;
			return ;
		}
		_Cgi = std::make_shared<Cgi>(_request.getRootedURI(), path.extension());
		if (onCgiAccepted && _request.getMethod() == "POST") {
			_Cgi->setBody(_request.getBody());
			onCgiAccepted(_Cgi->getWriteFd(), EPOLLOUT);
			onCgiAccepted(_Cgi->getReadFd(), EPOLLIN);
		} else if (onCgiAccepted && _request.getMethod() == "DELETE") {
			onCgiAccepted(_Cgi->getReadFd(), EPOLLIN);
		}
		_Cgi->startCgi();
	} else if (_Cgi->getState() == cgiState::COMPLETE) {
		_state = clientState::RESPONDING;
	} else if (_Cgi->getState() == cgiState::ERROR) {
			_state = clientState::ERROR;
			return;
	}
}

void	Client::handleResponseState() {
	_responseString =_response.createResponseStr(_request);
	_epoll.modifyFd(_fd, EPOLLOUT);
}

void	Client::setRequestStr(std::string request) {
	_requestString = request;
}

void	Client::setServer(Server& server) {
	_serverPtr = &server;
}

int Client::getSocketFd(){
	return _socketFd;
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

std::shared_ptr<Cgi>	Client::getCgi() {
	return _Cgi;
}

Request& Client::getRequest() {
	return _request;
}

RequestParser& Client::getRequestParser() {
	return _requestParser;
}