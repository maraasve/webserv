/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:06:59 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/07 18:20:21 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "./Request.hpp"
# include "./Server.hpp"
# include "./Response.hpp"
# include "../Networks/Epoll.hpp"

# include <string>
# include <unordered_map>

class Client {
	private:
		int				_fd;
		Server*			_serverPtr;
		Epoll&			_epoll;
		std::string		_requestString; //better in request class?
		std::string		_responseString; //better in response clasee??
		Request			_request; //inhertis from _requestParser
		Response		_response;

	public:
		Client(int fd, Epoll& epoll);

		bool					readRequest();
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
