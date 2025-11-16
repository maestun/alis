//
//  render3d.c
//  alis
//
//  Created by Vadim Kindl on 04.10.2025.
//  Copyright © 2025 Zlot. All rights reserved.
//

#include "alis.h"
#include "image.h"
#include "mem.h"
#include "render3d.h"


u8 fprectop  = 0;
u8 fprectopa = 0;
u8 fbottom   = 0;
u8 notopa    = 0;
u16 prectopa = 0;
u16 precbota = 0;
u16 prectopb = 0;
u16 precbotb = 0;
u16 prectopc = 0;
u16 precbotc = 0;
u16 prectopi = 0;
u16 precboti = 0;
u16 botalt   = 0;
u16 bothigh  = 0;
u16 solha    = 0;
u32 adresa   = 0;

u32 CONCAT13(u8 a, u32 b)
{
    return ((u32)a << 24) | (b & 0x00ffffff);
}

u32 CONCAT31(u8 a, u32 b)
{
    return ((u32)a << 8) | (b & 0x000000ff);
}

u32 CONCAT22(u16 a, u16 b)
{
    return ((u32)a << 16) | b;
}

u32 CARRY4(u32 p1, u32 p2)
{
    return (p1 + p2) < p1;
}

s16 SCARRY2(s32 p1, s32 p2)
{
    return (p1 + p2) >= INT16_MAX & (p1 + p2) <= INT16_MIN;
}

u8 SBORROW2(s16 x, s16 y)
{
    s16 r = (s16)((s32)x - (s32)y);
    return ((((x ^ y) & (x ^ r)) & 0x8000) != 0);
}

extern u8 tabatan[];
extern s16 *tabsin;
extern s16 *tabcos;

typedef struct {
    u32 top : 16;
    u32 bot : 16;
} S32;

typedef union {
  
    u32 value;
    S32 comp;
} U32;

typedef struct {
    
    s16 d0w;
    s16 d1w;
    s16 d3w;
    s16 d4w;
    
} sLSTLResult;

typedef struct {
    
    s32 d0;
    s32 d1;
    u32 d2;
    s16 d3w;
    s16 d4w;
    s16 d5w;
    s16 d6w;
    s32 d7;
    
} sCLTPResult;

sLSTLResult lenstoland(s32 sceneadr, s32 a3, s16 d0w, s16 d1w, s16 d2w)
{
    s16 angle = xread16(sceneadr + 0x38);
    s32 cosval = tabcos[angle];
    s32 sinval = tabsin[angle];

    sLSTLResult result;
    result.d0w = xread16(sceneadr + 0x16) + (s16)((d0w * cosval - d1w * sinval) >> 9);
    result.d1w = xread16(sceneadr + 0x18) + (s16)((d0w * sinval + d1w * cosval) >> 9);
    result.d3w = result.d0w >> ((u16)xread16(a3 - 0x3c0) & 0x3f);
    result.d4w = result.d1w >> ((u16)xread16(a3 - 0x3c0) & 0x3f);

    // TODO: why was it in original code???
    // s16 unkn = xread16(sceneadr + 0x1a) + d2w;

    return result;
}

sCLTPResult clandtopix(s32 sceneadr, s16 d0w, s16 d1w, s16 d2w)
{
    sCLTPResult result;

    s32 value = xread16(sceneadr + 0x38);

    s32 d3 = tabcos[value];
    s32 d4 = tabsin[value];
    s32 d5 = d0w * d4;

    s32 d0 = ((d0w * d3) + (d1w * d4)) >> 9;
    s32 d1 = ((d1w * d3) - d5) >> 9;
    
    if (xread16(sceneadr + 0x94) < d1)
    {
        result.d1 = -1;
        result.d0 = 0;
        result.d2 = 0;
        result.d3w = 0;
        result.d4w = 0;
        result.d5w = -1;
        result.d6w = 0x100;
        return result;
    }
    
    s32 d7 = xread16(sceneadr + 0x34);
    d7 = -d7;
    d7 += d7;
    d7 = tabsin[d7];
    d7 = (s16)d1 * d7;
    d7 >>= 9;
    s32 d2 = d2w;
    d2 += d7;
    d2 = -d2;
    d0 *= xread16(sceneadr + 0x76);
    d2 *= xread16(sceneadr + 0x7a);
    s32 d6 = xread16(sceneadr + 0x7a);
    d6 <<= 8;
    d7 = (s16)d1;
    d7 += xread16(sceneadr + 0x70);
    
    if (d7 == 0 || ((d7 > INT16_MAX || d7 < INT16_MIN) && d7 > 0) || (d7 <= INT16_MAX && d7 >= INT16_MIN && d7 < 0))
    {
        d7 = 1;
    }

    d3 = d0 / d7;
    if (d3 < INT16_MIN || d3 > INT16_MAX)
    {
        d3 = (d3 < 0 ? INT16_MIN : INT16_MAX) * (d7 < 0 ? -1 : 1);
    }
    
    d7 = d1 + xread16(sceneadr + 0x74);
    if (d7 == 0 || ((d7 > INT16_MAX || d7 < INT16_MIN) && d7 > 0) || (d7 <= INT16_MAX && d7 >= INT16_MIN && d7 < 0))
    {
        d7 = 1;
    }
    
    d4 = d2 / d7;
    if (d4 < INT16_MIN || d4 > INT16_MAX)
    {
        d4 = (d4 < 0 ? INT16_MIN : INT16_MAX) * (d7 < 0 ? -1 : 1);
    }
    
    d6 = (xread16(sceneadr + 0x7a) << 8) / d7;
    if (d6 < INT16_MIN || d6 > INT16_MAX)
    {
        d6 = (d6 < 0 ? INT16_MIN : INT16_MAX) * (d7 < 0 ? -1 : 1);
    }
    
    d5 = d1;
    
    result.d1 = d1;
    result.d0 = d0;
    result.d2 = d2; // 0x80c
    result.d3w = d3;
    result.d4w = d4; // 0x001
    result.d5w = d5;
    result.d6w = d6; // 0x18
    return result;
}

void calctoy(s32 sceneadr, s32 render_context)
{
    xwrite16(sceneadr + 0x94, xread16(render_context - 0x3f2) * 2 + xread16(sceneadr + 0x6c));
    xwrite16(render_context - 0x26c, xread16(sceneadr + 0xa));
    xwrite16(render_context - 0x26a, xread16(sceneadr + 0xc));
    xwrite16(render_context - 0x268, xread16(sceneadr + 0x16));
    xwrite16(render_context - 0x266, xread16(sceneadr + 0x18));
    xwrite16(render_context - 0x264, xread16(sceneadr + 0x1a));
    s16 idxa = xread16(sceneadr + 0x34);
    s16 idxb = xread16(sceneadr + 0x38);
    xwrite16(render_context - 0x28e, tabcos[idxa]);
    xwrite16(render_context - 0x28a, tabcos[idxb]);
    xwrite16(render_context - 0x290, tabsin[idxa]);
    xwrite16(render_context - 0x28c, tabsin[idxb]);
    xwrite16(render_context - 0x360, xread16(sceneadr + 0x16));
    xwrite16(render_context - 0x35e, xread16(sceneadr + 0x18));
    xwrite16(render_context - 0x35c, xread16(sceneadr + 0x1a));
    
    sLSTLResult res;
    res = lenstoland(sceneadr, render_context, (-xread16(render_context - 0x3f4) - (xread16(sceneadr + 0x6a) >> 1)) - xread16(render_context - 0x3f4), xread16(sceneadr + 0x6c), 0);
    xwrite16(render_context - 0x37e, res.d0w);
    xwrite16(render_context - 0x37c, res.d1w);
    xwrite16(render_context - 0x3a2, res.d3w);
    xwrite16(render_context - 0x3a0, res.d4w);

    res = lenstoland(sceneadr, render_context, ((xread16(sceneadr + 0x6a) >> 1) - xread16(render_context - 0x3f4)) - xread16(render_context - 0x3f4), xread16(sceneadr + 0x6c), 0);
    xwrite16(render_context - 0x378, res.d0w);
    xwrite16(render_context - 0x376, res.d1w);
    xwrite16(render_context - 0x39c, res.d3w);
    xwrite16(render_context - 0x39a, res.d4w);

    res = lenstoland(sceneadr, render_context, (-xread16(render_context - 0x3f4) - (xread16(sceneadr + 0x64) >> 1)) - xread16(render_context - 0x3f4), xread16(sceneadr + 0x66), 0);
    xwrite16(render_context - 0x372, res.d0w);
    xwrite16(render_context - 0x370, res.d1w);
    xwrite16(render_context - 0x396, res.d3w);
    xwrite16(render_context - 0x394, res.d4w);

    res = lenstoland(sceneadr, render_context, ((xread16(sceneadr + 0x64) >> 1) - xread16(render_context - 0x3f4)) - xread16(render_context - 0x3f4), xread16(sceneadr + 0x66), 0);
    xwrite16(render_context - 0x36c, res.d0w);
    xwrite16(render_context - 0x36a, res.d1w);
    xwrite16(render_context - 0x390, res.d3w);
    xwrite16(render_context - 0x38e, res.d4w);

    res = lenstoland(sceneadr, render_context, -xread16(render_context - 0x3f4), xread16(sceneadr + 0x6c), 0);
    xwrite16(render_context - 0x366, res.d0w);
    xwrite16(render_context - 0x364, res.d1w);
    xwrite16(render_context - 0x38a, res.d3w);
    xwrite16(render_context - 0x388, res.d4w);

    res = lenstoland(sceneadr, render_context, -xread16(render_context - 0x3f4), xread16(sceneadr + 0x66), 0);
    xwrite16(render_context - 0x35a, res.d0w);
    xwrite16(render_context - 0x358, res.d1w);
    xwrite16(render_context - 0x384, res.d3w);
    xwrite16(render_context - 0x382, res.d4w);
    u32 uVar1 = (u16)xread16(render_context - 0x3c0);

    sCLTPResult res2;
    res2 = clandtopix(sceneadr, (xread16(render_context - 0x3a2) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x16), (xread16(render_context - 0x3a0) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x18), -xread16(sceneadr + 0x1a));
    xwrite32(render_context - 0x330, res2.d0);
    xwrite32(render_context - 0x32c, res2.d1);
    xwrite32(render_context - 0x328, res2.d2);
    xwrite16(render_context - 0x354, res2.d3w);
    xwrite16(render_context - 0x352, res2.d4w);
    xwrite16(render_context - 0x350, res2.d5w);

    res2 = clandtopix(sceneadr, (xread16(render_context - 0x39c) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x16), (xread16(render_context - 0x39a) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x18), -xread16(sceneadr + 0x1a));
    xwrite32(render_context - 0x324, res2.d0);
    xwrite32(render_context - 0x320, res2.d1);
    xwrite32(render_context - 0x31c, res2.d2);
    xwrite16(render_context - 0x34e, res2.d3w);
    xwrite16(render_context - 0x34c, res2.d4w);
    xwrite16(render_context - 0x34a, res2.d5w);

    res2 = clandtopix(sceneadr, (xread16(render_context - 0x396) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x16), (xread16(render_context - 0x394) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x18), -xread16(sceneadr + 0x1a));
    xwrite32(render_context - 0x318, res2.d0);
    xwrite32(render_context - 0x314, res2.d1);
    xwrite32(render_context - 0x310, res2.d2);
    xwrite16(render_context - 0x348, res2.d3w);
    xwrite16(render_context - 0x346, res2.d4w);
    xwrite16(render_context - 0x344, res2.d5w);

    res2 = clandtopix(sceneadr, (xread16(render_context - 0x390) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x16), (xread16(render_context - 0x38e) << (uVar1 & 0x3f)) - xread16(sceneadr + 0x18), -xread16(sceneadr + 0x1a));
    xwrite32(render_context - 0x30c, res2.d0);
    xwrite32(render_context - 0x308, res2.d1);
    xwrite32(render_context - 0x304, res2.d2);
    xwrite16(render_context - 0x342, res2.d3w);
    xwrite16(render_context - 0x340, res2.d4w);
    xwrite16(render_context - 0x33e, res2.d5w);

    res2 = clandtopix(sceneadr, xread16(render_context - 0x366) - xread16(sceneadr + 0x16), xread16(render_context - 0x364) - xread16(sceneadr + 0x18), -0x100);
    xwrite32(render_context - 0x300, res2.d0);
    xwrite32(render_context - 0x2fc, res2.d1);
    xwrite32(render_context - 0x2f8, res2.d2);
    xwrite16(render_context - 0x33c, res2.d3w);
    xwrite16(render_context - 0x33a, res2.d4w);
    xwrite16(render_context - 0x338, res2.d5w);

    res2 = clandtopix(sceneadr, xread16(render_context - 0x35a) - xread16(sceneadr + 0x16), xread16(render_context - 0x358) - xread16(sceneadr + 0x18), -0x100);
    xwrite32(render_context - 0x2f4, res2.d0);
    xwrite32(render_context - 0x2f0, res2.d1);
    xwrite32(render_context - 0x2ec, res2.d2);
    xwrite16(render_context - 0x336, res2.d3w);
    xwrite16(render_context - 0x334, res2.d4w);
    xwrite16(render_context - 0x332, res2.d5w);
    xwrite16(sceneadr + 0x94, xread16(sceneadr + 0x6c) - xread16(render_context - 0x3f2));
}

