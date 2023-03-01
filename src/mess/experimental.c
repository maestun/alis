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
#define BASESP_VAR(x) ((SpriteVariables *)(BASEMNMEM_PTR + x))


u8 flagmain = 0;

u8 *atpalet = 0;
u8 *ampalet = 0;
u8 *dkpalet = 0;

u8 tpalet[1024];
u8 mpalet[1024];

u32 pald = 0;
u32 ftopal = 0;
u8 thepalet = 0;
u8 defpalet = 0;

u8 *ptabfen = 0;
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
u16 clipx1;
u16 clipy1;
u16 clipx2;
u16 clipy2;
u16 clipl;
u16 cliph;
u8 fclip;

u8 *physic;
u8 *logic;
u8  fphysic = 0;
u8  fphytolog = 0;
u8  fremouse = 0;
u8  fremap = 0;
u8  fremouse2 = 0;
u8  vtiming = 0;
s16 savmouse[2];
u32 savmouse2[8];
u8  switchgo;
u8 *wlogic;
u16 wlogx1;
u8  wlogx2;
u16 wloglarg;
u32 logx1;
u32 logx2;
u16 loglarg;
u8 insid;
u8  spag = 0;
u8  wpag = 0;
u32 backx1;
u32 backx2;

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

#pragma pack(0)

typedef enum {
    
    sprite_state                        = 0x00,
    sprite_numelem                      = 0x01,
    sprite_screen_id                    = 0x02,
    sprite_to_next                      = 0x04,
    sprite_link                         = 0x06,
    sprite_newad                        = 0x08,
    sprite_newf                         = 0x08 + 3,
    sprite_newx                         = 0x0c,
    sprite_newy                         = 0x0e,
    sprite_newd                         = 0x10,
    sprite_data                         = 0x12,
    sprite_flaginvx                     = 0x12 + 3, // +0 on big endian
    sprite_depx                         = 0x16,
    sprite_depy                         = 0x18,
    sprite_depz                         = 0x1a,
    sprite_credon_off                   = 0x1c,
    sprite_creducing                    = 0x1d,
    sprite_clinking                     = 0x1e,
    sprite_cordspr                      = 0x20,
    sprite_chsprite                     = 0x21,
    sprite_cnoise                       = 0x22,
    sprite_0x28                         = 0x24,
    sprite_width                        = 0x28,
    sprite_height                       = 0x2a,
    sprite_newzoomx                     = 0x2c,
    sprite_newzoomy                     = 0x2e,

} SpriteVar;

s32 adresdei(s32 idx)
{
    u8 *addr = alis.mem + alis.main->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform);
    s32 e = read16(addr + l + 4, alis.platform);
    if (e > idx)
    {
        s32 a = read32(addr + l, alis.platform) + l + idx * 4;
        return read32(addr + a, alis.platform) + a;
    }

    return 0xf;
}

s32 adresdes(s32 idx)
{
    if (flagmain != '\0')
        return adresdei(idx);
    
    u8 *addr = alis.mem + alis.script->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform);
    s32 e = read16(addr + l + 4, alis.platform);
    if (e > idx)
    {
        s32 a = read32(addr + l, alis.platform) + l + idx * 4;
        return read32(addr + a, alis.platform) + a;
    }

    return 0xf;
}

void adresform(s16 idx)
{
    u8 *addr = alis.mem + alis.script->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform);
    if (idx - read16(addr + l + 10, alis.platform) < 0)
    {
        
    }
}

s32 adresmui(s32 idx)
{
    u8 *addr = alis.mem + alis.main->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform);
    s32 e = read16(addr + 0x10 + l, alis.platform);
    if (e > idx)
    {
        s32 a = read32(addr + 0xc + l, alis.platform) + l + idx * 4;
        return read32(addr + a, alis.platform) + a;
    }

    return 0x11;
}

s32 adresmus(s32 idx)
{
    if (flagmain != '\0')
        return adresmui(idx);

    u8 *addr = alis.mem + alis.script->context._0x14_script_org_offset;
    s32 l = read32(addr + 0xe, alis.platform);
    s32 e = read16(addr + 0x10 + l, alis.platform);
    if (e > idx)
    {
        s32 a = read32(addr + 0xc + l, alis.platform) + l + idx * 4;
        return read32(addr + a, alis.platform) + a;
    }

    return 0x11;
}

//u32 tdarkpal(void)
//{
//  u32 in_D0;
//  u32 uVar1;
//  u32 uVar3;
//  u32 *in_A0;
//  u32 *in_A1;
//  s8 *pbVar4;
//  u16 *puVar5;
//  u16 *puVar6;
//  u16 uVar2;
//
//  puVar5 = dkpalet + (u32)((u16)((s16)in_A1 - (s16)atpalet) >> 2) * 3;
//  uVar1 = in_D0;
//  do {
//    uVar3 = *in_A0;
//    in_A0 = (u32 *)((s32)in_A0 + 3);
//    bufrvb = uVar3 & 0xffffff00;
//    if ((bufrvb == 0) && (((0x100 < *puVar5 || (0x100 < puVar5[1])) || (0x100 < puVar5[2])))) {
//      bufrvb = 0x1010001;
//    }
//    uVar3 = (bufrvb >> 0x18) * (u32)*puVar5;
//    if (0xffff < uVar3) {
//      uVar3 = 0xffff;
//    }
//    *in_A1 = (s8)(uVar3 >> 9) & 0x7e;
//    puVar6 = puVar5 + 2;
//    uVar3 = (bufrvb >> 0x10 & 0xff) * (u32)puVar5[1];
//    if (0xffff < uVar3) {
//      uVar3 = 0xffff;
//    }
//    in_A1[1] = (s8)(uVar3 >> 9) & 0x7e;
//    pbVar4 = in_A1 + 3;
//    in_A1[2] = 0;
//    puVar5 = puVar5 + 3;
//    uVar3 = (bufrvb >> 8 & 0xff) * (u32)*puVar6;
//    if (0xffff < uVar3) {
//      uVar3 = 0xffff;
//    }
//    in_A1 = in_A1 + 4;
//    *pbVar4 = (s8)(uVar3 >> 9) & 0x7e;
//    uVar2 = (s16)uVar1 - 1;
//    uVar1 = (u32)uVar2;
//  } while (uVar2 != 0xffff);
//  return in_D0;
//}

