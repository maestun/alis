//
//  render3d.c
//  alis
//
//  Created by Vadim Kindl on 04.10.2025.
//  Copyright © 2025 Zlot. All rights reserved.
//

#include "alis.h"
#include "export.h"
#include "image.h"
#include "mem.h"
#include "render3d.h"


u8 fprectop  = 0;  // overhang: current column has top surface
u8 fprectopa = 0;  // overhang: previous column had top surface
u8 fbottom   = 0;  // overhang: has bottom surface
u8 notopa    = 0;  // overhang: skip top clipping
u16 prectopa = 0;  // overhang top screen Y, column A (previous)
u16 precbota = 0;  // overhang bottom screen Y, column A (previous)
u16 prectopb = 0;  // overhang top screen Y, column B (2 cols back)
u16 precbotb = 0;  // overhang bottom screen Y, column B (2 cols back)
u16 prectopc = 0;  // overhang top screen Y, column C (3 cols back)
u16 precbotc = 0;  // overhang bottom screen Y, column C (3 cols back)
u16 prectopi = 0;  // overhang top screen Y, interpolated
u16 precboti = 0;  // overhang bottom screen Y, interpolated
u16 botalt   = 0;  // overhang bottom altitude
u16 bothigh  = 0;  // overhang bottom high bound
u16 solha    = 0;  // previous column ground height
u32 adresa   = 0;  // previous column terrain cell address

extern u8 tabatan[];
extern s16 *tabsin;
extern s16 *tabcos;

u16 concat11(u8 a, u8 b)
{
    return ((u16)a << 8) | b;
}

u32 concat13(u8 a, u32 b)
{
    return ((u32)a << 24) | (b & 0x00ffffff);
}

u32 concat22(u16 a, u16 b)
{
    return ((u32)a << 16) | b;
}

u32 concat31(u8 a, u32 b)
{
    return ((u32)a << 8) | (b & 0x000000ff);
}

u32 carry4(u32 p1, u32 p2)
{
    return (p1 + p2) < p1;
}

u8 scarry2(s32 p1, s32 p2)
{
    return (p1 + p2) > INT16_MAX || (p1 + p2) < INT16_MIN;
}

u8 sborrow2(s16 x, s16 y)
{
    s16 r = (s16)((s32)x - (s32)y);
    return ((((x ^ y) & (x ^ r)) & 0x8000) != 0);
}

typedef struct {
    u32 top : 16;
    u32 bot : 16;
} S32;

typedef union {
  
    u32 value;
    S32 comp;
} U32;

typedef struct {

    s16 grid_x;
    s16 grid_y;
    s16 scaled_x;
    s16 scaled_y;

} sLSTLResult;

typedef struct {

    s32 cam_x;
    s32 cam_y;
    u32 alt_proj;
    s16 screen_x;
    s16 screen_y;
    s16 depth;
    s16 zoom;

} sCLTPResult;

sLSTLResult lenstoland(s32 scene_addr, s32 render_context, s16 screen_x, s16 screen_y, s16 alt_offset)
{
    // Rotate screen-space offset by yaw angle into world-space grid position
    s16 yaw_idx = xread16(scene_addr + 0x38);
    s32 cosval = tabcos[yaw_idx];
    s32 sinval = tabsin[yaw_idx];

    sLSTLResult result;
    result.grid_x = xread16(scene_addr + 0x16) + (s16)((screen_x * cosval - screen_y * sinval) >> 9);
    result.grid_y = xread16(scene_addr + 0x18) + (s16)((screen_x * sinval + screen_y * cosval) >> 9);

    // Scale grid coordinates by world-to-grid shift factor
    result.scaled_x = result.grid_x >> ((u16)xread16(render_context - 0x3c0) & 0x3f);
    result.scaled_y = result.grid_y >> ((u16)xread16(render_context - 0x3c0) & 0x3f);

    // TODO: why was it in original code???
    // s16 unkn = xread16(scene_addr + 0x1a) + alt_offset;

    return result;
}

sCLTPResult clandtopix(s32 scene_addr, s16 rel_x, s16 rel_y, s16 rel_z)
{
    sCLTPResult result;

    // Rotate world-space deltas into camera-space
    s16 angle = xread16(scene_addr + 0x38);
    s32 cosval = tabcos[angle];
    s32 sinval = tabsin[angle];

    s32 cam_x = ((rel_x * cosval) + (rel_y * sinval)) >> 9;
    s32 cam_y = ((rel_y * cosval) - (rel_x * sinval)) >> 9;

    // Beyond far clip distance
    if (xread16(scene_addr + 0x94) < cam_y)
    {
        result.cam_y = -1;
        result.cam_x = 0;
        result.alt_proj = 0;
        result.screen_x = 0;
        result.screen_y = 0;
        result.depth = -1;
        result.zoom = 0x100;
        return result;
    }

    // Pitch-corrected altitude projection
    s16 pitch = xread16(scene_addr + 0x34);
    s32 pitch_corr = ((s16)cam_y * tabsin[-2 * pitch]) >> 9;
    s32 alt_proj = -(rel_z + pitch_corr);

    // Scale by FOV factors (modifies cam_x and alt_proj in place, stored in result)
    cam_x *= xread16(scene_addr + 0x76);
    alt_proj *= xread16(scene_addr + 0x7a);

    // Perspective divide for screen X (uses 16-bit truncated cam_y)
    s32 x_divisor = (s16)cam_y + xread16(scene_addr + 0x70);
    if (x_divisor == 0 || ((x_divisor > INT16_MAX || x_divisor < INT16_MIN) && x_divisor > 0) || (x_divisor <= INT16_MAX && x_divisor >= INT16_MIN && x_divisor < 0))
    {
        x_divisor = 1;
    }

    s32 screen_x = cam_x / x_divisor;
    if (screen_x < INT16_MIN || screen_x > INT16_MAX)
    {
        screen_x = (screen_x < 0 ? INT16_MIN : INT16_MAX) * (x_divisor < 0 ? -1 : 1);
    }

    // Perspective divide for screen Y and zoom (uses full 32-bit cam_y)
    s32 y_divisor = cam_y + xread16(scene_addr + 0x74);
    if (y_divisor == 0 || ((y_divisor > INT16_MAX || y_divisor < INT16_MIN) && y_divisor > 0) || (y_divisor <= INT16_MAX && y_divisor >= INT16_MIN && y_divisor < 0))
    {
        y_divisor = 1;
    }

    s32 screen_y = alt_proj / y_divisor;
    if (screen_y < INT16_MIN || screen_y > INT16_MAX)
    {
        screen_y = (screen_y < 0 ? INT16_MIN : INT16_MAX) * (y_divisor < 0 ? -1 : 1);
    }

    s32 zoom = (xread16(scene_addr + 0x7a) << 8) / y_divisor;
    if (zoom < INT16_MIN || zoom > INT16_MAX)
    {
        zoom = (zoom < 0 ? INT16_MIN : INT16_MAX) * (y_divisor < 0 ? -1 : 1);
    }

    result.cam_y = cam_y;
    result.cam_x = cam_x;
    result.alt_proj = alt_proj;
    result.screen_x = screen_x;
    result.screen_y = screen_y;
    result.depth = cam_y;
    result.zoom = zoom;
    return result;
}

void calctoy(s32 scene_addr, s32 render_context)
{
    // Compute combined far clip distance
    xwrite16(scene_addr + 0x94, xread16(render_context - 0x3f2) * 2 + xread16(scene_addr + 0x6c));

    // Copy camera position to render context
    xwrite16(render_context - 0x26c, xread16(scene_addr + 0xa));
    xwrite16(render_context - 0x26a, xread16(scene_addr + 0xc));
    xwrite16(render_context - 0x268, xread16(scene_addr + 0x16));
    xwrite16(render_context - 0x266, xread16(scene_addr + 0x18));
    xwrite16(render_context - 0x264, xread16(scene_addr + 0x1a));

    // Precompute trig values for rotation angles
    s16 pitch_idx = xread16(scene_addr + 0x34);
    s16 yaw_idx = xread16(scene_addr + 0x38);
    xwrite16(render_context - 0x28e, tabcos[pitch_idx]);
    xwrite16(render_context - 0x28a, tabcos[yaw_idx]);
    xwrite16(render_context - 0x290, tabsin[pitch_idx]);
    xwrite16(render_context - 0x28c, tabsin[yaw_idx]);

    // Copy camera position to secondary render context slots
    xwrite16(render_context - 0x360, xread16(scene_addr + 0x16));
    xwrite16(render_context - 0x35e, xread16(scene_addr + 0x18));
    xwrite16(render_context - 0x35c, xread16(scene_addr + 0x1a));

    // Project frustum corners to terrain grid (far plane)
    sLSTLResult res;
    res = lenstoland(scene_addr, render_context, (-xread16(render_context - 0x3f4) - (xread16(scene_addr + 0x6a) >> 1)) - xread16(render_context - 0x3f4), xread16(scene_addr + 0x6c), 0);
    xwrite16(render_context - 0x37e, res.grid_x);
    xwrite16(render_context - 0x37c, res.grid_y);
    xwrite16(render_context - 0x3a2, res.scaled_x);
    xwrite16(render_context - 0x3a0, res.scaled_y);

    res = lenstoland(scene_addr, render_context, ((xread16(scene_addr + 0x6a) >> 1) - xread16(render_context - 0x3f4)) - xread16(render_context - 0x3f4), xread16(scene_addr + 0x6c), 0);
    xwrite16(render_context - 0x378, res.grid_x);
    xwrite16(render_context - 0x376, res.grid_y);
    xwrite16(render_context - 0x39c, res.scaled_x);
    xwrite16(render_context - 0x39a, res.scaled_y);

    // Project frustum corners to terrain grid (near plane)
    res = lenstoland(scene_addr, render_context, (-xread16(render_context - 0x3f4) - (xread16(scene_addr + 0x64) >> 1)) - xread16(render_context - 0x3f4), xread16(scene_addr + 0x66), 0);
    xwrite16(render_context - 0x372, res.grid_x);
    xwrite16(render_context - 0x370, res.grid_y);
    xwrite16(render_context - 0x396, res.scaled_x);
    xwrite16(render_context - 0x394, res.scaled_y);

    res = lenstoland(scene_addr, render_context, ((xread16(scene_addr + 0x64) >> 1) - xread16(render_context - 0x3f4)) - xread16(render_context - 0x3f4), xread16(scene_addr + 0x66), 0);
    xwrite16(render_context - 0x36c, res.grid_x);
    xwrite16(render_context - 0x36a, res.grid_y);
    xwrite16(render_context - 0x390, res.scaled_x);
    xwrite16(render_context - 0x38e, res.scaled_y);

    // Project frustum center edges to terrain grid
    res = lenstoland(scene_addr, render_context, -xread16(render_context - 0x3f4), xread16(scene_addr + 0x6c), 0);
    xwrite16(render_context - 0x366, res.grid_x);
    xwrite16(render_context - 0x364, res.grid_y);
    xwrite16(render_context - 0x38a, res.scaled_x);
    xwrite16(render_context - 0x388, res.scaled_y);

    res = lenstoland(scene_addr, render_context, -xread16(render_context - 0x3f4), xread16(scene_addr + 0x66), 0);
    xwrite16(render_context - 0x35a, res.grid_x);
    xwrite16(render_context - 0x358, res.grid_y);
    xwrite16(render_context - 0x384, res.scaled_x);
    xwrite16(render_context - 0x382, res.scaled_y);

    // Grid-to-world shift factor
    u32 grid_shift = (u16)xread16(render_context - 0x3c0);

    // Project grid positions back to screen space (far corners)
    sCLTPResult res2;
    res2 = clandtopix(scene_addr, (xread16(render_context - 0x3a2) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x16), (xread16(render_context - 0x3a0) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x18), -xread16(scene_addr + 0x1a));
    xwrite32(render_context - 0x330, res2.cam_x);
    xwrite32(render_context - 0x32c, res2.cam_y);
    xwrite32(render_context - 0x328, res2.alt_proj);
    xwrite16(render_context - 0x354, res2.screen_x);
    xwrite16(render_context - 0x352, res2.screen_y);
    xwrite16(render_context - 0x350, res2.depth);

    res2 = clandtopix(scene_addr, (xread16(render_context - 0x39c) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x16), (xread16(render_context - 0x39a) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x18), -xread16(scene_addr + 0x1a));
    xwrite32(render_context - 0x324, res2.cam_x);
    xwrite32(render_context - 0x320, res2.cam_y);
    xwrite32(render_context - 0x31c, res2.alt_proj);
    xwrite16(render_context - 0x34e, res2.screen_x);
    xwrite16(render_context - 0x34c, res2.screen_y);
    xwrite16(render_context - 0x34a, res2.depth);

    // Project grid positions back to screen space (near corners)
    res2 = clandtopix(scene_addr, (xread16(render_context - 0x396) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x16), (xread16(render_context - 0x394) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x18), -xread16(scene_addr + 0x1a));
    xwrite32(render_context - 0x318, res2.cam_x);
    xwrite32(render_context - 0x314, res2.cam_y);
    xwrite32(render_context - 0x310, res2.alt_proj);
    xwrite16(render_context - 0x348, res2.screen_x);
    xwrite16(render_context - 0x346, res2.screen_y);
    xwrite16(render_context - 0x344, res2.depth);

    res2 = clandtopix(scene_addr, (xread16(render_context - 0x390) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x16), (xread16(render_context - 0x38e) << (grid_shift & 0x3f)) - xread16(scene_addr + 0x18), -xread16(scene_addr + 0x1a));
    xwrite32(render_context - 0x30c, res2.cam_x);
    xwrite32(render_context - 0x308, res2.cam_y);
    xwrite32(render_context - 0x304, res2.alt_proj);
    xwrite16(render_context - 0x342, res2.screen_x);
    xwrite16(render_context - 0x340, res2.screen_y);
    xwrite16(render_context - 0x33e, res2.depth);

    // Project center grid positions to screen space (fixed altitude)
    res2 = clandtopix(scene_addr, xread16(render_context - 0x366) - xread16(scene_addr + 0x16), xread16(render_context - 0x364) - xread16(scene_addr + 0x18), -0x100);
    xwrite32(render_context - 0x300, res2.cam_x);
    xwrite32(render_context - 0x2fc, res2.cam_y);
    xwrite32(render_context - 0x2f8, res2.alt_proj);
    xwrite16(render_context - 0x33c, res2.screen_x);
    xwrite16(render_context - 0x33a, res2.screen_y);
    xwrite16(render_context - 0x338, res2.depth);

    res2 = clandtopix(scene_addr, xread16(render_context - 0x35a) - xread16(scene_addr + 0x16), xread16(render_context - 0x358) - xread16(scene_addr + 0x18), -0x100);
    xwrite32(render_context - 0x2f4, res2.cam_x);
    xwrite32(render_context - 0x2f0, res2.cam_y);
    xwrite32(render_context - 0x2ec, res2.alt_proj);
    xwrite16(render_context - 0x336, res2.screen_x);
    xwrite16(render_context - 0x334, res2.screen_y);
    xwrite16(render_context - 0x332, res2.depth);

    // Restore far clip distance
    xwrite16(scene_addr + 0x94, xread16(scene_addr + 0x6c) - xread16(render_context - 0x3f2));
}

