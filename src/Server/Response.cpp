#include "./Response.hpp"

Response::Response(Request &request, Location *location, Server &server) : _request(request), _location(location), _server(server),
																		   _errorsTexts({{"200", "OK"},
																						 {"201", "Created"},
																						 {"301", "Moved Permanently"},
																						 {"302", "Found"},
																						 {"400", "Bad Request"},
																						 {"403", "Forbidden"},
																						 {"404", "Not Found"},
																						 {"405", "Method Not Allowed"},
																						 {"408", "Request Timeout"},
																						 {"413", "Request Entity Too Large"},
																						 {"500", "Internal Server Error"},
																						 {"501", "Not Implemented"},
																						 {"502", "Bad Gateway"},
																						 {"505", "HTTP Version Not Supported"},
																						 {"504", "Gateway Timeout"}}) {}

void Response::setErrorCodeText()
{
	_error_code = _request.getErrorCode();
	auto it = _errorsTexts.find(_error_code);
	if (it != _errorsTexts.end())
	{
		_error_text = it->second;
	}
	else
	{
		_error_text = "Unknown";
	}
}

void Response::createErrorPage(std::string filename)
{
	std::ifstream file(filename);
	if (!file)
	{
		_body = "<h1>" + _error_code + "-" + _error_text + "</h1>";
	}
	else
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		_body = buffer.str();
	}
	createHeaders("text/html", std::to_string(_body.size()));
}

void Response::createHeaders(const std::string &content_type, const std::string &content_length)
{
	_headers["Content-Type"] = content_type;
	_headers["Content-Length"] = content_length;
	_headers["Connection"] = "close";
}

std::string Response::formatStatusLine()
{
	return "HTTP/1.1 " + _error_code + " " + _error_text + "\r\n";
}

std::string Response::formatHeaders()
{
	std::stringstream ss;
	for (auto it = _headers.begin(); it != _headers.end(); ++it)
	{
		ss << it->first << ":" << it->second << "\r\n";
	}
	return ss.str();
}

std::string Response::setContentType(const std::string &path)
{
	size_t pos = path.find_last_of(".");
	if (pos == std::string::npos)
	{
		return "application/octet-stream";
	}
	std::string extension = path.substr(pos + 1);
	if ((extension == "html") | (extension == "htm"))
	{
		return "text/html";
	}
	else if ((extension == "jpg") | (extension == "jpeg"))
	{
		return "image/jpeg";
	}
	else if (extension == "txt")
	{
		return "text/plain";
	}
	else if (extension == "png")
	{
		return "image/png";
	}
	else if (extension == "ico")
	{
		return "image/x-icon";
	}
	else
	{
		return "application/octet-stream";
	}
}

void Response::serveFile()
{
	std::ifstream file(_request.getRootedURI());
	if (!file)
	{
		_request.setErrorCode("500");
		setErrorCodeText();
		_request.setErrorPagePath(resolveErrorPagePath(_error_code));
		createErrorPage(_request.getErrorPagePath());
		return;
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	_body = buffer.str();
	createHeaders(setContentType(_request.getRootedURI()), std::to_string(_body.size()));
	file.close();
}

void Response::serveDirectoryListing()
{
	std::stringstream dirc_listing;

	dirc_listing << "<html><head><title>Index of " << _request.getRootedURI() << "</title></head>";
	dirc_listing << "<body><h1>Index of " << _request.getRootedURI() << "</h1><ul>";

	std::experimental::filesystem::path path(_request.getRootedURI());
	if (!std::experimental::filesystem::exists(path) || !std::experimental::filesystem::is_directory(path))
	{
		_request.setErrorCode("500");
		setErrorCodeText();
		_request.setErrorPagePath(resolveErrorPagePath(_error_code));
		createErrorPage(_request.getErrorPagePath());
		return;
	}
	for (const auto &entry : std::experimental::filesystem::directory_iterator(path))
	{
		std::string name = entry.path().filename().string();
		std::string href = name + (std::experimental::filesystem::is_directory(entry.path()) ? "/" : "");
		dirc_listing << "<li><a href=\"" << href << "\">" << href << "</a></li>";
		// if (std::experimental::filesystem::is_directory(entry.path())) {
		// 	name += "/";
		// }
		// dirc_listing << "<li>" << name << "</li>";
	}
	dirc_listing << "</ul></body></html>";
	_body = dirc_listing.str();
	createHeaders("text/html", std::to_string(_body.size()));
}

void Response::handleRequest()
{
	if (_request.getFileType() == CGI_FILE)
	{
		_body = _request.getBody();
		createHeaders("text/html", std::to_string(_body.size()));
		return;
	}
	std::string method = _request.getMethod();
	if (method == "GET")
	{
		if (_request.getFileType() != AUTOINDEX)
		{
			serveFile();
		}
		else if (_request.getFileType() == AUTOINDEX)
		{
			serveDirectoryListing();
		}
	}
}

std::string Response::resolveErrorPagePath(const std::string &code)
{
	if (_location)
	{
		auto const &loc_map = _location->getErrorPage();
		auto it = loc_map.find(code);
		if (it != loc_map.end() && !it->second.empty())
		{
			return "." + it->second;
		}
	}
	auto const &srv_map = _server.getErrorPage();
	auto it2 = srv_map.find(code);
	if (it2 != srv_map.end() && !it2->second.empty())
	{
		return "." + it2->second;
	}
	return "./variables/errors/" + code + ".html";
}

std::string Response::createResponseStr()
{
	setErrorCodeText();
	if (_error_code == "301" || _error_code == "302")
	{
		_headers["Location"] = _request.getRedirectionURI();
		createHeaders("text/html", "0");
	}
	else if (_error_code != "200")
	{
		_request.setErrorPagePath(resolveErrorPagePath(_error_code));
		createErrorPage(_request.getErrorPagePath());
	}
	else
	{
		handleRequest();
	}
	std::stringstream response;
	response << formatStatusLine();
	response << formatHeaders();
	response << "\r\n";
	response << _body;
	return response.str();
}
