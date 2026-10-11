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

#include "image.h"

#include <stdbool.h>

#include "alis.h"
#include "alis_private.h"
#include "debug.h"
#include "mem.h"
#include "video.h"
#include "render3d.h"
#include "screen.h"
#include "utils.h"

#if ALIS_SDL_VER < 2
# include <SDL/SDL.h>
extern volatile u8 dirty_pal;   // set from the Timer-C ISR (itroutine) on native
extern SDL_Rect dirty_rects[256];
extern u8 dirty_len;
#endif

#if defined(ALIS_PROFILE_DRAW)
// Lightweight per-phase wall-clock profiler (sys_profile_ticks units: 200 Hz /
// 5 ms on native Atari). Accumulate over a run; dump every PROF_DUMP_FRAMES
// frames. g_prof_frame_vsync is added by sys_render (the flip+Vsync wait).
u32 g_prof_frame_oldfen, g_prof_frame_affiscr, g_prof_frame_drawtot, g_prof_frame_vsync;
u32 g_prof_frame_frames, g_prof_frame_remap;
// Overdraw accounting per destofen blit: drawn pixels, opaque/transparent split,
// blit count, "big" (Blitter-worthy) pixel total.
u32 g_od_pix, g_od_pix_op, g_od_pix_tr, g_od_blits, g_od_big;
u32 g_prof_frame_depscreen;   // time in depscreen() (the per-element list re-link) within affiscr
#define OD_BIG_AREA 2048      // a blit counts as "big" (Blitter-worthy) above this
#define PROF_DUMP_FRAMES 100
#endif

u16 moduly;
u16 modumap16;
u16 xdeschart;
u16 ydeschart;
u8 (*chartproc)(u32 a5);
u16 chartvncol;
u8 chartvcol0;

u8 cga_palette[] = {
    0x00, 0x00, 0x00, 0x00,
    0x55, 0xff, 0xff, 0x00,
    0xff, 0x55, 0xff, 0x00,
    0xff, 0xff, 0xff, 0x00 };

u8 ega_palette[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xaa, 0x00,
    0x00, 0xaa, 0x00, 0x00,
    0x00, 0xaa, 0xaa, 0x00,
    0xaa, 0x00, 0x00, 0x00,
    0xaa, 0x00, 0xaa, 0x00,
    0xaa, 0x55, 0x00, 0x00,
    0xaa, 0xaa, 0xaa, 0x00,
    0x55, 0x55, 0x55, 0x00,
    0x55, 0x55, 0xff, 0x00,
    0x55, 0xff, 0x55, 0x00,
    0x55, 0xff, 0xff, 0x00,
    0xff, 0x55, 0x55, 0x00,
    0xff, 0x55, 0xff, 0x00,
    0xff, 0xff, 0x55, 0x00,
    0xff, 0xff, 0xff, 0x00};

u8 masks[4] = { 0b11000000, 0b00110000, 0b00001100, 0b00000011 };
u8 rots[4] = { 6, 4, 2, 0 };

sImage image = {
    .backprof = 0,
    .numelem = 0,
    .invert_x = 0,
    .depx = 0,
    .ddrawdist = 0,        // double draw distance --far
#if ALIS_SDL_VER > 1
    .emode = 0,            // enhanced mode --fog
    .fogcol = 0xFF8090A0,   // color terrain fade to
    .fogbeg = 80,          // fog begins at ~50% distance
    .fogend = 255,         // fully fogged at ~100% distance
#endif
    .depy = 0,
    .depz = 0,
    .oldcx = 0,
    .oldcy = 0,
    .oldcz = 0,
    .spag = 0,
    .wpag = 0,
    .sback = 0,
    .wback = 0,
    .cback = 0,
    .pback = 0
};

void draw_mac_rect(sRect *pos, sRect *bmp, u8 color);
void draw_rect(sRect *pos, sRect *bmp, u8 color);
void draw_mac_mono_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_mac_mono_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_dos_cga_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_dos_cga_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_st_4bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_st_4bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_ami_5bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip);
void draw_ami_5bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip);
void draw_4to8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset);
void draw_4to8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset);
void draw_8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip);
void draw_fli_video(u8 *bitmap);
void draw_transarctica_map(sSprite *sprite, u32 mapaddr, sRect lim);
void draw_requiem_map(sSprite *sprite, u32 bitmap);


#pragma mark -
#pragma mark Robinsons Requiem


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1) // Ensure no padding in structures

// BMP File Header (14 bytes)
typedef struct {
    u16 bfType;      // File type ('BM')
    u32 bfSize;      // File size in bytes
    u16 bfReserved1; // Reserved (0)
    u16 bfReserved2; // Reserved (0)
    u32 bfOffBits;   // Offset to pixel data
} BMPFileHeader;

// BMP Info Header (40 bytes)
typedef struct {
    u32 biSize;          // Header size
    s32 biWidth;          // Image width
    s32 biHeight;         // Image height
    u16 biPlanes;        // Number of color planes (1)
    u16 biBitCount;      // Bits per pixel (8 for grayscale)
    u32 biCompression;   // Compression type (0 = BI_RGB)
    u32 biSizeImage;     // Image data size (can be 0 for BI_RGB)
    s32 biXPelsPerMeter;  // X pixels per meter
    s32 biYPelsPerMeter;  // Y pixels per meter
    u32 biClrUsed;       // Number of colors in the palette (256 for 8-bit)
    u32 biClrImportant;  // Important colors (0 means all are important)
} BMPInfoHeader;

#pragma pack(pop) // Restore default alignment

// Function to save an 8-bit grayscale bitmap as a BMP file
int save_bitmap_as_bmp(u8 *data, s32 width, s32 height, const char *path) {
    FILE *file;
    s32 rowSize = (width + 3) & ~3; // BMP row size must be a multiple of 4 bytes
    s32 dataSize = rowSize * height;
    s32 paletteSize = 256 * 4; // 256 colors * 4 bytes (R, G, B, 0)
    s32 fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + paletteSize + dataSize;

    // Create BMP Headers
    BMPFileHeader fileHeader = {
        .bfType = 0x4D42, // 'BM' in little-endian
        .bfSize = fileSize,
        .bfReserved1 = 0,
        .bfReserved2 = 0,
        .bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + paletteSize
    };

    BMPInfoHeader infoHeader = {
        .biSize = sizeof(BMPInfoHeader),
        .biWidth = width,
        .biHeight = -height, // Negative height to store in top-down order
        .biPlanes = 1,
        .biBitCount = 8, // 8-bit grayscale
        .biCompression = 0, // BI_RGB (no compression)
        .biSizeImage = dataSize,
        .biXPelsPerMeter = 2835, // 72 DPI
        .biYPelsPerMeter = 2835, // 72 DPI
        .biClrUsed = 256,
        .biClrImportant = 256
    };

    // Open file for writing
    file = fopen(path, "wb");
    if (!file) {
        printf("Error opening file %s\n", path);
        return -1;
    }

    // Write BMP headers
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    // Write grayscale palette (256 shades of gray)
    for (int i = 0; i < 256; i++) {
        u8 color[4] = {i, i, i, 0}; // R, G, B, Reserved (0)
        fwrite(color, sizeof(color), 1, file);
    }

    // Write pixel data (row-by-row, ensuring 4-byte alignment)
    u8 *row = (u8 *)malloc(rowSize);
    if (!row) {
        fclose(file);
        return -1;
    }

    for (int y = 0; y < height; y++) {
        memcpy(row, data + y * width, width);
        memset(row + width, 0, rowSize - width); // Padding bytes
        fwrite(row, rowSize, 1, file);
    }

    // Clean up
    free(row);
    fclose(file);
    return 0;
}

#pragma mark -
#pragma mark Palette management


// Entries one palette bank spans: banks are 64 apart for <8bpp data (thepalet*64), one bank for 8bpp.
static int pal_bank_len(void) { return alis.platform.bpp != 8 ? 64 : 256; }

void topalet(void)
{
    image.ftopal = 0;

    // Fades always run over the fixed base (all banks), as the original: the selected bank
    // pointers can move under this (it runs from the timer thread / ISR).
    if (image.palc == 1)
    {
        memcpy(image.mpalet, image.tpalet, 256 * sizeof(u32));
    }
    else
    {
        switch (image.pal_format) {
            case EPalARGB:
                TOPALET_INTERP_LOOP(sColorARGB, image.mpalet, image.tpalet);
                break;
            case EPalABGR:
                TOPALET_INTERP_LOOP(sColorABGR,   image.mpalet, image.tpalet);
                break;
            case EPalRGBA32:
                TOPALET_INTERP_LOOP(sColorRGBA32, image.mpalet, image.tpalet);
                break;
            case EPal565:
                TOPALET_INTERP_LOOP(sColor565,  (u16 *)image.mpalet, (u16 *)image.tpalet);
                break;
        }
    }
    
    image.ftopal = 1;
    set_update_cursor();
}

void topalette(u8 *paldata, s32 duration)
{
    if (alis.platform.kind == EPlatformMac)
    {
        // Entry 0 = black, entries 1..255 = white
        
        PAL_WRITE_RGB(image.atpalet, 0, 0, 0, 0);
        PAL_WRITE_RGB(image.ampalet, 0, 0, 0, 0);

        for (int i = 1; i < 256; i++)
        {
            PAL_WRITE_RGB(image.atpalet, i, 255, 255, 255);
            PAL_WRITE_RGB(image.ampalet, i, 255, 255, 255);
        }
        image.ftopal = 0xff;
    }
    else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
    {
        u8 *addr = cga_palette;
        PAL_WRITE(image.atpalet, 0, 4, RGB32(addr));
        addr = cga_palette;
        PAL_WRITE(image.ampalet, 0, 4, RGB32(addr));
    }
    else
    {
        selpalet();

        s16 colors = alis.platform.version == 10 ? 0 : paldata[1];
        if (colors == 0) // 4 bit palette
        {
            image.palc = 0;
            u8 *palptr = &paldata[2];
            
            // Manhattan Amiga keeps the Atari 3-bit palettes
            if (alis.platform.kind == EPlatformAmiga || alis.platform.kind == EPlatformAmigaAGA)
            {
                PAL_WRITE(image.atpalet, 0, 16, RGB12(palptr));
            }
            else
            {
                PAL_WRITE(image.atpalet, 0, 16, RGB9(palptr));
            }
        }
        else // 8 bit palette
        {
            if (alis.platform.kind == EPlatformAmiga)
            {
                image.palc = 0;
                u8 *palptr = &paldata[2];
                PAL_WRITE(image.atpalet, 0, 32, RGB12(palptr));
            }
            else
            {
                image.palc = 0;
                u8 offset = paldata[2];

                paldata+=4;
                PAL_WRITE(image.atpalet, offset, colors + 1, RGB24(paldata));
            }
        }
        
        if (image.fdarkpal && image.thepalet == 0)
        {
            u16 *dkpalptr = (u16 *)image.dkpalet;
            s16 colors = alis.platform.bpp <= 4 ? 16 : 256;

            for (s32 i = 0; i < colors; i++, dkpalptr += 3)
            {
                u8 _r, _g, _b;
                PAL_READ_RGB(image.atpalet, i, _r, _g, _b);
                PAL_WRITE_RGB(image.atpalet, i, (u8)(_r * (dkpalptr[0] / 256.0)), (u8)(_g * (dkpalptr[1] / 256.0)), (u8)(_b * (dkpalptr[2] / 256.0)));
            }
        }
        
        if (duration == 0)
            memcpy(image.ampalet, image.atpalet, pal_bank_len() * sizeof(u32));   // only the written bank

        image.thepalet = 0;
        image.defpalet = 0;
        selpalet();

        if (duration != 0)
        {
            switch (image.pal_format) {
            case EPalARGB: PAL_DELTA_LOOP(sColorARGB, image.mpalet, image.tpalet, duration); break;
            case EPalABGR: PAL_DELTA_LOOP(sColorABGR,   image.mpalet, image.tpalet, duration); break;
            case EPalRGBA32: PAL_DELTA_LOOP(sColorRGBA32, image.mpalet, image.tpalet, duration); break;
            case EPal565: PAL_DELTA_LOOP(sColor565,  (u16 *)image.mpalet, (u16 *)image.tpalet, duration); break;
            }

            image.palt = 1;
            image.palt0 = 1;
            image.palc = duration;
        }
        else
        {
            image.ftopal = 0xff;
#if ALIS_SDL_VER < 2
            dirty_pal = 1;
#endif
        }
    }
    
    set_update_cursor();
}

void toblackpal(s16 duration)
{
    selpalet();

    // Black only the selected bank (the original clears 16 colours of addr_tpalet).
    memset(image.atpalet, 0, pal_bank_len() * sizeof(u32));
    if (duration == 0)
        memset(image.ampalet, 0, pal_bank_len() * sizeof(u32));

    image.thepalet = 0;
    image.defpalet = 0;
    selpalet();

    if (duration == 0)
    {
        image.ftopal = 0xff;
        image.palc = 0;
#if ALIS_SDL_VER < 2
            dirty_pal = 1;
#endif
    }
    else
    {
        switch (image.pal_format) {
        case EPalARGB: PAL_DELTA_LOOP(sColorARGB, image.mpalet, image.tpalet, duration); break;
        case EPalABGR: PAL_DELTA_LOOP(sColorABGR,   image.mpalet, image.tpalet, duration); break;
        case EPalRGBA32: PAL_DELTA_LOOP(sColorRGBA32, image.mpalet, image.tpalet, duration); break;
        case EPal565: PAL_DELTA_LOOP(sColor565,  (u16 *)image.mpalet, (u16 *)image.tpalet, duration); break;
        }

        image.palt = 1;
        image.palt0 = 1;
        image.palc = duration;
    }
}

void savepal(s16 mode)
{
    if (mode < 0 && -3 < mode)
    {
        u32 *tgt = (mode != -1) ? image.svpalet2 : image.svpalet;
        memcpy(tgt, image.tpalet, 256 * sizeof(u32));
    }
}

void restorepal(s16 mode, s32 duration)
{
    // not used in some games (Metal Mutant, ...)
    if (alis.platform.uid == EGameMetalMutant)
    {
        return;
    }

    u32 *src = (mode != -1) ? image.svpalet2 : image.svpalet;
    memcpy(image.tpalet, src, 256 * sizeof(u32));

    image.thepalet = 0;
    image.defpalet = 0;
    selpalet();
    
    if (duration == 0)
    {
        memcpy(image.mpalet, src, 256 * sizeof(u32));
        image.ftopal = 0xff;
        image.palc = 0;
#if ALIS_SDL_VER < 2
        dirty_pal = 1;
#endif
    }
    else
    {
        switch (image.pal_format) {
        case EPalARGB: PAL_DELTA_LOOP(sColorARGB, image.mpalet, image.tpalet, duration); break;
        case EPalABGR: PAL_DELTA_LOOP(sColorABGR,   image.mpalet, image.tpalet, duration); break;
        case EPalRGBA32: PAL_DELTA_LOOP(sColorRGBA32, image.mpalet, image.tpalet, duration); break;
        case EPal565: PAL_DELTA_LOOP(sColor565,  (u16 *)image.mpalet, (u16 *)image.tpalet, duration); break;
        }

        topalet();
        image.palt = 1;
        image.palt0 = 1;
        image.palc = duration;
    }
}

// ============================================================================
// DOS palette model (PC data, version >= 12): the originals keep a 6-bit target (tpal) and a
// live palette (mpal) and fade by chasing mpal toward tpal on the vblank tick. m68k games
// never come here. Families differ in a few opcode details (see dos_pal_family).
// ============================================================================

enum { DPAL_NONE, DPAL_C16, DPAL_BUNNY, DPAL_ISHAR, DPAL_B32 };

static struct {
    u8 tpal[768], mpal[768], sav1[768], sav2[768];
    u8 step, reload, skip;
    s8 counter;
    volatile u8 ftopal, lock;
    u32 acc;
} dpal;

// Colorado: 16 colours, no fades. Bunny Bricks / Ishar 1, 2 (Transarctica assumed): the
// 16-bit driver. RRQ / Ishar 3: the 32-bit driver. Other PC games keep the generic path.
static int dos_pal_family(void)
{
    if (alis.platform.kind != EPlatformPC)
        return DPAL_NONE;

    switch (alis.platform.uid) {
        case EGameColorado:             return DPAL_C16;
        case EGameBunnyBricks:          return DPAL_BUNNY;
        case EGameIshar_1:
        case EGameIshar_2:
        case EGameTransarctica:         return DPAL_ISHAR;
        case EGameIshar_3:
        case EGameRobinsonsRequiem0:
        case EGameRobinsonsRequiem1:    return DPAL_B32;
        default:                        return DPAL_NONE;
    }
}

int dos_pal_active(void) { return dos_pal_family() != DPAL_NONE; }

static u8 dpal_expand(u8 v) { v &= 0x3f; return (u8)((v << 2) | (v >> 4)); }

// The DAC write: the live 6-bit palette becomes the host palette.
static void dpal_upload(void)
{
    int n = dos_pal_family() == DPAL_C16 ? 16 : 256;
    for (int i = 0; i < n; i++)
        PAL_WRITE_RGB(image.mpalet, i, dpal_expand(dpal.mpal[i * 3]), dpal_expand(dpal.mpal[i * 3 + 1]), dpal_expand(dpal.mpal[i * 3 + 2]));
#if ALIS_SDL_VER < 2
    dirty_pal = 1;
#endif
    set_update_cursor();   // the host cursor image is built from the palette
}

// image.tpalet mirrors the target so save states keep it.
static void dpal_mirror_target(void)
{
    for (int i = 0; i < 256; i++)
        PAL_WRITE_RGB(image.tpalet, i, dpal_expand(dpal.tpal[i * 3]), dpal_expand(dpal.tpal[i * 3 + 1]), dpal_expand(dpal.tpal[i * 3 + 2]));
}

static void dpal_chase(void)
{
    dpal.ftopal = 0;
    for (int i = 0; i < 768; i++)
    {
        u8 t = dpal.tpal[i], m = dpal.mpal[i];
        if (t == m)
            continue;

        dpal.ftopal = 1;
        if (t > m)
            dpal.mpal[i] = m + dpal.step < t ? m + dpal.step : t;
        else
            dpal.mpal[i] = m - dpal.step > t ? m - dpal.step : t;
    }
}

// Fade speed from a duration. Returns 0 when the set is immediate.
static int dpal_steps(s16 dur, int inc)
{
    dpal.step = 1;
    if (dur == 0)
        return 0;

    if (dur < 63)
    {
        u8 cl = dur & 0xff;
        if (cl == 0)
            return 0;   // the original divides by zero here

        dpal.step = 63 / cl + 1;
        dpal.reload = 1;
        dpal.counter = 1;
    }
    else
    {
        s32 q = dur / 63 + inc;
        if (q > 127) q = 127;
        dpal.reload = (u8)q;
        dpal.counter = (s8)q;
    }

    return 1;
}

// Immediate set or the first fade step; the 16-bit driver writes the DAC at once.
static void dpal_apply(int fade)
{
    dpal.lock = 1;
    if (fade)
    {
        dpal_chase();
        dpal.ftopal = 1;
    }
    else
    {
        dpal.counter = dpal.reload = 0;
        memcpy(dpal.mpal, dpal.tpal, sizeof(dpal.mpal));
        dpal.ftopal = 0xff;
        if (dos_pal_family() != DPAL_B32)
            dpal_upload();
    }
    dpal.lock = 0;
    dpal_mirror_target();
}

