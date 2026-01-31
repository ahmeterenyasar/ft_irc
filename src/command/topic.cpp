#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - TOPIC command
// Syntax: TOPIC <channel> [<topic>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOTONCHANNEL (442),
//                  ERR_CHANOPRIVSNEEDED (482), RPL_NOTOPIC (331),
//                  RPL_TOPIC (332)
void Server::topicCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " TOPIC :Not enough parameters\r\n");
        return;
    }

    if (!cli.isRegistered())
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered\r\n");
        return;
    }

    std::string channelName = msg.Parameters[0];

    if (!haschannel(channelName))
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel* channel = NULL;
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i].getName() == channelName)
        {
            channel = &_channels[i];
            break;
        }
    }

    if (channel == NULL)
        return;

    if (!channel->hasUser(msg.fd))
    {
        sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // TOPIC SORGULAMA
    // Sadece topic sorgusu mu?
    if (msg.Parameters.size() == 1)
    {
        // Topic boş mu?
        if (channel->getTopic().empty())
        {
            // RPL_NOTOPIC (331)
            sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set\r\n");
        }
        else
        {
            // RPL_TOPIC (332)
            sendReply(msg.fd, ":server 332 " + nick + " " + channelName + " :" + channel->getTopic() + "\r\n");
        }
        return;
    }

    // TOPIC DEĞİŞTİRME
    // Yeni topic'i al - TÜM kelimeleri birleştir (multi-word topic)
    std::string newTopic;
    for (size_t i = 1; i < msg.Parameters.size(); ++i)
    {
        if (i > 1)
            newTopic += " ";
        newTopic += msg.Parameters[i];
    }

    // Topic restriction kontrolü (+t modu)
    if (channel->isTopicRestricted())
    {
        // Sadece operator değiştirebilir
        if (!channel->isOperator(msg.fd))
        {
            // ERR_CHANOPRIVSNEEDED (482)
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator\r\n");
            return;
        }
    }
    // +t modu yoksa kanalda olan herkes topic değiştirebilir

    // Topic'i değiştir
    channel->setTopic(newTopic);

    // TOPIC değişikliğini tüm kanal üyelerine broadcast et (değiştiren dahil!)
    std::string topicMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() +
                           " TOPIC " + channelName + " :" + newTopic + "\r\n";
    
    std::vector<size_t> members = channel->getMembers();
    for (size_t i = 0; i < members.size(); ++i)
    {
        sendReply(members[i], topicMsg);
    }
}