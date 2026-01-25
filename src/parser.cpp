#include "../inc/server.hpp"
#include "../inc/client.hpp"

IRCMessage::IRCMessage() : Prefix(""), Command(""), fd(0) {
}

IRCMessage parser(std::string line, size_t fd)
{
    IRCMessage msg;
    msg.fd = fd;

    std::cout << "Parsing line: [" << line << "] from FD: " << fd << std::endl;
    // Boş satır kontrolü
    if (line.empty())
        return msg;

    size_t pos = 0;

    // 1. Prefix kontrolü (: ile başlıyorsa)
    if (line[0] == ':')
    {
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos)
            return msg; // Sadece prefix var, geçersiz mesaj
        msg.Prefix = line.substr(1, spacePos - 1); // ':' karakterini atla
        pos = spacePos + 1;
    }

    // Baştaki boşlukları atla
    while (pos < line.size() && line[pos] == ' ')
        pos++;

    // 2. Command kısmını ayır (ilk kelime)
    size_t cmdEnd = line.find(' ', pos);
    if (cmdEnd == std::string::npos)
    {
        // Sadece command var, parametre yok
        msg.Command = line.substr(pos);
        return msg;
    }
    msg.Command = line.substr(pos, cmdEnd - pos);
    pos = cmdEnd + 1;

    // 3. Parametreleri işle
    while (pos < line.size())
    {
        // Baştaki boşlukları atla
        while (pos < line.size() && line[pos] == ' ')
            pos++;

        if (pos >= line.size())
            break;

        // RFC 2812: Maksimum 15 parametre
        // Eğer 15. parametreye ulaşıldıysa, geri kalan her şeyi son parametre olarak al
        if (msg.Parameters.size() >= 15)
        {
            msg.Parameters.push_back(line.substr(pos));
            break;
        }

        // Trailing parametre kontrolü (: ile başlıyorsa)
        if (line[pos] == ':')
        {
            // : karakterinden sonraki her şey tek bir parametre
            msg.Parameters.push_back(line.substr(pos + 1));
            break;
        }
        else
        {
            // Normal parametre (bir sonraki boşluğa kadar)
            size_t paramEnd = line.find(' ', pos);
            if (paramEnd == std::string::npos)
            {
                msg.Parameters.push_back(line.substr(pos));
                break;
            }
            msg.Parameters.push_back(line.substr(pos, paramEnd - pos));
            pos = paramEnd + 1;
        }
    }

    // DEBUG
    std::cout << "========== IRC MESSAGE DEBUG ==========" << std::endl;
    std::cout << "FD: " << msg.fd << std::endl;
    std::cout << "Prefix: [" << msg.Prefix << "]" << std::endl;
    std::cout << "Command: [" << msg.Command << "]" << std::endl;
    std::cout << "Parameters (" << msg.Parameters.size() << "):" << std::endl;
    for (size_t i = 0; i < msg.Parameters.size(); i++)
        std::cout << "  [" << i << "]: [" << msg.Parameters[i] << "]" << std::endl;
    std::cout << "=======================================" << std::endl;

    return msg;
}