//u16 topalet(void)
//{
//    u32 uVar1;
//    u32 *puVar2;
//    u32 *puVar3;
//    u16 in_D0;
//    s8 bVar4;
//    u32 uVar5;
//    s8 cVar8;
//    u32 uVar6;
//    u32 uVar7;
//    s16 sVar9;
//    u32 *puVar10;
//    u32 *puVar11;
//
//    sVar9 = 0xff;
//    ftopal = 0;
//    puVar2 = (u32 *)tpalet;
//    puVar3 = (u32 *)mpalet;
//
//topale1:
//
//    do
//    {
//        puVar11 = puVar3;
//        puVar10 = puVar2;
//        puVar2 = puVar10 + 1;
//        puVar3 = puVar11 + 1;
//        if (*puVar11 == *puVar10)
//        {
//            sVar9 = sVar9 - 1;
//            if (sVar9 != -1)
//            {
//                goto topale1;
//            }
//        }
//
//        if (*puVar11 == *puVar10)
//        {
//            return in_D0;
//        }
//
//        uVar7 = *puVar11;
//        uVar1 = *puVar10;
//        bVar4 = (s8)uVar1;
//        cVar8 = (s8)uVar7;
//        if (cVar8 < (s8)bVar4)
//        {
//            uVar6 = uVar7 & 0xffffff00 | (u32)(s8)(cVar8 + pald);
//            if (bVar4 < (s8)(cVar8 + pald))
//            {
//                uVar6 = uVar7 & 0xffffff00 | uVar1 & 0xff;
//            }
//        }
//        else
//        {
//            uVar6 = uVar7 & 0xffffff00 | (u32)(s8)(cVar8 - pald);
//            if ((s8)(cVar8 - pald) <= (s8)bVar4)
//            {
//                uVar6 = uVar7 & 0xffffff00 | uVar1 & 0xff;
//            }
//        }
//
//        uVar5 = (uint3)((uVar6 << 0x10) >> 8) | (uint3)(s8)(uVar6 >> 0x18);
//        bVar4 = (s8)(uVar1 >> 0x10);
//        cVar8 = (s8)(uVar6 >> 0x10);
//        if (cVar8 < (s8)bVar4)
//        {
//            uVar7 = CONCAT31(uVar5,cVar8 + pald);
//            if (bVar4 < (s8)(cVar8 + pald))
//            {
//                uVar7 = CONCAT31(uVar5,bVar4);
//            }
//        }
//        else
//        {
//            uVar7 = CONCAT31(uVar5,cVar8 - pald);
//            if ((s8)(cVar8 - pald) <= (s8)bVar4)
//            {
//                uVar7 = CONCAT31(uVar5,bVar4);
//            }
//        }
//
//        bVar4 = (s8)(uVar1 >> 0x18);
//        uVar1 = (u32)(u16)((s16)uVar7 << 8);
//        cVar8 = (s8)(uVar7 >> 8);
//        if (cVar8 < (s8)bVar4)
//        {
//            uVar1 = uVar7 & 0xffff0000 | uVar1;
//            uVar7 = uVar1 | (s8)(cVar8 + pald);
//            if (bVar4 < (s8)(cVar8 + pald))
//            {
//                uVar7 = uVar1 | bVar4;
//            }
//        }
//        else
//        {
//            uVar1 = uVar7 & 0xffff0000 | uVar1;
//            uVar7 = uVar1 | (s8)(cVar8 - pald);
//            if ((s8)(cVar8 - pald) <= (s8)bVar4)
//            {
//                uVar7 = uVar1 | bVar4;
//            }
//        }
//
//        *puVar11 = (u32)(u16)((u16)uVar7 << 8 | (u16)uVar7 >> 8) << 0x10 | uVar7 >> 0x10;
//        ftopal = 1;
//        sVar9 = sVar9 - 1;
//        if (sVar9 == -1)
//        {
//            ftopal = 1;
//            return in_D0;
//        }
//    }
//    while (1);
//}

//void ctopale1(void)
//{
//  s8 bVar1;
//  s16 sVar3;
//  u16 uVar4;
//  u32 uVar2;
//  s16 unaff_D6w;
//  s32 in_A0;
//  s8 *pbVar5;
//  s8 *pbVar6;
//  s8 *pbVar7;
//  u32 *puVar8;
//  s8 *pcVar9;
//  s8 *pbVar10;
//  u32 *puVar11;
//
//  pbVar5 = (s8 *)(in_A0 + 2);
//  uVar4 = (u16)*(s8 *)(in_A0 + 1);
//  if (*(s8 *)(in_A0 + 1) == 0) {
//    if (thepalet != '\0') {
//      thepalet = 0;
//      defpalet = 0;
//      return;
//    }
//    palc = 0;
//    sVar3 = 0xf;
//    puVar8 = atpalet;
//    do {
//      pbVar6 = pbVar5 + 1;
//      *(s8 *)puVar8 = *pbVar5 << 4;
//      pbVar5 = pbVar5 + 2;
//      bVar1 = *pbVar6;
//      *(s8 *)((s32)puVar8 + 1) = bVar1 & 0xf0;
//      pcVar9 = (s8 *)((s32)puVar8 + 3);
//      *(u8 *)((s32)puVar8 + 2) = 0;
//      puVar8 = puVar8 + 1;
//      *pcVar9 = bVar1 << 4;
//      sVar3 = sVar3 - 1;
//    } while (sVar3 != -1);
//  }
//  else {
//    palc = 0;
//    pbVar6 = (s8 *)(in_A0 + 4);
//    pbVar5 = (s8 *)((s32)(s16)((u16)*pbVar5 * 4) + (s32)atpalet);
//    if (fdarkpal == '\0') {
//      do {
//        *pbVar5 = *pbVar6 >> 1 & 0xfe;
//        pbVar7 = pbVar6 + 2;
//        pbVar5[1] = pbVar6[1] >> 1 & 0xfe;
//        pbVar10 = pbVar5 + 3;
//        pbVar5[2] = 0;
//        pbVar6 = pbVar6 + 3;
//        pbVar5 = pbVar5 + 4;
//        *pbVar10 = *pbVar7 >> 1 & 0xfe;
//        uVar4 = uVar4 - 1;
//      } while (uVar4 != 0xffff);
//    }
//    else {
//      tdarkpal();
//    }
//  }
//  thepalet = 0;
//  defpalet = 0;
//  if (unaff_D6w != 0) {
//    if (0x7d < unaff_D6w) {
//      pald = 1;
//      uVar2 = topalet();
//      palt = (u8)((u32)(s32)unaff_D6w / (uVar2 & 0xffff));
//      palt0 = palt;
//      palc = (s8)uVar2;
//      return;
//    }
//    pald = (s8)(0x7e / ((s32)unaff_D6w & 0xffffU)) + '\x01';
//    topalet();
//    palt = 1;
//    palt0 = 1;
//    palc = (s8)unaff_D6w;
//    return;
//  }
//  sVar3 = 0xff;
//  puVar8 = atpalet;
//  puVar11 = (u32 *)mpalet;
//  do {
//    *puVar11 = *puVar8;
//    sVar3 = sVar3 - 1;
//    puVar8 = puVar8 + 1;
//    puVar11 = puVar11 + 1;
//  } while (sVar3 != -1);
//  ftopal = 0xff;
//  return;
//}

void savecoord(u8 *at)
{
    oldcx = at[0x00];
    oldcy = at[0x04];
    oldcz = at[0x08];
    oldacx = at[0x0c];
    oldacy = at[0x10];
    oldacz = at[0x14];
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
    backsprite = 0x8028;
    texsprite = 0x8050;
    *(s32 *)(alis.spritemem + cursprit + 0x5c) = 0;
    *(s8 *) (alis.spritemem + cursprit + 0x50) = 0xfe;
    mousflag = 0;
//    s16 size = -0x7f88;
    libsprit = 0x78;
    cursprit += 0x78;
    
    do
    {
        *(s16 *)(SPRITEMEM_PTR + cursprit + 0x04) = cursprit + 0x30;
        cursprit += 0x30;

//        size += 0x28;
//        *(s16 *)(alis.spritemem + cursprit + 0x04) = size;
//        cursprit = base + size;
    }
    while (cursprit < finsprit);
    
    *(s16 *)(SPRITEMEM_PTR + cursprit + -0x24) = 0;

//    u16 sp = libsprit;
//    for (int i = 0; i < 100; i++)
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
    printf ("%s start\n", __PRETTY_FUNCTION__);

    u16 screen_id;
    s8 num;
    
    *previdx = 0;
    *curidx = alis.script->context._0x18_unknown;
    if (*curidx == 0)
    {
        printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
        return 1;
    }
    
    SpriteVariables *cursprvar = NULL;
    
    do
    {
        cursprvar = SPRITE_VAR(*curidx);
        screen_id = cursprvar->screen_id;  // *(u16 *)(SPRITEMEM_PTR + sprite_screen_id + *curidx);
        if (screen_id <= alis.script->context._0x16_screen_id)
        {
            if (alis.script->context._0x16_screen_id != screen_id)
            {
                printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
                return 1;
            }

            num = cursprvar->numelem;
            if (num <= numelem)
            {
                if (numelem != num)
                {
                    printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
                    return 1;
                }

                printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
                return 0;
            }
        }

        *previdx = *curidx;
        *curidx = cursprvar->to_next;
    }
    while (*curidx != 0);

    printf("   SEARCH cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
    return 0;
}

u8 testnum(u16 *curidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);
    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);
    return ((cursprvar->screen_id == alis.script->context._0x16_screen_id) && (cursprvar->numelem == numelem)) ? 1 : 0;
}

