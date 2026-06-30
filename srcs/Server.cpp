#include "../includes/Server.hpp"
#include <errno.h>

static volatile int keepRunning = 1;

//reception du signal ctrl + c
void intHandler(int dummy)
{
    (void)dummy;
    cout << WHITE << "\nServer closed" << RESET << endl;
    keepRunning = 0;
}

//memset pour eviter de retrouver des donnees "poubelles"
//dans la structure _serverAddr
Server::Server()
:   _port(0),
    _socket_fd(-1),
    _epoll_fd(-1),
    _server_passcode("0")
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::Server(int port, string serverpass)
:   _port(port),
    _socket_fd(-1),
    _epoll_fd(-1),
    _server_passcode(serverpass)
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::~Server() {}


void Server::close_fds()
{
    map<int, Client>::iterator it = _clients.begin();
    for (; it != _clients.end(); ++it)
    {
        if (it->first != -1) 
        {
            if (_epoll_fd != -1)
                epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
            close(it->first);
        }
    }
    _clients.clear();

    if (_socket_fd != -1)
    {
        close(_socket_fd);
        _socket_fd = -1;
    }

    if (_epoll_fd != -1)
    {
        close(_epoll_fd);
        _epoll_fd = -1;
    }
}

//regarde si la ligne envoyee correspond au mdp
bool Server::is_passcode(string line)
{
    return line == _server_passcode;
}

//cette fonction sert a checker si un nickname ou username
//existe pas deja dans la base de donnees des clients
bool Server::check_double(string tokens, string flag)
{
    map<int, Client>::iterator it = _clients.begin();
    for (; it != _clients.end(); ++it)
    {
        if (it->second.get_nickname() == tokens && flag == "nick")
            return 1;
        else if  (it->second.get_username() == tokens && flag == "user")
            return 1;
    }
    return 0;
}

void Server::set_nick(string tokens, map<int, Client>::iterator it)
{
    if (check_double(tokens, "nick"))
    {
        sendToClient(it->first, name_alrdy_taken);
        return ;
    }
    it->second.set_nickname(tokens);
    it->second.has_nickname();
}

void Server::set_user(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() < 5)
        return ;

    string realname = tokens[4];

    if (realname.empty() || realname[0] != ':')
        return ;
    realname = realname.substr(1);

    for (size_t i = 5; i < tokens.size(); i++)
        realname += " " + tokens[i];

    it->second.set_username(tokens[1]);
    it->second.has_username();
    it->second.set_realname(realname);
}

//cette fonction sert a verifier si le client a bien rentre le mdp, nickname et username
//avant de valider la connection
void Server::connection_process(string line, map<int, Client>::iterator it)
{
    stringstream ss(line);
    string token;
    vector<string> tokens;
    while (ss >> token)
        tokens.push_back(token);
    if (tokens.empty())
        return ;
    if (tokens[0] == "PASS" && tokens.size() == 2 && it->second.get_password_status() == 0)
    {
        if (is_passcode(tokens[1]) == true)
            it->second.has_password();
        else
        {
            sendToClient(it->first, pass_incorrect);
            return ;
        }
    }
    if (it->second.get_password_status() == 0)
        return ;
    if (tokens[0] == "NICK" && tokens.size() == 2 && it->second.get_nickname_status() == 0)
    {
        if (tokens[1].size() > 9)
        {
            sendToClient(it->first, nick_too_long);
            return ;
        }
        set_nick(tokens[1], it);
    }
    else if (tokens[0] == "USER" && tokens.size() >= 5 && it->second.get_username_status() == 0)
        set_user(tokens, it);
    if (it->second.get_password_status() && it->second.get_nickname_status()
        && it->second.get_username_status() && it->second.get_connection() == 0)
    {
        string connected = "Welcome " + it->second.get_nickname() + "!"  + it->second.get_username()
        + "@" + it->second.get_ip() + " to the ft_irc network !\r\n";
        it->second.set_connection(true);
        sendToClient(it->first, connected);
    }
}

void Server::sendToClient(int fd, const std::string& msg)
{
    if (send(fd, msg.c_str(), msg.length(), 0) < 0)
        cerr << RED << "Error: not send" << RESET << endl;
}

