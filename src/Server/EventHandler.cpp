#include "EventHandler.hpp"

EventHandler::~EventHandler() {}

ssize_t EventHandler::readIncomingData(std::string &appendToStr, int fd)
{
	char buffer[BUFSIZ];

	ssize_t bytes = recv(fd, buffer, BUFSIZ, MSG_DONTWAIT);
	if (bytes > 0)
	{
		appendToStr.append(buffer, bytes);
	}
	return bytes;
}
