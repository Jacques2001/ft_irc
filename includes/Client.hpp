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
        string  _password;
        string _buffer;

        bool _has_pass;
        bool _has_nick;
        bool _has_user;
        bool _state;

        bool _is_operator;

    public :
        Client();
        Client(int fd);
        ~Client();

        // getters
        int getfd() const;
        string get_nickname() const;
        string get_username() const;
        bool get_connection() const;
        string get_password() const;
        bool get_operator() const;

        //setters
        void set_nickname(string nick);
        void set_username(string user);
        void set_connection(bool connect);
        void set_operator();

        //setters
        void has_password();
        void has_nickname();
        void has_username();

        //checkers
        bool get_password_status() const;
        bool get_nickname_status() const;
        bool get_username_status() const;

        void appendToBuffer(const char* data, int size);
        std::string& getBuffer();
};

#endif