void altiland(s32 sceneadr, s32 render_context)
{
    s16 sVar6 = xread16(render_context - 0x3f4);
    xwrite16(sceneadr + 0x6c, sVar6 + xread16(sceneadr + 0x6c));
    xwrite16(sceneadr + 0x66, xread16(sceneadr + 0x66) - sVar6);
    calctoy(sceneadr, render_context);
    sVar6 = xread16(render_context - 0x3f4);
    xwrite16(sceneadr + 0x6c, xread16(sceneadr + 0x6c) - sVar6);
    xwrite16(sceneadr + 0x66, sVar6 + xread16(sceneadr + 0x66));
    xwrite32(render_context - 0x2c8, xread32(render_context - 0x32c) << 0x10);

    s32 iVar1 = xread32(render_context - 0x2ec);
    s32 iVar2 = xread32(render_context - 0x2f8);
    s32 iVar4 = xread32(render_context - 0x2f8);
    s32 iVar10 = xread32(render_context - 0x32c) << 0x10;
    s32 iVar3 = (s16)(((xread32(render_context - 0x314) - xread32(render_context - 0x32c)) * 0x100) / 0x31) - 1;
    xwrite16(render_context - 0x262, (s16)iVar3);
    
    u32 d0;
    u32 d3;

    u32 ataltiptr = image.atalti;

    for (int s = 0; s < 0x31; s++)
    {
        d0 = (s16)(iVar4 / (s32)(s16)(xread16(render_context - 0x3a8) + (s16)((u32)iVar10 >> 0x10))) * 0x100;
        xwrite16(ataltiptr, 20000);
        d3 = d0;
        ataltiptr += 2;
        
        for (int i = 0; i < 0xff; i++, ataltiptr += 2)
        {
            d3 = (d3 << 16) | ((d3 >> 16) & 0xffff);
            s16 lower_word = -(s16)d3;
            xwrite16(ataltiptr, lower_word);
            d3 = (d3 & 0xffff0000) | (-lower_word & 0xffff);
            d3 = (d3 << 16) | ((d3 >> 16) & 0xffff);
            d3 += d0;
//            u16 lw = (u16)(d3 >> 0x10);
//            xwrite16(ataltiptr, -lw);
//            d3 = d0 + ((u32)lw << 0x10 | (d3 & 0xffff));
        }
        
        iVar10 += iVar3 * 0x100;
        iVar4 += (s16)((iVar1 - iVar2) / 0x31);
    }
    
    for (int i = 0; i < 0xff; i++, ataltiptr += 2)
    {
        xwrite16(ataltiptr, 20000);
    }

    image.fhorizon = (s16)(((s16)((s16)(((s16)(xread16(sceneadr + 0x6c) * 4) * 0x168) / 0x140) + 0x80U & 0xff00) * 0x140) / 0x168);
}


void iniland(s32 sceneadr, s32 render_context)
{
    image.purey2 = xread16(sceneadr + 0x6c) - xread16(sceneadr + 0x66);
    image.purey = image.purey2 >> 2;
    image.purey2 >>= 4;
    xwrite16(render_context - 0x298, 0);
    xwrite16(render_context - 0x296, 0);
    xwrite16(render_context - 0x294, xread16(render_context - 0x3e8));
    xwrite16(render_context - 0x292, xread16(render_context - 0x3e6));
    u16 atllnxt = ((u16)xread16(render_context - 0x3ca)) << 1;
    s16 atlllen = xread16(render_context - 0x3cc) - 1;
    
    if ((s8)xread8(render_context - 1) < 0)
    {
        // get address of block allocated in calloctab
        render_context = xread32(xread32(render_context));
    }
    
    atlllen <<= 2;
    for (int i = 0; i <= atlllen; i+=4, render_context += atllnxt)
    {
        xwrite32(image.atlland + i, render_context);
    }

    u16 atlpnxt = xread16(sceneadr + 0x12) + 1;
    render_context = xread32(xread32(sceneadr + 0x30));
    for (int i = 0; i <= 800; i+=4, render_context += atlpnxt)
    {
        xwrite32(image.atlpix + i, render_context);
    }
    
    u32 offset = image.atalias;
    for (int d = 1; d <= 0x40; d++)
    {
        for (int i = 0; i < 0x39 + 7; i++, offset+=2)
        {
            xwrite16(offset, ((i * 0x100) / d));
//            printf("\n0x%.6x: 0x%.4x", offset, xread16(offset));
        }
    }
}

void inilens(s32 sceneadr, s32 render_context)
{
    xwrite16(sceneadr + 0x94, (xread16(sceneadr + 0x6c) - xread16(render_context - 0x3f2)) - xread16(render_context - 0x3f2));
    xwrite16(sceneadr + 0x92, xread16(sceneadr + 0x66) >> 1);
    xwrite16(sceneadr + 0x70, (s16)((s32)((u32)(u16)xread16(sceneadr + 0x6c) * (u32)(u16)xread16(sceneadr + 100) - (u32)(u16)xread16(sceneadr + 0x66) * (u32)(u16)xread16(sceneadr + 0x6a)) / (s32)(xread16(sceneadr + 0x6a) - xread16(sceneadr + 100))));
    xwrite16(sceneadr + 0x76, (s16)(((s32)(xread16(sceneadr + 0x70) + xread16(sceneadr + 0x6c)) * (s32)(xread16(sceneadr + 0x12) + 1)) / (s32)xread16(sceneadr + 0x6a)));
    xwrite16(sceneadr + 0x74, (s16)((s32)((u32)(u16)xread16(sceneadr + 0x6c) * (u32)(u16)xread16(sceneadr + 0x68) - (u32)(u16)xread16(sceneadr + 0x66) * (u32)(u16)xread16(sceneadr + 0x6e)) / (s32)(xread16(sceneadr + 0x6e) - xread16(sceneadr + 0x68))));
    xwrite16(sceneadr + 0x7a, (s16)(((s32)(xread16(sceneadr + 0x74) + xread16(sceneadr + 0x6c)) * (s32)(xread16(sceneadr + 0x14) + 1)) / (s32)xread16(sceneadr + 0x6e)));
    xwrite16(render_context - 0x3ac, xread16(sceneadr + 0x70));
    xwrite16(render_context - 0x3aa, xread16(sceneadr + 0x76));
    xwrite16(render_context - 0x3a8, xread16(sceneadr + 0x74));
    xwrite16(render_context - 0x3a6, xread16(sceneadr + 0x7a));
    xwrite16(render_context - 0x3a4, xread16(sceneadr + 0x6a) / xread16(render_context - 0x3f4));
}

