#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - KICK command
// Syntax: KICK <channel> <user> [<comment>]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_CHANOPRIVSNEEDED (482), ERR_USERNOTINCHANNEL (441),
//                  ERR_NOTONCHANNEL (442)
void Server::kickCommand(IRCMessage& msg)
{
    // Client'ın var olup olmadığını kontrol et
    std::map<size_t, Client>::iterator clientIt = _clients.find(msg.fd);
    if (clientIt == _clients.end())
        return;
    
    Client& cli = clientIt->second;
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();

    // En az 2 parametre gerekli: kanal ve kullanıcı
    if (msg.Parameters.size() < 2)
    {
        sendReply(msg.fd, ":server 461 " + nick + " KICK :Not enough parameters");
        return;
    }
    //Kayıt kontrolü
    //Parametreleri al
    //Kanal var mı kontrolü
    //Kanalı bul ve referans al
    //KICK eden kullanıcı kanalda mı kontrolü
    //KICK eden kullanıcı operator mı kontrolü
    //Atılacak kullanıcıyı bul
    //Atılacak kullanıcı kanalda mı kontrolü
    //KICK mesajını tüm kanal üyelerine broadcast et
    //Kullanıcıyı kanaldan çıkar
    //Eğer atılan kullanıcı operator idi, operator listesinden de çıkar
    //Kanal boş kaldıysa kanalı sil (opsiyonel)
}
