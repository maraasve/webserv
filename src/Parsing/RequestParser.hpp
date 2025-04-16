/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:05:58 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/16 18:25:23 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <iostream>
# include <string>
# include <unordered_map>
# include <regex>

enum clientState {
	PARSE_HEADER = 0,
	HEADER_READY,
	PARSE_BODY,
	READ_CGI,
	WRITE_CGI,
	ERROR,
	SEND,
	READY
};

class RequestParser
{
protected:
	std::string 									_method;
	std::string 									_uri;
	std::string										_rooted_uri;
	std::string										_path;
	std::string 									_query;
	std::string 									_http_version;
	std::string 									_body;
	std::string										_header;
	std::unordered_map<std::string, std::string>	_headers;
	std::string 									_error_code;
	ssize_t											_content_length;
	ssize_t											_bytes_read;
	
public:
	bool											_request_ready;
	bool											_header_ready;

	RequestParser();
	virtual ~RequestParser();

	void	parseRequestLine(std::istringstream& stream);
	void	parseHeaders(std::istringstream& stream);
	void	parseHeader(std::string& request);
	void	parseBody(std::string& request, ssize_t bytes);

	void	splitUri();

	bool	checkPath() const;
	bool	checkQuery() const;
	bool	checkMethod() const;
	bool	checkHTTP() const;
	int		checkHeader();
	bool 	checkMatchURI();

	std::string	trim(std::string str);

	std::string&	getMethod();
	std::string&	getURI();
	std::string&	getQueryString();
	std::string&	getHTTPVersion();
	std::string&	getBody();
	std::unordered_map<std::string, std::string>& getHeaders();
	std::string&	getErrorCode();
	std::string		getHost();
	bool			getRequestReady();

	void			setErrorCode(std::string code);
};

#endif
