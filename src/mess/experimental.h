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

// to be consistent with old code

typedef union {
    
    struct __attribute__((packed)) {
        
        u8 state;               // 0x00
        s8 numelem;             // 0x01
        s16 screen_id;          // 0x02
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
        u16 script_ent;         // 0x22
        u32 sprite_0x28;        // 0x24
        s16 width;              // 0x28
        s16 height;             // 0x2a
        u16 newzoomx;           // 0x2c
        u16 newzoomy;           // 0x2e
    };
    
    u8 variables[0x30];
    
} SpriteVariables;

//typedef union {
//    
//    struct __attribute__((packed)) {
//        
//        u8 state;               // 0x0
//        u8 numelem;             // 0x1
//        u16 screen_id;          // 0x2
//        u16 to_next;            // 0x4
//        u16 link;               // 0x6
//        u16 unknown0x08;        // 0x8
//        u16 unknown0x0a;        // 0xa
//        u16 unknown0x0c;        // 0xc
//        s16 newx;               // 0xe
//        s16 newy;               // 0x10
//        u16 width;              // 0x12
//        u16 height;             // 0x14
//        u16 depx;               // 0x16
//        u16 depy;               // 0x18
//        u16 depz;               // 0x1a
//        u8 credon_off;          // 0x1c
//        u8 creducing;           // 0x1d
//        u16 clinking;           // 0x1e
//
//        s8 unknown0x20;
//        s8 unknown0x21;
//        s8 unknown0x22;
//        s8 unknown0x23;
//        s8 unknown0x24;
//        s8 unknown0x25;
//        
//        s8 unknown0x26;
//        s8 unknown0x27;
//        s8 unknown0x28;
//
//        u8 unknown0x29;
//
//        u16 unknown0x2a;
//        u16 unknown0x2c;
//        u16 unknown0x2e;
//    };
//    
//    u8 variables[0x30];
//    
//} SceneVariables;

u8  get_scene_state(u32 scene);
s8  get_scene_numelem(u32 scene);
u16 get_scene_screen_id(u32 scene);
u16 get_scene_to_next(u32 scene);
u16 get_scene_link(u32 scene);
u16 get_scene_unknown0x08(u32 scene);
u16 get_scene_unknown0x0a(u32 scene);
u16 get_scene_unknown0x0c(u32 scene);
s16 get_scene_newx(u32 scene);
s16 get_scene_newy(u32 scene);
u16 get_scene_width(u32 scene);
u16 get_scene_height(u32 scene);
u16 get_scene_depx(u32 scene);
u16 get_scene_depy(u32 scene);
u16 get_scene_depz(u32 scene);
u8  get_scene_credon_off(u32 scene);
u8  get_scene_creducing(u32 scene);
u16 get_scene_clinking(u32 scene);
s8  get_scene_unknown0x20(u32 scene);
s8  get_scene_unknown0x21(u32 scene);
s8  get_scene_unknown0x22(u32 scene);
s8  get_scene_unknown0x23(u32 scene);
s8  get_scene_unknown0x24(u32 scene);
s8  get_scene_unknown0x25(u32 scene);
s8  get_scene_unknown0x26(u32 scene);
s8  get_scene_unknown0x27(u32 scene);
s8  get_scene_unknown0x28(u32 scene);
u8  get_scene_unknown0x29(u32 scene);
u16 get_scene_unknown0x2a(u32 scene);
u16 get_scene_unknown0x2c(u32 scene);
u16 get_scene_unknown0x2e(u32 scene);

void set_scene_state(u32 scene, u8 val);
void set_scene_numelem(u32 scene, s8 val);
void set_scene_screen_id(u32 scene, u16 val);
void set_scene_to_next(u32 scene, u16 val);
void set_scene_link(u32 scene, u16 val);
void set_scene_unknown0x08(u32 scene, u16 val);
void set_scene_unknown0x0a(u32 scene, u16 val);
void set_scene_unknown0x0c(u32 scene, u16 val);
void set_scene_newx(u32 scene, s16 val);
void set_scene_newy(u32 scene, s16 val);
void set_scene_width(u32 scene, u16 val);
void set_scene_height(u32 scene, u16 val);
void set_scene_depx(u32 scene, u16 val);
void set_scene_depy(u32 scene, u16 val);
void set_scene_depz(u32 scene, u16 val);
void set_scene_credon_off(u32 scene, u8 val);
void set_scene_creducing(u32 scene, u8 val);
void set_scene_clinking(u32 scene, u16 val);
void set_scene_unknown0x20(u32 scene, s8 val);
void set_scene_unknown0x21(u32 scene, s8 val);
void set_scene_unknown0x22(u32 scene, s8 val);
void set_scene_unknown0x23(u32 scene, s8 val);
void set_scene_unknown0x24(u32 scene, s8 val);
void set_scene_unknown0x25(u32 scene, s8 val);
void set_scene_unknown0x26(u32 scene, s8 val);
void set_scene_unknown0x27(u32 scene, s8 val);
void set_scene_unknown0x28(u32 scene, s8 val);
void set_scene_unknown0x29(u32 scene, u8 val);
void set_scene_unknown0x2a(u32 scene, u16 val);
void set_scene_unknown0x2c(u32 scene, u16 val);
void set_scene_unknown0x2e(u32 scene, u16 val);

extern u8 thepalet;
extern u8 defpalet;
extern u8 *ampalet;

extern u8 *sprit_mem;

extern u16 ptscreen;

int adresdes(s32 idx);
int adresmus(s32 idx);

void inisprit(void);

void createlem(u16 *curidx, u16 *previdx);
u8 searchelem(u16 *curidx, u16 *previdx);
void killelem(u16 *curidx, u16 *previdx);


void put(u8 idx);
void putin(u8 idx);

// void put(int depx, int depy, int depz, int numelem);

void scadd(s16 screen);
void scbreak(s16 screen);
void scdosprite(s16 screen);
void vectoriel(u16 screen);

void itroutine(void);
void image(void);
void ctopalette(u8 *paldata, s32 duration);
void ctoblackpal(s16 duration);

//void moteur(void);

s16 debprotf(u16 d2w);

void alis_putchar(s8 character);
void alis_putstring(void);

s16 tabint(s16 offset, u8 *address);
u16 tabchar(u16 offset, u8 *address);
s16 tabstring(s16 offset, u8 *address);

void vald0(u8 *string, s16 value);

void log_sprites(void);

#endif /* experimental_h */