//cette fonction sert a traiter les messages privees envoyees entre client
//le format du message envoye suit les directives du protocole IRC
void Server::handle_prv_msg(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() < 3)
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    if (tokens[1][0] == '#')
    {
        // cout << PURPLE << "[DEBUG] PRIVMSG channel: [" << tokens[1] << "]" << RESET << endl;
        handle_channel_msg(tokens, it);
        return ;
    }

    // cout << PURPLE << "[DEBUG] PRIVMSG user: [" << tokens[1] << "]" << RESET << endl;
    map<int, Client>::iterator ite = _clients.begin();
    for (; ite != _clients.end(); ++ite)
    {
        // cout << PURPLE << "[DEBUG] compare with client nick: ["
        //  << ite->second.get_nickname() << "]" << RESET << endl;
        if (ite->second.get_nickname() == tokens[1])
            break;
    }

    if (ite == _clients.end())
    {
        sendToClient(it->first, usr_not_found);
        return ;
    }

    if (tokens[2][0] != ':')
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string final_msg = ":" + it->second.get_nickname() + "!" 
                        + it->second.get_username() + "@" + it->second.get_ip()
                        + " PRIVMSG " + tokens[1];

    for (size_t i = 2; i < tokens.size(); i++)
        final_msg += " " + tokens[i];

    final_msg += "\r\n";

    sendToClient(ite->first, final_msg);
}

//cette fonction va parser et executer la ligne recu par le client
//elle va d'abord checker si le client a bien le droit d'envoyer des messages
//continuer a coder le channel
void Server::parse_line(string line, int curr_fd)
{

    // cout << YELLOW << "[DEBUG] fd " << curr_fd << " line: [" << line << "]" << RESET << endl;

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

    // cout << BLUE << "[DEBUG] tokens:";
    // for (size_t i = 0; i < tokens.size(); i++)
    //     cout << " [" << tokens[i] << "]";
    // cout << RESET << endl;

    if (tokens.empty())
        return ;

    if (tokens[0] == "NICK" && tokens.size() == 2)
        set_nick(tokens[1], it);
    else if (tokens[0] == "PRIVMSG" && tokens.size() >= 3)
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

//cette fonction va gerer la connection de tous les nouveaux clients
//et ajouter ce client a la liste de surveillance epoll
void Server::handle_connection()
{
    // on accueille le nouveau client
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);
    int client_fd = accept(_socket_fd, 
        (struct sockaddr *)&client_addr, &addr_size); // on accepte le client                    
    if (client_fd < 0)
    {
        cerr << RED <<  "not accepted" << RESET << endl;
        return ;
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
        throw runtime_error("fcntl(1)");
    cout << GREEN << "Client " << client_fd << " connected" << RESET << endl;
    _clients.insert(make_pair(client_fd, Client(client_fd))); // on met le client dans notre std::map
    _clients[client_fd].set_ip(inet_ntoa(client_addr.sin_addr)); // on recupere l'adresse ip

    struct epoll_event client_ev; // on ajoute le client dans la liste de surveillance
    client_ev.events = EPOLLIN;
    client_ev.data.fd = client_fd;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0)
        throw runtime_error("epoll_ctl(1)");
}

//cette fonction va gerer tout ce que le client (qui est connecte)
//va entrer comme input, l'input sera ensuite envoye a parse_line(...)
void Server::handle_input(int i)
{
    //recv est l'equivalent de la fonction read()
    int curr_fd = _events[i].data.fd; // on prend le fd de l'event

    char buf[1024]; //buffer pour stocker le message du client
    int size_buf = recv(curr_fd, buf, 1024, 0); //size_buf correspond a ce qui a pu etre lu


    // cout << BLUE << "[DEBUG] recv fd " << curr_fd
    //  << " size: " << size_buf << RESET << endl;

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

    // cout << BLUE << "[DEBUG] raw buffer: [";
    // cout.write(buf, size_buf);
    // cout << "]" << RESET << endl;

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

        // cout << BLUE << "[DEBUG] parsed line before parse_line: ["
            // << line << "]" << RESET << endl;

        parse_line(line, curr_fd);
    }
}

void Server::handle_join(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() < 2 || tokens.size() > 3)
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string channelName = tokens[1];

    if (channelName.empty() || channelName[0] != '#')
    {
        sendToClient(it->first, incor_format);
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
        sendToClient(it->first, "Error: invite only channel\r\n");
        return ;
    }

    if (channel.has_key())
    {
        if (tokens.size() != 3 || tokens[2] != channel.get_key())
        {
            sendToClient(it->first, "Error: bad channel key\r\n");
            return ;
        }
    }

    if (channel.has_limit() && channel.get_members().size() >= static_cast<size_t>(channel.get_limit()))
    {
        sendToClient(it->first, "Error: channel is full\r\n");
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
}

