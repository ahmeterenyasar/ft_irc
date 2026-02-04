/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:22 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:23 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

void Server::passCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string userName;

    if (cli.getUsername().empty())
        userName = "*";
    else
        userName = cli.getUsername();

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + userName + " PASS :Not enough parameters");
        return;
    }
    if (cli.isRegistered() || cli.isAuthenticated() == true)
    {
        sendReply(msg.fd, ":server 462 " + userName + " :You may not reregister");
        return;
    }
    if (msg.Parameters[0] != this->_password)
    {
        sendReply(msg.fd, ":server 464 " + userName + " :Password incorrect");
        cli.setAuthenticated(false);
        return;
    }
    cli.setAuthenticated(true);
}