void altiland(s32 scene_addr, s32 render_context)
{
    // Temporarily adjust clip planes for calctoy
    s16 clip_adjust = xread16(render_context - 0x3f4);
    xwrite16(scene_addr + 0x6c, clip_adjust + xread16(scene_addr + 0x6c));
    xwrite16(scene_addr + 0x66, xread16(scene_addr + 0x66) - clip_adjust);
    calctoy(scene_addr, render_context);
    clip_adjust = xread16(render_context - 0x3f4);
    xwrite16(scene_addr + 0x6c, xread16(scene_addr + 0x6c) - clip_adjust);
    xwrite16(scene_addr + 0x66, clip_adjust + xread16(scene_addr + 0x66));
    xwrite32(render_context - 0x2c8, xread32(render_context - 0x32c) << 0x10);

    // Set up row interpolation parameters
    s32 scale_end = xread32(render_context - 0x2ec);
    s32 scale_start = xread32(render_context - 0x2f8);
    s32 row_scale = xread32(render_context - 0x2f8);
    s32 screen_y_fp = xread32(render_context - 0x32c) << 0x10;
    s32 screen_y_step = (s16)(((xread32(render_context - 0x314) - xread32(render_context - 0x32c)) * 0x100) / 0x31) - 1;
    xwrite16(render_context - 0x262, (s16)screen_y_step);

    u32 row_factor;
    u32 accum;

    u32 alti_ptr = image.atalti;

    // Fill altitude lookup table (0x31 rows x 0xFF columns)
    for (int s = 0; s < 0x31; s++)
    {
        row_factor = (s16)(row_scale / (s32)(s16)(xread16(render_context - 0x3a8) + (s16)((u32)screen_y_fp >> 0x10))) * 0x100;
        xwrite16(alti_ptr, 20000);
        accum = row_factor;
        alti_ptr += 2;

        // Accumulate fixed-point altitude offsets
        for (int i = 0; i < 0xff; i++, alti_ptr += 2)
        {
            accum = (accum << 16) | ((accum >> 16) & 0xffff);      // SWAP.L
            s16 int_part = -(s16)accum;                              // NEG.W
            xwrite16(alti_ptr, int_part);
            accum = (accum & 0xffff0000) | (-int_part & 0xffff);    // restore low word
            accum = (accum << 16) | ((accum >> 16) & 0xffff);      // SWAP.L
            accum += row_factor;                                     // accumulate
//            u16 lw = (u16)(accum >> 0x10);
//            xwrite16(alti_ptr, -lw);
//            accum = row_factor + ((u32)lw << 0x10 | (accum & 0xffff));
        }

        screen_y_fp += screen_y_step * 0x100;

        row_scale += (s16)((scale_end - scale_start) / 0x31);
    }

    // Sentinel row (beyond last visible row)
    for (int i = 0; i < 0xff; i++, alti_ptr += 2)
    {
        xwrite16(alti_ptr, 20000);
    }

    // Compute horizon line position (aspect-corrected from far clip distance)
    image.fhorizon = (s16)(((s16)((s16)(((s16)(xread16(scene_addr + 0x6c) * 4) * 0x168) / 0x140) + 0x80U & 0xff00) * 0x140) / 0x168);
}


void iniland(s32 sceneadr, s32 render_context)
{
    // Compute depth range between far and near planes
    image.purey2 = xread16(sceneadr + 0x6c) - xread16(sceneadr + 0x66);
    image.purey = image.purey2 >> 2;
    image.purey2 >>= 4;

    // Initialize grid wrap/bounds
    xwrite16(render_context - 0x298, 0);
    xwrite16(render_context - 0x296, 0);
    xwrite16(render_context - 0x294, xread16(render_context - 0x3e8));
    xwrite16(render_context - 0x292, xread16(render_context - 0x3e6));

    // Build terrain strip pointer table
    u16 strip_stride = ((u16)xread16(render_context - 0x3ca)) << 1;
    s16 strip_count = xread16(render_context - 0x3cc) - 1;

    if ((s8)xread8(render_context - 1) < 0)
    {
        // Get address of block allocated in calloctab
        render_context = xread32(xread32(render_context));
    }

    strip_count <<= 2;
    for (int i = 0; i <= strip_count; i += 4, render_context += strip_stride)
    {
        xwrite32(image.atlland + i, render_context);
    }

    // Build pixel row pointer table (one entry per scanline)
    u16 row_stride = xread16(sceneadr + 0x12) + 1;
    render_context = xread32(xread32(sceneadr + 0x30));
    for (int i = 0; i <= 800; i += 4, render_context += row_stride)
    {
        xwrite32(image.atlpix + i, render_context);
    }

    // Build anti-aliasing lookup table: alias[d][i] = (i * 256) / d
    u32 alias_ptr = image.atalias;
    for (int d = 1; d <= 0x40; d++)
    {
        for (int i = 0; i < 0x39 + 7; i++, alias_ptr += 2)
        {
            xwrite16(alias_ptr, ((i * 0x100) / d));
        }
    }
}

void inilens(s32 sceneadr, s32 render_context)
{
    // Scene frustum offsets:
    //   0x64 = near_width_x, 0x66 = near_dist, 0x68 = near_height_y
    //   0x6a = far_width_x,  0x6c = far_dist,  0x6e = far_height_y

    // Combined far clip distance
    xwrite16(sceneadr + 0x94, (xread16(sceneadr + 0x6c) - xread16(render_context - 0x3f2)) - xread16(render_context - 0x3f2));

    // Half near distance
    xwrite16(sceneadr + 0x92, xread16(sceneadr + 0x66) >> 1);

    // Focal distance X: (far_dist * near_width - near_dist * far_width) / (far_width - near_width)
    xwrite16(sceneadr + 0x70, (s16)((s32)((u32)(u16)xread16(sceneadr + 0x6c) * (u32)(u16)xread16(sceneadr + 0x64) - (u32)(u16)xread16(sceneadr + 0x66) * (u32)(u16)xread16(sceneadr + 0x6a)) / (s32)(xread16(sceneadr + 0x6a) - xread16(sceneadr + 0x64))));

    // FOV scale X: (focal_x + far_dist) * viewport_width / far_width
    xwrite16(sceneadr + 0x76, (s16)(((s32)(xread16(sceneadr + 0x70) + xread16(sceneadr + 0x6c)) * (s32)(xread16(sceneadr + 0x12) + 1)) / (s32)xread16(sceneadr + 0x6a)));

    // Focal distance Y: (far_dist * near_height - near_dist * far_height) / (far_height - near_height)
    xwrite16(sceneadr + 0x74, (s16)((s32)((u32)(u16)xread16(sceneadr + 0x6c) * (u32)(u16)xread16(sceneadr + 0x68) - (u32)(u16)xread16(sceneadr + 0x66) * (u32)(u16)xread16(sceneadr + 0x6e)) / (s32)(xread16(sceneadr + 0x6e) - xread16(sceneadr + 0x68))));

    // FOV scale Y: (focal_y + far_dist) * viewport_height / far_height
    xwrite16(sceneadr + 0x7a, (s16)(((s32)(xread16(sceneadr + 0x74) + xread16(sceneadr + 0x6c)) * (s32)(xread16(sceneadr + 0x14) + 1)) / (s32)xread16(sceneadr + 0x6e)));

    // Copy focal/scale values to render context
    xwrite16(render_context - 0x3ac, xread16(sceneadr + 0x70));
    xwrite16(render_context - 0x3aa, xread16(sceneadr + 0x76));
    xwrite16(render_context - 0x3a8, xread16(sceneadr + 0x74));
    xwrite16(render_context - 0x3a6, xread16(sceneadr + 0x7a));

    // Column step factor: far_width / half_shift
    xwrite16(render_context - 0x3a4, xread16(sceneadr + 0x6a) / xread16(render_context - 0x3f4));
}

void initltra(void)
{
    // NOTE: not realy needed on anything but real Atari ST
//    for (s32 i = 0; i < 0x100; i += 4)
//    {
//        s32 subidx = (i >> 2) * 2;
//        alis.tlinetra[i + 0] = xswap16(0x59a - subidx);
//        alis.tlinetra[i + 1] = xswap16(0x6b8 - subidx);
//        alis.tlinetra[i + 2] = xswap16(0x7d6 - subidx);
//        alis.tlinetra[i + 3] = xswap16(0x47e - subidx);
//    }
//
//    for (s32 i = 0; i < 0x100; i += 4)
//    {
//        s32 subidx = (i >> 2) * 2;
//        alis.itlinetra[i + 0] = xswap16(0x30c - subidx);
//        alis.itlinetra[i + 1] = xswap16(0x428 - subidx);
//        alis.itlinetra[i + 2] = xswap16(0x546 - subidx);
//        alis.itlinetra[i + 3] = xswap16(0x666 - subidx);
//    }
//
//    for (s32 i = 0; i < 0x100; i += 4)
//    {
//        s32 subidx = (i >> 2) * 2;
//        alis.trlinetra[i + 0] = xswap16(0x302 - subidx);
//        alis.trlinetra[i + 1] = xswap16(0x386 - subidx);
//        alis.trlinetra[i + 2] = xswap16(0x40a - subidx);
//        alis.trlinetra[i + 3] = xswap16(0x27e - subidx);
//    }
//
//    for (s32 i = 1; i < 0x82; i++)
//    {
//        alis.trlinetra[-i] = xswap16(0x280);
//    }
//    
//    for (s32 i = 0; i < 0x100; i += 4)
//    {
//        s32 subidx = (i >> 2) * 2;
//        alis.tglinetra[i + 0] = xswap16(0x302 - subidx);
//        alis.tglinetra[i + 1] = xswap16(0x386 - subidx);
//        alis.tglinetra[i + 2] = xswap16(0x40a - subidx);
//        alis.tglinetra[i + 3] = xswap16(0x27e - subidx);
//    }
//
//    for (s32 i = 1; i < 0x82; i++)
//    {
//        alis.tglinetra[-i] = xswap16(0x280);
//    }
}

void openland(s16 scene_id)
{
    s32 scene_addr = alis.basemain + scene_id;
    s32 render_context = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));

    // Allocate VRAM buffer for scene (width * height + header)
    if (xread32(scene_addr + 0x30) == 0)
    {
        u32 buffer = io_malloc((u32)(u16)(xread16(scene_addr + 0x12) + 1) * (u32)(u16)(xread16(scene_addr + 0x14) + 1) + 0x140);
        xwrite32(scene_addr + 0x30, buffer);
    }

    // Allocate shared pixel, altitude, and alias tables
    if (image.tlpix == 0)
    {
        image.tlpix = io_malloc(0x8720);
        image.atlpix = xread32(image.tlpix);
        image.atalti = image.atlpix + 0x320;
        image.atalias = image.atalti + 0x6400;
    }

    // Allocate terrain strip table
    if (image.tlland == 0)
    {
        image.tlland = io_malloc(xread16(render_context - 0x3cc) * 4);
        image.atlland = xread32(image.tlland);
    }

    // Initialize scene parameters: default altitude and zero rotation angles
    xwrite16(scene_addr + 0x1a, 0x14);
    xwrite16(scene_addr + 0x34, 0);
    xwrite16(scene_addr + 0x36, 0);
    xwrite16(scene_addr + 0x38, 0);

    // Initialize renderer subsystems
    initltra();
    inilens(scene_addr, render_context);
    iniland(scene_addr, render_context);
    altiland(scene_addr, render_context);
    image.landone = 0;
}