void initltra(void)
{
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

void openland(s16 scene)
{
    s32 sceneadr = alis.basemain + scene;
    s32 render_context = xread16(sceneadr + 0x42) + xread32(alis.atent + xread16(sceneadr + 0x40));
    if (xread32(sceneadr + 0x30) == 0)
    {
        u32 addr = io_malloc((u32)(u16)(xread16(sceneadr + 0x12) + 1) * (u32)(u16)(xread16(sceneadr + 0x14) + 1) + 0x140);
        xwrite32(sceneadr + 0x30, addr);
    }
    
    if (image.tlpix == 0)
    {
        image.tlpix = io_malloc(0x8720);
        image.atlpix = xread32(image.tlpix);
        image.atalti = image.atlpix + 0x320;
        image.atalias = image.atalti + 0x6400;
    }
    
    if (image.tlland == 0)
    {
        image.tlland = io_malloc(xread16(render_context - 0x3cc) * 4);
        image.atlland = xread32(image.tlland);
    }
    
    xwrite16(sceneadr + 0x1a, 0x14);
    xwrite16(sceneadr + 0x34, 0);
    xwrite16(sceneadr + 0x36, 0);
    xwrite16(sceneadr + 0x38, 0);
    initltra();
    inilens(sceneadr, render_context);
    iniland(sceneadr, render_context);
    altiland(sceneadr, render_context);
    image.landone = 0;
}

void landtopix(s32 sceneadr, u16 d1w)
{
    sSprite *sprite = SPRITE_VAR(d1w);
    image.newad = sprite->data;
    
    if (1 < (s8)sprite->creducing)
    {
        u16 uVar4;
        for (uVar4 = (sprite->sprite_0x28 - xread16(sceneadr + 0x38)) + 900; -1 < (int)((u32)uVar4 << 0x10); uVar4 -= 0x168) {}
        
        u32 uVar3 = ((u32)(u16)(s16)(char)sprite->creducing * (u32)(u16)(uVar4 + 0x168) + 0xb4) / 0x168;
        s16 sVar5 = (s16)uVar3;
        if ((s8)sprite->creducing <= (s8)uVar3)
        {
            sVar5 = 0;
        }
        
        image.newad += sVar5 * 4;
    }
    
    u32 bmp0 = xread32(image.newad) + image.newad;
    u32 bmp1 = bmp0;
    u32 data = image.zoombid;
    if (xread8(bmp0) == 3)
    {
        image.newad += xread16(bmp0 + 2) * 4;
        bmp1 = xread32(image.newad) + image.newad;
        data = bmp0;
    }
    
    u16 shift;
    image.newf = xread8(data + 1) ^ sprite->flaginvx;
    if ((s8)sprite->credon_off == -0x80)
    {
        image.newzoomy = sprite->newzoomy;
        image.newx = xread16(sceneadr + 0xa) + sprite->depx;
        image.newy = xread16(sceneadr + 0xc) - sprite->depz;
        image.newd = sprite->depy;
        shift = 0;
    }
    else
    {
        sCLTPResult res = clandtopix(sceneadr, sprite->depx - xread16(sceneadr + 0x16), sprite->depy - xread16(sceneadr + 0x18), sprite->depz - xread16(sceneadr + 0x1a));
        image.newzoomy = res.d6w;
        image.newd = res.d5w;
        image.newx = xread16(sceneadr + 0xa) + res.d3w;
        image.newy = xread16(sceneadr + 0xc) + res.d4w;
        shift = sprite->credon_off;
    }
    
    if ((xread8(bmp1 + 1) & 0x20U) != 0)
    {
        image.newzoomy *= 2;
    }
    
    if (8 < (char)shift)
    {
        image.newzoomy *= (shift - 8) >> 3;
        shift = 3;
    }
    
    image.newzoomx = image.newzoomy;
    if (0xdfffff < bmp1)
    {
        image.newad = image.landnewad;
        bmp1 = image.landdata;
        image.newd = -1000;
        sprite->state = 1;
    }
    
    shift = (shift + 1) & 0x3f;
    
    s32 varh = ((((xread16(bmp1 + 4) + 1) * image.newzoomy * 2 >> shift) + 0xff) >> 8) - 1;
    if (varh < 0)
        varh = 0;
    
    s32 varl = ((((xread16(bmp1 + 2) + 1) * image.newzoomx * 2 >> shift) + 0xff) >> 8) - 1;
    if (varl < 0)
        varl = 0;
    
    image.newh = (s16)varh;
    image.newl = (s16)varl;
    image.newy += (s16)(((s32)(s16)(((u16)xread16(bmp1 + 4) >> 1) - xread16(data + 6)) * (s32)image.newzoomy >> 8) * -2 >> shift);
    image.newx += (s16)(((s32)(s16)(((u16)xread16(bmp1 + 2) >> 1) - xread16(data + 4)) * (s32)image.newzoomx >> 8) * -2 >> shift);
}

void spritland(s32 sceneadr, u16 d3w)
{
    u16 d1w = d3w;
    u16 d2w;

    while (true)
    {
        d2w = d1w;

        sSprite *spriteXX = SPRITE_VAR(d2w);
        d1w = spriteXX->link;
        if (d1w == 0)
            break;
        
        sSprite *sprite0 = SPRITE_VAR(d1w);
        
//        printf("\nspx depx: %.4x, %.4x, %.4xn", (u16)spriteXX->depx, (u16)spriteXX->depy, (u16)spriteXX->depz);
//        printf("sp0 depx: %.4x, %.4x, %.4x\n", (u16)sprite0->depx, (u16)sprite0->depy, (u16)sprite0->depz);

        if (sprite0->state != 0)
        {
            if (sprite0->state == 1)
            {
                spriteXX->link = sprite0->link;
                sprite0->to_next = image.libsprit;

//                printf("\nspritlandA: 0x%.4x", (u16)d1w);
//                printf("      0x%.4x\n", (u16)sprite0->newd);

                image.libsprit = d1w;
                d1w = d2w;
            }
            else
            {
                landtopix(sceneadr, d1w);
                
//                printf("\nspritlandB: 0x%.4x", (u16)d1w);
//                printf("      0x%.6x\n", (u32)image.newad);
//                printf("      0x%.4x\n", (u16)image.newx);
//                printf("      0x%.4x\n", (u16)image.newy);
//                printf("      0x%.4x\n", (u16)image.newd);
//                printf("      0x%.2x\n", (u8)image.newf);
//                printf("      0x%.4x\n", (u16)image.newl);
//                printf("      0x%.4x\n", (u16)image.newh);
//                printf("      0x%.4x\n", (u16)image.newzoomx);
//                printf("      0x%.4x\n", (u16)image.newzoomy);

                sprite0->newad = image.newad;
                sprite0->newx = image.newx;
                sprite0->newy = image.newy;
                sprite0->newd = image.newd;
                sprite0->newf = image.newf;
                sprite0->width = image.newl;
                sprite0->height = image.newh;
                sprite0->newzoomx = image.newzoomx;
                sprite0->newzoomy = image.newzoomy;

                spriteXX->link = sprite0->link;

                d2w = rangesprite(d1w, d2w, d3w);
                d1w = d2w;
            }
        }
    }
}

void calclan0(s32 sceneadr, s32 render_context)
{
    s16 sVar3;
    s32 uVar1;
    u16 uVar5;
    
    u32 uVar6 = (u32)(u16)xread16(render_context - 0x3c0);
    image.signedx = xread16(render_context - 0x378) - xread16(render_context - 0x37e);
    s16 sVar2 = image.signedx;
    if (image.signedx < 0)
    {
        sVar2 = -image.signedx;
    }
    
    image.signedy = xread16(render_context - 0x376) - xread16(render_context - 0x37c);
    s16 sVar4 = image.signedy;
    if (image.signedy < 0)
    {
        sVar4 = -image.signedy;
    }
    
    if (sVar2 < sVar4)
    {
        xwrite16(render_context - 0x2b0, sVar4);
        uVar5 = xread16(render_context - 0x36a) - xread16(render_context - 0x370);
        if ((int)((u32)uVar5 << 0x10) < 0) {
            uVar5 = -uVar5;
        }
        
        xwrite16(render_context - 0x2ac, uVar5);
        sVar2 = xread16(render_context - 0x378) - xread16(render_context - 0x37e);
        sVar3 = (s16)(((int)((u32)(u16)(sVar2 % sVar4) << 0x10) >> 1) / (int)sVar4);
        xwrite32(render_context - 0x2c4, (u32)(u16)(sVar2 / sVar4) * 0x10000 + sVar3 * 2);
        uVar1 = 0x10000;
        if (image.signedy < 0)
        {
            uVar1 = 0xffff0000;
        }
        
        xwrite32(render_context - 0x2c0, uVar1);
        sVar2 = (s16)((int)(s16)(xread16(render_context - 0x37c) - xread16(render_context - 0x370)) * (int)(s16)(xread32(render_context - 0x2c4) >> 2) >> 0xe);
        if (xread32(render_context - 0x2c0) < 0)
        {
            sVar2 = -sVar2;
        }
        
        uVar5 = (xread16(render_context - 0x372) + sVar2) - xread16(render_context - 0x37e);
        if ((int)((u32)uVar5 << 0x10) < 0)
        {
            uVar5 = -uVar5;
        }
        xwrite16(render_context - 0x24e, (s16)uVar5 >> (uVar6 & 0x3f));
    }
    else
    {
        xwrite16(render_context - 0x2b0, sVar2);
        uVar5 = xread16(render_context - 0x36c) - xread16(render_context - 0x372);
        if ((int)((u32)uVar5 << 0x10) < 0)
        {
            uVar5 = -uVar5;
        }
        
        xwrite16(render_context - 0x2ac, uVar5);
        sVar4 = xread16(render_context - 0x376) - xread16(render_context - 0x37c);
        sVar3 = (s16)(((int)((u32)(u16)(sVar4 % sVar2) << 0x10) >> 1) / (int)sVar2);
        xwrite32(render_context - 0x2c0, (u32)(u16)(sVar4 / sVar2) * 0x10000 + sVar3 * 2);
        uVar1 = 0x10000;
        if (image.signedx < 0)
        {
            uVar1 = 0xffff0000;
        }
        
        xwrite32(render_context - 0x2c4, uVar1);
        sVar2 = (s16)((int)(s16)(xread16(render_context - 0x37e) - xread16(render_context - 0x372)) * (int)(s16)(xread32(render_context - 0x2c0) >> 2) >> 0xe);
        if (xread32(render_context - 0x2c4) < 0)
        {
            sVar2 = -sVar2;
        }
        
        uVar5 = (xread16(render_context - 0x370) + sVar2) - xread16(render_context - 0x37c);
        if ((int)((u32)uVar5 << 0x10) < 0)
        {
            uVar5 = -uVar5;
        }
        
        xwrite16(render_context - 0x24e, (s16)uVar5 >> (uVar6 & 0x3f));
    }
    
    sVar2 = xread16(render_context - 0x24e);
    sVar4 = xread16(render_context - 0x372) - xread16(render_context - 0x37e);
    sVar3 = (s16)(((int)((u32)(u16)(sVar4 % sVar2) << 0x10) >> 1) / (int)sVar2);
    xwrite32(render_context - 0x2bc, (int)((u32)(u16)(sVar4 / sVar2) * 0x10000 + sVar3 * 2) >> (uVar6 & 0x3f));
    sVar4 = xread16(render_context - 0x370) - xread16(render_context - 0x37c);
    sVar3 = (s16)(((int)((u32)(u16)(sVar4 % sVar2) << 0x10) >> 1) / (int)sVar2);
    xwrite32(render_context - 0x2b8, (int)((u32)(u16)(sVar4 / sVar2) * 0x10000 + sVar3 * 2) >> (uVar6 & 0x3f));
    uVar1 = 0x10000;
    
    if (xread32(render_context - 700) < 0)
    {
        uVar1 = 0xffff0000;
    }
    
    xwrite32(render_context - 0x2a0, uVar1);
    uVar1 = 0x10000;
    
    if (xread32(render_context - 0x2b8) < 0)
    {
        uVar1 = 0xffff0000;
    }
    
    xwrite32(render_context - 0x29c, uVar1);
    xwrite32(render_context - 0x2b4, ((int)(s16)(((int)(s16)(xread16(render_context - 0x2ac) - xread16(render_context - 0x2b0)) << 8) / (int)xread16(render_context - 0x24e)) << 8) >> (uVar6 & 0x3f));
    xwrite32(render_context - 0x252, 0);
    xwrite32(render_context - 0x256, (u16)xread16(sceneadr + 0x9e) << 0x10);
    
    if (xread16(sceneadr + 0x98) != 0)
    {
        xwrite32(render_context - 0x256, 0x60000);
        uVar5 = xread16(sceneadr + 0x9e);
        if ((s16)uVar5 < 7)
        {
            xwrite32(render_context - 0x256, 0);
        }
        
        if (-1 < xread16(sceneadr + 0x98))
        {
            xwrite32(render_context - 0x256, (u32)(u16)xread16(sceneadr + 0xa0) * 0x10000 - 1);
        }
        
        xwrite32(render_context - 0x252, (int)(s16)(((int)((u32)uVar5 * 0x10000 - xread32(render_context - 0x256)) >> 8) / (int)(s16)(xread16(render_context - 0x24e) + 1)) << 8);
    }

    sVar2 = xread16(render_context - 0x24e);

    xwrite32(render_context - 0x2e4, (int)(s16)((xread32(render_context - 0x318) - xread32(render_context - 0x330)) / (int)sVar2));
    xwrite32(render_context - 0x2dc, (int)(s16)(((xread32(render_context - 0x2f0) - xread32(render_context - 0x2fc)) * 0x100) / (int)sVar2) << 8);
    xwrite32(render_context - 0x2d4, (int)(s16)((xread32(render_context - 0x310) - xread32(render_context - 0x328)) / (int)sVar2));
    xwrite32(render_context - 0x2cc, (int)(s16)(((xread32(render_context - 0x2ec) - xread32(render_context - 0x2f8)) * 0x100) / (int)sVar2) << 8);
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
    
    image.vgamodulo = image.wloglarg - ((image.clipx2 - image.clipx1) + 0);
    image.bitmodulo = image.loglarg * 2 - ((image.clipx2 - image.clipx1) + 0);
    
    u8 *ptr = image.logic;
    // TODO: why?
    // u8 *ptr = alis.fswitch == 0 ? image.logic : image.physic;
    u8 *tgt = ptr + image.clipx1 + image.clipy1 * image.loglarg * 2;
    for (int y = image.clipy1; y < image.clipy2; y++, tgt+=image.bitmodulo, src+=image.vgamodulo)
    {
        for (int x = image.clipx1; x < image.clipx2; x++, tgt++, src++)
        {
            *tgt = *src & 0xf;
        }
    }
}

u32 skyfast(u32 tgt, u8 fill, s16 d5w, s16 height)
{
    for (int l = 0; l < height; l++, tgt += (image.clipl + d5w))
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

void skytofen(s32 scene, s32 a3)
{
    s16 sVar1;
    u16 uVar2;
    s16 sVar4;
    s32 iVar3;
    s16 sVar5;
    s16 sVar6;
    s16 sVar7;
    u32 pcVar8;
    u32 pcVar9;
    u32 puVar10;
    u32 puVar11;
    s32 piVar12;
    u32 tgt;
    u32 puVar13;
    u32 puVar14;
    
    if (image.ftstpix != 0)
    {
        return;
    }
    
    piVar12 = xread32(scene + 0xa4);
    pcVar8 = xread32(piVar12) + piVar12;
    
    sVar4 = 0;
    pcVar9 = pcVar8;
    if (xread8(pcVar8) == 3)
    {
        piVar12 = ((s32)(s16)(xread16(pcVar8 + 2) * 4) + (s32)piVar12);
        pcVar9 = xread32(piVar12) + piVar12;
        sVar4 = xread16(pcVar8 + 6);
    }
    
    if (xread8(pcVar9) == 1)
    {
        skyfast((s32)image.clipx1 + xread32(image.atlpix + (s16)((image.clipy1 - image.wlogy1) * 4)), (xread8(pcVar9 + 1) & 0xfU) | (xread8(pcVar9 + 1) & 0xfU) << 4, image.wloglarg - image.clipl, image.cliph - 1);
    }
    else
    {
        image.skyposy = (sVar4 + xread16(scene + 0xc) + xread16(a3 - 0x352)) - ((u16)(xread16(pcVar9 + 4) + 1U) >> 1);
        sVar4 = xread16(pcVar9 + 2) + 1;
        sVar1 = (s16)((s16)((u32)((s32)xread16(scene + 0x38) * (s32)xread16(scene + 0xa8)) >> 8) - image.clipx1) % sVar4;
        uVar2 = -sVar1;
        if (0 < sVar1) {
            uVar2 = sVar4 + uVar2;
        }
        image.skyposx = uVar2 & 0xfffe;
        image.skyleft = sVar4 - image.skyposx;
        if ((s16)image.clipl < (s16)(sVar4 - image.skyposx)) {
            image.skyleft = image.clipl;
        }
        
        tgt = ((s32)image.clipx1 + xread32(image.atlpix + (s16)((image.clipy1 - image.wlogy1) * 4)));
        sVar4 = image.wloglarg - image.clipl;
        sVar1 = image.skyposy - image.clipy1;
        if (sVar1 != 0 && image.clipy1 <= image.skyposy) {
            if (image.cliph < sVar1) {
                sVar1 = image.cliph;
            }
            
            tgt = skyfast(tgt, xread8(pcVar9 + 6), sVar4, sVar1 - 1);
        }
        
        sVar1 = image.skyposy;
        if (image.skyposy < image.clipy1) {
            sVar1 = image.clipy1;
        }
        sVar5 = xread16(pcVar9 + 4) + image.skyposy;
        if (image.clipy2 < (s16)(xread16(pcVar9 + 4) + image.skyposy)) {
            sVar5 = image.clipy2;
        }
        sVar7 = sVar5 - sVar1;
        if (sVar7 != 0 && sVar1 <= sVar5) {
            uVar2 = xread16(pcVar9 + 2) + 1;
            iVar3 = 0;
            if ((u16)(image.clipy1 - image.skyposy) != 0 && image.skyposy <= image.clipy1) {
                iVar3 = (u32)(u16)(image.clipy1 - image.skyposy) * (u32)uVar2;
            }
            puVar14 = (pcVar9 + (s16)image.skyposx + iVar3 + 6);
            sVar1 = (image.skyleft >> 1) - 1;
            do {
                puVar11 = puVar14;
                puVar10 = puVar14;
                puVar13 = tgt;
                sVar5 = sVar1;
                if (-1 < sVar1) {
                    do {
                        puVar11 = puVar10 + 2;
                        tgt = puVar13 + 2;
                        xwrite16(puVar13, xread16(puVar10));
                        sVar5 = sVar5 - 1;
                        puVar10 = puVar11;
                        puVar13 = tgt;
                    } while (sVar5 != -1);
                }
                sVar5 = (image.clipl >> 1) - sVar1;
                sVar6 = sVar5 - 2;
                if (1 < sVar5) {
                    puVar11 = ((s32)puVar11 - (s32)(s16)uVar2);
                    puVar10 = tgt;
                    do {
                        tgt = puVar10 + 2;
                        xwrite16(puVar10, xread16(puVar11));
                        sVar6 = sVar6 - 1;
                        puVar11 = puVar11 + 2;
                        puVar10 = tgt;
                    } while (sVar6 != -1);
                }
                tgt = ((s32)sVar4 + (s32)tgt);
                puVar14 = ((s32)(s16)uVar2 + (s32)puVar14);
            }
            while ((--sVar7) != -1);
        }
        
        sVar1 = (image.clipy2 - image.skyposy) - xread16(pcVar9 + 4);
        if (sVar1 != 0 && xread16(pcVar9 + 4) <= (s16)(image.clipy2 - image.skyposy)) {
            if (image.cliph < sVar1) {
                sVar1 = image.cliph;
            }
            
            skyfast((s32)tgt, xread8(pcVar9 + (u32)(u16)(xread16(pcVar9 + 2) + 1) * (u32)xread16(pcVar9 + 4) + 6), sVar4, sVar1 - 1);
        }
    }
}

void skytofen2(s32 sceneadr, s32 a3)
{
    if (image.ftstpix != 0)
        return;
    
    s32 offset = xread32(sceneadr + 0xa4);
    s32 bitmap = xread32(offset) + offset;
    SkyData *sky = (SkyData *)(alis.mem + bitmap);

    s16 offset_y = 0;

    if (sky->type == 3)
    {
        offset_y = xread16(bitmap + 6);

        offset += (xswap16(sky->width) * 4);
        bitmap = xread32(offset) + offset;
        sky = (SkyData *)(alis.mem + bitmap);
    }

    s16 width_add = image.wloglarg - image.clipl;
    if (sky->type == 1)
    {
        u8 color = (sky->data1 & 0x0F) << 4 | (sky->data1 & 0x0F);
        u32 tgt = image.clipx1 + xread32(image.atlpix + ((image.clipy1 - image.wlogy1) * 4));
        skyfast(tgt, color, width_add, image.cliph - 1);
    }
    else
    {
        s16 width = xswap16(sky->width);
        s16 height = xswap16(sky->height);

        image.skyposy = (offset_y + xread16(sceneadr + 0xc) + xread16(a3 - 0x352)) - ((height + 1) >> 1);
        image.skyposx = -((((s32)xread16(sceneadr + 0x38) * (s32)xread16(sceneadr + 0xa8)) >> 8) - image.clipx1) % (width + 1);
        if (image.skyposx < 0)
            image.skyposx += width + 1;
        
        image.skyleft = width - image.skyposx;
        if (image.clipl < (width - image.skyposx))
            image.skyleft = image.clipl;

        u32 tgt = image.clipx1 + xread32(image.atlpix + ((image.clipy1 - image.wlogy1) * 4));
        s32 add_x = image.wloglarg - image.clipl;
        s16 lines = image.skyposy - image.clipy1;
        if (lines != 0 && image.clipy1 < image.skyposy)
        {
            if (image.cliph < lines)
                lines = image.cliph;

            tgt = skyfast(tgt, sky->color, add_x, lines - 1);
        }
        
        s16 clip_h = image.skyposy;
        if (image.skyposy < image.clipy1)
            clip_h = image.clipy1;
        
        s16 clip_b = xread16(bitmap + 4) + image.skyposy;
        if (image.clipy2 < (xread16(bitmap + 4) + image.skyposy))
            clip_b = image.clipy2;
        
        lines = clip_b - clip_h;
        if (lines > 0)
        {
            s32 offset = 0;
            if ((image.clipy1 - image.skyposy) > 0)
                offset = (image.clipy1 - image.skyposy) * (width + 1);
//                offset = clip_h/*(image.clipy1 - image.skyposy)*/ * (width + 1);
            
            s32 bitmap2 = (bitmap + image.skyposx + offset + 6);
            s32 prevbmp = bitmap2;
            s16 skylh = (image.skyleft >> 1) - 1;
            s16 bytes = 0;
            
            do
            {
                bytes = skylh + 1;
                if (-1 < skylh)
                {
                    memcpy(alis.mem + tgt, alis.mem + bitmap2, bytes << 1);
                    bitmap2 += bytes << 1;
                    tgt += bytes << 1;
                }
                
                bytes = ((image.clipl >> 1) - skylh) - 1;
                if (0 < clip_b)
                {
                    bitmap2 -= (width + 1);
                    memcpy(alis.mem + tgt, alis.mem + bitmap2, bytes << 1);
                    bitmap2 += bytes << 1;
                    tgt += bytes << 1;
                }
                
                tgt += add_x;
                prevbmp += (width + 1);
                bitmap2 = prevbmp;
            }
            while ((--lines) != -1);
        }
        
        lines = (image.clipy2 - image.skyposy) - height;
        if (lines != 0 && xread16(bitmap + 4) < (image.clipy2 - image.skyposy))
        {
            if (image.cliph < lines)
                lines = image.cliph;
            
            u8 color = xread8(bitmap + 6 + (width + 1) * height);
            skyfast(tgt, color, add_x, lines);
        }
    }
}

void glandtopix(s32 render_context, s16 *d0w, s16 *d1w, s16 d2w, s16 d3w, s16 d4w)
{
    s32 d0 = (s32)(s16)(((s32)*d1w * (s32)xread16(render_context - 0x28c) + (s32)*d0w * (s32)xread16(render_context - 0x28a)) >> 9) * (s32)xread16(render_context - 0x3aa);
    s32 d1 = (s32)(s16)(((s32)d3w * (s32)xread16(render_context - 0x28c) + (s32)d2w * (s32)xread16(render_context - 0x28a)) >> 9) * (s32)xread16(render_context - 0x3aa);
    d2w = xread16(render_context - 0x3ac) + d4w;
    if (d2w <= 0)
    {
        d2w = 1;
    }

    *d0w = d0 / d2w;
    *d1w = d1 / d2w;
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
    s16 maxpixels = barwidth - 1;
    
    s16 render1 = xread16(render_context - 0x3c4);
    u16 render2 = (u16)xread16(render1 + terrain_cell) & 0xc0;
    
    u16 pixels = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0) + (xread8(terrain_cell + 2) & 0xc0) + (xread8(terrain_cell) & 0xc0) * 2) * -2;
    u32 color = (u32)pixels;
    if ((s32)(color << 0x10) < 0)
    {
        color = 0;
    }
    
    color = (u16)xread16(alis.ptrdark + (s16)CONCAT31(((u16)color >> 8), (s8)xread8(render_context + 8 + index) * 2));
    color = CONCAT22(color, color);
    s16 lines = xread16(render_context - 0x246) - drawy;
    s16 midlines = barheight;
    
    if (lines > 1)
    {
        midlines -= lines;
        if (barheight < lines)
        {
            lines += midlines;
        }
        
        if (lines > 0)
        {
            u32 tgt;
            u32 prevtgt;
            u8 *tgtptr;

            u32 tempcolor;
            u32 prevcolor;

            u32 pixfraction;
            u16 pixmult = xread16(render_context - 0x246) - bary;
            image.vbarmid = midlines;
            
            if (1 < (s16)pixmult)
            {
                if ((pixmult < 0x41) && (image.vbarlarg < 0x40))
                {
                    pixmult = (u16)xread16(image.atalias + (s16)((image.vbarlarg + (pixmult - 1) * 0x40) * 2));
                }
                else
                {
                    pixmult >>= 2;
                    if (0x40 < pixmult)
                    {
                        pixmult = 0x40;
                    }
                    
                    pixmult = xread16(image.atalias + (s16)(image.vbarlarg + (pixmult - 1) * 0x80 & 0xfffe)) >> 1;
                }
                
                if (xread16(render_context - 0x246) == xread16(render_context - 0x25c))
                {
                    pixfraction = 0;
                    if (bary < (s16)drawy)
                    {
                        pixfraction = ((u16)(drawy - bary) & 0xff) * 0x100 * (u32)pixmult;
                    }
                    
                    if ((s16)maxpixels < (s16)image.vbarlarg)
                    {
                        pixfraction = (u32)(u16)((image.vbarx + (s16)(pixfraction >> 0x10)) - image.precx) << 0x10 | (pixfraction & 0xffff);
                    }
                    
                    if ((drawy & 1) != 0)
                    {
                        color = rot8(color);
                    }
                    
                    if ((xread8(render_context - 0x24d) & 1) != 0)
                    {
                        color = rot8(color);
                    }
                    
                    tgt = ((s32)image.precx + xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4)));
                    
                    pixels = pixfraction >> 0x10;
                    prevcolor = rot8(color);
                    prevtgt = (u32)tgt;
                    
                    do
                    {
                        tgtptr = alis.mem + tgt;
                        tempcolor = prevcolor;

                        if ((s16)maxpixels < (s16)pixels || (--lines) == -1)
                        {
                            if (lines < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, tgt, image.vbarmid, maxpixels, color);
                                }
                                return;
                            }
                            
                            if (-1 < (s16)pixels)
                            {
                                pixels = maxpixels;
                                prevcolor = tempcolor;
                                continue;
                            }
                            
                            if ((--lines) < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, tgt, image.vbarmid, maxpixels, color);
                                }
                                return;
                            }
                        }
                        else
                        {
                            for (int i = 0; i < (s16)pixels + 1; i++, color = rot8(color))
                                tgtptr[i] = (tgt & 1) == 0 ? rot8(color) : color;
                        }
                        
                        pixfraction += (s16)pixmult * 0x100;
                        tgt = ((s32)image.wloglarg + prevtgt);
                        pixels = pixfraction >> 0x10;
                        prevcolor = color;
                        color = tempcolor;
                        prevtgt = (u32)tgt;
                    }
                    while (true);
                }
                else
                {
                    pixfraction = 0;
                    if (bary < (s16)drawy)
                    {
                        pixfraction = ((u16)(drawy - bary) & 0xff) * 0x100 * (u32)pixmult;
                    }
                    
                    if ((s16)maxpixels < (s16)image.vbarlarg)
                    {
                        pixfraction = (u32)(u16)(((maxpixels + image.precx + (s16)(pixfraction >> 0x10)) - image.vbarx) - image.vbarlarg) << 0x10 | (pixfraction & 0xffff);
                    }
                    
                    if ((drawy & 1) != 0)
                    {
                        color = rot8(color);
                    }
                    
                    if ((xread8(render_context - 0x24d) & 1) != 0)
                    {
                        color = rot8(color);
                    }
                    
                    prevcolor = rot8(color);
                    pixels = pixfraction >> 0x10;
                    
                    tgt = (s32)(s16)maxpixels + (s32)image.precx + xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4)) + 1;
                    prevtgt = tgt;
                    
                    do
                    {
                        tgtptr = alis.mem + tgt;
                        tempcolor = prevcolor;
                        
                        if ((s16)maxpixels < (s16)pixels || (--lines) == -1)
                        {
                            if (lines < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, (tgt - 1) - maxpixels, image.vbarmid, maxpixels, color);
                                }
                                return;
                            }
                            
                            if (-1 < (s16)pixels)
                            {
                                pixels = maxpixels;
                                prevcolor = tempcolor;
                                continue;
                            }
                            
                            if ((--lines) < 0)
                            {
                                if ((s16)(image.vbarmid - 1) >= 0)
                                {
                                    bartramin(render_context, (tgt - 1) - maxpixels, image.vbarmid, maxpixels, color);
                                }
                                return;
                            }
                        }
                        else
                        {
                            xwrite8(tgt - 1, (char)color);
                            for (int i = 0; i < (s16)pixels; i++, color = rot8(color))
                                tgtptr[-i] = (tgt & 1) == 0 ? rot8(color) : color;
                            
                            tempcolor = color;
                        }
                        
                        pixfraction += (s16)pixmult * 0x100;
                        tgt = (s32)image.wloglarg + prevtgt;
                        pixels = pixfraction >> 0x10;
                        prevcolor = color;
                        color = tempcolor;
                        prevtgt = tgt;
                    }
                    while (true);
                }
            }
        }
    }
    
    if (-1 < (s16)(midlines - 1))
    {
        if ((drawy & 1) != 0)
        {
            color = rot8(color);
        }
        
        if ((xread8(render_context - 0x24d) & 1) != 0)
        {
            color = rot8(color);
        }
        
        u32 tgt = (s32)image.precx + xread32(image.atlpix + (s16)((drawy - image.wlogy1) * 4));
        bartramin(render_context, tgt, midlines, maxpixels, color);
    }
}

