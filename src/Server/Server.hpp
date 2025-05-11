#ifndef SERVER_HPP
#define SERVER_HPP

#include "./Request.hpp"
#include "../Networks/Socket.hpp"
#include "../Networks/Epoll.hpp"
#include "EventHandler.hpp"
#include "./Location.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <stdbool.h>
#include <unordered_map>

class Server : public EventHandler {
private:
	unsigned int									_port = 8080;
	u_long											_host_u_long = INADDR_ANY;
	std::string										_host_string = "0.0.0.0";
	unsigned long long								_client_max_body = 1048576;
	std::vector<std::string>						_server_names {""};
	std::string										_root;
	std::string										_index;
	bool											_auto_index = false;
	std::unordered_map<std::string, std::string>	_error_page;
	std::vector<Location>							_locations;
	std::shared_ptr<Socket>							_serverSocket;
	
public:
	Server() = default;
	~Server() = default;
	
	void	handleIncoming() override;
	void	handleOutgoing() override;
	
	void	setPort(int port);
	void	setAutoIndex(bool auto_index);
	void 	setErrorPage(std::string error_code, std::string path);
	void 	setHost_u_long(u_long host);
	void	setHost_string(std::string host);
	void	setClientMaxBody(unsigned long long client_max_body);
	void	setServerNames(std::vector<std::string> server_names);
	void	setIndex(std::string index);
	void	setRoot(std::string root);
	void	setSocket(const std::shared_ptr<Socket>& socket);
	
	std::unordered_map<std::string, std::string>	getErrorPage();
	std::vector<std::string>						getServerNames() const;
	std::vector<Location>&							getLocations();
	unsigned long long								getClientMaxBody() const;
	std::string 									getRoot() const;
	std::string 									getIndex() const;
	std::string 									getHost_string() const;
	u_long 											getHost_u_long() const;
	bool 											getAutoIndex() const;
	int 											getPort() const;
	int												getSocketFd();
	
	std::function<void(int)>						onClientAccepted;
};

#endif
