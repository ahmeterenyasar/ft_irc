#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - TOPIC command
// Syntax: TOPIC <channel> [<topic>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOTONCHANNEL (442),
//                  ERR_CHANOPRIVSNEEDED (482), RPL_NOTOPIC (331),
//                  RPL_TOPIC (332)
void Server::topicCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    // ERR_NEEDMOREPARAMS (461)
    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " TOPIC :Not enough parameters");
        return;
    }
    
}
