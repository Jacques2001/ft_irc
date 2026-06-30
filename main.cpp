#include "includes/Client.hpp"
#include "includes/Server.hpp"

void check_args(char** av)
{
    if (!av[1] || !av[1][0])
        throw runtime_error("port incorrect");

    for (size_t i = 0 ; av[1][i]; ++i)
    {
        if (!isdigit(av[1][i]))
                throw runtime_error("non digit port");
    }

    int port = std::atoi(av[1]);
    if (port < 1024 || port > 65535)
    throw runtime_error("port incorrect");
}

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "./ircserv <port> <password>" << std::endl;
        return 1;
    }
    try
    {
        check_args(av);
        Server serv(std::atoi(av[1]), av[2]);
        serv.init();
        serv.start();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error :" << e.what() << '\n';
        return 1;
    }
    return 0;
}