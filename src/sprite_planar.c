//
// 8-bit planar + Blitter render path for sprite games (Ishar etc.).
//
// Sprites are converted once to planar colour indices (palette-independent); the hardware
// CLUT applies the palette at scan-out, so fades are free.
//
// This file is Atari-native only. The 16bpp path (image_draw_16.c) stays for the
// true-3D game (Robinson's Requiem).
//

#include "config.h"

#if defined(ALIS_NATIVE_PLANAR)

#include "alis.h"
#include "mem.h"
#include "image.h"
#include "sys/sys.h"
#include "image_atari.h"
#include <stdlib.h>

extern int  g_planar_nplanes;   // 4/6/8 — set at load from the deepest colour index (convert_sprites_inplace)

// ---------------------------------------------------------------------------
// Flipped-sprite cache (open addressing): bitmap address -> lazily-built mirrored copy.
// ---------------------------------------------------------------------------
#define PLT_SIZE 1024                              // power of two; a full table flushes
typedef struct { u32 key; u16 *flip; } plt_entry;
static plt_entry g_plt[PLT_SIZE];

// Draw-path profile counters (dumped periodically by planar_dump_profile in image_draw_planar.c).
u32 g_prof_destofen_total, g_prof_destofen_transp, g_prof_destofen_op_shift, g_prof_destofen_op_align, g_prof_destofen_blit, g_prof_destofen_blit_area;
u32 g_prof_destofen_skip_flip, g_prof_destofen_skip_miss, g_prof_destofen_skip_fmt;   // why a sprite fell to the chunky path
u32 g_prof_destofen_tr_align, g_prof_destofen_tr_shift, g_prof_destofen_tr_align_area, g_prof_destofen_tr_shift_area;   // transparent split
u32 g_prof_destofen_cfb, g_prof_destofen_cfb_area;   // transparent X-clipped sprites (edge path)
u32 g_prof_destofen_op_calls, g_prof_destofen_op_area;   // opaque sprites on the edge path

static u32 plt_hash(u32 k) { return ((k >> 2) * 2654435761u) & (PLT_SIZE - 1); }

// chunky{8,4}_to_planar live in opcodes.c (next to conv_finish) so they also compile in the
// desktop ALIS_PLANAR_CONV build; the flip rebuild below uses them on Atari.
extern void chunky8_to_planar(const u8 *idx8, s32 w, s32 h, int flip, u8 *dst);
extern void chunky4_to_planar(const u8 *idx8, s32 w, s32 h, int flip, u8 *dst);

// 4-plane interleaved -> chunky indices 0..15 (inverse of chunky4_to_planar, no flip).
void planar4_to_chunky8(const u8 *planar, s32 w, s32 h, u8 *out8)
{
    s32 cpr = (w + 15) >> 4;
    for (s32 y = 0; y < h; y++) {
        for (s32 x = 0; x < w; x++) {
            const u16 *pw = (const u16 *)(planar + ((s32)y * cpr + (x >> 4)) * 8);  // 4 words/chunk
            int i = x & 15; u8 idx = 0;
            for (int k = 0; k < 4; k++)
                idx |= (u8)(((pw[k] >> (15 - i)) & 1) << k);
            out8[(s32)y * w + x] = idx;
        }
    }
}

// 8-plane interleaved -> 8-bit chunky indices (inverse of chunky8_to_planar, no flip).
void planar8_to_chunky8(const u8 *planar, s32 w, s32 h, u8 *out8)
{
    s32 cpr = (w + 15) >> 4;
    for (s32 y = 0; y < h; y++) {
        for (s32 x = 0; x < w; x++) {
            const u16 *pw = (const u16 *)(planar + ((s32)y * cpr + (x >> 4)) * 16);
            int i = x & 15; u8 idx = 0;
            for (int k = 0; k < 8; k++)
                idx |= (u8)(((pw[k] >> (15 - i)) & 1) << k);
            out8[(s32)y * w + x] = idx;
        }
    }
}

