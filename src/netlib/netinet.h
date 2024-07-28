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
// "compile-time" dependencies (except assert() that can be disabled).
#include <stdint.h>
#include <stdbool.h>
// Uncomment (or define via compiler) to disable runtime assert()
//#define NDEBUG
#include <assert.h> // Defines _Static_assert() to be static_assert()


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

typedef enum endianness {
    little_endian,
    big_endian,
} endianness_t;

// Check endianness at compile-time for the target machine
#if defined(__BYTE_ORDER__) && !(defined(__LITTLE_ENDIAN__) \
                                || defined(__BIG_ENDIAN__))
#   if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#       define __LITTLE_ENDIAN__
#   elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#       define __BIG_ENDIAN__
#   else
#       error "Define endianness: __LITTLE_ENDIAN__ or __BIG_ENDIAN__."
#   endif
#endif

// Double-check endianness at runtime on the target machine.
inline endianness_t check_endianness() {
    union {
        int  word;
        char byte[sizeof (int)];
    } same_memory;

    same_memory.word = 1;
    // Check if the least-significant stored byte is 1 (little endian)
    endianness_t runtime_endianness = (same_memory.byte[0] == 1) ?
                                        little_endian : big_endian;

    return runtime_endianness;
}


/* Protocol Definitions */
#include "./protos/ip.h"
#include "./protos/tcp.h"


#endif // NETINET_H

// ---------------------------------------------------------------------
// END OF FILE: netinet.h
// ---------------------------------------------------------------------