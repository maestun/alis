//
//  render3d_dos.c
//  alis
//
//  DOS-specific terrain rendering functions extracted from render3d.c
//

#include "alis.h"
#include "image.h"
#include "mem.h"
#include "render3d.h"
#include "render3d_dos.h"

// Shared globals defined in render3d.c
extern u8 fprectop;
extern u8 fprectopa;
extern u8 fbottom;
extern u8 notopa;
extern u16 prectopa;
extern u16 precbota;
extern u16 prectopb;
extern u16 precbotb;
extern u16 prectopc;
extern u16 precbotc;
extern u16 prectopi;
extern u16 precboti;
extern u16 botalt;
extern u16 bothigh;
extern u16 solha;
extern u32 adresa;
extern u8 terrain_fill_color;
extern s16 bartra_saved_si;
extern s16 bottom_type_index;

void vgatofen_dos(void)
{
    image.switchgo = 1;
    u8 *src = image.wlogic + (image.clipx1 + (image.clipy1 - image.wlogy1) * image.wloglarg);
    
    image.vgamodulo = image.wloglarg - ((image.clipx2 - image.clipx1) + 1);
    image.bitmodulo = image.loglarg * 2 - ((image.clipx2 - image.clipx1) + 1);
    
    u8 *ptr = image.logic;
    u8 *tgt = ptr + image.clipx1 + image.clipy1 * image.loglarg * 2;
    for (int y = image.clipy1; y <= image.clipy2; y++, tgt+=image.bitmodulo, src+=image.vgamodulo) {
        for (int x = image.clipx1; x <= image.clipx2; x++, tgt++, src++) {
            *tgt = *src;
        }
    }
}

void clrvga_dos(void)
{
    // DOS: fill with 0 using 4-byte writes, width = (fenx2-fenx1)+1
    u16 width = (image.fenx2 - image.fenx1) + 1;
    if (image.fenx1 + width > image.wloglarg)
        width = image.wloglarg - image.fenx1;
    s16 rows = image.feny2 - image.feny1;
    u16 w4 = width >> 2;
    u16 modulo = image.wloglarg - (w4 << 2);
    u32 *ptr = (u32 *)(image.wlogic + image.fenx1 + image.feny1 * image.wloglarg);
    for (s16 y = rows; y >= 0; y--)
    {
        for (u16 x = 0; x < w4; x++)
        {
            *ptr++ = 0;
        }
        ptr = (u32 *)((u8 *)ptr + modulo);
    }
}

void calclan0_dos(s32 scene_addr, s32 render_context)
{
    u32 grid_shift = (u32)(u16)xread16(render_context - 0x3c0);

    // Compute signed axis deltas and their absolute values
    image.signedx = xread16(render_context - 0x378) - xread16(render_context - 0x37e);
    s16 abs_dx = image.signedx;
    if (image.signedx < 0)
    {
        abs_dx = -image.signedx;
    }

    image.signedy = xread16(render_context - 0x376) - xread16(render_context - 0x37c);
    s16 abs_dy = image.signedy;
    if (image.signedy < 0)
    {
        abs_dy = -image.signedy;
    }

    // Determine dominant axis and compute fixed-point stepping
    if (abs_dx < abs_dy)
    {
        // Y-dominant: step along Y, interpolate X
        xwrite16(render_context - 0x2b0, abs_dy);
        u16 minor_span = xread16(render_context - 0x36a) - xread16(render_context - 0x370);
        if ((int)((u32)minor_span << 0x10) < 0) {
            minor_span = -minor_span;
        }

        xwrite16(render_context - 0x2ac, minor_span);
        s16 step_num = xread16(render_context - 0x378) - xread16(render_context - 0x37e);
        s16 frac = (s16)(((int)((u32)(u16)(step_num % abs_dy) << 0x10) >> 1) / (int)abs_dy);
        xwrite32(render_context - 0x2c4, (u32)(u16)(step_num / abs_dy) * 0x10000 + frac * 2);
        s32 dir_sign = 0x10000;
        if (image.signedy < 0)
        {
            dir_sign = 0xffff0000;
        }

        xwrite32(render_context - 0x2c0, dir_sign);
        s16 cross_corr = (s16)((int)(s16)(xread16(render_context - 0x37c) - xread16(render_context - 0x370)) * (int)(s16)(xread32(render_context - 0x2c4) >> 2) >> 0xe);
        if (xread32(render_context - 0x2c0) < 0)
        {
            cross_corr = -cross_corr;
        }

        u16 strip_span = (xread16(render_context - 0x372) + cross_corr) - xread16(render_context - 0x37e);
        if ((int)((u32)strip_span << 0x10) < 0)
        {
            strip_span = -strip_span;
        }
        xwrite16(render_context - 0x24e, (s16)strip_span >> (grid_shift & 0x3f));
    }
    else
    {
        // X-dominant: step along X, interpolate Y
        xwrite16(render_context - 0x2b0, abs_dx);
        u16 minor_span = xread16(render_context - 0x36c) - xread16(render_context - 0x372);
        if ((int)((u32)minor_span << 0x10) < 0)
        {
            minor_span = -minor_span;
        }

        xwrite16(render_context - 0x2ac, minor_span);
        s16 step_num = xread16(render_context - 0x376) - xread16(render_context - 0x37c);
        s16 frac = (s16)(((int)((u32)(u16)(step_num % abs_dx) << 0x10) >> 1) / (int)abs_dx);
        xwrite32(render_context - 0x2c0, (u32)(u16)(step_num / abs_dx) * 0x10000 + frac * 2);
        s32 dir_sign = 0x10000;
        if (image.signedx < 0)
        {
            dir_sign = 0xffff0000;
        }

        xwrite32(render_context - 0x2c4, dir_sign);
        s16 cross_corr = (s16)((int)(s16)(xread16(render_context - 0x37e) - xread16(render_context - 0x372)) * (int)(s16)(xread32(render_context - 0x2c0) >> 2) >> 0xe);
        if (xread32(render_context - 0x2c4) < 0)
        {
            cross_corr = -cross_corr;
        }

        u16 strip_span = (xread16(render_context - 0x370) + cross_corr) - xread16(render_context - 0x37c);
        if ((int)((u32)strip_span << 0x10) < 0)
        {
            strip_span = -strip_span;
        }

        xwrite16(render_context - 0x24e, (s16)strip_span >> (grid_shift & 0x3f));
    }

    // Compute per-strip grid stepping (16.16 fixed-point)
    s16 strip_count = xread16(render_context - 0x24e);
    s16 delta_x = xread16(render_context - 0x372) - xread16(render_context - 0x37e);
    s16 frac_x = (s16)(((int)((u32)(u16)(delta_x % strip_count) << 0x10) >> 1) / (int)strip_count);
    xwrite32(render_context - 0x2bc, (int)((u32)(u16)(delta_x / strip_count) * 0x10000 + frac_x * 2) >> (grid_shift & 0x3f));
    s16 delta_y = xread16(render_context - 0x370) - xread16(render_context - 0x37c);
    s16 frac_y = (s16)(((int)((u32)(u16)(delta_y % strip_count) << 0x10) >> 1) / (int)strip_count);
    xwrite32(render_context - 0x2b8, (int)((u32)(u16)(delta_y / strip_count) * 0x10000 + frac_y * 2) >> (grid_shift & 0x3f));

    // Direction signs for grid stepping
    s32 dir_sign_x = 0x10000;
    if (xread32(render_context - 700) < 0)
    {
        dir_sign_x = 0xffff0000;
    }

    xwrite32(render_context - 0x2a0, dir_sign_x);

    s32 dir_sign_y = 0x10000;
    if (xread32(render_context - 0x2b8) < 0)
    {
        dir_sign_y = 0xffff0000;
    }

    xwrite32(render_context - 0x29c, dir_sign_y);

    // DOS: dimension scaling delta — full 32-bit quotient (no s16 truncation)
    xwrite32(render_context - 0x2b4, (((s32)(xread16(render_context - 0x2ac) - xread16(render_context - 0x2b0)) << 8) / (int)xread16(render_context - 0x24e) << 8) >> (grid_shift & 0x3f));

    // Fog/palette interpolation setup
    xwrite32(render_context - 0x252, 0);
    xwrite32(render_context - 0x256, (u16)xread16(scene_addr + 0x9e) << 0x10);

    if (xread16(scene_addr + 0x98) != 0)
    {
        xwrite32(render_context - 0x256, 0x60000);
        u16 fog_level = xread16(scene_addr + 0x9e);
        if ((s16)fog_level < 7)
        {
            xwrite32(render_context - 0x256, 0);
        }

        if (-1 < xread16(scene_addr + 0x98))
        {
            xwrite32(render_context - 0x256, (u32)(u16)xread16(scene_addr + 0xa0) * 0x10000 - 1);
        }

        // DOS: full 32-bit fog stepping delta (no s16 truncation)
        xwrite32(render_context - 0x252, (((int)((u32)fog_level * 0x10000 - xread32(render_context - 0x256)) >> 8) / (int)(s16)(xread16(render_context - 0x24e) + 1)) << 8);
    }

    // Per-strip interpolation deltas and initial values — DOS: full 32-bit quotients
    strip_count = xread16(render_context - 0x24e);

    xwrite32(render_context - 0x2e4, (xread32(render_context - 0x318) - xread32(render_context - 0x330)) / (int)strip_count);
    xwrite32(render_context - 0x2dc, ((xread32(render_context - 0x2f0) - xread32(render_context - 0x2fc)) * 0x100 / (int)strip_count) << 8);
    xwrite32(render_context - 0x2d4, (xread32(render_context - 0x310) - xread32(render_context - 0x328)) / (int)strip_count);
    xwrite32(render_context - 0x2cc, ((xread32(render_context - 0x2ec) - xread32(render_context - 0x2f8)) * 0x100 / (int)strip_count) << 8);
    xwrite32(render_context - 0x2e8, xread32(render_context - 0x330));
    xwrite32(render_context - 0x2e0, xread32(render_context - 0x32c) << 0x10);
    xwrite32(render_context - 0x2d8, xread32(render_context - 0x328));
    xwrite32(render_context - 0x2d0, xread32(render_context - 0x2f8));
}

// Draw a textured scanline for DOS 8-bit terrain bars
// direction: 1 = left-to-right, -1 = right-to-left
// DOS ASM uses OR-based texture addressing:
//   texel = tex_data[(v_coord & v_mask) | ((h_coord >> 8) & tex_width)]
// where v_mask = (tex_height << shift) for non-subtile,
//       v_mask = ((tex_height - subtile) << shift) for subtile (with tex_data offset by subtile << shift)
void bartra_dos_line(u32 dst, s16 start_col, s16 end_col,
                                   u16 v_coord, u8 shift,
                                   u16 tex_width, u16 tex_height, u16 subtile,
                                   u32 tex_data, s16 dark_page, s8 direction)
{
    u16 v_mask;
    u32 tex_base;
    if (subtile != 0)
    {
        v_mask = (tex_height - subtile) << shift;
        tex_base = tex_data + ((u32)subtile << shift);
    }
    else
    {
        v_mask = tex_height << shift;
        tex_base = tex_data;
    }

    u16 v_masked = v_coord & v_mask;

    if (direction < 0)
    {
        // Right-to-left: ASM adds 0x2000 to tex_hbase, then iterates with h_coord -= tex_hstep
        u16 h_coord = image.tex_hbase + 0x2000;
        for (s16 col = end_col; col >= start_col; col--)
        {
            u16 x_wrapped = (h_coord >> 8) & tex_width;
            u16 combined = v_masked | x_wrapped;
            u8 texel = xread8(tex_base + combined);
            if (subtile == 0 || texel != 0)
            {
                xwrite8(dst + col, xread8(alis.ptrdark + ((u16)dark_page << 8) + texel));
            }
            h_coord -= image.tex_hstep;
        }
    }
    else
    {
        // Left-to-right: h_coord starts from tex_hbase directly
        u16 h_coord = image.tex_hbase;
        for (s16 col = start_col; col <= end_col; col++)
        {
            u16 x_wrapped = (h_coord >> 8) & tex_width;
            u16 combined = v_masked | x_wrapped;
            u8 texel = xread8(tex_base + combined);
            if (subtile == 0 || texel != 0)
            {
                xwrite8(dst + col, xread8(alis.ptrdark + ((u16)dark_page << 8) + texel));
            }
            h_coord += image.tex_hstep;
        }
    }
}

// Fixed bartra_dos(): fixes DH masking / 16-bit accumulator behavior and avoids leaking tex_hbase changes.
// Main fixes vs your version:
//  1) Horizontal coord is a true 16-bit accumulator (DX semantics). We DO NOT "renormalize" it.
//  2) We mask (h >> 8) with tex_width_mask at the moment of indexing (DH &= mask per pixel).
//  3) We never mutate image.tex_hbase in-place; we use a local working copy that can be +/- 0x2000.
//
// Notes:
//  - This keeps your overall control flow, but replaces the inner pixel loops to match the asm.
//  - Assumes xread*/xwrite*, carry4(), and your globals (image, alis, etc.) exist as in your code.

