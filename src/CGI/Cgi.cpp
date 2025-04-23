#include "Cgi.hpp"

Cgi::Cgi(const std::string& file_path, const std::string& extension)
	: _filePathString(file_path)
	, _extension(extension)
	, _args(setArgs())
	, _execPath(getExecPath())
	, _filePath(file_path.c_str())
	, _exitStatus(0)
	, _state(cgiState::INITIALIZED)
	, _cgiPid(-1)
	, _env(nullptr)
{
	if (!_args || _execPath.empty()) {
		errorHandler();
		return ;
    }
}

Cgi::~Cgi() {
	if (_cgiPid == 0) {
		std::cout << "The child process CGI is being destroyed" << std::endl;
	} else if (_state == cgiState::ERROR) {
		std::cout << "The CGI had an error" << std::endl;
	}
	freeArgs();
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

void	Cgi::handleIncoming() {
	char buffer[BUFSIZ];
	ssize_t bytes = read(_readFromChild[0], buffer, sizeof(buffer));
	if (bytes > 0) {
		_body.append(buffer, bytes);
		_state = cgiState::READING_OUTPUT;
	} else if ((bytes == 0 && childFailed()) || bytes != 0) {
		errorHandler();
		return ;
	} else if (bytes == 0 && onCgiPipeDone) {
		onCgiPipeDone(_readFromChild[0]);
		close(_readFromChild[0]);
		_readFromChild[0] = -1;
		_state = cgiState::COMPLETE; //here is the right state right?
	}
}

void	Cgi::handleOutgoing() {
	ssize_t bytes = write(_writeToChild[1], _body.c_str(), _body.size());
	if (bytes > 0) {
		_body.erase(0, bytes);
		_state = cgiState::SENDING_BODY;
	} else if (bytes == 0 && onCgiPipeDone) {
		onCgiPipeDone(_writeToChild[1]); //delete it from epoll and from the event handler
		close(_writeToChild[1]); //we have to close the reading end as well but only once we know the child has done reading from it
		_writeToChild[1] = -1;
		if (bytes != 0) {
			errorHandler();
			return ;
		}
		_state = cgiState::RUNNING; //here is the right state right?
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
			errorHandler();
			return nullptr;
		}
	}
	array[size] = nullptr;
	return array;
}

char**	Cgi::setArgs() {
	std::vector<std::string>	argsv;

	if (_extension == "py")
		argsv.emplace_back("python3");
	else if (_extension == "php")
		argsv.emplace_back("php-cgi");
	argsv.emplace_back(_filePath);
	vecTo2DArray(argsv);
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
	close(_writeToChild[1]);
	close(_readFromChild[0]);
	if (dup2(_writeToChild[0], STDIN_FILENO) == -1 ||
	dup2(_readFromChild[1], STDOUT_FILENO) == -1) {
		exit(1);
	}
	close(_writeToChild[0]);
	close(_readFromChild[1]);
	execve(_execPath.c_str(), _args, _env);
	exit(1);
}

void	Cgi::startCgi() {
	if(pipe(_writeToChild) == -1 || pipe(_readFromChild) == -1) {
		errorHandler();
		return ;
	}
	_cgiPid = fork();
	if (_cgiPid < 0) {
		errorHandler();
		return ;
	}
	if (_cgiPid == 0) {
		executeChildProcess();
	} else {
		close(_writeToChild[0]);
		close(_readFromChild[1]);
	}
}

int         Cgi::getWriteFd() {
	return _writeToChild[1];
}

int         Cgi::getReadFd() {
	return _readFromChild[0];
}

void	Cgi::freeArgs() {
	if (_args) {
		size_t	i = 0;
		while (_args[i]) {
			free(_args[i]);
			i++;
		}
		free(_args);
		_args = nullptr;
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

void Cgi::errorHandler() {
	_state = cgiState::ERROR;
	freeArgs();
}
