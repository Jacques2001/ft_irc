#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <poll.h>
#include <sys/epoll.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>

#define MAX_EVENT 64

class Server
{
    private :
        int _port;
        int _serversocket;
        struct sockaddr_in _serverAddr;
        std::vector<struct pollfd> _fds;
        void accept_new_client(int epollfd);

    public :
        Server();
        Server(int port);
        ~Server();
        void init();
        void start();
};

#endif