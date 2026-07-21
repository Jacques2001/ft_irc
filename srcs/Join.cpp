#include "../includes/Server.hpp"

void Server::handle_join(vector<string> tokens, map<int, Client>::iterator it)
{
	if (tokens.size() < 2)
	{
		std::string msgError = ircServerMsg("461", it->second.get_nickname(), "JOIN", "Not enough parameters");
		sendToClient(it->first, msgError);
		return ;
	}
	else if (tokens.size() > 3)
	{
		std::string msgError = ircServerMsg("461", it->second.get_nickname(), "JOIN", "Invalid parameters");
		sendToClient(it->first, msgError);
		return ;
	}

	string channelName = tokens[1];

	if (channelName.empty() || channelName[0] != '#')
	{
		std::string msgError = ircServerMsg("403", it->second.get_nickname(), channelName, "No such channel");
		sendToClient(it->first, msgError);
		return ;
	}

	bool channelCreated = false;

	if (_channels.find(channelName) == _channels.end())
	{
		_channels[channelName] = Channel(channelName);
		channelCreated = true;
	}

	Channel& channel = _channels[channelName];

	if (channel.is_member(it->first))
		return ;

	if (channel.is_invite_only() && !channel.is_invited(it->first))
	{
		std::string	msgError = ircServerMsg("473", it->second.get_nickname(), channelName, "Cannot join channel (+i)");
		sendToClient(it->first, msgError);
		return ;
	}

	if (channel.has_key())
	{
		if (tokens.size() != 3 || tokens[2] != channel.get_key())
		{
			std::string	msgError = ircServerMsg("475", it->second.get_nickname(), channelName, "Cannot join channel (+k)");
			sendToClient(it->first, msgError);
			return ;
		}
	}

	if (channel.has_limit() && channel.get_members().size() >= static_cast<size_t>(channel.get_limit()))
	{
		std::string	msgError = ircServerMsg("471", it->second.get_nickname(), channelName, "Cannot join channel (+l)");
		sendToClient(it->first, msgError);
		return ;
	}

	channel.add_member(it->first);
	channel.remove_invited(it->first);

	if (channelCreated)
		channel.add_operator(it->first);

	string joinMsg = ":" + it->second.get_nickname() + "!"
				   + it->second.get_username() + "@"
				   + it->second.get_ip()
				   + " JOIN " + channelName + "\r\n";

	sendToClient(it->first, joinMsg);
	broadcastToChannel(channelName, joinMsg, it->first);

	std::string	nick_list;
	set<int>	members_fd = channel.get_members();

	for (set<int>::iterator members_it = members_fd.begin(); members_it != members_fd.end(); ++members_it)
	{
		int	member_fd = *members_it;
		if (!nick_list.empty())
			nick_list += ' ';
		if (channel.is_operator(member_fd))
			nick_list += '@';
		nick_list += _clients[member_fd].get_nickname();
	}

	std::string	msg353 = ircServerMsg("353", it->second.get_nickname(), "= " + channelName, nick_list);
	sendToClient(it->first, msg353);

	std::string	msg366 = ircServerMsg("366", it->second.get_nickname(), channelName, "End of /NAMES list");
	sendToClient(it->first, msg366);
}