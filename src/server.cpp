#include "../inc/server.hpp"

Server::Server() : _port(0), _password("") {}

Server::Server(int port, const std::string& password) : _port(port), _password(password) {
}

Server::Server(const Server& other) {
    *this = other;
}

Server:: ~Server() {
    if (_server_fd >= 0) {
        close(_server_fd);
    }
}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        this->_port = other._port;
        this->_password = other._password;
    }
    return *this;
}   

void Server::start_sockaddr_struct() 
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));

    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_addr.s_addr = INADDR_ANY;
    _serverAddr.sin_port = htons(_port);

    std::cout << "========== SOCKET ADDRESS DEBUG ==========" << std::endl;
    std::cout << "IP Address : "<< inet_ntoa(_serverAddr.sin_addr) << std::endl;
    std::cout << "Port       : "<< ntohs(_serverAddr.sin_port) << std::endl;
    std::cout << "Family     : -> "<< familyToString(_serverAddr.sin_family) << std::endl;
    std::cout << "=========================================" << std::endl;
}

void  Server::init() 
{
    // Socket initialization
    _server_fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (_server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    start_sockaddr_struct(); // Initialize sockaddr_in structure
    bindValue = bind(_server_fd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr));
    if (bindValue < 0) {
        perror("Bind failed");
        close(_server_fd);
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server socket created, fd = " << _server_fd << std::endl;  
}