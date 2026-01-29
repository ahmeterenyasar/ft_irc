#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - TOPIC command
// Syntax: TOPIC <channel> [<topic>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOTONCHANNEL (442),
//                  ERR_CHANOPRIVSNEEDED (482), RPL_NOTOPIC (331),
//                  RPL_TOPIC (332)
void Server::topicCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    // ERR_NEEDMOREPARAMS (461)
    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " TOPIC :Not enough parameters");
        return;
    }
    
}

// TOPIC İKİ MODDA ÇALIŞIR:
// 1. TOPIC SORGULAMA: TOPIC #kanal
// 2. TOPIC DEĞİŞTİRME: TOPIC #kanal :Yeni topic

// ============= ANA KONTROLLER =============

// 1. Parametre kontrolü - ZATEN VAR
//    - msg.Parameters.empty() ise ERR_NEEDMOREPARAMS (461)
//    - Format: TOPIC <channel> [<topic>]

// 2. Kayıt kontrolü
//    - cli.isRegistered() false ise ERR_NOTREGISTERED (451)

// 3. Parametreleri al
//    - channelName = msg.Parameters[0]

// 4. Kanal var mı kontrolü
//    - haschannel(channelName) false ise ERR_NOSUCHCHANNEL (403)

// 5. Kanalı bul ve referans al
//    - _channels vektöründe channelName'i ara
//    - Channel& channel = _channels[index]

// 6. Kullanıcı kanalda mı kontrolü
//    - channel.hasUser(msg.fd) false ise ERR_NOTONCHANNEL (442)

// ============= TOPIC SORGULAMA (parametre yok) =============

// 7. Sadece topic sorgusu mu? (msg.Parameters.size() == 1)
//    - Eğer evet:
//      a) Topic boş mu kontrol et
//         - channel.getTopic().empty() ise
//         - RPL_NOTOPIC (331) gönder
//         - Format: ":server 331 <nick> <channel> :No topic is set\r\n"
//      b) Topic varsa
//         - RPL_TOPIC (332) gönder
//         - Format: ":server 332 <nick> <channel> :<topic>\r\n"
//    - return

// ============= TOPIC DEĞİŞTİRME (yeni topic var) =============

// 8. Yeni topic'i al
//    - newTopic = msg.Parameters[1]
//    - Boş string olabilir (topic silme)

// 9. Topic restriction kontrolü (+t modu)
//    - channel.isTopicRestricted() true ise
//      a) Sadece operator değiştirebilir
//         - channel.isOperator(msg.fd) false ise
//         - ERR_CHANOPRIVSNEEDED (482) gönder
//         - Format: ":server 482 <nick> <channel> :You're not channel operator\r\n"
//         - return
//    - channel.isTopicRestricted() false ise
//      a) Kanalda olan herkes topic değiştirebilir

// 10. Topic'i değiştir
//     - channel.setTopic(newTopic)

// 11. TOPIC değişikliğini tüm kanal üyelerine broadcast et
//     - Format: ":<nick>!<user>@<host> TOPIC <channel> :<newTopic>\r\n"
//     - channel.getMembers() ile tüm üyelere gönder (değiştiren dahil!)

// ============= ÖRNEKLER =============
// TOPIC #kanal                    → Topic'i sorgula
// TOPIC #kanal :Yeni konu         → Topic'i değiştir
// TOPIC #kanal :                  → Topic'i sil (boş topic)

// ============= ÖZEL DURUMLAR =============
// - +t modu aktifse sadece operator topic değiştirebilir
// - +t modu yoksa herkes değiştirebilir
// - Topic boş string olabilir (topic yok)
// - Kaydolmamış kullanıcı TOPIC kullanamaz