static void dpal_load16(u8 *words, int first)
{
    int fam = dos_pal_family();
    for (int i = 0; i < 16 && first + i < 256; i++)
    {
        u8 hi = words[i * 2], lo = words[i * 2 + 1];
        u8 c[3] = { (u8)((hi & 0x0f) << 3), (u8)((lo & 0xf0) >> 1), (u8)((lo & 0x0f) << 3) };
        for (int k = 0; k < 3; k++)
        {
            if (fam == DPAL_B32)    // expanded to 8 bits, then the normal >>2 load
                c[k] = c[k] ? (u8)(((c[k] >> 3) << 5 | 0x1f) >> 2) : 0;
            else if (fam != DPAL_C16 && c[k])
                c[k] |= 7;
            dpal.tpal[(first + i) * 3 + k] = c[k];
        }
    }
}

// RRQ's dark palette applied while loading (8-bit source).
static u8 dpal_dark(u8 c8, u16 dk)
{
    u32 p = (u32)c8 * dk;
    return (u8)((p > 0xffff ? 0xffff : p) >> 10);
}

static void dpal_load_dark(u8 *rgb8, int first, int n)
{
    u16 *dk = (u16 *)image.dkpalet;
    for (int i = 0; i < n; i++)
    {
        int e = first + i;
        u8 r = rgb8[i * 3], g = rgb8[i * 3 + 1], b = rgb8[i * 3 + 2];
        if (!r && !g && !b && (dk[e * 3] > 0x100 || dk[e * 3 + 1] > 0x100 || dk[e * 3 + 2] > 0x100))
            r = g = b = 2;
        dpal.tpal[e * 3]     = dpal_dark(r, dk[e * 3]);
        dpal.tpal[e * 3 + 1] = dpal_dark(g, dk[e * 3 + 1]);
        dpal.tpal[e * 3 + 2] = dpal_dark(b, dk[e * 3 + 2]);
    }
}

// topalette of a 0xFE resource (NULL: keep the target, as the restore path).
static void dpal_topalette(u8 *res, s16 dur)
{
    int fam = dos_pal_family();
    int fade = dpal_steps(dur, fam != DPAL_BUNNY);

    if (fam == DPAL_C16)
    {
        if (res)
            dpal_load16(res + 2, 0);
        dpal_apply(0);
        return;
    }

    if (res)
    {
        int n = res[1] + 1, first = res[2];
        if (n == 1)
        {
            dpal_load16(res + 2, first);
            if (fam != DPAL_B32)
                fade = 1;   // the 16-bit driver always takes the fade path here
        }
        else
        {
            if (first + n > 256)
                n = 256 - first;

            if (fam == DPAL_B32 && image.fdarkpal)
                dpal_load_dark(res + 4, first, n);
            else
                for (int k = 0; k < n * 3; k++)
                    dpal.tpal[first * 3 + k] = res[4 + k] >> 2;
        }
    }

    dpal_apply(fade);
    dpal.skip = 0;
}

static void dpal_ctopalsav(s16 idx, s16 dur)
{
    u8 *src = idx == -1 ? dpal.sav1 : dpal.sav2;
    if (dos_pal_family() == DPAL_B32 && image.fdarkpal)
    {
        u8 rgb8[768];
        for (int k = 0; k < 768; k++)
            rgb8[k] = src[k] << 2;
        dpal_load_dark(rgb8, 0, 256);
    }
    else
    {
        memcpy(dpal.tpal, src, sizeof(dpal.tpal));
    }

    dpal_topalette(NULL, dur);
}

void dos_pal_reset(void)
{
    memset(&dpal, 0, sizeof(dpal));
}

void dos_cpalette(s16 idx, u8 *res)
{
    int fam = dos_pal_family();
    if (idx < 0)
    {
        if ((fam == DPAL_ISHAR || fam == DPAL_B32) && idx >= -2)
        {
            dpal.counter = dpal.reload = 0;
            dpal_ctopalsav(idx, 0);
        }
    }
    else if ((fam == DPAL_BUNNY || fam == DPAL_C16 || !dpal.skip) && res[0] == 0xfe)
    {
        dpal.counter = dpal.reload = 0;
        dpal_topalette(res, 0);
        if (fam != DPAL_B32 && fam != DPAL_C16)
            dpal.ftopal = 1;
    }
    dpal.skip = 0;
}

void dos_ctopalet(s16 idx, s16 dur, u8 *res)
{
    int fam = dos_pal_family();
    if (fam == DPAL_BUNNY || fam == DPAL_C16)
    {
        if (idx >= 0 && res[0] == 0xfe)
            dpal_topalette(res, fam == DPAL_C16 ? 0 : dur);
    }
    else if (!dpal.skip)
    {
        if (idx < 0)
        {
            if (idx >= -2)
                dpal_ctopalsav(idx, dur);
        }
        else if (res[0] == 0xfe)
        {
            dpal_topalette(res, dur);
        }
    }
    dpal.skip = 0;
}

void dos_ctoblack(s16 dur)
{
    if (dos_pal_family() == DPAL_C16)
    {
        memset(dpal.tpal, 0, sizeof(dpal.tpal));
        dpal_apply(0);
        return;
    }

    int fade = dpal_steps(dur, 0);
    memset(dpal.tpal, 0, sizeof(dpal.tpal));
    dpal.lock = 1;
    if (!fade)
    {
        dpal.counter = dpal.reload = 0;
        memset(dpal.mpal, 0, sizeof(dpal.mpal));
    }
    dpal_chase();
    dpal.ftopal = 1;
    dpal.lock = 0;
    dpal_mirror_target();
    dpal.skip = 0;
}

void dos_cselpalet(s16 val)
{
    if (dos_pal_family() != DPAL_C16)
        dpal.skip = (u8)val;
}

void dos_cdefcolor(s16 idx, u16 val)
{
    switch (dos_pal_family()) {
        case DPAL_C16:
            if (idx >= 0 && idx < 16)
            {
                u8 w[2] = { (u8)(val >> 8), (u8)val };
                dpal_load16(w, idx);
                memcpy(&dpal.mpal[idx * 3], &dpal.tpal[idx * 3], 3);
                dpal_upload();
            }
            break;
        case DPAL_BUNNY:
        case DPAL_ISHAR:
            // The original indexes the 3-byte entries by 2 (its bug).
            if (idx >= 0 && idx * 2 + 1 < 768)
            {
                dpal.mpal[idx * 2] = (u8)(val >> 8);
                dpal.mpal[idx * 2 + 1] = (u8)val;
                dpal_upload();
            }
            break;
        default:
            break;
    }
}

void dos_csavepal(s16 idx)
{
    int fam = dos_pal_family();
    if (fam != DPAL_ISHAR && fam != DPAL_B32)
        return;

    if (idx == -1)
        memcpy(dpal.sav1, dpal.tpal, sizeof(dpal.tpal));
    else if (idx == -2)
        memcpy(dpal.sav2, dpal.tpal, sizeof(dpal.tpal));
}

// 0xFE resource drawn with put: unreachable in the originals (their path crashes); set it now.
void dos_putin_palette(u8 *res)
{
    dpal.counter = dpal.reload = 0;
    dpal_topalette(res, 0);
}

// RRQ/Ishar 3 film colour chunk: written straight into the live palette, entry 0 black.
int dos_film_palette(u8 *addr)
{
    if (dos_pal_family() != DPAL_B32)
        return 0;

    u16 packets = read16le(addr), index = 0;
    addr += 2;
    dpal.lock = 1;
    do
    {
        index += *addr++;
        u16 len = *addr++;
        if (len == 0)
            len = 256;
        if (index + len > 256)
        {
            len = 256 - index;
            packets = 1;
        }
        for (int k = 0; k < len * 3; k++)
            dpal.mpal[index * 3 + k] = addr[k] & 0x3f;
        addr += len * 3;
        index += len;
    }
    while (--packets);
    dpal.mpal[0] = dpal.mpal[1] = dpal.mpal[2] = 0;
    dpal.lock = 0;
    dpal_upload();
    return 1;
}

// One vblank of the original timer: upload what is live, then step the fade.
static void dpal_tick(void)
{
    if (dpal.lock)
        return;

    dpal_upload();
    if (dos_pal_family() == DPAL_B32 && dpal.ftopal == 0xff)
    {
        dpal.ftopal = 0;
        return;
    }

    if (--dpal.counter >= 0)
        return;

    dpal.counter = dpal.reload;
    dpal_chase();
}

// Called from itroutine; fade steps follow the pace rate (the DOS originals: VGA refresh).
void dos_pal_tick(void)
{
    u32 hz = sys_timeclock_hz ? sys_timeclock_hz : 60;
    for (dpal.acc += sys_pace_hz(); dpal.acc >= hz; dpal.acc -= hz)
        if (dpal.ftopal)
            dpal_tick();
}

// After a save state load: rebuild the 6-bit state from the restored host palettes.
void dos_pal_sync_from_image(void)
{
    if (!dos_pal_active())
        return;

    for (int i = 0; i < 256; i++)
    {
        u8 r, g, b;
        PAL_READ_RGB(image.mpalet, i, r, g, b);
        dpal.mpal[i * 3] = r >> 2; dpal.mpal[i * 3 + 1] = g >> 2; dpal.mpal[i * 3 + 2] = b >> 2;
        PAL_READ_RGB(image.tpalet, i, r, g, b);
        dpal.tpal[i * 3] = r >> 2; dpal.tpal[i * 3 + 1] = g >> 2; dpal.tpal[i * 3 + 2] = b >> 2;
    }
    dpal.counter = dpal.reload = 0;
    dpal.step = 1;
    dpal.ftopal = 1;
}

#if defined(ALIS_TRACE_PAL)
void pal_trace(const char *op, s32 a, s32 b)
{
    printf("PAL %-10s %5d %5d  [%s] sel=%d flinepal=%d tl:", op, (int)a, (int)b,
           alis.script ? alis.script->name : "?", (int)image.thepalet, (int)image.flinepal);
    for (int i = 0; i < 8 && image.tlinepal[i * 2] < 0xff; i++)
        printf(" %d:%d", image.tlinepal[i * 2], image.tlinepal[i * 2 + 1]);
    // Colours 1..3 of bank 0 and bank 2: shown (m) and target (t).
    printf("  m0=%06x,%06x,%06x m2=%06x,%06x,%06x t0=%06x,%06x,%06x t2=%06x,%06x,%06x palc=%d",
           image.mpalet[1] & 0xffffff, image.mpalet[2] & 0xffffff, image.mpalet[3] & 0xffffff,
           image.mpalet[129] & 0xffffff, image.mpalet[130] & 0xffffff, image.mpalet[131] & 0xffffff,
           image.tpalet[1] & 0xffffff, image.tpalet[2] & 0xffffff, image.tpalet[3] & 0xffffff,
           image.tpalet[129] & 0xffffff, image.tpalet[130] & 0xffffff, image.tpalet[131] & 0xffffff,
           (int)image.palc);
    printf("\n");
    fflush(stdout);
}
#endif

void selpalet(void)
{
    if (alis.platform.bpp != 8)
    {
        s16 offset = image.thepalet * 64;
        image.ampalet = image.mpalet + offset;
        image.atpalet = image.tpalet + offset;
        // Only the write bank moves: the display stays on bank 0 (other banks show through
        // clinepalet), as the original. Flag a pal16/CLUT refresh.
#if ALIS_SDL_VER < 2
        dirty_pal = 1;
#endif
    }
}

void linepal(void)
{
    s32 begline = 0xc6;
    s16 endline = 0x2f;

    s16 line;
    s16 dummy;
    
    s16 *tlinepal_ptr = image.tlinepal;
    s16 *palentry = image.firstpal;
    
    for (int i = 0; i < 4; i++)
    {
        line = tlinepal_ptr[0];
        if (alis.platform.height < line)
            break;
        
        dummy = line == 0 ? line : ((u16)(line * begline) >> 8) + endline;
        
        palentry[0] = dummy;
        palentry[1] = line;
        palentry[2] = tlinepal_ptr[1] * 64;
        
        tlinepal_ptr += 2;
        palentry += 2 + (sizeof(u8 *) >> 1);
    }
    
    palentry[0] = 0xff;
    palentry[1] = 0;
    palentry[2] = 0;
}

// Native-planar clinepal: stamp the per-row bank into planes 4-5 (image_draw_planar.c).
#if defined(ALIS_NATIVE_PLANAR)
extern void planar_linepal_enable(void);
extern void planar_linepal_disable(void);
#define PLANAR_LINEPAL_ENABLE()  planar_linepal_enable()
#define PLANAR_LINEPAL_DISABLE() planar_linepal_disable()
#else
#define PLANAR_LINEPAL_ENABLE()  ((void)0)
#define PLANAR_LINEPAL_DISABLE() ((void)0)
#endif

void setlinepalet(void) {

    if (alis.varD7 < 0)
    {
        image.flinepal = 0;
        image.tlinepal[0] = 0;
        image.tlinepal[1] = 0;
        image.tlinepal[2] = 0xff;
        image.tlinepal[3] = 0;
        linepal();
#if ALIS_SDL_VER < 2
        dirty_pal = 1;
#endif
        PLANAR_LINEPAL_DISABLE();
        return;
    }
    
    if (image.logy2 <= alis.varD7)
    {
        alis.varD7 = image.logy2;
    }

    s16 *tlinepal_ptr = image.tlinepal;
    s16 *prevtlpal_ptr;
    
    do
    {
        prevtlpal_ptr = tlinepal_ptr;
        if (alis.varD7 == prevtlpal_ptr[0])
        {
            if (alis.varD6 == prevtlpal_ptr[1])
                return;
            
            prevtlpal_ptr[1] = alis.varD6;
            image.flinepal = 1;
            linepal();
#if ALIS_SDL_VER < 2
            dirty_pal = 1;
#endif
            PLANAR_LINEPAL_ENABLE();
            return;
        }
        
        tlinepal_ptr = prevtlpal_ptr + 2;
    }
    while (prevtlpal_ptr[0] < alis.platform.height);
    
    // check whether we are not using too many palettes
    if (prevtlpal_ptr - (image.tlinepal + 8) < 0)
    {
        do
        {
            prevtlpal_ptr = tlinepal_ptr;
            prevtlpal_ptr[0] = prevtlpal_ptr[-2];
            prevtlpal_ptr[1] = prevtlpal_ptr[-1];
            tlinepal_ptr = prevtlpal_ptr - 2;
        }
        while ((s16)(alis.varD7 - prevtlpal_ptr[0]) < 0);
        
        prevtlpal_ptr[0] = alis.varD7;
        prevtlpal_ptr[1] = alis.varD6;
        image.flinepal = 1;
        linepal();
#if ALIS_SDL_VER < 2
        dirty_pal = 1;
#endif
        PLANAR_LINEPAL_ENABLE();
    }
}

void setmpalet(void)
{
    image.ftopal = 0xff;
    image.thepalet = 0;
    image.defpalet = 0;
    // Direct palette write (cdefcolor): rebuild pal16 / CLUT.
#if ALIS_SDL_VER < 2
    dirty_pal = 1;
#endif
}


#pragma mark -
#pragma mark Sprite management


void printelem(void)
{
    s32 cursprit = image.debsprit;
    cursprit += 0x78;
    cursprit += 0x60;

    do
    {
        cursprit += 0x30;
    }
    while (cursprit < image.debsprit + 16 * 0x30);
}

void inisprit(void)
{
    image.debsprit = 0;
    image.finsprit = 0xffff;

    s32 cursprit = image.debsprit;
    image.tvsprite = 0x8000;
    image.basesprite = image.debsprit + 0x8000;
    *(s32 *)(image.spritemem + cursprit + 0x0c) = image.logx1;
    *(s16 *)(image.spritemem + cursprit + 0x16) = image.logx2;
    *(s16 *)(image.spritemem + cursprit + 0x18) = image.logy2;
    *(s8 *) (image.spritemem + cursprit + 0x29) = image.logy1;
    image.backsprite = 0x000e; // 0x8028;
    image.texsprite = 0x0042; // 0x8050;
    *(s32 *)(image.spritemem + cursprit + 0x5c) = 0;
    *(s8 *) (image.spritemem + cursprit + 0x50) = 0xfe;
    alis.mousflag = 0;
    image.atexsprite = 0x78;
    image.libsprit = 0xa8;
    cursprit += 0xa8;
    
    sSprite *sprite = SPRITE_VAR(cursprit);
    for (; cursprit < image.finsprit; sprite = SPRITE_VAR(cursprit), cursprit += 0x30)
    {
        sprite->to_next = cursprit + 0x30;
    }

    sprite->to_next = 0;
}

u8 searchelem(u16 *curidx, u16 *previdx)
{
    *curidx = get_0x18_unknown(alis.script->vram_org);
    if (*curidx != 0)
    {
        sSprite *cursprvar = NULL;
        s16 screen_id;
        s8 num;

        do
        {
            cursprvar = SPRITE_VAR(*curidx);
            screen_id = cursprvar->screen_id;
            if (screen_id <= get_0x16_screen_id(alis.script->vram_org))
            {
                if (get_0x16_screen_id(alis.script->vram_org) != screen_id)
                    break;
                
                num = cursprvar->numelem;
                if (num <= image.numelem)
                {
                    if (image.numelem == num)
                    {
                        return 1;
                    }
                    
                    break;
                }
            }
            
            *previdx = *curidx;
            *curidx = cursprvar->to_next;
        }
        while (*curidx != 0);
    }
    
    return 0;
}

s8 searchtete(u16 *curidx, u16 *previdx)
{
    *previdx = 0;
    *curidx = get_0x18_unknown(alis.script->vram_org);
    if (*curidx != 0)
    {
        s16 screenid = get_0x16_screen_id(alis.script->vram_org);
        
        do
        {
            sSprite *sprite = SPRITE_VAR(*curidx);
            s16 sprscid = sprite->screen_id;
            if (sprscid <= screenid)
            {
                if (screenid == sprscid)
                    return screenid == sprscid;
                
                break;
            }
            
            *previdx = *curidx;
            *curidx = sprite->to_next;
        }
        while (*curidx != 0);
    }
    
    return 0;
}

u8 testnum(u16 *curidx)
{
    sSprite *cursprvar = SPRITE_VAR(*curidx);
    return (cursprvar != NULL && cursprvar->screen_id == get_0x16_screen_id(alis.script->vram_org) && cursprvar->numelem == image.numelem) ? 1 : 0;
}

u8 nextnum(u16 *curidx, u16 *previdx)
{
    *previdx = *curidx;
    *curidx = SPRITE_VAR(*curidx)->to_next;
    sSprite *cursprvar = SPRITE_VAR(*curidx);
    return (cursprvar != NULL && cursprvar->screen_id == get_0x16_screen_id(alis.script->vram_org) && cursprvar->numelem == image.numelem) ? 1 : 0;
}

void createlem(u16 *curidx, u16 *previdx)
{
    sSprite *cursprvar = SPRITE_VAR(image.libsprit);

    if (image.libsprit != 0)
    {
        u16 sprit = image.libsprit;
        u16 nextsprit = cursprvar->to_next;

        if (*previdx == 0)
        {
            set_0x18_unknown(alis.script->vram_org, image.libsprit);
            image.libsprit = nextsprit;
        }
        else
        {
            sSprite *prevsprvar = SPRITE_VAR(*previdx);
            prevsprvar->to_next = image.libsprit;
            image.libsprit = nextsprit;
        }

        s16 scrnidx = get_scr_screen_id((get_0x16_screen_id(alis.script->vram_org)));
        sSprite *scrnsprvar = SPRITE_VAR(scrnidx);

        cursprvar->state = -1;
        cursprvar->numelem = image.numelem;
        cursprvar->screen_id = get_0x16_screen_id(alis.script->vram_org);
        cursprvar->to_next = *curidx;
        cursprvar->link = scrnsprvar->link;
        scrnsprvar->link = sprit;

        *curidx = sprit;
    }
}

