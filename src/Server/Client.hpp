/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:06:59 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/16 17:54:08 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "./Request.hpp"
# include "./Server.hpp"
# include "./Response.hpp"
# include "../Networks/Epoll.hpp"
# include "EventHandler.hpp"

# include <string>
# include <unordered_map>

enum clientState {
	PARSE_HEADER = 0,
	PARSE_BODY,
	READ_CGI,
	WRITE_CGI,
	ERROR,
	SEND,
	READY
};

class Client : public EventHandler {
	private:
		int				_state;
		int				_fd;
		Server*			_serverPtr;
		Epoll&			_epoll;
		id_t			_socketFd;
		std::string		_requestString;
		std::string		_responseString; 
		Request			_request; //inhertis from _requestParser
		Response		_response;

	public:
		Client(int fd, Epoll& epoll, int socket_fd);

		void					parseRequest();
		bool					sendResponse();

		void					setRequestStr(std::string request);
		void					setResponseStr(Request& request);
		void					setServer(std::unordered_map<int, std::vector<Server*>>	_socketFdToServer);

		int				getFd();
		std::string&	getRequestStr();
		std::string&	getResponseStr();
		Server*			getServer();
		Request&		getRequest();

		void closeConnection();
};

#endif
