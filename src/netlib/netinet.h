// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// netinet.h is a part of Blitzping.
// ---------------------------------------------------------------------

_Pragma ("once")
#ifndef NETINET_H
#define NETINET_H


// NOTE: Unfortunately, <netinet/ip.h> is NOT a part of the POSIX
// standard, while <netinet/in.h> and <netinet/tcp.h> somehow are.
// However, even the latter are rather useless, because most of their
// definitions are hidden behind non-standard BSD or System V-specific
// feature flags.  In any case, the headers defined there are far
// too outdated, still referring to many fields as "reserved."
// For that reason, and for future compatibility with Win32,
// it is better to define our own protocol header structures.


// NOTE: These included header files should not actually make this
// file dependent upon libc (at runtime), because they are all
// "compile-time" dependencies.  In other words, they should compile
// just fine under a -ffreestanding or -nolibc environment.
#include <stdint.h>
#include <stdbool.h>


// NOTE: Binary integer literals (e.g., 0b10101010) are a GNU extension;
// unfortunately, they are not part of the C11 standard. (C23 N2549)

// NOTE: Use the standard C99 _Pragma statement rather than #pragma.

// NOTE: You cannot use enums directly inside macro expansions!

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


// NOTE: Unfortunately, only GCC supports the scalar_storage_order
// attribute/pragma, and LLVM/MSVC don't have it yet; for that reason,
// we have to resort to old-fashioned macro definitions.
// https://km.kkrach.de/p_little_vs_big_endian
// https://github.com/llvm/llvm-project/issues/34641
#include "../utils/endian.h"


/* Protocol Definitions */
#include "./protos/ip.h"
#include "./protos/tcp.h"

typedef enum osi_layer {
    LAYER_2,
    LAYER_3,
    LAYER_4
} osi_layer_t;

typedef enum osi_protocol {
    // Layer 3 (Network)
    PROTO_L3_RAW,
    PROTO_L3_IPV4,
    PROTO_L3_IPV6,
    // Layer 4 (Transport)
    PROTO_L4_RAW,
    PROTO_L4_TCP,
    PROTO_L4_UDP,
    PROTO_L4_ICMP // ICMP shouldn't belong in L3.
} osi_proto_t;


#endif // NETINET_H

// ---------------------------------------------------------------------
// END OF FILE: netinet.h
// ---------------------------------------------------------------------
