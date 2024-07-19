// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// netinet.h is a part of Blitzping.
// ---------------------------------------------------------------------

#pragma once
#ifndef NETINET_H
#define NETINET_H


// NOTE: Unfortunately, <netinet/ip.h> is not a part of the POSIX
// standard, while <netinet/in.h> and <netinet/tcp.h> somehow are.
// However, even the latter are rather useless, because most of their
// definitions are hidden behind non-standard BSD or System V-specific
// feature flags.  In any case, the headers defined there are far
// too outdated, still referring to many fields as "reserved."
// For that reason, and for future compatibility with Win32,
// it is better to define our own protocol header structures.


// NOTE: These included header files should not actually make this
// file dependent upon libc (at runtime), because they are all
// "compile-time" dependencies (except assert() that can be disabled).
#include <stdint.h>
#include <stdbool.h>
// Uncomment (or define via compiler) to disable runtime assert()
//#define NDEBUG
#include <assert.h> // Defines _Static_assert() to be static_assert()


// NOTE: Binary integer literals (e.g., 0b10101010) are a GNU extension;
// unfortunately, they are not part of the C11 standard. (C23 N2549)

// NOTE: C does not let you put "flexible array members" inside
// of unions (or nested structs in unions), even if they both
// have the exact same type...

// NOTE: You can not define anonymous members of unions inside structs,
// so you will have to copy-paste the same union definition for each.
// To make it easier, I had to define some union definitions as pre-
// processor macros.  This is especially imperative if your union
// contains non-byte bitfields (which are expected to get packed
// in the exterior struct), because the compiler does not seem
// to honor the "internal" bitfields inside of nested unions/structs.

// NOTE: Do NOT pack the "main" structs (e.g., ip_hdr or tcp_hdr);
// they will be casted to pointers, and we don't want half-words there.

// NOTE: You cannot have namespaces (i.e., enums inside named structs)
// in C, unlike C++.  For that reason, make sure to use prefixes to
// properly denote enums/defines, such as TCP_FLAGS_SYN instead of
// just SYN or TCP_SYN.

// NOTE: The C Standardization Committee was sometimes so out-of-
// touch with their decisions; namely, at least before C23, you are
// unable to specify the underlying "type" of an enum.  This means that
// an enum with values that are all between 0-255 "may or may not"
// result in a uint8_t, depending on the compiler's discretion.
// Yes, C was supposed to be a "portable assembler," but it is also
// widely used as a systems programming language, where bit-level and
// endianness control is crucial. (C23 N3030)
// Check for C23 support
#if __STDC_VERSION__ >= 202300L // C23 or later
#   define ENUM_UNDERLYING(type) : type
#else // Fallback for older standards (e.g., C11)
#   define ENUM_UNDERLYING(type) __attribute__((packed))
#endif


typedef enum endianness {
    little_endian,
    big_endian
} endianness_t;

// Check endianness at compile-time for the target machine
#if defined(__BYTE_ORDER__) && !(defined(__LITTLE_ENDIAN__) \
                                || defined(__BIG_ENDIAN__))
#   if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#       define __LITTLE_ENDIAN__
#       define TARGET_ENDIANNESS little_endian
#   elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#       define __BIG_ENDIAN__
#       define TARGET_ENDIANNESS big_endian
#   else
#       pragma message("Unknown endianness: \n" \
            "define \'__LITTLE_ENDIAN__\' or \'__BIG_ENDIAN__\'.")
#   endif
#elif defined(__LITTLE_ENDIAN__)
#   define TARGET_ENDIANNESS little_endian
#elif defined(__BIG_ENDIAN__)
#   define TARGET_ENDIANNESS big_endian
#endif

// Double-check endianness at runtime on the target machine.
inline endianness_t check_endianness() {
    union {
        int word;
        char byte[sizeof (int)];
    } same_memory;
    same_memory.word = 1;
    // Check if the least-significant stored byte is 1 (little endian)
    endianness_t runtime_endianness = (same_memory.byte[0] == 1) ?
                                        little_endian : big_endian;

    return runtime_endianness;
}


/*  START OF THE
    Internet Protocol 4 (IPv4)
    Header Structure
*/

// IP Address type(s)
typedef union ip_addr_view {
    uint32_t address;  // IP address in uint32_t form
    uint8_t octets[4]; // Four octets (bytes) notation
} ip_addr_t;


