#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

void Server::capCommand(IRCMessage& msg)
{
    if (msg.Parameters.empty())
        return;

    std::string subcommand = msg.Parameters[0];

    for (size_t i = 0; i < subcommand.length(); i++)
        subcommand[i] = std::toupper(subcommand[i]);

    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (subcommand == "LS")
        sendReply(msg.fd, ":server CAP " + nick + " LS :");
    else if (subcommand == "REQ")
    {
        if (msg.Parameters.size() > 1)
            sendReply(msg.fd, ":server CAP " + nick + " NAK :" + msg.Parameters[1]);
    }
    else if (subcommand == "END"){}
    else if (subcommand == "LIST")
        sendReply(msg.fd, ":server CAP " + nick + " LIST :");
}