void barland(s32 terrain_cell, s32 render_context, s16 tstx, s16 tsty, s16 bary, s16 barheight, s16 index, s16 barx, s32 altitude_index, s32 d6)
{
    s16 prev_screen_x = image.precx;
    s16 barclipy = (bary - image.landclipy1) - image.landcliph;
    u16 drawy = bary;
    if (((u16)image.landcliph <= (u16)(bary - image.landclipy1) && barclipy != 0) || ((s16)(-barheight - barclipy) < 0))
    {
        if (image.clipy2 < bary)
        {
            return;
        }
        
        s16 barbot = bary + barheight - 1;
        if (barbot < (s16)image.landclipy1)
        {
            return;
        }
        
        if (bary < (s16)image.landclipy1)
        {
            barheight = bary + (barheight - image.landclipy1);
            drawy = image.landclipy1;
        }
        
        if (image.clipy2 < barbot)
        {
            barheight = image.clipy2 + (barheight - barbot);
        }
    }
    
    if (image.clipx1 <= barx)
    {
        image.vbarlarg = (barx - image.precx) - 1;
        image.vbarx = image.precx;
        if (image.precx < image.clipx1)
        {
            image.precx = image.clipx1;
        }
        
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
        
        if (-1 < xread16(render_context - 0x24e))
        {
            if (image.ftstpix == 0)
            {
                bartra(terrain_cell, render_context, drawy, index, barwidth, barheight, bary);
            }
            else
            {
                if ((s16)drawy <= image.ytstpix && image.ytstpix < (s16)(barheight + drawy) && image.precx <= image.xtstpix && image.xtstpix < (s16)(barwidth + image.precx))
                {
                    image.ntstpix = (u16)xread8(terrain_cell);
                    image.cztstpix = (u16)xread8(terrain_cell + 1);
                    image.cxtstpix = (u16)((s16)((u32)altitude_index >> 0x10) * 2 - tstx << ((u16)xread16(render_context - 0x3c0) & 0x3f)) >> 1;
                    image.cytstpix = (u16)((s16)altitude_index * 2 - tsty << ((u16)xread16(render_context - 0x3c0) & 0x3f)) >> 1;
                    image.etstpix = 0xfffe;
                    image.dtstpix = xread16(render_context - 0x2e0);
                }
                
                barlands();
            }
        }
    }
}