// IP Protocol definitions
#pragma pack(push)
typedef enum __attribute__((packed)) ip_proto {
    IP_PROTO_IP   = 0,
    IP_PROTO_ICMP = 1,
    IP_PROTO_TCP  = 6,
    IP_PROTO_UDP  = 17,
} ip_proto_t;
#pragma pack(pop)


#pragma pack(push)
typedef enum __attribute__((packed)) ip_tos_prec {
    // RFC 791
    IP_TOS_PREC_ROUTINE              = 1, // 0b000
    IP_TOS_PREC_PRIORITY             = 2, // 0b001
    IP_TOS_PREC_IMMEDIATE            = 3, // 0b010
    IP_TOS_PREC_FLASH                = 4, // 0b011
    IP_TOS_PREC_FLASH_OVERRIDE       = 5, // 0b100
    IP_TOS_PREC_CRITIC_ECP           = 5, // 0b101
    IP_TOS_PREC_INTERNETWORK_CONTROL = 6, // 0b110
    IP_TOS_PREC_NETWORK_CONTROL      = 7, // 0b111
} ip_tos_prec_t;
#pragma pack(pop)

// ECN Operations definitions
// (NOT backward-compatible with the old ToS field.)
#pragma pack(push)
typedef enum __attribute__((packed)) ip_ecn_code {
    // RFC 3168
    IP_ECN_NOT_ECT = 0, // 0b00: Not ECN-Capable Transport
    // RFC 8311 / RFC Errata 5399 / RFC 9331
    IP_ECN_ECT_1   = 1, // 0b01: ECN-Capable Transport 1 (experimental use)
    // RFC 3168
    IP_ECN_ECT_0   = 2, // 0b10: ECN-Capable Transport 0
    IP_ECN_CE      = 3, // 0b11: Congestion Experienced (CE)
} ip_ecn_code_t;
#pragma pack(pop)

// DSCP definitions / Per-Hop Behaviors
// (somewhat backward-compatible with the old ToS precedence bits.)
// iana.org/assignments/dscp-registry/dscp-registry.xhtml
#pragma pack(push)
typedef enum __attribute__((packed)) ip_dscp_code {
    /* DSCP Pool 1 Codepoints () */
    // RFC 2474 (Class Selector PHBs)
    IP_DSCP_DF   = 0,  // Default Forwarding (DF) PHB
    IP_DSCP_CS0  = 0,  // CS 0 (standard)
    IP_DSCP_CS1  = 8,  // CS 1 (low-priority data)
    IP_DSCP_CS2  = 16, // CS 2 (network operations/OAM)
    IP_DSCP_CS3  = 24, // CS 3 (broadcast video))
    IP_DSCP_CS4  = 32, // CS 4 (real-time interactive)
    IP_DSCP_CS5  = 40, // CS 5 (signaling)
    IP_DSCP_CS6  = 48, // CS 6 (network control)
    IP_DSCP_CS7  = 56, // CS 7 (reserved)
    // RFC 2597 (Assured Forwarding (AF) PHB)
    IP_DSCP_AF11 = 10, // AF 11 (high-throughput)
    IP_DSCP_AF12 = 12, // AF 12 (high-throughput)
    IP_DSCP_AF13 = 14, // AF 13 (high-throughput)
    IP_DSCP_AF21 = 18, // AF 21 (low-latency)
    IP_DSCP_AF22 = 20, // AF 22 (low-latency)
    IP_DSCP_AF23 = 22, // AF 23 (low-latency)
    IP_DSCP_AF31 = 26, // AF 31 (multimedia stream)
    IP_DSCP_AF32 = 28, // AF 32 (multimedia stream)
    IP_DSCP_AF33 = 30, // AF 33 (multimedia stream)
    IP_DSCP_AF41 = 34, // AF 41 (multimedia conference)
    IP_DSCP_AF42 = 36, // AF 42 (multimedia conference)
    IP_DSCP_AF43 = 38, // AF 43 (multimedia conference)
    // RFC 3246 (Expedited Forwarding (EF) PHB)
    IP_DSCP_EF   = 46, // EF (telephony)
    // RFC 5865 (Voice-Admit)
    IP_DSCP_VA   = 44, // Voice-Admit
    /* DSCP Pool 2 Codepoints */
    // "Reserved for Experimental and Local Use" (empty)
    /* DSCP Pool 3 Codepoints */
    // RFC 8622
    IP_DSCP_LE   = 1,  // Lower-Effort PHB
} ip_dscp_code_t;
#pragma pack(pop)

