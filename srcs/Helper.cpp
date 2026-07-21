#include "../includes/Server.hpp"

bool Server::is_passcode(string line)
{
	return line == _server_passcode;
}

bool Server::check_double(string tokens, string flag)
{
	map<int, Client>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it)
	{
		if (it->second.get_nickname() == tokens && flag == "nick")
			return 1;
		else if  (it->second.get_username() == tokens && flag == "user")
			return 1;
	}
	return 0;
}

void Server::set_nick(string tokens, map<int, Client>::iterator it)
{
	std::string oldNick = it->second.get_nickname();

	if (check_double(tokens, "nick"))
	{
		std::string	msgError = ircServerMsg("433", it->second.get_nickname(), tokens, "Nickname is already in use");
		sendToClient(it->first, msgError);
		return ;
	}
	it->second.set_nickname(tokens);
	it->second.has_nickname();

	if (!oldNick.empty())
		broadcastNickChange(oldNick, tokens, it);
}

void Server::broadcastNickChange(const string& oldNick, const string& newNick, map<int, Client>::iterator it)
{
	set<int> recipients;
	recipients.insert(it->first);

	for (map<string, Channel>::iterator chanIt = _channels.begin(); chanIt != _channels.end(); ++chanIt)
	{
		if (!chanIt->second.is_member(it->first))
			continue ;

		set<int> members = chanIt->second.get_members();
		recipients.insert(members.begin(), members.end());
	}

	string nickMsg = ":" + oldNick + "!"
				   + it->second.get_username() + "@"
				   + it->second.get_ip()
				   + " NICK :" + newNick + "\r\n";

	for (set<int>::iterator recIt = recipients.begin(); recIt != recipients.end(); ++recIt)
	{
		if (_clients.find(*recIt) != _clients.end())
			sendToClient(*recIt, nickMsg);
	}
}

void Server::set_user(vector<string> tokens, map<int, Client>::iterator it)
{
	if (tokens.size() < 5)
		return ;

	string realname = tokens[4];

	if (realname.empty() || realname[0] != ':')
		return ;
	realname = realname.substr(1);

	for (size_t i = 5; i < tokens.size(); i++)
		realname += " " + tokens[i];

	it->second.set_username(tokens[1]);
	it->second.has_username();
	it->second.set_realname(realname);
}

void Server::sendToClient(int fd, const std::string& msg)
{
	if (send(fd, msg.c_str(), msg.length(), 0) < 0)
		cerr << RED << "Error: not send" << RESET << endl;
}

std::string	Server::ircServerMsg(const std::string& code, const std::string& nick, const std::string& detail, const std::string& msg)
{
	std::string result = ":ircserv " + code + " ";
	if (nick.empty())
		result += "*";
	else
		result += nick;
	if (!detail.empty())
		result += " " + detail;
	result += " :" + msg + "\r\n";

	return (result);
}

void Server::removeClientFromChannels(int fd)
{
	map<string, Channel>::iterator it = _channels.begin();

	while (it != _channels.end())
	{
		if (it->second.is_member(fd))
			it->second.remove_member(fd);

		if (it->second.empty())
			_channels.erase(it++);
		else
			++it;
	}
}