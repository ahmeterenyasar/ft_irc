#include "../inc/server.hpp"
#include "../inc/client.hpp"


Server::Server() : _server_fd(-1), _port(0), _password("") {}

Server::Server(int port, const std::string& password) : _server_fd(-1), _port(port), _password(password) {
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
}

void Server::socket_initialization()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
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
    socket_configuration(); // setsocketopt (bind fail önlemek için ve fcntl ile non-blocking)
    start_sockaddr_struct(); // Initialize sockaddr_in structure
    server_bind();
    server_listen();
}

void Server::init_run()
{
    struct pollfd serverPollFd;
    serverPollFd.fd = _server_fd; // Sunucu soket dosya tanıtıcısı
    serverPollFd.events = POLLIN; // Gelen bağlantılar için dinle
    serverPollFd.revents = 0; // Başlangıçta olay yok
    _pollFds.push_back(serverPollFd); // Sunucu soketini pollFds vektörüne ekler
}

void Server::sendSimpleWelcome(int clientFd)
{
    // RFC 2812: Send NOTICE to unauthenticated clients
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
        socklen_t client_len = sizeof(client_address); // İstemci adres uzunluğu
        int client_fd = accept(_server_fd, (struct sockaddr*)&client_address, &client_len); // Yeni bağlantıyı kabul et
        if (client_fd < 0) 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break; // Tüm bağlantılar kabul edildi
            perror("Accept failed");
            break;
        } 
        int flags = fcntl(client_fd, F_GETFL, 0);  // Mevcut bayrakları al
        if (flags != -1) 
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        pollfd p; // Yeni pollfd yapısı
        p.fd = client_fd; // Yeni istemci soket dosya tanıtıcısı
        p.events = POLLIN; // Gelen veriler için dinle
        p.revents = 0; // Başlangıçta olay yok
        _pollFds.push_back(p); // Yeni istemci soketini pollFds vektörüne ekle
        _clients[client_fd] = Client(client_fd); // Yeni Client nesnesi oluştur ve ekle
    }
    sendSimpleWelcome(_pollFds.back().fd); // Yeni bağlanan istemciye hoş geldin mesajı gönder
}

void Server::disconnectClient(size_t index) 
{
    if (index >= _pollFds.size())// geçersiz indeks
        return;
    int client_fd = _pollFds[index].fd; // İstemci dosya tanıtıcısı
    close(client_fd); // İstemci soketini kapat
    _pollFds.erase(_pollFds.begin() + index); // pollFds vektöründen kaldır
    _inbuf.erase(client_fd); // İstemci verilerini sil
}

void Server::client_read(size_t fd, size_t index)
{
    char buf[512];

    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n == 0)
    {
        std::cout << "[DEBUG] Client closed connection fd=" << fd << "\n";
        disconnectClient(index);
        return;
    }
    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        perror("recv");
        disconnectClient(index);
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
        std::cout << "[IRC] fd=" << fd << " cmd=\"" << line << "\"\n";
        
        // TEST
        IRCMessage msg = parser(line, fd);
        executeCommand(msg);
    }
}


void Server::run() 
{
    init_run();
    while (42)
    {
        int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);
        if (pollCount < 0)
        {
             if (errno == EINTR)
                continue;
            perror("Poll failed");
            break;
        }
        for (size_t i = 0; i < _pollFds.size(); ++i )
        {
            if (_pollFds[i].revents == 0)
                continue;
            if (_pollFds[i].revents & POLLIN)
            {
                if (_pollFds[i].fd == _server_fd)
                {
                    if (_pollFds[i].revents & POLLIN)
                        accept_new_connection();
                }
                else
                {
                    if(_pollFds[i].revents & POLLIN)
                        client_read(_pollFds[i].fd, i); // İstemciden veri okuma
                    if(_pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                    {
                        disconnectClient(i); // İstemci bağlantısını kesme
                        --i;
                        continue;
                    }
                }
            }
        }
    }
}

std::string Server::getUserList(const Channel& channel) const
{
    std::string userList;
    std::vector<int> members = channel.getMembers();
    
    for (size_t i = 0; i < members.size(); ++i)
    {
        int fd = members[i];
        std::map<int, Client>::const_iterator it = _clients.find(fd);
        if (it == _clients.end())
            continue; // FD bulunamazsa atla
        
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