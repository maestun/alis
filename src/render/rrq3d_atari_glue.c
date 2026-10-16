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

//
// rrq3d_atari_glue.c — per-call sync between the engine state (image.*/alis.*)
// and the lifted original doland asm's mirror block (doland_mirror in rrq3d_atari.S).
// Contract: tools/asm_lift/entry_contract.md; layout lock: asserts below.
//
#include "config.h"

#if defined(ALIS_RRQ_ASM_DOLAND) && ALIS_RRQ_ASM_DOLAND

#include <stddef.h>
#include <string.h>
#include "alis.h"
#include "image.h"
#include "rrq3d.h"
#include "rrq3d_68k.h"
#include "rrq3d_atari_glue.h"

/* Freeze the asm-visible offsets. If one of these fires, the .equ block in
   rrq3d_atari.S no longer matches — fix BOTH, never just the struct. */
#define RRQ_OFF(f, o) _Static_assert(offsetof(doland_mirror_t, f) == (o), "doland_mirror." #f)
RRQ_OFF(atlland, 0);   RRQ_OFF(atlpix, 4);    RRQ_OFF(atalti, 8);   RRQ_OFF(atalias, 12);
RRQ_OFF(ptrdark, 16);  RRQ_OFF(basesprite, 20); RRQ_OFF(atexsprite, 24); RRQ_OFF(mapscreen, 28);
RRQ_OFF(memfix, 32);
RRQ_OFF(clipx1, 36);   RRQ_OFF(clipy2, 38);   RRQ_OFF(landclipy1, 40); RRQ_OFF(landclipx2, 42);
RRQ_OFF(landcliph, 44); RRQ_OFF(vbarclipx2, 46); RRQ_OFF(wlogy1, 48); RRQ_OFF(wloglarg, 50);
RRQ_OFF(vdarkw, 52);   RRQ_OFF(precx, 54);    RRQ_OFF(spritprof, 56); RRQ_OFF(spritnext, 58);
RRQ_OFF(solh, 60);     RRQ_OFF(solpixy, 62);  RRQ_OFF(toph, 64);    RRQ_OFF(toppixy, 66);
RRQ_OFF(vbarlarg, 68); RRQ_OFF(vbarx, 70);    RRQ_OFF(vbarmid, 72); RRQ_OFF(vbarbot, 74);
RRQ_OFF(ftstpix, 76);  RRQ_OFF(xtstpix, 78);  RRQ_OFF(ytstpix, 80);
RRQ_OFF(cxtstpix, 82); RRQ_OFF(cytstpix, 84); RRQ_OFF(cztstpix, 86);
RRQ_OFF(dtstpix, 88);  RRQ_OFF(ntstpix, 90);  RRQ_OFF(etstpix, 92);
RRQ_OFF(is4bit, 94);
RRQ_OFF(tex_saved_persp, 96); RRQ_OFF(tex_persp, 100); RRQ_OFF(tex_vstep, 104);
RRQ_OFF(tex_hstep, 108);      RRQ_OFF(tex_v, 112);      RRQ_OFF(tex_base, 116);
RRQ_OFF(tex_wmask, 120);      RRQ_OFF(tex_hmask, 122);  RRQ_OFF(tex_dark, 124);
_Static_assert(sizeof(doland_mirror_t) == 128, "doland_mirror size");
#undef RRQ_OFF

/* The lifted asm walks OUR sprite records directly (depth dispatch reads
   state/link/newd/... at raw original offsets). sSprite is packed to the
   original layout — lock the fields the asm touches. */
_Static_assert(sizeof(sSprite) == 0x30, "sSprite stride");
#define SPR_OFF(f, o) _Static_assert(offsetof(sSprite, f) == (o), "sSprite." #f)
SPR_OFF(state, 0x00); SPR_OFF(screen_id, 0x02); SPR_OFF(link, 0x06);
SPR_OFF(newx, 0x0c);  SPR_OFF(newy, 0x0e);     SPR_OFF(newd, 0x10);
SPR_OFF(depx, 0x16);  SPR_OFF(clinking, 0x1e); SPR_OFF(width, 0x28);
SPR_OFF(height, 0x2a); SPR_OFF(newzoomx, 0x2c);
#undef SPR_OFF

static void doland_asm_run(s32 scene_addr, s32 render_context);

