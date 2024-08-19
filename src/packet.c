// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// packet.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "packet.h"

/* WORK IN-PROGRESS */



// TODO: Split the packet crafting logic to another function.
// Thread callback
static int send_loop(void *arg) {
    const struct ProgramArgs *const program_args =
        (const struct ProgramArgs *const)arg;

    // Initialize the seed for random number generation.
    // TODO: make this a program parameter
    srand(time(0));

    // Initialize an empty packet buffer on the stack and align it
    // for potentially faster access from memory.
    // TODO: malloc() this buffer instead?
    _Alignas (_Alignof (max_align_t)) uint8_t
        packet_buffer[IP_PKT_MTU] = {0};


    // Make the IP and TCP header structures "point" to their
    // respective locations in the aforementioned buffer
    struct ip_hdr  *ip_header  =
        (struct ip_hdr *)packet_buffer;
    struct tcp_hdr *tcp_header =
        (struct tcp_hdr *)(packet_buffer + sizeof (struct ip_hdr));


    *ip_header = *(program_args->ipv4);

    // TODO: parametrize
    *tcp_header = (struct tcp_hdr){
        .sport = htons(rand() % 65536),
        .dport = htons(80),
        .seqnum = rand(),
        .flags.syn = true
    };


    // if override_source
    uint32_t ip_diff = 0;
    // If CIDR
    //uint32_t ip_diff = program_args->src_ip_end.address
    //    - program_args->src_ip_start.address + 1;

    // Set the default destination address
    struct sockaddr_in dest_info = {
        .sin_family = AF_INET,
        .sin_port = ntohs(tcp_header->dport),
        .sin_addr.s_addr = ntohl(ip_header->daddr.address)
    };

    // NOTE: Instead of using sento() or sendmmsg(), both of which
    // require a "destination info" struct, you can pre-bind your
    // socket to a fixed destination by using connect() accompanied
    // by write() or writev().  However, if you do want to change
    // your destination with every call, then it might be better
    // to use the former functions, because they'd be handling
    // this binding inside kernelspace, bypassing what would
    // otherwise be an extraneous overhead to a separate connect().
    int connection_status = connect(
        program_args->socket,
        (struct sockaddr *)&dest_info,
        sizeof(dest_info)
    );

    if (connection_status != 0) {
        logger(LOG_ERROR,
            "Failed to bind socket to the destination address: %s",
            strerror(errno)
        );
        return 1;
    }

    size_t packet_length = ntohs(ip_header->len);


    //int num_threads = 4; // number of threads

    //int X = UIO_MAXIOV / (packet_length * num_threads);

    // NOTE: The addresses in this array of buffers all point
    // to the same packet buffer.  This might seem wasteful at
    // first, but a benefit is that the "for-loop" part of this
    // iteration will now be able to exist in kernelspace,
    // bypassing expensive syscalls.
    // TODO: Maybe pre-craft the changing parts of multiple packeets,
    // as to "queue" them for sending?
    _Alignas (_Alignof (max_align_t)) struct iovec iov[UIO_MAXIOV];
    for (int i = 0; i < 37; i++) {
        iov[i].iov_base = packet_buffer;
        iov[i].iov_len = packet_length;
    }
    // Compiler optimizations likely override this anyhow
    if (!program_args->advanced.no_cpu_prefetch) {
        PREFETCH(packet_buffer, 1, 3);
        PREFETCH(iov, 0, 3);
    }

    for (;;) {
        // Randomize source IP and port
        ip_header->saddr.address = htonl(
            program_args->ipv4->saddr.address
            + (rand() % ip_diff)
        );
        tcp_header->sport = htons(rand() % 65536);

        writev(program_args->socket, iov, 37);
    }


    // For maximal performance, do the bare-minimum processing in this
    // loop.  As of now, the Kernel syscall is the bottleneck.
    /*for (;;) {
        // Randomize source IP and port
        ip_header->saddr.address = htonl(
            program_args->src_ip_start.address
            + (rand() % ip_diff)
        );
        tcp_header->sport = htons(rand() % 65536);

        //sendmsg(program_args->socket, &msg, 0);
        //write(program_args->socket, packet_buffer, packet_length);
        writev(program_args->socket, iov, 37);
        //ssize_t bytes_written = writev(program_args->socket, iov, 37);
        //if (bytes_written == -1) {
        //    perror("writev failed");
        //}
    }*/

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
    return thrd_success;
#else
    return 0;
#endif
}



