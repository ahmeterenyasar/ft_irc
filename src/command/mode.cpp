#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - MODE command
// Channel modes: MODE <channel> [<modestring> [<mode arguments>...]]
// User modes: MODE <nickname> [<modestring>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_CHANOPRIVSNEEDED (482),
//                  ERR_NOSUCHNICK (401), ERR_NOSUCHCHANNEL (403),
//                  ERR_UNKNOWNMODE (472), RPL_CHANNELMODEIS (324),
//                  RPL_UMODEIS (221)
void Server::modeCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters");
        return;
    }

}
