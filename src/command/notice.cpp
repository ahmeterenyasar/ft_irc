#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - NOTICE command
// Syntax: NOTICE <target>{,<target>} :<message>
// NOTICE is similar to PRIVMSG but MUST NOT generate automatic replies
// No numeric replies are sent for NOTICE command errors
void Server::noticeCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    // NOTICE should not generate error replies, so we silently ignore errors
    if (msg.Parameters.empty() || msg.Parameters.size() < 2)
        return;
}
