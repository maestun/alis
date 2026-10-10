//
// 16-bit (Falcon BPS16 truecolor) direct blitters.
//
// Native renderer variant selected by ALIS_NATIVE_16BPP (USE_NATIVE_ATARI +
// FALCON_16BPP). image.physic/logic are 16-bit ST-RAM Videl framebuffers; these
// routines write `pal16[index]` (RGB565) straight into them — no chunky
// intermediate, no expand, free Setscreen flip (mirrors the planar path).
//
// Ported mechanically from the chunky draws in image.c: the source decoding and
// pixel arithmetic are identical; only the target pointer becomes u16* and each
// write goes through pal16[]. (u16* scales the same pixel offsets, so the
// host.pixelbuf.w / wadd / twadd strides are unchanged.)
//
// First cut: 8-bit and 4-to-8-bit sprites (the cheap, common formats) + clrfen +
// trsfen. Other formats are stubbed. NOTE: render3d.c writes 8-bit chunky, so 3D
// terrain scenes are NOT handled by this path yet — this is for 2D sprite scenes.
//
#include "config.h"
#if defined(ALIS_NATIVE_16BPP)

#include "image.h"
#include "alis.h"
#include "sys/sys.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

extern u16 pal16[256];   // 8-bit palette index -> RGB565 (built in sys_atari.c)
extern void mac_update_pos(s16 *x, s16 *y);
extern u8 dirty_len;     // native uses this only as a flip gate (see sys_render)

// Offload solid fills to the Falcon BLiTTER above this area (smaller fills aren't
// worth the Supexec setup). Toggle the whole thing with -DALIS_USE_BLITTER=0.
#ifndef ALIS_USE_BLITTER
#define ALIS_USE_BLITTER 1
#endif
#define BLIT_FILL_MIN_AREA 256

// ---------------------------------------------------------------------------
// Line palette (clinepal): per-row palette slice, folded into the blitters' pal16 lookup
// as a per-row offset (0 when inactive). Rebuilt each frame from image.firstpal.
// ---------------------------------------------------------------------------
u16 g_line_paloff[256];

// Source pixels expanded into the sprite cache this frame; spc_get caps it so a new scene
// ramps over a few frames instead of spiking.
u32 g_spc_exp_frame;
#define SPC_EXP_PX_PER_FRAME 32768

void build_line_paloff(void)
{
    g_spc_exp_frame = 0;
    int h = alis.platform.height;
    if (h > 256) h = 256;
    if (!image.flinepal) {
        memset(g_line_paloff, 0, (size_t)h * sizeof(u16));
        return;
    }
    // firstpal entries are { dummy, line, paloff, <ptr-pad> } — stride 4 s16 on
    // 32-bit. Walk to the entry covering each row; cur[2] is its palette offset.
    int stride = 2 + (sizeof(u8 *) >> 1);
    s16 *cur = image.firstpal;
    s16 *nxt = cur + stride;
    for (int y = 0; y < h; y++) {
        while (nxt[0] != (s16)0xff && y >= nxt[1]) { cur = nxt; nxt = cur + stride; }
        g_line_paloff[y] = (u16)cur[2];
    }
}

// pal16 lookup for the screen row currently being blitted (pos->y1 + h). Used by
// the 4-bit sprite blitters; pos and h must be in scope.
#define PX16(idx) pal16[(idx) + g_line_paloff[(pos->y1) + h]]

// ===========================================================================
// Sprite expansion cache: avoids the per-pixel pal16 LUT (random ST-RAM reads) by
// expanding each sprite to 16bpp once per (source, palette) and Blitter-copying it.
// Keyed by newad + palette gen + content checksum. ST-RAM only (Blitter can't reach alt-RAM).
// ===========================================================================
#if ALIS_USE_BLITTER
extern u32 g_pal16_gen;   // bumped by sys_pal16_refresh on a real palette change

#define SPC_ENTRIES   128
#define SPC_BUDGET    (1024u * 1024u)   // 1 MB ST-RAM cap
#define SPC_MIN_AREA  4096              // opaque (Blitter) only pays for big blits
#define SPC_RLE_MAX_AREA 16384          // RLE only for small transparent sprites (text/combat);
                                        // big transparent images use the CPU blitter

typedef struct {
    u32 newad, palgen, csum;
    u16 w, h;
    u32 lru, bytes;
    u16 *data;       // opaque: w*h 16bpp (Blitter-copied). RLE: run stream.
    u32 *rowoff;     // RLE only: per-row offset (u16 units) into data; NULL for opaque
    u8  is_rle;
} spc_entry;

