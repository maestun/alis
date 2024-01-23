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


extern u8 *logic;
extern u8 *physic;

extern u8 ftopal;
extern u8 thepalet;
extern u8 defpalet;
extern u8 mpalet[768 * 4];

extern s16 fenx1;
extern s16 feny1;
extern s16 fenx2;
extern s16 feny2;
extern s16 clipx1;
extern s16 clipy1;
extern s16 clipx2;
extern s16 clipy2;
extern s16 clipl;
extern s16 cliph;
extern s16 wlogx1;
extern s16 wlogx2;
extern s16 wlogy1;
extern s16 wlogy2;
extern s16 wloglarg;
extern s16 loglarg;

u32 endframe = 0;

sFLICData bfilm;

// fls/fla video player

u8 fls_ham6 = 0;
u8 fls_s512 = 0;

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
u8 *vgalogic_df = pvgalogic + 0xdf00 + 0x13f;
extern u8 tpalet[768 * 4];

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

    bfilm.addr1 = 0;
    fls_state = 0;
    
    fls_ham6 = 0;
    fls_s512 = 0;
//    last_touche = 0;
}

u32 fls_decomp(u32 addr)
{
    u8 *prevlogic = vgalogic_df;
    if (fls_drawing != 0)
    {
        vgalogic_df = vgalogic;
        vgalogic = prevlogic;
        fls_drawing = 0;
    }

    bfilm.frame ++;
    u32 endframe = addr + xread32(addr);

    u8 *vgaptr = vgalogic_df;//(u8 *)(vgalogic_df + 0xa0);
    vgaptr += 0xa0;

    addr += 6;

    while (endframe > addr)
    {
        u8 len = xread8(addr); addr++;
        if (len >= 0x80)
        {
            s16 test = (s8)len;
            test += 0x80;
            
            s32 length = 0x100 - len;
            memcpy(vgaptr, alis.mem + addr, length); vgaptr += length; addr += length;
        }
        else
        {
            if ((s8)len < 2)
            {
                if (len == 1)
                {
                    vgaptr += xread16(addr); addr+=2;
                }
                else
                {
                    vgaptr += xread8(addr); addr++;
                }
            }
            else
            {
                u8 color = xread8(addr); addr++;
                memset(vgaptr, color, len); vgaptr += len;;
            }
        }
    }
    
    return endframe;
}

void fls_init(u32 addr)
{
    fls_ham6 = alis.platform.kind == EPlatformAmiga;
    fls_s512 = alis.platform.kind == EPlatformAtari;
    
    fls_pallines = (u16)xread8(addr + 8) * 2;
    bfilm.frames = xread16(addr + 6);
    bfilm.endptr = addr + xread32(addr);
    bfilm.frame = 0;
}

u32 fls_next(u32 addr)
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

