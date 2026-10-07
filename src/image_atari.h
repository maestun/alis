//
// image_atari.h — m68k planar sprite blitters (image_atari.S).
//
#pragma once

#include "config.h"

// Original "fen" parameter block P (Docs/destofen-port.md §1.2).
typedef struct {
    u16 nchunks;     // +0  chunks drawn across (asm subtracts 1 for the dbf)
    u16 nlines;      // +2  rows - 1 (row dbf counter)
    u16 dst_rowrem;  // +4  BYTES added to dst between rows
    u16 src_hskip;   // +6  clipped-off chunks/row (asm <<4 -> source byte skip)
    u16 src_start;   // +8  start chunk offset into the sprite (kept 0; src pre-offset)
    u16 leftfrac;    // +10 (shifted path)
    u16 rightfrac;   // +12 (shifted path)
    u16 shift;       // +14 destX & 15
    u8  leftclip;    // +16
    u8  rightclip;   // +17
    u16 nplanes;     // +18 active plane count (4/6/8); asm RMWs only this many planes/chunk
} destofen_blk;

// destofen_edge_asm parameter block (same field offsets as destofen_blk).
typedef struct {
    u16 pad0;
    u16 nlines;      // +2  rows - 1
    u16 dst_stride;  // +4  bytes
    u16 prev_stride; // +6  bytes (0 = zero chunk)
    u16 cur_stride;  // +8  bytes (0 = zero chunk)
    u16 mask;        // +10 clip mask
    u16 pad12;
    u16 shift;       // +14
    u16 pad16;
    u16 nplanes;     // +18
} destofen_edge_blk;

void destofen_8bpp_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_8bpp_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_8bpp_opaque_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_8bpp_opaque_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_4bpp_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_4bpp_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_4bpp_opaque_aligned_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_4bpp_opaque_shifted_asm(const destofen_blk *P, const u16 *src, u16 *dst);
void destofen_edge_asm(const destofen_edge_blk *P, const u16 *prev, const u16 *cur, u16 *dst);
void destofen_edge_opaque_asm(const destofen_edge_blk *P, const u16 *prev, const u16 *cur, u16 *dst);

// Up to 16 chunky 8-bit pixels -> one 8-plane chunk at startbit.
void cdestofen_span_asm(const u8 *src, u16 *dst, s32 count, s32 startbit);
void cdestofen_span_opaque_asm(const u8 *src, u16 *dst, s32 count, s32 startbit);

// 8-bit chunky rectangle -> 8-plane interleaved; widths in 16-pixel chunks.
void c2p8_rect(const u8 *src, s32 src_pitch, u8 *dst, s32 dst_pitch, s32 chunks, s32 rows);
