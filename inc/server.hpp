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

class Server
{
	private:
		int  _server_fd; // Sunucu soket dosya tanıtıcısı
		int			_port; // Sunucu portu
		std::string	_password; // Sunucu parolası
		struct sockaddr_in _serverAddr; // Sunucu adres bilgisi
		std::vector<struct pollfd> _pollFds; // poll için kullanılan dosya tanıtıcıları
		int bindValue; // bind fonksiyonunun dönüş değeri

	public:
		Server();
		Server(int port, const std::string& password);
		Server(const Server &other);
		Server& operator=(const Server& other);
		~Server();

		void init(); // Sunucuyu başlatmak için
		void start_sockaddr_struct(); // sockaddr_in yapısını başlatmak için
        void socket_initialization();
        void socket_configuration();
        void server_bind();
        void server_listen();
        void run();
};

// debug fonksiyonu
std::string familyToString(int family);

// utils fonk
bool isValidPort(char *port_str);

#endif