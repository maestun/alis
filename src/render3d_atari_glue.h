//
// render3d_atari_glue.h — contract between the C side and the lifted original
// Robinson's Requiem renderer asm (render3d_atari.S).
//
// The struct layouts are FROZEN (append-only): the .S addresses fields as
// SYM(mirror)+OFF via hand-maintained .equ blocks; _Static_asserts lock every
// offset (zoom: render3d.c, doland: render3d_atari_glue.c).
// See tools/asm_lift/conversion_notes.md §4.
//
#pragma once

#include "config.h"

#if defined(ALIS_RRQ_ASM_ZOOM) && ALIS_RRQ_ASM_ZOOM

typedef struct {
    /* --- render globals the original read/wrote as abs.l (image.*) --- */
    u8  *wlogic;         /* +0   VGA work buffer base (real ptr)            */
    s32  ztflowx;        /* +4   X fractional accumulator  (in/out)         */
    s32  ztflowy;        /* +8   Y fractional accumulator  (in/out)         */
    /* --- register-contract inputs (C wrapper computes, veneer loads) --- */
    u8  *bitmap;         /* +12  A2 = sprite bitmap ptr                     */
    u32  zoom_y_step;    /* +16  D4 = combined 8.8 Y step                   */
    u32  zoom_x_step;    /* +20  D5 = per-row X step                        */
    s16  wlogy1;         /* +24  top scanline of work buffer                */
    s16  wloglarg;       /* +26  work buffer stride (bytes/row)             */
    s16  screen_x;       /* +28  D2 = clipped left                          */
    s16  screen_y;       /* +30  D0 = clipped top                           */
    s16  newf;           /* +32  D3 = flip flag (sprite->newf)              */
    s16  clipped_width;  /* +34  D6                                         */
    s16  clipped_height; /* +36  D7                                         */
    s16  newy;           /* +38  A4 = sprite->newy (unclipped)              */
    s16  newx;           /* +40  A6 = sprite->newx (unclipped)              */
    s16  bitmap_width;   /* +42  A5 = read16(bitmap+2)+1                    */
    s16  data_offset;    /* +44  6 or 8 — replaces the hardcoded lea +8     */
    s16  pad;            /* +46  keep struct even                           */
    /* --- zoomtofen_full_asm inputs: it resolves + clips the bitmap itself, so
       screen_x/y, clipped_*, newx/y, bitmap, bitmap_width are unused there --- */
    u8  *mem;            /* +48  alis.mem base — newad offset->ptr resolve  */
    s16  clipy1;         /* +52  vertical clip top                          */
    s16  clipy2;         /* +54  vertical clip bottom                       */
    s16  cliph;          /* +56  vertical clip height limit                 */
    s16  clipx1;         /* +58  horizontal clip left                       */
    s16  clipx2;         /* +60  horizontal clip right                      */
    s16  clipl;          /* +62  horizontal clip width limit                */
    u32 *rows;           /* +64  per-row opaque columns (zoom_rows), NULL = draw whole rows */
    /* append new fields BELOW; update .equ block in render3d_atari.S + asserts */
} zoomtofen_mirror_t;

extern zoomtofen_mirror_t zoomtofen_mirror;      /* defined in render3d_atari.S (.bss, .even) */

/* Render-only entry: C did the setup; runs the DDA loops from the register
   contract in zoomtofen_mirror. ztflowx/ztflowy are written back on exit. */
void zoomtofen_render_asm(void);

/* Full entry: bitmap-resolve + format-check + clip + zoom-scale + render for
   the CD-native billboard formats (0x1c/0x1e). A0 = sprite; reads
   mem, clip window, wlogic, wlogy1, wloglarg, ztflow accumulators and
   data_offset from the mirror. Draws nothing (safe return) if the sprite is
   fully clipped or not 0x1c/0x1e. ztflowx/ztflowy are written back on exit. */
void zoomtofen_full_asm(void *sprite);

#endif /* ALIS_RRQ_ASM_ZOOM */

#if defined(ALIS_RRQ_ASM_DOLAND) && ALIS_RRQ_ASM_DOLAND

