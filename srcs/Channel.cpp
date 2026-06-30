#include "../includes/Channel.hpp"

//constructeurs/destructeur
Channel::Channel() :   _name(""), _topic("") {}
Channel::Channel(const std::string& name): _name(name), _topic("") {}
Channel::~Channel() {}

//getters
string  Channel::get_name() const {return (_name); }
string  Channel::get_topic() const {return (_topic); }
set<int>  Channel::get_members() const {return (_members); }

//setters
void Channel::set_topic(string topic) { _topic = topic; }

//members
void Channel::add_member(int fd)
{
    _members.insert(fd);
}

void Channel::remove_member(int fd)
{
    _members.erase(fd);
    _operators.erase(fd);
}

bool Channel::is_member(int fd) const
{
    return (_members.find(fd) != _members.end());
}

void Channel::add_operator(int fd)
{
    if (is_member(fd))
        _operators.insert(fd);
}

void Channel::remove_operator(int fd)
{
    _operators.erase(fd);
}

bool Channel::is_operator(int fd) const
{
    return (_operators.find(fd) != _operators.end());
}

bool Channel::empty() const
{
    return (_members.empty());
}