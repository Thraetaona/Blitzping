// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// parser.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PARSER_H
#define PARSER_H


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "./netlib/netinet.h"

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


// TODO: use ifndef to enable/disable num-threads here.
// TODO: Do I use \r\n here, or continue relying on libc?
//
// NOTE: Unfortunately, C preprocessor is unable to include actual .txt
// files (which could otherwise be holding this text) into strings.
// (C23 is apparently able to do this using the new #embed directive.)
static const char HELP_TEXT[] =
"Usage: blitzping [options]\n"
"\n"
"Options may use either -U (unix style), --gnu-style, or /dos-style\n"
"conventions, and the \"=\" sign may be omitted.\n"
"\n"
"Example:\n"
"  blitzping --num-threads=4 --proto=tcp --dest-ip=10.10.10.10\n"
"  blitzping --help=proto\n"
"\n"
"General:\n"
"  -? --help=<command>     Display this help message or more info.\n"
"  -! --about              Display information about the Program.\n"
"  -V --version            Display the Program version.\n"
"  -Q --quiet              Suppress all output except errors.\n"
"Advanced:\n"
"     --bypass-checks      Ignore system compatibility issues (e.g., \n"
"                          wrong endianness) if startup checks fail.\n"
"                          (May result in unexpected behavior.)\n"
"     --num-threads=<0-n>  Number of threads to poll the socket with.\n"
"                          (Default: system thread count; 0: disable\n"
"                          threading, run everything in main thread.)\n"
"     --no-async-sock      Use blocking (not asynchronous) sockets.\n"
"                          (Will severely hinder performance.)\n"
"     --no-mem-lock        Don't lock memory pages; allow disk swap.\n"
"                          (May reduce performance.)\n"
"     --no-prefetch        Don't prefetch packet buffer to CPU cache.\n"
"                          (May reduce performance.)\n"
"IPv4/6 Options:\n"
"     --source-ip=<addr>   IPv4/6 source IP address to spoof.\n"
"     --dest-ip=<addr>     IPv4/6 destination IP address.\n"
"     --ver=<4|6|0-15>     IP version to use/spoof (automatic).\n"
"     --proto=<...|0-255>  [Override] protocol number (e.g., tcp, 6)\n"
"                          (\"--help=proto\" for textual entries.)\n"
"     --len=<0-65535>      [Override] total packet (+data) length.\n"
"     --ttl=<0-255>        Time-to-live (hop-limit) for the packet.\n"
"IPv4 Options:\n"
"  -4 --ipv4               IPv4 mode; same as \"--ver=4\"\n"
"     --ihl=<0-15>         [Override] IPv4 header length in 32-bit\n"
"                          increments; minimum \"should\" be 5 (i.e.,\n"
"                          5x32 = 160 bits = 20 bytes) by standard.\n"
"     --tos=<0-255>        Type of Service; obsolete by DSCP+ECN.\n"
"     |                    ToS itself is divided into precedence,\n"
"     |                    throughput, reliability, cost, and mbz:\n"
"     | --prec=<...|0-7>     IP Precedence/priority (RFC 791).\n"
"     |                      (\"--help=prec\" for textual entries.)\n"
"     | --min-delay=<0|1>    Minimize delay\n"
"     | --max-tput =<0|1>    Maximize throughput\n"
"     | --max-rely =<0|1>    Maximize reliability\n"
"     | --min-cost =<0|1>    Minimize monetary cost (RFC 1349)\n"
"     | --must-zero=<0|1>    MBZ/Must-be-Zero (or must it?)\n"
"     --dscp=<...|0-64>    Differentiated Services Code Point\n"
"                          (\"--help=dscp\" for textual entries.)\n"
"     --ecn=<...|0-3>      Explicit Congestion Notification\n"
"                          (\"--help=ecn\" for textual entries.)\n"
"     --ident=<0-65535>    Packet identification (for fragmentation).\n"
"     --flags=<0-7>        Bitfield for IPv4 flags:\n"
"     | --evil-bit =<0|1>    RFC 3514 Evil/Reserved bit\n"
"     | --dont-frag=<0|1>    Don't Fragment (DF)\n"
"     | --more-frag=<0|1>    More Fragments (MF)\n"
"     --frag-ofs=<0-8191>  Fragment Offset\n"
"     --checksum=<0-65535> [Override] IPv4 header checksum.\n"
"     --options=<UNIMPLEMENTED>\n"
"IPv6 Options:\n"
"   [[UNIMPLEMENTED]]\n"
"  -6 --ipv6               IPv6 mode; same as \"--ver=6\"\n"
"     --next-header=proto  Alias to \"--proto\" for IPv6.\n"
"     --hop-limit=ttl      Alias to \"--ttl\" for IPv6.\n"
"     --flow-label=<0-1048575>\n"
"                          Flow Label (experimental; RFC 2460).\n"
"\n";


enum OptionKind {
    OPTION_UNKNOWN = 0,
    OPTION_THREADS,
    OPTION_SOURCE,
    OPTION_DEST_IP,
};

struct Option {
    const char *name;
    bool has_arg;
    enum OptionKind kind;
};

static const struct Option options[] = {
    {"\0", false, OPTION_UNKNOWN},
    {"num-threads", true, OPTION_THREADS},
    {"source-ip", true, OPTION_SOURCE},
    {"dest-ip", true, OPTION_DEST_IP},
};


struct ip_addr_range {
    ip_addr_t start;
    ip_addr_t end;
};

struct program_args {
    int num_threads;
    struct ip_addr_range src_ip_range;
    ip_addr_t dest_ip;
    int dest_port;
};


int parse_ip_cidr(const char *cidr_str, struct ip_addr_range *ip_range);
int parse_ip_port(const char *ip_port_str, ip_addr_t *ip, int *port);
int parse_args(int argc, char const *argv[], struct program_args *args);


#endif // PARSER_H

// ---------------------------------------------------------------------
// END OF FILE: parser.h
// ---------------------------------------------------------------------