// TODO: Use xorshift
// TODO: check for POSIX_MEMLOCK
int send_packets(struct ProgramArgs *const program_args) {

    if (!program_args->advanced.no_mem_lock) {
        if (mlockall(MCL_FUTURE) == -1) {
            logger(LOG_ERROR,
                "Failed to lock memory: %s", strerror(errno)
            );
            return 1;
        }
        else {
            logger(LOG_INFO, "Locked memory.");
        }
    }

    // TODO: See if this is required for a raw sendto()?
    /*
    struct sockaddr_in dest_info;
    dest_info.sin_family = AF_INET;
    dest_info.sin_port = tcp_header->dport;
    dest_info.sin_addr.s_addr = ip_header->daddr.address;
    size_t packet_length = ntohs(ip_header->len);
    struct sockaddr *dest_addr = (struct sockaddr *)&dest_info;
    size_t addr_len = sizeof (dest_info);
    */

    /*struct msghdr msg = {
        .msg_name = &(struct sockaddr_in){
            .sin_family = AF_INET,
            .sin_port = tcp_header->dport,
            .sin_addr.s_addr = ip_header->daddr.address
        },
        .msg_namelen = sizeof (struct sockaddr_in),
        .msg_iov = (struct iovec[1]){
            {
                .iov_base = packet_buffer,
                .iov_len = ntohs(ip_header->len)
            }
        },
        .msg_iovlen = 1
    };*/

    /*
    printf("Packet:\n");
    printf("Source IP: %s\n", inet_ntoa(*(struct in_addr*)&ip_header->saddr.address));
    printf("Destination IP: %s\n", inet_ntoa(*(struct in_addr*)&ip_header->daddr.address));
    printf("Source Port: %d\n", ntohs(tcp_header->sport));
    printf("Destination Port: %d\n", ntohs(tcp_header->dport));
    printf("TTL: %d\n", ip_header->ttl);
    printf("Header Length: %d\n", ip_header->ihl * 4); // ihl is in 32-bit words
    printf("Total Length: %d\n", ntohs(ip_header->len));
    printf("SYN Flag: %s\n", tcp_header->flags.syn ? "Set" : "Not set");
    printf("\n");
    */



   unsigned int num_threads = program_args->advanced.num_threads;

// TODO: Use dlsym to check for thrds at RUNTIME.
    if (num_threads == 0) { // Run in main thread.
        send_loop(program_args);
    }
    else { // Multi-threaded
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
        //thrd_t *threads =
        //    malloc(args.num_threads * sizeof (thrd_t));
        thrd_t threads[MAX_THREADS];

        for (unsigned int i = 0; i < num_threads; i++) {
            int thread_status = thrd_create(
                &threads[i], send_loop, program_args
            );

            if (thread_status != thrd_success) {
                logger(LOG_ERROR, "Failed to spawn thread %d.", i);
                // Cleanup already-created threads
                for (unsigned int j = 0; j < i; j++) {
                    thrd_join(threads[j], NULL);
                }
                //free(threads);
                return 1;
            }
        }

        // TODO: This is never reached; add a signal handler?
        for (unsigned int i = 0; i < num_threads; i++) {
            thrd_join(threads[i], NULL);
        }
#else
        return 1;
#endif
    }




    if (!program_args->advanced.no_mem_lock) {
        if (munlockall() == -1) {
            logger(LOG_ERROR,
                "Failed to unlock used memory: %s", strerror(errno)
            );
            return 1;
        }
        else {
            logger(LOG_INFO, "Unlocked used memory.");
        }
    }

    return 0;
}


// ---------------------------------------------------------------------
// END OF FILE: packet.c
// ---------------------------------------------------------------------
