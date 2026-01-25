#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - INVITE command
// Syntax: INVITE <nickname> <channel>
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHNICK (401),
//                  ERR_NOTONCHANNEL (442), ERR_USERONCHANNEL (443),
//                  ERR_CHANOPRIVSNEEDED (482), RPL_INVITING (341)
void Server::inviteCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();


}
