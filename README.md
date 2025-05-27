# Webserver
A C++ project focused on building a lightweight HTTP web server capable of handling core HTTP methods such as GET, POST, and DELETE. The server is designed from scratch with support for multiple simultaneous clients using epoll for efficient I/O multiplexing. It also features CGI (Common Gateway Interface) execution for dynamic content handling. The goal of the project is to deepen understanding of low-level networking, HTTP protocol mechanics, and scalable server architecture.

## Build Instructions
First git clone it in your root directory with the name WEBSERVER
```bash
git clone git@github.com:andmadri/CUBE3D.git WEBSERVER
```

Afterwards move to WEBSERVER and run make all, once it's done, you need the executable `webserv` and a configuration file, all available configuration files are inside the configuration_files/ folder. If no configuration file is specified, then a default.conf file will be used instead.
```bash
cd WEBSERVER
make all
...
./webserv configuration_files/default.conf
```

## Configuration File
The configuration file takes inspiration from the `server` section of the NGINX configuration file.
```
server {
	listen 8081;                  #Choose a port
	host 127.0.0.1;               #Choose a host
	server_name mywebsite.com;    #Set up server names
	location / {
    root /variables;            #Directory where the requested file should be located
		allowed_methods GET;
		index index_2.html;         #Set up a default file to serve
	}

	location /cgi-bin {
		root /variables/cgi;
		client_max_body 1g;        #Set the maximum size for the request body
		allowed_methods GET POST;  #List of accepted HTTP methods
		upload_dir /data/uploads;  #Accept where files can be saved
	}

  location /gallery {
		root /variables;
		auto_index on;             #Enable or disable directory listing
	}

	location /old_blog{
    return 301 /blog;          #Define HTTP Redirections
	}
```

## Epoll & Sockets
An HTTP web server uses sockets—special file descriptors bound to an IP address and port—to accept and handle client connections. Think of the IP address as the street name of an apartment building and the port as the specific apartment door.

After parsing the configuration file, the server knows which addresses and ports it must listen on. For each one, it:

1. Creates a socket and binds it to the designated IP + port.
2. Listens on that socket so it can accept incoming client connections.
3. Registers the socket’s file descriptor with an epoll instance, so the server can efficiently wait for—and respond to—new connection requests.

Epoll is like a “watch list” for file descriptors: you add descriptors to it `addFd()`, tell it what events you care about `(EPOLLIN, EPOLLOUT, EPOLLHUP)`, and then call `epoll_wait()`. When descriptors become “ready” (for example, a client is trying to connect, or data is available to read), `epoll_wait()` returns a list of those ready descriptors so your server can handle them immediately.

### Epoll Event Flags
When using epoll to drive our event loop, we register file descriptors with one or more "event types" that tell epoll what to watch for. Although there are countless flags offered by epoll, the most important flags in our server are:

- **EPOLLIN**
  Indicates the socket is ready for *reading*.  
  Use `handler->handleIncoming()` to process it.
  - For a listening socket (server sockets), it means **new client connections** are waiting to be `accept()`-ed.
  - For a client or CGI pipe, it means **incoming data**(HTTP request bytes or CGI output) can be `recv()` (in case of sockets) or `read()` (in case of pipes) without blocking.
  - Once we have read everything from a Client/CGI we `modifyFd()` so that we can now monitor `EPOLLOUT`.

- **EPOLLOUT**
  Indicates the socket is ready for *writing*.  
  Use `handler->handleOutgoing()` to flush our buffers.
  - Used when we have buffered response data (HTTP headers, body or CGI input) to send.

- **EPOLLHUP**
  Indicates that a pipe has hung up or closed its end of the connection.
  For example, when reading CGI script output, once the child process closes its write end, EPOLLIN will no longer trigger.
  Without EPOLLHUP, we wouldn’t return to handler->handleIncoming() to mark the CGI as COMPLETE.
  By watching for EPOLLHUP, we detect the closure and can transition the CGI state appropriately.

## Main Loop
At the core of the server is the `Webserver::run()` method, which drives the event loop. An **event** is a notification from the OS (via epoll) that something interesting has happened on a monitored file descriptor—typically that it’s ready for reading (`EPOLLIN`), ready for writing (`EPOLLOUT`), or has been closed by its peer (`EPOLLHUP`).

In `Webserver::run()`, each cycle calls `epoll_wait()`, which returns a list of these events. The webserver then iterates through them and dispatches each one, calling `handleIncoming()` or `handleOutgoing()` on the appropriate handler.