void landtopix(s32 scene_addr, u16 sprite_id)
{
    sSprite *sprite = SPRITE_VAR(sprite_id);
    image.newad = sprite->data;

    // Select rotation frame based on camera-relative angle
    if (1 < (s8)sprite->creducing)
    {
        u16 norm_angle;
        for (norm_angle = (sprite->sprite_0x28 - xread16(scene_addr + 0x38)) + 900; -1 < (int)((u32)norm_angle << 0x10); norm_angle -= 0x168) {}

        u32 frame_idx = ((u32)(u16)(s16)(char)sprite->creducing * (u32)(u16)(norm_angle + 0x168) + 0xb4) / 0x168;
        s16 frame_offset = (s16)frame_idx;
        if ((s8)sprite->creducing <= (s8)frame_idx)
        {
            frame_offset = 0;
        }

        image.newad += frame_offset * 4;
    }

    // Resolve bitmap data and metadata pointers
    u32 sprite_hdr = xread32(image.newad) + image.newad;
    u32 bitmap = sprite_hdr;
    u32 meta = image.zoombid;
    if (xread8(sprite_hdr) == 3)
    {
        image.newad += xread16(sprite_hdr + 2) * 4;
        bitmap = xread32(image.newad) + image.newad;
        meta = sprite_hdr;
    }

    // Project sprite position to screen space
    u16 zoom_shift;
    image.newf = xread8(meta + 1) ^ sprite->flaginvx;
    if ((s8)sprite->credon_off == -0x80)
    {
        image.newzoomy = sprite->newzoomy;
        image.newx = xread16(scene_addr + 0xa) + sprite->depx;
        image.newy = xread16(scene_addr + 0xc) - sprite->depz;
        image.newd = sprite->depy;
        zoom_shift = 0;
    }
    else
    {
        sCLTPResult res = clandtopix(scene_addr, sprite->depx - xread16(scene_addr + 0x16), sprite->depy - xread16(scene_addr + 0x18), sprite->depz - xread16(scene_addr + 0x1a));
        image.newzoomy = res.zoom;
        image.newd = res.depth;
        image.newx = xread16(scene_addr + 0xa) + res.screen_x;
        image.newy = xread16(scene_addr + 0xc) + res.screen_y;
        zoom_shift = sprite->credon_off;
    }

    // Apply zoom adjustments
    if ((xread8(bitmap + 1) & 0x20U) != 0)
    {
        image.newzoomy *= 2;
    }

    if (8 < (char)zoom_shift)
    {
        image.newzoomy *= (zoom_shift - 8) >> 3;
        zoom_shift = 3;
    }

    image.newzoomx = image.newzoomy;

    // Fallback for out-of-range bitmap address
    if (0xdfffff < bitmap)
    {
        image.newad = image.landnewad;
        bitmap = image.landdata;
        image.newd = -1000;
        sprite->state = 1;
    }

    zoom_shift = (zoom_shift + 1) & 0x3f;

    // Compute scaled sprite dimensions
    s32 scaled_h = ((((xread16(bitmap + 4) + 1) * image.newzoomy * 2 >> zoom_shift) + 0xff) >> 8) - 1;
    if (scaled_h < 0)
        scaled_h = 0;

    s32 scaled_w = ((((xread16(bitmap + 2) + 1) * image.newzoomx * 2 >> zoom_shift) + 0xff) >> 8) - 1;
    if (scaled_w < 0)
        scaled_w = 0;

    image.newh = (s16)scaled_h;
    image.newl = (s16)scaled_w;

    // Adjust position by hotspot offset
    image.newy += (s16)(((s32)(s16)(((u16)xread16(bitmap + 4) >> 1) - xread16(meta + 6)) * (s32)image.newzoomy >> 8) * -2 >> zoom_shift);
    image.newx += (s16)(((s32)(s16)(((u16)xread16(bitmap + 2) >> 1) - xread16(meta + 4)) * (s32)image.newzoomx >> 8) * -2 >> zoom_shift);
}

// Walk the terrain sprite list: remove dead sprites, project live ones to screen space
void spritland(s32 scene_addr, u16 list_head)
{
    u16 current = list_head;
    u16 prev;

    while (true)
    {
        prev = current;

        sSprite *prev_sprite = SPRITE_VAR(prev);
        current = prev_sprite->link;
        if (current == 0)
            break;

        sSprite *sprite = SPRITE_VAR(current);

        if (sprite->state != 0)
        {
            if (sprite->state == 1)
            {
                // Dead sprite: unlink and return to free list
                prev_sprite->link = sprite->link;
                sprite->to_next = image.libsprit;
                image.libsprit = current;
                current = prev;
            }
            else
            {
                // Active sprite: project to screen and depth-sort into list
                landtopix(scene_addr, current);

                sprite->newad = image.newad;
                sprite->newx = image.newx;
                sprite->newy = image.newy;
                sprite->newd = image.newd;
                sprite->newf = image.newf;
                sprite->width = image.newl;
                sprite->height = image.newh;
                sprite->newzoomx = image.newzoomx;
                sprite->newzoomy = image.newzoomy;

                prev_sprite->link = sprite->link;
                prev = rangesprite(current, prev, list_head);
                current = prev;
            }
        }
    }
}

void calclan0(s32 scene_addr, s32 render_context)
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

    // Dimension scaling delta per strip
    xwrite32(render_context - 0x2b4, ((int)(s16)(((int)(s16)(xread16(render_context - 0x2ac) - xread16(render_context - 0x2b0)) << 8) / (int)xread16(render_context - 0x24e)) << 8) >> (grid_shift & 0x3f));

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

        xwrite32(render_context - 0x252, (int)(s16)(((int)((u32)fog_level * 0x10000 - xread32(render_context - 0x256)) >> 8) / (int)(s16)(xread16(render_context - 0x24e) + 1)) << 8);
    }

    // Per-strip interpolation deltas and initial values
    strip_count = xread16(render_context - 0x24e);

    xwrite32(render_context - 0x2e4, (int)(s16)((xread32(render_context - 0x318) - xread32(render_context - 0x330)) / (int)strip_count));
    xwrite32(render_context - 0x2dc, (int)(s16)(((xread32(render_context - 0x2f0) - xread32(render_context - 0x2fc)) * 0x100) / (int)strip_count) << 8);
    xwrite32(render_context - 0x2d4, (int)(s16)((xread32(render_context - 0x310) - xread32(render_context - 0x328)) / (int)strip_count));
    xwrite32(render_context - 0x2cc, (int)(s16)(((xread32(render_context - 0x2ec) - xread32(render_context - 0x2f8)) * 0x100) / (int)strip_count) << 8);
    xwrite32(render_context - 0x2e8, xread32(render_context - 0x330));
    xwrite32(render_context - 0x2e0, xread32(render_context - 0x32c) << 0x10);
    xwrite32(render_context - 0x2d8, xread32(render_context - 0x328));
    xwrite32(render_context - 0x2d0, xread32(render_context - 0x2f8));
}

void clrvga(void)
{
    u16 width = image.fenx2 - image.fenx1;
    u16 height = image.feny2 - image.feny1;
    u16 w2 = width >> 1;
    s16 loglarg = image.wloglarg - 1;
    u16 *ptr = (u16 *)(image.wlogic + image.fenx1 + (u32)(u16)(image.feny1 - image.wlogy1) * (u32)image.wloglarg);
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < w2; x++)
        {
            *ptr = 0x1010;
            ptr--;
        }

        ptr += loglarg - width;
    }
}

void vgatofen(void)
{
    image.switchgo = 1;
    u8 *src = image.wlogic + (image.clipx1 + (image.clipy1 - image.wlogy1) * image.wloglarg);
    
    image.vgamodulo = image.wloglarg - ((image.clipx2 - image.clipx1) + 1);
    image.bitmodulo = image.loglarg * 2 - ((image.clipx2 - image.clipx1) + 1);
    
    u8 limit = alis.platform.bpp == 4 ? 0xf : 0xff;

    u8 *ptr = image.logic;
    // TODO: why?
    // u8 *ptr = alis.fswitch == 0 ? image.logic : image.physic;
    u8 *tgt = ptr + image.clipx1 + image.clipy1 * image.loglarg * 2;
    for (int y = image.clipy1; y <= image.clipy2; y++, tgt+=image.bitmodulo, src+=image.vgamodulo)
    {
        for (int x = image.clipx1; x <= image.clipx2; x++, tgt++, src++)
        {
            *tgt = *src & limit;
        }
    }
}

u32 skyfast(u32 tgt, u8 fill, s16 stride, s16 height)
{
    for (int l = 0; l < height; l++, tgt += (image.clipl + stride))
        memset(alis.mem + tgt, fill, image.clipl);
    
    return tgt;
}

typedef struct {
    s8 type;
    s8 data1;
    s16 width;
    s16 height;
    u8 color; // Assuming color at offset 6
    u8 *data;
} SkyData;

void skytofen(s32 scene_addr, s32 render_context)
{
    if (image.ftstpix != 0)
    {
        return;
    }

    // Resolve sky bitmap data (follow pointer chain, handle type 3 redirect)
    s32 sky_res = xread32(scene_addr + 0xa4);
    u32 sky_data = xread32(sky_res) + sky_res;

    s16 hotspot_y = 0;
    u32 bitmap = sky_data;
    if (xread8(sky_data) == 3)
    {
        sky_res = ((s32)(s16)(xread16(sky_data + 2) * 4) + (s32)sky_res);
        bitmap = xread32(sky_res) + sky_res;
        hotspot_y = xread16(sky_data + 6);
    }

    if (xread8(bitmap) == 1)
    {
        // Solid color sky fill
        skyfast((s32)image.clipx1 + xread32(image.atlpix + (s16)((image.clipy1 - image.wlogy1) * 4)), (xread8(bitmap + 1) & 0xfU) | (xread8(bitmap + 1) & 0xfU) << 4, image.wloglarg - image.clipl, image.cliph - 1);
    }
    else
    {
        // Compute sky scroll position
        image.skyposy = (hotspot_y + xread16(scene_addr + 0xc) + xread16(render_context - 0x352)) - ((u16)(xread16(bitmap + 4) + 1U) >> 1);
        s16 tex_width = xread16(bitmap + 2) + 1;
        s16 scroll_mod = (s16)((s16)((u32)((s32)xread16(scene_addr + 0x38) * (s32)xread16(scene_addr + 0xa8)) >> 8) - image.clipx1) % tex_width;
        u16 scroll_pos = -scroll_mod;
        if (0 < scroll_mod) {
            scroll_pos = tex_width + scroll_pos;
        }
        image.skyposx = scroll_pos & 0xfffe;
        image.skyleft = tex_width - image.skyposx;
        if ((s16)image.clipl < (s16)(tex_width - image.skyposx)) {
            image.skyleft = image.clipl;
        }

        u32 dst = ((s32)image.clipx1 + xread32(image.atlpix + (s16)((image.clipy1 - image.wlogy1) * 4)));
        s16 stride_gap = image.wloglarg - image.clipl;
        s16 data_offset = alis.platform.bpp == 4 ? 6 : 8;

        // Fill top gap above sky texture
        s16 top_gap = image.skyposy - image.clipy1;
        if (top_gap != 0 && image.clipy1 <= image.skyposy) {
            if (image.cliph < top_gap) {
                top_gap = image.cliph;
            }

            dst = skyfast(dst, xread8(bitmap + data_offset), stride_gap, top_gap - 1);
        }

        // Compute visible sky row range (clamped to clip rect)
        s16 sky_top = image.skyposy;
        if (image.skyposy < image.clipy1) {
            sky_top = image.clipy1;
        }
        s16 sky_bottom = xread16(bitmap + 4) + image.skyposy;
        if (image.clipy2 < (s16)(xread16(bitmap + 4) + image.skyposy)) {
            sky_bottom = image.clipy2;
        }
        s16 row_count = sky_bottom - sky_top;

        // Blit sky texture rows with horizontal wrapping
        if (row_count != 0 && sky_top <= sky_bottom) {
            u16 tex_stride = xread16(bitmap + 2) + 1;
            s32 src_offset = 0;
            if ((u16)(image.clipy1 - image.skyposy) != 0 && image.skyposy <= image.clipy1) {
                src_offset = (u32)(u16)(image.clipy1 - image.skyposy) * (u32)tex_stride;
            }
            u32 row_src = (bitmap + (s16)image.skyposx + src_offset + data_offset);
            s16 half_left = (image.skyleft >> 1) - 1;
            do {
                u32 src_end = row_src;
                u32 src_cur = row_src;
                u32 dst_cur = dst;
                s16 col_count = half_left;

                // Copy left portion (from skyposx to texture edge)
                if (-1 < half_left) {
                    do {
                        src_end = src_cur + 2;
                        dst = dst_cur + 2;
                        xwrite16(dst_cur, xread16(src_cur));
                        col_count = col_count - 1;
                        src_cur = src_end;
                        dst_cur = dst;
                    } while (col_count != -1);
                }

                // Copy wrapped portion (src_end rewinds to texture start, src_cur reused as write cursor)
                s16 wrap_total = (image.clipl >> 1) - half_left;
                s16 wrap_count = wrap_total - 2;
                if (1 < wrap_total) {
                    src_end = ((s32)src_end - (s32)(s16)tex_stride);
                    src_cur = dst;
                    do {
                        dst = src_cur + 2;
                        xwrite16(src_cur, xread16(src_end));
                        wrap_count = wrap_count - 1;
                        src_end = src_end + 2;
                        src_cur = dst;
                    } while (wrap_count != -1);
                }
                dst = ((s32)stride_gap + (s32)dst);
                row_src = ((s32)(s16)tex_stride + (s32)row_src);
            }
            while ((--row_count) != -1);
        }

        // Fill bottom gap below sky texture
        s16 bottom_gap = (image.clipy2 - image.skyposy) - xread16(bitmap + 4);
        if (bottom_gap != 0 && xread16(bitmap + 4) <= (s16)(image.clipy2 - image.skyposy)) {
            if (image.cliph < bottom_gap) {
                bottom_gap = image.cliph;
            }

            skyfast((s32)dst, xread8(bitmap + (u32)(u16)(xread16(bitmap + 2) + 1) * (u32)xread16(bitmap + 4) + data_offset), stride_gap, bottom_gap - 1);
        }
    }
}

