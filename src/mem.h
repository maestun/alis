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

#pragma once

#include "config.h"

typedef u16 (*fRead16)(const u8 *);
typedef u32 (*fRead24)(const u8 *);
typedef u32 (*fRead32)(const u8 *);

extern fRead16  read16;
extern fRead24  read24;
extern fRead32  read32;

extern fRead16  read16be;
extern fRead32  read32be;

extern fRead16  read16le;
extern fRead32  read32le;

void write32(const u8 *ptr, u32 value);

u16 fread16(FILE * fp);
u32 fread32(FILE * fp);
u16 swap16(const u8 *);
u32 swap32(const u8 *);

u8 *get_vram(s16 offset);

// VM memory base and endianness flag — set by vram_init() and alis_load_main().
// Used by the NDEBUG inline xread/xwrite path; always defined so alis.c can set them.
extern u8 *_xmem;
extern u8  _xle;
extern u8  _xwc;

#ifdef ALIS_MEM_NATIVE_ENDIAN
// Compile-time native endian: zero overhead direct access
#define xread8(o)                   (*(u8  *)(alis.mem + (o)))
#define xread16(o)                  ((s16)*(u16 *)(alis.mem + (o)))
#define xread32(o)                  ((s32)*(u32 *)(alis.mem + (o)))

#define xwrite8(o, v)               (*(u8  *)(alis.mem + (o)) = (u8 )(v))
#define xwrite16(o, v)              (*(s16 *)(alis.mem + (o)) = (s16)(v))
#define xwrite32(o, v)              (*(s32 *)(alis.mem + (o)) = (s32)(v))

#define xadd8(o, v)                 (*(s8  *)(alis.mem + (o)) += (s8 )(v))
#define xadd16(o, v)                (*(s16 *)(alis.mem + (o)) += (s16)(v))
#define xadd32(o, v)                (*(s32 *)(alis.mem + (o)) += (s32)(v))

#define xsub8(o, v)                 (*(s8  *)(alis.mem + (o)) -= (s8 )(v))
#define xsub16(o, v)                (*(s16 *)(alis.mem + (o)) -= (s16)(v))
#define xsub32(o, v)                (*(s32 *)(alis.mem + (o)) -= (s32)(v))

#define xswap16(v)                  ((s16)(v))
#define xswap32(v)                  ((s32)(v))
#define xswap16be(v)                ((s16)(v))
#define xswap32be(v)                ((s32)(v))

#define xread16be(o)                xread16(o)
#define xread32be(o)                xread32(o)

#define xread32_wc(base, off)       xread32((base) + (off))
#define xwrite32_wc(base, off, v)   xwrite32((base) + (off), (v))

#elif defined(NDEBUG)
// Release build: inline with runtime endianness branch (no function call overhead)

// Host endianness constant — known at compile time, enables dead-code elimination
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ALIS_HOST_LE 1
#else
#define ALIS_HOST_LE 0
#endif

// The inline functions below use _xmem / _xle declared above.

// -DALIS_MEM_ASM_BSWAP=1: hand-coded m68k byte swaps (GCC emits a long shift/mask sequence).
#if defined(ALIS_MEM_ASM_BSWAP) && ALIS_MEM_ASM_BSWAP && defined(__m68k__)
static inline u16 _xbswap16(u16 v) { __asm__("rol.w #8,%0" : "+d"(v)); return v; }
static inline u32 _xbswap32(u32 v) {
    __asm__("rol.w #8,%0\n\tswap %0\n\trol.w #8,%0" : "+d"(v));
    return v;
}
#else
static inline u16 _xbswap16(u16 v) { return (v << 8) | (v >> 8); }
static inline u32 _xbswap32(u32 v) {
    return ((v >> 24) & 0xff) | ((v << 8) & 0xff0000) |
           ((v >> 8) & 0xff00) | ((v << 24) & 0xff000000);
}
#endif

// -DALIS_MEM_SWAP_ALWAYS=1: assume game and host endianness differ (e.g. DOS data on Atari), removing
// the per-access runtime test. main.c refuses matching-endian games in this build.
#if defined(ALIS_MEM_SWAP_ALWAYS) && ALIS_MEM_SWAP_ALWAYS
#define _XSWAP_NEEDED   1
#else
#define _XSWAP_NEEDED   __builtin_expect(_xle != ALIS_HOST_LE, 0)
#endif

static inline u8  xread8(u32 o)  { return *(_xmem + o); }
static inline s16 xread16(u32 o) {
    u16 v = *(u16 *)(_xmem + o);
    if (_XSWAP_NEEDED) v = _xbswap16(v);
    return (s16)v;
}
static inline s32 xread32(u32 o) {
    u32 v = *(u32 *)(_xmem + o);
    if (_XSWAP_NEEDED) v = _xbswap32(v);
    return (s32)v;
}

