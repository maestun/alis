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

#include "alis.h"
#include "audio.h"
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "screen.h"
#include "sys/sys.h"
#include "utils.h"
#include "video.h"

#if ALIS_SDL_VER < 2
# include <SDL/SDL.h>
#elif defined(_MSC_VER)
# include "SDL.h"
#elif __has_include(<SDL.h>)
# include <SDL.h>
#else
# include <SDL2/SDL.h>
#endif

// Dirty-rect state: SDL1 + native Atari only (SDL2 renders full-frame).
#if ALIS_SDL_VER < 2
extern volatile u8 dirty_pal;
extern SDL_Rect dirty_rects[256];
extern u8 dirty_len;
#endif

// -DALIS_FLI_AUDIO_DEBUG=1: log FLI audio decisions to fli_audio.log.
#ifndef ALIS_FLI_AUDIO_DEBUG
#define ALIS_FLI_AUDIO_DEBUG 0
#endif

#if ALIS_FLI_AUDIO_DEBUG
#include <stdio.h>
#include <stdarg.h>
extern SDL_AudioSpec *audio_spec;
static FILE *fli_dbg_fp = NULL;
static u32   fli_dbg_t0 = 0;
static u32   fli_dbg_chunks_enqueued = 0;
static u32   fli_dbg_total_bytes = 0;
static void fli_dbg_open(void) {
    if (fli_dbg_fp) return;
    fli_dbg_fp = fopen("fli_audio.log", "w");
    if (!fli_dbg_fp) fli_dbg_fp = fopen("C:\\fli_audio.log", "w");
    if (!fli_dbg_fp) return;
    fprintf(fli_dbg_fp,
            "FLI audio diagnostic log\n"
            "audio_spec: freq=%d channels=%d format=0x%04x samples=%d\n"
            "sys_timeclock_hz=%u sys_sfx_tick_hz=%u\n",
            audio_spec ? audio_spec->freq : -1,
            audio_spec ? audio_spec->channels : -1,
            audio_spec ? (unsigned)audio_spec->format : 0,
            audio_spec ? audio_spec->samples : -1,
            (unsigned)sys_timeclock_hz, (unsigned)sys_sfx_tick_hz);
    fflush(fli_dbg_fp);
    fli_dbg_t0 = sys_ticks();
}
// Exported for sys.c (mixer-side logging).
void fli_dbg_log(const char *fmt, ...) {
    fli_dbg_open();
    if (!fli_dbg_fp) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fli_dbg_fp, fmt, ap);
    va_end(ap);
    fflush(fli_dbg_fp);
}
#define FLI_DBG_OPEN()           fli_dbg_open()
#define FLI_DBG(...) do {                          \
    fli_dbg_open();                                 \
    if (fli_dbg_fp) {                               \
        fprintf(fli_dbg_fp, __VA_ARGS__);           \
        fflush(fli_dbg_fp);                         \
    }                                               \
} while (0)
#else
#define FLI_DBG_OPEN()           ((void)0)
#define FLI_DBG(...)             ((void)0)
#endif

u8 *endframe = NULL;

sFLICData bfilm;

#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
u32 g_flic_decode = 0;   // FLI RLE-decode ticks (200Hz), read+reset by the native FLIC profiler
#endif

// Wall-clock pacing for runfilm; reset per film, anchored on the first audio chunk.
static u32 fli_pace_anchor   = 0;
static u32 fli_pace_total_ms = 0;

u16 fls_drawing = 0;
u16 fls_pallines = 0;
s8  fls_state = 0;

u16 pal[16] = {
    0x0000,
    0x0019,
    0x0011,
    0x0811,
    0x0089,
    0x0881,
    0x0081,
    0x0088,
    0x0018,
    0x0012,
    0x0889,
    0x0812,
    0x0181,
    0x0181,
    0x0181,
    0x0181,
};

extern void dbglog(const char *fmt, ...);   // flushing debug log (no-op unless logfile open)

// Film composition buffer (FLS ping-pong frames / FLIC chunky frame), allocated per film.
u8 *pvgalogic = NULL;
u8 *vgalogic = NULL;
u8 *vgalogic_df = NULL;

int vgalogic_alloc(void)
{
    if (!pvgalogic && !(pvgalogic = calloc(1, kVgaLogicSize)))
        return 0;
    vgalogic = pvgalogic + 0x40;
    vgalogic_df = pvgalogic + 0xdf00 + 319;
    return 1;
}

void vgalogic_free(void)
{
    free(pvgalogic);
    pvgalogic = vgalogic = vgalogic_df = NULL;
}
u8 g_s512_ending = 0;   // S512 film reached its end — sys_render flushes the final frame then → None

void fls_savscreen(void)
{
    vgalogic = (u8 *)(vgalogic + 0xffU);
    memset(vgalogic, 0, 57000);
    
    // TODO: store old palette/res

    fls_state = 0xff;
}

void fls_cleanup(void)
{
    // TODO: restore palette and screen

    bfilm.type = eAlisVideoNone;
    bfilm.addr1 = 0;
    fls_state = 0;
    
}

u8 fls_dirty_lines[200];
u8 fls_prev_dirty[200];
u8 fls_dirty_x1[200];
u8 fls_dirty_x2[200];
u8 fls_prev_x1[200];
u8 fls_prev_x2[200];

