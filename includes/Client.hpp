#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
    private :

    public :
        Client();
        Client(const Client &other);
        Client &operator=(const Client &other);
        ~Client();
};

#endif