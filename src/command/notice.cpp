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

    std::string target = msg.Parameters[0];
    std::string message = msg.Parameters[1];
    std::string prefix = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname();

    // Check if target is a channel
    if (target[0] == '#' || target[0] == '&')
    {
        // Channel must exist
        if (_channels.find(target) == _channels.end())
            return;

        // User should be in channel to send notice
        if (!cli.isInChannel(target))
            return;

        // Broadcast notice to all channel members except sender
        std::vector<int> members = _channels[target].getMembers();
        for (size_t i = 0; i < members.size(); i++)
        {
            if (members[i] != (int)msg.fd)
            {
                sendReply(members[i], prefix + " NOTICE " + target + " :" + message);
            }
        }
    }
    else
    {
        // Target is a user
        int targetFd = -1;
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second.getUsername() == target)
            {
                targetFd = it->first;
                break;
            }
        }

        // Silently ignore if user doesn't exist
        if (targetFd == -1)
            return;

        // Send notice to target
        sendReply(targetFd, prefix + " NOTICE " + target + " :" + message);
    }
}
