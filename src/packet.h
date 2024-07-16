// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// packet.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PACKET_H
#define PACKET_H


#include <stdlib.h>
#include <stdalign.h>
#include <time.h>

#if defined(_POSIX_C_SOURCE)
#   include "netinet.h"
#   include <netinet/in.h>
#   include <netinet/ip.h>
#   include <netinet/tcp.h>
#elif defined(_WIN32)
//#include <winsock2.h>
#endif

#define IP_PKT_MTU 1500 // Same as Ethernet II MTU (bytes)


struct pkt_args {
    int sock;
    in_addr_t src_ip_start;
    in_addr_t src_ip_end;
    //int src_port;
    in_addr_t dest_ip;
    int dest_port;
};


// Thread callback
int send_packets(void *arg);


#endif // PACKET_H

// ---------------------------------------------------------------------
// END OF FILE: packet.h
// ---------------------------------------------------------------------
