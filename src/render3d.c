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
#include "render3d_dos.h"
#include "render3d_68k.h"


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

u8 terrain_fill_color = 0;  // DOS: solid fill color for barlands gap fill
s16 bartra_saved_si = 0;    // DAT_0001ab5e: terrain-type extra height from doland altitude lookup (0 for tbarland)
s16 bottom_type_index = 0;  // DAT_0001ab64: type index for bottom section texture (set by tbarland)

extern u8 tabatan[];
extern s16 *tabsin;
extern s16 *tabcos;

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

    s32 screen_x;
    if (alis.platform.is_little_endian)
    {
        // DOS: CDQ+IDIV — 64-bit dividend / 32-bit divisor
        screen_x = (s32)((s64)cam_x / x_divisor);
        if (screen_x < INT16_MIN || screen_x > INT16_MAX)
        {
            screen_x = screen_x < 0 ? INT16_MIN : INT16_MAX;
        }
    }
    else
    {
        screen_x = cam_x / x_divisor;
        if (screen_x < INT16_MIN || screen_x > INT16_MAX)
        {
            screen_x = (screen_x < 0 ? INT16_MIN : INT16_MAX) * (x_divisor < 0 ? -1 : 1);
        }
    }

    // Perspective divide for screen Y and zoom (uses full 32-bit cam_y)
    s32 y_divisor = cam_y + xread16(scene_addr + 0x74);
    if (y_divisor == 0 || ((y_divisor > INT16_MAX || y_divisor < INT16_MIN) && y_divisor > 0) || (y_divisor <= INT16_MAX && y_divisor >= INT16_MIN && y_divisor < 0))
    {
        y_divisor = 1;
    }

    s32 screen_y;
    s32 zoom;
    if (alis.platform.is_little_endian)
    {
        // DOS: CDQ+IDIV — 64-bit dividend / 32-bit divisor
        screen_y = (s32)((s64)alt_proj / y_divisor);
        if (screen_y < INT16_MIN || screen_y > INT16_MAX)
        {
            screen_y = screen_y < 0 ? INT16_MIN : INT16_MAX;
        }

        zoom = (s32)((s64)(xread16(scene_addr + 0x7a) << 8) / y_divisor);
        if (zoom < INT16_MIN || zoom > INT16_MAX)
        {
            zoom = zoom < 0 ? INT16_MIN : INT16_MAX;
        }
    }
    else
    {
        screen_y = alt_proj / y_divisor;
        if (screen_y < INT16_MIN || screen_y > INT16_MAX)
        {
            screen_y = (screen_y < 0 ? INT16_MIN : INT16_MAX) * (y_divisor < 0 ? -1 : 1);
        }

        zoom = (xread16(scene_addr + 0x7a) << 8) / y_divisor;
        if (zoom < INT16_MIN || zoom > INT16_MAX)
        {
            zoom = (zoom < 0 ? INT16_MIN : INT16_MAX) * (y_divisor < 0 ? -1 : 1);
        }
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
    s16 cos_pitch = tabcos[pitch_idx];
    s16 sin_pitch = tabsin[pitch_idx];
    s16 cos_yaw = tabcos[yaw_idx];
    s16 sin_yaw = tabsin[yaw_idx];
    xwrite16(render_context - 0x28e, cos_pitch);
    xwrite16(render_context - 0x28a, cos_yaw);
    xwrite16(render_context - 0x290, sin_pitch);
    xwrite16(render_context - 0x28c, sin_yaw);

    // DOS also writes trig values to scene_addr for use by other systems
    xwrite16(scene_addr + 0xaa, cos_pitch);
    xwrite16(scene_addr + 0xac, sin_pitch);
    xwrite16(scene_addr + 0xb2, cos_yaw);
    xwrite16(scene_addr + 0xb4, sin_yaw);

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

    // DOS uses 179 rows (0xB3) for finer depth resolution; m68k uses 49 (0x31)
    s32 num_rows = alis.platform.is_little_endian ? 0xb3 : 0x31;
    s32 sentinel_count = alis.platform.is_little_endian ? 0x100 : 0xff;

    // Set up row interpolation parameters
    s32 scale_end = xread32(render_context - 0x2ec);
    s32 scale_start = xread32(render_context - 0x2f8);
    s32 row_scale = xread32(render_context - 0x2f8);
    s32 screen_y_fp = xread32(render_context - 0x32c) << 0x10;
    s32 screen_y_step = (s16)(((xread32(render_context - 0x314) - xread32(render_context - 0x32c)) * 0x100) / num_rows) - 1;
    xwrite16(render_context - 0x262, (s16)screen_y_step);

    u32 row_factor;
    u32 accum;

    u32 alti_ptr = image.atalti;

    // Fill altitude lookup table (num_rows rows x 0xFF columns)
    for (int s = 0; s < num_rows; s++)
    {
        s16 divisor_val = (s16)(xread16(render_context - 0x3a8) + (s16)((u32)screen_y_fp >> 0x10));
        s16 quotient_val = (s16)(row_scale / (s32)divisor_val);
        row_factor = quotient_val * 0x100;
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
        }

        screen_y_fp += screen_y_step * 0x100;

        row_scale += (s16)((scale_end - scale_start) / num_rows);
    }

    // Sentinel row (beyond last visible row)
    for (int i = 0; i < sentinel_count; i++, alti_ptr += 2)
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
    u16 row_stride = alis.platform.is_little_endian ? 0x140 : xread16(sceneadr + 0x12) + 1;
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

    // Half near distance: DOS divides by 4, m68k divides by 2
    xwrite16(sceneadr + 0x92, xread16(sceneadr + 0x66) >> (alis.platform.is_little_endian ? 2 : 1));

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

    // Allocate VRAM buffer for scene
    if (xread32(scene_addr + 0x30) == 0)
    {
        if (alis.platform.is_little_endian)
        {
            // DOS: 320 bytes/row with 4-row top padding (Ghidra line 16318-16320)
            u32 buffer = io_malloc((u32)(u16)(xread16(scene_addr + 0x14) + 9) * 0x140);
            xwrite32(scene_addr + 0x30, buffer);
            xwrite32(buffer, xread32(buffer) + 0x500);  // offset by 4 scanlines
        }
        else
        {
            u32 buffer = io_malloc((u32)(u16)(xread16(scene_addr + 0x12) + 1) * (u32)(u16)(xread16(scene_addr + 0x14) + 1) + 0x140);
            xwrite32(scene_addr + 0x30, buffer);
        }
    }

    // Allocate shared pixel, altitude, and alias tables
    if (image.tlpix == 0)
    {
        if (alis.platform.is_little_endian)
        {
            // DOS layout: single allocation containing altitude data, scanline ptrs,
            // alias table, and terrain strips all in one block.
            // Layout: [altitude 0x16800] [atlpix 0x320] [atalias 0x2000] [atlland strip_count*4]
            u16 strip_count = xread16(render_context - 0x3cc);
            image.tlpix = io_malloc(strip_count * 4 + 0x18b20);
            u32 raw_base = xread32(image.tlpix);
            image.atalti = raw_base;
            image.atlpix = raw_base + 0x16800;
            image.atalias = image.atlpix + 0x320;
            image.atlland = image.atalias + 0x2000;
            image.tlland = image.tlpix; // same allocation, mark as allocated
        }
        else
        {
            // m68k layout: separate allocations
            image.tlpix = io_malloc(0x8720);
            image.atlpix = xread32(image.tlpix);
            image.atalti = image.atlpix + 0x320;
            image.atalias = image.atalti + 0x6400;
        }
    }

    // Allocate terrain strip table (m68k only, DOS includes it in the single alloc above)
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
        // DOS: sprite rotation angle is at sprite offset 0x26 (high word of u32 at 0x24 on LE)
        s16 sprite_angle;
        if (alis.platform.is_little_endian)
            sprite_angle = (s16)(sprite->sprite_0x28 >> 16);
        else
            sprite_angle = (s16)sprite->sprite_0x28;

        u16 norm_angle;
        for (norm_angle = (sprite_angle - xread16(scene_addr + 0x38)) + 900; -1 < (int)((u32)norm_angle << 0x10); norm_angle -= 0x168) {}

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
    // DOS: no bitmap double-size flag check
    if (!alis.platform.is_little_endian && (xread8(bitmap + 1) & 0x20U) != 0)
    {
        image.newzoomy *= 2;
    }

    if (8 <= (char)zoom_shift)
    {
        // Multiply first, then shift — preserves precision for small (zoom_shift - 8) values
        image.newzoomy = (u16)((u32)image.newzoomy * (u16)(zoom_shift - 8) >> 3);
        zoom_shift = 3;
    }

    if (alis.platform.is_little_endian && (s8)sprite->credon_off == -0x80)
    {
        // DOS 0x80 path: load newzoomx from separate sprite field
        image.newzoomx = sprite->newzoomx;
    }
    else
    {
        image.newzoomx = image.newzoomy;
    }

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
    // DOS: axes are swapped — height uses newzoomx, width uses newzoomy
    u16 h_zoom = alis.platform.is_little_endian ? image.newzoomx : image.newzoomy;
    u16 w_zoom = alis.platform.is_little_endian ? image.newzoomy : image.newzoomx;

    s32 scaled_h = ((((xread16(bitmap + 4) + 1) * h_zoom * 2 >> zoom_shift) + 0xff) >> 8) - 1;
    if (scaled_h < 0)
        scaled_h = 0;

    s32 scaled_w = ((((xread16(bitmap + 2) + 1) * w_zoom * 2 >> zoom_shift) + 0xff) >> 8) - 1;
    if (scaled_w < 0)
        scaled_w = 0;

    image.newh = (s16)scaled_h;
    image.newl = (s16)scaled_w;

    // Adjust position by hotspot offset
    // DOS: axes are swapped — Y hotspot uses newzoomx, X hotspot uses newzoomy
    if (alis.platform.is_little_endian)
    {
        // DOS (Ghidra rrq-dos.c:16626): single shift by (zoom_shift + 8), no intermediate truncation
        image.newy += (s16)((s32)(s16)(((u16)xread16(bitmap + 4) >> 1) - xread16(meta + 6)) * (s32)(s16)h_zoom * -2 >> (zoom_shift + 8));
        image.newx += (s16)((s32)(s16)(((u16)xread16(bitmap + 2) >> 1) - xread16(meta + 4)) * (s32)(s16)w_zoom * -2 >> (zoom_shift + 8));
    }
    else
    {
        image.newy += (s16)(((s32)(s16)(((u16)xread16(bitmap + 4) >> 1) - xread16(meta + 6)) * (s32)h_zoom >> 8) * -2 >> zoom_shift);
        image.newx += (s16)(((s32)(s16)(((u16)xread16(bitmap + 2) >> 1) - xread16(meta + 4)) * (s32)w_zoom >> 8) * -2 >> zoom_shift);
    }
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