static void bartra_dos(s32 terrain_cell, s32 render_context, u16 drawy, s16 index, s16 barwidth, s16 barheight, s16 bary)
{
    if (barheight <= 0 || barwidth <= 0)
        return;

    // --- Prologue ---
    s32 _precx = (s32)(s16)image.precx;
    u32 stride = (u32)(u16)image.wloglarg;
    u32 stride_gap = stride - (u32)(u16)barwidth;
    u8 width_byte = (u8)barwidth;

    u16 tex_hstep = image.tex_hstep;

    u8 shift = xread8(render_context + index);
    u16 tex_vstep = (u16)((u16)(tex_hstep >> ((8 - shift) & 0x1f)) + 1);

    // Work on a LOCAL copy (DX in asm is a working accumulator stored in [ab50], not a global that persists)
    u16 hbase0 = (u16)image.tex_hbase;

    s16 slope_clip = xread16(render_context - 0x246); // unaff_BP
    s16 sVar9 = (s16)drawy;
    s16 slope_height_s = (s16)(slope_clip - sVar9);
    u16 slope_height = (u16)slope_height_s;
    u8 slope_rows = (u8)slope_height_s;

    // --- Load texture from type entry ---
    u32 tex_ptr_addr = xread32(render_context + index + 4);
    if (tex_ptr_addr == 0)
        return;

    s32 tex_offset = xread32(tex_ptr_addr);
    u8  tex_width_mask = xread8(tex_ptr_addr + tex_offset + 2);     // [ab52]
    s16 tex_height_raw = xread16(tex_ptr_addr + tex_offset + 4);
    u16 subtile = xread16(render_context + index + 2);
    u32 tex_base = tex_ptr_addr + (u32)tex_offset;

    // --- Darkness ---
    u16 cell = xread16(terrain_cell);

    // keep your math, but preserve it as 8-bit->page in the same way
    s16 dark_page = (s16)(u8)(image.vdarkw >> 8) - (s16)((u8)(cell >> 8) >> 6) * 2;
    if ((s16)((u16)dark_page << 8) < 0) dark_page = 0;
    u32 dark_base = ((u32)(u8)dark_page) << 8;

    // --- Shared working vars ---
    u16 v_coord = 0;
    u32 tex_data = 0;           // points to tex_base + 8 + optional subtile offset
    u16 v_mask = 0;
    u32 screen_ptr = 0;
    s16 remaining = barheight;

    // Helper macros
    #define TEXEL_INDEX(vmask16, v16, h16, hmask8) \
        ( (u16)(((vmask16) & 0xFF00) | (u16)(((u8)(vmask16)) | (u8)(((u8)((h16) >> 8)) & (hmask8)))) )

    // draw one scanline, solid, LTR
    #define DRAW_LINE_SOLID_LTR(dstptr_u32, v16, hbase16, count_u8)            \
        do {                                                                  \
            u32 _dst = (dstptr_u32);                                          \
            u16 _h = (hbase16);                                               \
            u16 _vm = (u16)((v16) & v_mask);                                  \
            u8  _c = (u8)(count_u8);                                          \
            while (_c--) {                                                    \
                u8 hm = (u8)((_h >> 8) & tex_width_mask);                     \
                u16 idx = (u16)((_vm & 0xFF00) | (u16)(((u8)_vm) | hm));       \
                u8 texel = xread8(tex_data + idx);                            \
                u8 pixel = xread8(alis.ptrdark + dark_base + texel);          \
                xwrite8(_dst, pixel);                                         \
                _dst++;                                                       \
                _h = (u16)(_h + tex_hstep);                                   \
            }                                                                 \
        } while (0)

    // draw one scanline, solid, RTL (dst points *one past* the last pixel in asm; we pre-decrement)
    #define DRAW_LINE_SOLID_RTL(dstptr_u32, v16, hbase16, count_u8)            \
        do {                                                                  \
            u32 _dst = (dstptr_u32);                                          \
            u16 _h = (hbase16);                                               \
            u16 _vm = (u16)((v16) & v_mask);                                  \
            u8  _c = (u8)(count_u8);                                          \
            while (_c--) {                                                    \
                u8 hm = (u8)((_h >> 8) & tex_width_mask);                     \
                u16 idx = (u16)((_vm & 0xFF00) | (u16)(((u8)_vm) | hm));       \
                u8 texel = xread8(tex_data + idx);                            \
                u8 pixel = xread8(alis.ptrdark + dark_base + texel);          \
                _dst--;                                                       \
                xwrite8(_dst, pixel);                                         \
                _h = (u16)(_h - tex_hstep);                                   \
            }                                                                 \
        } while (0)

    // draw one scanline, transparent (skip texel==0), LTR
    #define DRAW_LINE_XP_LTR(dstptr_u32, v16, hbase16, count_u8)               \
        do {                                                                  \
            u32 _dst = (dstptr_u32);                                          \
            u16 _h = (hbase16);                                               \
            u16 _vm = (u16)((v16) & v_mask);                                  \
            u8  _c = (u8)(count_u8);                                          \
            while (_c--) {                                                    \
                u8 hm = (u8)((_h >> 8) & tex_width_mask);                     \
                u16 idx = (u16)((_vm & 0xFF00) | (u16)(((u8)_vm) | hm));       \
                u8 texel = xread8(tex_data + idx);                            \
                if (texel) {                                                  \
                    u8 pixel = xread8(alis.ptrdark + dark_base + texel);      \
                    xwrite8(_dst, pixel);                                     \
                }                                                             \
                _dst++;                                                       \
                _h = (u16)(_h + tex_hstep);                                   \
            }                                                                 \
        } while (0)

    // --- Top part selection ---
    if (slope_height_s == 0 || slope_clip < sVar9) {
        // No slope
        v_mask = (u16)((u16)(tex_height_raw - (s16)subtile) << (shift & 0x1f));
        u32 subtile_off = ((u32)subtile) << (shift & 0x1f);
        tex_data = tex_base + subtile_off + 8;

        screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx;
        v_coord = 0;
        remaining = barheight;
    } else {
        // Slope exists
        if (subtile == 0) {
            // --- No-subtile slope ---
            remaining = (s16)(barheight - (s16)slope_height);
            if (barheight < (s16)slope_height) {
                slope_rows = (u8)((s8)slope_rows + (s8)remaining);
            }

            // atalias lookup (your logic preserved)
            u16 slope_dist = (u16)(slope_clip - bary);
            s16 slope_step;
            if (slope_dist < 0x41) {
                slope_step = xread16(image.atalias + (u16)(((slope_dist - 1) * 0x40u + (u16)barwidth) - 1u) * 2u);
            } else {
                slope_dist >>= 2;
                if (0x40 < slope_dist) slope_dist = 0x40;
                slope_step = (s16)(xread16(image.atalias + (u16)(((slope_dist - 1) * 0x40u + ((u16)barwidth >> 1)) - 1u) * 2u) >> 1);
            }

            u8 col_width = 1;
            v_coord = 0;
            if (bary < sVar9) {
                v_coord = (u16)(sVar9 - bary) * tex_vstep;
                col_width = (u8)((u16)((u16)(sVar9 - bary) * (u16)slope_step) >> 8) + 1;
            }

            v_mask = (u16)((u16)tex_height_raw << (shift & 0x1f));
            tex_data = tex_base + 8;

            // direction check
            if (slope_clip == xread16(render_context - 0x25c)) {
                // LTR slope (solid)
                screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx;
                u16 col_frac = (u16)col_width << 8;

                u8 rows = slope_rows;
                do {
                    u8 cols = (u8)(col_frac >> 8);
                    DRAW_LINE_SOLID_LTR(screen_ptr, v_coord, hbase0, cols);
                    v_coord = (u16)(v_coord + tex_vstep);
                    col_frac = (u16)(col_frac + (u16)slope_step);
                    screen_ptr += stride;
                } while (--rows);
            } else {
                // RTL slope (solid) - uses +0x2000 hbase tweak locally
                u16 hbase = (u16)(hbase0 + 0x2000u);
                screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx + (u32)width_byte;

                u16 col_frac = (u16)col_width << 8;
                u8 rows = slope_rows;
                do {
                    u8 cols = (u8)(col_frac >> 8);
                    DRAW_LINE_SOLID_RTL(screen_ptr, v_coord, hbase, cols);
                    v_coord = (u16)(v_coord + tex_vstep);
                    col_frac = (u16)(col_frac + (u16)slope_step);
                    screen_ptr += stride;
                } while (--rows);

                screen_ptr -= (u32)width_byte; // match asm tail adjust
            }

            if (remaining < 1) return;
        } else {
            // --- Subtile slope ---
            remaining = (s16)(barheight - (s16)slope_height);
            if (barheight < (s16)slope_height) {
                slope_height = (u16)((u16)slope_height + (u16)remaining);
            }
            u16 slope_h = slope_height;

            // NOTE: This portion is complex in the asm; we keep your structure but fix pixel indexing.
            if (slope_clip == (s16)(bary - bartra_saved_si)) {
                // PATH A: flat subtile (transparent), LTR
                v_coord = 0;
                if (bary < sVar9) {
                    v_coord = (u16)(sVar9 - bary) * tex_vstep;
                }

                v_mask = (u16)((u16)(tex_height_raw - (s16)subtile) << (shift & 0x1f));
                u32 subtile_off = ((u32)subtile) << (shift & 0x1f);
                tex_data = tex_base + 8;

                screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx;

                u8 sub_rows = (u8)(slope_h & 0xFF);
                do {
                    DRAW_LINE_XP_LTR(screen_ptr, v_coord, hbase0, width_byte);
                    v_coord = (u16)(v_coord + tex_vstep);
                    screen_ptr += stride;
                } while (--sub_rows);

                if (remaining < 1) return;

                tex_data += subtile_off;
                v_coord = 0;
                // NOTE: v_mask stays at (tex_height_raw - subtile) << shift
                // The original does NOT reset to full height here (Ghidra lines 18110-18111)
            } else {
                // PATH B: perspective subtile
                // ASM: slope_dist = (slope_clip - SI) + DAT_0001ab5e
                //   SI = ground_clip_y (with extra_height), DAT_0001ab5e = extra_height
                //   so slope_dist = slope_clip - ground_base_y (without extra height)
                u16 slope_dist = (u16)((slope_clip - bary) + bartra_saved_si);
                s16 slope_step;
                if (slope_dist < 0x41) {
                    slope_step = xread16(image.atalias + (u16)((slope_dist + ((u16)barwidth - 1u) * 0x40u) - 1u) * 2u);
                } else {
                    slope_dist >>= 2;
                    if (0x40 < slope_dist) slope_dist = 0x40;
                    slope_step = (s16)(xread16(image.atalias + (u16)((slope_dist + ((((u16)(barwidth - 1) * 0x40u)) >> 1)) - 1u) * 2u) >> 1);
                }

                u8 slope_step_rows = (u8)((u16)slope_step >> 8);
                s32 scanline_delta = (s32)slope_step_rows * (s32)(s16)image.wloglarg;

                u16 v_start = 0;
                if (bary < sVar9) {
                    v_start = (u16)(sVar9 - bary) * tex_vstep;
                }

                v_mask = (u16)((u16)(tex_height_raw - (s16)subtile) << (shift & 0x1f));
                u32 subtile_off = ((u32)subtile) << (shift & 0x1f);
                tex_data = tex_base + 8;

                // Direction check
                if (slope_clip == xread16(render_context - 0x25c)) {
                    // LTR perspective (transparent)
                    s32 drawy_idx = (s16)(sVar9 - image.wlogy1);
                    u32 end_ptr = xread32(image.atlpix + (s16)((slope_h & 0xFF) + drawy_idx) * 4) + _precx;
                    u32 start_ptr = xread32(image.atlpix + (s16)drawy_idx * 4) + _precx;

                    u32 h_accum = (u32)hbase0;
                    u8 col_count = width_byte;

                    do {
                        // Mask h byte before accumulation (Ghidra: bVar20 = (byte)(uVar18>>8) & DAT_0001ab52)
                        u8 h_masked = (u8)((u16)h_accum >> 8) & tex_width_mask;
                        u32 h_saved = (h_accum & 0xFFFF0000u) | ((u32)h_masked << 8) | (h_accum & 0xFF);

                        u32 cur = start_ptr;
                        u16 vc = v_start;

                        while (cur < end_ptr) {
                            u16 vm = (u16)(vc & v_mask);
                            u16 idx = (u16)((vm & 0xFF00) | (u16)(((u8)vm) | h_masked));
                            u8 texel = xread8(tex_data + idx);
                            if (texel) {
                                xwrite8(cur, texel);
                            }
                            vc = (u16)(vc + tex_vstep);
                            cur += stride;
                        }

                        // Advance h_accum with carry (Ghidra: uVar18 = uVar19 + _DAT_0001ab4a)
                        u32 addv = ((u32)(u8)slope_step << 24) | (u32)(tex_hstep & 0xFFFF);
                        u32 new_h = h_saved + addv;
                        if (carry4(h_saved, addv)) {
                            start_ptr += stride;
                        }

                        if (end_ptr <= start_ptr + (u32)scanline_delta) break;
                        start_ptr += (u32)scanline_delta + 1;
                        h_accum = new_h;
                    } while (--col_count);

                    if (remaining < 1) return;
                    tex_data += subtile_off;
                    v_coord = 0;
                    v_start = tex_hstep;
                    // NOTE: v_mask stays at (tex_height_raw - subtile) << shift
                    // The original does NOT reset to full height here (Ghidra lines 18188-18190)
                } else {
                    // RTL perspective (transparent)
                    s32 drawy_idx = (s16)(sVar9 - image.wlogy1);
                    u32 end_ptr = xread32(image.atlpix + (s16)((slope_h & 0xFF) + drawy_idx) * 4) + _precx;
                    u32 start_ptr = xread32(image.atlpix + (s16)drawy_idx * 4) + _precx + (u32)width_byte;

                    u16 hbase = (u16)(hbase0 + 0x2000u);
                    u32 h_accum = (u32)hbase;
                    u8 col_count = width_byte;

                    do {
                        // Mask h byte before accumulation (Ghidra RTL)
                        u8 h_masked = (u8)((u16)h_accum >> 8) & tex_width_mask;
                        u32 h_saved = (h_accum & 0xFFFF0000u) | ((u32)h_masked << 8) | (h_accum & 0xFF);

                        u32 cur = start_ptr;
                        u16 vc = v_start;

                        while (cur < end_ptr) {
                            u16 vm = (u16)(vc & v_mask);
                            u16 idx = (u16)((vm & 0xFF00) | (u16)(((u8)vm) | h_masked));
                            u8 texel = xread8(tex_data + idx);
                            if (texel) {
                                xwrite8(cur, texel);
                            }
                            vc = (u16)(vc + tex_vstep);
                            cur += stride;
                        }

                        u32 subv = ((u32)(u8)slope_step << 24) | (u32)(tex_hstep & 0xFFFF);
                        u32 new_h = h_saved - subv;
                        if (h_saved < subv) {
                            start_ptr += stride;
                        }

                        if (end_ptr <= start_ptr + (u32)scanline_delta) break;
                        start_ptr += (u32)scanline_delta - 1;
                        h_accum = new_h;
                    } while (--col_count);

                    if (remaining < 1) return;
                    tex_data += subtile_off;
                    v_coord = 0;
                    v_start = tex_hstep;
                    // NOTE: v_mask stays at (tex_height_raw - subtile) << shift
                    // The original does NOT reset to full height here (Ghidra lines 18246-18248)
                }
            }
        }
    }

    // --- Mid/flat section (solid, LTR), matches LAB_0000df53 family ---
    s16 mid_rows = (s16)(remaining - image.vbarbot);
    if (mid_rows > 0) {
        s16 mid_drawy = (s16)(sVar9 + (barheight - remaining));
        screen_ptr = xread32(image.atlpix + (s16)(mid_drawy - image.wlogy1) * 4) + _precx;

        u8 mr = (u8)mid_rows;
        do {
            DRAW_LINE_SOLID_LTR(screen_ptr, v_coord, hbase0, width_byte);
            v_coord = (u16)(v_coord + tex_vstep);
            screen_ptr += stride;
        } while (--mr);
    }

    // --- Bottom section ---
    if (image.vbarbot > 0) {
        // Your bottom section logic kept, but inner loops fixed to use DX semantics/masking
        u16 bot_dist = (u16)(botalt - bothigh);
        if (bot_dist != 0 && bothigh <= botalt) {
            s16 bot_slope_step;
            u16 bot_width_val = (u16)width_byte;
            if (bot_dist < 0x41) {
                bot_slope_step = xread16(image.atalias + (u16)(((bot_dist - 1u) * 0x40u + bot_width_val) - 1u) * 2u);
            } else {
                bot_dist >>= 2;
                if (0x40 < bot_dist) bot_dist = 0x40;
                bot_slope_step = (s16)(xread16(image.atalias + (u16)(((bot_dist - 1u) * 0x40u + (bot_width_val >> 1)) - 1u) * 2u) >> 1);
            }

            u8 bot_rows = (u8)image.vbarbot;
            u16 bot_v_coord = 0;

            u32 bot_tex_ptr = xread32(render_context + bottom_type_index + 4);
            if (bot_tex_ptr != 0) {
                s32 bot_tex_off = xread32(bot_tex_ptr);
                u8  bot_tex_width_mask = xread8(bot_tex_ptr + bot_tex_off + 2);
                u8  bot_shift = xread8(render_context + bottom_type_index);
                u16 bot_v_mask = (u16)(xread16(bot_tex_ptr + bot_tex_off + 4) << (bot_shift & 0x1f));
                u32 bot_tex_data = bot_tex_ptr + (u32)bot_tex_off + 8;

                u8 bot_dark = (u8)(image.vdarkw >> 8) - (u8)((u8)(cell >> 8) >> 6) * 2;
                if ((s16)((u16)bot_dark << 8) < 0) bot_dark = 0;
                u32 bot_dark_base = ((u32)bot_dark) << 8;

                u32 bot_screen = xread32(image.atlpix + (s16)(bothigh - image.wlogy1) * 4) + _precx;

                // col_frac starts at width in high byte, 0xFF low byte
                u16 bot_col_frac = (u16)(((u16)width_byte << 8) | 0xFF);

                if (botalt == precboti) {
                    // RTL bottom (solid)
                    bot_screen += (u32)width_byte;
                    u16 hbase = (u16)(hbase0 + 0x2000u);

                    do {
                        u8 cols = (u8)(bot_col_frac >> 8);

                        // custom inner loop (uses bot masks & tex pointers)
                        u32 _dst = bot_screen;
                        u16 _h = hbase;
                        u16 _vm = (u16)(bot_v_coord & bot_v_mask);
                        u8  _c = cols;
                        while (_c--) {
                            u8 hm = (u8)((_h >> 8) & bot_tex_width_mask);
                            u16 idx = (u16)((_vm & 0xFF00) | (u16)(((u8)_vm) | hm));
                            u8 texel = xread8(bot_tex_data + idx);
                            u8 pixel = xread8(alis.ptrdark + bot_dark_base + texel);
                            _dst--;
                            xwrite8(_dst, pixel);
                            _h = (u16)(_h - tex_hstep);
                        }

                        bot_v_coord = (u16)(bot_v_coord + tex_vstep);
                        bot_col_frac = (u16)(bot_col_frac - (u16)bot_slope_step);
                        bot_screen += stride;
                    } while (--bot_rows);
                } else {
                    // LTR bottom (solid)
                    do {
                        u8 cols = (u8)(bot_col_frac >> 8);

                        u32 _dst = bot_screen;
                        u16 _h = hbase0;
                        u16 _vm = (u16)(bot_v_coord & bot_v_mask);
                        u8  _c = cols;
                        while (_c--) {
                            u8 hm = (u8)((_h >> 8) & bot_tex_width_mask);
                            u16 idx = (u16)((_vm & 0xFF00) | (u16)(((u8)_vm) | hm));
                            u8 texel = xread8(bot_tex_data + idx);
                            u8 pixel = xread8(alis.ptrdark + bot_dark_base + texel);
                            xwrite8(_dst, pixel);
                            _dst++;
                            _h = (u16)(_h + tex_hstep);
                        }

                        bot_v_coord = (u16)(bot_v_coord + tex_vstep);
                        bot_col_frac = (u16)(bot_col_frac - (u16)bot_slope_step);
                        bot_screen += stride;
                    } while (--bot_rows);
                }
            }
        }
    }

    #undef DRAW_LINE_SOLID_LTR
    #undef DRAW_LINE_SOLID_RTL
    #undef DRAW_LINE_XP_LTR
    #undef TEXEL_INDEX
}