static spc_entry g_spc[SPC_ENTRIES];
static u32 g_spc_clock, g_spc_bytes;
static u8  g_spc_hitflag;                       // set by spc_get: 1 = clean hit, 0 = (re)expanded

// Entries are keyed by VM offset: drop them all when program memory moves.
void sprite_cache_flush(void)
{
    for (int i = 0; i < SPC_ENTRIES; i++) {
        sys_stram_free(g_spc[i].data);
        g_spc[i].data = 0; g_spc[i].rowoff = 0;
    }
    g_spc_bytes = 0;
}
u32 g_spc_hit, g_spc_exp, g_spc_skip; u16 g_spc_skipfmt;   // per-window stats (profiler)
// Set by draw() each frame: 1 when the palette changed this frame (intro fade /
// time-of-day). While fading, bypass the cache and CPU-blit so the scene re-bakes
// with the live palette instead of serving a hit baked at an older palette.
u8 g_pal16_fading;

static u32 spc_srcbytes(u8 fmt, u16 w, u16 h)
{   // both formats have a 2-byte sub-header at `at`, pixels start at at+2
    return (fmt == 0x16) ? ((u32)w * h + 2) : ((u32)(w >> 1) * h + 2);
}
static u32 spc_csum(const u8 *at, u32 nbytes)
{
    u32 c = nbytes ^ 0x9e3779b9u;
    for (int k = 0; k < 8; k++) c = c * 31u + at[(nbytes * (u32)k) >> 3];
    return c;
}
// Expand the full opaque sprite into e->data. fmt 0x16 = 8-bit, 0x12 = 4-bit+pal_offset.
static void spc_expand(spc_entry *e, u8 fmt, const u8 *at)
{
    u16 w = e->w, h = e->h, *d = e->data;
    if (fmt == 0x16) {                          // 8-bit: 1 byte/px, pixels at at+2 (draw_8bit_2)
        const u8 *s = at + 2;
        u32 n = (u32)w * h;
        for (u32 i = 0; i < n; i++) d[i] = pal16[s[i]];
    } else {                                    // 0x12: pal_offset at[0], pixels at+2
        const u16 *p = pal16 + at[0];
        const u8 *s = at + 2;
        u32 swadd = (u32)w >> 1;
        for (u16 y = 0; y < h; y++) {
            const u8 *sr = s + (u32)y * swadd;
            u16 *dr = d + (u32)y * w, x = 0;
            for (; x + 1 < w; x += 2) { u8 b = *sr++; *dr++ = p[b >> 4]; *dr++ = p[b & 0x0f]; }
            if (x < w) *dr = p[*sr >> 4];        // odd trailing pixel
        }
    }
}
// Transparent sprites (fmt 0x14 8-bit, 0x10 4-bit; raw index 0 = transparent). Encode
// each row as runs: [skip:u16][count:u16][count 16bpp px], 0xffff ends a row. Pass 1
// (out==NULL) returns the stream u16 count; pass 2 fills out[] and rowoff[].
static u32 spc_rle_build(u16 *out, u32 *rowoff, u8 fmt, const u8 *at, u16 w, u16 h)
{
    int is4 = (fmt == 0x10);
    const u16 *pal = is4 ? (pal16 + at[0]) : pal16;
    const u8 *base = at + 2;
    u32 swadd = is4 ? ((u32)w >> 1) : (u32)w;
    u32 pos = 0;
    for (u16 y = 0; y < h; y++) {
        if (rowoff) rowoff[y] = pos;
        const u8 *s = base + (u32)y * swadd;
        u16 x = 0;
        while (x < w) {
            u16 skip = 0, startx, cnt = 0;
            while (x < w) { u8 r = is4 ? ((x&1)?(s[x>>1]&0x0f):(s[x>>1]>>4)) : s[x]; if (r) break; skip++; x++; }
            if (x >= w) break;
            startx = x;
            while (x < w) { u8 r = is4 ? ((x&1)?(s[x>>1]&0x0f):(s[x>>1]>>4)) : s[x]; if (!r) break; cnt++; x++; }
            if (out) {
                out[pos] = skip; out[pos+1] = cnt;
                for (u16 k = 0; k < cnt; k++) {
                    u16 xx = startx + k;
                    u8 r = is4 ? ((xx&1)?(s[xx>>1]&0x0f):(s[xx>>1]>>4)) : s[xx];
                    out[pos+2+k] = pal[r];
                }
            }
            pos += 2 + cnt;
        }
        if (out) out[pos] = 0xffff;
        pos += 1;
    }
    return pos;
}
// Burst-copy n 16bpp words src->framebuffer (movem amortises the VIDEL bus arbitration).
static void copy_run(u16 *dst, const u16 *src, s32 n)
{
    s32 blocks = n >> 4;                         // 16 words (8 longs) per movem block
    if (blocks > 0) {
        const u16 *send = src + (blocks << 4);   // end-of-bulk pointer (no spare data reg for a counter)
        __asm__ volatile(
            "1: movem.l (%0)+,%%d0-%%d7\n\t"
            "   movem.l %%d0-%%d7,(%1)\n\t"
            "   lea 32(%1),%1\n\t"
            "   cmpa.l %2,%0\n\t"
            "   blt.s 1b\n\t"
            : "+a"(src), "+a"(dst)
            : "a"(send)
            : "d0","d1","d2","d3","d4","d5","d6","d7","memory","cc");
    }
    s32 r = n & 15;                              // short-run remainder: move.l pairs (2px) then odd
    while (r >= 2) { *(u32 *)dst = *(const u32 *)src; dst += 2; src += 2; r -= 2; }
    if (r) *dst = *src;
}
static void spc_draw_rle(spc_entry *e, sRect *pos, sRect *bmp)
{
    s32 px1 = pos->x1, py1 = pos->y1, pitch = host.pixelbuf.w;
    s32 cx1 = bmp->x1, cx2 = bmp->x2;
    for (s32 y = bmp->y1; y < bmp->y2; y++) {
        u16 *p = e->data + e->rowoff[y];
        u16 *fbrow = (u16 *)image.logic + (u32)(y + py1) * pitch + px1;
        s32 x = 0;
        while (*p != 0xffff) {
            u16 skip = *p++, cnt = *p++;
            u16 *runpix = p; p += cnt;
            x += skip;
            s32 a = x < cx1 ? cx1 : x, b = (x + cnt) > cx2 ? cx2 : (x + cnt);
            if (b > a) copy_run(fbrow + a, runpix + (a - x), b - a);
            x += cnt;
        }
    }
}
static spc_entry *spc_get(u32 newad, u8 fmt, const u8 *at, u16 w, u16 h)
{
    int rle = (fmt == 0x10 || fmt == 0x14);
    u32 csum = spc_csum(at, spc_srcbytes(fmt, w, h));
    spc_entry *slot = 0;
    spc_entry *stale = 0;
    for (int i = 0; i < SPC_ENTRIES; i++) {
        spc_entry *e = &g_spc[i];
        if (e->data && e->newad == newad && e->w == w && e->h == h && e->is_rle == rle) {
            if (e->palgen == g_pal16_gen && e->csum == csum) { g_spc_hitflag = 1; e->lru = ++g_spc_clock; return e; }
            stale = e;   // matching but stale; free below only if we're allowed to expand
            break;
        }
        if (!e->data && !slot) slot = e;
    }
    // Cap expansion COST per frame: over budget, CPU-blit now (leave stale/missing to a later
    // frame). Always allow at least one (so a sprite bigger than the budget still caches).
    u32 _px = (u32)w * h;
    if (g_spc_exp_frame > 0 && g_spc_exp_frame + _px > SPC_EXP_PX_PER_FRAME) return 0;
    g_spc_exp_frame += _px;
    if (stale) { sys_stram_free(stale->data); g_spc_bytes -= stale->bytes; stale->data = 0; stale->rowoff = 0; slot = stale; }
    u32 nwords = rle ? spc_rle_build(0, 0, fmt, at, w, h) : (u32)w * h;   // RLE pass 1 = count
    u32 bytes = nwords * 2 + (rle ? (u32)h * 4 : 0);                       // data (+ rowoff)
    if (bytes > SPC_BUDGET) return 0;
    while (!slot || g_spc_bytes + bytes > SPC_BUDGET) {
        spc_entry *v = 0;
        for (int i = 0; i < SPC_ENTRIES; i++)
            if (g_spc[i].data && (!v || g_spc[i].lru < v->lru)) v = &g_spc[i];
        if (!v) break;
        sys_stram_free(v->data); g_spc_bytes -= v->bytes; v->data = 0; v->rowoff = 0; slot = v;
    }
    if (!slot) return 0;
    u8 *mem = (u8 *)sys_stram_alloc(bytes);
    if (!mem) return 0;
    g_spc_bytes += bytes;
    slot->data   = (u16 *)mem;
    slot->rowoff = rle ? (u32 *)(mem + nwords * 2) : 0;
    slot->bytes  = bytes; slot->is_rle = rle;
    slot->newad  = newad; slot->w = w; slot->h = h;
    if (rle) spc_rle_build(slot->data, slot->rowoff, fmt, at, w, h);       // pass 2 = fill
    else     spc_expand(slot, fmt, at);
    slot->palgen = g_pal16_gen; slot->csum = csum; slot->lru = ++g_spc_clock;
    g_spc_hitflag = 0;
    return slot;
}
// Called from destofen (bmp is absolute here). Returns 1 if drawn from the cache, 0 to
// fall through to the CPU blitter. Opaque (0x12/0x16) → Blitter block-copy; transparent
// (0x10/0x14) → CPU movem run-copy of the RLE-encoded opaque spans.
#ifndef ALIS_SPRITE_CACHE
#define ALIS_SPRITE_CACHE 1
#endif
int sprite_cache_try_draw(u8 fmt, u8 *at, u32 newad, sRect *pos, sRect *bmp, s32 width, s32 height, s8 flip)
{
#if !ALIS_SPRITE_CACHE
    (void)fmt;(void)at;(void)newad;(void)pos;(void)bmp;(void)width;(void)height;(void)flip; return 0;
#else
    if (g_pal16_fading) return 0;                               // fading: CPU-blit (re-bake live)
    s32 cw = bmp->x2 - bmp->x1, ch = bmp->y2 - bmp->y1;
    if (cw <= 0 || ch <= 0) return 0;
    int opaque = (fmt == 0x12 || fmt == 0x16);
    int rle    = (fmt == 0x10 || fmt == 0x14);
    if (flip || !(opaque || rle) || width <= 0 || height <= 0 || width > 4096 || height > 4096) {
        if (cw * ch >= SPC_MIN_AREA) { g_spc_skip++; g_spc_skipfmt = fmt; }
        return 0;
    }
    if (opaque && cw * ch < SPC_MIN_AREA) return 0;            // small opaque: Blitter setup not worth it
    if (rle && (s32)width * height > SPC_RLE_MAX_AREA) {       // big transparent: stay on the CPU blitter
        g_spc_skip++; g_spc_skipfmt = fmt; return 0;
    }

    spc_entry *e = spc_get(newad, fmt, at, (u16)width, (u16)height);
    if (!e) { g_spc_skip++; return 0; }
    if (g_spc_hitflag) g_spc_hit++; else g_spc_exp++;

    if (e->is_rle) {
        spc_draw_rle(e, pos, bmp);
    } else {
        u16 *src = e->data + (u32)bmp->y1 * (u32)width + bmp->x1;
        u16 *dst = (u16 *)image.logic + (u32)(bmp->y1 + pos->y1) * host.pixelbuf.w + (bmp->x1 + pos->x1);
        sys_blit_copy_16(src, (int)width, dst, (int)cw, (int)ch);
    }
    return 1;
#endif
}
#else
int sprite_cache_try_draw(u8 fmt, u8 *at, u32 newad, sRect *pos, sRect *bmp, s32 width, s32 height, s8 flip)
{ (void)fmt;(void)at;(void)newad;(void)pos;(void)bmp;(void)width;(void)height;(void)flip; return 0; }
#endif

