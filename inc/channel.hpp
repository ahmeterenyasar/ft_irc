#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "client.hpp"

class Channel
{
private:
    std::string             _name;
    std::string             _topic;
    std::string             _key;
    std::vector<size_t>        _members;
    std::vector<std::string>_inviteList;
    std::vector<size_t>     _operators; // Placeholder for operator list
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
    std::vector<size_t> getMembers() const;
    std::vector<size_t> getOperators() const; // Placeholder for operator list

    // Setters
    void setTopic(const std::string& topic);
    void setKey(const std::string& key);
    void setInviteOnly(bool status);
    void setTopicRestricted(bool status);
    void setUserLimit(int limit);

    // Member management
    void addOperator(size_t fd); // Placeholder for adding operator
    void addUser(size_t fd);
    void removeUser(size_t fd);
    bool hasUser(size_t fd) const;
    
    // Invite management
    void addInvite(const std::string& nick); // Adds a nick to the invite list
    void removeInvite(const std::string& nick); // Removes a nick from the invite list
    bool isOperator (size_t fd) const; // Placeholder for operator check
    bool isInvited(const std::string& nick) const; // Checks if a nick is invited

    // Mode string for MODE query
    std::string getModeString() const;
};

#endif
