#include "Epoll.hpp"

Epoll::Epoll() : epollfd(epoll_create(1024)), ready_fds(0)
{
	error_check(epollfd, "Epoll create");
	std::cout << "Epoll instance created" << std::endl;
}

Epoll::~Epoll()
{
	if (epollfd > 0)
	{
		close(epollfd);
	}
	std::cout << "Epoll instance closed" << std::endl;
}

void Epoll::addFd(int client_fd, int event_type)
{
	struct epoll_event event{};

	event.events = event_type;
	event.data.fd = client_fd;

	int val = epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &event);
	error_check(val, "Epoll add file descriptor");
	std::cout << "Socket fd: " << client_fd << " has been added to the epoll watch list" << std::endl;
}

void Epoll::modifyFd(int client_fd, int event_type)
{
	struct epoll_event event{};

	event.events = event_type;
	event.data.fd = client_fd;
	int val = epoll_ctl(epollfd, EPOLL_CTL_MOD, client_fd, &event);
	error_check(val, "Epoll modify file descriptor");
	std::cout << "Socket fd: " << client_fd << " has been modified in the epoll watch list" << std::endl;
}

void Epoll::deleteFd(int client_fd)
{
	int val = epoll_ctl(epollfd, EPOLL_CTL_DEL, client_fd, nullptr);
	error_check(val, "Epoll delete file descriptor");
	std::cout << "Socket fd: " << client_fd << " has been deleted from the epoll watch list" << std::endl;
}

int Epoll::getReadyFd()
{
	int timeout = 100;
	ready_fds = epoll_wait(epollfd, events, MAX_EVENTS, timeout);
	if (ready_fds == -1)
	{
		if (errno == EINTR)
		{
			return (0);
		}
	}
	error_check(ready_fds, "Epoll waiting on file descriptor");
	return ready_fds;
}

int Epoll::getEpollFd()
{
	return epollfd;
}

struct epoll_event *Epoll::getEvents()
{
	return events;
}

void Epoll::error_check(int val, const std::string &msg) const
{
	if (val < 0)
	{
		if (epollfd >= 0)
		{
			close(epollfd);
		}
		throw std::runtime_error(msg + ": " + strerror(errno));
	}
}
