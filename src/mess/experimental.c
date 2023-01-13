//
//  experimental.c
//  alis
//
//  Created by Vadim Kindl on 12.01.2023.
//  Copyright © 2023 Zlot. All rights reserved.
//

#include "experimental.h"

#include "alis.h"
#include "alis_private.h"
#include "utils.h"

// NOTE: Mostly cut and paste code from ghidra I'm trying to make sense at the moment


int varA6 = 0;

u8 flagmain = 0;

u16 atpalet[256];
u16 dkpalet[256];

s32 bufrvb;


//int adresdei(void)
//{
//    s32 *piVar2 = (int *)(*(int *)(*(int *)(varA6 - 0x14) + 0xe) + *(int *)(varA6 - 0x14));
//    if ((s16)(alis.varD7 - *(s16 *)(piVar2 + 1)) < 0)
//    {
//        piVar2 = (int *)((int)piVar2 + (int)(s16)(alis.varD7 * 4) + *piVar2);
//        return *piVar2 + (int)piVar2;
//    }
//
////    perror(in_stack_00000000);
//    return 0xf;
//}

int adresdes(int idx, char *cvar)
{
    int a, e;

    int add = alis.mem[alis.script->data_org] ? 6 : 0;
        
    uint32_t l = read32(alis.mem + alis.script->data_org + 0xe + add, alis.platform) + add;
    e = read16(alis.mem + alis.script->data_org + l + 4, alis.platform);
    if (e > idx)
    {
        a = l + idx * 4 + read32(alis.mem + alis.script->data_org + l, alis.platform);
        return a + read32(alis.mem + alis.script->data_org + a, alis.platform);
    }

    perror(cvar);
    return 0xf;

//    if (flagmain != '\0')
//        return adresdei();
//
//    s32 *piVar2 = (int *)(*(int *)(*(int *)(varA6 - 0x14) + 0xe) + *(int *)(varA6 - 0x14));
//    if ((s16)(alis.varD7 - *(s16 *)(piVar2 + 1)) < 0)
//    {
//        piVar2 = (int *)((int)piVar2 + (int)(s16)(alis.varD7 * 4) + *piVar2);
//        return *piVar2 + (int)piVar2;
//    }
//
//    perror(in_stack_00000000);
//    return 0xf;
}