void tbarland(s32 terrain_cell, s32 render_context, s16 terrain_step_x, s32 a5, u16 bary, s16 barheight, s16 index, s32 screen_x, s32 d4, s32 d6)
{
    s16 prev_screen_x = image.precx;
    s16 clip_calc = (bary - image.landclipy1) - image.landcliph;
    u16 drawy = bary;
    if (((u16)image.landcliph <= (u16)(bary - image.landclipy1) && clip_calc != 0) || ((s16)(-barheight - clip_calc) < 0))
    {
        if (image.clipy2 < (s16)bary)
        {
            return;
        }
        
        clip_calc = bary + barheight - 1;
        if (clip_calc < (s16)image.landclipy1)
        {
            return;
        }
        
        if ((s16)bary < (s16)image.landclipy1)
        {
            barheight = bary + (barheight - image.landclipy1);
            drawy = image.landclipy1;
        }
        
        if (image.clipy2 < clip_calc)
        {
            barheight = image.clipy2 + (barheight - clip_calc);
        }
    }
    
    clip_calc = (s16)screen_x;
    if (image.clipx1 <= clip_calc)
    {
        image.vbarlarg = (clip_calc - image.precx) - 1;
        image.vbarx = image.precx;
        if (image.precx < image.clipx1)
        {
            image.precx = image.clipx1;
        }

        s16 barwidth;
        if (image.vbarclipx2 < clip_calc)
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
            barwidth = -(image.precx - clip_calc);
            if (barwidth == 0 || 0 < (s16)(image.precx - clip_calc))
            {
                image.vbarx = prev_screen_x;
                return;
            }
        }
        
        if (-1 < xread16(render_context - 0x24e))
        {
            if (bothigh < xread16(render_context - 0x246))
            {
                bothigh = xread16(render_context - 0x246);
            }
            
            image.vbarbot = 0;
            if (fbottom != 0)
            {
                if (bothigh <= (s16)drawy)
                {
                    barwidth --;
                    u16 uVar1x = image.vdarkw + (((u16)xread16(xread16(render_context - 0x3c4) + terrain_cell) & 0xc0) + (xread8(terrain_cell + 2) & 0xc0) + (xread8(terrain_cell) & 0xc0) * 2) * -2;
                    u32 color = (u32)uVar1x;
                    if ((s32)((u32)uVar1x << 0x10) < 0)
                    {
                        color = 0;
                    }
                    
                    uVar1x = (u16)xread16(alis.ptrdark + (s16)CONCAT31((color >> 8), (s8)xread8(render_context + 8 + index) * 2));
                    color = CONCAT22(uVar1x, uVar1x);
                    
                    bartrab(render_context, barwidth, barheight, drawy, bothigh, color);
                    return;
                }
                
                clip_calc = (barheight + drawy) - bothigh;
                if (clip_calc != 0 && bothigh <= (s16)(barheight + drawy))
                {
                    image.vbarbot = clip_calc;
                }
            }
            
            if (image.ftstpix == 0)
            {
                bartra(terrain_cell, render_context, drawy, index, barwidth, barheight - image.vbarbot, bary);
            }
            else
            {
                if ((s16)drawy <= image.ytstpix && image.ytstpix < (s16)(barheight + drawy) && image.precx <= image.xtstpix && image.xtstpix < (s16)(barwidth + image.precx))
                {
                    image.ntstpix = (u16)xread8(terrain_cell);
                    image.cztstpix = (u16)xread8(terrain_cell + 1);
                    image.cxtstpix = (u16)((s16)((u32)d4 >> 0x10) * 2 - terrain_step_x << ((u16)xread16(render_context - 0x3c0) & 0x3f)) >> 1;
                    image.cytstpix = (u16)((s16)d4 * 2 - (s16)a5 << ((u16)xread16(render_context - 0x3c0) & 0x3f)) >> 1;
                    image.etstpix = 0xfffe;
                    image.dtstpix = xread16(render_context - 0x2e0);
                }
                
                barlands();
            }
        }
    }
}

