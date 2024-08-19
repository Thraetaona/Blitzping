// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// parser.c is a part of Blitzping.
// ---------------------------------------------------------------------


#include "parser.h"



static char *duplicate_string(const char *str) {
    char *new_str = (char *)malloc(strlen(str) + 1);
    if (new_str == NULL) {
        logger(LOG_ERROR, "Memory allocation failed.");
        return NULL;
    }
    strcpy(new_str, str);
    return new_str;
}

static char *to_lowercase(const char *str) {
    char *lowercase_str = duplicate_string(str);
    if (lowercase_str == NULL) {
        return NULL;
    }
    for (char *p = lowercase_str; *p; ++p) {
        *p = tolower((unsigned char)*p);
    }

    return lowercase_str;
}

/*
static char *to_uppercase(const char *str) {
    char *uppercase_str = duplicate_string(str);
    if (uppercase_str == NULL) {
        return NULL;
    }
    for (char *p = uppercase_str; *p; ++p) {
        *p = toupper((unsigned char)*p);
    }

    return uppercase_str;
}
*/

struct NameKey {
    const char *const name;
    const int key;
};

static long get_key_from_name(
    const struct NameKey *const name_keys,
    const size_t num_keys,
    const char *const value_str,
    const bool case_insensitive,
    const char *const error_name,
    bool *const error_occured
) {
    char *comparison_value = NULL;
    if (case_insensitive) {
        comparison_value = to_lowercase(value_str);
    }
    else {
        comparison_value = (char *)value_str;
    }

    for (size_t i = 0; i < num_keys; i++) {
        char *comparison_name = NULL;
        if (case_insensitive) {
            comparison_name = to_lowercase(name_keys[i].name);
        }
        else {
            comparison_name = (char *)name_keys[i].name;
        }

        if (strcmp(comparison_name, comparison_value) == 0) {
            if (case_insensitive) {
                free(comparison_value);
                free(comparison_name);
            }
            return name_keys[i].key;
        }

        if (case_insensitive) {
            free(comparison_name);
        }
    }

    if (case_insensitive) {
        free(comparison_value);
    }
    // Not found

    // Make a list of valid options to print to the user
    char *valid_options = NULL;
    size_t len = 0;
    for (size_t i = 0; i < num_keys; i++) {
        len += strlen(name_keys[i].name) + 2;
    }
    valid_options = malloc(len + 1);
    if (valid_options == NULL) {
        DEBUG_MSG("Memory allocation failed.", 0);
    }
    valid_options[0] = '\0';
    for (size_t i = 0; i < num_keys; i++) {
        strcat(valid_options, name_keys[i].name);
        if (i < num_keys - 1) {
            strcat(valid_options, ", ");
        }
    }
    logger(LOG_ERROR,
        "\"%s\" is not a valid textual entry for the \"--%s\" option;\n"
        "valid entries are as follows (%s):\n  %s.\n"
        "(Use \"--help=%s\" for a description on those.)",
        value_str, error_name,
        case_insensitive ? "case-insensitive" : "case-sensitive",
        valid_options, error_name
    );
    free(valid_options);
    
    *error_occured = true;
    return -1;
}

static long validate_range(
    const char *const value_str,
    const long min,
    const long max,
    const char *const error_name,
    bool *const error_occured
) {
    errno = 0;
    
    char *endptr;
    long result = strtol(value_str, &endptr, 10);

    // Check for conversion errors and out-of-bounds values
    if (errno != 0 || *endptr != '\0' || result < min || result > max) {
        if (errno != 0) {
            logger(LOG_ERROR,
                "Error in strtol() conversion: %s", strerror(errno)
            );
        }

        logger(LOG_ERROR,
            "Value of \"--%s\" must be [%ld, %ld].",
            error_name, min, max
        );

        *error_occured = true;
        return -1;
    }

    return result;
}

static long parse_text_or_int(
    const struct NameKey *const name_keys,
    const size_t num_keys,
    const char *const value_str,
    const bool case_insensitive,
    const long min,
    const long max,
    const char *const error_name,
    bool *const error_occured
) {
    long result;

    if (isdigit(*value_str)) {
        result = validate_range(
            value_str, min, max, error_name, error_occured
        );
    }
    else {
        result = get_key_from_name(
                name_keys, num_keys, value_str, case_insensitive,
                error_name, error_occured
            );
    }

    return result;
}


