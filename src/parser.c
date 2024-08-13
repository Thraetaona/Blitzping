// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// parser.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "parser.h"


// TODO: use ifndef to enable/disable num-threads here.
// TODO: Do I use \r\n here, or continue relying on libc?
// TODO: Create a man.1 (man page) file using this
//
// TODO: Add an option to control memory alignment?
//
// NOTE: Unfortunately, C preprocessor is unable to include actual .txt
// files (which could otherwise be holding this text) into strings.
// (C23 is apparently able to do this using the new #embed directive.)
static const char HELP_TEXT_OVERVIEW[] =
"Usage: blitzping [options]\n"
"\n"
"Options may use either -U (unix style), --gnu-style, or /dos-style\n"
"conventions, and the \"=\" sign may be omitted.\n"
"\n"
"Example:\n"
"  blitzping --num-threads=4 --proto=tcp --dest-ip=10.10.10.10\n"
"  blitzping --help=proto\n"
"\n"
"General:\n"
"  -? --help=<command>     Display this help message or more info.\n"
"  -! --about              Display information about the Program.\n"
"  -V --version            Display the Program version.\n"
"  -Q --quiet              Suppress all output except errors.\n"
"Advanced:\n"
"  -$ --bypass-checks      Ignore system compatibility issues (e.g., \n"
"                          wrong endianness) if startup checks fail.\n"
"                          (May result in unexpected behavior.)\n"
"  -# --num-threads=<0-n>  Number of threads to poll the socket with.\n"
"                          (Default: system thread count; 0: disable\n"
"                          threading, run everything in main thread.)\n"
"     --native-threads     If both C11 libc <threads.h> and native\n"
"                          (i.e., POSIX/Win32) threads are available,\n"
"                          will prefer the native implementation.\n"
"     --buffer-size=<0-n>  Number of packets to pre-craft and pass to\n"
"                          the kernel in batches. (default: as many\n"
"                          as possible; 0: disable buffering.)\n"
"     --no-async-sock      Use blocking (not asynchronous) sockets.\n"
"                          (Will severely hinder performance.)\n"
"     --no-mem-lock        Don't lock memory pages; allow disk swap.\n"
"                          (May reduce performance.)\n"
"     --no-prefetch        Don't prefetch packet buffer to CPU cache.\n"
"                          (May reduce performance.)\n";

static const char HELP_TEXT_IPV4[] =
"IPv4 Header:\n"
"  -4 --ipv4               IPv4 mode (automatically determined).\n"
"     --source-ip=<addr>   [Override] IPv4 source address to spoof.\n"
"     --dest-ip=<addr>     IPv4 destination address.\n"
"     --ver=<4|0-15>       [Override] IP version to spoof.\n"
"     --ihl=<0-15>         [Override] IPv4 header length in 32-bit\n"
"                          increments; minimum \"should\" be 5 (i.e.,\n"
"                          5x32 = 160 bits = 20 bytes) by standard.\n"
"     --tos=<0-255>        Type of Service; obsolete by DSCP+ECN.\n"
"     |                    ToS itself is divided into precedence,\n"
"     |                    throughput, reliability, cost, and mbz:\n"
"     | --prec=<...|0-7>     IP Precedence/priority (RFC 791).\n"
"     |                      (\"--help=prec\" for textual entries.)\n"
"     | --min-delay=<0|1>    Minimize delay\n"
"     | --max-tput =<0|1>    Maximize throughput\n"
"     | --max-rely =<0|1>    Maximize reliability\n"
"     | --min-cost =<0|1>    Minimize monetary cost (RFC 1349)\n"
"     | --must-zero=<0|1>    MBZ/Must-be-Zero (or must it?)\n"
"     --dscp=<...|0-64>    Differentiated Services Code Point\n"
"                          (\"--help=dscp\" for textual entries.)\n"
"     --ecn=<...|0-3>      Explicit Congestion Notification\n"
"                          (\"--help=ecn\" for textual entries.)\n"
"     --len=<0-65535>      [Override] total packet (+data) length.\n"
"     --ident=<0-65535>    Packet identification (for fragmentation).\n"
"     --flags=<0-7>        Bitfield for IPv4 flags:\n"
"     | --evil-bit =<0|1>    RFC 3514 Evil/Reserved bit\n"
"     | --dont-frag=<0|1>    Don't Fragment (DF)\n"
"     | --more-frag=<0|1>    More Fragments (MF)\n"
"     --frag-ofs=<0-8191>  Fragment Offset\n"
"     --ttl=<0-255>        Time-to-live (hop-limit) for the packet.\n"
"     --proto=<...|0-255>  [Override] protocol number (e.g., tcp, 6)\n"
"                          (\"--help=proto\" for textual entries.)\n"
"     --checksum=<0-65535> [Override] IPv4 header checksum.\n"
"     --options=<UNIMPLEMENTED>\n";