void nextnum(u16 *curidx, u16 *previdx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    *previdx = *curidx;
    *curidx = SPRITE_VAR(*curidx)->to_next;
    SpriteVariables *cursprvar = SPRITE_VAR(*curidx);
    if ((cursprvar->state != 0 && cursprvar->screen_id == alis.script->context._0x16_screen_id) && cursprvar->numelem == numelem)
    {
        printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
        return;
    }

    printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
}

void createlem(u16 *curidx, u16 *previdx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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

        s16 scrnidx = BASESP_VAR(alis.script->context._0x16_screen_id)->screen_id;
        SpriteVariables *scrnsprvar = SPRITE_VAR(scrnidx);

        cursprvar->state = 0xff;
        cursprvar->numelem = numelem;
        cursprvar->screen_id = alis.script->context._0x16_screen_id;
        cursprvar->to_next = *curidx;
        cursprvar->link = scrnsprvar->link;
        scrnsprvar->link = sprit;

        *curidx = sprit;
    }
    
    printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);
}

s16 delprec(u16 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    SpriteVariables *cursprvar = BASESP_VAR(elemidx);
    s16 scridx = cursprvar->screen_id;

    SpriteVariables *scrsprvar = BASESP_VAR(scridx);
    if (elemidx == scrsprvar->screen_id)
    {
        scrsprvar->screen_id = cursprvar->link;
        return scridx;
    }
    
    s16 tmpidx = scrsprvar->screen_id;
    SpriteVariables *tmpsprvar = BASESP_VAR(tmpidx);
    while (elemidx != tmpsprvar->link)
    {
        tmpidx = tmpsprvar->link;
        tmpsprvar = BASESP_VAR(tmpidx);
    }
    
    tmpsprvar->link = cursprvar->link;
    return tmpidx;
}

void killelem(u16 *curidx, u16 *previdx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);
    printf("   cursprite: 0x%.6x prevsprite: 0x%.6x numelem: 0x%.6x\n", *curidx, *previdx, numelem);

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
            return;
        }
    }
    
    *curidx = prevsprvar->to_next;
}

void put(u8 idx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    fmuldes = 0;
    putin(idx);
}

void putin(u8 idx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);
    printf("  idx: 0x%.2x numelem: 0x%.6x\n", idx, numelem);

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
            
            s16 curelem = read16(currsrc + 0, alis.platform);
            s16 curdepx = read16(currsrc + 2, alis.platform);
            if (*((u8 *)(&flaginvx)) != 0)
                curdepx = -curdepx;

            depx += curdepx;

            s16 curdepy = read16(currsrc + 4, alis.platform);
            depy = curdepy + y;

            s16 curdepz = read16(currsrc + 6, alis.platform);
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

            if (-1 < *(s8 *)(SPRITEMEM_PTR + cursprite))
            {
put30:
                *(u8 *)(SPRITEMEM_PTR + cursprite) = 2;
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
                    if (*(s8 *)(SPRITEMEM_PTR + cursprite) == 0)
                        goto put30;
                  
                    nextnum(&cursprite, &prevsprite);
                }
                while (cursprite != 0); //(bVar3);
            }
            
            createlem(&cursprite, &prevsprite);
        }
        
//        u32 unaff_A6 = 0;
        
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
}

s16 inilink(s16 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    blocx1 = 0x7fff;
    blocy1 = 0x7fff;
    blocx2 = 0x8000;
    blocy2 = 0x8000;
    return *(s16 *)(SPRITEMEM_PTR + elemidx + sprite_clinking);
}

void calcfen(s16 elemidx1, s16 elemidx3)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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
        s16 tmp1 = blocx1;
        if (blocx1 < idx3sprvar->newx)
            tmp1 = idx3sprvar->newx;
        
        feny1 = blocy1;
        if ((s16)blocy1 < idx3sprvar->newy)
            feny1 = idx3sprvar->newy;
        
        s16 tmp2 = blocx2;
        if (tmpx < blocx2)
            tmp2 = tmpx;
        
        feny2 = blocy2;
        if (tmpy < blocy2)
            feny2 = tmpy;
            
        fenx1 = tmp1 & 0xfff0;
        fenx2 = tmp2 | 0xf;
        fenlargw = (u16)((fenx2 - fenx1) + 1) >> 2;
        adfenintv = (u32)feny1 * 0x140 + physic + (s32)(s16)(fenx1 >> 1);
    }
}

u8 clipfen(u8 *sprite)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    fclip = 0;
    s16 spritex2 = *(s16 *)(sprite + sprite_newx);
    if (fenx2 < spritex2)
        return fclip;

    s16 spritey2 = *(s16 *)(sprite + sprite_newy);
    if (feny2 < spritey2)
        return fclip;

    s16 spritex1 = *(s16 *)(sprite + sprite_depx);
    if (spritex1 < fenx1)
        return fclip;

    s16 spritey1 = *(s16 *)(sprite + sprite_depy);
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

void rangesprite(s16 elemidx1, s16 elemidx3)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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
            break;
        
        previdx3 = nextidx3;
        nextidx3 = SPRITE_VAR(nextidx3)->link;
    }

    idx1sprvar->link = nextidx3;
    SPRITE_VAR(previdx3)->link = elemidx1;
    idx1sprvar->state = 0;
}

void tstjoints(s16 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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

u32 scalaire(SpriteVariables *sprite, u32 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    if ((s8)sprite->numelem < 0)
        return elemidx;
    
//    return (elemidx & 0x0000ffff) | ((u32)sprite->sprite_0x28 << 4);
    return (elemidx & 0xffff0000) | sprite->sprite_0x28;
}

void depscreen(SpriteVariables *sprite, s16 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    sprite->depx += sprite->height;
    sprite->depy += sprite->newzoomx;
    sprite->depz += sprite->newzoomy;
    
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

void deptopix(SpriteVariables *scrsprvar, u32 d0, s16 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    if ((scrsprvar->numelem & 2) == 0)
    {
        SpriteVariables *elemsprvar = SPRITE_VAR(elemidx);
        
        s16 scridx = elemsprvar->screen_id;
        SpriteVariables *sprite = BASESP_VAR(scridx);
        newf = elemsprvar->flaginvx;
        s16 tmpdepx = elemsprvar->depx - *(s16 *)(sprite + sprite_depx);
        s16 tmpdepy = elemsprvar->depy - *(s16 *)(sprite + sprite_depy);
        s16 tmpdepz = elemsprvar->depz - *(s16 *)(sprite + sprite_depz);
        
        // TODO: test on atari!
        d0 = scalaire(sprite, d0);
        
        s16 sVar3 = 0;
        u8 cred = sprite->creducing;
        if ((-1 < (s8)cred) && -1 < (s8)elemsprvar->credon_off)
        {
            if (tmpdepz == 0)
            {
                tmpdepz = 1;
            }
            
            tmpdepx = (s16)(((s32)tmpdepx << (cred & 0x3f)) / (s32)tmpdepz);
            tmpdepy = (s16)(((s32)tmpdepy << (cred & 0x3f)) / (s32)tmpdepz);
            scridx = (tmpdepz >> (elemsprvar->credon_off & 0x3f)) + (s16)elemsprvar->creducing;
            if ((s16)scridx < 0)
            {
                sVar3 = 0;
            }
            else
            {
                if ((u16)(s16)sprite->clinking < scridx)
                {
                    scridx = (s16)sprite->clinking;
                }
                
                sVar3 = scridx << 2;
            }
        }
        
        tmpdepx = (tmpdepx >> (sprite->credon_off & 0x3f)) + sprite->newd;
        tmpdepy = (tmpdepy >> (sprite->credon_off & 0x3f)) + sprite->newx;
        
        newad = (elemsprvar->data & 0xffffff);
        // newad = (elemsprvar->data & 0xffffff) + sVar3);
        u8 *spritedata = (alis.mem + alis.script->data_org + newad);
        if (spritedata[0] == '\x03')
        {
            tmpdepx += (elemsprvar->flaginvx ? -1 : 1) * read16(spritedata + 4, alis.platform);
            tmpdepy += read16(spritedata + 6, alis.platform);
            if (spritedata[1] != 0)
            {
                newf = newf ^ 1;
            }
            
            newad = (newad + (read16(spritedata + 2, alis.platform) << 2));
            spritedata = (alis.mem + alis.script->data_org + newad);
        }
        
        newl = read16(spritedata + 2, alis.platform);
        newh = read16(spritedata + 4, alis.platform);
        newzoomx = 0;
        newzoomy = 0;
        newx = tmpdepx - (newl >> 1);
        newy = tmpdepy - (newh >> 1);
        newd = tmpdepz;
    }
}

void waitdisk(void)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);
}

