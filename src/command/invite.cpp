#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - INVITE command
// Syntax: INVITE <nickname> <channel>
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHNICK (401),
//                  ERR_NOTONCHANNEL (442), ERR_USERONCHANNEL (443),
//                  ERR_CHANOPRIVSNEEDED (482), RPL_INVITING (341)
void Server::inviteCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getUsername().empty() ? "*" : cli.getUsername();

    //Parametre kontrolü
    //Kayıt kontrolü 
    //parametrelerden kullanıcı ve kanal adını al
    //kanal kontrolü
    //kanalı bul 
    //davet eden kullanıcının kanalda olup olmadığını kontrol et
    //Davet eden kullanıcı operator mı kontrolü
    //Davet edilecek kullanıcıyı bul
    //Davet edilen kullanıcı zaten kanalda mı kontrolü
    //Davet listesine ekle
    // Davet edene onay mesajı gönder
    //Davet edilen kullanıcıya INVITE mesajı gönder
}
