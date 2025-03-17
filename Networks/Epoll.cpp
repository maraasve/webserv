#include "Epoll.hpp"

Epoll::Epoll(): epollfd(epoll_create(1024)), ready_fds(0) {
	if (epollfd < 0) {
		throw std::exception();
	}
}

Epoll::~Epoll() {
	
	close(epollfd); //close can fail you need to protect that
	std::cout << "Epoll file descriptor closed" << std::endl;
}

//add fd
void Epoll::addFD(int event_type, int client_fd) {
	//epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	//EPOLL_CTL_ADD -> Add an entry to the interest list of the epoll fd
	struct epoll_event event {};

	event.events = event_type; //EPOLLIN
	event.data.fd = client_fd;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
		close(epollfd); //check or better not?
		throw std::exception();
	}
}

void Epoll::deleteFD(int client_fd) {
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, client_fd, nullptr) < 0) {
		close(epollfd); //check or better not?
		throw std::exception();
	}
}

int Epoll::getReadyFDs()
{
	//specify the events.events[maxevents] shit;
	//this needs to be called in a while loop so that you keep on getting the events again and again
	ready_fds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
	if (ready_fds < 0) {
		close(epollfd); //should I check if the close failed or I am already throwing an exception anyways?
		throw std::exception();
	}
	return ready_fds;
	//you must loop and read until recv() returns EAGAIN or EWOULDBLOCK, which means there's not more data for now
	//if bytes_read is == -1 and EAGAIN (no more data for now), then continue
}

struct epoll_event* Epoll::getEvents() {
	return events;
}

//EPOLLOUT is needed when the socket buffer to send info back to the client is full
//and therefore you need to know when the socket is ready to write
//Should I use EPOLLOUT?

//in level-triggering your program is repeatedly notified while the event condition persists. Edge-triggering, on the other ahnd, only
//notifies you once when the event transitions from inactive to active, requiring you to process all data promptly