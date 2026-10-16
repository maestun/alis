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

// FLS: Silmarils film format of the ST/Amiga versions (big-endian "ST" header,
// "SU"/"SW" RLE frames, "SV" 12-bit palettes) shown as S512/HAM6.

#include "alis.h"
#include "audio.h"
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "screen.h"
#include "../sys/sys.h"
#include "utils.h"
#include "video.h"

u16 fls_drawing = 0;
u16 fls_pallines = 0;
s8  fls_state = 0;

void fls_savscreen(void)
{
    vgalogic = (u8 *)(vgalogic + 0xffU);
    memset(vgalogic, 0, 57000);
    
    // TODO: store old palette/res

    fls_state = 0xff;
}

void fls_cleanup(void)
{
    // TODO: restore palette and screen

    bfilm.type = eAlisVideoNone;
    bfilm.addr1 = 0;
    fls_state = 0;
    
}

u8 fls_dirty_lines[200];
u8 fls_prev_dirty[200];
u8 fls_dirty_x1[200];
u8 fls_dirty_x2[200];
u8 fls_prev_x1[200];
u8 fls_prev_x2[200];

// Mark dirty scanlines + per-line byte ranges for a write spanning 'len'
// bytes at offset 'off' within the 32000-byte bitplane buffer (160 bytes/line).
// Also tracks which half (left 0-79, right 80-159) is dirty per line.
static void fls_mark_dirty(u32 off, u32 len)
{
    if (alis.platform.kind == EPlatformAmiga)
    {
        // Amiga HAM6: 6 contiguous 8000-byte planes, 40 bytes/row; a run may straddle a
        // plane boundary (end row wraps below start).
        u32 p0 = off % 8000, p1 = (off + len - 1) % 8000;
        u32 l0 = p0 / 40, l1 = p1 / 40;
        if (l1 < l0)  // straddled a plane boundary
        {
            for (u32 l = l0; l < 200; l++)
                fls_dirty_lines[l] |= 3;
            
            for (u32 l = 0; l <= l1 && l < 200; l++)
                fls_dirty_lines[l] |= 3;
        }
        else
        {
            for (u32 l = l0; l <= l1 && l < 200; l++)
                fls_dirty_lines[l] |= 3;
        }

        return;
    }

    u32 end = off + len - 1;
    // x/160 via reciprocal multiply (exact for x < 65536); avoids divul.l.
    u32 l0 = (off * 52429u) >> 23;
    u32 l1 = (end * 52429u) >> 23;
    if (l1 >= 200)
        l1 = 199;
    
    for (u32 l = l0; l <= l1; l++)
    {
        u8 x1 = (l == l0) ? (u8)(off - l0 * 160) : 0;
        u8 x2 = (l == l1) ? (u8)(end - l1 * 160) : 159;
        if (!fls_dirty_lines[l])
        {
            fls_dirty_x1[l] = x1;
            fls_dirty_x2[l] = x2;
        }
        else
        {
            if (x1 < fls_dirty_x1[l])
                fls_dirty_x1[l] = x1;
            
            if (x2 > fls_dirty_x2[l])
                fls_dirty_x2[l] = x2;
        }

        // Encode half-dirty in bits: bit 0 = left (bytes 0-79), bit 1 = right (80-159)
        u8 halves = 0;
        if (x1 < 80)
            halves |= 1;
        
        if (x2 >= 80)
            halves |= 2;
        fls_dirty_lines[l] |= halves;
    }
}

u8 *fls_decomp(u8 *addr)
{
    u8 *prevlogic = vgalogic_df;
    if (fls_drawing != 0)
    {
        vgalogic_df = vgalogic;
        vgalogic = prevlogic;
        fls_drawing = 0;
    }

    memset(fls_dirty_lines, 0, 200);

    bfilm.frame ++;

    u8 *endframe = addr + read32(addr);

    u8 *base = vgalogic_df + 0xa0;
    u8 *vgaptr = base;

    addr += 6;

    while (endframe > addr)
    {
        u8 len = *(addr); addr++;
        if (len >= 0x80)
        {
            s32 length = 0x100 - len;
            fls_mark_dirty((u32)(vgaptr - base), length);
            memcpy(vgaptr, addr, length); vgaptr += length; addr += length;
        }
        else
        {
            if ((s8)len < 2)
            {
                if (len == 1)
                {
                    vgaptr += read16(addr); addr+=2;
                }
                else
                {
                    vgaptr += *(addr); addr++;
                }
            }
            else
            {
                fls_mark_dirty((u32)(vgaptr - base), len);
                u8 color = *(addr); addr++;
                memset(vgaptr, color, len); vgaptr += len;
            }
        }
    }

    return endframe;
}

void fls_init(u8 *addr)
{
    fls_pallines = (u16)*(addr + 8) * 2;
    bfilm.frames = read16(addr + 6);
    bfilm.endptr = addr + read32(addr);
    bfilm.frame = 0;
}

u8 *fls_next(u8 *addr)
{
    if (fls_state < 0)
    {
        addr = fls_decomp(addr);
        fls_drawing = 1;
        fls_state = 1;
        return addr;
    }
    else
    {
        addr = fls_decomp(addr);
        fls_drawing = 1;
        return addr;
    }
}

u8 *fls_pal(u8 *addr)
{
    // set palette
    u8 *palette = addr + 6;
    PAL_WRITE(image.mpalet, 0, 16, RGB12(palette));

    u8 *rawpal = addr + 6;
    for (int i = 0; i < 16; i++, rawpal += 2)
    {
        bfilm.ham_pal[i][0] = (rawpal[0] & 0x0f) << 4;  // R
        bfilm.ham_pal[i][1] = (rawpal[1] >> 4) << 4;     // G
        bfilm.ham_pal[i][2] = (rawpal[1] & 0x0f) << 4;   // B
    }

    // HAM palette change dirties every row.
    memset(fls_dirty_lines, 3, 200);

    addr += read32(addr);
    return addr;
}

u32 flstofen(s16 clean)
{
    if (clean < 0)
    {
        fls_cleanup();
    }
    else
    {
        u8 *addr = bfilm.addr1;
        if (addr != 0)
        {
            s16 type = read16(addr + 4);
            if (type == 0x5354)
            {
                fls_init(addr);
                
                addr += alis.platform.kind == EPlatformAmiga ? 0x8 : 0xc;
            }
            else
            {
                if (fls_state == 0)
                {
                    fls_savscreen();
                }
                
                if (alis.platform.kind == EPlatformAmiga)
                {
                    if (type == 0x5355)
                    {
                        addr = fls_next(addr);
                        fls_drawing = 1;
                    }
                    else if (type == 0x5356)
                    {
                        addr = fls_pal(addr);
                    }
                    else
                    {
                        bfilm.addr1 = 0;
                        return 0;
                    }
                }
                else // Atari ST
                {
                    if (type == 0x5357)
                    {
                        addr = fls_next(addr);
                        fls_drawing = 1;
                    }
                    else
                    {
                        bfilm.addr1 = 0;
                        return 0;
                    }
                }
            }
            
            bfilm.addr1 = ((u64)addr & 1) + addr;
            if (bfilm.addr1 < bfilm.endptr)
            {
                return 1;
            }
        }
    }
    
    bfilm.addr1 = 0;
    return 0;
}