### Polymorphism
In order to keep the `Webserver::run()` loop as clean as possible we implemented polymorphism. This is achieved through the use of an abstract base class (`EventHandler`) and **virtual methods** (`handleIncoming()`) and (`handleOutgoing`).

The abstract interface looks like this:
```c++
class EventHandler {
protected:
	virtual ~EventHandler();

public:
	virtual void handleIncoming() = 0;
	virtual void handleOutgoing() = 0;
};
```
The methods are **pure virtual** (`= 0`), meaning derived classes such as `Server`, `Client` and `CGI` *must* override them; each class **provides its own implementation**. These functions are not implemented at all in the `EventHandler` class. The destructor is virtual to ensure correct cleanup.

#### Usage in the Event Loop
We have an *unordered_map* called `_eventHandlers` where we store an int (file descriptor) and an EventHandler object(`Server`, `Client` or `CGI`). We then try to find the file descriptor returned by `epoll_wait()` inside the map. If we find it, it will call the appropriate function:

```c++
void WebServer::run()
{
	setupServerSockets(_epoll);

	while (!shutdownRequested)
	{
		checkTimeouts();
		int ready_fds = _epoll.getReadyFd(); //function to call epoll_wait;
		struct epoll_event *ready_events = _epoll.getEvents();
		for (int i = 0; i < ready_fds; ++i)
		{
			struct epoll_event event = ready_events[i];
			int event_fd = event.data.fd;
			auto handler = _eventHandlers.find(event_fd);
			if (handler != _eventHandlers.end())
			{
				if (event.events & EPOLLIN || event.events & EPOLLHUP)
				{
					handler->second->handleIncoming();
				}
				if (event.events & EPOLLOUT)
				{
					handler->second->handleOutgoing();
				}
			}
		}
	}
	cleanUpResources();
}
```

### Setting Up Servers
The `WebServer::setupServerSockets()` method is responsible for creating, binding, and listening on  appropriate sockets for each configured server. We avoid creating multiple sockets that bind to the same IP and port by keeping track of already-used (host, port) pairs. We create a hash map `addressToFd` to store a unique socket for each `(host, port)` pair.
```
std::unordered_map<std::pair<u_long, unsigned int>, std::shared_ptr<Socket>, hashPair>
```
For each Server instance, we retrieve its host and port. We check if this combination has already been bound. If it has not yet been bound, we create a new Socket object, the socket is bound and set to listen, then it is added to the `epoll` instance with `EPOLLIN`, so we get notified when clients connect. The `Server` is linked to this shared socket. A callback `onClientAccepted` is set to handle new client connections. The socket file descriptor is associated with the `Server` object in the `_eventHandlers` map for event dispatching. The `Server` object is been made a shared pointer, wich will be stored as an EventHandler pointer and therefore allowed to perfome the polymorphic dessign on the `Webserver::run()` function. In the case the combinantion of (host, port) has already been bound, it simply reuses the same Socket object. 

```c++
void WebServer::setupServerSockets(Epoll &epoll)
{
	std::unordered_map<std::pair<u_long, unsigned int>, std::shared_ptr<Socket>, hashPair> addressToFd;

	for (auto &server : _servers)
	{
		u_long host = server.getHost_u_long();
		unsigned int port = server.getPort();
		std::pair<u_long, unsigned int> key = {host, port};

		int socketFd;
		if (addressToFd.find(key) == addressToFd.end())
		{
			auto serverSocket = std::make_shared<Socket>();
			socketFd = serverSocket->getSocketFd();
			serverSocket->bindSocket(port, host);
			serverSocket->listenSocket();
			epoll.addFd(socketFd, EPOLLIN);
			addressToFd[key] = serverSocket;
			server.setSocket(serverSocket);
			server.onClientAccepted = [this, &server](int client_fd)
			{
				this->handleNewClient(client_fd, server);
			};
			_eventHandlers[socketFd] = std::make_shared<Server>(server);
		}
		else
		{
			server.setSocket(addressToFd[key]);
		}
	}
}
```

## Server
In the file descriptor pointed by `epoll_wait()` matches that of a saved server file descriptor in the `_eventHandlers` map. The corresponding `handleIncoming()` function from the `Server` will be called. This function will `acceptConnection()` to create a client socket and will call the previoulsy mentioned call back function defined when we set up the servers. 

```c++
void Server::handleIncoming()
{
	int client_fd = _serverSocket->acceptConnection();
	if (client_fd > 0 && onClientAccepted)
	{
		onClientAccepted(client_fd);
	}
}
```

