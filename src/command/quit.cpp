#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - QUIT command
// Syntax: QUIT [<quit message>]
// No numeric replies defined for successful QUIT
void Server::quitCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();
    std::string quitMsg = "Client Quit";

}
