#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - KICK command
// Syntax: KICK <channel> <user> [<comment>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_CHANOPRIVSNEEDED (482), ERR_USERNOTINCHANNEL (441),
//                  ERR_NOTONCHANNEL (442)
void Server::kickCommand(IRCMessage& msg)
{
    // Client'ın var olup olmadığını kontrol et
    std::map<int, Client>::iterator clientIt = _clients.find(msg.fd);
    if (clientIt == _clients.end())
        return;
    
    Client& cli = clientIt->second;
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    // En az 2 parametre gerekli: kanal ve kullanıcı
    if (msg.Parameters.size() < 2)
    {
        sendReply(msg.fd, ":server 461 " + nick + " KICK :Not enough parameters");
        return;
    }

    std::string channelName = msg.Parameters[0];
    std::string targetNick = msg.Parameters[1];
    std::string reason = "No reason given";
    
    // Kick sebebi varsa al
    if (msg.Parameters.size() >= 3)
        reason = msg.Parameters[2];

    // Kanal var mı kontrol et
    if (_channels.find(channelName) == _channels.end())
    {
        sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
        return;
    }

    Channel& channel = _channels[channelName];

    // Kick yapan kullanıcı kanalda mı?
    if (!channel.hasUser(msg.fd))
    {
        sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :You're not on that channel");
        return;
    }

    // Kick yapan kullanıcı operator mı?
    if (!cli.isOperator(channelName))
    {
        sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator");
        return;
    }

    // Hedef kullanıcıyı bul
    int targetFd = -1;
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == targetNick)
        {
            targetFd = it->first;
            break;
        }
    }

    // Hedef kullanıcı bulunamadı
    if (targetFd == -1)
    {
        sendReply(msg.fd, ":server 441 " + nick + " " + targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    // Hedef kullanıcı kanalda değil
    if (!channel.hasUser(targetFd))
    {
        sendReply(msg.fd, ":server 441 " + nick + " " + targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    // Hedef client'ı al
    std::map<int, Client>::iterator targetIt = _clients.find(targetFd);
    if (targetIt == _clients.end())
        return;
    
    Client& targetCli = targetIt->second;

    // KICK mesajı
    std::string kickMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() + 
                          " KICK " + channelName + " " + targetNick + " :" + reason;

    // tüm üyelere KICK mesajını gönder
    std::vector<int> members = channel.getMembers();
    for (size_t i = 0; i < members.size(); ++i)
    {
        sendReply(members[i], kickMsg);
    }

    // Hedef kullanıcıyı kanaldan çıkar
    channel.removeUser(targetFd);
    targetCli.leaveChannel(channelName);
    targetCli.setOperator(channelName, false);

    // Kanal boşaldıysa kanalı sil
    if (channel.getUserCount() == 0)
    {
        _channels.erase(channelName);
    }
}
