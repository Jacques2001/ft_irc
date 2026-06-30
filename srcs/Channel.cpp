#include "../includes/Channel.hpp"

//constructeurs/destructeur
Channel::Channel()
:   _name(""),
    _topic(""),
    _invite_only(false),
    _topic_restricted(false),
    _has_key(false),
    _key(""),
    _has_limit(false),
    _limit(0) {}

Channel::Channel(const std::string& name)
:   _name(name),
    _topic(""),
    _invite_only(false),
    _topic_restricted(false),
    _has_key(false),
    _key(""),
    _has_limit(false),
    _limit(0) {}
Channel::~Channel() {}

//getters
string  Channel::get_name() const {return (_name); }
string  Channel::get_topic() const {return (_topic); }
set<int>  Channel::get_members() const {return (_members); }
bool Channel::is_operator(int fd) const { return (_operators.find(fd) != _operators.end()); }
bool Channel::is_invite_only() const { return _invite_only; }
bool Channel::is_topic_restricted() const { return _topic_restricted; }
bool Channel::has_key() const { return _has_key; }
string Channel::get_key() const { return _key; }
bool Channel::has_limit() const { return _has_limit; }
int Channel::get_limit() const {return _limit; }


//setters
void Channel::set_topic(string topic) { _topic = topic; }
void Channel::set_invite_only(bool value){ _invite_only = value; }
void Channel::set_topic_restricted(bool value) { _topic_restricted = value; }
void Channel::set_key(string key)
{
    _key = key;
    _has_key = true;
}
void Channel::set_limit(int limit)
{
    _limit = limit;
    _has_limit = true;
}

//members
void Channel::add_member(int fd)
{
    _members.insert(fd);
}

void Channel::remove_member(int fd)
{
    _members.erase(fd);
    _operators.erase(fd);
    _invited.erase(fd);
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

bool Channel::empty() const
{
    return (_members.empty());
}

void Channel::add_invited(int fd)
{
    _invited.insert(fd);
}

void Channel::remove_invited(int fd)
{
    _invited.erase(fd);
}

bool Channel::is_invited(int fd) const
{
    return (_invited.find(fd) != _invited.end());
}

void Channel::remove_key()
{
    _key = "";
    _has_key = false;
}

void Channel::remove_limit()
{
    _limit = 0;
    _has_limit = false;
}