// ---------------------------------------------------------------------------
// 8-bit chunky source
// ---------------------------------------------------------------------------
void draw_8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)   // transparent
{
    // Copy struct fields + pal16 base into locals: otherwise GCC reloads bmp->x2
    // (the loop bound) from memory every pixel, unable to prove the framebuffer
    // write doesn't alias *bmp. Locals stay in registers.
    const s32 x1 = bmp->x1, x2 = bmp->x2, y1 = bmp->y1, y2 = bmp->y2;
    const s32 pitch = host.pixelbuf.w;
    const u16 *restrict p = pal16;
    u8 color;
    u16 wadd = pitch - (x2 - x1);
    u8 *src = (flip ? width - 1 : 0) + at + 2 + y1 * width;
    u16 *tgt = (u16 *)image.logic + (x1 + pos->x1) + (y1 + pos->y1) * pitch;

    if (flip) {
        for (s32 h = y1; h < y2; h++, src += width, tgt += wadd)
            for (s32 w = x1; w < x2; w++, tgt++)
                if ((color = *(src - w)) != 0) *tgt = p[color];
    } else {
        for (s32 h = y1; h < y2; h++, src += width, tgt += wadd)
            for (s32 w = x1; w < x2; w++, tgt++)
                if ((color = *(src + w)) != 0) *tgt = p[color];
    }
}

