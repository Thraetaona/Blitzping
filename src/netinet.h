// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// netinet.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef NETINET_H
#define NETINET_H


// NOTE: Unfortunately, <netinet/ip.h> is not a part of the POSIX
// standard, while <netinet/in.h> and <netinet/tcp.h> somehow are.
// However, even the latter are rather useless, because most of their
// definitions are hidden behind non-standard BSD or System V-specific
// feature flags.  In any case, the headers defined there are far
// too outdated, still referring to many fields as "reserved."
// For that reason, and for future compatibility with Win32,
// it is better to define our own protocol header structures.


#include <stdint.h>


#if defined(__BYTE_ORDER__) && !(defined(__LITTLE_ENDIAN__) \
                                || defined(__BIG_ENDIAN__))
#   if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#       define __LITTLE_ENDIAN__
#   elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#       define __BIG_ENDIAN__
#   else
#       pragma message("Unknown endianness; \n" \
            "define \'__LITTLE_ENDIAN__\' or \'__BIG_ENDIAN__\'.")
#   endif
#endif


// TCP flag definitions using binary bitflags
#define TCP_FIN 0b00000001  // Finish
#define TCP_SYN 0b00000010  // Synchronize
#define TCP_RST 0b00000100  // Reset
#define TCP_PSH 0b00001000  // Push
#define TCP_ACK 0b00010000  // Acknowledgment
#define TCP_URG 0b00100000  // Urgent
#define TCP_ECE 0b01000000  // ECN-Echo
#define TCP_CWR 0b10000000  // Congestion Window Reduced


struct tcp_hdr {
    union { // Different views of the TCP header
        struct { // View 1
            uint16_t sport;        // Source port number
            uint16_t dport;        // Destination port number
            uint32_t seqnum;       // Sequence number
            uint32_t acknum;       // Acknowledgement number
#if defined(__LITTLE_ENDIAN__)
            uint8_t  reserved : 4; // Reserved (unused) bits
            uint8_t  dataofs  : 4; // Offset to data/options
#elif defined(__BIG_ENDIAN__)
            uint8_t  dataofs  : 4; // Offset to data/options
            uint8_t  reserved : 4; // Reserved (unused) bits
#endif
            uint8_t  flags;        // TCP flags
            uint16_t window;       // Window size
            uint16_t chksum;       // Checksum
            uint16_t urgptr;       // Pointer to urgent data
            uint32_t options[10];    // Options (0-320 bits)
        };
        struct { // View 2 (with expanded flags / shortened names)
            uint16_t spt;
            uint16_t dpt;
            uint32_t sqn;
            uint32_t akn;
#if defined(__LITTLE_ENDIAN__)
            uint8_t  rsv : 4;
            uint8_t  dat : 4;
            uint8_t  fin : 1;
            uint8_t  syn : 1;
            uint8_t  rst : 1;
            uint8_t  psh : 1;
            uint8_t  ack : 1;
            uint8_t  urg : 1;
            uint8_t  ece : 1;
            uint8_t  cwr : 1;
#elif defined(__BIG_ENDIAN__)
            uint8_t  dat : 4;
            uint8_t  rsv : 4;
            uint8_t  cwr : 1;
            uint8_t  ece : 1;
            uint8_t  urg : 1;
            uint8_t  ack : 1;
            uint8_t  psh : 1;
            uint8_t  rst : 1;
            uint8_t  syn : 1;
            uint8_t  fin : 1;
#endif
            uint16_t wnd;
            uint16_t chk;
            uint16_t ptr;
            uint32_t opt[10]; // TODO: Can we expand this out, too?
        };
    };
};


#endif // NETINET_H

// ---------------------------------------------------------------------
// END OF FILE: netinet.h
// ---------------------------------------------------------------------
