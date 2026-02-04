/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:15 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:16 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"
#include <set>

static bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;
    
    char first = nick[0];
    if (!std::isalpha(first) && first != '[' && first != ']' && 
        first != '\\' && first != '`' && first != '_' && 
        first != '^' && first != '{' && first != '|' && first != '}')
        return false;
    
    for (size_t i = 1; i < nick.length(); i++)
    {
        char c = nick[i];
        if (!std::isalnum(c) && c != '[' && c != ']' && 
            c != '\\' && c != '`' && c != '_' && 
            c != '^' && c != '{' && c != '|' && c != '}' && c != '-')
            return false;
    }
    return true;
}

void Server::nickCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 431 * :No nickname given");
        return;
    }
    std::string newNick = msg.Parameters[0];
    if (!isValidNickname(newNick))
    {
        std::string target = cli.getNickname().empty() ? "*" : cli.getNickname();
        sendReply(msg.fd, ":server 432 " + target + " " + newNick + " :Erroneous nickname");
        return;
    }
    for (std::map<size_t, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == newNick)
        {
            std::string target = cli.getNickname().empty() ? "*" : cli.getNickname();
            sendReply(msg.fd, ":server 433 " + target + " " + newNick + " :Nickname is already in use");
            return;
        }
    }
    if (cli.isRegistered())
    {
        std::string oldNick = cli.getNickname();
        std::string oldUser = cli.getUsername();
        std::string oldHost = cli.getHostname();

        std::string msgToSend = ":" + oldNick + "!" + oldUser + "@" + oldHost + " NICK :" + newNick + "\r\n";
        
        send(msg.fd, msgToSend.c_str(), msgToSend.length(), 0);
        
        broadcastToCommonChannels(this, cli, msgToSend, msg.fd);
    }
    cli.setNickname(newNick);
    if (cli.isAuthenticated() && !cli.getUsername().empty() && 
        !cli.getNickname().empty() && !cli.isRegistered())
    {
        cli.setRegistered(true);
        
        std::string host = cli.getHostname().empty() ? "localhost" : cli.getHostname();

        sendReply(msg.fd, ":server 001 " + newNick + " :Welcome to the Internet Relay Network " + newNick + "!" + cli.getUsername() + "@" + host);
        sendReply(msg.fd, ":server 002 " + newNick + " :Your host is server, running version 1.0");
        sendReply(msg.fd, ":server 003 " + newNick + " :This server was created " + std::string(__DATE__));
        sendReply(msg.fd, ":server 004 " + newNick + " server 1.0 o o");
    }
}
