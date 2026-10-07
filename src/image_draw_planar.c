//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "config.h"
#if defined(ALIS_NATIVE_PLANAR)

#include "image.h"
#include "alis.h"
#include "sys/sys.h"
#include <string.h>

// 8-plane interleaved bitplane format:
// Every 16 pixels = 16 bytes = 8 words (one per plane)
// Planar pitch = width bytes per scanline (320 pixels * 8 planes / 8 = 320)
#define PLANAR_PITCH    (alis.platform.width)
#define PLANES          8

// Planes used by the game's colour depth (4/6/8); the screen stays 8-plane and the high
// planes stay 0. Set at load from the deepest colour index (convert_sprites_inplace).
int g_planar_nplanes = 8;

// Planes moved by the physic<->logic copy (trsfen): g_planar_nplanes, or more while clinepal
// keeps its bank in planes 4-5. Fills keep g_planar_nplanes so they preserve those planes.
int g_used_planes = 8;

extern void mac_update_pos(s16 *x, s16 *y);
// masks[] and rots[] are defined in image.c, declared extern in image.h

#include "image_atari.h"

// Profiling counters — call counts for each drawing function
static u32 prof_pixel=0, prof_line=0, prof_boxf=0, prof_rect=0;
static u32 prof_st4_0=0, prof_st4_2=0, prof_8bit_0=0, prof_8bit_2=0;
static u32 prof_4to8_0=0, prof_4to8_2=0, prof_trsfen=0, prof_clrfen=0;
static u32 prof_frame=0;

// Sprite-path counters (sprite_planar.c).
extern u32 g_prof_destofen_total, g_prof_destofen_transp, g_prof_destofen_op_shift, g_prof_destofen_op_align, g_prof_destofen_blit, g_prof_destofen_blit_area;
extern u32 g_prof_destofen_skip_flip, g_prof_destofen_skip_miss, g_prof_destofen_skip_fmt;
extern u32 g_prof_destofen_tr_align, g_prof_destofen_tr_shift, g_prof_destofen_tr_align_area, g_prof_destofen_tr_shift_area;
extern u32 g_prof_destofen_cfb, g_prof_destofen_cfb_area;
extern u32 g_prof_destofen_op_calls, g_prof_destofen_op_area;
void planar_dump_profile(void)
{
#if defined(ALIS_PROFILE_DRAW)
    prof_frame++;
    // Dump and reset every 100 frames.
    if (prof_frame >= 100) {
        extern void dbglog(const char *fmt, ...);
        extern u32  sys_profile_ticks_safe(void);
        static u32  prof_t0 = 0;
        u32 _now = sys_profile_ticks_safe();
        u32 _dt  = prof_t0 ? (_now - prof_t0) : 0;   // ticks @200Hz for the last 100 frames
        prof_t0  = _now;
        dbglog("PROFILE/100f TIME: %u ticks @200Hz = %u ms  (~%u fps)  [nplanes=%d]\n",
               _dt, _dt * 5, _dt ? (20000u / _dt) : 0, g_planar_nplanes);
        dbglog("PROFILE/100f OLD-c2p: st4_0=%u st4_2=%u 8bit_0=%u 8bit_2=%u 4to8_0=%u 4to8_2=%u trsfen=%u clrfen=%u boxf=%u rect=%u\n",
               prof_st4_0, prof_st4_2, prof_8bit_0, prof_8bit_2, prof_4to8_0, prof_4to8_2, prof_trsfen, prof_clrfen, prof_boxf, prof_rect);
        dbglog("PROFILE/100f destofen_planar: total=%u transp=%u op_align=%u op_shift=%u BLIT=%u blit_area=%u\n",
               g_prof_destofen_total, g_prof_destofen_transp, g_prof_destofen_op_align, g_prof_destofen_op_shift, g_prof_destofen_blit, g_prof_destofen_blit_area);
        dbglog("PROFILE/100f OLD-path reason: flip=%u table_miss=%u other_fmt=%u\n",
               g_prof_destofen_skip_flip, g_prof_destofen_skip_miss, g_prof_destofen_skip_fmt);
        dbglog("PROFILE/100f transp split: align=%u (area=%u) shift=%u (area=%u)  C-FALLBACK=%u (area=%u)\n",
               g_prof_destofen_tr_align, g_prof_destofen_tr_align_area, g_prof_destofen_tr_shift, g_prof_destofen_tr_shift_area, g_prof_destofen_cfb, g_prof_destofen_cfb_area);
        dbglog("PROFILE/100f OPAQUE-C: calls=%u area=%u\n", g_prof_destofen_op_calls, g_prof_destofen_op_area);
        prof_frame = 0;
        prof_pixel=prof_line=prof_boxf=prof_rect=0;
        prof_st4_0=prof_st4_2=prof_8bit_0=prof_8bit_2=0;
        prof_4to8_0=prof_4to8_2=prof_trsfen=prof_clrfen=0;
        g_prof_destofen_total=g_prof_destofen_transp=g_prof_destofen_op_shift=g_prof_destofen_op_align=g_prof_destofen_blit=g_prof_destofen_blit_area=0;
        g_prof_destofen_skip_flip=g_prof_destofen_skip_miss=g_prof_destofen_skip_fmt=0;
        g_prof_destofen_tr_align=g_prof_destofen_tr_shift=g_prof_destofen_tr_align_area=g_prof_destofen_tr_shift_area=0;
        g_prof_destofen_cfb=g_prof_destofen_cfb_area=0;
        g_prof_destofen_op_calls=g_prof_destofen_op_area=0;
    }
#endif // ALIS_PROFILE_DRAW
}

