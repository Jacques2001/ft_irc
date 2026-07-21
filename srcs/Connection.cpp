#include "../includes/Server.hpp"

void Server::connection_process(string line, map<int, Client>::iterator it)
{
	stringstream ss(line);
	string token;
	vector<string> tokens;
	while (ss >> token)
		tokens.push_back(token);
	if (tokens.empty())
		return ;
	if (tokens[0] == "PASS")
	{
		if (tokens.size() != 2)
		{
			std::string msgError = ircServerMsg("461", "", "PASS", "Not enough parameters");
			sendToClient(it->first, msgError);
			return ;
		}
		if (it->second.get_password_status() == 0)
		{
			if (is_passcode(tokens[1]) == true)
				it->second.has_password();
			else
			{
				std::string	msgError = ircServerMsg("464", "", "", "Password incorrect");
				sendToClient(it->first, msgError);
				return ;
			}
		}
	}
	if (it->second.get_password_status() == 0)
		return ;
	if (tokens[0] == "NICK")
	{
		if (tokens.size() != 2)
		{
			std::string msgError = ircServerMsg("461", "", "NICK", "Not enough parameters");
			sendToClient(it->first, msgError);
			return ;
		}
		if (it->second.get_nickname_status() == 0)
		{
			if (tokens[1].size() > 9)
			{
				std::string msgError = ircServerMsg("432", "", tokens[1], "Erroneous nickname");
				sendToClient(it->first, msgError);
				return ;
			}
			set_nick(tokens[1], it);
		}
	}
	if (tokens[0] == "USER")
	{
		if (tokens.size() < 5)
		{
			std::string	msgError = ircServerMsg("461", it->second.get_nickname(), "USER", "Not enough parameters");
			sendToClient(it->first, msgError);
			return ;
		}
		if (it->second.get_username_status() == 0)
			set_user(tokens, it);
	}

	if (it->second.get_password_status() && it->second.get_nickname_status()
		&& it->second.get_username_status() && it->second.get_connection() == 0)
	{
		string	connected = ircServerMsg("001", it->second.get_nickname(), "", "Welcome to the ft_irc network " +  it->second.get_nickname());
		it->second.set_connection(true);
		sendToClient(it->first, connected);
	}
}

void Server::handle_connection()
{
	struct sockaddr_in client_addr;
	socklen_t addr_size;
	addr_size = sizeof(struct sockaddr_in);
	int client_fd = accept(_socket_fd,
		(struct sockaddr *)&client_addr, &addr_size);
	if (client_fd < 0)
	{
		cerr << RED <<  "not accepted" << RESET << endl;
		return ;
	}
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close_fds();
		throw runtime_error("fcntl(1)");
	}
	cout << GREEN << "Client " << client_fd << " connected" << RESET << endl;
	_clients.insert(make_pair(client_fd, Client(client_fd)));
	_clients[client_fd].set_ip(inet_ntoa(client_addr.sin_addr));

	struct epoll_event client_ev;
	client_ev.events = EPOLLIN;
	client_ev.data.fd = client_fd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0)
	{
		close_fds();
		throw runtime_error("epoll_ctl(1)");
	}
}