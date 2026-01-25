#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>

class IRCMessage
{
  public:
    IRCMessage();

    std::string Prefix;            // Sender (Nick or Server)
    std::string Command;           // COMMAND (e.g., "PRIVMSG" or "001")
    std::vector<std::string> Parameters;  // Parameter list
    size_t fd;                     // File descriptor of the sender
};


IRCMessage parser(std::string line, size_t fd);

// ahmed nabüyün la
#endif