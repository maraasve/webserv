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

## Servers

## Clients

### Request

### Response

## CGI

### Timeout Handling & Errors in Script







