#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

/**
 * PING - Ping command
 * 
 * The PING command is used to test the presence of an active client or
 * server at the other end of the connection. A PING message results in
 * a PONG reply.
 * 
 * Format: PING <server1> [<server2>]
 * Response: PONG <server2> <server1>
 */
void Server::pingCommand(IRCMessage& msg)
{
    if (msg.Parameters.empty())
    {
        // ERR_NOORIGIN (409)
        Client& cli = _clients[msg.fd];
        std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
        sendReply(msg.fd, ":server 409 " + nick + " :No origin specified");
        return;
    }

    // Respond with PONG
    std::string token = msg.Parameters[0];
    sendReply(msg.fd, ":server PONG server :" + token);
}
