#include "Epoll.hpp"

Epoll::Epoll(): epollfd(epoll_create(1024)) {
	if (epollfd < 0) {
		throw std::exception();
	}
}

Epoll::~Epoll() {
	
	close(epollfd); //close can fail you need to protect that
	std::cout << "Epoll file descriptor closed" << std::endl;
}

// void Epoll::EpollWatch(int fd) const {
// 	if (epoll_ctl(epollfd, )
// }