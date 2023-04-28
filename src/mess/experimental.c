//
//  experimental.c
//  alis
//
//  Created by Vadim Kindl on 12.01.2023.
//  Copyright © 2023 Zlot. All rights reserved.
//

#include "experimental.h"

#include <stdbool.h>

#include "alis.h"
#include "alis_private.h"

#include "utils.h"

// NOTE: Mostly cut and paste code from ghidra I'm trying to make sense at the moment

#ifndef max
# define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
# define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define FRAME_TICKS (1000000 / 10) // 10 fps looks about right in logo animation

#define BASEMNMEM_PTR alis.mem + alis.basemain
#define SPRITEMEM_PTR alis.spritemem + alis.basesprite

#define SPRITE_VAR(x) (x ? (SpriteVariables *)(SPRITEMEM_PTR + x) : NULL)
#define SCENE_VAR(x) ((SceneVariables *)(BASEMNMEM_PTR + x))
#define ELEMIDX(x) ((((x - 0x78) / 0x30) * 0x28) + 0x8078) // return comparable number to what we see in ST debugger

#define DBTRACE(f, ...) printf("%s " f, __FUNCTION__,  ## __VA_ARGS__); // __PRETTY_FUNCTION__

#define DEBUG_ELEM 0
#define DEBUG_LINK 0
#define DEBUG_CLIP 0
#define DEBUG_DRAW 0
#define DEBUG_FCE  0

#if DEBUG_ELEM > 0
# define VERIFYINTEGRITY verifyintegrity()
# define ELEM_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#else
# define VERIFYINTEGRITY
# define ELEM_TRACE(x, ...)
#endif

#if DEBUG_LINK > 0
# define LINK_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#else
# define LINK_TRACE(x, ...)
#endif

#if DEBUG_CLIP > 0
# define CLIP_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#else
# define CLIP_TRACE(x, ...)
#endif

#if DEBUG_DRAW > 0
# define DRAW_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#else
# define DRAW_TRACE(x, ...)
#endif

#if DEBUG_FCE > 0
# define DEBUGFCE DBTRACE("\n")
#else
# define DEBUGFCE
#endif

u8 tvmode = 0;

u8 tpalet[1024];
u8 mpalet[1024];
u8 dpalet[1024];

u8 *atpalet = tpalet;
u8 *ampalet = mpalet;
u8 *dkpalet = 0;

u8 palc = 0;
// u8 pald = 0;
u8 palt = 0;
u8 palt0 = 0;
u8 ftopal = 0;
u8 fdarkpal = 0;
u8 thepalet = 0;
u8 defpalet = 0;

u16 *ptabfen = 0;
u16 tabfen[640];

s32 bufrvb;

s32 mousflag;

s16 oldcx = 0;
s16 oldcy = 0;
s16 oldcz = 0;

s16 oldacx;
s16 oldacy;
s16 oldacz;

s8 nmode = 0;
s8 falcon = 0;
s8 a1200 = 0;
s8 amiga = 0;
s8 atari = 0;
s8 fsound = 1;
s8 fmusic = 1;

s8 *savsp = 0;
u32 command = 0;
u32 debram = 0;
u32 finram = 0;
u32 savessp = 0;

u8 *bufpack = 0; // 0x20a00;
u32 mworkbuff = 0x20a00;
u32 adtmono = 0x33700;
u32 finnoyau = 0x33900;
u32 basemem = 0x33900;
u32 basevar = 0;
u32 ophysic = 0x33900;

u8 timing = 0;
u16 ptscreen = 0;
u16 vprotect = 0;
s8 fmouse = 0xff;
u16 fonum = 0xffff;
u8 sback = 0;

u32 adaline = 0;

u32 newad;
s16 newx;
s16 newy;
s16 newd;
s16 newf;
s16 newh;
s16 newl;
s16 newzoomx;
s16 newzoomy;
s16 blocx1;
s16 blocy1;
s16 blocx2;
s16 blocy2;
u8 joints;
s16 fenx1;
s16 feny1;
s16 fenx2;
s16 feny2;
s16 fenlargw;
u8* adfenintv;
u8 sback;
u8 wback;
u8 cback;
u8 pback;
u16 backprof;
s16 clipx1;
s16 clipy1;
s16 clipx2;
s16 clipy2;
s16 clipl;
s16 cliph;
u8 fclip;

u8 buffer0[320*200];
u8 buffer1[320*200];

u8 *physic = buffer0;
u8 *logic = buffer1;
u8 fitroutine = 0;
u8 fphysic = 0;
u8 fphytolog = 0;
u8 fremouse = 0;
u8 fremap = 0;
u8 fremouse2 = 0;
u8 vtiming = 0;
s16 savmouse[2];
u32 savmouse2[8];
u8 switchgo;
u16 backlarg;
u8 *wlogic;
s16 wlogx1;
s16 wlogx2;
s16 wlogy1;
s16 wlogy2;
u16 wloglarg;
s32 logx1 = 0;          // originx 0x0
s32 logx2 = 0x13f; // 0x013f00c7; // dimensions 319x199
s16 logy1 = 0;
s16 logy2 = 0xc7;
u16 loglarg = 0xa0; // 0x50 for st
u8 insid = 0;
u8 spag = 0;
u8 wpag = 0;
u8 *backmap;
s32 backx1;
s32 backx2;
s16 backy1;
s16 backy2;
u32 timeclock = 0;

// to be consistent with old code
#pragma pack(1)

typedef union {
    
    struct {
        
        u8 state;               // 0x00
        u8 numelem;             // 0x01
        u16 screen_id;          // 0x02
        u16 to_next;            // 0x04
        u16 link;               // 0x06
        u32 newad:24;           // 0x08
        u8 newf:8;              // 0x08 + 3
        s16 newx;               // 0x0c
        s16 newy;               // 0x0e
        s16 newd;               // 0x10
        u32 data:24;            // 0x12
        u8 flaginvx:8;          // 0x12 + 3 // +0 on big endian machines
        s16 depx;               // 0x16
        s16 depy;               // 0x18
        s16 depz;               // 0x1a
        u8 credon_off;          // 0x1c
        u8 creducing;           // 0x1d
        u16 clinking;           // 0x1e
        u8 cordspr;             // 0x20
        u8 chsprite;            // 0x21
        u16 cnoise;             // 0x22
        u32 sprite_0x28;        // 0x24
        u16 width;              // 0x28
        u16 height;             // 0x2a
        u16 newzoomx;           // 0x2c
        u16 newzoomy;           // 0x2e
    };
    
    u8 variables[0x30];
    
} SpriteVariables;

typedef union {
    
    struct {
        
        u8 state;               // 0x0
        u8 numelem;             // 0x1
        u16 screen_id;          // 0x2
        u16 to_next;            // 0x4
        u16 link;               // 0x6
        u16 unknown0x08;        // 0x8
        u16 unknown0x0a;        // 0xa
        u16 unknown0x0c;        // 0xc
        s16 newx;               // 0xe
        s16 newy;               // 0x10
        u16 width;              // 0x12
        u16 height;             // 0x14
        u16 depx;               // 0x16
        u16 depy;               // 0x18
        u16 depz;               // 0x1a
        u8 credon_off;          // 0x1c
        u8 creducing;           // 0x1d
        u16 clinking;           // 0x1e

        u8 unknown0x20;
        u8 unknown0x21;
        u8 unknown0x22;
        u8 unknown0x23;
        u8 unknown0x24;
        u8 unknown0x25;
        
        u8 unknown0x26;
        u8 unknown0x27;
        u8 unknown0x28;

        u8 unknown0x29;

        u16 unknown0x2a;
        u16 unknown0x2c;
        u16 unknown0x2e;
    };
    
    u8 variables[0x30];
    
} SceneVariables;

#pragma pack(0)

s32 adresdei(s32 idx)
{
    u8 *addr = alis.mem + alis.main->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform.is_little_endian);
    s32 e = read16(addr + l + 4, alis.platform.is_little_endian);
    if (e > idx)
    {
        s32 a = read32(addr + l, alis.platform.is_little_endian) + l + idx * 4;
        return read32(addr + a, alis.platform.is_little_endian) + a;
    }

    return 0xf;
}

s32 adresdes(s32 idx)
{
    if (alis.flagmain != 0)
        return adresdei(idx);
    
    u8 *addr = alis.mem + alis.script->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform.is_little_endian);
    s32 e = read16(addr + l + 4, alis.platform.is_little_endian);
    if (e > idx)
    {
        s32 a = read32(addr + l, alis.platform.is_little_endian) + l + idx * 4;
        return read32(addr + a, alis.platform.is_little_endian) + a;
    }

    return 0xf;
}

void adresform(s16 idx)
{
    u8 *addr = alis.mem + alis.script->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform.is_little_endian);
    if (idx - read16(addr + l + 10, alis.platform.is_little_endian) < 0)
    {
        
    }
}

s32 adresmui(s32 idx)
{
    u8 *addr = alis.mem + alis.main->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform.is_little_endian);
    s32 e = read16(addr + 0x10 + l, alis.platform.is_little_endian);
    if (e > idx)
    {
        s32 a = read32(addr + 0xc + l, alis.platform.is_little_endian) + l + idx * 4;
        return read32(addr + a, alis.platform.is_little_endian) + a;
    }

    return 0x11;
}

s32 adresmus(s32 idx)
{
    if (alis.flagmain != 0)
        return adresmui(idx);

    u8 *addr = alis.mem + alis.script->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform.is_little_endian);
    s32 e = read16(addr + 0x10 + l, alis.platform.is_little_endian);
    if (e > idx)
    {
        s32 a = read32(addr + 0xc + l, alis.platform.is_little_endian) + l + idx * 4;
        return read32(addr + a, alis.platform.is_little_endian) + a;
    }

    return 0x11;
}

void tdarkpal(u8 *paldata)
{
    // TODO: ...
//    u32 color;
//    u32 *in_A1;
//    s8 *pbVar4;
//    u16 *puVar6;
//    u16 uVar2;
//
//    u16 *puVar5 = dkpalet + (((s16)in_A1 - (s16)atpalet) >> 2) * 3;
//
//    do
//    {
//        color = *paldata;
//        paldata += 3;
//        bufrvb = color & 0xffffff00;
//        if ((bufrvb == 0) && (((0x100 < *puVar5 || (0x100 < puVar5[1])) || (0x100 < puVar5[2]))))
//        {
//            bufrvb = 0x1010001;
//        }
//
//        color = (bufrvb >> 0x18) * (u32)*puVar5;
//        if (0xffff < color)
//        {
//            color = 0xffff;
//        }
//
//        in_A1[0] = (s8)(color >> 9) & 0x7e;
//        puVar6 = puVar5 + 2;
//        color = (bufrvb >> 0x10 & 0xff) * (u32)puVar5[1];
//        if (0xffff < color) {
//            color = 0xffff;
//        }
//        in_A1[1] = (s8)(color >> 9) & 0x7e;
//        pbVar4 = in_A1 + 3;
//        in_A1[2] = 0;
//        puVar5 = puVar5 + 3;
//        color = (bufrvb >> 8 & 0xff) * (u32)*puVar6;
//        if (0xffff < color) {
//            color = 0xffff;
//        }
//
//        in_A1 += 4;
//        *pbVar4 = (s8)(color >> 9) & 0x7e;
//        uVar2 --;
//    }
//    while (uVar2 != 0xffff);
}

void topalet(void)
{
    ftopal = 0;

    if (palc == 1)
    {
        memcpy(mpalet, tpalet, 1024);
        ftopal = 1;
        return;
    }
    
    for (s32 i = 0; i < 768; i++)
    {
        if (mpalet[i] == tpalet[i])
        {
            continue;
        }
        
        if (mpalet[i] < tpalet[i])
        {
            mpalet[i] += dpalet[i];
            if (mpalet[i] + dpalet[i] > tpalet[i])
                mpalet[i] = tpalet[i];
        }
        else if (mpalet[i] > tpalet[i])
        {
            mpalet[i] -= dpalet[i];
            if (mpalet[i] - dpalet[i] < tpalet[i])
                mpalet[i] = tpalet[i];
        }
    }
    
    ftopal = 1;
}

void ctopalette(u8 *paldata, s32 duration)
{
    u16 colors = paldata[1];
    if (colors == 0) // ST palette
    {
        if (thepalet != 0)
        {
            thepalet = 0;
            defpalet = 0;
            return;
        }
        
        palc = 0;
        u8 *palptr = &paldata[2];

        u16 to = 0;
        for (s32 i = 0; i < 16; i++)
        {
            u8 r = palptr[i * 2 + 0];
            r = (r & 0b00000111) << 5;
            u8 g = palptr[i * 2 + 1];
            g = (g >> 4) << 5;
            u8 b = palptr[i * 2 + 1];
            b = (b & 0b00000111) << 5;
            
            atpalet[to++] = r;
            atpalet[to++] = g;
            atpalet[to++] = b;
        }
    }
    else // 8 bit palette
    {
        palc = 0;
        u16 offset = paldata[2];
        if (fdarkpal == 0)
        {
            memcpy(atpalet + (offset * 3), paldata + 4, colors * 3);
        }
        else
        {
            tdarkpal(paldata);
        }
    }
    
    thepalet = 0;
    defpalet = 0;
    
    if (duration != 0)
    {
        for (s32 i = 0; i < 256; i++)
        {
            dpalet[i] = abs(tpalet[i] - mpalet[i]) / duration;
        }

        topalet();
        palt = 1;
        palt0 = 1;
        palc = duration;
    }
    else
    {
        memcpy(mpalet, atpalet, 256 * 3);
        ftopal = 0xff;
    }
}

void OPCODE_CTOPALET_0xbe(s32 palidx, s32 duration)
{
    alis.flagmain = 0;
    if ((((nmode == '\x01') || (nmode == '\x05')) || (nmode == '\x03')) || (nmode == '\a'))
    {
        return;
    }
    
    if (palidx < 0)
    {
        // TODO: ...
//    u8 bVar1;
//    u32 in_D0;
//    u16 uVar3;
//    u32 uVar2;
//    u8 *pbVar4;
//    u8 *pbVar5;
//    u32 *puVar6;
//    u32 *puVar7;
//    u8 *puVar8;
//    s8 *pcVar9;
//    u32 *puVar10;
//    u32 *puVar11;
//    u16 *pwVar12;
//    u16 *puVar13;
//        puVar8 = svpalet;
//        if (palidx != -1)
//        {
//            puVar8 = svpalet2;
//        }
//
//        palidx = 0xff;
//        puVar7 = atpalet;
//        if (fdarkpal == '\0')
//        {
//            do {
//                *puVar7 = *(u32 *)puVar8;
//                palidx = palidx + -1;
//                puVar8 = (u8 *)((s32)puVar8 + 4);
//                puVar7 = puVar7 + 1;
//            } while (palidx != -1);
//        }
//        else
//        {
//            pwVar12 = dkpalet;
//            do
//            {
//                uVar2 = (u32)(u8)*puVar8 * (u32)*pwVar12;
//                if (0x7fff < uVar2)
//                {
//                    uVar2 = 0x7fff;
//                }
//                *(u8 *)puVar7 = (u8)(uVar2 >> 8) & 0xfe;
//                puVar13 = pwVar12 + 2;
//                uVar2 = (u32)*(u8 *)((s32)puVar8 + 1) * (u32)pwVar12[1];
//                if (0x7fff < uVar2)
//                {
//                    uVar2 = 0x7fff;
//                }
//                *(u8 *)((s32)puVar7 + 1) = (u8)(uVar2 >> 8) & 0xfe;
//                pbVar4 = (u8 *)((s32)puVar8 + 3);
//                pbVar5 = (u8 *)((s32)puVar7 + 3);
//                *(u8 *)((s32)puVar7 + 2) = 0;
//                puVar8 = (u8 *)((s32)puVar8 + 4);
//                pwVar12 = pwVar12 + 3;
//                uVar2 = (u32)*pbVar4 * (u32)*puVar13;
//                if (0x7fff < uVar2)
//                {
//                    uVar2 = 0x7fff;
//                }
//                puVar7 = puVar7 + 1;
//                *pbVar5 = (u8)(uVar2 >> 8) & 0xfe;
//                palidx = palidx + -1;
//            }
//            while (palidx != -1);
//        }
    }
    else
    {
        s32 addr = adresdes(palidx);
        u8 *paldata = alis.mem + alis.script->data_org + addr;
        ctopalette(paldata, duration);
    }
}