void draw_8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)   // opaque
{
    const s32 x1 = bmp->x1, x2 = bmp->x2, y1 = bmp->y1, y2 = bmp->y2;
    const s32 pitch = host.pixelbuf.w;
    const u16 *restrict p = pal16;
    u16 wadd = pitch - (x2 - x1);
    u8 *src = (flip ? width - 1 : 0) + at + 2 + y1 * width;
    u16 *tgt = (u16 *)image.logic + (x1 + pos->x1) + (y1 + pos->y1) * pitch;

    if (flip) {
        for (s32 h = y1; h < y2; h++, src += width, tgt += wadd) {
            const u8 *s = src;
            for (s32 w = x1; w < x2; w++) *tgt++ = p[*(s - w)];
        }
    } else {
        for (s32 h = y1; h < y2; h++, src += width, tgt += wadd) {
            const u8 *s = src + x1;
            for (s32 w = x1; w < x2; w++) *tgt++ = p[*s++];
        }
    }
}

// ---------------------------------------------------------------------------
// 4-bit source expanded through pal_offset to an 8-bit index
// ---------------------------------------------------------------------------
void draw_4to8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset)   // transparent
{
    // Locals: bmp->x1/x2 get modified below (odd-column trim), and pulling x2/y2
    // out of *bmp stops GCC reloading the loop bounds through the pointer every
    // pixel. p folds pal_offset into the table base (one add hoisted out). The
    // main loop reads each source byte ONCE and splits both nibbles.
    s32 x1 = bmp->x1, x2 = bmp->x2;
    const s32 y1 = bmp->y1, y2 = bmp->y2;
    const s32 pitch = host.pixelbuf.w;
    const u16 *restrict p = pal16 + pal_offset;
    u8 color;
    u16 swadd = width >> 1;
    u8 *src = at + y1 * swadd;
    u16 *tgt = (u16 *)image.logic + (pos->y1 + y1) * pitch;

    if (flip) {
        if (x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (x1 >> 1) + src;
            u16 *tptr = x1 + pos->x1 + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                if ((color = *sptr >> 4)) *tptr = p[color];
            x1++;
        }
        if ((x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - (x2 >> 1) + src;
            u16 *tptr = (x2 + pos->x1 - 1) + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                if ((color = *sptr & 0x0f)) *tptr = p[color];
            x2--;
        }
        if (x1 != x2) {
            u8 *sbase = swadd - 1 + src;
            u16 *trow = (x1 + pos->x1) + tgt;
            x1 >>= 1; x2 >>= 1;
            for (s32 h = y1; h < y2; h++, sbase += swadd, trow += pitch) {
                const u8 *sp = sbase - x1;
                u16 *tp = trow;
                for (s32 w = x1; w < x2; w++, tp += 2) {
                    u8 b = *sp--;
                    if ((color = b & 0x0f)) tp[0] = p[color];
                    if ((color = b >> 4))   tp[1] = p[color];
                }
            }
        }
    } else {
        if (x1 % 2 == 1) {
            u8 *sptr = (x1 >> 1) + src;
            u16 *tptr = (x1 + pos->x1) + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                if ((color = *sptr & 0x0f)) *tptr = p[color];
            x1++;
        }
        if ((x2 - 1) % 2 == 0) {
            x2--;
            u8 *sptr = (x2 >> 1) + src;
            u16 *tptr = (x2 + pos->x1) + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                if ((color = *sptr >> 4)) *tptr = p[color];
        }
        if (x1 != x2) {
            u8 *sbase = src;
            u16 *trow = (x1 + pos->x1) + tgt;
            x1 >>= 1; x2 >>= 1;
            for (s32 h = y1; h < y2; h++, sbase += swadd, trow += pitch) {
                const u8 *sp = sbase + x1;
                u16 *tp = trow;
                for (s32 w = x1; w < x2; w++, tp += 2) {
                    u8 b = *sp++;
                    if ((color = b >> 4))   tp[0] = p[color];
                    if ((color = b & 0x0f)) tp[1] = p[color];
                }
            }
        }
    }
}

void draw_4to8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset)   // opaque
{
    s32 x1 = bmp->x1, x2 = bmp->x2;
    const s32 y1 = bmp->y1, y2 = bmp->y2;
    const s32 pitch = host.pixelbuf.w;
    const u16 *restrict p = pal16 + pal_offset;
    u16 swadd = width >> 1;
    u8 *src = at + y1 * swadd;
    u16 *tgt = (u16 *)image.logic + (pos->y1 + y1) * pitch;

    if (flip) {
        if (x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (x1 >> 1) + src;
            u16 *tptr = x1 + pos->x1 + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                *tptr = p[*sptr >> 4];
            x1++;
        }
        if ((x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - (x2 >> 1) + src;
            u16 *tptr = (x2 + pos->x1 - 1) + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                *tptr = p[*sptr & 0x0f];
            x2--;
        }
        if (x1 != x2) {
            u8 *sbase = swadd - 1 + src;
            u16 *trow = (x1 + pos->x1) + tgt;
            x1 >>= 1; x2 >>= 1;
            for (s32 h = y1; h < y2; h++, sbase += swadd, trow += pitch) {
                const u8 *sp = sbase - x1;
                u16 *tp = trow;
                for (s32 w = x1; w < x2; w++) {
                    u8 b = *sp--;
                    *tp++ = p[b & 0x0f];
                    *tp++ = p[b >> 4];
                }
            }
        }
    } else {
        if (x1 % 2 == 1) {
            u8 *sptr = (x1 >> 1) + src;
            u16 *tptr = (x1 + pos->x1) + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                *tptr = p[*sptr & 0x0f];
            x1++;
        }
        if ((x2 - 1) % 2 == 0) {
            x2--;
            u8 *sptr = (x2 >> 1) + src;
            u16 *tptr = (x2 + pos->x1) + tgt;
            for (s32 h = y1; h < y2; h++, sptr += swadd, tptr += pitch)
                *tptr = p[*sptr >> 4];
        }
        if (x1 != x2) {
            u8 *sbase = src;
            u16 *trow = (x1 + pos->x1) + tgt;
            x1 >>= 1; x2 >>= 1;
            for (s32 h = y1; h < y2; h++, sbase += swadd, trow += pitch) {
                const u8 *sp = sbase + x1;
                u16 *tp = trow;
                for (s32 w = x1; w < x2; w++) {
                    u8 b = *sp++;
                    *tp++ = p[b >> 4];
                    *tp++ = p[b & 0x0f];
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Window clear / transfer
// ---------------------------------------------------------------------------
void clrfen(void)
{
    s16 fx1 = image.fenx1 < 0 ? 0 : image.fenx1;
    s16 fy1 = image.feny1 < 0 ? 0 : image.feny1;
    s16 fx2 = image.fenx2 >= alis.platform.width  ? alis.platform.width  - 1 : image.fenx2;
    s16 fy2 = image.feny2 >= alis.platform.height ? alis.platform.height - 1 : image.feny2;
    // fen is INCLUSIVE (matches original); width/height are +1.
    s16 tmpx = (fx2 - fx1) + 1;
    s16 hgt  = (fy2 - fy1) + 1;
    if (tmpx <= 0 || hgt <= 0)
        return;
    u16 c0 = pal16[0];
#if ALIS_USE_BLITTER
    // The viewport blank is the single biggest solid fill of the dungeon view —
    // hand it to the Falcon Blitter (bus-rate, no per-pixel CPU) like draw_boxf.
    if ((s32)tmpx * hgt >= BLIT_FILL_MIN_AREA) {
        sys_blit_fill_16((u16 *)image.logic + fx1 + fy1 * host.pixelbuf.w, tmpx, hgt, c0);
        return;
    }
#endif
    for (s16 y = fy1; y <= fy2; y++) {
        u16 *row = (u16 *)image.logic + fx1 + y * host.pixelbuf.w;
        for (s16 x = 0; x < tmpx; x++) row[x] = c0;
    }
}

void trsfen(u8 *src8, u8 *tgt8)
{
    s16 fx1 = image.fenx1, fy1 = image.feny1, fx2 = image.fenx2, fy2 = image.feny2;
    if (fx1 < 0) fx1 = 0;
    if (fy1 < 0) fy1 = 0;
    if (fx2 >= alis.platform.width)  fx2 = alis.platform.width  - 1;
    if (fy2 >= alis.platform.height) fy2 = alis.platform.height - 1;
    if (fx2 < fx1 || fy2 < fy1)
        return;

    u16 *src = (u16 *)src8 + fx1 + fy1 * alis.platform.width;
    u16 *tgt = (u16 *)tgt8 + fx1 + fy1 * alis.platform.width;
    s16 skip = alis.platform.width - (fx2 - fx1 + 1);
    for (s32 y = fy1; y <= fy2; y++, tgt += skip, src += skip)
        for (s32 x = fx1; x <= fx2; x++)
            *tgt++ = *src++;
}

// ---------------------------------------------------------------------------
// 4-bit ST sprites (no pal_offset — nibble is the index directly)
// ---------------------------------------------------------------------------
void draw_st_4bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)   // transparent
{
    u8 color;
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u16 *tgt = (u16 *)image.logic + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (bmp->x1 >> 1) + src;
            u16 *tptr = bmp->x1 + pos->x1 + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                if ((color = *sptr >> 4)) *tptr = PX16(color);
            bmp->x1++;
        }
        if ((bmp->x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - ((bmp->x2) >> 1) + src;
            u16 *tptr = (bmp->x2 + pos->x1 - 1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                if ((color = *sptr & 0x0f)) *tptr = PX16(color);
            bmp->x2--;
        }
        if (bmp->x1 != bmp->x2) {
            u8 *sptr = swadd - 1 + src;
            u16 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);
            bmp->x1 >>= 1; bmp->x2 >>= 1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd)
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr += 2) {
                    if ((color = sptr[-w] & 0x0f)) tptr[0] = PX16(color);
                    if ((color = sptr[-w] >> 4))   tptr[1] = PX16(color);
                }
        }
    } else {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = (bmp->x1 >> 1) + src;
            u16 *tptr = (bmp->x1 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                if ((color = *sptr & 0x0f)) *tptr = PX16(color);
            bmp->x1++;
        }
        if ((bmp->x2 - 1) % 2 == 0) {
            bmp->x2--;
            u8 *sptr = (bmp->x2 >> 1) + src;
            u16 *tptr = (bmp->x2 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                if ((color = *sptr >> 4)) *tptr = PX16(color);
        }
        if (bmp->x1 != bmp->x2) {
            u8 *sptr = src;
            u16 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);
            bmp->x1 >>= 1; bmp->x2 >>= 1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd)
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr += 2) {
                    if ((color = sptr[w] >> 4))   tptr[0] = PX16(color);
                    if ((color = sptr[w] & 0x0f)) tptr[1] = PX16(color);
                }
        }
    }
}