// New bartra_dos: rewritten from Ghidra decompiled rrq-dos.c bartra() function.
// Register-to-parameter mapping:
//   in_EAX low = drawy, in_ECX = barwidth, in_DX = barheight
//   unaff_BP = xread16(render_context - 0x246) (slope clip Y)
//   unaff_SI = bary (pre-clip bar Y)
//   unaff_DI = index (type entry offset into render_context via GS)
//   GS segment = render_context, FS segment = alis.mem base
// DAT variable mapping:
//   _DAT_0001ab38 = stride (wloglarg)
//   _DAT_0001ab3c = stride_gap (stride - barwidth)
//   _DAT_0001ab46 byte = width_byte (barwidth as byte, inner column counter)
//   DAT_0001ab4a = tex_hstep (image.tex_hstep)
//   DAT_0001ab4e = tex_vstep
//   DAT_0001ab50 = image.tex_hbase (horizontal texture base)
//   DAT_0001ab52 = tex_width (byte from texture header +2)
//   _DAT_0001ab54 = v_mask ((tex_height - subtile) << shift)
//   _DAT_0001ab5a = subtile_offset (subtile << shift)
//   DAT_0001ab58 = terrain cell type byte (for darkness calc)
//   DAT_0001ab44 = slope_step (from atalias lookup)
//   _DAT_0001ab40 = scanline_delta (for perspective subtile)
//   DAT_0001ab66 = image.vbarbot
//   DAT_0001ab64 = bottom_type_index
//   DAT_0001aad2 = precboti (bottom direction reference)
//   _DAT_0001ab5e = bartra_saved_si
//   _wlogx2 = image.wlogy1
//   GS:[0x99e] = xread16(render_context - 0x25c) (direction ref)

