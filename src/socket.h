// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// socket.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef SOCKET_H
#define SOCKET_H


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_POSIX_C_SOURCE)
#   include <sys/socket.h>
#   include <arpa/inet.h>
#elif defined(_WIN32)
//#include <winsock2.h>
#endif

extern int errno; // Declared in <errno.h>


int create_raw_async_socket();


#endif // SOCKET_H

// ---------------------------------------------------------------------
// END OF FILE: socket.h
// ---------------------------------------------------------------------