void delprec(u16 elemidx)
{
    sSprite *cursprvar = SPRITE_VAR(elemidx);
    s16 scridx = cursprvar->screen_id;
    if (elemidx == get_scr_screen_id(scridx))
    {
        // Deleting the list head: new head is the sprite's link (not its screen_id).
        set_scr_screen_id(scridx, cursprvar->link);
        return;
    }

    u16 spridx = get_scr_screen_id(scridx);
    sSprite *tmpsprvar = SPRITE_VAR(spridx);
    while (tmpsprvar != NULL && elemidx != tmpsprvar->link)
    {
        spridx = tmpsprvar->link;
        tmpsprvar = SPRITE_VAR(spridx);
    }
    
    if (tmpsprvar != NULL)
    {
        tmpsprvar->link = cursprvar->link;
    }
}

void killelem(u16 *curidx, u16 *previdx)
{
    sSprite *cursprvar = SPRITE_VAR(*curidx);
    
    if (alis.ferase == 0 && -1 < cursprvar->state)
    {
        cursprvar->state = 1;
        
        if (*previdx == 0)
        {
            set_0x18_unknown(alis.script->vram_org, cursprvar->to_next);
        }
        else
        {
            SPRITE_VAR(*previdx)->to_next = cursprvar->to_next;
        }
    }
    else
    {
        if (*previdx == 0)
        {
            set_0x18_unknown(alis.script->vram_org, cursprvar->to_next);
        }
        else
        {
            SPRITE_VAR(*previdx)->to_next = cursprvar->to_next;
        }
        
        cursprvar->to_next = image.libsprit;
        image.libsprit = *curidx;
        
        delprec(*curidx);
    }

    if (*previdx == 0)
    {
        *curidx = get_0x18_unknown(alis.script->vram_org);
        return;
    }
    
    *curidx = SPRITE_VAR(*previdx)->to_next;
}

void getelem(u16 *newidx, u16 *oldidx)
{
    u8 ret = searchelem(newidx, oldidx);
    if (ret)
    {
        do
        {
            if (SPRITE_VAR(*newidx)->state == 0)
            {
                SPRITE_VAR(*newidx)->state = 2;
                return;
            }
            
            ret = nextnum(newidx, oldidx);
        }
        while (ret);
    }
    
    createlem(newidx, oldidx);
}

void put(u16 idx)
{
    alis.fmuldes = 0;
    putin(idx);
}

void putfin(void)
{
    // kill leftover sprite elements after composite processing
    if (alis.fmuldes == 0)
    {
        u16 newidx = 0;
        u16 oldidx = 0;

        if (searchelem(&newidx, &oldidx) != 0)
        {
            do
            {
                while (SPRITE_VAR(newidx)->state == 0)
                {
                    killelem(&newidx, &oldidx);
                    if (newidx == 0)
                    {
                        alis.fadddes = 0;
                        return;
                    }

                    if (!testnum(&newidx))
                    {
                        alis.fadddes = 0;
                        return;
                    }
                }
            }
            while (nextnum(&newidx, &oldidx));
        }
    }
    
    alis.fadddes = 0;
}

void scalaire_v10(s16 scene, s16 *x, s16 *y, s16 *z)
{
    u32 m = alis.basemain + scene + 0x1a;
    s16 x0 = *x, y0 = *y, z0 = *z;
    if (get_scr_numelem(scene) < 0)
    {
        *x = (s8)xread8(m + 0) * x0 + (s8)xread8(m + 2) * z0;
        if (alis.platform.kind == EPlatformPC)
            *x += (s8)xread8(m + 1) * y0;
        *y = (s8)xread8(m + 3) * x0 + (s8)xread8(m + 4) * y0 + (s8)xread8(m + 5) * z0;
        *z = (s8)xread8(m + 6) * x0 + (s8)xread8(m + 7) * y0 + (s8)xread8(m + 8) * z0;
    }
    else
    {
        *y = y0 * (s8)xread8(m + 4) - z0;
        *z = y0;
    }
}

static void topix_v10(s16 scene, s16 *x, s16 *y, s16 *z)
{
    u32 m = alis.basemain + scene;
    *x = image.oldcx - xread16(m + 0x12);
    *y = image.oldcy - xread16(m + 0x14);
    *z = image.oldcz - xread16(m + 0x16);
    scalaire_v10(scene, x, y, z);

    u8 sh = xread8(m + 0x18) & 0x3f;
    *x = (*x >> sh) + xread16(m + 0xa);
    *y = (*y >> sh) + xread16(m + 0xc);
    *z >>= sh;
}

void picture_v10(u16 idx)
{
    s32 addr = adresdes(idx);
    u8 *res = alis.mem + addr;
    if (res[0] == 0xfe)
    {
        if (alis.platform.kind != EPlatformPC)
            topalette(res, 0);
        return;
    }

    if (!(res[0] == 0 || (alis.platform.kind == EPlatformPC && res[0] <= 2)))
    {
        s16 x = image.depx, y = image.depy, z = image.depz;
        u8 invx = image.invert_x;
        for (u8 *cur = res + 2, *end = cur + res[1] * 8; cur < end; cur += 8)
        {
            s16 elem = read16(cur);
            image.depx += image.invert_x ? -read16(cur + 2) : read16(cur + 2);
            image.depy += read16(cur + 4);
            image.depz += read16(cur + 6);
            if (elem < 0)
            {
                elem &= 0x7fff;
                image.invert_x ^= 1;
            }

            picture_v10(elem);
            image.depx = x;
            image.depy = y;
            image.depz = z;
            image.invert_x = invx;
        }
        return;
    }

    sSprite pic = {0};
    pic.data = pic.newad = addr;
    pic.newf = image.invert_x;
    pic.width = read16(res + 2);
    pic.height = read16(res + 4);
    pic.newx = alis.poldx + image.depx - (pic.width >> 1);
    pic.newy = alis.poldy + image.depy - (pic.height >> 1);

    s16 cx1 = image.clipx1, cy1 = image.clipy1, cx2 = image.clipx2, cy2 = image.clipy2;
    image.clipx1 = 0;
    image.clipy1 = 0;
    image.clipx2 = alis.platform.width - 1;
    image.clipy2 = alis.platform.height - 1;
    destofen(&pic);
    image.clipx1 = cx1;
    image.clipy1 = cy1;
    image.clipx2 = cx2;
    image.clipy2 = cy2;
}

void putin(u16 idx)
{
    // if there is no scene opened, there is nowhere to add sprite
    s16 scrnidx = get_scr_screen_id((get_0x16_screen_id(alis.script->vram_org)));
    if (scrnidx == 0)
        return;
    
    s16 x = image.depx;
    s16 z = image.depz;
    s16 y = image.depy;
    
    u8 *resourcedata;
    u8 compositimg;
    
    s32 addr = adresdes(idx);
    if (alis.platform.version == 10)
    {
        resourcedata = alis.mem + addr;
        if (alis.platform.kind == EPlatformPC)
        {
            compositimg = resourcedata[0] > 2;
            if (compositimg && resourcedata[0] == 0xfe)
                return;
        }
        else
        {
            compositimg = resourcedata[0];
        }
    }
    else
    {
        addr += xread32(addr);
        resourcedata = alis.mem + addr;
        compositimg = resourcedata[0] > 0x80;
    }
    
    if (compositimg)
    {
        if (resourcedata[0] == 0xfe)
        {
            if (dos_pal_active())
                dos_putin_palette(resourcedata);
            else
                topalette(resourcedata, 0);
            return;
        }
        
        // handle composit images
        
        s16 rsrccount = resourcedata[1];
        u8 *currsrc = resourcedata + 2;
        
        u8 invx;
        u8 muldes;
        
        for (s32 i = 0; i < rsrccount; i++)
        {
            invx = image.invert_x;
            muldes = alis.fmuldes;
            
            s16 curelem = read16(currsrc + 0);
            s16 curdepx = read16(currsrc + 2);
            if (image.invert_x != 0)
                curdepx = -curdepx;
            
            image.depx += curdepx;
            
            s16 curdepy = read16(currsrc + 4);
            image.depy = curdepy + y;
            
            s16 curdepz = read16(currsrc + 6);
            image.depz = curdepz + z;
            
            if (curelem < 0)
            {
                curelem = curelem & 0x7fff;
                image.invert_x ^= 1;
            }
            
            alis.fmuldes = 1;
            
            putin(curelem);
            
            image.depx = x;
            image.depy = y;
            image.depz = z;
            
            image.invert_x = invx;
            alis.fmuldes = muldes;
            
            currsrc += 8;
        }
        
        alis.fmuldes = 0;
    }
    else
    {
        u16 newidx = 0;
        u16 oldidx = 0;
        
        if (alis.fadddes == 0)
        {
            if (alis.fmuldes)
            {
                getelem(&newidx, &oldidx);
            }
            else
            {
                if (searchelem(&newidx, &oldidx) == 0)
                    createlem(&newidx, &oldidx);
                
                if (-1 < SPRITE_VAR(newidx)->state)
                    SPRITE_VAR(newidx)->state = 2;
            }
        }
        else
        {
            getelem(&newidx, &oldidx);
        }
        
        addr = adresdes(idx);
        putmapin(newidx, addr);
        return;
    }
    
    putfin();
}

void putmapin(u16 spridx, s32 bitmap)
{
    sSprite *sprite = SPRITE_VAR(spridx);
    sprite->data = bitmap;
    sprite->newad = 0;
    sprite->flaginvx = image.invert_x;

    if (alis.platform.version == 10)
    {
        s16 x, y, z;
        topix_v10(get_0x16_screen_id(alis.script->vram_org), &x, &y, &z);
        sprite->script_ent = get_0x0e_script_ent(alis.script->vram_org);
        sprite->clinking = -1;
        sprite->cordspr = 0;
        sprite->chsprite = -1;
        sprite->creducing = 0;
        sprite->credon_off = -1;
        sprite->depx = x + image.depx;
        sprite->depy = y + image.depy;
        sprite->depz = z + image.depz;
        putfin();
        return;
    }
    
    sprite->sprite_0x28 = get_0x28_unknown(alis.script->vram_org);
    sprite->script_ent = get_0x0e_script_ent(alis.script->vram_org);
    sprite->clinking = get_0x2a_clinking(alis.script->vram_org);
    sprite->cordspr = get_0x2b_cordspr(alis.script->vram_org);
    sprite->creducing = get_0x27_creducing(alis.script->vram_org);
    sprite->credon_off = get_0x25_credon_credoff(alis.script->vram_org);

    s32 contextsize = get_context_size();
    if (contextsize > 0x2e)
    {
        sprite->chsprite = get_0x2f_chsprite(alis.script->vram_org);

        if ((s8)sprite->credon_off == -0x80)
        {
            sprite->newzoomx = get_0x38_unknown(alis.script->vram_org);
            sprite->newzoomy = get_0x36_unknown(alis.script->vram_org);
            sprite->creducing = 0;
        }
    }

    if (alis.platform.kind == EPlatformPC && alis.platform.uid == EGameColorado)
    {
        // Colorado DOS: its credon test never branches, so the context values are always taken.
        sprite->creducing = get_0x27_creducing(alis.script->vram_org);
        sprite->credon_off = get_0x26_creducing(alis.script->vram_org);
    }
    else if ((s8)sprite->credon_off >= 0)
    {
        sprite->creducing = get_0x27_creducing(alis.script->vram_org);
        sprite->credon_off = get_0x26_creducing(alis.script->vram_org);
        if ((s8)sprite->credon_off < 0)
        {
            sprite->creducing = 0;
            sprite->credon_off = xread8(alis.basemain + get_0x16_screen_id(alis.script->vram_org) + 0x1f);
        }
    }
    
    sprite->depx = image.oldcx + image.depx;
    sprite->depy = image.oldcy + image.depy;
    sprite->depz = image.oldcz + image.depz;
    
    putfin();
}

void put_char(s8 character)
{
    if (alis.charmode == 0)
    {
        printf("%c", character);
        // Prompts ("NDECOR: ") have no newline; flush before getval() blocks.
        fflush(stdout);
        return;
    }
    
    if (alis.charmode != 1)
    {
        if (alis.charmode != 2)
        {
            return;
        }
        
        if (character == '\n')
        {
            image.depz -= alis.fohaut;
        }
        else if (character == '\r')
        {
            image.depx = 0;
        }
        else
        {
            if (character != ' ')
            {
                if (alis.fonum < 0)
                {
                    return;
                }
                
                s16 charidx = ((s16)character) - alis.foasc;
                if ((-1 < charidx) && ((s16)(charidx - alis.fomax) < 0))
                {
                    alis.flagmain = 1;
                    image.invert_x = 0;
                    alis.fadddes = 1;
                    put(charidx + alis.fonum);
                }
            }
            
            image.depx += alis.folarg;
        }
    }
}

void put_string(void)
{
#ifndef NDEBUG
    if (disalis) {
       ALIS_DEBUG(EDebugInfo, " [\"%s\"]", alis.sd7);
    }
#endif
    for (char *strptr = alis.sd7; *strptr; strptr++)
    {
        put_char(*strptr);
    }
}

u32 tprintd0[] = { 0x3b9aca00, 0x05f5e100, 0x00989680, 0x000f4240, 0x000186a0, 0x00002710, 0x000003e8, 0x00000064, 0x0000000a, 0x00000001 };

void valtostr(char *string, s16 value)
{
    char *strptr = string;
    char *tmpptr;

    s32 tmpval = value;
    if (tmpval < 0)
    {
        strptr = string + 1;
        *string = 0x2d;
        tmpval = -tmpval;
    }
    
    s32 *tabptr = (s32 *)&tprintd0;
    s32 tprintv;
    s16 length = 9;
    s16 cw;
    char c;
    u8 res = 0;

    do
    {
        tprintv = *tabptr++;
        cw = -0x30;
        
        do
        {
            tmpval -= tprintv;
            if (tmpval < 0)
                break;
            
            cw --;
        }
        while (cw != -1);
        
        c = -(char)cw;
        if (c != 0x30)
        {
            res = 1;
            tmpptr = strptr + 1;
            *strptr = c;
        }
        else if (res)
        {
            tmpptr = strptr + 1;
            *strptr = c;
        }
        else
        {
            tmpptr = strptr;
        }
 
        tmpval += tprintv;
        length --;
        strptr = tmpptr;
        
        if (length == -1)
        {
            if (!res)
            {
                *strptr++ = c;
            }

            *strptr = 0;
            return;
        }
    }
    while (1);
}


#pragma mark -
#pragma mark calculate what to draw and where


// u16: the free list runs past 0x7fff; an s16 index sign-flips in SPRITE_VAR math.
s16 inilink(u16 elemidx)
{
    image.blocx1 = 0x7fff;
    image.blocy1 = 0x7fff;
    image.blocx2 = 0x8000;
    image.blocy2 = 0x8000;

    return SPRITE_VAR(elemidx)->clinking;
}

u8 calcfen(u16 elemidx1, u16 elemidx3)
{
    if (image.joints == 0)
    {
        sSprite *idx1sprvar = SPRITE_VAR(elemidx1);
        if (idx1sprvar->newd < 0)
            return 0;
        
        // Inclusive bounds (width is stored as w-1), as the original.
        image.blocx1 = idx1sprvar->newx;
        image.blocy1 = idx1sprvar->newy;
        image.blocx2 = image.blocx1 + idx1sprvar->width;
        image.blocy2 = image.blocy1 + idx1sprvar->height;
    }

    sSprite *idx3sprvar = SPRITE_VAR(elemidx3);
    s16 tmpx = idx3sprvar->newx;
    s16 tmpy = idx3sprvar->newy;
    s16 tmpw = idx3sprvar->depx;
    s16 tmph = idx3sprvar->depy;
    
    if (image.blocx1 <= tmpw && image.blocy1 <= tmph && tmpx <= image.blocx2 && tmpy <= image.blocy2)
    {
        image.fenx1 = image.blocx1;
        if (image.blocx1 < tmpx)
            image.fenx1 = tmpx;
        
        image.feny1 = image.blocy1;
        if (image.blocy1 < tmpy)
            image.feny1 = tmpy;
        
        image.fenx2 = image.blocx2;
        if (tmpw < image.blocx2)
            image.fenx2 = tmpw;
        
        image.feny2 = image.blocy2;
        if (tmph < image.blocy2)
            image.feny2 = tmph;

        // 16px-align X after clamping, as the original. clrvga_68k needs odd width,
        // clrvga_dos width % 4 == 0, skytofen constant clipx1 parity; planar blits whole chunks.
        // (DOS original aligns bloc before clamping.)
        {
            s16 ax1 = image.fenx1 & ~15;
            s16 ax2 = (image.fenx2 | 15);
            if (ax1 < 0) ax1 = 0;
            if (ax2 > alis.platform.width - 1) ax2 = alis.platform.width - 1;
            image.fenx1 = ax1;
            image.fenx2 = ax2;
        }

        image.fenlargw = (u16)((image.fenx2 - image.fenx1) + 1) >> 2;
        
        return 1;
    }
    
    return 0;
}

u8 clipfen(sSprite *sprite)
{
    image.fclip = 0;
    s16 spritex1 = sprite->newx;
    if (image.fenx2 < spritex1)
        return image.fclip;

    s16 spritey1 = sprite->newy;
    if (image.feny2 < spritey1)
        return image.fclip;

    s16 spritex2 = sprite->depx;
    if (spritex2 < image.fenx1)
        return image.fclip;

    s16 spritey2 = sprite->depy;
    if (spritey2 < image.feny1)
        return image.fclip;

    if (spritex1 < image.fenx1)
        spritex1 = image.fenx1;

    if (spritey1 < image.feny1)
        spritey1 = image.feny1;

    if (image.fenx2 < spritex2)
        spritex2 = image.fenx2;

    if (image.feny2 < spritey2)
        spritey2 = image.feny2;

    image.clipx1 = spritex1;
    image.clipy1 = spritey1;
    image.clipx2 = spritex2;
    image.clipy2 = spritey2;
    
    image.clipl = (image.clipx2 - image.clipx1) + 1;
    image.cliph = (image.clipy2 - image.clipy1) + 1;
    image.fclip = 1;
    return image.fclip;
}

u16 rangesprite(u16 elemidx1, u16 elemidx2, u16 elemidx3)
{
    sSprite *sprite1 = SPRITE_VAR(elemidx1);
    s16 newd1  =  sprite1->newd;
    s8 cordspr1 = sprite1->cordspr;
    s8 numelem1 = sprite1->numelem;
    if (image.wback != 0 && elemidx1 != image.backsprite && image.backprof <= newd1)
    {
        image.pback = 1;
    }

    sSprite *sprite3 = SPRITE_VAR(elemidx3);
    while (sprite3->link != 0)
    {
        sSprite *linksprite = SPRITE_VAR(sprite3->link);
        // Signed compare: cscback sets the backsprite's cordspr to 0x80 (-128) so it
        // sorts first at equal depth.
        if (-1 < linksprite->state && linksprite->newd <= newd1 && (linksprite->newd < newd1 || ((s8)linksprite->cordspr <= cordspr1 && ((s8)linksprite->cordspr < cordspr1 || (linksprite->numelem <= numelem1 && (linksprite->numelem < numelem1 || -1 == sprite1->state))))))
        {
            break;
        }
        
        elemidx3 = sprite3->link;
        sprite3 = linksprite;
    }

    sprite1->state = 0;
    sprite1->link = sprite3->link;
    sprite3->link = elemidx1;

    return (elemidx2 == elemidx3) ? elemidx1 : elemidx2;
}