void waitphysic(void)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);
    do {} while (fphysic != '\0');
}

void phytolog(void)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);
    fenx1 = 0;
    feny1 = 0;
    fenx2 = 0x13f;
    feny2 = 199;
//    trsfen();
}

void mouserase(void)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);
    
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
            puVar8 = (u32 *)((s32)puVar8 + sVar3 + sprite_newd);
            puVar5 = (u32 *)((s32)puVar5 + sVar2 + sprite_newd);
            sVar4 --;
        }
        while (sVar4 != -1);
    }
}

void oldfen(void)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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
    printf ("%s start\n", __PRETTY_FUNCTION__);
//    if (insid == '\0')
//    {
//        DAT_00ff8203 = (u8)((u32)physic >> 8);
//        DAT_00ff8201 = (u8)((u32)physic >> 0x10);
//    }
//
//    bufpack = logic;
//    fphysic = 1;
}

u8 *folscreen(u8 *sprite)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    s16 *psVar1 = *(s16 **)(atent + *(s16 *)(sprite + 0x60));
    if ((-1 < *(s16 *)(sprite + 0x60)) && (psVar1 != (s16 *)0x0))
    {
        if ((sprite[0x84] & 1) != 0)
        {
            if ((s16)(*psVar1 + *(s16 *)(sprite + 0x86)) != *(s16 *)(sprite + sprite_depx))
            {
                *(s16 *)(sprite + sprite_depx) = *psVar1 + *(s16 *)(sprite + 0x86);
                *sprite = *sprite | 0x80;
            }
            if ((s16)(psVar1[4] + *(s16 *)(sprite + 0x88)) != *(s16 *)(sprite + sprite_depy))
            {
                *(s16 *)(sprite + sprite_depy) = psVar1[4] + *(s16 *)(sprite + 0x88);
                *sprite = *sprite | 0x80;
            }
            if ((s16)(psVar1[8] + *(s16 *)(sprite + 0x8a)) != *(s16 *)(sprite + sprite_depz))
            {
                *(s16 *)(sprite + sprite_depz) = psVar1[8] + *(s16 *)(sprite + 0x8a);
                *sprite = *sprite | 0x80;
            }
        }
        
        if ((sprite[0x84] & 2) != 0)
        {
            s16 sVar2 = psVar1[sprite_newx] + psVar1[0x20] + *(s16 *)(sprite + 0x8c);
            if (sVar2 != *(s16 *)(sprite + 0x34))
            {
                *(s16 *)(sprite + 0x34) = sVar2;
                *sprite = *sprite | 0x80;
            }
            sVar2 = psVar1[sprite_newd] + psVar1[0x22] + *(s16 *)(sprite + 0x8e);
            if (sVar2 != *(s16 *)(sprite + 0x36))
            {
                *(s16 *)(sprite + 0x36) = sVar2;
                *sprite = *sprite | 0x80;
            }
            sVar2 = psVar1[0x14] + psVar1[0x24] + *(s16 *)(sprite + 0x90);
            if (sVar2 != *(s16 *)(sprite + 0x38))
            {
                *(s16 *)(sprite + 0x38) = sVar2;
                *sprite = *sprite | 0x80;
            }
        }
    }
    
    return sprite;
}

void addlink(s16 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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

void inouvlink(SpriteVariables *scsprvar, u32 varD0, s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    deptopix(scsprvar, varD0, elemidx1);
    
    SpriteVariables *elem1sprvar = SPRITE_VAR(elemidx1);
    elem1sprvar->newad = newad;
    elem1sprvar->newx = newx;
    elem1sprvar->newd = newd;
    elem1sprvar->newf = newf;
    elem1sprvar->width = newl;
    elem1sprvar->width = newl;
    elem1sprvar->height = newh;
    elem1sprvar->newzoomx = newzoomx;
    elem1sprvar->newzoomy = newzoomy;

    SpriteVariables *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    rangesprite(elemidx1, elemidx3);
}

void iremplink(SpriteVariables *scsprvar, u32 d0, s16 elemidx1, s16 elemidx2, s16 elemidx3)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    addlink(elemidx1);
    
    SpriteVariables *elem1sprvar = SPRITE_VAR(elemidx1);
    if (wback != 0 && (s16)backprof <= elem1sprvar->newd)
    {
        pback = 1;
        inouvlink(scsprvar, backprof, elemidx1, elemidx2, elemidx3);
        return;
    }
    
    deptopix(scsprvar, d0, elemidx1);
    
    elem1sprvar->newad = newad;
    elem1sprvar->newx = newx;
    elem1sprvar->newd = newd;
    elem1sprvar->newf = newf;
    elem1sprvar->width = newl;
    elem1sprvar->width = newl;
    elem1sprvar->height = newh;
    elem1sprvar->newzoomx = newzoomx;
    elem1sprvar->newzoomy = newzoomy;

    SpriteVariables *elem2sprvar = SPRITE_VAR(elemidx2);
    elem2sprvar->link = elem1sprvar->link;

    addlink(elemidx1);
    rangesprite(elemidx1, elemidx3);
}

void iefflink(s16 elemidx1, s16 elemidx2)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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
    printf ("%s start\n", __PRETTY_FUNCTION__);

    s16 tmpx = fenx2 - fenx1;
    // s16 tmpy = feny2 - feny1;
    
    for (s16 y = feny1; y < feny2; y++)
    {
        memset(physic + fenx1 + y * 320, 0, tmpx);
    }
}

void clipback(void)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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

