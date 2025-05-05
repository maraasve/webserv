#ifndef REQUEST_HPP
# define REQUEST_HPP


# include <iostream>
# include <sstream>
# include <string>
# include <regex>
# include <sys/socket.h>
# include <sys/types.h>
# include <unordered_map>

# define REGULAR_FILE 0
# define DIRECTORY 1
# define AUTOINDEX 2
# define CGI_FILE 3

class	Request {
	private:
		std::string																		_method;
		std::string																		_uri;
		std::string																		_host;
		std::string																		_port;
		std::string																		_rooted_uri;
		std::string																		_path;
		std::string																		_http_version;
		std::string																		_body;
		std::unordered_map<std::string, std::string>									_headers;
		std::string																		_error_code = "200";
		ssize_t																			_content_length;
		std::string																		_query_string;
		std::string																		_redirection_uri;
		int																				_file_type = -1;

	public:
		Request() = default;
		~Request();

		std::string																		getHost() const;
		std::string																		getMethod() const;
		std::string																		getPath() const;
		std::string																		getURI() const;
		std::string&																	getRootedURI();
		std::string																		getRootedURI() const;
		std::string																		getQueryString() const;
		std::string																		getHTTPVersion() const;
		std::string																		getBody() const;
		std::string																		getErrorCode() const;
		std::string																		getRedirectionURI() const;
		std::unordered_map<std::string, std::string>									getHeaders() const;
		ssize_t 																		getContentLength() const;
		int																				getFileType() const;



		// better to pass const std::string& in all of these:
		void	setErrorCode(std::string code);
		void	setURI(std::string uri);
		void	setPath(std::string path);
		void	setQueryString(std::string query_string);
		void	setHTTPVersion(std::string httpVersion);
		void	setBody(std::string body); //maybe change this to appendBody
		void	setHeaders(std::unordered_map<std::string, std::string> headers); //don't know if i need this anymore
		void	setHost(std::string host);
		void	setPort(std::string port);
		void	setMethod(std::string method);
		void	setRequestLine(std::string method, std::string uri, std::string version);
		void	setContentLength(ssize_t contentLength);
		void	setRootedUri(std::string rootedUri);
		void	setRedirectionURI(std::string redirection_uri);
		void	addHeader(std::string key, std::string value);
		void	setFileType(int file_type);
};

#endif
