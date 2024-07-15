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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdalign.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

extern int errno; // Declared in <errno.h>

#define IP_PKT_MTU 1500 // Same as Ethernet II MTU (bytes)

#define NUM_THREADS 4 // TODO: Take as an argument


struct pkt_args {
    int sock;
    in_addr_t src_ip_start;
    in_addr_t src_ip_end;
    int src_port;
    in_addr_t dest_ip;
    int dest_port;
};


// Thread callback
void* send_packets(void *arg) {
    struct pkt_args *packet_info = (struct pkt_args *)arg;

    // Initialize an empty packet buffer on the stack and align it
    // for potentially faster access from memory.
    alignas(4) uint8_t packet_buffer[IP_PKT_MTU];
    memset(packet_buffer, 0, IP_PKT_MTU);
    // Make the IP and TCP header structures "point" to their
    // respective locations in the aforementioned buffer
    struct iphdr  *ip_header  =
        (struct iphdr *)packet_buffer;
    struct tcphdr *tcp_header =
        (struct tcphdr *)(packet_buffer + sizeof (struct iphdr));

    // Initialize the seed for random number generation.
    srand(time(0));

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
    ip_header->tot_len  = htons(sizeof(struct iphdr)
        + sizeof(struct tcphdr));

    // Source port
    tcp_header->th_sport = htons(packet_info->src_port);
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

    return 0;
}

int create_raw_socket() {
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
        return -1;
	}

    // On success, return the socket descriptor.
    return socket_descriptor;
}

struct ip_addr_range {
    in_addr_t start;
    in_addr_t end;
};

// A CIDR notation string is in the following format xxx.xxx.xxx.xxx/xx
// (e.g., 255.255.255.255/32); there can be a total of 18 characters.
#define MAX_CIDR_LENGTH 18

int parse_cidr(const char *cidr_str, struct ip_addr_range *ip_range) {
    if (strlen(cidr_str) > MAX_CIDR_LENGTH) {
        fprintf(stderr, "IP/CIDR notation is too long: %s\n", cidr_str);
        return -1;
    }

    char cidr_copy[MAX_CIDR_LENGTH+1]; // +1 for null terminator
    strncpy(cidr_copy, cidr_str, sizeof (cidr_copy));
    cidr_copy[sizeof (cidr_copy) - 1] = '\0';

    char *ip_str = strtok(cidr_copy, "/");
    char *prefix_len_str = strtok(NULL, "/");

    if (ip_str == NULL || prefix_len_str == NULL) {
        fprintf(stderr, "Invalid IP/CIDR notation: %s\n", cidr_str);
        return -1;
    }

    struct in_addr ip_addr;
    if (inet_pton(AF_INET, ip_str, &ip_addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_str);
        return -1;
    }

    int prefix_len = atoi(prefix_len_str);
    if (prefix_len < 0 || prefix_len > 32) {
        fprintf(stderr, "Invalid prefix length: %s\n", prefix_len_str);
        return -1;
    }

    uint32_t mask = htonl((uint32_t)~0 << (32 - prefix_len));
    ip_range->start = ip_addr.s_addr & mask;
    ip_range->end = ip_addr.s_addr | ~mask;

    {
        char start[INET_ADDRSTRLEN];
        char end[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(ip_range->start), start, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ip_range->end), end, INET_ADDRSTRLEN);

        printf("Starting IP range: %s\n", start);
        printf("Ending IP range: %s\n", end);
    }

    return 0;
}

#define PORT_MAX 65535 // 2^16 - 1

int main(int argc, char const *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <source IP/CIDR> <source port> "
            "<destination IP> <destination port>\n", argv[0]);

        return EXIT_FAILURE;
    }

    const char *src_cidr_str = argv[1];
    int src_port = atoi(argv[2]);
    const char *dest_ip_str = argv[3];
    int dest_port = atoi(argv[4]);

    if (src_port < 0 || src_port > PORT_MAX) {
        fprintf(stderr, "Invalid source port: %d\n", src_port);
        return EXIT_FAILURE;
    }

    if (dest_port < 0 || dest_port > PORT_MAX) {
        fprintf(stderr, "Invalid destination port: %d\n", dest_port);
        return EXIT_FAILURE;
    }

    struct ip_addr_range src_ip_range;
    if (parse_cidr(src_cidr_str, &src_ip_range) == -1) {
        fprintf(stderr, "Failed to parse source IP/CIDR; exiting.\n");
        return EXIT_FAILURE;
    }

    in_addr_t dest_ip = inet_addr(dest_ip_str);
    if (dest_ip == INADDR_NONE) {
        fprintf(stderr, "Invalid destination IP address: %s\n",
            dest_ip_str);
        return EXIT_FAILURE;
    }

    int socket_descriptor = create_raw_socket();
    if (socket_descriptor == -1) {
        fprintf(stderr, "Socket creation failed; exiting.\n");
        return EXIT_FAILURE;
    }


    struct pkt_args thread_args;
    thread_args.sock = socket_descriptor;
    thread_args.src_ip_start = htonl(src_ip_range.start);
    thread_args.src_ip_end = htonl(src_ip_range.end);
    thread_args.src_port = src_port;
    thread_args.dest_ip = dest_ip;
    thread_args.dest_port = dest_port;

    // TODO: Error-check and make thread creation optional
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(
                &threads[i], NULL, send_packets, &thread_args) != 0)
        {
            printf("Failed to create thread(s)\n");
            return EXIT_FAILURE;
        }
    }

    // TODO: This is never executed; add a signal handler in threads?
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    close(socket_descriptor);

    return EXIT_SUCCESS;
}
