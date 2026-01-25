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

    std::string target = msg.Parameters[0];
    std::string message = msg.Parameters[1];
    std::string prefix = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname();

    // Check if target is a channel
    if (target[0] == '#' || target[0] == '&')
    {
        // ERR_NOSUCHCHANNEL (403)
        if (_channels.find(target) == _channels.end())
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel");
            return;
        }

        // ERR_CANNOTSENDTOCHAN (404) - Check if user is in the channel
        // (Some servers allow non-members to send, but we'll require membership)
        if (!cli.isInChannel(target))
        {
            sendReply(msg.fd, ":server 404 " + nick + " " + target + " :Cannot send to channel");
            return;
        }

        // Broadcast message to all channel members except sender
        std::vector<int> members = _channels[target].getMembers();
        for (size_t i = 0; i < members.size(); i++)
        {
            if (members[i] != (int)msg.fd)
            {
                sendReply(members[i], prefix + " PRIVMSG " + target + " :" + message);
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

        // ERR_NOSUCHNICK (401)
        if (targetFd == -1)
        {
            sendReply(msg.fd, ":server 401 " + nick + " " + target + " :No such nick/channel");
            return;
        }

        // Send private message to target
        sendReply(targetFd, prefix + " PRIVMSG " + target + " :" + message);
    }
}
