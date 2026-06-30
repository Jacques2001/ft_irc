#include "../includes/Client.hpp"

Client::Client()
:   _client_fd(-1),
    _has_pass(false),
    _has_nick(false),
    _has_user(false),
    _state(false) {}

Client::Client(int fd)
:   _client_fd(fd),
    _has_pass(false),
    _has_nick(false),
    _has_user(false),
    _state(false) 
{}

Client::~Client() {}

int             Client::getfd() const {return _client_fd;}
string          Client::get_nickname() const {return _nickname;}
string          Client::get_username() const {return _username;}
bool            Client::get_connection() const {return _state;}
string          Client::get_ip() const {return _ip_addr;}
string          Client::get_realname() const { return _realname; }

void            Client::set_nickname(string nick) {_nickname = nick;}
void            Client::set_username(string user) {_username = user;}
void            Client::set_realname(string real) {_realname = real;}
void            Client::set_connection(bool connect) {_state = connect;}
void            Client::set_ip(string ip) {_ip_addr = ip;}

void            Client::has_password() {_has_pass = true;}
void            Client::has_nickname() {_has_nick = true;}
void            Client::has_username() {_has_user = true;}

bool            Client::get_password_status() const {return _has_pass;}
bool            Client::get_nickname_status() const {return _has_nick;}
bool            Client::get_username_status() const {return _has_user;}

void            Client::appendToBuffer(const char* data, int size) {_buffer.append(data, size);}
std::string&    Client::getBuffer() {return _buffer;}