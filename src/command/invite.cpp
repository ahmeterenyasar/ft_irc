#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"


void Server::inviteCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkRegistered(this, msg, cli))
        return;

    if (!checkMinParams(this, msg, cli, 2, "INVITE"))
        return;

    std::string targetNick = msg.Parameters[0];
    std::string channelName = msg.Parameters[1];
    
    if (targetNick == nick)
    {
        sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :Cannot invite yourself");
        return;
    }

    size_t targetFd = 0;
    Client* targetClient = findClientByNick(this, targetNick, targetFd);

    if (targetClient == NULL)
    {
        sendReply(msg.fd, ":server 401 " + nick + " " + targetNick + " :No such nick/channel");
        return;
    }

    Channel* channel = findChannel(this, channelName);

    if (channel != NULL)
    {
        if (!channel->hasUser(msg.fd))
        {
            sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :You're not on that channel");
            return;
        }
        if (channel->hasUser(targetFd))
        {
            sendReply(msg.fd, ":server 443 " + nick + " " + targetNick + " " + channelName + " :is already on channel");
            return;
        }
        if (channel->isInviteOnly() && !channel->isOperator(msg.fd))
        {
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator");
            return;
        }
    }
    if (channel != NULL)
    {
        channel->addInvite(targetNick);
    }

    sendReply(msg.fd, ":server 341 " + nick + " " + targetNick + " " + channelName);

    std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
    std::string inviteMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + 
                            " INVITE " + targetNick + " " + channelName + "\r\n";
    sendReply(targetFd, inviteMsg);

}
