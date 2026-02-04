/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   who.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:36 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:42 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

void Server::whoCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkRegistered(this, msg, cli))
        return;

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 315 " + nick + " * :End of WHO list");
        return;
    }

    std::string mask = msg.Parameters[0];
    bool operatorsOnly = (msg.Parameters.size() > 1 && msg.Parameters[1] == "o");

    if (mask[0] == '#' || mask[0] == '&')
    {
        Channel* channel = findChannel(this, mask);
        
        if (channel == NULL)
        {
            sendReply(msg.fd, ":server 315 " + nick + " " + mask + " :End of WHO list");
            return;
        }

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
                memberReal = memberNick;
            std::string memberHost = member.getHostname().empty() ? "localhost" : member.getHostname();

            if (operatorsOnly && !channel->isOperator(memberFd))
                continue;

            std::string flags = "H";
            if (channel->isOperator(memberFd))
                flags += "@";

            std::string whoReply = ":server 352 " + nick + " " + mask + " " 
                                 + memberUser + " " + memberHost + " server " + memberNick + " " 
                                 + flags + " :0 " + memberReal;
            sendReply(msg.fd, whoReply);
        }

        sendReply(msg.fd, ":server 315 " + nick + " " + mask + " :End of WHO list");
    }
    else
    {
        size_t targetFd = 0;
        Client* targetClient = findClientByNick(this, mask, targetFd);

        if (targetClient != NULL)
        {
            std::string targetUser = targetClient->getUsername();
            std::string targetReal = targetClient->getRealname();
            if (targetReal.empty())
                targetReal = mask;
            std::string targetHost = targetClient->getHostname().empty() ? "localhost" : targetClient->getHostname();

            std::string whoReply = ":server 352 " + nick + " * " 
                                 + targetUser + " " + targetHost + " server " + mask + " H :0 " + targetReal;
            sendReply(msg.fd, whoReply);
        }

        sendReply(msg.fd, ":server 315 " + nick + " " + mask + " :End of WHO list");
    }
}
