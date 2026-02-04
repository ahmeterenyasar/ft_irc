/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayasar <ayasar@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:43:57 by ayasar            #+#    #+#             */
/*   Updated: 2026/02/04 16:43:58 by ayasar           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/server.hpp"

bool isValidPort(char *port_str)
{
    int port;

    for (int i = 0; port_str[i]; i++)
    {
        if (!isdigit(port_str[i]))
            return false;
    }
    port = std::atoi(port_str);
    if (port < 1 || port > 65535)
        return false;
    return true;
}