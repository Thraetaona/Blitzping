// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// socket.h is a part of Blitzping.
// ---------------------------------------------------------------------


#pragma once
#ifndef SOCKET_H
#define SOCKET_H

#include "./cmdline/logger.h"

#include <string.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_POSIX_C_SOURCE)
#   include <fcntl.h>
#   include <sys/mman.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#   include <net/if.h>
#   if defined(__linux__)
#       include <linux/if_packet.h>
#   endif
#elif defined(_WIN32)
//#include <winsock2.h>
#endif

int setup_posix_socket(const bool is_raw, const bool is_async);


#endif // SOCKET_H

// ---------------------------------------------------------------------
// END OF FILE: socket.h
// ---------------------------------------------------------------------