static const char HELP_TEXT_IPV6[] =
"IPv6 Header: [[UNIMPLEMENTED]]\n"
"  -6 --ipv6               IPv6 mode (automatically determined).\n"
"     --source-ip6=<addr>  [Override] IPv6 source address to spoof.\n"
"     --dest-ip6=<addr>    IPv6 destination address.\n"
"     --next-header=<...|0-255>\n"
"                          Similar to IPv4's \"proto\" field.\n"
"                          (\"--help=next-header\" for text entries.)\n"
"     --hop-limit=<0-255>  Similar to IPv4's \"ttl\" field.\n"
"     --flow-label=<0-1048575>\n"
"                          Flow Label (experimental; RFC 2460).\n";


// NOTE: These were "divided" into sections because C99+ compilers
// might not support string literals exceeding 4095 characters.
//
// NOTE: The help texts alone take up a few kilobytes of space; you
// could change this to an empty string to save on that, if need be.
#define HELP_TEXT "%s%s%s", \
    HELP_TEXT_OVERVIEW, HELP_TEXT_IPV4, HELP_TEXT_IPV6

// This indirection is necessary for eager evaluation of macros.
// https://stackoverflow.com/a/5459929/12660750
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

// TODO: Bring more macro detections from here:
// TODO: add runtime reports?
// https://github.com/cpredef/predef
static const char ABOUT_TEXT[] =
"Blitzping  Copyright (C) 2024  Fereydoun Memarzanjany\n"
"This program comes with ABSOLUTELY NO WARRANTY.\n"
"\n"
"This is free software, and you are welcome to redistribute it\n"
"under certain conditions; see the GNU General Public License v3.0.\n"
"If you wish to report a bug or contribute to this project,\n"
"visit this repository: https://github.com/Thraetaona/Blitzping\n"
"\n"
"Compilation Diagnostics:\n"
#if defined(__DATE__) && defined(__TIME__)
"  Build Time           : " __DATE__ " @ " __TIME__ "\n"
#else
"  Build Time           : unknown\n"
#endif

#if defined(__VERSION__)
"  Compiler             : " __VERSION__ "\n"
#else
"  Compiler             : unknown\n"
#endif

// NOTE: These are defined in the makefile.
"  Target Triplet       : " TARGET_TRIPLET "\n"
"    Architecture       : " TARGET_ARCH "\n"
"      Sub-Architecture : " TARGET_SUBARCH "\n"
"    Vendor             : " TARGET_VENDOR "\n"
"    Operating System   : " TARGET_OS "\n"
"    C Library (libc)   : " TARGET_LIBC "\n"
"    Float ABI          : " TARGET_FLOAT_ABI "\n"

#if defined(__LITTLE_ENDIAN__)
"    Endianness         : little\n"
#elif defined(__BIG_ENDIAN__)
"    Endianness         : big\n"
#else
"    Endianness         : unknown\n";
#endif

#if defined(__STDC_VERSION__)
"  C Version            : " STRINGIFY(__STDC_VERSION__) "\n"
#   if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
"    C11 Threads        : compiled\n"
#   else
"    C11 Threads        : not compiled\n"
#   endif
#endif

#if defined(_POSIX_C_SOURCE)
"  Userland API         : POSIX\n"
"    POSIX C Source     : " STRINGIFY(_POSIX_C_SOURCE) "\n"
#   ifdef _POSIX_THREADS
"    POSIX Threads      : compiled\n"
#   else
"    POSIX Threads      : not compiled\n"
#   endif
#elif defined(_WIN32)
"  Userland API         : Win32\n"
#else
"  Userland API         : unknown\n"
#endif
"\n";


