/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:33 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:34 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

void Server::topicCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkMinParams(this, msg, cli, 1, "TOPIC"))
        return;

    if (!checkRegistered(this, msg, cli))
        return;

    std::string channelName = msg.Parameters[0];

    if (!haschannel(channelName))
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
        return;
    }

    Channel* channel = findChannel(this, channelName);
    if (channel == NULL)
        return;

    if (!checkUserInChannel(this, msg, cli, channel, channelName))
        return;

    if (msg.Parameters.size() == 1)
    {
        if (channel->getTopic().empty())
            sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set");
        else
            sendReply(msg.fd, ":server 332 " + nick + " " + channelName + " :" + channel->getTopic());
        return;
    }

    std::string newTopic;
    for (size_t i = 1; i < msg.Parameters.size(); ++i)
    {
        if (i > 1)
            newTopic += " ";
        newTopic += msg.Parameters[i];
    }

    if (channel->isTopicRestricted())
    {
        if (!channel->isOperator(msg.fd))
        {
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator");
            return;
        }
    }

    channel->setTopic(newTopic);

    std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
    std::string topicMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname +
                           " TOPIC " + channelName + " :" + newTopic + "\r\n";
    
    broadcastToChannel(this, channel, topicMsg, 0);
}