u32 skyfast(u32 tgt, u8 fill, s16 stride, s16 height)
{
    for (int l = 0; l < height; l++, tgt += (image.clipl + stride))
        memset(alis.mem + tgt, fill, image.clipl);
    
    return tgt;
}

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
                // Odd clipl: 16-bit loops wrote (clipl & ~1) bytes; advance dst past the
                // missing last pixel so the stride_gap brings us to the correct next row.
                if (image.clipl & 1)
                    dst++;

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

void barlands(u16 drawy, s16 barheight, s16 barwidth)
{
    if (!alis.platform.is_little_endian)
        return;

    // asm: TEST DX,DX; JZ return
    if (barheight == 0)
        return;

    // asm fill: (DAT_0001aac6 & 0x0F) + 0x80
    // Replace DAT_0001aac6 with whatever global/field in your port represents it.
    u8 fill = (terrain_fill_color & 0x0F) + 0x80;

    // asm: AX -= wlogx2; EDI = atlpix[(u16)AX] + (s16)precx
    s16 y_idx = (s16)((s16)drawy - (s16)image.wlogy1);
    u32 dst = xread32(image.atlpix + ((u16)y_idx) * 4);

    // IMPORTANT: signed add like MOVSX
    dst += (s32)(s16)image.precx;

    // asm keeps width in EBX and uses REP STOSB; we’ll do a simple loop.
    // If you have memset8(dst, fill, barwidth) use that.
    u16 w = (u16)barwidth;

    for (s16 h = barheight; h != 0; --h)
    {
        u32 p = dst;
        for (u16 i = 0; i < w; i++)
            xwrite8(p + i, fill);

        dst += (u32)(u16)image.wloglarg;
    }
}

