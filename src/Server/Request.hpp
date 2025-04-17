#ifndef REQUEST_HPP
# define REQUEST_HPP


# include <iostream>
# include <sstream>
# include <string>
# include <regex>
# include <sys/socket.h>
# include <sys/types.h>
# include <unordered_map>

class	Request {
	private:
		std::string 									_method;
		std::string 									_uri;
		std::string										_host;
		std::string										_rooted_uri;
		std::string										_path;
		std::string 									_query;
		std::string 									_http_version;
		std::string 									_body;
		std::unordered_map<std::string, std::string>	_headers;
		std::string 									_error_code = "200";
		ssize_t											_content_length;

	public:
		Request() = default;
		~Request();

		//probably better to make all of these const
		std::string										getHost() const;
		std::string										getMethod() const;
		std::string										getPath() const;
		std::string										getURI() const;
		std::string										getQueryString() const;
		std::string										getHTTPVersion() const;
		std::string										getBody() const;
		std::string										getErrorCode() const;
		std::unordered_map<std::string, std::string>	getHeaders() const;
		ssize_t 										getContentLength() const;

		// better to pass const std::string& in all of these:
		void	setErrorCode(std::string code);
		void	setURI(std::string uri);
		void	setPath(std::string path);
		void	setQueryString(std::string queryString);
		void	setHTTPVersion(std::string httpVersion);
		void	setBody(std::string body); //maybe change this to appendBody
		void	setHeaders(std::unordered_map<std::string, std::string> headers); //don't know if i need this anymore
		void	setErrorCode(std::string errorCode);
		void	setHost(std::string host);
		void	setMethod(std::string method);
		void	setRequestLine(std::string method, std::string uri, std::string version);
		void	setContentLength(ssize_t contentLength);
		void	addHeader(std::string key, std::string value);

		bool	isCGI() const; //don't know if i need this
};

#endif
