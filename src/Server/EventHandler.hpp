#ifndef EVENTHANDLER_HPP
# define EVENTHANDLER_HPP

# include <iostream>
# include <sys/socket.h>

class EventHandler {
	protected:
		virtual			~EventHandler ();
	
	public:
		virtual	void	handleIncoming();
		virtual	void	handleOutgoing();
		virtual	int		getFd(); //not sure if we need this?
		ssize_t			readIncomingData(std::string& appendToStr, int fd);
		//				writeOutgoingData();
};

#endif