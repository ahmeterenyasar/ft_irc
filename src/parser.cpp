/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:52 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:53 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/server.hpp"
#include "../inc/client.hpp"

IRCMessage::IRCMessage() : Prefix(""), Command(""), fd(0) {
}

IRCMessage parser(std::string line, size_t fd)
{
    IRCMessage msg;
    msg.fd = fd;

    if (line.empty())
        return msg;

    size_t pos = 0;

    if (line[0] == ':')
    {
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos)
            return msg;
        msg.Prefix = line.substr(1, spacePos - 1);
        pos = spacePos + 1;
    }

    while (pos < line.size() && line[pos] == ' ')
        pos++;

    size_t cmdEnd = line.find(' ', pos);

    if (cmdEnd == std::string::npos)
    {
        msg.Command = line.substr(pos);
        return msg;
    }

    msg.Command = line.substr(pos, cmdEnd - pos);
    pos = cmdEnd + 1;

    while (pos < line.size())
    {
        while (pos < line.size() && line[pos] == ' ')
            pos++;

        if (pos >= line.size())
            break;

        if (msg.Parameters.size() >= 15)
        {
            msg.Parameters.push_back(line.substr(pos));
            break;
        }

        if (line[pos] == ':')
        {
            msg.Parameters.push_back(line.substr(pos + 1));
            break;
        }
        else
        {
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

    return msg;
}