static void bartra_dosx(s32 terrain_cell, s32 render_context, u16 drawy, s16 index, s16 barwidth, s16 barheight, s16 bary)
{
    if (barheight <= 0 || barwidth <= 0)
        return;

    // === Prologue (Ghidra lines 17896-17908) ===
    s32 _precx = (s32)(s16)image.precx;
    u32 stride = (u32)(u16)image.wloglarg;
    u32 stride_gap = stride - (u32)(u16)barwidth;
    u8 width_byte = (u8)barwidth;
    u16 tex_hstep = image.tex_hstep;
    u8 shift = xread8(render_context + index);
    u16 tex_vstep = (tex_hstep >> ((8 - shift) & 0x1f)) + 1;

    s16 slope_clip = xread16(render_context - 0x246);  // unaff_BP
    s16 sVar9 = (s16)drawy;
    u16 slope_height = (u16)(slope_clip - sVar9);
    u8 slope_rows = (u8)slope_height;

    // === Load texture from type entry ===
    u32 tex_ptr_addr = xread32(render_context + index + 4);
    if (tex_ptr_addr == 0)
        return;
    s32 tex_offset = xread32(tex_ptr_addr);
    u8 tex_width = xread8(tex_ptr_addr + tex_offset + 2);
    s16 tex_height_raw = xread16(tex_ptr_addr + tex_offset + 4);
    u16 subtile = xread16(render_context + index + 2);
    u32 tex_base = tex_ptr_addr + tex_offset;  // piVar8 + iVar25 in Ghidra

    // === Compute darkness (Ghidra: DAT_0001ab58 derived) ===
    u16 cell = xread16(terrain_cell);
    s16 dark_page = (u8)(image.vdarkw >> 8) - (s16)((u8)(cell >> 8) >> 6) * 2;
    if ((s16)((u16)dark_page << 8) < 0) {
        dark_page = 0;
    }
    u32 dark_base = (u32)(u8)dark_page << 8;

    // Working variables (set by each path before mid section)
    u16 v_coord = 0;
    u32 tex_data = 0;  // iVar25 equivalent (includes +8 data offset)
    u16 v_mask = 0;
    u32 screen_ptr = 0;
    s16 remaining = barheight;

    if (slope_height == 0 || slope_clip < sVar9) {
        // === No slope path (Ghidra lines 17909-17928) ===
        // Load texture with subtile adjustment
        v_mask = (u16)(tex_height_raw - (s16)subtile) << (shift & 0x1f);
        u32 subtile_off = (u32)subtile << (shift & 0x1f);
        tex_data = tex_base + subtile_off + 8;

        screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx;
        v_coord = 0;
        remaining = barheight;
    }
    else {
        // === Slope exists ===
        if (subtile == 0) {
            // === No-subtile slope (Ghidra lines 17931-18050) ===
            remaining = barheight - (s16)slope_height;
            if (barheight < (s16)slope_height) {
                slope_rows += (u8)(s8)remaining;  // cVar21 += (char)sVar17
            }

            // atalias lookup for perspective step (Ghidra lines 17936-17949)
            // Use image.vbarlarg (full unclipped width) for correct slope geometry
            // (matches m68k bartra which uses image.vbarlarg, not barwidth)
            u16 slope_dist = (u16)(slope_clip - bary);  // unaff_BP - unaff_SI
            u16 slope_width = (image.vbarlarg > 0) ? image.vbarlarg : (u16)barwidth;
            s16 slope_step;
            if (slope_dist < 0x41) {
                slope_step = xread16(image.atalias + (u16)(((slope_dist - 1) * 0x40 + slope_width) - 1) * 2);
            }
            else {
                slope_dist >>= 2;
                if (0x40 < slope_dist)
                    slope_dist = 0x40;
                slope_step = xread16(image.atalias + (u16)(((slope_dist - 1) * 0x40 + (slope_width >> 1)) - 1) * 2) >> 1;
            }

            // v_coord offset and initial column width (Ghidra lines 17950-17955)
            u8 col_width = 1;
            v_coord = 0;
            if (bary < sVar9) {
                v_coord = (u16)(sVar9 - bary) * tex_vstep;
                col_width = (u8)((u16)((sVar9 - bary) * slope_step) >> 8) + 1;
            }

            // Texture setup (no subtile)
            v_mask = (u16)tex_height_raw << (shift & 0x1f);
            tex_data = tex_base + 8;

            // Direction check (Ghidra line 17956)
            if (slope_clip == xread16(render_context - 0x25c)) {
                // === LEFT-TO-RIGHT slope (Ghidra lines 17957-17999) ===
                screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx;
                u16 col_frac = (u16)col_width << 8;  // uVar18 = bVar20 << 8

                u8 rows = slope_rows;
                do {
                    u8 cols = (u8)(col_frac >> 8);
                    if (cols > width_byte) cols = width_byte;
                    u16 h_coord = ((image.tex_hbase >> 8) & tex_width) << 8 | (image.tex_hbase & 0xFF);
                    u32 v_masked = (u32)(v_coord & v_mask);
                    u32 dst = screen_ptr;

                    u8 c = cols;
                    do {
                        u8 h_hi = (u8)(h_coord >> 8);
                        u16 texel_idx = (u16)(v_masked & 0xFF00) | ((u8)v_masked | h_hi);
                        u8 texel = xread8(tex_data + texel_idx);
                        u8 pixel = xread8(alis.ptrdark + dark_base + texel);
                        xwrite8(dst, pixel);
                        dst++;
                        h_coord += tex_hstep;
                        h_coord = ((h_coord >> 8) & tex_width) << 8 | (h_coord & 0xFF);
                        c--;
                    } while (c != 0);

                    v_coord += tex_vstep;
                    col_frac += (u16)slope_step;
                    screen_ptr += stride;
                    rows--;
                } while (rows != 0);
            }
            else {
                // === RIGHT-TO-LEFT slope (Ghidra lines 18000-18046) ===
                screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx
                             + (u32)(u8)width_byte;
                image.tex_hbase += 0x2000;
                u16 col_frac = (u16)col_width << 8;

                u8 rows = slope_rows;
                do {
                    u8 cols = (u8)(col_frac >> 8);
                    if (cols > width_byte) cols = width_byte;
                    u16 h_coord = ((image.tex_hbase >> 8) & tex_width) << 8 | (image.tex_hbase & 0xFF);
                    u32 v_masked = (u32)(v_coord & v_mask);
                    u32 dst = screen_ptr;

                    u8 c = cols;
                    do {
                        u8 h_hi = (u8)(h_coord >> 8);
                        u16 texel_idx = (u16)(v_masked & 0xFF00) | ((u8)v_masked | h_hi);
                        u8 texel = xread8(tex_data + texel_idx);
                        u8 pixel = xread8(alis.ptrdark + dark_base + texel);
                        dst--;
                        xwrite8(dst, pixel);
                        h_coord -= tex_hstep;
                        h_coord = ((h_coord >> 8) & tex_width) << 8 | (h_coord & 0xFF);
                        c--;
                    } while (c != 0);

                    v_coord += tex_vstep;
                    col_frac += (u16)slope_step;
                    screen_ptr += stride;
                    rows--;
                } while (rows != 0);

                screen_ptr -= (u32)(u8)width_byte;
                image.tex_hbase += (s16)0xe000;  // -= 0x2000, restore
            }

            if (remaining < 1) {
                return;
            }
        }
        else {
            // === Subtile slope (Ghidra lines 18051-18251) ===
            remaining = barheight - (s16)slope_height;
            if (barheight < (s16)slope_height) {
                slope_height += (u16)remaining;
            }
            u16 slope_h = slope_height;

            if (slope_clip == (s16)(bary - bartra_saved_si)) {
                // === PATH A: Flat subtile (Ghidra lines 18057-18112) ===
                v_coord = 0;
                if (bary < sVar9) {
                    v_coord = (u16)(sVar9 - bary) * tex_vstep;
                }

                // Texture with subtile reduction
                v_mask = (u16)(tex_height_raw - (s16)subtile) << (shift & 0x1f);
                u32 subtile_off = (u32)subtile << (shift & 0x1f);
                tex_data = tex_base + 8;  // without subtile offset initially

                screen_ptr = xread32(image.atlpix + (s16)(sVar9 - image.wlogy1) * 4) + _precx;

                // Render subtile slope rows (LTR with transparency)
                u16 sub_rows = slope_h & 0xFF;
                do {
                    u16 h_coord = ((image.tex_hbase >> 8) & tex_width) << 8 | (image.tex_hbase & 0xFF);
                    u32 v_masked = (u32)(v_coord & v_mask);
                    u32 dst = screen_ptr;
                    u8 c = width_byte;

                    do {
                        u8 h_hi = (u8)(h_coord >> 8);
                        u16 texel_idx = (u16)(v_masked & 0xFF00) | ((u8)v_masked | h_hi);
                        u8 texel = xread8(tex_data + texel_idx);
                        if (texel != 0) {
                            xwrite8(dst, xread8(alis.ptrdark + dark_base + texel));
                        }
                        dst++;
                        h_coord += tex_hstep;
                        h_coord = ((h_coord >> 8) & tex_width) << 8 | (h_coord & 0xFF);
                        c--;
                    } while (c != 0);

                    v_coord += tex_vstep;
                    screen_ptr += stride_gap + (u32)width_byte;  // = stride
                    sub_rows--;
                } while (sub_rows != 0);

                if (remaining < 1) {
                    return;
                }

                // Advance tex_data past subtile section
                tex_data += subtile_off;
                v_coord = 0;
            }
            else {
                // === PATH B: Perspective subtile slope (Ghidra lines 18113-18250) ===
                // atalias lookup
                u16 slope_dist = (u16)(slope_clip - bary) + (u16)bartra_saved_si;
                s16 slope_step;
                if (slope_dist < 0x41) {
                    slope_step = xread16(image.atalias + (u16)((slope_dist + ((u16)barwidth - 1) * 0x40) - 1) * 2);
                }
                else {
                    slope_dist >>= 2;
                    if (0x40 < slope_dist)
                        slope_dist = 0x40;
                    slope_step = xread16(image.atalias + (u16)((slope_dist + (((u16)(barwidth - 1) * 0x40) >> 1)) - 1) * 2) >> 1;
                }

                // Scanline delta (Ghidra lines 18129-18132)
                u8 slope_step_rows = (u8)((u16)slope_step >> 8);
                s32 scanline_delta = (s32)slope_step_rows * (s32)(s16)image.wloglarg;

                // v_coord offset
                u16 v_start = 0;
                if (bary < sVar9) {
                    v_start = (u16)(sVar9 - bary) * tex_vstep;
                }

                // Texture with subtile reduction
                v_mask = (u16)(tex_height_raw - (s16)subtile) << (shift & 0x1f);
                u32 subtile_off = (u32)subtile << (shift & 0x1f);
                tex_data = tex_base + 8;

                // Direction check (Ghidra line 18137)
                if (slope_clip == xread16(render_context - 0x25c)) {
                    // === LTR perspective subtile (Ghidra lines 18138-18190) ===
                    s32 drawy_idx = (s16)(sVar9 - image.wlogy1);
                    u32 end_ptr = xread32(image.atlpix + (s16)((slope_h & 0xFF) + drawy_idx) * 4) + _precx;
                    u32 start_ptr = xread32(image.atlpix + (s16)drawy_idx * 4) + _precx;

                    u32 h_accum = (u32)image.tex_hbase;
                    u8 col_count = width_byte;

                    do {
                        u8 h_masked = (u8)(h_accum >> 8) & tex_width;
                        u32 h_saved = (h_accum & 0xFFFF0000) | ((u32)h_masked << 8) | (h_accum & 0xFF);
                        u32 cur = start_ptr;
                        u16 vc = v_start;

                        do {
                            u16 v_m = vc & v_mask;
                            u16 texel_idx = (v_m & 0xFF00) | ((u8)v_m | h_masked);
                            u8 texel = xread8(tex_data + texel_idx);
                            if (texel != 0) {
                                xwrite8(cur, xread8(alis.ptrdark + dark_base + texel));
                            }
                            vc += tex_vstep;
                            cur += stride;
                        } while (cur < end_ptr);

                        // Advance h_coord with carry
                        u32 new_h = h_saved + ((u32)(u8)slope_step << 24 | (u32)(tex_hstep & 0xFFFF));
                        if (carry4(h_saved, ((u32)(u8)slope_step << 24 | (u32)(tex_hstep & 0xFFFF)))) {
                            start_ptr += stride;
                        }

                        if (end_ptr <= start_ptr + (u32)scanline_delta) break;
                        start_ptr += (u32)scanline_delta + 1;
                        h_accum = new_h;
                        col_count--;
                    } while (col_count != 0);

                    if (remaining < 1) {
                        return;
                    }
                    tex_data += subtile_off;
                    v_coord = 0;
                    v_start = tex_hstep;  // Ghidra: uVar16 = DAT_0001ab4a
                }
                else {
                    // === RTL perspective subtile (Ghidra lines 18192-18249) ===
                    s32 drawy_idx = (s16)(sVar9 - image.wlogy1);
                    u32 end_ptr = xread32(image.atlpix + (s16)((slope_h & 0xFF) + drawy_idx) * 4) + _precx;
                    u32 start_ptr = xread32(image.atlpix + (s16)drawy_idx * 4) + _precx + (u32)(u8)width_byte;

                    image.tex_hbase += 0x2000;
                    u32 h_accum = (u32)image.tex_hbase;
                    u8 col_count = width_byte;

                    do {
                        u8 h_masked = (u8)(h_accum >> 8) & tex_width;
                        u32 h_saved = (h_accum & 0xFFFF0000) | ((u32)h_masked << 8) | (h_accum & 0xFF);
                        u32 cur = start_ptr;
                        u16 vc = v_start;

                        do {
                            u16 v_m = vc & v_mask;
                            u16 texel_idx = (v_m & 0xFF00) | ((u8)v_m | h_masked);
                            u8 texel = xread8(tex_data + texel_idx);
                            if (texel != 0) {
                                xwrite8(cur, xread8(alis.ptrdark + dark_base + texel));
                            }
                            vc += tex_vstep;
                            cur += stride;
                        } while (cur < end_ptr);

                        // Subtract h_step (RTL)
                        u32 new_h = h_saved - ((u32)(u8)slope_step << 24 | (u32)(tex_hstep & 0xFFFF));
                        if (h_saved < ((u32)(u8)slope_step << 24 | (u32)(tex_hstep & 0xFFFF))) {
                            start_ptr += stride;
                        }

                        if (end_ptr <= start_ptr + (u32)scanline_delta) break;
                        start_ptr += (u32)scanline_delta - 1;
                        h_accum = new_h;
                        col_count--;
                    } while (col_count != 0);

                    image.tex_hbase += (s16)0xe000;  // -= 0x2000, restore

                    if (remaining < 1) {
                        return;
                    }
                    tex_data += subtile_off;
                    v_coord = 0;
                    v_start = tex_hstep;
                }

                // Recompute screen_ptr for mid section after perspective subtile
                // (continuing from where slope ended)
            }

            // NOTE: v_mask stays at (tex_height_raw - subtile) << shift
            // The original does NOT reset to full height here — body uses reduced mask
        }
    }

    // === Mid/flat section (Ghidra lines 18253-18278) ===
    // Render (remaining - vbarbot) rows at full width, left-to-right
    s16 mid_rows = remaining - image.vbarbot;
    if (mid_rows != 0 && image.vbarbot <= remaining) {
        // Recompute screen_ptr for mid section: advance past slope rows
        s16 mid_drawy = sVar9 + (barheight - remaining);
        screen_ptr = xread32(image.atlpix + (s16)(mid_drawy - image.wlogy1) * 4) + _precx;

        u8 mr = (u8)mid_rows;
        do {
            u16 h_coord = ((image.tex_hbase >> 8) & tex_width) << 8 | (image.tex_hbase & 0xFF);
            u32 v_masked = (u32)(v_coord & v_mask);
            u32 dst = screen_ptr;
            u8 c = width_byte;

            do {
                u8 h_hi = (u8)(h_coord >> 8);
                u16 texel_idx = (u16)(v_masked & 0xFF00) | ((u8)v_masked | h_hi);
                u8 texel = xread8(tex_data + texel_idx);
                u8 pixel = xread8(alis.ptrdark + dark_base + texel);
                xwrite8(dst, pixel);
                dst++;
                h_coord += tex_hstep;
                h_coord = ((h_coord >> 8) & tex_width) << 8 | (h_coord & 0xFF);
                c--;
            } while (c != 0);

            v_coord += tex_vstep;
            screen_ptr += stride;
            mr--;
        } while (mr != 0);
    }

    // === Bottom section (Ghidra lines 18279-18380) ===
    // Renders vbarbot rows using a separate texture type (bottom_type_index)
    if (image.vbarbot != 0 && 0 < image.vbarbot) {
        u16 bot_dist = (u16)(botalt - bothigh);
        if (bot_dist != 0 && bothigh <= botalt) {
            // atalias lookup for bottom slope (Ghidra lines 18281-18293)
            s16 bot_slope_step;
            u16 bot_width_val = (u16)width_byte;  // barwidth as byte
            if (bot_dist < 0x41) {
                bot_slope_step = xread16(image.atalias + (u16)(((bot_dist - 1) * 0x40 + bot_width_val) - 1) * 2);
            }
            else {
                bot_dist >>= 2;
                if (0x40 < bot_dist) bot_dist = 0x40;
                bot_slope_step = xread16(image.atalias + (u16)(((bot_dist - 1) * 0x40 + (bot_width_val >> 1)) - 1) * 2) >> 1;
            }

            u8 bot_rows = (u8)image.vbarbot;
            u16 bot_v_coord = 0;
            u16 bot_hstep = image.tex_hstep;

            // Load bottom texture from bottom_type_index
            u32 bot_tex_ptr = xread32(render_context + bottom_type_index + 4);
            if (bot_tex_ptr != 0) {
                s32 bot_tex_off = xread32(bot_tex_ptr);
                u8 bot_tex_width = xread8(bot_tex_ptr + bot_tex_off + 2);
                u8 bot_shift = xread8(render_context + bottom_type_index);
                u16 bot_v_mask = xread16(bot_tex_ptr + bot_tex_off + 4) << (bot_shift & 0x1f);
                u32 bot_tex_data = bot_tex_ptr + bot_tex_off + 8;

                // Darkness for bottom section
                u8 bot_dark = (u8)(image.vdarkw >> 8) - (u8)((u8)(cell >> 8) >> 6) * 2;
                if ((s16)((u16)bot_dark << 8) < 0)
                    bot_dark = 0;

                // Bottom screen address
                u32 bot_screen = xread32(image.atlpix + (s16)(bothigh - image.wlogy1) * 4) + _precx;

                // Column width starts at full width (Ghidra: CONCAT31(width_byte, 0xFF))
                u16 bot_col_frac = ((u16)width_byte << 8) | 0xFF;

                // Direction: botalt == precboti → RTL, else LTR (Ghidra line 18297)
                if (botalt == precboti) {
                    // === RTL bottom (Ghidra lines 18298-18339) ===
                    bot_screen += (u32)(u8)width_byte;
                    image.tex_hbase += 0x2000;

                    do {
                        u8 cols = (u8)(bot_col_frac >> 8);
                        u16 h_coord = ((image.tex_hbase >> 8) & bot_tex_width) << 8 | (image.tex_hbase & 0xFF);
                        u32 v_masked = (u32)(bot_v_coord & bot_v_mask);
                        u32 dst = bot_screen;

                        u8 c = cols;
                        do {
                            u8 h_hi = (u8)(h_coord >> 8);
                            u16 texel_idx = (u16)(v_masked & 0xFF00) | ((u8)v_masked | h_hi);
                            u8 texel = xread8(bot_tex_data + texel_idx);
                            u8 pixel = xread8(alis.ptrdark + ((u32)bot_dark << 8) + texel);
                            dst--;
                            xwrite8(dst, pixel);
                            h_coord -= bot_hstep;
                            h_coord = ((h_coord >> 8) & bot_tex_width) << 8 | (h_coord & 0xFF);
                            c--;
                        } while (c != 0);

                        bot_v_coord += tex_vstep;
                        bot_col_frac -= bot_slope_step;
                        bot_screen += stride;
                        bot_rows--;
                    } while (bot_rows != 0);

                    image.tex_hbase += (s16)-0x2000;
                }
                else {
                    // === LTR bottom (Ghidra lines 18341-18378) ===
                    do {
                        u8 cols = (u8)(bot_col_frac >> 8);
                        u16 h_coord = ((image.tex_hbase >> 8) & bot_tex_width) << 8 | (image.tex_hbase & 0xFF);
                        u32 v_masked = (u32)(bot_v_coord & bot_v_mask);
                        u32 dst = bot_screen;

                        u8 c = cols;
                        do {
                            u8 h_hi = (u8)(h_coord >> 8);
                            u16 texel_idx = (u16)(v_masked & 0xFF00) | ((u8)v_masked | h_hi);
                            u8 texel = xread8(bot_tex_data + texel_idx);
                            u8 pixel = xread8(alis.ptrdark + ((u32)bot_dark << 8) + texel);
                            xwrite8(dst, pixel);
                            dst++;
                            h_coord += bot_hstep;
                            h_coord = ((h_coord >> 8) & bot_tex_width) << 8 | (h_coord & 0xFF);
                            c--;
                        } while (c != 0);

                        bot_v_coord += tex_vstep;
                        bot_col_frac -= bot_slope_step;
                        bot_screen += stride;
                        bot_rows--;
                    } while (bot_rows != 0);
                }
            }
        }
    }
}