// Free every cached buffer and reset the table on overflow (entries rebuild lazily).
// Whole-table reset avoids open-addressing tombstones.
void planar_tab_flush(void)
{
    for (int i = 0; i < PLT_SIZE; i++) {
        free(g_plt[i].flip);
        g_plt[i].key = 0; g_plt[i].flip = 0;
    }
}

// Flipped sprites: lazily build + cache a mirrored copy from the in-place planar data
// (decode to 8-bit, mirror, re-encode).
u16 *planar_tab_get_flip(const u8 *bitmap)
{
    u32 key = (u32)bitmap, h = plt_hash(key);
    plt_entry *slot = 0;
    for (int i = 0; i < PLT_SIZE; i++) {
        plt_entry *e = &g_plt[(h + i) & (PLT_SIZE - 1)];
        if (e->key == key) return e->flip;          // already built (may be NULL on OOM)
        if (e->key == 0) { slot = e; break; }
    }
    if (!slot) { planar_tab_flush(); slot = &g_plt[h]; }

    s32 w  = read16((u8 *)bitmap + 2) + 1;
    s32 hh = read16((u8 *)bitmap + 4) + 1;
    s32 cpr = (w + 15) >> 4;
    int sp = (bitmap[6] == 4) ? 4 : 8;             // source plane count (set at conversion)
    u8  *tmp8 = (u8 *)malloc((u32)w * hh);
    u16 *flip = (u16 *)malloc((u32)hh * cpr * (sp * 2));  // 4- or 8-plane flip buffer
    if (tmp8 && flip) {
        if (sp == 4) { planar4_to_chunky8((u8 *)bitmap + 8, w, hh, tmp8);
                       chunky4_to_planar(tmp8, w, hh, 1, (u8 *)flip); }
        else         { planar8_to_chunky8((u8 *)bitmap + 8, w, hh, tmp8);
                       chunky8_to_planar(tmp8, w, hh, 1, (u8 *)flip); }
    } else { free(flip); flip = 0; }
    free(tmp8);
    slot->key = key; slot->flip = flip;
    return flip;
}
// One pixel-clipped boundary chunk via the asm edge blitter. prev/cur are source chunks c-1/c
// (or a zero chunk with stride 0 at a leading/spill boundary).
static void destofen_edge_chunk(const u16 *data, s32 spc, s32 sc, s32 start_sc, s32 s,
                            s32 br0, s32 nln, s32 py, s32 fbw, u16 *fb, s32 fbstride,
                            int nplanes, u16 cm, int opaque)
{
    static const u16 zc[8] = {0,0,0,0,0,0,0,0};
    s32 c   = sc - start_sc;
    s32 srcw = (nplanes == 4) ? 4 : 8;           // source plane-words/chunk (16-colour → 4)
    int pvz = !(c >= 1 && c - 1 < spc);          // prev is the zero chunk?
    int cuz = !(c >= 0 && c < spc);              // cur is the zero chunk?
    destofen_edge_blk P;
    P.nlines      = (u16)(nln - 1);
    P.dst_stride  = (u16)fbw;                    // fb row = fbw bytes (8-plane)
    P.prev_stride = (u16)(pvz ? 0 : spc * srcw * 2);
    P.cur_stride  = (u16)(cuz ? 0 : spc * srcw * 2);
    P.mask        = cm;
    P.shift       = (u16)s;
    P.nplanes    = (u16)nplanes;
    const u16 *prev = pvz ? zc : data + (u32)br0 * spc * srcw + (u32)(c - 1) * srcw;
    const u16 *cur  = cuz ? zc : data + (u32)br0 * spc * srcw + (u32)c * srcw;
    u16 *dst = fb + (u32)(py + br0) * fbstride + (u32)sc * 8;
    if (opaque) destofen_edge_opaque_asm(&P, prev, cur, dst);
    else        destofen_edge_asm(&P, prev, cur, dst);
}

