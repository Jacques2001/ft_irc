#include "../includes/Server.hpp"

void Server::handle_mode(vector<string> tokens, map<int, Client>::iterator it)
{
	if (tokens.size() < 3)
	{
		std::string msgError = ircServerMsg("461", it->second.get_nickname(), "MODE", "Not enough parameters");
		sendToClient(it->first, msgError);
		return ;
	}

	string channelName = tokens[1];
	string mode = tokens[2];

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

	if (mode == "+i")
		channel.set_invite_only(true);
	else if (mode == "-i")
		channel.set_invite_only(false);
	else if (mode == "+t")
		channel.set_topic_restricted(true);
	else if (mode == "-t")
		channel.set_topic_restricted(false);
	else if (mode == "+k")
	{
		if (tokens.size() != 4)
		{
			std::string msgError = ircServerMsg("461", it->second.get_nickname(), "MODE " + mode, "Not enough parameters");
			sendToClient(it->first, msgError);
			return ;
		}

		channel.set_key(tokens[3]);
	}
	else if (mode == "-k")
		channel.remove_key();
	else if (mode == "+l")
	{
		if (tokens.size() != 4)
		{
			std::string msgError = ircServerMsg("461", it->second.get_nickname(), "MODE " + mode, "Not enough parameters");
			sendToClient(it->first, msgError);
			return ;
		}

		std::stringstream ss(tokens[3]);
		int limit;
		char leftover;

		if (!(ss >> limit) || (ss >> leftover) || limit <= 0)
		{
			sendToClient(it->first, "Error: invalid channel limit\r\n");
			return ;
		}

		channel.set_limit(limit);
	}
	else if (mode == "-l")
		channel.remove_limit();
	else if (mode == "+o" || mode == "-o")
	{
		if (tokens.size() != 4)
		{
			std::string msgError = ircServerMsg("461", it->second.get_nickname(), "MODE " + mode, "Not enough parameters");
			sendToClient(it->first, msgError);
			return ;
		}

		string targetNick = tokens[3];

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

		if (!channel.is_member(targetIt->first))
		{
			std::string	msgError = ircServerMsg("441", it->second.get_nickname(), targetNick + " " + channelName, "They aren't on that channel");
			sendToClient(it->first, msgError);
			return ;
		}

		if (mode == "+o")
			channel.add_operator(targetIt->first);
		else
			channel.remove_operator(targetIt->first);
	}
	else
	{
		std::string	modeFlag = mode;
		if (mode.size() > 1 && (mode[0] == '+' || mode[0] == '-'))
			modeFlag = mode.substr(1);
		std::string	msgError = ircServerMsg("472", it->second.get_nickname(), modeFlag, "is unknown mode char to me");
		sendToClient(it->first, msgError);
		return ;
	}

	string modeMsg = ":" + it->second.get_nickname() + "!"
			   + it->second.get_username() + "@"
			   + it->second.get_ip()
			   + " MODE " + channelName + " " + mode;

	if (mode == "+k" || mode == "+l" || mode == "+o" || mode == "-o")
		modeMsg += " " + tokens[3];

	modeMsg += "\r\n";

	sendToClient(it->first, modeMsg);
	broadcastToChannel(channelName, modeMsg, it->first);
}