//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

// fixmath.h -- 64-bit products and quotients without libgcc: muls.l / divs.l on the 68020-040 (from lov.re).
#ifndef ALIS_FIXMATH_H
#define ALIS_FIXMATH_H

#include "config.h"

#if defined(ALIS_USE_NATIVE_ATARI) && defined(__m68k__) && !defined(__mcoldfire__)
#define FX_ASM 1
extern u8 sys_mul64;
#else
#define FX_ASM 0
#endif

static inline __attribute__((always_inline)) s64 fx_mul64(s32 a, s32 b)
{
#if FX_ASM
    if (sys_mul64)
    {
        s32 hi;
        __asm__("muls.l %2,%0:%1" : "=d"(hi), "+d"(a) : "d"(b) : "cc");
        return (s64)(((u64)(u32)hi << 32) | (u32)a);
    }
#endif
    return (s64)a * b;
}

static inline __attribute__((always_inline)) s32 fx_div64(s64 num, s32 den)
{
#if FX_ASM
    if (sys_mul64)
    {
        s32 hi = (s32)(num >> 32), lo = (s32)num;
        __asm__("divs.l %2,%0:%1" : "+d"(hi), "+d"(lo) : "d"(den) : "cc");
        return lo;
    }
#endif
    return (s32)(num / den);
}

#endif