void ctoblackpal(s16 duration)
{
    if ((((nmode == '\x01') || (nmode == '\x05')) || (nmode == '\x03')) || (nmode == '\a'))
    {
        return;
    }
    
    palc = 0;
    
    memset(atpalet, 0, 256 * 4);
    
    thepalet = 0;
    defpalet = 0;
    if (duration == 0)
    {
        memset(ampalet, 0, 256 * 4);
        ftopal = 0xff;
    }
    else
    {
        for (s32 i = 0; i < 256; i++)
        {
            dpalet[i] = abs(tpalet[i] - mpalet[i]) / duration;
        }

        topalet();
        palt = 1;
        palt0 = 1;
        palc = duration;
    }
}

void printelem(void)
{
    s32 cursprit = alis.debsprit;
    cursprit += 0x78;
    cursprit += 0x60;

    do
    {
        ELEM_TRACE("0x%.4x\n%.2x%.2x %.4x\n%.4x %.4x\n", ELEMIDX(cursprit), *(u8 *)(SPRITEMEM_PTR + cursprit + 0), *(u8 *)(SPRITEMEM_PTR + cursprit + 1), *(u16 *)(SPRITEMEM_PTR + cursprit + 2),
                   *(u16 *)(SPRITEMEM_PTR + cursprit + 4) == 0 ? 0 : ELEMIDX(*(u16 *)(SPRITEMEM_PTR + cursprit + 4)),
                   *(u16 *)(SPRITEMEM_PTR + cursprit + 6) == 0 ? 0 : ELEMIDX(*(u16 *)(SPRITEMEM_PTR + cursprit + 6)));
        cursprit += 0x30;
    }
    while (cursprit < alis.debsprit + 16 * 0x30);
}

u8 verifyintegrity(void)
{
    DEBUGFCE;
    
    u8 result = true;

    if (alis.libsprit == 0)
    {
        ELEM_TRACE("ERROR: alis.libsprit = 0!\n");
        result = false;
    }
    
    SpriteVariables *cursprvar = NULL;
    
    ELEM_TRACE("  list\n");

    u16 previdx = 0;
    u16 curidx = alis.script->context._0x18_unknown;
    while (curidx != 0)
    {
        cursprvar = SPRITE_VAR(curidx);
        
        u16 screen_id = cursprvar->screen_id;
        u16 state = cursprvar->state;
        u16 link = cursprvar->link;
        u32 clink = cursprvar->clinking;

        ELEM_TRACE("    N: 0x%.4x L: 0x%.4x Sc: 0x%.4x St: 0x%.2x Cl: 0x%.8x\n", ELEMIDX(curidx), ELEMIDX(link), ELEMIDX(screen_id), state, clink);

        if (link == curidx)
        {
            ELEM_TRACE("ERROR: link = 0\n");
            result = false;
        }
        
        if (link)
        {
            u16 link2 = SPRITE_VAR(link)->link;
            
            if (link2 == curidx)
            {
                ELEM_TRACE("ERROR: infinite loop\n");
                result = false;
                cursprvar->link = 0;
            }
        }

        previdx = curidx;
        curidx = cursprvar->to_next;
    }

    u16 scsprite = ptscreen;
    
    while (scsprite != 0)
    {
        SceneVariables *scene = SCENE_VAR(scsprite);
        if ((scene->state & 0x40) == 0)
        {
            ELEM_TRACE("  screen [0x%.4x]\n", ELEMIDX(scsprite));

            for (curidx = scene->screen_id; curidx != 0; curidx = SPRITE_VAR(curidx)->link)
                ELEM_TRACE("    0x%.4x\n", ELEMIDX(curidx));
        }

        scsprite = scene->to_next;
    }

    if (!result)
    {
        ELEM_TRACE("INTEGRITY COMPROMISED!\n");
    }
 
    return result;
}

void inisprit(void)
{
    // alis.basemain = 0;
    alis.debsprit = 0;
    alis.finsprit = 0x7fff;

    s32 cursprit = alis.debsprit;
    alis.tvsprite = 0x8000;
    alis.basesprite = alis.debsprit + 0x8000;
    *(s32 *)(alis.spritemem + cursprit + 0x0c) = 0;
    *(s16 *)(alis.spritemem + cursprit + 0x16) = 0x13f;
    *(s16 *)(alis.spritemem + cursprit + 0x18) = 199;
    *(s8 *) (alis.spritemem + cursprit + 0x29) = 0;
    alis.backsprite = 0x000e; // 0x8028;
    alis.texsprite = 0x0042; // 0x8050;
    *(s32 *)(alis.spritemem + cursprit + 0x5c) = 0;
    *(s8 *) (alis.spritemem + cursprit + 0x50) = 0xfe;
    alis.mousflag = 0;
    alis.libsprit = 0x78; // 0x8078;
    cursprit += 0x78;
    
//    s16 sprit = 0x8078;
//
//    do
//    {
//        do
//        {
//            sprit += 0x28;
//        }
//        while (sprit == 0);
//
//        *(s16 *)(alis.spritemem + cursprit + 4) = sprit;
//        cursprit = alis.basesprite + sVar1;
//    }
//    while (cursprit < alis.finsprit);

    do
    {
        SpriteVariables *sprite = SPRITE_VAR(cursprit);
        sprite->to_next = cursprit + 0x30;
        cursprit += 0x30;
    }
    while (cursprit < alis.finsprit);
    
    *(s16 *)(SPRITEMEM_PTR + cursprit + -0x24) = 0;
}

