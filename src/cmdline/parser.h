// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// parser.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef PARSER_H
#define PARSER_H


#include "../program.h"
#include "../netlib/netinet.h"
#include "./logger.h"
#include "docs.h"


#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

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


int parse_args(
    const int argc,
    char *const argv[],
    struct ProgramArgs *const program_args
);

// This indirection is necessary for eager evaluation of macros.
// https://stackoverflow.com/a/5459929/12660750
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // PARSER_H

// ---------------------------------------------------------------------
// END OF FILE: parser.h
// ---------------------------------------------------------------------
