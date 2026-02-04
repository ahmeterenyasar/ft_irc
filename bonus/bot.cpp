#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <cerrno>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6667
#define BOT_PASS "123"
#define BOT_NICK "SelamBot"

bool isRunning = true;
int sock = -1;

void signalHandler(int signum) {
    (void)signum;
    std::cout << "\nInterrupt signal received. Shutting down..." << std::endl;
    isRunning = false;
}

void sendRaw(int socket, std::string msg) {
    msg += "\r\n";
    if (send(socket, msg.c_str(), msg.length(), 0) == -1) {
        std::cerr << "Send failed!" << std::endl;
    }
}

int main(int argc, char const *argv[]) {
    int port = SERVER_PORT;
    std::string password = BOT_PASS;

    if (argc >= 2)
        port = atoi(argv[1]);
    if (argc >= 3)
        password = argv[2];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << "Error setting up signal handler" << std::endl;
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Connected to server! Bot starting..." << std::endl;

    sendRaw(sock, "PASS " + password);
    sendRaw(sock, "NICK " + std::string(BOT_NICK));
    sendRaw(sock, "USER botuser 0 * :Bot Description");

    char tempBuffer[4096];
    std::string incompleteData = "";

    while (isRunning) {
        memset(tempBuffer, 0, 4096);
        int bytesRead = recv(sock, tempBuffer, 4095, 0);

        if (bytesRead < 0) {
            if (errno == EINTR) {
                break; 
            }
            std::cerr << "Recv error" << std::endl;
            break;
        }
        if (bytesRead == 0) {
            std::cout << "Server closed connection." << std::endl;
            break;
        }

        incompleteData.append(tempBuffer);
        size_t pos = 0;
        while ((pos = incompleteData.find("\r\n")) != std::string::npos) {
            std::string line = incompleteData.substr(0, pos);
            incompleteData.erase(0, pos + 2);

            std::cout << "Received: " << line << std::endl;

            if (line.find("PING") == 0) {
                std::string token = line.substr(5);
                sendRaw(sock, "PONG " + token);
            }

            if (line.find(" PRIVMSG ") != std::string::npos) {
                size_t exclamPos = line.find("!");
                size_t privmsgPos = line.find(" PRIVMSG ");
                
                if (exclamPos != std::string::npos && privmsgPos != std::string::npos) {
                    std::string senderNick = line.substr(1, exclamPos - 1);
                    size_t targetStart = privmsgPos + 9;
                    size_t msgStart = line.find(" :", targetStart);
                    
                    if (msgStart != std::string::npos) {
                        std::string target = line.substr(targetStart, msgStart - targetStart);
                        std::string messageContent = line.substr(msgStart + 2);
                        std::string replyTarget = (target[0] == '#') ? target : senderNick;

                        if (senderNick != BOT_NICK) {
                            if (messageContent.find("Selam") != std::string::npos || 
                                messageContent.find("selam") != std::string::npos) {
                                std::string reply = "PRIVMSG " + replyTarget + " :Merhaba " + senderNick + ", nasılsın?";
                                sendRaw(sock, reply);
                            }
                        }
                    }
                }
            }
        }
    }

    close(sock);
    std::cout << "Bot stopped cleanly." << std::endl;
    return 0;
}