#if ALIS_SDL_VER < 2
# include <SDL/SDL.h>
extern u8 dirty_len;
extern SDL_Rect dirty_rects[256];
#endif

// ===========================================================================
// Planar helpers — word-at-a-time operations
// Modeled after the original Falcon blitter routines (rrq-falcon.asm)
// ===========================================================================

// Single pixel write (used for draw_pixel, draw_line, and edge cases)
static inline void planar_put_pixel(u8 *buf, s32 x, s32 y, u8 color)
{
    u16 *base = (u16 *)(buf + y * PLANAR_PITCH + (x >> 4) * 16);
    u16 mask = 0x8000 >> (x & 15);
    u16 nmask = ~mask;
    for (int p = 0; p < PLANES; p++) {
        if (color & (1 << p))
            base[p] |= mask;
        else
            base[p] &= nmask;
    }
}

// Expand 8-bit color to 8 plane words (0x0000 or 0xFFFF per plane)
// Same technique as blotofen in the Falcon ASM: bit-test + expand
static inline void color_to_planes(u8 color, u16 *pw)
{
    for (int p = 0; p < PLANES; p++)
        pw[p] = (color & (1 << p)) ? 0xFFFF : 0x0000;
}

// One scanline of solid fill with a COMPILE-TIME plane count NP. Constant NP lets gcc
// fold the inner p<NP loops to fixed move.l plane-pair stores (no runtime dispatch);
// the colour planes pw[] hoist into registers so the middle loop is store-only.
__attribute__((always_inline))
static inline void hfill_row_np(u8 *line, s32 x1, s32 x2, const u16 *pw, const int NP)
{
    // Left edge: partial 16-pixel group
    s32 first_full = (x1 + 15) & ~15;  // first 16-aligned pixel >= x1
    if (first_full > x2) first_full = x2;

    if (x1 & 15) {
        u16 *base = (u16 *)(line + (x1 >> 4) * 16);
        // Mask: bits for pixels x1..(first_full-1) within this 16px group
        u16 mask = (0xFFFF >> (x1 & 15));
        if (first_full < ((x1 >> 4) + 1) * 16)
            mask &= ~(0xFFFF >> (first_full & 15 ? first_full & 15 : 16));
        u16 nmask = ~mask;
        for (int p = 0; p < NP; p++)
            base[p] = (base[p] & nmask) | (pw[p] & mask);
    }

    // Full 16-pixel groups
    s32 last_full = x2 & ~15;  // last 16-aligned pixel < x2
    for (s32 x = first_full; x < last_full; x += 16) {
        u16 *base = (u16 *)(line + (x >> 4) * 16);
        for (int p = 0; p < NP; p++)
            base[p] = pw[p];
    }

    // Right edge: partial 16-pixel group
    if (last_full < x2 && last_full >= first_full) {
        u16 *base = (u16 *)(line + (last_full >> 4) * 16);
        u16 mask = ~(0xFFFF >> (x2 & 15 ? x2 & 15 : 16));
        if (last_full < first_full) mask &= (0xFFFF >> (x1 & 15));
        u16 nmask = ~mask;
        for (int p = 0; p < NP; p++)
            base[p] = (base[p] & nmask) | (pw[p] & mask);
    }
}

// Solid horizontal span; writes only g_planar_nplanes planes (preserves clinepal planes).
// no-unroll keeps the middle loop in the i-cache (see planar_hline_copy).
__attribute__((optimize("no-unroll-loops")))
static void planar_hline_fill(u8 *buf, s32 x1, s32 x2, s32 y, u8 color)
{
    if (x2 <= x1) return;

    u16 pw[PLANES];
    color_to_planes(color, pw);

    u8 *line = buf + y * PLANAR_PITCH;

    switch (g_planar_nplanes) {
        case 4:  hfill_row_np(line, x1, x2, pw, 4); break;
        case 6:  hfill_row_np(line, x1, x2, pw, 6); break;
        default: hfill_row_np(line, x1, x2, pw, 8); break;
    }
}

// One scanline of planar copy with a compile-time plane count NP (folds to move.l plane pairs).
__attribute__((always_inline))
static inline void hcopy_row_np(u8 *dst, u8 *src, s32 offset,
                                s32 x1, s32 x2, s32 first_full, s32 last_full,
                                const int NP)
{
    // Left edge: partial 16px group, per-pixel RMW (preserves the other columns).
    for (s32 x = x1; x < first_full && x < x2; x++) {
        u16 *sb = (u16 *)(src + offset + (x >> 4) * 16);
        u16 *db = (u16 *)(dst + offset + (x >> 4) * 16);
        u16 mask = 0x8000 >> (x & 15), nmask = ~mask;
        for (int p = 0; p < NP; p++)
            db[p] = (db[p] & nmask) | (sb[p] & mask);
    }

    // Full 16px groups: copy NP plane-words/chunk, step a full 8-word chunk (skip high).
    if (last_full > first_full) {
        u16 *sb = (u16 *)(src + offset + (first_full >> 4) * 16);
        u16 *db = (u16 *)(dst + offset + (first_full >> 4) * 16);
        for (s32 x = first_full; x < last_full; x += 16, sb += 8, db += 8)
            for (int p = 0; p < NP; p++)
                db[p] = sb[p];
    }

    // Right edge: partial 16px group, per-pixel RMW.
    for (s32 x = (last_full > first_full ? last_full : first_full); x < x2; x++) {
        u16 *sb = (u16 *)(src + offset + (x >> 4) * 16);
        u16 *db = (u16 *)(dst + offset + (x >> 4) * 16);
        u16 mask = 0x8000 >> (x & 15), nmask = ~mask;
        for (int p = 0; p < NP; p++)
            db[p] = (db[p] & nmask) | (sb[p] & mask);
    }
}