static void hittest_bar(s32 terrain_cell, s32 render_context, u16 drawy, s16 barheight, s16 barwidth, s32 packed_coord, s16 step_x, s16 step_y)
{
    if (alis.platform.is_little_endian)
    {
        // DOS: clip boundary = dynamic clip Y written by doland_dos to rc-0x246
        // (ASM uses GS:[0x9ac] for both clip and dynamic-Y since they're the same location in DOS)
        s16 clip_boundary = xread16(render_context - 0x246);
        if ((s16)drawy < clip_boundary)
        {
            barheight -= (clip_boundary - (s16)drawy);
            drawy = clip_boundary;
            if (barheight <= 0)
            {
                barlands(drawy, 0, barwidth);
                return;
            }
        }
    }

    if ((s16)drawy <= image.ytstpix && image.ytstpix < (s16)(barheight + drawy) && image.precx <= image.xtstpix && image.xtstpix < (s16)(barwidth + image.precx))
    {
        u16 shift = (u16)xread16(render_context - 0x3c0) & 0x3f;
        image.ntstpix = (u16)(xread16(terrain_cell) >> 8);
        image.cztstpix = (u16)(xread16(terrain_cell) & 0xff);
        image.cxtstpix = (u16)((s16)((u32)packed_coord >> 0x10) * 2 - step_x << shift) >> 1;
        image.cytstpix = (u16)((s16)packed_coord * 2 - step_y << shift) >> 1;
        image.etstpix = 0xfffe;
        image.dtstpix = xread32hi16(render_context - 0x2e0);
    }
    barlands(drawy, barheight, barwidth);
}

// DOS barland: rewritten from Ghidra decompiled rrq-dos.c barland() function.
// Ghidra register-to-parameter mapping:
//   param_1 (AX) = bary (initial screen Y, clipped to drawy)
//   param_2 (DX) = step_y
//   param_3 (CX) = scan_x (terrain scan coordinate, used in ftstpix altitude lookup)
//   unaff_BX = barheight
//   unaff_BP = barx (screen X of bar right edge)
//   unaff_SI = extra_height from doland terrain-type altitude lookup (saved to DAT_0001ab5e)
//   unaff_DI = index (type entry offset into render_context)
//   GS segment = render_context base, FS segment = alis.mem base
// Ghidra DAT mapping:
//   DAT_0001ab66 = image.vbarbot
//   _DAT_0001ab5e = bartra_saved_si (extra_height, set by doland_dos before call)
//   GS:[0x9b6] = xread16(render_context - 0x24e) (bar visibility)
//   GS:[0x9ac] = xread16(render_context - 0x254) (ftstpix clip boundary)
//   GS:[DI+8] = xread8(render_context + 8 + index) (texture check)
//   GS:[0x840] = xread16(render_context - 0x3c0) (zoom shift)
//   GS:[0x922] = xread32hi16(render_context - 0x2e0) (depth/altitude)
static void barland_dos(s32 terrain_cell, s32 render_context, s16 step_x, s16 step_y, s16 bary, s16 barheight, s16 index, s16 barx, s32 packed_coord, s32 d6)
{
    s16 sVar4;
    u16 vbarlarg;

    // bartra_saved_si is set by doland_dos before this call (extra_height from altitude table)

    // === Y clipping (Ghidra lines 17407-17422) ===
    s16 drawy = bary;  // param_1 = AX
    sVar4 = (drawy - image.landclipy1) - image.landcliph;
    if ((image.landcliph <= (u16)(drawy - image.landclipy1) && sVar4 != 0) ||
       ((s16)(-barheight - sVar4) < 0)) {
        if (image.clipy2 < drawy) {
            return;
        }
        sVar4 = barheight + drawy + -1;
        if (sVar4 < (s16)image.landclipy1) {
            return;
        }
        if (drawy < (s16)image.landclipy1) {
            barheight = (barheight - image.landclipy1) + drawy;
            drawy = image.landclipy1;
        }
        if (image.clipy2 < sVar4) {
            barheight = (barheight - sVar4) + image.clipy2;
        }
    }

    // === X clipping (Ghidra lines 17423-17434) ===
    if (image.clipx1 <= barx)
    {
        vbarlarg = barx - image.precx;
        if (vbarlarg == 0 || barx < image.precx || vbarlarg >= 0x80)
            return;

        image.vbarx = image.precx;
        image.vbarlarg = vbarlarg;

        // sVar4 = dynamic clip Y from doland_dos (Ghidra line 17435)
        sVar4 = xread16(render_context - 0x246);
        // DAT_0001ab66 = 0 (Ghidra line 17436)
        image.vbarbot = 0;

        // GS:[0x9b6] >= 0 (visibility check) (Ghidra line 17437)
        if (-1 < xread16(render_context - 0x24e)) {

            if (image.ftstpix == 0) {
                // Normal rendering: GS:[DI+8] texture check (Ghidra lines 17439-17445)
                if (xread8(render_context + 8 + index) != 0) {
                    bartra_dos(terrain_cell, render_context, drawy, index, (s16)vbarlarg, barheight, bary);
                    return;
                }
                barlands(drawy, barheight, (s16)vbarlarg);
                return;
            }

            // === ftstpix hit-test path (Ghidra lines 17449-17465) ===
            if (drawy < sVar4) {
                if ((s16)(drawy + barheight) <= sVar4) {
                    image.vbarbot = 0;
                    return;
                }
                barheight = (barheight + sVar4) - drawy;
                drawy = sVar4;
            }

            if (((drawy <= image.dtstpix) && (image.dtstpix < (s16)(drawy + barheight))) &&
               ((image.precx <= image.xtstpix && (image.xtstpix < (s16)(image.precx + (s16)vbarlarg))))) {

                // Terrain cell reads (DOS LE: byte0=height, byte1=type)
                image.ntstpix = (u16)(xread16(terrain_cell) >> 8);
                image.cztstpix = (u16)(xread16(terrain_cell) & 0xff);

                // GS:[0x840] = zoom shift (Ghidra: bVar3)
                u8 bVar3 = (u8)(xread16(render_context - 0x3c0) & 0x3f);
                image.cxtstpix = (u16)((s16)((u32)packed_coord >> 0x10) * 2 - step_x << (bVar3 & 0x1f)) >> 1;
                image.cytstpix = (u16)((s16)packed_coord * 2 - step_y << (bVar3 & 0x1f)) >> 1;
                image.etstpix = 0xfffe;
                // GS:[0x922] = depth/altitude
                image.dtstpix = xread32hi16(render_context - 0x2e0);
            }
        }
    }
}

