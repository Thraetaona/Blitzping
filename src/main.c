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


#include "./program.h"
#include "./cmdline/logger.h"
#include "./cmdline/parser.h"
#include "packet.h"
#include "socket.h"
#include "./netlib/netinet.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
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
#   if defined(_POSIX_THREADS) && _POSIX_THREADS >= 0
#       include <pthread.h>
int __attribute__((weak)) pthread_create(
    pthread_t *thread, const pthread_attr_t *attr,
    void *(*start_routine) (void *), void *arg
);
int __attribute__((weak)) pthread_join(
    pthread_t thread, void **retval
);
#   endif
#elif defined(_WIN32)
//#
#endif



void diagnose_system(struct ProgramArgs *const program_args) {
    //bool checks_succeeded = true;

    program_args->diagnostics.runtime.endianness = check_endianness();
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
    program_args->diagnostics.runtime.c11_threads =
        (&thrd_create != NULL) && (&thrd_join != NULL);
#endif
#if defined(_POSIX_THREADS) && _POSIX_THREADS >= 0
    program_args->diagnostics.runtime.posix_threads =
        (&pthread_create != NULL) && (&pthread_join != NULL);
#endif
    program_args->diagnostics.runtime.num_cores =
#if defined(_POSIX_C_SOURCE)
        sysconf(_SC_NPROCESSORS_ONLN);
#else
        0;
#endif

/*
    // Check to see if the currently running machine's endianness
    // matches what was expected to be the target's endianness at
    // the time of compilation.
#if defined(__LITTLE_ENDIAN__)
    if (runtime_endianness != little_endian) {
        logger(LOG_ERROR,
            "Program was compiled for little endian,\n"
            "but this machine is somehow big endian!\n"
        );
#elif defined(__BIG_ENDIAN__)
    if (runtime_endianness != big_endian) {
        logger(LOG_ERROR,
            "Program was compiled for big endian,\n"
            "but this machine is somehow little endian!\n"
        );
#endif
        checks_succeeded = false;
    }

    if (!thrd_create || !thrd_join) {
        fprintf(stderr,
            "This program was compiled with C11 <threads.h>,\n"
            "but this system appears to lack thrd_create() or\n"
            "thrd_join(); this could be due to an old C library.\n"
            "try using \"--native-threads\" for POSIX/Win32\n"
            "threads or \"--num-threads=0\" to disable threading.\n"
        );
        return 1;
    }
    
    fprintf(stderr,
        "This program was compiled without C11 <threads.h>;\n"
        "try using \"--native-threads\" for POSIX/Win32\n"
        "threads or \"--num-threads=0\" to disable threading.\n"
    );
*/
    //return checks_succeeded;
}

void fill_defaults(struct ProgramArgs *const program_args) {
    // General
    program_args->general.logger_level = LOG_INFO;

    // Advanced
    program_args->advanced.num_threads =
        program_args->diagnostics.runtime.num_cores;

    // IPv4
    //
    // NOTE: Unfortunately, there is no POSIX-compliant way to
    // get the current interface's ip address; getifaddrs() is
    // not standardized.
    // TODO: Use unprivilaged sendto() as an alternative.
    *(program_args->ipv4) = (struct ip_hdr){
        .ver = 4,
        .ihl = 5,
        .ttl = 128,
        .proto = IP_PROTO_TCP,
        .len = htons(sizeof(struct ip_hdr) + sizeof(struct tcp_hdr)),
        .saddr.address = 0,
        .daddr.address = 0
    };
}

int main(int argc, char *argv[]) {
     struct ProgramArgs program_args = {0};
    struct ip_hdr *ipv4_header_args = 
        (struct ip_hdr *)calloc(1, sizeof(struct ip_hdr));

    if (ipv4_header_args == NULL) {
        program_args.diagnostics.unrecoverable_error = true;
        logger(LOG_ERROR,
            "Failed to allocate memory for program arguments.");
        goto CLEANUP;
    }
    program_args.ipv4 = ipv4_header_args;


    diagnose_system(&program_args);

    fill_defaults(&program_args);

    /*if (!diagnose_system(&program_args)) {
        // TODO: Add an argument to let the user continue regardless.
        //if () {
            program_args.diagnostics.unrecoverable_error = true;
            goto CLEANUP;
        //}
        logger(LOG_WARN,
            "System verification failed but was told to continue\n"
            "regardless; expect potentially undefined behavior.\n"
        );
    }*/


    if (parse_args(argc, argv, &program_args) != 0) {
        program_args.diagnostics.unrecoverable_error = true;
        logger(LOG_INFO, "Quitting due to invalid arguments.");
        goto CLEANUP;
    }
    else if (program_args.general.opt_info) {
        // User just wanted to see the --help, --about, etc. text.
        goto CLEANUP;
    }


    logger_set_level(program_args.general.logger_level);
    logger_set_timestamps(!program_args.advanced.no_log_timestamp);

    int socket_descriptor = create_raw_async_socket();
    if (socket_descriptor == EXIT_FAILURE) {
        program_args.diagnostics.unrecoverable_error = true;
        logger(LOG_INFO, "Quitting after failing to create a socket.");
        goto CLEANUP;
    }

    program_args.socket = socket_descriptor;

    send_packets(&program_args);


    if (shutdown(socket_descriptor, SHUT_RDWR) == -1) {
        logger(LOG_WARN, "Socket shutdown failed: %s", strerror(errno));
    }
    else {
        logger(LOG_INFO, "Socket shutdown successfully.");
    }

    if (close(socket_descriptor) == -1) {
        logger(LOG_WARN, "Socket closing failed: %s", strerror(errno));
    }
    else {
        logger(LOG_INFO, "Socket closed successfully.");
    }

CLEANUP:

    free(ipv4_header_args);

    logger(LOG_INFO, "Done; exiting program...");

    if (program_args.diagnostics.unrecoverable_error) {
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}


// ---------------------------------------------------------------------
// END OF FILE: main.c
// ---------------------------------------------------------------------
