#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - NICK command
// Syntax: NICK <nickname>
// Numeric replies: ERR_NONICKNAMEGIVEN (431), ERR_ERRONEUSNICKNAME (432),
//                  ERR_NICKNAMEINUSE (433), ERR_NICKCOLLISION (436),
//                  ERR_RESTRICTED (484)

static bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;
    
    // First character must be a letter or special character
    char first = nick[0];
    if (!std::isalpha(first) && first != '[' && first != ']' && 
        first != '\\' && first != '`' && first != '_' && 
        first != '^' && first != '{' && first != '|' && first != '}')
        return false;
    
    // Rest can be letters, digits, or special characters
    for (size_t i = 1; i < nick.length(); i++)
    {
        char c = nick[i];
        if (!std::isalnum(c) && c != '[' && c != ']' && 
            c != '\\' && c != '`' && c != '_' && 
            c != '^' && c != '{' && c != '|' && c != '}' && c != '-')
            return false;
    }
    return true;
}

void Server::nickCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];

    if (msg.Parameters.empty())
    {
        sendReply(msg.fd, ":server 431 * :No nickname given");
        return;
    }
    std::string newNick = msg.Parameters[0];
    if (!isValidNickname(newNick))
    {
        // Hata durumunda hedef '*' veya mevcut nick olmalı
        std::string target = cli.getNickname().empty() ? "*" : cli.getNickname();
        sendReply(msg.fd, ":server 432 " + target + " " + newNick + " :Erroneous nickname");
        return;
    }
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == newNick)
        {
            std::string target = cli.getNickname().empty() ? "*" : cli.getNickname();
            sendReply(msg.fd, ":server 433 " + target + " " + newNick + " :Nickname is already in use");
            return;
        }
    }
    if (cli.isRegistered())
    {
        std::string oldNick = cli.getNickname();
        std::string oldUser = cli.getUsername();
        std::string oldHost = cli.getHostname();

        // Mesaj: :EskiNick!User@Host NICK :YeniNick
        std::string msgToSend = ":" + oldNick + "!" + oldUser + "@" + oldHost + " NICK :" + newNick + "\r\n";
        
        send(msg.fd, msgToSend.c_str(), msgToSend.length(), 0);
        
        // 2. [EKSİK OLAN KISIM] Ortak kanallara bildir
        // Bu fonksiyonu sonradan yazman gerekecek:
        // broadcastToCommonChannels(cli, msgToSend);
    }
    cli.setNickname(newNick);
    if (cli.isAuthenticated() && !cli.getUsername().empty() && 
        !cli.getNickname().empty() && !cli.isRegistered())
    {
        cli.setRegistered(true);
        sendReply(msg.fd, ":server 001 " + newNick + " :Welcome to the Internet Relay Network " + newNick + "!" + cli.getUsername() + "@server");
        sendReply(msg.fd, ":server 002 " + newNick + " :Your host is server, running version 1.0");
        sendReply(msg.fd, ":server 003 " + newNick + " :This server was created " + std::string(__DATE__));
        sendReply(msg.fd, ":server 004 " + newNick + " server 1.0 o o");
    }
}
