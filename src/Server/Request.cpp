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

void	Request::parseRequest(std::string &request) {
	std::istringstream stream(request);
	stream >> _method >> _uri >> _http_version;
	if (!checkMethod()){
		_error_code = "501";
		return ;
	}
	if (!checkUri()){
		_error_code = "400";
		return ;
	}
	if (!checkHTTP()){
		_error_code = "505";
		return ;
	}
	parseHeaders(stream.str());
}

bool	Request::checkMethod() const {
	return (_method == "GET" || _method == "POST" || _method == "DELETE");
}

bool	Request::checkUri() {
	std::regex uri_regex(R"(^\/([a-zA-Z0-9\-_~.]+(?:\/[a-zA-Z0-9\-_~.]+)*)\??([^#]*)#?.*$)");
	std::smatch match;
	if (std::regex_match(_uri, match, uri_regex) || _uri == "/")
	{
		size_t pos = _uri.find("?");
		if (pos != std::string::npos){
			_query_string = _uri.substr(pos + 1);
			_uri.erase(pos, _uri.length() - pos); //should it be length or size?
		}
		return (true);
	}
	return (false);
}

bool	Request::checkHTTP() const {
	return (_http_version != "HTTP/1.1\r\n");
}

void	Request::parseHeaders(std::string &request){
}

void Request::setErrorCode(std::string error_code) {
	_error_code = error_code;
}

std::string	Request::getMethod() {
	return _method;
}

std::string Request::getURI() {
	return _uri;
}

std::string Request::getQueryString() {
	return _query_string;
}

std::string Request::getHTTPVersion() {
	return _http_version;
}

std::string Request::getBody() {
	return _body;
}

std::unordered_map<std::string, std::string> Request::getHeaders() {
	return _headers;
}

std::string Request::getErrorCode() {
	return _error_code;
}
