#include "../../inc/server.hpp"
#include "../../inc/client.hpp"

// RFC 2812 - QUIT command
// Syntax: QUIT [<quit message>]
// No numeric replies defined for successful QUIT
void Server::quitCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
    std::string user = cli.getUsername().empty() ? "*" : cli.getUsername();
    std::string host = cli.getHostname().empty() ? "localhost" : cli.getHostname();

    std::string quitMessage = "Client Quit";
    if (!msg.Parameters.empty())
        quitMessage = msg.Parameters[0];

    std::string quitMsg = ":" + nick + "!" + user + "@" + host + " QUIT :" + quitMessage;

    // Kullanıcının bulunduğu tüm kanallardaki diğer kullanıcılara bildir
    std::vector<std::string> channels = cli.getChannels();
    for (size_t i = 0; i < channels.size(); i++)
    {
        // channeldeki tüm üyelere QUIT mesajı gönder (kendisi hariç)
    }

    sendReply(msg.fd, "ERROR :Closing Link: " + host + " (" + quitMessage + ")");

    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        // if (_pollFds[i].fd == msg.fd)
        // {
        //     _clients.erase(msg.fd);
        //     disconnectClient(i);
        //     break;
        // }
    }
}
