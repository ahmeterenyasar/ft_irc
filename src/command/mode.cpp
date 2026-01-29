#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - MODE command
// Channel modes: MODE <channel> [<modestring> [<mode arguments>...]]
// User modes: MODE <nickname> [<modestring>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_CHANOPRIVSNEEDED (482),
//                  ERR_NOSUCHNICK (401), ERR_NOSUCHCHANNEL (403),
//                  ERR_UNKNOWNMODE (472), RPL_CHANNELMODEIS (324),
//                  RPL_UMODEIS (221)
void Server::modeCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 461 " + nick + " MODE :Not enough parameters");
        return;
    }
//Kayıt kontrolü
// Target'ı belirle (kanal mı, kullanıcı modu mu?)
//Sadece MODE sorgusu mu? (modestring yok)
//Kanal var mı kontrolü
// Kanalı bul ve referans al
//MODE değiştiren kullanıcı kanalda mı?
//MODE değiştiren kullanıcı operator mı?
//Mode string'i parse et
//Her mod karakterini işle
//    /     - bool adding = true (+ modundayız)
//     - bool removing = false (- modundayız)
//     - Her karakter için:
//       - '+' görürsen → adding = true, removing = false
//       - '-' görürsen → adding = false, removing = true
//       - 'i' görürsen → invite-only modu
//       - 't' görürsen → topic restriction modu
//       - 'k' görürsen → key (şifre) modu
//       - 'o' görürsen → operator verme/alma
//       - 'l' görürsen → user limit modu
    // ============= MOD DETAYLARI =============

// MODE +i (invite-only)
//    - channel.setInviteOnly(true)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> +i\r\n"

// MODE -i (invite-only kaldır)
//    - channel.setInviteOnly(false)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> -i\r\n"

// MODE +t (topic restriction)
//    - channel.setTopicRestricted(true)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> +t\r\n"

// MODE -t (topic restriction kaldır)
//    - channel.setTopicRestricted(false)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> -t\r\n"

// MODE +k <key> (şifre koy)
//    - Parametre kontrolü: msg.Parameters.size() > paramIndex
//    - key = msg.Parameters[paramIndex++]
//    - channel.setKey(key)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> +k <key>\r\n"

// MODE -k (şifreyi kaldır)
//    - channel.setKey("")
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> -k\r\n"

// MODE +o <nick> (operator ver)
//    - Parametre kontrolü: msg.Parameters.size() > paramIndex
//    - targetNick = msg.Parameters[paramIndex++]
//    - Hedef kullanıcıyı bul (ERR_NOSUCHNICK 401)
//    - Hedef kullanıcı kanalda mı kontrol et (ERR_USERNOTINCHANNEL 441)
//    - channel.addOperator(targetFd)
//    - targetClient.setOperator(channelName, true)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> +o <targetNick>\r\n"

// MODE -o <nick> (operator al)
//    - Parametre kontrolü: msg.Parameters.size() > paramIndex
//    - targetNick = msg.Parameters[paramIndex++]
//    - Hedef kullanıcıyı bul (ERR_NOSUCHNICK 401)
//    - Hedef kullanıcı kanalda mı kontrol et (ERR_USERNOTINCHANNEL 441)
//    - channel'dan operator'u çıkar (removeOperator metodu gerekebilir)
//    - targetClient.setOperator(channelName, false)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> -o <targetNick>\r\n"

// MODE +l <limit> (user limit koy)
//    - Parametre kontrolü: msg.Parameters.size() > paramIndex
//    - limit = atoi(msg.Parameters[paramIndex++])
//    - limit > 0 kontrolü
//    - channel.setUserLimit(limit)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> +l <limit>\r\n"

// MODE -l (user limit kaldır)
//    - channel.setUserLimit(0) veya channel.setUserLimit(-1)
//    - Broadcast: ":<nick>!<user>@<host> MODE <channel> -l\r\n"

// Bilinmeyen mod karakteri
//    - ERR_UNKNOWNMODE (472)
//    - Format: ":server 472 <nick> <char> :is unknown mode char to me\r\n"

// ============= ÖZEL DURUMLAR =============
// - Birden fazla mod tek komutta işlenebilir: MODE #kanal +ik-t key123
// - Parametreler sırayla işlenir (k için key, o için nick, l için limit)
// - Broadcast mesajları tüm kanal üyelerine gönderilir
// - MODE değişikliği yapan kullanıcı dahil
}
