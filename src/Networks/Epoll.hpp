#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "Socket.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 3000

//the length of the struct epoll_events array pointed to by events;

/*int epoll_create(int size)
-> Creates a new epoll() instance, since Linux 2.6.8 the size argument
is ignored but it must be greater than zero.
-> It returns a file descriptor referring to the new epoll instance. It should be closed by close()
*/

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
	void modifyFd(int client_fd, int event_type)
	int getReadyFd();
	struct epoll_event* getEvents();

};

#endif
