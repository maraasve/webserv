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
# include "../Parsing/RequestParser.hpp"
# include "./Server.hpp"
# include "./Response.hpp"
# include "../Networks/Epoll.hpp"
# include "EventHandler.hpp"
# include "../CGI/Cgi.hpp"

# include <string>
# include <unordered_map>
# include <optional>
# include <set>

enum class clientState {
	READING_HEADERS = 0,
	READING_BODY,
	PARSING_CHECKS,
	CGI,
	RESPONDING,
	COMPLETE,
	ERROR
};

class Client : public EventHandler {
private:
	clientState				_state = clientState::READING_HEADERS;
	int						_fd;
	Server*					_serverPtr;
	Location				_location;
	Epoll&					_epoll;
	id_t					_socketFd; //are we still using this?
	std::string				_requestString;
	std::string				_responseString; 
	Request					_request;
	RequestParser			_requestParser;
	Response				_response;
	std::shared_ptr<Cgi>	_Cgi;

public:
	Client(int fd, Epoll& epoll, int socket_fd);

	
	void									handleIncoming() override;
	void									handleOutgoing() override;
	void									handleHeaderState();
	void									handleBodyState();
	void									handleParsingCheckState();
	void									handleCgiState();
	void									handleResponseState();
	// void									handleErrorState();
	// void									handleCompleteState();
	bool									resolveLocation(std::string uri);
	
	void									setRequestStr(std::string request);
	void									setResponseStr(Request& request);
	void									setServer(Server& server);
	
	int										getFd();
	int										getSocketFd();
	std::string&							getRequestStr();
	std::string&							getResponseStr();
	Server*									getServer();
	Request&								getRequest();
	std::shared_ptr<Cgi>					getCgi();
	
	bool 									shouldRunCgi() const;

	std::function<void()>					assignServer;
	std::function<void(int, int)>			onCgiAccepted;
	std::function<void()>					closeClientConnection;
};

#endif
