#include "includes/Client.hpp"
#include "includes/Server.hpp"

int main(int ac, char **av)
{
    (void)ac;
    // if (ac != 3)
    // {
    //     std::cerr << "./ircserv <port> <password>" << std::endl;
    //     return 1;
    // }
    try
    {
        Server serv(std::atoi(av[1]));
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}