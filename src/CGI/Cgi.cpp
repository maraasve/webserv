#include "Cgi.hpp"
#include "../Server/Client.hpp"

Cgi::Cgi(const std::string& file_path, const std::string& extension, const std::string& method, Client* client)
	: _filePathString(file_path)
	, _extension(extension)
	, _filePath(file_path.c_str())
	, _args(setArgs())
	, _execPath(getExecPath())
	, _exitStatus(0)
	, _state(cgiState::INITIALIZED)
	, _writeToChild{-1, -1}
	, _readFromChild{-1, -1}
	, _cgiPid(-1)
	, _env(nullptr)
	, _method(method)
	, _client(client)
{
	if (!_args || _execPath.empty()) {
		errorHandler(_args);
		return ;
    }
	if (_method == "POST") {
		if (pipe(_writeToChild) ==  -1) {
			errorHandler(_args);
			return;
		}
	}
	if(pipe(_readFromChild) == -1) {
		errorHandler(_args);
		return;
	}
}

Cgi::~Cgi() {
	if (_cgiPid == 0) {
		std::cout << "The child process CGI is being destroyed" << std::endl;
	} else if (_state == cgiState::ERROR) {
		std::cout << "The CGI had an error" << std::endl;
	}
	freeArgs(_args);
}

bool	Cgi::childFailed() {
	int status;
	pid_t result = waitpid(_cgiPid, &status, WNOHANG);
	if (result == _cgiPid) {
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
			return true;
		} else if (WIFSIGNALED(status)) {
			return true;
		}
	}
	return false;
}

//if there is an error I have to call the client's handleincoming
//Is there a better solution?
void	Cgi::handleIncoming() {
	char buffer[BUFSIZ];
	//the readFromChild has ended up being the same file-descriptor number as the client's socket so when your cgi
	ha
	ssize_t bytes = read(_readFromChild[0], buffer, sizeof(buffer));
	if (bytes > 0) {
		printf("This is the body of what I read: %s\n", buffer);
		_body.append(buffer, bytes);
		if (bytes < (ssize_t)sizeof(buffer)) {
			if (onCgiPipeDone) {
				onCgiPipeDone(_readFromChild[0]);
			}
			close(_readFromChild[0]);
			_readFromChild[0] = -1;
			_state = cgiState::COMPLETE;
			std::cout << "CGI: COMPLETE" << std::endl;
			//This should go back to the handleincoming, create the response and be done!
			_client->handleIncoming();
			return;
		}
	} else {
		errorHandler(_args);
	}
}

void	Cgi::handleOutgoing() {
	ssize_t bytes = write(_writeToChild[1], _body.c_str(), _body.size());
	if (bytes > 0) {
		_body.erase(0, bytes);
		_state = cgiState::SENDING_BODY;
		std::cout << "CGI: SENDING_BODY" << std::endl;
	} else if (bytes == 0 && onCgiPipeDone) {
		onCgiPipeDone(_writeToChild[1]); //delete it from epoll and from the event handler
		close(_writeToChild[1]); //we have to close the reading end as well but only once we know the child has done reading from it
		_writeToChild[1] = -1;
		if (bytes != 0) {
			errorHandler(_args);
			return ;
		}
		_state = cgiState::RUNNING; //here is the right state right?
		std::cout << "CGI: RUNNING!" << std::endl;
	}
}

char**  Cgi::vecTo2DArray(std::vector<std::string>& vec) {
	size_t size = vec.size();
	char **array = static_cast<char **>(malloc((size + 1) * sizeof(char *)));
	if (!array) {
		return nullptr;
	}
	for (size_t i = 0; i < size; ++i) {
		array[i] = strdup(vec[i].c_str()); //do we need to use strdup here?
		if (!array[i]) {
			errorHandler(array);
			return nullptr;
		}
	}
	array[size] = nullptr;
	return array;
}

char**	Cgi::setArgs() {
	std::vector<std::string>	argsv;

	if (_extension == "py") {
		argsv.emplace_back("python3");
	}
	else if (_extension == "php") {
		argsv.emplace_back("php-cgi");

	}
	argsv.emplace_back(_filePath);
	return vecTo2DArray(argsv);
}

std::string Cgi::getExecPath() {
	const char* pathStr = getenv("PATH");
	if (!pathStr)
		return "";
	std::string command(_args[0]);
	std::string path;
	std::stringstream stream(pathStr);
	while (std::getline(stream, path, ':')) {
		std::string full_path = path + "/" + command;
		if (access(full_path.c_str(), X_OK) == 0) {
			return full_path;
		}
	}
	return "";
}

void Cgi::executeChildProcess() {
	std::cout << "CGI: Child Executing" << std::endl;
	printf("This is the execution path: %s\n", _execPath.c_str());
	printf("This are the arguments: %s\n", _args[1]);
	if (_method == "POST") {
		close(_writeToChild[1]);
		_writeToChild[1] = -1;
		if (dup2(_writeToChild[0], STDIN_FILENO) == -1) {
			errorHandler(_args);
			exit(1);
		}
		close(_writeToChild[0]);
		_writeToChild[0] = -1;
	}
	close(_readFromChild[0]);
	_readFromChild[0] = -1;
	if (dup2(_readFromChild[1], STDOUT_FILENO) == -1) {
		errorHandler(_args);
		exit(1);
	}
	close(_readFromChild[1]);
	_readFromChild[1] = -1;
	std::cout << _execPath.c_str() << std::endl;
	execve(_execPath.c_str(), _args, environ);
	freeArgs(_args);
	exit(1);
}

void	Cgi::startCgi() {
	_cgiPid = fork();
	if (_cgiPid < 0) {
		std::cerr << "CGI: Error Forking" << std::endl;
		errorHandler(_args);
		return ;
	}
	if (_cgiPid == 0) {
		executeChildProcess();
	} else {
		std::cout << "CGI: Parent Process Forked & Closing Unnecessary Pipes" << std::endl;
		if (_method == "POST") {
			close(_writeToChild[0]); // we are not going to use the read end [0] cause we are going to write to it
			_writeToChild[0] = -1;
		}
		close(_readFromChild[1]);
		_readFromChild[1] = -1;
	}
}

int         Cgi::getWriteFd() {
	return _writeToChild[1];
}

int         Cgi::getReadFd() {
	return _readFromChild[0];
}

void	Cgi::freeArgs(char **array) {
	if (array) {
		size_t	i = 0;
		while (array[i]) {
			free(array[i]);
			i++;
		}
		free(array);
		array = nullptr;
	}
	return ;
}


void	Cgi::setBody(std::string	body) {
	_body = body;
}

std::string	Cgi::getBody() const{
	return _body;
}

cgiState Cgi::getState() const {
	return _state;
}

int	Cgi::getExitStatus() const {
	return _exitStatus;
}

void Cgi::errorHandler(char **array) {
	if (_state == cgiState::ERROR) {
		return;
	}
	_state = cgiState::ERROR;
	freeArgs(array);
	if (_writeToChild[0] != -1) {
		close(_writeToChild[0]);
	}
	if (_writeToChild[1] != -1) {
		close(_writeToChild[1]);
	}
	if (_readFromChild[0] != -1) {
		close(_readFromChild[0]);
	}
	if (_readFromChild[1] != -1) {
		close(_readFromChild[1]);
	}
}
