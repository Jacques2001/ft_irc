#include "../includes/Server.hpp"

void Server::handle_invite(vector<string> tokens, map<int, Client>::iterator it)
{
	if (tokens.size() != 3)
	{
		std::string msgError = ircServerMsg("461", it->second.get_nickname(), "INVITE", "Not enough parameters");
		sendToClient(it->first, msgError);
		return ;
	}

	string targetNick = tokens[1];
	string channelName = tokens[2];

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

	if (channel.is_invite_only() && !channel.is_operator(it->first))
	{
		std::string	msgError = ircServerMsg("482", it->second.get_nickname(), channelName, "You're not channel operator");
		sendToClient(it->first, msgError);
		return ;
	}

	map<int, Client>::iterator targetIt = _clients.begin();
	for (; targetIt != _clients.end(); ++targetIt)
	{
		if (targetIt->second.get_nickname() == targetNick)
			break ;
	}

	if (targetIt == _clients.end())
	{
		std::string	msgError = ircServerMsg("401", it->second.get_nickname(), targetNick, "No such nick/channel");
		sendToClient(it->first, msgError);
		return ;
	}

	if (channel.is_member(targetIt->first))
	{
		std::string	msgError = ircServerMsg("443", it->second.get_nickname(), targetNick + " " + channelName, "is already on channel");
		sendToClient(it->first, msgError);
		return ;
	}

	channel.add_invited(targetIt->first);

	string inviteMsg = ":" + it->second.get_nickname() + "!"
					 + it->second.get_username() + "@"
					 + it->second.get_ip()
					 + " INVITE " + targetNick + " " + channelName + "\r\n";

	sendToClient(targetIt->first, inviteMsg);

	std::string	msg341 = ":ircserv 341 ";
	if (it->second.get_nickname().empty())
		msg341 += "*";
	else
		msg341 += it->second.get_nickname();
	msg341 += " " + targetNick + " " + channelName + "\r\n";
	sendToClient(it->first, msg341);
}