void zoomtofen(sSprite *sprite)
{
    // Get sprite bitmap data with offset stored in header
    u8 *bitmap = alis.mem + sprite->newad + xread32(sprite->newad);

    u8 data_offset = (*bitmap == 0x18 || *bitmap == 0x1a) ? 6 : 8;
    
    // Only process valid sprite formats (0x18, 0x1a, 0x1c, 0x1e)
    if (*bitmap == 0x14 || *bitmap == 0x18 || *bitmap == 0x1a || *bitmap == 0x1c || *bitmap == 0x1e)
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
void affiland(s32 scene)
{
    s32 scene_addr = alis.basemain + scene;

    // Set up logic buffer and viewport from scene data
    image.mapscreen = scene_addr;
    image.wloglarg = alis.platform.is_little_endian ? 0x140 : xread16(scene_addr + 0x12) + 1;
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

// Function pointers for platform-specific dispatch
void (*calclan0)(s32 scene_addr, s32 render_context) = NULL;
void (*doland)(s32 scene_addr, s32 render_context) = NULL;
void (*landtofi)(s16 unused, s16 scene_id) = NULL;
void (*clrvga)(void) = NULL;
void (*vgatofen)(void) = NULL;

void render3d_init(void)
{
    if (alis.platform.is_little_endian)
    {
        calclan0 = calclan0_dos;
        doland = doland_dos;
        landtofi = landtofi_dos;
        clrvga = clrvga_dos;
        vgatofen = vgatofen_dos;
    }
    else
    {
        calclan0 = calclan0_68k;
        doland = doland_68k;
        landtofi = landtofi_68k;
        clrvga = clrvga_68k;
        vgatofen = vgatofen_68k;
    }
}
