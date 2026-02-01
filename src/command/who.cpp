#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

/**
 * WHO - Who query command
 * 
 * RFC 2812 - WHO command
 * Syntax: WHO [<mask> ["o"]]
 * 
 * The WHO command is used to query information about users.
 * It can be used to get information about users on a specific channel
 * or matching a specific mask.
 * 
 * Numeric replies:
 * - RPL_WHOREPLY (352)
 * - RPL_ENDOFWHO (315)
 */
void Server::whoCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkRegistered(this, msg, cli))
        return;

    // WHO without parameters - list all visible users (we'll skip this for simplicity)
    if (msg.Parameters.empty())
    {
        // RPL_ENDOFWHO (315)
        sendReply(msg.fd, ":server 315 " + nick + " * :End of WHO list");
        return;
    }

    std::string mask = msg.Parameters[0];
    bool operatorsOnly = (msg.Parameters.size() > 1 && msg.Parameters[1] == "o");

    // Check if mask is a channel
    if (mask[0] == '#' || mask[0] == '&')
    {
        // WHO for a specific channel
        Channel* channel = findChannel(this, mask);
        
        if (channel == NULL)
        {
            // Channel doesn't exist, just send end of WHO
            sendReply(msg.fd, ":server 315 " + nick + " " + mask + " :End of WHO list");
            return;
        }

        // Send WHO reply for each member (no privacy check for public channels)
        std::vector<size_t> members = channel->getMembers();
        for (size_t i = 0; i < members.size(); ++i)
        {
            size_t memberFd = members[i];
            if (_clients.find(memberFd) == _clients.end())
                continue;

            Client& member = _clients[memberFd];
            std::string memberNick = member.getNickname();
            std::string memberUser = member.getUsername();
            std::string memberReal = member.getRealname();
            if (memberReal.empty())
                memberReal = memberNick; // Varsayılan olarak nickname kullan
            std::string memberHost = member.getHostname().empty() ? "localhost" : member.getHostname();

            // Skip if operators only and user is not operator
            if (operatorsOnly && !channel->isOperator(memberFd))
                continue;

            // Determine user status: H = Here, G = Gone (away)
            // Flags: @ = operator, + = voice (voice desteklenmiyorsa boş)
            std::string flags = "H";
            if (channel->isOperator(memberFd))
                flags += "@";

            // RPL_WHOREPLY (352)
            // Format: :<server> 352 <requester> <channel> <user> <host> <server> <nick> <flags> :<hopcount> <realname>
            std::string whoReply = ":server 352 " + nick + " " + mask + " " 
                                 + memberUser + " " + memberHost + " server " + memberNick + " " 
                                 + flags + " :0 " + memberReal;
            sendReply(msg.fd, whoReply);
        }

        // RPL_ENDOFWHO (315)
        sendReply(msg.fd, ":server 315 " + nick + " " + mask + " :End of WHO list");
    }
    else
    {
        // WHO for a specific user (mask is a nickname)
        size_t targetFd = 0;
        Client* targetClient = findClientByNick(this, mask, targetFd);

        if (targetClient != NULL)
        {
            std::string targetUser = targetClient->getUsername();
            std::string targetReal = targetClient->getRealname();
            if (targetReal.empty())
                targetReal = mask; // Varsayılan olarak nickname kullan
            std::string targetHost = targetClient->getHostname().empty() ? "localhost" : targetClient->getHostname();

            // RPL_WHOREPLY (352)
            // For user queries, channel is "*"
            std::string whoReply = ":server 352 " + nick + " * " 
                                 + targetUser + " " + targetHost + " server " + mask + " H :0 " + targetReal;
            sendReply(msg.fd, whoReply);
        }

        // RPL_ENDOFWHO (315)
        sendReply(msg.fd, ":server 315 " + nick + " " + mask + " :End of WHO list");
    }
}
