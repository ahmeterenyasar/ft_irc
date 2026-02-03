#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"



// RFC 2812 - PART command
// Syntax: PART <channel>{,<channel>} [<part message>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_NOTONCHANNEL (442)
void Server::partCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkMinParams(this, msg, cli, 1, "PART"))
        return;
    
    if (!checkRegistered(this, msg, cli))
        return;
    
    std::vector<std::string> channels = split(msg.Parameters[0], ',');
    std::string partMessage = "";
    if (msg.Parameters.size() > 1)
    {
        partMessage = msg.Parameters[1];
    }
    
    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channelName = channels[i];
        
        if (channelName.empty() || channelName[0] != '#')
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
            continue;
        }
        
        if (!haschannel(channelName))
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
            continue;
        }
        
        Channel* channel = findChannel(this, channelName);
        
        if (channel == NULL)
            continue;
        
        if (!checkUserInChannel(this, msg, cli, channel, channelName))
            continue;
        
        std::vector<size_t> members = channel->getMembers();
        
        std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
        std::string partMsg;
        if (!partMessage.empty())
            partMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " PART " + channelName + " :" + partMessage + "\r\n";
        else
            partMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " PART " + channelName + "\r\n";
        
        for (size_t m = 0; m < members.size(); ++m)
        {
            send(members[m], partMsg.c_str(), partMsg.length(), 0);
        }
        
        channel->removeUser(msg.fd);
        if (channel->isOperator(msg.fd))
            channel->removeOperator(msg.fd);
        cli.leaveChannel(channelName);
        
        if (channel->getUserCount() == 0)
        {
            for (size_t k = 0; k < _channels.size(); ++k)
            {
                if (_channels[k].getName() == channelName)
                {
                    _channels.erase(_channels.begin() + k);
                    break;
                }
            }
        }
    }
}
