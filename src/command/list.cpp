#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"


void Server::listCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkRegistered(this, msg, cli))
        return;

    sendReply(msg.fd, ":server 321 " + nick + " Channel :Users  Name");

    if (msg.Parameters.empty())
    {
        for (size_t i = 0; i < _channels.size(); ++i)
        {
            Channel& channel = _channels[i];
            std::string channelName = channel.getName();
            std::string topic = channel.getTopic();
            int userCount = channel.getUserCount();

            std::stringstream ss;
            ss << userCount;
            std::string userCountStr = ss.str();

            sendReply(msg.fd, ":server 322 " + nick + " " + channelName + " " + userCountStr + " :" + topic);
        }

        sendReply(msg.fd, ":server 323 " + nick + " :End of LIST");
        return;
    }

    std::string channelList = msg.Parameters[0];
    std::vector<std::string> channels = split(channelList, ',');

    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channelName = channels[i];
        
        if (channelName.empty())
            continue;

        Channel* channel = findChannel(this, channelName);
        
        if (channel != NULL)
        {
            std::string topic = channel->getTopic();
            int userCount = channel->getUserCount();

            std::stringstream ss;
            ss << userCount;
            std::string userCountStr = ss.str();

            sendReply(msg.fd, ":server 322 " + nick + " " + channelName + " " + userCountStr + " :" + topic);
        }
    }

    sendReply(msg.fd, ":server 323 " + nick + " :End of LIST");
}