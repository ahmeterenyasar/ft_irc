#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - PART command
// Syntax: PART <channel>{,<channel>} [<part message>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_NOTONCHANNEL (442)
void Server::partCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " PART :Not enough parameters");
        return;
    }

}
