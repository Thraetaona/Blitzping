// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// ip.h is a part of Blitzping.
// ---------------------------------------------------------------------

_Pragma ("once")
#ifndef IP_H
#define IP_H


// IP Address type(s)
typedef union ip_addr_view {
    uint32_t address;  // IP address in uint32_t form
    uint8_t octets[4]; // Four octets (bytes) notation
} ip_addr_t;


// IP Protocol definitions
_Pragma ("pack(push)")
typedef enum __attribute__((packed)) ip_proto {
    IP_PROTO_IP   = 0,
    IP_PROTO_ICMP = 1,
    IP_PROTO_TCP  = 6,
    IP_PROTO_UDP  = 17,
} ip_proto_t;
_Pragma ("pack(pop)")


_Pragma ("pack(push)")
typedef enum __attribute__((packed)) ip_tos_prec {
    // RFC 791
    IP_TOS_PREC_ROUTINE              = 0, // 0b000
    IP_TOS_PREC_PRIORITY             = 1, // 0b001
    IP_TOS_PREC_IMMEDIATE            = 2, // 0b010
    IP_TOS_PREC_FLASH                = 3, // 0b011
    IP_TOS_PREC_FLASH_OVERRIDE       = 4, // 0b100
    IP_TOS_PREC_CRITIC_ECP           = 5, // 0b101
    IP_TOS_PREC_INTERNETWORK_CONTROL = 6, // 0b110
    IP_TOS_PREC_NETWORK_CONTROL      = 7, // 0b111
} ip_tos_prec_t;
_Pragma ("pack(pop)")

// ECN Operations definitions
// (NOT backward-compatible with the old ToS field.)
_Pragma ("pack(push)")
typedef enum __attribute__((packed)) ip_ecn_code {
    // RFC 3168
    IP_ECN_NOT_ECT = 0, // 0b00: Not ECN-Capable Transport
    // RFC 8311 / RFC Errata 5399 / RFC 9331
    IP_ECN_ECT_1   = 1, // 0b01: ECN-Capable Transport 1 (experimental)
    // RFC 3168
    IP_ECN_ECT_0   = 2, // 0b10: ECN-Capable Transport 0
    IP_ECN_CE      = 3, // 0b11: Congestion Experienced (CE)
} ip_ecn_code_t;
_Pragma ("pack(pop)")

// DSCP definitions / Per-Hop Behaviors
// (somewhat backward-compatible with the old ToS precedence bits.)
// iana.org/assignments/dscp-registry/dscp-registry.xhtml
_Pragma ("pack(push)")
typedef enum __attribute__((packed)) ip_dscp_code {
    /* DSCP Pool 1 Codepoints */
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
    // RFC 2597 (Assured Forwarding [AF] PHB)
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
    // RFC 3246 (Expedited Forwarding [EF] PHB)
    IP_DSCP_EF   = 46, // EF (telephony)
    // RFC 5865 (Voice-Admit)
    IP_DSCP_VA   = 44, // Voice-Admit
    /* DSCP Pool 2 Codepoints */
    // "Reserved for Experimental and Local Use" (empty)
    /* DSCP Pool 3 Codepoints */
    // RFC 8622
    IP_DSCP_LE   = 1,  // Lower-Effort PHB
} ip_dscp_code_t;
_Pragma ("pack(pop)")

// The "Type of Service" field is largely unused (or ignored) nowadays.
// It went through many changes, which makes it confusing to see how it
// maps out to values.  I included the original (now deprecated) ToS
// field with its expanded bits as well as the "modern" DSCP + ECN.
union ip_tos_view {
    uint8_t bitfield;
    // RFC 791 and RFC 1349 (deprecated)
    struct { // ToS
#if defined(__LITTLE_ENDIAN__)
        bool          mbz_bit          : 1; // "Must-be-Zero" bit
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
        bool          mbz_bit          : 1;
#endif
    };
};

// IP flag definitions
_Pragma ("pack(push)")
typedef enum __attribute__((packed)) ip_flag {
    IP_FLAG_EV = 1 << 2, // 0b100: Reserved bit (RFC 3514 "evil bit")
    IP_FLAG_DF = 1 << 1, // 0b010: Don't Fragment
    IP_FLAG_MF = 1 << 0, // 0b001: More Fragments
} ip_flag_t;
_Pragma ("pack(pop)")

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


//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |Version|  IHL  |    DSCP   |ECN|          Total Length         |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         Identification        |Flags|      Fragment Offset    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  Time to Live |    Protocol   |         Header Checksum       |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                       Source Address                          |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    Destination Address                        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    Options                    |    Padding    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
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
    bool                    mf        : 1;  // More Fragments
    bool                    df        : 1;  // Don't Fragment
    bool                    ev        : 1;  // Evil/reserved bit
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
    bool                    mf        : 1;
    bool                    df        : 1;
    bool                    ev        : 1;
    uint16_t __struct_bug__ : 13; // [Read ip_flag_view note] (*)
    } flags;
};
#endif
    uint8_t                 ttl;            // Time to live
    ip_proto_t              proto     : 8;  // Protocol
    uint16_t                chksum;         // Header Checksum
    ip_addr_t               saddr;          // Source Address
    ip_addr_t               daddr;          // Destination Address
    uint32_t                options[];      // IP Options (0-320 bits)
} ip_hdr_t;
// TODO: Make a struct for IP options
_Static_assert(sizeof (ip_hdr_t) == 20,
            "An empty ip_hdr struct should only be 20 bytes!");



#endif // IP_H

// ---------------------------------------------------------------------
// END OF FILE: ip.h
// ---------------------------------------------------------------------
