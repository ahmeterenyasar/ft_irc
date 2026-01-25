#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - PRIVMSG command
// Syntax: PRIVMSG <target>{,<target>} :<message>
// Numeric replies: ERR_NORECIPIENT (411), ERR_NOTEXTTOSEND (412),
//                  ERR_CANNOTSENDTOCHAN (404), ERR_NOTOPLEVEL (413),
//                  ERR_WILDTOPLEVEL (414), ERR_NOSUCHNICK (401),
//                  RPL_AWAY (301)
void Server::privmsgCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    // ERR_NORECIPIENT (411)
    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 411 " + nick + " :No recipient given (PRIVMSG)");
        return;
    }

    // ERR_NOTEXTTOSEND (412)
    if (msg.Parameters.size() < 2 || msg.Parameters[1].empty())
    {
        sendReply(msg.fd, ":server 412 " + nick + " :No text to send");
        return;
    }

}
