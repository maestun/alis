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

#include "config.h"

u16         read16(const u8 *ptr);
u32         read24(const u8 *ptr);
u32         read32(const u8 *ptr);
void        write32(const u8 *ptr, u32 value);

u16         fread16(FILE * fp);
u32         fread32(FILE * fp);
u16         swap16(const u8 *);
u32         swap32(const u8 *);


u16             xswap16(u16 value);
u32             xswap32(u32 value);

u8              xread8(u32 offset);
s16             xread16(u32 offset);
s32             xread32(u32 offset);

void            xwrite8(u32 offset, u8 value);
void            xwrite16(u32 offset, s16 value);
void            xwrite32(u32 offset, s32 value);

void            xadd8(s32 offset, s8 value);
void            xadd16(s32 offset, s16 value);
void            xadd32(s32 offset, s32 add);

void            xsub8(s32 offset, s8 value);
void            xsub16(s32 offset, s16 value);
void            xsub32(s32 offset, s32 sub);

void            xpush32(s32 value);
s32             xpeek32(void);
s32             xpop32(void);

s16             xpcread16(u32 offset);
s32             xpcread32(u32 offset);

void            xpcwrite16(u32 offset, s16 value);
void            xpcwrite32(u32 offset, s32 value);

#define vread16 xread16
#define vread32 xread32
#define vwrite8 xwrite8
#define vwrite16 xwrite16
#define vwrite32 xwrite32