void tstjoints(u16 elemidx)
{
    sSprite *sprvar = SPRITE_VAR(elemidx);
    s16 tmpx = sprvar->newx + sprvar->width;
    s16 tmpy = sprvar->newy + sprvar->height;
    
    if (-1 < sprvar->newd && -1 < image.newd && sprvar->newx <= image.newx + image.newl && sprvar->newy <= image.newy + image.newh && image.newx <= tmpx && image.newy <= tmpy)
    {
        image.blocx1 = sprvar->newx;
        if (image.newx < sprvar->newx)
            image.blocx1 = image.newx;

        image.blocy1 = sprvar->newy;
        if (image.newy < sprvar->newy)
            image.blocy1 = image.newy;

        if (tmpx < (s16)(image.newx + image.newl))
            tmpx = image.newx + image.newl;

        if (tmpy < (s16)(image.newy + image.newh))
            tmpy = image.newy + image.newh;

        // Inclusive bounds, like the whole bloc/fen pipeline.
        image.blocx2 = tmpx;
        image.blocy2 = tmpy;
        image.joints = 1;

        return;
    }
    
    image.joints = 0;
}

void scalaire(s16 scene, s16 *x, s16 *y, s16 *z)
{
    if (get_scr_numelem(scene) < 0)
    {
        s32 prevx = (get_scr_unknown0x20(scene) * *x + get_scr_unknown0x21(scene) * *y + get_scr_unknown0x22(scene) * *z);
        s32 prevy = (get_scr_unknown0x23(scene) * *x + get_scr_unknown0x24(scene) * *y + get_scr_unknown0x25(scene) * *z);
        s32 prevz = (get_scr_unknown0x26(scene) * *x + get_scr_unknown0x27(scene) * *y + get_scr_unknown0x28(scene) * *z);
        *x = prevx;
        *y = prevz;
        *z = prevy;
    }
    else
    {
        s16 prevy = *y;
        *y = *y * get_scr_unknown0x24(scene) - *z;
        *z = prevy;
    }
}

void depscreen(u16 scene, u16 elemidx)
{
    if (alis.platform.version == 10)
        return;

    set_scr_depx(scene, get_scr_depx(scene) + get_scr_unknown0x2a(scene));
    set_scr_depy(scene, get_scr_depy(scene) + get_scr_unknown0x2c(scene));
    set_scr_depz(scene, get_scr_depz(scene) + get_scr_unknown0x2e(scene));
    
    for (sSprite *sprite = SPRITE_VAR(elemidx); sprite; sprite = SPRITE_VAR(sprite->link))
    {
        if (-1 < (s8)sprite->chsprite && sprite->state == 0)
            sprite->state = 2;
    }
    
    if ((alis.platform.uid == EGameTarghan0 || alis.platform.uid == EGameTarghan1) && (alis.platform.kind == EPlatformAmiga || alis.platform.kind == EPlatformAtari))
    {
        set_scr_state(scene, get_scr_state(scene) & 0x7f);
    }
}

void deptopix(u16 scene, u16 elemidx)
{
    if (alis.platform.version == 10)
    {
        sSprite *sprite = SPRITE_VAR(elemidx);
        u8 *bmp = alis.mem + sprite->data;
        image.newf = sprite->flaginvx;
        image.newad = sprite->data;
        image.newl = read16(bmp + 2);
        image.newh = read16(bmp + 4);
        image.newzoomx = 0;
        image.newzoomy = 0;
        image.newx = sprite->depx - (image.newl >> 1);
        image.newy = sprite->depy - (image.newh >> 1);
        image.newd = sprite->depz;
        return;
    }

    if ((get_scr_numelem(scene) & 2) != 0)
    {
        landtopix(alis.basemain + scene, elemidx);
    }
    else
    {
        sSprite *elemsprvar = SPRITE_VAR(elemidx);
        
        scene = elemsprvar->screen_id;
        image.newf = elemsprvar->flaginvx;

        s16 tmpdepx = elemsprvar->depx - get_scr_depx(scene);
        s16 tmpdepy = elemsprvar->depy - get_scr_depy(scene);
        s16 tmpdepz = elemsprvar->depz - get_scr_depz(scene);

        scalaire(scene, &tmpdepx, &tmpdepy, &tmpdepz);

        s16 offset = 0;
        u8 cred = get_scr_creducing(scene);
        if (-1 < (s8)cred && -1 < (s8)elemsprvar->credon_off)
        {
            if (tmpdepz == 0)
            {
                tmpdepz = 1;
            }
            
            tmpdepx = (s16)(((s32)tmpdepx << (cred & 0x3f)) / tmpdepz);
            tmpdepy = (s16)(((s32)tmpdepy << (cred & 0x3f)) / tmpdepz);

            offset = (tmpdepz >> (elemsprvar->credon_off & 0x3f)) + (s8)elemsprvar->creducing;
            if (offset < 0)
            {
                offset = 0;
            }
            else
            {
                // NOTE: must be byte read - 16-bit read includes next field, gives wrong
                // values on big-endian. May need s16 for v3.0+ games in the future.
                int clink = (s8)xread8(alis.basemain + scene + 0x1e);
                if (clink < offset)
                {
                    offset = clink;
                }
                
                offset <<= 2;
            }
        }
        
        tmpdepx = (tmpdepx >> (get_scr_credon_off(scene) & 0x3f)) + get_scr_unknown0x0a(scene);
        tmpdepy = (tmpdepy >> (get_scr_credon_off(scene) & 0x3f)) + get_scr_unknown0x0c(scene);

        image.newad = elemsprvar->data + offset;
        
        u8 *spritedata = (alis.mem + image.newad + xread32(image.newad));
        if (spritedata[0] == 3)
        {
            tmpdepx += (elemsprvar->flaginvx ? -1 : 1) * read16(spritedata + 4);
            tmpdepy += read16(spritedata + 6);
            if (spritedata[1] != 0)
            {
                image.newf ^= 1;
            }
            
            image.newad += (s16)(read16(spritedata + 2) << 2);
            spritedata = alis.mem + image.newad + xread32(image.newad);
        }
        
        image.newl = read16(spritedata + 2);
        image.newh = read16(spritedata + 4);
        image.newzoomx = 0;
        image.newzoomy = 0;
        
        
        if (alis.platform.kind == EPlatformMac)
        {
            mac_update_pos(&tmpdepx, &tmpdepy);
        }
        
        image.newx = tmpdepx - (image.newl >> 1);
        image.newy = tmpdepy - (image.newh >> 1);
        image.newd = tmpdepz;
    }
}

void waitphysic(void)
{
    do {} while (image.fphysic != 0);
}

#if ALIS_DEBUG_PLANAR || !defined(ALIS_NATIVE_PLANAR) && !defined(ALIS_NATIVE_16BPP)   // native: image_draw_planar.c
void trsfen(u8 *src, u8 *tgt)
{
    s16 fx1 = image.fenx1;
    s16 fy1 = image.feny1;
    s16 fx2 = image.fenx2;
    s16 fy2 = image.feny2;

    // Clamp to screen bounds
    if (fx1 < 0) fx1 = 0;
    if (fy1 < 0) fy1 = 0;
    if (fx2 >= alis.platform.width)  fx2 = alis.platform.width - 1;
    if (fy2 >= alis.platform.height) fy2 = alis.platform.height - 1;

    if (fx2 < fx1 || fy2 < fy1)
        return;

    src += fx1 + fy1 * alis.platform.width;
    tgt += fx1 + fy1 * alis.platform.width;

    s16 skip = alis.platform.width - (fx2 - fx1 + 1);

    for (s32 y = fy1; y <= fy2; y++)
    {
        for (s32 x = fx1; x <= fx2; x++, src++, tgt++)
        {
            *tgt = *src;
        }

        tgt += skip;
        src += skip;
    }
}
#endif // !ALIS_NATIVE_PLANAR (trsfen)

void phytolog(void)
{
    image.fenx1 = 0;
    image.feny1 = 0;
    image.fenx2 = alis.platform.width - 1;
    image.feny2 = alis.platform.height - 1;
    trsfen(image.physic, image.logic);
}

void tvtofen(void)
{
    trsfen(image.physic, image.logic);
}

void memfen(void)
{
    image.ptabfen[0] = image.fenx1;
    image.ptabfen[1] = image.fenx2;
    image.ptabfen[2] = image.feny1;
    image.ptabfen[3] = image.feny2;
    image.ptabfen += 4;

    s16 *endtabfen = image.tabfen + (sizeof(image.tabfen) / sizeof(image.tabfen[0]));
    if (image.ptabfen < endtabfen)
    {
        return;
    }

    // error
    ALIS_DEBUG(EDebugError, "ptabfen pointing outside of available mem!\n");
}

void oldfen(void)
{
    for (s16 *tabptr = image.tabfen; tabptr < image.ptabfen; tabptr += 4)
    {
        image.fenx1 = tabptr[0];
        image.fenx2 = tabptr[1];
        image.feny1 = tabptr[2];
        image.feny2 = tabptr[3];
        tvtofen();
    }
}

void setphysic(void)
{
    if (image.insid == 0)
    {
        host.pixelbuf.data = image.physic;
    }

    image.bufpack = image.logic;
    image.fphysic = 1;
}

// when position or angle changes set screen update flag
void folscreen(s32 scene)
{
    if (-1 < xread16(alis.basemain + scene + 0x60))
    {
        s32 addr = xread32(alis.atent + xread16(alis.basemain + scene + 0x60));
        if (addr != 0)
        {
            s16 val;
            if ((xread8(alis.basemain + scene + 0x84) & 1) != 0)
            {
                val = xread16(alis.basemain + scene + 0x86) + xread16(addr + ALIS_SCR_WCX);
                if (val != xread16(alis.basemain + scene + 0x16))
                {
                    xwrite16(alis.basemain + scene + 0x16, val);
                    xwrite8(alis.basemain + scene, xread8(alis.basemain + scene) | 0x80);
                }
                
                val = xread16(alis.basemain + scene + 0x88) + xread16(addr + ALIS_SCR_WCY);
                if (val != xread16(alis.basemain + scene + 0x18))
                {
                    xwrite16(alis.basemain + scene + 0x18, val);
                    xwrite8(alis.basemain + scene, xread8(alis.basemain + scene) | 0x80);
                }
                
                val = xread16(alis.basemain + scene + 0x8a) + xread16(addr + ALIS_SCR_WCZ);
                if (val != xread16(alis.basemain + scene + 0x1a))
                {
                    xwrite16(alis.basemain + scene + 0x1a, val);
                    xwrite8(alis.basemain + scene, xread8(alis.basemain + scene) | 0x80);
                }
            }
            
            if ((xread8(alis.basemain + scene + 0x84) & 2) != 0)
            {
                val = xread16(alis.basemain + scene + 0x8c) + xread16(addr + 0x40) + xread16(addr + ALIS_SCR_WCAX);
                if (val != xread16(alis.basemain + scene + 0x34))
                {
                    xwrite16(alis.basemain + scene + 0x34, val);
                    xwrite8(alis.basemain + scene, xread8(alis.basemain + scene) | 0x80);
                }
                
                val = xread16(alis.basemain + scene + 0x8e) + xread16(addr + 0x44) + xread16(addr + ALIS_SCR_WCAY);
                if (val != xread16(alis.basemain + scene + 0x36))
                {
                    xwrite16(alis.basemain + scene + 0x36, val);
                    xwrite8(alis.basemain + scene, xread8(alis.basemain + scene) | 0x80);
                }
                
                val = xread16(alis.basemain + scene + 0x90) + xread16(addr + 0x48) + xread16(addr + ALIS_SCR_WCAZ);
                if (val != xread16(alis.basemain + scene + 0x38))
                {
                    xwrite16(alis.basemain + scene + 0x38, val);
                    xwrite8(alis.basemain + scene, xread8(alis.basemain + scene) | 0x80);
                }
            }
        }
    }
}

void addlink(u16 elemidx)
{
    sSprite *elemsprvar = SPRITE_VAR(elemidx);
    if (-1 < elemsprvar->newd)
    {
        s16 tmp = elemsprvar->newx;
        if (tmp <= image.blocx1)
            image.blocx1 = tmp;

        tmp += elemsprvar->width;
        if (image.blocx2 <= tmp)
            image.blocx2 = tmp;

        tmp = elemsprvar->newy;
        if (tmp <= image.blocy1)
            image.blocy1 = tmp;

        tmp += elemsprvar->height;
        if (image.blocy2 <= tmp)
            image.blocy2 = tmp;
    }
}

