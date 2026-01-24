#include "../inc/server.hpp"
#include "../inc/client.hpp"

void Server::passCommand(int fd, std::vector<std::string> params)
{
    Client *cli = getClientFd(fd);
    if (params.size() < 1)
    {
        // ERR_NEEDMOREPARAMS
        return;
    }
    if (cli->isRegistered())
    {
        // ERR_ALREADYREGISTERED
        return;
    }
    if (params[0] != this->_password)
    {
        // ERR_PASSWDMISMATCH
        cli->setAuthenticated(false);
        return;
    }
    cli->setAuthenticated(true);
}
