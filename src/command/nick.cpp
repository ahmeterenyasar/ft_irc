#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"
#include <set>

// RFC 2812 - NICK command
// Syntax: NICK <nickname>
// Numeric replies: ERR_NONICKNAMEGIVEN (431), ERR_ERRONEUSNICKNAME (432),
//                  ERR_NICKNAMEINUSE (433), ERR_NICKCOLLISION (436),
//                  ERR_RESTRICTED (484)

static bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;
    
    char first = nick[0];
    if (!std::isalpha(first) && first != '[' && first != ']' && 
        first != '\\' && first != '`' && first != '_' && 
        first != '^' && first != '{' && first != '|' && first != '}')
        return false;
    
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
        sendReply(msg.fd, ":server 431 * :No nickname given\r\n");
        return;
    }
    std::string newNick = msg.Parameters[0];
    if (!isValidNickname(newNick))
    {
        std::string target = cli.getNickname().empty() ? "*" : cli.getNickname();
        sendReply(msg.fd, ":server 432 " + target + " " + newNick + " :Erroneous nickname\r\n");
        return;
    }
    for (std::map<size_t, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == newNick)
        {
            std::string target = cli.getNickname().empty() ? "*" : cli.getNickname();
            sendReply(msg.fd, ":server 433 " + target + " " + newNick + " :Nickname is already in use\r\n");
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
        
        // Kendisine bildir
        send(msg.fd, msgToSend.c_str(), msgToSend.length(), 0);
        
        // Ortak kanallardaki kullanıcılara bildir
        broadcastToCommonChannels(this, cli, msgToSend, msg.fd);
    }
    cli.setNickname(newNick);
    if (cli.isAuthenticated() && !cli.getUsername().empty() && 
        !cli.getNickname().empty() && !cli.isRegistered())
    {
        cli.setRegistered(true);
        sendReply(msg.fd, ":server 001 " + newNick + " :Welcome to the Internet Relay Network " + newNick + "!" + cli.getUsername() + "@server\r\n");
        sendReply(msg.fd, ":server 002 " + newNick + " :Your host is server, running version 1.0\r\n");
        sendReply(msg.fd, ":server 003 " + newNick + " :This server was created " + std::string(__DATE__) + "\r\n");
        sendReply(msg.fd, ":server 004 " + newNick + " server 1.0 o o\r\n");
    }
}