// Renders a zoomed 2D sprite to the framebuffer with clipping and optional collision detection
void zoomtofen(sSprite *sprite)
{
    // Get sprite bitmap data with offset stored in header
    u8 *bitmap = alis.mem + sprite->newad + xread32(sprite->newad);

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
        u32 zoom_x_step = CONCAT22((s16)((zoom_y_int << 0x18) >> 0x10), bitmap_width * (char)(zoom_y_scale >> 8));

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
                image.ztflowy = CONCAT13((char)texture_y, image.ztflowy >> 0x8);
                texture_y = (texture_y >> 8 & 0xffff) * (u32)(u16)(read16(bitmap + 2) + 1);
            }

            bool carry_flag;

            // Rendering path for horizontally flipped sprites
            if ((sprite->newf & 1) != 0)
            {
                // Start from right edge of bitmap for flipped rendering
                s32 bitmap_row_offset = (u16)read16(bitmap + 2) + texture_y;
                image.ztflowx = CONCAT22(0xffff, image.ztflowx >> 0x10);

                // Calculate X texture offset accounting for clipping on left edge
                if ((u16)(screen_x - x_orig) != 0)
                {
                    zoom_y_frac = (u32)(u16)(screen_x - x_orig) * ((zoom_y_frac & 0xff) << 8 | (zoom_y_int & 0xff));
                    image.ztflowx = CONCAT13(-1 - (char)zoom_y_frac, image.ztflowx >> 0x8);
                    bitmap_row_offset -= (zoom_y_frac >> 8);
                }

                s16 line_skip = image.wloglarg - clipped_width;
                zoom_y_frac = image.ztflowy;

                // Render pixels row by row
                for (int y = 0; y < clipped_height; y++, framebuffer += line_skip)
                {
                    zoom_y_int = CONCAT22((s16)(image.ztflowx >> 0x10), (s16)zoom_y_frac);

                    // Render individual pixels in row, stepping backward through texture (flipped)
                    for (int x = 0; x < clipped_width; x++, framebuffer++, zoom_y_int = CONCAT22((s16)((zoom_y_int - zoom_y_step) >> 0x10), (s16)(zoom_y_int - zoom_y_step) - (u16)(zoom_y_int < zoom_y_step)))
                    {
                        u8 pixel = bitmap[(s16)zoom_y_int + bitmap_row_offset + 8];
                        if (pixel != 0)  // Skip transparent pixels
                            *framebuffer = pixel;
                    }

                    // Step to next texture row with fixed-point arithmetic
                    carry_flag = CARRY4(zoom_x_step, zoom_y_frac);
                    zoom_y_frac = zoom_x_step + zoom_y_frac;
                    if (carry_flag)  // Handle fractional overflow to next line
                    {
                        zoom_y_frac = CONCAT22((s16)(zoom_y_frac >> 0x10), bitmap_width + (s16)zoom_y_frac);
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
                    image.ztflowx = CONCAT13((char)zoom_y_frac, image.ztflowx >> 0x8);
                    texture_y += (zoom_y_frac >> 8);
                }

                s16 line_skip = image.wloglarg - clipped_width;
                zoom_y_frac = image.ztflowy;

                // Render pixels row by row
                for (int y = 0; y < clipped_height; y++, framebuffer += line_skip)
                {
                    zoom_y_int = CONCAT22((s16)(image.ztflowx >> 0x10), (s16)zoom_y_frac);

                    // Render individual pixels in row, stepping forward through texture (normal)
                    for (int x = 0; x < clipped_width; x++, framebuffer++, zoom_y_int = CONCAT22((s16)((zoom_y_step + zoom_y_int) >> 0x10), (s16)(zoom_y_step + zoom_y_int) + (u16)CARRY4(zoom_y_step, zoom_y_int)))
                    {
                        u8 pixel = bitmap[(s16)zoom_y_int + texture_y + 8];
                        if (pixel != 0)  // Skip transparent pixels
                            *framebuffer = pixel;
                    }

                    // Step to next texture row with fixed-point arithmetic
                    carry_flag = CARRY4(zoom_x_step, zoom_y_frac);
                    zoom_y_frac += zoom_x_step;
                    if (carry_flag)  // Handle fractional overflow to next line
                    {
                        zoom_y_frac = CONCAT22((s16)(zoom_y_frac >> 0x10), bitmap_width + (s16)zoom_y_frac);
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

void barsprite(s32 render_context, s16 index, s16 world_x, s16 world_y, s16 world_z)
{
    sSprite *sprite = SPRITE_VAR(image.atexsprite);
    u32 bitmap_ptr = xread32(render_context + 0x10 + index);
    if (xread32(bitmap_ptr) != 0)
    {
        sCLTPResult result = clandtopix(image.mapscreen, (world_x << (xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x268), (world_y << (xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x266), image.solh - xread16(render_context - 0x264));
        s16 sprite_depth = result.d5w;
        if (sprite_depth < xread16(image.mapscreen + 0x92))
        {
            sprite_depth = -1;
        }
        
        sprite->newx = xread16(render_context - 0x26c) + (s16)result.d3w;
        sprite->newy = xread16(render_context - 0x26a) + (s16)result.d4w;
        sprite->newd = sprite_depth;
        sprite->newzoomx = result.d6w;
        sprite->newzoomy = result.d6w;
        
        if (-1 < sprite_depth)
        {
            sprite->script_ent = xread16(render_context + 0x1c + index);
            sprite->credon_off = xread8(render_context + 0x15 + index);
            
            u32 bitmap_base = xread32(bitmap_ptr) + bitmap_ptr;
            u32 zoom_data_ptr = image.zoombid;
            u32 bitmap = bitmap_base;
            if ((s8)xread8(bitmap) == 3)
            {
                bitmap_ptr += xread16(bitmap + 2) * 4;
                bitmap = xread32(bitmap_ptr) + bitmap_ptr;
                zoom_data_ptr = bitmap_base;
            }
            
            sprite->newad = bitmap_ptr;
            sprite->newf = xread8(zoom_data_ptr + 1);
            u8 detail_level = sprite->credon_off;
            if ((xread8(bitmap + 1) & 0x20) == 0)
            {
                detail_level += 1;
            }
            
            u32 detail_shift = (u32)detail_level;
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
            sprite->newy = (s16)(((s32)(s16)((xread16(bitmap + 4) >> 1) - xread16(zoom_data_ptr + 6)) * (s32)sprite->newzoomy >> 8) * -2 >> (detail_shift & 0x3f)) + sprite->newy;
            sprite->newx = (s16)(((s32)(s16)((xread16(bitmap + 2) >> 1) - xread16(zoom_data_ptr + 4)) * (s32)sprite->newzoomx >> 8) * -2 >> (detail_shift & 0x3f)) + sprite->newx;
            zoomtofenf(sprite);
        }
    }
}

void spritaff(u16 d0w)
{
    u16 idx = image.spritnext;
    while (true)
    {
        if (idx == 0)
        {
            image.spritnext = idx;
            image.spritprof = 0x8000;
            return;
        }

        sSprite *sprite = SPRITE_VAR(idx);
        if (sprite->newd <= d0w)
        {
            break;
        }
        
        if (-1 < sprite->state && -1 < sprite->newf && -1 < sprite->newd)
        {
            zoomtofenf(sprite);
        }
        
        idx = sprite->link;
    }

    image.spritprof = SPRITE_VAR(idx)->newd;
    image.spritnext = idx;
}

#include "export.h"

// Main 3D terrain rendering function for Robinson's Requiem
// Renders a 3D landscape using a fixed-point perspective grid with terrain height map
// TERRAIN DATA FORMAT:
//   atlland: Base pointer to terrain height map grid
//   Each cell stores 4-byte data accessible via terrain_data + (x * 4) or terrain_data + ((x - bounds) * 4)
//   Cell data contains:
//     - u8 at [0]: Terrain height index (0-0xFF) into altitude_table
//     - u8 at [1]: Processed height value used during rendering
//     - Additional properties including passability flags in upper bits
//
//   atalti: Altitude table (base height values for terrain)
//   Storage: Array of s16 values (2 bytes each) indexed by height index
//   Size: 512 entries (0x200) per altitude segment, accessible via:
//     altitude_table + (height_index * 2) for current segment
//     altitude_table + offset for adjacent segments
//   Segments indexed with stride of 0x200 bytes (256 entries)
//
void doland(s32 scene_addr, s32 render_context)
{
    image.spritprof = 0x8000;

    sSprite *sprite = SPRITE_VAR((u16)xread16(scene_addr + 0x2));
    image.spritnext = sprite->link;
    if (image.spritnext != 0)
    {
        sSprite *next_sprite = SPRITE_VAR(image.spritnext);
        image.spritprof = next_sprite->newd;       // Sprite depth ordering from linked sprite
    }

    if (xread16(scene_addr + 0xa2) != 0)
    {
        skytofen(scene_addr, render_context);
    }

    // Load terrain data pointers
    // atlland is loaded at init time: image.tlland = io_malloc(width * 4)
    // Each row of terrain grid is 4 bytes per cell (contains height + flags)
    s32 altitude_table = image.atalti;      // Points to base altitude values (s16 array)
    s32 terrain_data = image.atlland;       // Points to terrain height map grid
    
    // RENDERING ALGORITHM OVERVIEW:
    // 1. Grid-based perspective projection: iterates Y rows from far to near
    // 2. For each row, scan X columns with terrain lookup
    // 3. Terrain lookup: terrain_data[(x,y)] -> altitude_table[height_index] -> screen Y pixel
    // 4. Altitude table is segmented: base + (height_index * 2) for s16 lookup
    // 5. Terrain cells store: height index (u8), flags (u8), and type bits for texture selection
    //
    // ALTITUDE TABLE STRUCTURE (image.atalti):
    // - Allocated at init: io_malloc(0x8720 bytes total)
    // - Organization: 0x100 height indices per segment, 0x200 bytes per segment stride
    // - Each segment has independent height range for different altitude zones
    // - Accessed via: altitude_table + (position_index << 8) + (height_index * 2)
    // - Storage format: s16 values (screen Y pixel offset from base)

    fprectop = 0;
    fprectopa = 0;
    adresa = 0;
    prectopa = 1000;
    prectopb = 1000;
    prectopc = 1000;
    precbota = 1000;
    precbotb = 1000;
    precbotc = 1000;

    xwrite32(render_context - 0x25a, 0);
    xwrite32(render_context - 0x27c, 0);
    u16 position_x = (u16)xread16(render_context - 0x3a2);  // Camera X position in world space
    u16 position_y = (u16)xread16(render_context - 0x3a0);  // Camera Y position in world space
    xwrite32(render_context - 0x2a8, (u32)position_x << 0x10);
    xwrite32(render_context - 0x2a4, (u32)position_y << 0x10);
    u32 tempstep_x = (u32)xread32(render_context - 0x2c4);
    u32 terrainstep_x = tempstep_x << 0x10 | tempstep_x >> 0x10;
    u32 tempstep_y = (u32)xread32(render_context - 0x2c0);
    u32 terrainstep_y = tempstep_y << 0x10 | tempstep_y >> 0x10;
    xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + xread16(render_context - 0x352)));
    s16 screen_x = (position_x << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x360);
    s16 screen_y = (position_y << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x35e);
    
    glandtopix(render_context, &screen_x, &screen_y, (xread16(render_context - 0x378) + screen_x) - xread16(render_context - 0x37e), (xread16(render_context - 0x376) + screen_y) - xread16(render_context - 0x37c), xread16(render_context - 0x2e0));
    xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + screen_x) << 0x10);
    xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(screen_y - screen_x) << 8) / (s32)xread16(render_context - 0x3a4)) << 8);

    u32 tmppos_x = (u32)position_x << 16;
    u32 tmppos_y = (u32)position_y << 16;

    s32 uVar1, d1, iVar10;
    u16 uVar2, uVar4, uVar5, uVar9;
    s16 sVar3, barx, sVar12;
    u32 uVar6, d3, unkpos_x, unkpos_y, terrain_cell;
    
    export_terrain_debug(scene_addr, render_context, "terrain_dbg.txt");
    export_terrain_debug_endian(scene_addr, render_context, "terrain_dbg_end.txt");
    export_terrain_column_analysis(scene_addr, render_context, "terrain_column_analysis.txt");
    export_terrain_obj(scene_addr, render_context, "map.obj");
    export_terrain_overhangs(scene_addr, render_context, "map_overhangs.obj");

    do
    {
        xwrite32(render_context - 0x2a8, xread32(render_context - 0x2bc) + xread32(render_context - 0x2a8));
        xwrite32(render_context - 0x2a4, xread32(render_context - 0x2b8) + xread32(render_context - 0x2a4));
        u32 newpos_x = tmppos_x;
        u32 newpos_y = tmppos_y + (xread16(render_context - 0x29c) << 16);

        if (tempstep_x == 0x00010000)
        {
            if (xread16(render_context - 0x2bc) < 0)
            {
                while (xread16(render_context - 0x2a8) < (s16)(newpos_x >> 16))
                {
                    newpos_y -= tempstep_y;
                    newpos_x -= tempstep_x;
                }
            }
            else
            {
                while ((s16)(newpos_x >> 16) < xread16(render_context - 0x2a8))
                {
                    newpos_y += tempstep_y;
                    newpos_x += tempstep_x;
                }
            }
        }
        else if (tempstep_x == 0xffff0000)
        {
            if (xread16(render_context - 0x2bc) < 0)
            {
                while (xread16(render_context - 0x2a8) < (s16)(newpos_x >> 16))
                {
                    newpos_y += tempstep_y;
                    newpos_x += tempstep_x;
                }
            }
            else
            {
                while ((s16)(newpos_x >> 16) < xread16(render_context - 0x2a8))
                {
                    newpos_y -= tempstep_y;
                    newpos_x -= tempstep_x;
                }
            }
        }
        else
        {
            newpos_y = tmppos_y;
            newpos_x = tmppos_x + (xread16(render_context - 0x2a0) << 16);
            if (tempstep_y == 0x00010000)
            {
                if (xread16(render_context - 0x2b8) < 0)
                {
                    while (xread16(render_context - 0x2a4) < (s16)(newpos_y >> 16))
                    {
                        newpos_y -= tempstep_y;
                        newpos_x -= tempstep_x;
                    }
                }
                else
                {
                    while ((s16)(newpos_y >> 16) < xread16(render_context - 0x2a4))
                    {
                        newpos_y += tempstep_y;
                        newpos_x += tempstep_x;
                    }
                }
            }
            else
            {
                if (xread16(render_context - 0x2b8) < 0)
                {
                    while (xread16(render_context - 0x2a4) < (s16)(newpos_y >> 16))
                    {
                        newpos_y += tempstep_y;
                        newpos_x += tempstep_x;
                    }
                }
                else
                {
                    while ((s16)(newpos_y >> 16) < xread16(render_context - 0x2a4))
                    {
                        newpos_y -= tempstep_y;
                        newpos_x -= tempstep_x;
                    }
                }
            }
        }
        
        xwrite32(render_context - 0x256, xread32(render_context - 0x252) + xread32(render_context - 0x256));
        image.vdarkw = xread8(render_context - 0x255) << 8;
        iVar10 = xread32(render_context - 0x2dc) + xread32(render_context - 0x2e0);
        xwrite32(render_context - 0x2e0, iVar10);
        if (iVar10 < 0)
        {
            xwrite32(render_context - 0x2e0, xread32(render_context - 0x2e0) - xread32(render_context - 0x2dc));
        }
        
        xwrite32(render_context - 0x2d8, xread32(render_context - 0x2d4) + xread32(render_context - 0x2d8));
        uVar6 = (u32)xread32(render_context - 0x27c) << 0x10 | (u32)xread32(render_context - 0x27c) >> 0x10;
        d3 = (u32)xread32(render_context - 0x280) << 0x10 | (u32)xread32(render_context - 0x280) >> 0x10;
        xwrite32(render_context - 0x278, xread32(render_context - 0x270));
        if (-1 < xread16(render_context - 0x24e))
        {
            screen_x = ((s16)(newpos_x >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x360);
            screen_y = ((s16)(newpos_y >> 16) << ((u16)xread16(render_context - 0x3c0) & 0x3f)) - xread16(render_context - 0x35e);
            glandtopix(render_context, &screen_x, &screen_y, (xread16(render_context - 0x378) + screen_x) - xread16(render_context - 0x37e), (xread16(render_context - 0x376) + screen_y) - xread16(render_context - 0x37c), xread16(render_context - 0x2e0));
            xwrite32(render_context - 0x280, (u32)(u16)(xread16(render_context - 0x26c) + screen_x) << 0x10);
            xwrite32(render_context - 0x27c, (s32)(s16)(((s32)(s16)(screen_y - screen_x) << 6) / (s32)xread16(render_context - 0x3a4)) << 10);
            position_x = xread16(render_context - 0x3a8) + xread16(render_context - 0x2e0);
            if (position_x == 0 || SCARRY2(xread16(render_context - 0x3a8),xread16(render_context - 0x2e0)) != (s32)((u32)position_x << 0x10) < 0)
            {
                position_x = 1;
            }
            
            xwrite32(render_context - 0x270, (u32)(u16)(xread16(render_context - 0x26a) + (s16)(xread32(render_context - 0x2d8) / (s32)(s16)position_x)));
        }
        
        if (xread16(render_context - 0x2e0) <= image.spritprof)
        {
            spritaff((u16)xread16(render_context - 0x2e0));
        }
        
        altitude_table += xread16(render_context - 0x25a);
        position_x = (u16)(((xread32(render_context - 0x2e0) - xread32(render_context - 0x2c8)) >> 8) / (s32)xread16(render_context - 0x262));
        if (xread16(render_context - 0x24e) == 1)
        {
            position_x = 0x31;
        }
        
        if (0x31 < position_x)
        {
            if ((s16)position_x < 0x32)
            {
                position_x = 0;
            }
            else
            {
                position_x = 0x31;
            }
        }
        
        xwrite16(render_context - 0x25a, ((s16)image.atalti + position_x * 0x200) - (s16)altitude_table);
        image.precx = image.clipx1 - 0x10;
        xwrite16(render_context - 0x260, image.clipy2);
        xwrite16(render_context - 0x25c, image.clipy2);
        fprectop = 0;
        fprectopa = 0;
        position_x = (u16)xread16(render_context - 0x24e) - 1;
        xwrite16(render_context - 0x24e, position_x);
        if (((s32)((u32)position_x << 0x10) < 0) && (xread16(render_context - 0x24e) < -3))
        {
            spritaff(-1);
            return;
        }
        
        uVar1 = xread32(render_context - 0x280);
        unkpos_x = CONCAT22(newpos_x, newpos_x >> 16);
        unkpos_y = CONCAT22(newpos_y, newpos_y >> 16);
        
        do
        {
            screen_x = xread16(render_context - 0x260);
            screen_y = xread16(render_context - 0x25e);
            if (xread16(render_context - 0x25e) < screen_x)
            {
                screen_y = screen_x;
            }
            
            barx = (s16)d3;
            sVar12 = xread16(render_context - 0x280);
            
            while (sVar12 < barx)
            {
                xwrite16(render_context - 0x25e, screen_x);
                u16 position_x = 0;

                u16 testpos_x = (s16)(terrainstep_x + unkpos_x) + (u16)CARRY4(terrainstep_x, unkpos_x);
                unkpos_x = CONCAT22((s16)((terrainstep_x + unkpos_x) >> 0x10), testpos_x);
                
                u16 testpos_y = (s16)(terrainstep_y + unkpos_y) + (u16)CARRY4(terrainstep_y,unkpos_y);
                unkpos_y = CONCAT22((s16)((terrainstep_y + unkpos_y) >> 0x10), testpos_y);
                
                xwrite32(render_context - 0x280, xread32(render_context - 0x27c) + xread32(render_context - 0x280));
                if ((u16)xread16(render_context - 0x294) < testpos_x)
                {
                    if (xread8(render_context - 0x3fe) == 1)
                    {
                        screen_x = 0;
                        if (xread16(render_context - 0x294) <= (s16)testpos_x)
                        {
                            screen_x = xread16(render_context - 0x294) * 2;
                        }

                        // TERRAIN DATA ACCESS PATTERN:
                        // terrain_data is a 4-byte-per-entry grid
                        // Access formula: terrain_data + ((grid_x - clamped_x) * 4)
                        // Returns: 32-bit value containing altitude base pointer for this X column
                        iVar10 = xread32(terrain_data + (s16)((screen_x - testpos_x) * 4));
                        position_x = (u16)xread16(render_context - 0x292);  // Grid height dimension
                        if (testpos_y <= position_x)
                        {
                            if ((s16)position_x <= (s16)testpos_y)
                            {
                                // Offset by Y coordinate to reach correct altitude table entry
                                // Stride is 4 bytes per altitude value (s32 aligned)
                                iVar10 += (s32)xread16(render_context - 0x292) << 2;
                            }

                            // Address into altitude table for this grid cell
                            adresa = ((iVar10 - (s16)testpos_y) - (s32)(s16)testpos_y);
                        }
                        else
                        {
                            adresa = (iVar10 + (s16)testpos_y + (s32)(s16)testpos_y);
                        }

                        // Read processed height value (u8 at offset +1 in terrain cell)
                        position_x = (u16)xread8((s32)adresa + 1);
                    }
                }
                else
                {
                    // TODO: is this ok?
                    if (xread8(render_context - 0x3fe) == 1)
                    {
                        if ((u16)xread16(render_context - 0x292) < testpos_y)
                        {
                            // Y coordinate is within grid bounds
                            // Fetch 4-byte altitude base pointer for current X column
                            iVar10 = xread32(terrain_data + (s16)(testpos_x * 4));
                            position_x = (u16)xread16(render_context - 0x292);  // Grid height
                            if ((s16)position_x <= (s16)testpos_y)
                            {
                                // Y exceeds grid height, offset altitude table pointer
                                iVar10 += (s32)xread16(render_context - 0x292) << 2;
                            }

                            // Calculate address into altitude table for grid cell (x, y)
                            adresa = ((iVar10 - (s16)testpos_y) - (s32)(s16)testpos_y);
                        }
                        else
                        {
                            // Y coordinate below grid, use alternative calculation
                            iVar10 = xread32(terrain_data + (s16)(testpos_x * 4));
                            adresa = (iVar10 + (s16)testpos_y + (s32)(s16)testpos_y);
                        }

                        // Extract height value from terrain cell (byte at offset +1)
                        position_x = (u16)xread8((s32)adresa + 1);
                    }
                }
                
//dolanc21:
                
                screen_x = xread16(render_context - 0x26e) + xread16(altitude_table + (s16)(xread16(render_context - 0x25a) + position_x * 2));
                xwrite16(render_context - 0x260, screen_x);
                if (screen_y < screen_x)
                {
                    screen_y = screen_x;
                }
                
                sVar12 = xread16(render_context - 0x280);
            }
            
            position_x = (u16)(tmppos_x >> 16);
            position_y = (u16)(tmppos_y >> 16);
            if ((u16)xread16(render_context - 0x294) < position_x)
            {
                if (xread8(render_context - 0x3fe) == 1)
                {
                    screen_x = 0;
                    if (xread16(render_context - 0x294) <= (s16)position_x)
                    {
                        screen_x = xread16(render_context - 0x294) * 2;
                    }
                    
                    iVar10 = xread32(terrain_data + (s16)((screen_x - position_x) * 4));
                    uVar9 = (u16)xread16(render_context - 0x292);
                    if (position_y <= uVar9)
                    {
                        goto dolanc34;
                    }
                    else
                    {
                        goto dolanc39;
                    }
                }
            }
            else
            {
                if ((u16)xread16(render_context - 0x292) < position_y)
                {
                    if (xread8(render_context - 0x3fe) != 1)
                        goto dobar3;
                    
                    iVar10 = xread32(terrain_data + (s16)(position_x << 2));
                    uVar9 = (u16)xread16(render_context - 0x292);
                    
dolanc34:
                    
                    if ((s16)uVar9 <= (s16)position_y)
                    {
                        iVar10 += (s32)xread16(render_context - 0x292) << 2;
                    }
                    
                    terrain_cell = ((iVar10 - (s16)position_y) - (s32)(s16)position_y);
                }
                else
                {
                    iVar10 = xread32(terrain_data + (s16)(position_x << 2));
                    
dolanc39:
                    
                    terrain_cell = (iVar10 + (s16)position_y + (s32)(s16)position_y);
                }
                
                s16 bary = xread16(render_context - 0x25c);
                uVar9 = xread16(terrain_cell);
                image.solh = uVar9 & 0xff;
                image.solpixy = xread16(render_context - 0x276) + xread16(altitude_table + (s16)(image.solh * 2));
                xwrite16(render_context - 0x246, image.solpixy);
                xwrite16(render_context - 0x25c, image.solpixy);
                if (image.solpixy < bary)
                {
                    xwrite16(render_context - 0x246, bary);
                    bary = image.solpixy;
                }
                
                u16 barheight = screen_y - bary;
                sVar12 = (s16)(tempstep_x >> 0x10);
                
                // Extract terrain type index from height value
                // uVar9 contains: u8 at [0] = height index, u8 at [1] = used during rendering
                // Upper byte contains terrain type flags (0x3f00 mask = 6 bits for type)
                // Offset calculation: (type_bits >> 3) - 0xc00 gives signed terrain index
                s16 index = ((uVar9 & 0x3f00) >> 3) - 0xc00;

                if (barheight != 0 && SBORROW2(screen_y, bary) == (s32)((u32)barheight << 0x10) < 0)
                {
                    u32 test = CONCAT22((tmppos_x), (tmppos_x >> 16));
                    // Render vertical terrain bar using:
                    // - terrain_cell: pointer to terrain cell data in altitude table
                    // - barheight: pixel height to draw
                    // - index: terrain type selector
                    barland(terrain_cell, render_context, sVar12, (s16)(tempstep_y >> 0x10), bary, barheight, index, barx, test, unkpos_x);
                }
                
                if (xread8(render_context + index + 0x14) != 0)
                {
                    if ((s8)xread8(render_context + index + 0x14) < 0)
                    {
                        notopa = 1;
                        s16 uVar7 = (s16)(unkpos_x >> 0x10);
                        iVar10 = CONCAT22(uVar7, prectopa);
                        uVar9 = precbota;
                        if (adresa != 0)
                        {
                            solha = xread16(adresa) & 0xff;
                            s16 index2 = ((xread16(adresa) & 0x3f00) >> 3) - 0xc00;
                            
                            if ((s8)xread8(render_context + 0x14 + index2) < '\0')
                            {
                                screen_y = xread16(render_context + 0x16 + index2);
                                uVar4 = (u16)xread8((s32)adresa + xread32(render_context + 0x10 + index2) + 1);
                                uVar5 = uVar4 - solha;
                                if ((uVar5 != 0 && SBORROW2(uVar4, solha) == (s32)((u32)uVar5 << 0x10) < 0) && ((uVar2 = screen_y - uVar5) != 0 && SBORROW2(screen_y, uVar5) == (s32)((u32)uVar2 << 0x10) < 0))
                                {
                                    uVar5 = uVar4;
                                    if ((s16)(solha + uVar2) < (s16)uVar4)
                                    {
                                        uVar5 = (solha + uVar2) - solha;
                                    }
                                    
                                    screen_x = xread16(render_context - 0x26e) + xread16(altitude_table + (s16)(xread16(render_context - 0x25a) + uVar4 * 2));
                                    uVar5 = xread16(render_context - 0x26e) + xread16(altitude_table + (s16)(xread16(render_context - 0x25a) + (uVar5 - uVar4) * -2));
                                    adresa = 0;
                                    if (fprectopa == 0)
                                    {
                                        fprectopa = 1;
                                        prectopc = 5000;
                                        precbotc = 0xec78;
                                        prectopa = screen_x;
                                        precbota = uVar5;
                                    }
                                    else
                                    {
                                        prectopb = prectopc;
                                        precbotb = precbotc;
                                        if (prectopa < screen_x)
                                        {
                                            iVar10 = CONCAT22(uVar7, screen_x);
                                        }
                                        
                                        screen_y = (s16)iVar10;
                                        if (screen_y < prectopc)
                                        {
                                            iVar10 = CONCAT22((s16)((u32)iVar10 >> 0x10), prectopc);
                                        }
                                        
                                        uVar4 = precbota;
                                        if ((s16)uVar5 < (s16)precbota)
                                        {
                                            uVar4 = uVar5;
                                        }
                                        
                                        uVar9 = uVar4;
                                        if ((s16)precbotc < (s16)uVar4)
                                        {
                                            uVar9 = precbotc;
                                        }
                                        
                                        prectopa = screen_x;
                                        precbota = uVar5;
                                        prectopc = screen_y;
                                        precbotc = uVar4;
                                        
                                        if ((s16)iVar10 < (s16)uVar9)
                                        {
                                            notopa = 0;
                                        }
                                    }
                                    goto dotopa5;
                                }
                            }
                            
                            fprectopa = 0;
                        }
                        
                    dotopa5:
                        
                        terrain_cell = (xread32(render_context + 0x10 + index) + (s32)(s16)position_y + (s32)(s16)position_y + xread32(terrain_data + (s16)(position_x << 2)));
                        fbottom = 0;
                        position_x = xread16(terrain_cell);
                        image.toph = position_x & 0xff;
                        s16 index3 = ((position_x & 0x3f00) >> 3) - 0xc00;
                        position_x = image.toph - image.solh;
                        if ((position_x != 0 && SBORROW2(image.toph, image.solh) == (s32)((u32)position_x << 0x10) < 0) && ((position_y = xread16(render_context + 0x16 + index) - position_x) != 0 && SBORROW2(xread16(render_context + 0x16 + index), position_x) == (s32)((u32)position_y << 0x10) < 0))
                        {
                            fbottom = (s16)(image.solh + position_y) < image.toph;
                            if ((bool)fbottom)
                            {
                                position_x = (image.solh + position_y) - image.solh;
                            }
                            
                            sVar3 = position_x - image.toph;
                            position_x = xread16(render_context - 0x276) + xread16(altitude_table + (s16)(image.toph * 2));
                            screen_y = xread16(render_context - 0x246);
                            image.toppixy = position_x;
                            xwrite16(render_context - 0x246, position_x);
                            position_y = prectopi;
                            prectopi = position_x;
                            if ((s16)position_x < (s16)position_y)
                            {
                                xwrite16(render_context - 0x246, prectopi);
                                //                            prectopi = altitude_index;
                                position_y = prectopi;
                                position_x = prectopi;
                            }
                            
                            prectopi = position_x;
                            if (fbottom != 0)
                            {
                                screen_y = xread16(render_context - 0x276) + xread16(altitude_table + (s16)(sVar3 * -2));
                            }
                            
                            botalt = precboti;
                            bothigh = screen_y;
                            if (precboti < screen_y)
                            {
                                bothigh = precboti;
                                botalt = screen_y;
                            }
                            
                            precboti = screen_y;
                            if (fprectop != 0)
                            {
                                position_x = botalt - position_y;
                                d1 = position_x;
                                if (position_x != 0 && SBORROW2(botalt,position_y) == (s32)((u32)position_x << 0x10) < 0)
                                {
                                    if (notopa == 0)
                                    {
                                        screen_y = (s16)iVar10;
                                        uVar5 = screen_y - position_y;
                                        iVar10 = CONCAT22((s16)((u32)iVar10 >> 0x10),uVar5);
                                        if (uVar5 == 0 || SBORROW2(screen_y, position_y) != (s32)((u32)uVar5 << 0x10) < 0)
                                        {
                                            screen_y = position_y + position_x;
                                            position_x = screen_y - uVar9;
                                            d1 = position_x;
                                            position_y = uVar9;
                                            if (position_x == 0 || SBORROW2(screen_y, uVar9) != (s32)((u32)position_x << 0x10) < 0)
                                                goto dotop4;
                                        }
                                        else if ((s16)position_x <= (s16)(uVar9 - position_y))
                                        {
                                            fbottom = 0;
                                            d1 = uVar5;
                                        }
                                    }
                                    
                                    uVar7 = xread16(render_context - 0x25c);
                                    xwrite16(render_context - 0x25c, prectopi);
                                    u32 test = CONCAT22((tmppos_x), (tmppos_x >> 16));
                                    tbarland(terrain_cell, render_context, sVar12, terrainstep_y, position_y, d1, index3, d3, test, iVar10);
                                    xwrite16(render_context - 0x25c, uVar7);
                                }
                            }
                            
dotop4:

                            fprectop = 1;
                        }
                        
                        goto dobar3;
                    }
                    
                    if ((s8)xread8(render_context + index + 0x14) != 1)
                        goto dobar3;
                    
                    barsprite(render_context, index, position_x, position_y, (s16)unkpos_x);
                }
                
                fprectop = 0;
                fprectopa = 0;
            }
            
dobar3:
            
            image.precx = barx;
            if (image.landclipx2 < barx)
                break;
            
            tmppos_x += tempstep_x;
            tmppos_y += tempstep_y;
            iVar10 = uVar6 + d3;
            d3 = CONCAT22((s16)((u32)iVar10 >> 0x10), (s16)iVar10 + (u16)CARRY4(uVar6,d3));
        }
        while (true);
        
        xwrite32(render_context - 0x280, uVar1);
        tmppos_x = newpos_x;
        tmppos_y = newpos_y;
    }
    while (true);
}

