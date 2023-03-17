//
//  experimental.h
//  alis
//
//  Created by Vadim Kindl on 12.01.2023.
//  Copyright © 2023 Zlot. All rights reserved.
//

#ifndef experimental_h
#define experimental_h

#include "alis.h"
#include "alis_private.h"
#include "utils.h"

extern u8 *ampalet;

extern u8 numelem;

extern u8 flaginvx;
extern u8 fmuldes;
extern u8 fadddes;

extern s32 atent;
extern s32 debent;
extern s32 finent;
extern s32 maxent;
extern s16 nbent;
extern s16 dernent;

extern s32 basemain;
extern s32 basesprite;
extern u16 libsprit;
extern s32 debsprit;
extern s32 finsprit;
extern s32 backsprite;
extern s32 tvsprite;
extern s32 texsprite;
extern s32 atexsprite;
extern s32 mousprite;
extern s32 mousflag;

extern u16 depx;
extern u16 depy;
extern u16 depz;

extern u8 *sprit_mem;

int adresdes(s32 idx);
int adresmus(s32 idx);

void inisprit(void);

void createlem(u16 *curidx, u16 *previdx);
u8 searchelem(u16 *curidx, u16 *previdx);
void killelem(u16 *curidx, u16 *previdx);


void put(u8 idx);
void putin(u8 idx);

// void put(int depx, int depy, int depz, int numelem);

void FUN_STARTUP(s8 *param_1, size_t param_2, uint param_3);
s32 io_tomono(s32 param_1, s32 script_start);

void scadd(s16 screen);
void scbreak(s16 screen);
void scdosprite(s16 screen);
void vectoriel(u16 screen);

void OPCODE_CDEFSC_0x46(u8 *ptr, u16 offset);

void itroutine(void);
void image(void);
void ctopalette(u8 *paldata, s32 duration);
void ctoblackpal(s16 duration);

#endif /* experimental_h */
