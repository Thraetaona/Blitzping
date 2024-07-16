// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// packet.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "packet.h"


// Thread callback
// TODO: Split the packet crafting logic to another function.
int send_packets(void *arg) {
    struct pkt_args *packet_info = (struct pkt_args *)arg;

    // Initialize the seed for random number generation.
    srand(time(0));

    // Initialize an empty packet buffer on the stack and align it
    // for potentially faster access from memory.
    alignas(4) uint8_t packet_buffer[IP_PKT_MTU] = {0};
    // Make the IP and TCP header structures "point" to their
    // respective locations in the aforementioned buffer
    struct iphdr  *ip_header  =
        (struct iphdr *)packet_buffer;
    struct tcphdr *tcp_header =
        (struct tcphdr *)(packet_buffer + sizeof (struct iphdr));


    // Craft the static parts of our packet

    // IP Version
    ip_header->version  = 4; // IPv4
    // Header length (in 32-bit words)
    ip_header->ihl      = 5; // Minimum size (5*4 = 20 bytes)
    // Time to live
    ip_header->ttl      = 128;
    // Source IP
    ip_header->saddr    = htonl(packet_info->src_ip_start + (rand()
        % (packet_info->src_ip_end - packet_info->src_ip_start + 1)));
    // Destination IP
    ip_header->daddr    = packet_info->dest_ip;
    // IP Protocol
    ip_header->protocol = IPPROTO_TCP;
    // Total length
    ip_header->tot_len  = htons(sizeof (struct iphdr)
                                + sizeof (struct tcphdr));

    // Source port
    tcp_header->th_sport = htons(rand() % 65536);
    // Destination port
    tcp_header->th_dport = htons(packet_info->dest_port);
    // Sequence number
    tcp_header->th_seq   = rand();
    // TCP Flag(s)
    tcp_header->th_flags = TH_SYN;

    // TODO: See if this is required for a raw sendto()?
    struct sockaddr_in dest_info;
    dest_info.sin_family = AF_INET;
    dest_info.sin_port = tcp_header->th_dport;
    dest_info.sin_addr.s_addr = ip_header->daddr;
    size_t packet_length = ntohs(ip_header->tot_len);
    struct sockaddr *dest_addr = (struct sockaddr *)&dest_info;
    size_t addr_len = sizeof (dest_info);

    // For maximal performance, do the bare-minimum processing in this
    // loop.  As of now, the Kernel (i.e., sendto) is the bottleneck.
    for (;;) {
        // Randomize source IP and port
        ip_header->saddr = htonl(
            packet_info->src_ip_start
            + (rand() % (packet_info->src_ip_end
            - packet_info->src_ip_start + 1))
        );
        tcp_header->th_sport = htons(rand() % 65536);

        // Send the Packet
        sendto(
            packet_info->sock,
            packet_buffer,
            packet_length,
            0,
            dest_addr,
            addr_len
        );
    }

    return EXIT_SUCCESS;
}


// ---------------------------------------------------------------------
// END OF FILE: packet.c
// ---------------------------------------------------------------------
