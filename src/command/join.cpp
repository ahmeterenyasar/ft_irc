#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"


// RFC 2812 - JOIN command
// Syntax: JOIN <channel>{,<channel>} [<key>{,<key>}]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_TOOMANYCHANNELS (405), ERR_BADCHANNELKEY (475),
//                  ERR_BANNEDFROMCHAN (474), ERR_INVITEONLYCHAN (473),
//                  ERR_CHANNELISFULL (471), RPL_TOPIC (332),
//                  RPL_NAMREPLY (353), RPL_ENDOFNAMES (366)
std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) 
    {
        tokens.push_back(token);
    }
    return tokens;
}

void Server::joinCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick;

    if(cli.getNickname().empty())
        nick = "*";
    else
        nick = cli.getNickname();

    if (!cli.isRegistered()) 
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered");
        return;
    }
    
    if (msg.Parameters.empty()) 
    {
        sendReply(msg.fd, ":server 461 " + nick + " JOIN :Not enough parameters");
        return;
    }

    if (msg.Parameters[0] == "0")
    {
        std::vector<std::string> userChannels = cli.getChannels();
        for (size_t i = 0; i < userChannels.size(); ++i)
        {
            msg.Parameters[0] = userChannels[i];
            partCommand(msg);
        }
        return;
    }
    
    std::vector<std::string> jchannels = split(msg.Parameters[0], ',');
    std::vector<std::string> keys;
    if (msg.Parameters.size() > 1)
        keys = split(msg.Parameters[1], ',');
    
    for(size_t i = 0; i < jchannels.size(); ++i)
    {
        std::string channelName = jchannels[i];
        if (channelName.empty() || channelName[0] != '#')
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
            continue;
        }
        
        if (channelName.length() > 50)
        {
            sendReply(msg.fd, ":server 479 " + nick + " " + channelName + " :Illegal channel name (too long)");
            continue;
        }
        
        bool validName = true;
        for (size_t j = 1; j < channelName.length(); ++j)
        {
            char c = channelName[j];
            if (c == ' ' || c == ',' || c == '\r' || c == '\n' || c == '\0' || c == 7)
            {
                validName = false;
                break;
            }
        }
        if (!validName)
        {
            sendReply(msg.fd, ":server 479 " + nick + " " + channelName + " :Illegal channel name");
            continue;
        }
        
        else if (haschannel(channelName))
        {
            Channel* channel = NULL;
            for (size_t j = 0; j < _channels.size(); ++j)
            {
                if (_channels[j].getName() == channelName)
                {
                    channel = &_channels[j];
                    break;
                }
            }
            
            if (channel->hasUser(msg.fd))
            {
                continue;
            }
            
            if (cli.getChannels().size() >= 10)
            {
                sendReply(msg.fd, ":server 405 " + nick + " " + channelName + " :You have joined too many channels");
                continue;
            }
            
            std::string key = (i < keys.size()) ? keys[i] : "";
            if (!channel->getKey().empty() && channel->getKey() != key)
            {
                sendReply(msg.fd, ":server 475 " + nick + " " + channelName + " :Cannot join channel (+k)");
                continue;
            }
            
            if (channel->isInviteOnly() && !channel->isInvited(nick))
            {
                sendReply(msg.fd, ":server 473 " + nick + " " + channelName + " :Cannot join channel (+i)");
                continue;
            }
            
            if (channel->getUserLimit() > 0 && channel->getUserCount() >= channel->getUserLimit())
            {
                sendReply(msg.fd, ":server 471 " + nick + " " + channelName + " :Cannot join channel (+l)");
                continue;
            }
            
            channel->addUser(msg.fd);
            cli.joinChannel(channelName);
            
            if (channel->isInvited(nick))
                channel->removeInvite(nick);
            
            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string joinMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " JOIN " + channelName + "\r\n";
            std::vector<size_t> members = channel->getMembers();
            
            for (size_t m = 0; m < members.size(); ++m)
            {
                send(members[m], joinMsg.c_str(), joinMsg.length(), 0);
            }
            
            if (!channel->getTopic().empty())
            {
                sendReply(msg.fd, ":server 332 " + nick + " " + channelName + " :" + channel->getTopic());
            }
            else
            {
                sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set");
            }
            
            std::string userList = getUserList(*channel);
            sendReply(msg.fd, ":server 353 " + nick + " = " + channelName + " :" + userList);
            sendReply(msg.fd, ":server 366 " + nick + " " + channelName + " :End of /NAMES list");
        }
        else 
        {
            if (cli.getChannels().size() >= 10)
            {
                sendReply(msg.fd, ":server 405 " + nick + " " + channelName + " :You have joined too many channels");
                continue;
            }
            
            Channel newChannel(channelName);
            
            newChannel.addUser(msg.fd);
            newChannel.addOperator(msg.fd);
            
            std::string key = (i < keys.size()) ? keys[i] : "";
            if (!key.empty())
            {
                newChannel.setKey(key);
            }
            
            _channels.push_back(newChannel);
            
            cli.joinChannel(channelName);
            cli.setOperator(channelName, true);
            
            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string joinMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " JOIN " + channelName + "\r\n";
            
            send(msg.fd, joinMsg.c_str(), joinMsg.length(), 0);
            
            sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set");
            
            std::string userList = "@" + nick;
            sendReply(msg.fd, ":server 353 " + nick + " = " + channelName + " :" + userList);
            sendReply(msg.fd, ":server 366 " + nick + " " + channelName + " :End of /NAMES list");
        }
    }
}



