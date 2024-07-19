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
    struct ip_hdr  *ip_header  =
        (struct ip_hdr *)packet_buffer;
    struct tcp_hdr *tcp_header =
        (struct tcp_hdr *)(packet_buffer + sizeof (struct ip_hdr));


    // Craft the static parts of our packet

    // TODO: Refactor and clean this code up now that I have "netinet.h"
    // generate the struct with the {} initialization notation

    // IP Version
    ip_header->ver      = 4; // IPv4
    // Header length (in 32-bit words)
    ip_header->ihl      = 5; // Minimum size (5*4 = 20 bytes)
    // Time to live
    ip_header->ttl      = 128;
    // Source IP
    ip_header->saddr.address   = htonl(packet_info->src_ip_start.address
        + (rand() % (packet_info->src_ip_end.address
        - packet_info->src_ip_start.address + 1)));
    // Destination IP
    ip_header->daddr.address  = packet_info->dest_ip.address;
    // IP Protocol
    ip_header->proto    = IP_PROTO_TCP;
    // Total length
    ip_header->len  = htons(sizeof (struct ip_hdr)
                                + sizeof (struct tcp_hdr));

    // Source port
    tcp_header->sport = htons(rand() % 65536);
    // Destination port
    tcp_header->dport = htons(packet_info->dest_port);
    // Sequence number
    tcp_header->seqnum   = rand();
    // TCP Flag(s)
    tcp_header->flags.syn = true;

    // TODO: See if this is required for a raw sendto()?
    struct sockaddr_in dest_info;
    dest_info.sin_family = AF_INET;
    dest_info.sin_port = tcp_header->dport;
    dest_info.sin_addr.s_addr = ip_header->daddr.address;
    size_t packet_length = ntohs(ip_header->len);
    struct sockaddr *dest_addr = (struct sockaddr *)&dest_info;
    size_t addr_len = sizeof (dest_info);


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

    // For maximal performance, do the bare-minimum processing in this
    // loop.  As of now, the Kernel (i.e., sendto) is the bottleneck.
    for (;;) {
        // Randomize source IP and port
        ip_header->saddr.address = htonl(
            packet_info->src_ip_start.address
            + (rand() % (packet_info->src_ip_end.address
            - packet_info->src_ip_start.address + 1))
        );
        tcp_header->sport = htons(rand() % 65536);

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
