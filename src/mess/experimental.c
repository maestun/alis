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
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define BASEMNMEM_PTR alis.spritemem + basemain
#define SPRITEMEM_PTR alis.spritemem + basesprite

#define SPRITE_VAR(x) ((SpriteVariables *)(SPRITEMEM_PTR + x))
#define SCENE_VAR(x) ((SceneVariables *)(BASEMNMEM_PTR + x))

#define DBTRACE(f, ...) printf("%s " f, __FUNCTION__,  ## __VA_ARGS__); // __PRETTY_FUNCTION__
//#define ELEM_TRACE(x, ...)
#define ELEM_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#define LINK_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#define CLIP_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#define DRAW_TRACE(x, ...) DBTRACE(x,  ## __VA_ARGS__)
#define DEBUGFCE
// #define DEBUGFCE DBTRACE("\n")
#define ELEMIDX(x) ((((x - 0x78) / 0x30) * 0x28) + 0x8078) // return comparable number to what we see in ST debugger
#define VERIFYINTEGRITY
// #define VERIFYINTEGRITY verifyintegrity()

u8 tvmode = 0;
u8 flagmain = 0;

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
u8 tabfen = 0;
u8 *ptrent = 0;
u8 tablent = 0;


s32 bufrvb;

u8 numelem = 0;

u8 flaginvx = 0;
u8 fmuldes = 0;
u8 fadddes = 0;

s32 atent;
s32 debent;
s32 finent;
s32 maxent;
s16 nbent;
s16 dernent;

s32 basemain;
s32 basesprite;
u16 libsprit;
s32 debsprit;
s32 finsprit;
s32 backsprite;
s32 tvsprite;
s32 texsprite;
s32 atexsprite;
s32 mousprite;
s32 mousflag;

u16 depx = 0;
u16 depy = 0;
u16 depz = 0;

s32 oldcx;
s32 oldcy;
s32 oldcz;

s32 oldacx;
s32 oldacy;
s32 oldacz;

u8 priorson;
u8 typeson;
u16 longson;
u16 freqson;
u16 dfreqson;
u8 volson;
u16 dvolson;
u16 pereson;

s8 nmode = 0;
s8 falcon = 0;
s8 a1200 = 0;
s8 amiga = 0;
s8 atari = 0;
s8 fsound = 1;
s8 fmusic = 1;

s8 *sd7 = 0;
s8 *sd6 = 0;
s8 *oldsd7 = 0;
s8 *bsd7bis = 0;

s8 bsd7[256];
s8 bsd6[256];

s8 *savsp = 0;
u32 command = 0;
u32 debram = 0;
u32 finram = 0;
u32 savessp = 0;

u32 bufpack = 0x20a00;
u32 mworkbuff = 0x20a00;
u32 adtmono = 0x33700;
u32 finnoyau = 0x33900;
u32 basemem = 0x33900;
u32 basevar = 0;
u32 ophysic = 0x33900;

u16 nbprog = 0;
u8 fswitch = 0;
u8 timing = 0;
u16 ptscreen = 0;
u16 vprotect = 0;
s8 fmouse = 0xff;
u16 fonum = 0xffff;
u8 sback = 0;
u32 debprog = 0;

u32 finprog;
u32 dernprog;

u16 maxprog;

s32 ferase = 0;

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

u8 *physic;
// u8 *logic;
u8 logic[320*200];
u8  fphysic = 0;
u8  fphytolog = 0;
u8  fremouse = 0;
u8  fremap = 0;
u8  fremouse2 = 0;
u8  vtiming = 0;
s16 savmouse[2];
u32 savmouse2[8];
u8  switchgo;
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
u8 insid;
u8  spag = 0;
u8  wpag = 0;
u8 *backmap;
s32 backx1;
s32 backx2;
s16 backy1;
s16 backy2;

// to be consistent with old code
#pragma pack(1)

typedef union {
    
    struct {
        
        u8 state;
        u8 numelem;
        u16 screen_id;
        u16 to_next;
        u16 link;
        u32 newad:24;
        u8 newf:8;
        s16 newx;
        s16 newy;
        s16 newd;
        u32 data:24;
        u8 flaginvx:8;
        s16 depx;
        s16 depy;
        s16 depz;
        u8 credon_off;
        u8 creducing;
        u16 clinking;
        u8 cordspr;
        u8 chsprite;
        u16 cnoise;
        u32 sprite_0x28;
        u16 width;
        u16 height;
        u16 newzoomx;
        u16 newzoomy;
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

//typedef enum {
//
//    sprite_state                        = 0x00,
//    sprite_numelem                      = 0x01,
//    sprite_screen_id                    = 0x02,
//    sprite_to_next                      = 0x04,
//    sprite_link                         = 0x06,
//    sprite_newad                        = 0x08,
//    sprite_newf                         = 0x08 + 3,
//    sprite_newx                         = 0x0c,
//    sprite_newy                         = 0x0e,
//    sprite_newd                         = 0x10,
//    sprite_data                         = 0x12,
//    sprite_flaginvx                     = 0x12 + 3, // +0 on big endian
//    sprite_depx                         = 0x16,
//    sprite_depy                         = 0x18,
//    sprite_depz                         = 0x1a,
//    sprite_credon_off                   = 0x1c,
//    sprite_creducing                    = 0x1d,
//    sprite_clinking                     = 0x1e,
//    sprite_cordspr                      = 0x20,
//    sprite_chsprite                     = 0x21,
//    sprite_cnoise                       = 0x22,
//    sprite_0x28                         = 0x24,
//    sprite_width                        = 0x28,
//    sprite_height                       = 0x2a,
//    sprite_newzoomx                     = 0x2c,
//    sprite_newzoomy                     = 0x2e,
//
//} SpriteVar;

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
    if (flagmain != '\0')
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
    if (flagmain != '\0')
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

//        if (0x7d < duration)
//        {
//            pald = 1;
//            topalet();
//            palt = duration / 0x7e;
//            palt0 = palt;
//            palc = 0x7e;
//        }
//        else
//        {
//            pald = (0x7e / (duration & 0xffffU)) + 1;
//            topalet();
//            palt = 1;
//            palt0 = 1;
//            palc = (s8)duration;
//        }
    }
    else
    {
        memcpy(mpalet, atpalet, 256 * 3);
        ftopal = 0xff;
    }
}

void OPCODE_CTOPALET_0xbe(s32 palidx, s32 duration)
{
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
    
    flagmain = 0;
//  palidx = readexec_opername();
//  length = readexec_opername_saveD6();
    if ((((nmode == '\x01') || (nmode == '\x05')) || (nmode == '\x03')) || (nmode == '\a'))
    {
        return;
    }
    
    if (palidx < 0)
    {
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

void savecoord(u8 *at)
{
    oldcx = at[0x00];
    oldcy = at[0x04];
    oldcz = at[0x08];
    oldacx = at[0x0c];
    oldacy = at[0x10];
    oldacz = at[0x14];
}

bool verifyintegrity(void)
{
    DEBUGFCE;
    
    bool result = true;

    if (libsprit == 0)
    {
        ELEM_TRACE("ERROR: libsprit = 0!\n");
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
//    prevmouse = fremouse;
//    u8 *oldscr_ptr = physic;
    
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
    basemain = 0;
    s32 cursprit = debsprit;
    s32 base = debsprit + 0x8000;
    tvsprite = 0x8000;
    basesprite = base;
    *(s32 *)(alis.spritemem + cursprit + 0x0c) = 0;
    *(s16 *)(alis.spritemem + cursprit + 0x16) = 0x13f;
    *(s16 *)(alis.spritemem + cursprit + 0x18) = 199;
    *(s8 *) (alis.spritemem + cursprit + 0x29) = 0;
    backsprite = 0x000e; // 0x8028;
    texsprite = 0x0042; // 0x8050;
    *(s32 *)(alis.spritemem + cursprit + 0x5c) = 0;
    *(s8 *) (alis.spritemem + cursprit + 0x50) = 0xfe;
    mousflag = 0;
    libsprit = 0x78;
    cursprit += 0x78;
    
    do
    {
        SPRITE_VAR(cursprit)->to_next = cursprit + 0x30;
        cursprit += 0x30;
    }
    while (cursprit < finsprit);
    
    *(s16 *)(SPRITEMEM_PTR + cursprit + -0x24) = 0;

//    u16 sp = libsprit;
//    for (s32 i = 0; i < 100; i++)
//    {
//        sp = *(s16 *)(SPRITEMEM_PTR + sprite_to_next + sp);
//    }
    
    
//    basemain = 0;
//    s32 cursprit = debsprit;
//    s32 base = debsprit + 0x8000;
//    tvsprite = 0x8000;
//    basesprite = base;
//    *(s32 *)(alis.spritemem + cursprit + sprite_newx) = 0;
//    *(s16 *)(alis.spritemem + cursprit + sprite_depx) = 0x13f;
//    *(s16 *)(alis.spritemem + cursprit + sprite_depy) = 199;
//    *(s8 *) (alis.spritemem + cursprit + 0x31) = 0;
//    backsprite = 0x8030;
//    texsprite = 0x8060;
//    atexsprite = cursprit + 0x90;
//    mousprite = 0x8090;
//    *(s32 *)(alis.spritemem + cursprit + 0x9c) = 0;
//    *(s8 *) (alis.spritemem + cursprit + 0x90) = 0xfe;
//    mousflag = 0;
//    // s16 size = -0x7f40;
//    s16 size = -0x7f88;
////    libsprit = 0x80c0;
////    libsprit = 0x8078;
//    libsprit = 0x80c8;
////    cursprit += 0xc0;
//    cursprit += 0xc8;
//
//    do
//    {
//        // size += 0x30;       // Falcon
//        size += 0x28;       // ST
//
//        // save offsets for eaech sprit
//        *(s16 *)(alis.spritemem + cursprit + sprite_to_next) = size;
//        cursprit = base + size;
//    }
//    while (cursprit < finsprit);
//
//    // save 0 for the last offset to indicate end
////    *(s32 *)(alis.spritemem + cursprit - sprite_newzoomx) = 0;
//    *(s32 *)(alis.spritemem + cursprit - 0x24) = 0;
}

u8 searchelem(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;

    u16 screen_id;
    s8 num;
    
    *previdx = 0;
    *curidx = alis.script->context._0x18_unknown;
    if (*curidx == 0)
    {
        ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
        return 1;
    }
    
    SpriteVariables *cursprvar = NULL;
    
    do
    {
        cursprvar = SPRITE_VAR(*curidx);
        screen_id = cursprvar->screen_id;
        if (screen_id <= alis.script->context._0x16_screen_id)
        {
            if (alis.script->context._0x16_screen_id != screen_id)
            {
                ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
                return 1;
            }

            num = cursprvar->numelem;
            if (num <= numelem)
            {
                if (numelem != num)
                {
                    ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
                    return 1;
                }

                ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
                return 0;
            }
        }

        *previdx = *curidx;
        *curidx = cursprvar->to_next;
    }
    while (*curidx != 0);

    ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
    return 0;
}

u8 testnum(u16 *curidx)
{
    DEBUGFCE;
    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);
    return ((cursprvar->screen_id == alis.script->context._0x16_screen_id) && (cursprvar->numelem == numelem)) ? 1 : 0;
}

void nextnum(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;

    *previdx = *curidx;
    *curidx = SPRITE_VAR(*curidx)->to_next;
    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);
    if ((cursprvar->state != 0 && cursprvar->screen_id == alis.script->context._0x16_screen_id) && cursprvar->numelem == numelem)
    {
        ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
        return;
    }

    ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
}

void createlem(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;

    SpriteVariables *cursprvar = SPRITE_VAR(libsprit);

    if (libsprit != 0)
    {
        u16 sprit = libsprit;
        u16 nextsprit = cursprvar->to_next;

        if (*previdx == 0)
        {
            alis.script->context._0x18_unknown = libsprit;
            libsprit = nextsprit;
        }
        else
        {
            SpriteVariables *prevsprvar = SPRITE_VAR(*previdx);
            prevsprvar->to_next = libsprit;
            libsprit = nextsprit;
        }

        s16 scrnidx = SCENE_VAR(alis.script->context._0x16_screen_id)->screen_id;
        SpriteVariables *scrnsprvar = SPRITE_VAR(scrnidx);

        cursprvar->state = 0xff;
        cursprvar->numelem = numelem;
        cursprvar->screen_id = alis.script->context._0x16_screen_id;
        cursprvar->to_next = *curidx;
        cursprvar->link = scrnsprvar->link;
        scrnsprvar->link = sprit;

        *curidx = sprit;
    }

    VERIFYINTEGRITY;
    ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);
}

