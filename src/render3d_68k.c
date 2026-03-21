//
//  render3d_68k.c
//  alis
//
//  m68k-specific terrain rendering functions extracted from render3d.c
//

#include "alis.h"
#include "image.h"
#include "mem.h"
#include "render3d.h"
#include "render3d_68k.h"

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

void vgatofen_68k(void)
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
    for (int y = image.clipy1; y <= image.clipy2; y++, tgt+=image.bitmodulo, src+=image.vgamodulo) {
        for (int x = image.clipx1; x <= image.clipx2; x++, tgt++, src++) {
            *tgt = *src & limit;
        }
    }
}

void clrvga_68k(void)
{
    // m68k: fill with 0x1010 using 2-byte writes, width = fenx2-fenx1
    u16 width = image.fenx2 - image.fenx1;
    s16 rows = image.feny2 - image.feny1;
    u16 w2 = (width >> 1) + 1;
    s16 modulo = (image.wloglarg - 1) - width;
    u16 *ptr = (u16 *)(image.wlogic + image.fenx1 + (u32)(u16)(image.feny1 - image.wlogy1) * (u32)image.wloglarg);
    for (s16 y = rows; y >= 0; y--)
    {
        for (u16 x = 0; x < w2; x++)
        {
            *ptr++ = 0x1010;
        }
        ptr = (u16 *)((u8 *)ptr + modulo);
    }
}

void calclan0_68k(s32 scene_addr, s32 render_context)
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
            
            if ((xread16(render_context - 0x24e) & 1) != 0)
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
            
            if ((xread16(render_context - 0x24e) & 1) != 0)
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
    u16 dark_level = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0) + ((xread16(terrain_cell + 2) >> 8) & 0xc0) + ((xread16(terrain_cell) >> 8) & 0xc0) * 2) * -2;
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

                    if ((xread16(render_context - 0x24e) & 1) != 0)
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

                    if ((xread16(render_context - 0x24e) & 1) != 0)
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

        if ((xread16(render_context - 0x24e) & 1) != 0)
        {
            color = rot8(color);
        }

        u32 dst = (s32)image.precx + xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4));
        bartramin(render_context, dst, mid_height, max_cols, color);
    }
}


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
        image.ntstpix = (u16)(xread16(terrain_cell) >> 8);
        image.cztstpix = (u16)(xread16(terrain_cell) & 0xff);
        image.cxtstpix = (u16)((s16)((u32)packed_coord >> 0x10) * 2 - step_x << shift) >> 1;
        image.cytstpix = (u16)((s16)packed_coord * 2 - step_y << shift) >> 1;
        image.etstpix = 0xfffe;
        image.dtstpix = xread32hi16(render_context - 0x2e0);
    }
    barlands(drawy, barheight, barwidth);
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
                barlands(drawy, barheight, barwidth);
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
                    u16 dark_level = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0) + ((xread16(terrain_cell + 2) >> 8) & 0xc0) + ((xread16(terrain_cell) >> 8) & 0xc0) * 2) * -2;
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


