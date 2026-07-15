#include "../includes/Server.hpp"

//cette fonction sert a traiter les messages privees envoyees entre client
//le format du message envoye suit les directives du protocole IRC
void Server::handle_prv_msg(vector<string> tokens, map<int, Client>::iterator it)
{
	// pour gerer le cas de mettre que PRIVMSG -> 461 code
	if (tokens.size() < 2)
	{
		std::string	msgError = ircServerMsg("461", it->second.get_nickname(), "PRIVMSG", "Not enough parameters");
		sendToClient(it->first, msgError);
		return ;
	}

	if (tokens.size() < 3)
	{
		std::string	msgError = ircServerMsg("412", it->second.get_nickname(), "", "No text to send");
		sendToClient(it->first, msgError);
		return ;
	}

	if (tokens[1][0] == '#')
	{
		handle_channel_msg(tokens, it);
		return ;
	}

	map<int, Client>::iterator ite = _clients.begin();
	for (; ite != _clients.end(); ++ite)
	{
		if (ite->second.get_nickname() == tokens[1])
			break;
	}

	if (ite == _clients.end())
	{
		std::string	msgError = ircServerMsg("401", it->second.get_nickname(), tokens[1], "No such nick/channel");
		sendToClient(it->first, msgError);
		return ;
	}

<<<<<<< HEAD
	// ***** faire en sorte que lorsque le client envoie seulement 1 message, qu'il puisse le faire 
	// sans l'argument ':'
	if (tokens[2][0] != ':')
=======
	if (tokens[2][0] != ':' && tokens.size() > 3)
>>>>>>> Jac
	{
		sendToClient(it->first, incor_format);
		return ;
	}

	string final_msg = ":" + it->second.get_nickname() + "!"
						+ it->second.get_username() + "@" + it->second.get_ip()
						+ " PRIVMSG " + tokens[1];
	
	for (size_t i = 2; i < tokens.size(); i++)
	{
		if (tokens.size() == 3 && tokens[2][0] != ':')
		{
			final_msg += " :" + tokens[2];
			break;
		}	
		final_msg += " " + tokens[i];
	}

	final_msg += "\r\n";

	sendToClient(ite->first, final_msg);
}