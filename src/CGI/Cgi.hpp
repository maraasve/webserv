
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: maraasve <maraasve@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 16:10:01 by maraasve          #+#    #+#             */
/*   Updated: 2025/03/31 13:34:56 by maraasve         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <iostream>
# include <sstream>
# include <map>
# include <vector>
# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <signal.h>

# include "../Server/EventHandler.hpp"
# include "../Server/WebServer.hpp"


# define TIMEOUT 5000

enum class cgiState {
	INITIALIZED = 0,
	SENDING_BODY,
	RUNNING,
	READING_OUTPUT,
	COMPLETE,
	ERROR
};

class Client;

class Cgi : public EventHandler {
	private:
		const std::string	_filePathString;
		const std::string	_extension;
		const char			*_filePath;
		char				**_args;
		const std::string	_execPath;
		int					_exitStatus;
		cgiState			_state;
		int					_writeToChild[2];
		int					_readFromChild[2];
		pid_t				_cgiPid;
		std::string			_body;
		std::string			_method;
		Client*				_client;
		char				**_env;
	
		char**  		vecTo2DArray(std::vector<std::string>& vec);
		void			freeArgs(char **array);
		char**			setArgs();
	public:
		Cgi() = default;
		Cgi(const std::string& file_path, const std::string& extension, const std::string& method, Client* client);
		~Cgi();

		void			startCgi();
		void 			executeChildProcess();

		void			setBody(std::string body);
		bool			setUpEnvironment();
	
		void			handleIncoming() override;
		void			handleOutgoing() override;
		void 			errorHandler(char **array);
		
		bool			childExited();
		int				getExitStatus() const;
		cgiState 		getState() const;
        int         	getWriteFd();
        int         	getReadFd();
		std::string		getExecPath();
		std::string		getBody() const;

		std::function<void(int)> onCgiPipeDone;
};

#endif