void doland_68k(s32 scene_addr, s32 render_context)
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
               xread32hi16(render_context - 0x2e0));

    xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + proj_x) << 0x10);
    xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(proj_y - proj_x) << 6) / (s32)xread16(render_context - 0x3a4)) << 10);

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
        u32 row_start_y = trav_y + (xread32hi16(render_context - 0x29c) << 16);

        // ----- Snap column position to terrain grid alignment -----
        // The column step is ±1 in either X or Y axis.
        // Snap the starting position so it aligns with the accumulated row position.
        // Safety bound: if s16 wrap-around causes divergence, break after 512 iterations.
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
            xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(proj_y - proj_x) << 6) / (s32)xread16(render_context - 0x3a4)) << 10);

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
        alt_table += xread16(render_context - 0x25a);
        u16 alt_seg_idx = (u16)(((xread32(render_context - 0x2e0) - xread32(render_context - 0x2c8)) >> 8) / (s32)xread16(render_context - 0x262));

        {
            u16 max_seg = 0x31;

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

            xwrite16(render_context - 0x25a, ((s16)image.atalti + alt_seg_idx * 0x200) - (s16)alt_table);
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

                        s32 strip_ptr = xread32(terrain_grid + (s32)(s16)(wrap_x - next_scan_x) * 4);
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
                max_y = xread32lo16(render_context - 0x270) + xread16(alt_table + (s16)(xread16(render_context - 0x25a) + prescan_height * 2));
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

                    terrain_wrapped = 1;
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
                image.solpixy = xread32lo16(render_context - 0x278) + xread16(alt_table + (s16)(image.solh * 2));
                xwrite16(render_context - 0x246, image.solpixy);
                xwrite16(render_context - 0x25c, image.solpixy);
                if (image.solpixy < ground_clip_y)
                {
                    xwrite16(render_context - 0x246, ground_clip_y);
                    ground_clip_y = image.solpixy;
                }

                s16 col_dir_x = (s16)(col_step_x_raw >> 0x10);

                // Extract terrain type index: bits [13:8] encode type, shifted for table lookup
                s16 terrain_type_idx = ((cell_data & 0x3f00) >> 3) - 0xc00;
                u16 bar_height = prev_max_y - ground_clip_y;

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
                                    s16 oh_top_scr = xread32lo16(render_context - 0x270) + xread16(alt_table + (s16)(xread16(render_context - 0x25a) + oh_ceil_h * 2));
                                    u16 oh_bot_scr = xread32lo16(render_context - 0x270) + xread16(alt_table + (s16)(xread16(render_context - 0x25a) + (oh_bottom_h - oh_ceil_h) * -2));
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

void landtofi_68k(s16 unused, s16 scene_id)
{
    s32 scene_addr = alis.basemain + scene_id;

    // m68k path: original behavior with fdoland gate
    s16 save_cliph = image.cliph;
    s16 save_clipl = image.clipl;
    s16 save_clipy2 = image.clipy2;
    s16 save_clipx2 = image.clipx2;
    s16 save_clipy1 = image.clipy1;
    s16 save_clipx1 = image.clipx1;

    image.wloglarg = xread16(scene_addr + 0x12) + 1;
    image.wlogic = (u8 *)(alis.mem + xread32(xread32(scene_addr + 0x30)));
    image.wlogx1 = xread16(scene_addr + 0xe);
    image.wlogy1 = xread16(scene_addr + 0x10);
    image.mapscreen = scene_addr;

    if (image.fdoland != 0)
    {
        image.landclipx2 = xread16(scene_addr + 0x12) + xread16(scene_addr + 0xe);
        image.landcliph = (image.clipy2 - image.clipy1) + 1;
        image.clipl = (image.clipx2 - image.clipx1) + 1;
        image.vbarclipx2 = image.clipx2 + 1;
        image.landclipy1 = image.clipy1;

        s32 render_context = xread16(scene_addr + 0x42) + xread32(alis.atent + xread16(scene_addr + 0x40));
        image.cliph = image.landcliph;
        calctoy(scene_addr, render_context);
        spritland(scene_addr, xread16(scene_addr + 2));
        calclan0_68k(scene_addr, render_context);

        if (((xread8(scene_addr + 1) & 0x40) == 0) && (xread16(scene_addr + 0xa2) == 0))
        {
            clrvga_68k();
        }

        doland_68k(scene_addr, render_context);

        image.fdoland = 0;
        image.landone = 1;
    }

    image.clipx1 = save_clipx1;
    image.clipy1 = save_clipy1;
    image.clipx2 = save_clipx2;
    image.clipy2 = save_clipy2;
    image.clipl = save_clipl;
    image.cliph = save_cliph;
    vgatofen();

    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogy1 = image.logy1;
    image.wlogx2 = image.logx2;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;
}