// Mark dirty scanlines + per-line byte ranges for a write spanning 'len'
// bytes at offset 'off' within the 32000-byte bitplane buffer (160 bytes/line).
// Also tracks which half (left 0-79, right 80-159) is dirty per line.
static void fls_mark_dirty(u32 off, u32 len) {
    if (alis.platform.kind == EPlatformAmiga) {
        // Amiga HAM6: 6 contiguous 8000-byte planes, 40 bytes/row; a run may straddle a
        // plane boundary (end row wraps below start).
        u32 p0 = off % 8000, p1 = (off + len - 1) % 8000;
        u32 l0 = p0 / 40, l1 = p1 / 40;
        if (l1 < l0) {                       // straddled a plane boundary
            for (u32 l = l0; l < 200; l++) fls_dirty_lines[l] |= 3;
            for (u32 l = 0; l <= l1 && l < 200; l++) fls_dirty_lines[l] |= 3;
        } else {
            for (u32 l = l0; l <= l1 && l < 200; l++) fls_dirty_lines[l] |= 3;
        }
        return;
    }
    u32 end = off + len - 1;
    // x/160 via reciprocal multiply (exact for x < 65536); avoids divul.l.
    u32 l0 = (off * 52429u) >> 23;
    u32 l1 = (end * 52429u) >> 23;
    if (l1 >= 200) l1 = 199;
    for (u32 l = l0; l <= l1; l++) {
        u8 x1 = (l == l0) ? (u8)(off - l0 * 160) : 0;
        u8 x2 = (l == l1) ? (u8)(end - l1 * 160) : 159;
        if (!fls_dirty_lines[l]) {
            fls_dirty_x1[l] = x1;
            fls_dirty_x2[l] = x2;
        } else {
            if (x1 < fls_dirty_x1[l]) fls_dirty_x1[l] = x1;
            if (x2 > fls_dirty_x2[l]) fls_dirty_x2[l] = x2;
        }
        // Encode half-dirty in bits: bit 0 = left (bytes 0-79), bit 1 = right (80-159)
        u8 halves = 0;
        if (x1 < 80) halves |= 1;
        if (x2 >= 80) halves |= 2;
        fls_dirty_lines[l] |= halves;
    }
}

u8 *fls_decomp(u8 *addr)
{
    u8 *prevlogic = vgalogic_df;
    if (fls_drawing != 0)
    {
        vgalogic_df = vgalogic;
        vgalogic = prevlogic;
        fls_drawing = 0;
    }
    memset(fls_dirty_lines, 0, 200);

    bfilm.frame ++;

    u8 *endframe = addr + read32(addr);

    u8 *base = vgalogic_df + 0xa0;
    u8 *vgaptr = base;

    addr += 6;

    while (endframe > addr)
    {
        u8 len = *(addr); addr++;
        if (len >= 0x80)
        {
            s32 length = 0x100 - len;
            fls_mark_dirty((u32)(vgaptr - base), length);
            memcpy(vgaptr, addr, length); vgaptr += length; addr += length;
        }
        else
        {
            if ((s8)len < 2)
            {
                if (len == 1)
                {
                    vgaptr += read16(addr); addr+=2;
                }
                else
                {
                    vgaptr += *(addr); addr++;
                }
            }
            else
            {
                fls_mark_dirty((u32)(vgaptr - base), len);
                u8 color = *(addr); addr++;
                memset(vgaptr, color, len); vgaptr += len;
            }
        }
    }

    return endframe;
}

void fls_init(u8 *addr)
{
    fls_pallines = (u16)*(addr + 8) * 2;
    bfilm.frames = read16(addr + 6);
    bfilm.endptr = addr + read32(addr);
    bfilm.frame = 0;
}

u8 *fls_next(u8 *addr)
{
    if (fls_state < 0)
    {
        addr = fls_decomp(addr);
        fls_drawing = 1;
        fls_state = 1;
        return addr;
    }
    else
    {
        addr = fls_decomp(addr);
        fls_drawing = 1;
        return addr;
    }
}

u8 *fls_pal(u8 *addr)
{
    // set palette
    u8 *palette = addr + 6;
    PAL_WRITE(image.mpalet, 0, 16, RGB12(palette));

    u8 *rawpal = addr + 6;
    for (int i = 0; i < 16; i++, rawpal += 2)
    {
        bfilm.ham_pal[i][0] = (rawpal[0] & 0x0f) << 4;  // R
        bfilm.ham_pal[i][1] = (rawpal[1] >> 4) << 4;     // G
        bfilm.ham_pal[i][2] = (rawpal[1] & 0x0f) << 4;   // B
    }

    // HAM palette change dirties every row.
    memset(fls_dirty_lines, 3, 200);

    addr += read32(addr);
    return addr;
}

u32 flstofen(s16 clean)
{
    if (clean < 0)
    {
        fls_cleanup();
    }
    else
    {
        u8 *addr = bfilm.addr1;
        if (addr != 0)
        {
            s16 type = read16(addr + 4);
            { static int _ft_dbg = 0; if (_ft_dbg < 8) {
                dbglog("flstofen: addr=%p type=%04x fls_state=%d\n", (void*)addr, (u16)type, (int)fls_state);
                _ft_dbg++; } }
            if (type == 0x5354)
            {
                fls_init(addr);
                
                addr += alis.platform.kind == EPlatformAmiga ? 0x8 : 0xc;
            }
            else
            {
                if (fls_state == 0)
                {
                    fls_savscreen();
                }
                
                if (alis.platform.kind == EPlatformAmiga)
                {
                    if (type == 0x5355)
                    {
                        addr = fls_next(addr);
                        fls_drawing = 1;
                    }
                    else if (type == 0x5356)
                    {
                        addr = fls_pal(addr);
                    }
                    else
                    {
                        bfilm.addr1 = 0;
                        return 0;
                    }
                }
                else // Atari ST
                {
                    if (type == 0x5357)
                    {
                        addr = fls_next(addr);
                        fls_drawing = 1;
                    }
                    else
                    {
                        bfilm.addr1 = 0;
                        return 0;
                    }
                }
            }
            
            bfilm.addr1 = ((u64)addr & 1) + addr;
            if (bfilm.addr1 < bfilm.endptr)
            {
                return 1;
            }
        }
    }
    
    bfilm.addr1 = 0;
    return 0;
}

void endfilm(void)
{
    dbglog("endfilm: type was %d frame=%d/%d\n", (int)bfilm.type, (int)bfilm.frame, (int)bfilm.frames);
    flstofen(-1);
    bfilm.type = eAlisVideoNone;
    // 16-bit film path stays in 16-bit mode holding the last frame (as the original);
    // sys_render restores planar on the first game draw.
}


// fli (flic video) video player

