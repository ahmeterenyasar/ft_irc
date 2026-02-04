/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:24 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:25 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

void Server::pingCommand(IRCMessage& msg)
{
    if (msg.Parameters.empty())
    {
        Client& cli = _clients[msg.fd];
        std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
        sendReply(msg.fd, ":server 409 " + nick + " :No origin specified");
        return;
    }

    std::string token = msg.Parameters[0];
    sendReply(msg.fd, ":server PONG server :" + token);
}