void glandtopix(s32 render_context, s16 *out_x, s16 *out_y, s16 offset_x, s16 offset_y, s16 depth)
{
    // Rotate + FOV-scale two points through the same projection
    s32 proj_num_x = (s32)(s16)(((s32)*out_y * (s32)xread16(render_context - 0x28c) + (s32)*out_x * (s32)xread16(render_context - 0x28a)) >> 9) * (s32)xread16(render_context - 0x3aa);
    s32 proj_num_y = (s32)(s16)(((s32)offset_y * (s32)xread16(render_context - 0x28c) + (s32)offset_x * (s32)xread16(render_context - 0x28a)) >> 9) * (s32)xread16(render_context - 0x3aa);

    // Perspective divide
    s16 divisor = xread16(render_context - 0x3ac) + depth;
    if (divisor <= 0)
    {
        divisor = 1;
    }

    *out_x = proj_num_x / divisor;
    *out_y = proj_num_y / divisor;
}

void barlands(void)
{
    return;
}

static inline u32 rot8(u32 x) { return (x << 8) | (x >> 24); }

// draw bar bottom
void bartrab(u32 render_context, s16 maxpixels, s16 vbarbot, s16 botval, s16 vbothigh, u32 color)
{
    s16 pixels;
    
    image.vbarbot = 0;
    s16 lines = vbarbot - 1;
    if (-1 < lines)
    {
        u16 pixmult = botalt - bothigh;
        if ((pixmult < 0x41) && (image.vbarlarg < 0x40))
        {
            pixmult = (u16)xread16(image.atalias + (s16)((image.vbarlarg + (pixmult - 1) * 0x40) * 2));
        }
        else
        {
            pixmult = pixmult >> 2;
            if (0x40 < pixmult)
                pixmult = 0x40;

            pixmult = xread16(image.atalias + (s16)(image.vbarlarg + (pixmult - 1) * 0x80 & 0xfffe)) >> 1;
        }
        
        if (botalt == precboti)
        {
            s32 botf = 0;
            if (vbothigh < (s16)botval)
                botf = ((u16)(botval - vbothigh) & 0xff) * 0x100 * (u32)pixmult;

            u16 topf = image.vbarlarg + (s16)((u32)-botf >> 0x10);
            if (maxpixels < (s16)image.vbarlarg)
                topf = ((maxpixels + image.precx + topf) - image.vbarx) - image.vbarlarg;

            u32 pixfraction = (u32)topf << 0x10 | (-botf & 0xffffU);
            if ((botval & 1) != 0)
                color = rot8(color);
            
            if ((xread8(render_context - 0x24d) & 1) != 0)
                color = rot8(color);

            u32 tgt = (s32)maxpixels + (s32)image.precx + xread32(image.atlpix + (s16)((botval - image.wlogy1) * 4)) + 1;
            u32 prevtgt = tgt;

            do
            {
                pixels = (s16)(pixfraction >> 0x10);
                if (maxpixels < pixels)
                    pixels = maxpixels;

                for (int i = 0; i <= (s16)pixels; i++, tgt--)
                    xwrite8(tgt, (tgt & 1) ? color : (color >> 8));

                pixfraction += (s16)pixmult * -0x100;
                color = rot8(color);

                tgt = (s32)image.wloglarg + prevtgt;
                prevtgt = tgt;
            }
            while ((s32)pixfraction >= 0 && (--lines) != -1);
        }
        else
        {
            s32 botf = 0;
            if (vbothigh < (s16)botval)
                botf = ((u16)(botval - vbothigh) & 0xff) * 0x100 * (u32)pixmult;

            u16 topf = image.vbarlarg + (s16)((u32)-botf >> 0x10);
            if (maxpixels < (s16)image.vbarlarg)
                topf = (image.vbarx + topf) - image.precx;

            u32 pixfraction = (u32)topf << 0x10 | (-botf & 0xffffU);
            if ((botval & 1) != 0)
                color = rot8(color);
            
            if ((xread8(render_context - 0x24d) & 1) != 0)
                color = rot8(color);
            
            u32 tgt = (s32)image.precx + xread32(image.atlpix + (s16)((botval - image.wlogy1) * 4));
            u32 prevtgt = tgt;

            do
            {
                pixels = (s16)(pixfraction >> 0x10);
                if (maxpixels < pixels)
                    pixels = maxpixels;

                for (int i = 0; i <= (s16)pixels; i++, tgt++)
                    xwrite8(tgt, (tgt & 1) ? color : (color >> 8));

                pixfraction += (s16)pixmult * -0x100;
                color = rot8(color);

                tgt = (s32)image.wloglarg + prevtgt;
                prevtgt = tgt;
            }
            while ((s32)pixfraction >= 0 && (--lines) != -1);
        }
    }
}

// draw bar mid
void bartramin(s32 render_context, u32 tgt, s16 lines, s32 maxpixels, u32 color)
{
    u8 *tgtptr = alis.mem + tgt;
    
    u32 tempcolor = color;
    for (int y = 0; y < (s16)lines; y++, tgt += image.wloglarg, tgtptr = alis.mem + tgt)
    {
        for (int i = 0; i < (s16)maxpixels + 2; i++, tempcolor = rot8(tempcolor))
            tgtptr[i] = (tgt & 1) == 0 ? rot8(tempcolor) : tempcolor;
        
        tempcolor = color = rot8(color);
    }
    
    lines = -1;

    if (image.vbarbot == 0)
        return;
    
    bartrab(render_context, maxpixels, image.vbarbot, bothigh, bothigh, color);
}


// draw bar top
void bartra(s32 terrain_cell, s32 render_context, u16 drawy, s16 index, s16 barwidth, s16 barheight, s16 bary)
{
    s16 max_cols = barwidth - 1;

    // Compute dark level from terrain cell data and distance fog
    u16 dark_level = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0) + (xread8(terrain_cell + 2) & 0xc0) + (xread8(terrain_cell) & 0xc0) * 2) * -2;
    u32 color = (u32)dark_level;
    if ((s32)(color << 0x10) < 0)
    {
        color = 0;
    }

    // Look up dithered color pattern from darkness table
    u16 dark_row = (color >> 8);
    if (alis.platform.bpp == 4)
    {
        color = (u16)xread16(alis.ptrdark + (s16)concat31(dark_row, (s8)xread8(render_context + 8 + index) * 2));
    }
    else
    {
        color = concat11(xread8(alis.ptrdark + (s16)concat31(dark_row, xread8(render_context + 8 + index))),
                         xread8(alis.ptrdark + (s16)concat31(dark_row, xread8(render_context + 9 + index))));
    }

    color = concat22(color, color);
    s16 top_lines = xread16(render_context - 0x246) - drawy;
    s16 mid_height = barheight;

    if (top_lines > 1)
    {
        mid_height -= top_lines;
        if (barheight < top_lines)
        {
            top_lines += mid_height;
        }

        if (top_lines > 0)
        {
            u32 dst;
            u32 prev_dst;
            u8 *dst_ptr;

            u32 saved_color;
            u32 alt_color;

            u32 tex_frac;
            u16 tex_step = xread16(render_context - 0x246) - bary;
            image.vbarmid = mid_height;

            if (1 < (s16)tex_step)
            {
                if ((tex_step < 0x41) && (image.vbarlarg < 0x40))
                {
                    tex_step = (u16)xread16(image.atalias + (s16)((image.vbarlarg + (tex_step - 1) * 0x40) * 2));
                }
                else
                {
                    tex_step >>= 2;
                    if (0x40 < tex_step)
                    {
                        tex_step = 0x40;
                    }

                    tex_step = xread16(image.atalias + (s16)(image.vbarlarg + (tex_step - 1) * 0x80 & 0xfffe)) >> 1;
                }

                // Left-to-right rendering path
                if (xread16(render_context - 0x246) == xread16(render_context - 0x25c))
                {
                    tex_frac = 0;
                    if (bary < (s16)drawy)
                    {
                        tex_frac = ((u16)(drawy - bary) & 0xff) * 0x100 * (u32)tex_step;
                    }

                    if ((s16)max_cols < (s16)image.vbarlarg)
                    {
                        tex_frac = (u32)(u16)((image.vbarx + (s16)(tex_frac >> 0x10)) - image.precx) << 0x10 | (tex_frac & 0xffff);
                    }

                    if ((drawy & 1) != 0)
                    {
                        color = rot8(color);
                    }

                    if ((xread8(render_context - 0x24d) & 1) != 0)
                    {
                        color = rot8(color);
                    }

                    dst = ((s32)image.precx + xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4)));

                    u16 col_count = tex_frac >> 0x10;
                    alt_color = rot8(color);
                    prev_dst = (u32)dst;

                    do
                    {
                        dst_ptr = alis.mem + dst;
                        saved_color = alt_color;

                        if ((s16)max_cols < (s16)col_count || (--top_lines) == -1)
                        {
                            if (top_lines < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, dst, image.vbarmid, max_cols, color);
                                }
                                return;
                            }

                            if (-1 < (s16)col_count)
                            {
                                col_count = max_cols;
                                alt_color = saved_color;
                                continue;
                            }

                            if ((--top_lines) < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, dst, image.vbarmid, max_cols, color);
                                }
                                return;
                            }
                        }
                        else
                        {
                            for (int i = 0; i < (s16)col_count + 1; i++, color = rot8(color))
                                dst_ptr[i] = (dst & 1) == 0 ? rot8(color) : color;
                        }

                        tex_frac += (s16)tex_step * 0x100;
                        dst = ((s32)image.wloglarg + prev_dst);
                        col_count = tex_frac >> 0x10;
                        alt_color = color;
                        color = saved_color;
                        prev_dst = (u32)dst;
                    }
                    while (true);
                }
                // Right-to-left rendering path
                else
                {
                    tex_frac = 0;
                    if (bary < (s16)drawy)
                    {
                        tex_frac = ((u16)(drawy - bary) & 0xff) * 0x100 * (u32)tex_step;
                    }

                    if ((s16)max_cols < (s16)image.vbarlarg)
                    {
                        tex_frac = (u32)(u16)(((max_cols + image.precx + (s16)(tex_frac >> 0x10)) - image.vbarx) - image.vbarlarg) << 0x10 | (tex_frac & 0xffff);
                    }

                    if ((drawy & 1) != 0)
                    {
                        color = rot8(color);
                    }

                    if ((xread8(render_context - 0x24d) & 1) != 0)
                    {
                        color = rot8(color);
                    }

                    alt_color = rot8(color);
                    u16 col_count = tex_frac >> 0x10;

                    dst = (s32)(s16)max_cols + (s32)image.precx + xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4)) + 1;
                    prev_dst = dst;

                    do
                    {
                        dst_ptr = alis.mem + dst;
                        saved_color = alt_color;

                        if ((s16)max_cols < (s16)col_count || (--top_lines) == -1)
                        {
                            if (top_lines < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, (dst - 1) - max_cols, image.vbarmid, max_cols, color);
                                }
                                return;
                            }

                            if (-1 < (s16)col_count)
                            {
                                col_count = max_cols;
                                alt_color = saved_color;
                                continue;
                            }

                            if ((--top_lines) < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, (dst - 1) - max_cols, image.vbarmid, max_cols, color);
                                }
                                return;
                            }
                        }
                        else
                        {
                            xwrite8(dst - 1, (char)color);
                            for (int i = 0; i < (s16)col_count; i++, color = rot8(color))
                                dst_ptr[-i] = (dst & 1) == 0 ? rot8(color) : color;

                            saved_color = color;
                        }

                        tex_frac += (s16)tex_step * 0x100;
                        dst = (s32)image.wloglarg + prev_dst;
                        col_count = tex_frac >> 0x10;
                        alt_color = color;
                        color = saved_color;
                        prev_dst = dst;
                    }
                    while (true);
                }
            }
        }
    }

    if (-1 < (s16)(mid_height - 1))
    {
        if ((drawy & 1) != 0)
        {
            color = rot8(color);
        }

        if ((xread8(render_context - 0x24d) & 1) != 0)
        {
            color = rot8(color);
        }

        u32 dst = (s32)image.precx + xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4));
        bartramin(render_context, dst, mid_height, max_cols, color);
    }
}

