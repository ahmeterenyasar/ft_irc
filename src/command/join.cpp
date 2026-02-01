#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"


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

void Server::joinCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick;

    if(cli.getNickname().empty())
        nick = "*";
    else
        nick = cli.getNickname();

    if (!cli.isRegistered()) 
    {
        sendReply(msg.fd, ":server 451 " + nick + " :You have not registered");
        return;
    }
    
    if (msg.Parameters.empty()) 
    {
        sendReply(msg.fd, ":server 461 " + nick + " JOIN :Not enough parameters");
        return;
    }
    ////BU EN SONRA YAPILACAK
    //// JOIN 0
    //// Kullanıcının üye olduğu tüm kanallardan çıkması


    if (msg.Parameters[0] == "0")
    {
        std::vector<std::string> userChannels = cli.getChannels();
        for (size_t i = 0; i < userChannels.size(); ++i)
        {
            msg.Parameters[0] = userChannels[i];
            partCommand(msg);
        }
        return;
    }
    
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
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel");
            continue;
        }
        
        // Kanal ismi uzunluk kontrolü (max 50 karakter)
        if (channelName.length() > 50)
        {
            sendReply(msg.fd, ":server 479 " + nick + " " + channelName + " :Illegal channel name (too long)");
            continue;
        }
        
        // Kanal ismi geçersiz karakter kontrolü (boşluk, virgül, kontrol karakterleri)
        bool validName = true;
        for (size_t j = 1; j < channelName.length(); ++j)
        {
            char c = channelName[j];
            if (c == ' ' || c == ',' || c == '\r' || c == '\n' || c == '\0' || c == 7)
            {
                validName = false;
                break;
            }
        }
        if (!validName)
        {
            sendReply(msg.fd, ":server 479 " + nick + " " + channelName + " :Illegal channel name");
            continue;
        }
        
        else if (haschannel(channelName))
        {
            /* kanal varsa*/
            // Kanalı bul
            //kullanıcı kanalda mı 
            // key şifre kontrolü 
            //  Invetie only kontrolü
            // user limit kontrolü
            // kullanıcı kanala ekle
            // kanalın üyelerine JOIN mesajı gönder
            //Topic mesajı gönder
            // isim listesi gönder
            
            // Kanalı bul
            Channel* channel = NULL;
            for (size_t j = 0; j < _channels.size(); ++j)
            {
                if (_channels[j].getName() == channelName)
                {
                    channel = &_channels[j];
                    break;
                }
            }
            
            // Kullanıcı zaten kanalda mı?
            if (channel->hasUser(msg.fd))
            {
                continue; // Kullanıcı zaten kanalda, işlem yapma
            }
            
            // Kullanıcının kaç kanalda olduğunu kontrol et (max 10 kanal)
            if (cli.getChannels().size() >= 10)
            {
                sendReply(msg.fd, ":server 405 " + nick + " " + channelName + " :You have joined too many channels");
                continue;
            }
            
            // Key şifre kontrolü
            std::string key = (i < keys.size()) ? keys[i] : "";
            if (!channel->getKey().empty() && channel->getKey() != key)
            {
                sendReply(msg.fd, ":server 475 " + nick + " " + channelName + " :Cannot join channel (+k)");
                continue;
            }
            
            // Invite only kontrolü
            if (channel->isInviteOnly() && !channel->isInvited(nick))
            {
                sendReply(msg.fd, ":server 473 " + nick + " " + channelName + " :Cannot join channel (+i)");
                continue;
            }
            
            // User limit kontrolü
            if (channel->getUserLimit() > 0 && channel->getUserCount() >= channel->getUserLimit())
            {
                sendReply(msg.fd, ":server 471 " + nick + " " + channelName + " :Cannot join channel (+l)");
                continue;
            }
            
            // Kullanıcıyı kanala ekle
            channel->addUser(msg.fd);
            cli.joinChannel(channelName);
            
            // Invite listesinden çıkar (eğer invite ile girdiyse)
            if (channel->isInvited(nick))
                channel->removeInvite(nick);
            
            // Kanalın üyelerine JOIN mesajı gönder (RFC2812: JOIN #channel)
            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string joinMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " JOIN " + channelName + "\r\n";
            std::vector<size_t> members = channel->getMembers();
            
            for (size_t m = 0; m < members.size(); ++m)
            {
                send(members[m], joinMsg.c_str(), joinMsg.length(), 0);
            }
            
            // Topic mesajı gönder
            if (!channel->getTopic().empty())
            {
                sendReply(msg.fd, ":server 332 " + nick + " " + channelName + " :" + channel->getTopic());
            }
            else
            {
                sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set");
            }
            
            // İsim listesi gönder (RPL_NAMREPLY 353 ve RPL_ENDOFNAMES 366)
            std::string userList = getUserList(*channel);
            sendReply(msg.fd, ":server 353 " + nick + " = " + channelName + " :" + userList);
            sendReply(msg.fd, ":server 366 " + nick + " " + channelName + " :End of /NAMES list");
        }
        else 
        {
            // Kullanıcının kaç kanalda olduğunu kontrol et (max 10 kanal)
            if (cli.getChannels().size() >= 10)
            {
                sendReply(msg.fd, ":server 405 " + nick + " " + channelName + " :You have joined too many channels");
                continue;
            }
            
            // Yeni kanal oluştur
            Channel newChannel(channelName);
            
            // İlk kullanıcıyı ekle ve operator yap
            newChannel.addUser(msg.fd);
            newChannel.addOperator(msg.fd);
            
            // Eğer key varsa, kanala şifre koy
            std::string key = (i < keys.size()) ? keys[i] : "";
            if (!key.empty())
            {
                newChannel.setKey(key);
            }
            
            // Kanalı sunucuya ekle
            _channels.push_back(newChannel);
            
            // Client'a kanal bilgisini ekle
            cli.joinChannel(channelName);
            cli.setOperator(channelName, true);
            
            // JOIN mesajı gönder (RFC2812: JOIN #channel)
            std::string hostname = cli.getHostname().empty() ? "localhost" : cli.getHostname();
            std::string joinMsg = ":" + nick + "!" + cli.getUsername() + "@" + hostname + " JOIN " + channelName + "\r\n";
            
            send(msg.fd, joinMsg.c_str(), joinMsg.length(), 0);
            
            // No topic mesajı (RPL_NOTOPIC 331)
            sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set");
            
            // İsim listesi gönder (sadece operator olan kullanıcı - @ prefix ile)
            // Operator kullanıcılar @ ile gösterilir
            std::string userList = "@" + nick;
            sendReply(msg.fd, ":server 353 " + nick + " = " + channelName + " :" + userList);
            sendReply(msg.fd, ":server 366 " + nick + " " + channelName + " :End of /NAMES list");
        }
    }
}



