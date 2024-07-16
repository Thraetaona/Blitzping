// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// parser.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PARSER_H
#define PARSER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_POSIX_C_SOURCE)
#   include <arpa/inet.h>
#elif defined(_WIN32)
//#   include <winsock2.h>
#endif

// A CIDR notation string is in the following format xxx.xxx.xxx.xxx/xx
// (e.g., 255.255.255.255/32); there can be a total of 18 characters.
#define MAX_IP_CIDR_LENGTH 18 // xxx.xxx.xxx.xxx/xx
#define MAX_IP_PORT_LENGTH 21 // xxx.xxx.xxx.xxx:xxxxx

#define PORT_MIN 0
#define PORT_MAX 65535 // 2^16 - 1


struct ip_addr_range {
    in_addr_t start;
    in_addr_t end;
};

struct program_args {
    int num_threads;
    struct ip_addr_range src_ip_range;
    in_addr_t dest_ip;
    int dest_port;
};


int parse_ip_cidr(const char *cidr_str, struct ip_addr_range *ip_range);
int parse_ip_port(const char *ip_port_str, in_addr_t *ip, int *port);
int parse_args(int argc, char const *argv[], struct program_args *args);


#endif // PARSER_H

// ---------------------------------------------------------------------
// END OF FILE: parser.h
// ---------------------------------------------------------------------