typedef struct {
    /* real pointers (alis.mem + offset), synced per call */
    u8  *atlland;        /* +0  */
    u8  *atlpix;         /* +4  */
    u8  *atalti;         /* +8  */
    u8  *atalias;        /* +12 */
    u8  *ptrdark;        /* +16 */
    u8  *basesprite;     /* +20 */
    u8  *atexsprite;     /* +24 */
    u8  *mapscreen;      /* +28 */
    u32  memfix;         /* +32  alis.mem, for atlland/atlpix ENTRY fixups (entries are offsets) */
    /* clip / viewport scalars (in) */
    s16  clipx1;         /* +36 */
    s16  clipy2;         /* +38 */
    s16  landclipy1;     /* +40 */
    s16  landclipx2;     /* +42 */
    s16  landcliph;      /* +44 */
    s16  vbarclipx2;     /* +46 */
    s16  wlogy1;         /* +48 */
    s16  wloglarg;       /* +50 */
    /* per-frame scalars (in/out) */
    s16  vdarkw;         /* +52  doland writes its HIGH byte (BE) — copy verbatim both ways */
    s16  precx;          /* +54 */
    s16  spritprof;      /* +56  s16 here, s32 in image.* — glue narrows/widens */
    s16  spritnext;      /* +58 */
    s16  solh;           /* +60 */
    s16  solpixy;        /* +62 */
    s16  toph;           /* +64 */
    s16  toppixy;        /* +66 */
    s16  vbarlarg;       /* +68 */
    s16  vbarx;          /* +70 */
    s16  vbarmid;        /* +72 */
    s16  vbarbot;        /* +74 */
    /* hit-test (in: ftstpix/x/y; out: c*/
    u8   ftstpix;        /* +76 */
    u8   pad0;           /* +77 */
    s16  xtstpix;        /* +78 */
    s16  ytstpix;        /* +80 */
    s16  cxtstpix;       /* +82 */
    s16  cytstpix;       /* +84 */
    s16  cztstpix;       /* +86 */
    s16  dtstpix;        /* +88 */
    s16  ntstpix;        /* +90 */
    s16  etstpix;        /* +92 */
    s16  is4bit;         /* +94  1 if bpp==4 (ST/Amiga one-word shade lookup) */
    /* --- Falcon CD textured (0x0B) fill: per-bar constants computed by the C
       setup helper bartra_tex_setup() and consumed by the lifted asm fill in
       blob_c_textured.inc.S. Mirror the DAT_0001fe7a/7e/84/86/88/8c globals of
       the original CD binary + the tex_base pointer + dark word. --- */
    u32  tex_saved_persp;/* +96   pre-glandtopix rc[-0x27c], saved by asm doland  */
    u32  tex_persp;      /* +100  persp step  (== CD DAT_0001fe7a / tex_persp_step) */
    u32  tex_vstep;      /* +104  per-scanline V step (CD DAT_0001fe7e)            */
    u32  tex_hstep;      /* +108  per-scanline H init (CD DAT_0001fe88)            */
    u32  tex_v;          /* +112  initial V accumulator (CD DAT_0001fe8c)          */
    u8  *tex_base;       /* +116  real ptr = alis.mem + texel base (CD A5)         */
    s16  tex_wmask;      /* +120  width mask  (CD DAT_0001fe84)                     */
    s16  tex_hmask;      /* +122  height mask (CD DAT_0001fe86)                     */
    s16  tex_dark;       /* +124  darkness word; hi byte = (dark & 0xff00)          */
    s16  slowbar;        /* +126  hit-test, textured terrain or 4-bit shades: no barfast */
    /* append new fields BELOW this line only; update .equ block + asserts */
} doland_mirror_t;

extern doland_mirror_t doland_mirror;             /* defined in render3d_atari.S (.bss, .even) */

/* Real pointers to the atlland strips (the asm's a2 base). Size = DL_LANDTAB_MAX in render3d_atari.S. */
#define DOLAND_LANDTAB_MAX 2048
extern u8 *doland_landtab[DOLAND_LANDTAB_MAX];
/* Real row pointers for screen y = -64..319 at [y + 64] (the asm indexes doland_pixtab[y]). */
extern u8 *doland_pixrows[384];

/* asm veneer (render3d_atari.S): loads a2=scene ptr, a3=rc ptr, runs the lifted body */
void doland_body_asm(u8 *scene_ptr, u8 *rc_ptr);

/* asm one-time init: builds the tlinetra/itlinetra/trlinetra/tglinetra tables */
void initltra_asm(void);

/* C entry matching the `doland` function-pointer signature */
void doland_asm(s32 scene_addr, s32 render_context);

/* C backends of the asm thunk veneers in blob_b_doland.inc.S (called with
   alis.mem-relative offsets; they sync the needed doland_mirror<->image fields) */
void spritaff_thunk(s16 depth);
void barsprite_thunk(s32 render_context, s16 type_idx, s16 world_x, s16 world_y);

/* Falcon CD textured fill: per-bar constant setup (called from the asm fill
   veneer Lbartra_tex_setup in blob_c). Reproduces bartra_textured_68k's setup
   exactly (the 3 loaded-resource fixups + memfix + persp divide) into doland_mirror. */
void bartra_tex_setup(s32 render_context, s32 terrain_cell, s16 index,
                   u16 drawy, s16 barwidth, s16 bary);

#endif /* ALIS_RRQ_ASM_DOLAND */