// The "Type of Service" field is largely unused (or ignored) nowadays.
// It went through many changes, which makes it confusing to see how it
// maps out to values.  I included the original (now deprecated) ToS
// field with its expanded bits as well as the "modern" DSCP + ECN.
union ip_tos_view {
    uint8_t bitfield;
    // RFC 791 and RFC 1349 (deprecated)
    struct { // ToS
#if defined(__LITTLE_ENDIAN__)
        bool          must_be_zero     : 1;
        bool          low_cost         : 1; // RFC 1349
        bool          high_reliability : 1;
        bool          high_throughput  : 1;
        bool          low_delay        : 1;
        ip_tos_prec_t precedence       : 3;
#elif defined(__BIG_ENDIAN__)
        ip_tos_prec_t precedence       : 3;
        bool          low_delay        : 1;
        bool          high_throughput  : 1;
        bool          high_reliability : 1;
        bool          low_cost         : 1;
        bool          must_be_zero     : 1;
#endif
    };
};

// IP flag definitions
#pragma pack(push)
typedef enum __attribute__((packed)) ip_flags {
    IP_FLAG_EV = 1 << 2, // 0b100: Reserved bit (RFC 3514 "evil bit")
    IP_FLAG_DF = 1 << 1, // 0b010: Don't Fragment
    IP_FLAG_MF = 1 << 0, // 0b001: More Fragments
} ip_flag_t;
#pragma pack(pop)

// Cannot nest bitfields and pack them externally; read the top note.
//
// Basically, because the fragment offset (fragofs) and flags field
// in an IPv4 packet are packed in a weird way (13 + 3 bits = 16),
// C compilers packs those 3 bits (if they are defined inside their
// own struct) into a full 8-bit byte, meaning that it becomes 13 + 8
// = 21 bits.  To fix that, I had to copy-paste this otherwise-neat
// union inside the ip_hdr struct and also add some hacky padding
// ("uint16_t __fragofs_bug : 13;") to work around this issue.
union ip_flag_view {
    // View no. 1 (as an entire bitfield)
    ip_flag_t bitfield : 3;
    // View no. 2 (with expanded flag bits)
    struct {
#if defined(__LITTLE_ENDIAN__)
        bool mf : 1; // More Fragments
        bool df : 1; // Don't Fragment
        bool ev : 1; // Evil/reserved bit
#elif defined(__BIG_ENDIAN__)
        bool ev : 1;
        bool df : 1;
        bool mf : 1;
#endif
    };
};

typedef struct ip_hdr {
#if defined(__LITTLE_ENDIAN__)
    uint8_t                 ihl       : 4;  // Internet Header Length
    uint8_t                 ver       : 4;  // IP Version
#elif defined(__BIG_ENDIAN__)
    uint8_t                 ver       : 4; 
    uint8_t                 ihl       : 4;
#endif
    union {
        // RFC 791 and RFC 1349
        union ip_tos_view   tos;            // Type of Service (ToS)
        // RFC 2474 (DSCP + ECN)
        struct {
#if defined(__LITTLE_ENDIAN__)
            ip_ecn_code_t   ecn       : 2;  // Explicit Congestion Notif
            ip_dscp_code_t  dscp      : 6;  // Differentiated Services
#elif defined(__BIG_ENDIAN__)
            ip_dscp_code_t  dscp      : 6;
            ip_ecn_code_t   ecn       : 2;
#endif
        };
    };
    uint16_t                len;            // Total Length
    uint16_t                id;             // Identification
#if defined(__LITTLE_ENDIAN__)
union {
    struct {
    uint16_t                fragofs   : 13; // Fragment Offset
    //union ip_flag_view    flags;          // "Would-be" flags
    ip_flag_t               flag_bits : 3;  // Workaround flag bitfield
};
struct {
    uint16_t __struct_bug__ : 13; // [Read ip_flag_view note] (*)
    uint8_t                 mf        : 1;  // More Fragments
    uint8_t                 df        : 1;  // Don't Fragment
    uint8_t                 ev        : 1;  // Evil/reserved bit
    } flags;
};
#elif defined(__BIG_ENDIAN__)
union {
    struct {
    ip_flag_t               flag_bits : 3;
    //union ip_flag_view    flags;        
    uint16_t                fragofs   : 13;
};
struct {
    uint8_t                 mf        : 1;
    uint8_t                 df        : 1;
    uint8_t                 ev        : 1;
    uint16_t __struct_bug__ : 13; // [Read ip_flag_view note] (*)
    } flags;
};
#endif
    uint8_t                 ttl;            // Time to live
    ip_proto_t              proto   : 8;    // Protocol
    uint16_t                chksum;         // Header Checksum
    ip_addr_t               saddr;          // Source Address
    ip_addr_t               daddr;          // Destination Address
    //uint32_t              options[];      // IP Options (0-320 bits)
} ip_hdr_t;
// TODO: Make a struct for IP options
static_assert(sizeof (ip_hdr_t) == 20,
            "An empty ip_hdr struct should only be 20 bytes!");