void Server::handle_channel_msg(vector<string> tokens, map<int, Client>::iterator it)
{
    string channelName = tokens[1];

    map<string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end())
    {
        sendToClient(it->first, "Error: channel not found\r\n");
        return ;
    }

    Channel& channel = chanIt->second;

    if (!channel.is_member(it->first))
    {
        sendToClient(it->first, "Error: you're not on that channel\r\n");
        return ;
    }

    if (tokens[2].empty() || tokens[2][0] != ':')
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string final_msg = ":" + it->second.get_nickname() + "!"
                     + it->second.get_username() + "@"
                     + it->second.get_ip()
                     + " PRIVMSG " + channelName;

    for (size_t i = 2; i < tokens.size(); i++)
        final_msg += " " + tokens[i];

    final_msg += "\r\n";

    broadcastToChannel(channelName, final_msg, it->first);
}

void Server::broadcastToChannel(const string& channelName, const string& msg, int exceptFd)
{
    map<string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end())
        return ;

    set<int> members = chanIt->second.get_members();

    for (set<int>::iterator it = members.begin(); it != members.end(); ++it)
    {
        int fd = *it;

        if (fd == exceptFd)
            continue ;

        if (_clients.find(fd) != _clients.end())
            sendToClient(fd, msg);
    }
}

void Server::handle_part(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() != 2)
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string channelName = tokens[1];

    map<string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end())
    {
        sendToClient(it->first, "Error: channel not found\r\n");
        return ;
    }

    Channel& channel = chanIt->second;

    if (!channel.is_member(it->first))
    {
        sendToClient(it->first, "Error: you're not on that channel\r\n");
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

void Server::removeClientFromChannels(int fd)
{
    map<string, Channel>::iterator it = _channels.begin();

    while (it != _channels.end())
    {
        if (it->second.is_member(fd))
            it->second.remove_member(fd);

        if (it->second.empty())
            _channels.erase(it++);
        else
            ++it;
    }
}

void Server::handle_topic(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() < 2)
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string channelName = tokens[1];

    map<string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end())
    {
        sendToClient(it->first, "Error: channel not found\r\n");
        return ;
    }

    Channel& channel = chanIt->second;

    if (!channel.is_member(it->first))
    {
        sendToClient(it->first, "Error: you're not on that channel\r\n");
        return ;
    }

    if (tokens.size() == 2)
    {
        if (channel.get_topic().empty())
            sendToClient(it->first, "No topic is set\r\n");
        else
            sendToClient(it->first, "Topic: " + channel.get_topic() + "\r\n");
        return ;
    }

    if (channel.is_topic_restricted() && !channel.is_operator(it->first))
    {
        sendToClient(it->first, "Error: you're not an operator on that channel\r\n");
        return ;
    }

    if (tokens[2].empty() || tokens[2][0] != ':')
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string topic = tokens[2].substr(1);

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

void Server::handle_kick(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() < 3)
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string channelName = tokens[1];
    string targetKick = tokens[2];

    map<string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end())
    {
        sendToClient(it->first, "Error: channel not found\r\n");
        return ;
    }

    Channel& channel = chanIt->second;

    if (!channel.is_member(it->first))
    {
        sendToClient(it->first, "Error: you're not on that channel\r\n");
        return ;
    }

    if (!channel.is_operator(it->first))
    {
        sendToClient(it->first, "Error: you're not an operator on that channel\r\n");
        return ;
    }

    map<int, Client>::iterator userIt = _clients.begin();
    for (; userIt != _clients.end(); ++userIt)
    {
        if (userIt->second.get_nickname() == targetKick)
            break ;
    }

    if (userIt == _clients.end())
    {
        sendToClient(it->first, "Error: user not found\r\n");
        return ;
    }

    if (!channel.is_member(userIt->first))
    {
        sendToClient(it->first, "Error: user is not on that channel\r\n");
        return ;
    }

    string kickMsg = ":" + it->second.get_nickname() + "!"
                   + it->second.get_username() + "@"
                   + it->second.get_ip()
                   + " KICK " + channelName + " " + targetKick + "\r\n";

    sendToClient(it->first, kickMsg);
    broadcastToChannel(channelName, kickMsg, it->first);

    channel.remove_member(userIt->first);

    if (channel.empty())
        _channels.erase(chanIt);
}

