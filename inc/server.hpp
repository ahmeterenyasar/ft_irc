#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <arpa/inet.h> // inet_pton ve inet_ntop için IP adresi dönüşümleri için kullanılan kütüphane
#include <sys/socket.h> // socket oluşturmak için 
#include <netinet/in.h> // sockaddr_in yapısı için
#include <stdexcept>
#include <poll.h>	// poll için
#include <fcntl.h> // fcntl için 
#include <cerrno> // errno için
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <sstream>
#include <cctype>
#include "client.hpp"
#include "IRCMessage.hpp"
#include "channel.hpp"

class Server
{
	private:
		int  _server_fd; // Sunucu soket dosya tanıtıcısı
		int			_port; // Sunucu portu
		std::string	_password; // Sunucu parolası
		struct sockaddr_in _serverAddr; // Sunucu adres bilgisi
		std::vector<struct pollfd> _pollFds; // poll için kullanılan dosya tanıtıcıları
		std::map<int, std::string> _inbuf; // İstemcilerden gelen verileri depolamak için
		std::map<size_t, Client> _clients; // Bağlı istemciler
		std::vector<Channel> _channels; // IRC kanalları

	public:
		Server();
		Server(int port, const std::string& password);
		Server(const Server &other);
		Server& operator=(const Server& other);
		~Server();

		void init(); // Sunucuyu başlatmak için
		void start_sockaddr_struct(); // sockaddr_in yapısını başlatmak için
        void socket_initialization(); // socket oluşturma
        void socket_configuration(); // setsockopt ve fcntl ayarları
        void server_bind(); // bind işlemi
        void server_listen(); // listen işlemi
		void init_run(); // pollfd yapılarını başlatmak için
        void run(); // Sunucuyu çalıştırmak için
		void accept_new_connection(); // Yeni bağlantıları kabul etmek için
		void disconnectClient(size_t index); // İstemci bağlantısını kesmek için
		void client_read(size_t fd, size_t index); // İstemciden veri okumak için
		void sendSimpleWelcome(int clientFd);
		
		// Commands Section - Utility
		void sendReply(int fd, const std::string &reply);
		void executeCommand(IRCMessage& msg);

		// Commands Section - Connection Registration
		void passCommand(IRCMessage& msg);
		void nickCommand(IRCMessage& msg);
		void cmdUser(IRCMessage& msg);
		void quitCommand(IRCMessage& msg);

		// Commands Section - Channel Operations
		void joinCommand(IRCMessage& msg);
		void partCommand(IRCMessage& msg);
		void topicCommand(IRCMessage& msg);
		void kickCommand(IRCMessage& msg);
		void inviteCommand(IRCMessage& msg);

		// Commands Section - Mode
		void modeCommand(IRCMessage& msg);
		void handleChannelMode(IRCMessage& msg, const std::string& nick, const std::string& channel);
		void handleUserMode(IRCMessage& msg, const std::string& nick, const std::string& target);

		// Commands Section - Messaging
		void privmsgCommand(IRCMessage& msg);
		void noticeCommand(IRCMessage& msg);
		bool haschannel(std::string name);

		// User list for NAMES reply (placeholder - needs nick mapping)
    	std::string getUserList(const Channel& channel) const;
};

// debug fonksiyonu
std::string familyToString(int family);

// utils fonk
bool isValidPort(char *port_str);

#endif