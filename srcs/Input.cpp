#include "../includes/Server.hpp"

//cette fonction va gerer tout ce que le client (qui est connecte)
//va entrer comme input, l'input sera ensuite envoye a parse_line(...)
void Server::handle_input(int i)
{
	//recv est l'equivalent de la fonction read()
	int curr_fd = _events[i].data.fd; // on prend le fd de l'event

	char buf[1024]; //buffer pour stocker le message du client
	int size_buf = recv(curr_fd, buf, 1024, 0); //size_buf correspond a ce qui a pu etre lu

	if (size_buf == 0) // si c'est = 0 c'est que le client s'est deconnecte
	{
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, curr_fd, NULL);
		removeClientFromChannels(curr_fd);
		close(curr_fd);
		_clients.erase(curr_fd);

		cout << YELLOW << "Client " << curr_fd << " disconnected" << RESET << endl;
		return ;
	}

	if (size_buf < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;

		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, curr_fd, NULL);
		removeClientFromChannels(curr_fd);
		close(curr_fd);
		_clients.erase(curr_fd);

		cerr << RED << "Error: recv failed on client " << curr_fd << RESET << endl;
		return ;
	}

	//ces lignes ci-dessous sont faites pour regler le probleme de
	//"donnees partielles" recues par recv()
	_clients[curr_fd].appendToBuffer(buf, size_buf);
	std::string &client_buffer = _clients[curr_fd].getBuffer();

	size_t pos;
	while ((pos = client_buffer.find('\n')) != std::string::npos)
	{
		std::string line = client_buffer.substr(0, pos);

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		client_buffer.erase(0, pos + 1);

		parse_line(line, curr_fd);
	}
}