// tbarland_dos: DOS-faithful overhang/ceiling bar renderer.
// Matches Ghidra-decompiled tbarland() from rrq-dos.c (lines 17480-17713).
// Key differences from m68k tbarland:
//   1. Inline bottom surface rendering when bothigh <= drawy (narrowing textured slope)
//   2. DOS-simple X clipping (barx - precx, no vbarclipx2)
//   3. bothigh clamped against rc-0x254 (not rc-0x246)
// Register-to-parameter mapping:
//   in_EAX low = bary/drawy, in_ECX = barwidth (from barx - precx)
//   in_DX = barheight (BX in Ghidra), unaff_BP = barx
//   unaff_SI = 0 (doland does SUB SI,SI before calling tbarland), unaff_DI = index
static void tbarland_dos(s32 terrain_cell, s32 render_context, s16 step_x, s16 step_y, s16 bary, s16 barheight, s16 index, s16 barx, s32 packed_coord, s32 d6)
{
    // ASM: MOV [DAT_0001ab5e], SI where SI=0 (SUB SI,SI at doland 0xf21e)
    bartra_saved_si = 0;

    // === Y clipping (Ghidra lines 17514-17532) ===
    s16 drawy = bary;
    s16 sVar4 = (drawy - image.landclipy1) - image.landcliph;
    if ((image.landcliph <= (u16)(drawy - image.landclipy1) && sVar4 != 0) ||
       ((s16)(-barheight - sVar4) < 0)) {
        if (image.clipy2 < drawy)
            return;
        sVar4 = barheight + drawy + -1;
        if (sVar4 < (s16)image.landclipy1)
            return;
        if (drawy < (s16)image.landclipy1) {
            barheight = (barheight - image.landclipy1) + drawy;
            drawy = image.landclipy1;
        }
        if (image.clipy2 < sVar4) {
            barheight = (barheight - sVar4) + image.clipy2;
        }
    }

    // === X clipping (matching m68k tbarland lines 3282-3320) ===
    if (image.clipx1 > barx)
        return;

    u16 vbarlarg = barx - image.precx;
    if (vbarlarg == 0 || barx < image.precx || vbarlarg >= 0x80)
        return;

    image.vbarx = image.precx;
    image.vbarlarg = vbarlarg;

    // === Visibility check (Ghidra line 17536) ===
    if (xread16(render_context - 0x24e) < 0)
        return;

    // === Clip boundary = dynamic clip Y from doland_dos (Ghidra lines 17537-17539) ===
    s16 sVar_clip = xread16(render_context - 0x246);
    if ((s16)bothigh < sVar_clip)
        bothigh = sVar_clip;

    // DAT_0001ab66 = 0 (Ghidra line 17541)
    image.vbarbot = 0;

    // === fbottom handling (Ghidra lines 17543-17679) ===
    if (fbottom != 0) {
        // DAT_0001ab64 = unaff_DI (Ghidra line 17544)
        bottom_type_index = index;

        if ((s16)bothigh <= drawy) {
            // === INLINE BOTTOM SURFACE RENDERING ===
            // Renders a narrowing textured slope for the overhang bottom surface.
            // (Ghidra lines 17546-17674, Assembly LAB_0000dfdb at 0xdfdb-0xe261)

            s32 _precx = (s32)(s16)image.precx;
            u32 stride = (u32)(u16)image.wloglarg;
            u8 shift = xread8(render_context + index);
            u16 tex_hstep = image.tex_hstep;
            u16 tex_vstep = (tex_hstep >> ((8 - shift) & 0x1f)) + 1;

            // Early exit: barheight <= 0 (Ghidra lines 17553-17557)
            if (barheight < 1) {
                image.vbarbot = 0;
                return;
            }

            // Distance for atalias lookup (Ghidra lines 17558-17563)
            u16 distance = botalt - bothigh;
            if (distance == 0 || botalt < (s16)bothigh) {
                image.vbarbot = 0;
                return;
            }

            // Atalias lookup for slope narrowing step (Ghidra lines 17564-17577)
            // Use image.vbarlarg (full unclipped width) for correct slope geometry
            u16 slope_width_t = (image.vbarlarg > 0) ? image.vbarlarg : vbarlarg;
            s16 slope_step;
            if (distance < 0x41) {
                slope_step = xread16(image.atalias + (u16)(((distance - 1) * 0x40 + slope_width_t) - 1) * 2);
            } else {
                distance >>= 2;
                if (0x40 < distance) distance = 0x40;
                slope_step = xread16(image.atalias + (u16)(((distance - 1) * 0x40 + (slope_width_t >> 1)) - 1) * 2) >> 1;
            }

            // Initialize v_coord and col_width (Ghidra lines 17578-17582)
            u16 v_coord = 0;
            u8 col_width = (u8)vbarlarg;
            if ((s16)bothigh < drawy) {
                u16 pre_rows = (u16)(drawy - (s16)bothigh);
                v_coord = pre_rows * tex_vstep;
                col_width -= (u8)((u16)(pre_rows * (u16)slope_step) >> 8);
            }

            // Load texture from type entry (Ghidra lines 17585-17591 / 17632-17637)
            u32 tex_ptr_addr = xread32(render_context + index + 4);
            if (tex_ptr_addr == 0)
                return;
            s32 tex_offset = xread32(tex_ptr_addr);
            u8 tex_width = xread8(tex_ptr_addr + tex_offset + 2);
            u16 tex_height_raw = xread16(tex_ptr_addr + tex_offset + 4);
            u32 tex_data = tex_ptr_addr + tex_offset + 8;

            // Compute darkness (Ghidra lines 17595-17598 / 17641-17644)
            // Formula: fog_byte - 2 * (type_byte >> 6), clamped >= 0
            u8 fog = (u8)((u16)image.vdarkw >> 8);
            u8 type_dark = (xread8(terrain_cell + 1) >> 6) * 2;
            s16 dark_page = (s16)fog - (s16)type_dark;
            if (dark_page < 0) dark_page = 0;

            // Row counter: low byte of barheight (Ghidra line 17599)
            u8 row_count = (u8)barheight;

            // Col fraction: (col_width << 8) | 0xFF (Ghidra line 17600)
            u16 col_frac = ((u16)col_width << 8) | 0xFF;

            // Screen pointer for first row
            u32 screen_base = xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4)) + _precx;

            // Direction check (Ghidra line 17584/17631)
            if (botalt == precboti) {
                // === RTL path: bar narrows from left side (Ghidra lines 17585-17629) ===
                do {
                    u8 cols = (u8)(col_frac >> 8);
                    if (cols > 0) {
                        s16 start_col = (s16)((u8)vbarlarg) - (s16)cols;
                        s16 end_col = (s16)((u8)vbarlarg) - 1;
                        bartra_dos_line(screen_base, start_col, end_col, v_coord, shift,
                                        tex_width, tex_height_raw, 0, tex_data, dark_page, -1);
                    }

                    v_coord += tex_vstep;
                    col_frac -= (u16)slope_step;
                    screen_base += stride;
                    row_count--;
                } while (row_count != 0);
            } else {
                // === LTR path: bar narrows from right side (Ghidra lines 17632-17673) ===
                do {
                    u8 cols = (u8)(col_frac >> 8);
                    if (cols > 0) {
                        bartra_dos_line(screen_base, 0, (s16)cols - 1, v_coord, shift,
                                        tex_width, tex_height_raw, 0, tex_data, dark_page, 1);
                    }

                    v_coord += tex_vstep;
                    col_frac -= (u16)slope_step;
                    screen_base += stride;
                    row_count--;
                } while (row_count != 0);
            }

            return;
        }

        // Not entirely below clip: compute vbarbot (Ghidra lines 17676-17679)
        s16 clip_calc = (barheight + drawy) - (s16)bothigh;
        if (clip_calc != 0 && (s16)bothigh <= (s16)(barheight + drawy)) {
            image.vbarbot = clip_calc;
        }
    }

    // === Normal dispatch (Ghidra lines 17681-17688) ===
    if (image.ftstpix == 0) {
        if (xread8(render_context + 8 + index) != 0) {
            bartra_dos(terrain_cell, render_context, drawy, index, (s16)vbarlarg, barheight, bary);
        } else {
            barlands(drawy, barheight, (s16)vbarlarg);
        }
        return;
    }

    // === ftstpix hit-test path (Ghidra lines 17690-17709) ===
    if (drawy < sVar_clip) {
        if ((s16)(drawy + barheight) <= sVar_clip) {
            return;
        }
        barheight = (barheight + sVar_clip) - drawy;
        drawy = sVar_clip;
    }

    if (((drawy <= image.dtstpix) && (image.dtstpix < (s16)(drawy + barheight))) &&
       ((image.precx <= image.xtstpix && (image.xtstpix < (s16)(image.precx + (s16)vbarlarg))))) {

        image.ntstpix = (u16)(xread16(terrain_cell) >> 8);
        image.cztstpix = (u16)(xread16(terrain_cell) & 0xff);

        u8 bVar3 = (u8)(xread16(render_context - 0x3c0) & 0x3f);
        image.cxtstpix = (u16)((s16)((u32)packed_coord >> 0x10) * 2 - step_x << (bVar3 & 0x1f)) >> 1;
        image.cytstpix = (u16)((s16)packed_coord * 2 - step_y << (bVar3 & 0x1f)) >> 1;
        image.etstpix = 0xfffe;
        image.dtstpix = xread32hi16(render_context - 0x2e0);
    }
}


// =============================================================================
// doland_dos: DOS-specific terrain column renderer
//
// Clean extraction of the DOS (little-endian) rendering path from doland().
// Follows the Ghidra-decompiled program flow from rrq-dos.c:18388-18948.
//
// Key differences from m68k doland():
//   - tex_hbase computed per row from integer grid coords (not once from fractions)
//   - Perspective shifts use <<8/<<8 (not <<6/<<10)
//   - 179-row altitude table (0xB3 segments, not 0x31)
//   - Calls barland_dos()/tbarland_dos() instead of barland()/tbarland()
//   - tex_hstep computed after per-row state reset (matching DOS ASM ordering)
//   - No is_little_endian guards; word-swap writeback, terrain_fill_color,
//     extra height adjust, and wrapped terrain skip are all unconditional
// =============================================================================

