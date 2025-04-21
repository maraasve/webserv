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

# include "../Server/Request.hpp"
# include "../Server/Server.hpp"

# include <iostream>
# include <string>
# include <unordered_map>
# include <regex>

enum requestState {
	PARSING_HEADER = 0,
	PARSING_BODY,
	COMPLETE,
	ERROR
};

class RequestParser
{
	private:
		int			_state = PARSING_HEADER;
		ssize_t		_bytes_read;
		Request		_request;
		
	public:
		RequestParser();
		virtual	~RequestParser();

		void	parseRequestLine(std::istringstream& stream);
		void	parseHeaders(std::istringstream& stream);
		bool	parseHeader(std::string& requestStr);
		bool	parseBody(std::string& requestStr, ssize_t bytes);

		void	splitUri(std::string uri);

		bool	checkPath() const;
		bool	checkQuery() const;
		bool	checkMethod() const;
		bool	checkHTTP() const;
		void	checkBasicHeaders();
		bool	checkHost(const std::unordered_map<std::string, std::string>& headers);
		bool	checkContentLength(const std::unordered_map<std::string, std::string>& headers);
		void	checkServerDependentHeaders(const Server& server, const Location& location);
		bool	checkMatchURI(const Server& server, const Location& location);
		void	checkFile();
		std::string	checkRequestURI(int mode);
		bool    checkAllowedMethods(const Location& location);
		bool	checkBodyLength(const Server& server, const Location& location);

		std::string	getErrorCode();
		int			getState();
		Request&	getRequest();

		std::string		trim(std::string str);
};

#endif
