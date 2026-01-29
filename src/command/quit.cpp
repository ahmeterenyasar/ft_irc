#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

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

    std::string quitMsg = ":" + nick + "!" + user + "@" + host + " QUIT :" + quitMessage;

    // Kullanıcının bulunduğu tüm kanallardaki diğer kullanıcılara bildir
    std::vector<std::string> channels = cli.getChannels();
    for (size_t i = 0; i < channels.size(); i++)
    {
        // channeldeki tüm üyelere QUIT mesajı gönder (kendisi hariç)
    }

    sendReply(msg.fd, "ERROR :Closing Link: " + host + " (" + quitMessage + ")");

    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        // if (_pollFds[i].fd == msg.fd)
        // {
        //     _clients.erase(msg.fd);
        //     disconnectClient(i);
        //     break;
        // }
    }
}

// ============= ANA KONTROLLER =============

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