void draw_st_4bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)   // opaque
{
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u16 *tgt = (u16 *)image.logic + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (bmp->x1 >> 1) + src;
            u16 *tptr = bmp->x1 + pos->x1 + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                *tptr = PX16(*sptr >> 4);
            bmp->x1++;
        }
        if ((bmp->x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - ((bmp->x2) >> 1) + src;
            u16 *tptr = (bmp->x2 + pos->x1 - 1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                *tptr = PX16(*sptr & 0x0f);
            bmp->x2--;
        }
        if (bmp->x1 != bmp->x2) {
            u8 *sptr = swadd - 1 + src;
            u16 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);
            bmp->x1 >>= 1; bmp->x2 >>= 1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd)
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr += 2) {
                    tptr[0] = PX16(sptr[-w] & 0x0f);
                    tptr[1] = PX16(sptr[-w] >> 4);
                }
        }
    } else {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = (bmp->x1 >> 1) + src;
            u16 *tptr = (bmp->x1 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                *tptr = PX16(*sptr & 0x0f);
            bmp->x1++;
        }
        if ((bmp->x2 - 1) % 2 == 0) {
            bmp->x2--;
            u8 *sptr = (bmp->x2 >> 1) + src;
            u16 *tptr = (bmp->x2 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w)
                *tptr = PX16(*sptr >> 4);
        }
        if (bmp->x1 != bmp->x2) {
            u8 *sptr = src;
            u16 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);
            bmp->x1 >>= 1; bmp->x2 >>= 1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd)
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr += 2) {
                    tptr[0] = PX16(sptr[w] >> 4);
                    tptr[1] = PX16(sptr[w] & 0x0f);
                }
        }
    }
}

