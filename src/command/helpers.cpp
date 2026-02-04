/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:42:47 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:42:48 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"
#include <set>


bool checkRegistered(Server* server, IRCMessage& msg, const Client& cli)
{
    if (!cli.isRegistered())
    {
        std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
        server->sendReply(msg.fd, ":server 451 " + nick + " :You have not registered");
        return false;
    }
    return true;
}

bool checkMinParams(Server* server, IRCMessage& msg, const Client& cli, size_t minParams, const std::string& command)
{
    if (msg.Parameters.size() < minParams)
    {
        std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
        server->sendReply(msg.fd, ":server 461 " + nick + " " + command + " :Not enough parameters");
        return false;
    }
    return true;
}

bool checkChannelOperator(Server* server, IRCMessage& msg, const Client& cli, Channel* channel, const std::string& channelName)
{
    if (!channel->isOperator(msg.fd))
    {
        std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
        server->sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator");
        return false;
    }
    return true;
}

bool checkUserInChannel(Server* server, IRCMessage& msg, const Client& cli, Channel* channel, const std::string& channelName)
{
    if (!channel->hasUser(msg.fd))
    {
        std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
        server->sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :You're not on that channel");
        return false;
    }
    return true;
}


Client* findClientByNick(Server* server, const std::string& nickname, size_t& fd)
{
    std::map<size_t, Client>& clients = server->getClients();
    for (std::map<size_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == nickname)
        {
            fd = it->first;
            return &(it->second);
        }
    }
    fd = 0;
    return NULL;
}

Channel* findChannel(Server* server, const std::string& channelName)
{
    std::vector<Channel>& channels = server->getChannels();
    for (size_t i = 0; i < channels.size(); ++i)
    {
        if (channels[i].getName() == channelName)
            return &channels[i];
    }
    return NULL;
}


void broadcastToChannel(Server* server, Channel* channel, const std::string& message, size_t excludeFd)
{
    std::vector<size_t> members = channel->getMembers();
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (excludeFd == 0 || members[i] != excludeFd)
            server->sendReply(members[i], message);
    }
}

void broadcastToCommonChannels(Server* server, const Client& cli, const std::string& message, size_t senderFd)
{
    std::set<size_t> notifiedUsers;
    std::vector<std::string> channels = cli.getChannels();
    std::vector<Channel>& serverChannels = server->getChannels();
    
    for (size_t i = 0; i < channels.size(); ++i)
    {
        for (size_t j = 0; j < serverChannels.size(); ++j)
        {
            if (serverChannels[j].getName() == channels[i])
            {
                std::vector<size_t> members = serverChannels[j].getMembers();
                for (size_t m = 0; m < members.size(); ++m)
                {
                    if (members[m] != senderFd && notifiedUsers.find(members[m]) == notifiedUsers.end())
                    {
                        send(members[m], message.c_str(), message.length(), 0);
                        notifiedUsers.insert(members[m]);
                    }
                }
                break;
            }
        }
    }
}


void removeUserFromAllChannels(Server* server, size_t fd)
{
    std::map<size_t, Client>& clients = server->getClients();
    std::map<size_t, Client>::iterator clientIt = clients.find(fd);
    if (clientIt == clients.end())
        return;
    
    std::vector<std::string> channels = clientIt->second.getChannels();
    std::vector<Channel>& serverChannels = server->getChannels();
    
    for (size_t i = 0; i < channels.size(); ++i)
    {
        for (size_t j = 0; j < serverChannels.size(); ++j)
        {
            if (serverChannels[j].getName() == channels[i])
            {
                serverChannels[j].removeUser(fd);
                if (serverChannels[j].isOperator(fd))
                    serverChannels[j].removeOperator(fd);
                break;
            }
        }
    }
}

void cleanupEmptyChannels(Server* server)
{
    std::vector<Channel>& channels = server->getChannels();
    for (size_t i = 0; i < channels.size(); )
    {
        if (channels[i].getUserCount() == 0)
            channels.erase(channels.begin() + i);
        else
            ++i;
    }
}