void fli_palette(u8 *addr)
{
    if (!dos_film_palette(addr))
    {
        u16 packets = read16le(addr), index = 0;
        addr+=2;
    
        do
        {
            index += *(addr); addr++;
            u16 len = *(addr); addr++;
            if (len == 0)
            {
                len = 256;
            }
        
            if (index + len > 256)
            {
                len = 256 - index;
                packets = 1;
            }

            PAL_WRITE(image.mpalet, index, len, RGB18(addr));
            index += len;
        }
        while (--packets);
    }

    image.ftopal = 0xff;

#if defined(ALIS_FLI_FALCON16)
    // 16-bit film path: refresh the 8->16 LUT now so this frame's own pixel chunks
    // (which follow in the same fli_next) decode with the new colours.
    extern u16 *g_fli16_screen;
    extern void fli16_build_lut(void);
    if (g_fli16_screen) fli16_build_lut();
#endif
}

void fli_blackdata(void)
{
    memset(vgalogic, 0, bfilm.width * bfilm.height);
#if ALIS_SDL_VER < 2
    dirty_rects[0] = (SDL_Rect){ .x = image.fenx1, .y = image.feny1, .w = bfilm.width, .h = bfilm.height };
    dirty_len = 0xff;
#endif
}

void fli_data(u8 *addr)
{
    u8 *ptr = vgalogic;
    for (int i = 0; i < bfilm.height; i++, ptr += bfilm.width, addr += bfilm.width)
    {
        memcpy(ptr, addr, bfilm.width);
    }
#if ALIS_SDL_VER < 2
    dirty_rects[0] = (SDL_Rect){ .x = image.fenx1, .y = image.feny1, .w = bfilm.width, .h = bfilm.height };
    dirty_len = 0xff;
#endif
}

// Optimised FLC decoder (ALIS_FLI_OPT); fli_decomp_ref is the bit-exact reference.
// No divide, no libc call, invariants hoisted so the loop fits the registers left by a4/a5.