`onClientAccepted()` will call the function `handleNewClient()` which takes the client file descriptor passed by during the call to `handleIncoming()` and a reference to a Server object wich we catched through a lambda will setting up the servers. This function will create a new `Client` object which is been made a shared pointer to store it in the `_eventHandlers` map with its appropriate filedescriptor. Something interesting to point out, thanks to call back functionnality, is that because this function is defined in the `WebServer` context, we have access to this private member variables that do not exist for instance inside our `Server` or `Client` class. Afterwards we add the client file descriptor to epoll with `EPOLLIN` to monitor when the client wants to be read from. We continue to predefine call back functions which the client will be able to use during its execution through `handleIncoming()` such as being able to `assignServer()` als in the case of a `onCgiAccepted()` and when we need to `closeClientConnection()`.

```c++
void WebServer::handleNewClient(int client_fd, Server &server)
{
	auto newClient = std::make_shared<Client>(client_fd, _epoll, server.getSocketFd());
	_eventHandlers[client_fd] = newClient;
	_epoll.addFd(client_fd, EPOLLIN);

	std::weak_ptr<Client> weakClient = newClient;
	newClient->assignServer = [this](Client &client)
	{
		this->assignServer(client);
	};
	newClient->onCgiAccepted = [this, weakClient](int cgiFd, int event_type)
	{
		if (auto client = weakClient.lock())
		{
			_epoll.addFd(cgiFd, event_type);
			auto newCgi = client->getCgi();
			_eventHandlers[cgiFd] = newCgi;
			newCgi->onCgiPipeDone = [this](int cgiFd)
			{
				_epoll.deleteFd(cgiFd);
				_eventHandlers.erase(cgiFd);
			};
			newCgi->closeInheritedFds = [this]()
			{
				for (auto it : _eventHandlers)
				{
					close(it.first);
				}
				close(_epoll.getEpollFd());
			};
		}
	};
	newClient->closeClientConnection = [this](int client_fd)
	{
		_epoll.deleteFd(client_fd);
		_eventHandlers.erase(client_fd);
		close(client_fd);
	};
}
```

### Why a shared & weak pointers?
We use `shared_ptr` for objecst like `Server` `Client` and `CGI` because these objects are owned and managaed by multiple parts of the system. For example, both the `_eventHandlers` map and any lambdas/callbacks need to access and potentially keep them alive.

- `shared_ptr`
  Handles automatic memory management. When no more references to the object remain, it gets destroyed automatically. This prevents memory leaks and manual `delete` calls.
  It allows safe, flexible ownership transfer and access across the event-driven architecture of our server.

#### Why not only use `shared_ptr`?
Using `shared_ptr` everywhere can lead to **cyclic references**, which cause **memory leaks** as the reference count of that `shared_ptr` never reach zero.

#### What are **cyclic references**?
They happen when two ore more `shared_ptr` instances reference each other, creating a *reference cycle*. This means that each object keeps the other alive by holding a `shared_ptr` to it. As a result, their reference counts *never reach zero*, and the objects are never destroyed, even if nothing else is using them. To avoid this, we need to make **one of the pointers** a `weak_ptr`. This way, the reference does not increase the count, allowinf proper cleanup.

Lambdas can catch from the context you are defining them multiple things, they can catch references to objects or pointers. As an example, this caused a **cyclic reference**:

```c++
auto newClient = std::make_shared<Client>(client_fd, _epoll, server.getSocketFd());
newClient->onCgiAccepted = [this, newClient](int cgiFd, int event_type)
{	_epoll.addFd(cgiFd, event_type);
	auto newCgi = newClient->getCgi();
	_eventHandlers[cgiFd] = newCgi;

	newCgi->onCgiPipeDone = [this](int cgiFd)
	{
		_epoll.deleteFd(cgiFd);
		_eventHandlers.erase(cgiFd);
	};
};
```

