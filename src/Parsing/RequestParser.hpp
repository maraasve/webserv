/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andmadri <andmadri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:05:58 by maraasve          #+#    #+#             */
/*   Updated: 2025/05/04 16:15:09 by andmadri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include "../Server/Request.hpp"
# include "../Server/Server.hpp"

# include <iostream>
# include <string>
# include <unordered_map>
# include <regex>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>

enum class requestState {
	PARSING_HEADER = 0,
	PARSING_BODY,
	COMPLETE,
	ERROR
};

class RequestParser
{
	private:
		requestState	_state = requestState::PARSING_HEADER;
		ssize_t				_bytes_read;
		Request				_request;
		
	public:
		RequestParser();
		virtual	~RequestParser();

		void	parseRequestLine(std::istringstream& stream);
		void	parseHeaders(std::istringstream& stream);
		bool	parseHeader(std::string& requestStr);
		bool	parseBody(std::string& requestStr, ssize_t bytes);

		void	splitUri(std::string uri);
		bool	extractQueryString(std::string& uri);
		bool	hasCgiPrefix(const std::string& uri) const;

		bool	checkPath() const;
		bool	checkQuery() const;
		bool	checkMethod() const;
		bool	checkHTTP() const;
		void	checkBasicHeaders();
		bool	checkReadingAccess();
		bool	checkHost(const std::unordered_map<std::string, std::string>& headers);
		bool	checkContentLength(const std::unordered_map<std::string, std::string>& headers);
		void	checkServerDependentHeaders(const Server& server, Location& location);
		bool	checkMatchURI(const Server& server, Location& location);
		bool	checkFile(const Server& server, Location& location);
		bool	checkCgiScript();
		bool	checkRequestURI();
		bool	checkAllowedMethods(Location& location);
		bool	checkBodyLength(const Server& server, Location& location);

		std::string			getErrorCode();
		requestState		getState();
		Request					getRequest() &&;
		const Request&	getRequest() const &;

		std::string			trim(std::string str);
};

#endif
