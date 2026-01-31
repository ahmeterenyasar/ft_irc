#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - INVITE command
// Syntax: INVITE <nickname> <channel>
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHNICK (401),
//                  ERR_NOTONCHANNEL (442), ERR_USERONCHANNEL (443),
//                  ERR_CHANOPRIVSNEEDED (482), RPL_INVITING (341)
void Server::inviteCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    // 1. Kayıt kontrolü
    if (!cli.isRegistered())
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered\r\n");
        return;
    }

    // 2. Parametre kontrolü - ERR_NEEDMOREPARAMS (461)
    if (msg.Parameters.size() < 2)
    {
        sendReply(msg.fd, ":server 461 " + nick + " INVITE :Not enough parameters\r\n");
        return;
    }

    std::string targetNick = msg.Parameters[0];
    std::string channelName = msg.Parameters[1];

    // 3. Hedef kullanıcıyı bul - ERR_NOSUCHNICK (401)
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
        return;
    }

    // Client& targetClient = _clients[targetFd];

    // 4. Kanal VARSA kontroller yap
    Channel* channel = NULL;
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i].getName() == channelName)
        {
            channel = &_channels[i];
            break;
        }
    }

    if (channel != NULL)
    {
        // 4a. Davet eden kanalda mı? - ERR_NOTONCHANNEL (442)
        if (!channel->hasUser(msg.fd))
        {
            sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
            return;
        }

        // 4b. Hedef kullanıcı zaten kanalda mı? - ERR_USERONCHANNEL (443)
        if (channel->hasUser(targetFd))
        {
            sendReply(msg.fd, ":server 443 " + nick + " " + targetNick + " " + channelName + " :is already on channel\r\n");
            return;
        }

        // 4c. Kanal +i (invite-only) ise operator kontrolü - ERR_CHANOPRIVSNEEDED (482)
        if (channel->isInviteOnly() && !channel->isOperator(msg.fd))
        {
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator\r\n");
            return;
        }
    }

    // 5. Davet listesine ekle (kanal varsa)
    if (channel != NULL)
    {
        channel->addInvite(targetNick);
    }

    // 6. RPL_INVITING (341) - Davet edene onay
    sendReply(msg.fd, ":server 341 " + nick + " " + targetNick + " " + channelName + "\r\n");

    // 7. INVITE mesajı - Davet edilen kullanıcıya
    std::string inviteMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() + 
                            " INVITE " + targetNick + " " + channelName + "\r\n";
    sendReply(targetFd, inviteMsg);

}