u32 fenetre(u32 param_2, u16 elemidx)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    do
    {
        elemidx = *(u16 *)(SPRITEMEM_PTR + elemidx + sprite_link);
        if (elemidx == 0)
        {
            return param_2;
        }
        
        u8 command = *(u8 *)(SPRITEMEM_PTR + elemidx + sprite_state);
        printf("   command: 0x%.2x\n", command);
        if (command != 0)
        {
            if (command < 0xff)
            {
                printf("   ???: 0x%.2x\n", command);
            }
            
            continue;
        }

        u32 spnewad = (*(u32 *)(SPRITEMEM_PTR + elemidx + sprite_newad) & 0xffffff);
        if (spnewad == 0)
        {
            printf("   drawing error: uninitialized data!\n");
        }
        
        u8 *sprite = (alis.mem + alis.script->data_org + spnewad);
        
        if (wback || pback)
        {
            continue;
        }
        
//        if (wback == 0 && pback == 0)
        {
            u8 *at = sprite + 6;
            
            s32 clear = 0;
            
            SpriteVariables *elemsprvar = SPRITE_VAR(elemidx);
            u32 newx2 = elemsprvar->newx;
            u32 newd2 = elemsprvar->newd;
            u32 newf2 = elemsprvar->newf;
            u32 width = elemsprvar->width + 1;
            u32 height = elemsprvar->height + 1;

            printf("   drawing: 0x%.6x\n", elemidx);
            printf("     to:  %.3dx%.3d\n", newx2, newd2);
            printf("     dim: %.3dx%.3d\n", width, height);

            s32 xx = newx2;
            s32 yy = host.pixelbuf0.h - (newd2 + ((height + 1) / 2));
            
            s32 vs = 0;
            s32 vf = yy;
            s32 vt = height;
            if (vf < 0)
            {
                vf = 0;
                vt += yy;
                vs -= yy;
            }
            
            s32 hs = 0;
            s32 hf = xx;
            s32 ht = width;
            if (hf < 0)
            {
                hf = 0;
                ht += xx;
                hs -= xx;
            }
            
            // if ((entry->type == image4ST) ||(entry->type == image4) ||(entry->type == image8))
            if (sprite[0] == 0 || sprite[0] == 2) // 4 b color
            {
                s32 clear = 0;
//                if (entry->type == image4)
//                {
//                    clear = entry->params[4] + entry->params[5];
//                }
//
//                if (entry->type == image8)
//                {
//                    clear = entry->params[5];
//                }
                
                for (s32 h = vs; h < vs + vt; h++)
                {
                    for (s32 w = hs; w < hs + ht; w++)
                    {
                        if (xx + w < 0)
                            continue;
                        
                        if (xx + w > 319)
                            break;
                        
                        u16 wh = w / 2;
                        u8 *at2 = at + (newf2 ? width - (wh + 1) : wh) + h * (width / 2);
                        u8 r = *at2;
                        u8 a = ((r & 0b11110000) >> 4);
                        u8 b = (r & 0b00001111);
                        
                        uint8_t color = newf2 + w % 2 == 0 ? a : b;
                        if (color != clear)
                        {
                            uint8_t *ptr = host.pixelbuf0.data + xx + w + ((yy + h) * host.pixelbuf0.w);
                            *ptr = color;
                        }
                    }
                }
            }
            else if (sprite[0] == 0x1) // rectangle
            {
                hf = max(0, xx);
                ht = min(319, xx + width);
                if (hf < 319 && ht - hf > 0)
                {
                    for (s32 h = vs; h < vt; h++)
                    {
                       // memset(host.pixelbuf0.data + hf + ((yy + h) * host.pixelbuf0.w), 6 + ht % 6, ht - hf);
                    }
                }
            }
        }
    }
    while (true);
    
//    for (s32 h = vs; h < vs + vt; h++)
//    {
//        for (s32 w = hs; w < hs + ht; w++)
//        {
//            if (xx + w < 0)
//                continue;
//
//            if (xx + w > 319)
//                break;
//
//            uint8_t color = entry->buffer.data[(!inv ? width - (w + 1) : w) + h * width];
//            if (color != clear)
//            {
//                uint8_t *ptr = host.pixelbuf0.data + xx + w + ((yy + h) * host.pixelbuf0.w);
//                *ptr = color;
//            }
//        }
//    }

        
//        return (_entry_map[index] = new Entry(data_type::image4ST, params, Buffer(data, width * height)));

//  u16 uVar1;
//  s8 cVar2;
//  s16 sVar3;
//  u32 unaff_D3;
//  s8 *pcVar4;
//  u32 unaff_D4;
//  s8 *pcVar5;
//  u8 *unaff_A2;
//  bool bVar6;
//  s8 **ppcVar7;
//  u16 auStack_40 [2];
//  s8 *local_3c;
//  s8 *pcStack_38;
//
//  if ((*unaff_A2 & 0x40) != 0)
//  {
//    return param_2;
//  }
//
//  ppcVar7 = &local_3c;
//  scdirect = 0;
//  bVar6 = true;
//  local_3c = param_2;
//  pcStack_38 = param_3;
//  calcfen((s16)param_3,basesprite);
//  if (bVar6) {
//    if ((unaff_A2[1] & 2) != 0) {
//      fdoland = 1;
//    }
//    pcVar4 = (s8 *)(unaff_D3 & 0xffff0000 | (u32)ptscreen);
//    uVar1 = ptscreen;
//    while (cVar2 = fremouse, uVar1 != 0)
//    {
//      if (((BASEMNMEM_PTR)[(s16)pcVar4] & 0x40U) == 0)
//      {
//        uVar1 = *(u16 *)(basemain + (s16)pcVar4 + 2);
//        param_3 = (s8 *)((u32)param_3 & 0xffff0000 | (u32)uVar1);
//        if (uVar1 != 0)
//        {
//          ppcVar7[-1] = basemain;
//          ppcVar7[-2] = pcVar4;
//          ppcVar7[-3] = param_3;
//          ppcVar7[-4] = (s8 *)0x18faa;
//          clipfen(uVar1);
//          param_3 = ppcVar7[-3];
//          pcVar4 = ppcVar7[-2];
//          pcVar5 = ppcVar7[-1];
//          if (fclip != '\0')
//          {
//            sVar3 = (s16)pcVar4;
//            if ((pcVar5[sVar3 + 1] & 2U) == 0)
//            {
//              cback = '\0';
//              if ((pcVar5[sVar3 + 1] & 4U) != 0)
//              {
//                *(s16 *)(ppcVar7 + -1) = (s16)param_3;
//                ppcVar7[-2] = (s8 *)0x18fe4;
//                clipback();
//                param_3 = (s8 *)(s32)*(s16 *)(ppcVar7 + -1);
//                ppcVar7 = (s8 **)((s32)ppcVar7 + -2);
//              }
//              if ((pcVar5[sVar3 + 1] & 0x40U) == 0)
//              {
//                *(u32 *)((s32)ppcVar7 + -4) = 0x18ff8;
//                param_2 = (s8 *)clrfen();
//              }
//
//              if (cback != '\0')
//              {
//                if (cback < '\0')
//                {
//                  if ((wback != '\0') && (pback != '\0'))
//                  {
//                    *(s8 **)((s32)ppcVar7 + -4) = param_3;
//                    *(s16 *)((s32)ppcVar7 + -6) = clipy1;
//                    *(s16 *)((s32)ppcVar7 + -8) = clipy2;
//                    unaff_D4 = unaff_D4 & 0xffff0000 | (u32)backsprite;
//                    wlogic = backmap;
//                    wlogx1 = backx1;
//                    wlogx2 = backx2;
//                    wloglarg = backlarg;
//                    if (clipy1 <= (s16)backx1)
//                    {
//                      clipy1 = (s16)backx1;
//                    }
//
//                    param_2 = (s8 *)((u32)param_2 & 0xffff0000 | backx2 & 0xffff);
//                    sVar3 = (s16)(backx2 & 0xffff);
//                    if (sVar3 <= clipy2)
//                    {
//                      clipy2 = sVar3;
//                    }
//
//                    while( true )
//                    {
//                      uVar1 = *(u16 *)(SPRITEMEM_PTR + 6 + (s32)(s16)param_3);
//                      param_3 = (s8 *)((u32)param_3 & 0xffff0000 | (u32)uVar1);
//                      if ((uVar1 == 0) || (uVar1 == (u16)unaff_D4))
//                          break;
//
//                      pcVar5 = (s8 *)(SPRITEMEM_PTR + (s16)uVar1);
//                      if (((-1 < *pcVar5) && (-1 < pcVar5[8])) && (-1 < *(s16 *)(pcVar5 + sprite_newd)))
//                      {
//                        *(u8 **)((s32)ppcVar7 + -0xc) = unaff_A2;
//                        *(s32 *)((s32)ppcVar7 + -0x10) = basesprite;
//                        *(u32 *)((s32)ppcVar7 + -0x14) = unaff_D4;
//                        *(s8 **)((s32)ppcVar7 + -0x18) = pcVar4;
//                        *(s8 **)((s32)ppcVar7 + -0x1c) = param_3;
//                        *(u32 *)((s32)ppcVar7 + -0x20) = 0x19236;
//                        destofen(pcVar5);
//                        param_3 = *(s8 **)((s32)ppcVar7 + -0x1c);
//                        pcVar4 = *(s8 **)((s32)ppcVar7 + -0x18);
//                        unaff_D4 = *(u32 *)((s32)ppcVar7 + -0x14);
//                        basesprite = *(s32 *)((s32)ppcVar7 + -0x10);
//                        unaff_A2 = *(u8 **)((s32)ppcVar7 + -0xc);
//                      }
//                    }
//
//                    wlogic = logic;
//                    wlogx1 = logx1;
//                    wlogx2 = logx2;
//                    wloglarg = loglarg;
//                    clipy2 = *(s16 *)((s32)ppcVar7 + -8);
//                    clipy1 = *(s16 *)((s32)ppcVar7 + -6);
//                    param_3 = *(s8 **)((s32)ppcVar7 + -4);
//                  }
//                }
//                else
//                {
//                  param_2 = (s8 *)((u32)param_2 & 0xffff0000 | (u32)param_3 & 0xffff);
//                  unaff_D4 = unaff_D4 & 0xffff0000 | (u32)backsprite;
//                  pcVar5 = (s8 *)((u32)param_3 & 0xffff0000 | (u32)backsprite);
//                  if (pback == '\0')
//                      goto fenetre31;
//
//                  wlogic = backmap;
//                  wlogx1 = backx1;
//                  wlogx2 = backx2;
//                  wloglarg = backlarg;
//                }
//              }
//
//              while( true )
//              {
//                uVar1 = *(u16 *)(SPRITEMEM_PTR + 6 + (s32)(s16)param_3);
//                param_3 = (s8 *)((u32)param_3 & 0xffff0000 | (u32)uVar1);
//                if (uVar1 == 0)
//                    break;
//
//                pcVar5 = param_3;
//                if ((sback != '\0') && (uVar1 == (u16)unaff_D4))
//                {
//                  wlogic = logic;
//                  wlogx1 = logx1;
//                  wlogx2 = logx2;
//                  wloglarg = loglarg;
//                }
//fenetre31:
//                param_3 = pcVar5;
//                pcVar5 = (s8 *)(SPRITEMEM_PTR + (s16)param_3);
//                if (((-1 < *pcVar5) && (-1 < pcVar5[8])) && (-1 < *(s16 *)(pcVar5 + sprite_newd)))
//                {
//                  *(u8 **)((s32)ppcVar7 + -4) = unaff_A2;
//                  *(s32 *)((s32)ppcVar7 + -8) = basesprite;
//                  *(u32 *)((s32)ppcVar7 + -0xc) = unaff_D4;
//                  *(s8 **)((s32)ppcVar7 + -0x10) = pcVar4;
//                  *(s8 **)((s32)ppcVar7 + -0x14) = param_3;
//                  *(u32 *)((s32)ppcVar7 + -0x18) = 0x190b4;
//                  destofen(pcVar5);
//                  param_3 = *(s8 **)((s32)ppcVar7 + -0x14);
//                  pcVar4 = *(s8 **)((s32)ppcVar7 + -0x10);
//                  unaff_D4 = *(u32 *)((s32)ppcVar7 + -0xc);
//                  basesprite = *(s32 *)((s32)ppcVar7 + -8);
//                  unaff_A2 = *(u8 **)((s32)ppcVar7 + -4);
//                  switchgo = 1;
//                }
//              }
//            }
//            else
//            {
//              ppcVar7[-1] = "`";
//              param_2 = (s8 *)calctop();
//            }
//          }
//        }
//      }
//
//      uVar1 = *(u16 *)(basemain + (s16)pcVar4 + 4);
//      pcVar4 = (s8 *)((u32)pcVar4 & 0xffff0000 | (u32)uVar1);
//    }
//
//    if (((-1 < fmouse) && (fmouse != '\x02')) && (fswitch == '\0'))
//    {
//      do
//      {
//        fremouse = cVar2 + '\x01';
//      }
//        while (fremouse != '\0');
//
//      if (tvmode == '\0')
//      {
//        ppcVar7[-1] = "\b*";
//        mousefen((s32)param_2);
//      }
//      else
//      {
//        ppcVar7[-1] = "`";
//        mouserase();
//      }
//    }
//    if ((unaff_A2[1] & 0x20) == 0)
//    {
//      if (fswitch != '\0')
//      {
//        if (wpag == '\0')
//        {
//          ppcVar7[-1] = "`";
//          memfen(*ppcVar7,(s16)param_3,basesprite);
//        }
//        goto fenetrex;
//      }
//
//      if (wpag == '\0')
//      {
//        ppcVar7[-1] = "`";
//        fentotv();
//      }
//    }
//
//    if (fswitch == '\0')
//    {
//      if ((tvmode != '\0') && (-1 < fmouse))
//      {
//        ppcVar7[-1] = (s8 *)0x19176;
//        mouseput();
//      }
//      fremouse = -1;
//    }
//  }
//fenetrex:
//  return *ppcVar7;
    return 0;
}

