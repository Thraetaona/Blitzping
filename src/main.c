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

#if defined(_POSIX_C_SOURCE)
#   include <unistd.h>
#elif defined(_WIN32)
//#
#endif

#include "parser.h"
#include "packet.h"
#include "socket.h"
#include "./netlib/netinet.h"


bool verify_system() {
    bool checks_succeeded = true;

    // Check to see if the currently running machine's endianness
    // matches what was expected to be the target's endianness at
    // the time of compilation.
    endianness_t runtime_endianness = check_endianness();

#if defined(__LITTLE_ENDIAN__)
    if (runtime_endianness != little_endian) {
        fprintf(stderr,
                "[CRITICAL]: Code was compiled for little endian,\n"
                "but this machine is somehow big endian!\n");
#elif defined(__BIG_ENDIAN__)
    if (runtime_endianness != big_endian) {
        fprintf(stderr,
                "[CRITICAL]: Code was compiled for big endian,\n"
                "but this machine is somehow little endian!\n");
#endif
        checks_succeeded = false;
    }

    return checks_succeeded;
}


int main(int argc, char *argv[]) {
    if (!verify_system()) {
        // TODO: Add an argument to let the user continue regardless.
        //if () {
            return EXIT_FAILURE;
        //}
        fprintf(stderr, "[WARNING]: System verification failed but "
                        "was told to continue regardless; expect "
                        "potentially undefined behavior.\n");
    }

    struct ProgramArgs args;
    if (parse_args(argc, argv, &args) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    int socket_descriptor = create_raw_async_socket();
    if (socket_descriptor == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }


    //printf("%s", HELP_TEXT);


    struct pkt_args thread_args;
    /*
    thread_args.sock = socket_descriptor;
    thread_args.num_threads = args.num_threads;
    thread_args.src_ip_start.address = args.src_ip_range.start.address;
    thread_args.src_ip_end.address = args.src_ip_range.end.address;
    thread_args.dest_ip.address = args.dest_ip.address;
    thread_args.dest_port = args.dest_port;
    */


    send_packets(&thread_args);


    if (shutdown(socket_descriptor, SHUT_RDWR) == -1) {
        perror("[WARN] Socket shutdow failed");
    }
    else {
        printf("[INFO] Socket shutdown successfully.\n");
    }

    if (close(socket_descriptor) == -1) {
        perror("[WARN] Socket closing failed");
    }
    else {
        printf("[INFO] Socket closed successfully.\n");
    }

    printf("[INFO] Done; exiting...\n");
    return EXIT_SUCCESS;
}


// ---------------------------------------------------------------------
// END OF FILE: main.c
// ---------------------------------------------------------------------