u16 inouvlink(u16 scene, u16 elemidx1, u16 elemidx2, u16 elemidx3)
{
    deptopix(scene, elemidx1);
    
    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    elem1sprvar->newad = image.newad;
    elem1sprvar->newx = image.newx;
    elem1sprvar->newy = image.newy;
    elem1sprvar->newd = image.newd;
    elem1sprvar->newf = image.newf;
    elem1sprvar->width = image.newl;
    elem1sprvar->height = image.newh;
    elem1sprvar->newzoomx = image.newzoomx;
    elem1sprvar->newzoomy = image.newzoomy;

    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

u16 iremplink(u16 scene, u16 elemidx1, u16 elemidx2, u16 elemidx3)
{
    addlink(elemidx1);
    
    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    if (image.wback != 0 && image.backprof <= elem1sprvar->newd)
    {
        image.pback = 1;
        return inouvlink(scene, elemidx1, elemidx2, elemidx3);
    }
    
    deptopix(scene, elemidx1);
    
    elem1sprvar->newad = image.newad;
    elem1sprvar->newx = image.newx;
    elem1sprvar->newy = image.newy;
    elem1sprvar->newd = image.newd;
    elem1sprvar->newf = image.newf;
    elem1sprvar->width = image.newl;
    elem1sprvar->height = image.newh;
    elem1sprvar->newzoomx = image.newzoomx;
    elem1sprvar->newzoomy = image.newzoomy;

    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

u16 iefflink(u16 elemidx1, u16 elemidx2)
{
    addlink(elemidx1);

    sSprite *elem1sprvar = SPRITE_VAR(elemidx1);
    sSprite *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;
    elem1sprvar->to_next = image.libsprit;
    
    image.libsprit = elemidx1;
    if (image.wback != 0 && image.backprof <= elem1sprvar->newd)
    {
        image.pback = 1;
    }
    
    return elemidx2;
}

#if ALIS_DEBUG_PLANAR || !defined(ALIS_NATIVE_PLANAR) && !defined(ALIS_NATIVE_16BPP)   // native: image_draw_planar.c
void clrfen(void)
{
    s16 fx1 = image.fenx1 < 0 ? 0 : image.fenx1;
    s16 fy1 = image.feny1 < 0 ? 0 : image.feny1;
    s16 fx2 = image.fenx2 >= alis.platform.width ? alis.platform.width - 1 : image.fenx2;
    s16 fy2 = image.feny2 >= alis.platform.height ? alis.platform.height - 1 : image.feny2;
    // fen is inclusive.
    s16 tmpx = (fx2 - fx1) + 1;
    if (tmpx <= 0 || fy2 < fy1)
        return;
    for (s16 y = fy1; y <= fy2; y++)
    {
        memset(image.logic + fx1 + y * alis.platform.width, 0, tmpx);
    }
}
#endif // !ALIS_NATIVE_PLANAR (clrfen)

void clipback(void)
{
    if (image.clipy1 < image.backy1)
    {
        if (image.clipy2 < image.backy1)
        {
            return;
        }
    }
    else
    {
        if (image.clipy2 <= image.backy2)
        {
            image.cback = 1;
            return;
        }
        
        if (image.backy2 < image.clipy1)
        {
            return;
        }
    }
    
    image.cback = -1;
}

void destofen(sSprite *sprite)
{
    // Blit destination; re-read every call to follow buffer swaps.
    image.wdraw = image.logic;

#if !defined(ALIS_NATIVE_16BPP)
    // Background cache: the backsprite copies the flattened background from the shadow
    // (laid out like image.logic) to the screen as a clipped rect copy over the back rect.
    if (image.sback != 0 && image.backmap != NULL
        && sprite == SPRITE_VAR(image.backsprite))
    {
        s16 bx1 = image.clipx1 > image.backx1 ? image.clipx1 : image.backx1;
        s16 by1 = image.clipy1 > image.backy1 ? image.clipy1 : image.backy1;
        s16 bx2 = image.clipx2 < image.backx2 ? image.clipx2 : image.backx2;
        s16 by2 = image.clipy2 < image.backy2 ? image.clipy2 : image.backy2;
        if (bx1 <= bx2 && by1 <= by2)
        {
            s32 w = alis.platform.width;
#if defined(ALIS_NATIVE_PLANAR)
            // Planar: a row is `w` bytes of 8-plane 16px chunks. Copy whole chunk columns
            // covering [bx1..bx2] (back rect x-bounds are 16px-aligned by cscback/calcfen;
            // rounding out only pulls extra correct-background pixels at the edges).
            s32 c0 = (bx1 >> 4) * 16;
            s32 nb = (((bx2 >> 4) - (bx1 >> 4)) + 1) * 16;
            for (s16 y = by1; y <= by2; y++)
                memcpy(image.logic + (s32)y * w + c0,
                       image.backmap + (s32)y * w + c0, nb);
#else
            // Chunky: one byte per pixel.
            s32 span = (s32)(bx2 - bx1) + 1;
            for (s16 y = by1; y <= by2; y++)
                memcpy(image.logic + (s32)y * w + bx1,
                       image.backmap + (s32)y * w + bx1, span);
#endif
        }
        return;
    }
    // While fenetre has wlogic pointed at the cache, composite behind-sprites into the shadow.
    if (image.backmap != NULL && image.wlogic == image.backmap)
        image.wdraw = image.backmap;
#endif

    u32 addr = sprite->newad;
    if (addr == 0 || sprite->data == 0)
        return;
    
    if (alis.platform.version > 10)
        addr += xread32(addr);

    u8 *bitmap = alis.mem + addr;

    // ST/Amiga engines skip negative types; Falcon and DOS don't test it
    if ((s8)bitmap[0] < 0 && alis.platform.version > 10
        && (alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga
            || alis.platform.kind == EPlatformAmigaAGA || alis.platform.kind == EPlatformMac))
        return;

    sRect pos = {
        .x1 = sprite->newx,
        .y1 = sprite->newy,
        .x2 = sprite->newx + (s16)read16(bitmap + 2),
        .y2 = sprite->newy + (s16)read16(bitmap + 4) };

    s8 flip = sprite->newf;
    s32 width = sprite->width + 1;
    s32 height = sprite->height + 1;

    if (image.clipx2 < pos.x1)
        return;

    if (image.clipy2 < pos.y1)
        return;

    if (pos.x2 < image.clipx1)
        return;

    if (pos.y2 < image.clipy1)
        return;

    image.blocx1 = pos.x1;
    if (pos.x1 < image.clipx1)
        image.blocx1 = image.clipx1;

    image.blocy1 = pos.y1;
    if (pos.y1 < image.clipy1)
        image.blocy1 = image.clipy1;

    image.blocx2 = pos.x2;
    if (image.clipx2 < pos.x2)
        image.blocx2 = image.clipx2;

    image.blocy2 = pos.y2;
    if (image.clipy2 < pos.y2)
        image.blocy2 = image.clipy2;

    u8 *at = bitmap + 6;

    sRect bmp = {
        .x1 = 0,
        .y1 = 0,
        .x2 = width,
        .y2 = height };

    if (image.blocx1 < 0)
    {
        bmp.x1 -= image.blocx1;
        bmp.x2 += image.blocx1;
    }
    
    if (image.blocx1 > pos.x1)
    {
        bmp.x1 += (image.blocx1 - pos.x1);
        bmp.x2 -= (image.blocx1 - pos.x1);
    }
    
    if (image.blocx2 <= pos.x2)
    {
        bmp.x2 -= (pos.x2 - image.blocx2);
    }

    if (image.blocy1 > pos.y1)
    {
        bmp.y1 += (image.blocy1 - pos.y1);
        bmp.y2 -= (image.blocy1 - pos.y1);
    }

    if (image.blocy2 < pos.y2)
    {
        bmp.y2 -= (pos.y2 - image.blocy2);
    }
    
#if ALIS_SDL_VER < 2
    if (dirty_len > 0xfd)
    {
        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 0xff;
    }
    else if (bmp.x2 > 0 && bmp.y2 > 0)
    {
        dirty_rects[dirty_len] = (SDL_Rect){ .x = pos.x1 + bmp.x1, .y = pos.y1 + bmp.y1, .w = bmp.x2, .h = bmp.y2 };
        dirty_len++;
    }
#endif
    
#if defined(ALIS_PROFILE_DRAW)
    {   // overdraw accounting: bmp.x2/y2 are still the clipped width/height here
        extern u32 g_od_pix, g_od_pix_op, g_od_pix_tr, g_od_blits, g_od_big;
        s32 area = (s32)bmp.x2 * (s32)bmp.y2;
        if (area > 0) {
            u8 fmt = bitmap[0];
            int opaque = (fmt == 0x02 || fmt == 0x12 || fmt == 0x16);
            g_od_pix += area; g_od_blits++;
            if (opaque) g_od_pix_op += area; else g_od_pix_tr += area;
            if (area >= 2048) g_od_big += area;
        }
    }
#endif

    bmp.x2 += bmp.x1;
    bmp.y2 += bmp.y1;

#if defined(ALIS_NATIVE_16BPP)
    // Big opaque sprites: pre-expanded 16bpp Blitter-copy cache; CPU blitters on miss.
    {
        extern int sprite_cache_try_draw(u8 fmt, u8 *at, u32 newad, sRect *pos, sRect *bmp, s32 width, s32 height, s8 flip);
        if (sprite_cache_try_draw(bitmap[0], at, sprite->newad, &pos, &bmp, width, height, flip))
            return;
    }
#endif

#if ALIS_DEBUG_PLANAR || defined(ALIS_NATIVE_PLANAR) && ALIS_NATIVE_PLANAR
    // Sprites planarized in place at load (convert_sprites_inplace); flips use a lazily
    // mirrored copy. Per-resource test: on DOS data only some scripts are planarized
    // (byte 6 = plane count, 0 = chunky). Others fall through to the chunky blitters.
    {
        extern int sprite_is_planar(const u8 *bitmap);
        if (sprite_is_planar(bitmap)) {
            extern u16 *planar_tab_get_flip(const u8 *bitmap);
            extern void destofen_planar(const u8 *bitmap, u16 *data, sRect *pos, sRect *bmp);
            if (!flip) { destofen_planar(bitmap, (u16 *)(bitmap + 8), &pos, &bmp); return; }
            u16 *_fd = planar_tab_get_flip(bitmap);
            if (_fd) { destofen_planar(bitmap, _fd, &pos, &bmp); return; }
            return;   // OOM building the flip — skip rather than draw garbage
        }
    }
#endif

    switch (bitmap[0])
    {
        case 0x01:
        {
            if (alis.platform.kind == EPlatformMac)
            {
                draw_mac_rect(&pos, &bmp, bitmap[1]);
            }
            else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
            {
                draw_rect(&pos, &bmp, bitmap[1] % 4);
            }
            else
            {
                draw_rect(&pos, &bmp, bitmap[1]);
            }
            break;
        }
            
        case 0x00:
        {
            if (alis.platform.kind == EPlatformMac)
            {
                draw_mac_mono_0(at, &pos, &bmp, width, flip);
            }
            else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
            {
                draw_dos_cga_0(at, &pos, &bmp, width, flip);
            }
            else
            {
                draw_st_4bit_0(at, &pos, &bmp, width, flip);
            }

            break;
        }
            
        case 0x02:
        {
            if (alis.platform.kind == EPlatformMac)
            {
                draw_mac_mono_0(at, &pos, &bmp, width, flip);
            }
            else if (alis.platform.kind == EPlatformPC && alis.platform.version <= 11)
            {
                draw_dos_cga_2(at, &pos, &bmp, width, flip);
            }
            else
            {
                draw_st_4bit_2(at, &pos, &bmp, width, flip);
            }

            break;
        }
            
        case 0x10:
        {
            if (alis.platform.px_format == EPxFormatAmPlanar)
            {
                draw_ami_5bit_0(at, &pos, &bmp, width, height, flip);
            }
            else
            {
                draw_4to8bit_0(at + 2, &pos, &bmp, width, flip, at[0]);
            }

            break;
        }

        case 0x12:
        {
            if (alis.platform.px_format == EPxFormatAmPlanar)
            {
                draw_ami_5bit_2(at, &pos, &bmp, width, height, flip);
            }
            else
            {
                draw_4to8bit_2(at + 2, &pos, &bmp, width, flip, at[0]);
            }
            break;
        }
            
        case 0x14:
        {
            draw_8bit_0(at, &pos, &bmp, width, flip);
            break;
        }

        case 0x16:
        {
            draw_8bit_2(at, &pos, &bmp, width, flip);
            break;
        }
            
        case 0x40:
        {
            draw_fli_video(bitmap);
            break;
        }
            
        case 0x7f:
        {
            switch (alis.platform.uid)
            {
                case EGameTransarctica:
                    draw_transarctica_map(sprite, (sprite->newad + xread32(sprite->newad)), (sRect){ .x1 = (bmp.x1 + pos.x1), .y1 = (bmp.y1 + pos.y1), .x2 = (bmp.x2 + pos.x1), .y2 = (bmp.y2 + pos.y1) });
                    break;
                    
                case EGameRobinsonsRequiem0:
                case EGameRobinsonsRequiem1:
                    draw_requiem_map(sprite, (sprite->newad + xread32(sprite->newad)));
                    break;
                    
                default:
                    ALIS_DEBUG(EDebugError, "MISSING DRAW MAP IMPLEMENTETATION");
                    break;
            }
            
            break;
        }
            
        default:
            ALIS_DEBUG(EDebugError, "UNKNOWN RESOURCE TYPE: %d", bitmap[0]);
    }
}

s32 calctop(u32 scene_addr, s16 grid_col, s16 grid_row)
{
    // Compute byte offset into 2D terrain texture map: (row + col * stride) * 2
    s32 tex_offset = ((s32)grid_row + (s32)grid_col * xread16(scene_addr - 0x3ca)) * 2;
    return tex_offset;
}

void fentotv(void)
{
    // Single buffer: nothing to copy, but the window (cleared areas included) must be presented.
    if (image.logic == image.physic)
    {
#if ALIS_SDL_VER < 2
        if (dirty_len > 0xfd)
        {
            dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
            dirty_len = 0xff;
        }
        else if (image.fenx2 >= image.fenx1 && image.feny2 >= image.feny1)
        {
            dirty_rects[dirty_len] = (SDL_Rect){ .x = image.fenx1, .y = image.feny1, .w = image.fenx2 - image.fenx1 + 1, .h = image.feny2 - image.feny1 + 1 };
            dirty_len++;
        }
#endif
        return;
    }

#if ALIS_VM_PROFILE
    u32 _c2p0 = sys_profile_ticks();
#endif
    trsfen(image.logic, image.physic);
#if ALIS_VM_PROFILE
    g_prof_c2p += sys_profile_ticks() - _c2p0;
#endif
}

void fenetre(u16 scene, u16 elemidx1, u16 elemidx3, u16 prevspidx)
{
    u16 tmpidx;

    sSprite *sprite;

    if ((get_scr_state(scene) & 0x40) != 0)
    {
        return;
    }

    image.scdirect = 0;
    if (calcfen(elemidx1, elemidx3))
    {
        if ((get_scr_numelem(scene) & 2) != 0)
        {
            image.fdoland = 1;
        }
        
        for (s16 scridx = screen.ptscreen; scridx != 0; scridx = get_scr_to_next(scridx))
        {
            if ((get_scr_state(scridx) & 0x40) == 0)
            {
                tmpidx = get_scr_screen_id(scridx);
                if (tmpidx != 0)
                {
                    clipfen(SPRITE_VAR(tmpidx));
                    
                    if (image.fclip != 0)
                    {
                        if ((get_scr_numelem(scridx) & 2) == 0)
                        {
                            image.cback = 0;
                            if ((get_scr_numelem(scridx) & 4) != 0)
                            {
                                clipback();
                            }
                            
                            if ((get_scr_numelem(scridx) & 0x40) == 0)
                            {
                                clrfen();
                            }
                            
                            if (image.cback)
                            {
                                if (image.cback < 0)
                                {
                                    if ((image.wback != 0) && (image.pback != 0))
                                    {
                                        // Partial overlap: flatten the behind-sprites into the cache
                                        // (clipped to the back rect's Y extent), then restore state.
                                        s16 saveclipy1 = image.clipy1;
                                        s16 saveclipy2 = image.clipy2;

                                        image.wlogic = image.backmap;
                                        image.wlogx1 = image.backx1;
                                        image.wlogx2 = image.backx2;
                                        image.wlogy1 = image.backy1;
                                        image.wlogy2 = image.backy2;
                                        image.wloglarg = image.backlarg;

                                        if (image.clipy1 <= image.backy1)
                                            image.clipy1 = image.backy1;

                                        if (image.backy2 <= image.clipy2)
                                            image.clipy2 = image.backy2;

                                        sprite = SPRITE_VAR(tmpidx);
                                        while (true)
                                        {
                                            tmpidx = sprite->link;
                                            if ((tmpidx == 0) || (tmpidx == image.backsprite))
                                                break;

                                            sprite = SPRITE_VAR(tmpidx);
                                            if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
                                            {
                                                destofen(sprite);
                                            }
                                        }

                                        image.wlogic = image.logic;
                                        image.wlogx1 = image.logx1;
                                        image.wlogx2 = image.logx2;
                                        image.wlogy1 = image.logy1;
                                        image.wlogy2 = image.logy2;
                                        image.wloglarg = image.loglarg;

                                        image.clipy1 = saveclipy1;
                                        image.clipy2 = saveclipy2;
                                        tmpidx = get_scr_screen_id(scridx);
                                    }
                                }
                                else
                                {
                                    if (image.pback == 0)
                                    {
                                        // Cache hit: draw only the backsprite then the front sprites.
#if !defined(ALIS_NATIVE_16BPP)
                                        // 16bpp has no cache: tmpidx stays the screen head.
                                        tmpidx = image.backsprite;
#endif
                                        goto fenetre31;
                                    }

                                    image.wlogic = image.backmap;
                                    image.wlogx1 = image.backx1;
                                    image.wlogx2 = image.backx2;
                                    image.wlogy1 = image.backy1;
                                    image.wlogy2 = image.backy2;
                                    image.wloglarg = image.backlarg;
                                }
                            }
                            
                            sprite = SPRITE_VAR(tmpidx);
                            while ((tmpidx = sprite->link))
                            {
                                if (image.sback != 0 && tmpidx == image.backsprite)
                                {
                                    image.wlogic = image.logic;
                                    image.wlogx1 = image.logx1;
                                    image.wlogx2 = image.logx2;
                                    image.wlogy1 = image.logy1;
                                    image.wlogy2 = image.logy2;
                                    image.wloglarg = image.loglarg;
                                }
                                
                            fenetre31:
                                
                                sprite = SPRITE_VAR(tmpidx);
                                if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
                                {
                                    destofen(sprite);
                                    image.switchgo = 1;
                                }
                            }
                        }
                        else
                        {
                            landtofi(prevspidx, scridx);
                        }
                    }
                }
            }
        }
        
        // NOTE: originaly mouse cursore erase, now handled elsewhere
        
        if ((get_scr_numelem(scene) & 0x20) == 0)
        {
            if (alis.fswitch != 0)
            {
                if (image.wpag == 0)
                {
                    memfen();
                }
                
                return;
            }
            
            if (image.wpag == 0)
            {
                fentotv();
            }
        }
        
        // NOTE: originaly mouse cursore draw, now handled elsewhere
    }
}

void scrolpage(void)
{
}

u16 suitlin1(u16 a2, u16 d1w, u16 d2w, u16 d3w, u16 d4w)
{
    while ((void)(d2w = d1w), (d1w = SPRITE_VAR(d2w)->link) != 0)
    {
        if (d4w == SPRITE_VAR(d1w)->clinking)
        {
            s8 state = SPRITE_VAR(d1w)->state;
            if (state != 0)
            {
                if (state < 0)
                {
                    d1w = inouvlink(a2, d1w, d2w, d3w);
                }
                else if (state == 2)
                {
                    d1w = iremplink(a2, d1w, d2w, d3w);
                }
                else
                {
                    d1w = iefflink(d1w, d2w);
                }
            }
        }
    }

    image.joints = 1;
    return d1w;
}

void affiscr(u16 scene, u16 screenidx)
{
    u16 spriteidx = screenidx;
    u16 prevspidx;
    // u16: clinking exceeds 0x7fff in Ishar 3.
    u16 linkidx;
    
    if (alis.platform.version >= 30 && xread8(alis.basemain + scene + 0x84) != 0)
    {
        folscreen(scene);
    }

#if defined(ALIS_FORCE_LAND_REDRAW)
    // Profiling: redraw 3D scenes every frame as if the camera moved.
    if (alis.platform.version != 0 && (get_scr_numelem(scene) & 2) != 0)
        set_scr_state(scene, get_scr_state(scene) | 0x80);
#endif

    if ((image.fremap != 0) || ((s8)get_scr_state(scene) < 0))
    {
#if defined(ALIS_PROFILE_DRAW)
        extern u32 sys_profile_ticks_safe(void); extern u32 g_prof_frame_depscreen;
        u32 _dp0 = sys_profile_ticks_safe();
        depscreen(scene, screenidx);
        g_prof_frame_depscreen += sys_profile_ticks_safe() - _dp0;
#else
        depscreen(scene, screenidx);
#endif
    }

    s8 scrflags = alis.platform.version == 10 ? 0 : get_scr_numelem(scene);
    u8 draw = false;
    if (alis.platform.version == 0)
    {
        draw = alis.fswitch == 0;
    }
    else if ((scrflags & 2) == 0)
    {
        draw = true;
    }
    else
    {
        affiland(scene);
        draw = (get_scr_state(scene) & 0x80) == 0;
    }
    
    if (draw)
    {
        image.wback = (scrflags & 4) != 0;
        image.wpag = 0;

        if ((get_scr_state(scene) & 0x20) != 0)
        {
            image.wpag = 1;
            image.spag --;
            if (image.spag == 0)
            {
                image.wpag = -1;
            }
        }
        
        if ((scrflags & 0x10) == 0)
        {
            while ((void)(prevspidx = spriteidx), (spriteidx = SPRITE_VAR(spriteidx)->link) != 0)
            {
                s8 state = SPRITE_VAR(spriteidx)->state;
                if (state != 0)
                {
                    image.joints = 0;
                    image.pback = 0;
                    if (state == 2)
                    {
                        linkidx = inilink(spriteidx);
                        if (linkidx >= 0)
                        {
                            spriteidx = iremplink(scene, spriteidx, prevspidx, screenidx);
                            suitlin1(scene, spriteidx, prevspidx, screenidx, linkidx);
                        }
                        else
                        {
                            sSprite *sprite = SPRITE_VAR(spriteidx);

                            // Only effect of this compare is pback (Ghidra's "isback" is CC residue).
                            if (image.wback != 0 && image.backprof <= sprite->newd)
                            {
                                image.pback = 1;
                            }

                            deptopix(scene, spriteidx);
                            tstjoints(spriteidx);
                            
                            if (image.joints == 0)
                            {
                                SPRITE_VAR(prevspidx)->link = sprite->link;
                                fenetre(scene, spriteidx, screenidx, prevspidx);
                            }
                            else
                            {
                                SPRITE_VAR(prevspidx)->link = sprite->link;
                            }
                            
                            sprite->newad = image.newad;
                            sprite->newx = image.newx;
                            sprite->newy = image.newy;
                            sprite->newd = image.newd;
                            sprite->newf = image.newf;
                            sprite->width = image.newl;
                            sprite->height = image.newh;
                            sprite->newzoomx = image.newzoomx;
                            sprite->newzoomy = image.newzoomy;
                            
                            prevspidx = rangesprite(spriteidx, prevspidx, screenidx);
                        }
                    }
                    else
                    {
                        linkidx = inilink(spriteidx);
                        
                        if (state == -1)
                        {
                            spriteidx = inouvlink(scene, spriteidx, prevspidx, screenidx);
                        }
                        else
                        {
                            spriteidx = iefflink(spriteidx, prevspidx);
                        }
                        
                        if (linkidx < 0)
                        {
                            image.joints = 1;
                        }
                        else
                        {
                            suitlin1(scene, spriteidx, prevspidx, screenidx, linkidx);
                        }
                    }
                    
                    fenetre(scene, spriteidx, screenidx, prevspidx);
                    spriteidx = prevspidx;
                }
            }
        }
        else
        {
            image.fenx1 = get_scr_newx(scene);
            image.feny1 = get_scr_newy(scene);
            image.fenx2 = image.fenx1 + get_scr_width(scene);
            image.feny2 = image.feny1 + get_scr_height(scene);
            image.clipl = (image.fenx2 - image.fenx1) + 1;
            image.cliph = (image.feny2 - image.feny1) + 1;
            
            image.clipx1 = image.fenx1;
            image.clipy1 = image.feny1;
            image.clipx2 = image.fenx2;
            image.clipy2 = image.feny2;
            
            if ((get_scr_numelem(scene) & 0x40) == 0)
            {
                clrfen();
            }
            
            while ((void)(prevspidx = spriteidx), (spriteidx = SPRITE_VAR(spriteidx)->link) != 0)
            {
                sSprite *psprite = SPRITE_VAR(prevspidx);
                sSprite *csprite = SPRITE_VAR(spriteidx);
                if (csprite->state != 0)
                {
                    if (csprite->state == 1)
                    {
                        psprite->link = csprite->link;
                        csprite->to_next = image.libsprit;
                        image.libsprit = spriteidx;
                        spriteidx = prevspidx;
                    }
                    else
                    {
                        deptopix(scene, spriteidx);
                        psprite->link = csprite->link;
                        csprite->newad = image.newad;
                        csprite->newx = image.newx;
                        csprite->newy = image.newy;
                        csprite->newd = image.newd;
                        csprite->newf = image.newf;
                        csprite->width = image.newl;
                        csprite->height = image.newh;
                        csprite->newzoomx = image.newzoomx;
                        csprite->newzoomy = image.newzoomy;
                        spriteidx = rangesprite(spriteidx, prevspidx, screenidx);
                    }
                }
            }
            
            for (spriteidx = screen.ptscreen; spriteidx; spriteidx = get_scr_to_next(spriteidx))
            {
                if (((get_scr_state(spriteidx) & 0x40) == 0) && ((prevspidx = get_scr_screen_id(spriteidx)) != 0))
                {
                    clipfen(SPRITE_VAR(prevspidx));
                    
                    if (image.fclip != 0)
                    {
                        while ((prevspidx = SPRITE_VAR(prevspidx)->link) != 0)
                        {
                            if (-1 < SPRITE_VAR(prevspidx)->state && -1 < SPRITE_VAR(prevspidx)->newf && -1 < SPRITE_VAR(prevspidx)->newd)
                            {
                                destofen(SPRITE_VAR(prevspidx));
                                image.switchgo = 1;
                            }
                        }
                    }
                }
            }

            if ((alis.fswitch == 0) && (image.wpag == 0))
            {
                fentotv();
            }
        }
    }
    else
    {
        for (spriteidx = get_scr_to_next(scene); spriteidx; spriteidx = get_scr_to_next(spriteidx))
        {
            if (((get_scr_state(spriteidx) & 0x40) == 0) && ((prevspidx = get_scr_screen_id(spriteidx)) != 0))
            {
                clipfen(SPRITE_VAR(prevspidx));
                
                if (image.fclip != 0)
                {
                    while ((prevspidx = SPRITE_VAR(prevspidx)->link) != 0)
                    {
                        if (-1 < SPRITE_VAR(prevspidx)->state && -1 < SPRITE_VAR(prevspidx)->newf && -1 < SPRITE_VAR(prevspidx)->newd)
                        {
                            destofen(SPRITE_VAR(prevspidx));
                            image.switchgo = 1;
                        }
                    }
                }
            }
        }

        if ((alis.fswitch == 0) && (image.wpag == 0))
        {
            fentotv();
        }
    }
    
    if (image.wpag < 0)
    {
        set_scr_state(scene, get_scr_state(scene) & 0xdf);
        image.fenx1 = get_scr_newx(scene);
        image.feny1 = get_scr_newy(scene);
        image.fenx2 = get_scr_newx(scene) + get_scr_width(scene);
        image.feny2 = get_scr_newy(scene) + get_scr_height(scene);
        scrolpage();
    }
    
    // TODO: fix this hack
    // following 'if ()' is needed to draw rails under train on minimap in Transarctica
    if (alis.platform.uid != EGameTransarctica)
    {
        set_scr_state(scene, get_scr_state(scene) & 0x7f);
    }
}

u32 itroutine(u32 interval, void *param)
{
    u8 prevtiming = image.vtiming;
    alis.timeclock ++;
    image.fitroutine = 1;
    image.vtiming ++;
    if (image.vtiming == 0)
    {
        image.vtiming = prevtiming;
    }

    if (dos_pal_active())
    {
        dos_pal_tick();
    }
    else if (image.palc != 0 && (--image.palt) == 0)
    {
        image.palt = image.palt0;
        topalet();
        image.palc --;

#if ALIS_SDL_VER < 2
        dirty_pal = 1;
#endif
    }

    image.fitroutine = 0;
    return alis.platform.is_little_endian ? 17 : 20;
}

void draw(void)
{
    sys_delay_frame();

    // Render time excludes the frame-cap sleep.
#if ALIS_VM_PROFILE
    extern u32 g_prof_draw;
    extern void prof_frame_end(void);
    u32 _pd0 = sys_profile_ticks();
#endif

    sys_lock_renderer();

#if defined(ALIS_NATIVE_PLANAR) && ALIS_NATIVE_PLANAR
    // During a film the native FLIC/FLS path owns both screen buffers.
    if (bfilm.type != eAlisVideoNone) {
        sys_unlock_renderer();
        return;
    }
#endif

#if defined(ALIS_PROFILE_DRAW)
    extern void dbglog(const char *fmt, ...);
    extern u32 sys_profile_ticks_safe(void);   // supervisor-safe $4BA read
    u32 _pf_draw0 = sys_profile_ticks_safe();
    u32 _fr_oldfen = 0, _fr_affiscr = 0;
#endif

#if defined(ALIS_NATIVE_16BPP)
    // Per-row line-palette offsets (clinepal) for the 4-bit blitters.
    extern void build_line_paloff(void);
    build_line_paloff();

    // 16-bit bakes RGB at draw time: on a palette change rebuild pal16 and set fremap
    // so depscreen() re-flags sprites for redraw.
    int _was_dirty_pal = dirty_pal;
    if (dirty_pal) {
        // Clear before consuming: the Timer-C ISR may set it again mid-refresh.
        dirty_pal = 0;
        extern void sys_pal16_refresh(void);
        sys_pal16_refresh();
        image.fremap = 1;
    }
    // Sprite cache is bypassed while the palette is moving (fades).
    {
        extern u32 g_pal16_gen; extern u8 g_pal16_fading;
        static u32 _spc_last_palgen = 0;
        g_pal16_fading = (g_pal16_gen != _spc_last_palgen);
        _spc_last_palgen = g_pal16_gen;
    }
    {
        extern void dbglog(const char *fmt, ...);
        static int _pal_dbg = 0;
        if (_pal_dbg < 120) {
            dbglog("draw #%d: dirty_pal=%d dirty_len=%d fswitch=%d fremap=%d clip=[%d,%d..%d,%d]\n",
                   _pal_dbg, _was_dirty_pal, (int)dirty_len, (int)alis.fswitch, (int)image.fremap,
                   (int)image.clipx1, (int)image.clipy1, (int)image.clipx2, (int)image.clipy2);
            _pal_dbg++;
        }
    }
#endif

    // Last flip was non-blocking: wait for the VBL latch before writing image.logic.
    sys_flip_wait();

    if ((alis.fswitch != 0) && (image.fphytolog != 0))
    {
        image.fphytolog = 0;
        phytolog();
    }
    
    image.vtiming = 0;
    
    if (alis.fswitch != 0)
    {
        // Remove image.logic's own cursor (from when it was front) before oldfen.
        sys_mouse_erase();

#if defined(ALIS_PROFILE_DRAW)
        u32 _pf_o0 = sys_profile_ticks_safe();
        oldfen();
        _fr_oldfen = sys_profile_ticks_safe() - _pf_o0;
        g_prof_frame_oldfen += _fr_oldfen;
#else
        oldfen();
#endif

        // oldfen copied the front cursor into image.logic; remove it.
        sys_mouse_uncopy();

        image.ptabfen = image.tabfen;
    }
    
    image.switchgo = 0;
    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogx2 = image.logx2;
    image.wlogy1 = image.logy1;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;
    
    s16 scnidx = screen.ptscreen;
    u8 *oldphys = image.physic;

#if defined(ALIS_PROFILE_DRAW)
    u32 _pf_a0b = sys_profile_ticks_safe();
#endif
    while (scnidx != 0)
    {
        if ((get_scr_state(scnidx) & 0x40) == 0)
        {
            affiscr(scnidx, get_scr_screen_id(scnidx));
        }

        scnidx = get_scr_to_next(scnidx);
    }
#if defined(ALIS_PROFILE_DRAW)
    _fr_affiscr = sys_profile_ticks_safe() - _pf_a0b;
    g_prof_frame_affiscr += _fr_affiscr;
    if (image.fremap) g_prof_frame_remap++;
#endif

#if ALIS_DEBUG_PLANAR || defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
    // Invalidate the 030 data cache if the Blitter wrote this frame.
    sys_blit_frame_sync();
#endif

    image.fremap = 0;
    if (alis.fswitch != 0)
    {
        // Draw the cursor before the swap so the presented frame contains it.
        sys_mouse_draw();

        image.physic = image.logic;
        image.logic = oldphys;
        
        setphysic();
    }

#if defined(ALIS_PROFILE_DRAW)
    u32 _fr_draw = sys_profile_ticks_safe() - _pf_draw0;
    g_prof_frame_drawtot += _fr_draw;
    // Per-frame overdraw deltas + worst-frame capture.
    static u32 _pp_pix, _pp_op, _pp_tr, _pp_blits;
    u32 _fp = g_od_pix - _pp_pix, _fop = g_od_pix_op - _pp_op, _ftr = g_od_pix_tr - _pp_tr, _fb = g_od_blits - _pp_blits;
    _pp_pix = g_od_pix; _pp_op = g_od_pix_op; _pp_tr = g_od_pix_tr; _pp_blits = g_od_blits;
    static u32 _mx_draw, _mx_old, _mx_aff, _mx_pix, _mx_op, _mx_tr, _mx_blits;
    if (_fr_draw > _mx_draw) {
        _mx_draw = _fr_draw; _mx_old = _fr_oldfen; _mx_aff = _fr_affiscr;
        _mx_pix = _fp; _mx_op = _fop; _mx_tr = _ftr; _mx_blits = _fb;
    }
    if (++g_prof_frame_frames >= PROF_DUMP_FRAMES) {
        extern void dbglog(const char *fmt, ...);
        dbglog("PROF %u frames (%u remap): draw=%u oldfen=%u affiscr=%u vsync=%u ticks "
               "[per-frame: draw=%u.%02u oldfen=%u.%02u affiscr=%u.%02u vsync=%u.%02u]\n",
               g_prof_frame_frames, g_prof_frame_remap,
               g_prof_frame_drawtot, g_prof_frame_oldfen, g_prof_frame_affiscr, g_prof_frame_vsync,
               g_prof_frame_drawtot / g_prof_frame_frames, (g_prof_frame_drawtot * 100 / g_prof_frame_frames) % 100,
               g_prof_frame_oldfen / g_prof_frame_frames, (g_prof_frame_oldfen * 100 / g_prof_frame_frames) % 100,
               g_prof_frame_affiscr / g_prof_frame_frames, (g_prof_frame_affiscr * 100 / g_prof_frame_frames) % 100,
               g_prof_frame_vsync / g_prof_frame_frames, (g_prof_frame_vsync * 100 / g_prof_frame_frames) % 100);
        u32 nf = PROF_DUMP_FRAMES;
        dbglog("  OVERDRAW/frame: pixels=%u (op=%u tr=%u) blits=%u big_pix=%u  "
               "[viewport~36000 -> overdraw x%u.%02u; opaque=%u%% big=%u%%]\n",
               g_od_pix / nf, g_od_pix_op / nf, g_od_pix_tr / nf, g_od_blits / nf, g_od_big / nf,
               (g_od_pix / nf) / 36000, (((g_od_pix / nf) * 100) / 36000) % 100,
               g_od_pix ? (g_od_pix_op * 100 / g_od_pix) : 0,
               g_od_pix ? (g_od_big * 100 / g_od_pix) : 0);
        dbglog("  WORST frame: draw=%u oldfen=%u affiscr=%u ticks (%ums); pixels=%u (op=%u tr=%u) blits=%u\n",
               _mx_draw, _mx_old, _mx_aff, _mx_draw * 5, _mx_pix, _mx_op, _mx_tr, _mx_blits);
        dbglog("  SPLIT: depscreen=%u ticks (%u%% of affiscr) -> blit-loop=%u ticks  [affiscr=%u]\n",
               g_prof_frame_depscreen, g_prof_frame_affiscr ? (g_prof_frame_depscreen * 100 / g_prof_frame_affiscr) : 0,
               g_prof_frame_affiscr > g_prof_frame_depscreen ? g_prof_frame_affiscr - g_prof_frame_depscreen : 0, g_prof_frame_affiscr);
#if defined(ALIS_NATIVE_16BPP)
        {
            extern u32 g_spc_hit, g_spc_exp, g_spc_skip; extern u16 g_spc_skipfmt;
            dbglog("  SPRITE-CACHE: hit=%u expand=%u skip=%u (last skipped fmt=0x%02x)\n",
                   g_spc_hit, g_spc_exp, g_spc_skip, (unsigned)g_spc_skipfmt);
            g_spc_hit = g_spc_exp = g_spc_skip = 0;
        }
#endif
        g_prof_frame_oldfen = g_prof_frame_affiscr = g_prof_frame_drawtot = g_prof_frame_vsync = 0;
        g_prof_frame_frames = g_prof_frame_remap = 0;
        g_od_pix = g_od_pix_op = g_od_pix_tr = g_od_blits = g_od_big = 0;
        _pp_pix = _pp_op = _pp_tr = _pp_blits = 0;
        g_prof_frame_depscreen = 0;
        _mx_draw = _mx_old = _mx_aff = _mx_pix = _mx_op = _mx_tr = _mx_blits = 0;
    }
#endif

    sys_unlock_renderer();

#if ALIS_VM_PROFILE
    g_prof_draw += sys_profile_ticks() - _pd0;
    prof_frame_end();
#endif
}

// TODO: move to script.c
s16 debprotf(s16 target_id)
{
    s16 current_id;
    
    int start = 0;
    int mid;
    int end = alis.nbprog - 1;
    
    do
    {
        mid = (end + start) >> 1;
        current_id = read16((alis.mem + alis.atprog_ptr[mid]));
        if (current_id == target_id)
        {
            return mid;
        }

        if (current_id < target_id)
        {
            start = mid + 1;
        }
        else
        {
            end = mid - 1;
        }
    }
    while (start <= end);
    
    return -1;
}


#pragma mark -
#pragma mark Draw functions

// Native (ALIS_NATIVE_PLANAR) provides planar-optimized versions of all the
// draw_* / trsfen / clrfen routines in image_draw_planar.c; exclude the generic
// versions here to avoid duplicate-symbol link errors. SDL1 builds use these.
#if ALIS_DEBUG_PLANAR || !defined(ALIS_NATIVE_PLANAR) && !defined(ALIS_NATIVE_16BPP)

void draw_pixel(s16 x0, s16 y0)
{
    image.logic[x0 + y0 * alis.platform.width] = image.inkcolor;
}

void draw_line(s16 x0, s16 y0, s16 x1, s16 y1)
{
    s32 dx = abs(x1 - x0);
    s32 sx = x0 < x1 ? 1 : -1;
    s32 dy = -abs(y1 - y0);
    s32 sy = y0 < y1 ? 1 : -1;
    s32 error = dx + dy;
    
    while (true)
    {
        draw_pixel(x0, y0);
        if (x0 == x1 && y0 == y1)
            break;
            
        s32 e2 = 2 * error;
        if (e2 >= dy)
        {
            if (x0 == x1)
                break;
                
            error += dy;
            x0 += sx;
        }
        
        if (e2 <= dx)
        {
            if (y0 == y1)
                break;
            
            error += dx;
            y0 += sy;
        }
    }
}

#endif // !ALIS_NATIVE_PLANAR (draw_pixel/draw_line — planar versions exist)

// draw_box stays compiled for all targets — it only calls draw_line
void draw_box(s16 x1,s16 y1,s16 x2,s16 y2)
{
    if (alis.platform.kind == EPlatformMac)
    {
        mac_update_pos(&x1, &y1);
        mac_update_pos(&x2, &y2);
    }

#if ALIS_SDL_VER < 2
    if (dirty_len > 0xfd)
    {
        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 0xff;
    }
    else
    {
        dirty_rects[dirty_len] = (SDL_Rect){ .x = x1, .y = y1, .w = x2 - x1 + 1, .h = y2 - y1 + 1 };
        dirty_len++;
    }
#endif

    draw_line(x1, y1, x2, y1);
    draw_line(x2, y1, x2, y2);
    draw_line(x2, y2, x1, y2);
    draw_line(x1, y2, x1, y1);
}

#if ALIS_DEBUG_PLANAR || !defined(ALIS_NATIVE_PLANAR) && !defined(ALIS_NATIVE_16BPP)   // native: image_draw_planar.c
void draw_boxf(s16 x1,s16 y1,s16 x2,s16 y2)
{
    if (alis.platform.kind == EPlatformMac)
    {
        mac_update_pos(&x1, &y1);
        mac_update_pos(&x2, &y2);
    }
    
    s16 tmpx = min(x1, x2);
    x2 = max(x1, x2);
    x1 = tmpx;
    if (!boxf_clip(&x1, &y1, &x2, &y2))
        return;
    tmpx = x2 - x1;

#if ALIS_SDL_VER < 2
    if (dirty_len > 0xfd)
    {
        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 0xff;
    }
    else
    {
        dirty_rects[dirty_len] = (SDL_Rect){ .x = x1, .y = y1, .w = x2 - x1 + 1, .h = y2 - y1 + 1 };
        dirty_len++;
    }
#endif

    for (s16 y = y1; y <= y2; y++)
    {
        memset(image.logic + x1 + y * alis.platform.width, image.inkcolor, tmpx);
    }
}

void draw_mac_rect(sRect *pos, sRect *bmp, u8 color)
{
    u8 *tgt = image.wdraw + pos->x1 + ((bmp->y1 + pos->y1) * host.pixelbuf.w);
    if (color == 15) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) tgt[w] = (w + h) % 2;
        }
    }
    else {
        color = !color;
        for (s32 h = bmp->y1; h < bmp->y2; h++, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) tgt[w] = color;
        }
    }
}

