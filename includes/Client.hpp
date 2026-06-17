#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

using namespace std;

class Client
{
    private :
        int _client_fd;
        string _nickname;
        string _username;
    public :
        Client();
        ~Client();
};

#endif