// Returns false if the bar is fully outside the Y clip region
static bool clip_bar_y(s16 bary, s16 *barheight, u16 *drawy)
{
    s16 barclipy = (bary - image.landclipy1) - image.landcliph;
    *drawy = bary;
    if (((u16)image.landcliph <= (u16)(bary - image.landclipy1) && barclipy != 0) || ((s16)(-*barheight - barclipy) < 0))
    {
        if (image.clipy2 < bary)
            return false;

        s16 barbot = bary + *barheight - 1;
        if (barbot < (s16)image.landclipy1)
            return false;

        if (bary < (s16)image.landclipy1)
        {
            *barheight = bary + (*barheight - image.landclipy1);
            *drawy = image.landclipy1;
        }

        if (image.clipy2 < barbot)
        {
            *barheight = image.clipy2 + (*barheight - barbot);
        }
    }
    return true;
}

static void hittest_bar(s32 terrain_cell, s32 render_context, u16 drawy, s16 barheight, s16 barwidth, s32 packed_coord, s16 step_x, s16 step_y)
{
    if ((s16)drawy <= image.ytstpix && image.ytstpix < (s16)(barheight + drawy) && image.precx <= image.xtstpix && image.xtstpix < (s16)(barwidth + image.precx))
    {
        u16 shift = (u16)xread16(render_context - 0x3c0) & 0x3f;
        image.ntstpix = (u16)xread8(terrain_cell);
        image.cztstpix = (u16)xread8(terrain_cell + 1);
        image.cxtstpix = (u16)((s16)((u32)packed_coord >> 0x10) * 2 - step_x << shift) >> 1;
        image.cytstpix = (u16)((s16)packed_coord * 2 - step_y << shift) >> 1;
        image.etstpix = 0xfffe;
        image.dtstpix = xread16(render_context - 0x2e0);
    }
    barlands();
}

void barland(s32 terrain_cell, s32 render_context, s16 step_x, s16 step_y, s16 bary, s16 barheight, s16 index, s16 barx, s32 packed_coord, s32 d6)
{
    s16 prev_screen_x = image.precx;

    // Clip bar vertically
    u16 drawy;
    if (!clip_bar_y(bary, &barheight, &drawy))
        return;

    if (image.clipx1 <= barx)
    {
        // Compute horizontal span from previous cursor to current bar edge
        image.vbarlarg = (barx - image.precx) - 1;
        image.vbarx = image.precx;
        if (image.precx < image.clipx1)
        {
            image.precx = image.clipx1;
        }

        // Clip bar width against right boundary
        s16 barwidth;
        if (image.vbarclipx2 < barx)
        {
            barwidth = -(image.precx - image.vbarclipx2);
            if (barwidth == 0 || 0 < (s16)(image.precx - image.vbarclipx2))
            {
                barlands();
                return;
            }
        }
        else
        {
            barwidth = -(image.precx - barx);
            if (barwidth == 0 || 0 < (s16)(image.precx - barx))
            {
                image.vbarx = prev_screen_x;
                return;
            }
        }

        // Render or hit-test the bar
        if (-1 < xread16(render_context - 0x24e))
        {
            if (image.ftstpix == 0)
            {
                bartra(terrain_cell, render_context, drawy, index, barwidth, barheight, bary);
            }
            else
            {
                hittest_bar(terrain_cell, render_context, drawy, barheight, barwidth, packed_coord, step_x, step_y);
            }
        }
    }
}


void tbarland(s32 terrain_cell, s32 render_context, s16 step_x, s32 step_y, u16 bary, s16 barheight, s16 index, s32 screen_x, s32 packed_coord, s32 d6)
{
    s16 prev_screen_x = image.precx;

    // Clip bar vertically
    u16 drawy;
    if (!clip_bar_y((s16)bary, &barheight, &drawy))
        return;

    s16 barx = (s16)screen_x;
    if (image.clipx1 <= barx)
    {
        // Compute horizontal span from previous cursor to current bar edge
        image.vbarlarg = (barx - image.precx) - 1;
        image.vbarx = image.precx;
        if (image.precx < image.clipx1)
        {
            image.precx = image.clipx1;
        }

        // Clip bar width against right boundary
        s16 barwidth;
        if (image.vbarclipx2 < barx)
        {
            barwidth = -(image.precx - image.vbarclipx2);
            if (barwidth == 0 || 0 < (s16)(image.precx - image.vbarclipx2))
            {
                image.vbarx = prev_screen_x;
                return;
            }
        }
        else
        {
            barwidth = -(image.precx - barx);
            if (barwidth == 0 || 0 < (s16)(image.precx - barx))
            {
                image.vbarx = prev_screen_x;
                return;
            }
        }

        if (-1 < xread16(render_context - 0x24e))
        {
            // Clamp bottom high-water mark
            if (bothigh < xread16(render_context - 0x246))
            {
                bothigh = xread16(render_context - 0x246);
            }

            image.vbarbot = 0;
            if (fbottom != 0)
            {
                // Bar entirely below bottom clip — fill with solid dark color
                if (bothigh <= (s16)drawy)
                {
                    barwidth --;
                    u16 dark_level = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0) + (xread8(terrain_cell + 2) & 0xc0) + (xread8(terrain_cell) & 0xc0) * 2) * -2;
                    u32 color = (u32)dark_level;
                    if ((s32)((u32)dark_level << 0x10) < 0)
                    {
                        color = 0;
                    }

                    u16 dark_row = (color >> 8);
                    if (alis.platform.bpp == 4)
                    {
                        dark_level = (u16)xread16(alis.ptrdark + (s16)concat31(dark_row, (s8)xread8(render_context + 8 + index) * 2));
                    }
                    else
                    {
                        dark_level = concat11(xread8(alis.ptrdark + (s16)concat31(dark_row, xread8(render_context + 8 + index))),
                                              xread8(alis.ptrdark + (s16)concat31(dark_row, xread8(render_context + 9 + index))));
                    }
                    color = concat22(dark_level, dark_level);

                    bartrab(render_context, barwidth, barheight, drawy, bothigh, color);
                    return;
                }

                // Clip bar height against bottom boundary
                s16 clip_calc = (barheight + drawy) - bothigh;
                if (clip_calc != 0 && bothigh <= (s16)(barheight + drawy))
                {
                    image.vbarbot = clip_calc;
                }
            }

            // Render or hit-test the bar
            if (image.ftstpix == 0)
            {
                bartra(terrain_cell, render_context, drawy, index, barwidth, barheight - image.vbarbot, bary);
            }
            else
            {
                hittest_bar(terrain_cell, render_context, drawy, barheight, barwidth, packed_coord, step_x, (s16)step_y);
            }
        }
    }
}

// Renders a zoomed 2D sprite to the framebuffer with clipping and optional collision detection
void zoomtofen(sSprite *sprite)
{
    // Get sprite bitmap data with offset stored in header
    u8 *bitmap = alis.mem + sprite->newad + xread32(sprite->newad);

    u8 data_offset = (*bitmap == 0x18 || *bitmap == 0x1a) ? 6 : 8;
    
    // Only process valid sprite formats (0x18, 0x1a, 0x1c, 0x1e)
    if (*bitmap == 0x18 || *bitmap == 0x1a || *bitmap == 0x1c || *bitmap == 0x1e)
    {
        // Initialize clipped dimensions (add 1 because dimensions are 0-based)
        s16 clipped_width = sprite->width + 1;
        u16 clipped_height = sprite->height + 1;

        // Calculate zoom scale factor (fixed-point: 8.8 format)
        // Formula: (bitmap_height << 8) / clipped_height
        u32 zoom_y_scale = ((u32)(u16)(read16(bitmap + 4) + 1) << 8) / (u32)clipped_height;
        u32 zoom_y_int = (u32)(s16)zoom_y_scale;
        u32 zoom_y_frac = (zoom_y_int << 8) >> 0x10;
        u32 zoom_y_step = zoom_y_int << 0x18 | zoom_y_frac;  // Combined step value for tight inner loop

        // Clip sprite against vertical boundaries
        s16 y_extent = (sprite->newy - image.clipy1) - image.cliph;
        s16 screen_y = sprite->newy;
        if ((image.cliph <= (u16)(sprite->newy - image.clipy1) && y_extent != 0) || ((s16)(-clipped_height - y_extent) < 0))
        {
            if (image.clipy2 < sprite->newy)
                return;  // Sprite entirely below clipping area

            y_extent = sprite->newy + clipped_height - 1;
            if (y_extent < image.clipy1)
                return;  // Sprite entirely above clipping area

            // Clip top edge if above clipping boundary
            if (sprite->newy < image.clipy1)
            {
                clipped_height = sprite->newy + (clipped_height - image.clipy1);
                screen_y = image.clipy1;
            }

            // Clip bottom edge if below clipping boundary
            if (image.clipy2 < y_extent)
            {
                clipped_height = image.clipy2 + (clipped_height - y_extent);
            }
        }

        // Clip sprite against horizontal boundaries
        s16 x_orig = sprite->newx;
        s16 x_extent = (x_orig - image.clipx1) - image.clipl;
        s16 screen_x = x_orig;
        if ((image.clipl <= (u16)(x_orig - image.clipx1) && x_extent != 0) || ((s16)(-clipped_width - x_extent) < 0))
        {
            if (image.clipx2 < x_orig)
                return;  // Sprite entirely right of clipping area

            x_extent = x_orig + clipped_width - 1;
            if (x_extent < image.clipx1)
                return;  // Sprite entirely left of clipping area

            // Clip left edge if left of clipping boundary
            if (x_orig < image.clipx1)
            {
                clipped_width = x_orig + (clipped_width - image.clipx1);
                screen_x = image.clipx1;
            }

            // Clip right edge if right of clipping boundary
            if (image.clipx2 < x_extent)
            {
                clipped_width = image.clipx2 + (clipped_width - x_extent);
            }
        }

        // Calculate zoom X scale (bitmap width in pixels)
        s16 bitmap_width = read16(bitmap + 2) + 1;
        u32 zoom_x_step = concat22((s16)((zoom_y_int << 0x18) >> 0x10), bitmap_width * (char)(zoom_y_scale >> 8));

        // Standard rendering path (not collision test mode)
        if (image.ftstpix == 0)
        {
            // Calculate target framebuffer address
            char *framebuffer = (char *)((s32)screen_x + (u32)(u16)(screen_y - image.wlogy1) * (u32)image.wloglarg + image.wlogic);
            image.ztflowy &= 0xffff;

            // Calculate initial Y texture offset for clipped sprite
            u16 y_offset = screen_y - sprite->newy;
            u32 texture_y = (u32)y_offset;
            if (y_offset != 0)
            {
                // Accumulate Y texture coordinate with zoom factor
                texture_y = (u32)y_offset * ((zoom_y_frac & 0xff) << 8 | (zoom_y_int & 0xff));
                image.ztflowy = concat13((char)texture_y, image.ztflowy >> 0x8);
                texture_y = (texture_y >> 8 & 0xffff) * (u32)(u16)(read16(bitmap + 2) + 1);
            }

            bool carry_flag;

            // Rendering path for horizontally flipped sprites
            if ((sprite->newf & 1) != 0)
            {
                // Start from right edge of bitmap for flipped rendering
                s32 bitmap_row_offset = (u16)read16(bitmap + 2) + texture_y;
                image.ztflowx = concat22(0xffff, image.ztflowx >> 0x10);

                // Calculate X texture offset accounting for clipping on left edge
                if ((u16)(screen_x - x_orig) != 0)
                {
                    zoom_y_frac = (u32)(u16)(screen_x - x_orig) * ((zoom_y_frac & 0xff) << 8 | (zoom_y_int & 0xff));
                    image.ztflowx = concat13(-1 - (char)zoom_y_frac, image.ztflowx >> 0x8);
                    bitmap_row_offset -= (zoom_y_frac >> 8);
                }

                s16 line_skip = image.wloglarg - clipped_width;
                zoom_y_frac = image.ztflowy;

                // Render pixels row by row
                for (int y = 0; y < clipped_height; y++, framebuffer += line_skip)
                {
                    zoom_y_int = concat22((s16)(image.ztflowx >> 0x10), (s16)zoom_y_frac);

                    // Render individual pixels in row, stepping backward through texture (flipped)
                    for (int x = 0; x < clipped_width; x++, framebuffer++, zoom_y_int = concat22((s16)((zoom_y_int - zoom_y_step) >> 0x10), (s16)(zoom_y_int - zoom_y_step) - (u16)(zoom_y_int < zoom_y_step)))
                    {
                        u8 pixel = bitmap[(s16)zoom_y_int + bitmap_row_offset + data_offset];
                        if (pixel != 0)  // Skip transparent pixels
                            *framebuffer = pixel;
                    }

                    // Step to next texture row with fixed-point arithmetic
                    carry_flag = carry4(zoom_x_step, zoom_y_frac);
                    zoom_y_frac = zoom_x_step + zoom_y_frac;
                    if (carry_flag)  // Handle fractional overflow to next line
                    {
                        zoom_y_frac = concat22((s16)(zoom_y_frac >> 0x10), bitmap_width + (s16)zoom_y_frac);
                    }
                }
            }
            // Rendering path for normal (non-flipped) sprites
            else
            {
                image.ztflowx &= 0xffff;

                // Calculate X texture offset accounting for clipping on left edge
                if ((u16)(screen_x - x_orig) != 0)
                {
                    zoom_y_frac = (u32)(u16)(screen_x - x_orig) * ((zoom_y_frac & 0xff) << 8 | (zoom_y_int & 0xff));
                    image.ztflowx = concat13((char)zoom_y_frac, image.ztflowx >> 0x8);
                    texture_y += (zoom_y_frac >> 8);
                }

                s16 line_skip = image.wloglarg - clipped_width;
                zoom_y_frac = image.ztflowy;

                // Render pixels row by row
                for (int y = 0; y < clipped_height; y++, framebuffer += line_skip)
                {
                    zoom_y_int = concat22((s16)(image.ztflowx >> 0x10), (s16)zoom_y_frac);

                    // Render individual pixels in row, stepping forward through texture (normal)
                    for (int x = 0; x < clipped_width; x++, framebuffer++, zoom_y_int = concat22((s16)((zoom_y_step + zoom_y_int) >> 0x10), (s16)(zoom_y_step + zoom_y_int) + (u16)carry4(zoom_y_step, zoom_y_int)))
                    {
                        u8 pixel = bitmap[(s16)zoom_y_int + texture_y + data_offset];
                        if (pixel != 0)  // Skip transparent pixels
                            *framebuffer = pixel;
                    }

                    // Step to next texture row with fixed-point arithmetic
                    carry_flag = carry4(zoom_x_step, zoom_y_frac);
                    zoom_y_frac += zoom_x_step;
                    if (carry_flag)  // Handle fractional overflow to next line
                    {
                        zoom_y_frac = concat22((s16)(zoom_y_frac >> 0x10), bitmap_width + (s16)zoom_y_frac);
                    }
                }
            }
        }
        // Collision detection path (pixel-perfect hit testing)
        else if (screen_y <= image.ytstpix && image.ytstpix < (s16)(clipped_height + screen_y) && screen_x <= image.xtstpix && image.xtstpix < (s16)(clipped_width + screen_x))
        {
            // Calculate sprite-space coordinates of the test point
            u32 test_x = (u32)(u16)(image.xtstpix - x_orig) * ((zoom_y_frac << 8 & 0xffff) | (zoom_y_int & 0xff)) >> 8;
            s32 test_y = ((u32)(u16)(image.ytstpix - sprite->newy) * ((zoom_y_frac << 8 & 0xffff) | (zoom_y_int & 0xff)) >> 8 & 0xffff) * (u32)(u16)(read16(bitmap + 2) + 1U);

            // Mirror coordinates if sprite is flipped
            if (sprite->newf != 0)
            {
                test_y = (s16)(read16(bitmap + 2) + 1U) + test_y - 1;
                test_x = -test_x;
            }

            // Check if hit point is non-transparent in the sprite bitmap
            if ((bitmap[test_x + test_y + 8] != 0) && (xread32(sprite->script_ent + alis.atent) != 0))
            {
                // Record hit info for entity script callback
                image.cxtstpix = sprite->depx;
                image.cytstpix = sprite->depy;
                image.cztstpix = sprite->depz;
                image.etstpix = sprite->script_ent;
                image.antstpix = sprite->newad;
                image.dtstpix = sprite->newd;
            }
        }
    }
}