// ---------------------------------------------------------------------------
// Planar sprite draw into the 8-plane interleaved framebuffer, dispatching to the asm
// blitters (ports of the original destofen). Transparency = index 0 = all planes zero.
void destofen_planar(const u8 *bitmap, u16 *data, sRect *pos, sRect *bmp)
{
    int  transp = !(bitmap[0] & 2);                // 0x10/0x14 transparent, 0x12/0x16 opaque
    s32  sw  = read16((u8 *)(bitmap + 2)) + 1;
    s32  sh  = read16((u8 *)(bitmap + 4)) + 1;
    s32  spc = (sw + 15) >> 4;                      // sprite chunks per row (padded to 16px)
    s32  fbw  = host.pixelbuf.w, fbh = host.pixelbuf.h;
    s32  fbstride = fbw >> 1;                       // framebuffer words/row (8-plane interleaved)
    s32  fbchunks = fbw >> 4;
    // image.wdraw: the screen, or the background-cache shadow (same layout) while capturing.
    u16 *fb   = (u16 *)image.wdraw;

    // Per-sprite plane count (bitmap[6]), not g_planar_nplanes, which can grow after conversion.
    int  spw  = (bitmap[6] == 4) ? 4 : 8;           // this sprite's stored plane count
    int  p4   = (spw == 4);
    s32  srcw = spw;                                 // source plane-words per chunk

    s32 px = pos->x1, py = pos->y1;
    s32 s        = ((px & 15) + 16) & 15;          // shift 0..15 (handles negative px)
    s32 start_sc = (px - s) >> 4;                  // first dest chunk: px == start_sc*16 + s

    s32 vlo = image.clipx1 >> 4, vhi = image.clipx2 >> 4;   // visible chunk range (clamped)
    if (vlo < 0) vlo = 0;
    if (vhi >= fbchunks) vhi = fbchunks - 1;

    // Clamp rows to the resource height (sprite->height can exceed it).
    s32 r0 = bmp->y1 < 0 ? 0 : bmp->y1;
    s32 r1 = bmp->y2 > sh ? sh : bmp->y2;

    // Pixel-exact X clip: the engine restores exactly [px+bmp.x1, px+bmp.x2), so a
    // chunk-granular clip would leave stray pixels.
    s32 xclip = (bmp->x1 != 0) || (bmp->x2 != sw);
    s32 cx_lo = px + bmp->x1, cx_hi = px + bmp->x2;   // dest pixel range [cx_lo, cx_hi)
    if (cx_lo < 0)   cx_lo = 0;
    if (cx_hi > fbw) cx_hi = fbw;
    // Clipped edges on 16px boundaries (calcfen windows): full-chunk blit is exact.
    s32 clip_aligned = ((bmp->x1 == 0) || ((cx_lo & 15) == 0))
                    && ((bmp->x2 == sw) || ((cx_hi & 15) == 0));

    // Profile: categorize every call (+ transparent area).
    g_prof_destofen_total++;
    if (transp) {
        g_prof_destofen_transp++;
        s32 _a = (s32)spc * (r1 > r0 ? r1 - r0 : 0);   // ~chunks drawn
        if (s == 0) { g_prof_destofen_tr_align++; g_prof_destofen_tr_align_area += _a; }
        else        { g_prof_destofen_tr_shift++; g_prof_destofen_tr_shift_area += _a; }
    }
    else if (s != 0)   g_prof_destofen_op_shift++;
    else               g_prof_destofen_op_align++;

    // Opaque, word-aligned, unclipped 8-plane sprite: Blitter copy (small ones don't pay off).
    if (!transp && s == 0 && !xclip && !p4) {      // 4-plane → 4bpp CPU path (no clean 4→8 Blitter copy)
        s32 c0 = start_sc < vlo ? vlo : start_sc;                       // first visible chunk
        s32 c1 = (start_sc + spc - 1) < vhi ? (start_sc + spc - 1) : vhi;
        s32 br0 = r0, br1 = r1;
        if (py + br0 < 0)   br0 = -py;                                  // clamp rows to [0, fbh)
        if (py + br1 > fbh) br1 = fbh - py;
        s32 bcw = c1 - c0 + 1, bh = br1 - br0;
        s32 sc_off = c0 - start_sc;                                    // src chunk offset (>= 0)
        u16 *bsrc = data + (u32)br0 * spc * 8 + (u32)sc_off * 8;
        // Blitter has 24-bit addresses (ST-RAM only): not for TT-RAM sources, nor while
        // drawing into the fast-RAM shadow (fb != image.logic).
        if (g_hw_blitter && fb == (u16 *)image.logic
            && bcw > 0 && bh > 0 && bcw * bh >= 16 && (u32)bsrc < 0x01000000u) {
            u16 *bdst = fb   + (u32)(py + br0) * fbstride + (u32)c0 * 8;
            sys_blit_copy_planar(bsrc, spc * 16, bdst, fbw, bcw, bh, 8);
            g_prof_destofen_blit++; g_prof_destofen_blit_area += (u32)bcw * bh;
            return;
        }
    }

    // prev<<16|cur >> s, correct for s==0..15.
#define PD_SHIFT(prev, cur)  ((u16)(((u32)(prev) << 16 | (cur)) >> s))

    // Unshifted transparent: original aligned blitter.
    if (transp && s == 0 && (!xclip || clip_aligned)) {
        s32 c0 = start_sc < vlo ? vlo : start_sc;
        s32 c1 = (start_sc + spc - 1) < vhi ? (start_sc + spc - 1) : vhi;
        s32 br0 = r0, br1 = r1;
        if (py + br0 < 0)   br0 = -py;
        if (py + br1 > fbh) br1 = fbh - py;
        s32 nch = c1 - c0 + 1, nln = br1 - br0;
        if (nch > 0 && nln > 0) {
            destofen_blk P;
            P.nchunks    = (u16)nch;
            P.nlines     = (u16)(nln - 1);
            P.dst_rowrem = (u16)(fbw - nch * 16);          // bytes to next row, same column
            P.src_hskip  = (u16)(spc - nch);               // clipped-off chunks per row
            P.src_start  = 0;                              // src pre-offset below
            P.nplanes    = (u16)spw;
            const u16 *bsrc = data + (u32)br0 * spc * srcw + (u32)(c0 - start_sc) * srcw;
            u16 *bdst = fb + (u32)(py + br0) * fbstride + (u32)c0 * 8;
            if (p4) destofen_4bpp_aligned_asm(&P, bsrc, bdst);
            else    destofen_8bpp_aligned_asm(&P, bsrc, bdst);
        }
        return;
    }

    // Shifted transparent: original shifted blitter. Dest chunks: leading=start_sc, middle,
    // trailing=start_sc+spc; leftclip/rightclip drop the leading/trailing side outside [vlo,vhi].
    if (transp && !xclip) {
        s32 lead = start_sc, trail = start_sc + spc;
        s32 d0 = lead < vlo ? vlo : lead;      // first visible dest chunk
        s32 d1 = trail > vhi ? vhi : trail;    // last visible dest chunk
        if (d1 < d0) return;                   // fully clipped horizontally
        int leftclip  = (d0 > lead);
        int rightclip = (d1 < trail);
        s32 first_mid = leftclip  ? d0 : (lead + 1);
        s32 last_mid  = rightclip ? d1 : (trail - 1);
        s32 mid_count = last_mid - first_mid + 1; if (mid_count < 0) mid_count = 0;
        s32 src_start = leftclip ? (d0 - lead - 1) : 0;
        s32 nchunks   = leftclip ? mid_count : (mid_count + 1);
        s32 br0 = r0, br1 = r1;
        if (py + br0 < 0)   br0 = -py;
        if (py + br1 > fbh) br1 = fbh - py;
        s32 nln = br1 - br0;
        if (nln > 0 && nchunks > 0) {
            destofen_blk P;
            P.nchunks    = (u16)nchunks;
            P.nlines     = (u16)(nln - 1);
            P.dst_rowrem = (u16)(fbw - nchunks * 16);
            // A2 walks src_start+1 + mid_count chunks per row, so this returns it to the next
            // row's src_start (the src_start term cancels).
            P.src_hskip  = (u16)(spc - 1 - mid_count);
            P.src_start  = (u16)src_start;
            P.leftfrac   = (u16)(0xffffu >> s);            // leading end-mask (used iff !leftclip)
            P.rightfrac  = (u16)((~(0xffffu >> s)) & 0xffffu); // trailing end-mask (used iff !rightclip)
            P.shift      = (u16)s;
            P.leftclip   = leftclip  ? 0xff : 0;
            P.rightclip  = rightclip ? 0xff : 0;
            P.nplanes    = (u16)spw;
            const u16 *bsrc = data + (u32)br0 * spc * srcw;   // row base; asm adds src_start*chunk
            u16 *bdst = fb + (u32)(py + br0) * fbstride + (u32)d0 * 8;
            if (p4) destofen_4bpp_shifted_asm(&P, bsrc, bdst);
            else    destofen_8bpp_shifted_asm(&P, bsrc, bdst);
        }
        return;
    }

    if (transp) {
        g_prof_destofen_cfb++; g_prof_destofen_cfb_area += (u32)spc * (r1 > r0 ? r1 - r0 : 0);
        // Pixel-exact clip: interior chunks [bd0+1, bd1-1] via the aligned/shifted blitter, the
        // boundary chunks bd0/bd1 via destofen_edge_chunk with the clip mask (the original skips them).
        s32 bd0 = cx_lo >> 4, bd1 = (cx_hi - 1) >> 4;     // first / last visible chunks
        s32 br0 = r0, br1 = r1;                           // row clamp (shared by all chunks)
        if (py + br0 < 0)   br0 = -py;
        if (py + br1 > fbh) br1 = fbh - py;
        s32 nln = br1 - br0;
        if (nln > 0 && bd1 >= bd0) {
            u16 leftmask  = 0xffff, rightmask = 0xffff;
            if ((bd0 << 4) < cx_lo)      leftmask  = (u16)(0xffffu >> (cx_lo - (bd0 << 4)));
            if ((bd1 << 4) + 16 > cx_hi) rightmask = (u16)((0xffffu << ((bd1 << 4) + 16 - cx_hi)) & 0xffffu);
            if (bd0 == bd1) {
                destofen_edge_chunk(data, spc, bd0, start_sc, s, br0, nln, py, fbw, fb, fbstride,
                                spw, (u16)(leftmask & rightmask), 0);
            } else {
                destofen_edge_chunk(data, spc, bd0, start_sc, s, br0, nln, py, fbw, fb, fbstride,
                                spw, leftmask, 0);
                if (bd1 > bd0 + 1) {                         // interior full chunks via the blitter
                    s32 ic = bd1 - bd0 - 1;
                    destofen_blk P;
                    P.nchunks    = (u16)ic;
                    P.nlines     = (u16)(nln - 1);
                    P.dst_rowrem = (u16)(fbw - ic * 16);
                    P.nplanes    = (u16)spw;
                    u16 *ibdst = fb + (u32)(py + br0) * fbstride + (u32)(bd0 + 1) * 8;
                    if (s == 0) {                            // unshifted interior copy
                        P.src_hskip = (u16)(spc - ic);
                        P.src_start = 0;
                        const u16 *ibsrc = data + (u32)br0 * spc * srcw + (u32)(bd0 + 1 - start_sc) * srcw;
                        if (p4) destofen_4bpp_aligned_asm(&P, ibsrc, ibdst);
                        else    destofen_8bpp_aligned_asm(&P, ibsrc, ibdst);
                    } else {                                 // shifted interior (middle-only)
                        P.src_hskip = (u16)(spc - 1 - ic);
                        P.src_start = (u16)(bd0 - start_sc);
                        P.leftfrac  = 0; P.rightfrac = 0;
                        P.shift     = (u16)s;
                        P.leftclip  = 0xff; P.rightclip = 0xff;
                        const u16 *ibsrc = data + (u32)br0 * spc * srcw;
                        if (p4) destofen_4bpp_shifted_asm(&P, ibsrc, ibdst);
                        else    destofen_8bpp_shifted_asm(&P, ibsrc, ibdst);
                    }
                }
                destofen_edge_chunk(data, spc, bd1, start_sc, s, br0, nln, py, fbw, fb, fbstride,
                                spw, rightmask, 0);
            }
        }
    } else {
        // Opaque aligned: width a multiple of 16 (no padding chunk drawn black) and inside the viewport.
        if (s == 0 && (sw & 15) == 0 && !xclip
            && start_sc >= vlo && (start_sc + spc - 1) <= vhi) {
            s32 br0 = r0, br1 = r1;
            if (py + br0 < 0)   br0 = -py;
            if (py + br1 > fbh) br1 = fbh - py;
            s32 nln = br1 - br0;
            if (nln > 0) {
                destofen_blk P;
                P.nchunks    = (u16)spc;
                P.nlines     = (u16)(nln - 1);
                P.dst_rowrem = (u16)(fbw - spc * 16);
                P.src_hskip  = 0;
                P.src_start  = 0;
                P.nplanes    = (u16)spw;
                const u16 *bsrc = data + (u32)br0 * spc * srcw;
                u16 *bdst = fb + (u32)(py + br0) * fbstride + (u32)start_sc * 8;
                if (p4) destofen_4bpp_opaque_aligned_asm(&P, bsrc, bdst);
                else    destofen_8bpp_opaque_aligned_asm(&P, bsrc, bdst);
                return;
            }
        }
        // Opaque shifted: same gate, plus the spill chunk start_sc+spc must be visible.
        if (s != 0 && (sw & 15) == 0 && !xclip
            && start_sc >= vlo && (start_sc + spc) <= vhi) {
            s32 br0 = r0, br1 = r1;
            if (py + br0 < 0)   br0 = -py;
            if (py + br1 > fbh) br1 = fbh - py;
            s32 nln = br1 - br0;
            if (nln > 0) {
                destofen_blk P;
                P.nchunks    = (u16)(spc - 1);             // middle chunk count
                P.nlines     = (u16)(nln - 1);
                P.dst_rowrem = (u16)(fbw - (spc + 1) * 16); // leading + (spc-1) middle + trailing
                P.src_hskip  = 0;
                P.src_start  = 0;
                P.shift      = (u16)s;
                P.leftfrac   = (u16)(0xffffu >> s);              // leading: sprite cols s..15
                P.rightfrac  = (u16)((~(0xffffu >> s)) & 0xffff);// trailing: spill cols 0..s-1
                P.nplanes    = (u16)spw;
                const u16 *bsrc = data + (u32)br0 * spc * srcw;
                u16 *bdst = fb + (u32)(py + br0) * fbstride + (u32)start_sc * 8;
                if (p4) destofen_4bpp_opaque_shifted_asm(&P, bsrc, bdst);
                else    destofen_8bpp_opaque_shifted_asm(&P, bsrc, bdst);
                return;
            }
        }
        // Opaque odd-width / clipped: every visible chunk via the opaque edge blitter
        // (the mask trims the padding past the edge).
        s32 bd0 = cx_lo >> 4, bd1 = (cx_hi - 1) >> 4;
        s32 br0 = r0, br1 = r1;
        if (py + br0 < 0)   br0 = -py;
        if (py + br1 > fbh) br1 = fbh - py;
        s32 nln = br1 - br0;
        g_prof_destofen_op_calls++; g_prof_destofen_op_area += (u32)(bd1 - bd0 + 1) * (nln > 0 ? nln : 0);
        if (nln > 0) {
            for (s32 sc = bd0; sc <= bd1; sc++) {
                s32 cpx = sc << 4;
                u16 cm = 0xffff;
                if (cpx < cx_lo)      { s32 dd = cx_lo - cpx;      if (dd >= 16) continue; cm  = (u16)(0xffffu >> dd); }
                if (cpx + 16 > cx_hi) { s32 dd = cpx + 16 - cx_hi; if (dd >= 16) continue; cm &= (u16)((0xffffu << dd) & 0xffffu); }
                destofen_edge_chunk(data, spc, sc, start_sc, s, br0, nln, py, fbw, fb, fbstride,
                                spw, cm, 1);
            }
        }
    }
#undef PD_SHIFT
}

#endif // ALIS_NATIVE_PLANAR
