#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - PRIVMSG command
// Syntax: PRIVMSG <target>{,<target>} :<message>
// Numeric replies: ERR_NORECIPIENT (411), ERR_NOTEXTTOSEND (412),
//                  ERR_CANNOTSENDTOCHAN (404), ERR_NOTOPLEVEL (413),
//                  ERR_WILDTOPLEVEL (414), ERR_NOSUCHNICK (401),
//                  RPL_AWAY (301)
void Server::privmsgCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    // 1. ERR_NORECIPIENT (411)
    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 411 " + nick + " :No recipient given (PRIVMSG)\r\n");
        return;
    }

    // 2. ERR_NOTEXTTOSEND (412)
    if (msg.Parameters.size() < 2 || msg.Parameters[1].empty())
    {
        sendReply(msg.fd, ":server 412 " + nick + " :No text to send\r\n");
        return;
    }

    // 3. Kayıt kontrolü - ERR_NOTREGISTERED (451)
    if (!cli.isRegistered())
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered\r\n");
        return;
    }

    // 4. Parametreleri al
    std::string targets = msg.Parameters[0];
    std::string message = msg.Parameters[1];

    // 5. Hedef listesini parse et
    std::vector<std::string> targetList = split(targets, ',');

    // 6. Her hedef için işlem yap
    for (size_t i = 0; i < targetList.size(); ++i)
    {
        std::string target = targetList[i];
        
        if (target.empty())
            continue;

        // 7. Hedef tipi belirleme
        if (target[0] == '#')
        {
            // ============= KANAL'A MESAJ =============
            
            // 8. Kanal var mı kontrolü - ERR_NOSUCHCHANNEL (403)
            if (!haschannel(target))
            {
                sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel\r\n");
                continue;
            }

            // 9. Kanalı bul
            Channel* channel = NULL;
            for (size_t j = 0; j < _channels.size(); ++j)
            {
                if (_channels[j].getName() == target)
                {
                    channel = &_channels[j];
                    break;
                }
            }

            if (channel == NULL)
                continue;

            // 10. Gönderen kanalda mı kontrolü - ERR_CANNOTSENDTOCHAN (404)
            if (!channel->hasUser(msg.fd))
            {
                sendReply(msg.fd, ":server 404 " + nick + " " + target + " :Cannot send to channel\r\n");
                continue;
            }

            // 11. Mesajı kanal üyelerine broadcast et (gönderen hariç)
            std::string privmsgMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() +
                                     " PRIVMSG " + target + " :" + message + "\r\n";
            
            std::vector<size_t> members = channel->getMembers();
            for (size_t j = 0; j < members.size(); ++j)
            {
                if (members[j] != msg.fd) // Gönderene echo yapma!
                {
                    sendReply(members[j], privmsgMsg);
                }
            }
        }
        else
        {
            // ============= KULLANICIYA MESAJ =============
            
            // 12. Hedef kullanıcıyı bul - ERR_NOSUCHNICK (401)
            int targetFd = -1;
            for (std::map<size_t, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
            {
                if (it->second.getNickname() == target)
                {
                    targetFd = it->first;
                    break;
                }
            }

            if (targetFd == -1)
            {
                sendReply(msg.fd, ":server 401 " + nick + " " + target + " :No such nick/channel\r\n");
                continue;
            }

            // 13. Mesajı hedef kullanıcıya gönder
            std::string privmsgMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() +
                                     " PRIVMSG " + target + " :" + message + "\r\n";
            sendReply(targetFd, privmsgMsg);

        }
    }
}