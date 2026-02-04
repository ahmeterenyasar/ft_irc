/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:39 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:45 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

void Server::cmdUser(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!cli.isAuthenticated())
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered");
        return;
    }

    if (msg.Parameters.size() < 4)
    {
        sendReply(msg.fd, ":server 461 " + nick + " USER :Not enough parameters");
        return;
    }

    if (cli.isRegistered())
    {
        sendReply(msg.fd, ":server 462 " + nick + " :You may not reregister");
        return;
    }

    cli.setUsername(msg.Parameters[0]);
    cli.setRealname(msg.Parameters[3]);
    
    if (!cli.getNickname().empty() && !cli.getUsername().empty())
    {
        cli.setRegistered(true);
        
        std::string finalNick = cli.getNickname();
        std::string finalUser = cli.getUsername();
        std::string host = cli.getHostname().empty() ? "localhost" : cli.getHostname();

        sendReply(msg.fd, ":server 001 " + finalNick + " :Welcome to the Internet Relay Network " + finalNick + "!" + finalUser + "@" + host);
        sendReply(msg.fd, ":server 002 " + finalNick + " :Your host is ft_irc, running version 1.0");
        sendReply(msg.fd, ":server 003 " + finalNick + " :This server was created " + std::string(__DATE__));
        sendReply(msg.fd, ":server 004 " + finalNick + " ft_irc 1.0 o o");
    }
}