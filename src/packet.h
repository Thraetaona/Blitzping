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

#include "netinet.h"

#if defined(_POSIX_C_SOURCE)
#   include <arpa/inet.h>
#elif defined(_WIN32)
//#include <winsock2.h>
#endif

#define IP_PKT_MTU 1500 // Same as Ethernet II MTU (bytes)


struct pkt_args {
    int sock;
    ip_addr_t src_ip_start;
    ip_addr_t src_ip_end;
    //int src_port;
    ip_addr_t dest_ip;
    int dest_port;
};


// Thread callback
int send_packets(void *arg);


#endif // PACKET_H

// ---------------------------------------------------------------------
// END OF FILE: packet.h
// ---------------------------------------------------------------------
