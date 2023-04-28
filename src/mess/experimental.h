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

extern u8 thepalet;
extern u8 defpalet;
extern u8 *ampalet;

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

u16 moteur1(s16 offset);
void moteur2(s16 offset);

//void killent(u16 d0w, u32 d3, u16 d5w);
s16 debprotf(u16 d2w);
void shrinkprog(sAlisScript *script, s8 unload_script);

void alis_putchar(s8 character);
void alis_putstring(void);
void sparam(void);

s16 tabint(s16 offset);

#endif /* experimental_h */
