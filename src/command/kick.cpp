/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:02 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:03 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

void Server::kickCommand(IRCMessage& msg)
{
    std::map<size_t, Client>::iterator clientIt = _clients.find(msg.fd);
    if (clientIt == _clients.end())
        return;
    
    Client& cli = clientIt->second;
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkMinParams(this, msg, cli, 2, "KICK"))
        return;
    
    if (!checkRegistered(this, msg, cli))
        return;

    std::vector<std::string> channels = split(msg.Parameters[0], ',');
    std::vector<std::string> users = split(msg.Parameters[1], ',');
    std::string reason = (msg.Parameters.size() >= 3) ? msg.Parameters[2] : nick;

    if (channels.size() != 1 && channels.size() != users.size())
    {
        sendReply(msg.fd, ":server 461 " + nick + " KICK :Not enough parameters");
        return;
    }

    for (size_t i = 0; i < users.size(); ++i)
    {
        std::string channelName = (channels.size() == 1) ? channels[0] : channels[i];
        std::string targetUser = users[i];

        if (!haschannel(channelName))
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
            continue;
        }

        Channel* channel = findChannel(this, channelName);
        if (channel == NULL)
            continue;
        
        size_t channelIndex = 0;
        for (size_t j = 0; j < _channels.size(); ++j)
        {
            if (_channels[j].getName() == channelName)
            {
                channelIndex = j;
                break;
            }
        }

        if (!checkUserInChannel(this, msg, cli, channel, channelName))
            continue;

        if (!checkChannelOperator(this, msg, cli, channel, channelName))
            continue;

        size_t targetFd = 0;
        Client* targetClient = findClientByNick(this, targetUser, targetFd);
        
        if (targetFd == msg.fd)
        {
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You cannot kick yourself");
            continue;
        }

        if (targetClient == NULL)
        {
            sendReply(msg.fd, ":server 401 " + nick + " " + targetUser + " :No such nick/channel");
            continue;
        }

        if (!channel->hasUser(targetFd))
        {
            sendReply(msg.fd, ":server 441 " + nick + " " + targetUser + " " + channelName + " :They aren't on that channel");
            continue;
        }

        std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
        std::string kickMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname +
                              " KICK " + channelName + " " + targetUser + " :" + reason + "\r\n";
        
        broadcastToChannel(this, channel, kickMsg, 0);

        channel->removeUser(targetFd);
        
        if (channel->isOperator(targetFd))
            channel->removeOperator(targetFd);
        
        std::map<size_t, Client>::iterator targetIt = _clients.find(targetFd);
        if (targetIt != _clients.end())
            targetIt->second.leaveChannel(channelName);
        if (channel->getMembers().empty())
            _channels.erase(_channels.begin() + channelIndex);
    }
}