void affiland(s32 scene)
{
    s32 scnaddr = alis.basemain + scene;
    
    image.mapscreen = scnaddr;
    image.wloglarg = xread16(scnaddr + 0x12) + 1;
    image.wlogic = (u8 *)(alis.mem + xread32(xread32(scnaddr + 0x30)));
    image.wlogx1 = xread16(scnaddr + 0xe);
    image.wlogy1 = xread16(scnaddr + 0x10);
    s32 render_context = xread16(scnaddr + 0x42) + xread32(alis.atent + xread16(scnaddr + 0x40));
    image.fenx1 = xread16(scnaddr + 0xe);
    image.landclipy1 = xread16(scnaddr + 0x10);
    image.landclipx2 = xread16(scnaddr + 0x12) + image.fenx1;
    image.feny2 = xread16(scnaddr + 0x14) + image.landclipy1;
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
        if ((xread8(scnaddr) & 0x80) == 0)
        {
            if ((alis.fswitch != 0) && (image.switchland != 0))
            {
//                u8 cVar1 = fremouse;
//                if (-1 < fmouse)
//                {
//                    do {
//                    } while (fremouse2 != 0);
//                    cVar1 = fremouse + 1;
//                    if ((char)(fremouse + 1) != 0)
//                    {
//                        cVar1 = fremouse;
//                    }
//                }
//
//                fremouse = cVar1;
                image.fenlargw = image.wloglarg >> 2;
                image.wloglarg = image.wloglarg >> 1;
                tvtofen();
                image.switchland = 0;
//                if (-1 < fmouse)
//                {
//                    fremouse2 = 1;
//                    if (fmouse != '\x02')
//                    {
//                        mouserase();
//                    }
//
//                    fremouse2 = 0;
//                    fremouse = -1;
//                    if (fmouse == 0)
//                    {
//                        fmouse = -1;
//                    }
//                }
            }
        }
        else
        {
            calctoy(scnaddr, render_context);
            spritland(scnaddr, xread16(scnaddr + 2));
            calclan0(scnaddr, render_context);
            if (((xread8(scnaddr + 1) & 0x40) == 0) && (xread16(scnaddr + 0xa2) == 0))
            {
                clrvga();
            }
            
            doland(scnaddr, render_context);
            vgatofen();
            image.landone = 1;
            image.switchland = 1;
        }
    }
    else
    {
        calclan0(scnaddr, render_context);
        doland(scnaddr, render_context);
    }
    
    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogy1 = image.logy1;
    image.wlogx2 = image.logx2;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;
    image.fenlargw = image.loglarg >> 2;
}

