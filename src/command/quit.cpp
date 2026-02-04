#include "../../inc/server.hpp"
#include "../../inc/client.hpp"
#include "../../inc/command_helpers.hpp"
#include <set>

void Server::quitCommand(IRCMessage& msg)
{
    Client& cli = _clients[msg.fd];
    std::string nick = cli.getNickname().empty() ? "*" : cli.getNickname();
    std::string user = cli.getUsername().empty() ? "*" : cli.getUsername();
    std::string host = cli.getHostname().empty() ? "localhost" : cli.getHostname();

    std::string quitMessage = "Client Quit";
    if (!msg.Parameters.empty())
        quitMessage = msg.Parameters[0];

    std::string quitMsg = ":" + nick + "!" + user + "@" + host + " QUIT :" + quitMessage + "\r\n";

    std::vector<std::string> channels = cli.getChannels();
    
    broadcastToCommonChannels(this, cli, quitMsg, msg.fd);
    removeUserFromAllChannels(this, msg.fd);
    cleanupEmptyChannels(this);
    
    sendReply(msg.fd, "ERROR :Closing Link: " + host + " (" + quitMessage + ")");
    
    for (size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == static_cast<int>(msg.fd))
        {
            close(_pollFds[i].fd);
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }
    _clients.erase(msg.fd);
}
