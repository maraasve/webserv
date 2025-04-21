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

bool	RequestParser::parseHeader(std::string& requestStr) {
	size_t header_end = requestStr.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		return false; // what if it never finds "\r\n\r\n" 
	}
	std::string	header = requestStr.substr(0, header_end);
	std::istringstream stream(header);
	parseRequestLine(stream);
	parseHeaders(stream);
	checkBasicHeaders();
	if (_request.getMethod() == "POST") {
		_bytes_read = requestStr.size() - (header_end + 4);
		_state = PARSING_BODY;
		return true;
	}
	_state = COMPLETE;
	return true;
}

bool	RequestParser::parseBody(std::string& requestStr, ssize_t bytes) {
	_bytes_read += bytes;
	if (_bytes_read == _request.getContentLength()) {
		size_t pos = requestStr.find("\r\n\r\n");
		if (pos == std::string::npos) {
			_request.setBody("");
		} else {
			_request.setBody(requestStr.substr(pos + 4, _bytes_read));
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

	if (!checkHost(headers)) {
		return ;
	}
	if (!checkContentLength(headers)) {
		return ;
	}
	return ;
}

bool	RequestParser::checkHost(const std::unordered_map<std::string, std::string>& headers) {
	auto it = headers.find("Host");
	if (it == headers.end()) {
		_request.setErrorCode("400"); 
		return false;
	}
	else {
		std::string host = it->second;
		size_t pos = host.find(":");
		if (pos != std::string::npos) {
			_request.setHost(host.substr(0, pos));
			_request.setPort(host.substr(pos + 1));
		}
		else {
			_request.setHost(host);
		}
	}
	return true;
}

bool	RequestParser::checkContentLength(const std::unordered_map<std::string, std::string>& headers) {
	const std::string& method = _request.getMethod();
	int	contentLength;
	if (method == "POST") {
		if (headers.find("Content-Length") == headers.end()) {
			_request.setErrorCode("411"); 
			return false;
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
				return false;
			}
			_request.setContentLength(contentLength);
		}
	}
	return true;
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

void	RequestParser::checkServerDependentHeaders(const Server& server, const Location& location) {
	if (!checkMatchURI(server, location)) {
		_request.setErrorCode("404");
		return ;
    }
	if (!checkAllowedMethods(location)) {
		_request.setErrorCode("405");
		return ;
    }
	if (_request.getMethod() == "POST") {
		if (!checkBodyLength(server, location)) {
			_request.setErrorCode("413");
			return ;
		}
	}
}

bool	RequestParser::checkMatchURI(const Server& server, const Location& location) {
	std::string rest_uri = _request.getURI().substr(location._path.size());
	if (rest_uri.empty() || rest_uri[0] != '/') {
		rest_uri = "/";
	}
	const std::string& base_root = location._root.empty() ? server.getRoot() : location._root;
	_request.setRootedUri("." + base_root + rest_uri);
}

bool	RequestParser::checkAllowedMethods(const Location& location) {
	if (location._allowed_methods.empty()) {
		return true;
	}
	for (const std::string& allowed_method : location._allowed_methods) {
		std::cout << "This is the allowed: " << allowed_method << std::endl;
		if (allowed_method == _request.getMethod()) {
			return true;
		}
	}
	return false;
}

bool RequestParser::checkBodyLength(const Server& server, const Location& location) {
	ssize_t	contentLength = _request.getContentLength();
	if (location._client_max_body > 0 && contentLength <= location._client_max_body) {
		return false;
	}
	else if (contentLength <= server.getClientMaxBody()) {
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

Request&	RequestParser::getRequest() {
	return (_request);
}
