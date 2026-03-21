//
//  render3d.h
//  alis
//
//  Created by Vadim Kindl on 04.10.2025.
//  Copyright © 2025 Zlot. All rights reserved.
//

#ifndef render3d_h
#define render3d_h

#include "config.h"
#include "mem.h"

// Shared inline helpers (used by render3d.c, render3d_dos.c, render3d_68k.c)

static inline u16 concat11(u8 a, u8 b) {
    return ((u16)a << 8) | b;
}

static inline u32 concat13(u8 a, u32 b) {
    return ((u32)a << 24) | (b & 0x00ffffff);
}

static inline u32 concat22(u16 a, u16 b) {
    return ((u32)a << 16) | b;
}

static inline u32 carry4(u32 p1, u32 p2) {
    return (p1 + p2) < p1;
}

static inline u8 scarry2(s32 p1, s32 p2) {
    return (p1 + p2) > INT16_MAX || (p1 + p2) < INT16_MIN;
}

static inline u32 concat31(u8 a, u32 b) {
    return ((u32)a << 8) | (b & 0x000000ff);
}

static inline u8 sborrow2(s16 x, s16 y) {
    s16 r = (s16)((s32)x - (s32)y);
    return ((((x ^ y) & (x ^ r)) & 0x8000) != 0);
}

// Read the high 16 bits (integer part) of a 32-bit value stored with xwrite32.
static inline s16 xread32hi16(s32 offset) {
    return (s16)(xread32(offset) >> 16);
}

// Read the low 16 bits of a 32-bit value stored with xwrite32.
static inline s16 xread32lo16(s32 offset) {
    return (s16)xread32(offset);
}

// Shared functions in render3d.c
void openland(s16 scene_id);
void pointpix(s16 x, s16 y);
void landtopix(s32 scene_addr, u16 sprite_id);
void affiland(s32 scene);
void calctoy(s32 scene_addr, s32 render_context);
void spritland(s32 scene_addr, u16 list_head);
void barlands(u16 drawy, s16 barheight, s16 barwidth);
void skytofen(s32 scene_addr, s32 render_context);
void glandtopix(s32 render_context, s16 *out_x, s16 *out_y, s16 offset_x, s16 offset_y, s16 depth);
void barsprite(s32 render_context, s16 type_idx, s16 world_x, s16 world_y, s16 unused);
void spritaff(s16 depth_layer);

// Function pointers — set by render3d_init() based on platform
extern void (*calclan0)(s32 scene_addr, s32 render_context);
extern void (*doland)(s32 scene_addr, s32 render_context);
extern void (*landtofi)(s16 unused, s16 scene_id);
extern void (*clrvga)(void);
extern void (*vgatofen)(void);

void render3d_init(void);

#endif /* render3d_h */