void doland_asm(s32 scene_addr, s32 render_context)
{
    /* The asm covers 8- and 4-bit buffers, flat and textured (0x0B) terrain,
       and the ftstpix hit-test (barlapix probe, orig 0x2077c; sprite hits stay
       in C spritaff via spritaff_thunk). ddrawdist needs no gate: openland
       doubles scene 0x6a/0x6c/0x6e, which reach the asm through the rc. */
    if ((alis.platform.bpp != 8 && alis.platform.bpp != 4)
        || (u16)xread16(render_context - 0x294) >= DOLAND_LANDTAB_MAX)
    {
        doland_68k(scene_addr, render_context);
        return;
    }

    /* one-time: build the four line jump tables (idempotent guard inside asm) */
    static u8 tables_built = 0;
    if (!tables_built)
    {
        initltra_asm();
        tables_built = 1;
    }

    doland_asm_run(scene_addr, render_context);
}

static void doland_asm_run(s32 scene_addr, s32 render_context)
{
    /* --- in-sync --- */
    doland_mirror.atlland    = alis.mem + image.atlland;
    doland_mirror.atlpix     = alis.mem + image.atlpix;
    doland_mirror.atalti     = alis.mem + image.atalti;
    doland_mirror.atalias    = alis.mem + image.atalias;
    doland_mirror.ptrdark    = alis.mem + alis.ptrdark;
    /* Sprites do NOT live in the VM arena in this port: record address =
       image.spritemem + image.basesprite + index (SPRITE_VAR). The original's
       basesprite global is exactly that base. */
    doland_mirror.basesprite = image.spritemem + image.basesprite;
    doland_mirror.atexsprite = image.spritemem + image.basesprite + image.atexsprite;
    doland_mirror.mapscreen  = alis.mem + scene_addr;
    doland_mirror.memfix     = (u32)alis.mem;

    /* Both source tables are base + i * stride, so (where, count, first, last) identifies them;
       rebuild only when that changes. */
    static u8 *pix_mem, *land_mem;
    static u32 pix_key[4], land_key[4];

    /* row y -> atlpix[y - wlogy1], the same entries the original indexed (even out of range) */
    u32 pk[4] = { image.atlpix, (u32)(s32)image.wlogy1, xread32(image.atlpix), xread32(image.atlpix + 199 * 4) };
    if (pix_mem != alis.mem || memcmp(pk, pix_key, sizeof(pk)))
    {
        for (s16 y = -64; y < 320; y++)
            doland_pixrows[y + 64] = alis.mem + xread32(image.atlpix + (s16)((y - image.wlogy1) * 4));
        memcpy(pix_key, pk, sizeof(pk));
        pix_mem = alis.mem;
    }

    /* in-bounds cells (x <= rc[-0x294]) read strips through this table */
    u16 n = (u16)xread16(render_context - 0x294);
    u32 lk[4] = { image.atlland, n, xread32(image.atlland), xread32(image.atlland + n * 4) };
    if (land_mem != alis.mem || memcmp(lk, land_key, sizeof(lk)))
    {
        for (u16 i = 0; i <= n; i++)
            doland_landtab[i] = alis.mem + xread32(image.atlland + i * 4);
        memcpy(land_key, lk, sizeof(lk));
        land_mem = alis.mem;
    }

    doland_mirror.clipx1     = image.clipx1;
    doland_mirror.clipy2     = image.clipy2;
    doland_mirror.landclipy1 = image.landclipy1;
    doland_mirror.landclipx2 = image.landclipx2;
    doland_mirror.landcliph  = image.landcliph;
    doland_mirror.vbarclipx2 = image.vbarclipx2;
    doland_mirror.wlogy1     = image.wlogy1;
    doland_mirror.wloglarg   = image.wloglarg;

    doland_mirror.vdarkw     = image.vdarkw;      /* verbatim; asm does its BE high-byte write */
    doland_mirror.precx      = image.precx;
    doland_mirror.spritprof  = (s16)image.spritprof;   /* original width is s16 */
    doland_mirror.spritnext  = (s16)image.spritnext;
    doland_mirror.ftstpix    = image.ftstpix;
    doland_mirror.is4bit     = (alis.platform.bpp == 4);   /* ST/Amiga shade lookup */
    doland_mirror.slowbar    = image.ftstpix || doland_mirror.is4bit || (s8)xread8(render_context - 0x400) == 0x0B;
    doland_mirror.xtstpix    = image.xtstpix;
    doland_mirror.ytstpix    = image.ytstpix;
    if (image.ftstpix)
    {
        /* Seed the hit-test result fields so a no-hit (or partial-hit) traversal
           emits the pointpix defaults (etstpix/ntstpix = -1, dtstpix = the input
           depth) rather than stale mirror values — the asm barlapix only WRITES
           these on an actual hit, exactly like C doland_68k. */
        doland_mirror.etstpix  = (s16)image.etstpix;
        doland_mirror.ntstpix  = (s16)image.ntstpix;
        doland_mirror.dtstpix  = (s16)image.dtstpix;
        doland_mirror.cxtstpix = (s16)image.cxtstpix;
        doland_mirror.cytstpix = (s16)image.cytstpix;
        doland_mirror.cztstpix = (s16)image.cztstpix;
    }

    /* --- run the lifted original --- */
    doland_body_asm(alis.mem + scene_addr, alis.mem + render_context);

    /* --- out-sync --- */
    image.precx     = doland_mirror.precx;
    image.solh      = doland_mirror.solh;
    image.solpixy   = doland_mirror.solpixy;
    image.toph      = doland_mirror.toph;
    image.toppixy   = doland_mirror.toppixy;
    image.vdarkw    = doland_mirror.vdarkw;
    image.spritprof = doland_mirror.spritprof;
    image.spritnext = doland_mirror.spritnext;
    image.vbarlarg  = doland_mirror.vbarlarg;
    image.vbarx     = doland_mirror.vbarx;
    image.vbarmid   = doland_mirror.vbarmid;
    image.vbarbot   = doland_mirror.vbarbot;
    if (image.ftstpix)
    {
        image.cxtstpix = doland_mirror.cxtstpix;
        image.cytstpix = doland_mirror.cytstpix;
        image.cztstpix = doland_mirror.cztstpix;
        image.dtstpix  = doland_mirror.dtstpix;
        image.ntstpix  = doland_mirror.ntstpix;
        image.etstpix  = doland_mirror.etstpix;
    }
}