u32 fls_pal(u32 addr)
{
    // set palette
    u8 *palette = alis.mem + addr + 6;

    addr += xread32(addr);

    int c = 0;
    for (int i = 0; i < 16; i++, palette += 2, c += 3)
    {
        mpalet[c + 0] = (palette[0] & 0b00001111) << 4;
        mpalet[c + 1] = (palette[1] >> 4) << 4;
        mpalet[c + 2] = (palette[1] & 0b00001111) << 4;
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
        u32 addr = bfilm.addr1;
        if (addr != 0)
        {
            s16 type = xread16(addr + 4);
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
            
            bfilm.addr1 = (addr & 1) + addr;
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
//    if (pvgalogic != 0)
//    {
//        io_mfree();
//        pvgalogic = 0;
//    }
}


// fli (flic video) video player

void fli_palette(u32 addr)
{
    u16 packets = *(u16 *)(alis.mem + addr), index = 0;
    addr+=2;
    
    do
    {
        index += (u8)(xread8(addr)); addr++;
        u16 len = xread8(addr); addr++;
        if (len == 0)
        {
            len = 256;
        }
        
        if (index + len > 256)
        {
            len = 256 - index;
            packets = 1;
        }

        u8 *ptr = mpalet + 3 * index;
        for (int c = 0; c < len; c++)
        {
            ptr[0] = xread8(addr) * 2; addr++;
            ptr[1] = xread8(addr) * 2; addr++;
            ptr[2] = xread8(addr) * 2; addr++;
            ptr += 3;
        }
        
        index += len;
    }
    while (--packets);

    ftopal = 0xff;
}

void fli_blackdata(void)
{
    memset(vgalogic, 0, 64000);
}

void fli_data(u32 a5)
{
    u8 *ptr = vgalogic;
    for (int i = 0; i < 200; i++, ptr += 320, a5 += 320)
    {
        memcpy(ptr, alis.mem + a5, 320);
    }
}

void fli_decomp(u32 addr, u8 partial)
{
    u32 index = 0;
    u16 len = 200;
    
    if (partial)
    {
        index = *(u16 *)(alis.mem + addr) * 320; addr+=2;
        len = *(u16 *)(alis.mem + addr); addr+=2;
    }
    
    while (len--)
    {
        u8 packets = xread8(addr); addr++;
        u16 col = 0;
        while (packets--)
        {
            if (partial)
            {
                col += (u16)(xread8(addr)); addr++;
            }
            
            short int count = (signed char) xread8(addr); addr++;
            if (partial) count = -count;
            if (count >= 0)
            {
                if (count == 0)
                    count = 256;
                
                if (col + count > 320)
                {
                    count = 320 - col;
                    len = packets = 0;
                }
                
                memset(vgalogic + index + col, xread8(addr), count); addr++;
            }
            else
            {
                count = -count;
                if (col + count > 320)
                {
                    count = 320 - col;
                    len = packets = 0;
                }
                
                memcpy(vgalogic + index + col, alis.mem + addr, count); addr+= count;
            }
            
            col += count;
        }
        
        index += 320;
    }
}

void fli_elements(u32 addr)
{
    if (endframe <= addr)
    {
        return;
    }

    u32 offset = *(u32 *)(alis.mem + addr);
    addr+=4;

    u16 type = *(u16 *)(alis.mem + addr);
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
    
    memcpy(physic, vgalogic, 320*200);
    memcpy(logic, vgalogic, 320*200);
}

void fli_init(u32 flcaddr)
{
    u32 length = *(u32 *)(alis.mem + flcaddr);
    bfilm.endptr = flcaddr + length;
    bfilm.addr1 = flcaddr + 0x80;
    bfilm.frames = *(u16 *)(alis.mem + flcaddr + 6);
    bfilm.frame = 0;
}

void fli_next(u32 addr)
{
    u32 length = *(u32 *)(alis.mem + addr);
    endframe = addr + length;
    fli_elements(addr + 16);
    bfilm.addr1 = endframe;
    bfilm.frame++;
}

s16 flitofen(void)
{
    u32 flcaddr = bfilm.addr1;
    u16 type = *(u16 *)(alis.mem + flcaddr + 4);
    if (type == 0xaf11)
    {
        fli_init(flcaddr);
        return 1;
    }
    else if (type == 0xf1fa && bfilm.endptr > flcaddr)
    {
        fli_next(flcaddr);
        return 1;
    }
    
    return 0;
}

void inifilm(void)
{
    bfilm.playing = 0;
    
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
    fenx1 = *(u16 *)(alis.mem + basemain + 0xe);
    fenx2 = *(u16 *)(alis.mem + basemain + 0x12) + fenx1;
    clipl = *(u16 *)(alis.mem + basemain + 0x12) + 1;
    feny1 = *(u16 *)(alis.mem + basemain + 0x10);
    feny2 = *(u16 *)(alis.mem + basemain + 0x14) + feny1;
    cliph = *(u16 *)(alis.mem + basemain + 0x14) + 1;
    wloglarg = loglarg;
    wlogx1 = 0;
    wlogy1 = 0;
    clipx1 = fenx1;
    clipy1 = feny1;
    clipx2 = fenx2;
    clipy2 = feny2;
    
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
            
            do { } while (alis.timeclock < (u32)(index + prevclock));
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
