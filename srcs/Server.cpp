#include "../includes/Server.hpp"
#include <errno.h>

static volatile int keepRunning = 1;

void intHandler(int dummy)
{
	(void)dummy;
	cout << WHITE << "\nServer closed" << RESET << endl;
	keepRunning = 0;
}

Server::Server()
:   _port(0),
	_socket_fd(-1),
	_epoll_fd(-1),
	_server_passcode("0")
{
	std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::Server(int port, string serverpass)
:   _port(port),
	_socket_fd(-1),
	_epoll_fd(-1),
	_server_passcode(serverpass)
{
	std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::~Server() {}


void Server::close_fds()
{
	map<int, Client>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it)
	{
		if (it->first != -1) 
		{
			if (_epoll_fd != -1)
				epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
			close(it->first);
		}
	}
	_clients.clear();

	if (_socket_fd != -1)
	{
		close(_socket_fd);
		_socket_fd = -1;
	}

	if (_epoll_fd != -1)
	{
		close(_epoll_fd);
		_epoll_fd = -1;
	}
}

void Server::init()
{
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0)
	{
		close(_socket_fd);
		throw runtime_error("socket");
	}

	int opt = 1;
	if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(_socket_fd);
		throw runtime_error("setsockopt");
	}

	if (fcntl(_socket_fd, F_SETFL, O_NONBLOCK) < 0) 
	{
		close(_socket_fd);
		throw runtime_error("fcntl");
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(_port);
	_serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_socket_fd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0)
	{
		close(_socket_fd);
		throw runtime_error("bind");
	}
	if (listen(_socket_fd, 5) < 0)
	{
		close(_socket_fd);
		throw runtime_error("listen");
	}
}

void Server::start()
{
	signal(SIGINT, intHandler);
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd < 0)
	{
		close_fds();
		throw runtime_error("epoll_create1");
	}
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _socket_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev) < 0)
	{
		close_fds();
		throw runtime_error("epoll_ctl");
	}

	cout << PURPLE << "Server listening ..." << RESET << endl;

	while (keepRunning)
	{
		int ev_rdy = epoll_wait(_epoll_fd, _events, MAX_EVENT, -1);
		if (ev_rdy < 0 && keepRunning)
		{
			close_fds();
			throw runtime_error("epoll_wait()");
		}
		for (int i = 0; i < ev_rdy; i++)
		{
			if (_events[i].data.fd == _socket_fd)
				handle_connection();
			else
				handle_input(i);
		}
	}
	close_fds();
}