/* --- thunk backends: called from the asm veneers with alis.mem-relative offsets --- */

void spritaff_thunk(s16 depth)
{
    /* C spritaff reads/writes image.spritprof/spritnext — resync both directions */
    image.spritprof = doland_mirror.spritprof;
    image.spritnext = doland_mirror.spritnext;
    /* Hit-test coherence: asm terrain probe writes doland_mirror, C sprite probe
       (zoomtofen) writes image.*. The original had ONE global set, last writer
       (nearest, far->near traversal) wins — keep both in lockstep so the final
       out-sync yields the nearest hit. antstpix is C-only, survives in image.*. */
    if (image.ftstpix)
    {
        image.etstpix  = (s16)doland_mirror.etstpix;
        image.dtstpix  = (s16)doland_mirror.dtstpix;
        image.ntstpix  = (s16)doland_mirror.ntstpix;
        image.cxtstpix = (s16)doland_mirror.cxtstpix;
        image.cytstpix = (s16)doland_mirror.cytstpix;
        image.cztstpix = (s16)doland_mirror.cztstpix;
    }

    spritaff(depth);
    if (image.ftstpix)
    {
        doland_mirror.etstpix  = (s16)image.etstpix;
        doland_mirror.dtstpix  = (s16)image.dtstpix;
        doland_mirror.ntstpix  = (s16)image.ntstpix;
        doland_mirror.cxtstpix = (s16)image.cxtstpix;
        doland_mirror.cytstpix = (s16)image.cytstpix;
        doland_mirror.cztstpix = (s16)image.cztstpix;
    }

    doland_mirror.spritprof = (s16)image.spritprof;
    doland_mirror.spritnext = (s16)image.spritnext;
}

void barsprite_thunk(s32 render_context, s16 type_idx, s16 world_x, s16 world_y)
{
    /* asm doland just wrote solh (dobar, orig 0x229ca); C barsprite reads
       image.solh. Nothing to sync back. Original 5th arg (zoom scale in D6) is
       recomputed by the C body; pass 0. */
    image.solh = doland_mirror.solh;
    barsprite(render_context, type_idx, world_x, world_y, 0);
}

