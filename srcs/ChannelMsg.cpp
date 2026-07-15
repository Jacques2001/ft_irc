#include "../includes/Server.hpp"

void Server::broadcastToChannel(const string& channelName, const string& msg, int exceptFd)
{
	map<string, Channel>::iterator chanIt = _channels.find(channelName);
	if (chanIt == _channels.end())
		return ;

	set<int> members = chanIt->second.get_members();

	for (set<int>::iterator it = members.begin(); it != members.end(); ++it)
	{
		int fd = *it;

		if (fd == exceptFd)
			continue ;

		if (_clients.find(fd) != _clients.end())
			sendToClient(fd, msg);
	}
}

void Server::handle_channel_msg(vector<string> tokens, map<int, Client>::iterator it)
{
	string channelName = tokens[1];

	map<string, Channel>::iterator chanIt = _channels.find(channelName);
	if (chanIt == _channels.end())
	{
		std::string msgError = ircServerMsg("403", it->second.get_nickname(), channelName, "No such channel");
		sendToClient(it->first, msgError);
		return ;
	}

	Channel& channel = chanIt->second;

	if (!channel.is_member(it->first))
	{
		std::string	msgError = ircServerMsg("442", it->second.get_nickname(), channelName, "You're not on that channel");
		sendToClient(it->first, msgError);
		return ;
	}

	if (tokens[2].empty())
	{
		std::string	msgError = ircServerMsg("412", it->second.get_nickname(), "", "No text to send");
		sendToClient(it->first, msgError);
		return ;
	}

	if (tokens[2][0] != ':')
	{
		sendToClient(it->first, incor_format);
		return ;
	}

	string final_msg = ":" + it->second.get_nickname() + "!"
					 + it->second.get_username() + "@"
					 + it->second.get_ip()
					 + " PRIVMSG " + channelName;

	for (size_t i = 2; i < tokens.size(); i++)
		final_msg += " " + tokens[i];

	final_msg += "\r\n";

	broadcastToChannel(channelName, final_msg, it->first);
}