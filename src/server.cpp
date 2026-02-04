#include "../inc/server.hpp"
#include "../inc/client.hpp"

bool Server::_signalReceived = false;

Server::Server() : _server_fd(-1), _port(0), _password(""), _isShutdown(false) {}

Server::Server(int port, const std::string& password) : _server_fd(-1), _port(port), _password(password), _isShutdown(false) {
}

Server::Server(const Server& other) {
    *this = other;
}

Server::~Server() {
    shutdown();
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
}

void Server::socket_initialization()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
}

void Server::socket_configuration() 
{
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(_server_fd);
        exit(EXIT_FAILURE);
    }

    if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl failed");
        close(_server_fd);
        exit(EXIT_FAILURE);
    }
}

void  Server::server_bind()
{
    if (bind(_server_fd, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0) {
        perror("Bind failed");
        close(_server_fd);
        exit(EXIT_FAILURE);
    }
}

bool Server::haschannel(std::string name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i].getName() == name)
            return true;
    }
    return false;
}


void Server::server_listen()
{
    if (listen(_server_fd, SOMAXCONN) < 0) {
        perror("Listen failed");
        close(_server_fd);
        exit(EXIT_FAILURE);
    }
}

void  Server::init() 
{
    socket_initialization();
    socket_configuration();
    start_sockaddr_struct();
    server_bind();
    server_listen();
}

void Server::init_run()
{
    struct pollfd serverPollFd;
    serverPollFd.fd = _server_fd;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;
    _pollFds.push_back(serverPollFd);
}

void Server::sendSimpleWelcome(int clientFd)
{
    std::string welcomeMsg;
    
    welcomeMsg = "NOTICE AUTH :*** Looking up your hostname...\r\n";
    welcomeMsg += "NOTICE AUTH :*** Checking Ident\r\n";
    welcomeMsg += "NOTICE AUTH :*** Please authenticate using PASS command\r\n";

    if (send(clientFd, welcomeMsg.c_str(), welcomeMsg.length(), 0) == -1) {
        std::cerr << "Error: Failed to send welcome message" << std::endl;
    }
}

void Server::accept_new_connection() 
{
    while(41)
    {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_fd = accept(_server_fd, (struct sockaddr*)&client_address, &client_len); 
        if (client_fd < 0) 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            perror("Accept failed");
            break;
        } 
        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags != -1) 
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        pollfd p;
        p.fd = client_fd;
        p.events = POLLIN;
        p.revents = 0;
        _pollFds.push_back(p);
        _clients[client_fd] = Client(client_fd);
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
        _clients[client_fd].setHostname(client_ip);
        
        sendSimpleWelcome(_pollFds.back().fd); 
    }
}

void Server::disconnectClient(size_t index) 
{
    if (index >= _pollFds.size())
        return;
    int client_fd = _pollFds[index].fd;
    
    std::map<size_t, Client>::iterator clientIt = _clients.find(client_fd);
    if (clientIt != _clients.end())
    {
        std::vector<std::string> channels = clientIt->second.getChannels();
        for (size_t i = 0; i < channels.size(); ++i)
        {
            for (size_t j = 0; j < _channels.size(); ++j)
            {
                if (_channels[j].getName() == channels[i])
                {
                    _channels[j].removeUser(client_fd);
                    if (_channels[j].isOperator(client_fd))
                        _channels[j].removeOperator(client_fd);
                    
                    if (_channels[j].getUserCount() == 0)
                    {
                        _channels.erase(_channels.begin() + j);
                        break;
                    }
                    break;
                }
            }
        }
        _clients.erase(clientIt);
    }
    
    close(client_fd);
    _pollFds.erase(_pollFds.begin() + index);
    _inbuf.erase(client_fd);
}

void Server::client_read(size_t fd, size_t index)
{
    char buf[512];
    std::memset(buf, 0, sizeof(buf));

    ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
    if (n == 0)
    {
        disconnectClient(index);
        return;
    }
    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        disconnectClient(index);
        return;
    }
    if (_inbuf[fd].size() + n > 4096)
    {
        _inbuf[fd].clear();
        sendReply(fd, "ERROR :Input buffer overflow\r\n");
        return;
    }
    
    _inbuf[fd].append(buf, n);
    std::string &buffer = _inbuf[fd];
    size_t pos;
    while ((pos = buffer.find("\n")) != std::string::npos)
    {
        std::string line = buffer.substr(0, pos);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        buffer.erase(0, pos + 1);
        
        IRCMessage msg = parser(line, fd);
        executeCommand(msg);
    }
}


void Server::run() 
{
    init_run();
    while (!_signalReceived)
    {
        int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);
        if (pollCount < 0)
        {
             if (errno == EINTR)
             {
                if (_signalReceived)
                    break;
                continue;
             }
            perror("Poll failed");
            break;
        }
        for (size_t i = 0; i < _pollFds.size(); ++i)
        {
            if (_pollFds[i].revents == 0)
                continue;
            if (_pollFds[i].fd == _server_fd)
            {
                if (_pollFds[i].revents & POLLIN)
                    accept_new_connection();
            }
            else
            {
                if (_pollFds[i].revents & POLLIN)
                    client_read(_pollFds[i].fd, i);

                if (_pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                {
                    disconnectClient(i);
                    i--;
                }
            }
        }
    }
    
    std::cout << "\nPerforming graceful shutdown..." << std::endl;
    shutdown();
}

std::string Server::getUserList(const Channel& channel) const
{
    std::string userList;
    std::vector<size_t> members = channel.getMembers();
    
    for (size_t i = 0; i < members.size(); ++i)
    {
        size_t fd = members[i];
        std::map<size_t, Client>::const_iterator it = _clients.find(fd);
        if (it == _clients.end())
            continue;
        
        const Client& cli = it->second;
        std::string nick = cli.getNickname();
        
        if (cli.isOperator(channel.getName()))
            userList += "@";
        
        userList += nick;
        if (i < members.size() - 1)
            userList += " ";
    }
    return userList;
}

void Server::signalHandler(int signum)
{
    (void)signum;
    _signalReceived = true;
    std::cout << "\nShutdown signal received..." << std::endl;
}

void Server::shutdown()
{
    if (_isShutdown)
        return;
    _isShutdown = true;
    
    std::cout << "Shutting down server..." << std::endl;
    
    std::map<size_t, Client>::iterator it = _clients.begin();
    while (it != _clients.end())
    {
        size_t fd = it->first;
        std::string quitMsg = "ERROR :Server shutting down\r\n";
        send(fd, quitMsg.c_str(), quitMsg.length(), 0);
        close(fd);
        ++it;
    }
    
    _clients.clear();
    _channels.clear();
    _inbuf.clear();
    _pollFds.clear();
    
    if (_server_fd >= 0)
    {
        close(_server_fd);
        _server_fd = -1;
    }
    
    std::cout << "Server shutdown complete." << std::endl;
}