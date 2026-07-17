#include "../includes/Server.hpp"
#include <errno.h>

static volatile int keepRunning = 1;

//reception du signal ctrl + c
void intHandler(int dummy)
{
	(void)dummy;
	cout << WHITE << "\nServer closed" << RESET << endl;
	keepRunning = 0;
}

//memset pour eviter de retrouver des donnees "poubelles"
//dans la structure _serverAddr
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

//configuration serveur et connection au reseau
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

	// fcntl fait en sorte que les connections au serveur soient non bloquantes
	if (fcntl(_socket_fd, F_SETFL, O_NONBLOCK) < 0) 
	{
		close(_socket_fd);
		throw runtime_error("fcntl");
	}

	_serverAddr.sin_family = AF_INET; //IPv4
	_serverAddr.sin_port = htons(_port); // converti le type pour pouvoir l'envoyer a travers le reseau
	_serverAddr.sin_addr.s_addr = INADDR_ANY; //accepte n'importe quel type de connection

	if (bind(_socket_fd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0)
	{
		close(_socket_fd);
		throw runtime_error("bind");
	}
	if (listen(_socket_fd, 5) < 0) // value to change to reveive more client
	{
		close(_socket_fd);
		throw runtime_error("listen");
	}
}

//configuation d'epoll() qui servira a verifier s'il y a 
//de nouveaux evenements a traiter
void Server::start()
{
	signal(SIGINT, intHandler);
	_epoll_fd = epoll_create1(0); // _epollfd sera un fd qui surveillera les _events
	if (_epoll_fd < 0)
	{
		close_fds();
		throw runtime_error("epoll_create1");
	}
	struct epoll_event ev;
	ev.events = EPOLLIN; // evenements rentrants, lecture seule
	ev.data.fd = _socket_fd;

	//  ajoute mon serveur (_socket_fd) dans la liste des evenements a surveiller (_epoll_fd)
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev) < 0)
	{
		close_fds();
		throw runtime_error("epoll_ctl");
	}

	cout << PURPLE << "Server listening ..." << RESET << endl;

	while (keepRunning) // tant que je ne recois pas de signal ctrl + C
	{
		int ev_rdy = epoll_wait(_epoll_fd, _events, MAX_EVENT, -1); // endors le programme
		// le programme se reveillera quand il y aura un evenement a gerer
		if (ev_rdy < 0 && keepRunning)
		{
			close_fds();
			throw runtime_error("epoll_wait()");
		}
		for (int i = 0; i < ev_rdy; i++) // rentre dans la boucle d'evenements
		{
			if (_events[i].data.fd == _socket_fd) // si c'est un client qui rentre
				handle_connection();
			else // si le client ecrit
				handle_input(i);
		}
	}
	close_fds(); // on ferme le fd (il y a surement d'autres fd a fermer)
}