void draw_rect(sRect *pos, sRect *bmp, u8 color)
{
    u8 *tgt = image.wdraw + pos->x1 + ((bmp->y1 + pos->y1) * host.pixelbuf.w);
    for (s32 h = bmp->y1; h < bmp->y2; h++, tgt+=host.pixelbuf.w) {
        for (s32 w = bmp->x1; w < bmp->x2; w++) tgt[w] = color;
    }
}

void draw_mac_mono_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 index, color;
    u16 swadd = width >> 2;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + pos->x1 + ((bmp->y1 + pos->y1) * host.pixelbuf.w);
    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = 3 - w % 4;
                color = (src[(width - (w + 1)) >> 2] & masks[index]) >> rots[index];
                if (!(color & 2))
                    tgt[w] = !(color & 1);
            }
        }
    }
    else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = w % 4;
                color = (src[w >> 2] & masks[index]) >> rots[index];
                if (!(color & 2))
                    tgt[w] = !(color & 1);
            }
        }
    }
}

void draw_mac_mono_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 index;
    u16 swadd = width >> 2;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + pos->x1 + ((bmp->y1 + pos->y1) * host.pixelbuf.w);
    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = 3 - w % 4;
                tgt[w] = !(((src[(width - (w + 1)) >> 2] & masks[index]) >> rots[index]) & 1);
            }
        }
    }
    else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = w % 4;
                tgt[w] = !(((src[w >> 2] & masks[index]) >> rots[index]) & 1);
            }
        }
    }
}

void draw_dos_cga_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + pos->x1 + ((bmp->y1 + pos->y1) * host.pixelbuf.w);
    u8 index;
    s16 wh;

    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                wh = ((width - (w + 1)) >> 2) << 1;
                index = 3 - w % 4;
                if (!((src[wh] & masks[index]) >> rots[index]))
                    tgt[w] = (src[wh + 1] & masks[index]) >> rots[index];
            }
        }
    }
    else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                wh = (w >> 2) << 1;
                index = w % 4;
                if (!((src[wh] & masks[index]) >> rots[index]))
                    tgt[w] = (src[wh + 1] & masks[index]) >> rots[index];
            }
        }
    }
}

