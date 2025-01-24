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

#include "alis.h"
#include "audio.h"
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "screen.h"
#include "utils.h"
#include "video.h"

#if ALIS_SDL_VER < 2
# include <SDL/SDL.h>
extern u8 dirty_pal;
extern SDL_Rect dirty_rects[4096];
extern int dirty_len;
#endif

u8 *endframe = NULL;

sFLICData bfilm;

u16 fls_drawing = 0;
u16 fls_pallines = 0;
s8  fls_state = 0;

u16 pal[16] = {
    0x0000,
    0x0019,
    0x0011,
    0x0811,
    0x0089,
    0x0881,
    0x0081,
    0x0088,
    0x0018,
    0x0012,
    0x0889,
    0x0812,
    0x0181,
    0x0181,
    0x0181,
    0x0181,
};

u8 pvgalogic[1024 * 1024];
u8 *vgalogic = pvgalogic + 0x40;
u8 *vgalogic_df = pvgalogic + 0xdf00 + 319;

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
    
//    last_touche = 0;
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

    bfilm.frame ++;
    u8 *endframe = addr + read32(addr);

    u8 *vgaptr = vgalogic_df;//(u8 *)(vgalogic_df + 0xa0);
    vgaptr += 0xa0;

    addr += 6;

    while (endframe > addr)
    {
        u8 len = *(addr); addr++;
        if (len >= 0x80)
        {
            s32 length = 0x100 - len;
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
                u8 color = *(addr); addr++;
                memset(vgaptr, color, len); vgaptr += len;;
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

    addr += read32(addr);

    int c = 0;
    for (int i = 0; i < 16; i++, palette += 2, c += 4)
    {
        image.mpalet[c + 0] = (palette[0] & 0b00001111) << 4;
        image.mpalet[c + 1] = (palette[1] >> 4) << 4;
        image.mpalet[c + 2] = (palette[1] & 0b00001111) << 4;
    }
    
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

void endfilm(void)
{
    flstofen(-1);
    bfilm.type = eAlisVideoNone;

//    if (pvgalogic != 0)
//    {
//        io_mfree();
//        pvgalogic = 0;
//    }
}


// fli (flic video) video player

void fli_palette(u8 *addr)
{
    u16 packets = read16le(addr), index = 0;
    addr+=2;
    
    do
    {
        index += *(addr); addr++;
        u16 len = *(addr); addr++;
        if (len == 0)
        {
            len = 256;
        }
        
        if (index + len > 256)
        {
            len = 256 - index;
            packets = 1;
        }

        u8 *ptr = image.mpalet + 4 * index;
        for (int c = 0; c < len; c++, ptr += 4)
        {
            ptr[0] = *(addr) << 2; addr++;
            ptr[1] = *(addr) << 2; addr++;
            ptr[2] = *(addr) << 2; addr++;
        }
        
        index += len;
    }
    while (--packets);

    image.ftopal = 0xff;
}

void fli_blackdata(void)
{
    memset(vgalogic, 0, 64000);
}

void fli_data(u8 *addr)
{
    u8 *ptr = vgalogic;
    for (int i = 0; i < alis.platform.height; i++, ptr += alis.platform.width, addr += alis.platform.width)
    {
        memcpy(ptr, addr, alis.platform.width);
    }
}

void fli_decomp(u8 *addr, u8 partial)
{
    u32 index = 0;
    u16 len = alis.platform.height;
    
    if (partial)
    {
        index = read16le(addr) * alis.platform.width; addr+=2;
        len = read16le(addr); addr+=2;
    }
    
    while (len--)
    {
        u8 packets = *addr; addr++;
        u16 col = 0;
        while (packets--)
        {
            if (partial)
            {
                col += *(addr); addr++;
            }
            
            short int count = (signed char) *(addr); addr++;
            if (partial) count = -count;
            if (count >= 0)
            {
                if (count == 0)
                    count = 256;
                
                if (col + count > alis.platform.width)
                {
                    count = alis.platform.width - col;
                    len = packets = 0;
                }
                
                memset(vgalogic + index + col, *(addr), count); addr++;
            }
            else
            {
                count = -count;
                if (col + count > alis.platform.width)
                {
                    count = alis.platform.width - col;
                    len = packets = 0;
                }
                
                memcpy(vgalogic + index + col, addr, count); addr+= count;
            }
            
            col += count;
        }
        
        index += alis.platform.width;
    }
}

void fli_elements(u8 *addr)
{
    if (endframe <= addr)
    {
        return;
    }

    u32 offset = read32le(addr);
    addr+=4;

    u16 type = read16le(addr);
    addr+=2;
    
    switch (type) {
        case 0xb:
            fli_palette(addr);
            break;
        case 0xc:
            fli_decomp(addr, 1);
            break;
        case 0xd:
            fli_blackdata();
            break;
        case 0xf:
            fli_decomp(addr, 0);
            break;
        case 0x10:
            fli_data(addr);
            break;
    }

    fli_elements(addr + offset - 6);
    
#if ALIS_SDL_VER >= 2
    memcpy(image.physic, vgalogic, alis.platform.width*alis.platform.height);
    memcpy(image.logic, vgalogic, alis.platform.width*alis.platform.height);
#endif
}

void fli_init(u8 *flcaddr)
{
    u32 length = read32le(flcaddr);
    bfilm.endptr = flcaddr + length;
    bfilm.addr1 = flcaddr + 0x80;
    bfilm.frames = read16le(flcaddr + 6);
    bfilm.frame = 0;
}

void fli_next(u8 *addr)
{
    u32 length = read32le(addr);
    endframe = addr + length;
    fli_elements(addr + 16);
    bfilm.addr1 = endframe;
    bfilm.frame++;
}

s16 flitofen(void)
{
    if (bfilm.addr1 != NULL)
    {
        u16 type = read16le(bfilm.addr1 + 4);
        if (type == 0xaf11)
        {
            fli_init(bfilm.addr1);
            return 1;
        }
        else if (type == 0xf1fa && bfilm.endptr > bfilm.addr1)
        {
            fli_next(bfilm.addr1);
            return 1;
        }
    }
    
    return 0;
}

void inifilm(void)
{
    bfilm.playing = 0;
    switch (alis.platform.kind) {
        case EPlatformAtari:
            bfilm.type = eAlisVideoS512;
            break;
        case EPlatformAmiga:
            bfilm.type = eAlisVideoHAM6;
            break;
        default:
            bfilm.type = eAlisVideoFLIC;
            break;
    };

    if (alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga)
    {
        flstofen(0);
    }
    else
    {
        flitofen();
    }
}

void runfilm(void)
{
    u32 basemain = bfilm.basemain;
    image.fenx1 = read16le(alis.mem + basemain + 0xe);
    image.fenx2 = read16le(alis.mem + basemain + 0x12) + image.fenx1;
    image.clipl = read16le(alis.mem + basemain + 0x12) + 1;
    image.feny1 = read16le(alis.mem + basemain + 0x10);
    image.feny2 = read16le(alis.mem + basemain + 0x14) + image.feny1;
    image.cliph = read16le(alis.mem + basemain + 0x14) + 1;
    image.wloglarg = image.loglarg;
    image.wlogx1 = 0;
    image.wlogy1 = 0;
    image.clipx1 = image.fenx1;
    image.clipy1 = image.feny1;
    image.clipx2 = image.fenx2;
    image.clipy2 = image.feny2;
    
    bfilm.playing = 1;
    
    while (true)
    {
        u32 prevclock = alis.timeclock;
        s16 result = (alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga) ? flstofen(0) : flitofen();
        if (0 < bfilm.waitclock)
        {
            s16 index = (s16)(((u32)(u16)bfilm.waitclock * 5) / 7);
            if (index == 0)
            {
                index = 1;
            }
            
            do {
#if ALIS_SDL_VER == 1
                sys_delay_frame();
                itroutine(20, NULL);
#endif
            } while (alis.timeclock < (u32)(index + prevclock));
        }
        
        if (result == 0)
            break;
        
        bfilm.batchframes--;
        if (bfilm.batchframes == 0)
        {
            bfilm.result = 0;
            return;
        }
        
        if (bfilm.batchframes < 0)
        {
            bfilm.batchframes++;
        }
    }
    
    bfilm.addr1 = bfilm.addr2;
    bfilm.result = -0x7ffc;
}
