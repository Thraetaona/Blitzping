// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// parser.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PARSER_H
#define PARSER_H


#include "./netlib/netinet.h"


#include <stdbool.h>
#include <limits.h>
#include <string.h>

#include <stdlib.h>
#include <errno.h>

#include <stdio.h>

#if defined(_POSIX_C_SOURCE)
#   include <arpa/inet.h>
#   include <sys/uio.h>
#   include <unistd.h>
#elif defined(_WIN32)
//#   include <winsock2.h>
#endif

// A CIDR notation string is in the following format xxx.xxx.xxx.xxx/xx
// (e.g., 255.255.255.255/32); there can be a total of 18 characters.
#define MAX_IP_CIDR_LENGTH 18 // xxx.xxx.xxx.xxx/xx
#define MAX_IP_PORT_LENGTH 21 // xxx.xxx.xxx.xxx:xxxxx

// TODO: Move to netlib
#define PORT_MIN 0
#define PORT_MAX 65535 // 2^16 - 1

extern int errno; // Declared in <errno.h>

struct ip_addr_range {
    ip_addr_t start;
    ip_addr_t end;
};

// It is better to contain everything within a single struct, as
// opposed to having a bunch of global variables all over the place.
//
// NOTE: I did not use bitfields here because they are not
// going to help with compile-time safety, and they'd also
// come with a performance cost and lack of addressability.
struct ProgramArgs {
    // General
    struct {
        char *executable_name; // argv[0]
        endianness_t compile_endianness;
        endianness_t runtime_endianness;
        bool has_c11_threads;
        bool has_posix_threads;
    } diagnostics;
    struct {
        bool opt_help;
        bool opt_about;
        bool opt_version;
        bool opt_quiet;
    } general;
    // Advanced
    struct {
        bool bypass_checks;
        unsigned int num_threads;
        bool native_threads;
        unsigned int buffer_size;
        bool no_async_sock;
        bool no_mem_lock;
        bool no_prefetch;
    } advanced;
    // IPv4
    struct ip_hdr *ipv4;
    struct {
        bool is_ipv4;
        bool is_ipv6;
        bool is_cidr;
        struct {
            ip_addr_t start;
            ip_addr_t end;
        } source_cidr;
    } ip_misc;
    // TODO: IPv6
};

int parse_ip_cidr(
    const char *cidr_str,
    struct ip_addr_range *ip_range
);
int parse_ip_port(
    const char *ip_port_str,
    ip_addr_t *ip, int *port
);
int parse_args(
    const int argc,
    char *argv[],
    struct ProgramArgs *const program_args
);


#endif // PARSER_H

// ---------------------------------------------------------------------
// END OF FILE: parser.h
// ---------------------------------------------------------------------
