#include "../includes/Server.hpp"

void Server::handle_topic(vector<string> tokens, map<int, Client>::iterator it)
{
	if (tokens.size() < 2)
	{
		std::string msgError = ircServerMsg("461", it->second.get_nickname(), "TOPIC", "Not enough parameters");
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

	if (tokens.size() == 2)
	{
		if (channel.get_topic().empty())
		{
			std::string	msgError = ircServerMsg("331", it->second.get_nickname(), channelName, "No topic is set");
			sendToClient(it->first, msgError);
		}
		else
		{
			std::string	msg = ircServerMsg("332", it->second.get_nickname(), channelName, channel.get_topic());
			sendToClient(it->first, msg);
		}
		return ;
	}

	if (channel.is_topic_restricted() && !channel.is_operator(it->first))
	{
		std::string	msgError = ircServerMsg("482", it->second.get_nickname(), channelName, "You're not channel operator");
		sendToClient(it->first, msgError);
		return ;
	}

	if (tokens.size() > 3 && tokens[2][0] != ':')
	{
		sendToClient(it->first, incor_format);
		return ;
	}

	string topic;
	if (tokens[2][0] == ':')
		topic = tokens[2].substr(1);
	else
		topic = tokens[2];

	for (size_t i = 3; i < tokens.size(); i++)
		topic += " " + tokens[i];

	channel.set_topic(topic);

	string topicMsg = ":" + it->second.get_nickname() + "!"
					+ it->second.get_username() + "@"
					+ it->second.get_ip()
					+ " TOPIC " + channelName + " :" + topic + "\r\n";

	sendToClient(it->first, topicMsg);
	broadcastToChannel(channelName, topicMsg, it->first);
}