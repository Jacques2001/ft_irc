#include "../includes/Server.hpp"

void Server::handle_input(int i)
{
	int curr_fd = _events[i].data.fd;

	char buf[1024];
	int size_buf = recv(curr_fd, buf, 1024, 0);

	if (size_buf == 0)
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