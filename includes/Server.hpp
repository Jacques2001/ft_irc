#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
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
#include "Channel.hpp"


using namespace std;

#define MAX_EVENT 64
#define pass_incorrect "Error: incorrect password\r\n"
#define prior_connect "Error: you must be connected first\r\n"
#define usr_not_found "Error: user not found\r\n"
#define incor_format "Error: incorrect format\r\n"
#define name_alrdy_taken "Error: nick or username already taken\r\n"
#define err_args "Error: argument not valid\r\n"
#define nick_too_long "Error: nickname cannot exceed 9 caracters\r\n"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define PURPLE "\033[1;35m"
#define YELLOW "\033[1;33m"
#define WHITE "\033[1;37m"
#define RESET "\033[0m"
#define BLUE "\033[1;34m"

class Server
{
    private :
        //server
        map<int, Client> _clients;
        map<string, Channel> _channels;

        int _port;
        int _socket_fd;
        int _epoll_fd;

        struct sockaddr_in _serverAddr;
        string _server_passcode;

        struct epoll_event _events[MAX_EVENT];

        //method
        void handle_prv_msg(vector<string> tokens, map<int, Client>::iterator it);
        void handle_join(vector<string> tokens, map<int, Client>::iterator it);
        void handle_part(vector<string> tokens, map<int, Client>::iterator it);
        void handle_topic(vector<string> tokens, map<int, Client>::iterator it);
        void handle_kick(vector<string> tokens, map<int, Client>::iterator it);
        void handle_invite(vector<string> tokens, map<int, Client>::iterator it);
        
        void parse_line(string line, int curr_fd);
        void connection_process(string line, map<int, Client>::iterator it);
        bool is_passcode(string line);
        bool check_double(string tokens, string flag);
        void set_nick(string tokens, map<int, Client>::iterator it);
        void set_user(vector<string> tokens, map<int, Client>::iterator it);
        void handle_connection();
        void handle_input(int i);
        void close_fds();
        void sendToClient(int fd, const std::string& msg);
        void broadcastToChannel(const string& channelName, const string& msg, int exceptFd);
        void handle_channel_msg(vector<string> tokens, map<int, Client>::iterator it);
        void removeClientFromChannels(int fd);

    public :
        Server();
        Server(int port, string serverpass);
        ~Server();
        
        void init();
        void start();
};

#endif