#include "includes/Client.hpp"
#include "includes/Server.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "./ircserv <port> <password>" << std::endl;
        return 1;
    }
    try
    {
        Server serv(std::atoi(av[1]), av[2]);
        serv.init();
        serv.start();
        serv.loop();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error :" << e.what() << '\n';
    }
    return 0;
}