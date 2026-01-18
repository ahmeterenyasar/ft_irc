#include "../inc/server.hpp"


Server::Server(int port, std::string& password) : _port(port), _password(password) {}

Server::Server(Server& other) {
    *this = other;
}

void Server::init() 
{
    // Socket initialization
}