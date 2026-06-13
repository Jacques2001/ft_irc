#include "../includes/Server.hpp"

Server::Server() : _port(0)
{
    _serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serversocket < 0)
        throw std::runtime_error("Socket not created");
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(_port);
    _serverAddr.sin_addr.s_addr = INADDR_ANY;
}

Server::Server(int port) : _port(port)
{
    _serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serversocket < 0)
        throw std::runtime_error("Socket not created");
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(_port);
    _serverAddr.sin_addr.s_addr = INADDR_ANY;
}

Server::~Server()
{
}

void Server::init()
{
    bind(_serversocket, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr));
    listen(_serversocket, 5);
}