void doland_dos(s32 scene_addr, s32 render_context)
{
    // =========================================================================
    // INITIALIZATION (based on working older doland() with DOS improvements)
    // =========================================================================

    // --- Sprite depth sorting setup ---
    image.spritprof = 0x8000;
    sSprite *sprite = SPRITE_VAR((u16)xread16(scene_addr +0x2));
    image.spritnext = sprite->link;
    if (image.spritnext != 0)
    {
        image.spritprof = SPRITE_VAR(image.spritnext)->newd;
    }

    // --- Render sky background ---
    if (xread16(scene_addr +0xa2) != 0)
    {
        skytofen(scene_addr, render_context);
    }

    // --- Terrain data pointers ---
    s32 alt_table = image.atalti;
    s32 dos_alt_delta = 0;  // 32-bit altitude segment delta (avoids s16 overflow for 179 DOS segments)
    s32 terrain_grid = image.atlland;

    // --- Reset overhang interpolation state ---
    fprectop = 0;
    fprectopa = 0;
    adresa = 0;
    prectopa = 1000;
    prectopb = 1000;
    prectopc = 1000;
    precbota = 1000;
    precbotb = 1000;
    precbotc = 1000;

    // --- Initialize rendering accumulators ---
    xwrite32(render_context - 0x25a, 0);
    xwrite32(render_context - 0x27c, 0);

    u16 cam_grid_x = (u16)xread16(render_context - 0x3a2);
    u16 cam_grid_y = (u16)xread16(render_context - 0x3a0);
    xwrite32(render_context - 0x2a8, (u32)cam_grid_x << 0x10);
    xwrite32(render_context - 0x2a4, (u32)cam_grid_y << 0x10);

    // Column step direction in terrain grid (16.16 fixed-point)
    // Word-swapped form used for 68k ADDX carry-chain emulation
    u32 col_step_x_raw = (u32)xread32(render_context - 0x2c4);
    u32 col_step_x = col_step_x_raw << 0x10 | col_step_x_raw >> 0x10;
    u32 col_step_y_raw = (u32)xread32(render_context - 0x2c0);
    u32 col_step_y = col_step_y_raw << 0x10 | col_step_y_raw >> 0x10;

    // NOTE: The original x86 does ROL reg,16; MOV GS:[],reg which writes the word-swapped
    // value back to memory. However, since our code reads col_step_x_raw in native format
    // (for snapping comparisons and trav_x advancement), we must NOT write back the
    // word-swapped value — it would corrupt col_step_x_raw on the next frame.

    // Initial screen Y projection
    xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + xread16(render_context - 0x352)));

    // --- Initial perspective projection ---
    u16 cell_shift = (u16)xread16(render_context - 0x3c0) & 0x3f;
    s16 proj_x = (cam_grid_x << cell_shift) - xread16(render_context - 0x360);
    s16 proj_y = (cam_grid_y << cell_shift) - xread16(render_context - 0x35e);

    glandtopix(render_context, &proj_x, &proj_y,
               (xread16(render_context - 0x378) + proj_x) - xread16(render_context - 0x37e),
               (xread16(render_context - 0x376) + proj_y) - xread16(render_context - 0x37c),
               xread32hi16(render_context - 0x2e0));

    xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + proj_x) << 0x10);
    // DOS: <<8 / <<8 perspective (not <<6 / <<10 like m68k)
    xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(proj_y - proj_x) << 8) / (s32)xread16(render_context - 0x3a4)) << 8);

    // Current traversal position in terrain grid (16.16 fixed-point)
    u32 trav_x = (u32)cam_grid_x << 16;
    u32 trav_y = (u32)cam_grid_y << 16;

    // DOS: initialize tex_hbase once before outer loop
    // It accumulates +0x2000 per column through the entire doland() call.
    image.tex_hbase = ((u16)trav_x + (u16)trav_y) << 4;

    // =========================================================================
    // OUTER LOOP: Process terrain rows from far to near
    // =========================================================================
    do
    {
        // ----- Advance row position accumulators -----
        xwrite32(render_context - 0x2a8, xread32(render_context - 0x2bc) + xread32(render_context - 0x2a8));
        xwrite32(render_context - 0x2a4, xread32(render_context - 0x2b8) + xread32(render_context - 0x2a4));

        u32 row_start_x = trav_x;
        u32 row_start_y = trav_y + (xread32hi16(render_context - 0x29c) << 16);

        // ----- Snap column position to terrain grid alignment -----
        // Safety bound: break after 512 iterations to prevent infinite loops.
        int _snap;
        if (col_step_x_raw == 0x00010000)
        {
            if (xread32hi16(render_context - 0x2bc) < 0)
            {
                for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a8) < (s16)(row_start_x >> 16); _snap--)
                {
                    row_start_y -= col_step_y_raw;
                    row_start_x -= col_step_x_raw;
                }
            }
            else
            {
                for (_snap = 512; _snap > 0 && (s16)(row_start_x >> 16) < xread32hi16(render_context - 0x2a8); _snap--)
                {
                    row_start_y += col_step_y_raw;
                    row_start_x += col_step_x_raw;
                }
            }
        }
        else if (col_step_x_raw == 0xffff0000)
        {
            if (xread32hi16(render_context - 0x2bc) < 0)
            {
                for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a8) < (s16)(row_start_x >> 16); _snap--)
                {
                    row_start_y += col_step_y_raw;
                    row_start_x += col_step_x_raw;
                }
            }
            else
            {
                for (_snap = 512; _snap > 0 && (s16)(row_start_x >> 16) < xread32hi16(render_context - 0x2a8); _snap--)
                {
                    row_start_y -= col_step_y_raw;
                    row_start_x -= col_step_x_raw;
                }
            }
        }
        else
        {
            // Column step is primarily in Y direction
            row_start_y = trav_y;
            row_start_x = trav_x + (xread32hi16(render_context - 0x2a0) << 16);
            if (col_step_y_raw == 0x00010000)
            {
                if (xread32hi16(render_context - 0x2b8) < 0)
                {
                    for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a4) < (s16)(row_start_y >> 16); _snap--)
                    {
                        row_start_y -= col_step_y_raw;
                        row_start_x -= col_step_x_raw;
                    }
                }
                else
                {
                    for (_snap = 512; _snap > 0 && (s16)(row_start_y >> 16) < xread32hi16(render_context - 0x2a4); _snap--)
                    {
                        row_start_y += col_step_y_raw;
                        row_start_x += col_step_x_raw;
                    }
                }
            }
            else
            {
                if (xread32hi16(render_context - 0x2b8) < 0)
                {
                    for (_snap = 512; _snap > 0 && xread32hi16(render_context - 0x2a4) < (s16)(row_start_y >> 16); _snap--)
                    {
                        row_start_y += col_step_y_raw;
                        row_start_x += col_step_x_raw;
                    }
                }
                else
                {
                    for (_snap = 512; _snap > 0 && (s16)(row_start_y >> 16) < xread32hi16(render_context - 0x2a4); _snap--)
                    {
                        row_start_y -= col_step_y_raw;
                        row_start_x -= col_step_x_raw;
                    }
                }
            }
        }

        // ----- Update fog/distance shading -----
        xwrite32(render_context - 0x256, xread32(render_context - 0x252) + xread32(render_context - 0x256));
        image.vdarkw = ((xread32(render_context - 0x256) >> 16) & 0xFF) << 8;

        // ----- Update depth (distance from camera) -----
        s32 depth = xread32(render_context - 0x2dc) + xread32(render_context - 0x2e0);
        xwrite32(render_context - 0x2e0, depth);
        if (depth < 0)
        {
            xwrite32(render_context - 0x2e0, xread32(render_context - 0x2e0) - xread32(render_context - 0x2dc));
        }

        // ----- Update vertical height projection -----
        xwrite32(render_context - 0x2d8, xread32(render_context - 0x2d4) + xread32(render_context - 0x2d8));

        // Prepare column rendering: word-swap for 68k fixed-point carry emulation
        u32 col_x_step_swp = (u32)xread32(render_context - 0x27c) << 0x10 | (u32)xread32(render_context - 0x27c) >> 0x10;
        u32 col_x_pos_swp = (u32)xread32(render_context - 0x280) << 0x10 | (u32)xread32(render_context - 0x280) >> 0x10;

        // DOS: tex_hstep computation (ceiling division)
        // ROL(step, 24) extracts middle bytes for divisor
        {
            s32 raw_step = xread32(render_context - 0x27c);
            u32 rot = ((u32)raw_step >> 8) | ((u32)raw_step << 24);
            u16 divisor = (u16)rot;
            if (divisor > 0)
            {
                u32 quotient = 0x200000 / divisor;
                if (0x200000 % divisor != 0)
                    quotient++; // ceiling
                image.tex_hstep = (u16)quotient;
            }
            else
                image.tex_hstep = 0;
        }

        xwrite32(render_context - 0x278, xread32(render_context - 0x270));

        // ----- Recalculate perspective if row is in front of camera -----
        if (-1 < xread16(render_context - 0x24e))
        {
            proj_x = ((s16)(row_start_x >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x360);
            proj_y = ((s16)(row_start_y >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x35e);
            glandtopix(render_context, &proj_x, &proj_y,
                       (xread16(render_context - 0x378) + proj_x) - xread16(render_context - 0x37e),
                       (xread16(render_context - 0x376) + proj_y) - xread16(render_context - 0x37c),
                       xread32hi16(render_context - 0x2e0));
            xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + proj_x) << 0x10);
            // DOS: <<8 / <<8 perspective
            xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(proj_y - proj_x) << 8) / (s32)xread16(render_context - 0x3a4)) << 8);

            u16 proj_denom = (u16)(xread16(render_context - 0x3a8) + xread32hi16(render_context - 0x2e0));
            if (proj_denom == 0 || scarry2(xread16(render_context - 0x3a8), xread32hi16(render_context - 0x2e0)) != (s32)((u32)proj_denom << 0x10) < 0)
            {
                proj_denom = 1;
            }

            xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + (s16)(xread32(render_context - 0x2d8) / (s32)(s16)proj_denom)));
        }

        // ----- Render sprites at this depth layer -----
        if (xread32hi16(render_context - 0x2e0) <= image.spritprof)
        {
            spritaff(xread32hi16(render_context - 0x2e0));
        }

        // ----- Select altitude table segment for current distance -----
        s32 prev_alt_table = alt_table;
        alt_table += dos_alt_delta;
        u16 alt_seg_idx = (u16)(((xread32(render_context - 0x2e0) - xread32(render_context - 0x2c8)) >> 8) / (s32)xread16(render_context - 0x262));

        // DOS uses 179 segments (0xB3), 512 bytes per segment
        {
            u16 max_seg = 0xb3;

            if (xread16(render_context - 0x24e) == 1)
            {
                alt_seg_idx = max_seg;
            }

            if (max_seg < alt_seg_idx)
            {
                if ((s16)alt_seg_idx < 0)
                {
                    alt_seg_idx = 0;
                }
                else
                {
                    alt_seg_idx = max_seg;
                }
            }

            // 32-bit delta to avoid s16 overflow (179 segs * 0x200 = 91648 > 32767)
            dos_alt_delta = (image.atalti + (s32)alt_seg_idx * 0x200) - alt_table;
        }

        // ----- Reset per-row rendering state -----
        image.precx = image.clipx1 - 0x10;
        xwrite16(render_context - 0x260, image.clipy2);
        xwrite16(render_context - 0x25c, image.clipy2);
        fprectop = 0;
        fprectopa = 0;

        // ----- Decrement row counter -----
        u16 rows_remaining = (u16)xread16(render_context - 0x24e) - 1;
        xwrite16(render_context - 0x24e, rows_remaining);
        if (((s32)((u32)rows_remaining << 0x10) < 0) && (xread16(render_context - 0x24e) < -3))
        {
            spritaff(-1);
            return;
        }

        // =====================================================================
        // INNER LOOP: Process columns left-to-right within this row
        // =====================================================================
        s32 saved_col_x = xread32(render_context - 0x280);
        u32 scan_x = concat22(row_start_x, row_start_x >> 16);
        u32 scan_y = concat22(row_start_y, row_start_y >> 16);

        do
        {
            s16 max_y = xread16(render_context - 0x260);
            s16 prev_max_y = xread16(render_context - 0x25e);
            if (prev_max_y < max_y)
                prev_max_y = max_y;

            s16 bar_screen_x = (s16)col_x_pos_swp;
            s16 col_target_x = xread32hi16(render_context - 0x280);

            // ----- Pre-scan: step through grid cells between columns -----
            while (col_target_x < bar_screen_x)
            {
                xwrite16(render_context - 0x25e, max_y);
                u16 prescan_height = 0;

                // Advance scan position by one column step (with carry)
                u16 next_scan_x = (s16)(col_step_x + scan_x) + (u16)carry4(col_step_x, scan_x);
                scan_x = concat22((s16)((col_step_x + scan_x) >> 0x10), next_scan_x);

                u16 next_scan_y = (s16)(col_step_y + scan_y) + (u16)carry4(col_step_y, scan_y);
                scan_y = concat22((s16)((col_step_y + scan_y) >> 0x10), next_scan_y);

                xwrite32(render_context - 0x280, xread32(render_context - 0x27c) + xread32(render_context - 0x280));

                // Look up terrain cell at (next_scan_x, next_scan_y)
                if ((u16)xread16(render_context - 0x294) < next_scan_x)
                {
                    if (xread8(render_context - 0x3fe) == 1)
                    {
                        s16 wrap_x = 0;
                        if (xread16(render_context - 0x294) <= (s16)next_scan_x)
                        {
                            wrap_x = xread16(render_context - 0x294) * 2;
                        }

                        s32 strip_ptr = xread32(terrain_grid + (s16)((wrap_x - next_scan_x) * 4));
                        u16 grid_height = (u16)xread16(render_context - 0x292);
                        if (grid_height < next_scan_y)
                        {
                            if ((s16)grid_height <= (s16)next_scan_y)
                            {
                                strip_ptr += (s32)xread16(render_context - 0x292) << 2;
                            }

                            adresa = ((strip_ptr - (s16)next_scan_y) - (s32)(s16)next_scan_y);
                        }
                        else
                        {
                            adresa = (strip_ptr + (s16)next_scan_y + (s32)(s16)next_scan_y);
                        }

                        prescan_height = (u16)(xread16((s32)adresa) & 0xff);
                    }
                }
                else
                {
                    // X in bounds: only prescan if wrapping is enabled
                    if (xread8(render_context - 0x3fe) == 1)
                    {
                        if ((u16)xread16(render_context - 0x292) < next_scan_y)
                        {
                            s32 strip_ptr = xread32(terrain_grid + (s16)(next_scan_x * 4));
                            u16 grid_height = (u16)xread16(render_context - 0x292);
                            if ((s16)grid_height <= (s16)next_scan_y)
                            {
                                strip_ptr += (s32)xread16(render_context - 0x292) << 2;
                            }

                            adresa = ((strip_ptr - (s16)next_scan_y) - (s32)(s16)next_scan_y);
                        }
                        else
                        {
                            s32 strip_ptr = xread32(terrain_grid + (s16)(next_scan_x * 4));
                            adresa = (strip_ptr + (s16)next_scan_y + (s32)(s16)next_scan_y);
                        }

                        prescan_height = (u16)(xread16((s32)adresa) & 0xff);
                    }
                }

                // Convert height to screen Y and track maximum
                max_y = xread32lo16(render_context - 0x270) + xread16(alt_table + dos_alt_delta + prescan_height * 2);
                xwrite16(render_context - 0x260, max_y);
                if (prev_max_y < max_y)
                {
                    prev_max_y = max_y;
                }

                col_target_x = xread32hi16(render_context - 0x280);
            }

            // =================================================================
            // Main terrain lookup at center column position
            // =================================================================
            u16 center_grid_x = (u16)(trav_x >> 16);
            u16 center_grid_y = (u16)(trav_y >> 16);
            u32 terrain_cell;
            s32 strip_ptr_main;
            u16 grid_h;
            u8 terrain_wrapped = 0;

            if ((u16)xread16(render_context - 0x294) < center_grid_x)
            {
                if (xread8(render_context - 0x3fe) == 1)
                {
                    terrain_wrapped = 1;
                    s16 wrap_x = 0;
                    if (xread16(render_context - 0x294) <= (s16)center_grid_x)
                    {
                        wrap_x = xread16(render_context - 0x294) * 2;
                    }

                    strip_ptr_main = xread32(terrain_grid + (s16)((wrap_x - center_grid_x) * 4));
                    grid_h = (u16)xread16(render_context - 0x292);
                    if (grid_h < center_grid_y)
                    {
                        goto dos_cell_calc_subtract;
                    }
                    else
                    {
                        goto dos_cell_calc_add;
                    }
                }
            }
            else
            {
                if ((u16)xread16(render_context - 0x292) < center_grid_y)
                {
                    if (xread8(render_context - 0x3fe) != 1)
                        goto dos_advance_column;

                    terrain_wrapped = 1;
                    strip_ptr_main = xread32(terrain_grid + (s16)(center_grid_x << 2));
                    grid_h = (u16)xread16(render_context - 0x292);

                dos_cell_calc_subtract:

                    if ((s16)grid_h <= (s16)center_grid_y)
                    {
                        strip_ptr_main += (s32)xread16(render_context - 0x292) << 2;
                    }

                    terrain_cell = ((strip_ptr_main - (s16)center_grid_y) - (s32)(s16)center_grid_y);
                }
                else
                {
                    strip_ptr_main = xread32(terrain_grid + (s16)(center_grid_x << 2));

                dos_cell_calc_add:

                    terrain_cell = (strip_ptr_main + (s16)center_grid_y + (s32)(s16)center_grid_y);
                }

                // --- Read terrain cell and compute ground screen Y ---
                s16 ground_clip_y = xread16(render_context - 0x25c);
                u16 cell_data = xread16(terrain_cell);
                image.solh = cell_data & 0xff;
                image.solpixy = xread32lo16(render_context - 0x278) + xread16(alt_table + (s16)(image.solh * 2));
                xwrite16(render_context - 0x246, image.solpixy);
                xwrite16(render_context - 0x25c, image.solpixy);
                if (image.solpixy < ground_clip_y)
                {
                    xwrite16(render_context - 0x246, ground_clip_y);
                    ground_clip_y = image.solpixy;
                }

                s16 col_dir_x = (s16)(col_step_x_raw >> 0x10);

                // Extract terrain type index
                s16 terrain_type_idx = ((cell_data & 0x3f00) >> 3) - 0xc00;

                // DOS: set terrain fill color for barlands() gap fill
                terrain_fill_color = xread8(render_context + 8 + terrain_type_idx);

                // DOS: per-terrain-type extra height adjustment from altitude table
                bartra_saved_si = 0;
                {
                    s16 type_extra = xread16(render_context + terrain_type_idx + 0x2);
                    if (type_extra != 0)
                    {
                        bartra_saved_si = xread16(alt_table + type_extra * 2);
                        ground_clip_y += bartra_saved_si;
                    }
                }

                u16 bar_height = prev_max_y - ground_clip_y;

                // --- Render ground terrain bar ---
                if (bar_height != 0 && sborrow2(prev_max_y, ground_clip_y) == (s32)((u32)bar_height << 0x10) < 0)
                {
                    u32 packed_pos = concat22((trav_x), (trav_x >> 16));
                    barland_dos(terrain_cell, render_context, col_dir_x, (s16)(col_step_y_raw >> 0x10), ground_clip_y, bar_height, terrain_type_idx, bar_screen_x, packed_pos, scan_x);
                }

                // =============================================================
                // Check for terrain features: overhangs or billboard sprites
                // =============================================================
                if (xread8(render_context + terrain_type_idx + 0x14) != 0)
                {
                    // DOS: skip overhangs for wrapped terrain cells
                    if (terrain_wrapped)
                        goto dos_advance_column;

                    if ((s8)xread8(render_context + terrain_type_idx + 0x14) < 0)
                    {
                        // =====================================================
                        // OVERHANG / BRIDGE RENDERING
                        // =====================================================
                        notopa = 1;
                        s16 prev_col_grid_x = (s16)(scan_x >> 0x10);
                        s32 oh_packed = concat22(prev_col_grid_x, prectopa);
                        u16 oh_prev_bot = precbota;

                        // --- Check previous column for overhang continuity ---
                        if (adresa != 0)
                        {
                            solha = xread16(adresa) & 0xff;
                            s16 prev_type_idx = ((xread16(adresa) & 0x3f00) >> 3) - 0xc00;

                            if ((s8)xread8(render_context + 0x14 + prev_type_idx) < '\0')
                            {
                                // Previous column also has overhang: compute transition
                                s16 oh_thickness = xread16(render_context + 0x16 + prev_type_idx);
                                u16 oh_ceil_h = (u16)(xread16((s32)adresa + xread32(render_context + 0x10 + prev_type_idx)) & 0xff);
                                u16 oh_delta_h = oh_ceil_h - solha;

                                if ((oh_delta_h != 0 && sborrow2(oh_ceil_h, solha) == (s32)((u32)oh_delta_h << 0x10) < 0) && ((u16)(oh_thickness - oh_delta_h) != 0 && sborrow2(oh_thickness, oh_delta_h) == (s32)((u32)(u16)(oh_thickness - oh_delta_h) << 0x10) < 0))
                                {
                                    u16 oh_bottom_h = oh_ceil_h;
                                    if ((s16)(solha + (u16)(oh_thickness - oh_delta_h)) < (s16)oh_ceil_h)
                                    {
                                        oh_bottom_h = (solha + (u16)(oh_thickness - oh_delta_h)) - solha;
                                    }

                                    // Convert overhang heights to screen Y
                                    s16 oh_top_scr = xread32lo16(render_context - 0x270) + xread16(alt_table + dos_alt_delta + oh_ceil_h * 2);
                                    u16 oh_bot_scr = xread32lo16(render_context - 0x270) + xread16(alt_table + dos_alt_delta + (oh_bottom_h - oh_ceil_h) * -2);
                                    adresa = 0;

                                    if (fprectopa == 0)
                                    {
                                        // First overhang column in this span
                                        fprectopa = 1;
                                        prectopc = 5000;
                                        precbotc = 0xec78;
                                        prectopa = oh_top_scr;
                                        precbota = oh_bot_scr;
                                    }
                                    else
                                    {
                                        // Continuing overhang: update interpolation tracking
                                        prectopb = prectopc;
                                        precbotb = precbotc;
                                        if (prectopa < oh_top_scr)
                                        {
                                            oh_packed = concat22(prev_col_grid_x, oh_top_scr);
                                        }

                                        s16 oh_interp_top = (s16)oh_packed;
                                        if (oh_interp_top < prectopc)
                                        {
                                            oh_packed = concat22((s16)((u32)oh_packed >> 0x10), prectopc);
                                        }

                                        u16 oh_min_bot = precbota;
                                        if ((s16)oh_bot_scr < (s16)precbota)
                                        {
                                            oh_min_bot = oh_bot_scr;
                                        }

                                        oh_prev_bot = oh_min_bot;
                                        if ((s16)precbotc < (s16)oh_min_bot)
                                        {
                                            oh_prev_bot = precbotc;
                                        }

                                        prectopa = oh_top_scr;
                                        precbota = oh_bot_scr;
                                        if ((s16)oh_top_scr > (s16)prectopc)
                                            prectopc = oh_top_scr;
                                        if ((s16)oh_bot_scr < (s16)precbotc)
                                            precbotc = oh_bot_scr;

                                        if ((s16)oh_packed < (s16)oh_prev_bot)
                                        {
                                            notopa = 0;
                                        }
                                    }
                                    goto dos_overhang_render;
                                }
                            }

                            fprectopa = 0;
                        }

                    dos_overhang_render:

                        // --- Look up overhang terrain cell ---
                        terrain_cell = xread32(render_context + 0x10 + terrain_type_idx) + (s32)(s16)center_grid_y + (s32)(s16)center_grid_y + xread32(terrain_grid + (s16)(center_grid_x << 2));
                        fbottom = 0;

                        u16 oh_cell_data = xread16(terrain_cell);
                        image.toph = oh_cell_data & 0xff;
                        s16 oh_type_idx = ((oh_cell_data & 0x3f00) >> 3) - 0xc00;

                        // Overhang height relative to ground
                        u16 oh_rel_height = image.toph - image.solh;
                        u16 oh_thickness_avail = xread16(render_context + 0x16 + terrain_type_idx);

                        if ((oh_rel_height != 0 && sborrow2(image.toph, image.solh) == (s32)((u32)oh_rel_height << 0x10) < 0) && ((u16)(oh_thickness_avail - oh_rel_height) != 0 && sborrow2(oh_thickness_avail, oh_rel_height) == (s32)((u32)(u16)(oh_thickness_avail - oh_rel_height) << 0x10) < 0))
                        {
                            u16 oh_thickness_rem = oh_thickness_avail - oh_rel_height;
                            fbottom = (s16)(image.solh + oh_thickness_rem) < image.toph;
                            if ((bool)fbottom)
                            {
                                oh_rel_height = (image.solh + oh_thickness_rem) - image.solh;
                            }

                            s16 oh_bottom_offset = oh_rel_height - image.toph;
                            u16 oh_top_pix = xread32lo16(render_context - 0x278) + xread16(alt_table + (s16)(image.toph * 2));
                            s16 oh_clip_y = xread16(render_context - 0x246);
                            image.toppixy = oh_top_pix;
                            xwrite16(render_context - 0x246, oh_top_pix);

                            u16 prev_oh_top = prectopi;
                            prectopi = oh_top_pix;
                            if ((s16)oh_top_pix < (s16)prev_oh_top)
                            {
                                xwrite16(render_context - 0x246, prectopi);
                                prev_oh_top = prectopi;
                                oh_top_pix = prectopi;
                            }

                            prectopi = oh_top_pix;
                            if (fbottom != 0)
                            {
                                oh_clip_y = xread32lo16(render_context - 0x278) + xread16(alt_table + (s16)(oh_bottom_offset * -2));
                            }

                            botalt = precboti;
                            bothigh = oh_clip_y;
                            if (precboti < oh_clip_y)
                            {
                                bothigh = precboti;
                                botalt = oh_clip_y;
                            }

                            precboti = oh_clip_y;

                            // --- Render overhang bar ---
                            if (fprectop != 0)
                            {
                                u16 oh_bar_h = botalt - prev_oh_top;
                                s32 oh_bar_height = oh_bar_h;
                                if (oh_bar_h != 0 && sborrow2(botalt, prev_oh_top) == (s32)((u32)oh_bar_h << 0x10) < 0)
                                {
                                    if (notopa == 0)
                                    {
                                        s16 oh_check_top = (s16)oh_packed;
                                        u16 oh_delta = oh_check_top - prev_oh_top;
                                        oh_packed = concat22((s16)((u32)oh_packed >> 0x10), oh_delta);
                                        if (oh_delta == 0 || sborrow2(oh_check_top, prev_oh_top) != (s32)((u32)oh_delta << 0x10) < 0)
                                        {
                                            s16 oh_new_top = prev_oh_top + oh_bar_h;
                                            u16 oh_adj = oh_new_top - oh_prev_bot;
                                            oh_bar_height = oh_adj;
                                            prev_oh_top = oh_prev_bot;
                                            if (oh_adj == 0 || sborrow2(oh_new_top, oh_prev_bot) != (s32)((u32)oh_adj << 0x10) < 0)
                                                goto dos_overhang_done;
                                        }
                                        else if ((s16)oh_bar_h <= (s16)(oh_prev_bot - prev_oh_top))
                                        {
                                            fbottom = 0;
                                            oh_bar_height = oh_delta;
                                        }
                                    }

                                    s16 saved_clip = xread16(render_context - 0x25c);
                                    xwrite16(render_context - 0x25c, prectopi);
                                    u32 packed_pos = concat22((trav_x), (trav_x >> 16));
                                    tbarland_dos(terrain_cell, render_context, col_dir_x, (s16)col_step_y, (s16)prev_oh_top, (s16)oh_bar_height, oh_type_idx, (s16)col_x_pos_swp, packed_pos, oh_packed);
                                    xwrite16(render_context - 0x25c, saved_clip);
                                }
                            }

                        dos_overhang_done:

                            fprectop = 1;
                        }

                        goto dos_advance_column;
                    }

                    if ((s8)xread8(render_context + terrain_type_idx + 0x14) != 1)
                        goto dos_advance_column;

                    // --- Billboard sprite at this terrain cell ---
                    barsprite(render_context, terrain_type_idx, center_grid_x, center_grid_y, (s16)scan_x);
                }

                fprectop = 0;
                fprectopa = 0;
            }

        dos_advance_column:

            image.tex_hbase += 0x2000;

            image.precx = bar_screen_x;
            if (image.landclipx2 < bar_screen_x)
                break;

            // Step traversal to next column
            trav_x += col_step_x_raw;
            trav_y += col_step_y_raw;

            // Advance screen column X (fixed-point with carry emulation)
            s32 col_x_sum = col_x_step_swp + col_x_pos_swp;
            col_x_pos_swp = concat22((s16)((u32)col_x_sum >> 0x10), (s16)col_x_sum + (u16)carry4(col_x_step_swp, col_x_pos_swp));
        }
        while (true);

        // Restore row state for next iteration
        xwrite32(render_context - 0x280, saved_col_x);
        trav_x = row_start_x;
        trav_y = row_start_y;
    }
    while (true);
}