// Copy a horizontal span between planar buffers, moving only g_used_planes planes per chunk.
// no-unroll-loops (also applied to the inlined body) keeps the middle loop within the
// 256-byte i-cache, so instruction fetch doesn't compete with pixel data on the bus.
__attribute__((optimize("no-unroll-loops")))
static void planar_hline_copy(u8 *dst, u8 *src, s32 x1, s32 x2, s32 y)
{
    if (x2 <= x1) return;

    s32 offset = y * PLANAR_PITCH;
    s32 first_full = (x1 + 15) & ~15;
    s32 last_full = x2 & ~15;

    switch (g_used_planes) {
        case 4:  hcopy_row_np(dst, src, offset, x1, x2, first_full, last_full, 4); break;
        case 6:  hcopy_row_np(dst, src, offset, x1, x2, first_full, last_full, 6); break;
        default: hcopy_row_np(dst, src, offset, x1, x2, first_full, last_full, 8); break;
    }
}

// Core chunky-to-planar conversion for up to 16 pixels.
// Branchless bit expansion: -(u16)(x & 1) yields 0xFFFF if bit set, 0 otherwise.
// Unrolled plane loop avoids inner loop overhead.
// Modeled after Falcon cdestofen AND-OR write pattern.

static inline void cdestofen_span_c(
    const u8 *pixels, int count, u16 *dst, int startbit)
{
    u16 p0=0, p1=0, p2=0, p3=0, p4=0, p5=0, p6=0, p7=0;
    u16 tmask = 0;

    for (int i = 0; i < count; i++) {
        u8 c = pixels[i];
        if (c == 0) continue;
        u16 bit = 0x8000 >> (startbit + i);
        tmask |= bit;
        p0 |= bit & -(u16)((c     ) & 1);
        p1 |= bit & -(u16)((c >> 1) & 1);
        p2 |= bit & -(u16)((c >> 2) & 1);
        p3 |= bit & -(u16)((c >> 3) & 1);
        p4 |= bit & -(u16)((c >> 4) & 1);
        p5 |= bit & -(u16)((c >> 5) & 1);
        p6 |= bit & -(u16)((c >> 6) & 1);
        p7 |= bit & -(u16)((c >> 7) & 1);
    }

    if (tmask == 0) return;

    u16 nm = ~tmask;
    dst[0] = (dst[0] & nm) | p0;
    dst[1] = (dst[1] & nm) | p1;
    dst[2] = (dst[2] & nm) | p2;
    dst[3] = (dst[3] & nm) | p3;
    dst[4] = (dst[4] & nm) | p4;
    dst[5] = (dst[5] & nm) | p5;
    dst[6] = (dst[6] & nm) | p6;
    dst[7] = (dst[7] & nm) | p7;
}

static inline void cdestofen_span_opaque_c(
    const u8 *pixels, int count, u16 *dst, int startbit)
{
    u16 p0=0, p1=0, p2=0, p3=0, p4=0, p5=0, p6=0, p7=0;

    for (int i = 0; i < count; i++) {
        u8 c = pixels[i];
        u16 bit = 0x8000 >> (startbit + i);
        p0 |= bit & -(u16)((c     ) & 1);
        p1 |= bit & -(u16)((c >> 1) & 1);
        p2 |= bit & -(u16)((c >> 2) & 1);
        p3 |= bit & -(u16)((c >> 3) & 1);
        p4 |= bit & -(u16)((c >> 4) & 1);
        p5 |= bit & -(u16)((c >> 5) & 1);
        p6 |= bit & -(u16)((c >> 6) & 1);
        p7 |= bit & -(u16)((c >> 7) & 1);
    }

    if (count == 16 && startbit == 0) {
        dst[0]=p0; dst[1]=p1; dst[2]=p2; dst[3]=p3;
        dst[4]=p4; dst[5]=p5; dst[6]=p6; dst[7]=p7;
    } else {
        u16 mask = 0;
        for (int i = 0; i < count; i++)
            mask |= 0x8000 >> (startbit + i);
        u16 nm = ~mask;
        dst[0]=(dst[0]&nm)|p0; dst[1]=(dst[1]&nm)|p1;
        dst[2]=(dst[2]&nm)|p2; dst[3]=(dst[3]&nm)|p3;
        dst[4]=(dst[4]&nm)|p4; dst[5]=(dst[5]&nm)|p5;
        dst[6]=(dst[6]&nm)|p6; dst[7]=(dst[7]&nm)|p7;
    }
}

