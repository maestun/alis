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

#pragma once

#include "alis.h"
#include "config.h"

// to be consistent with old code

PACK_PUSH typedef struct PACK_ATTR {

    s8 state;               // 0x00
    s8 numelem;             // 0x01
    s16 screen_id;          // 0x02
    u16 to_next;            // 0x04
    u16 link;               // 0x06
    u32 newad : 24;         // 0x08
    s8 newf : 8;            // 0x08 + 3
    s16 newx;               // 0x0c
    s16 newy;               // 0x0e
    s16 newd;               // 0x10
    u32 data : 24;          // 0x12
    u8 flaginvx : 8;        // 0x12 + 3 // +0 on big endian machines
    s16 depx;               // 0x16
    s16 depy;               // 0x18
    s16 depz;               // 0x1a
    u8 credon_off;          // 0x1c
    u8 creducing;           // 0x1d
    s16 clinking;           // 0x1e
    u8 cordspr;             // 0x20
    u8 chsprite;            // 0x21
    u16 script_ent;         // 0x22
    u32 sprite_0x28;        // 0x24
    s16 width;              // 0x28
    s16 height;             // 0x2a
    u16 newzoomx;           // 0x2c
    u16 newzoomy;           // 0x2e
} sSprite; PACK_POP

typedef struct {
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
} sRect;

typedef struct {
    
    u8 *spritemem;

    u8 *physic;
    u8 *logic;

    s16 logx1;
    s16 logx2;
    s16 logy1;
    s16 logy2;

    s8 numelem;
    u8 invert_x;

    s16 depx;
    s16 depy;
    s16 depz;
    
    s16 oldcx;
    s16 oldcy;
    s16 oldcz;

    s16 oldacx;
    s16 oldacy;
    s16 oldacz;
    
    s16 dkpalet[768];
    u8 fdarkpal;

    s32 backdes;
    s32 backaddr;
    s16 backprof;
    s16 backx1;
    s16 backx2;
    s16 backy1;
    s16 backy2;
    s16 backlarg;
    u8 *backmap;

    s32 basesprite;
    u16 libsprit;
    s32 debsprit;
    s32 finsprit;
    s32 backsprite;
    s32 tvsprite;
    s32 texsprite;
    s32 atexsprite;
    
    s8 spag;
    s8 wpag;
    s16 pagdx;
    s16 pagdy;
    s16 pagcount;
    
    u8 sback;
    u8 wback;
    s8 cback;
    u8 pback;
    
    u16 inkcolor;
    u16 line_a_mode;
    
    u8 tvmode;

    u8 svpalet[768];
    u8 svpalet2[768];
    u8 tpalet[768 * 4];
    u8 mpalet[768 * 4];
    float dpalet[768];

    u8 *atpalet;
    u8 *ampalet;

    u8 flinepal;
    s16 firstpal[64];
    s16 tlinepal[64];

    u8 palc;
    u8 palt;
    u8 palt0;
    u8 ftopal;
    u8 thepalet;
    u8 defpalet;

    s16 tabfen[640];
    s16 *ptabfen;

    s32 bufrvb;

    s32 mousflag;

    u8 *bufpack;

    u8 timing;
    s8 fmouse;
    s16 fonum;

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
    s16 clipx1;
    s16 clipy1;
    s16 clipx2;
    s16 clipy2;
    s16 clipl;
    s16 cliph;
    u8 fclip;

    u8 fitroutine;
    u8 fphysic;
    u8 fphytolog;
    u8 fremouse;
    u8 fremap;
    u8 fremouse2;
    u8 vtiming;
    s16 savmouse[2];
    u32 savmouse2[8];
    u8 switchgo;
    u8 *wlogic;
    s16 wlogx1;
    s16 wlogx2;
    s16 wlogy1;
    s16 wlogy2;
    s16 wloglarg;
    s16 loglarg; // 0x50 for st
    u8 insid;

} sImage;

extern sImage image;

void inisprit(void);

void createlem(u16 *curidx, u16 *previdx);
u8 searchelem(u16 *curidx, u16 *previdx);
s8 searchtete(u16 *curidx, u16 *previdx);
void killelem(u16 *curidx, u16 *previdx);
u8 testnum(u16 *curidx);
u8 nextnum(u16 *curidx, u16 *previdx);

void put(u16 idx);
void put_char(s8 character);
void put_string(void);

void putin(u16 idx);

u32 itroutine(u32 interval, void *param);
void draw(void);

void topalette(u8 *paldata, s32 duration);
void toblackpal(s16 duration);

void savepal(s16 mode);
void restorepal(s16 mode, s32 duration);

void selpalet(void);
void setmpalet(void);

void setlinepalet(void);

s16 debprotf(s16 d2w);
u16 rangesprite(u16 elemidx1, u16 elemidx2, u16 elemidx3);

void valtostr(char *string, s16 value);

void log_sprites(void);

void mac_update_pos(short *x,short *y);

extern u8 cga_palette[12];
extern u8 masks[4];
extern u8 rots[4];
