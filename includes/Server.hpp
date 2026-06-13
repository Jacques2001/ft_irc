#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <poll.h>

class Server
{
    private :
        int _port;
        int _serversocket;
        struct sockaddr_in _serverAddr;
        std::vector<struct pollfd> _fds;

    public :
        Server();
        Server(int port);
        ~Server();
        void init();
};

#endif