#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <sstream>

// Ayarlar
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6667
#define BOT_PASS "1234"      // Sunucunun şifresi
#define BOT_NICK "SelamBot" // Botun adı

// Yardımcı fonksiyon: Sunucuya mesaj göndermek için
void sendRaw(int socket, std::string msg) {
    msg += "\r\n"; // IRC komutları \r\n ile bitmeli
    send(socket, msg.c_str(), msg.length(), 0);
    std::cout << "Sent: " << msg; // Loglamak için
}

int main(int argc, char const *argv[]) {
    // Port numarasını argüman olarak alabiliriz (Opsiyonel)
    int port = SERVER_PORT;
    if (argc == 2)
        port = atoi(argv[1]);

    // 1. Soket Oluşturma
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

    // 2. Sunucuya Bağlanma
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    // 3. Giriş İşlemleri (Handshake)
    // Sırasıyla: PASS -> NICK -> USER
    sendRaw(sock, "PASS " + std::string(BOT_PASS));
    sendRaw(sock, "NICK " + std::string(BOT_NICK));
    sendRaw(sock, "USER botuser 0 * :Bot Description");

    // 4. Dinleme Döngüsü
    char buffer[4096];
    while (true) {
        memset(buffer, 0, 4096);
        int bytesRead = recv(sock, buffer, 4096, 0);
        if (bytesRead <= 0) break;

        std::string data(buffer);
        std::cout << "Received: " << data; // Gelen veriyi gör

        // Satır satır işleme (buffer birden fazla satır içerebilir)
        std::stringstream ss(data);
        std::string line;
        
        while (std::getline(ss, line)) {
            if (!line.empty() && line[line.length()-1] == '\r')
                line.erase(line.length()-1); // \r temizle

            // A. PING Kontrolü (Sunucu "Ölmedin dimi?" derse PONG dönmeliyiz)
            if (line.find("PING") == 0) {
                std::string token = line.substr(5);
                sendRaw(sock, "PONG " + token);
            }

            // B. Mesaj Kontrolü (PRIVMSG)
            // Format: :Nick!User@Host PRIVMSG #kanal :Mesajın kendisi
            size_t privmsgPos = line.find(" PRIVMSG ");
            if (privmsgPos != std::string::npos) {
                
                // Gönderen kişinin Nick'ini bul (:Nick!...)
                size_t exclamPos = line.find("!");
                std::string senderNick = line.substr(1, exclamPos - 1);

                // Mesajın içeriğini bul (: işaretinden sonrası)
                size_t colonPos = line.find(" :", privmsgPos);
                if (colonPos != std::string::npos) {
                    std::string messageContent = line.substr(colonPos + 2);
                    
                    // Kanalı veya hedefi bul
                    // PRIVMSG <hedef> :mesaj
                    size_t targetStart = privmsgPos + 9;
                    size_t targetEnd = colonPos;
                    std::string target = line.substr(targetStart, targetEnd - targetStart);

                    // Eğer özelden yazıldıysa cevap Nick'e, kanaldan yazıldıysa Kanala dönsün
                    std::string replyTarget = (target[0] == '#') ? target : senderNick;

                    // C. "Selam" Kelimesini Ara (Büyük/Küçük harf duyarlılığı olmadan bakmak daha iyi olur ama basit tutalım)
                    if (messageContent.find("Selam") != std::string::npos || 
                        messageContent.find("selam") != std::string::npos) {
                        
                        // CEVAP VER!
                        std::string reply = "PRIVMSG " + replyTarget + " :Merhaba " + senderNick + ", nasılsın?";
                        sendRaw(sock, reply);
                    }
                }
            }
        }
    }

    close(sock);
    return 0;
}