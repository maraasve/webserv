#ifndef EVENTHANDLER_HPP
# define EVENTHANDLER_HPP

# include <iostream>
# include <sys/socket.h>

class EventHandler {
	protected:
		virtual			~EventHandler();
	
	public:
		virtual	void	handleIncoming() = 0;
		virtual	void	handleOutgoing() = 0;
		ssize_t			readIncomingData(std::string& appendToStr, int fd);
		//				writeOutgoingData();
};

#endif