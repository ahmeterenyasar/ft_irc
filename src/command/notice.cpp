/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   notice.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:17 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:18 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/client.hpp"
#include "../../inc/server.hpp"
#include "../../inc/command_helpers.hpp"

void Server::noticeCommand(IRCMessage &msg)
{
	Channel	*channel;
	size_t	targetFd;

	Client &cli = _clients[msg.fd];
	std::string nick;
	if (cli.getNickname().empty())
		nick = "*";
	else
		nick = cli.getNickname();
	if (msg.Parameters.empty() || msg.Parameters.size() < 2)
		return ;
	if (!cli.isRegistered())
		return ;

	std::string targets = msg.Parameters[0];
	std::string message = msg.Parameters[1];
	std::vector<std::string> targetList = split(targets, ',');

	for (size_t i = 0; i < targetList.size(); ++i)
	{
		std::string target = targetList[i];
		if (target.empty())
			continue ;
		if (target[0] == '#')
		{
			if (!haschannel(target))
				continue ;
			channel = findChannel(this, target);
			if (channel == NULL)
				continue ;
			if (!channel->hasUser(msg.fd))
				continue ;
			std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
			std::string noticeMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " NOTICE " + target + " :" + message + "\r\n";
			broadcastToChannel(this, channel, noticeMsg, msg.fd);
		}
		else
		{
			Client* targetClient = findClientByNick(this, target, targetFd);
			if (!targetClient)
				continue ;

			std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
			std::string noticeMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " NOTICE " + target + " :" + message + "\r\n";
			send(targetFd, noticeMsg.c_str(), noticeMsg.length(), 0);
		}
	}
}