// ---------------------------------------------------------------------------
// Primitives
// ---------------------------------------------------------------------------
void draw_pixel(s16 x0, s16 y0)
{
    ((u16 *)image.logic)[x0 + y0 * alis.platform.width] = pal16[image.inkcolor];
}

void draw_line(s16 x0, s16 y0, s16 x1, s16 y1)   // identical to chunky: just calls draw_pixel
{
    s32 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    s32 dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    s32 error = dx + dy;
    while (1) {
        draw_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        s32 e2 = 2 * error;
        if (e2 >= dy) { if (x0 == x1) break; error += dy; x0 += sx; }
        if (e2 <= dx) { if (y0 == y1) break; error += dx; y0 += sy; }
    }
}

void draw_boxf(s16 x1, s16 y1, s16 x2, s16 y2)
{
    if (alis.platform.kind == EPlatformMac) { mac_update_pos(&x1, &y1); mac_update_pos(&x2, &y2); }
    s16 t = min(x1, x2); x2 = max(x1, x2); x1 = t;
    if (!boxf_clip(&x1, &y1, &x2, &y2)) return;
    s16 tmpx = x2 - x1;
    if (dirty_len < 0xfe) dirty_len++;   // native uses dirty_len only as a flip gate
    u16 c = pal16[image.inkcolor];
    s16 hgt = y2 - y1 + 1;
#if ALIS_USE_BLITTER
    if ((s32)tmpx * hgt >= BLIT_FILL_MIN_AREA) {
        sys_blit_fill_16((u16 *)image.logic + x1 + y1 * host.pixelbuf.w, tmpx, hgt, c);
        return;
    }
#endif
    for (s16 y = y1; y <= y2; y++) {
        u16 *row = (u16 *)image.logic + x1 + y * alis.platform.width;
        for (s16 w = 0; w < tmpx; w++) row[w] = c;
    }
}

