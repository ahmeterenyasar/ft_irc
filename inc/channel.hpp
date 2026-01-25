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
    Channel();
    Channel(const std::string& name);
    ~Channel() {};

    // Getters
    std::string getName() const;
    std::string getTopic() const;
    std::string getKey() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    int getUserLimit() const;
    int getUserCount() const;
    std::vector<int> getMembers() const;

    // Setters
    void setTopic(const std::string& topic);
    void setKey(const std::string& key);
    void setInviteOnly(bool status);
    void setTopicRestricted(bool status);
    void setUserLimit(int limit);

    // Member management
    void addUser(int fd);
    void removeUser(int fd);
    bool hasUser(int fd) const;
    // Invite management
    void addInvite(const std::string& nick);
    void removeInvite(const std::string& nick);
    bool isInvited(const std::string& nick) const;

    // Mode string for MODE query
    std::string getModeString() const;
};

#endif
