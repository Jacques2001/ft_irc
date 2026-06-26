#include "includes/Client.hpp"
#include "includes/Server.hpp"

void check_args(string av)
{
    if (av.size() != 4)
        throw runtime_error("port incorrect");
    for(size_t i = 0; i < av.size(); i++)
        if (!isdigit(av[i]))
            throw runtime_error("non digit port");
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
        check_args(av[1]);
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