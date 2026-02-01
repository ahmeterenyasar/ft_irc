#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"

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
    if (!checkMinParams(this, msg, cli, 1, "PRIVMSG"))
    {
        sendReply(msg.fd, ":server 411 " + nick + " :No recipient given (PRIVMSG)");
        return;
    }

    // 2. ERR_NOTEXTTOSEND (412)
    if (msg.Parameters.size() < 2 || msg.Parameters[1].empty())
    {
        sendReply(msg.fd, ":server 412 " + nick + " :No text to send");
        return;
    }

    if (!checkRegistered(this, msg, cli))
        return;

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
                sendReply(msg.fd, ":server 403 " + nick + " " + target + " :No such channel");
                continue;
            }

            // 9. Kanalı bul
            Channel* channel = findChannel(this, target);

            if (channel == NULL)
                continue;

            // 10. Gönderen kanalda mı kontrolü - ERR_CANNOTSENDTOCHAN (404)
            if (!channel->hasUser(msg.fd))
            {
                sendReply(msg.fd, ":server 404 " + nick + " " + target + " :Cannot send to channel");
                continue;
            }

            // 11. Mesajı kanal üyelerine broadcast et (gönderen hariç)
            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string privmsgMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname +
                                     " PRIVMSG " + target + " :" + message + "\r\n";
            
            broadcastToChannel(this, channel, privmsgMsg, msg.fd);
        }
        else
        {
            // ============= KULLANICIYA MESAJ =============
            
            // 12. Hedef kullanıcıyı bul - ERR_NOSUCHNICK (401)
            size_t targetFd = 0;
            Client* targetClient = findClientByNick(this, target, targetFd);

            if (targetClient == NULL)
            {
                sendReply(msg.fd, ":server 401 " + nick + " " + target + " :No such nick/channel");
                continue;
            }

            // 13. Mesajı hedef kullanıcıya gönder
            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string privmsgMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname +
                                     " PRIVMSG " + target + " :" + message + "\r\n";
            send(targetFd, privmsgMsg.c_str(), privmsgMsg.length(), 0);

        }
    }
}