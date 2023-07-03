//
//  image.h
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

// to be consistent with old code

typedef union {
    
    struct __attribute__((packed)) {
        
        s8 state;               // 0x00
        s8 numelem;             // 0x01
        s16 screen_id;          // 0x02
        u16 to_next;            // 0x04
        u16 link;               // 0x06
        u32 newad:24;           // 0x08
        s8 newf:8;              // 0x08 + 3
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
        u16 script_ent;         // 0x22
        u32 sprite_0x28;        // 0x24
        s16 width;              // 0x28
        s16 height;             // 0x2a
        u16 newzoomx;           // 0x2c
        u16 newzoomy;           // 0x2e
    };
    
    u8 variables[0x30];
    
} sSprite;

typedef struct {
    
    u8 *spritemem;

    s8 numelem;
    u8 invert_x;

    s16 depx;
    s16 depy;
    s16 depz;
    
    s16 oldcx;
    s16 oldcy;
    s16 oldcz;

    s32 basesprite;
    u16 libsprit;
    s32 debsprit;
    s32 finsprit;
    s32 backsprite;
    s32 tvsprite;
    s32 texsprite;
    s32 atexsprite;
} sImage;

extern sImage image;

extern u8 *sprit_mem;

void inisprit(void);

void createlem(s16 *curidx, s16 *previdx);
u8 searchelem(s16 *curidx, s16 *previdx);
void killelem(s16 *curidx, s16 *previdx);

void put(u16 idx);
void put_char(s8 character);
void put_string(void);

void putin(u16 idx);

void itroutine(void);
void draw(void);

void topalette(u8 *paldata, s32 duration);
void toblackpal(s16 duration);

void savepal(s16 mode);
void restorepal(s16 mode, s32 duration);

s16 debprotf(s16 d2w);

void valtostr(u8 *string, s16 value);

void log_sprites(void);

#endif /* experimental_h */
