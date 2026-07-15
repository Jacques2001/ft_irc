#include "../includes/Server.hpp"

void Server::handle_part(vector<string> tokens, map<int, Client>::iterator it)
{
	if (tokens.size() != 2)
	{
		std::string msgError = ircServerMsg("461", it->second.get_nickname(), "PART", "Not enough parameters");
		sendToClient(it->first, msgError);
		return ;
	}

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

	string partMsg = ":" + it->second.get_nickname() + "!"
				   + it->second.get_username() + "@"
				   + it->second.get_ip()
				   + " PART " + channelName + "\r\n";

	sendToClient(it->first, partMsg);
	broadcastToChannel(channelName, partMsg, it->first);

	channel.remove_member(it->first);

	if (channel.empty())
		_channels.erase(chanIt);
}