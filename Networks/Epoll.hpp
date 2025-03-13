#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "Socket.hpp"
#include <sys/epoll.h>
#include <unistd.h>

/*int epoll_create(int size)
-> Creates a new epoll() instance, since Linux 2.6.8 the size argument
is ignored but it must be greater than zero.
-> It returns a file descriptor referring to the new epoll instance. It should be closed by close()
*/

class Epoll {
private:
	// Socket socket;
	int epollfd;
	struct epoll_event event; //need to google about this struct

public:
	Epoll();
	~Epoll();
	void EpollWatch(int fd) const;

};

#endif
