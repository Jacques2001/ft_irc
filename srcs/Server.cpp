#include "../includes/Server.hpp"

//memset pour eviter de retrouver des donnees "poubelles"
//dans la structure _serverAddr
Server::Server() : _port(0), _serversocket(-1)
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::Server(int port) : _port(port), _serversocket(-1)
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::~Server()
{
}

//configuration serveur et connection au reseau
void Server::init()
{
    _serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serversocket < 0)
        throw std::runtime_error("socket");

    int opt = 1;
    if (setsockopt(_serversocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt");
    if (fcntl(_serversocket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("fcntl");

    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(_port);
    _serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serversocket, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0)
        throw std::runtime_error("bind");
    if (listen(_serversocket, 5) < 0) // value to change to reveive more client
        throw std::runtime_error("listen");
}

void Server::start()
{
    struct pollfd server_fd;
    server_fd.fd = _serversocket;
    server_fd.events = POLLIN;
    server_fd.revents = 0;
    _fds.push_back(server_fd);
    std::cout << "Server listening ..." << std::endl;

    while (1)
    {
        
    }
}