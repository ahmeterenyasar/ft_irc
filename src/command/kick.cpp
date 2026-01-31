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
    std::map<size_t, Client>::iterator clientIt = _clients.find(msg.fd);
    if (clientIt == _clients.end())
        return;
    
    Client& cli = clientIt->second;
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    // 1. En az 2 parametre gerekli: kanal ve kullanıcı - ERR_NEEDMOREPARAMS (461)
    if (msg.Parameters.size() < 2)
    {
        sendReply(msg.fd, ":server 461 " + nick + " KICK :Not enough parameters\r\n");
        return;
    }

    // 2. Kayıt kontrolü - ERR_NOTREGISTERED (451)
    if (!cli.isRegistered())
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered\r\n");
        return;
    }

    // 3. Parametreleri al ve parse et
    std::vector<std::string> channels = split(msg.Parameters[0], ',');
    std::vector<std::string> users = split(msg.Parameters[1], ',');
    std::string reason = (msg.Parameters.size() >= 3) ? msg.Parameters[2] : nick;

    // 4. Kanal-User eşleşme kontrolü
    // Geçerli: 1 kanal + N user VEYA N kanal + N user
    if (channels.size() != 1 && channels.size() != users.size())
    {
        sendReply(msg.fd, ":server 461 " + nick + " KICK :Not enough parameters\r\n");
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
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel\r\n");
            continue;
        }

        // 7. Kanalı bul
        Channel* channel = NULL;
        size_t channelIndex = 0;
        for (size_t j = 0; j < _channels.size(); ++j)
        {
            if (_channels[j].getName() == channelName)
            {
                channel = &_channels[j];
                channelIndex = j;
                break;
            }
        }

        if (channel == NULL)
            continue;

        // 8. KICK eden kullanıcı kanalda mı? - ERR_NOTONCHANNEL (442)
        if (!channel->hasUser(msg.fd))
        {
            sendReply(msg.fd, ":server 442 " + nick + " " + channelName + " :You're not on that channel\r\n");
            continue;
        }

        // 9. KICK eden kullanıcı operator mı? - ERR_CHANOPRIVSNEEDED (482)
        if (!channel->isOperator(msg.fd))
        {
            sendReply(msg.fd, ":server 482 " + nick + " " + channelName + " :You're not channel operator\r\n");
            continue;
        }

        // 10. Atılacak kullanıcıyı bul
        int targetFd = -1;
        for (std::map<size_t, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second.getNickname() == targetUser)
            {
                targetFd = it->first;
                break;
            }
        }

        if (targetFd == -1)
        {
            sendReply(msg.fd, ":server 401 " + nick + " " + targetUser + " :No such nick/channel\r\n");
            continue;
        }

        // 11. Atılacak kullanıcı kanalda mı? - ERR_USERNOTINCHANNEL (441)
        if (!channel->hasUser(targetFd))
        {
            sendReply(msg.fd, ":server 441 " + nick + " " + targetUser + " " + channelName + " :They aren't on that channel\r\n");
            continue;
        }

        // 12. KICK mesajını tüm kanal üyelerine broadcast et (atılan kullanıcı dahil!)
        std::string kickMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() +
                              " KICK " + channelName + " " + targetUser + " :" + reason + "\r\n";
        
        std::vector<size_t> members = channel->getMembers();
        for (size_t j = 0; j < members.size(); ++j)
        {
            sendReply(members[j], kickMsg);
        }

        // 13. Kullanıcıyı kanaldan çıkar
        channel->removeUser(targetFd);
        
        // 14. Client'ın channel listesinden de çıkar
        Client& targetClient = _clients[targetFd];
        targetClient.leaveChannel(channelName);

        // 15. Kanal boş kaldıysa kanalı sil
        if (channel->getMembers().empty())
        {
            _channels.erase(_channels.begin() + channelIndex);
        }
    }
}
