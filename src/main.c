// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Blitzping: Sending IP packets as fast as possible in userland.
// Copyright (C) 2024  Fereydoun Memarzanjany
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <https://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <threads.h> // C11 threads (glibc >=2.28, musl >=1.1.5)

#if defined(_POSIX_C_SOURCE)
#   include <unistd.h>
#elif defined(_WIN32)
//# 
#endif

#include "parser.h"
#include "packet.h"
#include "socket.h"
#include "./netlib/netinet.h"

#define MAX_THREADS 100 // Arbitrary limit


#include <stdio.h>


bool verify_system() {
    bool checks_succeeded = true;

    // Check to see if the currently running machine's endianness
    // matches what was expected to be the target's endianness at
    // the time of compilation.
    endianness_t runtime_endianness = check_endianness();

#if defined(__LITTLE_ENDIAN__)
    if (runtime_endianness != little_endian) {
        fprintf(stderr, "[CRITICAL]: Code was compiled for little "
                "endian, but this machine is somehow big endian!\n");
#elif defined(__BIG_ENDIAN__)
    if (runtime_endianness != big_endian) {
        fprintf(stderr, "[CRITICAL]: Code was compiled for big "
                "endian, but this machine is somehow little endian!\n");
#endif
        checks_succeeded = false;
    }

    return checks_succeeded;
}


int main(int argc, char const *argv[]) {
    if (!verify_system()) {
        // TODO: Add an argument to let the user continue regardless.
        //if () {
            return EXIT_FAILURE;
        //}
        fprintf(stderr, "[WARNING]: System verification failed but "
                        "was told to continue regardless; expect "
                        "potentially undefined behavior.\n");
    }

    struct program_args args;
    if (parse_args(argc, argv, &args) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    int socket_descriptor = create_raw_async_socket();
    if (socket_descriptor == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }


    struct pkt_args thread_args;
    thread_args.sock = socket_descriptor;
    thread_args.src_ip_start.address = args.src_ip_range.start.address;
    thread_args.src_ip_end.address = args.src_ip_range.end.address;
    thread_args.dest_ip.address = args.dest_ip.address;
    thread_args.dest_port = args.dest_port;


    if (args.num_threads == 1) { // Single-threaded; run in main thread.
        send_packets(&thread_args);
    }
    else { // Multi-threaded; spawn additional ones.
        //thrd_t *threads = malloc(args.num_threads * sizeof (thrd_t));
        thrd_t threads[MAX_THREADS];

        for (int i = 0; i < args.num_threads; i++) {
            int thread_status = thrd_create(
                &threads[i], send_packets, &thread_args
            );

            if (thread_status != thrd_success) {
                fprintf(stderr, "Failed to spawn threads.\n");
                return EXIT_FAILURE;
            }
        }

        // TODO: This is never executed; add a signal handler?
        for (int i = 0; i < args.num_threads; i++) {
            thrd_join(threads[i], NULL);
        }
    }


    if (shutdown(socket_descriptor, SHUT_RDWR) == -1) {
        perror("Failed to shutdown the socket");
        return EXIT_FAILURE;
    }

    if (close(socket_descriptor) == -1) {
        perror("Failed to close the socket");
        return EXIT_FAILURE;
    }

    printf("Done; exiting...\n");
    return EXIT_SUCCESS;
}


// ---------------------------------------------------------------------
// END OF FILE: main.c
// ---------------------------------------------------------------------
