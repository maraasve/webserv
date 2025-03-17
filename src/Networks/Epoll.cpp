#include "Epoll.hpp"

Epoll::Epoll(): epollfd(epoll_create(1024)), ready_fds(0) {
	error_check(epollfd, "Epoll create");
	std::cout << "Epoll instance created" << std::endl;
}

Epoll::~Epoll() {
	if (epollfd > 0) {
		close(epollfd); //close can fail you need to protect that
	}
	std::cout << "Epoll instance closed" << std::endl;
}

void Epoll::addFd(int client_fd, int event_type) {
	struct epoll_event event {};

	// event.events = EPOLLIN | EPOLLOUT;
	event.events = event_type;
	event.data.fd = client_fd;

	int val = epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &event);
	error_check(val, "Epoll add file descriptor");
	std::cout << "Socket fd: " << client_fd << " has been added to the epoll watch list" << std::endl;

}

void Epoll::deleteFd(int client_fd) {
	int val = epoll_ctl(epollfd, EPOLL_CTL_DEL, client_fd, nullptr);
	error_check(val, "Epoll delete file descriptor");
	std::cout << "Socket fd: " << client_fd << " has been deleted from the epoll watch list" << std::endl;
}

int Epoll::getReadyFd()
{
	int timeout = 100;
	//this needs to be called in a while loop so that you keep on getting the events again and again
	ready_fds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
	//if the socket has data available to read, epoll_wait() will return with EPOLLIN set
	//if the socket has data ready to write (i.e., the send buffer is not full), epoll_wait() will return with EPOLLOUT set.
	//To handle this:
	//if (events[i].events & EPOLLIN)
	//if (events[i].events & EPOLLOUT)
	error_check(ready_fds, "Epoll waiting on file descriptor");
	return ready_fds;
	//you must loop and read until recv() returns EAGAIN or EWOULDBLOCK, which means there's not more data for now
	//if bytes_read is == -1 and EAGAIN (no more data for now), then continue
}

struct epoll_event* Epoll::getEvents() {
	return events;
}

void Epoll::error_check(int val, const std::string& msg) const {
    if (val < 0) {
				if (epollfd >= 0) {
					close(epollfd);
				}
        throw std::runtime_error(msg + ": " + strerror(errno));
    }
}
