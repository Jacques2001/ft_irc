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

void Server::accept_new_client(int epollfd)
{
    std::cout << "Client connected" << std::endl;

    // on accueille le nouveau client
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);
    int client_fd = accept(_serversocket, 
        (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd < 0)
        std::cerr << "not accepted" << std::endl;

    struct epoll_event client_ev;
    client_ev.events = EPOLLIN;
    client_ev.data.fd = client_fd;
     if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0)
        throw std::runtime_error("epoll_ctl(1)");
}

// void Server::rcv_new_msg()
// {
//     recv
// }

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
    int client_fd = -1;
    int epollfd = epoll_create1(0);
    if (epollfd < 0)
        throw std::runtime_error("epoll_create1");

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = _serversocket;

    struct epoll_event events[MAX_EVENT];

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, _serversocket, &ev) < 0)
        throw std::runtime_error("epoll_ctl");

    std::cout << "Server listening ..." << std::endl;

    while (1)
    {
        int ev_rdy = epoll_wait(epollfd, events, MAX_EVENT, -1); // endors le programme
        if (ev_rdy < 0)
            throw std::runtime_error("epoll_wait()");
        for (int i = 0; i < ev_rdy; i++)
        {
            if (events[i].data.fd == _serversocket)
            {
                std::cout << "Client connected" << std::endl;

                // on accueille le nouveau client
                struct sockaddr_in client_addr;
                socklen_t addr_size;
                addr_size = sizeof(struct sockaddr_in);
                client_fd = accept(_serversocket, 
                    (struct sockaddr *)&client_addr, &addr_size);
                if (client_fd < 0)
                    std::cerr << "not accepted" << std::endl;

                struct epoll_event client_ev;
                client_ev.events = EPOLLIN;
                client_ev.data.fd = client_fd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0)
                    throw std::runtime_error("epoll_ctl(1)");
            }
            else
            {
                char buf[1024];
                int size_buf = recv(client_fd, buf, 1024, 0);
                buf[size_buf] = '\0';
                (void)size_buf;
                std::cout << buf;
            }
        }
    }
    close(epollfd);
}