// Renders a zoomed 2D sprite to the framebuffer using floating-point arithmetic
// Same functionality as zoomtofen but with improved precision using floats
void zoomtofenf(sSprite *sprite)
{
    // Get sprite bitmap data with offset stored in header
    u8 *bitmap = alis.mem + sprite->newad + xread32(sprite->newad);

    // Only process valid sprite formats (0x18, 0x1a, 0x1c, 0x1e)
    if (*bitmap == 0x18 || *bitmap == 0x1a || *bitmap == 0x1c || *bitmap == 0x1e)
    {
        // Initialize clipped dimensions (add 1 because dimensions are 0-based)
        s16 clipped_width = sprite->width + 1;
        u16 clipped_height = sprite->height + 1;

        // Calculate zoom scale factors using floating-point arithmetic
        float bitmap_height = (float)(read16(bitmap + 4) + 1);
        float bitmap_width = (float)(read16(bitmap + 2) + 1);
        float zoom_y_scale = bitmap_height / (float)clipped_height;
        float zoom_x_scale = bitmap_width / (float)clipped_width;

        // Clip sprite against vertical boundaries
        s16 y_extent = (sprite->newy - image.clipy1) - image.cliph;
        s16 screen_y = sprite->newy;
        if ((image.cliph <= (u16)(sprite->newy - image.clipy1) && y_extent != 0) || ((s16)(-clipped_height - y_extent) < 0))
        {
            if (image.clipy2 < sprite->newy)
                return;  // Sprite entirely below clipping area

            y_extent = sprite->newy + clipped_height - 1;
            if (y_extent < image.clipy1)
                return;  // Sprite entirely above clipping area

            // Clip top edge if above clipping boundary
            if (sprite->newy < image.clipy1)
            {
                clipped_height = sprite->newy + (clipped_height - image.clipy1);
                screen_y = image.clipy1;
            }

            // Clip bottom edge if below clipping boundary
            if (image.clipy2 < y_extent)
            {
                clipped_height = image.clipy2 + (clipped_height - y_extent);
            }
        }

        // Clip sprite against horizontal boundaries
        s16 x_orig = sprite->newx;
        s16 x_extent = (x_orig - image.clipx1) - image.clipl;
        s16 screen_x = x_orig;
        if ((image.clipl <= (u16)(x_orig - image.clipx1) && x_extent != 0) || ((s16)(-clipped_width - x_extent) < 0))
        {
            if (image.clipx2 < x_orig)
                return;  // Sprite entirely right of clipping area

            x_extent = x_orig + clipped_width - 1;
            if (x_extent < image.clipx1)
                return;  // Sprite entirely left of clipping area

            // Clip left edge if left of clipping boundary
            if (x_orig < image.clipx1)
            {
                clipped_width = x_orig + (clipped_width - image.clipx1);
                screen_x = image.clipx1;
            }

            // Clip right edge if right of clipping boundary
            if (image.clipx2 < x_extent)
            {
                clipped_width = image.clipx2 + (clipped_width - x_extent);
            }
        }

        // Standard rendering path (not collision test mode)
        if (image.ftstpix == 0)
        {
            // Calculate target framebuffer address
            char *framebuffer = (char *)((s32)screen_x + (u32)(u16)(screen_y - image.wlogy1) * (u32)image.wloglarg + image.wlogic);

            // Calculate initial texture Y coordinate accounting for clipping
            float texture_y = (float)(screen_y - sprite->newy) * zoom_y_scale;

            // Rendering path for horizontally flipped sprites
            if ((sprite->newf & 1) != 0)
            {
                s16 line_skip = image.wloglarg - clipped_width;

                // Render pixels row by row
                for (int y = 0; y < clipped_height; y++, framebuffer += line_skip)
                {
                    // Calculate X texture coordinate at left edge (accounting for clipping)
                    float texture_x = ((float)(screen_x - x_orig) * zoom_x_scale);

                    // For flipped sprites, mirror the X coordinate
                    texture_x = bitmap_width - 1.0f - texture_x;

                    // Render individual pixels in row, stepping backward through texture (flipped)
                    for (int x = 0; x < clipped_width; x++, framebuffer++)
                    {
                        int px = (int)texture_x;
                        int py = (int)texture_y;

                        // Clamp to bitmap boundaries
                        if (px >= 0 && px < (int)bitmap_width && py >= 0 && py < (int)bitmap_height)
                        {
                            u8 pixel = bitmap[py * (int)bitmap_width + px + 8];
                            if (pixel != 0)  // Skip transparent pixels
                                *framebuffer = pixel;
                        }

                        texture_x -= zoom_x_scale;  // Step backward through texture
                    }

                    texture_y += zoom_y_scale;  // Step to next texture row
                }
            }
            // Rendering path for normal (non-flipped) sprites
            else
            {
                s16 line_skip = image.wloglarg - clipped_width;

                // Render pixels row by row
                for (int y = 0; y < clipped_height; y++, framebuffer += line_skip)
                {
                    // Calculate X texture coordinate at left edge (accounting for clipping)
                    float texture_x = ((float)(screen_x - x_orig) * zoom_x_scale);

                    // Render individual pixels in row, stepping forward through texture (normal)
                    for (int x = 0; x < clipped_width; x++, framebuffer++)
                    {
                        int px = (int)texture_x;
                        int py = (int)texture_y;

                        // Clamp to bitmap boundaries
                        if (px >= 0 && px < (int)bitmap_width && py >= 0 && py < (int)bitmap_height)
                        {
                            u8 pixel = bitmap[py * (int)bitmap_width + px + 8];
                            if (pixel != 0)  // Skip transparent pixels
                                *framebuffer = pixel;
                        }

                        texture_x += zoom_x_scale;  // Step forward through texture
                    }

                    texture_y += zoom_y_scale;  // Step to next texture row
                }
            }
        }
        // Collision detection path (pixel-perfect hit testing)
        else if (screen_y <= image.ytstpix && image.ytstpix < (s16)(clipped_height + screen_y) && screen_x <= image.xtstpix && image.xtstpix < (s16)(clipped_width + screen_x))
        {
            // Calculate sprite-space coordinates of the test point
            float test_x_f = (float)(image.xtstpix - x_orig) * zoom_x_scale;
            float test_y_f = (float)(image.ytstpix - sprite->newy) * zoom_y_scale;

            int test_x = (int)test_x_f;
            int test_y = (int)test_y_f;

            // Mirror coordinates if sprite is flipped
            if (sprite->newf != 0)
            {
                test_x = (int)bitmap_width - 1 - test_x;
            }

            // Check if hit point is within bitmap and non-transparent
            if (test_x >= 0 && test_x < (int)bitmap_width && test_y >= 0 && test_y < (int)bitmap_height)
            {
                if ((bitmap[test_y * (int)bitmap_width + test_x + 8] != 0) && (xread32(sprite->script_ent + alis.atent) != 0))
                {
                    // Record hit info for entity script callback
                    image.cxtstpix = sprite->depx;
                    image.cytstpix = sprite->depy;
                    image.cztstpix = sprite->depz;
                    image.etstpix = sprite->script_ent;
                    image.antstpix = sprite->newad;
                    image.dtstpix = sprite->newd;
                }
            }
        }
    }
}

