#include "../inc/server.hpp"

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