static const char VERSION_TEXT[] = "Blitzping v1.7.0";

enum OptionKind {
    OPTION_NONE = 0,
    // General
    OPTION_HELP,
    OPTION_ABOUT,
    OPTION_VERSION,
    OPTION_QUIET,
    // Advanced
    OPTION_BYPASS_CHECKS,
    OPTION_NUM_THREADS,
    OPTION_NATIVE_THREADS,
    OPTION_BUFFER_SIZE,
    OPTION_NO_ASYNC_SOCK,
    OPTION_NO_MEM_LOCK,
    OPTION_NO_PREFETCH,
    // IPv4 Header
    OPTION_IPV4,
    OPTION_IP_SOURCE,
    OPTION_IP_DEST,
    OPTION_IP_VER,
    OPTION_IP_IHL,
    OPTION_IP_TOS,
    OPTION_IP_PREC,
    OPTION_IP_MIN_DELAY,
    OPTION_IP_MAX_TPUT,
    OPTION_IP_MAX_RELY,
    OPTION_IP_MIN_COST,
    OPTION_IP_MUST_ZERO,
    OPTION_IP_DSCP,
    OPTION_IP_ECN,
    OPTION_IP_LEN,
    OPTION_IP_IDENT,
    OPTION_IP_FLAGS,
    OPTION_IP_EVIL_BIT,
    OPTION_IP_DONT_FRAG,
    OPTION_IP_MORE_FRAG,
    OPTION_IP_FRAG_OFS,
    OPTION_IP_TTL,
    OPTION_IP_PROTO,
    OPTION_IP_CHECKSUM,
    OPTION_IP_OPTIONS,
    // IPv6 Header
    OPTION_IPV6,
    OPTION_IP6_SOURCE,
    OPTION_IP6_DEST,
    OPTION_IP6_NEXT_HEADER,
    OPTION_IP6_HOP_LIMIT,
    OPTION_IP6_FLOW_LABEL,
    // [[UNFINISHED]]
};

struct Option {
    const char flag;
    const char *const name;
    const bool has_arg;
    const enum OptionKind kind;
};

