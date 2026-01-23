
#include "inc/server.hpp"

bool isValidPort(char *port_str)
{
    int port;

    for (int i = 0; i < port_str[i]; i++)
    {
        if (!isdigit(port_str[i]))
            return false;
    }
    port = std::atoi(port_str);
    if (port < 1 || port > 65535)
        return false;
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Error: Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    if (!isValidPort(argv[1]))
    {
        std::cerr << "Error: Invalid port. Use range 1 - 65535" << std::endl;
        return 1;
    }
    try             
    {
        int port = std::atoi(argv[1]);
        std::string password = argv[2];

        Server server(port, password);
        server.init();
    }   
    catch(const std::exception& e)
    {
        std::cerr << "Server Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
