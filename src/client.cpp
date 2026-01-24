#include "../inc/server.hpp"
#include "../inc/client.hpp"

Client::Client(): _fd(-1), _nickname(""), _username(""), _realname(""), _hostname(""), _buffer(""), _authenticated(false), _registered(false) {}    
Client::Client(int fd): _fd(fd), _nickname(""), _username(""), _realname(""), _hostname(""), _buffer(""), _authenticated(false), _registered(false) {}
Client::~Client() {} 


int Client::getClientFd(void) const
{
    return _fd;
}
std::string Client::getNickname(void) const
{
    return _nickname;
}
std::string Client::getUsername(void) const
{
    return _username;
}
std::string Client::getRealname(void) const
{
    return _realname;
}
std::string Client::getHostname(void) const
{
    return _hostname;
}
std::string Client::getBuffer(void) const
{
    return _buffer;
}
bool Client::isAuthenticated(void) const
{
    return _authenticated;
}
bool Client::isRegistered(void) const
{
    return _registered;
}

std::vector<std::string> Client::getChannels(void) const
{
    return _channels;
}
bool Client::isOperator(const std::string& channel) const
{
    std::map<std::string, bool>::const_iterator it = _operator_status.find(channel);
    if (it != _operator_status.end())
        return it->second;
    return false;
}
void Client::setNickname(const std::string& nickname)
{
    _nickname = nickname;
}
void Client::setUsername(const std::string& username)
{
    _username = username;
}
void Client::setRealname(const std::string& realname)
{
    _realname = realname;
}
void Client::setHostname(const std::string& hostname)
{
    _hostname = hostname;
}
void Client::setAuthenticated(bool status)
{
    _authenticated = status;
}
void Client::setRegistered(bool status)
{   
    _registered = status;
}
void Client::setOperator(const std::string& channel, bool status)
{
    _operator_status[channel] = status;
}