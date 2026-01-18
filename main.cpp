
#include "inc/server.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
        throw std::runtime_error("Usage: ./ircserv <port> <password>");

    try
    {
        // server init & start
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}