u8 searchelem(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;

    u16 screen_id;
    s8 num;
    
    *previdx = 0;
    *curidx = alis.script->context._0x18_unknown;
    
    u16 dbgidx = ELEMIDX(*curidx);
    if (*curidx == 0)
    {
        ELEM_TRACE("cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
        return 0;
    }
    
    SpriteVariables *cursprvar = NULL;
    
    do
    {
        cursprvar = SPRITE_VAR(*curidx);
        dbgidx = ELEMIDX(*curidx);
        screen_id = cursprvar->screen_id;
        if (screen_id <= alis.script->context._0x16_screen_id)
        {
            if (alis.script->context._0x16_screen_id != screen_id)
            {
                ELEM_TRACE("cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
                return 0;
            }

            num = cursprvar->numelem;
            if (num <= alis.numelem)
            {
                if (alis.numelem != num)
                {
                    ELEM_TRACE("cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
                    return 0;
                }

                ELEM_TRACE("cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
                return 1;
            }
        }

        *previdx = *curidx;
        *curidx = cursprvar->to_next;
    }
    while (*curidx != 0);

    ELEM_TRACE("cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
    return 0;
}

u8 testnum(u16 *curidx)
{
    DEBUGFCE;
    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);
    return ((cursprvar->screen_id == alis.script->context._0x16_screen_id) && (cursprvar->numelem == alis.numelem)) ? 1 : 0;
}

u8 nextnum(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;

    *previdx = *curidx;
    *curidx = SPRITE_VAR(*curidx)->to_next;
    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);
    if (cursprvar != NULL && cursprvar->screen_id == alis.script->context._0x16_screen_id && cursprvar->numelem == alis.numelem)
    {
        ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
        return 1;
    }

    ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
    return 0;
}

void createlem(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;

    SpriteVariables *cursprvar = SPRITE_VAR(alis.libsprit);

    if (alis.libsprit != 0)
    {
        u16 sprit = alis.libsprit;
        u16 nextsprit = cursprvar->to_next;

        if (*previdx == 0)
        {
            alis.script->context._0x18_unknown = alis.libsprit;
            alis.libsprit = nextsprit;
        }
        else
        {
            SpriteVariables *prevsprvar = SPRITE_VAR(*previdx);
            prevsprvar->to_next = alis.libsprit;
            alis.libsprit = nextsprit;
        }

        s16 scrnidx = SCENE_VAR(alis.script->context._0x16_screen_id)->screen_id;
        SpriteVariables *scrnsprvar = SPRITE_VAR(scrnidx);

        cursprvar->state = 0xff;
        cursprvar->numelem = alis.numelem;
        cursprvar->screen_id = alis.script->context._0x16_screen_id;
        cursprvar->to_next = *curidx;
        cursprvar->link = scrnsprvar->link;
        scrnsprvar->link = sprit;

        *curidx = sprit;
    }

//    VERIFYINTEGRITY;
    ELEM_TRACE(" cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);
}

void delprec(s16 elemidx)
{
    DEBUGFCE;

    SpriteVariables *cursprvar = SPRITE_VAR(elemidx);
    s16 scridx = cursprvar->screen_id;

    SceneVariables *scrsprvar = SCENE_VAR(scridx);
    if (elemidx == scrsprvar->screen_id)
    {
        scrsprvar->screen_id = cursprvar->screen_id;;
        return;
    }
    
    u16 spridx = scrsprvar->screen_id;
    SpriteVariables *tmpsprvar = SPRITE_VAR(spridx);
    while (tmpsprvar != NULL && elemidx != tmpsprvar->link)
    {
        spridx = tmpsprvar->link;
        tmpsprvar = SPRITE_VAR(spridx);
    }
    
    tmpsprvar->link = cursprvar->link;
}

void killelem(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;
    ELEM_TRACE("  cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), alis.numelem);

    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);
    if (alis.ferase == '\0' && -1 < (s8)cursprvar->state)
    {
        cursprvar->state = 1;
        
        if (*previdx == 0)
        {
            alis.script->context._0x18_unknown = cursprvar->to_next;
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
            alis.script->context._0x18_unknown = cursprvar->to_next;
        }
        else
        {
            SPRITE_VAR(*previdx)->to_next = cursprvar->to_next;
        }
        
        cursprvar->to_next = alis.libsprit;
        alis.libsprit = *curidx;
        
        delprec(*curidx);
    }

    if (*previdx == 0)
    {
        *curidx = alis.script->context._0x18_unknown;
//            VERIFYINTEGRITY;
        return;
    }
    
    *curidx = SPRITE_VAR(*previdx)->to_next;
//    VERIFYINTEGRITY;
}

void put(u8 idx)
{
    DEBUGFCE;

    alis.fmuldes = 0;
    ELEM_TRACE("\n");
    ELEM_TRACE("   idx: 0x%.2x numelem: 0x%.2x\n", idx, alis.numelem);
    putin(idx);
}

void putin(u8 idx)
{
    DEBUGFCE;
    SpriteVariables *debug = 0;

    s32 addr = adresdes(idx);
    u8 *resourcedata = alis.mem + alis.script->data_org + addr;

    u16 x = alis.depx;
    u16 z = alis.depz;
    u16 y = alis.depy;
    
    if (resourcedata[0] > 0x80)
    {
        if (resourcedata[0] == 0xfe)
        {
            // TODO: handle palette
            return;
        }
        
        // handle composit images
     
        u16 rsrccount = resourcedata[1];
        u8 *currsrc = resourcedata + 2;
        
        u8 invx;
        u8 muldes;

        for (s32 i = 0; i < rsrccount; i++)
        {
            invx = alis.flaginvx;
            muldes = alis.fmuldes;
            
            s16 curelem = read16(currsrc + 0, alis.platform.is_little_endian);
            s16 curdepx = read16(currsrc + 2, alis.platform.is_little_endian);
            if (*((u8 *)(&alis.flaginvx)) != 0)
                curdepx = -curdepx;

            alis.depx += curdepx;

            s16 curdepy = read16(currsrc + 4, alis.platform.is_little_endian);
            alis.depy = curdepy + y;

            s16 curdepz = read16(currsrc + 6, alis.platform.is_little_endian);
            alis.depz = curdepz + z;
            
            if (curelem < 0)
                *((u8 *)(&alis.flaginvx)) ^= 1;

            alis.fmuldes = 1;

            putin(curelem);

            alis.depx = x;
            alis.depy = y;
            alis.depz = z;
            
            alis.flaginvx = invx;
            alis.fmuldes = muldes;

            currsrc += 8;
        }
        
        alis.fmuldes = 0;
    }
    else
    {
        ELEM_TRACE("     idx: 0x%.2x numelem: 0x%.2x\n", idx, alis.numelem);

        u16 cursprite = 0;
        u16 prevsprite = 0;
        
//        printelem();
        
        if (alis.fadddes == 0)
        {
            if (alis.fmuldes)
                goto put13;

            u8 ret = searchelem(&cursprite, &prevsprite);
            if (ret == 0)
                createlem(&cursprite, &prevsprite);

            if (-1 < (s8)SPRITE_VAR(cursprite)->state)
            {
put30:
                SPRITE_VAR(cursprite)->state = 2;
            }
        }
        else
        {
put13:
            {
                u8 ret = searchelem(&cursprite, &prevsprite);
                if (ret)
                {
                    do
                    {
                        if (SPRITE_VAR(cursprite)->state == 0)
                            goto put30;
                        
                        ret = nextnum(&cursprite, &prevsprite);
                    }
                    while (ret);
                }
                
                createlem(&cursprite, &prevsprite);
            }
        }
        
        addr = adresdes(idx);

        ELEM_TRACE("  cursprite: 0x%.4x addr: 0x%.4x\n", ELEMIDX(cursprite), ELEMIDX(addr));

        SpriteVariables *cursprvar = SPRITE_VAR(cursprite);
        cursprvar->data       = alis.script->data_org + addr;
        cursprvar->flaginvx   = (u8)alis.flaginvx;
        cursprvar->depx       = oldcx + alis.depx;
        cursprvar->depy       = oldcy + alis.depy;
        cursprvar->depz       = oldcz + alis.depz;
        cursprvar->credon_off = alis.script->context._0x25_credon_credoff;
        cursprvar->creducing  = alis.script->context._0x27_creducing;
        cursprvar->clinking   = alis.script->context._0x2a_clinking;
        cursprvar->cordspr    = alis.script->context._0x2b_cordspr;
        cursprvar->chsprite   = alis.script->context._0x2f_chsprite;
        cursprvar->cnoise     = alis.script->context._0xe_czap_cexplode_cnoise;
//        cursprvar->sprite_0x28   = *(u16 *)(unaff_A6 + sprite_newl);
        if (*(s8 *)(alis.mem + alis.script->context._0x25_credon_credoff) == -0x80)
        {
            cursprvar->creducing = 0;
            // NOTE: falcon specific?
//            cursprvar->newzoomx = *(u16 *)(unaff_A6 - 0x36);
//            cursprvar->newzoomy = *(u16 *)(unaff_A6 - 0x38);
        }
        else
        {
            cursprvar->credon_off = alis.script->context._0x26_creducing;
            cursprvar->creducing  = alis.script->context._0x27_creducing;
        }
        debug = SPRITE_VAR(cursprite);
    }
    
    if (alis.fmuldes == 0)
    {
        u16 cursprite = 0;
        u16 prevsprite = 0;

        u8 res = searchelem(&cursprite, &prevsprite);
        if (res != 0)
        {
            return;
        }

        do
        {
            while (SPRITE_VAR(cursprite)->state == 0)
            {
                killelem(&cursprite, &prevsprite);
                if (cursprite == 0)
                {
                    alis.fadddes = 0;
                    return;
                }

                res = testnum(&cursprite);
                if (!res)
                {
                    alis.fadddes = 0;
                    return;
                }
            }

            res = nextnum(&cursprite, &prevsprite);
        }
        while (res);
    }
    
    alis.fadddes = 0;
//    VERIFYINTEGRITY;
    
//    printelem();
}

s16 inilink(s16 elemidx)
{
    DEBUGFCE;

    blocx1 = 0x7fff;
    blocy1 = 0x7fff;
    blocx2 = 0x8000;
    blocy2 = 0x8000;
    return SPRITE_VAR(elemidx)->clinking;
}

void calcfen(s16 elemidx1, s16 elemidx3)
{
    DEBUGFCE;

    if (joints == '\0')
    {
        SpriteVariables *idx1sprvar = SPRITE_VAR(elemidx1);
        if (idx1sprvar->newd < 0)
            return;

        blocx1 = idx1sprvar->newx;
        blocy1 = idx1sprvar->newy;
        blocx2 = blocx1 + idx1sprvar->width;
        blocy2 = blocy1 + idx1sprvar->height;
    }
    
    SpriteVariables *idx3sprvar = SPRITE_VAR(elemidx3);
    s16 tmpx = idx3sprvar->depx;
    s16 tmpy = idx3sprvar->depy;
    
    if (blocx1 <= tmpx && blocy1 <= tmpy && idx3sprvar->newx <= blocx2 && idx3sprvar->newy <= blocy2)
    {
        fenx1 = blocx1;
        if (blocx1 < idx3sprvar->newx)
            fenx1 = idx3sprvar->newx;
        
        feny1 = blocy1;
        if ((s16)blocy1 < idx3sprvar->newy)
            feny1 = idx3sprvar->newy;
        
        fenx2 = blocx2;
        if (tmpx < blocx2)
            fenx2 = tmpx;
        
        feny2 = blocy2;
        if (tmpy < blocy2)
            feny2 = tmpy;
     
        // NOTE: just for bitplane graphics
//        fenx1 &= 0xfff0;
//        fenx2 |= 0xf;

        fenlargw = (u16)((fenx2 - fenx1) + 1) >> 2;
        
        adfenintv = (u32)feny1 * 0x140 + physic + (s32)(fenx1 >> 1);
    }
    
    LINK_TRACE("0x%.4x 0x%.4x\n", ELEMIDX(elemidx1), ELEMIDX(elemidx3));
    LINK_TRACE("BLOCK 0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", blocx1, blocy1, blocx2, blocy2);
    LINK_TRACE("FEN   0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", fenx1, feny1, fenx2, feny2);
}

u8 clipfen(SpriteVariables *sprite)
{
    DEBUGFCE;

    fclip = 0;
    s16 spritex1 = sprite->newx;
    if (fenx2 < spritex1)
        return fclip;

    s16 spritey1 = sprite->newy;
    if (feny2 < spritey1)
        return fclip;

    s16 spritex2 = sprite->depx;
    if (spritex2 < fenx1)
        return fclip;

    s16 spritey2 = sprite->depy;
    if (spritey2 < feny1)
        return fclip;

    if (spritex1 < fenx1)
        spritex1 = fenx1;

    if (spritey1 < feny1)
        spritey1 = feny1;

    if (fenx2 < spritex2)
        spritex2 = fenx2;

    if (feny2 < spritey2)
        spritey2 = feny2;

    clipx1 = spritex1;
    clipy1 = spritey1;
    clipx2 = spritex2;
    clipy2 = spritey2;
    
    DRAW_TRACE("SPR   0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", spritex1, spritey1, spritex2, spritey2);
    DRAW_TRACE("FEN   0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", fenx1, feny1, fenx2, feny2);
    DRAW_TRACE("CLIP  0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", clipx1, clipy1, clipx2, clipy2);
    
    clipl = (spritex1 - spritex2) + 1;
    cliph = (spritey1 - spritey2) + 1;
    fclip = 1;
    return fclip;
}

s16 rangesprite(s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    DEBUGFCE;

    SpriteVariables *idx1sprvar = SPRITE_VAR(elemidx1);
    s16 newd1  =  idx1sprvar->newd;
    s8 cordspr1 = idx1sprvar->cordspr;
    s8 numelem1 = idx1sprvar->numelem;
    if (wback != 0 && elemidx1 != alis.backsprite && backprof <= newd1)
    {
        pback = 1;
    }

    s16 previdx3 = elemidx3;
    s16 nextidx3 = SPRITE_VAR(elemidx3)->link;
    SpriteVariables *idx3sprvar = NULL;

    while (nextidx3 != 0)
    {
        idx3sprvar = SPRITE_VAR(nextidx3);
        s16 newd2 =   idx3sprvar->newd;
        s8 cordspr2 = idx3sprvar->cordspr;
        s8 numelem2 = idx3sprvar->numelem;

        if (-1 < (s8)idx3sprvar->state && newd2 <= newd1 && (newd2 < newd1 || (cordspr2 <= cordspr1 && (cordspr2 < cordspr1 || (numelem2 <= numelem1 && (numelem2 < numelem1 || -1 == (s8)idx1sprvar->state))))))
        {
            break;
        }
        
        previdx3 = nextidx3;
        nextidx3 = SPRITE_VAR(nextidx3)->link;
    }

    // TODO: this is causing infinite loop
    LINK_TRACE("sprite 0x%.4x -> 0x%.4x replaced by 0x%.4x\n", ELEMIDX(elemidx1), ELEMIDX(idx1sprvar->link), ELEMIDX(nextidx3));
    LINK_TRACE("sprite 0x%.4x -> 0x%.4x replaced by 0x%.4x\n", ELEMIDX(previdx3), ELEMIDX(SPRITE_VAR(previdx3)->link), ELEMIDX(elemidx1));

    idx1sprvar->state = 0;
    idx1sprvar->link = nextidx3;
    SPRITE_VAR(previdx3)->link = elemidx1;

    if (elemidx2 == previdx3)
    {
        elemidx2 = elemidx1;
    }
    
    return elemidx2;
}

s16 rangesprite_i2(s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    SpriteVariables *idx1sprvar = SPRITE_VAR(elemidx1);
    s16 newd1  =  idx1sprvar->newd;
    s8 cordspr1 = idx1sprvar->cordspr;
    s8 numelem1 = idx1sprvar->numelem;
    
    s16 previdx3 = elemidx3;
    SpriteVariables *idx3sprvar = NULL;

    while ((elemidx3 = (SPRITE_VAR(previdx3)->link)) != 0)
    {
        idx3sprvar = SPRITE_VAR(elemidx3);
        s16 newd2 =   idx3sprvar->newd;
        s8 cordspr2 = idx3sprvar->cordspr;
        s8 numelem2 = idx3sprvar->numelem;

        if (((-1 < (s8)(SPRITE_VAR(elemidx3)->state)) &&
             (newd2 <= newd1)) && ((newd2 < newd1 ||
              ((cordspr2 <= cordspr1 && ((cordspr2 < cordspr1 ||
                  ((numelem2 <= numelem1 && ((numelem2 < numelem1 || ((s8)(SPRITE_VAR(elemidx1)->state) == -1))))))))))))
            break;
    }
    
    SPRITE_VAR(elemidx1)->state = 0;
    SPRITE_VAR(elemidx1)->link = elemidx3;
    SPRITE_VAR(previdx3)->link = elemidx1;
    
    if (elemidx2 == previdx3)
    {
        elemidx2 = elemidx1;
    }
    
    return elemidx2;
}

void tstjoints(s16 elemidx)
{
    DEBUGFCE;

    SpriteVariables *sprvar = SPRITE_VAR(elemidx);
    s16 tmpx = sprvar->newx + sprvar->width;
    s16 tmpy = sprvar->newy + sprvar->height;
    
    if (-1 < sprvar->newd && -1 < newd && sprvar->newx <= newx + newl && sprvar->newy <= newy + newh && newx <= tmpx && newy <= tmpy)
    {
        blocx1 = sprvar->newx;
        if (newx < sprvar->newx)
            blocx1 = newx;

        blocy1 = sprvar->newy;
        if (newy < sprvar->newy)
            blocy1 = newy;

        if (tmpx < (s16)(newx + newl))
            tmpx = newx + newl;

        if (tmpy < (s16)(newy + newh))
            tmpy = newy + newh;

        blocx2 = tmpx;
        blocy2 = tmpy;
        joints = 1;

        return;
    }
    
    joints = 0;
}

void scalaire(SceneVariables *scene, s16 *x, s16 *y, s16 *z)
{
    DEBUGFCE;

    if ((s8)scene->numelem < 0)
    {
        s32 prevx = (scene->unknown0x20 * *x + scene->unknown0x21 * *y + scene->unknown0x22 * *z);
        s32 prevy = (scene->unknown0x23 * *x + scene->unknown0x24 * *y + scene->unknown0x25 * *z);
        s32 prevz = (scene->unknown0x26 * *x + scene->unknown0x27 * *y + scene->unknown0x28 * *z);
        *x = prevx;
        *y = prevz;
        *z = prevy;
    }
    else
    {
        s16 prevy = *y;
        *y = *y * scene->unknown0x24 - *z;
        *z = prevy;
    }
}

void depscreen(SceneVariables *scene, s16 elemidx)
{
    DEBUGFCE;

    scene->depx += scene->unknown0x2a;
    scene->depy += scene->unknown0x2c;
    scene->depz += scene->unknown0x2e;
    
    SpriteVariables *elemsprvar = NULL;
    
    while (elemidx != 0)
    {
        elemsprvar = SPRITE_VAR(elemidx);
        
        if (-1 < (s8)elemsprvar->chsprite && elemsprvar->state == 0)
        {
            elemsprvar->state = 2;
        }

        elemidx = elemsprvar->link;
    }
}

void deptopix(SceneVariables *scene, s16 elemidx)
{
    DEBUGFCE;

    if ((scene->numelem & 2) == 0)
    {
        SpriteVariables *elemsprvar = SPRITE_VAR(elemidx);
        
        s16 scridx = elemsprvar->screen_id;
        SceneVariables *scene = SCENE_VAR(scridx);
        newf = elemsprvar->flaginvx;
        s16 tmpdepx = elemsprvar->depx - scene->depx;
        s16 tmpdepy = elemsprvar->depy - scene->depy;
        s16 tmpdepz = elemsprvar->depz - scene->depz;
        
        scalaire(scene, &tmpdepx, &tmpdepy, &tmpdepz);

        s16 offset = 0;
        u8 cred = scene->creducing;
        if ((-1 < (s8)cred) && -1 < (s8)elemsprvar->credon_off)
        {
            if (tmpdepz == 0)
            {
                tmpdepz = 1;
            }
            
            tmpdepx = (s16)(((s32)tmpdepx << (cred & 0x3f)) / (s32)tmpdepz);
            tmpdepy = (s16)(((s32)tmpdepy << (cred & 0x3f)) / (s32)tmpdepz);
            scridx = (tmpdepz >> (elemsprvar->credon_off & 0x3f)) + (s16)elemsprvar->creducing;
            if (scridx < 0)
            {
                offset = 0;
            }
            else
            {
                if (scene->clinking < scridx)
                {
                    scridx = scene->clinking;
                }
                
                offset = scridx << 2;
            }
        }
        
        tmpdepx = (tmpdepx >> (scene->credon_off & 0x3f)) + scene->unknown0x0a;
        tmpdepy = (tmpdepy >> (scene->credon_off & 0x3f)) + scene->unknown0x0c;
        
        newad = (elemsprvar->data & 0xffffff) + offset;
        u8 *spritedata = (alis.mem + newad);
        if (spritedata[0] == '\x03')
        {
            tmpdepx += (elemsprvar->flaginvx ? -1 : 1) * read16(spritedata + 4, alis.platform.is_little_endian);
            tmpdepy += read16(spritedata + 6, alis.platform.is_little_endian);
            if (spritedata[1] != 0)
            {
                newf = newf ^ 1;
            }
            
            newad = (newad + (read16(spritedata + 2, alis.platform.is_little_endian) << 2));
            spritedata = (alis.mem + newad);
        }
        
        newl = read16(spritedata + 2, alis.platform.is_little_endian);
        newh = read16(spritedata + 4, alis.platform.is_little_endian);
        newzoomx = 0;
        newzoomy = 0;
        newx = tmpdepx - (newl >> 1);
        newy = tmpdepy - (newh >> 1);
        newd = tmpdepz;
    }
}

void waitdisk(void)
{
    DEBUGFCE;
}

void waitphysic(void)
{
    DEBUGFCE;
    do {} while (fphysic != '\0');
}

void trsfen(u8 *src, u8 *tgt)
{
    DEBUGFCE;
    
    src += fenx1 + feny1 * 320;
    tgt += fenx1 + feny1 * 320;
    
    u16 skip = 320 - (fenx2 - fenx1);

    for (s32 y = feny1; y < feny2; y++)
    {
        for (s32 x = fenx1; x < fenx2; x++, src++, tgt++)
        {
            *tgt = *src;
        }
        
        tgt += skip;
        src += skip;
    }
}

void phytolog(void)
{
    DEBUGFCE;
    fenx1 = 0;
    feny1 = 0;
    fenx2 = 0x13f;
    feny2 = 199;
    trsfen(physic, logic);
}

void mouserase(void)
{
    DEBUGFCE;
    
    // TODO: ...
//    u8 *paVar7 = physic;
//    if (fremouse2 != '\0')
//    {
//        paVar7 = logic;
//    }
//
//    s16 sVar2 = 0;
//    s16 sVar4 = 0xf;
//    s16 sVar3 = 0x120;
//
//    if (-1 < savmouse[0])
//    {
//        u8 bVar1 = 0x12f < savmouse[0];
//        if (bVar1)
//        {
//            sVar2 = 0x10;
//            sVar3 = 0x130;
//        }
//
//        u32 *puVar5 = savmouse2;
//        if ((s16)(-savmouse[1] + 0xb8) < 0)
//        {
//            sVar4 = -savmouse[1] + 199;
//        }
//
//        u32 *puVar8 = (u32 *)(paVar7 + savmouse[1] * 0x140 + (savmouse[0] & 0xfff0));
//
//        do
//        {
//            u32 *puVar6;
//            u32 *puVar9;
//            if (!bVar1)
//            {
//                puVar8[0] = puVar5[0];
//                puVar8[1] = puVar5[1];
//                puVar6 = puVar5 + 3;
//                puVar9 = puVar8 + 3;
//                puVar8[2] = puVar5[2];
//                puVar5 = puVar5 + 4;
//                puVar8 = puVar8 + 4;
//                puVar9[0] = puVar6[0];
//            }
//
//            puVar8[0] = puVar5[0];
//            puVar8[1] = puVar5[1];
//            puVar8[2] = puVar5[2];
//            puVar8[3] = puVar5[3];
//            puVar8 = (u32 *)((s32)puVar8 + sVar3 + 0x10);
//            puVar5 = (u32 *)((s32)puVar5 + sVar2 + 0x10);
//            sVar4 --;
//        }
//        while (sVar4 != -1);
//    }
}

void tvtofen(void)
{
    DEBUGFCE;

    // fenx2 = fenlargw * 4 + fenx1 - 1;
    trsfen(physic, logic);
}

void memfen(void)
{
    ptabfen[0] = fenx1;
    ptabfen[1] = fenx2;
    ptabfen[2] = feny1;
    ptabfen[3] = feny2;
    ptabfen += 4;

    u16 *endtabfen = tabfen + sizeof(tabfen);
    if (ptabfen < endtabfen)
    {
        return;
    }

    // error
}

void oldfen(void)
{
    DEBUGFCE;

    for (u16 *tabptr = tabfen; tabptr < ptabfen; tabptr += 4)
    {
        fenx1 = tabptr[0];
        fenx2 = tabptr[1];
        feny1 = tabptr[2];
        feny2 = tabptr[3];
        tvtofen();
    }
}

void setphysic(void)
{
    DEBUGFCE;
    
    if (insid == 0)
    {
        host.pixelbuf.data = physic;
    }

    bufpack = logic;
    fphysic = 1;
}

u8 *folscreen(u8 *scene)
{
    DEBUGFCE;

    // TODO: ...
//    s16 *psVar1 = *(s16 **)(alis.atent + *(s16 *)(scene + 0x60));
//    if ((-1 < *(s16 *)(scene + 0x60)) && (psVar1 != (s16 *)0x0))
//    {
//        if ((scene[0x84] & 1) != 0)
//        {
//            if ((s16)(*psVar1 + *(s16 *)(scene + 0x86)) != *(s16 *)(scene + 0x16))
//            {
//                *(s16 *)(scene + 0x16) = *psVar1 + *(s16 *)(scene + 0x86);
//                *scene = *scene | 0x80;
//            }
//            if ((s16)(psVar1[4] + *(s16 *)(scene + 0x88)) != *(s16 *)(scene + 0x18))
//            {
//                *(s16 *)(scene + 0x18) = psVar1[4] + *(s16 *)(scene + 0x88);
//                *scene = *scene | 0x80;
//            }
//            if ((s16)(psVar1[8] + *(s16 *)(scene + 0x8a)) != *(s16 *)(scene + 0x1a))
//            {
//                *(s16 *)(scene + 0x1a) = psVar1[8] + *(s16 *)(scene + 0x8a);
//                *scene = *scene | 0x80;
//            }
//        }
//
//        if ((scene[0x84] & 2) != 0)
//        {
//            s16 sVar2 = psVar1[0x0c] + psVar1[0x20] + *(s16 *)(scene + 0x8c);
//            if (sVar2 != *(s16 *)(scene + 0x34))
//            {
//                *(s16 *)(scene + 0x34) = sVar2;
//                *scene = *scene | 0x80;
//            }
//            sVar2 = psVar1[0x10] + psVar1[0x22] + *(s16 *)(scene + 0x8e);
//            if (sVar2 != *(s16 *)(scene + 0x36))
//            {
//                *(s16 *)(scene + 0x36) = sVar2;
//                *scene = *scene | 0x80;
//            }
//            sVar2 = psVar1[0x14] + psVar1[0x24] + *(s16 *)(scene + 0x90);
//            if (sVar2 != *(s16 *)(scene + 0x38))
//            {
//                *(s16 *)(scene + 0x38) = sVar2;
//                *scene = *scene | 0x80;
//            }
//        }
//    }

    return scene;
}

void addlink(s16 elemidx)
{
    DEBUGFCE;

    SpriteVariables *elemsprvar = SPRITE_VAR(elemidx);
    if (-1 < elemsprvar->newd)
    {
        s16 tmp = elemsprvar->newx;
        if (tmp <= blocx1)
            blocx1 = tmp;

        tmp = tmp + elemsprvar->width;
        if (blocx2 <= tmp)
            blocx2 = tmp;

        tmp = elemsprvar->newy;
        if (tmp <= blocy1)
            blocy1 = tmp;

        tmp = tmp + elemsprvar->height;
        if (blocy2 <= tmp)
            blocy2 = tmp;
    }
}

s16 inouvlink(SceneVariables *scene, s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    DEBUGFCE;

    deptopix(scene, elemidx1);
    
    SpriteVariables *elem1sprvar = SPRITE_VAR(elemidx1);
    elem1sprvar->newad = newad;
    elem1sprvar->newx = newx;
    elem1sprvar->newy = newy;
    elem1sprvar->newd = newd;
    elem1sprvar->newf = newf;
    elem1sprvar->width = newl;
    elem1sprvar->height = newh;
    elem1sprvar->newzoomx = newzoomx;
    elem1sprvar->newzoomy = newzoomy;

    SpriteVariables *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

s16 iremplink(SceneVariables *scene, s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    DEBUGFCE;

    addlink(elemidx1);
    
    SpriteVariables *elem1sprvar = SPRITE_VAR(elemidx1);
    if (wback != 0 && (s16)backprof <= elem1sprvar->newd)
    {
        pback = 1;
        return inouvlink(scene, elemidx1, elemidx2, elemidx3);
    }
    
    deptopix(scene, elemidx1);
    
    elem1sprvar->newad = newad;
    elem1sprvar->newx = newx;
    elem1sprvar->newy = newy;
    elem1sprvar->newd = newd;
    elem1sprvar->newf = newf;
    elem1sprvar->width = newl;
    elem1sprvar->height = newh;
    elem1sprvar->newzoomx = newzoomx;
    elem1sprvar->newzoomy = newzoomy;

    SpriteVariables *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    return rangesprite(elemidx1, elemidx2, elemidx3);
}

s16 iefflink(s16 elemidx1, s16 elemidx2)
{
    DEBUGFCE;

    addlink(elemidx1);

    SpriteVariables *elem1sprvar = SPRITE_VAR(elemidx1);
    SpriteVariables *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;
    elem1sprvar->to_next = alis.libsprit;
    
    alis.libsprit = elemidx1;
    if (wback != 0 && backprof <= elem1sprvar->newd)
    {
        pback = 1;
    }
    
    return elemidx2;
}

u16 scdirect = 0;
u8 fdoland = 0;

void clrfen(void)
{
    DEBUGFCE;

    s16 tmpx = fenx2 - fenx1;
    // s16 tmpy = feny2 - feny1;
    
    for (s16 y = feny1; y < feny2; y++)
    {
        memset(physic + fenx1 + y * 320, 0, tmpx);
    }
}

void clipback(void)
{
    DEBUGFCE;

    if (clipy1 < ((u16 *)(&backx1))[1])
    {
        if (clipy2 <((u16 *)(&backx1))[1])
        {
            return;
        }
    }
    else
    {
        if (clipy2 <= ((u16 *)(&backx2))[1])
        {
            cback = 1;
            return;
        }
        
        if (((u16 *)(&backx2))[1] < clipy1)
        {
            return;
        }
    }
    
    cback = 0xff;
    return;
}

void destofen(SpriteVariables *sprite)
{
    u32 spnewad = sprite->newad & 0xffffff;
    if (spnewad == 0)
        return;
    
    u8 *bitmap = (alis.mem + spnewad);
    if (*bitmap < '\0')
        return;
    
    s16 posx1 = sprite->newx;
    if ((s16)clipx2 < posx1)
        return;

    s16 posy1 = sprite->newy;
    if (clipy2 < posy1)
        return;

    s16 xpos2 = posx1 + read16(bitmap + 2, alis.platform.is_little_endian);
    if (xpos2 < clipx1)
        return;

    s16 ypos2 = posy1 + read16(bitmap + 4, alis.platform.is_little_endian);
    if (ypos2 < clipx1)
        return;

    blocx1 = posx1;
    if (posx1 < clipx1)
        blocx1 = clipx1;

    blocy1 = posy1;
    if (posy1 < clipx1)
        blocy1 = clipx1;

    blocx2 = xpos2;
    if (clipx2 < xpos2)
        blocx2 = clipx2;

    blocy2 = ypos2;
    if (clipy2 < ypos2)
        blocy2 = clipy2;

    u8 *at = bitmap + 6;

    u8 color;
    u8 clear = 0;
    u8 palidx = 0;

    u32 flip = sprite->newf;
    u32 width = sprite->width + 1;
    u32 height = sprite->height + 1;

    s32 bmpy1 = 0;
    s32 bmpy2 = height;
    if (posy1 < 0)
    {
        bmpy2 += posy1;
        bmpy1 -= posy1;
    }
    
    s32 bmpx1 = 0;
    s32 bmpx2 = width;
    if (blocx1 < 0)
    {
        bmpx1 -= blocx1;
        bmpx2 += blocx1;
    }
    
    if (blocx1 > posx1)
    {
        bmpx1 += (blocx1 - posx1);
        bmpx2 -= (blocx1 - posx1);
    }
    
    if (blocx2 < xpos2)
    {
        bmpx2 -= (xpos2 - blocx2);
    }
    
//    s16 index = -1;
//    for (s32 i = 0; i < 24; i++)
//    {
//        s32 addr = adresdes(i);
//        if (sprite->data == addr)
//        {
//            index = i;
//            break;
//        }
//    }
    
    DRAW_TRACE("%.2d %.2x %.2x (%.2x %.2x)\n", index, sprite->state, sprite->numelem, bitmap[0], bitmap[1]);
    DRAW_TRACE("%.3d %.3d %.3d %.3d\n", blocx1, blocy1, blocx2, blocy2);
    DRAW_TRACE("%.3d %.3d %.3d %.3d\n", bmpx1 + posx1, bmpy1 + posy1, bmpx2 + posx1, bmpy2 + posy1);
    DRAW_TRACE("  to:  %.3dx%.3d\n", posx1, posy1);
    DRAW_TRACE("  dim: %.3dx%.3d\n", width, height);
    
    // NOTE: just a hack to write directly to output buffer
    // u8 *logic = host.pixelbuf.data;
    
    switch (bitmap[0])
    {
        case 0x01:
        {
            // rectangle
            
            color = bitmap[1];
            
            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    if ((posy1 + h) < 0 || (posy1 + h) >= 200)
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
            
            at = bitmap + 6;
            
            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    u16 wh = (flip ? (width - (w + 1)) : w) / 2;
                    color = *(at + wh + h * (width / 2));
                    color = 0 + w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                    if (color != clear)
                    {
                        if ((posy1 + h) < 0 || (posy1 + h) >= 200)
                            continue;
                        
                        tgt = logic + (w + posx1) + ((posy1 + h) * host.pixelbuf.w);
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
            clear = bitmap[7];
            
            at = bitmap + 8;

            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    if ((posy1 + h) < 0 || (posy1 + h) >= 200)
                        continue;
                    
                    u16 wh = (flip ? (width - (w + 1)) : w) / 2;
                    color = *(at + wh + h * (width / 2));
                    color = 0 + w % 2 == flip ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                    if (color != clear)
                        *tgt = palidx + color;
                }
            }
            
            break;
        }
            
        case 0x14:
        case 0x16:
        {
            // 8 bit image
            
            palidx = bitmap[6];
            clear = bitmap[7];

            at = bitmap + 8;

            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = logic + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    if ((posy1 + h) < 0 || (posy1 + h) >= 200)
                        continue;
                    
                    color = *(at + (flip ? width - (w + 1) : w) + h * width);
                    if (color != clear)
                        *tgt = palidx + color;
                }
            }
            
            break;
        }
            
        case 0x40:
        {
            // FLI video
            
            uint32_t size1 = read32(bitmap + 2, alis.platform.is_little_endian);
            s8 *fliname = (s8 *)&bitmap[6];
            DRAW_TRACE("FLI video (%s) %d bytes [", fliname, size1);

            uint32_t size2 = (*(uint32_t *)(&bitmap[32]));
            uint16_t frames = (*(uint16_t *)(&bitmap[38]));

            DRAW_TRACE("size: %d frames: %d]\n", size2, frames);

            // TODO: ...
            
            break;
        }
    }
}

void calctop(void)
{
    return;
}

void fentotv(void)
{
    trsfen(logic, physic);
}

void fenetre(SceneVariables *scene, u16 elemidx1, u16 elemidx3)
{
    DEBUGFCE;

    u16 tmpidx;
    u16 scridx;
    SpriteVariables *sprite;

    if ((scene->state & 0x40) != 0)
    {
        return;
    }

    scdirect = 0;
    calcfen(elemidx1, elemidx3);
    
    {
        if ((scene->numelem & 2) != 0)
        {
            fdoland = 1;
        }
        
        scridx = ptscreen;
        tmpidx = ptscreen;
        while (tmpidx != 0)
        {
            SceneVariables *scene = SCENE_VAR(scridx);

            if ((scene->state & 0x40U) == 0)
            {
                tmpidx = scene->screen_id;
                elemidx1 = tmpidx;
                if (tmpidx != 0)
                {
                    clipfen(SPRITE_VAR(tmpidx));
                    if (fclip != 0)
                    {
                        if ((scene->numelem & 2U) == 0)
                        {
                            cback = 0;
                            if ((scene->numelem & 4U) != 0)
                            {
                                clipback();
                            }

                            if ((scene->numelem & 0x40U) == 0)
                            {
                                clrfen();
                            }

                            if (cback)
                            {
                                if ((s8)cback < 0)
                                {
                                    if ((wback != 0) && (pback != 0))
                                    {
                                        wlogic = backmap;
                                        wlogx1 = backx1;
                                        wlogx2 = backx2;
                                        wlogy1 = backy1;
                                        wlogy2 = backy2;
                                        wloglarg = backlarg;
                                        
                                        if (clipy1 <= backx1)
                                            clipy1 = backx1;

                                        if (backx2 <= clipy2)
                                            clipy2 = backx2;

                                        sprite = SPRITE_VAR(tmpidx);
                                        while (true)
                                        {
                                            tmpidx = sprite->link;
                                            if ((tmpidx == 0) || (tmpidx == alis.backsprite))
                                                break;

                                            sprite = SPRITE_VAR(tmpidx);
                                            if (((-1 < (s8)sprite->state) && (-1 < (s8)sprite->newf)) && (-1 < sprite->newd))
                                            {
                                                DRAW_TRACE("SPRITE %.4x\n", ELEMIDX(tmpidx));
                                                destofen(sprite);
                                            }
                                        }

                                        wlogic = logic;
                                        wlogx1 = logx1;
                                        wlogx2 = logx2;
                                        wlogy1 = logy1;
                                        wlogy2 = logy2;
                                        wloglarg = loglarg;
                                    }
                                }
                                else
                                {
                                    if (pback == 0)
                                        goto fenetre31;

                                    wlogic = backmap;
                                    wlogx1 = backx1;
                                    wlogx2 = backx2;
                                    wlogy1 = backy1;
                                    wlogy2 = backy2;
                                    wloglarg = backlarg;
                                }
                            }

                            sprite = SPRITE_VAR(tmpidx);
                            while (true)
                            {
                                tmpidx = sprite->link;
                                if (tmpidx == 0)
                                    break;

                                if ((sback != 0) && (tmpidx == alis.backsprite))
                                {
                                    wlogic = logic;
                                    wlogx1 = logx1;
                                    wlogx2 = logx2;
                                    wlogy1 = logy1;
                                    wlogy2 = logy2;
                                    wloglarg = loglarg;
                                }
  fenetre31:
                                sprite = SPRITE_VAR(tmpidx);
                                if (((-1 < (s8)sprite->state) && (-1 < (s8)sprite->newf)) && (-1 < sprite->newd))
                                {
                                    DRAW_TRACE("SPRITE %.4x\n", ELEMIDX(tmpidx));
                                    destofen(sprite);
                                    switchgo = 1;
                                }
                            }
                        }
                        else
                        {
                            calctop();
                        }
                    }
                }
            }

            tmpidx = scene->to_next;
            scridx = tmpidx;
        }

        // TODO: mouse
//        cVar2 = fremouse;
//        if (((-1 < fmouse) && (fmouse != '\x02')) && (alis._cclipping == '\0'))
//        {
//            do
//            {
//                fremouse = cVar2 + '\x01';
//            }
//            while (fremouse != '\0');
//
//            if (tvmode == '\0')
//            {
//                mousefen((s32)d0);
//            }
//            else
//            {
//                mouserase();
//            }
//        }

        if ((scene->numelem & 0x20) == 0)
        {
            if (alis._cclipping != 0)
            {
                if (wpag == 0)
                {
                    memfen();
                }

                return;
            }

            if (wpag == 0)
            {
                fentotv();
            }
        }

        // TODO: mouse
//        if (alis._cclipping == '\0')
//        {
//            if ((tvmode != '\0') && (-1 < fmouse))
//            {
//                mouseput();
//            }
//
//            fremouse = -1;
//        }
    }
}

void affiscr(SceneVariables *scene, u16 elemidx3)
{
    u8 state;
    s16 elemidx1;
    s16 elemidx2;
    s16 tmpidx1;
    s16 tmpidx2;
    s16 tmpidx3;
    u8 issprit;
    u8 isback;
    
//    if (a2[0x84] != '\0')
//    {
//        a2 = folscreen(a2);
//    }
    
    if ((fremap != '\0') || ((s8)scene->state < '\0'))
    {
        depscreen(scene, elemidx3);
    }
    
    if ((scene->numelem & 2) == 0 || (scene->state & 0x80U) == 0)
    {
        wback = (scene->numelem & 4) != 0;
        wpag = '\0';
        if ((scene->state & 0x20U) != 0)
        {
            wpag = '\x01';
            spag = spag + -1;
            if (spag == '\0')
            {
                wpag = -1;
            }
        }
        
        elemidx1 = elemidx3;
        
        if ((scene->numelem & 0x10) == 0)
        {
affiscin:
            do
            {
                tmpidx2 = elemidx1;
                elemidx1 = SPRITE_VAR(tmpidx2)->link;
                if (elemidx1 == 0)
                    goto affiscr1;
                
                state = SPRITE_VAR(elemidx1)->state;
                if (state != 0)
                {
                    joints = 0;
                    pback = 0;
                    if (state == 2)
                    {
                        tmpidx1 = elemidx1;
                        tmpidx3 = inilink(elemidx1);
                        if (tmpidx3 < 0)
                        {
                            isback = wback == '\0';
                            if (!isback)
                            {
                                isback = backprof == SPRITE_VAR(elemidx1)->newd;
                                if (backprof <= SPRITE_VAR(elemidx1)->newd)
                                {
                                    pback = 1;
                                    isback = false;
                                }
                            }
                            
                            deptopix(scene,elemidx1);
                            tstjoints(elemidx1);
                            if (isback)
                            {
                                SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;
                                fenetre(scene,elemidx1,elemidx3);
                            }
                            else
                            {
                                SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;
                            }
                            
                            SpriteVariables *sprvar = SPRITE_VAR(elemidx1);
                            sprvar->newad = newad;
                            sprvar->newx = newx;
                            sprvar->newy = newy;
                            sprvar->newd = newd;
                            sprvar->newf = newf;
                            sprvar->width = newl;
                            sprvar->height = newh;
                            sprvar->newzoomx = newzoomx;
                            sprvar->newzoomy = newzoomy;
                            
                            tmpidx2 = rangesprite(elemidx1,tmpidx2,elemidx3);
                            fenetre(scene,elemidx1,elemidx3);
                            elemidx1 = tmpidx2;
                            goto affiscin;
                        }
                        
                        tmpidx1 = iremplink(scene,tmpidx1,tmpidx2,elemidx3);
                    }
                    else
                    {
                        if (state == 0xff)
                        {
                            tmpidx1 = elemidx1;
                            tmpidx3 = inilink(elemidx1);
                            tmpidx1 = inouvlink(scene,tmpidx1,tmpidx2,elemidx3);
                        }
                        else
                        {
                            tmpidx1 = elemidx1;
                            tmpidx3 = inilink(elemidx1);
                            tmpidx1 = iefflink(tmpidx1,tmpidx2);
                        }
                        
                        if (tmpidx3 < 0)
                        {
                            joints = 1;
                            fenetre(scene,elemidx1,elemidx3);
                            elemidx1 = tmpidx2;
                            goto affiscin;
                        }
                    }
                    
                    elemidx2 = tmpidx1;
                    while ((tmpidx1 = SPRITE_VAR(elemidx2)->link) != 0)
                    {
                        if (tmpidx3 == SPRITE_VAR(tmpidx1)->clinking)
                        {
                            state = SPRITE_VAR(tmpidx1)->state;
                            if (state != 0)
                            {
                                if ((s8)state < '\0')
                                {
                                    tmpidx1 = inouvlink(scene,tmpidx1,elemidx2,elemidx3);
                                }
                                else if (state == 2)
                                {
                                    tmpidx1 = iremplink(scene,tmpidx1,elemidx2,elemidx3);
                                }
                                else
                                {
                                    tmpidx1 = iefflink(tmpidx1,elemidx2);
                                }
                            }
                        }
                        
                        elemidx2 = tmpidx1;
                    }
                    
                    joints = 1;
                    fenetre(scene,elemidx1,elemidx3);
                    elemidx1 = tmpidx2;
                }
            }
            while(true);
        }
        
        fenx1 = scene->newx;
        feny1 = scene->newy;
        fenx2 = fenx1 + scene->width;
        feny2 = feny1 + scene->height;
        clipl = (fenx2 - fenx1) + 1;
        cliph = (feny2 - feny1) + 1;

        fenlargw = clipl >> 2;

        clipx1 = fenx1;
        clipy1 = feny1;
        clipx2 = fenx2;
        clipy2 = feny2;
        
        if ((scene->numelem & 0x40) == 0)
        {
            clrfen();
        }
        
        tmpidx2 = elemidx1;
        while ((elemidx1 = SPRITE_VAR(tmpidx2)->link) != 0)
        {
            if (SPRITE_VAR(elemidx1)->state != 0)
            {
                if (SPRITE_VAR(elemidx1)->state == 1)
                {
                    SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;
                    SPRITE_VAR(elemidx1)->to_next = alis.libsprit;
                    alis.libsprit = elemidx1;
                    elemidx1 = tmpidx2;
                }
                else
                {
                    deptopix(scene,elemidx1);
                    SPRITE_VAR(tmpidx2)->link = SPRITE_VAR(elemidx1)->link;

                    SpriteVariables *sprvar = SPRITE_VAR(elemidx1);
                    sprvar->newad = newad;
                    sprvar->newx = newx;
                    sprvar->newy = newy;
                    sprvar->newd = newd;
                    sprvar->newf = newf;
                    sprvar->width = newl;
                    sprvar->height = newh;
                    sprvar->newzoomx = newzoomx;
                    sprvar->newzoomy = newzoomy;
                    elemidx1 = rangesprite(elemidx1,tmpidx2,elemidx3);
                }
            }
        }
        
        issprit = ptscreen == 0;
        elemidx1 = ptscreen;
    }
    else
    {
        issprit = *(s16 *)((u8 *)scene + 4) == 0;
        elemidx1 = *(s16 *)((u8 *)scene + 4);
    }
    
    while (!issprit)
    {
        SceneVariables *elemscene = SCENE_VAR(elemidx1);
        if (((elemscene->state & 0x40U) == 0) && ((tmpidx2 = elemscene->screen_id) != 0))
        {
            clipfen(SPRITE_VAR(tmpidx2));
            
            if (fclip != '\0')
            {
                while ((tmpidx2 = SPRITE_VAR(tmpidx2)->link) != 0)
                {
                    if (((-1 < (s8)SPRITE_VAR(tmpidx2)->state) && (-1 < (s8)SPRITE_VAR(tmpidx2)->newf)) && (-1 < SPRITE_VAR(tmpidx2)->newd))
                    {
                        destofen(SPRITE_VAR(tmpidx2));
                        switchgo = 1;
                    }
                }
            }
        }
        
        issprit = SCENE_VAR(elemidx1)->to_next == 0;
        elemidx1 = SCENE_VAR(elemidx1)->to_next;
    }
    
    if ((alis._cclipping == '\0') && (wpag == '\0'))
    {
        fentotv();
    }
    
  affiscr1:
    
    if (wpag < '\0')
    {
        scene->state &= 0xdf;
        fenx1 = scene->newx;
        feny1 = scene->newy;
        fenx2 = scene->newx + scene->width;
        feny2 = scene->newy + scene->height;
        // scrolpage(a0,CONCAT22(uVar2,feny2));
    }
    
    scene->state &= 0x7f;
}

//void affiscr(SceneVariables *scene, u16 elemidx3)
//{
//    DEBUGFCE;
//
//    u8 state;
//    u8 issprit;
//    u16 uvaridx;
//    u16 nextidx;
//    s16 linkidx;
//    s16 previdx;
//    s16 elemidx;
//    s16 tempidx;
//
//    // NOTE: falcon or ishar 3 specific
////    if (((u8 *)scene)[0x84] != '\0')
////    {
////        scene = (s8 *)folscreen((u8 *)scene);
////    }
//
//    if (fremap != 0 || scene->state < 0)
//    {
//        depscreen(scene, elemidx3);
//    }
//
//    if ((scene->numelem & 2) == 0 || (scene->state & 0x80U) == 0)
//    {
//        wback = (scene->numelem & 4) != 0;
//        wpag = '\0';
//
//        if ((scene->state & 0x20U) != 0)
//        {
//            wpag = '\x01';
//            spag --;
//            if (spag == '\0')
//            {
//                wpag = -1;
//            }
//        }
//
//        SpriteVariables *nextsprvar = NULL;
//
//        DBTRACE("\n");
//
//        u8 first = true;
//        if ((scene->numelem & 0x10) == 0)
//        {
//
//affiscin:
//            nextidx = elemidx3;
//
//            do
//            {
//                previdx = elemidx3;
//                nextidx = SPRITE_VAR(nextidx)->link;
//                if (first && nextidx)
//                {
//                    u16 test = ELEMIDX(nextidx);
//                    DBTRACE("0x%.4x\n", test);
//
////                    first = false;
//                }
//
//                if (nextidx == 0)
//                    goto affiscr1;
//
//                nextsprvar = SPRITE_VAR(nextidx);
//                state = nextsprvar->state;
//
//                if (state != 0)
//                {
//                    joints = 0;
//                    pback = 0;
//                    elemidx = previdx;
//                    tempidx = nextidx;
//                    if (state == 2)
//                    {
//                        linkidx = inilink(nextidx);
//                        if (linkidx < 0)
//                        {
//                            u8 not_wback = wback == '\0';
//                            if (!(u8)not_wback)
//                            {
//                                not_wback = backprof == nextsprvar->newd;
//                                if ((s16)backprof <= nextsprvar->newd)
//                                {
//                                    pback = 1;
//                                    not_wback = false;
//                                }
//                            }
//
//                            deptopix(scene, nextidx);
//                            tstjoints(nextidx);
////                            VERIFYINTEGRITY;
//
//                            if ((u8)not_wback)
//                            {
//                                SPRITE_VAR(previdx)->link = nextsprvar->link;
//                                fenetre(scene, nextidx, elemidx3);
//                            }
//                            else
//                            {
//                                SPRITE_VAR(previdx)->link = nextsprvar->link;
//                            }
//
//                            nextsprvar->newad = newad;
//                            nextsprvar->newx = newx;
//                            nextsprvar->newy = newy;
//                            nextsprvar->newd = newd;
//                            nextsprvar->newf = newf;
//                            nextsprvar->width = newl;
//                            nextsprvar->height = newh;
//                            nextsprvar->newzoomx = newzoomx;
//                            nextsprvar->newzoomy = newzoomy;
//
//                            elemidx = nextidx;
//                            rangesprite(elemidx, elemidx3);
//                            fenetre(scene, elemidx, elemidx3);
////                            VERIFYINTEGRITY;
//
//                            goto affiscin;
//                        }
//
//                        iremplink(scene, tempidx, elemidx, elemidx3);
//                    }
//                    else
//                    {
//                        if (state == 0xff)
//                        {
////                            VERIFYINTEGRITY;
//                            // handle new sprites
//                            linkidx = inilink(nextidx);
//                            inouvlink(scene, tempidx, elemidx, elemidx3);
////                            VERIFYINTEGRITY;
//                        }
//                        else
//                        {
////                            VERIFYINTEGRITY;
//                            linkidx = inilink(nextidx);
//                            iefflink(tempidx, elemidx);
//                            tempidx = elemidx;
////                            VERIFYINTEGRITY;
//                        }
//
//                        if (linkidx < 0)
//                        {
////                            VERIFYINTEGRITY;
//                            joints = 1;
//                            fenetre(scene, nextidx, elemidx3);
////                            VERIFYINTEGRITY;
//                            nextidx = elemidx3;
//                            goto affiscin;
//                        }
//                    }
//
//                    while (true)
//                    {
////                        VERIFYINTEGRITY;
//                        elemidx = tempidx;
//                        nextidx = SPRITE_VAR(elemidx)->link;
//                        tempidx = nextidx;
//                        if (nextidx == 0)
//                            break;
//
//                        nextsprvar = SPRITE_VAR(nextidx);
//                        if (linkidx == nextsprvar->clinking)
//                        {
//                            state = nextsprvar->state;
//                            if (state != 0)
//                            {
//                                if ((s8)state < 0)
//                                {
////                                    VERIFYINTEGRITY;
//                                    inouvlink(scene, nextidx, elemidx, elemidx3);
////                                    VERIFYINTEGRITY;
//                                }
//                                else if (state == 2)
//                                {
////                                    VERIFYINTEGRITY;
//                                    iremplink(scene, nextidx, elemidx, elemidx3);
////                                    VERIFYINTEGRITY;
//                                }
//                                else
//                                {
////                                    VERIFYINTEGRITY;
//                                    iefflink(nextidx, elemidx);
////                                    VERIFYINTEGRITY;
//                                    nextidx = elemidx;
//                                }
//                            }
//                        }
//                    }
//
//                    joints = 1;
//                    fenetre(scene, previdx, elemidx3);
//                    nextidx = elemidx3;
//                }
//            }
//            while (true);
//        }
//
////        VERIFYINTEGRITY;
//        fenx1 = scene->newx;
//        feny1 = scene->newy;
//        fenx2 = fenx1 + scene->width;
//        feny2 = feny1 + scene->height;
//        clipl = (fenx2 - fenx1) + 1;
//        cliph = (feny2 - feny1) + 1;
//
//        fenlargw = clipl >> 2;
//
//        clipx1 = fenx1;
//        clipy1 = feny1;
//        clipx2 = fenx2;
//        clipy2 = feny2;
//        nextidx = elemidx3;
//        if ((scene->numelem & 0x40) == 0)
//        {
//            clrfen();
//        }
//
////        VERIFYINTEGRITY;
//
//        SpriteVariables *prevsprvar = NULL;
//        while ((nextidx = SPRITE_VAR((uvaridx = nextidx))->link) != 0)
//        {
////            VERIFYINTEGRITY;
//            nextsprvar = SPRITE_VAR(nextidx);
//            state = nextsprvar->state;
//            if (state != 0)
//            {
//                prevsprvar = SPRITE_VAR(uvaridx);
//
//                if (state == 1)
//                {
////                    VERIFYINTEGRITY;
//                    prevsprvar->link = nextsprvar->link;
//                    nextsprvar->to_next = alis.libsprit;
//                    alis.libsprit = nextidx;
//                    nextidx = uvaridx;
////                    VERIFYINTEGRITY;
//                }
//                else
//                {
////                    VERIFYINTEGRITY;
//                    deptopix(scene, nextidx);
////                    VERIFYINTEGRITY;
//
//                    prevsprvar->link = nextsprvar->link;
//                    nextsprvar->newad = newad;
//                    nextsprvar->newx = newx;
//                    nextsprvar->newy = newy;
//                    nextsprvar->newd = newd;
//                    nextsprvar->newf = newf;
//                    nextsprvar->width = newl;
//                    nextsprvar->height = newh;
//                    nextsprvar->newzoomx = newzoomx;
//                    nextsprvar->newzoomy = newzoomy;
//
////                    VERIFYINTEGRITY;
//                    rangesprite(nextidx, elemidx3);
////                    VERIFYINTEGRITY;
//                }
//            }
//        }
//
//        issprit = ptscreen == 0;
//        elemidx = ptscreen;
//    }
//    else
//    {
//        elemidx = scene->to_next;
//        issprit = elemidx == 0;
//    }
//
////    VERIFYINTEGRITY;
//
//    SceneVariables *parentscene = NULL;
//    SpriteVariables *linksprvar = NULL;
//
//    while (!issprit)
//    {
//        parentscene = SCENE_VAR(elemidx);
//
//        if ((((parentscene->state & 0x40U) == 0) && (linkidx = parentscene->screen_id != 0)) && clipfen(SPRITE_VAR(linkidx)) != 0)
//        {
//            while ((linkidx = SPRITE_VAR(linkidx)->link) != 0)
//            {
//                linksprvar = SPRITE_VAR(linkidx);
//                if (-1 < (s8)linksprvar->state && -1 < (s8)linksprvar->newf && -1 < linksprvar->newd)
//                {
//                    DRAW_TRACE("SPRITE %.4x\n", ELEMIDX(linkidx));
//                    destofen(linksprvar);
//                    switchgo = 1;
//                }
//            }
//        }
//
//        SceneVariables *scene = SCENE_VAR(elemidx);
//        elemidx = scene->to_next;
//        issprit = elemidx == 0;
//    }
//
//    if ((alis._cclipping == '\0') && (wpag == '\0'))
//    {
//        fentotv();
//    }
//
////    VERIFYINTEGRITY;
//
//affiscr1:
//
//    if (wpag < '\0')
//    {
//        scene->state &= 0xdf;
//        fenx1 = scene->newx;
//        feny1 = scene->newy;
//        fenx2 = scene->newx + scene->width;
//        feny2 = scene->newy + scene->height;
//        // scrolpage(alis.basesprite,BASEMNMEM_PTR,(u32)feny2);
//    }
//
//    scene->state &= 0x7f;
//}

void itroutine(void)
{
    u8 prevtiming = vtiming;
    timeclock ++;
    fitroutine = 1;
    vtiming = vtiming + '\x01';
    if (vtiming == 0) {
        vtiming = prevtiming;
    }
    
    if (palc != 0 && (--palt) == 0)
    {
        palt = palt0;
        topalet();
        palc = palc + -1;
    }
    
    // TODO: audio
//    if ((bcanal0 != '\0') && (-1 < bcanal0))
//    {
//        canal();
//    }
//
//    if ((bcanal1 != '\0') && (-1 < bcanal1))
//    {
//        canal();
//    }
//
//    if ((bcanal2 != '\0') && (-1 < bcanal2))
//    {
//        canal();
//        check3();
//    }
//
//    if ((bcanal3 != '\0') && (-1 < bcanal3))
//    {
//        canal();
//    }
    
    fitroutine = 0;
}

void waitframe(void) {
    
    struct timeval now, start;
    gettimeofday(&start, NULL);
    
    while ((void)(gettimeofday(&now, NULL)), ((now.tv_sec * 1000000 + now.tv_usec) - (alis.time.tv_sec * 1000000 + alis.time.tv_usec) < FRAME_TICKS)) {
        usleep(1000);
    }

    alis.time = now;
}

void image(void)
{
    DEBUGFCE;
    VERIFYINTEGRITY;
    
    waitframe();
    
//    if (amiga != '\0')
//    {
//        waitdisk();
//    }
//
//    waitphysic();
    if ((alis._cclipping != '\0') && (fphytolog != '\0'))
    {
        fphytolog = '\0';
        phytolog();
    }
    
    alis.running = sys_poll_event();
    
    // s8 prevmouse = fremouse;
    
    // wait for vblank
    // do {} while (vtiming < timing);
    
    vtiming = 0;
    
    // TODO: implement oldfen > transfen (it displays content of logic)
    if (alis._cclipping != '\0')
    {
        // TODO: mouse
        if (fmouse < '\0')
        {
            oldfen();
        }
        else
        {
//            do { } while (fremouse2 != '\0');
//
//            fremouse = fremouse + '\x01';
//            if (fremouse != '\0')
//            {
//                fremouse = prevmouse;
//            }
//
            oldfen();
//            fremouse2 = 1;
//            if (fmouse != '\x02')
//            {
//                mouserase();
//            }
//
//            fremouse2 = '\0';
//            fremouse = -1;
//            if (fmouse == '\0')
//            {
//                fmouse = -1;
//            }
        }

        ptabfen = tabfen;
    }
    
    switchgo = '\0';
    wlogic = logic;
    wlogx1 = logx1;
    wlogx2 = logx2;
    wlogy1 = logy1;
    wlogy2 = logy2;
    wloglarg = loglarg;
    
    // hack
    fremap = 1;
    
    u16 scnidx = ptscreen;
//    prevmouse = fremouse;
    u8 *oldphys = physic;
    
    while (scnidx != 0)
    {
        SceneVariables *scene = SCENE_VAR(scnidx);
        if ((scene->state & 0x40) == 0)
        {
            u16 sprite3 = scene->screen_id;
            affiscr(scene, sprite3);
        }

        if (scene->to_next == 0xc500)
        {
            sleep(0);
            break;
        }
        
        scnidx = scene->to_next;
    }

    fremap = 0;
    if (alis._cclipping != '\0')
    {
//        do
//        {
//            if (switchgo == '\0')
//            {
//                fremouse = prevmouse;
//                fremap = 0;
//            }
//
//            fremouse = prevmouse;
//        }
//        while (((fmouse != '\0') && (-1 < fmouse)) && ((void)(fremouse = prevmouse + '\x01'), fremouse != '\0'));
//
        physic = logic;
        logic = oldphys;
        setphysic();
        
//        if ((fmouse != '\0') && (-1 < fmouse))
//        {
//            fremouse2 = '\x01';
//            fremouse = -1;
//        }
    }
    
    sys_render(host.pixelbuf);

    VERIFYINTEGRITY;
}

u8 *buffer = 0;

void inisys(void)
{
    falcon = 1;
    s32 iVar1 = *(savsp + 4);
    command = iVar1 + 0x80;
//    __m68k_trap(1);
//    __m68k_trap(1);
    debram = ALIS_VM_RAM_ORG; // *(s32 *)(iVar1 + 0xc) + *(s32 *)(iVar1 + 0x14) + *(s32 *)(iVar1 + 0x1c) + -0x221e0; // 3bb520
//    __m68k_trap(1);
    finram = debram * 2;
//    __m68k_trap(1);
    savessp = finram;
//    logic = host.pixelbuf0.data; // (astruct *)(workbuff + 0xf0);
    bufpack = 0x20a00;
    mworkbuff = 0x20a00;
    adtmono = 0x33700;
    finnoyau = 0x33900;
    basemem = 0x33900;
    basevar = 0;
//    __m68k_trap(0xe);
    ophysic = 0x33900;
    u32 uVar2 = finram - 0x12d00U | 0xff;
    physic = host.pixelbuf.data; // uVar2 + 1;
    alis.finmem = uVar2 - 0x3ff;
    
}

void livemain(u16 *param_1, s16 param_2, s8 *param_3)
{
//    // A1 61f00, D1 0
//
//    // alis.atent 4d540
//    // alis.debent    4d546
//    // alis.finent 4d7d4
//    // total 654 (entry 8 bytes)
//    // alis.debsprit 511a0
//    // alis.finent   511a0
//
//
//    s32 iVar4 = (s16)param_1[9] + alis.finent;
//    u32 *puVar5 = (u32 *)((s16)param_1[0xb] + iVar4);
//    s32 iVar6 = (s32)puVar5 + 0x3e;
//    *(s32 *)(alis.atent + param_2) = iVar6;
//    u32 uVar2 = (u32)(u16)param_1[10];
//    s32 iVar1 = uVar2 + iVar6;
//    if (alis.debsprit <= (s32)(uVar2 + iVar6))
//    {
////        perror(param_3, 0xc, param_2, (s32)param_1);
//        return;
//    }
//
//    do
//    {
//        alis.finent = iVar1;
//        *(u16 *)((s32)puVar5 + uVar2 + 0x3c) = 0;
//        uVar2 = uVar2 - 2;
//        iVar1 = alis.finent;
//    }
//    while (uVar2 != 0);
//
//    s16 sVar3 = (s16)iVar4 - (s16)iVar6;
//    *(s16 *)(puVar5 + 0xd) = sVar3;
//    *(s16 *)((s32)puVar5 + 0x22) = sVar3;
//    *(s16 *)(puVar5 + 8) = sVar3;
//    *(u32 *)((s32)puVar5 + 0x36) = (u16)param_1[2] + 2 + (s32)param_1;
//    *(u16 *)((s32)puVar5 + 0x2e) = *param_1;
//    *(u16 **)((s32)puVar5 + 0x2a) = param_1;
//    *(u8 *)(puVar5 + 4) = *(u8 *)(param_1 + 1);
//    *(u8 *)(puVar5 + 0xf) = 1;
//    *(u8 *)((s32)puVar5 + 0x3d) = 1;
//    *(u8 *)((s32)puVar5 + 0x3a) = 0xff;
//    *(u16 *)(puVar5 + 9) = 0xffff;
//    *(s16 *)(puVar5 + 0xc) = param_2;
//    *(u8 *)((s32)puVar5 + 0x1a) = 6;
//    *(u16 *)((s32)puVar5 + 0x26) = 0;
//    *(u16 *)((s32)puVar5 + 0x32) = 0;
//    *(u16 *)(puVar5 + 7) = 0;
//    *(u16 *)((s32)puVar5 + sprite_clinking) = 0;
//    *(u8 *)((s32)puVar5 + 0x3b) = 0;
//    *(u8 *)((s32)puVar5 + 0x19) = 0;
//    *(u16 *)(puVar5 + 2) = 0;
//    *(u16 *)((s32)puVar5 + 6) = 0;
//    *(u8 *)(puVar5 + 6) = 0;
//    *puVar5 = 0;
//    *(u16 *)(puVar5 + 1) = 0;
//    *(u8 *)(puVar5 + 5) = 0;
//    *(u8 *)((s32)puVar5 + 0x12) = 0;
//    *(u8 *)((s32)puVar5 + 0x11) = 0;
//    *(u16 *)((s32)puVar5 + sprite_depx) = 0;
//    *(u8 *)((s32)puVar5 + 0xf) = 0;
//    *(u16 *)(puVar5 + 3) = 0;
//    *(u16 *)((s32)puVar5 + 10) = 0;
//    *(u8 *)((s32)puVar5 + 0xe) = 0;
}

void liveprog(s8 *param_1,u16 *param_2,s8 *param_3)
{
//    s16 unaff_D5w;
//
//    s16 sVar3 = alis.dernent;
//    s32 iVar6 = alis.atent;
//    u16 uVar2 = *(u16 *)(alis.atent + 4 + (s32)unaff_D5w);
//    *(s16 *)(alis.atent + 4 + (s32)unaff_D5w) = alis.dernent;
//    s32 iVar1 = (s32)alis.dernent;
//    alis.dernent = *(u16 *)(iVar6 + 4 + (s32)alis.dernent);
//    *(u16 *)(iVar6 + 4 + iVar1) = uVar2;
//    alis.nbent ++;
//    if (alis.maxent < alis.nbent)
//    {
////        perror(param_1, 5, sVar3, (s32)param_2);
//        return;
//    }
//
//    iVar6 = (s16)param_2[9] + alis.finent;
//    u32 *puVar7 = (u32 *)((s16)param_2[0xb] + iVar6);
//    s32 iVar8 = (s32)puVar7 + 0x3e;
//    *(s32 *)(alis.atent + sVar3) = iVar8;
//    u32 uVar4 = (u32)(u16)param_2[10];
//    iVar1 = uVar4 + iVar8;
//    if (alis.debsprit <= (s32)(uVar4 + iVar8))
//    {
////        perror(param_3, 0xc, sVar3, (s32)param_2);
//        return;
//    }
//
//    do
//    {
//        alis.finent = iVar1;
//        *(u16 *)((s32)puVar7 + uVar4 + 0x3c) = 0;
//        uVar4 = uVar4 - 2;
//        iVar1 = alis.finent;
//    }
//    while (uVar4 != 0);
//
//    s16 sVar5 = (s16)iVar6 - (s16)iVar8;
//    *(s16 *)(puVar7 + 0xd) = sVar5;
//    *(s16 *)((s32)puVar7 + 0x22) = sVar5;
//    *(s16 *)(puVar7 + 8) = sVar5;
//    *(u32 *)((s32)puVar7 + 0x36) = (u16)param_2[2] + 2 + (s32)param_2;
//    *(u16 *)((s32)puVar7 + 0x2e) = *param_2;
//    *(u16 **)((s32)puVar7 + 0x2a) = param_2;
//    *(u8 *)(puVar7 + 4) = *(u8 *)(param_2 + 1);
//    *(u8 *)(puVar7 + 0xf) = 1;
//    *(u8 *)((s32)puVar7 + 0x3d) = 1;
//    *(u8 *)((s32)puVar7 + 0x3a) = 0xff;
//    *(u16 *)(puVar7 + 9) = 0xffff;
//    *(s16 *)(puVar7 + 0xc) = sVar3;
//    *(u8 *)((s32)puVar7 + 0x1a) = 6;
//    *(u16 *)((s32)puVar7 + 0x26) = 0;
//    *(u16 *)((s32)puVar7 + 0x32) = 0;
//    *(u16 *)(puVar7 + 7) = 0;
//    *(u16 *)((s32)puVar7 + sprite_clinking) = 0;
//    *(u8 *)((s32)puVar7 + 0x3b) = 0;
//    *(u8 *)((s32)puVar7 + 0x19) = 0;
//    *(u16 *)(puVar7 + 2) = 0;
//    *(u16 *)((s32)puVar7 + 6) = 0;
//    *(u8 *)(puVar7 + 6) = 0;
//    *puVar7 = 0;
//    *(u16 *)(puVar7 + 1) = 0;
//    *(u8 *)(puVar7 + 5) = 0;
//    *(u8 *)((s32)puVar7 + 0x12) = 0;
//    *(u8 *)((s32)puVar7 + 0x11) = 0;
//    *(u16 *)((s32)puVar7 + 0x16) = 0;
//    *(u8 *)((s32)puVar7 + 0xf) = 0;
//    *(u16 *)(puVar7 + 3) = 0;
//    *(u16 *)((s32)puVar7 + 10) = 0;
//    *(u8 *)((s32)puVar7 + 0xe) = 0;
}

void initent(void)
{
    s32 at = alis.atent;
    s32 len = 0;
    
    do
    {
        len += 6;
        at += 6;
        *(s16 *)(alis.spritemem + at - 2) = len;
        *(u32 *)(alis.spritemem + at - 6) = 0;
    }
    while (at < alis.debent); // alis.debent 00 04
  
    *(u16 *)(alis.spritemem + alis.atent + 4) = 0;
    alis.nbent = 1;
    alis.dernent = 6;
}

void savecoord(u8 *a6)
{
    // ishar 1&2
    oldcx = *(u16 *)(a6 + 0);
    oldcy = *(u16 *)(a6 + 2);
    oldcz = *(u16 *)(a6 + 4);

//    //ishar 3
//    oldcx = *(u16 *)(a6 + 0x00);
//    oldcy = *(u16 *)(a6 + 0x08);
//    oldcz = *(u16 *)(a6 + 0x10);
//    oldacx = *(u16 *)(a6 + 0x18);
//    oldacy = *(u16 *)(a6 + 0x20);
//    oldacz = *(u16 *)(a6 + 0x28);
}

void updtcoord(u8 *a6)
{
    // ishar 1&2
    s16 addx = *(s16 *)(a6 + 0) - oldcx;
    s16 addy = *(s16 *)(a6 + 2) - oldcy;
    s16 addz = *(s16 *)(a6 + 4) - oldcz;
    if (addz != 0 || addx != 0 || addy != 0)
    {
        for (SpriteVariables *sprite = SPRITE_VAR(alis.script->context._0x18_unknown); sprite != NULL; sprite = SPRITE_VAR(sprite->to_next))
        {
            if (sprite->state == 0)
                sprite->state = 2;
            
            sprite->depx += addx;
            sprite->depy += addy;
            sprite->depz += addz;
        }
    }
    
    // ishar 3
//    s16 angle = *(u16 *)(a6 + 0x18);
//    if (angle != oldacx)
//    {
//        if ((0x168 < angle) && 0x168 < (angle = angle - 0x168))
//            angle = angle % 0x168;
//
//        if (angle < -0x168 && (angle += 0x168) < -0x168)
//            angle = angle % 0x168;
//
//        *(s16 *)(a6 + 0x18) = angle;
//    }
//
//    angle = *(u16 *)(a6 + 0x20);
//    if (angle != oldacy)
//    {
//        if ((0x168 < angle) && 0x168 < (angle = angle - 0x168))
//            angle = angle % 0x168;
//
//        if (angle < -0x168 && (angle += 0x168) < -0x168)
//            angle = angle % 0x168;
//
//        *(s16 *)(a6 + 0x20) = angle;
//    }
//
//    angle = *(u16 *)(a6 + 0x28);
//    if (angle != oldacy)
//    {
//        if ((0x168 < angle) && 0x168 < (angle = angle - 0x168))
//            angle = angle % 0x168;
//
//        if (angle < -0x168 && (angle += 0x168) < -0x168)
//            angle = angle % 0x168;
//
//        *(s16 *)(a6 + 0x28) = angle;
//    }
//
//    s16 addx = *(s16 *)(a6 + 0x00) - oldcx;
//    s16 addy = *(s16 *)(a6 + 0x08) - oldcy;
//    s16 addz = *(s16 *)(a6 + 0x10) - oldcz;
//    u16 unknown28 = *(u16 *)(a6 + 0x28);
//
//    if (angle != oldacz || addz != 0 || addx != 0 || addy != 0)
//    {
//        for (SpriteVariables *sprite = SPRITE_VAR(alis.script->context._0x18_unknown); sprite != NULL; sprite = SPRITE_VAR(sprite->to_next))
//        {
//            if (sprite->state == 0)
//                sprite->state = 2;
//
//            sprite->depx += addx;
//            sprite->depy += addy;
//            sprite->depz += addz;
//            sprite->sprite_0x28 = unknown28;
//        }
//    }
}


void setprog(u32 param_1, u16 *script_start, u32 param_3, s8 *param_4)
{
//    u16 **ppuVar6;
//    u16 **ppuVar5;
//    u16 **ppuVar7;
//
//    if ((param_1 & 1) != 0)
//    {
//        param_1 = param_1 + 1;
//    }
//
//    finprog = param_1 + finprog;
//    if (finmem < finprog)
//    {
////        perror(param_4, 0xb, (s16)param_1, (s32)param_2);
//        return;
//    }
//
//    u16 uVar1 = *script_start;
//    u16 **ppuVar4 = atprog;
//    do
//    {
//        ppuVar5 = ppuVar4;
//        ppuVar4 = ppuVar5 + 1;
//        if (dernprog < ppuVar4)
//            goto setprog4;
//
//        ppuVar6 = dernprog;
//    }
//    while (**ppuVar5 < uVar1);
//
//    do
//    {
//        ppuVar7 = ppuVar6 + -1;
//        *ppuVar6 = *ppuVar7;
//        ppuVar6 = ppuVar7;
//    }
//    while ((s32)ppuVar4 <= (s32)ppuVar7);
//
//setprog4:
//
//    *ppuVar5 = script_start;
//    dernprog = dernprog + 1;
//    nbprog = nbprog + 1;
//    if (maxprog < (s16)nbprog)
//    {
////        perror(param_4, 0x13, uVar1, (s32)param_2);
//        return;
//    }
//
////    uVar2 = io_pixel(param_3 & 0xffff0000 | (u32)nbprog,param_1 & 0xffff0000 | (u32)uVar1, (s32)script_start);           // D0 00430001 D1 00000000 A0 0004D400 (61f00) looks like sprite address
////    iVar3 = io_digit(uVar2,(s32)script_start);
//    if ((((nmode == '\x01') || (nmode == '\x03')) || (nmode == '\x05')) || (nmode == '\x02'))
//    {
////        io_tomono(iVar3,(s32)script_start);
//    }
}

u32 loadprog(void *param_1, size_t param_2, s8 *script, s8 *param_4)
{
//    s8 cVar1;
//    u32 uVar2;
//    s32 iVar3;
//    u32 uVar5;
//    s16 sVar7;
//    void *pvVar6;
//    u16 unaff_D2w;
//    u16 *puVar10;
//    u32 *puVar11;
//
//    // get platform s8
//
//    s8 platform = 'F';
//    if (nmode != '\b')
//    {
//        if (a1200)
//        {
//            platform = 'D';
//        }
//        else if (amiga)
//        {
//            platform = 'C';
//        }
//        else
//        {
//            platform = 'A';
//        }
//    }
//
//    // replace platform s8 in script path
//
//    s8 *ext = 0;
//    s8 *pl_script = sd7;
//
//    do
//    {
//        cVar1 = *script;
//        *pl_script = cVar1;
//        if (cVar1 == '.')
//        {
//            ext = pl_script + 1;
//        }
//
//        script = script + 1;
//        pl_script = pl_script + 1;
//    }
//    while (cVar1 != '\0');
//
//    if (ext != 0)
//    {
//        *ext = platform;
//    }
//
//    pl_script = sd7;
//
//    // unpack and read
//
//    void *data;
//
//    u8 is_packed = unaff_D2w == 0;
////    if (is_packed)
////    {
////        sVar7 = 6;
////        puVar11 = &buffer;
////        iVar3 = open(param_4,(s32)param_1);
////        data = read((s32)param_4,param_1,param_2,iVar3,sVar7,(s32)puVar11);
////    }
////    else
////    {
////        uVar2 = debprotf();
////        if (is_packed)
////        {
////            return uVar2;
////        }
////
////        sVar7 = 6;
////        puVar11 = &buffer;
////        typent = unaff_D2w;
////        iVar3 = open(param_4,(s32)param_1);
////        data = (void *)read((s32)param_4,param_1,param_2,iVar3,sVar7,(s32)puVar11);
////        if (DAT_0001fd04._0_2_ != 0)
////        {
////            typepack = g_buffer._0_1_;
////            puVar10 = finprog;
////            if ((s32)buffer < 0)
////            {
////                pvVar6 = (void *)((buffer & 0xffffff) - 6);
////                pvVar4 = unpack(pvVar4,pvVar6,(s8 *)finprog);
////            }
////            else
////            {
////                pvVar6 = (void *)((buffer & 0xffffff) - 6);
////                pvVar4 = (void *)read((s32)param_4,param_1,param_2,(ssize_t)pvVar4,(s16)pvVar6, (s32)finprog);
////            }
////
////            iVar3 = close((s32)param_4,(s32)data,(s16)pvVar6,(s32)puVar10);
////            uVar5 = debprot(iVar3);
////            bVar13 = *puVar10 == typent;
////            if (!is_packed)
////            {
////                debprotf();
////
////                if (is_packed)
////                {
////                    return uVar5;
////                }
////            }
////
////            setprog((u32)pvVar6,puVar10,uVar5);
////            return uVar5;
////        }
////    }
////
////    read((s32)param_4,param_1,param_2,(ssize_t)data,0x10,0x1fd06);
////    maxprog = DAT_0001fd04._2_2_;
//    atprog = basemem;
//    if (basevar != 0)
//    {
//        atprog = basevar;
//    }
//
////    alis.atent = atprog + (u32)DAT_0001fd04._2_2_ * 2;   // 00 04
//    alis.atent = atprog + 0x7fff;
//    nbprog = 0;
//    fswitch = 0;
//    timing = 0;
//    ptscreen = 0;
//    vprotect = 0;
//    mousflag = 0;
//    fmouse = 0xff;
//    fonum = 0xffff;
//    sback = 0;
////    alis.maxent = DAT_0001fd08;    // 00 6e
////    alis.debent = alis.atent + (u32)DAT_0001fd08 * 3;
////    alis.debsprit = ((u32)((s32)alis.debent + ram0x0001fd0e) | 0xf) + 1;
////    alis.finsprit = (u16 *)(alis.debsprit + (DAT_0001fd12 + 3U & 0xffff) * 0x30);
//    alis.maxent = 0x6e;
//    alis.debent = alis.atent + 0x7fff;
//
//    alis.debsprit = alis.debent;
//    alis.finsprit = alis.debent + 0x7fff;
//
//    debprog = alis.finsprit;
//    if (basevar != 0)
//    {
//        debprog = basemem;
//    }
//
////    s8 *mem = (s8 *)((s32)debprog + _DAT_0001fd0a);
////    if (mem <= finmem)
////    {
////        finmem = mem;
////    }
//
//    finprog = debprog;
//    dernprog = atprog;
//    alis.finent = alis.debent;
//    initent();
//    inisprit_sm();
////    typepack = buffer._0_1_;
//    puVar10 = finprog;
////    if ((s32)buffer < 0)
////    {
////        pvVar6 = (void *)((buffer & 0xffffff) - 0x16);
////        pvVar4 = unpack(pvVar4,pvVar6,(s8 *)finprog);
////    }
////    else
////    {
////        pvVar6 = (void *)((buffer & 0xffffff) - 0x16);
////        pvVar4 = (void *)read((s32)param_4,param_1,param_2,(ssize_t)pvVar4,(s16)pvVar6,(s32)finprog);
////    }
//
////    uVar5 = close((s32)param_4,(s32)data,(s16)pvVar6,(s32)puVar10);
////    setprog((u32)pvVar6,puVar10,uVar5);
////    livemain(puVar10, 0, (s8 *)0x1cf6e);
////    *(u16 *)(pcVar9 + -0x16) = 0;
////    alis.basemain = pcVar9;
//    return uVar5;
    return 0;
}

void setmpalet(void)
{
    ftopal = 0xff;
    thepalet = 0;
    defpalet = 0;
    return;
}

void bootmain(void *param_1,size_t param_2,s8 *script,u32 param_4)
{
//  io_main(param_4);
  setmpalet();
  loadprog(param_1,param_2,script,(s8 *)0x1cd6a);
  return;
}

void FUN_STARTUP(s8 *script, size_t param_2, u32 param_3)
{
    void *sp = NULL;
    
    u32 uVar1 = 0;

    atpalet = tpalet;         // 1024 bytes empty pal
    ampalet = mpalet;         // 1024 bytes empty pal
    ptabfen = tabfen;        // &tabfen == &tablent ?
    alis.ptrent = alis.tablent;
    fsound = 1;
    fmusic = 1;
    savsp = (void *)&sp;
    inisys();
//   uVar1 = inicanaux(); audio???
    bootmain((void *)0x1680a, param_2, script, uVar1);
//  moteur2(alis.atent,uVar1,param_3);
    return;
}

// NOTE: just copy already read data to my local spritemem
void OPCODE_CDEFSC_0x46(u8 *ptr, u16 screen)
{
    memcpy(BASEMNMEM_PTR + 0x06 + screen, ptr + screen + 0x06, 32);

    if (alis.libsprit == 0)
        return;

    SceneVariables *scene = SCENE_VAR(screen);
    scene->state = *(u8 *)(ptr + 0x00 + screen);
    scene->numelem = *(u8 *)(ptr + 0x01 + screen);
    scene->screen_id = alis.libsprit;
    scene->to_next = 0;
    scene->newx = read16((u8 *)&(scene->newx), alis.platform.is_little_endian);
    scene->newy = read16((u8 *)&(scene->newy), alis.platform.is_little_endian);
    scene->width = read16((u8 *)&(scene->width), alis.platform.is_little_endian);
    scene->height = read16((u8 *)&(scene->height), alis.platform.is_little_endian);
    scene->unknown0x0a = read16((u8 *)&(scene->unknown0x0a), alis.platform.is_little_endian);
    scene->unknown0x0c = read16((u8 *)&(scene->unknown0x0c), alis.platform.is_little_endian);
    scene->unknown0x2a = 0;
    scene->unknown0x2c = 0;
    scene->unknown0x2e = 0;

    SpriteVariables *sprite = SPRITE_VAR(alis.libsprit);
    sprite->link = 0;
    sprite->numelem = scene->numelem;
    sprite->newx = scene->newx;// & 0xfff0;
    sprite->newy = scene->newy;
    sprite->newd = 0x7fff;
    sprite->depx = scene->newx + scene->width;// | 0xf;
    sprite->depy = scene->newy + scene->height;

    printf("\n0x%.4x\n", ELEMIDX(alis.libsprit));
    printf("0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", scene->newx, scene->newy, scene->width, scene->height);
    printf("0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", sprite->newx, sprite->newy, sprite->depx, sprite->depy);

    alis.libsprit = sprite->to_next;

    scadd(screen);
    vectoriel(screen);
}

void scadd(s16 screen)
{
    SceneVariables *scene = SCENE_VAR(screen);
    scene->to_next = 0;
    
    s16 nextscreen = ptscreen;
    if (ptscreen == 0)
    {
        ptscreen = screen;
        return;
    }
    
    s16 curscreen;

    do
    {
        curscreen = nextscreen;
        nextscreen = SCENE_VAR(curscreen)->to_next;
    }
    while (nextscreen != 0);
    
    SCENE_VAR(curscreen)->to_next = screen;
    return;
}

void scbreak(s16 screen)
{
    if (ptscreen == 0)
        return;
    
    SceneVariables *scrscene = SCENE_VAR(screen);
    
    s16 prevptscreen = ptscreen;
    if (screen == ptscreen)
    {
        ptscreen = scrscene->to_next;
        return;
    }
    
    s16 nextptscreen;

    do
    {
        SceneVariables *prevscene = SCENE_VAR(prevptscreen);
        nextptscreen = prevscene->to_next;
        if (nextptscreen == screen)
        {
            prevscene->to_next = scrscene->to_next;
            return;
        }
        
        prevptscreen = nextptscreen;
    }
    while (nextptscreen != 0);
}

void scdosprite(s16 screen)
{
    SceneVariables *scene = SCENE_VAR(screen);
    s16 spritidx = scene->screen_id;

    SpriteVariables *sprite = SPRITE_VAR(spritidx);
    sprite->numelem = scene->numelem;
    sprite->newx = scene->newx;// & 0xfff0;
    sprite->newy = scene->newy;
    sprite->newd = 0x7fff;
    sprite->depx = scene->newx + scene->width;// | 0x0f;
    sprite->depy = scene->newy + scene->height;
}

void vectoriel(u16 screen)
{
    SceneVariables *scene = SCENE_VAR(screen);
    scene->unknown0x26 = scene->unknown0x21 * scene->unknown0x25 - scene->unknown0x24 * scene->unknown0x22;
    scene->unknown0x27 = scene->unknown0x22 * scene->unknown0x23 - scene->unknown0x25 * scene->unknown0x20;
    scene->unknown0x28 = scene->unknown0x20 * scene->unknown0x24 - scene->unknown0x23 * scene->unknown0x21;
}

// NOTE: not that usefull on modern systems as it converts to 2bit atari planar
s32 io_tomono(s32 param_1, s32 script_start)
{
//    u8 bVar3;
//    u16 uVar4;
//    u16 uVar5;
//    u16 uVar6;
//    u16 uVar8;
//    u16 uVar9;
//    u16 uVar11;
//    u16 uVar12;
//    u16 uVar13;
//    u16 uVar14;
//    s16 width;
//    u16 height;
//    u16 *img_data;
//    s8 *img;
//
//    u8 *mem = alis.mem + script_start;
//
//    uint32_t laddress = read32(mem + 0xe, alis.platform);
//
//    //s32 laddress = read32(alis.mem + script_start + 0xe, alis.platform) + script_start;
//    s16 elements = read16(mem + laddress + 0x04, alis.platform); // if
//    u16 tPuVar21 = read16(mem + laddress + sprite_data, alis.platform) + laddress + 0x20;
//
//    // s32 *address = (s32 *)(*(s32 *)(script_start + 0xe) + script_start);
//    if (elements)
//    {
//        u32 tVar21 = read16(mem + tPuVar21, alis.platform);
//        u16 *puVar21 = (u16 *)(mem + tPuVar21);
//        img_data = puVar21;
//
//        if ((nmode == '\x03') || (nmode == '\x01'))
//        {
//            for (s32 i = 0; i < 16; i++, img_data++)
//            {
//                *img_data = (read16((u8 *)img_data, alis.platform) & 0xf0f) | 0xc0c;
//            }
//        }
//        else
//        {
//            for (s32 i = 0; i < 16; i++, img_data++)
//            {
//                *img_data = read16((u8 *)img_data, alis.platform) >> 4 & 0xf0f;
//            }
//        }
//
//        u16 uVar2 = *puVar21;
//
////        s32 *address = (s32 *)(alis.mem + laddress);
////        address += laddress;
//
//        for (s32 e = 0; e < elements; e++)
//        {
//            u32 addr = e * 4 + read32(mem + laddress, alis.platform) + laddress;
//            u32 b = addr + read32(mem + addr, alis.platform);
//
////            pcVar19 = (s8 *)(*address + (s32)address);
//            img = (s8 *)mem + b;
//            if (*img == '\0')
//            {
//                *puVar21 = 0;
//                if (nmode == '\x03')
//                {
//                    img[1] = img[1] & 0xef;
//                    img[1] = img[1] | 0x20;
//                }
//io_tomon2:
//                if (((nmode == '\x01') || (nmode == '\x05')) || (nmode == '\x02'))
//                {
//                    img[1] = '\0';
//                }
//
//                height = read16((u8 *)(img + 4), alis.platform) + 1; // *(u16 *)(pcVar19 + 4);
//                width = ((read16((u8 *)(img + 2), alis.platform) + 1) >> 4); // ((u16)(*(s16 *)(pcVar19 + 2) + 1U) >> 4) - 1;
//                img_data = (u16 *)(img + 6);
//
//                for (s32 p = 0; p < width * height; p++)
//                {
//                    uVar8   = img_data[0];
//                    uVar9   = img_data[1];
//                    uVar11  = img_data[2];
//                    uVar12  = img_data[3];
//
//                    for (s32 b = 0xf; b >= 0; b--)
//                    {
//                        uVar4 = uVar12 & 1;
//                        uVar12 = uVar12 >> 1;
//                        uVar14 = uVar11 & 1;
//                        uVar11 = uVar11 >> 1;
//                        uVar5 = uVar9 & 1;
//                        uVar9 = uVar9 >> 1;
//                        uVar6 = uVar8 & 1;
//                        uVar8 = uVar8 >> 1;
//                        uVar4 = (u16)(uVar6 != 0) | ((u16)(uVar5 != 0) | ((u16)(uVar14 != 0) | (u16)(uVar4 != 0) << 1) << 1 ) << 1;
//                        uVar14 = uVar4 << 1;
//
//                        if (uVar4 != 0)
//                        {
//                            if ((b & 1) != 0)
//                            {
//                                uVar14 = uVar14 ^ 1;
//                            }
//
//                            if ((height & 1) != 0)
//                            {
//                                uVar14 = uVar14 ^ 1;
//                            }
//
//                            bVar3 = *(u8 *)(puVar21 + uVar14);
//                            uVar14 = (u16)bVar3;
//
//                            if ((bVar3 & 1) != 0)
//                            {
//                                uVar8 = uVar8 | 0x8000;
//                            }
//
//                            if ((bVar3 & 2) != 0)
//                            {
//                                uVar9 = uVar9 | 0x8000;
//                            }
//                        }
//
//                        if ((uVar14 & 4) != 0)
//                        {
//                            uVar11 = uVar11 | 0x8000;
//                        }
//
//                        if ((uVar14 & 8) != 0)
//                        {
//                            uVar12 = uVar12 | 0x8000;
//                        }
//                    }
//
//                    img_data[0] = uVar8;
//                    img_data[1] = uVar9;
//                    img_data[2] = uVar11;
//                    img_data[3] = uVar12;
//                    img_data += 4;
//                }
//            }
////            else
////            {
////                if (*img == '\x02')
////                {
////                    *puVar21 = uVar2;
////
////                    if (nmode == '\x03')
////                    {
////                        img[1] = img[1] | 0x10;
////                    }
////
////                    goto io_tomon2;
////                }
////
////                if (*img == '\x01')
////                {
////                    img[1] = *(s8 *)((s32)puVar21 + (s32)(s16)((u16)(u8)img[1] * 2));
////                }
////            }
//
//            // address = address + 1;
//        }
//    }
    
    return param_1;
}

s16 tabint(s16 offset)
{
    s16 *a0 = (s16 *)vram_ptr(offset - 2);
    s16 result = offset + alis.varD7 * 2;
    s16 length = *(s8 *)vram_ptr(offset - 1);
    
    for (int i = 0; i < length; i++, alis.acc++, a0--)
    {
        result += *(alis.acc) * *a0;
    }
    
    return result;
}

void moteur4(s16 offset)
{
    alis.varD5 = offset;
    
    u32 index = alis.script_vram_orgs[offset / sizeof(sScriptLoc)].vram_offset;
    sAlisScript *script = alis.scripts[index];
    
//    // HACK!!!
//    if (strcmp(script->name, "preson.ao") == 0)
//    {
//        offset = alis.script_vram_orgs[offset / sizeof(sScriptLoc)].offset;
//        index = alis.script_vram_orgs[offset / sizeof(sScriptLoc)].vram_offset;
//        script = alis.scripts[index];
//    }
    
    alis.script = script;

    u8 *vram_addr = alis.mem + alis.script->vram_org;

    alis.fallent = 0;
    alis.fseq = 0;
    
    if (alis.script->context._0x24_scan_inter.data < 0 && (alis.script->context._0x24_scan_inter.data & 2) == 0)
    {
        s32 script_offset = swap32((alis.mem + alis.script->context._0x14_script_org_offset + 10), alis.platform.is_little_endian);
        if (script_offset != 0)
        {
            alis.saversp = alis.script->context._0x10_script_id;
            savecoord(vram_addr);
            alis.script->pc = alis.script->context._0x14_script_org_offset + 10 + script_offset;
            alis_loop();
//            check(0x027a90 - 2);
            updtcoord(vram_addr);
        }
    }
}

void moteur3(s16 offset)
{
    u8 *vram_addr = alis.mem + alis.script->vram_org;
    
    savecoord(vram_addr);
    alis.script->pc = alis.script->context._0x8_script_ret_offset;
    
//    u32 xx = offset == 0 ? 0x2edf0 : 0x034bc0;
//    printf("\n %s PRE  A3 0x%.6x\n", alis.script->name, alis.script->pc - alis.script->pc_org + xx);
    
    // on real HW its assigned to d4w and later modified while doing reeadexec
    // and than stored with new value
    // set to d4w and can be changed in OPCODE_CJSR24_0x07 and likely others
    u16 unknown0x10 = alis.script->context._0x10_script_id;
    alis.fseq ++;
    alis_loop();
//    check(0x027a90 - 2);
    alis.script->context._0x10_script_id = unknown0x10;
    alis.script->context._0x8_script_ret_offset = alis.script->pc;

//    printf("\n %s POST A3 0x%.6x\n", alis.script->name, alis.script->pc - alis.script->pc_org + xx);

    s32 script_offset = swap32(alis.mem + alis.script->context._0x14_script_org_offset + 6, alis.platform.is_little_endian);
    if (script_offset != 0)
    {
        alis.fseq = 0;
        alis.saversp = alis.script->context._0x10_script_id;
        alis.script->pc = alis.script->context._0x14_script_org_offset + 6 + script_offset;
        alis_loop();
//        check(0x027a90 - 2);
    }
    
    updtcoord(vram_addr);
}

u16 moteur1(s16 offset)
{
    do
    {
        do
        {
            do
            {
                // TODO: call it from a timer?
                itroutine();
                
                offset = alis.script_vram_orgs[offset / sizeof(sScriptLoc)].offset;
                if (offset == 0)
                {
                    image();
                    return 0;
                }
                
                moteur4(offset);
            }
            while (alis.script->context._0x4_cstart_csleep == 0);
            
            if ((s8)alis.script->context._0x4_cstart_csleep < 0)
            {
                alis.script->context._0x4_cstart_csleep = 1;
            }
            
            alis.script->context._0x1_cstart --;
        }
        while (alis.script->context._0x1_cstart != 0);
        
        moteur3(offset);
        
        alis.script->context._0x1_cstart = alis.script->context._0x2_unknown;
    }
    while (true);
    
    return 0;
}

void moteur2(s16 offset)
{
    do
    {
        // TODO: call it from a timer?
        itroutine();
        
        moteur4(offset);
        
        if (alis.script->context._0x4_cstart_csleep != 0)
        {
            if ((s8)alis.script->context._0x4_cstart_csleep < 0)
            {
                alis.script->context._0x4_cstart_csleep = 1;
            }
            
            alis.script->context._0x1_cstart --;
            if (alis.script->context._0x1_cstart == 0)
            {
                moteur3(offset);

                alis.script->context._0x1_cstart = alis.script->context._0x2_unknown;

                offset = moteur1(offset);
                continue;
            }
        }
        
        offset = alis.script_vram_orgs[offset / sizeof(sScriptLoc)].offset;
        if (offset == 0)
        {
            image();
        }
    }
    while (true);
}
    
s16 debprotf(u16 script_id)
{
    for (int i = 0; i < alis.nbprog; i ++)
    {
        if (alis.scripts[i]->header.id == script_id)
            return i;
    }
        
    return -1;
}

void shrinkprog(sAlisScript *script, s8 unload_script)
{
                // a0 = 34ba8
                // a1 = 22404
                // a2 = 39df0
                // d1 = 5248
                // d2 = 1
                // d5 = 0
                // d7 = 1
    
    // finprog -= script->sz;

//    u16 previdx = 0;
//    u16 curidx = alis.script->context._0x18_unknown;
//    while (curidx != 0)
//    {
//        cursprvar = SPRITE_VAR(curidx);
//        u16 screen_id = cursprvar->screen_id;
//        u16 state = cursprvar->state;
//        u16 link = cursprvar->link;
//        u32 clink = cursprvar->clinking;
//
//        ELEM_TRACE("    N: 0x%.4x L: 0x%.4x Sc: 0x%.4x St: 0x%.2x Cl: 0x%.8x\n", ELEMIDX(curidx), ELEMIDX(link), ELEMIDX(screen_id), state, clink);
//
//        if (link == curidx)
//        {
//            ELEM_TRACE("ERROR: link = 0\n");
//            result = false;
//        }
//
//        if (link)
//        {
//            u16 link2 = SPRITE_VAR(link)->link;
//
//            if (link2 == curidx)
//            {
//                ELEM_TRACE("ERROR: infinite loop\n");
//                result = false;
//                cursprvar->link = 0;
//            }
//        }
//
//        previdx = curidx;
//        curidx = cursprvar->to_next;
//    }

//    SpriteVariables *cursprvar;

    if (unload_script != 0)
    {
//        alis.dernprog -= 4;
        alis.nbprog --;
        
//        u32 a0 = 0x34ba8;
//        u16 d5w = 0;
//
//        u16 curidx = 0;
//        u16 iVar8;
//        u16 sVar9;
//        cursprvar = SPRITE_VAR(curidx);
//        for (curidx = cursprvar->to_next; curidx != 0; curidx = cursprvar->to_next)
//        {
//            cursprvar = SPRITE_VAR(curidx);
//
//            // script
//            iVar8 = *(s32 *)(alis.mem + alis.atent + curidx);
//            if (script_id == alis.script->context._0x10_script_id)
//            {
//                killent(curidx, (u32)a0, d5w);
//                curidx = (u32)a0 & 0xffff;
//            }
//            else if ((s32)a0 <= alis.script->context._0x14_script_org_offset)
//            {
//                alis.script->context._0x14_script_org_offset -= script_size;
//                alis.script->context._0x8_script_ret_offset -= script_size;
//                u16 test = alis.script->context._0x14_script_org_offset + 0x16;
//                sVar9 = -0x34 - *(s16 *)(alis.mem + alis.script->context._0x14_script_org_offset + 0x16);
//                while (alis.script->context._0x10_script_id < sVar9)
//                {
//                    sVar9 -= 4;
//                    *(s32 *)(alis.mem + iVar8 + sVar9) -= script_size;
//                }
//            }
//        }
    }


//    u16 d5w;
//    u16 uVar1;
//    s32 iVar4;
//    u32 uVar5;
//    s16 sVar6;
//
//    // move headers into space freed by closing script
////    for (s16 *psVar7 = a0; (s32)a2 < finprog; psVar7++, a2++)
////    {
////        *psVar7 = *a2;
////    }
//
//    // shrink used mem by script size
//    finprog -= script_size;
//
//    if (unload_script != 0)
//    {
//        // shift script addresses to fill released spcace
////        for (u32 *puVar10 = (u32 *)(a1 + 4); (s32)puVar10 < dernprog; puVar10++)
////        {
////            puVar10[-1] = puVar10[0];
////        }
//
//        dernprog -= 4;
//        nbprog --;
//    }
//
//    s32 *piVar12 = atprog;
//
//    // shift actual script
////    do
////    {
////        if ((s32)a0 < *piVar12)
////        {
////            *piVar12 = *piVar12 - script_size;
////        }
////
////        piVar12 ++;
////    }
////    while ((s32)piVar12 < dernprog);
//
//    s32 iVar8 = alis.basesprite;
//    s32 iVar2 = alis.basemain;
//    s16 sVar9 = ptscreen;
//
//    if (unload_script != 0)
//    {
//        uVar5 = 0;
//        while (true)
//        {
//            uVar1 = *(u16 *)(alis.atent + 4 + uVar5);
//            uVar5 = (u32)uVar1;
//            iVar8 = alis.basesprite;
//            iVar2 = alis.basemain;
//            sVar9 = ptscreen;
//            if (uVar1 == 0)
//                break;
//
//            // script
//            iVar8 = *(s32 *)(alis.atent + (s16)uVar1);
//            if (script_id == alis.script->context._0x10_script_id)
//            {
//                killent(uVar1, (u32)a0, d5w);
//                uVar5 = (u32)a0 & 0xffff;
//            }
//            else if ((s32)a0 <= alis.script->context._0x14_script_org_offset)
//            {
//                alis.script->context._0x14_script_org_offset -= script_size;
//                alis.script->context._0x8_script_ret_offset -= script_size;
//                sVar9 = -0x34 - *(s16 *)(alis.script->context._0x14_script_org_offset + 0x16);
//                while (alis.script->context._0x10_script_id < sVar9)
//                {
//                    sVar9 -= 4;
//                    *(s32 *)(iVar8 + sVar9) -= script_size;
//                }
//            }
//        }
//    }
//
//    s16 *psVar7;
//    s32 iVar3 = alis.atent;
//
//    for (; iVar4 = alis.basemain, iVar3 = alis.basesprite, sVar9 != 0; sVar9 = *(s16 *)(iVar4 + 4 + (s32)sVar9))
//    {
//        sVar6 = *(s16 *)(alis.basemain + 2 + (s32)sVar9);
//        alis.basesprite = iVar8;
//        alis.basemain = iVar2;
//
//        while (sVar6 = *(s16 *)(iVar3 + 6 + (s32)sVar6), sVar6 != 0)
//        {
//            psVar7 = (s16 *)(*(u32 *)(iVar3 + 8 + (s32)sVar6) & 0xffffff);
//            if (a0 < psVar7)
//            {
//                iVar8 = (s32)psVar7 - script_size;
//                *(s16 *)(iVar3 + 10 + (s32)sVar6) = (s16)iVar8;
//                *(s8 *)(iVar3 + 9 + (s32)sVar6) = (s8)((u32)iVar8 >> 0x10);
//            }
//
//            psVar7 = (s16 *)(*(u32 *)(iVar3 + 0x12 + (s32)sVar6) & 0xffffff);
//            if (a0 < psVar7)
//            {
//                iVar8 = (s32)psVar7 - script_size;
//                *(s16 *)(iVar3 + 0x14 + (s32)sVar6) = (s16)iVar8;
//                *(s8 *)(iVar3 + 0x13 + (s32)sVar6) = (s8)((u32)iVar8 >> 0x10);
//            }
//        }
//
//        iVar8 = alis.basesprite;
//        iVar2 = alis.basemain;
//        alis.basemain = iVar4;
//        alis.basesprite = iVar3;
//    }
//
//    alis.basesprite = iVar8;
//    alis.basemain = iVar2;
}

void alis_putchar(s8 character)
{
    if (alis.charmode == 0)
    {
        // io_putchar(d0);
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
            alis.depz = alis.depz - (u16)alis.fohaut;
        }
        else if (character == '\r')
        {
            alis.depx = 0;
        }
        else
        {
            if (character != ' ')
            {
                if (alis.fonum < 0)
                {
                    return;
                }
                
                s16 charidx = ((u16)character) - alis.foasc;
                if ((-1 < charidx) && ((s16)(charidx - alis.fomax) < 0))
                {
                    alis.flagmain = 1;
                    alis.flaginvx = 0;
                    alis.fadddes = 1;
                    put(charidx + alis.fonum);
                }
            }
            
            alis.depx = (u16)alis.folarg + alis.depx;
        }
    }
}

void alis_putstring(void)
{
    for (u8 *strptr = alis.sd7; *strptr; strptr++)
    {
        alis_putchar(*strptr);
    }
}

void sparam(void)
{
    u8 *oldsd7 = alis.sd7;
    alis.sd7 = alis.sd6;
    alis.sd6 = oldsd7;
    readexec_opername();
}
