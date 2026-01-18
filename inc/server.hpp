#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <unistd.h>


class Server
{
	private:
		int			_port;
		std::string	_password;

	public:
		Server();
		Server(int port, std::string& password);
		Server(Server &other);
		Server& operator=(const Server& other);
		~Server();

		void init();
		void run();
};


#endif