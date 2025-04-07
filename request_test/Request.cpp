#include "./Request.hpp"

Request::Request(std::string& request): _error_code("200") {
	if (request.empty()) {
		_error_code = "400";
		return ;
	}
}

std::string	Request::getMethod() {
	return (_method);
}

void	Request::parseRequest(std::string& request) {
	std::string			header, body;
	size_t pos = request.find("\r\n\r\n");
	if (pos != std::string::npos) {
		header = request.substr(0, pos);
		body = request.substr(pos + 4);
		parseBody(body);
	}
	else
		header = request;
	std::istringstream	stream(header);
	parseRequestLine(stream);  //maybe check first if should be parsed
	parseHeaders(stream); //maybe check first if should be parsed
}

void	Request::parseRequestLine(std::istringstream& stream) {
	stream >> _method >> _uri >> _http_version;
	if (!checkMethod()){
		_error_code = "501"; 
		return ;
	}
	splitUri();
	if (!checkPath()){
		_error_code = "400";
		return ;
	}
	if (!checkQuery()){
		_error_code = "400";
		return ;
	}
	if (!checkHTTP()){
		_error_code = "505";
		return ;
	}
}

void	Request::parseHeaders(std::istringstream& stream) {
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
}

void	Request::parseBody() {
	
}

bool	Request::checkMethod() const {
	return (_method == "GET" || _method == "POST" || _method == "DELETE");
}

bool	Request::checkPath() const {
	std::regex path_regex(R"(^\/([a-zA-Z0-9\-_~.]+(?:\/[a-zA-Z0-9\-_~.]+)*$))");
	std::smatch match;
	if (std::regex_match(_path, match, path_regex) && _uri.find('\n') == std::string::npos &&_uri.find('\r') == std::string::npos) {
		return (true);
	}
	return (true);
}

bool	Request::checkQuery() const {
	std::regex query_regex(R"(^([a-zA-Z0-9\-_.~]+)=([a-zA-Z0-9\-_.~%]+))");
	std::smatch match;
	if (_query.empty() || std::regex_match(_query, match, query_regex))
		return (true);
	return (false);
}

bool	Request::checkHTTP() const {
	return (_http_version == "HTTP/1.1");
}

void	Request::splitUri() {
	size_t pos = _uri.find("?");
	if (pos != std::string::npos){
		_query = _uri.substr(pos + 1);
		_path = _uri.substr(0, pos);
	}
	else
		_path = _uri;
}

std::string	Request::trim(std::string str) {
	size_t first, last;
	first = str.find_first_not_of(" \n\t\r\f\v");
	last = str.find_last_not_of(" \n\t\r\f\v");
	if (first == std::string::npos || last == std::string::npos)
		return ("");
	return (str.substr(first, (last - first + 1)));
}

void Request::setErrorCode(std::string error_code) {
	_error_code = error_code;
}

std::string Request::getURI() {
	return (_uri);
}

std::string Request::getQueryString() {
	return (_query);
}

std::string Request::getHTTPVersion() {
	return (_http_version);
}

std::string Request::getBody() {
	return (_body);
}

std::unordered_map<std::string, std::string> Request::getHeaders() {
	return (_headers);
}

std::string Request::getErrorCode() {
	return (_error_code);
}
