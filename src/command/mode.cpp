#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - MODE command
// Channel modes: MODE <channel> [<modestring> [<mode arguments>...]]
// User modes: MODE <nickname> [<modestring>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_CHANOPRIVSNEEDED (482),
//                  ERR_NOSUCHNICK (401), ERR_NOSUCHCHANNEL (403),
//                  ERR_UNKNOWNMODE (472), RPL_CHANNELMODEIS (324),
//                  RPL_UMODEIS (221)
void Server::modeCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick;
    if (cli.getNickname().empty())
        nick = "*";
    else
        nick = cli.getNickname();
    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters\r\n");
        return;
    }
    if(cli.isRegistered() == false)
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered\r\n");
        return;
    } 

    std::string target = msg.Parameters[0];
    if (target.empty() || (target[0] != '#' && target[0] != '&'))
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel\r\n");
        return;
    }
    if (!haschannel(target))
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel\r\n");
        return;
    }

    Channel* channel = NULL;
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i].getName() == target)
        {
            channel = &_channels[i];
            break;
        }
    }
    if (channel == NULL)
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel\r\n");
        return;
    }
    if (!channel->hasUser(msg.fd))
    {
        sendReply(msg.fd, ":server 442 " + nick + " " + target + " :You're not on that channel\r\n");
        return;
    }
    if (msg.Parameters.size() == 1)
    {
        sendReply(msg.fd, ":server 324 " + nick + " " + target + " " + channel->getModeString() + "\r\n");
        return;
    }
    if (!channel->isOperator(msg.fd))
    {
        sendReply(msg.fd, ":server 482 " + nick + " " + target + " :You're not channel operator\r\n");
        return;
    }
    std::string modeString = msg.Parameters[1];
    bool adding = true;
    size_t paramIndex = 2;
    std::string prefix = ":" + nick + "!~" + cli.getUsername() + "@localhost";
    for (size_t i = 0; i < modeString.size(); i++)
    {
        char mode = modeString[i];
        if (mode == '+')
            adding = true;
        else if (mode == '-')
            adding = false;
        else if (mode == 'i')
        {
            channel->setInviteOnly(adding);
            std::string modeChange = prefix + " MODE " + target + " ";
            if (adding)
                modeChange += "+i\r\n";
            else
                modeChange += "-i\r\n";
            std::vector<size_t> members = channel->getMembers();
            for (size_t j = 0; j < members.size(); j++)
                sendReply(members[j], modeChange);
        }
        else if (mode == 't')
        {
            channel->setTopicRestricted(adding);
            std::string modeChange = prefix + " MODE " + target + " ";
            if (adding)
                modeChange += "+t\r\n";
            else
                modeChange += "-t\r\n";
            std::vector<size_t> members = channel->getMembers();
            for (size_t j = 0; j < members.size(); j++)
                sendReply(members[j], modeChange);
        }
        else if (mode == 'k')
        {
            if (adding)
            {
                if (msg.Parameters.size() <= paramIndex)
                {
                    sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters\r\n");
                    continue;
                }
                std::string key = msg.Parameters[paramIndex++];
                channel->setKey(key);
                std::string modeChange = prefix + " MODE " + target + " +k " + key + "\r\n";
                std::vector<size_t> members = channel->getMembers();
                for (size_t j = 0; j < members.size(); j++)
                    sendReply(members[j], modeChange);
            }
            else
            {
                channel->setKey("");
                std::string modeChange = prefix + " MODE " + target + " -k\r\n";
                std::vector<size_t> members = channel->getMembers();
                for (size_t j = 0; j < members.size(); j++)
                    sendReply(members[j], modeChange);
            }
        }
        else if (mode == 'o')
        {
            if (msg.Parameters.size() <= paramIndex)
            {
                sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters\r\n");
                continue;
            }
            std::string targetNick = msg.Parameters[paramIndex++];
            int targetFd = -1;
            for (std::map<size_t, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
            {
                if (it->second.getNickname() == targetNick)
                {
                    targetFd = it->first;
                    break;
                }
            }
            if (targetFd == -1)
            {
                sendReply(msg.fd, ":server 401 " + nick + " " + targetNick + " :No such nick/channel\r\n");
                continue;
            }
            if (!channel->hasUser((size_t)targetFd))
            {
                sendReply(msg.fd, ":server 441 " + nick + " " + targetNick + " " + target + " :They aren't on that channel\r\n");
                continue;
            }
            Client& targetClient = _clients[(size_t)targetFd];
            if (adding)
            {
                channel->addOperator((size_t)targetFd);
                targetClient.setOperator(target, true);
                std::string modeChange = prefix + " MODE " + target + " +o " + targetNick + "\r\n";
                std::vector<size_t> members = channel->getMembers();
                for (size_t j = 0; j < members.size(); j++)
                    sendReply(members[j], modeChange);
            }
            else
            {
                channel->removeOperator((size_t)targetFd);
                targetClient.setOperator(target, false);
                std::string modeChange = prefix + " MODE " + target + " -o " + targetNick + "\r\n";
                std::vector<size_t> members = channel->getMembers();
                for (size_t j = 0; j < members.size(); j++)
                    sendReply(members[j], modeChange);
            }
        }
        else if (mode == 'l')
        {
            if (adding)
            {
                if (msg.Parameters.size() <= paramIndex)
                {
                    sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters\r\n");
                    continue;
                }
                int limit = atoi(msg.Parameters[paramIndex++].c_str());
                if (limit <= 0)
                {
                    sendReply(msg.fd, ":server 461 " + nick + " MODE :Invalid limit parameter\r\n");
                    continue;
                }
                channel->setUserLimit(limit);
                std::string modeChange = prefix + " MODE " + target + " +l " + msg.Parameters[paramIndex - 1] + "\r\n";
                std::vector<size_t> members = channel->getMembers();
                for (size_t j = 0; j < members.size(); j++)
                    sendReply(members[j], modeChange);
            }
            else
            {
                channel->setUserLimit(0);
                std::string modeChange = prefix + " MODE " + target + " -l\r\n";
                std::vector<size_t> members = channel->getMembers();
                for (size_t j = 0; j < members.size(); j++)
                    sendReply(members[j], modeChange);
            }
        }
        else
            sendReply(msg.fd, ":server 472 " + nick + " " + std::string(1, mode) + " :is unknown mode char to me\r\n");
    }
}
