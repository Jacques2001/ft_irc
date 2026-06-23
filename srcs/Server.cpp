#include "../includes/Server.hpp"

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
Server::Server() : _port(0), _serversocket(-1), _server_passcode("0")
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::Server(int port, string serverpass) : _port(port), _serversocket(-1),
 _server_passcode(serverpass)
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::~Server() {}

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

//cette fonction sert a verifier si le client a bien rentre le mdp, nickname et username
//avant de valider la connection
void Server::connection_process(string line, map<int, Client>::iterator it)
{
    stringstream ss(line);
    string token;
    vector<string> tokens;
    while (ss >> token)
        tokens.push_back(token);
    if (tokens.size() != 2)
    {
        if (send(it->first, err_args, strlen(err_args), 0) < 0)
            cerr << RED << "Error: not send" << RESET << endl;
        return ;
    }
    if (tokens[0] == "PASS" || tokens[0] == "NICK" || tokens[0] == "USER")
    {
        if (tokens[0] == "PASS" && it->second.get_password_status() == 0)
        {
            if (is_passcode(tokens[1]) == true)
                it->second.has_password();
            else
                if (send(it->first, pass_incorrect, strlen(pass_incorrect), 0) < 0)
                   cerr << RED << "Error: not send" << RESET << endl;
        }
        else if (tokens[0] == "NICK" && it->second.get_nickname_status() == 0)
        {
            if (check_double(tokens[1], "nick"))
            {
                if (send(it->first, name_alrdy_taken, strlen(name_alrdy_taken), 0) < 0)
                   cerr << RED << "Error: not send" << RESET << endl;
                return ;
            }
            it->second.set_nickname(tokens[1]);
            it->second.has_nickname();
        }
        else if (tokens[0] == "USER" && it->second.get_username_status() == 0)
        {
            if (check_double(tokens[1], "user"))
            {
                if (send(it->first, name_alrdy_taken, strlen(name_alrdy_taken), 0) < 0)
                    cerr << RED << "Error: not send" << RESET << endl;
                return ;
            }
            it->second.set_username(tokens[1]);
            it->second.has_username();
        }
    }
    else
        if (send(it->first, prior_connect, strlen(prior_connect), 0) < 0)
           cerr << RED << "Error: not send" << RESET << endl;
    if (it->second.get_password_status() && it->second.get_nickname_status()
        && it->second.get_username_status())
    {
        string connected = "Welcome " + it->second.get_nickname() + "!"  + it->second.get_username()
        + "@" + _ip_address + " to the ft_irc network !\r\n";
        it->second.set_connection(true);
        if (send(it->first, connected.c_str(), connected.length(), 0) < 0)
           cerr << RED << "Error: not send" << RESET << endl;
    }
}

//cette fonction sert a traiter les messages privees envoyees entre client
//le format du message envoye suit les directives du protocole IRC
void Server::handle_prv_msg(vector<string> tokens, map<int, Client>::iterator it)
{
    map<int, Client>::iterator ite = _clients.begin();
    for (; ite != _clients.end(); ++ite)
    {
        if (ite->second.get_nickname() == tokens[1])
            break;
    }
    if (ite == _clients.end())
    {
        if (send(it->first, usr_not_found, strlen(usr_not_found), 0) < 0)
           cerr << RED << "Error: not send" << RESET << endl;
        return ;
    }
    if (tokens[2][0] != ':')
    {
        if (send(it->first, incor_format, strlen(incor_format), 0) < 0)
           cerr << RED << "Error: not send" << RESET << endl;
    }
    string final_msg = ":" + it->second.get_nickname() + "!" 
                        + it->second.get_username() + "@" + _ip_address;
    for (size_t i = 2; i < tokens.size(); i++)
        final_msg.append(" " + tokens[i]);
    final_msg += "\r\n";
    if (send(ite->first, final_msg.c_str(), final_msg.length(), 0) < 0)
       cerr << RED << "Error: not send" << RESET << endl;
}

//cette fonction va parser et executer la ligne recu par le client
//elle va d'abord checker si le client a bien le droit d'envoyer des messages
//continuer a coder le channel
void Server::parse_line(string line, int curr_fd)
{
    if (line.empty())
        return ;
    map<int, Client>::iterator it = _clients.find(curr_fd);
    if (it->second.get_connection() == 0)
        connection_process(line, it);
    stringstream ss(line);
    string token;
    vector<string> tokens;
    while (ss >> token)
        tokens.push_back(token);
    if (tokens[0] == "PRIVMSG" && tokens.size() >= 3)
        handle_prv_msg(tokens, it);
    // if (tokens[0] == "JOIN")
        
}