static const struct NameKey IP_PROTOCOLS[] = {
    {"ip", IP_PROTO_IP},
    {"icmp", IP_PROTO_ICMP},
    {"tcp", IP_PROTO_TCP},
    {"udp", IP_PROTO_UDP}
};

static const struct NameKey IP_TOS_PREC_CODES[] = {
    {"routine", IP_TOS_PREC_ROUTINE},
    {"priority", IP_TOS_PREC_PRIORITY},
    {"immediate", IP_TOS_PREC_IMMEDIATE},
    {"flash", IP_TOS_PREC_FLASH},
    {"override", IP_TOS_PREC_FLASH_OVERRIDE},
    {"critical", IP_TOS_PREC_CRITIC_ECP},
    {"internetwork", IP_TOS_PREC_INTERNETWORK_CONTROL},
    {"network", IP_TOS_PREC_NETWORK_CONTROL}
};

static const struct NameKey IP_ECN_CODES[] = {
    {"not", IP_ECN_NOT_ECT},
    {"ect1", IP_ECN_ECT_1},
    {"ect0", IP_ECN_ECT_0},
    {"ce", IP_ECN_CE}
};

static const struct NameKey IP_DSCP_CODES[] = {
    {"df", IP_DSCP_DF},
    {"cs0", IP_DSCP_CS0},
    {"cs1", IP_DSCP_CS1},
    {"cs2", IP_DSCP_CS2},
    {"cs3", IP_DSCP_CS3},
    {"cs4", IP_DSCP_CS4},
    {"cs5", IP_DSCP_CS5},
    {"cs6", IP_DSCP_CS6},
    {"cs7", IP_DSCP_CS7},
    {"af11", IP_DSCP_AF11},
    {"af12", IP_DSCP_AF12},
    {"af13", IP_DSCP_AF13},
    {"af21", IP_DSCP_AF21},
    {"af22", IP_DSCP_AF22},
    {"af23", IP_DSCP_AF23},
    {"af31", IP_DSCP_AF31},
    {"af32", IP_DSCP_AF32},
    {"af33", IP_DSCP_AF33},
    {"af41", IP_DSCP_AF41},
    {"af42", IP_DSCP_AF42},
    {"af43", IP_DSCP_AF43},
    {"ef", IP_DSCP_EF},
    {"va", IP_DSCP_VA},
    {"le", IP_DSCP_LE}
};



// TODO: Bring more macro detections from here:
// TODO: add runtime reports?
// https://github.com/cpredef/predef
static const char HELP_DIAGNOSTICS[] =
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
#if defined(TARGET_TRIPLET) && defined(TARGET_ARCH) \
    && defined(TARGET_SUBARCH) && defined(TARGET_VENDOR) \
    && defined(TARGET_OS) && defined(TARGET_LIBC) \
    && defined(TARGET_FLOAT_ABI)
"  Target Triplet       : " TARGET_TRIPLET "\n"
"    Architecture       : " TARGET_ARCH "\n"
"      Sub-Architecture : " TARGET_SUBARCH "\n"
"    Vendor             : " TARGET_VENDOR "\n"
"    Operating System   : " TARGET_OS "\n"
"    C Library (libc)   : " TARGET_LIBC "\n"
"    Float ABI          : " TARGET_FLOAT_ABI "\n"
#endif

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
#   if defined(_POSIX_THREADS) && _POSIX_THREADS >= 0
"    POSIX Threads      : compiled\n"
#   else
"    POSIX Threads      : not compiled\n"
#   endif
"    Max Vectored I/O   : " STRINGIFY(UIO_MAXIOV) "\n"
#elif defined(_WIN32)
"  Userland API         : Win32\n"
#else
"  Userland API         : unknown\n"
#endif
;


