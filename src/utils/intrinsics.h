// ---------------------------------------------------------------------
// SPDX-License-Identifier: GPL-3.0-or-later
// intrinsics.h is a part of Blitzping.
// ---------------------------------------------------------------------

_Pragma ("once")
#ifndef INTRINSICS_H
#define INTRINSICS_H


#if defined (__GNUC__) || defined (__llvm__)
#   define PREFETCH(addr, rw, locality) ( \
        __builtin_prefetch(addr, rw, locality) \
    )
#elif defined(_MSC_VER)
#   include <xmmintrin.h>
#   define PREFETCH(addr, rw, locality) ( \
        (void)(rw), (void)(locality), \
        _mm_prefetch((char*)(addr), _MM_HINT_T0) \
    )
#else
#   define PREFETCH(addr, rw, locality) ( \
        (void)(addr), (void)(rw), (void)(locality)
    )
#endif

#endif // INTRINSICS_H

// ---------------------------------------------------------------------
// END OF FILE: intrinsics.h
// ---------------------------------------------------------------------
