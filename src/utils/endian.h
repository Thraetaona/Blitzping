// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// endian.h is a part of Blitzping.
// ---------------------------------------------------------------------

_Pragma ("once")
#ifndef ENDIAN_H
#define ENDIAN_H


typedef enum Endianness {
    little_endian,
    big_endian,
} endianness_t;

// NOTE: Unfortunately, only GCC supports the scalar_storage_order
// attribute/pragma, and LLVM/MSVC don't have it yet; for that reason,
// we have to resort to old-fashioned macro definitions.
// https://km.kkrach.de/p_little_vs_big_endian
// https://github.com/llvm/llvm-project/issues/34641

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
inline endianness_t check_endianness(void) {
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


#endif // ENDIAN_H

// ---------------------------------------------------------------------
// END OF FILE: endian.h
// ---------------------------------------------------------------------
