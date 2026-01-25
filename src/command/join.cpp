#include "../../inc/server.hpp"
#include "../../inc/client.hpp"


// RFC 2812 - JOIN command
// Syntax: JOIN <channel>{,<channel>} [<key>{,<key>}]
// Numeric replies: ERR_NEEDMOREPARAMS (461), ERR_NOSUCHCHANNEL (403),
//                  ERR_TOOMANYCHANNELS (405), ERR_BADCHANNELKEY (475),
//                  ERR_BANNEDFROMCHAN (474), ERR_INVITEONLYCHAN (473),
//                  ERR_CHANNELISFULL (471), RPL_TOPIC (332),
//                  RPL_NAMREPLY (353), RPL_ENDOFNAMES (366)



std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) 
    {
        tokens.push_back(token);
    }
    return tokens;
}

//chanel zaten var mı kontrol et
bool Server::haschannel(std::string name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i].getName() == name)
            return true;
    }
    return false;
}

void Server::joinCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
    
    if (!cli.isRegistered()) 
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered\r\n");
        return;
    }
    
    if (msg.Parameters.empty()) 
    {
        sendReply(msg.fd, ":server 461 " + nick + " JOIN :Not enough parameters\r\n");
        return;
    }
    
    ////BU EN SONRA YAPILACAK
    //// JOIN 0
    //// Kullanıcının üye olduğu tüm kanallardan çıkması
    std::vector<std::string> jchannels = split(msg.Parameters[0], ',');
    std::vector<std::string> keys;
    if (msg.Parameters.size() > 1)
        keys = split(msg.Parameters[1], ',');
    
    //JChanel adı validasyonu
    for(size_t i = 0; i < jchannels.size(); ++i)
    {
        std::string channelName = jchannels[i];
        if (channelName.empty() || channelName[0] != '#')
        {
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel\r\n");
            continue;
        }
        else if ()
        {
            /* kanal varsa*/
        }
        else 
        {
            /* kanal yoksa */
        }
        
    }
    
    
    //YOKSA OLUŞTURMA
    // #yeniKanal adlı kanalı oluşturur
    // JOIN eden kullanıcıyı:
    // Kanalın ilk üyesi yapar
    // Channel Operator (op, +o) yetkisi verir
    // Kanalın başlangıç durumunu ayarlar:
    // Topic: boş
    // Modlar: varsayılan (sunucuya bağlı)
    // Kullanıcıya şu mesajları gönderir:
    // JOIN #yeniKanal
    // (Varsa) RPL_TOPIC veya RPL_NOTOPIC
    // RPL_NAMREPLY (kullanıcı listesi)




    // JOIN Başarılı Olursa Ne Olur?
    // Bir JOIN başarılı olduğunda sunucu:
    // Kullanıcıya bir JOIN mesajı gönderir (onay)
    // Kanalın topic’ini gönderir:
    // RPL_TOPIC
    // Kanal üye listesini gönderir:
    // RPL_NAMREPLY
    // Bu liste JOIN eden kullanıcıyı da içerir

}