// Render a billboard sprite at a terrain cell position
void barsprite(s32 render_context, s16 type_idx, s16 world_x, s16 world_y, s16 unused)
{
    sSprite *sprite = SPRITE_VAR(image.atexsprite);
    u32 tex_entry = xread32(render_context + 0x10 + type_idx);
    if (xread32(tex_entry) != 0)
    {
        // Project terrain position to screen space
        sCLTPResult result = clandtopix(image.mapscreen, (world_x << (xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x268), (world_y << (xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x266), image.solh - xread16(render_context - 0x264));
        s16 sprite_depth = result.depth;
        if (sprite_depth < xread16(image.mapscreen + 0x92))
        {
            sprite_depth = -1;
        }

        sprite->newx = xread16(render_context - 0x26c) + (s16)result.screen_x;
        sprite->newy = xread16(render_context - 0x26a) + (s16)result.screen_y;
        sprite->newd = sprite_depth;
        sprite->newzoomx = result.zoom;
        sprite->newzoomy = result.zoom;

        if (-1 < sprite_depth)
        {
            sprite->script_ent = xread16(render_context + 0x1c + type_idx);
            sprite->credon_off = xread8(render_context + 0x15 + type_idx);

            // Resolve bitmap and metadata pointers
            u32 bitmap_base = xread32(tex_entry) + tex_entry;
            u32 meta = image.zoombid;
            u32 bitmap = bitmap_base;
            if ((s8)xread8(bitmap) == 3)
            {
                tex_entry += xread16(bitmap + 2) * 4;
                bitmap = xread32(tex_entry) + tex_entry;
                meta = bitmap_base;
            }

            sprite->newad = tex_entry;
            sprite->newf = xread8(meta + 1);

            // Compute zoom shift from detail level
            u8 detail_level = sprite->credon_off;
            if ((xread8(bitmap + 1) & 0x20) == 0)
            {
                detail_level += 1;
            }

            u32 detail_shift = (u32)detail_level;

            // Compute scaled sprite dimensions
            s32 sprite_height = ((s32)(((u32)((s32)(s16)(xread16(bitmap + 4) + 1) * (s32)sprite->newzoomy) >> 8) * 2) >> (detail_shift & 0x3f)) - 1;
            if (sprite_height < 0)
            {
                sprite_height = 0;
            }

            s32 sprite_width_calc = ((s32)(((u32)((s32)(s16)(xread16(bitmap + 2) + 1) * (s32)sprite->newzoomx) >> 8) * 2) >> (detail_shift & 0x3f)) - 1;
            s16 sprite_width = (s16)sprite_width_calc;
            if (sprite_width_calc < 0)
            {
                sprite_width = 0;
            }

            sprite->width = sprite_width;
            sprite->height = sprite_height;

            // Adjust position by hotspot offset
            sprite->newy = (s16)(((s32)(s16)((xread16(bitmap + 4) >> 1) - xread16(meta + 6)) * (s32)sprite->newzoomy >> 8) * -2 >> (detail_shift & 0x3f)) + sprite->newy;
            sprite->newx = (s16)(((s32)(s16)((xread16(bitmap + 2) >> 1) - xread16(meta + 4)) * (s32)sprite->newzoomx >> 8) * -2 >> (detail_shift & 0x3f)) + sprite->newx;
            zoomtofen(sprite);
        }
    }
}

// Render all sprites in the depth-sorted list that are behind the current depth layer
void spritaff(s16 depth_layer)
{
    u16 idx = image.spritnext;
    while (true)
    {
        if (idx == 0)
        {
            // End of sprite list: reset depth sentinel
            image.spritnext = idx;
            image.spritprof = 0x8000;
            return;
        }

        sSprite *sprite = SPRITE_VAR(idx);
        if (sprite->newd <= depth_layer)
        {
            break;
        }

        // Render valid sprite
        if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
        {
            zoomtofen(sprite);
        }

        idx = sprite->link;
    }

    // Update next-to-render pointer and depth threshold
    image.spritprof = SPRITE_VAR(idx)->newd;
    image.spritnext = idx;
}

// =============================================================================
// doland: Main 3D terrain column renderer for Robinson's Requiem
//
// Renders a heightmap-based terrain with overhang/bridge support and billboard
// sprites using column-based raycasting with fixed-point perspective projection.
//
// Algorithm:
//   Outer loop: iterate terrain rows from far to near (decreasing depth)
//   Inner loop: iterate columns left-to-right within each row
//   For each column:
//     1. Look up terrain cell from heightmap grid
//     2. Read height index, convert to screen Y via altitude table
//     3. Render vertical terrain bar (barland)
//     4. Check for overhangs (tbarland) or billboard sprites (barsprite)
//
// Data structures:
//   terrain_grid (atlland): array of pointers to terrain strips, 4 bytes per X column.
//     Each strip contains terrain cells at 2-byte intervals indexed by Y.
//     Cell format: low byte = height index (0-255), bits [13:8] = terrain type.
//   alt_table (atalti): pre-computed screen Y offsets per height index.
//     Segmented by distance: 0x200 bytes (256 entries) per segment.
//   render_context: pre-computed rendering parameters at negative offsets,
//     populated by calclan0() before this function is called.
//
// Render context key offsets:
//   -0x3fe  terrain data valid flag (u8, must be 1)
//   -0x3c4  terrain lighting/normal table pointer
//   -0x3c0  terrain cell size shift (log2)
//   -0x3a8  horizontal projection denominator
//   -0x3a4  terrain scale factor
//   -0x3a2/-0x3a0  camera grid position (X, Y)
//   -0x360/-0x35e  camera world position (X, Y)
//   -0x37e/-0x37c  far-left world corner
//   -0x378/-0x376  far-right world corner
//   -0x294/-0x292  terrain grid bounds (width, height)
//   -0x2e0  current depth (distance from camera)
//   -0x2dc  depth step per row
//   -0x2c4/-0x2c0  column step direction (X, Y) in 16.16 fixed-point
//   -0x2bc/-0x2b8  row step direction (X, Y)
//   -0x280  screen column X position (16.16 fixed-point)
//   -0x27c  screen column X step per column
//   -0x276  altitude base Y offset
//   -0x26e  base Y offset for altitude lookups
//   -0x26c/-0x26a  screen center (X, Y)
//   -0x262  altitude segment step divisor
//   -0x260  max screen Y for current column
//   -0x25c  bottom clip Y
//   -0x25a  altitude table segment offset accumulator
//   -0x256/-0x252  fog accumulator / fog step
//   -0x24e  row counter (counts down, negative = behind camera)
//   -0x246  current clip Y for overhang tracking
//   rc+0x10+type  overhang height map layer offset
//   rc+0x14+type  feature flag: <0 = overhang, 1 = sprite, 0 = none
//   rc+0x16+type  overhang thickness
// =============================================================================
void doland(s32 scene_addr, s32 render_context)
{
    // =========================================================================
    // INITIALIZATION
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
    s32 terrain_grid = image.atlland;

    // --- Reset overhang interpolation state ---
    // These globals track overhang geometry across columns for smooth rendering.
    // prec = precedent (previous), top/bot = overhang top/bottom screen Y,
    // a/b/c = current/prev/prev-prev column, i = interpolated.
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

    // Initial screen Y projection
    xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + xread16(render_context - 0x352)));

    // --- Initial perspective projection ---
    u16 cell_shift = (u16)xread16(render_context - 0x3c0) & 0x3f;
    s16 proj_x = (cam_grid_x << cell_shift) - xread16(render_context - 0x360);
    s16 proj_y = (cam_grid_y << cell_shift) - xread16(render_context - 0x35e);

    glandtopix(render_context, &proj_x, &proj_y,
               (xread16(render_context - 0x378) + proj_x) - xread16(render_context - 0x37e),
               (xread16(render_context - 0x376) + proj_y) - xread16(render_context - 0x37c),
               xread16(render_context - 0x2e0));

    xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + proj_x) << 0x10);
    xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(proj_y - proj_x) << 8) / (s32)xread16(render_context - 0x3a4)) << 8);

    // Current traversal position in terrain grid (16.16 fixed-point)
    u32 trav_x = (u32)cam_grid_x << 16;
    u32 trav_y = (u32)cam_grid_y << 16;

    // =========================================================================
    // OUTER LOOP: Process terrain rows from far to near
    // =========================================================================
    do
    {
        // ----- Advance row position accumulators -----
        xwrite32(render_context - 0x2a8, xread32(render_context - 0x2bc) + xread32(render_context - 0x2a8));
        xwrite32(render_context - 0x2a4, xread32(render_context - 0x2b8) + xread32(render_context - 0x2a4));

        u32 row_start_x = trav_x;
        u32 row_start_y = trav_y + (xread16(render_context - 0x29c) << 16);

        // ----- Snap column position to terrain grid alignment -----
        // The column step is ±1 in either X or Y axis.
        // Snap the starting position so it aligns with the accumulated row position.
        if (col_step_x_raw == 0x00010000)
        {
            if (xread16(render_context - 0x2bc) < 0)
            {
                while (xread16(render_context - 0x2a8) < (s16)(row_start_x >> 16))
                {
                    row_start_y -= col_step_y_raw;
                    row_start_x -= col_step_x_raw;
                }
            }
            else
            {
                while ((s16)(row_start_x >> 16) < xread16(render_context - 0x2a8))
                {
                    row_start_y += col_step_y_raw;
                    row_start_x += col_step_x_raw;
                }
            }
        }
        else if (col_step_x_raw == 0xffff0000)
        {
            if (xread16(render_context - 0x2bc) < 0)
            {
                while (xread16(render_context - 0x2a8) < (s16)(row_start_x >> 16))
                {
                    row_start_y += col_step_y_raw;
                    row_start_x += col_step_x_raw;
                }
            }
            else
            {
                while ((s16)(row_start_x >> 16) < xread16(render_context - 0x2a8))
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
            row_start_x = trav_x + (xread16(render_context - 0x2a0) << 16);
            if (col_step_y_raw == 0x00010000)
            {
                if (xread16(render_context - 0x2b8) < 0)
                {
                    while (xread16(render_context - 0x2a4) < (s16)(row_start_y >> 16))
                    {
                        row_start_y -= col_step_y_raw;
                        row_start_x -= col_step_x_raw;
                    }
                }
                else
                {
                    while ((s16)(row_start_y >> 16) < xread16(render_context - 0x2a4))
                    {
                        row_start_y += col_step_y_raw;
                        row_start_x += col_step_x_raw;
                    }
                }
            }
            else
            {
                if (xread16(render_context - 0x2b8) < 0)
                {
                    while (xread16(render_context - 0x2a4) < (s16)(row_start_y >> 16))
                    {
                        row_start_y += col_step_y_raw;
                        row_start_x += col_step_x_raw;
                    }
                }
                else
                {
                    while ((s16)(row_start_y >> 16) < xread16(render_context - 0x2a4))
                    {
                        row_start_y -= col_step_y_raw;
                        row_start_x -= col_step_x_raw;
                    }
                }
            }
        }

        // ----- Update fog/distance shading -----
        xwrite32(render_context - 0x256, xread32(render_context - 0x252) + xread32(render_context - 0x256));
        image.vdarkw = xread8(render_context - 0x255) << 8;

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

        xwrite32(render_context - 0x278, xread32(render_context - 0x270));

        // ----- Recalculate perspective if row is in front of camera -----
        if (-1 < xread16(render_context - 0x24e))
        {
            proj_x = ((s16)(row_start_x >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x360);
            proj_y = ((s16)(row_start_y >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x35e);
            glandtopix(render_context, &proj_x, &proj_y,
                       (xread16(render_context - 0x378) + proj_x) - xread16(render_context - 0x37e),
                       (xread16(render_context - 0x376) + proj_y) - xread16(render_context - 0x37c),
                       xread16(render_context - 0x2e0));
            xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + proj_x) << 0x10);
            xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(proj_y - proj_x) << 6) / (s32)xread16(render_context - 0x3a4)) << 10);

            u16 proj_denom = (u16)(xread16(render_context - 0x3a8) + xread16(render_context - 0x2e0));
            if (proj_denom == 0 || scarry2(xread16(render_context - 0x3a8), xread16(render_context - 0x2e0)) != (s32)((u32)proj_denom << 0x10) < 0)
            {
                proj_denom = 1;
            }

            xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + (s16)(xread32(render_context - 0x2d8) / (s32)(s16)proj_denom)));
        }

        // ----- Render sprites at this depth layer -----
        if (xread16(render_context - 0x2e0) <= image.spritprof)
        {
            spritaff(xread16(render_context - 0x2e0));
        }

        // ----- Select altitude table segment for current distance -----
        alt_table += xread16(render_context - 0x25a);
        u16 alt_seg_idx = (u16)(((xread32(render_context - 0x2e0) - xread32(render_context - 0x2c8)) >> 8) / (s32)xread16(render_context - 0x262));
        if (xread16(render_context - 0x24e) == 1)
        {
            alt_seg_idx = 0x31;
        }

        if (0x31 < alt_seg_idx)
        {
            if ((s16)alt_seg_idx < 0x32)
            {
                alt_seg_idx = 0;
            }
            else
            {
                alt_seg_idx = 0x31;
            }
        }

        xwrite16(render_context - 0x25a, ((s16)image.atalti + alt_seg_idx * 0x200) - (s16)alt_table);

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
            s16 col_target_x = xread16(render_context - 0x280);

            // ----- Pre-scan: step through grid cells between columns -----
            // Scan intermediate terrain cells to find maximum height before
            // reaching the current screen column position.
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
                        if (next_scan_y <= grid_height)
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

                        prescan_height = (u16)xread8((s32)adresa + 1);
                    }
                }
                else
                {
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

                        prescan_height = (u16)xread8((s32)adresa + 1);
                    }
                }

                // Convert height to screen Y and track maximum
                max_y = xread16(render_context - 0x26e) + xread16(alt_table + (s16)(xread16(render_context - 0x25a) + prescan_height * 2));
                xwrite16(render_context - 0x260, max_y);
                if (prev_max_y < max_y)
                {
                    prev_max_y = max_y;
                }

                col_target_x = xread16(render_context - 0x280);
            }

            // =================================================================
            // Main terrain lookup at center column position
            // =================================================================
            u16 center_grid_x = (u16)(trav_x >> 16);
            u16 center_grid_y = (u16)(trav_y >> 16);
            u32 terrain_cell;
            s32 strip_ptr_main;
            u16 grid_h;

            if ((u16)xread16(render_context - 0x294) < center_grid_x)
            {
                if (xread8(render_context - 0x3fe) == 1)
                {
                    s16 wrap_x = 0;
                    if (xread16(render_context - 0x294) <= (s16)center_grid_x)
                    {
                        wrap_x = xread16(render_context - 0x294) * 2;
                    }

                    strip_ptr_main = xread32(terrain_grid + (s16)((wrap_x - center_grid_x) * 4));
                    grid_h = (u16)xread16(render_context - 0x292);
                    if (center_grid_y <= grid_h)
                    {
                        goto cell_calc_subtract;
                    }
                    else
                    {
                        goto cell_calc_add;
                    }
                }
            }
            else
            {
                if ((u16)xread16(render_context - 0x292) < center_grid_y)
                {
                    if (xread8(render_context - 0x3fe) != 1)
                        goto advance_column;

                    strip_ptr_main = xread32(terrain_grid + (s16)(center_grid_x << 2));
                    grid_h = (u16)xread16(render_context - 0x292);

                cell_calc_subtract:

                    if ((s16)grid_h <= (s16)center_grid_y)
                    {
                        strip_ptr_main += (s32)xread16(render_context - 0x292) << 2;
                    }

                    terrain_cell = ((strip_ptr_main - (s16)center_grid_y) - (s32)(s16)center_grid_y);
                }
                else
                {
                    strip_ptr_main = xread32(terrain_grid + (s16)(center_grid_x << 2));

                cell_calc_add:

                    terrain_cell = (strip_ptr_main + (s16)center_grid_y + (s32)(s16)center_grid_y);
                }

                // --- Read terrain cell and compute ground screen Y ---
                s16 ground_clip_y = xread16(render_context - 0x25c);
                u16 cell_data = xread16(terrain_cell);
                image.solh = cell_data & 0xff;
                image.solpixy = xread16(render_context - 0x276) + xread16(alt_table + (s16)(image.solh * 2));
                xwrite16(render_context - 0x246, image.solpixy);
                xwrite16(render_context - 0x25c, image.solpixy);
                if (image.solpixy < ground_clip_y)
                {
                    xwrite16(render_context - 0x246, ground_clip_y);
                    ground_clip_y = image.solpixy;
                }

                u16 bar_height = prev_max_y - ground_clip_y;
                s16 col_dir_x = (s16)(col_step_x_raw >> 0x10);

                // Extract terrain type index: bits [13:8] encode type, shifted for table lookup
                s16 terrain_type_idx = ((cell_data & 0x3f00) >> 3) - 0xc00;

                // --- Render ground terrain bar ---
                if (bar_height != 0 && sborrow2(prev_max_y, ground_clip_y) == (s32)((u32)bar_height << 0x10) < 0)
                {
                    u32 packed_pos = concat22((trav_x), (trav_x >> 16));
                    barland(terrain_cell, render_context, col_dir_x, (s16)(col_step_y_raw >> 0x10), ground_clip_y, bar_height, terrain_type_idx, bar_screen_x, packed_pos, scan_x);
                }

                // =============================================================
                // Check for terrain features: overhangs or billboard sprites
                // =============================================================
                // Feature flag at rc[0x14 + type]:
                //   < 0 (bit 7 set): overhang/bridge
                //   == 1: billboard sprite
                //   == 0: no feature
                if (xread8(render_context + terrain_type_idx + 0x14) != 0)
                {
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
                                u16 oh_ceil_h = (u16)xread8((s32)adresa + xread32(render_context + 0x10 + prev_type_idx) + 1);
                                u16 oh_delta_h = oh_ceil_h - solha;

                                if ((oh_delta_h != 0 && sborrow2(oh_ceil_h, solha) == (s32)((u32)oh_delta_h << 0x10) < 0) && ((u16)(oh_thickness - oh_delta_h) != 0 && sborrow2(oh_thickness, oh_delta_h) == (s32)((u32)(u16)(oh_thickness - oh_delta_h) << 0x10) < 0))
                                {
                                    u16 oh_bottom_h = oh_ceil_h;
                                    if ((s16)(solha + (u16)(oh_thickness - oh_delta_h)) < (s16)oh_ceil_h)
                                    {
                                        oh_bottom_h = (solha + (u16)(oh_thickness - oh_delta_h)) - solha;
                                    }

                                    // Convert overhang heights to screen Y
                                    s16 oh_top_scr = xread16(render_context - 0x26e) + xread16(alt_table + (s16)(xread16(render_context - 0x25a) + oh_ceil_h * 2));
                                    u16 oh_bot_scr = xread16(render_context - 0x26e) + xread16(alt_table + (s16)(xread16(render_context - 0x25a) + (oh_bottom_h - oh_ceil_h) * -2));
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
                                        // Running max/min (asm: D6 = max(D6, D0), D7 = min(D7, D1))
                                        if ((s16)oh_top_scr > (s16)prectopc)
                                            prectopc = oh_top_scr;
                                        if ((s16)oh_bot_scr < (s16)precbotc)
                                            precbotc = oh_bot_scr;

                                        if ((s16)oh_packed < (s16)oh_prev_bot)
                                        {
                                            notopa = 0;
                                        }
                                    }
                                    goto overhang_render;
                                }
                            }

                            fprectopa = 0;
                        }

                    overhang_render:

                        // --- Look up overhang terrain cell ---
                        // Overhang height data lives at a separate layer offset from ground
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
                            u16 oh_top_pix = xread16(render_context - 0x276) + xread16(alt_table + (s16)(image.toph * 2));
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
                                oh_clip_y = xread16(render_context - 0x276) + xread16(alt_table + (s16)(oh_bottom_offset * -2));
                            }

                            botalt = precboti;
                            bothigh = oh_clip_y;
                            if (precboti < oh_clip_y)
                            {
                                bothigh = precboti;
                                botalt = oh_clip_y;
                            }

                            precboti = oh_clip_y;

                            // --- Render overhang bar connecting to previous column ---
                            if (fprectop != 0)
                            {
                                u16 oh_bar_h = botalt - prev_oh_top;
                                s32 oh_bar_height = oh_bar_h;
                                if (oh_bar_h != 0 && sborrow2(botalt, prev_oh_top) == (s32)((u32)oh_bar_h << 0x10) < 0)
                                {
                                    if (notopa == 0)
                                    {
                                        // Clip overhang against previous column's bounds
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
                                                goto overhang_done;
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
                                    tbarland(terrain_cell, render_context, col_dir_x, col_step_y, prev_oh_top, oh_bar_height, oh_type_idx, col_x_pos_swp, packed_pos, oh_packed);
                                    xwrite16(render_context - 0x25c, saved_clip);
                                }
                            }

                        overhang_done:

                            fprectop = 1;
                        }

                        goto advance_column;
                    }

                    if ((s8)xread8(render_context + terrain_type_idx + 0x14) != 1)
                        goto advance_column;

                    // --- Billboard sprite at this terrain cell ---
                    barsprite(render_context, terrain_type_idx, center_grid_x, center_grid_y, (s16)scan_x);
                }

                fprectop = 0;
                fprectopa = 0;
            }

        advance_column:

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

