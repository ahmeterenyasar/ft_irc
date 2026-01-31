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
    std::string nick;

    if(cli.getNickname().empty())
        nick = "*";
    else
        nick = cli.getNickname();

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
            sendReply(msg.fd, ":server 403 " + nick + " " + channelName + " :No such channel\r\n");
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
            std::cout << "[DEBUG] Channel exists: " << channelName << std::endl;
            
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
            std::cout << "[DEBUG] Channel found: " << channel->getName() << std::endl;
            
            // Kullanıcı zaten kanalda mı?
            if (channel->hasUser(msg.fd))
            {
                std::cout << "[DEBUG] User already in channel" << std::endl;
                continue; // Zaten kanalda, işlem yapma
            }
            
            // Key şifre kontrolü
            std::string key = (i < keys.size()) ? keys[i] : "";
            if (!channel->getKey().empty() && channel->getKey() != key)
            {
                std::cout << "[DEBUG] Bad channel key" << std::endl;
                sendReply(msg.fd, ":server 475 " + nick + " " + channelName + " :Cannot join channel (+k)\r\n");
                continue;
            }
            
            // Invite only kontrolü
            if (channel->isInviteOnly() && !channel->isInvited(nick))
            {
                std::cout << "[DEBUG] Channel is invite only and user not invited" << std::endl;
                sendReply(msg.fd, ":server 473 " + nick + " " + channelName + " :Cannot join channel (+i)\r\n");
                continue;
            }
            
            // User limit kontrolü
            if (channel->getUserLimit() > 0 && channel->getUserCount() >= channel->getUserLimit())
            {
                std::cout << "[DEBUG] Channel is full" << std::endl;
                sendReply(msg.fd, ":server 471 " + nick + " " + channelName + " :Cannot join channel (+l)\r\n");
                continue;
            }
            
            // Kullanıcıyı kanala ekle
            channel->addUser(msg.fd);
            cli.joinChannel(channelName);
            std::cout << "[DEBUG] User added to channel" << std::endl;
            
            // Invite listesinden çıkar (eğer invite ile girdiyse)
            if (channel->isInvited(nick))
                channel->removeInvite(nick);
            
            // Kanalın üyelerine JOIN mesajı gönder
            std::string joinMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() + " JOIN " + channelName + "\r\n";
            std::vector<size_t> members = channel->getMembers();
            for (size_t m = 0; m < members.size(); ++m)
            {
                send(members[m], joinMsg.c_str(), joinMsg.length(), 0);
            }
            std::cout << "[DEBUG] JOIN message sent to all members" << std::endl;
            
            // Topic mesajı gönder
            if (!channel->getTopic().empty())
            {
                sendReply(msg.fd, ":server 332 " + nick + " " + channelName + " :" + channel->getTopic() + "\r\n");
                std::cout << "[DEBUG] Topic sent: " << channel->getTopic() << std::endl;
            }
            else
            {
                sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set\r\n");
                std::cout << "[DEBUG] No topic set" << std::endl;
            }
            
            // İsim listesi gönder (RPL_NAMREPLY 353 ve RPL_ENDOFNAMES 366)
            std::string userList = getUserList(*channel);
            sendReply(msg.fd, ":server 353 " + nick + " = " + channelName + " :" + userList + "\r\n");
            sendReply(msg.fd, ":server 366 " + nick + " " + channelName + " :End of /NAMES list\r\n");
            std::cout << "[DEBUG] NAMES list sent: " << userList << std::endl;
        }
        else 
        {
            /* kanal yoksa */
            //yeni kanal oluştur
            // ilk kullanıcıyı ekle ve operator yap
            //kanalı sunucuya ekle
            //Client a kanal bilgisini ekle
            // JOIN mesajı gönder
            //no topic mesahı 
            // isim listesi sadece operator olan kullanıcı
            std::cout << "[DEBUG] Channel does not exist, creating: " << channelName << std::endl;
            
            // Yeni kanal oluştur
            Channel newChannel(channelName);
            std::cout << "[DEBUG] New channel object created" << std::endl;
            
            // İlk kullanıcıyı ekle ve operator yap
            newChannel.addUser(msg.fd);
            newChannel.addOperator(msg.fd);
            std::cout << "[DEBUG] User added as member and operator" << std::endl;
            
            // Eğer key varsa, kanala şifre koy
            std::string key = (i < keys.size()) ? keys[i] : "";
            if (!key.empty())
            {
                newChannel.setKey(key);
                std::cout << "[DEBUG] Channel key set: " << key << std::endl;
            }
            
            // Kanalı sunucuya ekle
            _channels.push_back(newChannel);
            std::cout << "[DEBUG] Channel added to server, total channels: " << _channels.size() << std::endl;
            
            // Client'a kanal bilgisini ekle
            cli.joinChannel(channelName);
            cli.setOperator(channelName, true);
            std::cout << "[DEBUG] Client joined channel and set as operator" << std::endl;
            
            // JOIN mesajı gönder (sadece kendisine, çünkü tek üye)
            std::string joinMsg = ":" + nick + "!" + cli.getUsername() + "@" + cli.getHostname() + " JOIN " + channelName + "\r\n";
            send(msg.fd, joinMsg.c_str(), joinMsg.length(), 0);
            std::cout << "[DEBUG] JOIN message sent: " << joinMsg << std::endl;
            
            // No topic mesajı (RPL_NOTOPIC 331)
            sendReply(msg.fd, ":server 331 " + nick + " " + channelName + " :No topic is set\r\n");
            std::cout << "[DEBUG] No topic message sent" << std::endl;
            
            // İsim listesi gönder (sadece operator olan kullanıcı - @ prefix ile)
            // Operator kullanıcılar @ ile gösterilir
            std::string userList = "@" + nick;
            sendReply(msg.fd, ":server 353 " + nick + " = " + channelName + " :" + userList + "\r\n");
            sendReply(msg.fd, ":server 366 " + nick + " " + channelName + " :End of /NAMES list\r\n");
            std::cout << "[DEBUG] NAMES list sent: " << userList << std::endl;
        }
        
    }
    //////////////////////////////////////////////////////////////
    //KUllanıcı bir şekilde serverden düşerse kanaldan çıkmıyor
    /////////////////////////////////////////////////////////////

}



