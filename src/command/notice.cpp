#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - NOTICE command
// Syntax: NOTICE <target>{,<target>} :<message>
// NOTICE is similar to PRIVMSG but MUST NOT generate automatic replies
// No numeric replies are sent for NOTICE command errors
void Server::noticeCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    // NOTICE should not generate error replies, so we silently ignore errors
    if (msg.Parameters.empty() || msg.Parameters.size() < 2)
        return;
}

// ÖNEMLİ: NOTICE komutu ASLA hata mesajı döndürmez!
// PRIVMSG ile neredeyse aynıdır ama sessizdir (no error replies)

// ============= ANA FARK =============
// - NOTICE otomatik cevap üretmez (botlar için önemli)
// - Hiçbir hata mesajı gönderilmez (ERR_NOSUCHNICK, ERR_CANNOTSENDTOCHAN vb. YOK)
// - Sessizce ignore edilir

// ============= KONTROLLER (HATA VERİLMEZ!) =============

// 1. Parametre kontrolü - ZATEN VAR (sessizce return)
//    - msg.Parameters.empty() veya msg.Parameters.size() < 2
//    - Hata mesajı gönderme, sadece return

// 2. Kayıt kontrolü
//    - cli.isRegistered() false ise
//    - Hata mesajı gönderme, sadece return

// 3. Parametreleri al
//    - target = msg.Parameters[0]  (kullanıcı veya kanal)
//    - message = msg.Parameters[1] (mesaj içeriği)

// ============= KANAL'A NOTICE (target[0] == '#') =============

// 4. Kanal var mı kontrolü
//    - haschannel(target) false ise
//    - Sessizce return (ERR_NOSUCHCHANNEL gönderme!)

// 5. Kanalı bul ve referans al
//    - _channels vektöründe target'ı ara
//    - Channel& channel = _channels[index]

// 6. Gönderen kanalda mı kontrolü (opsiyonel)
//    - channel.hasUser(msg.fd) false ise
//    - Sessizce return (ERR_NOTONCHANNEL gönderme!)

// 7. NOTICE mesajını kanal üyelerine broadcast et
//    - Format: ":<nick>!<user>@<host> NOTICE <channel> :<message>\r\n"
//    - channel.getMembers() ile tüm üyelere gönder
//    - NOT: Gönderen kullanıcıya gönderme (kendisine değil)

// ============= KULLANICIYA NOTICE (target[0] != '#') =============

// 8. Hedef kullanıcıyı bul
//    - _clients map'inde target nick'ini ara
//    - Bulunamazsa sessizce return (ERR_NOSUCHNICK gönderme!)

// 9. NOTICE mesajını hedef kullanıcıya gönder
//    - Format: ":<nick>!<user>@<host> NOTICE <targetNick> :<message>\r\n"
//    - sendReply(targetFd, noticeMsg)

// ============= ÖNEMLİ NOTLAR =============
// - Hiçbir zaman hata mesajı gönderilmez
// - Otomatik cevap (auto-reply) üretmez
// - Botların birbirlerine sonsuz loop mesaj göndermesini engeller
// - PRIVMSG ile aynı mantık ama sessiz