enum OptionKind {
    OPTION_NONE = 0,
    // General
    OPTION_HELP,
    OPTION_ABOUT,
    OPTION_VERSION,
    OPTION_QUIET,
    // Advanced
    OPTION_BYPASS_CHECKS,
    OPTION_LOGGER_LEVEL,
    OPTION_NO_LOG_TIMESTAMP,
    OPTION_NUM_THREADS,
    OPTION_NATIVE_THREADS,
    OPTION_BUFFER_SIZE,
    OPTION_NO_ASYNC_SOCK,
    OPTION_NO_MEM_LOCK,
    OPTION_NO_CPU_PREFETCH,
    // IPv4 Header
    OPTION_IPV4,
    OPTION_SRC_IP,
    OPTION_DEST_IP,
    OPTION_IP_VER,
    OPTION_IP_IHL,
    OPTION_IP_TOS,
    OPTION_IP_PREC,
    OPTION_IP_MIN_DELAY,
    OPTION_IP_MAX_TPUT,
    OPTION_IP_MAX_RELY,
    OPTION_IP_MIN_COST,
    OPTION_IP_MBZ_ONE,
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
    OPTION_IP6_NEXT_HEADER,
    OPTION_IP6_HOP_LIMIT,
    OPTION_IP6_FLOW_LABEL,
    // [[UNFINISHED]]
    // TCP Header
    OPTION_TCP,
    // UDP Header
    OPTION_UDP,
    // ICMP Header
    OPTION_ICMP,
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
    {'\0', "logger-level", true, OPTION_LOGGER_LEVEL},
    {'\0', "no-log-timestamp", false, OPTION_NO_LOG_TIMESTAMP},
    {'#', "num-threads", true, OPTION_NUM_THREADS},
    {'\0', "native-threads", false, OPTION_NATIVE_THREADS},
    {'\0', "buffer-size", true, OPTION_BUFFER_SIZE},
    {'\0', "no-async-sock", false, OPTION_NO_ASYNC_SOCK},
    {'\0', "no-mem-lock", false, OPTION_NO_MEM_LOCK},
    {'\0', "no-cpu-prefetch", false, OPTION_NO_CPU_PREFETCH},
    // Multi-options (switches that may refer to multiple headers
    // and need extra processing to determine which one).
    {'\0', "src-ip", true, OPTION_SRC_IP},
    {'\0', "dest-ip", true, OPTION_DEST_IP},
    {'\0', "flags", true, OPTION_IP_FLAGS},
    {'\0', "chksum", true, OPTION_IP_CHECKSUM},
    {'\0', "options", true, OPTION_IP_OPTIONS},
    // IPv4 Header
    {'4', "ipv4", false, OPTION_IPV4},
    /* src-ip */
    /* dest-ip */
    {'\0', "ver", true, OPTION_IP_VER},
    {'\0', "ihl", true, OPTION_IP_IHL},
    {'\0', "tos", true, OPTION_IP_TOS},
    {'\0', "prec", true, OPTION_IP_PREC},
    {'\0', "min-delay", false, OPTION_IP_MIN_DELAY},
    {'\0', "max-tput", false, OPTION_IP_MAX_TPUT},
    {'\0', "max-rely", false, OPTION_IP_MAX_RELY},
    {'\0', "min-cost", false, OPTION_IP_MIN_COST},
    {'\0', "mbz-one", false, OPTION_IP_MBZ_ONE},
    {'\0', "dscp", true, OPTION_IP_DSCP},
    {'\0', "ecn", true, OPTION_IP_ECN},
    {'\0', "len", true, OPTION_IP_LEN},
    {'\0', "ident", true, OPTION_IP_IDENT},
    /* flags */
    {'\0', "evil-bit", false, OPTION_IP_EVIL_BIT},
    {'\0', "dont-frag", false, OPTION_IP_DONT_FRAG},
    {'\0', "more-frag", false, OPTION_IP_MORE_FRAG},
    {'\0', "frag-ofs", true, OPTION_IP_FRAG_OFS},
    {'\0', "ttl", true, OPTION_IP_TTL},
    {'\0', "proto", true, OPTION_IP_PROTO},
    /* chksum */
    /* options */
    // IPv6 Header
    {'6', "ipv6", false, OPTION_IPV6},
    /* src-ip */
    /* dest-ip */
    {'\0', "next-header", true, OPTION_IP6_NEXT_HEADER},
    {'\0', "hop-limit", true, OPTION_IP6_HOP_LIMIT},
    {'\0', "flow-label", true, OPTION_IP6_FLOW_LABEL},
    // unfinished
    // TCP Header
    {'T', "tcp", false, OPTION_TCP},
    /* src-port */
    /* dest-port */
    {'\0', "seq-num", true, OPTION_NONE},
    {'\0', "ack-num", true, OPTION_NONE},
    {'\0', "data-ofs", true, OPTION_NONE},
    {'\0', "reserved", true, OPTION_NONE},
    /* flags */
    {'\0', "cwr", false, OPTION_NONE},
    {'\0', "ece", false, OPTION_NONE},
    {'\0', "urg", false, OPTION_NONE},
    {'\0', "ack", false, OPTION_NONE},
    {'\0', "psh", false, OPTION_NONE},
    {'\0', "rst", false, OPTION_NONE},
    {'\0', "syn", false, OPTION_NONE},
    {'\0', "fin", false, OPTION_NONE},
    {'\0', "window", true, OPTION_NONE},
    /* chksum */
    {'\0', "urg-ptr", true, OPTION_NONE},
    /* options */
    // UDP Header
    {'U', "udp", false, OPTION_UDP},
    // ICMP Header
    {'I', "icmp", false, OPTION_ICMP},
};




