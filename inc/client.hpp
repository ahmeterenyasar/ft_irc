

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>

class Client
{
private:
    int                     _fd;
    std::string             _nickname;
    std::string             _username;
    std::string             _realname;
    std::string             _hostname;
    std::string             _buffer;
    bool                    _authenticated;
    bool                    _registered;
    std::vector<std::string> _channels;
    std::map<std::string, bool> _operator_status;

public:
    Client();
    Client(int fd);
    ~Client();

    // Getters
    int                 getClientFd(void) const;
    std::string         getNickname(void) const;
    std::string         getUsername(void) const;
    std::string         getRealname(void) const;
    std::string         getHostname(void) const;
    std::string         getBuffer(void) const;
    bool                isAuthenticated(void) const;
    bool                isRegistered(void) const;
    std::vector<std::string> getChannels(void) const;
    bool                isOperator(const std::string& channel) const;

    // Setters
    void                setNickname(const std::string& nickname);
    void                setUsername(const std::string& username);
    void                setRealname(const std::string& realname);
    void                setHostname(const std::string& hostname);
    void                setAuthenticated(bool status);
    void                setRegistered(bool status);
    void                setOperator(const std::string& channel, bool status);

    // Buffer management
    void                appendBuffer(const std::string& data);
    void                clearBuffer(void);
    bool                hasCompleteMessage(void) const;
    std::string         getNextMessage(void);

    // Channel management
    void                joinChannel(const std::string& channel);
    void                leaveChannel(const std::string& channel);
    bool                isInChannel(const std::string& channel) const;
};

#endif
