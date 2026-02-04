/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCMessage.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:42:31 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:42:32 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>

class IRCMessage
{
  public:
    IRCMessage();

    std::string Prefix;
    std::string Command;
    std::vector<std::string> Parameters;
    size_t fd;
};
IRCMessage parser(std::string line, size_t fd);
#endif