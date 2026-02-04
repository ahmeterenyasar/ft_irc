#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

void Server::modeCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
    
    if (!checkMinParams(this, msg, cli, 1, "MODE"))
        return;
    
    if (!checkRegistered(this, msg, cli))
        return; 

    std::string target = msg.Parameters[0];
    if (target.empty() || (target[0] != '#' && target[0] != '&'))
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel");
        return;
    }
    if (!haschannel(target))
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel");
        return;
    }

    Channel* channel = findChannel(this, target);
    if (channel == NULL)
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel");
        return;
    }
    
    if (!checkUserInChannel(this, msg, cli, channel, target))
        return;
    
    if (msg.Parameters.size() == 1)
    {
        sendReply(msg.fd, ":server 324 " + nick + " " + target + " " + channel->getModeString());
        return;
    }
    
    if (!checkChannelOperator(this, msg, cli, channel, target))
        return;
    std::string modeString = msg.Parameters[1];
    bool adding = true;
    size_t paramIndex = 2;
    std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
    std::string prefix = ":" + nick + "!" + cli.getUsername() + "@" + hostname;
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
            std::string modeChange = prefix + " MODE " + target + " " + (adding ? "+i" : "-i") + "\r\n";
            broadcastToChannel(this, channel, modeChange, 0);
        }
        else if (mode == 't')
        {
            channel->setTopicRestricted(adding);
            std::string modeChange = prefix + " MODE " + target + " " + (adding ? "+t" : "-t") + "\r\n";
            broadcastToChannel(this, channel, modeChange, 0);
        }
        else if (mode == 'k')
        {
            if (adding)
            {
                if (msg.Parameters.size() <= paramIndex)
                {
                    sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters");
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
                sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters");
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
                sendReply(msg.fd, ":server 401 " + nick + " " + targetNick + " :No such nick/channel");
                continue;
            }
            if (!channel->hasUser((size_t)targetFd))
            {
                sendReply(msg.fd, ":server 441 " + nick + " " + targetNick + " " + target + " :They aren't on that channel");
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
                std::vector<size_t> operators = channel->getOperators();
                if (operators.size() == 1 && channel->isOperator((size_t)targetFd))
                {
                    sendReply(msg.fd, ":server 482 " + nick + " " + target + " :Cannot remove last operator");
                    continue;
                }
                
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
                    sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters");
                    continue;
                }
                int limit = atoi(msg.Parameters[paramIndex++].c_str());
                if (limit <= 0)
                {
                    sendReply(msg.fd, ":server 696 " + nick + " " + target + " l * :Invalid user limit. Must be a positive integer");
                    continue;
                }
                if (limit > 1000)
                {
                    sendReply(msg.fd, ":server 696 " + nick + " " + target + " l * :User limit too large (max: 1000)");
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
    }
}
