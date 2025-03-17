#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "Socket.hpp"
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
	// Socket socket;
	int epollfd;
	int ready_fds;
	struct epoll_event events[MAX_EVENTS];

public:
	Epoll();
	~Epoll();

	void addFD(int event_type, int client_fd);
	void deleteFD(int client_fd);
	int getReadyFDs();
	struct epoll_event* getEvents();

};

#endif