// ===========================================================================
// Drawing functions
// ===========================================================================

static void dirty_rect(s16 x, s16 y, s16 w, s16 h)
{
#if ALIS_SDL_VER < 2
    if (dirty_len > 0xfd) {
        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 0xff;
    } else {
        dirty_rects[dirty_len] = (SDL_Rect){ .x = x, .y = y, .w = w, .h = h };
        dirty_len++;
    }
#endif
}

void draw_pixel(s16 x0, s16 y0)
{
    prof_pixel++;
    planar_put_pixel(image.logic, x0, y0, image.inkcolor);
    dirty_rect(x0, y0, 1, 1);
}

void draw_line(s16 x0, s16 y0, s16 x1, s16 y1)
{
    prof_line++;
    dirty_rect(min(x0, x1), min(y0, y1), abs(x1 - x0) + 1, abs(y1 - y0) + 1);
    s32 dx = abs(x1 - x0);
    s32 sx = x0 < x1 ? 1 : -1;
    s32 dy = -abs(y1 - y0);
    s32 sy = y0 < y1 ? 1 : -1;
    s32 error = dx + dy;

    while (true)
    {
        planar_put_pixel(image.logic, x0, y0, image.inkcolor);
        if (x0 == x1 && y0 == y1) break;
        s32 e2 = 2 * error;
        if (e2 >= dy) { if (x0 == x1) break; error += dy; x0 += sx; }
        if (e2 <= dx) { if (y0 == y1) break; error += dx; y0 += sy; }
    }
}

// draw_box stays in image.c

void draw_boxf(s16 x1, s16 y1, s16 x2, s16 y2)
{
    prof_boxf++;
    if (alis.platform.kind == EPlatformMac) {
        mac_update_pos(&x1, &y1);
        mac_update_pos(&x2, &y2);
    }

    s16 tmpx = min(x1, x2);
    x2 = max(x1, x2);
    x1 = tmpx;
    tmpx = x2 - x1;

    dirty_rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);

    if (tmpx <= 0) return;

    // Large in-bounds fill: per-plane Blitter fill; end-masks trim the boundary chunks.
    {
        s32 sw = host.pixelbuf.w, sh = host.pixelbuf.h;
        s32 w = x2 - x1 + 1, h = y2 - y1 + 1;
        if (g_hw_blitter && w * h >= 512 && x1 >= 0 && y1 >= 0 && x2 < sw && y2 < sh) {
            extern void sys_blit_fill_planar(u16 *, int, int, int, u16, u16, u16);
            // x2 is exclusive here: right edge from x2-1.
            s32 chunk0 = x1 >> 4, chunkN = (x2 - 1) >> 4, chunks_w = chunkN - chunk0 + 1;
            u16 em1 = (u16)(0xFFFFu >> (x1 & 15));
            u16 em3 = (u16)((0xFFFFu << (15 - ((x2 - 1) & 15))) & 0xFFFFu);
            if (chunks_w == 1) em1 = (u16)(em1 & em3);          // single chunk: combine edge masks
            u16 *dst = (u16 *)image.logic + (s32)y1 * (sw >> 1) + (s32)chunk0 * 8;
            sys_blit_fill_planar(dst, sw, chunks_w, h, image.inkcolor, em1, em3);
            return;
        }
    }

    for (s16 y = y1; y <= y2; y++)
        planar_hline_fill(image.logic, x1, x2, y, image.inkcolor);
}

void draw_mac_rect(sRect *pos, sRect *bmp, u8 color)
{
    if (color == 15) {
        for (s32 h = bmp->y1; h < bmp->y2; h++)
            for (s32 w = bmp->x1; w < bmp->x2; w++)
                planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h, (w + h) % 2);
    } else {
        color = !color;
        for (s32 h = bmp->y1; h < bmp->y2; h++)
            planar_hline_fill(image.wdraw, pos->x1 + bmp->x1, pos->x1 + bmp->x2, pos->y1 + h, color);
    }
}

void draw_rect(sRect *pos, sRect *bmp, u8 color)
{
    prof_rect++;

    // Large in-bounds rect: Blitter fill. x2/y2 are exclusive here.
    {
        s32 sw = host.pixelbuf.w, sh = host.pixelbuf.h;
        s32 fx1 = pos->x1 + bmp->x1, fx2 = pos->x1 + bmp->x2 - 1;   // inclusive last pixel
        s32 fy1 = pos->y1 + bmp->y1, fy2 = pos->y1 + bmp->y2 - 1;   // inclusive last row
        s32 w = fx2 - fx1 + 1, h = fy2 - fy1 + 1;
        // Blitter reaches ST-RAM only: not while drawing into the background cache.
        if (g_hw_blitter && image.wdraw == image.logic
            && w * h >= 512 && fx1 >= 0 && fy1 >= 0 && fx2 < sw && fy2 < sh) {
            extern void sys_blit_fill_planar(u16 *, int, int, int, u16, u16, u16);
            s32 chunk0 = fx1 >> 4, chunkN = fx2 >> 4, chunks_w = chunkN - chunk0 + 1;
            u16 em1 = (u16)(0xFFFFu >> (fx1 & 15));
            u16 em3 = (u16)((0xFFFFu << (15 - (fx2 & 15))) & 0xFFFFu);
            if (chunks_w == 1) em1 = (u16)(em1 & em3);
            u16 *dst = (u16 *)image.logic + (s32)fy1 * (sw >> 1) + (s32)chunk0 * 8;
            sys_blit_fill_planar(dst, sw, chunks_w, h, color, em1, em3);
            return;
        }
    }

    for (s32 h = bmp->y1; h < bmp->y2; h++)
        planar_hline_fill(image.wdraw, pos->x1 + bmp->x1, pos->x1 + bmp->x2, pos->y1 + h, color);
}

