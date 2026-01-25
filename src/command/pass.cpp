#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - PASS command
// Syntax: PASS <password>
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_ALREADYREGISTRED (462)
void Server::passCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string userName;

    if (cli.getUsername().empty())
        userName = "*";
    else
        userName = cli.getUsername();

    // ERR_NEEDMOREPARAMS (461)
    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + userName + " PASS :Not enough parameters");
        return;
    }
    // ERR_ALREADYREGISTRED (462)
    if (cli.isRegistered() || cli.isAuthenticated() == true)
    {
        sendReply(msg.fd, ":server 462 " + userName + " :You may not reregister");
        return;
    }
    // ERR_PASSWDMISMATCH (464)
    if (msg.Parameters[0] != this->_password)
    {
        sendReply(msg.fd, ":server 464 " + userName + " :Password incorrect");
        cli.setAuthenticated(false);
        return;
    }
    cli.setAuthenticated(true);
}