As it can be seen above, after newClient is been made a `shared_ptr`, we are capturing this `shared_ptr` directly in the lambda by value, so the closure holds its own `shared_ptr<Clien>`. The lambda is assigned to a **member** of the Client object, `Client::onCgiAccepted`, so:
- The **Client Object** (let's call it `A`) conatins the lambda.
- The lambda contains a copy of `newClient`, which is the same as `shared_ptr<Client>` that points back to `A`.
Therefore the reference count of C is at least two, one from the map and one from the lambda. Event when we erase `A` from our `_eventHandlers` map once we close the client connection, the lambda's copy remains, so the counf never drops to zero, and `A` is never destroyed.

#### How does `weak_ptr` breaks the cycle?
By changing the code so:

```c++
std::weak_ptr<Client> weakClient = newClient;
newClient->onCgiAccepted = [this, weakClient](…){
    if (auto client = weakClient.lock()) {
        ...
    }
};
```
The lambda now holds a **non-owning** `weak_ptr`. This does not increment the reference count of `A`. When no more `shared_ptr` owners exist, `A` can be destroyed even if the lambda still exists. Therefore using a `weak_ptr` in the lambda **breaks** the self-referencing cycle and prevents memeory leaks. 

### How do we assign servers to clients?
Although it might be intuitive to think that the `Server` object who accepted and created a `Client` is the corresponding `Server` to said `Client`, we can have to or more servers with the same host and port. Therefore the only way to truly know to which server the client belongs is to check the `server_names` of a server. 

```c++
void WebServer::assignServer(Client &client)
{
	int fd = client.getSocketFd();
	std::string host = client.getRequestParser().getRequest().getHost();
	Server *fallback = nullptr;
	for (Server &server : _servers)
	{
		if (fd == server.getSocketFd())
		{
			for (std::string serverName : server.getServerNames())
			{
				if (strcasecmp(host.c_str(), serverName.c_str()) == 0)
				{
					client.setServer(server);
					return;
				}
			}
			if (!fallback)
			{
				fallback = &server;
			}
		}
	}
	if (fallback)
	{
		client.setServer(*fallback);
	}
	else
	{
		client.getRequest().setErrorCode("400");
	}
}
```

## Clients
The `Client` class represents a single HTTP connection and drives its lifecylce through a simple **state machine**, handling reading, parsing, optional CGI execution, and writing the response. 

```c++
enum class clientState {
    READING_HEADERS,
    READING_BODY,
    PARSING_CHECKS,
    CGI,
    RESPONDING,
    ERROR
};


void Client::handleIncoming()
{
	switch (_state)
	{
		case clientState::READING_HEADERS:
			handleHeaderState();
			break;
		case clientState::READING_BODY:
			handleBodyState();
			break;
		case clientState::PARSING_CHECKS:
			handleParsingCheckState();
			break;
		case clientState::CGI:
			handleCgiState();
			break;
		case clientState::ERROR:
			handleErrorState();
			break;
		case clientState::RESPONDING:
			handleResponseState();
			break;
	}
}

```

The main function of `Client` is `handleIncoming()` which depending on the state of the current call the function proceeds to handling a certain state. All clients will first come to the `handleHeaderState()` function. This functinos reads bytes into a string where we save the HTTP request from the client. Depending on the type of the request, we move on to `READING_BODY`, `PARSING_CHECK` or `RESPONDING`. 

```c++
void Client::handleHeaderState()
{
	std::cout << "\n\t--Handling Header State--" << std::endl;
	ssize_t bytes = readIncomingData(_requestString, _fd);
	if (bytes == 0)
	{
		closeClientConnection(_fd);
	}
	if (bytes < 0)
	{
		_state = clientState::ERROR;
		handleIncoming();
		return;
	}
	if (_requestParser.parseHeader(_requestString))
	{
		if (_requestParser.getRequest().getErrorCode() != "200")
		{
			_request = std::move(_requestParser).getRequest();
			assignServer(*this);
			_state = clientState::RESPONDING;
			handleIncoming();
			return;
		}
		assignServer(*this);
		if (_requestParser.getState() == requestState::PARSING_BODY)
		{
			_state = clientState::READING_BODY;
			_requestParser.parseBody(_requestString, 0);
			if (_requestParser.getRequest().getErrorCode() != "200")
			{
				_request = std::move(_requestParser).getRequest();
				_state = clientState::RESPONDING;
				handleIncoming();
				return;
			}

		}
		if (_requestParser.getState() == requestState::COMPLETE)
		{
			_state = clientState::PARSING_CHECKS;
			handleIncoming();
			return;
		}
	}
}
```

`handleParsingCheckState()` will check to which location from our configuration file the URI from the request best matches, it will try to check if there are any non-allowed methods in said location to the corresponding method that the request had, if there is no Content-Length for the case of POST, if the file the client wants to GET has the right permissions, it inspects if the requested URI extension ends with .py or .php to decide if CGI is required, etc.

## CGI
The CGI enables our webserver to execute external programs or scripts. In accordance to NGINX, the webserver alone whouls be responsible of serving static files. If you want to check the time of the day or be able to upload a file or make a comment on a blog, a CGI is needed. 

A **CGI** is a standard protocol where the webserver `forks()` a separate child process to run a script or program. The webserver and CGI proces communicate via **pipes**. The server writes the HTTP request body (for `POST`) to the child's **stdin** and the child writes its output (typically HTTP body) to **stdout**, which the server reads from a pipe and forwards a response back to the client. 

First `Client::handleParsingCheckState()` calls `shouldRunCgi()` to check the URI extension. If we detect that there should be a CGI, we moved on to `Client::handleCgiState()`. Because this function will be call recursively, we first need to check if we do not already have a `shared_ptr` CGI initialized in our `Client` class. If we dont, then we make a new `shared_ptr` CGI and start the CGI process. 

```c++
if (!_Cgi) {
  _Cgi = std::make_shared<Cgi>(this);
  _Cgi->init();
  _Cgi->startCgi();
}
```

`startCgi()` creates two pipes: **_writeToChild** (server -> CGI stdin) for writing the body of the client request to the child process and **_readFromChild** (CGI stdout -> server) for reading the output of the child process which corresponds to the body of our response.

One of the callback functions that have been previously mentioned is `onCgiAccepted`. This function will add the pipe file descriptor to epoll, and add said pipe file descriptor and the `shared_ptr` cgi to our `_eventHandlers` map so that polymorphism can do its magic and call the `handleIncoming()` and `handleOutgoing()` of the CGI. After we call the `fork()` function to start the CGI, the child process which is receives the process id of 0, will execute the script by dupping the pipes we created before to *stdin* and *stdout*.

```c++
void Cgi::startCgi()
{
	std::cout << "CGI: Starting CGI" << std::endl;
	_startTimeCgi = std::chrono::steady_clock::now();
	if (_method == "POST")
	{
		if (pipe(_writeToChild) == -1)
		{
			errorHandler(_args);
			_client->handleIncoming();
			return;
		}
		_client->onCgiAccepted(_writeToChild[1], EPOLLOUT);
	}
	if (pipe(_readFromChild) == -1)
	{
		errorHandler(_args);
		_client->handleIncoming();
		return;
	}
	_client->onCgiAccepted(_readFromChild[0], EPOLLIN | EPOLLHUP);
	_cgiPid = fork();
	if (_cgiPid < 0)
	{
		std::cerr << "CGI: Error Forking" << std::endl;
		errorHandler(_args);
		_client->handleIncoming();
		return;
	}
	if (_cgiPid == 0)
	{
		executeChildProcess();
	}
	else
	{
		std::cout << "CGI: Parent Process Forked & Closing Unnecessary Pipes" << std::endl;
		if (_method == "POST")
		{
			close(_writeToChild[0]);
			_writeToChild[0] = -1;
		}
		close(_readFromChild[1]);
		_readFromChild[1] = -1;
	}
}

void Cgi::executeChildProcess()
{
	std::cout << "CGI: Child Executing" << std::endl;
	if (_method == "POST")
	{
		close(_writeToChild[1]);
		_writeToChild[1] = -1;
		if (dup2(_writeToChild[0], STDIN_FILENO) == -1)
		{
			errorHandler(_args);
			exit(1);
		}
		close(_writeToChild[0]);
		_writeToChild[0] = -1;
	}
	close(_readFromChild[0]);
	_readFromChild[0] = -1;
	if (dup2(_readFromChild[1], STDOUT_FILENO) == -1)
	{
		errorHandler(_args);
		exit(1);
	}
	close(_readFromChild[1]);
	_readFromChild[1] = -1;
	closeInheritedFds();
	execve(_execPath.c_str(), _args, _env);
	freeArgs(_args);
	freeArgs(_env);
	exit(1);
}
```

In the main loop, once the child process indicates that is ready to be read, we will go to the CGI own overwritten `handleIncoming()`, which after checking the the child has exited successfully, we will call the _client->handleIncoming() function as now the _state of the CGI is changed to COMPLETE and we can continue to process the response for the client.

```c++
void Cgi::handleIncoming()
{
	char buffer[BUFSIZ];
	std::cout << "CGI: Parent reads output from Child" << std::endl;
	ssize_t bytes = read(_readFromChild[0], buffer, sizeof(buffer));
	if (bytes > 0)
	{
		_body.append(buffer, bytes);
	}
	else if (bytes < 0)
	{
		errorHandler(_args);
		_client->handleIncoming();
		return;
	}
	if (childExited() || !bytes)
	{
		if (WIFEXITED(_exitStatus))
		{
			_exitStatus = WEXITSTATUS(_exitStatus);
		}
		if (_exitStatus)
		{
			_state = cgiState::ERROR;
		}
		else
		{
			_state = cgiState::COMPLETE;
			std::cout << "CGI: COMPLETE" << std::endl;
		}
		if (onCgiPipeDone)
		{
			onCgiPipeDone(_readFromChild[0]);
		}
		close(_readFromChild[0]);
		_readFromChild[0] = -1;
		_client->handleIncoming();
		return;
	}
```










