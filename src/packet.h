// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// packet.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PACKET_H
#define PACKET_H


#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <errno.h>
#include <time.h>

#include "./netlib/netinet.h"

#if defined(_POSIX_C_SOURCE)
#   include <unistd.h>
#   include <limits.h>
#   include <arpa/inet.h>
#   include <sys/mman.h>
#   include <sys/uio.h>
#elif defined(_WIN32)
//#include <winsock2.h>
#endif

extern int errno; // Declared in <errno.h>

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
