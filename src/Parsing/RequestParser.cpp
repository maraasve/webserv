/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:06:13 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/16 18:24:50 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

RequestParser::RequestParser() {
}

RequestParser::~RequestParser() {
}

bool	RequestParser::parseHeader(std::string& request) {
	size_t header_end = request.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		return false; // what if it never finds "\r\n\r\n" 
	}
	std::string	header = request.substr(0, header_end);
	std::istringstream stream(header);
	parseRequestLine(stream);
	parseHeaders(stream);
	checkBasicHeaders();
	if (_request.getMethod() == "POST") {
		_bytes_read = request.size() - (header_end + 4);
		_state = PARSING_BODY;
		return true;
	}
	_state = COMPLETE;
	return true;
}

bool	RequestParser::parseBody(std::string& request, ssize_t bytes) {
	_bytes_read += bytes;
	if (_bytes_read == _request.getContentLength()) {
		size_t pos = request.find("\r\n\r\n");
		if (pos == std::string::npos) {
			_request.setBody("");
		} else {
			_request.setBody(request.substr(pos + 4, _bytes_read));
		}
		if (!checkBodyLength()) {
			_request.setErrorCode("431");
		}
		_state = COMPLETE;
		return true;
	}
	return false;
}

void	RequestParser::parseRequestLine(std::istringstream& stream) {
	std::string	method, uri, httpVersion;
	stream >> method >> uri >> httpVersion;
	_request.setRequestLine(method, uri, httpVersion);
	if (!checkMethod()){
		_request.setErrorCode("501"); 
		return ;
	}
	if (!checkHTTP()){
		_request.setErrorCode("505"); 
		return ;
	}
	splitUri(uri);
	if (!checkPath() || !checkQuery()){
		_request.setErrorCode("400"); 
		return ;
	}
}

void	RequestParser::parseHeaders(std::istringstream& stream) {
	std::unordered_map <std::string, std::string>	headers;
	std::string										line, key, value;

	stream >> std::ws;
	while (std::getline(stream, line))
	{
		size_t pos = line.find(':');
		if (pos == std::string::npos) {
			_request.setErrorCode("400"); 
			return ;
		}
		key = trim(line.substr(0, pos));
		value = trim(line.substr(pos + 1));
		if (key.empty() || value.empty()) {
			_request.setErrorCode("400"); 
			return ;
		}
		_request.addHeader(key, value);
	}
}

void	RequestParser::splitUri(std::string uri) {
	size_t	pos = uri.find("?");
	if (pos != std::string::npos){
		_request.setQueryString(uri.substr(pos + 1));
		_request.setPath(uri.substr(0, pos));
	}
	else {
		_request.setPath(uri);
	}
}

std::string	RequestParser::trim(std::string str) {
	size_t first, last;
	first = str.find_first_not_of(" \n\t\r\f\v");
	last = str.find_last_not_of(" \n\t\r\f\v");
	if (first == std::string::npos || last == std::string::npos) {
		return ("");
	}
	return (str.substr(first, (last - first + 1)));
}


void	RequestParser::checkBasicHeaders() {
	const std::unordered_map<std::string, std::string>& headers = _request.getHeaders();
	const std::string& method = _request.getMethod();
	int contentLength;

	if (headers.find("Host") == headers.end()) {
		_request.setErrorCode("400"); 
		return ;
	}
	if (method == "POST") {
		if (headers.find("Content-Length") == headers.end()) {
			_request.setErrorCode("411"); 
			return ;
		}
		auto it = headers.find("Content-Length");
		if (it != headers.end()) {
			try {
				std::cout << "converting content length: " << it->second << std::endl;
				std::cout << std::stol(it->second) << std::endl;
				contentLength = static_cast<ssize_t>(std::stol(it->second));
			}
			catch (...) {
				_request.setErrorCode("500"); 
				return ;
			}
			_request.setContentLength(contentLength);
		}
	}
	return ;
}

bool	RequestParser::checkServerDependentHeaders(Server &server) {
	if (!checkMatchURI()) {
		_request.setErrorCode("404"); 
		return false;
    }
	if (!checkAllowedMethods()) {
		_request.setErrorCode("405"); 
		return false;
    }
}

bool	RequestParser::checkMethod() const {
	std::string	method = _request.getMethod();
	return (method == "GET" || method == "POST" || method == "DELETE");
}

bool	RequestParser::checkPath() const {
	std::string path, uri;
	std::regex	path_regex(R"(^\/([a-zA-Z0-9\-_~.]+(?:\/[a-zA-Z0-9\-_~.]+)*$))");
	std::smatch match;
	path = _request.getPath();
	uri = _request.getURI();
	if (std::regex_match(path, match, path_regex) && uri.find('\n') == std::string::npos && uri.find('\r') == std::string::npos) {
		return true;
	}
	return true;
}

bool	RequestParser::checkQuery() const {
	std::string	query = _request.getQueryString();
	std::regex	query_regex(R"(^([a-zA-Z0-9\-_.~]+)=([a-zA-Z0-9\-_.~%]+))");
	std::smatch match;
	if (query.empty() || std::regex_match(query, match, query_regex)) {
		return true;
	}
	return false;
}

bool	RequestParser::checkHTTP() const {
	return (_request.getHTTPVersion() == "HTTP/1.1");
}

bool RequestParser::checkMatchURI() {
    size_t bestMatchLength = 0;
    for (const auto& location : _server->getLocations()) {
        if (_uri.compare(0, location._path.size(), location._path) == 0) {
            if (location._path.size() < bestMatchLength) {
                continue;
            }
			//this is problematic because we need location
            bestMatchLength = location._path.size();
            _location = location;
            std::string rest_uri = _uri.substr(location._path.size());
            if (rest_uri.empty() || rest_uri[0] != '/') {
                rest_uri = '/' + rest_uri;
            }
            const std::string& base_root = location._root.empty() ? _server->getRoot() : location._root;
            _rooted_uri = "." + base_root + rest_uri;
        }
    }
    if (bestMatchLength == 0) {
        return false;
    }
    return true;
}

bool RequestParser::checkBodyLength() {
	ssize_t	contentLength = _request.getContentLength();
	if (_location._client_max_body > 0 && contentLength <= _location._client_max_body) {
		return false;
	} 
	else if (contentLength <= _server->getClientMaxBody()) {
		return false;
    }
    return true;
}

std::string	RequestParser::getErrorCode() {
	return (_request.getErrorCode());
}

int	RequestParser::getState() {
	return (_state);
}
