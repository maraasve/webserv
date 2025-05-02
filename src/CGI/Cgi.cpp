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
	, _env(environ)
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

void	Cgi::handleIncoming() {
	char buffer[BUFSIZ];
	std::cout << "CGI: Parent reads output from Child" << std::endl;
	ssize_t bytes = read(_readFromChild[0], buffer, sizeof(buffer));
	if (bytes > 0) {
		printf("CGI: Output of the child %s\n", buffer);
		_body.append(buffer, bytes);
		if (bytes < (ssize_t)(sizeof(buffer))) {
			if (onCgiPipeDone) {
				onCgiPipeDone(_readFromChild[0]);
			}
			close(_readFromChild[0]);
			_readFromChild[0] = -1;
			_state = cgiState::COMPLETE;
			std::cout << "CGI: COMPLETE" << std::endl;
			_client->handleIncoming();
			return;
		}
	} else {
		errorHandler(_args);
	}
}

void	Cgi::handleOutgoing() {
	std::cout << "I am sending body" << std::endl;
	if (_body.size() > 0) {
		ssize_t bytes = write(_writeToChild[1], _body.c_str(), _body.size());
		if (bytes > 0) {
			std::cout << "Bytes read are above 0" << std::endl;
			_body.erase(0, bytes);
			_state = cgiState::SENDING_BODY;
			std::cout << "CGI: SENDING_BODY" << std::endl;
		} else if (bytes < 0) {
			close(_writeToChild[1]); //we have to close the reading end as well but only once we know the child has done reading from it
			_writeToChild[1] = -1;
			errorHandler(_args);
			return ;
		}
	}
	if (_body.empty() && _writeToChild[1] != -1 && onCgiPipeDone) {
		onCgiPipeDone(_writeToChild[1]);
		close(_writeToChild[1]); 
		_writeToChild[1] = -1;
		_state = cgiState::RUNNING;
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
		array[i] = strdup(vec[i].c_str());
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
	execve(_execPath.c_str(), _args, _env);
	freeArgs(_args);
	exit(1);
}
/*
When you launch a CGI you must give the child two things:

    A stdin‐pipe that closes when you’re done writing (so the child sees EOF).

    An environment (envp) that contains at least these two vars:

CONTENT_LENGTH=⟨number of bytes⟩  
CONTENT_TYPE=⟨the multipart/form-data; boundary=… header⟩  
REQUEST_METHOD=POST  

Once those are in place, Python’s cgi.FieldStorage() will:

    read exactly CONTENT_LENGTH bytes from its STDIN until EOF,

    parse the multipart boundary,

    populate form['file'].

1) Setting the vars in C++

Inside your Cgi ctor—before you fork/exec—do:

// assume client holds the full request body in client->getRequest().getBody()
std::string body = client->getRequest().getBody();
std::string contentType = client->getRequestParser().getHeader("Content-Type");

// overwrite the process‐wide environ:
setenv("REQUEST_METHOD", "POST", 1);
setenv("CONTENT_LENGTH", std::to_string(body.size()).c_str(), 1);
setenv("CONTENT_TYPE", contentType.c_str(), 1);

// now `environ` contains those entries

If you build your own char** envp, push these three strings into it and pass that to execve(..., envp).
2) Ensuring EOF on stdin

In your parent‐side handleOutgoing() you must close the write-end after all bytes are written:

// after writing all of _body:
if (_body.empty() && _writeToChild[1] != -1) {
  // tell epoll to stop watching this fd:
  if (onCgiPipeDone) onCgiPipeDone(_writeToChild[1]);
  close(_writeToChild[1]);  
  _writeToChild[1] = -1;
  std::cout << "CGI: RUNNING!" << std::endl;
}

That close() is the EOF signal on the child’s STDIN.
3) What Python cgi then does

With those two in place, your CGI script:

#!/usr/bin/env python3
import cgi, os, sys

# FieldStorage will look at os.environ['CONTENT_LENGTH'] and CONTENT_TYPE
form = cgi.FieldStorage()  

# it will read exactly CONTENT_LENGTH bytes from sys.stdin until EOF
fileitem = form['file']  

if fileitem and fileitem.filename:
    data = fileitem.file.read()   # now contains the uploaded bytes
    # …save data to disk…

No further changes in Python are needed—once the C++ side closes stdin and provides the right env vars, form['file'] will be non-empty.
Recap

    C++: setenv("CONTENT_LENGTH", …), setenv("CONTENT_TYPE", …), then fork/exec.

    C++: after writing the body into the stdin-pipe, close(write_fd) so the child sees EOF.

    Python: call cgi.FieldStorage() at top; it will automatically read and parse the upload.

With those three steps, your CGI will finally see the uploaded file.
*/
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
			close(_writeToChild[0]);
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

void Cgi::setUpEnvironment() {
	std::string length = std::to_string(_body.size());
	if (setenv("CONTENT_LENGTH", length.c_str(), 1) != 0) {
		errorHandler(_args);
		//we gotta call the client handle incoming or something
		return;
	}
	if(setenv("REQUEST_METHOD", _method.c_str(), 1) != 0) {
		errorHandler(_args);
		//we gotta call the client handle incoming or something
		return;
	}
	auto headers = _client->getRequest().getHeaders();
	auto it = headers.find("Content-Type");
	if (it != headers.end()) {
		std::cout << "Does Content-type includes boundaries: " << it->second.c_str() << std::endl;
		if (setenv("CONTENT_TYPE", it->second.c_str(), 1) != 0) {
			errorHandler(_args);
			//we gotta call the client handle incoming or something
			return;
		}
	}
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