/* --- Falcon CD textured fill: per-bar constant setup --------------------------
 * Called once per textured bar from Lbartra_tex_setup (blob_c). Mirrors the
 * setup of bartra_textured_68k; kept in C because of the three port-only
 * fixups (shift from width_mask, height 2^n snap, +8 image header). persp comes
 * from the pre-glandtopix rc[-0x27c] the asm saved in tex_saved_persp. */
void bartra_tex_setup(s32 render_context, s32 terrain_cell, s16 index,
                   u16 drawy, s16 barwidth, s16 bary)
{
    s16 max_cols = barwidth - 1;
    s16 row_offset = (s16)drawy - bary;

    /* darkness */
    u16 dark_val = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0)
                 + ((xread16(terrain_cell + 2) >> 8) & 0xc0)
                 + ((xread16(terrain_cell) >> 8) & 0xc0) * 2) * -2;
    u32 dark = (u32)dark_val;
    if ((s32)(dark << 0x10) < 0)
        dark = 0;

    /* texture resolve; tex_table_ptr is an alis.mem offset */
    u32 tex_table_ptr = xread32(render_context + index + 4);
    s32 tex_offset = (s32)xread32(tex_table_ptr);
    u16 width_mask = xread16(tex_table_ptr + tex_offset + 2);

    /* FIXUP 1: shift, derived from width_mask when the field is 0 */
    u8 shift = xread8(render_context + index) & 0x3f;
    if (shift == 0 && width_mask != 0)
    {
        u16 w = width_mask + 1;
        while (w > 1)
        {
            shift++;
            w >>= 1;
        }
    }

    /* persp = 0x200000 / divisor, ROL-packed (as in doland_68k) */
    u32 persp;
    {
        u32 pv = doland_mirror.tex_saved_persp;
        u32 divisor = (pv >> 8) & 0xFFFF;
        if (divisor > 0)
        {
            u16 quot = (u16)(0x200000 / divisor);
            persp = ((u32)(u8)(quot) << 24) | (u8)(quot >> 8);
            if (0x200000 % divisor != 0)
                persp = ((u32)(u8)(quot) << 24) | (u16)((u8)(quot >> 8) + 1);
        }
        else
        {
            persp = 0;
        }
    }

    u32 persp_unrolled = (persp << 8) | (persp >> 24);
    u32 v_step = concat22((s16)(persp >> 16), (s16)persp << shift);

    u16 clip_cols = (u16)(image.vbarlarg - max_cols);
    u32 h_step_raw = persp_unrolled * (u32)clip_cols * 0x10000;
    u32 h_step_per_line = (h_step_raw << 8) | (h_step_raw >> 24);

    u32 v_init_raw = persp_unrolled * (u32)(u16)row_offset * 0x10000;
    u32 V = concat22((s16)((v_init_raw << 8) >> 16),
                     (u16)(u8)(v_init_raw >> 24) << shift);

    /* FIXUP 2: height mask, snapped to 2^n-1 for non-power-of-2 heights */
    s16 type_v_base = xread16(render_context + index + 2);
    s16 hmask_raw = xread16(tex_table_ptr + tex_offset + 4) - type_v_base;
    if (hmask_raw > 0 && (hmask_raw & (hmask_raw + 1)) != 0)
    {
        u16 h = (u16)hmask_raw;
        h |= h >> 1; h |= h >> 2; h |= h >> 4; h |= h >> 8;
        hmask_raw = (s16)(h >> 1);
    }

    s16 height_mask = hmask_raw << shift;

    /* FIXUP 3: loaded resources use the 8-byte header (+8, not CD-native +6) */
    s32 tex_base = tex_table_ptr + ((s32)type_v_base << shift) + tex_offset + 8;

    doland_mirror.tex_persp = persp;
    doland_mirror.tex_vstep = v_step;
    doland_mirror.tex_hstep = h_step_per_line;
    doland_mirror.tex_v     = V;
    doland_mirror.tex_base  = alis.mem + tex_base;
    doland_mirror.tex_wmask = (s16)width_mask;
    doland_mirror.tex_hmask = height_mask;
    doland_mirror.tex_dark  = (s16)dark;
}

#endif /* ALIS_RRQ_ASM_DOLAND */