s16 delprec(u16 elemidx)
{
    DEBUGFCE;

    SceneVariables *cursprvar = SCENE_VAR(elemidx);
    s16 scridx = cursprvar->screen_id;

    SceneVariables *scrsprvar = SCENE_VAR(scridx);
    if (elemidx == scrsprvar->screen_id)
    {
        scrsprvar->screen_id = cursprvar->link;
        return scridx;
    }
    
    s16 tmpidx = scrsprvar->screen_id;
    SceneVariables *tmpsprvar = SCENE_VAR(tmpidx);
    while (elemidx != tmpsprvar->link)
    {
        tmpidx = tmpsprvar->link;
        tmpsprvar = SCENE_VAR(tmpidx);
    }
    
    tmpsprvar->link = cursprvar->link;
    return tmpidx;
}

void killelem(u16 *curidx, u16 *previdx)
{
    DEBUGFCE;
    ELEM_TRACE("   cursprite: 0x%.4x prevsprite: 0x%.4x numelem: 0x%.2x\n", ELEMIDX(*curidx), ELEMIDX(*previdx), numelem);

    SpriteVariables *prevsprvar = SPRITE_VAR(*previdx);
    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);

    if (ferase == '\0' && -1 < (s8)cursprvar->state)
    {
        cursprvar->state = 1;
        
        if (*previdx == 0)
        {
            alis.script->context._0x18_unknown = cursprvar->to_next;
        }
        else
        {
             prevsprvar->to_next = cursprvar->to_next;
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
            prevsprvar->to_next = cursprvar->to_next;
        }
      
        cursprvar->to_next = libsprit;
        libsprit = *curidx;
        
        delprec(*curidx);
        
        if (*previdx)
        {
            *curidx = alis.script->context._0x18_unknown;
            VERIFYINTEGRITY;
            return;
        }
    }
    
    *curidx = prevsprvar->to_next;
    VERIFYINTEGRITY;
}

void put(u8 idx)
{
    DEBUGFCE;

    fmuldes = 0;
    putin(idx);
}

void putin(u8 idx)
{
    DEBUGFCE;
    ELEM_TRACE("  idx: 0x%.2x numelem: 0x%.6x\n", idx, numelem);

    // numelem = idx;
    
    s32 addr = adresdes(idx);
    u8 *resourcedata = alis.mem + alis.script->data_org + addr;

    u16 x = depx;
    u16 z = depz;
    u16 y = depy;
    
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
            invx = flaginvx;
            muldes = fmuldes;
            
            s16 curelem = read16(currsrc + 0, alis.platform.is_little_endian);
            s16 curdepx = read16(currsrc + 2, alis.platform.is_little_endian);
            if (*((u8 *)(&flaginvx)) != 0)
                curdepx = -curdepx;

            depx += curdepx;

            s16 curdepy = read16(currsrc + 4, alis.platform.is_little_endian);
            depy = curdepy + y;

            s16 curdepz = read16(currsrc + 6, alis.platform.is_little_endian);
            depz = curdepz + z;
            
            if (curelem < 0)
                *((u8 *)(&flaginvx)) ^= 1;

            fmuldes = 1;

            putin(curelem);

            depx = x;
            depy = y;
            depz = z;
            
            flaginvx = invx;
            fmuldes = muldes;

            currsrc += 8;
        }
        
        fmuldes = 0;
    }
    else
    {
        u16 cursprite = 0;
        u16 prevsprite = 0;

        if (fadddes == 0)
        {
            if (fmuldes)
                goto put13;

            searchelem(&cursprite, &prevsprite);
            if (cursprite == 0)
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
            searchelem(&cursprite, &prevsprite);
            if (cursprite)
            {
                do
                {
                    if (SPRITE_VAR(cursprite)->state == 0)
                        goto put30;
                  
                    nextnum(&cursprite, &prevsprite);
                }
                while (cursprite != 0); //(bVar3);
            }
            
            createlem(&cursprite, &prevsprite);
        }
        
        addr = adresdes(idx);

        SpriteVariables *cursprvar = SPRITE_VAR(cursprite);
        cursprvar->data       = addr;
        cursprvar->flaginvx   = (u8)flaginvx;
        cursprvar->depx       = oldcx + depx;
        cursprvar->depy       = oldcy + depy;
        cursprvar->depz       = oldcz + depz;
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
    }

    if (fmuldes == 0)
    {
        u16 cursprite = 0;
        u16 prevsprite = 0;

        searchelem(&cursprite, &prevsprite);
        if (fmuldes != 0)
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
                    fadddes = 0;
                    return;
                }

                u8 test = testnum(&cursprite);
                if (!test)
                {
                    fadddes = 0;
                    return;
                }
            }

            nextnum(&cursprite, &prevsprite);
        }
        while (cursprite != 0);
    }
    
    fadddes = 0;
    VERIFYINTEGRITY;
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
            
//        fenx1 &= 0xfff0;
//        fenx2 |= 0xf;

        fenlargw = (u16)((fenx2 - fenx1) + 1) >> 2;
        
        // TODO: uncoment if and when physic is initialized
        // adfenintv = (u32)feny1 * 0x140 + physic + (s32)(fenx1 >> 1);
    }
    
    LINK_TRACE("0x%.4x 0x%.4x\n", ELEMIDX(elemidx1), ELEMIDX(elemidx3));
    LINK_TRACE("BLOCK 0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", blocx1, blocy1, blocx2, blocy2);
    LINK_TRACE("FEN   0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", fenx1, feny1, fenx2, feny2);
}

u8 clipfen(SpriteVariables *sprite)
{
    DEBUGFCE;

    fclip = 0;
    s16 spritex2 = sprite->newx;
    if (fenx2 < spritex2)
        return fclip;

    s16 spritey2 = sprite->newy;
    if (feny2 < spritey2)
        return fclip;

    s16 spritex1 = sprite->depx;
    if (spritex1 < fenx1)
        return fclip;

    s16 spritey1 = sprite->depy;
    if (spritey1 < feny1)
        return fclip;

    if (spritex2 < fenx1)
        spritex2 = fenx1;

    if (spritey2 < feny1)
        spritey2 = feny1;

    if (fenx2 < spritex1)
        spritex1 = fenx2;

    if (feny2 < spritey1)
        spritey1 = feny2;

    clipx1 = spritex2;
    clipy1 = spritey2;
    clipx2 = spritex1;
    clipy2 = spritey1;
    clipl = (spritex1 - spritex2) + 1;
    cliph = (spritey1 - spritey2) + 1;
    fclip = 1;
    return fclip;
}

