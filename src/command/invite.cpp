#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

// RFC 2812 - INVITE command
// Syntax: INVITE <nickname> <channel>
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHNICK (401),
//                  ERR_NOTONCHANNEL (442), ERR_USERONCHANNEL (443),
//                  ERR_CHANOPRIVSNEEDED (482), RPL_INVITING (341)
void Server::inviteCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    if (!checkRegistered(this, msg, cli))
        return;

    if (!checkMinParams(this, msg, cli, 2, "INVITE"))
        return;

    std::string targetNick = msg.Parameters[0];
    std::string channelName = msg.Parameters[1];
    
    // Kendini invite etmeye çalışıyor mu?
    if (targetNick == nick)
    {
        sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :Cannot invite yourself");
        return;
    }

    // 3. Hedef kullanıcıyı bul - ERR_NOSUCHNICK (401)
    size_t targetFd = 0;
    Client* targetClient = findClientByNick(this, targetNick, targetFd);

    if (targetClient == NULL)
    {
        sendReply(msg.fd, ":server 401 " + nick + " " + targetNick + " :No such nick/channel");
        return;
    }

    // Client& targetClient = _clients[targetFd];

    // 4. Kanal VARSA kontroller yap
    Channel* channel = findChannel(this, channelName);

    if (channel != NULL)
    {
        // 4a. Davet eden kanalda mı? - ERR_NOTONCHANNEL (442)
        if (!channel->hasUser(msg.fd))
        {
            sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :You're not on that channel");
            return;
        }

        // 4b. Hedef kullanıcı zaten kanalda mı? - ERR_USERONCHANNEL (443)
        if (channel->hasUser(targetFd))
        {
            sendReply(msg.fd, ":server 443 " + nick + " " + targetNick + " " + channelName + " :is already on channel");
            return;
        }

        // 4c. Kanal +i (invite-only) ise operator kontrolü - ERR_CHANOPRIVSNEEDED (482)
        if (channel->isInviteOnly() && !channel->isOperator(msg.fd))
        {
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator");
            return;
        }
    }

    // 5. Davet listesine ekle (kanal varsa)
    if (channel != NULL)
    {
        channel->addInvite(targetNick);
    }

    // 6. RPL_INVITING (341) - Davet edene onay
    sendReply(msg.fd, ":server 341 " + nick + " " + targetNick + " " + channelName);

    // 7. INVITE mesajı - Davet edilen kullanıcıya
    std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
    std::string inviteMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + 
                            " INVITE " + targetNick + " " + channelName + "\r\n";
    sendReply(targetFd, inviteMsg);

}