void draw_rect(sRect *pos, sRect *bmp, u8 color)
{
    u16 *tgt = (u16 *)image.logic + pos->x1 + ((bmp->y1 + pos->y1) * host.pixelbuf.w);
    u16 c = pal16[color];
    s16 w16 = bmp->x2 - bmp->x1, h16 = bmp->y2 - bmp->y1;
#if ALIS_USE_BLITTER
    if ((s32)w16 * h16 >= BLIT_FILL_MIN_AREA) {
        sys_blit_fill_16(tgt + bmp->x1, w16, h16, c);
        return;
    }
#endif
    for (s32 h = bmp->y1; h < bmp->y2; h++, tgt += host.pixelbuf.w)
        for (s32 w = bmp->x1; w < bmp->x2; w++) tgt[w] = c;
}

void draw_mac_rect(sRect *pos, sRect *bmp, u8 color)
{
    u16 *tgt = (u16 *)image.logic + pos->x1 + ((bmp->y1 + pos->y1) * host.pixelbuf.w);
    if (color == 15) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, tgt += host.pixelbuf.w)
            for (s32 w = bmp->x1; w < bmp->x2; w++) tgt[w] = pal16[(w + h) % 2];
    } else {
        u16 c = pal16[!color];
        s16 w16 = bmp->x2 - bmp->x1, h16 = bmp->y2 - bmp->y1;
#if ALIS_USE_BLITTER
        if ((s32)w16 * h16 >= BLIT_FILL_MIN_AREA) {
            sys_blit_fill_16(tgt + bmp->x1, w16, h16, c);
            return;
        }
#endif
        for (s32 h = bmp->y1; h < bmp->y2; h++, tgt += host.pixelbuf.w)
            for (s32 w = bmp->x1; w < bmp->x2; w++) tgt[w] = c;
    }
}

// ---------------------------------------------------------------------------
// Not yet ported (not used by Atari 4-bit games).
// ---------------------------------------------------------------------------
void draw_mac_mono_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip) { (void)at; (void)pos; (void)bmp; (void)width; (void)flip; }
void draw_mac_mono_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip) { (void)at; (void)pos; (void)bmp; (void)width; (void)flip; }
void draw_dos_cga_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip) { (void)at; (void)pos; (void)bmp; (void)width; (void)flip; }
void draw_dos_cga_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip) { (void)at; (void)pos; (void)bmp; (void)width; (void)flip; }
void draw_ami_5bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip) { (void)at; (void)pos; (void)bmp; (void)width; (void)height; (void)flip; }
void draw_ami_5bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip) { (void)at; (void)pos; (void)bmp; (void)width; (void)height; (void)flip; }

#endif // ALIS_NATIVE_16BPP