// ===========================================================================
// Sprite blitters — 16-pixel batched conversion
// ===========================================================================

// 8-bit chunky source with transparency (color 0 = transparent)
// Non-flip: uses ASM inner loop directly on source data.
// Flip: reverses into small stack buffer per 16px group, then ASM.
void draw_8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    prof_8bit_0++;
    s32 dst_x0 = pos->x1 + bmp->x1;
    s32 span = bmp->x2 - bmp->x1;

    for (s32 h = bmp->y1; h < bmp->y2; h++) {
        u8 *line = image.wdraw + (pos->y1 + h) * PLANAR_PITCH;
        s32 x = dst_x0;
        s32 remaining = span;

        if (!flip) {
            u8 *sp = at + 2 + h * width + bmp->x1;
            while (remaining > 0) {
                s32 gstart = x & 15;
                s32 count = 16 - gstart;
                if (count > remaining) count = remaining;
                u16 *dst = (u16 *)(line + (x >> 4) * 16);
                cdestofen_span_asm(sp, dst, count, gstart);
                sp += count;
                x += count;
                remaining -= count;
            }
        } else {
            u8 *sp = at + 2 + h * width + (width - 1 - bmp->x1);
            while (remaining > 0) {
                s32 gstart = x & 15;
                s32 count = 16 - gstart;
                if (count > remaining) count = remaining;
                // Reverse into temp for ASM (16 bytes on stack)
                u8 tmp[16];
                for (s32 i = 0; i < count; i++)
                    tmp[i] = *(sp - i);
                sp -= count;
                u16 *dst = (u16 *)(line + (x >> 4) * 16);
                cdestofen_span_asm(tmp, dst, count, gstart);
                x += count;
                remaining -= count;
            }
        }
    }
}

// 8-bit chunky source, opaque
void draw_8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    prof_8bit_2++;
    s32 dst_x0 = pos->x1 + bmp->x1;
    s32 span = bmp->x2 - bmp->x1;

    for (s32 h = bmp->y1; h < bmp->y2; h++) {
        u8 *line = image.wdraw + (pos->y1 + h) * PLANAR_PITCH;
        s32 x = dst_x0;
        s32 remaining = span;

        if (!flip) {
            u8 *sp = at + 2 + h * width + bmp->x1;
            while (remaining > 0) {
                s32 gstart = x & 15;
                s32 count = 16 - gstart;
                if (count > remaining) count = remaining;
                u16 *dst = (u16 *)(line + (x >> 4) * 16);
                cdestofen_span_opaque_asm(sp, dst, count, gstart);
                sp += count;
                x += count;
                remaining -= count;
            }
        } else {
            u8 *sp = at + 2 + h * width + (width - 1 - bmp->x1);
            while (remaining > 0) {
                s32 gstart = x & 15;
                s32 count = 16 - gstart;
                if (count > remaining) count = remaining;
                u8 tmp[16];
                for (s32 i = 0; i < count; i++)
                    tmp[i] = *(sp - i);
                sp -= count;
                u16 *dst = (u16 *)(line + (x >> 4) * 16);
                cdestofen_span_opaque_asm(tmp, dst, count, gstart);
                x += count;
                remaining -= count;
            }
        }
    }
}

// Helper: decode a run of packed 4-bit nibbles into 8-bit bytes (16 max)
static inline void decode_nibbles(const u8 *src_base, s32 sw, s32 sw_dir, s32 count, u8 *out)
{
    for (s32 i = 0; i < count; i++, sw += sw_dir) {
        u8 byte = src_base[sw >> 1];
        out[i] = (sw & 1) ? (byte & 0x0f) : (byte >> 4);
    }
}

// ST 4-bit packed source with transparency
// Decode nibbles to temp, pass to ASM 8-bit transparent blitter
void draw_st_4bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    prof_st4_0++;
    u16 swadd = width >> 1;
    u8 *src_base = at + bmp->y1 * swadd;
    s32 dst_x0 = pos->x1 + bmp->x1;
    s32 span = bmp->x2 - bmp->x1;

    for (s32 h = bmp->y1; h < bmp->y2; h++, src_base += swadd) {
        u8 *line = image.wdraw + (pos->y1 + h) * PLANAR_PITCH;
        s32 x = dst_x0;
        s32 remaining = span;
        s32 sw = flip ? (width - 1 - bmp->x1) : bmp->x1;
        s32 sw_dir = flip ? -1 : 1;

        while (remaining > 0) {
            s32 gstart = x & 15;
            s32 count = 16 - gstart;
            if (count > remaining) count = remaining;

            u8 tmp[16];
            decode_nibbles(src_base, sw, sw_dir, count, tmp);
            sw += sw_dir * count;

            u16 *dst = (u16 *)(line + (x >> 4) * 16);
            cdestofen_span_asm(tmp, dst, count, gstart);

            x += count;
            remaining -= count;
        }
    }
}

