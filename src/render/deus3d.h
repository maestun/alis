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

typedef struct {
    s32 solidband;
    u8  solidflag;
    u8  identband;
    s32 bands;
} sV6Shad;

extern sV6Shad v6shad;

void v6shad_build3(u32 scr, u32 desc);

typedef struct {
    s16 m[3][3];
    u8  flags;
    u8  pad13;
    s32 t[3];
    s32 params[9];
    s32 scale;
    s32 params2[2];
} sV6Ctx;

int v6x3d_pickwalk(u32 scr, s32 px, s32 py, s32 zmin, s32 w3[3], u32 *elem, u32 *tag, u32 *depth);
extern u8 v6x3d_heapmoved;
void v6tf_build(u32 src, u32 dst, sV6Ctx *out);
void v6tf_apply(const s32 v[3], const sV6Ctx *c, s32 out[3]);
u32  v6x_isqrt(u32 n);

void v6col_sphere_scan(u32 chain, u16 srad, s16 scx, s16 scy, s16 scz, u16 selfid, u16 scroff);

extern s32 v6x_walkzoff;
extern s16 v6x_walkaoff;
void v6x3d_open(u32 scr);
void v6x3d_flush(u32 scr);
void v6x_folscreen(u32 s);
void v6x_mode6(u32 s);
void v6x_skyfill(u32 s);
extern u32 v6x3d_tick;