// Reference decoder, parameterised on target buffer + dirty state (flag-off path / verify).
#if !defined(ALIS_FLI_OPT) || defined(ALIS_FLI_DECODE_VERIFY)
static void fli_decomp_ref(u8 *addr, u8 partial, u8 *vga, SDL_Rect *dr, u8 *dlp)
{
    u8 dirty_len = *dlp;
    u32 index = 0;
    u16 len = bfilm.height;

    if (partial)
    {
        index = read16le(addr) * bfilm.width; addr+=2;
        len = read16le(addr); addr+=2;
    }

    while (len--)
    {
        u8 packets = *addr; addr++;
        u16 col = 0;
        while (packets--)
        {
            if (partial)
            {
                col += *(addr); addr++;
            }

            short int count = (signed char) *(addr); addr++;
            if (partial) count = -count;
            if (count >= 0)
            {
                if (count == 0)
                    count = 256;

                if (col + count > bfilm.width)
                {
                    count = bfilm.width - col;
                    len = packets = 0;
                }

                memset(vga + index + col, *(addr), count); addr++;
#if ALIS_SDL_VER < 2
            if (dirty_len > 0xfd)
            {
                dr[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                dirty_len = 0xff;
            }
            else
            {
                dr[dirty_len] = (SDL_Rect){.x = image.fenx1 + (index + col) % bfilm.width, .y = image.feny1 + (index + col) / bfilm.width, .w = count, .h = 1 };
                dirty_len++;
            }
#endif
            }
            else
            {
                count = -count;
                if (col + count > bfilm.width)
                {
                    count = bfilm.width - col;
                    len = packets = 0;
                }

                memcpy(vga + index + col, addr, count); addr+= count;
#if ALIS_SDL_VER < 2
                if (dirty_len > 0xfd)
                {
                    dr[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                    dirty_len = 0xff;
                }
                else
                {
                    dr[dirty_len] = (SDL_Rect){.x = image.fenx1 + (index + col) % bfilm.width, .y = image.feny1 + (index + col) / bfilm.width, .w = count, .h = 1 };
                    dirty_len++;
                }
#endif
            }

            col += count;
        }

        index += bfilm.width;
    }
    *dlp = dirty_len;
}
#endif /* reference compiled only when used */

#if defined(ALIS_FLI_OPT)
// Inline run fill/copy; FLI_OPT_ATTR keeps gcc from turning them back into memset/memcpy.
// Misaligned long access is legal on 020+.
static inline void fli_fill_run(u8 *d, u8 v, u32 n)
{
    u32 w = (u32)v; w |= w << 8; w |= w << 16;
    while (n >= 4) { *(u32 *)d = w; d += 4; n -= 4; }
    while (n) { *d++ = v; n--; }
}
static inline void fli_copy_run(u8 *d, const u8 *s, u32 n)
{
    while (n >= 4) { *(u32 *)d = *(const u32 *)s; d += 4; s += 4; n -= 4; }
    while (n) { *d++ = *s++; n--; }
}

// `partial` is a compile-time constant in each entry point.
static inline __attribute__((always_inline)) void
fli_decomp_impl(u8 *addr, u8 partial, u8 *vga, SDL_Rect *dr, u8 *dlp)
{
    const u16 W = bfilm.width;
#if ALIS_SDL_VER < 2
    const s16 fx = image.fenx1, fy = image.feny1;
#endif
    u8 *rowptr;
    u16 line, len;
    u8  dl = *dlp;

    if (partial) { line = read16le(addr); addr += 2; rowptr = vga + (u32)line * W; len = read16le(addr); addr += 2; }
    else         { line = 0; rowptr = vga; len = bfilm.height; }

    while (len--)
    {
        u8  packets = *addr++;
        u16 col = 0;
        while (packets--)
        {
            if (partial) col += *addr++;

            short count = (signed char)*addr++;
            if (partial) count = -count;

            if (count >= 0)
            {
                if (count == 0) count = 256;
                if (col + count > W) { count = W - col; len = packets = 0; }
                fli_fill_run(rowptr + col, *addr++, (u32)(u16)count);
            }
            else
            {
                count = -count;
                if (col + count > W) { count = W - col; len = packets = 0; }
                fli_copy_run(rowptr + col, addr, (u32)(u16)count); addr += count;
            }
#if ALIS_SDL_VER < 2
            if (dl > 0xfd) { dr[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h }; dl = 0xff; }
            else           { dr[dl] = (SDL_Rect){ .x = fx + col, .y = fy + line, .w = count, .h = 1 }; dl++; }
#endif
            col += count;
        }
        rowptr += W; line++;
    }
    *dlp = dl;
}

#define FLI_OPT_ATTR __attribute__((noinline, optimize("no-tree-loop-distribute-patterns,no-unroll-loops,no-peel-loops")))
FLI_OPT_ATTR static void fli_decomp_brun(u8 *a, u8 *vga, SDL_Rect *dr, u8 *dlp) { fli_decomp_impl(a, 0, vga, dr, dlp); }
FLI_OPT_ATTR static void fli_decomp_lc  (u8 *a, u8 *vga, SDL_Rect *dr, u8 *dlp) { fli_decomp_impl(a, 1, vga, dr, dlp); }
#endif /* ALIS_FLI_OPT */

void fli_decomp(u8 *addr, u8 partial)
{
#if defined(ALIS_FLI_OPT) && defined(ALIS_FLI_DECODE_VERIFY)
    // Verify: run the reference on a shadow, the optimised decoder live, then compare.
    extern u8 *vgalogic;
    static u8 shadow_vga[64000];
    static SDL_Rect shadow_dr[256];
    static u32 flidec_calls = 0, flidec_bad = 0;

    u32 vsz = (u32)bfilm.width * (u32)bfilm.height;
    if (vsz > sizeof(shadow_vga)) vsz = sizeof(shadow_vga);
    memcpy(shadow_vga, vgalogic, vsz);
    memcpy(shadow_dr, dirty_rects, sizeof(shadow_dr));
    u8 shadow_dl = dirty_len;

    fli_decomp_ref(addr, partial, shadow_vga, shadow_dr, &shadow_dl);
    if (partial) fli_decomp_lc(addr, vgalogic, dirty_rects, &dirty_len);
    else         fli_decomp_brun(addr, vgalogic, dirty_rects, &dirty_len);

    u32 pxdiff = 0; s32 firstoff = -1;
    for (u32 i = 0; i < vsz; i++)
        if (vgalogic[i] != shadow_vga[i]) { if (firstoff < 0) firstoff = (s32)i; pxdiff++; }
    int drbad = (dirty_len != shadow_dl);
    u32 ncmp = dirty_len < shadow_dl ? dirty_len : shadow_dl;
    if (ncmp > 256) ncmp = 256;
    if (!drbad && memcmp(dirty_rects, shadow_dr, (size_t)ncmp * sizeof(SDL_Rect))) drbad = 1;

    flidec_calls++;
    if (pxdiff || drbad)
    {
        flidec_bad++;
        dbglog("[flidec] BAD call=%lu partial=%u W=%u H=%u pxdiff=%lu firstoff=%ld dl=%u/%u drbad=%d\n",
               (unsigned long)flidec_calls, (unsigned)partial, (unsigned)bfilm.width, (unsigned)bfilm.height,
               (unsigned long)pxdiff, (long)firstoff, (unsigned)dirty_len, (unsigned)shadow_dl, drbad);
    }
    else if ((flidec_calls & 0xff) == 0)
        dbglog("[flidec] OK %lu calls (bad=%lu)\n", (unsigned long)flidec_calls, (unsigned long)flidec_bad);
#elif defined(ALIS_FLI_OPT)
    if (partial) fli_decomp_lc(addr, vgalogic, dirty_rects, &dirty_len);
    else         fli_decomp_brun(addr, vgalogic, dirty_rects, &dirty_len);
#elif ALIS_SDL_VER < 2
    fli_decomp_ref(addr, partial, vgalogic, dirty_rects, &dirty_len);
#else
    // SDL2 renders full-frame: no dirty tracking.
    u8 _dl = 0;
    fli_decomp_ref(addr, partial, vgalogic, NULL, &_dl);
#endif
}


// Per-slot copies of queued FLI speech chunks (bytes = freq * waitclock / 60; larger
// chunks are split). Allocated on first use, never freed: the mixer drains asynchronously.
#define FLI_AUDIO_CHUNK_MAX 4096
typedef s8 fli_audio_slot_t[FLI_AUDIO_CHUNK_MAX];
static fli_audio_slot_t *fli_audio_copybuf = NULL;

void fli_audio(u8 *nextaddr, u8 *addr, s16 type)
{
    // Check if this audio track is selected by bfilm.id bitmask
    // type 0x1000 -> bit 8 (French), 0x1001 -> bit 9 (English), 0x1002 -> bit 10 (German)
    u16 mask = (u16)(1 << (type - 0x1000)) << 8;
    if ((mask & (u16)bfilm.id) == 0) {
        FLI_DBG("fli_audio: type=0x%x SKIP (bfilm.id=0x%x mask=0x%x)\n",
                (int)type, (unsigned)bfilm.id, (unsigned)mask);
        return;
    }

    u32 length = (u32)(nextaddr - addr);
    if (length == 0) {
        FLI_DBG("fli_audio: type=0x%x SKIP (length=0)\n", (int)type);
        return;
    }

    // Chunks are word-aligned: drop the padding byte of odd-length data (else a click).
    if (length & 1u) {
        FLI_DBG("fli_audio: type=0x%x len=%u → trimming odd padding byte to %u\n",
                (int)type, length, length - 1);
        length--;
    }


    // Channel 3 set directly (playsample clamps freq too low for 22 kHz).
    // Rate = chunk bytes * 60 / waitclock: FLI files are sized for the DOS 60 Hz PIT
    // on every platform, so not sys_sfx_tick_hz.
    u32 src_fps = 60UL;

    s16 freq;
    {
        u32 f = (bfilm.waitclock > 0)
                ? (src_fps * length) / (u32)bfilm.waitclock
                : 22050UL;
        if (f > 32767UL) f = 32767UL;     // sChannel.freq is s16
        freq = (s16)f;
    }

#if ALIS_FLI_AUDIO_DEBUG
    fli_dbg_chunks_enqueued++;
    fli_dbg_total_bytes += length;
    {
        u32 q_depth = (u32)((fli_dbg_chunks_enqueued
                             - 0)) & 0xff;  // approx
        u32 q_used = (u32)((fli_audio_q_head - fli_audio_q_tail) & (FLI_AUDIO_QUEUE_SIZE - 1));
        FLI_DBG("fli_audio[%u]: type=0x%x len=%u waitclock=%d → freq=%d  q_used=%u/%u  +%u ms\n",
                fli_dbg_chunks_enqueued, (int)type, length,
                (int)bfilm.waitclock, (int)freq,
                q_used, (unsigned)FLI_AUDIO_QUEUE_SIZE,
                (unsigned)(sys_ticks() - fli_dbg_t0));
        (void)q_depth;
    }
#endif

    // Enqueue for gapless playback. Chunks are copied, not referenced: a streamed film
    // recycles its window before the mixer drains the queue.
    {
        if (fli_audio_copybuf == NULL) {
            fli_audio_copybuf = (fli_audio_slot_t *)
                malloc(sizeof(fli_audio_slot_t) * FLI_AUDIO_QUEUE_SIZE);
            if (fli_audio_copybuf == NULL)
                return;                  // no room for speech buffers — film plays silent
        }

        u8 *src = addr;
        u32 left = length;
        while (left != 0)
        {
            u32 piece = (left > FLI_AUDIO_CHUNK_MAX) ? FLI_AUDIO_CHUNK_MAX : left;
            u8 next = (u8)((fli_audio_q_head + 1) & (FLI_AUDIO_QUEUE_SIZE - 1));
            if (next == fli_audio_q_tail) {
                // Queue full (~2 s buffered): drop the rest.
                break;
            }
            memcpy(fli_audio_copybuf[fli_audio_q_head], src, piece);
            fli_audio_queue[fli_audio_q_head].addr   = fli_audio_copybuf[fli_audio_q_head];
            fli_audio_queue[fli_audio_q_head].length = piece;
            fli_audio_queue[fli_audio_q_head].freq   = freq;
            fli_audio_q_head = next;
            src  += piece;
            left -= piece;
        }
    }

    // Prime idle channel 3; the mixer then advances through the queue itself.
    sChannel *canal = &audio.channels[3];
    if (canal->type == eChannelTypeNone) {
        u8 t = fli_audio_q_tail;
        canal->type = eChannelTypeNone;   // gate against IRQ mid-update
        canal->address = fli_audio_queue[t].addr;
        canal->length  = fli_audio_queue[t].length;
        canal->freq    = fli_audio_queue[t].freq;
        canal->volume  = 0x7f;
        canal->loop    = 0;
        canal->played  = 0;
        canal->curson  = 0x7f;
        fli_audio_q_tail = (u8)((t + 1) & (FLI_AUDIO_QUEUE_SIZE - 1));
        canal->type = eChannelTypeSample; // last write — re-arm
    }
}

// Falcon 16-bit film path (ALIS_FLI_FALCON16), as the CD original: the FLC decoder
// LUT-expands each index to RGB565 straight into the framebuffer (no c2p).
// Selected when g_fli16_screen != NULL (sys_film16_begin).
#if defined(ALIS_FLI_FALCON16)
u16 *g_fli16_screen = NULL;   // 16-bit RGB565 film framebuffer (Videl BPS16); NULL = planar path
u32  g_fli16_stride = 320;    // screen width in PIXELS (row stride); set by sys_film16_begin
u16  fli_lut16[256];          // 8-bit index -> RGB565, rebuilt from the film palette

// Rebuild the 8->16 LUT from the film palette (same RGB565 fold as sys_pal16_refresh).
void fli16_build_lut(void)
{
    for (int i = 0; i < 256; i++) {
        sColorARGB *c = (sColorARGB *)&image.mpalet[i];
        fli_lut16[i] = (u16)(((c->r >> 3) << 11) | ((c->g >> 2) << 5) | (c->b >> 3));
    }
}

// Long-word run fill (2 RGB565 px/store) + word tail. dst = screen + pixel offset
// (word-aligned; 030 tolerates the odd case).
static inline void fli16_fill(u16 *d, u16 w, u32 n)
{
    u32 ww = ((u32)w << 16) | w;
    while (n >= 2) { *(u32 *)d = ww; d += 2; n -= 2; }
    if (n) *d = w;
}
static inline void fli16_expand(u16 *d, const u8 *s, u32 n)
{
    while (n--) *d++ = fli_lut16[*s++];
}

// LC (partial=1) / BRUN (partial=0) decoder, RGB565 straight to g_fli16_screen.
__attribute__((optimize("no-tree-loop-distribute-patterns,no-unroll-loops,no-peel-loops")))
static void fli_decomp16(const u8 *addr, u8 partial)
{
    const u16 W = bfilm.width;
    const u32 stride = g_fli16_stride;                          // screen row stride (pixels)
    u16 *const base = g_fli16_screen + (u32)image.feny1 * stride + image.fenx1;
    u16 *rowptr; u16 line, len;
    if (partial) { line = read16le(addr); addr += 2; rowptr = base + (u32)line * stride; len = read16le(addr); addr += 2; }
    else         { line = 0; rowptr = base; len = bfilm.height; }
    while (len--) {
        u8 packets = *addr++;
        u16 col = 0;
        while (packets--) {
            if (partial) col += *addr++;
            short count = (signed char)*addr++;
            if (partial) count = -count;
            if (count >= 0) {
                if (count == 0) count = 256;
                if (col + count > W) { count = W - col; len = packets = 0; }
                fli16_fill(rowptr + col, fli_lut16[*addr++], (u32)(u16)count);
            } else {
                count = -count;
                if (col + count > W) { count = W - col; len = packets = 0; }
                fli16_expand(rowptr + col, addr, (u32)(u16)count); addr += count;
            }
            col += count;
        }
        rowptr += stride; line++;
    }
}

// Raw full-frame (chunk 0x10) and black (chunk 0xd) 16-bit variants — same screen
// stride + fenx1/feny1 positioning as the delta path.
static void fli_data16(const u8 *addr)
{
    const u32 stride = g_fli16_stride;
    u16 *row = g_fli16_screen + (u32)image.feny1 * stride + image.fenx1;
    for (int y = 0; y < bfilm.height; y++, row += stride, addr += bfilm.width)
        fli16_expand(row, addr, bfilm.width);
}
static void fli_blackdata16(void)
{
    const u32 stride = g_fli16_stride;
    u16 *row = g_fli16_screen + (u32)image.feny1 * stride + image.fenx1;
    for (int y = 0; y < bfilm.height; y++, row += stride)
        fli16_fill(row, fli_lut16[0], bfilm.width);
}
#endif /* ALIS_FLI_FALCON16 */

void fli_elements(u8 *addr)
{
#if defined(ALIS_FLI_FALCON16)
    extern void sys_film16_keyframe(void);   // aim the 16-bit decoder at the off-screen back buffer
    extern void sys_film16_present(void);    // flip to it on vblank (hides the keyframe wipe-in)
#endif
    while (addr < endframe)
    {
        u32 offset = read32le(addr);
        addr+=4;

        u16 type = read16le(addr);
        addr+=2;
        
        u8 *nextaddr = addr + offset - 6;
        
        switch (type) {
            case 0xb:
                fli_palette(addr);
                break;
            case 0xc:   // LC delta — modifies the previous frame -> in place on the displayed front
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen) { fli_decomp16(addr, 1); break; }
#endif
                fli_decomp(addr, 1);
                break;
            case 0xd:   // black — self-contained full frame -> keyframe (decode off-screen, flip)
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen) { sys_film16_keyframe(); fli_blackdata16(); sys_film16_present(); break; }
#endif
                fli_blackdata();
                break;
            case 0xf:   // BRUN — self-contained full frame -> keyframe (decode off-screen, flip)
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen) { sys_film16_keyframe(); fli_decomp16(addr, 0); sys_film16_present(); break; }
#endif
                fli_decomp(addr, 0);
                break;
            case 0x10:  // raw — self-contained full frame -> keyframe (decode off-screen, flip)
#if defined(ALIS_FLI_FALCON16)
                if (g_fli16_screen) { sys_film16_keyframe(); fli_data16(addr); sys_film16_present(); break; }
#endif
                fli_data(addr);
                break;

            default:
                // 0x1000..0x1007 only: Silmarils audio tracks.
                if (type >= 0x1000 && type <= 0x1007) {
                    fli_audio(nextaddr, addr, type);
                } else {
                    FLI_DBG("fli_elements: skipping unknown chunk type=0x%x size=%u\n",
                            (int)type, offset);
                }
        }

        addr = nextaddr;
    }
}

