#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - USER command
// Syntax: USER <username> <mode> <unused> :<realname>
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_ALREADYREGISTRED (462)
void Server::cmdUser(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string userName;

    if (cli.getUsername().empty())
        userName = "*";
    else
        userName = cli.getUsername();

    // Check if authenticated (PASS required first)
    if (!cli.isAuthenticated())
    {
        sendReply(msg.fd, ":server 451 " + userName + " :You have not registered");
        return;
    }
    // ERR_NEEDMOREPARAMS (461)
    if (msg.Parameters.size() < 4)
    {
        sendReply(msg.fd, ":server 461 " + userName + " USER :Not enough parameters");
        return;
    }
    // ERR_ALREADYREGISTRED (462)
    if (cli.isRegistered())
    {
        sendReply(msg.fd, ":server 462 " + userName + " :You may not reregister");
        return;
    }

    cli.setUsername(msg.Parameters[0]);
    cli.setRealname(msg.Parameters[3]);

    if (!cli.getUsername().empty())
    {
        cli.setRegistered(true);
        userName = cli.getUsername();
        
        sendReply(msg.fd, ":server 001 " + userName + " :Welcome to the Internet Relay Network " + userName + "!" + cli.getUsername() + "@server");
        sendReply(msg.fd, ":server 002 " + userName + " :Your host is server, running version 1.0");
        sendReply(msg.fd, ":server 003 " + userName + " :This server was created " + std::string(__DATE__));
        sendReply(msg.fd, ":server 004 " + userName + " server 1.0 o o");
    }
}
