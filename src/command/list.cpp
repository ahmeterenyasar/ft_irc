#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

/**
 * LIST - List channels and their topics
 * 
 * RFC 2812 - LIST command
 * Syntax: LIST [<channel>{,<channel>}]
 * 
 * The list command is used to list channels and their topics.
 * If the <channel> parameter is used, only the status of that channel is displayed.
 * 
 * Numeric replies:
 * - RPL_LIST (322): :<server> 322 <nick> <channel> <# visible> :<topic>
 * - RPL_LISTEND (323): :<server> 323 <nick> :End of LIST
 */
void Server::listCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkRegistered(this, msg, cli))
        return;

    // RPL_LISTSTART (321) - KvIRC needs this header
    sendReply(msg.fd, ":server 321 " + nick + " Channel :Users  Name");

    // If no parameters, list all channels
    if (msg.Parameters.empty())
    {
        // List all channels
        for (size_t i = 0; i < _channels.size(); ++i)
        {
            Channel& channel = _channels[i];
            std::string channelName = channel.getName();
            std::string topic = channel.getTopic();
            int userCount = channel.getUserCount();

            // Convert user count to string
            std::stringstream ss;
            ss << userCount;
            std::string userCountStr = ss.str();

            // RPL_LIST (322)
            // Format: :<server> 322 <nick> <channel> <# visible> :<topic>
            sendReply(msg.fd, ":server 322 " + nick + " " + channelName + " " + userCountStr + " :" + topic);
        }

        // RPL_LISTEND (323)
        sendReply(msg.fd, ":server 323 " + nick + " :End of LIST");
        return;
    }

    // Parse channel list (comma-separated)
    std::string channelList = msg.Parameters[0];
    std::vector<std::string> channels = split(channelList, ',');

    // List specific channels
    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channelName = channels[i];
        
        if (channelName.empty())
            continue;

        // Find the channel
        Channel* channel = findChannel(this, channelName);
        
        if (channel != NULL)
        {
            std::string topic = channel->getTopic();
            int userCount = channel->getUserCount();

            // Convert user count to string
            std::stringstream ss;
            ss << userCount;
            std::string userCountStr = ss.str();

            // RPL_LIST (322)
            sendReply(msg.fd, ":server 322 " + nick + " " + channelName + " " + userCountStr + " :" + topic);
        }
    }

    // RPL_LISTEND (323)
    sendReply(msg.fd, ":server 323 " + nick + " :End of LIST");
}