// ============================================================================
#pragma mark - Film streaming (large films that do not fit in RAM)
// ============================================================================
// Fallback when the film doesn't fit: bounded sliding window, as the CD original.
// Each frame is made resident before parsing; the tail is compacted to the base
// only when a frame would run past the window end.

#define FLI_STREAM_TOPUP 32768   // minimum read size — keeps freads ~1/frame

int fli_stream_open(FILE *fp, u32 file_size)
{
    static const u32 sizes[] = { 512 * 1024, 256 * 1024, 128 * 1024 };
    u8 *buf = NULL;
    u32 cap = 0;
    for (int i = 0; i < 3 && buf == NULL; i++)
    {
        cap = sizes[i];
        buf = malloc(cap);
    }
    if (buf == NULL)
    {
        dbglog("[flistream] open FAILED: no RAM even for a %u-byte window (film %u bytes)\n",
               sizes[2], file_size);
        return 0;
    }

    fseek(fp, 0, SEEK_SET);
    bfilm.sfp        = fp;
    bfilm.sbuf       = buf;
    bfilm.sbuf_size  = cap;
    bfilm.sfill      = buf;          // window empty — fli_stream_ensure primes on demand
    bfilm.sremain    = file_size;
    bfilm.sfile_size = file_size;
    dbglog("[flistream] streaming film: size=%u window=%u\n", file_size, cap);
    return 1;
}