static bool handle_option(
    const struct Option *const cmdline_option,
    const char *const value,
    struct ProgramArgs *const program_args
) {
    bool error_occured = false;

    switch (cmdline_option->kind) {
        // General
        case OPTION_HELP: {
            // TODO: Handle sub-help commands.
            program_args->general.opt_info = true;

            fprintf(stderr, HELP_TEXT_ALL);
            break;
        }
        case OPTION_ABOUT: {
            program_args->general.opt_info = true;

            fprintf(stderr, "%s\n%s", ABOUT_TEXT, HELP_DIAGNOSTICS);
            break;
        }
        case OPTION_VERSION: {
            program_args->general.opt_info = true;

            fprintf(stderr, "%s\n", VERSION_TEXT);
            break;
        }
        case OPTION_QUIET: {
            program_args->general.logger_level = LOG_WARN;
            break;
        }
        // Advanced
        case OPTION_BYPASS_CHECKS: {
            program_args->advanced.bypass_checks = true;
            break;
        }
        case OPTION_LOGGER_LEVEL: {
            program_args->general.logger_level =
                (log_level_t)validate_range(
                    value, LOG_NONE, LOG_DEBUG, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_NO_LOG_TIMESTAMP: {
            program_args->advanced.no_log_timestamp = true;
            break;
        }
        case OPTION_NUM_THREADS: {
            program_args->advanced.num_threads =
                (unsigned int)validate_range(
                    value, 0, UINT_MAX, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_NATIVE_THREADS: {
            program_args->advanced.native_threads = true;
            break;
        }
        case OPTION_BUFFER_SIZE: {
            program_args->advanced.buffer_size =
                (unsigned int)validate_range(
                    value, 0, UIO_MAXIOV, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_NO_ASYNC_SOCK: {
            program_args->advanced.no_async_sock = true;
            break;
        }
        case OPTION_NO_MEM_LOCK: {
            program_args->advanced.no_mem_lock = true;
            break;
        }
        case OPTION_NO_CPU_PREFETCH: {
            program_args->advanced.no_cpu_prefetch = true;
            break;
        }
        // IPv4
        case OPTION_IPV4: {
            program_args->parser.current_layer = LAYER_3;
            program_args->parser.current_proto = PROTO_L3_IPV4;
            break;
        }
        case OPTION_SRC_IP: {
            program_args->ipv4_misc.override_source = true;

            uint32_t temp;
            if (inet_pton(AF_INET, value, &temp) != 1) {
                logger(LOG_ERROR,
                    "Invalid source IPv4 address: %s", value);
                error_occured = true;
            }
            program_args->ipv4->saddr.address = htonl(temp);
            break;
        }
        case OPTION_DEST_IP: {
            uint32_t temp;
            if (inet_pton(AF_INET, value, &temp) != 1) {
                logger(LOG_ERROR,
                    "Invalid dest IPv4 address: %s", value);
                error_occured = true;
            }
            program_args->ipv4->daddr.address = htonl(temp);
            break;
        }
        case OPTION_IP_VER: {
            program_args->ipv4->ver = (uint8_t)validate_range(
                    value, 0, 15, cmdline_option->name, &error_occured);
            break;
        }
        case OPTION_IP_IHL: {
            program_args->ipv4->ihl = (uint8_t)validate_range(
                    value, 0, 15, cmdline_option->name, &error_occured);
            break;
        }
        case OPTION_IP_TOS: {
            program_args->ipv4->tos.bitfield = (uint8_t)validate_range(
                    value, 0, 255, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_IP_PREC: {
            program_args->ipv4->tos.precedence = 
                (ip_tos_prec_t)parse_text_or_int(
                    IP_TOS_PREC_CODES,
                    ARRAY_SIZE(IP_TOS_PREC_CODES),
                    value,
                    true,
                    0,
                    7,
                    cmdline_option->name,
                    &error_occured
                );
            break;
        }
        case OPTION_IP_MIN_DELAY: {
            program_args->ipv4->tos.low_delay = true;
            break;
        }
        case OPTION_IP_MAX_TPUT: {
            program_args->ipv4->tos.high_throughput = true;
            break;
        }
        case OPTION_IP_MAX_RELY: {
            program_args->ipv4->tos.high_reliability = true;
            break;
        }
        case OPTION_IP_MIN_COST: {
            program_args->ipv4->tos.low_cost = true;
            break;
        }
        case OPTION_IP_MBZ_ONE: {
            program_args->ipv4->tos.mbz_bit = true;
            break;
        }
        case OPTION_IP_DSCP: {
            program_args->ipv4->dscp = 
                (ip_dscp_code_t)parse_text_or_int(
                    IP_DSCP_CODES,
                    ARRAY_SIZE(IP_DSCP_CODES),
                    value,
                    true,
                    0,
                    64,
                    cmdline_option->name,
                    &error_occured
                );
            break;
        }
        case OPTION_IP_ECN: {
            program_args->ipv4->ecn = 
                (ip_ecn_code_t)parse_text_or_int(
                    IP_ECN_CODES,
                    ARRAY_SIZE(IP_ECN_CODES),
                    value,
                    true,
                    0,
                    3,
                    cmdline_option->name,
                    &error_occured
                );
            break;
        }
        case OPTION_IP_LEN: {
            program_args->ipv4_misc.override_length = true;

            program_args->ipv4->len =
                (uint16_t)validate_range(
                    value, 0, 65535, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_IP_IDENT: {
            program_args->ipv4->id =
                (uint16_t)validate_range(
                    value, 0, 65535, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_IP_FLAGS: {
            program_args->ipv4->flag_bits =
                (ip_flag_t)validate_range(
                    value, 0, 7, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_IP_EVIL_BIT: {
            program_args->ipv4->flags.ev = true;
            break;
        }
        case OPTION_IP_DONT_FRAG: {
            program_args->ipv4->flags.df = true;
            break;
        }
        case OPTION_IP_MORE_FRAG: {
            program_args->ipv4->flags.mf = true;
            break;
        }
        case OPTION_IP_FRAG_OFS: {
            program_args->ipv4->fragofs =
                (uint16_t)validate_range(
                    value, 0, 8191, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_IP_TTL: {
            program_args->ipv4->ttl = (uint8_t)validate_range(
                    value, 0, 255, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_IP_PROTO: {
            program_args->ipv4->proto = 
                (ip_proto_t)parse_text_or_int(
                    IP_PROTOCOLS,
                    ARRAY_SIZE(IP_PROTOCOLS),
                    value,
                    true,
                    0,
                    255,
                    cmdline_option->name,
                    &error_occured
                );
            break;
        }
        case OPTION_IP_CHECKSUM: {
            program_args->ipv4_misc.override_checksum = true;

            program_args->ipv4->chksum = (uint16_t)validate_range(
                    value, 0, 65535, cmdline_option->name,
                    &error_occured);
            break;
        }
        case OPTION_IP_OPTIONS: {
            // TODO: options
            break;
        }
        // IPv6
        case OPTION_IPV6: {
            program_args->parser.current_layer = LAYER_3;
            program_args->parser.current_proto = PROTO_L3_IPV6;
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
        // unfinished
        // TCP Header
        case OPTION_TCP: {
            program_args->parser.current_layer = LAYER_4;
            program_args->parser.current_proto = PROTO_L4_TCP;
            break;
        }
        // UDP Header
        case OPTION_UDP: {
            program_args->parser.current_layer = LAYER_4;
            program_args->parser.current_proto = PROTO_L4_UDP;
            break;
        }
        // ICMP Header
        case OPTION_ICMP: {
            program_args->parser.current_layer = LAYER_4;
            program_args->parser.current_proto = PROTO_L4_ICMP;
            break;
        }
        // Null Option (this shouldn't happen)
        case OPTION_NONE:
        default: {
            logger(LOG_CRIT, "Null option given, somehow.");
            error_occured = true;
        }
    }

    return error_occured;
}


int parse_args(
    const int argc,
    char *const argv[],
    struct ProgramArgs *const program_args
) {
    program_args->diagnostics.executable_name = argv[0];

    // If no arguments were given, print the help text and return.
    if (argc == 1) {
        fprintf(stderr, HELP_TEXT_ALL);
        return 1;
    }

    // argv[0] is the program name itself.
    for (int i = 1; i < argc; i++) {
        const char *const arg = argv[i];

        if (arg[0] == '-' || arg[0] == '/') {
            const char *const opt_name = arg + (arg[1] == '-' ? 2 : 1);
            const char *opt_val = NULL;

            for (size_t j = 0; j < ARRAY_SIZE(OPTIONS); j++) {
                const size_t opt_len =
                    strlen(OPTIONS[j].name);

                // Check if the current argument is a valid option
                if (strncmp(opt_name, OPTIONS[j].name, opt_len) == 0 &&
                    (opt_name[opt_len] == '\0' ||
                    opt_name[opt_len] == '=')
                ) {
                    // Check if the argument contains an optional
                    // '=' sign, as well as where it's located.
                    const char *const has_equal =
                        strchr(opt_name, '=');

                    // If so, the value will come after the =
                    if (has_equal) {
                        opt_val = has_equal + 1;
                    }
                    else if (i + 1 < argc && argv[i + 1][0] != '-'
                        && argv[i + 1][0] != '/'
                    ) {
                        opt_val = argv[++i];
                    }

                    if (OPTIONS[j].has_arg && (!opt_val
                        || opt_val[0] == '\0' || opt_val[0] == ' ')
                    ) {
                        logger(LOG_ERROR,
                            "Option \"--%s\" requires an argument!",
                            OPTIONS[j].name
                        );
                        return 1;
                    }

                    bool error_occured = handle_option(
                        &OPTIONS[j], opt_val, program_args);

                    if (error_occured) {
                        return 1;
                    }

                    break;
                }
                // Compared all options and found no valid match
                else if (j == ARRAY_SIZE(OPTIONS)-1) {
                    logger(LOG_ERROR,
                        "\"--%s\" is not a known option;\n"
                        "use \"--help\" for more information.",
                        opt_name
                    );
                    return 1;
                }
            }

            // Rest of your code...
        }
        else {
            // Unrecognized switch character
            fprintf(stderr, HELP_TEXT_ALL);
            return 1;
        }
    }

    return 0;
}


/*
int parse_ip_cidr(
    const char *cidr_str, struct ip_addr_range *ip_range
) {
    if (strlen(cidr_str) > MAX_IP_CIDR_LENGTH) {
        fprintf(stderr, "IP/CIDR notation is too long: %s\n", cidr_str);
        return 1;
    }

    char cidr_copy[MAX_IP_CIDR_LENGTH+1]; // +1 for null terminator
    strncpy(cidr_copy, cidr_str, sizeof (cidr_copy));
    cidr_copy[sizeof (cidr_copy) - 1] = '\0';

    char *ip_str = strtok(cidr_copy, "/");
    char *prefix_len_str = strtok(NULL, "/");

    if (ip_str == NULL || prefix_len_str == NULL) {
        fprintf(stderr, "Invalid IP/CIDR notation: %s\n", cidr_str);
        return 1;
    }

    struct in_addr ip_addr;
    if (inet_pton(AF_INET, ip_str, &ip_addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_str);
        return 1;
    }

    int prefix_len = atoi(prefix_len_str);
    if (prefix_len < 0 || prefix_len > 32) {
        fprintf(stderr, "Invalid prefix length: %s\n", prefix_len_str);
        return 1;
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

    return 0;
}

int parse_ip_port(
    const char *ip_port_str, ip_addr_t *ip, int *port
) {
    if (strlen(ip_port_str) > MAX_IP_PORT_LENGTH) {
        fprintf(stderr, "IP:Port notation is too long: %s\n",
                        ip_port_str);
        return 1;
    }

    char ip_port_copy[MAX_IP_PORT_LENGTH + 1];
    strncpy(ip_port_copy, ip_port_str, sizeof (ip_port_copy));
    ip_port_copy[sizeof (ip_port_copy) - 1] = '\0';

    char *ip_str = strtok(ip_port_copy, ":");
    char *port_str = strtok(NULL, ":");

    if (ip_str == NULL || port_str == NULL) {
        fprintf(stderr, "Invalid IP:Port notation: %s\n",
                        ip_port_str);
        return 1;
    }

    ip->address = inet_addr(ip_str);
    if (ip->address  == INADDR_NONE) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_str);
        return 1;
    }

    *port = atoi(port_str);
    if (*port < PORT_MIN || *port > PORT_MAX) {
        fprintf(stderr, "Invalid port: %d\n", *port);
        return 1;
    }

    return 0;
}
*/


// ---------------------------------------------------------------------
// END OF FILE: parser.c
// ---------------------------------------------------------------------
