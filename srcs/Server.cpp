#include "../includes/Server.hpp"
using namespace std;

static volatile int keepRunning = 1;

//reception du signal ctrl + c
void intHandler(int dummy)
{
    (void)dummy;
    cout << "\nServer closed" << endl;
    keepRunning = 0;
}

//memset pour eviter de retrouver des donnees "poubelles"
//dans la structure _serverAddr
Server::Server() : _port(0), _serversocket(-1)
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::Server(int port) : _port(port), _serversocket(-1)
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

Server::~Server()
{
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
    // fcntl faire en sorte que les connections au serveur soient non bloquantes
    if (fcntl(_serversocket, F_SETFL, O_NONBLOCK) < 0) 
        throw runtime_error("fcntl");

    _serverAddr.sin_family = AF_INET; // protocol iPv4
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
    int client_fd = -1;
    int epollfd = epoll_create1(0); // epollfd sera un fd qui surveillera les events
    if (epollfd < 0)
        throw runtime_error("epoll_create1");

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = _serversocket;

    struct epoll_event events[MAX_EVENT]; // le nombre max d'event que mon serveur peut gerer

    //  ajoute mon serveur (_serversocket) dans la liste des evenements a surveiller (epollfd)
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, _serversocket, &ev) < 0)
        throw runtime_error("epoll_ctl");

    cout << "Server listening ..." << endl;

    while (keepRunning) // tant que je ne recois pas de signal ctrl + C
    {
        int ev_rdy = epoll_wait(epollfd, events, MAX_EVENT, -1); // endors le programme
        // le programme se reveillera quand il y aura un evenement a gerer
        if (ev_rdy < 0 && keepRunning)
            throw runtime_error("epoll_wait()");
        for (int i = 0; i < ev_rdy; i++) // rentre dans la boucle d'evenements
        {
            if (events[i].data.fd == _serversocket) // si c'est un client qui rentre
            {
                cout << "Client connected" << endl;

                // on accueille le nouveau client
                struct sockaddr_in client_addr;
                socklen_t addr_size;
                addr_size = sizeof(struct sockaddr_in);
                client_fd = accept(_serversocket, 
                    (struct sockaddr *)&client_addr, &addr_size); // on accepte le client
                if (client_fd < 0)
                    std::cerr << "not accepted" << endl;

                struct epoll_event client_ev; // on ajoute le client dans la liste de surveillance
                client_ev.events = EPOLLIN;
                client_ev.data.fd = client_fd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0)
                    throw runtime_error("epoll_ctl(1)");
            }
            else // si le client ecrit
            {
                //recv est l'equivalent de la fonction read()
                int curr_fd = events[i].data.fd;
                char buf[1024];
                int size_buf = recv(curr_fd, buf, 1024, 0);
                if (size_buf == 0)
                {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, curr_fd, NULL);
                    close(curr_fd);
                    cout << "Client disconnected" << endl;
                }
                if (size_buf < 0)
                {
                    close(curr_fd);
                    throw runtime_error("recv");
                }
                buf[size_buf] = '\0';
            }
        }
    }
    close(epollfd); // on ferme le fd (il y a surement d'autres fd a fermer)
}