void fli_stream_close(void)
{
    if (bfilm.sfp != NULL)
    {
        fclose(bfilm.sfp);
        bfilm.sfp = NULL;
    }
    // The window buffer is owned by bfilm.delptr (cdelfilm frees it).
    bfilm.sbuf = NULL;
    bfilm.sfill = NULL;
    bfilm.sbuf_size = 0;
    bfilm.sremain = 0;
    bfilm.sfile_size = 0;
}

// Make `need` bytes starting at bfilm.addr1 resident in the window.
// Returns 0 when that can never happen (EOF / I/O error / frame > window).
// Keeps bfilm.endptr == bfilm.sfill so flitofen's bounds check stays valid.
static int fli_stream_ensure(u32 need)
{
    u32 have = (u32)(bfilm.sfill - bfilm.addr1);
    if (have >= need)
    {
        bfilm.endptr = bfilm.sfill;
        return 1;
    }
    if (need > bfilm.sbuf_size)
    {
        dbglog("[flistream] frame of %u bytes exceeds the %u window — aborting film\n",
               need, bfilm.sbuf_size);
        return 0;
    }

    // Compact only when the frame would extend past the window end.
    if ((u32)(bfilm.addr1 - bfilm.sbuf) + need > bfilm.sbuf_size)
    {
        memmove(bfilm.sbuf, bfilm.addr1, have);
        bfilm.addr1 = bfilm.sbuf;
        bfilm.sfill = bfilm.sbuf + have;
    }

    u32 space = bfilm.sbuf_size - (u32)(bfilm.sfill - bfilm.sbuf);
    u32 want = need - have;
    if (want < FLI_STREAM_TOPUP) want = FLI_STREAM_TOPUP;
    if (want > space)        want = space;
    if (want > bfilm.sremain) want = bfilm.sremain;
    if (want != 0)
    {
        u32 got = (u32)fread(bfilm.sfill, 1, want, bfilm.sfp);
        bfilm.sfill += got;
        bfilm.sremain -= got;
        if (got < want)
            bfilm.sremain = 0;   // short read = EOF or I/O error — end the film cleanly
    }

    bfilm.endptr = bfilm.sfill;
    return (u32)(bfilm.sfill - bfilm.addr1) >= need;
}