u32 affiscr(SpriteVariables *scsprvar, u32 d0, u16 elemidx3)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

    u8 state;
    bool issprit;
    u8 uVar8;
    u16 uvaridx;
    u16 nextidx;
    s16 linkidx;
    s16 previdx;
    s16 elemidx;
    s16 tempidx;

    u32 varD0 = d0;

    // NOTE: falcon specific
//    if (((u8 *)scsprvar)[0x84] != '\0')
//    {
//        scsprvar = (s8 *)folscreen((u8 *)scsprvar);
//    }
    
    if (fremap != 0 || scsprvar->state < 0)
    {
        depscreen(scsprvar, elemidx3);
    }
    
    if ((scsprvar->numelem & 2) == 0 || (scsprvar->state & 0x80U) == 0)
    {
        wback = (scsprvar->numelem & 4) != 0;
        if (wback)
        {
            printf("    WBACK!!!\n");
        }
        
        wpag = '\0';
        if ((scsprvar->state & 0x20U) != 0)
        {
            wpag = '\x01';
            spag = spag + -1;
            if (spag == '\0')
            {
                wpag = -1;
            }
        }
        
        SpriteVariables *nextsprvar = NULL;
        
        if ((scsprvar->numelem & 0x10) == 0)
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

                varD0 = (varD0 & 0xffffff00) | (u32)state;
                
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
                                varD0 = (varD0 & 0xffff0000) | (u32)backprof;
                                uVar8 = backprof == nextsprvar->newd;
                                if ((s16)backprof <= nextsprvar->newd)
                                {
                                    pback = 1;
                                    uVar8 = false;
                                }
                            }
                            
                            deptopix(scsprvar, d0, nextidx);
                            tstjoints(nextidx);
                            
                            if ((bool)uVar8)
                            {
                                *(u16 *)(SPRITEMEM_PTR + previdx + sprite_link) = nextsprvar->link;
                                varD0 = fenetre(varD0, nextidx);
                            }
                            else
                            {
                                *(u16 *)(SPRITEMEM_PTR + previdx + sprite_link) = nextsprvar->link;
                            }
                            
                            nextsprvar->newad = newad;
                            nextsprvar->newx = newx;
                            nextsprvar->newd = newd;
                            nextsprvar->newf = newf;
                            nextsprvar->width = newl;
                            nextsprvar->height = newh;
                            nextsprvar->newzoomx = newzoomx;
                            nextsprvar->newzoomy = newzoomy;
                            
                            elemidx = nextidx;
                            rangesprite(elemidx, elemidx3);
                            varD0 = fenetre(varD0, elemidx);
                            // pcVar3 = (s8 *)((u32)pcVar3 & 0xffff0000 | uVar9 & 0xffff);
                            nextidx = elemidx3;
                            goto affiscin;
                        }
                        
                        iremplink(scsprvar, varD0, tempidx, elemidx, elemidx3);
                    }
                    else
                    {
                        if (state == 0xff)
                        {
                            // handle new sprites
                            linkidx = inilink(nextidx);
                            inouvlink(scsprvar, varD0, tempidx, elemidx, elemidx3);
                        }
                        else
                        {
                            linkidx = inilink(nextidx);
                            iefflink(tempidx, elemidx);
                            tempidx = elemidx;
                        }
                        
                        if (linkidx < 0)
                        {
                            joints = 1;
                            varD0 = fenetre(varD0, elemidx);
                            // pcVar3 = (s8 *)((u32)pcVar3 & 0xffff0000 | uVar9 & 0xffff);
                            nextidx = elemidx3;
                            goto affiscin;
                        }
                    }
                    
                    while (true)
                    {
                        elemidx = tempidx;
                        nextidx = SPRITE_VAR(elemidx)->link;
                        tempidx = nextidx;
                        if (nextidx == 0)
                            break;
                        
                        nextsprvar = SPRITE_VAR(nextidx);
                        if (linkidx == nextsprvar->clinking)
                        {
                            state = nextsprvar->state;
                            
                            varD0 = (varD0 & 0xffffff00) | state;
                            
                            if (state != 0)
                            {
                                if ((s8)state < 0)
                                {
                                    inouvlink(scsprvar, varD0, nextidx, elemidx, elemidx3);
                                }
                                else if (state == 2)
                                {
                                    iremplink(scsprvar, varD0, nextidx, elemidx, elemidx3);
                                }
                                else
                                {
                                    iefflink(nextidx, elemidx);
                                    nextidx = elemidx;
                                }
                            }
                        }
                    }
                    
                    joints = 1;
                    varD0 = fenetre(varD0, previdx);