void Server::handle_invite(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() != 3)
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string targetNick = tokens[1];
    string channelName = tokens[2];

    map<string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end())
    {
        sendToClient(it->first, "Error: channel not found\r\n");
        return ;
    }

    Channel& channel = chanIt->second;

    if (!channel.is_member(it->first))
    {
        sendToClient(it->first, "Error: you're not on that channel\r\n");
        return ;
    }

    if (channel.is_invite_only() && !channel.is_operator(it->first))
    {
        sendToClient(it->first, "Error: you're not an operator on that channel\r\n");
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
        sendToClient(it->first, "Error: user not found\r\n");
        return ;
    }

    if (channel.is_member(targetIt->first))
    {
        sendToClient(it->first, "Error: user already on channel\r\n");
        return ;
    }

    channel.add_invited(targetIt->first);

    string inviteMsg = ":" + it->second.get_nickname() + "!"
                     + it->second.get_username() + "@"
                     + it->second.get_ip()
                     + " INVITE " + targetNick + " " + channelName + "\r\n";

    sendToClient(targetIt->first, inviteMsg);
    sendToClient(it->first, "Invited " + targetNick + " to " + channelName + "\r\n");
}

void Server::handle_mode(vector<string> tokens, map<int, Client>::iterator it)
{
    if (tokens.size() < 3)
    {
        sendToClient(it->first, incor_format);
        return ;
    }

    string channelName = tokens[1];
    string mode = tokens[2];

    map<string, Channel>::iterator chanIt = _channels.find(channelName);
    if (chanIt == _channels.end())
    {
        sendToClient(it->first, "Error: channel not found\r\n");
        return ;
    }

    Channel& channel = chanIt->second;

    if (!channel.is_member(it->first))
    {
        sendToClient(it->first, "Error: you're not on that channel\r\n");
        return ;
    }

    if (!channel.is_operator(it->first))
    {
        sendToClient(it->first, "Error: you're not an operator on that channel\r\n");
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
            sendToClient(it->first, incor_format);
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
            sendToClient(it->first, incor_format);
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
            sendToClient(it->first, incor_format);
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
            sendToClient(it->first, "Error: user not found\r\n");
            return ;
        }

        if (!channel.is_member(targetIt->first))
        {
            sendToClient(it->first, "Error: user is not on that channel\r\n");
            return ;
        }

        if (mode == "+o")
            channel.add_operator(targetIt->first);
        else
            channel.remove_operator(targetIt->first);
    }
    else
    {
        sendToClient(it->first, "Error: unknown mode\r\n");
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

//configuration serveur et connection au reseau
void Server::init()
{
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd < 0)
    {
        close(_socket_fd);
        throw runtime_error("socket");
    }

    int opt = 1;
    if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(_socket_fd);
        throw runtime_error("setsockopt");
    }

    // fcntl fait en sorte que les connections au serveur soient non bloquantes
    if (fcntl(_socket_fd, F_SETFL, O_NONBLOCK) < 0) 
    {
        close(_socket_fd);
        throw runtime_error("fcntl");
    }

    _serverAddr.sin_family = AF_INET; //IPv4
    _serverAddr.sin_port = htons(_port); // converti le type pour pouvoir l'envoyer a travers le reseau
    _serverAddr.sin_addr.s_addr = INADDR_ANY; //accepte n'importe quel type de connection

    if (bind(_socket_fd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0)
    {
        close(_socket_fd);
        throw runtime_error("bind");
    }
    if (listen(_socket_fd, 5) < 0) // value to change to reveive more client
    {
        close(_socket_fd);
        throw runtime_error("listen");
    }
}

//configuation d'epoll() qui servira a verifier s'il y a 
//de nouveaux evenements a traiter
void Server::start()
{
    signal(SIGINT, intHandler);
    _epoll_fd = epoll_create1(0); // _epollfd sera un fd qui surveillera les _events
    if (_epoll_fd < 0)
        throw runtime_error("epoll_create1");

    struct epoll_event ev;
    ev.events = EPOLLIN; // evenements rentrants, lecture seule
    ev.data.fd = _socket_fd;

    //  ajoute mon serveur (_socket_fd) dans la liste des evenements a surveiller (_epoll_fd)
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev) < 0)
        throw runtime_error("epoll_ctl");

    cout << PURPLE << "Server listening ..." << RESET << endl;

    while (keepRunning) // tant que je ne recois pas de signal ctrl + C
    {
        int ev_rdy = epoll_wait(_epoll_fd, _events, MAX_EVENT, -1); // endors le programme
        // le programme se reveillera quand il y aura un evenement a gerer
        if (ev_rdy < 0 && keepRunning)
            throw runtime_error("epoll_wait()");
        for (int i = 0; i < ev_rdy; i++) // rentre dans la boucle d'evenements
        {
            if (_events[i].data.fd == _socket_fd) // si c'est un client qui rentre
                handle_connection();
            else // si le client ecrit
                handle_input(i);
        }
    }
    close_fds(); // on ferme le fd (il y a surement d'autres fd a fermer)
}