// ST 4-bit packed source, opaque
void draw_st_4bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    prof_st4_2++;
    u16 swadd = width >> 1;
    u8 *src_base = at + bmp->y1 * swadd;
    s32 dst_x0 = pos->x1 + bmp->x1;
    s32 span = bmp->x2 - bmp->x1;

    for (s32 h = bmp->y1; h < bmp->y2; h++, src_base += swadd) {
        u8 *line = image.wdraw + (pos->y1 + h) * PLANAR_PITCH;
        s32 x = dst_x0;
        s32 remaining = span;
        s32 sw = flip ? (width - 1 - bmp->x1) : bmp->x1;
        s32 sw_dir = flip ? -1 : 1;

        while (remaining > 0) {
            s32 gstart = x & 15;
            s32 count = 16 - gstart;
            if (count > remaining) count = remaining;

            u8 tmp[16];
            decode_nibbles(src_base, sw, sw_dir, count, tmp);
            sw += sw_dir * count;

            u16 *dst = (u16 *)(line + (x >> 4) * 16);
            cdestofen_span_opaque_asm(tmp, dst, count, gstart);

            x += count;
            remaining -= count;
        }
    }
}

// 4-to-8-bit with palette offset, transparent
// Helper: decode nibbles with palette offset (non-zero nibbles get offset added)
static inline void decode_nibbles_pal(const u8 *src_base, s32 sw, s32 sw_dir,
                                       s32 count, u8 *out, u8 pal_offset)
{
    for (s32 i = 0; i < count; i++, sw += sw_dir) {
        u8 byte = src_base[sw >> 1];
        u8 nibble = (sw & 1) ? (byte & 0x0f) : (byte >> 4);
        out[i] = nibble ? (pal_offset + nibble) : 0;
    }
}

// 4-to-8-bit with palette offset, transparent
// Decode nibbles + pal_offset to temp, pass to ASM transparent blitter
void draw_4to8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset)
{
    prof_4to8_0++;
    u16 swadd = width >> 1;
    u8 *src_base = at + bmp->y1 * swadd;
    s32 dst_x0 = pos->x1 + bmp->x1;
    s32 span = bmp->x2 - bmp->x1;

    for (s32 h = bmp->y1; h < bmp->y2; h++, src_base += swadd) {
        u8 *line = image.wdraw + (pos->y1 + h) * PLANAR_PITCH;
        s32 x = dst_x0;
        s32 remaining = span;
        s32 sw = flip ? (width - 1 - bmp->x1) : bmp->x1;
        s32 sw_dir = flip ? -1 : 1;

        while (remaining > 0) {
            s32 gstart = x & 15;
            s32 count = 16 - gstart;
            if (count > remaining) count = remaining;

            u8 tmp[16];
            decode_nibbles_pal(src_base, sw, sw_dir, count, tmp, pal_offset);
            sw += sw_dir * count;

            u16 *dst = (u16 *)(line + (x >> 4) * 16);
            cdestofen_span_asm(tmp, dst, count, gstart);

            x += count;
            remaining -= count;
        }
    }
}

// 4-to-8-bit with palette offset, opaque
void draw_4to8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset)
{
    prof_4to8_2++;
    u16 swadd = width >> 1;
    u8 *src_base = at + bmp->y1 * swadd;
    s32 dst_x0 = pos->x1 + bmp->x1;
    s32 span = bmp->x2 - bmp->x1;

    for (s32 h = bmp->y1; h < bmp->y2; h++, src_base += swadd) {
        u8 *line = image.wdraw + (pos->y1 + h) * PLANAR_PITCH;
        s32 x = dst_x0;
        s32 remaining = span;
        s32 sw = flip ? (width - 1 - bmp->x1) : bmp->x1;
        s32 sw_dir = flip ? -1 : 1;

        while (remaining > 0) {
            s32 gstart = x & 15;
            s32 count = 16 - gstart;
            if (count > remaining) count = remaining;

            u8 tmp[16];
            // Opaque: all nibbles get offset (even 0)
            for (s32 i = 0; i < count; i++, sw += sw_dir) {
                u8 byte = src_base[sw >> 1];
                tmp[i] = pal_offset + ((sw & 1) ? (byte & 0x0f) : (byte >> 4));
            }

            u16 *dst = (u16 *)(line + (x >> 4) * 16);
            cdestofen_span_opaque_asm(tmp, dst, count, gstart);

            x += count;
            remaining -= count;
        }
    }
}

// ===========================================================================
// Mac/DOS format decoders — pixel-by-pixel (rarely used on Atari)
// ===========================================================================

void draw_mac_mono_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 index, color;
    u16 swadd = width >> 2;
    u8 *src = at + bmp->y1 * swadd;
    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = 3 - w % 4;
                color = (src[(width - (w + 1)) >> 2] & masks[index]) >> rots[index];
                if (!(color & 2))
                    planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h, !(color & 1));
            }
    } else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = w % 4;
                color = (src[w >> 2] & masks[index]) >> rots[index];
                if (!(color & 2))
                    planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h, !(color & 1));
            }
    }
}