void draw_dos_cga_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u16 swadd = width >> 2;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + pos->x1 + ((pos->y1 + bmp->y1) * host.pixelbuf.w);
    u8 index;

    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = 3 - w % 4;
                tgt[w] = (src[(width - (w + 1)) >> 2] & masks[index]) >> rots[index];
            }
        }
    }
    else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; w++) {
                index = w % 4;
                tgt[w] = (src[w >> 2] & masks[index]) >> rots[index];
            }
        }
    }
}

void draw_st_4bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 color;
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (bmp->x1 >> 1) + src;
            u8 *tptr = bmp->x1 + pos->x1 + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr >> 4)) *tptr = color;
            }

            bmp->x1++;
        }

        if ((bmp->x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - ((bmp->x2) >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1 - 1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr & 0b00001111)) *tptr = color;
            }

            bmp->x2--;
        }

        if (bmp->x1 != bmp->x2) {
            u8 *sptr = swadd - 1 + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    if ((color = sptr[-w] & 0b00001111)) tptr[0] = color;
                    if ((color = sptr[-w] >> 4)) tptr[1] = color;
                }
            }
        }
    }
    else {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = (bmp->x1 >> 1) + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr & 0b00001111)) *tptr = color;
            }

            bmp->x1++;
        }

        if ((bmp->x2 - 1) % 2 == 0) {
            bmp->x2--;
            u8 *sptr = (bmp->x2 >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr >> 4)) *tptr = color;
            }
        }

        if (bmp->x1 != bmp->x2) {
            u8 *sptr = src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    if ((color = sptr[w] >> 4)) tptr[0] = color;
                    if ((color = sptr[w] & 0b00001111)) tptr[1] = color;
                }
            }
        }
    }
}

void draw_st_4bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (bmp->x1 >> 1) + src;
            u8 *tptr = bmp->x1 + pos->x1 + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = *sptr >> 4;
            }

            bmp->x1++;
        }

        if ((bmp->x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - ((bmp->x2) >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1 - 1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = *sptr & 0b00001111;
            }

            bmp->x2--;
        }

        if (bmp->x1 != bmp->x2) {
            u8 *sptr = swadd - 1 + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    tptr[0] = sptr[-w] & 0b00001111;
                    tptr[1] = sptr[-w] >> 4;
                }
            }
        }
    }
    else {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = (bmp->x1 >> 1) + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = *sptr & 0b00001111;
            }

            bmp->x1++;
        }

        if ((bmp->x2 - 1) % 2 == 0) {
            bmp->x2--;
            u8 *sptr = (bmp->x2 >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = *sptr >> 4;
            }
        }

        if (bmp->x1 != bmp->x2) {
            u8 *sptr = src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    tptr[0] = sptr[w] >> 4;
                    tptr[1] = sptr[w] & 0b00001111;
                }
            }
        }
    }
}

void draw_ami_5bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip)
{
    u32 planesize = (width * height) >> 3;
    s16 wh;
    s32 idx;
    
    u8 color, c0, c1, c2, c3, c4;

    u16 swadd = width;
    u16 src = bmp->y1 * swadd;
    u8 *tgt = image.wdraw + pos->x1 + ((pos->y1 + bmp->y1) * host.pixelbuf.w);

    if (flip) {
        s16 x1 = (bmp->x1 >> 3) << 3;
        if (x1 != bmp->x1)
        {
            s16 fr = bmp->x1 - x1;
            s16 to = (x1 + 8 > bmp->x2) ? bmp->x2 - x1 : 8;

            u8 *tptr = tgt;
            s32 w = bmp->x1;
            wh = width - (w + 1);
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = bmp->x1) {
                idx = (wh + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b < to; b++, w++) {
                    if ((color = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4))
                        tptr[w] = color;
                }
            }
            
            bmp->x1 = x1 + 8;
        }

        x1 = (bmp->x2 >> 3) << 3;
        if (x1 != bmp->x2 && bmp->x1 < bmp->x2)
        {
            s16 fr = 0;
            s16 to = bmp->x2 - x1;

            src = bmp->y1 * swadd;
            u8 *tptr = tgt;
            s32 w = x1;
            wh = width - (w + 1);
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = x1) {
                idx = (wh + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b < to; b++, w++) {
                    if ((color = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4))
                        tptr[w] = color;
                }
            }
            
            bmp->x2 = x1;
        }

        src = bmp->y1 * swadd;
        for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; ) {
                wh = width - (w + 1);
                idx = (wh + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = 0; b < 8; b++, w++) {
                    if ((color = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4))
                        tgt[w] = color;
                }
            }
        }
    }
    else {
        s16 x1 = (bmp->x1 >> 3) << 3;
        if (x1 != bmp->x1)
        {
            s16 fr = 7 - (bmp->x1 - x1);
            s16 to = (x1 + 8 > bmp->x2) ? -1 + (x1 + 8 - bmp->x2) : -1;

            u8 *tptr = tgt;
            s32 w = bmp->x1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = bmp->x1) {
                idx = (x1 + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b > to; b--, w++) {
                    if ((color = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4))
                        tptr[w] = color;
                }
            }
            
            bmp->x1 = x1 + 8;
        }

        x1 = (bmp->x2 >> 3) << 3;
        if (x1 != bmp->x2 && bmp->x1 < bmp->x2)
        {
            s16 fr = 7;
            s16 to = -1 + (x1 + 8 - bmp->x2);

            src = bmp->y1 * swadd;
            u8 *tptr = tgt;
            s32 w = x1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = x1) {
                idx = (x1 + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b > to; b--, w++) {
                    if ((color = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4))
                        tptr[w] = color;
                }
            }
            
            bmp->x2 = x1;
        }

        src = bmp->y1 * swadd;
        for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; ) {
                idx = (w + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = 7; b > -1; b--, w++) {
                    if ((color = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4))
                        tgt[w] = color;
                }
            }
        }
    }
}

void draw_ami_5bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s16 height, s8 flip)
{
    u32 planesize = (width * height) >> 3;
    s16 wh;
    s32 idx;
    
    u8 c0, c1, c2, c3, c4;

    u16 swadd = width;
    u16 src = bmp->y1 * swadd;
    u8 *tgt = image.wdraw + pos->x1 + ((pos->y1 + bmp->y1) * host.pixelbuf.w);

    if (flip) {
        s16 x1 = (bmp->x1 >> 3) << 3;
        if (x1 != bmp->x1)
        {
            s16 fr = bmp->x1 - x1;
            s16 to = (x1 + 8 > bmp->x2) ? bmp->x2 - x1 : 8;

            u8 *tptr = tgt;
            s32 w = bmp->x1;
            wh = width - (w + 1);
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = bmp->x1) {
                idx = (wh + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b < to; b++, w++) {
                    tptr[w] = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4;
                }
            }
            
            bmp->x1 = x1 + 8;
        }

        x1 = (bmp->x2 >> 3) << 3;
        if (x1 != bmp->x2 && bmp->x1 < bmp->x2)
        {
            s16 fr = 0;
            s16 to = bmp->x2 - x1;

            src = bmp->y1 * swadd;
            u8 *tptr = tgt;
            s32 w = x1;
            wh = width - (w + 1);
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = x1) {
                idx = (wh + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b < to; b++, w++) {
                    tptr[w] = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4;
                }
            }
            
            bmp->x2 = x1;
        }

        src = bmp->y1 * swadd;
        for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; ) {
                wh = width - (w + 1);
                idx = (wh + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = 0; b < 8; b++, w++) {
                    tgt[w] = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4;
                }
            }
        }
    }
    else {
        s16 x1 = (bmp->x1 >> 3) << 3;
        if (x1 != bmp->x1)
        {
            s16 fr = 7 - (bmp->x1 - x1);
            s16 to = (x1 + 8 > bmp->x2) ? -1 + (x1 + 8 - bmp->x2) : -1;

            u8 *tptr = tgt;
            s32 w = bmp->x1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = bmp->x1) {
                idx = (x1 + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b > to; b--, w++) {
                    tptr[w] = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4;
                }
            }
            
            bmp->x1 = x1 + 8;
        }

        x1 = (bmp->x2 >> 3) << 3;
        if (x1 != bmp->x2 && bmp->x1 < bmp->x2)
        {
            s16 fr = 7;
            s16 to = -1 + (x1 + 8 - bmp->x2);

            src = bmp->y1 * swadd;
            u8 *tptr = tgt;
            s32 w = x1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tptr+=host.pixelbuf.w, w = x1) {
                idx = (x1 + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = fr; b > to; b--, w++) {
                    tptr[w] = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4;
                }
            }
            
            bmp->x2 = x1;
        }

        src = bmp->y1 * swadd;
        for (s32 h = bmp->y1; h < bmp->y2; h++, src+=swadd, tgt+=host.pixelbuf.w) {
            for (s32 w = bmp->x1; w < bmp->x2; ) {
                idx = (w + src) >> 3;
                c0 = at[idx]; c1 = at[idx += planesize]; c2 = at[idx += planesize]; c3 = at[idx += planesize]; c4 = at[idx += planesize];
                for (s32 b = 7; b > -1; b--, w++) {
                    tgt[w] = ((c0 >> b) & 1) | ((c1 >> b) & 1) << 1 | ((c2 >> b) & 1) << 2 | ((c3 >> b) & 1) << 3 | ((c4 >> b) & 1) << 4;
                }
            }
        }
    }
}

void draw_4to8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset)
{
    u8 color;
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (bmp->x1 >> 1) + src;
            u8 *tptr = bmp->x1 + pos->x1 + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr >> 4)) *tptr = pal_offset + color;
            }

            bmp->x1++;
        }
        
        if ((bmp->x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - ((bmp->x2) >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1 - 1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr & 0b00001111)) *tptr = pal_offset + color;
            }

            bmp->x2--;
        }

        if (bmp->x1 != bmp->x2) {
            u8 *sptr = swadd - 1 + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    if ((color = sptr[-w] & 0b00001111)) tptr[0] = pal_offset + color;
                    if ((color = sptr[-w] >> 4)) tptr[1] = pal_offset + color;
                }
            }
        }
    }
    else {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = (bmp->x1 >> 1) + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr & 0b00001111)) *tptr = pal_offset + color;
            }
            
            bmp->x1++;
        }

        if ((bmp->x2 - 1) % 2 == 0) {
            bmp->x2--;
            u8 *sptr = (bmp->x2 >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                if ((color = *sptr >> 4)) *tptr = pal_offset + color;
            }
        }
        
        if (bmp->x1 != bmp->x2) {
            u8 *sptr = src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    if ((color = sptr[w] >> 4)) tptr[0] = pal_offset + color;
                    if ((color = sptr[w] & 0b00001111)) tptr[1] = pal_offset + color;
                }
            }
        }
    }
}

void draw_4to8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip, u8 pal_offset)
{
    u16 swadd = width >> 1;
    u8 *src = at + bmp->y1 * swadd;
    u8 *tgt = image.wdraw + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = swadd - 1 - (bmp->x1 >> 1) + src;
            u8 *tptr = bmp->x1 + pos->x1 + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = pal_offset + (*sptr >> 4);
            }

            bmp->x1++;
        }
        
        if ((bmp->x2 - 1) % 2 == 0) {
            u8 *sptr = swadd - 1 - ((bmp->x2) >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1 - 1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = pal_offset + (*sptr & 0b00001111);
            }

            bmp->x2--;
        }

        if (bmp->x1 != bmp->x2) {
            u8 *sptr = swadd - 1 + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    tptr[0] = pal_offset + (sptr[-w] & 0b00001111);
                    tptr[1] = pal_offset + (sptr[-w] >> 4);
                }
            }
        }
    }
    else {
        if (bmp->x1 % 2 == 1) {
            u8 *sptr = (bmp->x1 >> 1) + src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = pal_offset + (*sptr & 0b00001111);
            }
            
            bmp->x1++;
        }

        if ((bmp->x2 - 1) % 2 == 0) {
            bmp->x2--;
            u8 *sptr = (bmp->x2 >> 1) + src;
            u8 *tptr = (bmp->x2 + pos->x1) + tgt;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += host.pixelbuf.w) {
                *tptr = pal_offset + (*sptr >> 4);
            }
        }
        
        if (bmp->x1 != bmp->x2) {
            u8 *sptr = src;
            u8 *tptr = (bmp->x1 + pos->x1) + tgt;
            u16 twadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);

            bmp->x1>>=1;
            bmp->x2>>=1;
            for (s32 h = bmp->y1; h < bmp->y2; h++, sptr += swadd, tptr += twadd) {
                for (s32 w = bmp->x1; w < bmp->x2; w++, tptr+=2) {
                    tptr[0] = pal_offset + (sptr[w] >> 4);
                    tptr[1] = pal_offset + (sptr[w] & 0b00001111);
                }
            }
        }
    }
}

void draw_8bit_0(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u8 color;
    u16 wadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);
    u8 *src = (flip ? width - 1: 0) + at + 2 + bmp->y1 * width;
    u8 *tgt = image.wdraw + (bmp->x1 + pos->x1) + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += width, tgt += wadd) {
            for (s32 w = bmp->x1; w < bmp->x2; w++, tgt++) {
                if ((color = *(src - w)) != 0)
                    *tgt = color;
            }
        }
    }
    else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += width, tgt += wadd) {
            for (s32 w = bmp->x1; w < bmp->x2; w++, tgt++) {
                if ((color = *(src + w)) != 0)
                    *tgt = color;
            }
        }
    }
}

void draw_8bit_2(u8 *at, sRect *pos, sRect *bmp, s16 width, s8 flip)
{
    u16 wadd = host.pixelbuf.w - (bmp->x2 - bmp->x1);
    u8 *src = (flip ? width - 1: 0) + at + 2 + bmp->y1 * width;
    u8 *tgt = image.wdraw + (bmp->x1 + pos->x1) + (pos->y1 + bmp->y1) * host.pixelbuf.w;

    if (flip) {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += width, tgt += wadd) {
            for (s32 w = bmp->x1; w < bmp->x2; w++, tgt++)
                *tgt = *(src - w);
        }
    }
    else {
        for (s32 h = bmp->y1; h < bmp->y2; h++, src += width, tgt += wadd) {
            for (s32 w = bmp->x1; w < bmp->x2; w++, tgt++)
                *tgt = *(src + w);
        }
    }
}
#endif // !ALIS_NATIVE_PLANAR (draw_* family — planar versions in image_draw_planar.c)

void draw_fli_video(u8 *bitmap)
{
}

void mapchar(int a2,char *a4,s16 d3w,s16 d4w,s16 d5w)
{
    
}

u8 chartpalti16(u32 src)
{
    return chartvcol0 + (xread8(src + 1) >> 4);
}

u8 chartpalti8(u32 src)
{
    return chartvcol0 + (xread8(src + 1) >> 5);
}

u8 chartpalti6(u32 src)
{
    return chartvcol0 + (xread8(src + 1) / 43);
}

u8 chartpalti4(u32 src)
{
    return chartvcol0 + (xread8(src + 1) >> 6);
}

u8 chartpalti(u32 src)
{
    u32 tmp = (u32)xread8(src + 1) * (u32)chartvncol;
    return chartvcol0 + (s8)(tmp >> 0x10);
}

u8 chartpbyte(u32 src)
{
    return xread8(src + 1);
}

u8 chartpdeco(u32 src)
{
    return xread8(src);
}

#if defined(ALIS_NATIVE_PLANAR) && ALIS_NATIVE_PLANAR
// Plot one pixel into the 8-plane framebuffer (the RRQ map is drawn per-pixel through a mask).
static inline void planar_plot(u8 *buf, s32 x, s32 y, u8 c)
{
    if ((u32)x >= (u32)alis.platform.width || (u32)y >= (u32)alis.platform.height) return;
    u16 *chunk = (u16 *)(buf + (u32)y * alis.platform.width + (x & ~15));
    u16 m = (u16)(0x8000u >> (x & 15));
    for (int k = 0; k < 8; k++) {
        if ((c >> k) & 1) chunk[k] |= m; else chunk[k] &= (u16)~m;
    }
}
#endif

// DOS chart pixel: writes to tgt, skipped for missing textures (as the original).
// plot_x is the screen X; xdc (texture/dither phase) is not advanced across fully-masked
// column groups, so the planar path must not plot at xdc.
static void chartpixel(u8 *tgt, u32 src, u32 type_table_base, u16 xdc, u16 ydc, s32 plot_x)
{
    // Read terrain type from high byte of 16-bit cell
    u16 cell = (u16)xread16(src);
    u8 type = (cell >> 8) & 0x3F;

    // Index into type table (32 bytes per entry)
    u32 type_entry = type_table_base + type * 32;

    // Follow redirect if flag set
    if ((s8)xread8(type_entry + 0x14) < 0)
    {
        src += xread32(type_entry + 0x10);
        cell = (u16)xread16(src);
        type = (cell >> 8) & 0x3F;
        type_entry = type_table_base + type * 32;
    }

    // Check texture exists - skip pixel entirely if not (asm: JZ to exit without MOV [EDI])
    if (xread8(type_entry + 0x08) == 0 || xread32(type_entry + 0x04) == 0)
        return;

    // Resolve texture pointer
    u32 tex_ptr = xread32(type_entry + 0x04);
    u32 texture = tex_ptr + xread32(tex_ptr);

    // Compute texture coordinates
    u16 y_wrapped = (ydc & ((u16)xread16(texture + 4) - (u16)xread16(type_entry + 2))) + (u16)xread16(type_entry + 2);
    u16 y_scaled = y_wrapped * ((u16)xread16(texture + 2) + 1);
    u16 x_wrapped = xdc & (u16)xread16(texture + 2);
    u8 tex_pixel = xread8(texture + 8 + y_scaled + x_wrapped);

    // Compute shade from terrain cell data
    u8 height_nibble = (cell & 0xFF) >> 4;
    u8 normal_x2 = ((cell >> 14) & 3) << 1;
    s16 shade = ((-(s8)normal_x2 + (s8)alis.basedark) >> 1) + height_nibble;
    if (shade < 0) shade = 0;
    else if (shade > 15) shade = 15;

    // 2D palette lookup via darkness table, write directly to framebuffer
#if defined(ALIS_NATIVE_PLANAR) && ALIS_NATIVE_PLANAR
    // image.logic is planar: plot by coordinate.
    (void)tgt;
    {
        u8 mc = xread8(alis.ptrdark + ((u16)shade << 8) + tex_pixel);
        planar_plot(image.logic, plot_x, ydc, alis.platform.bpp == 4 ? (mc & 0xf) : mc);
    }
#else
    *tgt = xread8(alis.ptrdark + ((u16)shade << 8) + tex_pixel);
#endif
}

u8 chartptra(u32 src)
{
    // Compute VRAM offset from terrain type (lower 6 bits, scaled)
    s16 vram_offset = ((s8)xread8(src) & 0x3f) * 0x20 - 0xc00;

    // Follow terrain indirection if redirect flag is set
    if ((s8)xread8(alis.script->vram_org + 0x1 + vram_offset) < 0)
    {
        src += xread16(alis.script->vram_org + 0x10 + vram_offset);
        vram_offset = ((u16)xread16(src + 1) & 0x3f) * 0x20 - 0xc00;
    }

    // Compute shade level from source data and base darkness (clamped to 0-15)
    // NOTE: shade is computed but unused in the final lookup — matches original code
    s16 shade = (u16)((s8)xread8(src + 1) >> 4) + (alis.basedark - (u16)((s8)xread8(src) >> 6));
    if (shade < 0)
    {
        shade = 0;
    }
    else if (0xf < shade)
    {
        shade = 0xf;
    }

    // Look up color from darkness/dithering table
    s16 dark_idx = (s8)xread8(alis.script->vram_org + 0x8 + vram_offset) * 2;

    s8 result = ((xdeschart ^ ydeschart) & 1) == 0 ? (s8)xread8(alis.ptrdark + dark_idx) : (s8)xread8(alis.ptrdark + 1 + dark_idx);
    return result;
}

