// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// program.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PROGRAM_H
#define PROGRAM_H


#include "./netlib/netinet.h"
#include "./cmdline/logger.h"
#include "./utils/endian.h"

#include <stdbool.h>
#include <stdint.h>


// It is better to contain everything within a single struct, as
// opposed to having a bunch of global variables all over the place.
//
// NOTE: I did not use bitfields here because they are not
// going to help with compile-time safety, and they'd also
// come with a performance cost and lack of addressability.
typedef struct ProgramArgs {
    int socket; // Socket descriptor
    // Parser Internals
    struct {
        osi_layer_t current_layer;
        osi_proto_t current_proto;
    } parser;
    // System Diagnostics
    struct {
        char *executable_name; // argv[0]
        bool unrecoverable_error;
        struct {
            endianness_t endianness;
        } compile;
        struct {
            endianness_t endianness;
            unsigned int num_cores;
            bool c11_threads;
            bool posix_threads;
        } runtime;
    } diagnostics;
    // General
    struct {
        bool opt_info;
        log_level_t logger_level;
    } general;
    // Advanced
    struct {
        bool bypass_checks;
        bool no_log_timestamp;
        unsigned int num_threads;
        bool native_threads;
        unsigned int buffer_size;
        bool no_async_sock;
        bool no_mem_lock;
        bool no_cpu_prefetch;
    } advanced;
    // IPv4
    struct ip_hdr *ipv4;
    struct {
        bool is_cidr;
        struct {
            ip_addr_t start;
            ip_addr_t end;
        } source_cidr;
        bool override_checksum;
        bool override_source;
        bool override_length;
    } ipv4_misc;
    // TODO: IPv6
    // TCP
    struct tcp_hdr *tcp;
    // TODO: UDP
    // TODO: ICMP
} program_args_t;


#endif // PROGRAM_H

// ---------------------------------------------------------------------
// END OF FILE: program.h
// ---------------------------------------------------------------------