//                  pcVar3 = (s8 *)((u32)pcVar3 & 0xffff0000 | uVar9 & 0xffff);
                    nextidx = elemidx3;
                }
            }
            while (true);
        }
        
        fenx1 = scsprvar->newy;
        feny1 = scsprvar->newd;
        fenx2 = fenx1 + scsprvar->data;
        feny2 = feny1 + *(s16 *)(scsprvar + 0x14);
        clipl = (fenx2 - fenx1) + 1;
        cliph = (feny2 - feny1) + 1;
        
        fenlargw = clipl >> 2;
        varD0 = (varD0 & 0xffff0000) | (u32)fenlargw;
        
        clipx1 = fenx1;
        clipy1 = feny1;
        clipx2 = fenx2;
        clipy2 = feny2;
        nextidx = elemidx3;
        if ((scsprvar->numelem & 0x40) == 0)
        {
            clrfen();
        }
        
        SpriteVariables *prevsprvar = NULL;
        while ((nextidx = SPRITE_VAR((uvaridx = nextidx))->link) != 0)
        {
            nextsprvar = SPRITE_VAR(nextidx);
            state = nextsprvar->state;

            varD0 = (varD0 & 0xffffff00) | state;

            if (state != 0)
            {
                prevsprvar = SPRITE_VAR(uvaridx);

                if (state == 1)
                {
                    prevsprvar->link = nextsprvar->link;
                    nextsprvar->to_next = libsprit;
                    libsprit = nextidx;
                    nextidx = uvaridx;
                }
                else
                {
                    deptopix(scsprvar, d0, nextidx);
                    
                    prevsprvar->link = nextsprvar->link;
                    nextsprvar->newad = newad;
                    nextsprvar->newx = newx;
                    nextsprvar->newd = newd;
                    nextsprvar->newf = newf;
                    nextsprvar->width = newl;
                    nextsprvar->height = newh;
                    nextsprvar->newzoomx = newzoomx;
                    nextsprvar->newzoomy = newzoomy;
                    
                    rangesprite(nextidx, elemidx3);
                    nextidx = uvaridx;
                }
            }
        }
        
        issprit = ptscreen == 0;
        elemidx = ptscreen;
    }
    else
    {
        issprit = *(s16 *)(scsprvar + sprite_to_next) == 0;
        elemidx = *(s16 *)(scsprvar + sprite_to_next);
    }
    
    SpriteVariables *elemsprvar = NULL;
    SpriteVariables *linksprvar = NULL;

    while (!issprit)
    {
        elemsprvar = BASESP_VAR(elemidx);
        
        if ((((elemsprvar->state & 0x40U) == 0) && (linkidx = elemsprvar->screen_id != 0)) && clipfen(SPRITEMEM_PTR + linkidx) != 0)
        {
            while ((linkidx = SPRITE_VAR(linkidx)->link) != 0)
            {
                linksprvar = SPRITE_VAR(linkidx);
                if (-1 < (s8)linksprvar->state && -1 < (s8)linksprvar->newf && -1 < linksprvar->newd)
                {
                    // destofen(sprite);
                    switchgo = 1;
                }
            }
        }
        
        issprit = *(s16 *)(BASEMNMEM_PTR + elemidx + sprite_to_next) == 0;
        elemidx = *(s16 *)(BASEMNMEM_PTR + elemidx + sprite_to_next);
    }
    
    if ((fswitch == '\0') && (wpag == '\0'))
    {
//      fentotv();
    }
    
affiscr1:
    
    if (wpag < '\0')
    {
        scsprvar->state &= 0xdf;
        fenx1 = scsprvar->newy;
        feny1 = scsprvar->newd;
        fenx2 = scsprvar->newy + scsprvar->data;
        feny2 = scsprvar->newd + *(s16 *)((u8 *)scsprvar + 0x14);
        // scrolpage(basesprite,BASEMNMEM_PTR,(u32)varD0 & 0xffff0000 | (u32)feny2);
    }
    
    scsprvar->state &= 0x7f;
    return d0;
}

