/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:42:43 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:42:44 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

void Server::sendReply(int fd, const std::string& reply)
{
    std::string message = reply + "\r\n";
    send(fd, message.c_str(), message.length(), 0);
}

void Server::executeCommand(IRCMessage& msg)
{
    std::string cmd = msg.Command;
    
    if (cmd.empty())
        return;
    if (_clients.find(msg.fd) == _clients.end())
        return;
    for (size_t i = 0; i < cmd.length(); i++)
        cmd[i] = std::toupper(cmd[i]);

    if (cmd == "PASS")
        passCommand(msg);
    else if (cmd == "NICK")
        nickCommand(msg);
    else if (cmd == "USER")
        cmdUser(msg);
    else if (cmd == "QUIT")
        quitCommand(msg);
    else if (cmd == "CAP")
        capCommand(msg);
    else if (cmd == "PING")
        pingCommand(msg);
    else if (cmd == "WHO")
        whoCommand(msg);
    else if (cmd == "LIST")
        listCommand(msg);
    else if (cmd == "JOIN")
        joinCommand(msg);
    else if (cmd == "PART")
        partCommand(msg);
    else if (cmd == "PRIVMSG")
        privmsgCommand(msg);
    else if (cmd == "NOTICE")
        noticeCommand(msg);
    else if (cmd == "KICK")
        kickCommand(msg);
    else if (cmd == "INVITE")
        inviteCommand(msg);
    else if (cmd == "TOPIC")
        topicCommand(msg);
    else if (cmd == "MODE")
        modeCommand(msg);
    else
    {
        Client& cli = _clients[msg.fd];
        std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
        sendReply(msg.fd, ":server 421 " + nick + " " + msg.Command + " :Unknown command");
    }
}
