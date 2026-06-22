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
#include <vector>
#include <arpa/inet.h>

#include "Client.hpp"

using namespace std;

#define MAX_EVENT 64
#define pass_incorrect "Error: incorrect password\r\n"
#define prior_connect "Error: you must be connected first\r\n"
#define usr_not_found "Error: user not found\r\n"
#define incor_format "Error: incorrect format\r\n"
#define name_alrdy_taken "Error: nick or username already taken\r\n"
#define err_args "Error: argument not valid\r\n"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define PURPLE "\033[1;35m"
#define YELLOW "\033[1;33m"
#define WHITE "\033[1;37m"
#define RESET "\033[0m"

class Server
{
    private :
        map<int, Client> _clients;
        int _port;
        int _serversocket;
        struct sockaddr_in _serverAddr;
        std::vector<struct pollfd> _fds;
        string _server_passcode;
        string _ip_address;
        void handle_prv_msg(vector<string> tokens, map<int, Client>::iterator it);
        void parse_line(string line, int curr_fd);
        void connection_process(string line, map<int, Client>::iterator it);
        bool is_passcode(string line);
        bool check_double(string tokens, string flag);

    public :
        Server();
        Server(int port, string serverpass);
        ~Server();
        void init();
        void start();
};

#endif