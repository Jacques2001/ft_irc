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
#include <signal.h>
#include <map>
#include <sstream>

#include "Client.hpp"

using namespace std;

#define MAX_EVENT 64

class Server
{
    private :
        map<int, Client> _clients;
        int _port;
        int _serversocket;
        struct sockaddr_in _serverAddr;
        std::vector<struct pollfd> _fds;
        string _server_passcode;
        void parse_line(string line, int curr_fd);
        void connection_process(string line, map<int, Client>::iterator it);
        bool is_passcode(string line);

    public :
        Server();
        Server(int port, string serverpass);
        ~Server();
        void init();
        void start();
};

#endif