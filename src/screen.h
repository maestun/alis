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
#include "alis_private.h"
#include "utils.h"

typedef struct {

    u16 ptscreen;
    
} sScreen;

extern sScreen screen;

// u8 state;               // 0x0
// u8 numelem;             // 0x1
// s16 screen_id;          // 0x2
// s16 to_next;            // 0x4
// s16 link;               // 0x6
// s16 unknown0x08;        // 0x8
// s16 unknown0x0a;        // 0xa
// s16 unknown0x0c;        // 0xc
// s16 newx;               // 0xe
// s16 newy;               // 0x10
// s16 width;              // 0x12
// s16 height;             // 0x14
// s16 depx;               // 0x16
// s16 depy;               // 0x18
// s16 depz;               // 0x1a
// u8 credon_off;          // 0x1c
// u8 creducing;           // 0x1d
// s16 clinking;           // 0x1e

static inline s8  get_scr_state(u32 scridx)                    { return xread8(alis.basemain + scridx + 0x0); }
static inline void set_scr_state(u32 scridx, s8 val)           { xwrite8(alis.basemain + scridx + 0x0, val); }

static inline s8  get_scr_numelem(u32 scridx)                  { return xread8(alis.basemain + scridx + 0x1); }
static inline void set_scr_numelem(u32 scridx, s8 val)         { xwrite8(alis.basemain + scridx + 0x1, val); }

static inline u16 get_scr_screen_id(u32 scridx)                { return xread16(alis.basemain + scridx + 0x2); }
static inline void set_scr_screen_id(u32 scridx, u16 val)      { xwrite16(alis.basemain + scridx + 0x2, val); }

static inline u16 get_scr_to_next(u32 scridx)                  { return xread16(alis.basemain + scridx + 0x4); }
static inline void set_scr_to_next(u32 scridx, u16 val)        { xwrite16(alis.basemain + scridx + 0x4, val); }

static inline u16 get_scr_link(u32 scridx)                     { return xread16(alis.basemain + scridx + 0x6); }
static inline void set_scr_link(u32 scridx, u16 val)           { xwrite16(alis.basemain + scridx + 0x6, val); }

static inline s16 get_scr_unknown0x08(u32 scridx)              { return xread16(alis.basemain + scridx + 0x8); }
static inline void set_scr_unknown0x08(u32 scridx, s16 val)    { xwrite16(alis.basemain + scridx + 0x8, val); }

static inline s16 get_scr_unknown0x0a(u32 scridx)              { return xread16(alis.basemain + scridx + 0xa); }
static inline void set_scr_unknown0x0a(u32 scridx, s16 val)    { xwrite16(alis.basemain + scridx + 0xa, val); }

static inline s16 get_scr_unknown0x0c(u32 scridx)              { return xread16(alis.basemain + scridx + 0xc); }
static inline void set_scr_unknown0x0c(u32 scridx, s16 val)    { xwrite16(alis.basemain + scridx + 0xc, val); }

static inline s16 get_scr_newx(u32 scridx)                     { return xread16(alis.basemain + scridx + 0xe); }
static inline void set_scr_newx(u32 scridx, s16 val)           { xwrite16(alis.basemain + scridx + 0xe, val); }

static inline s16 get_scr_newy(u32 scridx)                     { return xread16(alis.basemain + scridx + 0x10); }
static inline void set_scr_newy(u32 scridx, s16 val)           { xwrite16(alis.basemain + scridx + 0x10, val); }

static inline s16 get_scr_width(u32 scridx)                    { return xread16(alis.basemain + scridx + 0x12); }
static inline void set_scr_width(u32 scridx, s16 val)          { xwrite16(alis.basemain + scridx + 0x12, val); }

static inline s16 get_scr_height(u32 scridx)                   { return xread16(alis.basemain + scridx + 0x14); }
static inline void set_scr_height(u32 scridx, s16 val)         { xwrite16(alis.basemain + scridx + 0x14, val); }

static inline s16 get_scr_depx(u32 scridx)                     { return xread16(alis.basemain + scridx + 0x16); }
static inline void set_scr_depx(u32 scridx, s16 val)           { xwrite16(alis.basemain + scridx + 0x16, val); }

