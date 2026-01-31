#include "../../inc/client.hpp"
#include "../../inc/server.hpp"

// RFC 2812 - NOTICE command
// Syntax: NOTICE <target>{,<target>} :<message>
// NOTICE is similar to PRIVMSG but MUST NOT generate automatic replies
// No numeric replies are sent for NOTICE command errors
void Server::noticeCommand(IRCMessage &msg)
{
	Channel	*channel;
	size_t	targetFd;
	bool	userFound;

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
			channel = NULL;
			for (size_t j = 0; j < _channels.size(); ++j)
			{
				if (_channels[j].getName() == target)
				{
					channel = &_channels[j];
					break ;
				}
			}
			if (channel == NULL)
				continue ;
			if (!channel->hasUser(msg.fd))
				continue ;
			std::string noticeMsg = ":" + nick + "!" + cli.getUsername() + "@"
				+ cli.getHostname() + " NOTICE " + target + " :" + message
				+ "\r\n";
			std::vector<size_t> members = channel->getMembers();
			for (size_t m = 0; m < members.size(); ++m)
			{
				if (members[m] != msg.fd)
					send(members[m], noticeMsg.c_str(), noticeMsg.length(), 0);
			}
		}
		else
		{
			targetFd = 0;
			userFound = false;
			std::map<size_t, Client>::iterator it;
			for (it = _clients.begin(); it != _clients.end(); ++it)
			{
				if (it->second.getNickname() == target)
				{
					targetFd = it->first;
					userFound = true;
					break ;
				}
			}
			if (!userFound)
				continue ;

			std::string noticeMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() + " NOTICE " + target + " :" + message + "\r\n";
			send(targetFd, noticeMsg.c_str(), noticeMsg.length(), 0);
		}
	}
}