// Make the whole chunk at bfilm.addr1 resident: the 0x80-byte FLC file
// header, or one complete 0xf1fa frame (its 32-bit length prefix tells us
// how much). Returns 0 at end of film.
static int fli_stream_frame(void)
{
    if (!fli_stream_ensure(6))
        return 0;
    u16 type = read16le(bfilm.addr1 + 4);
    u32 need = 6;
    if (type == 0xaf11)
        need = 0x80;                      // FLC header; first frame starts at +0x80
    else if (type == 0xf1fa)
        need = read32le(bfilm.addr1);     // complete frame chunk
    return fli_stream_ensure(need);
}

// Loop replay: rewind the file and empty the window; fli_stream_ensure
// re-primes on the next flitofen. Cheap when the film is about to be
// deleted instead of replayed (no read happens here).
static void fli_stream_rewind(void)
{
    fseek(bfilm.sfp, 0, SEEK_SET);
    bfilm.sremain = bfilm.sfile_size;
    bfilm.sfill = bfilm.sbuf;
    bfilm.addr1 = bfilm.sbuf;
    bfilm.addr2 = bfilm.sbuf;
    bfilm.endptr = bfilm.sbuf;
}

void fli_init(u8 *flcaddr)
{
    u32 length = read32le(flcaddr);
    bfilm.endptr = flcaddr + length;
    bfilm.addr1 = flcaddr + 0x80;
    bfilm.frames = read16le(flcaddr + 6);
    bfilm.width = read16le(flcaddr + 8);
    bfilm.height = read16le(flcaddr + 10);
    bfilm.frame = 0;
    // Streaming: the file-length-based endptr points far past the window;
    // the window fill boundary is the real bound.
    if (bfilm.sfp != NULL)
        bfilm.endptr = bfilm.sfill;
}

void fli_next(u8 *addr)
{
    u32 length = read32le(addr);
    endframe = addr + length;
    fli_elements(addr + 16);

#if ALIS_SDL_VER >= 2
    u16 fw = bfilm.width;
    u16 fh = bfilm.height;
    s16 dx = image.fenx1;
    s16 dy = image.feny1;
    u16 dw = alis.platform.width;

    for (int y = 0; y < fh; y++)
    {
        u8 *src = vgalogic + y * fw;
        u8 *dst = image.physic + (dy + y) * dw + dx;
        memcpy(dst, src, fw);
    }
    memcpy(image.logic, image.physic, dw * alis.platform.height);
#endif

    bfilm.addr1 = endframe;
    bfilm.frame++;
}

s16 flitofen(void)
{
    if (bfilm.addr1 != NULL)
    {
        // Streaming film: make the chunk at addr1 fully resident first.
        if (bfilm.sfp != NULL && !fli_stream_frame())
            return 0;

        u16 type = read16le(bfilm.addr1 + 4);
        if (type == 0xaf11)
        {
            fli_init(bfilm.addr1);
            return 1;
        }
        else if (type == 0xf1fa && bfilm.endptr > bfilm.addr1)
        {
            fli_next(bfilm.addr1);
            return 1;
        }
    }
    
    return 0;
}

void inifilm(void)
{
    bfilm.playing = 0;

    if (!vgalogic_alloc()) {
        dbglog("inifilm: no memory for the film buffer — film skipped\n");
        bfilm.type = eAlisVideoNone;
        return;
    }

    // Free channel 3 (a game sound may hold it; the queue only primes when idle) and
    // reset the queue. type = None first so the mixer skips it meanwhile.
    audio.channels[3].type   = eChannelTypeNone;
    audio.channels[3].curson = 0x80;
    fli_audio_q_head = 0;
    fli_audio_q_tail = 0;

    g_s512_ending = 0;   // fresh film — cancel any pending end-flush from a previous one

    dbglog("inifilm: kind=%d physic=%p logic=%p w=%d h=%d\n",
           (int)alis.platform.kind, (void*)image.physic, (void*)image.logic,
           (int)bfilm.width, (int)bfilm.height);

    switch (alis.platform.kind) {
        case EPlatformAtari:
            bfilm.type = eAlisVideoS512;
            break;
        case EPlatformAmiga:
            bfilm.type = eAlisVideoHAM6;
            break;
        default:
            bfilm.type = eAlisVideoFLIC;
            break;
    };
    
#if ALIS_SDL_VER == 1 && !defined(ALIS_NATIVE_PLANAR)
    memset(image.physic, 0, 64000);
    memset(image.logic, 0, 64000);
#endif
    // Native planar: no clear (the display is one of these buffers; the first frame is a keyframe).

    if (alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga)
    {
        flstofen(0);
    }
    else
    {
        flitofen();   // reads the FLC header (0xAF11) -> bfilm.width/height set
#if defined(ALIS_FLI_FALCON16)
        // Falcon: 16-bit Videl mode, decode straight to screen. No-op elsewhere.
        extern int sys_film16_begin(u16 w, u16 h);
        sys_film16_begin(bfilm.width, bfilm.height);
#endif
    }
}