//s16 rangesprite(s16 elemidx1, s16 elemidx2, s16 elemidx3)
void rangesprite(s16 elemidx1, s16 elemidx3)
{
    DEBUGFCE;

    SpriteVariables *idx1sprvar = SPRITE_VAR(elemidx1);
    s16 newd1  =  idx1sprvar->newd;
    s8 cordspr1 = idx1sprvar->cordspr;
    s8 numelem1 = idx1sprvar->numelem;
    if (wback != 0 && elemidx1 != backsprite && backprof <= newd1)
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
    VERIFYINTEGRITY;
    LINK_TRACE("----\n");
    LINK_TRACE("sprite 0x%.4x -> 0x%.4x replaced by 0x%.4x\n", ELEMIDX(elemidx1), ELEMIDX(idx1sprvar->link), ELEMIDX(nextidx3));
    LINK_TRACE("sprite 0x%.4x -> 0x%.4x replaced by 0x%.4x\n", ELEMIDX(previdx3), ELEMIDX(SPRITE_VAR(previdx3)->link), ELEMIDX(elemidx1));
    LINK_TRACE("----\n");
    idx1sprvar->link = nextidx3;
    SPRITE_VAR(previdx3)->link = elemidx1;
    VERIFYINTEGRITY;

    idx1sprvar->state = 0;
    
//    if (elemidx2 == previdx3)
//    {
//        elemidx2 = elemidx1;
//    }
//
//    return elemidx2;
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

        s16 sVar3 = 0;
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
                sVar3 = 0;
            }
            else
            {
                if (scene->clinking < scridx)
                {
                    scridx = scene->clinking;
                }
                
                sVar3 = scridx << 2;
            }
        }
        
        tmpdepx = (tmpdepx >> (scene->credon_off & 0x3f)) + scene->unknown0x0a;
        tmpdepy = (tmpdepy >> (scene->credon_off & 0x3f)) + scene->unknown0x0c;
        
        newad = (elemsprvar->data & 0xffffff);
        // newad = (elemsprvar->data & 0xffffff) + sVar3);
        u8 *spritedata = (alis.mem + alis.script->data_org + newad);
        if (spritedata[0] == '\x03')
        {
            tmpdepx += (elemsprvar->flaginvx ? -1 : 1) * read16(spritedata + 4, alis.platform.is_little_endian);
            tmpdepy += read16(spritedata + 6, alis.platform.is_little_endian);
            if (spritedata[1] != 0)
            {
                newf = newf ^ 1;
            }
            
            newad = (newad + (read16(spritedata + 2, alis.platform.is_little_endian) << 2));
            spritedata = (alis.mem + alis.script->data_org + newad);
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

void phytolog(void)
{
    DEBUGFCE;
    fenx1 = 0;
    feny1 = 0;
    fenx2 = 0x13f;
    feny2 = 199;
//    trsfen();
}

void mouserase(void)
{
    DEBUGFCE;
    
    u8 *paVar7 = physic;
    if (fremouse2 != '\0')
    {
        paVar7 = logic;
    }
    
    s16 sVar2 = 0;
    s16 sVar4 = 0xf;
    s16 sVar3 = 0x120;
    
    if (-1 < savmouse[0])
    {
        u8 bVar1 = 0x12f < savmouse[0];
        if (bVar1)
        {
            sVar2 = 0x10;
            sVar3 = 0x130;
        }
        
        u32 *puVar5 = savmouse2;
        if ((s16)(-savmouse[1] + 0xb8) < 0)
        {
            sVar4 = -savmouse[1] + 199;
        }
        
        u32 *puVar8 = (u32 *)(paVar7 + savmouse[1] * 0x140 + (savmouse[0] & 0xfff0));
        
        do
        {
            u32 *puVar6;
            u32 *puVar9;
            if (!bVar1)
            {
                puVar8[0] = puVar5[0];
                puVar8[1] = puVar5[1];
                puVar6 = puVar5 + 3;
                puVar9 = puVar8 + 3;
                puVar8[2] = puVar5[2];
                puVar5 = puVar5 + 4;
                puVar8 = puVar8 + 4;
                puVar9[0] = puVar6[0];
            }
            
            puVar8[0] = puVar5[0];
            puVar8[1] = puVar5[1];
            puVar8[2] = puVar5[2];
            puVar8[3] = puVar5[3];
            puVar8 = (u32 *)((s32)puVar8 + sVar3 + 0x10);
            puVar5 = (u32 *)((s32)puVar5 + sVar2 + 0x10);
            sVar4 --;
        }
        while (sVar4 != -1);
    }
}

void oldfen(void)
{
    DEBUGFCE;

    u16 *puVar1 = (u16 *)&tabfen;
    while ((s32)puVar1 < ptabfen)
    {
        fenx1 = puVar1[0];
        fenlargw = puVar1[1];
        feny1 = puVar1[2];
        feny2 = puVar1[3];
        puVar1 = &puVar1[4];
        // tvtofen();
    }
}

void setphysic(void)
{
    DEBUGFCE;
//    if (insid == '\0')
//    {
//        DAT_00ff8203 = (u8)((u32)physic >> 8);
//        DAT_00ff8201 = (u8)((u32)physic >> 0x10);
//    }
//
//    bufpack = logic;
//    fphysic = 1;
}

u8 *folscreen(u8 *scene)
{
    DEBUGFCE;

    s16 *psVar1 = *(s16 **)(atent + *(s16 *)(scene + 0x60));
    if ((-1 < *(s16 *)(scene + 0x60)) && (psVar1 != (s16 *)0x0))
    {
        if ((scene[0x84] & 1) != 0)
        {
            if ((s16)(*psVar1 + *(s16 *)(scene + 0x86)) != *(s16 *)(scene + 0x16))
            {
                *(s16 *)(scene + 0x16) = *psVar1 + *(s16 *)(scene + 0x86);
                *scene = *scene | 0x80;
            }
            if ((s16)(psVar1[4] + *(s16 *)(scene + 0x88)) != *(s16 *)(scene + 0x18))
            {
                *(s16 *)(scene + 0x18) = psVar1[4] + *(s16 *)(scene + 0x88);
                *scene = *scene | 0x80;
            }
            if ((s16)(psVar1[8] + *(s16 *)(scene + 0x8a)) != *(s16 *)(scene + 0x1a))
            {
                *(s16 *)(scene + 0x1a) = psVar1[8] + *(s16 *)(scene + 0x8a);
                *scene = *scene | 0x80;
            }
        }
        
        if ((scene[0x84] & 2) != 0)
        {
            s16 sVar2 = psVar1[0x0c] + psVar1[0x20] + *(s16 *)(scene + 0x8c);
            if (sVar2 != *(s16 *)(scene + 0x34))
            {
                *(s16 *)(scene + 0x34) = sVar2;
                *scene = *scene | 0x80;
            }
            sVar2 = psVar1[0x10] + psVar1[0x22] + *(s16 *)(scene + 0x8e);
            if (sVar2 != *(s16 *)(scene + 0x36))
            {
                *(s16 *)(scene + 0x36) = sVar2;
                *scene = *scene | 0x80;
            }
            sVar2 = psVar1[0x14] + psVar1[0x24] + *(s16 *)(scene + 0x90);
            if (sVar2 != *(s16 *)(scene + 0x38))
            {
                *(s16 *)(scene + 0x38) = sVar2;
                *scene = *scene | 0x80;
            }
        }
    }
    
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

void inouvlink(SceneVariables *scene, s16 elemidx1, s16 elemidx2, s16 elemidx3)
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
    rangesprite(elemidx1/*, elemidx2*/, elemidx3);
}

void iremplink(SceneVariables *scene, s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    DEBUGFCE;

    addlink(elemidx1);
    
    SpriteVariables *elem1sprvar = SPRITE_VAR(elemidx1);
    if (wback != 0 && (s16)backprof <= elem1sprvar->newd)
    {
        pback = 1;
        inouvlink(scene, elemidx1, elemidx2, elemidx3);
        return;
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
    rangesprite(elemidx1/*, elemidx2*/, elemidx3);
}

void iefflink(s16 elemidx1, s16 elemidx2)
{
    DEBUGFCE;

    addlink(elemidx1);

    SpriteVariables *elem1sprvar = SPRITE_VAR(elemidx1);
    SpriteVariables *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;
    elem1sprvar->to_next = libsprit;
    
    libsprit = elemidx1;
    if (wback != 0 && backprof <= elem1sprvar->newd)
    {
        pback = 1;
    }
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
//        memset(physic + fenx1 + y * 320, 0, tmpx);
        memset(host.pixelbuf0.data + fenx1 + y * 320, 0, tmpx);
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

    u8 *bitmap = (alis.mem + alis.script->data_org + spnewad);
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
    
    DRAW_TRACE("%.2x %.2x (%.2x %.2x)\n", sprite->state, sprite->numelem, bitmap[0], bitmap[1]);
    DRAW_TRACE("%.3d %.3d %.3d %.3d\n", blocx1, blocy1, blocx2, blocy2);
    DRAW_TRACE("%.3d %.3d %.3d %.3d\n", bmpx1 + posx1, bmpy1 + posy1, bmpx2 + posx1, bmpy2 + posy1);
    DRAW_TRACE("  to:  %.3dx%.3d\n", posx1, posy1);
    DRAW_TRACE("  dim: %.3dx%.3d\n", width, height);
    
    switch (bitmap[0])
    {
        case 0x01:
        {
            // rectangle
            
            color = bitmap[1];
            
            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = host.pixelbuf0.data + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf0.w);
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
            
            at = bitmap + 6;
            
            for (s32 h = bmpy1; h < bmpy1 + bmpy2; h++)
            {
                u8 *tgt = host.pixelbuf0.data + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf0.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    u16 wh = ((flip ? (width - (w + 2)) : w)) / 2;
                    color = *(at + wh + h * (width / 2));
                    color = 0 + w % 2 == 0 ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                    if (color != clear)
                    {
                        if ((posy1 + h) < 0 || (posy1 + h) >= 200)
                            continue;
                        
                        tgt = host.pixelbuf0.data + (w + posx1) + ((posy1 + h) * host.pixelbuf0.w);
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
                u8 *tgt = host.pixelbuf0.data + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf0.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
                    u16 wh = ((flip ? (width - (w + 2)) : w)) / 2;
                    color = *(at + wh + h * (width / 2));
                    color = 0 + w % 2 == 0 ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
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
                u8 *tgt = host.pixelbuf0.data + (bmpx1 + posx1) + ((posy1 + h) * host.pixelbuf0.w);
                for (s32 w = bmpx1; w < bmpx1 + bmpx2; w++, tgt++)
                {
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
    
    // TODO: to be removed, hack to show changes immediatelly
    sys_render(host.pixelbuf0);
}

//s16 DAT_0000d5e4;
//s16 DAT_0000d5e6;
//s16 DAT_0000d5e8;
//s16 DAT_0000d5ea;
//s16 DAT_0000df60;
//s16 DAT_0000df62;
//s16 DAT_0000df64;
//s16 DAT_0000df66;
//s16 DAT_0000df68;
//s16 DAT_0000df6a;
//s16 DAT_0000df6c;
//s16 DAT_0000df6e;
//s16 DAT_000195ba;
//s16 DAT_000195bc;
//s16 DAT_000195d0;
//s16 DAT_000196cc;
//s16 DAT_000196e6;
//s16 DAT_000196e6;
//s16 DAT_000196e6;
//s16 DAT_000196e6;
//s16 DAT_000196e6;
//s16 DAT_000196e6;
//
//u32 CONCAT11(u8 a, u8 b)
//{
//    u16 result = (((u16)a) << 8) + b;
//    return result;
//}
//
//u32 CONCAT22(u16 a, u16 b)
//{
//    u32 result = (((u32)a) << 16) + b;
//    return result;
//}
//
//
//u32 CONCAT21(u16 a, u8 b)
//{
//    u32 result = (((u32)a) << 8) + b;
//    return result;
//}
//
//u32 CONCAT31(u32 a, u16 b)
//{
//    u32 result = (((u32)a) << 8) + b;
//    return result;
//}
//
//void destofenX(SpriteVariables *sprite)
//{
//    u8 uVar1;
//    u8 uVar2;
//    u8 uVar3;
//    u8 uVar4;
//    u8 uVar5;
//    u8 uVar6;
//    s16 sVar10;
//    u16 uVar11;
//    u32 uVar7;
//    u32 uVar8;
//    u16 uVar12;
//    u16 uVar13;
//    s32 iVar9;
//    u32 uVar14;
//    u16 uVar16;
//    u16 uVar17;
//    s32 iVar15;
//    u32 uVar18;
//    u16 uVar21;
//    s32 iVar19;
//    u16 uVar24;
//    s32 iVar22;
//    u32 uVar25;
//    u32 uVar26;
//    u32 uVar28;
//    u16 wwidth;
//    u32 uVar29;
//    u16 uVar31;
//    u16 uVar32;
//    u16 *puVar33;
//    u16 *puVar34;
//    u16 *logicptr;
//    u16 *puVar36;
//    s32 rwwidth;
//    u16 *puVar39;
//    u32 *puVar40;
//    s16 sVar41;
//    u16 uStack_2;
//
//    // s8 *bitmap = (s8 *)(*(s32 *)(*(u32 *)(sprite + sprite_newad) & 0xffffff) + (s32)(s32 *)(*(u32 *)(sprite + sprite_newad) & 0xffffff));
//    u32 spnewad = sprite->newad & 0xffffff;
//    u8 *bitmap = (alis.mem + alis.script->data_org + spnewad);
//    if (*bitmap < '\0')
//        return;
//
//    s16 xpos1 = sprite->newx;
//    if ((s16)clipx2 < xpos1)
//        return;
//
//    s16 ypos1 = sprite->newy;
//    if (clipy2 < ypos1)
//        return;
//
//    s16 xpos2 = xpos1 + read16(bitmap + 2, alis.platform.is_little_endian);
//    if (xpos2 < clipx1)
//        return;
//
//    s16 ypos2 = ypos1 + read16(bitmap + 4, alis.platform.is_little_endian);
//    if (ypos2 < clipx1)
//        return;
//
//    blocx1 = xpos1;
//    if (xpos1 < clipx1)
//        blocx1 = clipx1;
//
//    blocy1 = ypos1;
//    if (ypos1 < clipx1)
//        blocy1 = clipx1;
//
//    blocx2 = xpos2;
//    if (clipx2 < xpos2)
//        blocx2 = clipx2;
//
//    blocy2 = ypos2;
//    if (clipy2 < ypos2)
//        blocy2 = clipy2;
//
//    uStack_2 = blocy2 - blocy1;
//    xpos1 = blocx1 & 0xfff0;
//    xpos2 = blocx2 | 0xf;
//    wwidth = (xpos2 - xpos1) >> 4;              // width in words
//    rwwidth = wloglarg - ((wwidth + 1) * 4); // remaining screen width in words
//    ypos2 = rwwidth * 2;
//    DAT_000195d0 = (u16)(read16(bitmap + 2, alis.platform.is_little_endian) + 1U) >> 2;
//    ypos1 = (DAT_000195d0 + (wwidth + 1) * -4) * 2;
//    uVar26 = (u32)(u16)(blocy1 - DAT_000196cc) * (u32)wloglarg * 2;
//    uVar25 = uVar26 & 0xffff0000;
//    logicptr = (u16 *)((s32)(s16)(((u16)(blocx1 - wlogx1) & 0xfff0) >> 1) + uVar26 + wlogic);
//
//    if (*bitmap == '\x01')
//    {
//        // rectangle
//
//        DAT_000195ba = (u16)(xpos1 <= sprite->newx);
//        DAT_000195bc = 0;
//        xpos1 = wwidth;
//        if ((read16(bitmap + 2, alis.platform.is_little_endian) + sprite->newx) < xpos2)
//        {
//            DAT_000195bc = 1;
//            xpos1 = wwidth - 1;
//            if ((s16)(wwidth - 1) < 0)
//            {
//                DAT_000195bc = -1;
//                xpos1 = wwidth;
//            }
//        }
//
//        ypos1 = DAT_000195bc;
//
//        // set solid color for four bitplanes
//
//        u16 bit1 = 0;
//        if ((bitmap[1] & 1U) != 0)
//            bit1 = 0xffff;
//
//        u16 bit2 = 0;
//        if ((bitmap[1] & 2U) != 0)
//            bit2 = 0xffff;
//
//        u16 bit3 = 0;
//        if ((bitmap[1] & 4U) != 0)
//            bit3 = 0xffff;
//
//        u16 bit4 = 0;
//        if ((bitmap[1] & 8U) != 0)
//            bit4 = 0xffff;
//
//        uVar28 = bit4 << 0x10 | bit4 >> 0x10;
//        uVar32 = (s16)(0x10000 >> (sprite->newx & 0xf)) - 1;
//        DAT_0000df60 = uVar32 & bit1;
//        DAT_0000df62 = uVar32 & bit2;
//        DAT_0000df64 = uVar32 & bit3;
//        DAT_0000df66 = uVar32 & (u16)bit4;
//        uVar31 = ~((s16)(0x8000 >> ((u16)(sprite->newx + read16(bitmap + 2, alis.platform.is_little_endian)) & 0xf)) - 1U);
//        DAT_0000df68 = uVar31 & bit1;
//        DAT_0000df6a = uVar31 & bit2;
//        DAT_0000df6c = uVar31 & bit3;
//        DAT_0000df6e = uVar31 & (u16)bit4;
//        if ((DAT_000195ba != 0) && (DAT_000195bc < 0))
//        {
//            DAT_000195ba = 0;
//            uVar31 = uVar31 & uVar32;
//            DAT_0000df68 = uVar32 & DAT_0000df68;
//            DAT_0000df6a = uVar32 & DAT_0000df6a;
//            DAT_0000df6c = uVar32 & DAT_0000df6c;
//            DAT_0000df6e = uVar32 & DAT_0000df6e;
//        }
//
//        uVar12 = DAT_000195ba;
//        uVar32 = ~uVar32;
//        uVar31 = ~uVar31;
//        bit4 = (u32)DAT_000195bc;
//
//        do
//        {
//            uVar13 = xpos1;
//            if (uVar12 == 0)
//            {
//                if (-1 < ypos1)
//                {
//                    uVar25 = (u32)bit1 | uVar25 & 0xffff0000;
//                    goto LAB_0000df1a;
//                }
//LAB_0000df2c:
//                uVar13 = logicptr[0] & uVar31 | DAT_0000df68;
//                uVar17 = logicptr[1] & uVar31 | DAT_0000df6a;
//                uVar16 = logicptr[2] & uVar31 | DAT_0000df6c;
//                uVar21 = logicptr[3] & uVar31 | DAT_0000df6e;
//                logicptr[0] = uVar13;
//                logicptr[1] = uVar17;
//                logicptr[2] = uVar16;
//                logicptr[3] = uVar21;
//
//                uVar25 = CONCAT22((s16)*logicptr >> 0xf,uVar13);
//                puVar39 = logicptr + 4;
//            }
//            else
//            {
//                uVar17 = logicptr[0];
//                puVar39 = logicptr + 4;
//                uVar16 = logicptr[1] & uVar32 | DAT_0000df62;
//                uVar21 = logicptr[2] & uVar32 | DAT_0000df64;
//                uVar24 = logicptr[3] & uVar32 | DAT_0000df66;
//                logicptr[0] = uVar17 & uVar32 | DAT_0000df60;
//                logicptr[1] = uVar16;
//                logicptr[2] = uVar21;
//                logicptr[3] = uVar24;
//                logicptr = puVar39;
//                if (ypos1 < 0)
//                    goto LAB_0000df2c;
//
//                uVar25 = CONCAT22((s16)uVar17 >> 0xf,bit1);
//
//                for (u16 i = 0; i < uVar13; i++)
//                {
//LAB_0000df1a:
//                    logicptr[0] = (u16)uVar25;
//                    logicptr[1] = bit2;
//                    logicptr[2] = bit3;
//                    logicptr[3] = (u16)(uVar28 >> 0x10);
//
//                    puVar39 = logicptr + 4;
//                    logicptr = puVar39;
//                }
//
//                uVar25 = uVar25 & 0xffff0000 | bit4 & 0xffff;
//                if (ypos1 != 0)
//                    goto LAB_0000df2c;
//            }
//
//            logicptr = puVar39 + rwwidth;
//            uVar13 = (s16)uVar28 - 1;
//            uVar28 = uVar28 & 0xffff0000 | (u32)uVar13;
//            if (uVar13 == 0xffff)
//            {
//                return;
//            }
//        }
//        while( true );
//    }
//
//    uVar26 = (u32)(u16)(blocy1 - sprite->newy) * (u32)DAT_000195d0 * 2;
//    iVar9 = uVar26 + 6;
//    uVar26 = uVar26 & 0xffff0000;
//    sVar10 = ((u16)((blocx1 - sprite->newx) - 1) >> 4) * 8;
//    if ((sprite->newf & 1) != 0)
//    {
//        ypos2 = ((u16)((blocx1 - sprite->newx) - 1) >> 4) << 3;
//        if (xpos1 <= sprite->newx)
//        {
//            ypos2 = 0;
//            ypos1 = ypos1 + 8;
//        }
//        DAT_000195ba = (u16)(xpos1 <= sprite->newx);
//        DAT_000195bc = 0;
//        puVar40 = (u32 *)(bitmap + (((s32)(s16)(DAT_000195d0 * 2) + (u32)(u16)(blocy1 - sprite->newy) * (u32)(u16)(DAT_000195d0 * 2) + -10) - (s32)ypos2));
//        ypos1 = ypos1 - (read16(bitmap + 2, alis.platform.is_little_endian) + 1U & 0xfffc);
//        xpos1 = wwidth;
//        if ((s16)((read16(bitmap + 2, alis.platform.is_little_endian) | 0xf) + sprite->newx) < xpos2)
//        {
//            DAT_000195bc = 1;
//            xpos1 = wwidth - 1;
//            if ((s16)(wwidth - 1) < 0)
//            {
//                DAT_000195bc = -1;
//                xpos1 = wwidth;
//            }
//            ypos1 = ypos1 + 8;
//        }
//
//        ypos2 = DAT_000195bc;
//        xpos2 = sprite->newx;
//        do
//        {
//            if (DAT_000195ba == 0)
//            {
//                wwidth = xpos1;
//                if (-1 < ypos2)
//                    goto LAB_0000e8da;
//LAB_0000e982:
//                uVar26 = 0;
//                uVar25 = 0;
//                uVar28 = puVar40[2];
//                uVar29 = puVar40[3];
//                puVar39 = logicptr;
//LAB_0000e98c:
//                uVar8 = (u32)(xpos2 & 0xf);
//                iVar9 = (uVar26 & 0xffff0000 | uVar28 >> 0x10) << uVar8;
//                iVar15 = (uVar25 & 0xffff0000 | uVar29 >> 0x10) << uVar8;
//                iVar19 = (uVar26 << 0x10 | uVar28 & 0xffff) << uVar8;
//                iVar22 = (uVar25 << 0x10 | uVar29 & 0xffff) << uVar8;
//                uVar1 = (&DAT_000196e6)[(s16)((u16)((u32)iVar19 >> 0x10) & 0xff)];
//                uVar2 = (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar19 >> 0x18)];
//                uVar3 = (&DAT_000196e6)[(s16)((u16)((u32)iVar15 >> 0x10) & 0xff)];
//                uVar4 = (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar15 >> 0x18)];
//                uVar5 = (&DAT_000196e6)[(s16)((u16)((u32)iVar22 >> 0x10) & 0xff)];
//                uVar6 = (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar22 >> 0x18)];
//                wwidth = ~(CONCAT11((&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x10)], (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x18)]) | CONCAT11(uVar1,uVar2) | CONCAT11(uVar3,uVar4) | CONCAT11(uVar5,uVar6));
//                logicptr = puVar39 + 4;
//                uVar11 = *puVar39 & wwidth;
//                uVar26 = CONCAT22((s16)*puVar39 >> 0xf,uVar11);
//                puVar39[0] = CONCAT11((&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x10)], (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x18)]) | uVar11;
//                puVar39[1] = CONCAT11(uVar1,uVar2) | puVar39[1] & wwidth;
//                puVar39[2] = CONCAT11(uVar3,uVar4) | puVar39[2] & wwidth;
//                puVar39[3] = CONCAT11(uVar5,uVar6) | puVar39[3] & wwidth;
//            }
//            else
//            {
//                uVar28 = 0;
//                uVar29 = 0;
//                uVar26 = puVar40[2];
//                uVar25 = puVar40[3];
//                puVar39 = logicptr;
//                uVar11 = xpos1;
//                if (ypos2 < 0)
//                    goto LAB_0000e98c;
//
//                while (true)
//                {
//                    uVar8 = (u32)(xpos2 & 0xf);
//                    iVar9 = (uVar26 & 0xffff0000 | uVar28 >> 0x10) << uVar8;
//                    iVar15 = (uVar25 & 0xffff0000 | uVar29 >> 0x10) << uVar8;
//                    iVar19 = (uVar26 << 0x10 | uVar28 & 0xffff) << uVar8;
//                    iVar22 = (uVar25 << 0x10 | uVar29 & 0xffff) << uVar8;
//                    uVar1 = (&DAT_000196e6)[(s16)((u16)((u32)iVar19 >> 0x10) & 0xff)];
//                    uVar2 = (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar19 >> 0x18)];
//                    uVar3 = (&DAT_000196e6)[(s16)((u16)((u32)iVar22 >> 0x10) & 0xff)];
//                    uVar4 = (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar22 >> 0x18)];
//                    uVar31 = (u16)CONCAT31(CONCAT21(uVar11,(&DAT_000196e6)[(s16)((u16)((u32)iVar15 >> 0x10) & 0xff)]), (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar15 >> 0x18)]);
//                    wwidth = ~(CONCAT11((&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x10)], (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x18)]) | CONCAT11(uVar1,uVar2) | uVar31 | CONCAT11(uVar3,uVar4));
//                    logicptr = puVar39 + 4;
//                    uVar32 = *puVar39 & wwidth;
//                    uVar26 = CONCAT22((s16)*puVar39 >> 0xf,uVar32);
//                    puVar39[0] = CONCAT11((&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x10)], (&DAT_000196e6)[(s16)(u16)(u8)((u32)iVar9 >> 0x18)]) | uVar32;
//                    puVar39[1] = CONCAT11(uVar1,uVar2) | puVar39[1] & wwidth;
//                    puVar39[2] = uVar31 | puVar39[2] & wwidth;
//                    puVar39[3] = CONCAT11(uVar3,uVar4) | puVar39[3] & wwidth;
//                    wwidth = uVar11 - 1;
//                    if ((u16)(uVar11 - 1) == 0xffff)
//                        break;
//LAB_0000e8da:
//                    uVar26 = *puVar40;
//                    uVar25 = puVar40[1];
//                    uVar28 = puVar40[2];
//                    uVar29 = puVar40[3];
//                    puVar40 = puVar40 + -2;
//                    puVar39 = logicptr;
//                    uVar11 = wwidth;
//                }
//
//                if (ypos2 != 0)
//                    goto LAB_0000e982;
//            }
//
//            puVar40 = (u32 *)((s32)puVar40 - (s32)ypos1);
//            logicptr = logicptr + rwwidth;
//            uStack_2 = uStack_2 - 1;
//            if (uStack_2 == 0xffff)
//            {
//                return;
//            }
//        }
//        while( true );
//    }
//
//    if ((sprite->newx & 0xf) == 0)
//    {
//        if (*bitmap == '\x02')
//        {
//            // ST image
//
//            uVar25 = (u32)(u16)(sVar10 + 8);
//            if (xpos1 <= sprite->newx)
//            {
//                uVar25 = 0;
//            }
//            uVar26 = uVar26 | uVar25;
//            // transfen((u8 *)(bitmap + (s16)uVar25 + iVar9), (u8 *)puVar35, ypos, height, uVar30, uStack_2);
//            return;
//        }
//
//        if ((bitmap[1] & 0x10U) == 0)
//        {
//            if ((bitmap[1] & 0x20U) == 0)
//            {
//                sVar10 += 8;
//                if (xpos1 <= sprite->newx) {
//                    sVar10 = 0;
//                }
//                puVar39 = (u16 *)(bitmap + sVar10 + iVar9);
//                xpos1 = wwidth;
//
//                do
//                {
//                    do
//                    {
//                        puVar36 = logicptr;
//                        xpos2 = *puVar39;
//                        uVar11 = puVar39[1];
//                        logicptr = puVar39 + 3;
//                        uVar32 = puVar39[2];
//                        puVar39 = puVar39 + 4;
//                        uVar31 = *logicptr;
//                        uVar25 = uVar26 & 0xffff0000;
//                        uVar12 = xpos2 | uVar11 | uVar32 | uVar31;
//                        uVar26 = uVar25 | uVar12;
//                        if (uVar12 != 0)
//                        {
//                            uVar12 = ~uVar12;
//                            uVar26 = uVar25 | uVar12;
//                            if (uVar12 == 0)
//                            {
//                                puVar36[0] = xpos2;
//                                puVar36[1] = uVar11;
//                                puVar36[2] = uVar32;
//                                puVar36[3] = uVar31;
//                            }
//                            else
//                            {
//                                uVar13 = *puVar36 & uVar12;
//                                uVar26 = CONCAT22((s16)*puVar36 >> 0xf,uVar13);
//                                puVar36[0] = xpos2 | uVar13;
//                                puVar36[1] = uVar11 | puVar36[1] & uVar12;
//                                puVar36[2] = uVar32 | puVar36[2] & uVar12;
//                                puVar36[3] = uVar31 | puVar36[3] & uVar12;
//                            }
//                        }
//
//                        xpos1 = xpos1 - 1;
//                        logicptr = puVar36 + 4;
//                    }
//                    while (xpos1 != 0xffff);
//
//                    puVar39 = (u16 *)((s32)ypos1 + (s32)puVar39);
//                    uStack_2 = uStack_2 - 1;
//                    logicptr = (u16 *)((s32)puVar36 + ypos2 + 8);
//                    xpos1 = wwidth;
//                }
//                while (uStack_2 != 0xffff);
//
//                return;
//            }
//
//            uVar25 = (u32)(u16)(sVar10 + 8);
//            if (xpos1 <= sprite->newx)
//            {
//                uVar25 = 0;
//            }
//            uVar26 = uVar26 | uVar25;
//            puVar39 = (u16 *)(bitmap + (s16)uVar25 + iVar9);
//            xpos1 = wwidth;
//
//            do
//            {
//                do
//                {
//                    puVar36 = logicptr;
//                    xpos2 = puVar39[0];
//                    uVar11 = puVar39[1];
//                    uVar32 = puVar39[2];
//                    puVar39 = puVar39 + 4;
//                    uVar31 = xpos2 | uVar11 | uVar32;
//                    if (uVar31 != 0)
//                    {
//                        uVar31 = ~uVar31;
//                        if (uVar31 == 0)
//                        {
//                            puVar36[0] = xpos2;
//                            puVar36[1] = uVar11;
//                            puVar36[2] = uVar32;
//                            puVar36[3] = 0;
//                        }
//                        else
//                        {
//                            xpos2 = puVar36[0] & uVar31 | xpos2;
//                            uVar26 = CONCAT22((s16)*puVar36 >> 0xf,xpos2);
//                            puVar36[0] = xpos2;
//                            puVar36[1] = puVar36[1] & uVar31 | uVar11;
//                            puVar36[2] = puVar36[2] & uVar31 | uVar32;
//                            puVar36[3] = puVar36[3] & uVar31;
//                        }
//                    }
//                    xpos1 = xpos1 - 1;
//                    logicptr = puVar36 + 4;
//                }
//                while (xpos1 != 0xffff);
//
//                puVar39 = (u16 *)((s32)ypos1 + (s32)puVar39);
//                uStack_2 = uStack_2 - 1;
//                logicptr = (u16 *)((s32)puVar36 + ypos2 + 8);
//                xpos1 = wwidth;
//            }
//            while (uStack_2 != 0xffff);
//
//            return;
//        }
//
//        uVar25 = (u32)(u16)(sVar10 + 8);
//        if (xpos1 <= sprite->newx)
//        {
//            uVar25 = 0;
//        }
//
//        uVar26 = uVar26 | uVar25;
//        puVar39 = (u16 *)(bitmap + (s16)uVar25 + iVar9);
//        xpos1 = wwidth;
//
//        do
//        {
//            do
//            {
//                puVar36 = logicptr;
//                xpos2 = *puVar39;
//                uVar11 = puVar39[1];
//                puVar39 = puVar39 + 4;
//                if ((xpos2 | uVar11) != 0)
//                {
//                    uVar32 = ~(xpos2 | uVar11);
//                    if (uVar32 == 0)
//                    {
//                        *puVar36 = xpos2;
//                        puVar36[1] = uVar11;
//                        puVar36[2] = 0;
//                        puVar36[3] = 0;
//                    }
//                    else
//                    {
//                        xpos2 = *puVar36 & uVar32 | xpos2;
//                        uVar26 = CONCAT22((s16)*puVar36 >> 0xf,xpos2);
//                        *puVar36 = xpos2;
//                        puVar36[1] = puVar36[1] & uVar32 | uVar11;
//                        puVar36[2] = puVar36[2] & uVar32;
//                        puVar36[3] = puVar36[3] & uVar32;
//                    }
//                }
//
//                xpos1 = xpos1 - 1;
//                logicptr = puVar36 + 4;
//            }
//            while (xpos1 != 0xffff);
//
//            puVar39 = (u16 *)((s32)ypos1 + (s32)puVar39);
//            uStack_2 = uStack_2 - 1;
//            logicptr = (u16 *)((s32)puVar36 + ypos2 + 8);
//            xpos1 = wwidth;
//        }
//        while (uStack_2 != 0xffff);
//
//        return;
//    }
//
//    sVar41 = ypos1 + -8;
//    if (xpos1 <= sprite->newx)
//    {
//        sVar10 = 0;
//        sVar41 = ypos1;
//    }
//
//    DAT_000195ba = (u16)(xpos1 <= sprite->newx);
//    DAT_000195bc = 0;
//    puVar39 = (u16 *)(bitmap + sVar10 + iVar9);
//    DAT_0000d5e6 = wwidth;
//    if ((s16)((read16(bitmap + 2, alis.platform.is_little_endian) | 0xf) + sprite->newx) < xpos2)
//    {
//        DAT_000195bc = 1;
//        DAT_0000d5e6 = wwidth - 1;
//        if ((s16)(wwidth - 1) < 0) {
//            DAT_000195bc = -1;
//            DAT_0000d5e6 = wwidth;
//        }
//        sVar41 = sVar41 + 8;
//    }
//
//    ypos1 = DAT_000195bc;
//    DAT_0000d5e4 = uStack_2;
//    if (*bitmap != '\x02')
//    {
//        if ((bitmap[1] & 0x10U) != 0)
//        {
//            xpos1 = sprite->newx;
//            DAT_0000d5ea = DAT_000195bc;
//            DAT_0000d5e8 = DAT_000195ba;
//            do
//            {
//                xpos2 = DAT_0000d5e6;
//                ypos1 = DAT_0000d5ea;
//                if (DAT_0000d5e8 != 0)
//                {
//                    uVar26 = 0;
//                    uVar25 = 0;
//                    goto LAB_0000dc48;
//                }
//
//                puVar36 = puVar39 + 1;
//                wwidth = *puVar39;
//                puVar39 = puVar39 + 4;
//                uVar26 = (u32)wwidth << 0x10;
//                uVar25 = (u32)*puVar36 << 0x10;
//                uVar28 = uVar26;
//                uVar29 = uVar25;
//                if (-1 < DAT_0000d5ea)
//                    goto LAB_0000dc48;
//
//                do
//                {
//                    xpos2 = 0;
//                    ypos1 = 0;
//                    uVar26 = uVar28;
//                    uVar25 = uVar29;
//                    puVar36 = logicptr;
//                    while (true)
//                    {
//                        uVar8 = uVar28 >> (xpos1 & 0xf);
//                        uVar11 = (u16)uVar8;
//                        uVar32 = (u16)(uVar29 >> (xpos1 & 0xf));
//                        wwidth = uVar11 | uVar32;
//                        if (wwidth != 0)
//                        {
//                            wwidth = ~wwidth;
//                            if (wwidth == 0)
//                            {
//                                *puVar36 = uVar11;
//                                puVar36[1] = uVar32;
//                                *(u32 *)(puVar36 + 2) = 0;
//                            }
//                            else
//                            {
//                                *puVar36 = *puVar36 & wwidth | uVar11;
//                                puVar36[1] = puVar36[1] & wwidth | uVar32;
//                                puVar36[2] = puVar36[2] & wwidth;
//                                puVar36[3] = puVar36[3] & wwidth;
//                            }
//                        }
//
//                        xpos2 = xpos2 - 1;
//                        logicptr = puVar36 + 4;
//                        if (xpos2 == 0xffff)
//                            break;
//LAB_0000dc48:
//                        puVar36 = puVar39 + 1;
//                        wwidth = *puVar39;
//                        uVar28 = uVar26 | wwidth;
//                        uVar29 = uVar25 | *puVar36;
//                        puVar39 = puVar39 + 4;
//                        uVar26 = (u32)wwidth << 0x10;
//                        uVar25 = (u32)*puVar36 << 0x10;
//                        puVar36 = logicptr;
//                    }
//
//                    logicptr = puVar36 + 4;
//                    uVar28 = uVar26;
//                    uVar29 = uVar25;
//                }
//                while (ypos1 != 0);
//
//                puVar39 = (u16 *)((s32)sVar41 + (s32)puVar39);
//                logicptr = (u16 *)((s32)ypos2 + (s32)logicptr);
//                uStack_2 = uStack_2 - 1;
//            }
//            while (-1 < (s16)uStack_2);
//
//            return;
//        }
//
//        if ((bitmap[1] & 0x20U) == 0)
//        {
//            xpos1 = sprite->newx;
//            DAT_0000d5ea = DAT_000195bc;
//            DAT_0000d5e8 = DAT_000195ba;
//            do
//            {
//                uVar26 = (u32)DAT_0000d5e6;
//                ypos1 = DAT_0000d5ea;
//                if (DAT_0000d5e8 != 0)
//                {
//                    uVar25 = 0;
//                    uVar28 = 0;
//                    uVar29 = 0;
//                    goto LAB_0000d830;
//                }
//
//                puVar36 = puVar39 + 1;
//                xpos2 = *puVar39;
//                puVar33 = puVar39 + 2;
//                puVar39 = puVar39 + 4;
//                uVar25 = (u32)xpos2 << 0x10;
//                uVar28 = (u32)*puVar36 << 0x10;
//                uVar29 = (u32)*puVar33 << 0x10;
//                uVar8 = uVar25;
//                uVar14 = uVar28;
//                uVar18 = uVar29;
//                xpos2 = DAT_0000d5e6;
//                if (-1 < DAT_0000d5ea)
//                    goto LAB_0000d830;
//
//                do
//                {
//                    uVar26 = 0;
//                    ypos1 = 0;
//                    uVar25 = uVar8;
//                    uVar28 = uVar14;
//                    uVar29 = uVar18;
//                    puVar36 = logicptr;
//
//                    while ( true )
//                    {
//                        uVar7 = (u32)(xpos1 & 0xf);
//                        wwidth = (u16)(uVar8 >> uVar7);
//                        uVar32 = (u16)(uVar14 >> uVar7);
//                        uVar31 = (u16)(uVar18 >> uVar7);
//                        uVar12 = (u16)(uVar26 >> uVar7);
//                        uVar11 = wwidth | uVar32 | uVar31 | uVar12;
//                        if (uVar11 != 0)
//                        {
//                            uVar11 = ~uVar11;
//                            if (uVar11 == 0)
//                            {
//                                *puVar36 = wwidth;
//                                puVar36[1] = uVar32;
//                                puVar36[2] = uVar31;
//                                puVar36[3] = uVar12;
//                            }
//                            else
//                            {
//                                *puVar36 = *puVar36 & uVar11 | wwidth;
//                                puVar36[1] = puVar36[1] & uVar11 | uVar32;
//                                puVar36[2] = puVar36[2] & uVar11 | uVar31;
//                                puVar36[3] = puVar36[3] & uVar11 | uVar12;
//                            }
//                        }
//                        uVar26 = uVar8 >> uVar7 & 0xffff0000 | (u32)(u16)(xpos2 - 1);
//                        logicptr = puVar36 + 4;
//                        if ((u16)(xpos2 - 1) == 0xffff)
//                            break;
//LAB_0000d830:
//                        xpos2 = (u16)uVar26;
//                        puVar36 = puVar39 + 1;
//                        wwidth = *puVar39;
//                        uVar8 = uVar25 | wwidth;
//                        puVar33 = puVar39 + 2;
//                        uVar14 = uVar28 | *puVar36;
//                        puVar34 = puVar39 + 3;
//                        uVar18 = uVar29 | *puVar33;
//                        puVar39 = puVar39 + 4;
//                        uVar26 = (u32)*puVar34;
//                        uVar25 = (u32)wwidth << 0x10;
//                        uVar28 = (u32)*puVar36 << 0x10;
//                        uVar29 = (u32)*puVar33 << 0x10;
//                        puVar36 = logicptr;
//                    }
//
//                    logicptr = puVar36 + 4;
//                    uVar8 = uVar25;
//                    uVar14 = uVar28;
//                    uVar18 = uVar29;
//                }
//                while (ypos1 != 0);
//
//                puVar39 = (u16 *)((s32)sVar41 + (s32)puVar39);
//                logicptr = (u16 *)((s32)ypos2 + (s32)logicptr);
//                uStack_2 = uStack_2 - 1;
//            }
//            while (-1 < (s16)uStack_2);
//
//            return;
//        }
//
//        xpos1 = sprite->newx;
//        DAT_0000d5ea = DAT_000195bc;
//        DAT_0000d5e8 = DAT_000195ba;
//        do
//        {
//            xpos2 = DAT_0000d5e6;
//            ypos1 = DAT_0000d5ea;
//            if (DAT_0000d5e8 != 0)
//            {
//                uVar26 = 0;
//                uVar25 = 0;
//                uVar28 = 0;
//                goto LAB_0000da5e;
//            }
//            puVar36 = puVar39 + 1;
//            wwidth = *puVar39;
//            puVar33 = puVar39 + 2;
//            puVar39 = puVar39 + 4;
//            uVar26 = (u32)wwidth << 0x10;
//            uVar25 = (u32)*puVar36 << 0x10;
//            uVar28 = (u32)*puVar33 << 0x10;
//            uVar29 = uVar26;
//            uVar8 = uVar25;
//            uVar14 = uVar28;
//            if (-1 < DAT_0000d5ea)
//                goto LAB_0000da5e;
//
//            do
//            {
//                xpos2 = 0;
//                ypos1 = 0;
//                uVar26 = uVar29;
//                uVar25 = uVar8;
//                uVar28 = uVar14;
//                puVar36 = logicptr;
//                while( true )
//                {
//                    uVar18 = (u32)(xpos1 & 0xf);
//                    uVar7 = uVar29 >> uVar18;
//                    uVar11 = (u16)uVar7;
//                    uVar32 = (u16)(uVar8 >> uVar18);
//                    uVar31 = (u16)(uVar14 >> uVar18);
//                    wwidth = uVar11 | uVar32 | uVar31;
//                    if (wwidth != 0)
//                    {
//                        wwidth = ~wwidth;
//                        if (wwidth == 0)
//                        {
//                            *puVar36 = uVar11;
//                            puVar36[1] = uVar32;
//                            puVar36[2] = uVar31;
//                            puVar36[3] = 0;
//                        }
//                        else
//                        {
//                            *puVar36 = *puVar36 & wwidth | uVar11;
//                            puVar36[1] = puVar36[1] & wwidth | uVar32;
//                            puVar36[2] = puVar36[2] & wwidth | uVar31;
//                            puVar36[3] = puVar36[3] & wwidth;
//                        }
//                    }
//
//                    xpos2 = xpos2 - 1;
//                    logicptr = puVar36 + 4;
//                    if (xpos2 == 0xffff)
//                        break;
//LAB_0000da5e:
//                    puVar36 = puVar39 + 1;
//                    wwidth = *puVar39;
//                    uVar29 = uVar26 | wwidth;
//                    puVar33 = puVar39 + 2;
//                    uVar8 = uVar25 | *puVar36;
//                    uVar14 = uVar28 | *puVar33;
//                    puVar39 = puVar39 + 4;
//                    uVar26 = (u32)wwidth << 0x10;
//                    uVar25 = (u32)*puVar36 << 0x10;
//                    uVar28 = (u32)*puVar33 << 0x10;
//                    puVar36 = logicptr;
//                }
//
//                logicptr = puVar36 + 4;
//                uVar29 = uVar26;
//                uVar8 = uVar25;
//                uVar14 = uVar28;
//            }
//            while (ypos1 != 0);
//
//            puVar39 = (u16 *)((s32)sVar41 + (s32)puVar39);
//            logicptr = (u16 *)((s32)ypos2 + (s32)logicptr);
//            uStack_2 = uStack_2 - 1;
//        }
//        while (-1 < (s16)uStack_2);
//
//        return;
//    }
//
//    if ((bitmap[1] & 0x10U) != 0)
//    {
//        xpos1 = sprite->newx & 0xf;
//        DAT_0000d5ea = DAT_000195bc;
//        DAT_0000d5e8 = DAT_000195ba;
//
//        do
//        {
//            xpos2 = DAT_0000d5e6;
//            if (DAT_0000d5e8 == 0)
//            {
//                wwidth = *puVar39;
//                uVar11 = puVar39[1];
//                puVar39 = puVar39 + 4;
//                puVar36 = logicptr;
//                if (-1 < ypos1)
//                    goto LAB_0000dd48;
//LAB_0000dd68:
//                xpos2 = (u16)(((u32)wwidth << 0x10) >> xpos1);
//                uVar11 = (u16)(((u32)uVar11 << 0x10) >> xpos1);
//                wwidth = ~(xpos2 | uVar11);
//                xpos2 = *puVar36 & wwidth | xpos2;
//                uVar26 = uVar26 & 0xffff0000 | (u32)xpos2;
//                logicptr = puVar36 + 4;
//                *puVar36 = xpos2;
//                puVar36[1] = puVar36[1] & wwidth | uVar11;
//                puVar36[2] = puVar36[2] & wwidth;
//                puVar36[3] = puVar36[3] & wwidth;
//            }
//            else
//            {
//                wwidth = *puVar39;
//                uVar11 = puVar39[1];
//                uVar32 = ~((wwidth | uVar11) >> xpos1);
//                uVar26 = (u32)uVar32;
//                uVar31 = *logicptr & uVar32 | wwidth >> xpos1;
//                rwwidth = CONCAT22(wwidth,uVar31);
//                wwidth = logicptr[1] & uVar32 | uVar11 >> xpos1;
//                iVar9 = CONCAT22(uVar11,wwidth);
//                *logicptr = uVar31;
//                logicptr[1] = wwidth;
//                logicptr[2] = logicptr[2] & uVar32;
//                logicptr[3] = logicptr[3] & uVar32;
//                while( true )
//                {
//                    logicptr = logicptr + 4;
//                    puVar39 = puVar39 + 4;
//                    uVar11 = (u16)((u32)iVar9 >> 0x10);
//                    wwidth = (u16)((u32)rwwidth >> 0x10);
//                    xpos2 = xpos2 - 1;
//                    if (xpos2 == 0xffff)
//                        break;
//LAB_0000dd48:
//                    uVar32 = puVar39[1];
//                    rwwidth = CONCAT22(wwidth,*puVar39) << 0x10;
//                    iVar9 = CONCAT22(uVar11,uVar32) << 0x10;
//                    uVar26 = CONCAT22(wwidth,*puVar39) >> xpos1;
//                    *logicptr = (u16)uVar26;
//                    logicptr[1] = (u16)(CONCAT22(uVar11,uVar32) >> xpos1);
//                    *(u32 *)(logicptr + 2) = 0;
//                }
//
//                puVar36 = logicptr;
//                if (ypos1 != 0)
//                    goto LAB_0000dd68;
//            }
//            puVar39 = (u16 *)((s32)sVar41 + (s32)puVar39);
//            logicptr = (u16 *)((s32)ypos2 + (s32)logicptr);
//            uStack_2 = uStack_2 - 1;
//            if ((s16)uStack_2 < 0)
//            {
//                return;
//            }
//        }
//        while( true );
//    }
//
//    if ((bitmap[1] & 0x20U) != 0)
//    {
//        xpos1 = sprite->newx & 0xf;
//        DAT_0000d5ea = DAT_000195bc;
//        DAT_0000d5e8 = DAT_000195ba;
//        do
//        {
//            xpos2 = DAT_0000d5e6;
//            if (DAT_0000d5e8 == 0)
//            {
//                wwidth = *puVar39;
//                uVar11 = puVar39[1];
//                uVar32 = puVar39[2];
//                puVar39 = puVar39 + 4;
//                puVar36 = logicptr;
//                if (-1 < ypos1)
//                    goto LAB_0000db86;
//LAB_0000dbb2:
//                uVar25 = (u32)xpos1;
//                xpos2 = (u16)(0 >> uVar25);
//                uVar11 = (u16)(((u32)uVar11 << 0x10) >> uVar25);
//                uVar32 = (u16)(((u32)uVar32 << 0x10) >> uVar25);
//                wwidth = ~(xpos2 | uVar11 | uVar32);
//                xpos2 = *puVar36 & wwidth | xpos2;
//                uVar26 = uVar26 & 0xffff0000 | (u32)xpos2;
//                logicptr = puVar36 + 4;
//                *puVar36 = xpos2;
//                puVar36[1] = puVar36[1] & wwidth | uVar11;
//                puVar36[2] = puVar36[2] & wwidth | uVar32;
//                puVar36[3] = puVar36[3] & wwidth;
//            }
//            else
//            {
//                wwidth = puVar39[1];
//                uVar11 = puVar39[2];
//                uVar32 = ~((u16)(*puVar39 | wwidth | uVar11) >> xpos1);
//                uVar26 = (u32)uVar32;
//                uVar31 = logicptr[1] & uVar32 | wwidth >> xpos1;
//                rwwidth = CONCAT22(wwidth,uVar31);
//                wwidth = logicptr[2] & uVar32 | uVar11 >> xpos1;
//                iVar9 = CONCAT22(uVar11,wwidth);
//                *logicptr = *logicptr & uVar32 | *puVar39 >> xpos1;
//                logicptr[1] = uVar31;
//                logicptr[2] = wwidth;
//                logicptr[3] = logicptr[3] & uVar32;
//                while( true )
//                {
//                    wwidth = 0;
//                    logicptr = logicptr + 4;
//                    puVar39 = puVar39 + 4;
//                    uVar32 = (u16)((u32)iVar9 >> 0x10);
//                    uVar11 = (u16)((u32)rwwidth >> 0x10);
//                    xpos2 = xpos2 - 1;
//                    if (xpos2 == 0xffff)
//                        break;
//LAB_0000db86:
//                    uVar31 = puVar39[1];
//                    uVar12 = puVar39[2];
//                    rwwidth = CONCAT22(uVar11,uVar31) << 0x10;
//                    iVar9 = CONCAT22(uVar32,uVar12) << 0x10;
//                    uVar26 = CONCAT22(wwidth,*puVar39) >> xpos1;
//                    *logicptr = (u16)uVar26;
//                    logicptr[1] = (u16)(CONCAT22(uVar11,uVar31) >> xpos1);
//                    logicptr[2] = (u16)(CONCAT22(uVar32,uVar12) >> xpos1);
//                    logicptr[3] = 0;
//                }
//                puVar36 = logicptr;
//                if (ypos1 != 0)
//                    goto LAB_0000dbb2;
//            }
//            puVar39 = (u16 *)((s32)sVar41 + (s32)puVar39);
//            logicptr = (u16 *)((s32)ypos2 + (s32)logicptr);
//            uStack_2 = uStack_2 - 1;
//            if ((s16)uStack_2 < 0)
//            {
//                return;
//            }
//        }
//        while( true );
//    }
//
//    xpos1 = sprite->newx & 0xf;
//    DAT_0000d5ea = DAT_000195bc;
//    DAT_0000d5e8 = DAT_000195ba;
//    do
//    {
//        xpos2 = DAT_0000d5e6;
//        uVar26 = uVar26 & 0xffff0000 | (u32)DAT_0000d5e6;
//        if (DAT_0000d5e8 == 0)
//        {
//            wwidth = *puVar39;
//            uVar11 = puVar39[1];
//            puVar36 = puVar39 + 3;
//            uVar32 = puVar39[2];
//            puVar39 = puVar39 + 4;
//            uVar31 = *puVar36;
//            puVar36 = logicptr;
//            if (-1 < ypos1)
//                goto LAB_0000d986;
//LAB_0000d9ba:
//            uVar25 = (u32)xpos1;
//            xpos2 = (u16)(0 >> uVar25);
//            uVar11 = (u16)(((u32)uVar11 << 0x10) >> uVar25);
//            uVar32 = (u16)(((u32)uVar32 << 0x10) >> uVar25);
//            uVar31 = (u16)(((u32)uVar31 << 0x10) >> uVar25);
//            wwidth = ~(xpos2 | uVar11 | uVar32 | uVar31);
//            xpos2 = *puVar36 & wwidth | xpos2;
//            uVar26 = uVar26 & 0xffff0000 | (u32)xpos2;
//            logicptr = puVar36 + 4;
//            *puVar36 = xpos2;
//            puVar36[1] = puVar36[1] & wwidth | uVar11;
//            puVar36[2] = puVar36[2] & wwidth | uVar32;
//            puVar36[3] = puVar36[3] & wwidth | uVar31;
//        }
//        else
//        {
//            wwidth = puVar39[1];
//            uVar11 = puVar39[2];
//            uVar32 = puVar39[3];
//            uVar31 = ~((u16)(*puVar39 | wwidth | uVar11 | uVar32) >> xpos1);
//            uVar26 = 0;
//            uVar12 = logicptr[1] & uVar31 | wwidth >> xpos1;
//            rwwidth = CONCAT22(wwidth,uVar12);
//            wwidth = logicptr[2] & uVar31 | uVar11 >> xpos1;
//            iVar9 = CONCAT22(uVar11,wwidth);
//            uVar11 = logicptr[3] & uVar31 | uVar32 >> xpos1;
//            iVar15 = CONCAT22(uVar32,uVar11);
//            *logicptr = *logicptr & uVar31 | *puVar39 >> xpos1;
//            logicptr[1] = uVar12;
//            logicptr[2] = wwidth;
//            logicptr[3] = uVar11;
//            while( true )
//            {
//                wwidth = 0;
//                logicptr = logicptr + 4;
//                puVar39 = puVar39 + 4;
//                uVar31 = (u16)((u32)iVar15 >> 0x10);
//                uVar32 = (u16)((u32)iVar9 >> 0x10);
//                uVar11 = (u16)((u32)rwwidth >> 0x10);
//                uVar26 = uVar26 & 0xffff0000 | (u32)(u16)(xpos2 - 1);
//                if ((u16)(xpos2 - 1) == 0xffff)
//                    break;
//LAB_0000d986:
//                xpos2 = (u16)uVar26;
//                uVar12 = puVar39[1];
//                uVar13 = puVar39[2];
//                uVar17 = puVar39[3];
//                rwwidth = CONCAT22(uVar11,uVar12) << 0x10;
//                iVar9 = CONCAT22(uVar32,uVar13) << 0x10;
//                iVar15 = CONCAT22(uVar31,uVar17) << 0x10;
//                uVar26 = CONCAT22(wwidth,*puVar39) >> xpos1;
//                logicptr[0] = (u16)(CONCAT22(wwidth,*puVar39) >> xpos1);
//                logicptr[1] = (u16)(CONCAT22(uVar11,uVar12) >> xpos1);
//                logicptr[2] = (u16)(CONCAT22(uVar32,uVar13) >> xpos1);
//                logicptr[3] = (u16)(CONCAT22(uVar31,uVar17) >> xpos1);
//            }
//            puVar36 = logicptr;
//            if (ypos1 != 0)
//                goto LAB_0000d9ba;
//        }
//        puVar39 = (u16 *)((s32)sVar41 + (s32)puVar39);
//        logicptr = (u16 *)((s32)ypos2 + (s32)logicptr);
//        uStack_2 = uStack_2 - 1;
//        if ((s16)uStack_2 < 0)
//        {
//            return;
//        }
//    } while( true );
//}

void calctop(void)
{
    return;
}

void memfen(void)
{
    ptabfen[0] = fenx1;
    ptabfen[1] = fenlargw;
    ptabfen[2] = feny1;
    ptabfen[3] = feny2;
    ptabfen = ptabfen + 4;
  
    // TODO: ...
//    if (ptabfen < 0x20581)
//    {
//        return;
//    }
    
    // perror(param_1,0x16,param_2,param_3);
    return;
}

void fentotv(void)
{
    // TODO: trsfen
    // trsfen();
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
                                            if ((tmpidx == 0) || (tmpidx == backsprite))
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

                                if ((sback != 0) && (tmpidx == backsprite))
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
//            tmpidx = scene->to_next;
//            scridx = (s8 *)((u32)scridx & 0xffff0000 | (u32)uVar1);
        }

        // TODO: mouse
//        cVar2 = fremouse;
//        if (((-1 < fmouse) && (fmouse != '\x02')) && (fswitch == '\0'))
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

//        if ((a2[1] & 0x20) == 0)
//        {
//            if (fswitch != '\0')
//            {
//                if (wpag == '\0')
//                {
//                    memfen();
//                }
//
//                return;
//            }
//
//            if (wpag == '\0')
//            {
//                fentotv();
//            }
//        }

        // TODO: mouse
//        if (fswitch == '\0')
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

// ishar 2
//void affiscr(u8 *scene,s16 d3w)
//{
//  u8 *a0;
//  u16 d1w;
//  s8 cVar1;
//  u32 uVar2;
//  s16 sVar5;
//  u16 d1w_00;
//  u32 uVar3;
//  s16 sVar6;
//  u8 uVar8;
//  s16 sVar4;
//
//  if ((fremap != '\0') || ((s8)*scene < '\0')) {
//    depscreen(scene, d3w);
//  }
//  wback = (scene[1] & 4) != 0;
//  wpag = '\0';
//  if ((*scene & 0x20) != 0) {
//    wpag = '\x01';
//    spag = spag + -1;
//    if (spag == '\0') {
//      wpag = -1;
//    }
//  }
//  if ((scene[1] & 0x10) == 0) {
//    uVar3 = (u32)(u16)d3w;
//affiscin:
//    do {
//      uVar2 = uVar3;
//      sVar4 = (s16)uVar2;
//      d1w_00 = *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar4);
//      uVar3 = (u32)d1w_00;
//      if (d1w_00 == 0) goto affiscr1;
//      cVar1 = *(s8 *)(SPRITEMEM_PTR + (s16)d1w_00);
//      if (cVar1 != '\0') {
//        joints = 0;
//        if (cVar1 == '\x02') {
//          sVar6 = inilink(d1w_00);
//          uVar8 = sVar6 == 0;
//          if (sVar6 < 0) {
//            deptopix(scene,d1w_00);
//            tstjoints(d1w_00);
//            if ((bool)uVar8) {
//              *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar4) =
//                   *(u16 *)(SPRITEMEM_PTR + 6 + (s32)(s16)d1w_00);
//              fenetre(scene,d1w_00,d3w);
//            }
//            else {
//              *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar4) =
//                   *(u16 *)(SPRITEMEM_PTR + 6 + (s32)(s16)d1w_00);
//            }
//            *(u32 *)(SPRITEMEM_PTR + 8 + (s32)(s16)d1w_00) = newad;
//              // *(u32 *)(SPRITEMEM_PTR + 0xc + (s32)(s16)d1w_00) = newx;
//              *(u16 *)(SPRITEMEM_PTR + 0xc + (s32)(s16)d1w_00) = newx;
//              *(u16 *)(SPRITEMEM_PTR + 0xe + (s32)(s16)d1w_00) = newy;
//              *(u16 *)(SPRITEMEM_PTR + 0x10 + (s32)(s16)d1w_00) = newd;
//            *(u8 *)(SPRITEMEM_PTR + 8 + (s32)(s16)d1w_00) = newf;
//            rangesprite(d1w_00,d3w);
//            fenetre(scene,d1w_00,d3w);
//            uVar3 = uVar2;
//            goto affiscin;
//          }
//          iremplink(scene,(s16)uVar3,sVar4,d3w);
//        }
//        else {
//          if (cVar1 == -1) {
//            sVar6 = inilink(d1w_00);
//            inouvlink(scene,(s16)uVar3,sVar4,d3w);
//          }
//          else {
//            sVar6 = inilink(d1w_00);
//            iefflink((s16)uVar3,sVar4);
//          }
//          if (sVar6 < 0) {
//            joints = 1;
//            fenetre(scene,d1w_00,d3w);
//            uVar3 = uVar2;
//            goto affiscin;
//          }
//        }
//        while( true ) {
//          sVar4 = (s16)uVar3;
//          d1w = *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar4);
//          uVar3 = (u32)d1w;
//          if (d1w == 0) break;
//          if (sVar6 == *(s16 *)(SPRITEMEM_PTR + 0x1e + (s32)(s16)d1w)) {
//            cVar1 = *(s8 *)(SPRITEMEM_PTR + (s16)d1w);
//            if (cVar1 != '\0') {
//              if (cVar1 < '\0') {
//                inouvlink(scene,d1w,sVar4,d3w);
//              }
//              else if (cVar1 == '\x02') {
//                iremplink(scene,d1w,sVar4,d3w);
//              }
//              else {
//                iefflink(d1w,sVar4);
//              }
//            }
//          }
//        }
//        joints = 1;
//        fenetre(scene,d1w_00,d3w);
//        uVar3 = uVar2;
//      }
//    } while( true );
//  }
//  fenx1 = *(s16 *)(scene + 0xe);
//  feny1 = *(s16 *)(scene + 0x10);
//  fenx2 = fenx1 + *(s16 *)(scene + 0x12);
//  feny2 = feny1 + *(s16 *)(scene + 0x14);
//  fenlargw = (u16)((fenx2 - fenx1) + 1U) >> 2;
//  clipx1 = fenx1;
//  clipy1 = feny1;
//  clipx2 = fenx2;
//  clipy2 = feny2;
//  sVar4 = d3w;
//  if ((scene[1] & 0x40) == 0) {
//    clrfen();
//  }
//  while (sVar5 = sVar4, sVar4 = *(s16 *)(SPRITEMEM_PTR + 6 + (s32)sVar5), sVar6 = ptscreen, sVar4 != 0) {
//    if (*(s8 *)(SPRITEMEM_PTR + sVar4) != '\0') {
//      if (*(s8 *)(SPRITEMEM_PTR + sVar4) == '\x01') {
//        *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar5) = *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar4);
//        *(s16 *)(SPRITEMEM_PTR + 4 + (s32)sVar4) = libsprit;
//        libsprit = sVar4;
//        sVar4 = sVar5;
//      }
//      else {
//        deptopix(scene,sVar4);
//        *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar5) = *(u16 *)(SPRITEMEM_PTR + 6 + (s32)sVar4);
//        *(u32 *)(SPRITEMEM_PTR + 8 + (s32)sVar4) = newad;
//        //*(u32 *)(SPRITEMEM_PTR + 0xc + (s32)sVar4) = newx;
//          *(u16 *)(SPRITEMEM_PTR + 0xc + (s32)(s16)sVar4) = newx;
//          *(u16 *)(SPRITEMEM_PTR + 0xe + (s32)(s16)sVar4) = newy;
//        *(u16 *)(SPRITEMEM_PTR + 0x10 + (s32)sVar4) = newd;
//        *(u8 *)(SPRITEMEM_PTR + 8 + (s32)sVar4) = newf;
//        rangesprite(sVar4,d3w);
//        sVar4 = sVar5;
//      }
//    }
//  }
//  for (; sVar6 != 0; sVar6 = *(s16 *)(BASEMNMEM_PTR + 4 + (s32)sVar6)) {
//    if ((((*(u8 *)(BASEMNMEM_PTR + sVar6) & 0x40) == 0) && (sVar4 = *(s16 *)(BASEMNMEM_PTR + 2 + (s32)sVar6), sVar4 != 0)) && (clipfen(SPRITE_VAR(sVar4)), fclip != '\0'))
//    {
//      while (sVar4 = *(s16 *)(SPRITEMEM_PTR + 6 + (s32)sVar4), sVar4 != 0) {
//        a0 = (u8 *)(SPRITEMEM_PTR + sVar4);
//        if (((-1 < (s8)*a0) && (-1 < (s8)a0[8])) && (-1 < *(s16 *)(a0 + 0x10))) {
//          destofen(a0);
//          switchgo = 1;
//        }
//      }
//    }
//  }
//  if ((fswitch == '\0') && (wpag == '\0')) {
//    fentotv();
//  }
//affiscr1:
//  if (wpag < '\0') {
//    *scene = *scene & 0xdf;
//    fenx1 = *(s16 *)(scene + 0xe);
//    feny1 = *(s16 *)(scene + 0x10);
//    fenx2 = *(s16 *)(scene + 0xe) + *(s16 *)(scene + 0x12);
//    feny2 = *(s16 *)(scene + 0x10) + *(s16 *)(scene + 0x14);
//    // scrolpage();
//  }
//  return;
//}

// ishar 3
void affiscr(SceneVariables *scene, u16 elemidx3)
{
    DEBUGFCE;

    u8 state;
    bool issprit;
    u8 uVar8;
    u16 uvaridx;
    u16 nextidx;
    s16 linkidx;
    s16 previdx;
    s16 elemidx;
    s16 tempidx;

    // NOTE: falcon or ishar 3 specific
//    if (((u8 *)scene)[0x84] != '\0')
//    {
//        scene = (s8 *)folscreen((u8 *)scene);
//    }

    if (fremap != 0 || scene->state < 0)
    {
        depscreen(scene, elemidx3);
        // VERIFYINTEGRITY;
    }

    if ((scene->numelem & 2) == 0 || (scene->state & 0x80U) == 0)
    {
        wback = (scene->numelem & 4) != 0;
        wpag = '\0';

        if ((scene->state & 0x20U) != 0)
        {
            wpag = '\x01';
            spag --;
            if (spag == '\0')
            {
                wpag = -1;
            }
        }

        SpriteVariables *nextsprvar = NULL;

        if ((scene->numelem & 0x10) == 0)
        {

affiscin:
            nextidx = elemidx3;

            do
            {
                previdx = elemidx3;
                nextidx = SPRITE_VAR(nextidx)->link;
                if (nextidx == 0)
                    goto affiscr1;

                nextsprvar = SPRITE_VAR(nextidx);
                state = nextsprvar->state;

                if (state != 0)
                {
                    joints = 0;
                    pback = 0;
                    elemidx = previdx;
                    tempidx = nextidx;
                    if (state == 2)
                    {
                        linkidx = inilink(nextidx);
                        if (linkidx < 0)
                        {
                            uVar8 = wback == '\0';
                            if (!(bool)uVar8)
                            {
                                uVar8 = backprof == nextsprvar->newd;
                                if ((s16)backprof <= nextsprvar->newd)
                                {
                                    pback = 1;
                                    uVar8 = false;
                                }
                            }

                            deptopix(scene, nextidx);
                            tstjoints(nextidx);
                            VERIFYINTEGRITY;

                            if ((bool)uVar8)
                            {
                                SPRITE_VAR(previdx)->link = nextsprvar->link;
                                fenetre(scene, nextidx, elemidx3);
                            }
                            else
                            {
                                SPRITE_VAR(previdx)->link = nextsprvar->link;
                            }

                            nextsprvar->newad = newad;
                            nextsprvar->newx = newx;
                            nextsprvar->newy = newy;
                            nextsprvar->newd = newd;
                            nextsprvar->newf = newf;
                            nextsprvar->width = newl;
                            nextsprvar->height = newh;
                            nextsprvar->newzoomx = newzoomx;
                            nextsprvar->newzoomy = newzoomy;

                            elemidx = nextidx;
                            rangesprite(elemidx/*, previdx*/, elemidx3);
                            fenetre(scene, elemidx, elemidx3);
                            // pcVar3 = (s8 *)((u32)pcVar3 & 0xffff0000 | uVar9 & 0xffff);
                            //nextidx = elemidx3;

                            VERIFYINTEGRITY;

                            goto affiscin;
                        }

                        iremplink(scene, tempidx, elemidx, elemidx3);
                    }
                    else
                    {
                        if (state == 0xff)
                        {
                            VERIFYINTEGRITY;
                            // handle new sprites
                            linkidx = inilink(nextidx);
                            inouvlink(scene, tempidx, elemidx, elemidx3);
                            VERIFYINTEGRITY;
                        }
                        else
                        {
                            VERIFYINTEGRITY;
                            linkidx = inilink(nextidx);
                            iefflink(tempidx, elemidx);
                            tempidx = elemidx;
                            VERIFYINTEGRITY;
                        }

                        if (linkidx < 0)
                        {
                            VERIFYINTEGRITY;
                            joints = 1;
//                            fenetre(scene, elemidx, elemidx3);
                            fenetre(scene, nextidx, elemidx3);
                            // pcVar3 = (s8 *)((u32)pcVar3 & 0xffff0000 | uVar9 & 0xffff);
                            VERIFYINTEGRITY;
                            nextidx = elemidx3;
                            goto affiscin;
                        }
                    }

                    while (true)
                    {
                        VERIFYINTEGRITY;
                        elemidx = tempidx;
                        nextidx = SPRITE_VAR(elemidx)->link;
                        tempidx = nextidx;
                        if (nextidx == 0)
                            break;

                        nextsprvar = SPRITE_VAR(nextidx);
                        if (linkidx == nextsprvar->clinking)
                        {
                            state = nextsprvar->state;
                            if (state != 0)
                            {
                                if ((s8)state < 0)
                                {
                                    VERIFYINTEGRITY;
                                    inouvlink(scene, nextidx, elemidx, elemidx3);
                                    VERIFYINTEGRITY;
                                }
                                else if (state == 2)
                                {
                                    VERIFYINTEGRITY;
                                    iremplink(scene, nextidx, elemidx, elemidx3);
                                    VERIFYINTEGRITY;
                                }
                                else
                                {
                                    VERIFYINTEGRITY;
                                    iefflink(nextidx, elemidx);
                                    VERIFYINTEGRITY;
                                    nextidx = elemidx;
                                }
                            }
                        }
                    }

                    joints = 1;
                    fenetre(scene, previdx, elemidx3);
//                  pcVar3 = (s8 *)((u32)pcVar3 & 0xffff0000 | uVar9 & 0xffff);
                    nextidx = elemidx3;
                }
            }
            while (true);
        }

        VERIFYINTEGRITY;
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
        nextidx = elemidx3;
        if ((scene->numelem & 0x40) == 0)
        {
            clrfen();
        }

        VERIFYINTEGRITY;

        SpriteVariables *prevsprvar = NULL;
        while ((nextidx = SPRITE_VAR((uvaridx = nextidx))->link) != 0)
        {
            VERIFYINTEGRITY;
            nextsprvar = SPRITE_VAR(nextidx);
            state = nextsprvar->state;
            if (state != 0)
            {
                prevsprvar = SPRITE_VAR(uvaridx);

                if (state == 1)
                {
                    VERIFYINTEGRITY;
                    prevsprvar->link = nextsprvar->link;
                    nextsprvar->to_next = libsprit;
                    libsprit = nextidx;
                    nextidx = uvaridx;
                    VERIFYINTEGRITY;

                }
                else
                {
                    VERIFYINTEGRITY;
                    deptopix(scene, nextidx);
                    VERIFYINTEGRITY;

                    prevsprvar->link = nextsprvar->link;
                    nextsprvar->newad = newad;
                    nextsprvar->newx = newx;
                    nextsprvar->newy = newy;
                    nextsprvar->newd = newd;
                    nextsprvar->newf = newf;
                    nextsprvar->width = newl;
                    nextsprvar->height = newh;
                    nextsprvar->newzoomx = newzoomx;
                    nextsprvar->newzoomy = newzoomy;

                    VERIFYINTEGRITY;
                    rangesprite(nextidx/*, uvaridx*/, elemidx3);
                    VERIFYINTEGRITY;
                    //nextidx = uvaridx;
                }
            }
        }

        issprit = ptscreen == 0;
        elemidx = ptscreen;
    }
    else
    {
        elemidx = scene->to_next;
        issprit = elemidx == 0;
    }

    VERIFYINTEGRITY;

    SceneVariables *parentscene = NULL;
    SpriteVariables *linksprvar = NULL;

    while (!issprit)
    {
        parentscene = SCENE_VAR(elemidx);

        if ((((parentscene->state & 0x40U) == 0) && (linkidx = parentscene->screen_id != 0)) && clipfen(SPRITE_VAR(linkidx)) != 0)
        {
            while ((linkidx = SPRITE_VAR(linkidx)->link) != 0)
            {
                linksprvar = SPRITE_VAR(linkidx);
                if (-1 < (s8)linksprvar->state && -1 < (s8)linksprvar->newf && -1 < linksprvar->newd)
                {
                    DRAW_TRACE("SPRITE %.4x\n", ELEMIDX(linkidx));
                    destofen(linksprvar);
                    switchgo = 1;
                }
            }
        }

        SceneVariables *scene = SCENE_VAR(elemidx);
        elemidx = scene->to_next;
        issprit = elemidx == 0;
    }

    if ((fswitch == '\0') && (wpag == '\0'))
    {
//      fentotv();
    }

    VERIFYINTEGRITY;
affiscr1:
    // VERIFYINTEGRITY;

    if (wpag < '\0')
    {
        scene->state &= 0xdf;
        fenx1 = scene->newx;
        feny1 = scene->newy;
        fenx2 = scene->newx + scene->width;
        feny2 = scene->newy + scene->height;
        // scrolpage(basesprite,BASEMNMEM_PTR,(u32)feny2);
    }

    scene->state &= 0x7f;
}

void image(void)
{
    DEBUGFCE;
    
    // TODO: just a hack to animate palette
    // following snippet is from itroutine
    if ((palc != 0) && ((--palt) == 0))
    {
        palt = palt0;
        topalet();
        palc --;
    }

    
//    if (amiga != '\0')
//    {
//        waitdisk();
//    }
//
//    waitphysic();
    if ((fswitch != '\0') && (fphytolog != '\0'))
    {
        fphytolog = '\0';
        phytolog();
    }
    
    s8 prevmouse = fremouse;
    
    // wait for vblank
    // do {} while (vtiming < timing);
    
    vtiming = 0;
//    if (fswitch != '\0')
//    {
//        if (fmouse < '\0')
//        {
//            oldfen();
//        }
//        else
//        {
//            do { } while (fremouse2 != '\0');
//
//            fremouse = fremouse + '\x01';
//            if (fremouse != '\0')
//            {
//                fremouse = prevmouse;
//            }
//
//            oldfen();
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
//        }
//
//        ptabfen = &tabfen;
//    }
    
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
//    u8 *oldscr_ptr = physic;
    
    while (scnidx != 0)
    {
        SceneVariables *scene = SCENE_VAR(scnidx);
        if ((scene->state & 0x40) == 0)
        {
            u16 sprite3 = scene->screen_id;
            affiscr(scene, sprite3);
        }

        scnidx = scene->to_next;
    }

//    fremap = 0;
//    if (fswitch != '\0')
//    {
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
//        physic = logic;
//        logic = oldscr_ptr;
//        setphysic();
//        if ((fmouse != '\0') && (-1 < fmouse))
//        {
//            fremouse2 = '\x01';
//            fremouse = -1;
//        }
//    }
}

u32 finmem = 0;
u32 adaline = 0;

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
    physic = host.pixelbuf0.data; // uVar2 + 1;
    finmem = uVar2 - 0x3ff;
    
}

void livemain(u16 *param_1, s16 param_2, s8 *param_3)
{
//    // A1 61f00, D1 0
//
//    // atent 4d540
//    // debent    4d546
//    // finent 4d7d4
//    // total 654 (entry 8 bytes)
//    // debsprit 511a0
//    // finent   511a0
//
//
//    s32 iVar4 = (s16)param_1[9] + finent;
//    u32 *puVar5 = (u32 *)((s16)param_1[0xb] + iVar4);
//    s32 iVar6 = (s32)puVar5 + 0x3e;
//    *(s32 *)(atent + param_2) = iVar6;
//    u32 uVar2 = (u32)(u16)param_1[10];
//    s32 iVar1 = uVar2 + iVar6;
//    if (debsprit <= (s32)(uVar2 + iVar6))
//    {
////        perror(param_3, 0xc, param_2, (s32)param_1);
//        return;
//    }
//
//    do
//    {
//        finent = iVar1;
//        *(u16 *)((s32)puVar5 + uVar2 + 0x3c) = 0;
//        uVar2 = uVar2 - 2;
//        iVar1 = finent;
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

u32 atprog;

void liveprog(s8 *param_1,u16 *param_2,s8 *param_3)
{
//    s16 unaff_D5w;
//
//    s16 sVar3 = dernent;
//    s32 iVar6 = atent;
//    u16 uVar2 = *(u16 *)(atent + 4 + (s32)unaff_D5w);
//    *(s16 *)(atent + 4 + (s32)unaff_D5w) = dernent;
//    s32 iVar1 = (s32)dernent;
//    dernent = *(u16 *)(iVar6 + 4 + (s32)dernent);
//    *(u16 *)(iVar6 + 4 + iVar1) = uVar2;
//    nbent = nbent + 1;
//    if (maxent < nbent)
//    {
////        perror(param_1, 5, sVar3, (s32)param_2);
//        return;
//    }
//
//    iVar6 = (s16)param_2[9] + finent;
//    u32 *puVar7 = (u32 *)((s16)param_2[0xb] + iVar6);
//    s32 iVar8 = (s32)puVar7 + 0x3e;
//    *(s32 *)(atent + sVar3) = iVar8;
//    u32 uVar4 = (u32)(u16)param_2[10];
//    iVar1 = uVar4 + iVar8;
//    if (debsprit <= (s32)(uVar4 + iVar8))
//    {
////        perror(param_3, 0xc, sVar3, (s32)param_2);
//        return;
//    }
//
//    do
//    {
//        finent = iVar1;
//        *(u16 *)((s32)puVar7 + uVar4 + 0x3c) = 0;
//        uVar4 = uVar4 - 2;
//        iVar1 = finent;
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
    s32 at = atent;
    s32 len = 0;
    
    do
    {
        len += 6;
        at += 6;
        *(s16 *)(alis.spritemem + at - 2) = len;
        *(u32 *)(alis.spritemem + at - 6) = 0;
    }
    while (at < debent); // debent 00 04
  
    *(u16 *)(alis.spritemem + atent + 4) = 0;
    nbent = 1;
    dernent = 6;
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
////    atent = atprog + (u32)DAT_0001fd04._2_2_ * 2;   // 00 04
//    atent = atprog + 0x7fff;
//    nbprog = 0;
//    fswitch = 0;
//    timing = 0;
//    ptscreen = 0;
//    vprotect = 0;
//    mousflag = 0;
//    fmouse = 0xff;
//    fonum = 0xffff;
//    sback = 0;
////    maxent = DAT_0001fd08;    // 00 6e
////    debent = atent + (u32)DAT_0001fd08 * 3;
////    debsprit = ((u32)((s32)debent + ram0x0001fd0e) | 0xf) + 1;
////    finsprit = (u16 *)(debsprit + (DAT_0001fd12 + 3U & 0xffff) * 0x30);
//    maxent = 0x6e;
//    debent = atent + 0x7fff;
//
//    debsprit = debent;
//    finsprit = debent + 0x7fff;
//
//    debprog = finsprit;
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
//    finent = debent;
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
////    basemain = pcVar9;
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

    sd7 = bsd7;               // 256 bytes empty mem
    sd6 = bsd6;               // 256 bytes empty mem
    oldsd7 = bsd7bis;         // 256 bytes empty mem
    atpalet = tpalet;         // 1024 bytes empty pal
    ampalet = mpalet;         // 1024 bytes empty pal
    ptabfen = &tabfen;        // &tabfen == &tablent ?
    ptrent = &tablent;
    fsound = 1;
    fmusic = 1;
    savsp = (void *)&sp;
    inisys();
//   uVar1 = inicanaux(); audio???
    bootmain((void *)0x1680a, param_2, script, uVar1);
//  moteur2(atent,uVar1,param_3);
    return;
}

// NOTE: just copy already read data to my local spritemem
void OPCODE_CDEFSC_0x46(u8 *ptr, u16 screen)
{
    memcpy(BASEMNMEM_PTR + 0x06 + screen, ptr + screen + 0x06, 32);

    if (libsprit == 0)
        return;

    SceneVariables *scene = SCENE_VAR(screen);
    scene->state = *(u8 *)(ptr + 0x00 + screen);
    scene->numelem = *(u8 *)(ptr + 0x01 + screen);
    scene->screen_id = libsprit;
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

    SpriteVariables *sprite = SPRITE_VAR(libsprit);
    sprite->link = 0;
    sprite->numelem = scene->numelem;
    sprite->newx = scene->newx & 0xfff0;
    sprite->newy = scene->newy;
    sprite->newd = 0x7fff;
    sprite->depx = scene->newx + scene->width | 0xf;
    sprite->depy = scene->newy + scene->height;

    printf("\n0x%.4x\n", ELEMIDX(libsprit));
    printf("0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", scene->newx, scene->newy, scene->width, scene->height);
    printf("0x%.4x 0x%.4x 0x%.4x 0x%.4x\n", sprite->newx, sprite->newy, sprite->depx, sprite->depy);

    libsprit = sprite->to_next;

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
    sprite->newx = scene->newx & 0xfff0;
    sprite->newy = scene->newy;
    sprite->newd = 0x7fff;
    sprite->depx = scene->newx + scene->width | 0x0f;
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