void landtofi(s16 d2w, s16 d3w)
{
    s16 oldcliph = image.cliph;
    s16 oldclipl = image.clipl;
    s16 oldclipy2 = image.clipy2;
    s16 oldclipx2 = image.clipx2;
    s16 oldclipy1 = image.clipy1;
    s16 oldclipx1 = image.clipx1;
    s32 sceneadr = alis.basemain + d3w;
    image.wloglarg = xread16(sceneadr + 0x12) + 1;
    image.wlogic = (u8 *)(alis.mem + xread32(xread32(sceneadr + 0x30)));
    image.wlogx1 = xread16(sceneadr + 0xe);
    image.wlogy1 = xread16(sceneadr + 0x10);
    image.mapscreen = sceneadr;
    if (image.fdoland != 0)
    {
        image.landclipx2 = xread16(sceneadr + 0x12) + xread16(sceneadr + 0xe);
        image.landcliph = (image.clipy2 - image.clipy1) + 1;
        image.clipl = (image.clipx2 - image.clipx1) + 1;
        image.vbarclipx2 = image.clipx2 + 1;
        image.landclipy1 = image.clipy1;
        s32 render_context = xread16(sceneadr + 0x42) + xread32(alis.atent + xread16(sceneadr + 0x40));
        image.cliph = image.landcliph;
        calctoy(sceneadr, render_context);
        d3w = xread16(sceneadr + 2);
        spritland(sceneadr, d3w);
        calclan0(sceneadr, render_context);
        if (((xread8(sceneadr + 1) & 0x40) == 0) && (xread16(sceneadr + 0xa2) == 0))
        {
            clrvga();
        }
        
        doland(sceneadr, render_context);
        image.fdoland = 0;
        image.landone = 1;
    }
    
    image.clipx1 = oldclipx1;
    image.clipy1 = oldclipy1;
    image.clipx2 = oldclipx2;
    image.clipy2 = oldclipy2;
    image.clipl = oldclipl;
    image.cliph = oldcliph;
    vgatofen();
    image.wlogic = image.logic;
    image.wlogx1 = image.logx1;
    image.wlogy1 = image.logy1;
    image.wlogx2 = image.logx2;
    image.wlogy2 = image.logy2;
    image.wloglarg = image.loglarg;
}

void pixtoland(s32 render_context)
{
    image.cxtstpix = xread16(render_context - 0x268) + ((s16)(((s32)(s16)((s32)(s16)(image.xtstpix - xread16(render_context - 0x26c)) * (s32)xread16(render_context - 0x28a) >> 9) * (s32)(s16)(xread16(render_context - 0x3ac) + image.dtstpix)) / (s32)xread16(render_context - 0x3aa)) - (s16)((s32)image.dtstpix * (s32)xread16(render_context - 0x28c) >> 9));
    image.cytstpix = xread16(render_context - 0x266) + (s16)((s32)image.dtstpix * (s32)xread16(render_context - 0x28a) >> 9) + (s16)(((s32)(s16)((s32)(s16)(image.xtstpix - xread16(render_context - 0x26c)) * (s32)xread16(render_context - 0x28c) >> 9) * (s32)(s16)(xread16(render_context - 0x3ac) + image.dtstpix)) / (s32)xread16(render_context - 0x3aa));
    image.cztstpix = xread16(render_context - 0x264) + (s16)((s32)image.dtstpix * (s32)xread16(render_context - 0x290) >> 9) + (s16)(((s32)(s16)(xread16(render_context - 0x26a) - image.ytstpix) * (s32)(s16)(xread16(render_context - 0x3a8) + image.dtstpix)) / (s32)xread16(render_context - 0x3a6));
}

void pointpix(s16 x, s16 y)
{
    u32 scene = xread16(alis.script->vram_org - 0x16);
    u32 sceneptr = alis.basemain + scene;
    image.dtstpix = xread16(sceneptr + 0x94);
    image.etstpix = -1;
    image.ntstpix = -1;
    image.ftstpix = 1;
    image.xtstpix = x;
    image.ytstpix = y;
    
    if ((xread8(sceneptr) & 0x40) == 0)
    {
        affiland(scene);
        if (-1 < image.etstpix)
        {
            u32 piVar1 = (xread32(xread32(xread32(alis.atent + image.etstpix) - 0x14) + 0xe) + xread32(xread32(alis.atent + image.etstpix) - 0x14));
            image.ntstpix = (s16)(((image.antstpix & 0xffffff) - (xread32(piVar1) + piVar1)) >> 2);
            if (xread16(piVar1 + 4) <= image.ntstpix)
            {
                image.ntstpix = (s16)(((image.antstpix & 0xffffff) - (xread32(alis.basemain + xread32(alis.basemain + 0xe)) + (s32)(alis.basemain + xread32(alis.basemain + 0xe)))) >> 2);
            }
        }
    }
    
    s32 render_context = xread16(sceneptr + 0x42) + xread32(alis.atent + xread16(sceneptr + 0x40));
    pixtoland(render_context);
    image.ftstpix = 0;
}