void draw_requiem_map(sSprite *sprite, u32 bitmap)
{
    s32 mapdata = (bitmap + 0x3d8);
    if (xread8(bitmap - 0x28) != 0x9)
    {
        if ((xread8(bitmap - 0x28) != 0xa) && (xread32(alis.atent + xread16(bitmap - 0x24)) != 0))
        {
            // Resolve tile data base address from entity's script
            s32 script_addr = xread32(xread32(alis.atent + xread16(bitmap - 0x24)) - 0x14);
            s32 tile_hdr = (xread32(script_addr + 0xe) + script_addr);
            s32 tile_base = xread32(tile_hdr) + tile_hdr;

            // Compute starting tile row and Y pixel offset
            u16 vert_offset = xread16(bitmap - 6) + (image.blocy1 - sprite->newy);
            u16 start_row = vert_offset / (u16)xread16(bitmap - 0x18);
            s16 tile_y = image.blocy1 - vert_offset % (u16)xread16(bitmap - 0x18);

            // Compute map pointer and tile grid dimensions
            u32 map_ptr = (mapdata + (int)(s16)(start_row + xread16(bitmap + 0x3d4) * ((u16)(xread16(bitmap - 0xa) + (image.blocx1 - sprite->newx)) / (u16)xread16(bitmap - 0x1c))));
            s16 tile_rows = (u16)(xread16(bitmap - 6) + (image.blocy2 - sprite->newy)) / (u16)xread16(bitmap - 0x18) - start_row;
            u32 tile_cols = (u32)(u16)(image.blocx2 - image.blocx1) / (u32)(u16)xread16(bitmap - 0x1c);
            s16 row_stride = xread16(bitmap + 0x3d4);

            // Half-tile offset adjustment for staggered grids
            if (((u8)xread8(bitmap - 0x26) & 2) != 0)
            {
                tile_y = ((u16)(xread16(bitmap - 0x18) - 1U) >> 1) + tile_y;
                if (start_row != 0)
                {
                    tile_rows++;
                    map_ptr--;
                    tile_y = tile_y - xread16(bitmap - 0x18);
                }

                tile_rows++;
            }

            u32 start_x = (u32)(u16)image.blocx1;
            u32 col_counter = tile_cols;
            s16 row_counter = tile_rows;
            u32 cur_x = image.blocx1;

            // Draw tile grid row by row, column by column
            do
            {
                u16 col_rem;
                do
                {
                    u16 tile_idx;
                    if ((u8)xread8(map_ptr) != 0 && (tile_idx = (xread16(bitmap - 0x22) + (u16)xread8(map_ptr)) - 1) <= (u16)xread16(bitmap - 0x20))
                    {
                        s32 tile_entry = (tile_base + (s16)(tile_idx * 4));
                        mapchar((int)mapdata,(char *)(alis.mem + xread32(tile_entry) + tile_entry), tile_rows, (s16)cur_x, tile_y);
                    }

                    map_ptr += xread16(bitmap + 0x3d4);
                    cur_x = (u32)(u16)(xread16(bitmap - 0x1c) + (s16)cur_x);
                    col_rem = (s16)col_counter - 1;
                    col_counter = (u32)col_rem;
                }
                while (col_rem != 0xffff);

                tile_y = xread16(bitmap - 0x18) + tile_y;
                cur_x = start_x & 0xffff;
                map_ptr += (s16)(1 - ((s16)tile_cols + 1) * row_stride);
                row_counter--;
                col_counter = tile_cols;
            }
            while (row_counter != -1);
        }
    }
    else
    {
        // Direct pixel-level map rendering (type 9)
        s32 base_col = (u16)xread16(mapdata - 0x3e2) / (u16)xread16(mapdata - 0x3f4);
        s32 pixel_x = image.blocx1 - sprite->newx;
        s32 base_row = (u16)xread16(mapdata - 0x3e0) / (u16)xread16(mapdata - 0x3f2);
        s32 pixel_y = (xread16(mapdata - 0x3d4) + sprite->newy) - image.blocy2;

        u32 entity_data = xread32(mapdata - 0x3ba);
        if (entity_data == 0)
        {
            return;
        }

        if (alis.platform.is_little_endian)
        {
            // DOS path: bitmap mask-based rendering with textured terrain

            // Bitmap pointer (mask data from mapdata)
            s32 col_groups_bm = base_col + (pixel_x >> 4);
            s32 adj_row_bm = base_row + pixel_y;

            u32 bitmap_stride = (u32)(u16)xread16(mapdata - 0x3c4);
            u32 bitmap_base = mapdata;
            if ((s8)xread8(mapdata - 1) < 0)
            {
                bitmap_base = xread32(xread32(mapdata));
            }
            u32 bitmap_src = bitmap_base + col_groups_bm * bitmap_stride + adj_row_bm * 2;

            // Entity source pointer (terrain cell data from entity_data)
            s16 x_log_diff = xread16(mapdata - 0x3c0) - xread16(entity_data - 0x3c0);
            s16 y_log_diff = xread16(mapdata - 0x3be) - xread16(entity_data - 0x3be);

            s32 col_groups_ent = col_groups_bm;
            if (x_log_diff > 0) col_groups_ent <<= x_log_diff;
            s32 adj_row_ent = adj_row_bm;
            if (y_log_diff > 0) adj_row_ent <<= y_log_diff;

            u32 entity_stride = (u32)(u16)xread16(entity_data - 0x3c4);
            u32 entity_base = entity_data;
            if ((s8)xread8(entity_data - 1) < 0)
            {
                entity_base = xread32(xread32(entity_data));
            }
            u32 entity_src = entity_base + col_groups_ent * entity_stride + adj_row_ent * 2;

            // Per-pixel and per-group strides
            s16 x_stride_diff = xread16(mapdata - 0x3c0) - 4 - xread16(entity_data - 0x3c0);
            u32 pixel_stride = x_stride_diff > 0 ? entity_stride << x_stride_diff : entity_stride;
            u32 group_stride = pixel_stride << 4;
            u32 row_advance = y_log_diff > 0 ? 2 << y_log_diff : 2;

            // Type table base for terrain lookups
            u32 type_table_base = entity_data - 0xC00;

            // Dimensions and screen setup
            // Align X to column group boundary (bitmap mask handles edge clipping)
            s16 sub_pixel_x = pixel_x & 0xf;
            s16 last_cg = base_col + ((image.blocx2 - sprite->newx) >> 4);
            s16 col_groups_count = last_cg - col_groups_bm + 1;
            s16 height = image.blocy2 - image.blocy1 + 1;

            xdeschart = image.blocx1 - image.wlogx1 - sub_pixel_x;
            ydeschart = image.blocy2 - image.wlogy1;

            u8 *tgt = image.logic + ydeschart * host.pixelbuf.w + xdeschart;
            // Screen X; diverges from xdeschart on fully-masked column groups (see chartpixel).
            s32 sx = xdeschart;

            // Render row-major, bottom-to-top
            while (height > 0)
            {
                u8 *tgt_save = tgt;
                u32 entity_src_save = entity_src;
                u32 bitmap_src_save = bitmap_src;
                u16 xdeschart_save = xdeschart;
                s32 sx_save = sx;

                for (s16 cg = 0; cg < col_groups_count; cg++)
                {
                    // NOTE: set 0xffff to draw whole map
                    u16 mask = ~(u16)xread16(bitmap_src);
                    bitmap_src += bitmap_stride;

                    if (mask == 0)
                    {
                        tgt += 16;
                        sx += 16;               // destination moves even though xdeschart does not
                        entity_src += group_stride;
                    }
                    else
                    {
                        for (int bit = 15; bit >= 0; bit--)
                        {
                            if (mask & (1 << bit))
                            {
                                chartpixel(tgt, entity_src, type_table_base, xdeschart, ydeschart, sx);
                            }
                            tgt++;
                            sx++;
                            xdeschart++;
                            entity_src += pixel_stride;
                        }
                    }
                }

                tgt = tgt_save;
                entity_src = entity_src_save;
                bitmap_src = bitmap_src_save;
                xdeschart = xdeschart_save;
                sx = sx_save;

                ydeschart--;
                bitmap_src += 2;
                entity_src += row_advance;
                tgt -= host.pixelbuf.w;
                height--;
            }
        }
        else
        {
            // Bitmap mask pointer setup
            s32 col_groups_bm = base_col + (pixel_x >> 4);
            s32 adj_row_bm = base_row + pixel_y;

            u32 bitmap_stride = (u32)(u16)xread16(mapdata - 0x3c4);
            u32 bitmap_base = mapdata;
            if ((s8)xread8(mapdata - 1) < 0)
            {
                bitmap_base = xread32(xread32(mapdata));
            }
            u32 bitmap_src = bitmap_base + col_groups_bm * bitmap_stride + adj_row_bm * 2;

            // Entity source setup
            u32 src = entity_data;
            if ((s8)xread8(entity_data - 1) < 0)
            {
                src = xread32(xread32(entity_data));
            }

            s16 x_log_diff = xread16(mapdata - 0x3c0) - xread16(entity_data - 0x3c0);
            s16 y_log_diff = xread16(mapdata - 0x3be) - xread16(entity_data - 0x3be);

            s16 src_stride = xread16(entity_data - 0x3c4);
            s16 x_stride_diff = x_log_diff - 4;
            s32 pixel_stride = x_stride_diff > 0 ? (s32)src_stride << x_stride_diff : (s32)src_stride;
            s32 group_stride = pixel_stride << 4;
            s32 row_advance = y_log_diff > 0 ? 2 << y_log_diff : 2;

            s32 col_groups_ent = col_groups_bm;
            if (x_log_diff > 0) col_groups_ent <<= x_log_diff;
            s32 adj_row_ent = adj_row_bm;
            if (y_log_diff > 0) adj_row_ent <<= y_log_diff;

            u32 entity_src = src + col_groups_ent * src_stride + adj_row_ent * 2;

            // Dimensions
            s16 sub_pixel_x = pixel_x & 0xf;
            s16 last_cg = base_col + ((image.blocx2 - sprite->newx) >> 4);
            s16 col_groups_count = last_cg - col_groups_bm + 1;
            s16 height = image.blocy2 - image.blocy1 + 1;

            // chartproc selection (matching Falcon destomap)
            s16 type = xread16(mapdata - 0x3b6);
            switch (type)
            {
                case 1:
                {
                    chartvcol0 = (u8)xread16(mapdata - 0x3b4);
                    chartvncol = xread16(mapdata - 0x3b2);
                    switch (chartvncol)
                    {
                        case 0x10: chartproc = &chartpalti16; break;
                        case 0x8:  chartproc = &chartpalti8; break;
                        case 0x6:  chartproc = &chartpalti6; break;
                        case 0x4:  chartproc = &chartpalti4; break;
                        default:   chartproc = &chartpalti; break;
                    }
                    break;
                }
                case 2:
                    chartproc = &chartpdeco;
                    break;
                default:
                    chartproc = &chartpbyte;
                    break;
            }

            xdeschart = image.blocx1 - image.wlogx1 - sub_pixel_x;
            ydeschart = image.blocy2 - image.wlogy1;

            u8 *tgt = image.logic + ydeschart * host.pixelbuf.w + xdeschart;

            // Render row-major, bottom-to-top (matching Falcon destomap)
            while (height > 0)
            {
                u8 *tgt_save = tgt;
                u32 entity_src_save = entity_src;
                u32 bitmap_src_save = bitmap_src;
                u16 xdeschart_save = xdeschart;

                for (s16 cg = 0; cg < col_groups_count; cg++)
                {
                    // Mask bit=1 → hidden, bit=0 → revealed; invert so 1=draw
                    u16 mask = ~(u16)xread16(bitmap_src);
                    bitmap_src += bitmap_stride;

                    if (mask == 0)
                    {
                        // Fully masked — skip 16 pixels
                        tgt += 16;
                        xdeschart += 16;
                        entity_src += group_stride;
                    }
                    else
                    {
                        for (int bit = 15; bit >= 0; bit--)
                        {
                            if (mask & (1 << bit))
                            {
#if defined(ALIS_NATIVE_PLANAR) && ALIS_NATIVE_PLANAR
                                u8 mc = (*chartproc)(entity_src);
                                planar_plot(image.logic, xdeschart, ydeschart,
                                            alis.platform.bpp == 4 ? (mc & 0xf) : mc);
#else
                                *tgt = (*chartproc)(entity_src);
#endif
                            }
                            tgt++;
                            xdeschart++;
                            entity_src += pixel_stride;
                        }
                    }
                }

                tgt = tgt_save;
                entity_src = entity_src_save;
                bitmap_src = bitmap_src_save;
                xdeschart = xdeschart_save;

                ydeschart--;
                bitmap_src += 2;
                entity_src += row_advance;
                tgt -= host.pixelbuf.w;
                height--;
            }
        }
    }
}

void draw_transarctica_map(sSprite *sprite, u32 mapaddr, sRect lim)
{
    s32 vram = xread32(xread16(mapaddr - 0x24) + alis.atent);
    if (vram != 0)
    {
        u16 tileidx;
        u16 tileoffset = xread16(mapaddr + 0x24);
        u16 tileadd = xread16(mapaddr - 0x22);
        u16 tilecount = xread16(mapaddr - 0x20);
        u16 tilex = xread16(mapaddr - 0x1c);
        u16 tiley = xread16(mapaddr - 0x18);
        
        s32 addr = get_0x14_script_org_offset(vram);
        vram = xread32(addr + 0xe) + addr;
        
        addr = xread32(vram) + vram;
        u16 tempy = (image.blocy1 - sprite->newy) + xread16(mapaddr - 6);
        
        s32 yc = tempy / (u16)tiley;
        s16 yo = image.blocy1 - tempy % (u16)tiley;

        u32 tileaddr = mapaddr + 0x28;
        s16 t1 = image.blocx1 - sprite->newx;
        s16 t2 = xread16(mapaddr - 10);
        
        tileaddr += (tileoffset * ((u16)(t1 + t2) / tilex) + yc);
        s16 mapheight = ((image.blocy2 - sprite->newy) + xread16(mapaddr - 6)) / (u16)tiley - yc;
        s16 mapwidth = (image.blocx2 - image.blocx1) / tilex;

        if ((xread8(mapaddr - 0x26) & 2) != 0)
        {
            yo += ((u16)(tiley - 1) >> 1);
            if (yc != 0)
            {
                mapheight ++;
                tileaddr --;
                yo -= tiley;
            }
            
            mapheight ++;
        }
        
        s32 addy = alis.platform.kind == EPlatformPC ? 0 : 1;

        if (alis.platform.bpp == 8)
        {
            u8 color = 63;
            
            for (s32 h = lim.y1; h < lim.y2; h++)
            {
                u8 *tgt = image.logic + lim.x1 + (h * host.pixelbuf.w);
                for (s32 w = lim.x1; w < lim.x2; w++, tgt++)
                {
                    *tgt = color;
                }
            }
        }

        for (int mh = mapheight; mh >= 0; mh--)
        {
            for (int mw = mapwidth; mw >= 0; mw--)
            {
                if (xread8(tileaddr) != 0 && (tileidx = (xread8(tileaddr) + tileadd) - 1) <= tilecount)
                {
                    vram = addr + (s16)(tileidx * 4);
                    u32 img = xread32(vram) + vram;
                    u8 *bitmap = (alis.mem + img);
                    s16 width = (s16)read16(bitmap + 2) + 1;
                    s16 height = (s16)read16(bitmap + 4) + 1;

                    u8 *at = bitmap + 6;

                    s16 posx1 = (mapwidth - mw) * tilex;
                    s16 posy1 = ((mapheight - mh) - addy) * tiley + (((tiley - 1) - height) / 2);

                    u8 flip = 0;
                    u8 color = 0;
                    u8 clear = 0;
                    u8 palidx = 0;

                    s16 bmpx1 = 0;
                    s16 bmpx2 = width;
                    s16 bmpy1 = 0;
                    s16 bmpy2 = height;
                    
                    switch (bitmap[0])
                    {
                        case 0x01:
                        {
                            // rectangle
                            
                            color = bitmap[1];
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    if ((w + posx1) < lim.x1)
                                        continue;

                                    if ((w + posx1) >= lim.x2)
                                        continue;

                                    if ((h + posy1) < lim.y1)
                                        continue;

                                    if ((h + posy1) >= lim.y2)
                                        continue;

                                    *tgt = color;
                                }
                            }
                            break;
                        }
                            
                        case 0x00:
                        case 0x02:
                        {
                            // ST image
                            
                            clear = bitmap[0] == 0 ? 0 : -1;
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                                    color = *(at + wh + h * (width / 2));
                                    color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                                    if (color != clear)
                                    {
                                        if ((w + posx1) < lim.x1)
                                            continue;

                                        if ((w + posx1) >= lim.x2)
                                            continue;

                                        if ((h + posy1) < lim.y1)
                                            continue;

                                        if ((h + posy1) >= lim.y2)
                                            continue;

                                        *tgt = color;
                                    }
                                }
                            }

                            break;
                        }
                            
                        case 0x10:
                        case 0x12:
                        {
                            // 4 bit image
                            
                            palidx = bitmap[6];
                            clear = bitmap[0] == 0x10 ? bitmap[7] : -1;
                            
                            at = bitmap + 8;
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    s16 wh = (flip ? (width - (w + 1)) : w) / 2;
                                    color = *(at + wh + h * (width / 2));
                                    color = w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                                    if (color != clear)
                                    {
                                        if ((w + posx1) < lim.x1)
                                            continue;

                                        if ((w + posx1) >= lim.x2)
                                            continue;

                                        if ((h + posy1) < lim.y1)
                                            continue;

                                        if ((h + posy1) >= lim.y2)
                                            continue;

                                        *tgt = palidx + color;
                                    }
                                }
                            }
                            
                            break;
                        }
                            
                        case 0x14:
                        case 0x16:
                        {
                            // 8 bit image
                            
                            palidx = bitmap[6]; // NOTE: not realy sure what it is, but definetly not palette index
                            clear = bitmap[0] == 0x14 ? bitmap[7] : -1;
                            
                            at = bitmap + 8;
                            
                            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
                            {
                                u8 *tgt = image.logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                                {
                                    color = *(at + (flip ? width - (w + 1) : w) + h * width);
                                    if (color != clear)
                                    {
                                        if ((w + posx1) < lim.x1)
                                            continue;
                                        
                                        if ((w + posx1) >= lim.x2)
                                            continue;
                                        
                                        if ((h + posy1) < lim.y1)
                                            continue;
                                        
                                        if ((h + posy1) >= lim.y2)
                                            continue;
                                        
                                        *tgt = color + palidx;
                                    }
                                }
                            }
                            
                            break;
                        }
                    };
                }
                
                tileaddr += tileoffset;
            }

            tileaddr += (s16)(1 - ((s16)mapwidth + 1) * tileoffset);
        }
    }
}


#pragma mark -
#pragma mark Helper functions


void mac_update_pos(s16 *x,s16 *y)
{
    *x *= 1.5;
    *y *= 1.5;
}
