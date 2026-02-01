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

    std::string quitMsg = getUserPrefix(cli) + " QUIT :" + quitMessage + "\r\n";

    // 1. Kullanıcının bulunduğu tüm kanalları al (state değişmeden önce)
    std::vector<std::string> channels = cli.getChannels();
    
    // 3. Ortak kanallardaki kullanıcılara broadcast et (tekrar göndermeden)
    broadcastToCommonChannels(this, cli, quitMsg, msg.fd);
    
    // 4. Kullanıcıyı tüm kanallardan çıkar ve boş kanalları sil
    removeUserFromAllChannels(this, msg.fd);
    cleanupEmptyChannels(this);
    
    // 5. ERROR mesajını kullanıcıya gönder (son mesaj)
    sendReply(msg.fd, "ERROR :Closing Link: " + host + " (" + quitMessage + ")\r\n");

    // 6. Client'ı sil
    _clients.erase(msg.fd);
    
    // 7. Bağlantıyı kapat ve pollfd'yi temizle
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == static_cast<int>(msg.fd))
        {
            close(_pollFds[i].fd);
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }
}

// ============= ANA KONTROLLER =============

/*

QUIT
 ├─ build quit message
 ├─ collect all channels
 ├─ for each channel
 │   ├─ notify members (once)
 │   ├─ remove user
 │   └─ delete channel if empty
 ├─ send ERROR to client
 ├─ erase client
 └─ close fd & poll cleanup

*/


// 1. Parametreleri al - ZATEN YAPILMIŞ
//    - quitMessage = msg.Parameters.empty() ? "Client Quit" : msg.Parameters[0]
//    - Quit mesajı opsiyonel (örnek: "Goodbye" veya "Leaving")

// 2. Quit mesajını oluştur - ZATEN YAPILMIŞ
//    - Format: ":<nick>!<user>@<host> QUIT :<quitMessage>\r\n"

// ============= KANALLARA BROADCAST =============

// 3. Kullanıcının bulunduğu tüm kanalları al - ZATEN YAPILMIŞ
//    - std::vector<std::string> channels = cli.getChannels()

// 4. Her kanal için QUIT mesajını broadcast et
//    - Kanalı bul (haschannel kontrolü)
//    - Channel& channel = _channels[index]
//    - channel.getMembers() ile tüm üyelere QUIT mesajı gönder
//    - NOT: Ayrılan kullanıcıya gönderme (msg.fd hariç)
//    - NOT: Aynı kullanıcıya birden fazla kez gönderme (tekrar eden kullanıcıları işaretle)
//    - std::set<size_t> notifiedUsers kullanabilirsin

// 5. Kullanıcıyı tüm kanallardan çıkar
//    - Her kanal için: channel.removeUser(msg.fd)
//    - Eğer operator ise: operator listesinden de çıkar
//    - cli.leaveChannel(channelName) her kanal için

// 6. Boş kalan kanalları temizle (opsiyonel)
//    - channel.getUserCount() == 0 ise
//    - _channels vektöründen kanalı sil

// ============= BAĞLANTIYI KES =============

// 7. Kullanıcıya ERROR mesajı gönder - ZATEN YAPILMIŞ (eksik \r\n)
//    - Format: "ERROR :Closing Link: <host> (<quitMessage>)\r\n"
//    - sendReply(msg.fd, errorMsg)

// 8. Kullanıcıyı _clients map'inden sil
//    - _clients.erase(msg.fd)

// 9. Bağlantıyı kes ve pollfd'yi temizle
//    - _pollFds vektöründe msg.fd'yi bul
//    - disconnectClient(i) çağır (close(fd) + pollfd silme)

// ============= ÖNEMLİ NOTLAR =============
// - QUIT her zaman başarılıdır (hata mesajı yok)
// - Aynı kullanıcıya birden fazla QUIT mesajı gönderme
//   (kullanıcı birden fazla ortak kanalda olabilir)
// - Kayıt olmamış kullanıcı da QUIT yapabilir
// - QUIT'ten sonra bağlantı kapatılmalı
// - ERROR mesajı sadece ayrılan kullanıcıya gönderilir
// - QUIT mesajı kanal üyelerine broadcast edilir