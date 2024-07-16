// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// parser.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "parser.h"


int parse_ip_cidr(
    const char *cidr_str, struct ip_addr_range *ip_range
) {
    if (strlen(cidr_str) > MAX_IP_CIDR_LENGTH) {
        fprintf(stderr, "IP/CIDR notation is too long: %s\n", cidr_str);
        return EXIT_FAILURE;
    }

    char cidr_copy[MAX_IP_CIDR_LENGTH+1]; // +1 for null terminator
    strncpy(cidr_copy, cidr_str, sizeof (cidr_copy));
    cidr_copy[sizeof (cidr_copy) - 1] = '\0';

    char *ip_str = strtok(cidr_copy, "/");
    char *prefix_len_str = strtok(NULL, "/");

    if (ip_str == NULL || prefix_len_str == NULL) {
        fprintf(stderr, "Invalid IP/CIDR notation: %s\n", cidr_str);
        return EXIT_FAILURE;
    }

    struct in_addr ip_addr;
    if (inet_pton(AF_INET, ip_str, &ip_addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_str);
        return EXIT_FAILURE;
    }

    int prefix_len = atoi(prefix_len_str);
    if (prefix_len < 0 || prefix_len > 32) {
        fprintf(stderr, "Invalid prefix length: %s\n", prefix_len_str);
        return EXIT_FAILURE;
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

    return EXIT_SUCCESS;
}

int parse_ip_port(
    const char *ip_port_str, in_addr_t *ip, int *port
) {
    if (strlen(ip_port_str) > MAX_IP_PORT_LENGTH) {
        fprintf(stderr, "IP:Port notation is too long: %s\n",
                        ip_port_str);
        return EXIT_FAILURE;
    }

    char ip_port_copy[MAX_IP_PORT_LENGTH + 1];
    strncpy(ip_port_copy, ip_port_str, sizeof (ip_port_copy));
    ip_port_copy[sizeof (ip_port_copy) - 1] = '\0';

    char *ip_str = strtok(ip_port_copy, ":");
    char *port_str = strtok(NULL, ":");

    if (ip_str == NULL || port_str == NULL) {
        fprintf(stderr, "Invalid IP:Port notation: %s\n",
                        ip_port_str);
        return EXIT_FAILURE;
    }

    *ip = inet_addr(ip_str);
    if (*ip == INADDR_NONE) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_str);
        return EXIT_FAILURE;
    }

    *port = atoi(port_str);
    if (*port < PORT_MIN || *port > PORT_MAX) {
        fprintf(stderr, "Invalid port: %d\n", *port);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int parse_args(
    int argc, char const *argv[], struct program_args *args
) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <num. threads> <source IP/CIDR> "
                        "<dest. IP:Port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    args->num_threads = atoi(argv[1]);
    if (args->num_threads < 1) {
        fprintf(stderr, "Invalid number of threads: %d\n", 
                        args->num_threads);
        return EXIT_FAILURE;
    }

    if (parse_ip_cidr(argv[2], &args->src_ip_range) == EXIT_FAILURE) {
        fprintf(stderr, "Failed to parse source IP/CIDR.\n");
        return EXIT_FAILURE;
    }

    if (parse_ip_port(argv[3], &args->dest_ip, &args->dest_port) == -1) {
        fprintf(stderr, "Failed to parse destination IP:Port.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


// ---------------------------------------------------------------------
// END OF FILE: parser.c
// ---------------------------------------------------------------------
