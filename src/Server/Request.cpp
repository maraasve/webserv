/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:14:13 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/07 15:54:46 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./Request.hpp"

Request::~Request(){
}

std::string	Request::getMethod() const{
	return (_method);
}

std::string Request::getURI() const {
	return (_uri);
}

std::string Request::getQueryString() const{
	return (_query);
}

std::string Request::getHTTPVersion() const{
	return (_http_version);
}

std::string Request::getBody() const{
	return (_body);
}

std::string	Request::getPath() const{
	return (_path);
}

std::string	Request::getHost() const{
	auto it = _headers.find("Host");
	if (it != _headers.end()) {
		return (it->second);
	}
	return ("");
}

std::unordered_map<std::string, std::string> Request::getHeaders() const{
	return (_headers);
}

std::string Request::getErrorCode() const{
	return (_error_code);
}

ssize_t	Request::getContentLength() const{
	return (_content_length);
}

void	Request::setErrorCode(std::string code) {
	_error_code = code;
}

void	Request::setURI(std::string uri) {
    _uri = uri;
}

void	Request::setQueryString(std::string queryString) {
    _query = queryString;
}

void	Request::setHTTPVersion(std::string httpVersion) {
    _http_version = httpVersion;
}

void	Request::setBody(std::string body) {
    _body = body;
}

void	Request::setHeaders(std::unordered_map<std::string, std::string> headers) {
    _headers = headers;
}

void	Request::setErrorCode(std::string errorCode) {
    _error_code = errorCode;
}

void	Request::setHost(std::string host) {
    _host = host;
}

void	Request::setMethod(std::string method) {
    _method = method;
}

void	Request::setMethod(std::string path) {
    _path = path;
}

void	Request::setRequestLine(std::string method, std::string uri, std::string version) {
	_method = method;
	_uri = uri;
	_http_version = version;
}

void	Request::setContentLength(ssize_t contentLength) {
	_content_length = contentLength;
}

void	Request::addHeader(std::string key, std::string value) {
	_headers.emplace(key, value);
}

bool Request::isCGI() const {
    std::string uri = _uri;
}
