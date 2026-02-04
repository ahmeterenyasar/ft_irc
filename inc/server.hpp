#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <stdexcept>
#include <poll.h>	
#include <fcntl.h> 
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <sstream>
#include <cctype>
#include <csignal>
#include "client.hpp"
#include "IRCMessage.hpp"
#include "channel.hpp"

class Server
{
	private:
		int  _server_fd;
		int			_port; 
		std::string	_password;
		struct sockaddr_in _serverAddr;
		std::vector<struct pollfd> _pollFds; 
		std::map<int, std::string> _inbuf; 
		std::map<size_t, Client> _clients; 
		std::vector<Channel> _channels; 
		static bool _signalReceived; 
	bool _isShutdown;
public:
	Server();		Server(int port, const std::string& password);
		Server(const Server &other);
		Server& operator=(const Server& other);
		~Server();

		void init();
		void start_sockaddr_struct();
        void socket_initialization(); 
        void socket_configuration(); 
        void server_bind();
        void server_listen(); 
		void init_run();
        void run();
		void accept_new_connection(); 
		void disconnectClient(size_t index);
		void client_read(size_t fd, size_t index); 
		void sendSimpleWelcome(int clientFd);
		void shutdown(); 
		static void signalHandler(int signum);
		
		void sendReply(int fd, const std::string &reply);
		void executeCommand(IRCMessage& msg);

		void passCommand(IRCMessage& msg);
		void nickCommand(IRCMessage& msg);
		void cmdUser(IRCMessage& msg);
		void quitCommand(IRCMessage& msg);
		void capCommand(IRCMessage& msg);
		void pingCommand(IRCMessage& msg);
		void whoCommand(IRCMessage& msg);
		void listCommand(IRCMessage& msg);

		void joinCommand(IRCMessage& msg);
		void partCommand(IRCMessage& msg);
		void topicCommand(IRCMessage& msg);
		void kickCommand(IRCMessage& msg);
		void inviteCommand(IRCMessage& msg);

		void modeCommand(IRCMessage& msg);
		void handleChannelMode(IRCMessage& msg, const std::string& nick, const std::string& channel);
		void handleUserMode(IRCMessage& msg, const std::string& nick, const std::string& target);

		void privmsgCommand(IRCMessage& msg);
		void noticeCommand(IRCMessage& msg);
		bool haschannel(std::string name);

    	std::string getUserList(const Channel& channel) const;
		
		std::map<size_t, Client>& getClients() { return _clients; }
		std::vector<Channel>& getChannels() { return _channels; }
};

bool isValidPort(char *port_str);

#endif