void draw_mac_mono_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 index;
    u16 swadd = width >> 2;
    u8 *src = at + bmp->y1 * swadd;
    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = 3 - w % 4;
                planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h,
                    !(((src[(width - (w + 1)) >> 2] & masks[index]) >> rots[index]) & 1));
            }
    } else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = w % 4;
                planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h,
                    !(((src[w >> 2] & masks[index]) >> rots[index]) & 1));
            }
    }
}

void draw_dos_cga_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u8 index;
    s16 wh;
    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                wh = ((width - (w + 1)) >> 2) << 1;
                index = 3 - w % 4;
                if (!((src[wh] & masks[index]) >> rots[index]))
                    planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h,
                        (src[wh + 1] & masks[index]) >> rots[index]);
            }
    } else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                wh = (w >> 2) << 1;
                index = w % 4;
                if (!((src[wh] & masks[index]) >> rots[index]))
                    planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h,
                        (src[wh + 1] & masks[index]) >> rots[index]);
            }
    }
}

void draw_dos_cga_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u16 swadd = width >> 2;
    u8 *src = at + bmp->y1 * swadd;
    u8 index;
    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = 3 - w % 4;
                planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h,
                    (src[(width - (w + 1)) >> 2] & masks[index]) >> rots[index]);
            }
    } else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd)
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = w % 4;
                planar_put_pixel(image.wdraw, pos->x1 + w, pos->y1 + h,
                    (src[w >> 2] & masks[index]) >> rots[index]);
            }
    }
}

// ===========================================================================
// Amiga 5-bit planar format decoders
// ===========================================================================

void draw_ami_5bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip)
{
    u32 planesize = (width * height) >> 3;
    u16 swadd = width;
    u16 srcoff = bmp->y1 * swadd;

    for (s32 h = bmp->y1; h < bmp->y2; h++, srcoff += swadd) {
        s32 dst_x = pos->x1 + bmp->x1;
        s32 dst_y = pos->y1 + h;
        s32 span = bmp->x2 - bmp->x1;
        u8 *line = image.wdraw + dst_y * PLANAR_PITCH;

        u8 row[512];
        for (s32 w = 0; w < span; w++) {
            s32 sw = flip ? (width - 1 - (bmp->x1 + w)) : (bmp->x1 + w);
            s32 idx = (sw + srcoff) >> 3;
            s32 bit = flip ? ((sw + srcoff) & 7) : (7 - ((sw + srcoff) & 7));
            row[w] = ((at[idx] >> bit) & 1)
                   | (((at[idx + planesize] >> bit) & 1) << 1)
                   | (((at[idx + planesize * 2] >> bit) & 1) << 2)
                   | (((at[idx + planesize * 3] >> bit) & 1) << 3)
                   | (((at[idx + planesize * 4] >> bit) & 1) << 4);
        }

        s32 x = dst_x;
        s32 w = 0;
        while (w < span) {
            s32 group_start = x & 15;
            s32 count = 16 - group_start;
            if (count > span - w) count = span - w;
            u16 *dst = (u16 *)(line + (x >> 4) * 16);
            cdestofen_span_c(&row[w], count, dst, group_start);
            x += count;
            w += count;
        }
    }
}

void draw_ami_5bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip)
{
    u32 planesize = (width * height) >> 3;
    u16 swadd = width;
    u16 srcoff = bmp->y1 * swadd;

    for (s32 h = bmp->y1; h < bmp->y2; h++, srcoff += swadd) {
        s32 dst_x = pos->x1 + bmp->x1;
        s32 dst_y = pos->y1 + h;
        s32 span = bmp->x2 - bmp->x1;
        u8 *line = image.wdraw + dst_y * PLANAR_PITCH;

        u8 row[512];
        for (s32 w = 0; w < span; w++) {
            s32 sw = flip ? (width - 1 - (bmp->x1 + w)) : (bmp->x1 + w);
            s32 idx = (sw + srcoff) >> 3;
            s32 bit = flip ? ((sw + srcoff) & 7) : (7 - ((sw + srcoff) & 7));
            row[w] = ((at[idx] >> bit) & 1)
                   | (((at[idx + planesize] >> bit) & 1) << 1)
                   | (((at[idx + planesize * 2] >> bit) & 1) << 2)
                   | (((at[idx + planesize * 3] >> bit) & 1) << 3)
                   | (((at[idx + planesize * 4] >> bit) & 1) << 4);
        }

        s32 x = dst_x;
        s32 w = 0;
        while (w < span) {
            s32 group_start = x & 15;
            s32 count = 16 - group_start;
            if (count > span - w) count = span - w;
            u16 *dst = (u16 *)(line + (x >> 4) * 16);
            cdestofen_span_opaque_c(&row[w], count, dst, group_start);
            x += count;
            w += count;
        }
    }
}

// ===========================================================================
// Screen transfer / clear — word-at-a-time
// ===========================================================================

// draw_fli_video stays in image.c

