/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:42:23 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:42:24 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
    std::vector<size_t>     _operators;
    bool                    _inviteOnly;
    bool                    _topicRestricted;
    int                     _userLimit;

public:
    Channel();
    Channel(const std::string& name);
    ~Channel() {};

    std::string getName() const;
    std::string getTopic() const;
    std::string getKey() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    int getUserLimit() const;
    int getUserCount() const;
    std::vector<size_t> getMembers() const;
    std::vector<size_t> getOperators() const;

    void setTopic(const std::string& topic);
    void setKey(const std::string& key);
    void setInviteOnly(bool status);
    void setTopicRestricted(bool status);
    void setUserLimit(int limit);

    void addOperator(size_t fd);
    void addUser(size_t fd);
    void removeUser(size_t fd);
    void removeOperator(size_t fd);
    bool hasUser(size_t fd) const;
    
    void addInvite(const std::string& nick);
    void removeInvite(const std::string& nick);
    bool isOperator (size_t fd) const;
    bool isInvited(const std::string& nick) const;

    std::string getModeString() const;
};

#endif
