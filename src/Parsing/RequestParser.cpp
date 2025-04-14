/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:06:13 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/14 18:01:39 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"

RequestParser::RequestParser() : _error_code("200") {
}

RequestParser::~RequestParser() {
}

std::string&	RequestParser::getMethod() {
	return (_method);
}

std::string& RequestParser::getURI() {
	return (_uri);
}

std::string& RequestParser::getQueryString() {
	return (_query);
}

std::string& RequestParser::getHTTPVersion() {
	return (_http_version);
}

std::string& RequestParser::getBody() {
	return (_body);
}

std::string	RequestParser::getHost() {
	auto it = _headers.find("Host");
	if (it != _headers.end()) {
		return (it->second);
	}
	return ("");
}

std::unordered_map<std::string, std::string>& RequestParser::getHeaders() {
	return (_headers);
}

std::string& RequestParser::getErrorCode() {
	return (_error_code);
}

void	RequestParser::setErrorCode(std::string code) {
	_error_code = code;
}

void	RequestParser::parseHeader(std::string& request) {
	size_t header_end = request.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		return ;
	}
	std::string	header = request.substr(0, header_end);
	std::istringstream stream(header);
	parseRequestLine(stream);
	parseHeaders(stream);
	_header_ready = true;
	if (_method == "GET") {
		_request_ready = true;
		return ;
	}
	if (_method == "POST") {
		_bytes_read = request.size() - (header_end + 4);
	}
}

void	RequestParser::parseBody(std::string& request, ssize_t bytes) {
	_bytes_read += bytes;
	std::cout << "bytes: " << std::endl;
	std::cout << "bytes read: " << _bytes_read << std::endl;
	std::cout << "content length: " << _content_length << std::endl;
	if (_bytes_read == _content_length) {
		_request_ready = true;
		size_t pos = request.find("\r\n\r\n");
		if (pos == std::string::npos) {
			_body = "";
		} else {
			_body = request.substr(pos + 4, _bytes_read);
		}
	}
}

void	RequestParser::parseRequestLine(std::istringstream& stream) {
	stream >> _method >> _uri >> _http_version;
	if (!checkMethod()){
		_error_code = "501"; 
		return ;
	}
	if (!checkHTTP()){
		_error_code = "505";
		return ;
	}
	splitUri();
	if (!checkPath() || !checkQuery()){
		_error_code = "400";
		return ;
	}
}

void	RequestParser::parseHeaders(std::istringstream& stream) {
	std::string	line, key, value;
	stream >> std::ws;
	while (std::getline(stream, line))
	{
		size_t pos = line.find(':');
		if (pos == std::string::npos) {
			_error_code = "400";
			return;
		}
		key = trim(line.substr(0, pos));
		value = trim(line.substr(pos + 1));
		if (key.empty() || value.empty()) {
			_error_code = "400";
			return ;
		}
		_headers.emplace(key, value);
	}
	if (!checkHeaders()) {
		_error_code = "400";
	}
}

bool	RequestParser::checkHeaders() {
	if (_headers.find("Host") == _headers.end()) {
		return (false);
	}
	if (_method == "POST") {
		if (_headers.find("Content-Length") == _headers.end()) {
			return (false);
		}
		auto it = _headers.find("Content-Length");
		if (it != _headers.end()) {
			try {
				std::cout << "converting content length: " << it->second << std::endl;
				std::cout << std::stol(it->second) << std::endl;
				_content_length = static_cast<ssize_t>(std::stol(it->second));
			}
			catch (...) {
				return (false);
			}
		}
	}
	return (true);
}

bool	RequestParser::checkMethod() const {
	return (_method == "GET" || _method == "POST" || _method == "DELETE");
}

bool	RequestParser::checkPath() const {
	std::regex path_regex(R"(^\/([a-zA-Z0-9\-_~.]+(?:\/[a-zA-Z0-9\-_~.]+)*$))");
	std::smatch match;
	if (std::regex_match(_path, match, path_regex) && _uri.find('\n') == std::string::npos &&_uri.find('\r') == std::string::npos) {
		return (true);
	}
	return (true);
}

bool	RequestParser::checkQuery() const {
	std::regex query_regex(R"(^([a-zA-Z0-9\-_.~]+)=([a-zA-Z0-9\-_.~%]+))");
	std::smatch match;
	if (_query.empty() || std::regex_match(_query, match, query_regex)) {
		return (true);
	}
	return (false);
}

bool	RequestParser::checkHTTP() const {
	return (_http_version == "HTTP/1.1");
}

void	RequestParser::splitUri() {
	size_t pos = _uri.find("?");
	if (pos != std::string::npos){
		_query = _uri.substr(pos + 1);
		_path = _uri.substr(0, pos);
	}
	else {
		_path = _uri;
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

bool			RequestParser::getRequestReady() {
	return _request_ready;
}