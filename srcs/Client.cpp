#include "../includes/Client.hpp"

Client::Client() : _client_fd(-1), _has_pass(0), _has_nick(0), _has_user(0), _state(0) {}
Client::Client(int fd) : _client_fd(fd), _has_pass(0), _has_nick(0), _has_user(0), _state(0) {}
Client::~Client() {}

int Client::getfd() const {return _client_fd;}
string Client::get_nickname() const {return _nickname;}
string Client::get_username() const {return _username;}
string Client::get_password() const {return _password;}
bool Client::get_connection() const {return _state;}

void Client::set_nickname(string nick) {_nickname = nick;}
void Client::set_username(string user) {_username = user;}
void Client::set_connection(bool connect) {_state = connect;}

void Client::has_password() {_has_pass = 1;}
void Client::has_nickname() {_has_nick = 1;}
void Client::has_username() {_has_user = 1;}

bool Client::get_password_status() {return _has_pass;}
bool Client::get_nickname_status() {return _has_nick;}
bool Client::get_username_status() {return _has_user;}

void Client::appendToBuffer(const char* data, int size) {_buffer.append(data, size);}
std::string &Client::getBuffer() {return _buffer;}