void affiland(s32 scene)
{
    s32 scene_addr = alis.basemain + scene;

    // Set up logic buffer and viewport from scene data
    image.mapscreen = scene_addr;
    image.wloglarg = xread16(scene_addr + 0x12) + 1;
    image.wlogic = (u8 *)(alis.mem + xread32(xread32(scene_addr + 0x30)));
    image.wlogx1 = xread16(scene_addr + 0xe);
    image.wlogy1 = xread16(scene_addr + 0x10);

    // Resolve render context from entity table
    s32 render_context = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));

    // Initialize clipping rectangle from scene viewport
    image.fenx1 = xread16(scene_addr + 0xe);
    image.landclipy1 = xread16(scene_addr + 0x10);
    image.landclipx2 = xread16(scene_addr + 0x12) + image.fenx1;
    image.feny2 = xread16(scene_addr + 0x14) + image.landclipy1;
    image.clipl = (image.landclipx2 - image.fenx1) + 1;
    image.landcliph = (image.feny2 - image.landclipy1) + 1;
    image.vbarclipx2 = image.landclipx2 + 1;
    image.feny1 = image.landclipy1;
    image.fenx2 = image.landclipx2;
    image.clipx1 = image.fenx1;
    image.clipy1 = image.landclipy1;
    image.clipx2 = image.landclipx2;
    image.clipy2 = image.feny2;
    image.cliph = image.landcliph;

    if (image.ftstpix == 0)
    {
        if ((xread8(scene_addr) & 0x80) == 0)
        {
            // No 3D flag: copy VGA buffer to display if switching
            if ((alis.fswitch != 0) && (image.switchland != 0))
            {
                image.fenlargw = image.wloglarg >> 2;
                image.wloglarg = image.wloglarg >> 1;
                tvtofen();
                image.switchland = 0;
            }
        }
        else
        {
            // Full 3D terrain render pipeline
            calctoy(scene_addr, render_context);
            spritland(scene_addr, xread16(scene_addr + 2));
            calclan0(scene_addr, render_context);
            if (((xread8(scene_addr + 1) & 0x40) == 0) && (xread16(scene_addr + 0xa2) == 0))
            {
                clrvga();
            }

            doland(scene_addr, render_context);
            vgatofen();
            image.landone = 1;
            image.switchland = 1;
        }
    }
    else
    {
        // Hit-test mode: skip rendering, only compute terrain data
        calclan0(scene_addr, render_context);
        doland(scene_addr, render_context);
    }

    // Restore logic buffer state
    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogy1 = image.logy1;
    image.wlogx2 = image.logx2;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;
    image.fenlargw = image.loglarg >> 2;
}

void landtofi(s16 unused, s16 scene_id)
{
    // Save current clipping state
    s16 save_cliph = image.cliph;
    s16 save_clipl = image.clipl;
    s16 save_clipy2 = image.clipy2;
    s16 save_clipx2 = image.clipx2;
    s16 save_clipy1 = image.clipy1;
    s16 save_clipx1 = image.clipx1;

    // Set up logic buffer from scene data
    s32 scene_addr = alis.basemain + scene_id;
    image.wloglarg = xread16(scene_addr + 0x12) + 1;
    image.wlogic = (u8 *)(alis.mem + xread32(xread32(scene_addr + 0x30)));
    image.wlogx1 = xread16(scene_addr + 0xe);
    image.wlogy1 = xread16(scene_addr + 0x10);
    image.mapscreen = scene_addr;

    // Render terrain into sub-window if flagged
    if (image.fdoland != 0)
    {
        // Set up clipping from current clip rect and scene viewport
        image.landclipx2 = xread16(scene_addr + 0x12) + xread16(scene_addr + 0xe);
        image.landcliph = (image.clipy2 - image.clipy1) + 1;
        image.clipl = (image.clipx2 - image.clipx1) + 1;
        image.vbarclipx2 = image.clipx2 + 1;
        image.landclipy1 = image.clipy1;

        // Resolve render context and run terrain render pipeline
        s32 render_context = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));
        image.cliph = image.landcliph;
        calctoy(scene_addr, render_context);
        spritland(scene_addr, xread16(scene_addr + 2));
        calclan0(scene_addr, render_context);
        if (((xread8(scene_addr + 1) & 0x40) == 0) && (xread16(scene_addr + 0xa2) == 0))
        {
            clrvga();
        }

        doland(scene_addr, render_context);
        image.fdoland = 0;
        image.landone = 1;
    }

    // Restore clipping state and copy VGA buffer to display
    image.clipx1 = save_clipx1;
    image.clipy1 = save_clipy1;
    image.clipx2 = save_clipx2;
    image.clipy2 = save_clipy2;
    image.clipl = save_clipl;
    image.cliph = save_cliph;
    vgatofen();

    // Restore logic buffer state
    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogy1 = image.logy1;
    image.wlogx2 = image.logx2;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;
}

void pixtoland(s32 render_context)
{
    // Inverse perspective projection: screen (x, y) + depth → world (x, y, z)

    // Read camera world position
    s16 cam_wx = xread16(render_context - 0x268);
    s16 cam_wy = xread16(render_context - 0x266);
    s16 cam_wz = xread16(render_context - 0x264);

    // Read camera screen center
    s16 cam_sx = xread16(render_context - 0x26c);
    s16 cam_sy = xread16(render_context - 0x26a);

    // Read precomputed trig values
    s32 cos_yaw = xread16(render_context - 0x28a);
    s32 sin_yaw = xread16(render_context - 0x28c);
    s32 sin_pitch = xread16(render_context - 0x290);

    // Read FOV projection factors
    s16 fov_num_x = xread16(render_context - 0x3ac);
    s32 fov_den_x = xread16(render_context - 0x3aa);
    s16 fov_num_y = xread16(render_context - 0x3a8);
    s32 fov_den_y = xread16(render_context - 0x3a6);

    s32 depth = image.dtstpix;

    // Screen deltas from camera center
    s16 dx = (s16)(image.xtstpix - cam_sx);
    s16 dy = (s16)(cam_sy - image.ytstpix);
    s32 depth_adj_x = (s16)(fov_num_x + depth);

    // Unproject X: rotate dx by cos(yaw), scale by perspective, subtract depth*sin(yaw)
    s16 rot_cos = (s16)((s32)dx * cos_yaw >> 9);
    s16 unproj_x = (s16)((s32)rot_cos * depth_adj_x / fov_den_x);
    s16 depth_sin = (s16)(depth * sin_yaw >> 9);
    image.cxtstpix = cam_wx + (unproj_x - depth_sin);

    // Unproject Y: rotate dx by sin(yaw), scale by perspective, add depth*cos(yaw)
    s16 rot_sin = (s16)((s32)dx * sin_yaw >> 9);
    s16 unproj_y = (s16)((s32)rot_sin * depth_adj_x / fov_den_x);
    s16 depth_cos = (s16)(depth * cos_yaw >> 9);
    image.cytstpix = cam_wy + depth_cos + unproj_y;

    // Unproject Z: scale dy by perspective, add depth*sin(pitch)
    s32 depth_adj_y = (s16)(fov_num_y + depth);
    s16 unproj_z = (s16)((s32)dy * depth_adj_y / fov_den_y);
    s16 depth_pitch = (s16)(depth * sin_pitch >> 9);
    image.cztstpix = cam_wz + depth_pitch + unproj_z;
}

void pointpix(s16 x, s16 y)
{
    // Resolve scene address from current script
    u32 scene_id = xread16(alis.script->vram_org - 0x16);
    u32 scene_addr = alis.basemain + scene_id;

    // Initialize hit-test state
    image.dtstpix = xread16(scene_addr + 0x94);
    image.etstpix = -1;
    image.ntstpix = -1;
    image.ftstpix = 1;
    image.xtstpix = x;
    image.ytstpix = y;

    // Run terrain hit-test if scene is not disabled
    if ((xread8(scene_addr) & 0x40) == 0)
    {
        affiland(scene_id);

        // Resolve hit entity sprite index from entity table
        if (-1 < image.etstpix)
        {
            u32 entity_data = (xread32(xread32(xread32(alis.atent + image.etstpix) - 0x14) + 0xe) + xread32(xread32(alis.atent + image.etstpix) - 0x14));
            image.ntstpix = (s16)(((image.antstpix & 0xffffff) - (xread32(entity_data) + entity_data)) >> 2);
            if (xread16(entity_data + 4) <= image.ntstpix)
            {
                // Sprite index out of range: fall back to base entity table
                image.ntstpix = (s16)(((image.antstpix & 0xffffff) - (xread32(alis.basemain + xread32(alis.basemain + 0xe)) + (s32)(alis.basemain + xread32(alis.basemain + 0xe)))) >> 2);
            }
        }
    }

    // Convert hit screen position back to world coordinates
    s32 render_context = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));
    pixtoland(render_context);
    image.ftstpix = 0;
}