/*  END OF THE
    Internet Protocol (IP)
    Header Structure
*/


/*  START OF THE
    Transmission Control Protocol (TCP)
    Header Structure
*/

// TCP flag definitions
#pragma pack(push)
typedef enum __attribute__((packed)) tcp_flags {
    TCP_FLAG_FIN = 1 << 0, // 0b00000001: Finish
    TCP_FLAG_SYN = 1 << 1, // 0b00000010: Synchronize
    TCP_FLAG_RST = 1 << 2, // 0b00000100: Reset
    TCP_FLAG_PSH = 1 << 3, // 0b00001000: Push
    TCP_FLAG_ACK = 1 << 4, // 0b00010000: Acknowledgment
    TCP_FLAG_URG = 1 << 5, // 0b00100000: Urgent
    TCP_FLAG_ECE = 1 << 6, // 0b01000000: ECN-Echo
    TCP_FLAG_CWR = 1 << 7, // 0b10000000: Congestion Window Reduced
} tcp_flag_t;
#pragma pack(pop)
//static_assert(sizeof (tcp_flag_t) == sizeof (uint8_t),
//            "TCP Flag field should not be different from a uint8_t!");

union tcp_flag_view {
    // View no. 1 (as an entire byte)
    tcp_flag_t bitfield : 8;
    // View no. 2 (with expanded flag bits)
    struct {
#if defined(__LITTLE_ENDIAN__)
        bool fin : 1; // Finish
        bool syn : 1; // Synchronize
        bool rst : 1; // Reset
        bool psh : 1; // Push
        bool ack : 1; // Acknowledgment
        bool urg : 1; // Urgent
        bool ece : 1; // ECN-Echo
        bool cwr : 1; // Congestion Window Reduced
#elif defined(__BIG_ENDIAN__)
        bool cwr : 1;
        bool ece : 1;
        bool urg : 1;
        bool ack : 1;
        bool psh : 1;
        bool rst : 1;
        bool syn : 1;
        bool fin : 1;
#endif
    };
};

typedef struct tcp_hdr {
    uint16_t             sport;        // Source port number
    uint16_t             dport;        // Destination port number
    uint32_t             seqnum;       // Sequence number
    uint32_t             acknum;       // Acknowledgement number
#if defined(__LITTLE_ENDIAN__)
    uint8_t              reserved : 4; // Reserved (unused) bits
    uint8_t              dataofs  : 4; // Offset to data/options
#elif defined(__BIG_ENDIAN__)
    uint8_t              dataofs  : 4; // Offset to data/options
    uint8_t              reserved : 4; // Reserved (unused) bits
#endif
    union tcp_flag_view  flags;        // TCP flags
    uint16_t             window;       // Window size
    uint16_t             chksum;       // Checksum
    uint16_t             urgptr;       // Pointer to urgent data
    //uint32_t           options[];    // TCP Options (0-320 bits)
} tcp_hdr_t;
// TODO: Make options a struct
static_assert(sizeof (tcp_hdr_t) == 20,
            "An empty tcp_hdr struct should only be 20 bytes!");


/*  END OF THE
    Transmission Control Protocol (TCP)
    Header Structure
*/


#endif // NETINET_H

// ---------------------------------------------------------------------
// END OF FILE: netinet.h
// ---------------------------------------------------------------------
