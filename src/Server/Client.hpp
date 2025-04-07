/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 15:06:59 by maraasve          #+#    #+#             */
/*   Updated: 2025/04/07 17:02:01 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "./Request.hpp"
# include "./Server.hpp"
# include "../Networks/Epoll.hpp"

# include <string>
# include <unordered_map>

class Client {
	private:
		int				_fd;
		Server*			_server_ptr;
		Epoll&			_epoll;
		std::string		_requestString;
		std::string		_responseString;
		Request			_request; //--> RequestParser

	public:
		Client(int fd, Epoll& epoll);
		~Client() = default;

		bool					readRequest();
		bool					handleResponse();

		void					setRequestStr(std::string request);
		void					setResopnseStr(std::string response);
		void					setServer(Server* server);

		int				getFd();
		std::string&	getRequestStr();
		std::string&	getResponseStr();
		Server*			getServer();

		void closeConnection();
};

#endif