void landtofi_dos(s16 unused, s16 scene_id)
{
    s32 scene_addr = alis.basemain + scene_id;

    // DOS path (Ghidra rrq-dos.c:16411-16452):
    // Only save/restore clipy1 and cliph. clipx1/clipx2/clipl/fen values
    // are NOT modified — they remain from the sprite dirty rect.
    s16 save_cliph = image.cliph;
    s16 save_clipy1 = image.clipy1;

    image.wloglarg = 0x140;  // DOS: hardcoded 320-byte rows (Ghidra line 16418)
    image.wlogic = (u8 *)(alis.mem + xread32(xread32(scene_addr + 0x30)));
    image.wlogx1 = xread16(scene_addr + 0xe);
    image.landclipx2 = image.wlogx1 + xread16(scene_addr + 0x12);
    image.clipy1 = xread16(scene_addr + 0x10);  // override to viewport top
    image.cliph = (image.clipy2 - image.clipy1) + 1;  // Ghidra: 1 - (clipy1 - clipy2)
    image.wlogy1 = image.clipy1;
    image.mapscreen = scene_addr;
    image.landclipy1 = image.clipy1;
    image.landcliph = image.cliph;

    // Tautology — always runs (Ghidra: if (fdoland==0 || fdoland!=0))
    image.fdoland = 0;
    if (((s16)(xread16(scene_addr + 0x12) + 1) == image.clipl) &&
        ((u16)(xread16(scene_addr + 0x14) + 1) == (u16)image.cliph))
    {
        image.fdoland = 1;
    }

    s32 render_context = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));
    calclan0_dos(scene_addr, render_context);

    if (((xread8(scene_addr + 1) & 0x40) == 0) && (xread16(scene_addr + 0xa2) == 0))
    {
        clrvga_dos();
    }

    doland_dos(scene_addr, render_context);
    image.fdoland = 0;

    // Restore only clipy1 and cliph (matching Ghidra)
    image.clipy1 = save_clipy1;
    image.cliph = save_cliph;

    vgatofen();

    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogy1 = image.logy1;
    image.wloglarg = image.loglarg;
}
