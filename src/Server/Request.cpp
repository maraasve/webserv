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
			_uri.erase(pos, _uri.length() - pos);
		}
		return (true);
	}
	return (false);
}

bool	Request::checkHTTP() const {
	return (_http_version != "HTTP/1.1\r\n");
}

void	Request::parseHeaders(std::string &request){
	if (_method == "POST"){
		if (request.find("Content-Length:") != std::string::npos){
			_headers["Content-Length"] = re
		}

	}
}