int adresmus(int idx, char *cvar)
{
    int a, e;

    int add = alis.mem[alis.script->data_org] ? 6 : 0;
        
    uint32_t l = read32(alis.mem + alis.script->data_org + 0xe + add, alis.platform) + add;
    e = read16(alis.mem + alis.script->data_org + l, alis.platform);
    if (e > idx)
    {
        a = read32(alis.mem + alis.script->data_org + 0xc + l, alis.platform) + l;
        return a + read32(alis.mem + alis.script->data_org + a, alis.platform);
    }

    perror(cvar);
    return 0x11;

//    if (flagmain != '\0')
//        return adresmui(cvar);
//
//    int iVar2 = *(int *)(*(int *)(varA6 + -0x14) + 0xe) + *(int *)(varA6 + -0x14);
//    if ((short)(unaff_D7w - *(short *)(iVar2 + 0x10)) < 0)
//    {
//      int *piVar1 = (int *)(*(int *)(iVar2 + 0xc) + iVar2 + (int)(short)(alis.varD7 * 4));
//      return *piVar1 + (int)piVar1;
//    }
//
//    perror(param_1);
//    return 0x11;
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
//  puVar5 = dkpalet + (uint)((u16)((s16)in_A1 - (s16)atpalet) >> 2) * 3;
//  uVar1 = in_D0;
//  do {
//    uVar3 = *in_A0;
//    in_A0 = (u32 *)((int)in_A0 + 3);
//    bufrvb = uVar3 & 0xffffff00;
//    if ((bufrvb == 0) && (((0x100 < *puVar5 || (0x100 < puVar5[1])) || (0x100 < puVar5[2])))) {
//      bufrvb = 0x1010001;
//    }
//    uVar3 = (bufrvb >> 0x18) * (uint)*puVar5;
//    if (0xffff < uVar3) {
//      uVar3 = 0xffff;
//    }
//    *in_A1 = (s8)(uVar3 >> 9) & 0x7e;
//    puVar6 = puVar5 + 2;
//    uVar3 = (bufrvb >> 0x10 & 0xff) * (uint)puVar5[1];
//    if (0xffff < uVar3) {
//      uVar3 = 0xffff;
//    }
//    in_A1[1] = (s8)(uVar3 >> 9) & 0x7e;
//    pbVar4 = in_A1 + 3;
//    in_A1[2] = 0;
//    puVar5 = puVar5 + 3;
//    uVar3 = (bufrvb >> 8 & 0xff) * (uint)*puVar6;
//    if (0xffff < uVar3) {
//      uVar3 = 0xffff;
//    }
//    in_A1 = in_A1 + 4;
//    *pbVar4 = (s8)(uVar3 >> 9) & 0x7e;
//    uVar2 = (s16)uVar1 - 1;
//    uVar1 = (uint)uVar2;
//  } while (uVar2 != 0xffff);
//  return in_D0;
//}

u32 pald = 0;
u32 ftopal = 0;
u32 *tpalet;
u32 *mpalet;

u16 topalet(void)
{
    u32 uVar1;
    u32 *puVar2;
    u32 *puVar3;
    u16 in_D0;
    s8 bVar4;
    u32 uVar5;
    s8 cVar8;
    u32 uVar6;
    u32 uVar7;
    s16 sVar9;
    u32 *puVar10;
    u32 *puVar11;

    sVar9 = 0xff;
    ftopal = 0;
    puVar2 = (u32 *)tpalet;
    puVar3 = (u32 *)mpalet;
    
topale1:
  
    do
    {
        puVar11 = puVar3;
        puVar10 = puVar2;
        puVar2 = puVar10 + 1;
        puVar3 = puVar11 + 1;
        if (*puVar11 == *puVar10)
        {
            sVar9 = sVar9 - 1;
            if (sVar9 != -1)
            {
                goto topale1;
            }
        }
      
        if (*puVar11 == *puVar10)
        {
            return in_D0;
        }
    
        uVar7 = *puVar11;
        uVar1 = *puVar10;
        bVar4 = (s8)uVar1;
        cVar8 = (char)uVar7;
        if (cVar8 < (char)bVar4)
        {
            uVar6 = uVar7 & 0xffffff00 | (u32)(s8)(cVar8 + pald);
            if (bVar4 < (s8)(cVar8 + pald))
            {
                uVar6 = uVar7 & 0xffffff00 | uVar1 & 0xff;
            }
        }
        else
        {
            uVar6 = uVar7 & 0xffffff00 | (uint)(s8)(cVar8 - pald);
            if ((char)(cVar8 - pald) <= (char)bVar4)
            {
                uVar6 = uVar7 & 0xffffff00 | uVar1 & 0xff;
            }
        }
      
        uVar5 = (uint3)((uVar6 << 0x10) >> 8) | (uint3)(s8)(uVar6 >> 0x18);
        bVar4 = (s8)(uVar1 >> 0x10);
        cVar8 = (char)(uVar6 >> 0x10);
        if (cVar8 < (char)bVar4)
        {
            uVar7 = CONCAT31(uVar5,cVar8 + pald);
            if (bVar4 < (s8)(cVar8 + pald))
            {
                uVar7 = CONCAT31(uVar5,bVar4);
            }
        }
        else
        {
            uVar7 = CONCAT31(uVar5,cVar8 - pald);
            if ((char)(cVar8 - pald) <= (char)bVar4)
            {
                uVar7 = CONCAT31(uVar5,bVar4);
            }
        }
      
        bVar4 = (s8)(uVar1 >> 0x18);
        uVar1 = (uint)(u16)((s16)uVar7 << 8);
        cVar8 = (char)(uVar7 >> 8);
        if (cVar8 < (char)bVar4)
        {
            uVar1 = uVar7 & 0xffff0000 | uVar1;
            uVar7 = uVar1 | (s8)(cVar8 + pald);
            if (bVar4 < (s8)(cVar8 + pald))
            {
                uVar7 = uVar1 | bVar4;
            }
        }
        else
        {
            uVar1 = uVar7 & 0xffff0000 | uVar1;
            uVar7 = uVar1 | (s8)(cVar8 - pald);
            if ((char)(cVar8 - pald) <= (char)bVar4)
            {
                uVar7 = uVar1 | bVar4;
            }
        }
      
        *puVar11 = (uint)(u16)((u16)uVar7 << 8 | (u16)uVar7 >> 8) << 0x10 | uVar7 >> 0x10;
        ftopal = 1;
        sVar9 = sVar9 - 1;
        if (sVar9 == -1)
        {
            ftopal = 1;
            return in_D0;
        }
    }
    while (1);
}

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
//  undefined4 *puVar8;
//  s8 *pcVar9;
//  s8 *pbVar10;
//  undefined4 *puVar11;
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
//      *(s8 *)((int)puVar8 + 1) = bVar1 & 0xf0;
//      pcVar9 = (char *)((int)puVar8 + 3);
//      *(undefined *)((int)puVar8 + 2) = 0;
//      puVar8 = puVar8 + 1;
//      *pcVar9 = bVar1 << 4;
//      sVar3 = sVar3 - 1;
//    } while (sVar3 != -1);
//  }
//  else {
//    palc = 0;
//    pbVar6 = (s8 *)(in_A0 + 4);
//    pbVar5 = (s8 *)((int)(s16)((u16)*pbVar5 * 4) + (int)atpalet);
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
//      palt = (undefined)((uint)(int)unaff_D6w / (uVar2 & 0xffff));
//      palt0 = palt;
//      palc = (char)uVar2;
//      return;
//    }
//    pald = (char)(0x7e / ((int)unaff_D6w & 0xffffU)) + '\x01';
//    topalet();
//    palt = 1;
//    palt0 = 1;
//    palc = (char)unaff_D6w;
//    return;
//  }
//  sVar3 = 0xff;
//  puVar8 = atpalet;
//  puVar11 = (undefined4 *)mpalet;
//  do {
//    *puVar11 = *puVar8;
//    sVar3 = sVar3 - 1;
//    puVar8 = puVar8 + 1;
//    puVar11 = puVar11 + 1;
//  } while (sVar3 != -1);
//  ftopal = 0xff;
//  return;
//}

void delprec(void)
{
//  s16 sVar1;
//  s16 in_D1w;
//  s32 in_A1;
//
//  sVar1 = *(s16 *)(in_A1 + 2 + (int)in_D1w);
//  if (in_D1w == *(s16 *)(basemain + 2 + (int)sVar1)) {
//    *(undefined2 *)(basemain + 2 + (int)sVar1) = *(undefined2 *)(in_A1 + 6 + (int)in_D1w);
//    return;
//  }
//
//  for (sVar1 = *(s16 *)(basemain + 2 + (int)sVar1); in_D1w != *(s16 *)(in_A1 + 6 + (int)sVar1);
//      sVar1 = *(s16 *)(in_A1 + 6 + (int)sVar1)) {
//  }
//  *(undefined2 *)(in_A1 + 6 + (int)sVar1) = *(undefined2 *)(in_A1 + 6 + (int)in_D1w);
  return;
}

void createlem(char *param_1)
{
//  undefined2 uVar1;
//  s16 sVar2;
//  s16 sVar3;
//  undefined2 in_D1w;
//  s16 unaff_D2w;
//  s32 in_A1;
//  s32 varA6;
//
//  sVar3 = libsprit;
//  if (libsprit != 0) {
//    uVar1 = *(undefined2 *)(in_A1 + 4 + (int)libsprit);
//    if (unaff_D2w == 0) {
//      *(s16 *)(varA6 - 0x18) = libsprit;
//      libsprit = uVar1;
//    }
//    else {
//      *(s16 *)(in_A1 + 4 + (int)unaff_D2w) = libsprit;
//      libsprit = uVar1;
//    }
//    *(undefined2 *)(in_A1 + 4 + (int)sVar3) = in_D1w;
//    *(undefined2 *)(in_A1 + 2 + (int)sVar3) = *(undefined2 *)(varA6 - 0x16);
//    *(undefined *)(in_A1 + 1 + (int)sVar3) = numelem;
//    *(undefined *)(in_A1 + sVar3) = 0xff;
//    sVar2 = *(s16 *)(basemain + 2 + (int)*(s16 *)(varA6 - 0x16));
//    *(undefined2 *)(in_A1 + 6 + (int)sVar3) = *(undefined2 *)(in_A1 + 6 + (int)sVar2);
//    *(s16 *)(in_A1 + 6 + (int)sVar2) = sVar3;
//    return;
//  }
//  perror(param_1);
  return;
}

void searchelem(int numelem)
{
  s16 sVar1;
  char cVar2;
  s16 sVar3;
  int in_A1;
  int varA6;
  
  sVar3 = *(s16 *)(varA6 - 0x18);
  if (sVar3 != 0) {
    do {
      sVar1 = *(s16 *)(in_A1 + 2 + (int)sVar3);
      if (sVar1 <= *(s16 *)(varA6 - 0x16)) {
        if (*(s16 *)(varA6 - 0x16) != sVar1) {
          return;
        }
        cVar2 = *(char *)(in_A1 + 1 + (int)sVar3);
        if (cVar2 <= numelem) {
          if (numelem != cVar2) {
            return;
          }
          return;
        }
      }
      sVar3 = *(s16 *)(in_A1 + 4 + (int)sVar3);
    } while (sVar3 != 0);
  }
  return;
}

char killelem(void)
{
//  char cVar1;
//  s16 in_D1w;
//  s16 unaff_D2w;
//  int in_A1;
//  int varA6;
//
//  if ((ferase == '\0') && (cVar1 = *(char *)(in_A1 + in_D1w), -1 < cVar1)) {
//    *(undefined *)(in_A1 + in_D1w) = 1;
//    if (unaff_D2w == 0) {
//      *(undefined2 *)(varA6 - 0x18) = *(undefined2 *)(in_A1 + 4 + (int)in_D1w);
//    }
//    else {
//      *(undefined2 *)(in_A1 + 4 + (int)unaff_D2w) = *(undefined2 *)(in_A1 + 4 + (int)in_D1w);
//    }
//  }
//  else {
//    if (unaff_D2w == 0) {
//      *(undefined2 *)(varA6 - 0x18) = *(undefined2 *)(in_A1 + 4 + (int)in_D1w);
//    }
//    else {
//      *(undefined2 *)(in_A1 + 4 + (int)unaff_D2w) = *(undefined2 *)(in_A1 + 4 + (int)in_D1w);
//    }
//    *(s16 *)(in_A1 + 4 + (int)in_D1w) = libsprit;
//    libsprit = in_D1w;
//    cVar1 = delprec();
//  }
//  if (unaff_D2w == 0) {
//    return cVar1;
//  }
//  return cVar1;
    return 0;
}

void nextnum(int numelem)
{
  s16 in_D1w;
  int in_A1;
  int varA6;
  
    s16 sVar1 = *(s16 *)(in_A1 + 4 + (int)in_D1w);
  if (((sVar1 != 0) && (*(s16 *)(in_A1 + 2 + (int)sVar1) == *(s16 *)(varA6 - 0x16))) && (*(char *)(in_A1 + 1 + (int)sVar1) == numelem))
  {
      return;
  }
    
  return;
}

int flaginvx = 0;
int fadddes = 0;

int oldcx;
int oldcy;
int oldcz;

int oldacx;
int oldacy;
int oldacz;

void savecoord(u8 *at)
{
  oldcx = at[0];
  oldcy = at[4];
  oldcz = at[8];
  oldacx = at[0xc];
  oldacy = at[0x10];
  oldacz = at[0x14];
}

void put(int depx, int depy, int depz, int numelem)
{
    char *cvar = "test";

    s16 sVar1;
    s16 sVar2;
    s16 in_D1w;
    s16 unaff_D2;
    u16 uVar3;
    s16 unaff_D5w;
    s16 cury;
    s16 curz;
    s8 *in_A0;
    s16 *psVar6;
    s16 *psVar7;
    s32 sprite;
    s32 varA6;
    u8 bVar9;
    u32 uStack_4;
  
    flaginvx = flaginvx & 0xff00;
    // uStack_4._2_2_ = 0x7be8;
    *(((u16 *)(&uStack_4)) + 1) = 0x7be8;
    // uStack_4._0_2_ = 1;
    *(((u16 *)(&uStack_4)) + 0) = 1;
    int addr = adresdes(numelem, cvar);
    curz = depz;
    cury = depy;
    //sprite = basesprite;
    if (*in_A0 < '\0')
    {
        if (*in_A0 == -2)
        {
            // ctopale1();
            return;
        }

        uVar3 = (u16)(s8)in_A0[1];
        psVar6 = (s16 *)(in_A0 + 2);
        while (uVar3 = uVar3 - 1, depy = cury, depz = curz, uVar3 != 0xffff)
        {
            // uStack_4._2_2_ = flaginvx;
            *(((u16 *)(&uStack_4)) + 1) = flaginvx;
            // uStack_4._0_2_ = depx;
            *(((u16 *)(&uStack_4)) + 0) = depx;
            sVar1 = psVar6[0];
            sVar2 = psVar6[1];

            // if (flaginvx._0_1_ != 0)
            if (*((u8 *)(&flaginvx)) != 0)
            {
                sVar2 = -sVar2;
            }
            
            depx = sVar2 + depx;
            psVar7 = psVar6 + 3;
            depy = psVar6[2] + cury;
            psVar6 = psVar6 + 4;
            depz = *psVar7 + curz;
            if (sVar1 < 0)
            {
                *((u8 *)(&flaginvx)) ^= 1;
            }
            //flaginvx = CONCAT11(*((u8 *)(&flaginvx)),1);
            //putin();
            // depx = uStack_4._0_2_;
            depx =  *(((u16 *)(&uStack_4)) + 0);
            // flaginvx = uStack_4._2_2_;
            flaginvx =  *(((u16 *)(&uStack_4)) + 1);
        }
        goto putfin;
    }
    
    bVar9 = fadddes == '\0';
    if (bVar9)
    {
        bVar9 = (char)flaginvx == '\0';
        if (!bVar9)
            goto put13;
        
        uStack_4 = 0x17cc8;
        searchelem(numelem);
        if (!bVar9)
        {
            uStack_4 = 0x17cd0;
            createlem(cvar);
        }

        if (-1 < *(char *)(sprite + in_D1w))
        {
put30:
            // undefined
            *(u8 *)(sprite + in_D1w) = 2;
        }
    }
    else
    {
put13:
        uStack_4 = 0x17ca4;
        searchelem(numelem);
        if (bVar9)
        {
            do
            {
                bVar9 = *(char *)(sprite + in_D1w) == '\0';
                if (bVar9)
                {
                    goto put30;
                }
                
                uStack_4 = 0x17cbc;
                nextnum(numelem);
            }
            while (bVar9);
        }
        
        uStack_4 = 0x17cac;
        createlem(cvar);
    }
    
    uStack_4 = 0x17ce2;
    adresdes(numelem, cvar);
    *(u32 *)(sprite + 0x12 + (int)in_D1w) = unaff_D2;
    *(s8 *)(sprite + 0x12 + (int)in_D1w) = *((s8 *)(&flaginvx));
    *(u16 *)(sprite + 0x24 + (int)in_D1w) = *(u16 *)(varA6 + 0x28);
    *(u16 *)(sprite + 0x22 + (int)in_D1w) = *(u16 *)(varA6 - 0xe);
    *(u16 *)(sprite + 0x1e + (int)in_D1w) = *(u16 *)(varA6 - 0x2a);
    *(u8 *)(sprite + 0x20 + (int)in_D1w) = *(u8 *)(varA6 - 0x2b);
    *(u8 *)(sprite + 0x21 + (int)in_D1w) = *(u8 *)(varA6 - 0x2f);
    *(u8 *)(sprite + 0x1d + (int)in_D1w) = *(u8 *)(varA6 - 0x27);
    *(u8 *)(sprite + 0x1c + (int)in_D1w) = *(u8 *)(varA6 - 0x25);
    if (*(s8 *)(varA6 - 0x25) == -0x80)
    {
        *(u16 *)(sprite + 0x2c + (int)in_D1w) = *(u16 *)(varA6 - 0x36);
        *(u16 *)(sprite + 0x2e + (int)in_D1w) = *(u16 *)(varA6 - 0x38);
        *(u8 *)(sprite + 0x1d + (int)in_D1w) = 0;
    }
    else
    {
        *(u8 *)(sprite + 0x1d + (int)in_D1w) = *(u8 *)(varA6 - 0x27);
        *(u8 *)(sprite + 0x1c + (int)in_D1w) = *(u8 *)(varA6 - 0x26);
    }

    // register0x0000003c = (BADSPACEBASE *)&uStack_4;
    cury = oldcy + depy;
    curz = oldcz + depz;
    psVar6 = (s16 *)(sprite + 0x16 + (int)in_D1w);
    psVar6[0] = oldcx + depx;
    psVar6[1] = cury;
    psVar6[2] = curz;
    *(((u16 *)(&uStack_4)) + 0) = unaff_D5w;
putfin:
    bVar9 = (char)flaginvx == '\0';
//  if (bVar9) {
//    //*(u32 *)((int)register0x0000003c - 4) = 0x17d78;
//    searchelem();
//    if (!bVar9) {
//      return;
//    }
//    do {
//      while (bVar9 = *(char *)(sprite + in_D1w) == '\0', bVar9) {
//        *(undefined4 *)((int)register0x0000003c - 4) = 0x17d92;
//        killelem();
//        bVar9 = in_D1w == 0;
//        if (bVar9) {
//          fadddes = 0;
//          return;
//        }
//        *(undefined4 *)((int)register0x0000003c - 4) = 0x17d9c;
//        testnum();
//        if (!bVar9) {
//          fadddes = 0;
//          return;
//        }
//      }
//      *(undefined4 *)((int)register0x0000003c - 4) = 0x17d82;
//      nextnum();
//    } while (bVar9);
//  }
    fadddes = 0;
    return;
}
