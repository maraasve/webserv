#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "./Request.hpp"
#include "./Server.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <experimental/filesystem>

class Response {
	private:
		Request&   											 _request;
		Location*  											_location;
		Server&    											_server;
		std::string											_error_text;
		const std::unordered_map<std::string, std::string>	_errorsTexts;
		std::string											_error_code;
		std::string											_body;
		std::unordered_map<std::string, std::string>		_headers;

		std::string	formatStatusLine();
		std::string	formatHeaders();

		void		serveFile();
		void		serveDirectoryListing();
		void		createHeaders(const std::string& content_type, const std::string& content_length);
		void		createErrorPage(std::string filename);
		void		setErrorCodeText();
		void		handleRequest();
		std::string setContentType(const std::string& path);
		std::string resolveErrorPagePath(const std::string& code);

	public:
		Response(Request& request, Location* location, Server& server);
		~Response() = default;
		std::string	createResponseStr();
};

#endif