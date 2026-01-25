#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - JOIN command
// Syntax: JOIN <channel>{,<channel>} [<key>{,<key>}]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_TOOMANYCHANNELS (405), ERR_BADCHANNELKEY (475),
//                  ERR_BANNEDFROMCHAN (474), ERR_INVITEONLYCHAN (473),
//                  ERR_CHANNELISFULL (471), RPL_TOPIC (332),
//                  RPL_NAMREPLY (353), RPL_ENDOFNAMES (366)
void Server::joinCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!cli.isRegistered()) 
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered\r\n");
        return;
    }

    if (msg.Parameters.empty()) 
    {
        sendReply(msg.fd, ":server 461 " + nick + " JOIN :Not enough parameters\r\n");
        return;
    }

    std::vector<std::string> chans = split(msg.Parameters[0], ',');
    std::vector<std::string> keys;
    if (msg.Parameters.size() > 1)
        keys = split(msg.Parameters[1], ',');

    for (size_t i = 0; i < chans.size(); ++i)
    {
        std::string chName = chans[i];
        std::string key = (i < keys.size()) ? keys[i] : "";

        if (chName.empty() || chName[0] != '#') 
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + chName + " :No such channel\r\n");
            continue;
        }

        if (_channels.find(chName) == _channels.end())
            _channels[chName] = Channel(chName);

        Channel& ch = _channels[chName];
        if (ch.hasKey() && ch.getKey() != key) {
            sendReply(msg.fd, ":server 475 " + nick + " " + chName + " :Cannot join channel (+k)\r\n");
            continue;
        }
        if (ch.isFull()) {
            sendReply(msg.fd, ":server 471 " + nick + " " + chName + " :Cannot join channel (+l)\r\n");
            continue;
        }
        if (ch.isInviteOnly() && !ch.isInvited(msg.fd)) {
            sendReply(msg.fd, ":server 473 " + nick + " " + chName + " :Cannot join channel (+i)\r\n");
            continue;
        }
        if (ch.isBanned(msg.fd)) {
            sendReply(msg.fd, ":server 474 " + nick + " " + chName + " :Cannot join channel (+b)\r\n");
            continue;
        }
        if (ch.hasMember(msg.fd))
            continue;
        ch.addMember(msg.fd);
        std::string joinMsg = ":" + cli.getNickname() + "!~" + cli.getUsername() + "@localhost JOIN :" + chName + "\r\n";
        const std::vector<int>& members = ch.getMembers();
        for (size_t j = 0; j < members.size(); ++j)
            sendReply(members[j], joinMsg);

        if (ch.getTopic().empty())
            sendReply(msg.fd, ":server 331 " + cli.getNickname() + " " + chName + " :No topic is set\r\n");
        else
            sendReply(msg.fd, ":server 332 " + cli.getNickname() + " " + chName + " :" + ch.getTopic() + "\r\n");

        std::string userList = getUserList(ch);
        sendReply(msg.fd, ":server 353 " + cli.getNickname() + " = " + chName + " :" + userList + "\r\n");
        sendReply(msg.fd, ":server 366 " + cli.getNickname() + " " + chName + " :End of /NAMES list\r\n");
    }
}



