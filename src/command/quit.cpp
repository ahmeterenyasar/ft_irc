#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"
#include <set>

// RFC 2812 - QUIT command
// Syntax: QUIT [<quit message>]
// No numeric replies defined for successful QUIT
void Server::quitCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
    std::string user = cli.getUsername().empty() ? "*" : cli.getUsername();
    std::string host = cli.getHostname().empty() ? "localhost" : cli.getHostname();

    std::string quitMessage = "Client Quit";
    if (!msg.Parameters.empty())
        quitMessage = msg.Parameters[0];

    std::string quitMsg = ":" + nick + "!" + user + "@" + host + " QUIT :" + quitMessage + "\r\n";

    // 1. Kullanıcının bulunduğu tüm kanalları al (state değişmeden önce)
    std::vector<std::string> channels = cli.getChannels();
    
    // 3. Ortak kanallardaki kullanıcılara broadcast et (tekrar göndermeden)
    broadcastToCommonChannels(this, cli, quitMsg, msg.fd);
    
    // 4. Kullanıcıyı tüm kanallardan çıkar ve boş kanalları sil
    removeUserFromAllChannels(this, msg.fd);
    cleanupEmptyChannels(this);
    
    // 5. ERROR mesajını kullanıcıya gönder (son mesaj)
    sendReply(msg.fd, "ERROR :Closing Link: " + host + " (" + quitMessage + ")");

    
    // 7. Bağlantıyı kapat ve pollfd'yi temizle
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == msg.fd)
        {
            close(_pollFds[i].fd);
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }
    // 6. Client'ı sil
    _clients.erase(msg.fd);
}
