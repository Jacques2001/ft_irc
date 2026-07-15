#include "../includes/Server.hpp"

//cette fonction va parser et executer la ligne recu par le client
//elle va d'abord checker si le client a bien le droit d'envoyer des messages
//continuer a coder le channel
void Server::parse_line(string line, int curr_fd)
{
	if (line.empty())
		return ;

	map<int, Client>::iterator it = _clients.find(curr_fd);
	if (it == _clients.end())
		return ;

	if (it->second.get_connection() == 0)
	{
		connection_process(line, it);
		return ;
	}

	stringstream ss(line);
	string token;
	vector<string> tokens;

	while (ss >> token)
		tokens.push_back(token);

	if (tokens.empty())
		return ;

	if (tokens[0] == "NICK" && tokens.size() == 2)
		set_nick(tokens[1], it);
	// else if (tokens[0] == "PRIVMSG" && tokens.size() >= 3)
	else if (tokens[0] == "PRIVMSG") // pour lier a handle_prv_msg, j'ai modifie cette partie
		handle_prv_msg(tokens, it);
	else if (tokens[0] == "JOIN")
		handle_join(tokens, it);
	else if (tokens[0] == "PART")
		handle_part(tokens, it);
	else if (tokens[0] == "TOPIC")
		handle_topic(tokens, it);
	else if (tokens[0] == "KICK")
		handle_kick(tokens, it);
	else if (tokens[0] == "INVITE")
		handle_invite(tokens, it);
	else if (tokens[0] == "MODE")
		handle_mode(tokens, it);
}