void runfilm(void)
{
    u32 basemain = bfilm.basemain;
    image.fenx1 = read16le(alis.mem + basemain + 0xe);
    image.fenx2 = read16le(alis.mem + basemain + 0x12) + image.fenx1;
    image.clipl = read16le(alis.mem + basemain + 0x12) + 1;
    image.feny1 = read16le(alis.mem + basemain + 0x10);
    image.feny2 = read16le(alis.mem + basemain + 0x14) + image.feny1;
    image.cliph = read16le(alis.mem + basemain + 0x14) + 1;
    image.wloglarg = image.loglarg;
    image.wlogx1 = 0;
    image.wlogy1 = 0;
    image.clipx1 = image.fenx1;
    image.clipy1 = image.feny1;
    image.clipx2 = image.fenx2;
    image.clipy2 = image.feny2;

#if defined(ALIS_SKIP_FLS)
    bfilm.frame = bfilm.frames;
    bfilm.result = -0x7ffc;
    return;
#endif

#if ALIS_FLI_AUDIO_DEBUG
    {
        u32 t = sys_ticks();
        fli_dbg_chunks_enqueued = 0;
        fli_dbg_total_bytes = 0;
        FLI_DBG("---- runfilm START at +%u ms ----\n  bfilm.frames=%u width=%u height=%u id=0x%x\n",
                t, (unsigned)bfilm.frames, (unsigned)bfilm.width,
                (unsigned)bfilm.height, (unsigned)bfilm.id);
    }
#endif

    // Reset wall-clock pacing state so this cutscene anchors fresh
    // (lazy — actual anchor sets on first audio chunk).
    fli_pace_anchor   = 0;
    fli_pace_total_ms = 0;

    bfilm.playing = 1;

    while (true)
    {
        u32 prevclock = alis.timeclock;
        u8  q_head_before = fli_audio_q_head;
        u32 chunks_target = fli_chunks_played;
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
        extern u32 g_flic_decode; extern u32 sys_profile_ticks_safe(void);
        u32 _dec0 = sys_profile_ticks_safe();
#endif
        s16 result = (alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga) ? flstofen(0) : flitofen();
#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_USE_NATIVE_ATARI)
        g_flic_decode += sys_profile_ticks_safe() - _dec0;
#endif
#if defined(ALIS_FLI_FALCON16)
        // 16-bit FLIC decodes on screen in place: wait a vblank. Not for FLS (sys_render flips).
        if (!(alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga)) {
            extern void sys_film16_vsync(void);
            sys_film16_vsync();
        }
#endif

        // Did flstofen/flitofen enqueue audio chunk(s) for this frame?
        u8 chunks_enqueued = (u8)((fli_audio_q_head - q_head_before)
                                  & (FLI_AUDIO_QUEUE_SIZE - 1));
        chunks_target += chunks_enqueued;

        if (0 < bfilm.waitclock)
        {
            // Audio frames: pace by chunk duration (wall clock; timeclock on native Atari).
            // Silent frames: timeclock.
            int use_wallclock_pace = chunks_enqueued;
#if defined(ALIS_USE_NATIVE_ATARI)
            use_wallclock_pace = 0;
#endif
            if (use_wallclock_pace)
            {
                // Anchor on the first chunk; accumulate each chunk's duration.
                if (fli_pace_anchor == 0) {
                    fli_pace_anchor   = sys_ticks();
                    fli_pace_total_ms = 0;
                }
                for (u8 k = 0; k < chunks_enqueued; k++) {
                    u8 idx = (u8)((q_head_before + k) & (FLI_AUDIO_QUEUE_SIZE - 1));
                    s16 cf = fli_audio_queue[idx].freq;
                    u32 cl = fli_audio_queue[idx].length;
                    if (cf > 0)
                        fli_pace_total_ms += (cl * 1000UL) / (u32)cf;
                }

                while (sys_ticks() - fli_pace_anchor < fli_pace_total_ms) {
                    sys_delay(1);
                }
            }
            else
            {
                // Interval = waitclock / file-format tick rate (FLI 60 Hz, FLS 50 Hz).
                u32 thz = sys_timeclock_hz ? sys_timeclock_hz : 50;
                u32 src = (alis.platform.kind == EPlatformAtari ||
                           alis.platform.kind == EPlatformAmiga) ? 50UL : 60UL;
                s16 index = (s16)(((u32)(u16)bfilm.waitclock * thz) / src);
                if (index == 0) index = 1;
                while (alis.timeclock < (u32)(index + prevclock)) {
                    sys_delay(1);
                }
            }
        }
        
        if (result == 0) {
#if ALIS_FLI_AUDIO_DEBUG
            FLI_DBG("---- runfilm END (result=0) at +%u ms — elapsed %u ms over %u chunks (%u bytes) ----\n",
                    sys_ticks(), sys_ticks() - fli_dbg_t0,
                    fli_dbg_chunks_enqueued, fli_dbg_total_bytes);
#endif
            // S512 data exhausted: sys_render still has to draw the last frame, then ends the film.
            if (bfilm.type == eAlisVideoS512 || bfilm.type == eAlisVideoHAM6)
                g_s512_ending = 1;
            break;
        }

        bfilm.batchframes--;
        if (bfilm.batchframes == 0)
        {
            bfilm.result = 0;

#if ALIS_SDL_VER == 1
            // some versions of game do not run cendfilm
            if (bfilm.frame + 1 >= bfilm.frames && bfilm.type == eAlisVideoFLIC)
            {
                bfilm.type = eAlisVideoCleanup;
            }
            // Same for S512 (no cleanup frame needed).
            else if (bfilm.frame + 1 >= bfilm.frames && bfilm.type == eAlisVideoS512)
            {
                bfilm.type = eAlisVideoNone;
            }
#endif

#if ALIS_FLI_AUDIO_DEBUG
            FLI_DBG("---- runfilm END (batchframes=0) at +%u ms — elapsed %u ms over %u chunks (%u bytes) ----\n",
                    sys_ticks(), sys_ticks() - fli_dbg_t0,
                    fli_dbg_chunks_enqueued, fli_dbg_total_bytes);
#endif
            return;
        }

        if (bfilm.batchframes < 0)
        {
            bfilm.batchframes++;
        }
    }

    if (bfilm.sfp != NULL)
        fli_stream_rewind();   // window mode: addr2 may be stale after compaction
    else
        bfilm.addr1 = bfilm.addr2;
    bfilm.result = -0x7ffc;
}
