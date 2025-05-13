#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "Socket.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 3000

class Epoll {
private:

	int epollfd;
	int ready_fds;
	struct epoll_event events[MAX_EVENTS];
	void error_check(int val, const std::string& msg) const;

public:
	Epoll();
	~Epoll();

	void addFd(int client_fd, int event_type);
	void deleteFd(int client_fd);
	void modifyFd(int client_fd, int event_type);
	int getReadyFd();
	int getEpollFd();
	struct epoll_event* getEvents();

};

#endif
