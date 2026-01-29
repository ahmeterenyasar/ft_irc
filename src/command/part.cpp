#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - PART command
// Syntax: PART <channel>{,<channel>} [<part message>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_NOTONCHANNEL (442)
void Server::partCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " PART :Not enough parameters");
        return;
    }

}

// ============= ANA KONTROLLER =============

// 1. Parametre kontrolü - ZATEN VAR
//    - msg.Parameters.empty() ise ERR_NEEDMOREPARAMS (461)
//    - Format: PART <channel>{,<channel>} [<part message>]

// 2. Kayıt kontrolü
//    - cli.isRegistered() false ise ERR_NOTREGISTERED (451)

// 3. Parametreleri al
//    - channels = msg.Parameters[0]  (virgülle ayrılmış kanal listesi)
//    - partMessage = msg.Parameters.size() > 1 ? msg.Parameters[1] : ""
//    - (part message opsiyonel, örnek: "Goodbye!")

// 4. Kanal listesini parse et
//    - split(channels, ',') ile kanal isimlerini ayır
//    - std::vector<std::string> channelList

// ============= HER KANAL İÇİN =============

// 5. Kanal adı validasyonu
//    - channelName.empty() veya channelName[0] != '#' ise
//    - ERR_NOSUCHCHANNEL (403) gönder
//    - continue (diğer kanallara geç)

// 6. Kanal var mı kontrolü
//    - haschannel(channelName) false ise
//    - ERR_NOSUCHCHANNEL (403) gönder
//    - continue

// 7. Kanalı bul ve referans al
//    - _channels vektöründe channelName'i ara
//    - Channel& channel = _channels[index]

// 8. Kullanıcı bu kanalda mı kontrolü
//    - channel.hasUser(msg.fd) false ise
//    - ERR_NOTONCHANNEL (442) gönder
//    - continue

// 9. PART mesajını tüm kanal üyelerine broadcast et
//    - Format: ":<nick>!<user>@<host> PART <channel> :<partMessage>\r\n"
//    - partMessage boşsa: ":<nick>!<user>@<host> PART <channel>\r\n"
//    - channel.getMembers() ile tüm üyelere gönder (ayrılan kullanıcı dahil!)

// 10. Kullanıcıyı kanaldan çıkar
//     - channel.removeUser(msg.fd)
//     - cli.leaveChannel(channelName)

// 11. Eğer kullanıcı operator idi, operator listesinden çıkar
//     - cli.isOperator(channelName) kontrolü
//     - cli.setOperator(channelName, false)

// 12. Kanal boş kaldıysa kanalı sil (opsiyonel)
//     - channel.getUserCount() == 0 ise
//     - _channels vektöründen kanalı çıkar

// ============= ÖRNEKLER =============
// PART #kanal1                    → #kanal1'den çık (mesaj yok)
// PART #kanal1 :Bye!             → #kanal1'den çık (mesajla)
// PART #kanal1,#kanal2           → İki kanaldan çık
// PART #kanal1,#kanal2 :Goodbye  → İki kanaldan mesajla çık