#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

/**
 * CAP - Capability Negotiation
 * 
 * Modern IRC clients use CAP to negotiate capabilities.
 * For basic IRC server, we acknowledge but don't support any capabilities.
 * 
 * Common CAP subcommands:
 * - CAP LS: List available capabilities
 * - CAP REQ: Request specific capabilities
 * - CAP END: End capability negotiation
 */
void Server::capCommand(IRCMessage& msg)
{
    if (msg.Parameters.empty())
        return;

    std::string subcommand = msg.Parameters[0];
    
    // Convert to uppercase for comparison
    for (size_t i = 0; i < subcommand.length(); i++)
        subcommand[i] = std::toupper(subcommand[i]);

    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (subcommand == "LS")
    {
        // List capabilities - we support none, so send empty list
        // CAP LS 302 format: some clients send "CAP LS 302" for IRCv3.2
        sendReply(msg.fd, ":server CAP " + nick + " LS :");
    }
    else if (subcommand == "REQ")
    {
        // Request capabilities - deny all requests
        if (msg.Parameters.size() > 1)
            sendReply(msg.fd, ":server CAP " + nick + " NAK :" + msg.Parameters[1]);
    }
    else if (subcommand == "END")
    {
        // End capability negotiation - just acknowledge
        // Client will proceed with normal registration
    }
    else if (subcommand == "LIST")
    {
        // List active capabilities - we have none
        sendReply(msg.fd, ":server CAP " + nick + " LIST :");
    }
}