void image(void)
{
    printf ("%s start\n", __PRETTY_FUNCTION__);

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
    wloglarg = loglarg;
    
    u16 scsprite = ptscreen;
    prevmouse = fremouse;
    u8 *oldscr_ptr = physic;
    
    while (scsprite != 0)
    {
        SpriteVariables *sprite_addr = BASESP_VAR(scsprite);
        if ((sprite_addr->state & 0x40) == 0)
        {
            u16 sprite3 = sprite_addr->screen_id;
            affiscr(sprite_addr, scsprite, sprite3);
        }

        scsprite = sprite_addr->to_next;
    }
//    u8 *sprite_addr = (u8 *)(BASEMNMEM_PTR + 0x42);
//    if ((*sprite_addr & 0x40) == 0)
//    {
//        u16 sprite3 = *(u16 *)(sprite_addr + sprite_screen_id);
//        affiscr(sprite_addr, scsprite, sprite3);
//    }

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

void draw(u8 *data)
{
    image();
    return;
    
    memset(host.pixelbuf0.data, 0, host.pixelbuf0.w * host.pixelbuf0.h);

    printf("\n");
    u16 sprite_idx = alis.script->context._0x18_unknown;
    if (sprite_idx == 0)
        return;
    
    u8 *sprite;
    alis_rsrc *entry;

    s32 minidx = 0xff;
    s32 maxidx = 0x00;
    
    s32 old_idx = sprite_idx;
    s32 other_idx = sprite_idx;

    do
    {
        sprite = SPRITEMEM_PTR + sprite_idx;
        u8 num = sprite[sprite_numelem];
        if (minidx > num)
            minidx = num;
        
        if (maxidx < num)
            maxidx = num;
        
        // get next
        
        old_idx = sprite_idx;
        sprite_idx = *(u16 *)(sprite + sprite_to_next);
        other_idx = *(u16 *)(sprite + sprite_link);
    }
    while (sprite_idx != 0);


    for (s32 i = minidx; i <= maxidx; i++)
    {
        u16 sprite_idx = alis.script->context._0x18_unknown;

        do
        {
            sprite = SPRITEMEM_PTR + sprite_idx;
            u8 vis = sprite[sprite_state];
            u8 num = sprite[sprite_numelem];
            
//            if (i != num)
//                continue;
            
            // -
            u8 idx = *(s16 *)(sprite + sprite_data + 1);
            u8 inv = !sprite[sprite_flaginvx];
            s16 px = sprite[sprite_depx];
            s16 py = sprite[sprite_depy];
            s16 pz = sprite[sprite_depz];
            
//            printf("  DRAW 0x%.4x %.2d %.2d at: %.3d x %.3d x %.3d\n", sprite_idx, num, idx, px, py, pz);
            // -
//            if (vis == 0)
//                break;
            
            if (vis && i == num)
            {
                printf("  DRAW 0x%.4x %.2d %.2d at: %.3d x %.3d x %.3d\n", sprite_idx, num, idx, px, py, pz);
                // u8 idx = *(s16 *)(sprite + sprite_0x28);
                entry = alis.script->resources[idx];
                if (entry)
                {
                    //                u8 inv = sprite[sprite_flaginvx];
                    //                s16 px = sprite[sprite_depx];
                    //                s16 py = sprite[sprite_depy];
                    //                s16 pz = sprite[sprite_depz];
                    
                    int16_t x = px;
                    int16_t z = pz;
                    int16_t y = z + py;
                    
                    uint32_t width = entry->params[2];
                    uint32_t height = entry->params[3];
                    
                    s32 xx = 1 + x - ((width + 1) / 2);
                    s32 yy = host.pixelbuf0.h - (y + ((height + 1) / 2));
                    
                    s32 vs = 0;
                    s32 vf = yy;
                    s32 vt = height;
                    if (vf < 0)
                    {
                        vf = 0;
                        vt += yy;
                        vs -= yy;
                    }
                    
                    s32 hs = 0;
                    s32 hf = xx;
                    s32 ht = width;
                    if (hf < 0)
                    {
                        hf = 0;
                        ht += xx;
                        hs -= xx;
                    }
                    
                    if ((entry->type == image4ST) ||(entry->type == image4) ||(entry->type == image8))
                    {
                        s32 clear = 0;
                        if (entry->type == image4)
                        {
                            clear = entry->params[4] + entry->params[5];
                        }
                        
                        if (entry->type == image8)
                        {
                            clear = entry->params[5];
                        }
                        
                        for (s32 h = vs; h < vs + vt; h++)
                        {
                            for (s32 w = hs; w < hs + ht; w++)
                            {
                                if (xx + w < 0)
                                    continue;
                                
                                if (xx + w > 319)
                                    break;
                                
                                uint8_t color = entry->buffer.data[(!inv ? width - (w + 1) : w) + h * width];
                                if (color != clear)
                                {
                                    uint8_t *ptr = host.pixelbuf0.data + xx + w + ((yy + h) * host.pixelbuf0.w);
                                    *ptr = color;
                                }
                            }
                        }
                    }
                    else if (entry->type == rectangle/* && height == 46*/)
                    {
                        hf = max(0, xx);
                        ht = min(319, xx + width);
                        if (hf < 319 && ht - hf > 0)
                        {
                            for (s32 h = vs; h < vt; h++)
                            {
                                memset(host.pixelbuf0.data + hf + ((yy + h) * host.pixelbuf0.w), 6 + num %4, ht - hf);
                            }
                        }
                    }
                    else
                    {
                        debug(EDebugWarning, " /* unknown */");
                    }
                }
            }
            
            // get next
            sprite_idx = *(u16 *)(sprite + sprite_to_next);
        }
        while (sprite_idx != 0);
    }
    printf("\n");
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
    logic = host.pixelbuf0.data; // (astruct *)(workbuff + 0xf0);
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
void OPCODE_CDEFSC_0x46(u8 *ptr, u16 offset)
{
    *(u8 *)(BASEMNMEM_PTR + offset) = *(u8 *)(ptr + offset);
    *(u8 *)(BASEMNMEM_PTR + sprite_numelem + offset) = *(u8 *)(ptr + sprite_numelem + offset);

    for (s32 i = 0; i < 32; i++)
    {
        *(u8 *)(BASEMNMEM_PTR + sprite_link + offset + i) = *(u8 *)(ptr + offset + sprite_link + i);
    }

    if (libsprit == 0)
    {
        return;
    }
    
    u16 nextsprite = *(u16 *)(SPRITEMEM_PTR + sprite_to_next + libsprit);
    *(s16 *)(BASEMNMEM_PTR + sprite_screen_id + offset) = libsprit;
    libsprit = nextsprite;
    *(u16 *)(SPRITEMEM_PTR + sprite_link + libsprit) = 0;
    *(u16 *)(BASEMNMEM_PTR + sprite_to_next + offset) = 0;
    *(u8 *) (SPRITEMEM_PTR + sprite_numelem + libsprit) = *(u8 *) (BASEMNMEM_PTR + sprite_numelem + offset);
    *(u16 *)(SPRITEMEM_PTR + sprite_newx + libsprit) = *(u16 *)(BASEMNMEM_PTR + sprite_newy + offset) & 0xfff0;
    *(u16 *)(SPRITEMEM_PTR + sprite_newy + libsprit) = *(u16 *)(BASEMNMEM_PTR + sprite_newd + offset);
    *(u16 *)(SPRITEMEM_PTR + sprite_newd + libsprit) = 0x7fff;
    *(u16 *)(SPRITEMEM_PTR + sprite_depx + libsprit) = *(s16 *)(BASEMNMEM_PTR + sprite_newy + offset) + *(s16 *)(BASEMNMEM_PTR + 0x12 + offset) | 0xf;
    *(s16 *)(SPRITEMEM_PTR + sprite_depy + libsprit) = *(s16 *)(BASEMNMEM_PTR + sprite_newd + offset) + *(s16 *)(BASEMNMEM_PTR + 0x14 + offset);
    *(u16 *)(BASEMNMEM_PTR + sprite_height + offset) = 0;
    *(u16 *)(BASEMNMEM_PTR + sprite_newzoomx + offset) = 0;
    *(u16 *)(BASEMNMEM_PTR + sprite_newzoomy + offset) = 0;
    scadd(offset);
    vectoriel(offset);
}

void scadd(s16 screen)
{
    *(s16 *)(BASEMNMEM_PTR + sprite_to_next + screen) = 0;
    
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
        nextscreen = *(s16 *)(BASEMNMEM_PTR + sprite_to_next + curscreen);
    }
    while (nextscreen != 0);
    
    *(s16 *)(BASEMNMEM_PTR + sprite_to_next + curscreen) = screen;
    return;
}

void scbreak(s16 screen)
{
    if (ptscreen == 0)
        return;
    
    s16 prevptscreen = ptscreen;
    if (screen == ptscreen)
    {
        ptscreen = *(s16 *)(BASEMNMEM_PTR + sprite_to_next + screen);
        return;
    }
    
    s16 nextptscreen;

    do
    {
        nextptscreen = *(s16 *)(BASEMNMEM_PTR + sprite_to_next + prevptscreen);
        if (screen == nextptscreen)
        {
            *(s16 *)(BASEMNMEM_PTR + sprite_to_next + prevptscreen) = *(s16 *)(BASEMNMEM_PTR + sprite_to_next + screen);
            return;
        }
        
        prevptscreen = nextptscreen;
    }
    while (nextptscreen != 0);
}

void scdosprite(s16 screen)
{
    u8 *scraddr = BASEMNMEM_PTR + screen;
    s16 sprit = *(s16 *)(scraddr + 0x02);

    u8 *spraddr = alis.mem + alis.script->vram_org + basesprite + sprit;
    *(s8 *) (spraddr + sprite_numelem) = *(s8 *) (scraddr + sprite_numelem);
    *(u16 *)(spraddr + sprite_newx) = *(u16 *)(scraddr + sprite_newy ) & 0xfff0;
    *(s16 *)(spraddr + sprite_newy) = *(s16 *)(scraddr + sprite_newd);
    *(s16 *)(spraddr + sprite_newd) = 0x7fff;
    *(u16 *)(spraddr + sprite_depx) = *(s16 *)(scraddr + sprite_newy ) + *(s16 *)(scraddr + sprite_data) | 0x0f;
    *(s16 *)(spraddr + sprite_depy) = *(s16 *)(scraddr + sprite_newd) + *(s16 *)(scraddr + 0x14);
}

void vectoriel(s32 at)
{
    u8 *addr = BASEMNMEM_PTR + at;
    addr[0x26] = addr[0x21] * addr[0x25] - addr[0x24] * addr[0x22];
    addr[0x27] = addr[0x22] * addr[0x23] - addr[0x25] * addr[0x20];
    addr[0x28] = addr[0x20] * addr[0x24] - addr[0x23] * addr[0x21];
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
