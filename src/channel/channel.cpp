

#include "../../inc/channel.hpp"

Channel::Channel() : _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}
Channel::Channel(const std::string& name) : _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}

std::string Channel::getName() const 
{
    return _name;
}

std::string Channel::getTopic() const 
{
    return _topic; 
}

std::string Channel::getKey() const 
{ 
    return _key; 
}

bool Channel::isInviteOnly() const 
{ 
    return _inviteOnly; 
}

bool Channel::isTopicRestricted() const 
{
     return _topicRestricted; 
}

int Channel::getUserLimit() const 
{ 
    return _userLimit;
}

int Channel::getUserCount() const 
{ 
    return _members.size();
}

std::vector<int> Channel::getMembers() const 
{ 
    return _members; 
}
void Channel::setTopic(const std::string& topic) 
{
     _topic = topic; 
}
void Channel::setKey(const std::string& key)
{ 
    _key = key;
}
void Channel::setInviteOnly(bool status) 
{ 
    _inviteOnly = status;
}
void Channel::setTopicRestricted(bool status) 
{
    _topicRestricted = status;
}
void Channel::setUserLimit(int limit) 
{ 
    _userLimit = limit; 
}

void Channel::addUser(int fd)
{
    if (std::find(_members.begin(), _members.end(), fd) == _members.end())
        _members.push_back(fd);
}

void Channel::removeUser(int fd)
{
    std::vector<int>::iterator it = std::find(_members.begin(), _members.end(), fd);
    if (it != _members.end())
        _members.erase(it);
}

bool Channel::hasUser(int fd) const
{
    return std::find(_members.begin(), _members.end(), fd) != _members.end();
}

void Channel::addInvite(const std::string& nick)
{
    if (std::find(_inviteList.begin(), _inviteList.end(), nick) == _inviteList.end())
        _inviteList.push_back(nick);
}

void Channel::removeInvite(const std::string& nick)
{
    std::vector<std::string>::iterator it = std::find(_inviteList.begin(), _inviteList.end(), nick);
    if (it != _inviteList.end())
        _inviteList.erase(it);
}

bool Channel::isInvited(const std::string& nick) const
{
    return std::find(_inviteList.begin(), _inviteList.end(), nick) != _inviteList.end();
}

std::string Channel::getModeString() const // modları string olarak döndüren fonksiyon
{
    std::string modes = "+";
    if (_inviteOnly) 
        modes += "i";
    if (_topicRestricted) 
        modes += "t";
    if (!_key.empty()) 
        modes += "k";
    if (_userLimit > 0) 
        modes += "l";
    return modes;
}

