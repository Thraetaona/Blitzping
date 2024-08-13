// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// packet.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PACKET_H
#define PACKET_H


#include "./utils/intrins.h"
#include "./netlib/netinet.h"

#include <stddef.h>
#if __STDC_VERSION__ >= 201112L
#   include <stdalign.h>
#else
typedef union {
    long long int __long_long_int;
    long double __long_double;
    void *__void_ptr;
} max_align_t;
#endif
#include <limits.h>

#include <stdlib.h>
#include <errno.h>

#include <stdio.h>
#include <time.h>
// C11 threads (glibc >=2.28, musl >=1.1.5, Windows SDK >~10.0.22620)
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
#   include <threads.h>
int __attribute__((weak)) thrd_create(
    thrd_t *thr, thrd_start_t func, void *arg
);
int __attribute__((weak)) thrd_join(
    thrd_t thr, int *res
);
#endif

#if defined(_POSIX_C_SOURCE)
#   include <unistd.h>
#   include <sched.h>
#   if __has_include(<pthread.h>)
#       include <pthread.h>
#   endif
#   include <arpa/inet.h>
#   include <sys/mman.h>
#   include <sys/uio.h>
#elif defined(_WIN32)
//#include <winsock2.h>
#endif

extern int errno; // Declared in <errno.h>

#define IP_PKT_MTU 1500 // Same as Ethernet II MTU (bytes)
#define MAX_THREADS 100 // Arbitrary limit (TODO: Remove?)

struct pkt_args {
    int sock;
    int num_threads;
    ip_addr_t src_ip_start;
    ip_addr_t src_ip_end;
    int src_port;
    ip_addr_t dest_ip;
    int dest_port;
};


// Thread callback
int send_packets(struct pkt_args *args);


#endif // PACKET_H

// ---------------------------------------------------------------------
// END OF FILE: packet.h
// ---------------------------------------------------------------------
