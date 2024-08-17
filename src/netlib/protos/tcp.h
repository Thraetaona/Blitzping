// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// tcp.h is a part of Blitzping.
// ---------------------------------------------------------------------

_Pragma ("once")
#ifndef TCP_H
#define TCP_H


// TCP flag definitions
_Pragma ("pack(push)")
typedef enum __attribute__((packed)) tcp_flag {
    TCP_FLAG_FIN = 1 << 0, // 0b00000001: Finish
    TCP_FLAG_SYN = 1 << 1, // 0b00000010: Synchronize
    TCP_FLAG_RST = 1 << 2, // 0b00000100: Reset
    TCP_FLAG_PSH = 1 << 3, // 0b00001000: Push
    TCP_FLAG_ACK = 1 << 4, // 0b00010000: Acknowledgment
    TCP_FLAG_URG = 1 << 5, // 0b00100000: Urgent
    TCP_FLAG_ECE = 1 << 6, // 0b01000000: ECN-Echo
    TCP_FLAG_CWR = 1 << 7, // 0b10000000: Congestion Window Reduced
} tcp_flag_t;
_Pragma ("pack(pop)")

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
        bool ece : 1; // ECN-Echo (RFC 3168)
        bool cwr : 1; // Congestion Window Reduced (RFC 3168)
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

// TCP options definitions
// iana.org/assignments/tcp-parameters/tcp-parameters.xhtml
// NOTE: [*] indicates that the option is considered "obsolete."
_Pragma ("pack(push)")
typedef enum __attribute__((packed)) tcp_option_kind {
    // RFC 9293
    TCP_OPT_KIND_EOL            = 0,   // End of Option List
    TCP_OPT_KIND_NOP            = 1,   // No-Operation
    TCP_OPT_KIND_MSS            = 2,   // Maximum Segment Size
    // RFC 7323
    TCP_OPT_KIND_WS             = 3,   // Window Scale
    // RFC 2018
    TCP_OPT_KIND_SACK_PERM      = 4,   // SACK Permitted
    TCP_OPT_KIND_SACK           = 5,   // SACK Block
    // RFC 1072 & RFC 6247
    TCP_OPT_KIND_ECHO           = 6,   // [*] Echo (8)
    TCP_OPT_KIND_ECHO_REPLY     = 7,   // [*] Echo-Reply (8)
    // RFC 7323
    TCP_OPT_KIND_TIMESTAMP      = 8,   // Timestamps
    // RFC 1693 & RFC 6247
    TCP_OPT_KIND_PARTIAL_PERM   = 9,   // [*] Part Order Connect Perm.
    TCP_OPT_KIND_PARTIAL_PROF   = 10,  // [*] Part Order Serve Profile
    TCP_OPT_KIND_CC             = 11,  // [*] Connection Count
    TCP_OPT_KIND_CC_NEW         = 12,  // [*] Connection Count New
    TCP_OPT_KIND_CC_ECHO        = 13,  // [*] Connection Count Echo
    TCP_OPT_KIND_ALT_CHK_REQ    = 14,  // [*] Alternative Chksum Req.
    TCP_OPT_KIND_ALT_CHK_DAT    = 15,  // [*] Alternative Chksum Data
    // "Stev Knowles"
    TCP_OPT_KIND_SKEETER        = 16,  // Skeeter (???)
    TCP_OPT_KIND_BUBBA          = 17,  // Bubba (???)
    // "Subbu Subramaniam" & "Monroe Bridges"
    TCP_OPT_KIND_TRAILER_CHK    = 18,  // Trailer Checksum Option
    // RFC 2385
    TCP_OPT_KIND_MD5_SIG        = 19,  // [*] MD5 Signature (29)
    // "Keith Scott"
    TCP_OPT_KIND_SCPS_CAPABLE   = 20,  // SCPS Capabilities
    TCP_OPT_KIND_SEL_NEG_ACK    = 21,  // Selective Negative Ack.
    TCP_OPT_KIND_RECORD_BOUNDS  = 22,  // Record Boundaries
    TCP_OPT_KIND_CORRUPT_EXP    = 23,  // Corruption Experienced
    TCP_OPT_KIND_SNAP           = 24,  // SNAP (???)
    // -blank-
    TCP_OPT_KIND_UNASSIGNED25   = 25,  // Unassigned (12/28/2000)
    // "Steve Bellovin"
    TCP_OPT_KIND_COMPRESS_FILT  = 26,  // Compression Filter
    // RFC 4782
    TCP_OPT_KIND_QUICK_START    = 27,  // Quick-Start Response
    // RFC 5482
    TCP_OPT_KIND_USER_TIMEOUT   = 28,  // User Timeout (other uses)
    // RFC 5925
    TCP_OPT_KIND_AUTH_OPT       = 29,  // Authentication (TCP-AO)
    // RFC 8684
    TCP_OPT_KIND_MULTIPATH      = 30,  // Multipath TCP (MPTCP)
    // [31-33] RESERVED
    // RFC 7413
    TCP_OPT_KIND_FAST_OPEN_COOK = 34,  // Fast Open Cookie
    // [35-68] RESERVED
    // RFC 8547
    TCP_OPT_KIND_ENCRYPT_NEGOT  = 69,  // Encryption Negotiatio
    // [70-171] RESERVED
    TCP_OPT_KIND_ACC_ECN_ORD0   = 172, // Accurate ECN Order 0
    // [173] RESERVED
    TCP_OPT_KIND_ACC_ECN_ORD1   = 174, // Accurate ECN Order 1
    // [175-252] RESERVED
    TCP_OPT_KIND_RFC3692_EXP1   = 253, // RFC 3692 Experiment 1
    TCP_OPT_KIND_RFC3692_EXP2   = 254, // RFC 3692 Experiment 2
} tcp_option_kind_t;
_Pragma ("pack(pop)")

//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |          Source Port          |       Destination Port        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                        Sequence Number                        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    Acknowledgment Number                      |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  Data |       |C|E|U|A|P|R|S|F|                               |
//   | Offset| Rsrvd |W|C|R|C|S|S|Y|I|            Window             |
//   |       |       |R|E|G|K|H|T|N|N|                               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |           Checksum            |         Urgent Pointer        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                           [Options]                           |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                                                               :
//   :                             Data                              :
//   :                                                               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
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
    uint32_t             options[];    // TCP Options (0-320 bits)
} tcp_hdr_t;
// TODO: Make options a struct
_Static_assert(sizeof (tcp_hdr_t) == 20,
            "An empty tcp_hdr struct should only be 20 bytes!");


#endif // TCP_H

// ---------------------------------------------------------------------
// END OF FILE: tcp.h
// ---------------------------------------------------------------------
