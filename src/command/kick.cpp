#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

// RFC 2812 - KICK command
// Syntax: KICK <channel> <user> [<comment>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_CHANOPRIVSNEEDED (482), ERR_USERNOTINCHANNEL (441),
//                  ERR_NOTONCHANNEL (442)
void Server::kickCommand(IRCMessage& msg)
{
    // Client'ın var olup olmadığını kontrol et
    std::map<size_t, Client>::iterator clientIt = _clients.find(msg.fd);
    if (clientIt == _clients.end())
        return;
    
    Client& cli = clientIt->second;
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    // Validation checks
    if (!checkMinParams(this, msg, cli, 2, "KICK"))
        return;
    
    if (!checkRegistered(this, msg, cli))
        return;

    // 3. Parametreleri al ve parse et
    std::vector<std::string> channels = split(msg.Parameters[0], ',');
    std::vector<std::string> users = split(msg.Parameters[1], ',');
    std::string reason = (msg.Parameters.size() >= 3) ? msg.Parameters[2] : nick;

    // 4. Kanal-User eşleşme kontrolü
    // Geçerli: 1 kanal + N user VEYA N kanal + N user
    if (channels.size() != 1 && channels.size() != users.size())
    {
        sendReply(msg.fd, ":server 461 " + nick + " KICK :Not enough parameters");
        return;
    }

    // 5. Her kick işlemi için
    for (size_t i = 0; i < users.size(); ++i)
    {
        // Kanal seçimi: tek kanal varsa hep o, yoksa index'e göre
        std::string channelName = (channels.size() == 1) ? channels[0] : channels[i];
        std::string targetUser = users[i];

        // 6. Kanal var mı kontrolü - ERR_NOSUCHCHANNEL (403)
        if (!haschannel(channelName))
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
            continue;
        }

        // 7. Kanalı bul
        Channel* channel = findChannel(this, channelName);
        if (channel == NULL)
            continue;
        
        size_t channelIndex = 0;
        for (size_t j = 0; j < _channels.size(); ++j)
        {
            if (_channels[j].getName() == channelName)
            {
                channelIndex = j;
                break;
            }
        }

        // 8. KICK eden kullanıcı kanalda mı? - ERR_NOTONCHANNEL (442)
        if (!checkUserInChannel(this, msg, cli, channel, channelName))
            continue;

        // 9. KICK eden kullanıcı operator mı? - ERR_CHANOPRIVSNEEDED (482)
        if (!checkChannelOperator(this, msg, cli, channel, channelName))
            continue;

        // 10. Atılacak kullanıcıyı bul
        size_t targetFd = 0;
        Client* targetClient = findClientByNick(this, targetUser, targetFd);
        
        // Kendini kicklemeye çalışıyor mu kontrol et (mantıksal hata)
        if (targetFd == msg.fd)
        {
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You cannot kick yourself");
            continue;
        }

        if (targetClient == NULL)
        {
            sendReply(msg.fd, ":server 401 " + nick + " " + targetUser + " :No such nick/channel");
            continue;
        }

        // 11. Atılacak kullanıcı kanalda mı? - ERR_USERNOTINCHANNEL (441)
        if (!channel->hasUser(targetFd))
        {
            sendReply(msg.fd, ":server 441 " + nick + " " + targetUser + " " + channelName + " :They aren't on that channel");
            continue;
        }

        // 12. KICK mesajını tüm kanal üyelerine broadcast et (atılan kullanıcı dahil!)
        std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
        std::string kickMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname +
                              " KICK " + channelName + " " + targetUser + " :" + reason + "\r\n";
        
        broadcastToChannel(this, channel, kickMsg, 0);

        // 13. Kullanıcıyı kanaldan çıkar
        channel->removeUser(targetFd);
        
        // Eğer operator ise operator listesinden de çıkar
        if (channel->isOperator(targetFd))
        {
            channel->removeOperator(targetFd);
        }
        
        // 14. Client'ın channel listesinden de çıkar
        // Client'ın hala bağlı olduğunu kontrol et (race condition)
        std::map<size_t, Client>::iterator targetIt = _clients.find(targetFd);
        if (targetIt != _clients.end())
        {
            targetIt->second.leaveChannel(channelName);
        }

        // 15. Kanal boş kaldıysa kanalı sil
        // DİKKAT: channel pointer bu noktadan sonra geçersiz olacak!
        if (channel->getMembers().empty())
        {
            _channels.erase(_channels.begin() + channelIndex);
            // channel pointer artık kullanılmamalı!
        }
    }
}
