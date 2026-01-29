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
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    // ERR_NORECIPIENT (411)
    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 411 " + nick + " :No recipient given (PRIVMSG)");
        return;
    }

    // ERR_NOTEXTTOSEND (412)
    if (msg.Parameters.size() < 2 || msg.Parameters[1].empty())
    {
        sendReply(msg.fd, ":server 412 " + nick + " :No text to send");
        return;
    }

}

// ============= ANA KONTROLLER =============

// 1. Parametre kontrolü - ZATEN VAR
//    - msg.Parameters.empty() ise ERR_NORECIPIENT (411)
//    - msg.Parameters.size() < 2 veya mesaj boş ise ERR_NOTEXTTOSEND (412)
//    - Format: PRIVMSG <target>{,<target>} :<message>

// 2. Kayıt kontrolü
//    - cli.isRegistered() false ise ERR_NOTREGISTERED (451)

// 3. Parametreleri al
//    - targets = msg.Parameters[0]  (virgülle ayrılmış hedef listesi)
//    - message = msg.Parameters[1]  (mesaj içeriği)

// 4. Hedef listesini parse et
//    - split(targets, ',') ile hedef isimlerini ayır
//    - std::vector<std::string> targetList
//    - Örnek: "nick1,#kanal1,nick2"

// ============= HER HEDEF İÇİN =============

// 5. Hedef tipi belirleme
//    - target[0] == '#' ise → KANAL
//    - target[0] != '#' ise → KULLANICI

// ============= KANAL'A MESAJ =============

// 6. Kanal var mı kontrolü
//    - haschannel(target) false ise
//    - ERR_NOSUCHCHANNEL (403) gönder
//    - continue

// 7. Kanalı bul ve referans al
//    - _channels vektöründe target'ı ara
//    - Channel& channel = _channels[index]

// 8. Gönderen kanalda mı kontrolü
//    - channel.hasUser(msg.fd) false ise
//    - ERR_CANNOTSENDTOCHAN (404) gönder
//    - continue

// 9. Mesajı kanal üyelerine broadcast et
//    - Format: ":<nick>!<user>@<host> PRIVMSG <channel> :<message>\r\n"
//    - channel.getMembers() ile tüm üyelere gönder
//    - NOT: Gönderen kullanıcıya GÖNDERMEMELİSİN! (echo yok)
//    - if (members[i] != msg.fd) sendReply(members[i], privmsgMsg)

// ============= KULLANICIYA MESAJ =============

// 10. Hedef kullanıcıyı bul
//     - _clients map'inde target nick'ini ara
//     - Bulunamazsa ERR_NOSUCHNICK (401) gönder
//     - continue

// 11. Mesajı hedef kullanıcıya gönder
//     - Format: ":<nick>!<user>@<host> PRIVMSG <targetNick> :<message>\r\n"
//     - sendReply(targetFd, privmsgMsg)

// ============= ÖRNEKLER =============
// PRIVMSG Alice :Hello!           → Alice'e özel mesaj
// PRIVMSG #kanal :Hi everyone     → Kanal mesajı
// PRIVMSG Alice,Bob :Same message → İki kişiye aynı mesaj
// PRIVMSG #kanal,Alice :Test      → Kanala ve Alice'e mesaj

// ============= ÖZEL DURUMLAR =============
// - Kaydolmamış kullanıcı mesaj gönderemez
// - Gönderen kendisine echo almamalı (kanalda)
// - Virgülle ayrılmış birden fazla hedefe gönderilebilir
// - Her hedef için ayrı hata kontrolü yapılmalı