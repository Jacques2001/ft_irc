#include "../includes/Server.hpp"

void Server::handle_kick(vector<string> tokens, map<int, Client>::iterator it)
{
	if (tokens.size() < 3)
	{
		std::string msgError = ircServerMsg("461", it->second.get_nickname(), "KICK", "Not enough parameters");
		sendToClient(it->first, msgError);
		return ;
	}

	string channelName = tokens[1];
	string targetKick = tokens[2];

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

	if (!channel.is_operator(it->first))
	{
		std::string	msgError = ircServerMsg("482", it->second.get_nickname(), channelName, "You're not channel operator");
		sendToClient(it->first, msgError);
		return ;
	}

	map<int, Client>::iterator userIt = _clients.begin();
	for (; userIt != _clients.end(); ++userIt)
	{
		if (userIt->second.get_nickname() == targetKick)
			break ;
	}

	if (userIt == _clients.end())
	{
		std::string	msgError = ircServerMsg("401", it->second.get_nickname(), targetKick, "No such nick/channel");
		sendToClient(it->first, msgError);
		return ;
	}

	if (!channel.is_member(userIt->first))
	{
		std::string	msgError = ircServerMsg("441", it->second.get_nickname(), targetKick + " " + channelName, "They aren't on that channel");
		sendToClient(it->first, msgError);
		return ;
	}

	string kickMsg = ":" + it->second.get_nickname() + "!"
				   + it->second.get_username() + "@"
				   + it->second.get_ip()
				   + " KICK " + channelName + " " + targetKick + "\r\n";

	sendToClient(it->first, kickMsg);
	broadcastToChannel(channelName, kickMsg, it->first);

	channel.remove_member(userIt->first);

	if (channel.empty())
		_channels.erase(chanIt);
}