static inline void xwrite8(u32 o, u8 v)   { *(_xmem + o) = v; }
static inline void xwrite16(u32 o, s16 v) {
    u16 w = (u16)v;
    if (_XSWAP_NEEDED) w = _xbswap16(w);
    *(u16 *)(_xmem + o) = w;
}
static inline void xwrite32(u32 o, s32 v) {
    u32 w = (u32)v;
    if (_XSWAP_NEEDED) w = _xbswap32(w);
    *(u32 *)(_xmem + o) = w;
}

static inline void xadd8(s32 o, s8 v)  { *(s8  *)(_xmem + o) += v; }
static inline void xadd16(s32 o, s16 v){ xwrite16(o, xread16(o) + v); }
static inline void xadd32(s32 o, s32 v){ xwrite32(o, xread32(o) + v); }

static inline void xsub8(s32 o, s8 v)  { *(s8  *)(_xmem + o) -= v; }
static inline void xsub16(s32 o, s16 v){ xwrite16(o, xread16(o) - v); }
static inline void xsub32(s32 o, s32 v){ xwrite32(o, xread32(o) - v); }

static inline s16 xswap16(u16 v) {
    if (_XSWAP_NEEDED) return (s16)_xbswap16(v);
    return (s16)v;
}
static inline s32 xswap32(u32 v) {
    if (_XSWAP_NEEDED) return (s32)_xbswap32(v);
    return (s32)v;
}

static inline s16 xswap16be(u16 v) {
    if (ALIS_HOST_LE) return (s16)_xbswap16(v);
    return (s16)v;
}
static inline s32 xswap32be(u32 v) {
    if (ALIS_HOST_LE) return (s32)_xbswap32(v);
    return (s32)v;
}

static inline s16 xread16be(u32 o) {
    u16 v = *(u16 *)(_xmem + o);
    if (ALIS_HOST_LE) v = _xbswap16(v);
    return (s16)v;
}
static inline s32 xread32be(u32 o) {
    u32 v = *(u32 *)(_xmem + o);
    if (ALIS_HOST_LE) v = _xbswap32(v);
    return (s32)v;
}

// World-coordinate (16.16) accessors: word-swapped on v3.0+ LE so xread16 sees the integer part.
// Out-of-line in mem.c (needs alis.platform); -DALIS_MEM_WC_INLINE=1 inlines them via `_xwc`.
#if defined(ALIS_MEM_WC_INLINE) && ALIS_MEM_WC_INLINE
static inline s32 xread32_wc(s32 base, s32 off) {
    if (_xwc) {
        u16 hi = (u16)xread16(base + off);
        u16 lo = (u16)xread16(base + off + 2);
        return ((s32)hi << 16) | lo;
    }
    return xread32(base + off);
}
static inline void xwrite32_wc(s32 base, s32 off, s32 val) {
    if (_xwc) {
        xwrite16(base + off, (s16)(val >> 16));
        xwrite16(base + off + 2, (s16)(val & 0xffff));
    } else {
        xwrite32(base + off, val);
    }
}
#else
s32 xread32_wc(s32 base, s32 wc_offset);
void xwrite32_wc(s32 base, s32 wc_offset, s32 val);
#endif

#else
// Debug build: function versions with ALIS_DEBUG logging
u8 xread8(u32 offset);
s16 xread16(u32 offset);
s32 xread32(u32 offset);

void xwrite8(u32 offset, u8 value);
void xwrite16(u32 offset, s16 value);
void xwrite32(u32 offset, s32 value);

void xadd8(s32 offset, s8 value);
void xadd16(s32 offset, s16 value);
void xadd32(s32 offset, s32 add);

void xsub8(s32 offset, s8 value);
void xsub16(s32 offset, s16 value);
void xsub32(s32 offset, s32 sub);

s16 xswap16(u16 value);
s32 xswap32(u32 value);

s16 xswap16be(u16 value);
s32 xswap32be(u32 value);

s16 xread16be(u32 offset);
s32 xread32be(u32 offset);

s32 xread32_wc(s32 base, s32 wc_offset);
void xwrite32_wc(s32 base, s32 wc_offset, s32 val);
#endif // ALIS_MEM_NATIVE_ENDIAN

void xpush32(s32 value);
s32 xpeek32(void);
s32 xpop32(void);

s32 io_malloc(s32 rawsize);
void io_mfree(s32 addr);
