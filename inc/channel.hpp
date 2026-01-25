#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>

class Channel
{
private:
    std::string             _name;
    std::string             _topic;
    std::string             _key;
    std::vector<int>        _members;
    std::vector<std::string> _inviteList;
    bool                    _inviteOnly;
    bool                    _topicRestricted;
    int                     _userLimit;

public:
    Channel() : _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}
    Channel(const std::string& name) : _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}
    ~Channel() {}

    // Getters
    std::string getName() const { return _name; }
    std::string getTopic() const { return _topic; }
    std::string getKey() const { return _key; }
    bool isInviteOnly() const { return _inviteOnly; }
    bool isTopicRestricted() const { return _topicRestricted; }
    int getUserLimit() const { return _userLimit; }
    int getUserCount() const { return _members.size(); }
    std::vector<int> getMembers() const { return _members; }

    // Setters
    void setTopic(const std::string& topic) { _topic = topic; }
    void setKey(const std::string& key) { _key = key; }
    void setInviteOnly(bool status) { _inviteOnly = status; }
    void setTopicRestricted(bool status) { _topicRestricted = status; }
    void setUserLimit(int limit) { _userLimit = limit; }

    // Member management
    void addUser(int fd)
    {
        if (std::find(_members.begin(), _members.end(), fd) == _members.end())
            _members.push_back(fd);
    }

    void removeUser(int fd)
    {
        std::vector<int>::iterator it = std::find(_members.begin(), _members.end(), fd);
        if (it != _members.end())
            _members.erase(it);
    }

    bool hasUser(int fd) const
    {
        return std::find(_members.begin(), _members.end(), fd) != _members.end();
    }

    // Invite management
    void addInvite(const std::string& nick)
    {
        if (std::find(_inviteList.begin(), _inviteList.end(), nick) == _inviteList.end())
            _inviteList.push_back(nick);
    }

    void removeInvite(const std::string& nick)
    {
        std::vector<std::string>::iterator it = std::find(_inviteList.begin(), _inviteList.end(), nick);
        if (it != _inviteList.end())
            _inviteList.erase(it);
    }

    bool isInvited(const std::string& nick) const
    {
        return std::find(_inviteList.begin(), _inviteList.end(), nick) != _inviteList.end();
    }

    // Mode string for MODE query
    std::string getModeString() const
    {
        std::string modes = "+";
        if (_inviteOnly) modes += "i";
        if (_topicRestricted) modes += "t";
        if (!_key.empty()) modes += "k";
        if (_userLimit > 0) modes += "l";
        return modes;
    }

    // User list for NAMES reply (placeholder - needs nick mapping)
    std::string getUserList() const
    {
        // This would need access to client nicknames
        // For now, return empty string - should be implemented with Server access
        return "";
    }
};

#endif
