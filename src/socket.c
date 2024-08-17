// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// socket.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "socket.h"


int create_raw_async_socket() {
    // Setting the 'errno' flag to 0 indicates "no errors" so
    // that a previously set value does not affect us.
    errno = 0;

    // Attempt to create a raw socket.
    int socket_descriptor = socket(
        AF_INET,                  // Domain
        SOCK_RAW | SOCK_NONBLOCK, // Type (+ options)
        IPPROTO_RAW               // Protocol (implies IP_HDRINCL)
    );

    // Check for errors.
	if (socket_descriptor == -1) {
		// Socket creation failed (maybe non-root privileges?)
		perror("Failed to create an asynchronous raw socket");
        return EXIT_FAILURE;
	}

    // On success, return the socket descriptor.
    return socket_descriptor;
}


// ---------------------------------------------------------------------
// END OF FILE: socket.c
// ---------------------------------------------------------------------
