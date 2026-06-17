#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

using namespace std;

class Client
{
    private :
        string _nickname;
        string _username;
    public :
        Client();
        Client(const Client &other);
        Client &operator=(const Client &other);
        ~Client();
};

#endif