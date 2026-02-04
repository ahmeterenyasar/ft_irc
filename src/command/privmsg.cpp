#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

void Server::privmsgCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkMinParams(this, msg, cli, 1, "PRIVMSG"))
    {
        sendReply(msg.fd, ":server 411 " + nick + " :No recipient given (PRIVMSG)");
        return;
    }

    if (msg.Parameters.size() < 2 || msg.Parameters[1].empty())
    {
        sendReply(msg.fd, ":server 412 " + nick + " :No text to send");
        return;
    }

    if (!checkRegistered(this, msg, cli))
        return;

    std::string targets = msg.Parameters[0];
    std::string message = msg.Parameters[1];

    std::vector<std::string> targetList = split(targets, ',');

    for (size_t i = 0; i < targetList.size(); ++i)
    {
        std::string target = targetList[i];
        
        if (target.empty())
            continue;

        if (target[0] == '#')
        {
            if (!haschannel(target))
            {
                sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel");
                continue;
            }

            Channel* channel = findChannel(this, target);

            if (channel == NULL)
                continue;

            if (!channel->hasUser(msg.fd))
            {
                sendReply(msg.fd, ":server 404 " + nick + " " + target + " :Cannot send to channel");
                continue;
            }

            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string privmsgMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname +
                                     " PRIVMSG " + target + " :" + message + "\r\n";
            
            broadcastToChannel(this, channel, privmsgMsg, msg.fd);
        }
        else
        {
            size_t targetFd = 0;
            Client* targetClient = findClientByNick(this, target, targetFd);

            if (targetClient == NULL)
            {
                sendReply(msg.fd, ":server 401 " + nick + " " + target + " :No such nick/channel");
                continue;
            }

            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string privmsgMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname +
                                     " PRIVMSG " + target + " :" + message + "\r\n";
            send(targetFd, privmsgMsg.c_str(), privmsgMsg.length(), 0);

        }
    }
}