void trsfen(u8 *src, u8 *tgt)
{
    prof_trsfen++;
    s16 fx1 = image.fenx1;
    s16 fy1 = image.feny1;
    s16 fx2 = image.fenx2;
    s16 fy2 = image.feny2;
    if (fx1 < 0) fx1 = 0;
    if (fy1 < 0) fy1 = 0;
    if (fx2 >= alis.platform.width)  fx2 = alis.platform.width - 1;
    if (fy2 >= alis.platform.height) fy2 = alis.platform.height - 1;
    if (fx2 < fx1 || fy2 < fy1)
        return;

    // Large chunk-aligned region: per-plane Blitter copy.
    {
        s32 w = alis.platform.width;
        s32 rw = fx2 - fx1 + 1, rh = fy2 - fy1 + 1;
        if (g_hw_blitter && (fx1 & 15) == 0 && ((fx2 + 1) & 15) == 0 && rw * rh >= 512) {
            s32 chunks_w = rw >> 4;
            s32 off = (s32)fy1 * w + (s32)fx1;     // byte offset to chunk(fx1), row fy1
            sys_blit_copy_planar((u16 *)(src + off), w, (u16 *)(tgt + off), w, chunks_w, rh, g_used_planes);
            return;
        }
    }

    for (s32 y = fy1; y <= fy2; y++)
        planar_hline_copy(tgt, src, fx1, fx2 + 1, y);
}

void clrfen(void)
{
    prof_clrfen++;
    s16 fx1 = image.fenx1 < 0 ? 0 : image.fenx1;
    s16 fy1 = image.feny1 < 0 ? 0 : image.feny1;
    s16 fx2 = image.fenx2 >= alis.platform.width ? alis.platform.width - 1 : image.fenx2;
    s16 fy2 = image.feny2 >= alis.platform.height ? alis.platform.height - 1 : image.feny2;
    // fen is inclusive; planar_hline_fill takes an exclusive x2.
    s16 tmpx = (fx2 - fx1) + 1;
    if (tmpx <= 0 || fy2 < fy1)
        return;
    for (s16 y = fy1; y <= fy2; y++)
        planar_hline_fill(image.logic, fx1, fx2 + 1, y, 0);
}

// ===========================================================================
// Line palette (clinepal) — per-row bank encoded in planes 4-5
// ===========================================================================
// A clinepal game is 4-bit and picks one of 4 palettes per row. The bank goes into planes
// 4-5 (CLUT index = bank*16 + pixel); 4-bit draws preserve those planes, so stamping both
// buffers when the table changes needs no per-draw work.

// Stamp planes 4-5 of one buffer for every row, walking the firstpal region table
// the same way build_line_paloff does (entries { dummy, line, paloff=bank*64, pad }).
static void planar_linepal_stamp_buf(u8 *buf, int chunks, int h, int stride)
{
    s16 *cur = image.firstpal;
    s16 *nxt = cur + stride;
    for (int y = 0; y < h; y++) {
        while (nxt[0] != (s16)0xff && y >= nxt[1]) { cur = nxt; nxt = cur + stride; }
        int bank = (u16)cur[2] / 64;                       // 0..3
        u16 p4 = (bank & 1) ? 0xFFFF : 0x0000;
        u16 p5 = (bank & 2) ? 0xFFFF : 0x0000;
        u16 *line = (u16 *)(buf + y * PLANAR_PITCH);
        for (int c = 0; c < chunks; c++) { line[c * 8 + 4] = p4; line[c * 8 + 5] = p5; }
    }
}

// clinepal enabled / a band changed: re-stamp both buffers and widen the screen copy
// to include the bank planes (4-5). Called from setlinepalet after linepal() rebuilds
// the table. (dirty_pal is already set there → set_palette re-packs the CLUT to ×16.)
void planar_linepal_enable(void)
{
    // Bank bits go in planes 4-5 → only free when pixels use planes 0-3 (a 4-bit game).
    // For deeper games plane 4 holds pixel data, so the high-plane encoding doesn't fit.
    if (g_planar_nplanes > 4)
        return;
    int h = alis.platform.height; if (h > 256) h = 256;
    int chunks = (alis.platform.width + 15) >> 4;
    int stride = 2 + (sizeof(u8 *) >> 1);
    if (image.physic) planar_linepal_stamp_buf(image.physic, chunks, h, stride);
    if (image.logic)  planar_linepal_stamp_buf(image.logic,  chunks, h, stride);
    g_used_planes = 6;                                     // planes 0-5 now carry data
    dirty_rect(0, 0, alis.platform.width, alis.platform.height);
}

// clinepal disabled: clear the bank planes in both buffers and restore the copy depth.
void planar_linepal_disable(void)
{
    if (g_planar_nplanes <= 4) {            // only clear bank planes we actually used
        int h = alis.platform.height; if (h > 256) h = 256;
        int chunks = (alis.platform.width + 15) >> 4;
        u8 *bufs[2] = { image.physic, image.logic };
        for (int bi = 0; bi < 2; bi++) {
            if (!bufs[bi]) continue;
            for (int y = 0; y < h; y++) {
                u16 *line = (u16 *)(bufs[bi] + y * PLANAR_PITCH);
                for (int c = 0; c < chunks; c++) { line[c * 8 + 4] = 0; line[c * 8 + 5] = 0; }
            }
        }
    }
    g_used_planes = g_planar_nplanes;
    dirty_rect(0, 0, alis.platform.width, alis.platform.height);
}

#endif /* ALIS_NATIVE_PLANAR */