static const struct Option OPTIONS[] = {
    {'\0', "\0", false, OPTION_NONE},
    // General
    {'?', "help", false, OPTION_HELP},
    {'!', "about", false, OPTION_ABOUT},
    {'V', "version", false, OPTION_VERSION},
    {'Q', "quiet", false, OPTION_QUIET},
    // Advanced
    {'$', "bypass-checks", false, OPTION_BYPASS_CHECKS},
    {'#', "num-threads", true, OPTION_NUM_THREADS},
    {'\0', "native-threads", false, OPTION_NATIVE_THREADS},
    {'\0', "buffer-size", true, OPTION_BUFFER_SIZE},
    {'\0', "no-async-sock", false, OPTION_NO_ASYNC_SOCK},
    {'\0', "no-mem-lock", false, OPTION_NO_MEM_LOCK},
    {'\0', "no-prefetch", false, OPTION_NO_PREFETCH},
    // IPv4 Header
    {'4', "ipv4", false, OPTION_IPV4},
    {'\0', "source-ip", true, OPTION_IP_SOURCE},
    {'\0', "dest-ip", true, OPTION_IP_DEST},
    {'\0', "ver", true, OPTION_IP_VER},
    {'\0', "ihl", true, OPTION_IP_IHL},
    {'\0', "tos", true, OPTION_IP_TOS},
    {'\0', "prec", true, OPTION_IP_PREC},
    {'\0', "min-delay", true, OPTION_IP_MIN_DELAY},
    {'\0', "max-tput", true, OPTION_IP_MAX_TPUT},
    {'\0', "max-rely", true, OPTION_IP_MAX_RELY},
    {'\0', "min-cost", true, OPTION_IP_MIN_COST},
    {'\0', "must-zero", true, OPTION_IP_MUST_ZERO},
    {'\0', "dscp", true, OPTION_IP_DSCP},
    {'\0', "ecn", true, OPTION_IP_ECN},
    {'\0', "len", true, OPTION_IP_LEN},
    {'\0', "ident", true, OPTION_IP_IDENT},
    {'\0', "flags", true, OPTION_IP_FLAGS},
    {'\0', "evil-bit", true, OPTION_IP_EVIL_BIT},
    {'\0', "dont-frag", true, OPTION_IP_DONT_FRAG},
    {'\0', "more-frag", true, OPTION_IP_MORE_FRAG},
    {'\0', "frag-ofs", true, OPTION_IP_FRAG_OFS},
    {'\0', "ttl", true, OPTION_IP_TTL},
    {'\0', "proto", true, OPTION_IP_PROTO},
    {'\0', "checksum", true, OPTION_IP_CHECKSUM},
    {'\0', "options", true, OPTION_IP_OPTIONS},
    // IPv6 Header
    {'6', "ipv6", false, OPTION_IPV6},
    {'\0', "source-ip6", true, OPTION_IP6_SOURCE},
    {'\0', "dest-ip6", true, OPTION_IP6_DEST},
    {'\0', "next-header", true, OPTION_IP6_NEXT_HEADER},
    {'\0', "hop-limit", true, OPTION_IP6_HOP_LIMIT},
    {'\0', "flow-label", true, OPTION_IP6_FLOW_LABEL},
};
#define NUM_OPTIONS (sizeof (OPTIONS) / sizeof (struct Option))

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
    ip_range->start.address = ntohl(ip_addr.s_addr & mask);
    ip_range->end.address = ntohl(ip_addr.s_addr | ~mask);

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
    const char *ip_port_str, ip_addr_t *ip, int *port
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

    ip->address = inet_addr(ip_str);
    if (ip->address  == INADDR_NONE) {
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

static long validate_range(
    const char *const value_str,
    const long min,
    const long max,
    const char *const error_name
) {
    errno = 0;
    
    char *endptr;
    long result = strtol(value_str, &endptr, 10);

    // Check for conversion errors and out-of-bounds values
    if (errno != 0 || *endptr != '\0' || result < min || result > max) {
        fprintf(stderr,
            "Value of \"%s\" must be [%ld, %ld].\n",
            error_name, min, max
        );

        if (errno != 0) {
            perror("Error in strtol() conversion");
        }

        exit(EXIT_FAILURE); // TODO: Terminate the program?
    }

    return result;
}

static void handle_option(
    const struct Option *const cmdline_option,
    const char *const value,
    struct ProgramArgs *const program_args
) {
    (void)value;

    switch (cmdline_option->kind) {
        // General
        case OPTION_HELP: {
            // TODO: Handle sub-help commands.
            program_args->general.opt_help = true;

            printf(HELP_TEXT);
            break;
        }
        case OPTION_ABOUT: {
            program_args->general.opt_about = true;

            printf("%s", ABOUT_TEXT);
            break;
        }
        case OPTION_VERSION: {
            program_args->general.opt_version = true;

            printf("%s\n", VERSION_TEXT);
            break;
        }
        case OPTION_QUIET: {
            program_args->general.opt_quiet = true;

            // TODO: Logger and logging levels.
            break;
        }
        // Advanced
        case OPTION_BYPASS_CHECKS: {
            program_args->advanced.bypass_checks = true;
            break;
        }
        case OPTION_NUM_THREADS: {
            program_args->advanced.num_threads =
                (unsigned int)validate_range(
                    value, 0, UINT_MAX, cmdline_option->name);
            break;
        }
        case OPTION_NATIVE_THREADS: {
            program_args->advanced.native_threads = true;
            break;
        }
        case OPTION_BUFFER_SIZE: {
            program_args->advanced.buffer_size =
                (unsigned int)validate_range(
                    value, 0, _POSIX_VERSION , cmdline_option->name);
            break;
        }
        case OPTION_NO_ASYNC_SOCK: {
            break;
        }
        case OPTION_NO_MEM_LOCK: {
            break;
        }
        case OPTION_NO_PREFETCH: {
            break;
        }
        // IPv4/6
        case OPTION_IP_SOURCE: {
            break;
        }
        case OPTION_IP_DEST: {
            break;
        }
        case OPTION_IP_VER: {
            break;
        }
        case OPTION_IP_PROTO: {
            break;
        }
        case OPTION_IP_LEN: {
            break;
        }
        case OPTION_IP_TTL: {
            break;
        }
        // IPv4
        case OPTION_IPV4: {
            break;
        }
        case OPTION_IP_IHL: {
            break;
        }
        case OPTION_IP_TOS: {
            break;
        }
        case OPTION_IP_PREC: {
            break;
        }
        case OPTION_IP_MIN_DELAY: {
            break;
        }
        case OPTION_IP_MAX_TPUT: {
            break;
        }
        case OPTION_IP_MAX_RELY: {
            break;
        }
        case OPTION_IP_MIN_COST: {
            break;
        }
        case OPTION_IP_MUST_ZERO: {
            break;
        }
        case OPTION_IP_DSCP: {
            break;
        }
        case OPTION_IP_ECN: {
            break;
        }
        case OPTION_IP_IDENT: {
            break;
        }
        case OPTION_IP_FLAGS: {
            break;
        }
        case OPTION_IP_EVIL_BIT: {
            break;
        }
        case OPTION_IP_DONT_FRAG: {
            break;
        }
        case OPTION_IP_MORE_FRAG: {
            break;
        }
        case OPTION_IP_FRAG_OFS: {
            break;
        }
        case OPTION_IP_CHECKSUM: {
            break;
        }
        case OPTION_IP_OPTIONS: {
            break;
        }
        // IPv6
        case OPTION_IPV6: {
            break;
        }
        case OPTION_IP6_NEXT_HEADER: {
            break;
        }
        case OPTION_IP6_HOP_LIMIT: {
            break;
        }
        case OPTION_IP6_FLOW_LABEL: {
            break;
        }
        // None
        case OPTION_NONE:
        default: {
            fprintf(stderr, "Error: Invalid option given.\n");
            break;
        }
    }
}

/*
int parse_args(
    int argc, char const *argv[], struct program_args *args
) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <num. threads> <source IP/CIDR> "
                        "<dest. IP:Port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    args->num_threads = atoi(argv[1]);
    if (args->num_threads < 0) {
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
*/



int parse_args(
    const int argc,
    char *argv[],
    struct ProgramArgs *const program_args
) {
    program_args->diagnostics.executable_name = argv[0];

    // If no arguments were given, print the help text and return.
    if (argc == 1) {
        printf(HELP_TEXT);
        return 1;
    }

    // argv[0] is the program name itself.
    for (int i = 1; i < argc; i++) {
        const char *const arg = argv[i];

        if (arg[0] == '-' || arg[0] == '/') {
            const char *const opt_name = arg + (arg[1] == '-' ? 2 : 1);
            const char *opt_val = NULL;

            for (size_t j = 0; j < NUM_OPTIONS; j++) {
                const size_t opt_len =
                    strlen(OPTIONS[j].name);

                // Check if the current argument is a valid option
                if (strncmp(opt_name, OPTIONS[j].name, opt_len) == 0 &&
                    (opt_name[opt_len] == '\0' ||
                    opt_name[opt_len] == '=')
                ) {

                    if (OPTIONS[j].has_arg) {
                        // Check if the argument contains an optional
                        // '=' sign, as well as where it's located.
                        const char *const has_equal =
                            strchr(opt_name, '=');

                        // If so, the value will come after the =
                        if (has_equal) {
                            opt_val = has_equal + 1;
                        }
                        else if (i + 1 < argc && argv[i + 1][0] != '-' && argv[i + 1][0] != '/') {
                            opt_val = argv[++i];
                        }

                        if (!opt_val || (opt_val[0] == '\0' || opt_val[0] == ' ')) {
                            fprintf(stderr,
                                "Error: Option \"%s\" "
                                "requires an argument!\n",
                                OPTIONS[j].name
                            );
                            return 1;
                        }
                    }

                    //printf("Option: %s, Value: %s\n", OPTIONS[j].name, opt_val ? opt_val : "(none)");
                    handle_option(&OPTIONS[j], opt_val, program_args);
                    break;
                }
                // Compared all options and found no valid match
                else if (j == NUM_OPTIONS-1) {
                    fprintf(stderr,
                        "Error: \"%s\" is not a known option.\n",
                        opt_name
                    );
                    return 1;
                }
            }

            // Rest of your code...
        }
        else {
            printf(HELP_TEXT);
            return 1;
        }
    }
    return 0;
}


// ---------------------------------------------------------------------
// END OF FILE: parser.c
// ---------------------------------------------------------------------