static inline s16 get_scr_depy(u32 scridx)                     { return xread16(alis.basemain + scridx + 0x18); }
static inline void set_scr_depy(u32 scridx, s16 val)           { xwrite16(alis.basemain + scridx + 0x18, val); }

static inline s16 get_scr_depz(u32 scridx)                     { return xread16(alis.basemain + scridx + 0x1a); }
static inline void set_scr_depz(u32 scridx, s16 val)           { xwrite16(alis.basemain + scridx + 0x1a, val); }

static inline u8  get_scr_credon_off(u32 scridx)               { return xread8(alis.basemain + scridx + 0x1c); }
static inline void set_scr_credon_off(u32 scridx, u8 val)      { xwrite8(alis.basemain + scridx + 0x1c, val); }

static inline u8  get_scr_creducing(u32 scridx)                { return xread8(alis.basemain + scridx + 0x1d); }
static inline void set_scr_creducing(u32 scridx, u8 val)       { xwrite8(alis.basemain + scridx + 0x1d, val); }

// cscreduce: highest reduction level and the default sprite shift (two bytes, both byte orders)
static inline void set_scr_redmax(u32 scridx, u8 val)          { xwrite8(alis.basemain + scridx + 0x1e, val); }
static inline void set_scr_redshift(u32 scridx, u8 val)        { xwrite8(alis.basemain + scridx + 0x1f, val); }

static inline s8  get_scr_unknown0x20(u32 scridx)              { return xread8(alis.basemain + scridx + 0x20); }
static inline void set_scr_unknown0x20(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x20, val); }

static inline s8  get_scr_unknown0x21(u32 scridx)              { return xread8(alis.basemain + scridx + 0x21); }
static inline void set_scr_unknown0x21(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x21, val); }

static inline s8  get_scr_unknown0x22(u32 scridx)              { return xread8(alis.basemain + scridx + 0x22); }
static inline void set_scr_unknown0x22(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x22, val); }

static inline s8  get_scr_unknown0x23(u32 scridx)              { return xread8(alis.basemain + scridx + 0x23); }
static inline void set_scr_unknown0x23(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x23, val); }

static inline s8  get_scr_unknown0x24(u32 scridx)              { return xread8(alis.basemain + scridx + 0x24); }
static inline void set_scr_unknown0x24(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x24, val); }

static inline s8  get_scr_unknown0x25(u32 scridx)              { return xread8(alis.basemain + scridx + 0x25); }
static inline void set_scr_unknown0x25(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x25, val); }

static inline s8  get_scr_unknown0x26(u32 scridx)              { return xread8(alis.basemain + scridx + 0x26); }
static inline void set_scr_unknown0x26(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x26, val); }

static inline s8  get_scr_unknown0x27(u32 scridx)              { return xread8(alis.basemain + scridx + 0x27); }
static inline void set_scr_unknown0x27(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x27, val); }

static inline s8  get_scr_unknown0x28(u32 scridx)              { return xread8(alis.basemain + scridx + 0x28); }
static inline void set_scr_unknown0x28(u32 scridx, s8 val)     { xwrite8(alis.basemain + scridx + 0x28, val); }

static inline u8  get_scr_unknown0x29(u32 scridx)              { return xread8(alis.basemain + scridx + 0x29); }
static inline void set_scr_unknown0x29(u32 scridx, u8 val)     { xwrite8(alis.basemain + scridx + 0x29, val); }

static inline s16 get_scr_unknown0x2a(u32 scridx)              { return xread16(alis.basemain + scridx + 0x2a); }
static inline void set_scr_unknown0x2a(u32 scridx, s16 val)    { xwrite16(alis.basemain + scridx + 0x2a, val); }

static inline s16 get_scr_unknown0x2c(u32 scridx)              { return xread16(alis.basemain + scridx + 0x2c); }
static inline void set_scr_unknown0x2c(u32 scridx, s16 val)    { xwrite16(alis.basemain + scridx + 0x2c, val); }

static inline s16 get_scr_unknown0x2e(u32 scridx)              { return xread16(alis.basemain + scridx + 0x2e); }
static inline void set_scr_unknown0x2e(u32 scridx, s16 val)    { xwrite16(alis.basemain + scridx + 0x2e, val); }

void scadd(s16 scridx);
void scbreak(s16 scridx);
void scdosprite(s16 scridx);
void vectoriel(s16 scridx);