//cette fonction va gerer la connection de tous les nouveaux clients
//et ajouter ce client a la liste de surveillance epoll
void Server::handle_connection()
{
    // on accueille le nouveau client
    struct sockaddr_in client_addr;
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);
    _client_fd = accept(_serversocket, 
        (struct sockaddr *)&client_addr, &addr_size); // on accepte le client                    
    if (_client_fd < 0)
        cerr << RED <<  "not accepted" << RESET << endl;
    if (fcntl(_client_fd, F_SETFL, O_NONBLOCK) < 0)
        throw runtime_error("fcntl(1)");
    cout << GREEN << "Client " << _client_fd << " connected" << RESET << endl;
    _clients.insert(make_pair(_client_fd, Client(_client_fd))); // on met le client dans notre std::map
    _ip_address = inet_ntoa(client_addr.sin_addr); // on recupere l'adresse ip

    struct epoll_event client_ev; // on ajoute le client dans la liste de surveillance
    client_ev.events = EPOLLIN;
    client_ev.data.fd = _client_fd;
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _client_fd, &client_ev) < 0)
        throw runtime_error("epoll_ctl(1)");
}

//cette fonction va gerer tout ce que le client (qui est connecte)
//va entrer comme input
void Server::handle_input(int i)
{
    //recv est l'equivalent de la fonction read()
    int curr_fd = _events[i].data.fd; // on prend le fd de l'event
    char buf[1024]; //buffer pour stocker le message du client
    int size_buf = recv(curr_fd, buf, 1024, 0); //size_buf correspond a ce qui a pu etre lu
    if (size_buf == 0) // si c'est = 0 c'est que le client s'est deconnecte
    {
        epoll_ctl(_epollfd, EPOLL_CTL_DEL, curr_fd, NULL);
        _clients.erase(curr_fd);
        close(curr_fd);
        cout << YELLOW << "Client " << curr_fd << " disconnected" << RESET << endl;
    }
    if (size_buf < 0) // si c'est < 0  on continue quand meme le programme
        close(curr_fd);
    //ces lignes ci-dessous sont faites pour regler le probleme de 
    //"donnees partielles" recues par recv()
    _clients[curr_fd].appendToBuffer(buf, size_buf);
    std::string &client_buffer = _clients[curr_fd].getBuffer();
    size_t pos;
    while ((pos = client_buffer.find("\r\n")) != std::string::npos)
    {
        std::string line = client_buffer.substr(0, pos);
        client_buffer.erase(0, pos + 2);
        parse_line(line, curr_fd);
    }
}

//configuration serveur et connection au reseau
void Server::init()
{
    _serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serversocket < 0)
        throw runtime_error("socket");

    int opt = 1;
    if (setsockopt(_serversocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw runtime_error("setsockopt");
    // fcntl fait en sorte que les connections au serveur soient non bloquantes
    if (fcntl(_serversocket, F_SETFL, O_NONBLOCK) < 0) 
        throw runtime_error("fcntl");

    _serverAddr.sin_family = AF_INET; //IPv4
    _serverAddr.sin_port = htons(_port); // converti le type pour pouvoir l'envoyer a travers le reseau
    _serverAddr.sin_addr.s_addr = INADDR_ANY; //accepte n'importe quel type de connection

    if (bind(_serversocket, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0)
        throw runtime_error("bind");
    if (listen(_serversocket, 5) < 0) // value to change to reveive more client
        throw runtime_error("listen");
}

//configuation d'epoll() qui servira a verifier s'il y a 
//de nouveaux evenements a traiter
void Server::start()
{
    signal(SIGINT, intHandler);
    _client_fd = -1;
    _epollfd = epoll_create1(0); // _epollfd sera un fd qui surveillera les _events
    if (_epollfd < 0)
        throw runtime_error("epoll_create1");

    struct epoll_event ev;
    ev.events = EPOLLIN; // evenements rentrants, lecture seule
    ev.data.fd = _serversocket;

    //  ajoute mon serveur (_serversocket) dans la liste des evenements a surveiller (_epollfd)
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _serversocket, &ev) < 0)
        throw runtime_error("epoll_ctl");

    cout << PURPLE << "Server listening ..." << RESET << endl;

    while (keepRunning) // tant que je ne recois pas de signal ctrl + C
    {
        int ev_rdy = epoll_wait(_epollfd, _events, MAX_EVENT, -1); // endors le programme
        // le programme se reveillera quand il y aura un evenement a gerer
        if (ev_rdy < 0 && keepRunning)
            throw runtime_error("epoll_wait()");
        for (int i = 0; i < ev_rdy; i++) // rentre dans la boucle d'evenements
        {
            if (_events[i].data.fd == _serversocket) // si c'est un client qui rentre
                handle_connection();
            else // si le client ecrit
                handle_input(i);
        }
    }
    close(_epollfd); // on ferme le fd (il y a surement d'autres fd a fermer)
}