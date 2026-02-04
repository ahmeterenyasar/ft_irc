#ifndef COMMAND_HELPERS_HPP
#define COMMAND_HELPERS_HPP

#include "server.hpp"
#include <string>

bool checkRegistered(Server* server, IRCMessage& msg, const Client& cli);
bool checkMinParams(Server* server, IRCMessage& msg, const Client& cli, size_t minParams, const std::string& command);
bool checkChannelOperator(Server* server, IRCMessage& msg, const Client& cli, Channel* channel, const std::string& channelName);
bool checkUserInChannel(Server* server, IRCMessage& msg, const Client& cli, Channel* channel, const std::string& channelName);

Client* findClientByNick(Server* server, const std::string& nickname, size_t& fd);
Channel* findChannel(Server* server, const std::string& channelName);

void broadcastToChannel(Server* server, Channel* channel, const std::string& message, size_t excludeFd = 0);
void broadcastToCommonChannels(Server* server, const Client& cli, const std::string& message, size_t senderFd);

void removeUserFromAllChannels(Server* server, size_t fd);
void cleanupEmptyChannels(Server* server);

#endif
