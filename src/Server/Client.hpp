#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "./Request.hpp"
#include "./Server.hpp"
#include "../Networks/Epoll.hpp"

#include <string>
#include <unordered_map>

class Client {
private:
	int					_fd;
	Server*			_server_ptr;
	std::string	_requestString;
	std::string	_responseString;

public:
	Client(int fd);
	~Client() = default;

	void					handleRequest(int event_fd, Epoll& epoll);
	void					handleResponse(Epoll& epoll);

	void					setRequestStr(std::string request);
	void					setResopnseStr(std::string response);
	void					setServer(Server* server);

	int						getFd();
	std::string&	getRequestStr();
	std::string&	getResponseStr();
	Server*				getServer();

};

#endif
