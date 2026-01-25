#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - KICK command
// Syntax: KICK <channel> <user> [<comment>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_CHANOPRIVSNEEDED (482), ERR_USERNOTINCHANNEL (441),
//                  ERR_NOTONCHANNEL (442)
void Server::kickCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    if (msg.Parameters.size() < 2)
    {
        sendReply(msg.fd, ":server 461 " + nick + " KICK :Not enough parameters");
        return;
    }

}
