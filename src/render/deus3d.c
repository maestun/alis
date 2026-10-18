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

#include "deus3d.h"

#include <stdint.h>
#include <string.h>
#include "alis.h"
#include "alis_private.h"
#include "audio.h"
#include "debug.h"
#include "image.h"
#include "mem.h"
#include "utils.h"

sV6Shad v6shad;

// weighted nearest-palette-colour search
static u8 v6shad_nearest(u32 pal, s32 r, s32 g, s32 b, u8 lo, u8 hi)
{
    s32 tr = r + 1;
    if (tr > 255)
        tr = 255; tr &= ~3;
    
    s32 tg = g + 1;
    if (tg > 255)
        tg = 255; tg &= ~3;
    
    s32 tb = b + 1;
    if (tb > 255)
        tb = 255; tb &= ~3;
    
    u32 best = 0xffffffff;
    u8 idx = lo;
    for (s32 i = lo; i <= hi; i++)
    {
        u32 d = 0;
        s32 dv = xread8(pal + i * 3 + 0) - tr;
        if (dv)
            d += (u32)(dv * dv) * 0x11;
        
        dv = xread8(pal + i * 3 + 1) - tg;
        if (dv)
            d += (u32)(dv * dv) * 0x18;
        
        dv = xread8(pal + i * 3 + 2) - tb;
        if (dv)
            d += (u32)(dv * dv) * 0x0a;
        
        if (d < best)
        {
            best = d;
            idx = (u8)i;
            if (d == 0)
                break;
        }
    }
    
    return idx;
}

// build 256x256 colour-mix LUT
static void v6shad_mixlut(u32 pal, u32 dst, u32 pre, u8 lo, u8 hi)
{
    for (s32 row = 0; row < 256; row++)
    {
        u8 rowR = xread8(pal + row * 3 + 0);
        u8 rowG = xread8(pal + row * 3 + 1);
        u8 rowB = xread8(pal + row * 3 + 2);
        if (pre && row == 0)
            pre++;
        
        xwrite8(dst + row * 256 + 0, (u8)row);
        for (s32 c = 1; c < 256; c++)
        {
            u8 al;
            int compute = 1;
            if (c < row)
            {
                al = xread8(dst + c * 256 + row);
                compute = 0;
            }
            else if (pre)
            {
                al = xread8(pre++);
                if (al >= lo && al <= hi)
                    compute = 0;
            }
            
            if (compute)
            {
                s32 r = (xread8(pal + c * 3 + 0) + rowR) >> 1;
                s32 g = (xread8(pal + c * 3 + 1) + rowG) >> 1;
                s32 b = (xread8(pal + c * 3 + 2) + rowB) >> 1;
                al = v6shad_nearest(pal, r, g, b, lo, hi);
            }
            
            xwrite8(dst + row * 256 + c, al);
        }
    }
}

// build fog/darkness remap rows
static void v6shad_fogrows(u32 scr, u32 pal, u32 dst, s32 width, s16 start)
{
    u8 lo = xread8(scr + 0xb8);
    if (lo == 0)
        lo = 1;
    
    u8 hi = xread8(scr + 0xba);
    u16 mode = xread16(scr + 0x94);
    s16 curR = start, curG = start, curB = start;
    static s16 stepR, stepG, stepB;
    s32 fogR = 0, fogG = 0, fogB = 0;
    s32 pivot;

    v6shad.solidband = -1;
    v6shad.identband = (u8)width;
    v6shad.bands = width - 1;

    if ((mode & 0xff) == 0)
    {
        if (!(mode & 0x100))
        {
            xwrite8(scr + 0x13b, 0);
        }
        
        if (start == 0)
        {
            v6shad.solidband = 0;
            v6shad.solidflag = 1;
        }

        if (mode & 0x400)
        {
            s32 d32 = xread32(scr + 0xa2) >> 7;
            s16 d = (s16)d32, num;
            if (d32)
            {
                num = 0x100 - curR;
                stepR = num / d; curR += num % d;
            }
            else
            {
                curR = 0x100;
                stepR = (s16)xread16(scr + 0x9a) / width;
            }
            
            if (d32)
            {
                num = 0x100 - curG;
                stepG = num / d;
                curG += num % d;
            }
            else
            {
                curG = 0x100;
                stepG = (s16)xread16(scr + 0x9c) / width;
            }
            
            if (d32)
            {
                num = 0x100 - curB;
                stepB = num / d;
                curB += num % d;
            }
            else
            {
                curB = 0x100;
                stepB = (s16)xread16(scr + 0x9e) / width;
            }
        }
        else
        {
            stepR = (s16)xread16(scr + 0x9a) / width;
            stepG = (s16)xread16(scr + 0x9c) / width;
            stepB = (s16)xread16(scr + 0x9e) / width;
        }
        
        pivot = width;
    }
    else
    {
        v6shad.solidband = v6shad.bands;
        v6shad.solidflag = 0;
        u8 fogidx = v6shad_nearest(pal, (s16)xread16(scr + 0x9a), (s16)xread16(scr + 0x9c), (s16)xread16(scr + 0x9e), lo, hi);
        if (!(mode & 0x100))
            xwrite8(scr + 0x13b, fogidx);
        
        fogR = xread8(pal + fogidx * 3 + 0) << 8;
        fogG = xread8(pal + fogidx * 3 + 1) << 8;
        fogB = xread8(pal + fogidx * 3 + 2) << 8;
        pivot = 0;
        if ((mode & 0xff) != 2)
        {
            if (mode & 0x400)
            {
                s32 d32 = xread32(scr + 0xa2) >> 7;
                s16 d = (s16)d32, num;
                if (d32)
                {
                    num = 0x100 - curR; stepR = num / d;
                    curR += num % d;
                }
                else
                {
                    curR = 0x100;
                    stepR = (s16)((s16)xread16(scr + 0x9a) + 0x100 - curR) / width;
                }
                
                if (d32)
                {
                    num = 0x100 - curG;
                    stepG = num / d;
                    curG += num % d;
                }
                else
                {
                    curG = 0x100;
                    stepG = (s16)((s16)xread16(scr + 0x9c) + 0x100 - curG) / width;
                }
                
                if (d32)
                {
                    num = 0x100 - curB;
                    stepB = num / d;
                    curB += num % d;
                }
                else
                {
                    curB = 0x100;
                    stepB = (s16)((s16)xread16(scr + 0x9e) + 0x100 - curB) / width;
                }
            }
            else
            {
                stepR = (s16)((s16)xread16(scr + 0x9a) + 0x100 - curR) / width;
                stepG = (s16)((s16)xread16(scr + 0x9c) + 0x100 - curG) / width;
                stepB = (s16)((s16)xread16(scr + 0x9e) + 0x100 - curB) / width;
            }
            
            pivot = width >> 1;
        }
    }

    for (s32 row = 0; row < width; row++)
    {
        for (s32 c = 0; c < 256; c++)
        {
            if (c == 0)
            {
                xwrite8(dst + row * 256, 0);
                continue;
            }
            
            if (c < lo || c > hi)
            {
                continue;
            }
            
            s32 tr, tg, tb;
            if (row < pivot)
            {
                u16 cr = curR < 0 ? 0 : (u16)curR;
                u16 cg = curG < 0 ? 0 : (u16)curG;
                u16 cb = curB < 0 ? 0 : (u16)curB;
                tr = (s16)((u32)(xread8(pal + c * 3 + 0) * cr) >> 8);
                tg = (s16)((u32)(xread8(pal + c * 3 + 1) * cg) >> 8);
                tb = (s16)((u32)(xread8(pal + c * 3 + 2) * cb) >> 8);
            }
            else
            {
                stepR = stepG = stepB = 0;
                const s32 den = width - pivot;
                s32 v, q;
                v = xread8(pal + c * 3 + 0) * (u32)(u16)curR;
                q = (s32)((s64)(v - fogR) * (row - pivot) / den);
                tr = (s16)((v - q) >> 8);
                v = xread8(pal + c * 3 + 1) * (u32)(u16)curG;
                q = (s32)((s64)(v - fogG) * (row - pivot) / den);
                tg = (s16)((v - q) >> 8);
                v = xread8(pal + c * 3 + 2) * (u32)(u16)curB;
                q = (s32)((s64)(v - fogB) * (row - pivot) / den);
                tb = (s16)((v - q) >> 8);
            }

            xwrite8(dst + row * 256 + c, v6shad_nearest(pal, tr, tg, tb, lo, hi));
        }

        curR += stepR;
        curG += stepG;
        curB += stepB;
    }
    
    for (s32 i = 0; i < 256; i++)
        xwrite8(dst + width * 256 + i, (u8)i);
}

void v6shad_build3(u32 scr, u32 desc)
{
    s32 width = (u16)xread16(scr + 0x96);
    if (xread32(scr + 0xb4) == 0)
    {
        u16 node = alis.platform.kind != EPlatform3DO ? node_take(0xb) : 0;
        xwrite32(scr + 0xb4, io_malloc(((width + 1) << 8) + 0x10000));
        if (node)
            scrnode_set(scr, 0xb4, node);
    }
    
    s32 hbuf = xread32(scr + 0xb4);
    if (hbuf == 0)
        return;
    
    u32 buf = xread32(hbuf);
    u32 pal = desc + 0xc;
    u32 pre = (xread8(desc + 0xb) & 1) ? desc + 0x30c : 0;
    v6shad_mixlut(pal, buf, pre, xread8(scr + 0xb8), xread8(scr + 0xba));
    v6shad_fogrows(scr, pal, buf + 0x10000, width, (s16)xread16(scr + 0x98));
}

#include "v6costab.h"

#define V6COS(i) ((s32)v6costab[(i) & 0xfff])
#define V6SIN(i) ((s32)v6costab[((i) + 0xc00) & 0xfff])

// ctx outputs / accumulators
sV6Ctx v6tf_objcam;
sV6Ctx v6tf_camobj;
sV6Ctx v6tf_light;
sV6Ctx v6tf_parentctx;
static sV6Ctx v6tf_w2c;
static sV6Ctx v6tf_c2w;
static sV6Ctx v6tf_destacc;
static sV6Ctx v6tf_scratch;
s32 v6tf_eye[4];

u8 v6tf_cacheon;
static sV6Ctx *v6tf_out;
static u8 v6tf_srcflags;
static u16 v6tf_srcscale;
static u32 v6tf_camera;

static u8 v6tf_pool[0x8020];
static u32 v6tf_poolnext, v6tf_poolend;
static u32 v6tf_hits, v6tf_allocs;

static void v6tf_identity(sV6Ctx *c)
{
    memset(c->m, 0, sizeof(c->m));
    c->m[0][0] = c->m[1][1] = c->m[2][2] = 0x7fff;
    c->t[0] = c->t[1] = c->t[2] = 0;
}

#define V6ROTL(lo, hi) { s64 a = (s64)(lo) * C - (s64)(hi) * S; s64 b = (s64)(lo) * S + (s64)(hi) * C; (lo) = (s32)(a >> 15); (hi) = (s32)(b >> 15); }

static void v6tf_rotx_left(sV6Ctx *c, s32 angle)
{
    u32 i = ((u16)angle) >> 4;
    if (!i)
        return;
    
    s32 C = V6COS(i), S = V6SIN(i);
    for (int col = 0; col < 3; col++)
    {
        s32 lo = c->m[1][col], hi = c->m[2][col];
        V6ROTL(lo, hi); c->m[1][col] = (s16)lo; c->m[2][col] = (s16)hi;
    }
    
    V6ROTL(c->t[1], c->t[2]);
}

static void v6tf_roty_left(sV6Ctx *c, s32 angle)
{
    u32 i = ((u16)angle) >> 4;
    if (!i)
        return;
    
    s32 C = V6COS(i), S = V6SIN(i);
    for (int col = 0; col < 3; col++)
    {
        s32 lo = c->m[2][col], hi = c->m[0][col];
        V6ROTL(lo, hi); c->m[2][col] = (s16)lo; c->m[0][col] = (s16)hi;
    }
    
    V6ROTL(c->t[2], c->t[0]);
}

static void v6tf_rotz_left(sV6Ctx *c, s32 angle)
{
    u32 i = ((u16)angle) >> 4;
    if (!i)
        return;
    
    s32 C = V6COS(i), S = V6SIN(i);
    for (int col = 0; col < 3; col++)
    {
        s32 lo = c->m[0][col], hi = c->m[1][col];
        V6ROTL(lo, hi); c->m[0][col] = (s16)lo; c->m[1][col] = (s16)hi;
    }
    
    V6ROTL(c->t[0], c->t[1]);
}

#define V6ROTR(lo, hi) { s64 a =  (s64)(lo) * C + (s64)(hi) * S; s64 b = -(s64)(lo) * S + (s64)(hi) * C; (lo) = (s32)(a >> 15); (hi) = (s32)(b >> 15); }

static void v6tf_rotx_right(sV6Ctx *c, s32 angle)
{
    u32 i = ((u16)angle) >> 4;
    if (!i)
        return;
    
    s32 C = V6COS(i), S = V6SIN(i);
    for (int r = 0; r < 3; r++)
    {
        s32 lo = c->m[r][1], hi = c->m[r][2];
        V6ROTR(lo, hi); c->m[r][1] = (s16)lo; c->m[r][2] = (s16)hi;
    }
}

static void v6tf_roty_right(sV6Ctx *c, s32 angle)
{
    u32 i = ((u16)angle) >> 4;
    if (!i)
        return;
    
    s32 C = V6COS(i), S = V6SIN(i);
    for (int r = 0; r < 3; r++)
    {
        s32 lo = c->m[r][2], hi = c->m[r][0];
        V6ROTR(lo, hi); c->m[r][2] = (s16)lo; c->m[r][0] = (s16)hi;
    }
}

static void v6tf_rotz_right(sV6Ctx *c, s32 angle)
{
    u32 i = ((u16)angle) >> 4;
    if (!i)
        return;
    
    s32 C = V6COS(i), S = V6SIN(i);
    for (int r = 0; r < 3; r++)
    {
        s32 lo = c->m[r][0], hi = c->m[r][1];
        V6ROTR(lo, hi); c->m[r][0] = (s16)lo; c->m[r][1] = (s16)hi;
    }
}

void v6tf_apply(const s32 v[3], const sV6Ctx *c, s32 out[3])
{
    s32 x = (s32)(((s64)c->m[0][0]*v[0] + (s64)c->m[0][1]*v[1] + (s64)c->m[0][2]*v[2]) >> 15) + c->t[0];
    s32 y = (s32)(((s64)c->m[1][0]*v[0] + (s64)c->m[1][1]*v[1] + (s64)c->m[1][2]*v[2]) >> 15) + c->t[1];
    s32 z = (s32)(((s64)c->m[2][0]*v[0] + (s64)c->m[2][1]*v[1] + (s64)c->m[2][2]*v[2]) >> 15) + c->t[2];
    out[2] = z; out[1] = y; out[0] = x;
}

static void v6tf_combine(const sV6Ctx *D, const sV6Ctx *A, sV6Ctx *out)
{
    if (D == (const sV6Ctx *)v6tf_pool)
    {
        memcpy(out, A, 0x20);
        return;
    }
    
    if (A == (const sV6Ctx *)v6tf_pool)
    {
        memcpy(out, D, 0x20);
        return;
    }
    
    sV6Ctx *o = out;
    if (D == out || A == out)
        o = &v6tf_scratch;
    
    for (int r = 0; r < 3; r++)
        for (int col = 0; col < 3; col++)
            o->m[r][col] = (s16)(((s64)D->m[r][0]*A->m[0][col] + (s64)D->m[r][1]*A->m[1][col] + (s64)D->m[r][2]*A->m[2][col]) >> 15);
    
    for (int r = 0; r < 3; r++)
        o->t[r] = D->t[r] + (s32)(((s64)D->m[r][0]*A->t[0] + (s64)D->m[r][1]*A->t[1] + (s64)D->m[r][2]*A->t[2]) >> 15);
    
    if (o == &v6tf_scratch)
        memcpy(out, o, 0x20);
}

// cache primitives
static sV6Ctx *v6tf_probe_src(u32 e)
{
    if (e == 0)
        return &v6tf_w2c;
    
    u16 w = xread16(e + 0x20);
    if (!w || !(w & 1))
        return NULL;
    
    v6tf_hits++;
    return (sV6Ctx *)(v6tf_pool + (w & ~3u));
}

static sV6Ctx *v6tf_probe_dst(u32 e)
{
    if (e == 0)
        return &v6tf_c2w;
    
    u16 w = xread16(e + 0x20);
    if (!w || !(w & 2))
        return NULL;
    
    v6tf_hits++;
    return (sV6Ctx *)(v6tf_pool + (w & ~3u) + 0x20);
}

static void v6tf_store(u32 e, const sV6Ctx *c, u16 bit, u32 half)
{
    u32 off = xread16(e + 0x20);
    if (!off)
    {
        if (v6tf_poolnext >= v6tf_poolend)
            return;
        
        off = v6tf_poolnext;
        xwrite16(e + 0x20, (u16)off);
        v6tf_poolnext += 0x40;
        v6tf_allocs++;
    }
    
    xwrite16(e + 0x20, xread16(e + 0x20) | bit);
    memcpy(v6tf_pool + (off & ~3u) + half, c, 0x20);
}

// apply src entitys local transform
static void v6tf_apply_src_local(sV6Ctx *c, u32 e)
{
    s32 px = xread32(e + 0x0c), py = xread32(e + 0x10), pz = xread32(e + 0x14);
    for (int r = 0; r < 3; r++)
        c->t[r] += (s32)(((s64)c->m[r][0]*px + (s64)c->m[r][1]*py + (s64)c->m[r][2]*pz) >> 15);
    
    v6tf_rotz_right(c, (s16)xread16(e + 0x1c));
    v6tf_roty_right(c, (s16)xread16(e + 0x1a));
    v6tf_rotx_right(c, (s16)xread16(e + 0x18));
}

// apply dest entity's inverse local transform
static void v6tf_apply_dst_inverse(sV6Ctx *c, u32 e)
{
    c->t[0] -= xread32(e + 0x0c);
    c->t[1] -= xread32(e + 0x10);
    c->t[2] -= xread32(e + 0x14);
    
    if (!(v6tf_srcflags & 0x8))
        v6tf_rotz_left(c, -(s16)xread16(e + 0x1c));
    
    v6tf_roty_left(c, -(s16)xread16(e + 0x1a));
    if (!(v6tf_srcflags & 0x2))
        v6tf_rotx_left(c, -(s16)xread16(e + 0x18));
}

static sV6Ctx *v6tf_walk_src(u32 e)
{
    sV6Ctx *hit = v6tf_probe_src(e);
    if (hit)
        return hit;
    
    sV6Ctx *acc = v6tf_walk_src(xread32(e + 0x8));
    if (acc != v6tf_out)
    {
        memcpy(v6tf_out, acc, 0x20);
        acc = v6tf_out;
    }
    
    v6tf_apply_src_local(acc, e);
    v6tf_store(e, acc, 1, 0);
    return acc;
}

static sV6Ctx *v6tf_walk_dst(u32 e)
{
    sV6Ctx *hit = v6tf_probe_dst(e);
    if (hit)
        return hit;
    
    sV6Ctx *acc = v6tf_walk_dst(xread32(e + 0x8));
    if (acc != &v6tf_destacc)
    {
        memcpy(&v6tf_destacc, acc, 0x20);
        acc = &v6tf_destacc;
    }
    
    v6tf_apply_dst_inverse(acc, e);
    v6tf_store(e, acc, 2, 0x20);
    return acc;
}

static void v6tf_cached(u32 src, u32 dst)
{
    sV6Ctx *A = v6tf_walk_src(src);
    if (dst != v6tf_camera)
    {
        sV6Ctx *D = v6tf_walk_dst(dst);
        if (src != v6tf_camera)
        {
            v6tf_combine(D, A, v6tf_out);
            return;
        }
        
        A = D;
    }
    
    if (A != v6tf_out)
        memcpy(v6tf_out, A, 0x20);
}

// full rebuild dest side
static void v6tf_full_dst(u32 src, u32 d, sV6Ctx *c)
{
    if (d == 0)
        return;
    
    v6tf_full_dst(src, xread32(d + 0x8), c);
    if (src != 0 && (xread8(d + 0x25) & 0x40))
    {
        v6tf_identity(c);
        c->scale = src ? (s32)(s16)xread16(src + 0x1e) : 0x100;
    }
    
    c->t[0] -= xread32(d + 0x0c);
    c->t[1] -= xread32(d + 0x10);
    c->t[2] -= xread32(d + 0x14);
    if (!(c->flags & 0x8))
        v6tf_rotz_left(c, -(s16)xread16(d + 0x1c));
    
    v6tf_roty_left(c, -(s16)xread16(d + 0x1a));
    if (!(c->flags & 0x2))
        v6tf_rotx_left(c, -(s16)xread16(d + 0x18));
}

static void v6tf_full(u32 src, u32 dst, sV6Ctx *c)
{
    c->flags = src ? xread8(src + 0x25) : 0;
    v6tf_identity(c);
    c->scale = src ? (s32)(s16)xread16(src + 0x1e) : 0x100;
    for (u32 e = src; e != 0; e = xread32(e + 0x8))
    {
        if (xread8(e + 0x25) & 0x40)
        {
            c->scale = (s32)(s16)xread16(e + 0x1e);
            s32 x = xread32(e + 0x0c), y = xread32(e + 0x10), z = xread32(e + 0x14);
            for (u32 p = xread32(e + 0x8); p != 0; p = xread32(p + 0x8))
            {
                x += xread32(p + 0x0c); y += xread32(p + 0x10); z += xread32(p + 0x14);
            }
            
            c->t[0] += x; c->t[1] += y; c->t[2] += z;
            return;
        }
        
        if (!(c->flags & 0x2))
            v6tf_rotx_left(c, (s16)xread16(e + 0x18));
        
        v6tf_roty_left(c, (s16)xread16(e + 0x1a));
        if (!(c->flags & 0x8))
            v6tf_rotz_left(c, (s16)xread16(e + 0x1c));
        
        c->t[0] += xread32(e + 0x0c);
        c->t[1] += xread32(e + 0x10);
        c->t[2] += xread32(e + 0x14);
        if (xread8(e + 0x25) & 0x20)
            return;
    }
    
    v6tf_full_dst(src, dst, c);
}

void v6tf_build(u32 src, u32 dst, sV6Ctx *out)
{
    if (!v6tf_cacheon)
    {
        v6tf_full(src, dst, out);
        return;
    }
    
    v6tf_out = out;
    v6tf_srcflags = 0;
    v6tf_srcscale = 0x100;
    if (src != 0)
    {
        if ((xread8(src + 0x25) & 0xea) || (dst != 0 && (xread8(dst + 0x25) & 0xea)))
        {
            v6tf_full(src, dst, out);
            return;
        }
        
        v6tf_srcflags = xread8(src + 0x25);
        v6tf_srcscale = xread16(src + 0x1e);
    }
    
    v6tf_cached(src, dst);
}

// per-frame transform setup
void v6tf_frame_setup(u32 s)
{
    v6tf_hits = v6tf_allocs = 0;
    v6tf_cacheon = 0;

    v6tf_camera = s;
    v6tf_build(0, s, &v6tf_w2c);
    v6tf_build(s, 0, &v6tf_c2w);
    v6tf_eye[0] = (s16)xread16(s + 0x42);
    v6tf_eye[1] = (s16)xread16(s + 0x44);
    v6tf_eye[2] = (s16)xread16(s + 0x46);
    v6tf_eye[3] = (u16)xread16(s + 0x40);
    v6tf_apply(v6tf_eye, &v6tf_c2w, v6tf_eye);
    v6tf_cacheon = 1;
    v6tf_poolnext = 0x20;
    v6tf_poolend  = 0x20 + 0x8000;

    u32 head = xread32(s + 0x5e);
    if (head != 0)
    {
        for (u32 e = xread32(head + 0x38); e != 0 && e != xread32(e + 0x38); e = xread32(e + 0x38))
        {
            xwrite16(e + 0x20, 0);
        }
    }
}

// depth-slot face lists
static u32 v6x3d_slothead[0x2000];
static u8  v6x3d_slotdirty[0x200];

// face-record arena
static u32 v6x3d_arena;
static u32 v6x3d_arenacur;
static u32 v6x3d_arenalim;
static u32 v6x3d_shl16;

// frame counters
static u32 v6x3d_cntfaces, v6x3d_cntverts;
u8 v6x3d_heapmoved;

typedef struct {
    s32 lx;                 // +0x00 left X 16.16 (integer part = word +0x02)
    u16 lv, lu;             // +0x04/+0x06 left linear uv 8.8 (v low, u high word)
    s32 lv15, lu15, lz15;   // +0x08/+0x0c/+0x10 perspective left v/u/z << 15
    u16 lband, rband;       // +0x14/+0x16 band 8.8 pair (left low, right high)
    s32 rx;                 // +0x18 right X 16.16 (integer part = word +0x1a)
    u16 rv, ru;             // +0x1c/+0x1e right linear uv 8.8
    s32 rv15, ru15, rz15;   // +0x20/+0x24/+0x28 perspective right
    s32 pad2c;              // +0x2c
    s32 rowoff;             // +0x30 y*pitch (FUN_000147fa, build-time)
    u8  tail[0xc];          // +0x34..+0x3f
} sV6Row;
static sV6Row v6x3d_rows[500];

static u8  v6x3d_pick;
static u8  v6x3d_pickhit;
static s32 v6x3d_pickx, v6x3d_picky;

typedef struct {
    u32 type;           // +0x00 elem type byte (0x10)
    s32 intensity;      // +0x04 s16(elem+0x48) * bandcount
    u32 range;          // +0x08
    u8  _pad0[4];
    s32 slope;          // +0x10 falloff slope / ambient add
    u32 fstart;         // +0x14 falloff start
    s32 int8fp;         // +0x18 intensity<<8
    s32 objpos[3];      // +0x1c per-object: pos in object space (143d2)
    s32 campos[3];      // +0x28 pos in camera space
    s32 pos[3];         // +0x34 pos (light-local / WORLD for directional)
    s32 dir[3];         // +0x40 direction
    u8  active;         // +0x4c per-object active flag (143d2)
    u8  spot;           // +0x4d
    u8  kind;           // +0x4e 1 = directional/ambient
    u8  _pad1;
    s32 cone;           // +0x50 spot cone threshold
    s32 objdir[3];      // +0x54 per-object rotated dir #2 (143d2)
    s32 camdir[3];      // +0x60 spot/camera-space dir
    u32 elem;           // +0x6c light element VM addr
    sV6Ctx ctx;         // +0x70 (0x20-byte composable part used)
} sV6Light;
static sV6Light v6x3d_light[0x40];
static int v6x3d_nlights;

// fog/light constants latched by the light build
typedef struct {
    s32 bands;          // fog band count
    s32 bandclamp;      // no-fog clamp
    u8  bandshift;
    u16 mode;           // cscdark mode
    s32 ambbase;
    s32 fogstart;
    s32 fogslope;
    s32 attslope;
    s32 baselevel;
} sV6Fog;
sV6Fog v6x3dfog;

// integer square root
u32 v6x_isqrt(u32 n)
{
    if (n == 0)
    {
        return 0;
    }
    
    u32 b = n, c = 0;
    if (b >= 0x10000)
    {
        c += 16;
        b >>= 16;
    }
    
    if (b >= 0x100)
    {
        c += 8;
        b >>= 8;
    }
    
    if (b >= 0x10)
    {
        c += 4;
        b >>= 4;
    }
    
    if (b >= 4)
    {
        c += 2;
        b >>= 2;
    }

    if (b >= 2)
    {
        c += 1;
        b >>= 1;
    }
    
    c >>= 1;
    
    u32 x = n >> c;
    x = (n / x + x) >> 1;
    x = (n / x + x) >> 1;
    x = (n / x + x) >> 1;
    return x;
}

// sphere-vs-view-sphere test
static int v6x3d_cull_vs(const s32 sph[4], u32 elem);
static int v6x3d_cull(u32 elem) { return v6x3d_cull_vs(v6tf_eye, elem); }

static int v6x3d_cull_vs(const s32 sph[4], u32 elem)
{
    s32 P[3];
    u8 b23 = xread8(elem + 0x23);
    if (b23 & 0x10)
    {
        P[0] = xread32(elem + 0x0c);
        P[1] = xread32(elem + 0x10);
        P[2] = xread32(elem + 0x14);
    }
    else if (b23 & 0x40)
    {
        P[0] = (s16)xread16(elem + 0x42);
        P[1] = (s16)xread16(elem + 0x44);
        P[2] = (s16)xread16(elem + 0x46);
        for (u32 e2 = elem; e2 != 0; e2 = xread32(e2 + 0x8))
        {
            P[0] += xread32(e2 + 0x0c);
            P[1] += xread32(e2 + 0x10);
            P[2] += xread32(e2 + 0x14);
        }
    }
    else
    {
        u32 e2 = elem;
        while (xread32(e2 + 0x8) != 0)
            e2 = xread32(e2 + 0x8);
        
        P[0] = xread32(e2 + 0x0c);
        P[1] = xread32(e2 + 0x10);
        P[2] = xread32(e2 + 0x14);
    }

    s32 d, q;
    s64 p;
    d = (P[0] - sph[0]) >> 1;
    p = (s64)d * d; if (p > 0x7fffffffLL) return 0;
    q = (s32)p;
    d = (P[1] - sph[1]) >> 1;
    p = (s64)d * d; if (p > 0x7fffffffLL) return 0;
    q += (s32)p;
    d = (P[2] - sph[2]) >> 1;
    p = (s64)d * d; if (p > 0x7fffffffLL) return 0;
    q += (s32)p;
    u32 R = ((u32)(u16)xread16(elem + 0x40) + (u32)sph[3]) >> 1;
    return (u32)(R * R) > (u32)q;
}

static s32 v6col_pw[4];
static s32 v6col_pc[3];
static s32 v6col_r7, v6col_r2, v6col_r;
static u32 v6col_matout;

// probe sphere -> world space
static void v6col_probe_world(u32 chain, u16 srad, s16 cx, s16 cy, s16 cz)
{
    sV6Ctx c;
    v6tf_build(chain, 0, &c);
    v6col_pw[0] = cx; v6col_pw[1] = cy; v6col_pw[2] = cz;
    v6tf_apply(v6col_pw, &c, v6col_pw);
    u32 r = srad;
    if (r < (u32)alis.v6focstep)
        r = (u32)alis.v6focstep;
    
    v6col_pw[3] = (s32)r;
}

// candidate sphere vs the probe
static int v6col_test_sphere(u32 cand, u16 prad)
{
    if (xread8(cand + 0x23) & 0x70)
    {
        if (!v6x3d_cull_vs(v6col_pw, cand))
            return 0;
    }
    
    if (xread8(cand + 0x25) & 4)
    {
        return 0;
    }
    
    u32 rsum = (u16)((u16)xread16(cand + 0x40) + prad);
    sV6Ctx c;
    v6tf_build(cand, 0, &c);
    s32 wc[3] = { (s16)xread16(cand + 0x42), (s16)xread16(cand + 0x44), (s16)xread16(cand + 0x46) };
    v6tf_apply(wc, &c, wc);
    u32 sum = 0;
    for (int i = 0; i < 3; i++)
    {
        s64 pp = (s64)(wc[i] - v6col_pw[i]) * (wc[i] - v6col_pw[i]);
        if ((u64)pp > 0xffffffffULL)
            return 0;
        
        sum += (u32)pp;
    }

    return sum < rsum * rsum;
}

static u32 v6col_facesA, v6col_verts, v6col_mats;
static u8 v6col_fbits[0x2000];

static int v6col_intri(u32 face, int second, const s32 F[3])
{
    u32 vb = v6col_verts;
    u32 P = vb + 12u * (u16)xread16(face + 0);
    u32 R = vb + 12u * (u16)xread16(face + (second ? 6 : 2));
    u32 Q = vb + 12u * (u16)xread16(face + 4);
    s32 p[3], q[3], r3[3];
    for (int i = 0; i < 3; i++)
    {
        p[i] = (s16)xread16(P + (u32)i * 2);
        q[i] = (s16)xread16(Q + (u32)i * 2);
        r3[i] = (s16)xread16(R + (u32)i * 2);
    }

    for (int k = 0; k < 3; k++)
    {
        const s32 *v = k == 0 ? p : k == 1 ? q : r3;
        u32 d2 = 0;
        for (int i = 0; i < 3; i++)
        {
            s32 d = v[i] - v6col_pc[i];
            d2 += (u32)(d * d);
        }

        if (d2 <= (u32)v6col_r2)
            return 1;
    }

    for (int k = 0; k < 3; k++)
    {
        const s32 *a = k == 0 ? p : k == 1 ? p : q;
        const s32 *b = k == 0 ? q : k == 1 ? r3 : r3;
        u32 d2 = 0;
        for (int i = 0; i < 3; i++)
        {
            s32 d = ((a[i] + b[i]) >> 1) - v6col_pc[i];
            d2 += (u32)(d * d);
        }

        if (d2 <= (u32)v6col_r2)
            return 1;
    }

    s32 c0 = (p[0]-r3[0])*(q[1]-r3[1]) - (p[1]-r3[1])*(q[0]-r3[0]);
    s32 denom = c0, mag = c0 < 0 ? -c0 : c0;
    int axis = 0;
    s32 c1 = (p[0]-r3[0])*(q[2]-r3[2]) - (p[2]-r3[2])*(q[0]-r3[0]);
    s32 m1 = c1 < 0 ? -c1 : c1;
    if (m1 >= mag)
    {
        denom = c1;
        mag = m1;
        axis = 1;
    }

    s32 c2 = (p[1]-r3[1])*(q[2]-r3[2]) - (p[2]-r3[2])*(q[1]-r3[1]);
    s32 m2 = c2 < 0 ? -c2 : c2;
    if (m2 >= mag)
    {
        denom = c2;
        mag = m2;
        axis = 2;
    }

    if (denom == 0)
        return 0;
    
    int ia = axis == 2 ? 1 : 0;
    int ib = axis == 0 ? 1 : 2;
    s32 n1 = (F[ia]-r3[ia])*(q[ib]-r3[ib]) - (F[ib]-r3[ib])*(q[ia]-r3[ia]);
    s32 n2 = (F[ib]-r3[ib])*(p[ia]-r3[ia]) - (F[ia]-r3[ia])*(p[ib]-r3[ib]);
    if (denom < 0)
    {
        n1 = -n1;
        denom = -denom;
        n2 = -n2;
    }

    if ((u32)n2 > (u32)denom)
        return 0;
    
    if ((u32)n1 > (u32)denom)
        return 0;
    
    if ((u32)(n1 + n2) > (u32)denom)
        return 0;
    return 1;
}

static int v6col_matgate(u32 face)
{
    u32 id = 0;
    u32 mat = v6col_mats + (u32)xread8(face + 0xb) * 0x20;
    if (xread8(mat + 0xa) == 0xff)
        return 0;
    
    if (xread32(mat) != 0)
    {
        u16 m16 = (u16)xread16(mat + 0x16);
        if (m16 != 0 && (alis.v6tstmask & m16) == 0)
            return 0;
        
        id = (u32)xread32(mat + 0x14);
        if (id == 0)
        {
            u32 ext = mat + xread32(mat);
            u16 e16 = (u16)xread16(ext + 0xa);
            if (e16 != 0 && (alis.v6tstmask & e16) == 0)
                return 0;
            
            id = (u32)xread32(ext + 0x8);
        }
    }

    v6col_matout = id;
    return 1;
}

// sphere vs face
static int v6col_face(u32 face)
{
    u16 flags = (u16)xread16(face + 0xc);
    u32 vb = v6col_verts;
    if (flags & 0x200)
    {
        u32 vA = vb + 12u * (u16)xread16(face + 0);
        u32 vB = vb + 12u * (u16)xread16(face + 2);
        s32 dx = ((s32)(s16)xread16(vA) - (s16)xread16(vB)) >> 2;
        s32 half = dx + (dx >> 1);
        s32 mx = (((s32)(s16)xread16(vA + 0) + (s16)xread16(vB + 0)) >> 1) - v6col_pc[0];
        s32 my = (((s32)(s16)xread16(vA + 2) + (s16)xread16(vB + 2)) >> 1) - v6col_pc[1];
        u32 d2xy = (u32)(my * my) + (u32)(mx * mx);
        s32 lim = (v6col_r7 >> 7) + half;
        if (d2xy > (u32)(lim * lim))
            return 0;
        
        s32 z01 = ((((s32)(s16)xread16(vA + 4) + (s16)xread16(vB + 4)) >> 1) - v6col_pc[2]) << 7;
        if (z01 > v6col_r7)
            return 0;
        
        u32 vC = vb + 12u * (u16)xread16(face + 4);
        u32 vD = vb + 12u * (u16)xread16(face + 6);
        s32 z23 = -(((((s32)(s16)xread16(vC + 4) + (s16)xread16(vD + 4)) >> 1) - v6col_pc[2])) << 7;
        if (z23 > v6col_r7)
            return 0;
        
        return v6col_matgate(face);
    }

    u32 v0 = vb + 12u * (u16)xread16(face + 0);
    u32 reach = (u32)((u32)xread8(v0 + 7) << 6) + (u32)v6col_r;
    u32 d2 = 0;
    for (int i = 0; i < 3; i++)
    {
        s32 d = (s16)xread16(v0 + (u32)i * 2) - v6col_pc[i];
        if ((u32)(d < 0 ? -d : d) > reach)
            return 0;
        
        d2 += (u32)(d * d);
    }

    int inside = 0;
    if (d2 <= (u32)v6col_r2)
        inside = 1;
    
    if (!inside)
    {
        u32 R2 = (u32)((u32)xread8(v0 + 7) << 6) * (u32)((u32)xread8(v0 + 7) << 6);
        if ((s32)(d2 - R2) >= v6col_r2)
            return 0;
        
        int nv = (flags & 0x8000) ? 3 : 2;
        for (int k = 1; k <= nv && !inside; k++)
        {
            u32 vk = vb + 12u * (u16)xread16(face + (u32)k * 2);
            u32 dk = 0;
            for (int i = 0; i < 3; i++)
            {
                s32 d = (s16)xread16(vk + (u32)i * 2) - v6col_pc[i];
                dk += (u32)(d * d);
            }

            if (dk <= (u32)v6col_r2)
                inside = 1;
        }
    }

    if (!inside)
    {
        s32 dot = (s8)xread8(face + 8)  * ((s32)(s16)xread16(v0 + 0) - v6col_pc[0])
                + (s8)xread8(face + 9)  * ((s32)(s16)xread16(v0 + 2) - v6col_pc[1])
                + (s8)xread8(face + 10) * ((s32)(s16)xread16(v0 + 4) - v6col_pc[2]);
        s32 ad = dot < 0 ? -dot : dot;
        if ((u32)ad > (u32)v6col_r7)
            return 0;
        
        s32 F[3];
        F[0] = (((s32)(s8)xread8(face + 8)  * dot) >> 14) + v6col_pc[0];
        F[1] = (((s32)(s8)xread8(face + 9)  * dot) >> 14) + v6col_pc[1];
        F[2] = (((s32)(s8)xread8(face + 10) * dot) >> 14) + v6col_pc[2];
        if (!v6col_intri(face, 0, F))
        {
            if (!(flags & 0x8000))
                return 0;
            if (!v6col_intri(face, 1, F))
                return 0;
        }
    }

    return v6col_matgate(face);
}

static int v6col_gridwalk(u32 model, u16 prad)
{
    v6col_facesA = model + xread32(model + 0x3c);
    v6col_verts  = model + xread32(model + 0x38);
    v6col_mats   = model + xread32(model + 0x44);
    s32 r = (s32)prad;
    s32 lo[3], hi[3];
    static const u8 orgo[3] = {0x50, 0x58, 0x60}, maxo[3] = {0x54, 0x5c, 0x64};
    static const u8 sho[3]  = {0x6d, 0x6e, 0x6f}, dimo[3] = {0x70, 0x74, 0x78};
    for (int a = 0; a < 3; a++)
    {
        s32 org = xread32(model + orgo[a]);
        s32 mx  = xread32(model + maxo[a]);
        u8  sh  = xread8(model + sho[a]);
        s32 dim = xread32(model + dimo[a]);
        s32 c   = v6col_pc[a];
        if (c - r > mx)
            return 0;
        
        lo[a] = (c - r - org) >> sh;
        if (lo[a] < 1)
            lo[a] = 0;
        
        if (c + r < org)
            return 0;
        
        hi[a] = (c + r - org) >> sh;
        if (hi[a] >= dim)
            hi[a] = dim - 1;
    }
    
    {
        u32 nfb = (((u32)(u16)((u16)xread16(model + 0x2e) + (u16)xread16(model + 0x30)) >> 5) + 1) * 4;
        if (nfb > sizeof(v6col_fbits))
            nfb = sizeof(v6col_fbits);
        memset(v6col_fbits, 0, nfb);
    }
    
    u32 base  = model + xread32(model + 0x68);
    u32 dim0  = (u32)xread32(model + 0x70);
    u32 dim1  = (u32)xread32(model + 0x74);
    u32 plane = dim1 * dim0 * 4;
    u32 rowst = dim0 * 4;
    for (s32 z = lo[2]; z <= hi[2]; z++)
    {
        for (s32 y = lo[1]; y <= hi[1]; y++)
        {
            u32 cp = base + plane * (u32)z + rowst * (u32)y + (u32)lo[0] * 4;
            for (s32 x = lo[0]; x <= hi[0]; x++, cp += 4)
            {
                u32 off = xread32(cp);
                if (off == 0)
                    continue;
                
                u32 fl = model + off;
                u16 cnt = (u16)xread16(fl);
                fl += 2;
                for (u16 i = 0; i < cnt; i++, fl += 2)
                {
                    u16 idx = (u16)xread16(fl);
                    if (idx == 0xffff)
                        continue;

                    if ((u32)(idx >> 3) >= sizeof(v6col_fbits))
                        continue;

                    if (v6col_fbits[idx >> 3] & (1u << (idx & 7)))
                        continue;

                    v6col_fbits[idx >> 3] |= (u8)(1u << (idx & 7));
                    if (v6col_face(v6col_facesA + (u32)idx * 0x18))
                        return 1;
                }
            }
        }
    }
    
    return 0;
}

static int v6col_test_mesh(u32 chain, u16 prad, s16 pcx, s16 pcy, s16 pcz, u32 cand, u32 *matout)
{
    *matout = 0;
    if (xread8(cand + 0x23) & 0x70)
    {
        if (!v6x3d_cull_vs(v6col_pw, cand))
            return 0;
    }
    
    sV6Ctx c;
    v6tf_build(chain, cand, &c);
    v6col_pc[0] = pcx; v6col_pc[1] = pcy; v6col_pc[2] = pcz;
    v6tf_apply(v6col_pc, &c, v6col_pc);
    s32 d2 = 0;
    for (int i = 0; i < 3; i++)
    {
        s32 d = (s16)xread16(cand + 0x42 + i * 2) - v6col_pc[i];
        d2 += d * d;
    }
    
    s32 rsum = (s32)((u16)xread16(cand + 0x40) + (u32)prad);
    if (rsum * rsum < d2)
        return 0;
    
    if (xread8(cand + 0x25) & 4)
        return 0;
    
    v6col_r = prad;
    v6col_r7 = (s32)prad << 7;
    v6col_r2 = (s32)prad * prad;
    u32 res = xread32(cand + 0x3c);
    res += xread32(res);
    int hit = 0;
    switch (xread8(res + 0x6c))
    {
        case 0:
        {
            v6col_verts = res + xread32(res + 0x38);
            v6col_mats  = res + xread32(res + 0x44);
            u16 cnt = (u16)((u16)xread16(res + 0x2e) + (u16)xread16(res + 0x30));
            u32 face = res + xread32(res + 0x3c);
            for (u16 i = 0; i < cnt && !hit; i++, face += 0x18)
                hit = v6col_face(face);
            break;
        }
            
        case 1:
        {
            hit = v6col_gridwalk(res, prad);
            break;
        }
            
        default:
        {
            static int warned = 0;
            if (!warned++)
                ALIS_DEBUG(EDebugWarning, "v6col: portal-cell walk (FUN_00010854/104a9) not yet ported — no portal-mesh collision\n");
            break;
        }
    }
    
    if (!hit)
        return 0;
    
    *matout = v6col_matout;
    return 1;
}

// sphere-probe world scan
void v6col_sphere_scan(u32 chain, u16 srad, s16 scx, s16 scy, s16 scz, u16 selfid, u16 scroff)
{
    v6tf_cacheon = 0;
    v6col_probe_world(chain, srad, scx, scy, scz);
    u32 e = alis.basemain + scroff;
    for (;;)
    {
        e = xread32(e + 0x38);
        if (e == 0 || e == xread32(e + 0x38))
            break;
        
        if ((alis.v6tstmask & (u16)xread16(e + 0x28)) == 0)
            continue;
        
        if (xread8(e + 0x24) & 0x40)
            continue;
        
        if (xread8(e + 0x25) & 0x04)
            continue;
        
        if ((u16)xread16(e + 0x2a) == selfid)
            continue;
        
        u8 t = xread8(e + 0x22);
        u32 mat;
        if (t == 0x33)
        {
            if (!v6col_test_sphere(e, srad))
                continue;
            
            mat = (u32)xread32(e + 0x26);
        }
        else if (t == 0x30)
        {
            u32 fm = 0;
            if (!v6col_test_mesh(chain, srad, scx, scy, scz, e, &fm))
                continue;
            
            mat = fm ? fm : (u32)xread32(e + 0x26);
        }
        else
        {
            continue;
        }

        if (v6ts_push((u16)xread16(e + 0x2a), mat)) // a112-a139
            break;
    }
}

static u32 v6x3d_slotinsert(s32 depth)
{
    u32 off = (u32)(depth & ~3);
    u32 rec;
    if (v6x3d_arenacur >= v6x3d_arenalim)
    {
        static int warned = 0;
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "v6x3d: face arena overflow (perror 0xa) — reusing last record\n");
        rec = v6x3d_arenacur - 0x50;
    }
    else
    {
        rec = v6x3d_arenacur;
        v6x3d_arenacur += 0x50;
    }

    xwrite32(rec, v6x3d_slothead[off >> 2]);
    v6x3d_slothead[off >> 2] = rec;
    v6x3d_slotdirty[off >> 6] = 1;
    v6x3d_cntfaces++;
    return rec;
}

static void v6x3d_lights_build(u32 s)
{
    v6x3d_nlights = 0;
    v6x3dfog.bands = (u16)xread16(s + 0x96);
    v6x3dfog.bandshift = v6x3dfog.bands <= 0x10 ? 4 : 5;
    v6x3dfog.bandclamp = (u16)xread16(s + 0xae);
    v6x3dfog.mode = (u16)xread16(s + 0x94);
    v6x3dfog.ambbase = xread32(s + 0x146);
    v6x3dfog.fogslope = xread32(s + 0xa6);
    v6x3dfog.attslope = xread32(s + 0xaa);
    v6x3dfog.fogstart = xread32(s + 0x146) >> 2;
    v6x3dfog.baselevel = xread32(s + 0xa2);

    u32 head = xread32(s + 0x5e);
    if (head == 0)
        return;
    
    for (u32 e = xread32(head + 0x38); e != 0 && e != xread32(e + 0x38); e = xread32(e + 0x38))
    {
        if (xread8(e + 0x22) != 0x10)
            continue;
        
        if ((xread8(e + 0x23) & 0x70) && !v6x3d_cull(e))
            continue;
        
        sV6Light *rec = &v6x3d_light[v6x3d_nlights];
        v6tf_build(e, s, &rec->ctx);
        s32 c[3];
        c[0] = (s16)xread16(e + 0x42);
        c[1] = (s16)xread16(e + 0x44);
        c[2] = (s16)xread16(e + 0x46);
        v6tf_apply(c, &rec->ctx, c);
        if ((u16)xread16(e + 0x40) != 0xffff)
        {
            s64 dx = (s16)xread16(s + 0x42) - c[0];
            s64 dy = (s16)xread16(s + 0x44) - c[1];
            s64 dz = (s16)xread16(s + 0x46) - c[2];
            s64 d2 = dx*dx + dy*dy + dz*dz;
            if ((u64)d2 >> 32)
                continue;
            
            u32 lim = (u32)((u16)xread16(s + 0x40) + (u16)xread16(e + 0x40));
            lim = lim * lim;
            if ((s32)(u32)d2 > (s32)lim)
                continue;
        }

        rec->elem = e;
        rec->type = xread8(e + 0x22);
        rec->kind = 0;
        rec->range = (u16)xread16(e + 0x40);
        if (xread8(e + 0x56) == 1)
        {
            v6tf_build(e, 0, &v6tf_camobj);
            rec->pos[0] = (s16)xread16(e + 0x42);
            rec->pos[1] = (s16)xread16(e + 0x44);
            rec->pos[2] = (s16)xread16(e + 0x46);
            v6tf_apply(rec->pos, &v6tf_camobj, rec->pos);
            s32 m = rec->pos[0] < 0 ? -rec->pos[0] : rec->pos[0];
            s32 t = rec->pos[1] < 0 ? -rec->pos[1] : rec->pos[1];
            if (t > m)
                m = t;
            
            t = rec->pos[2] < 0 ? -rec->pos[2] : rec->pos[2];
            if (t > m)
                m = t;
            
            if (m == 0)
                m = 1;
            
            m = -m;
            rec->dir[0] = (rec->pos[0] << 7) / m;
            rec->dir[1] = (rec->pos[1] << 7) / m;
            rec->dir[2] = (rec->pos[2] << 7) / m;
            rec->slope = 0;
            if (v6x3dfog.mode != 0)
                rec->slope = v6x3dfog.ambbase >> 1;
            
            rec->kind = 1;
            rec->spot = 0;
            rec->intensity = (s16)xread16(e + 0x48) * v6x3dfog.bands;
            v6tf_build(0, s, &rec->ctx);
            s32 zero[3] = {0, 0, 0}, t0[3];
            v6tf_apply(zero, &rec->ctx, t0);
            v6tf_apply(rec->dir, &rec->ctx, rec->camdir);
            rec->camdir[0] -= t0[0];
            rec->camdir[1] -= t0[1];
            rec->camdir[2] -= t0[2];
        }
        else
        {
            rec->pos[0] = (s16)xread16(e + 0x42);
            rec->pos[1] = (s16)xread16(e + 0x44);
            rec->pos[2] = (s16)xread16(e + 0x46);
            rec->dir[0] = (s8)xread8(e + 0x50);
            rec->dir[1] = (s8)xread8(e + 0x51);
            rec->dir[2] = (s8)xread8(e + 0x52);
            rec->spot = 0;
            u32 cone = xread8(e + 0x53);
            if (cone < 0x7f)
            {
                rec->spot = 1;
                rec->cone = (s32)(s16)v6costab[(cone >> 1) << 4] >> 8;
            }

            rec->intensity = (s16)xread16(e + 0x48) * v6x3dfog.bands;
            rec->fstart = rec->range;
            u32 fs = (u16)xread16(e + 0x54);
            if (fs < 0xc8)
                fs = 0xc8;
            
            if ((s32)fs < (s32)rec->range)
            {
                rec->fstart = fs;
                s32 span = (s32)rec->range - (s32)fs;
                rec->int8fp = rec->intensity << 8;
                rec->slope = -(rec->int8fp / span);
            }

            v6tf_apply(rec->pos, &rec->ctx, rec->campos);
            if (rec->spot)
                v6tf_apply(rec->dir, &rec->ctx, rec->camdir);
        }

        if (++v6x3d_nlights >= 0x40)
            break;
    }
}

static void v6x3d_framereset(u32 s)
{
    v6x3d_cntfaces = v6x3d_cntverts = 0;
    u32 h = xread32(s + 0xbc);
    if (h != 0)
    {
        u32 a = xread32(h);
        v6x3d_arena = a;
        v6x3d_arenalim = a + 0x1d4c0;
        v6x3d_shl16 = a + 0x1d4c0;
    }
    else
    {
        v6x3d_arena = 0;
    }

    for (int g = 0; g < 0x200; g++)
    {
        if (v6x3d_slotdirty[g])
        {
            memset(&v6x3d_slothead[g * 16], 0, 16 * sizeof(u32));
            v6x3d_slotdirty[g] = 0;
        }
    }

    v6x3d_arenacur = v6x3d_arena;
    v6tf_frame_setup(s);
    v6x3d_lights_build(s);
}

static u32 v6x3d_fogbase;
static u8  v6x3d_bands;
static u32 v6x3d_ae98;          // render-flags
static u32 v6x3d_ae9c;          // per-Y emit index
static u8  v6x3d_ae5f;          // u/v doubled
static u32 v6x3d_res;           // current texture resource
static u8  v6x3d_texw;
static u8  v6x3d_texw8;
static u8  v6x3d_texh;

typedef struct {
    u32 lower;
    u8  fresh;
    s32 rows;
    s32 x, dx;
    u16 v, u;
    s16 dv, du;
    s32 v15, u15, z15;
    s32 dv4, du4, dz4;
    s32 i;
    s32 c800r;
    s32 rdy;
    u16 band;
    s16 dband;
} sV6RastEdge;

typedef struct {
    s32 y;
    s32 n;
    int cur;
    int first;
    int end;
    sV6RastEdge e[2];
} sV6RastWS;
static sV6RastWS v6x3d_ws;

// face-record vertex-node copy accessors
static inline s16 v6nd_x(u32 n) { return (s16)xread16(n + 0); }
static inline u16 v6nd_d(u32 n) { return (u16)xread16(n + 2); }
static inline s16 v6nd_y(u32 n) { return (s16)xread16(n + 4); }

// write the -1 X markers into a row
static void v6x3d_rowterm(int ri)
{
    if (ri >= 0 && ri < 500)
    {
        v6x3d_rows[ri].lx = -1;
        v6x3d_rows[ri].rx = -1;
    }
}

static int v6x3d_edgebuild(int side, u32 na, u32 nb)
{
    sV6RastEdge *E = &v6x3d_ws.e[side];
    s16 dy = (s16)(v6nd_y(nb) - v6nd_y(na));
    if (dy < 0)
    {
        u32 tn = na; na = nb; nb = tn;
        dy = (s16)-dy;
    }

    E->lower = nb;
    E->rows = dy;
    if (dy == 0)
        return 1;
    
    E->fresh = 1;
    s16 xa = v6nd_x(na);
    E->x = (s32)xa << 16;
    E->dx = (((s32)(v6nd_x(nb) - xa)) << 16) / dy;
    if (!(v6x3d_ae98 & 0x80))
    {
        u8 ua = xread8(na + 6), va = xread8(na + 7);
        E->u  = (u16)((u16)ua << 8);
        E->du = (s16)((((s16)xread8(nb + 6) - ua) << 8) / dy);
        E->v  = (u16)((u16)va << 8);
        E->dv = (s16)((((s16)xread8(nb + 7) - va) << 8) / dy);
    }
    else
    {
        E->i = 0;
        s16 uu = xread8(na + 6), du = (s16)(xread8(nb + 6) - uu);
        s16 vv = xread8(na + 7), dv = (s16)(xread8(nb + 7) - vv);
        if (v6x3d_ae5f)
        {
            uu = (s16)(uu * 2); vv = (s16)(vv * 2);
            du = (s16)(du * 2); dv = (s16)(dv * 2);
        }

        E->u15 = (s32)uu << 15;  E->du4 = (s32)du << 4;
        E->v15 = (s32)vv << 15;  E->dv4 = (s32)dv << 4;
        u16 za = v6nd_d(na), zb = v6nd_d(nb);
        E->z15 = (s32)za << 15;
        E->dz4 = ((s32)zb - za) << 4;
        u32 zdiv = za ? za : 1;
        u32 r = ((u32)zb << 11) / zdiv;
        E->rdy   = (s32)(r * (u32)dy);
        E->c800r = (s32)(0x800 - r);
    }

    u8 ba = xread8(na + 0xb);
    s16 db = (s16)((((s16)xread8(nb + 0xb) - ba) << 8) / dy);
    if (db < 0)
        db += 2;
    
    db &= (s16)~1;
    E->band  = (u16)((u16)ba << 8);
    E->dband = db;
    return 0;
}

static int v6x3d_segment(void)
{
    sV6RastWS *w = &v6x3d_ws;
    s32 n = w->e[0].rows < w->e[1].rows ? w->e[0].rows : w->e[1].rows;
    w->e[0].rows -= n;
    w->e[1].rows -= n;
    if (n <= 0)
        return 1;
    
    w->n = n;
    s32 y = w->y;
    if ((s16)y > v6xclipy2)
        return 1;
    
    s32 skip;
    if ((s16)(y + n) >= v6xclipy1)
    {
        s32 over = (y + n) - v6xclipy2;
        if (over > 0)
            w->n -= (over - 1);
        
        skip = v6xclipy1 - y;
        if (skip <= 0)
            goto rows;
    }
    else
    {
        skip = n;
    }

    for (int s = 0; s < 2; s++)
    {
        sV6RastEdge *E = &w->e[s];
        E->x += E->dx * skip;
        E->band = (u16)(E->band + E->dband * skip);
        if (!(v6x3d_ae98 & 0x80))
        {
            E->u = (u16)(E->u + E->du * skip);
            E->v = (u16)(E->v + E->dv * skip);
        }
        else
        {
            E->i += skip;
        }
    }

    if ((s16)(w->y + w->n) < v6xclipy1)
        return 1;
    
    w->n -= skip;
    w->y += skip;
    
rows:
    
    w->cur = w->y;
    w->end = w->y + w->n;
    if (w->first == -1)
        w->first = w->cur;
    
    return 0;
}

static void v6x3d_orderfix(void)
{
    sV6RastWS *w = &v6x3d_ws;
    s32 n = w->n;
    int doswap = 0;
    if (w->e[1].x < w->e[0].x)
    {
        doswap = 1;
    }
    else if (w->e[1].x == w->e[0].x && (w->e[1].x + w->e[1].dx * n) < (w->e[0].x + w->e[0].dx * n))
    {
        doswap = 1;
    }

    if (doswap)
    {
        sV6RastEdge t = w->e[0];
        w->e[0] = w->e[1];
        w->e[1] = t;
    }

    if (v6x3d_ae98 & 0x10)
        return;
    
    for (int s = 0; s < 2; s++)
    {
        sV6RastEdge *E = &w->e[s];
        if (!E->fresh)
            continue;
        
        E->fresh = 0;
        E->x += 0x10000;

        int step = (s == 0) ? (E->dx < 0) : (E->dx >= 0);
        if (step)
        {
            E->x += E->dx;
            E->band = (u16)(E->band + E->dband);
            if (!(v6x3d_ae98 & 0x80))
            {
                E->u = (u16)(E->u + E->du);
                E->v = (u16)(E->v + E->dv);
            }
            else
            {
                E->i++;
            }
        }
    }
}

static void v6x3d_emit(void)
{
    sV6RastWS *w = &v6x3d_ws;
    u32 fl = v6x3d_ae98;
    int uvlin = (fl & 6) != 0 && !(fl & 0x80);
    int persp = (fl & 0x80) != 0;
    int ri = w->cur;
    if (ri < 0 || w->end > 499)
    {
        static int warned = 0;
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "v6x3d_emit: row range %d..%d out of bounds\n", ri, w->end);
        
        if (ri < 0)
            ri = 0;
        
        if (w->end > 499)
            w->end = 499;
    }

    for (; ri < w->end; ri++)
    {
        sV6Row *row = &v6x3d_rows[ri];
        row->lx = w->e[0].x;
        row->rx = w->e[1].x;
        row->lband = w->e[0].band;
        row->rband = w->e[1].band;
        if (uvlin)
        {
            row->lv = w->e[0].v;  row->lu = w->e[0].u;
            row->rv = w->e[1].v;  row->ru = w->e[1].u;
        }
        else if (persp)
        {
            for (int s = 0; s < 2; s++)
            {
                sV6RastEdge *E = &w->e[s];
                s64 den = (s64)E->i * E->c800r + E->rdy;
                if (den <= 0)
                    den = 1;
                s64 q = ((s64)E->i << 22) / den;
                if (q > 0x7fffffff)
                    q = 0x7fffffff;
                s32 t = (s32)q;
                s32 vv = (s32)((s64)E->v15 + (s64)t * E->dv4);
                s32 uu = (s32)((s64)E->u15 + (s64)t * E->du4);
                s32 zz = (s32)((s64)E->z15 + (s64)t * E->dz4);
                if (s == 0)
                {
                    row->lv15 = vv;
                    row->lu15 = uu;
                    row->lz15 = zz;
                }
                else
                {
                    row->rv15 = vv;
                    row->ru15 = uu;
                    row->rz15 = zz;
                }
            }
        }

        int bandstep = !(v6x3d_ae9c == 0 || v6x3d_ae9c == 2 || v6x3d_ae9c == 8);
        for (int s = 0; s < 2; s++)
        {
            sV6RastEdge *E = &w->e[s];
            E->x += E->dx;
            if (bandstep)
                E->band = (u16)(E->band + E->dband);
            
            if (uvlin)
            {
                E->u = (u16)(E->u + E->du);
                E->v = (u16)(E->v + E->dv);
            }

            if (persp)
                E->i++;
        }
    }

    v6x3d_rowterm(ri);
    w->cur = ri;
}

static void v6x3d_span_flat(int first, int end)
{
    u32 fl = v6x3d_ae98;
    u8 *fog = alis.mem + v6x3d_fogbase;
    u8 colour = xread8(v6x3d_res + 0x1c);
    u8 *mix = (fl & 0x10) ? fog - 0x10000 : NULL;
    for (int ri = first; ri < end; ri++)
    {
        sV6Row *row = &v6x3d_rows[ri];
        s16 lx = (s16)(row->lx >> 16);
        s16 rx = (s16)(row->rx >> 16);
        if (lx > v6xclipx2 || rx < v6xclipx1)
            continue;
        
        s32 len = rx - lx;
        if (len <= 0)
            continue;
        
        s16 dband = (s16)(((s32)(s16)row->rband - (s16)row->lband) / len);
        u16 band = row->lband;
        if (lx < v6xclipx1)
        {
            s32 dx = v6xclipx1 - lx;
            band = (u16)(band + dband * dx);
            lx = v6xclipx1;
        }

        if (rx >= v6xclipx2)
            rx = (s16)(v6xclipx2 + 1);
        
        len = rx - lx;
        if (len <= 0)
            continue;
        
        u8 c = colour;
        if (fl & 0x20)
        {
            lx = (s16)(lx & ~1);
            s32 units = (u16)(rx - lx + 1) >> 1;
            s16 si = (s16)(dband * 2);
            s32 pitch = alis.platform.width;
            u8 *dst = image.logic + row->rowoff * 2 + lx;
            for (s32 k = 0; k < units; k++, dst += 2)
            {
                u8 out = fog[((u32)band & 0xff00) | c];
                band = (u16)(band + si);
                if (mix)
                    out = mix[(dst[0] << 8) | out];
                
                dst[0] = dst[1] = dst[pitch] = dst[pitch + 1] = out;
            }

            continue;
        }

        if (!(fl & 4))
            c = fog[((u32)band & 0xff00) | c];

        u8 *dst = image.logic + row->rowoff + lx;
        if (fl & 0x40)
        {
            s32 units = len >> 1;
            s16 si = (s16)(dband * 2);
            u8 a = 0;
            for (s32 k = 0; k < units; k++, dst += 2)
            {
                u8 out = fog[((u32)band & 0xff00) | c];
                band = (u16)(band + si);
                a = mix[(dst[0] << 8) | out];
                dst[0] = mix[(a << 8) | dst[-1]];
                dst[1] = a;
            }

            if (len & 1)
                *dst = a;
            continue;
        }

        for (s32 nn = len; nn > 0; nn--)
        {
            u8 out = fog[((u32)band & 0xff00) | c];
            *dst = mix ? mix[(*dst << 8) | out] : out;
            dst++;
            band = (u16)(band + dband);
        }
    }
}

static void v6x3d_span_tex(int first, int end)
{
    u32 fl = v6x3d_ae98;
    int W = v6x3d_texw, H = v6x3d_texh;
    if (W < 1 || W > 8 || H < 1 || H > 8)
    {
        static int warned = 0;
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "v6x3d_span_tex: bad texture log2 dims W=%d H=%d (res %x)\n", W, H, v6x3d_res);
        return;
    }

    u8 *tex = alis.mem + v6x3d_res + 0x20;
    u8 *fog = alis.mem + v6x3d_fogbase;
    u8 *lut = fog - 0x10000;
    u8 *mix = (fl & 0x10) ? lut : NULL;
    int keyed = (fl & 1) == 0 && !mix;
    u32 vmaskw = ((u32)((1u << H) - 1)) << 8;
    u32 mask = 0xFFFE00FFu | vmaskw;
    for (int ri = first; ri < end; ri++)
    {
        sV6Row *row = &v6x3d_rows[ri];
        s16 lx = (s16)(row->lx >> 16);
        s16 rx = (s16)(row->rx >> 16);
        if (lx > v6xclipx2 || rx < v6xclipx1)
            continue;
        
        s32 len = rx - lx;
        if (len <= 0)
            continue;
        
        s32 dur = (s32)row->ru - (s32)row->lu;
        s32 dvr = (s32)row->rv - (s32)row->lv;
        if (v6x3d_ae5f)
        {
            dur *= 2;
            dvr *= 2;
        }

        s16 du = (s16)(dur / len);
        s16 dv = (s16)(dvr / len);
        s16 dband = 0;
        if (fl & 4)
        {
            s32 bd = (s16)row->rband - (s16)row->lband;
            s32 mag = (s32)(((u32)(bd < 0 ? -bd : bd) / (u32)len) & 0xfffe);
            dband = (s16)(bd < 0 ? -mag : mag);
        }

        u16 u0 = row->lu, v0 = row->lv;
        u16 band = row->lband;
        if (lx < v6xclipx1)
        {
            s32 dx = v6xclipx1 - lx;
            s16 duc = v6x3d_ae5f ? (s16)(du >> 1) : du;
            s16 dvc = v6x3d_ae5f ? (s16)(dv >> 1) : dv;
            u0 = (u16)(u0 + duc * dx);
            v0 = (u16)(v0 + dvc * dx);
            band = (u16)(band + dband * dx);
            lx = v6xclipx1;
        }

        if (rx >= v6xclipx2)
            rx = (s16)(v6xclipx2 + 1);
        
        len = rx - lx;
        if (len <= 0)
            continue;
        
        if (v6x3d_ae5f)
        {
            u0 = (u16)(u0 << 1);
            v0 = (u16)(v0 << 1);
        }

        int blk = (fl & 0x20) ? 1 : 0;
        int half = !blk && (fl & 0x40);
        int interp = fl & 4;
        s32 units = len;
        u8 *dst;
        if (blk)
        {
            lx = (s16)(lx & ~1);
            units = ((s32)rx - lx + (interp ? 1 : 0)) >> 1;
            du = (s16)(du * 2); dv = (s16)(dv * 2); dband = (s16)(dband * 2);
            dst = image.logic + row->rowoff * 2 + lx;
        }
        else if (half)
        {
            units = len >> 1;
            if (units == 0)
                continue;

            du = (s16)(du * 2); dv = (s16)(dv * 2); dband = (s16)(dband * 2);
            dst = image.logic + row->rowoff + lx;
        }
        else
        {
            dst = image.logic + row->rowoff + lx;
        }

        s32 pitch = alis.platform.width;
        u16 uS  = (u16)(u0 << (8 - W));
        u16 duS = (u16)(du << (8 - W));
        u32 uvb  = (((u32)band << 16) | v0) & mask;
        u32 step = ((u32)(u16)dband << 16) | (u16)dv;
        u8 bconst = (u8)(band >> 8);
        u8 al = 0, ah = 0;
        for (s32 nn = units; nn > 0; nn--)
        {
            u32 idx = (((uvb >> 8) & 0xff) << W) | ((u32)uS >> (16 - W));
            u8 texel = tex[idx];
            u8 bi = interp ? (u8)(uvb >> 24) : bconst;
            u8 out = fog[((u32)bi << 8) | texel];
            if (half)
            {
                if (mix)
                {
                    u8 a = mix[(dst[0] << 8) | out];
                    dst[0] = mix[(a << 8) | dst[-1]];
                    dst[1] = a;
                    ah = a;
                }
                else if (interp && keyed)
                {
                    if (texel)
                    {
                        al = lut[(out << 8) | dst[-1]];
                        dst[2] = al; dst[3] = out;
                        ah = out;
                    }
                    else
                    {
                        al = (u8)idx; ah = (u8)(idx >> 8);
                    }
                }
                else if (!keyed || out)
                {
                    dst[0] = lut[(out << 8) | dst[-1]];
                    dst[1] = out;
                    ah = out;
                }

                dst += 2;
            }
            else if (blk)
            {
                if (mix)
                {
                    u8 m = mix[(dst[interp ? 1 : 0] << 8) | out];
                    dst[0] = dst[1] = dst[pitch] = dst[pitch + 1] = m;
                }
                else if (!keyed || texel != 0)
                {
                    dst[0] = out; dst[1] = out;
                    dst[pitch] = out; dst[pitch + 1] = out;
                }

                dst += 2;
            }
            else
            {
                if (!keyed || texel != 0)
                    *dst = mix ? mix[(*dst << 8) | out] : out;
                
                dst++;
            }

            uvb += step;
            uS = (u16)(uS + duS);
            uvb &= mask;
        }

        if (half && (len & 1))
        {
            if (!interp)
            {
                u32 idxn = (((uvb >> 8) & 0xff) << W) | ((u32)uS >> (16 - W));
                al = (u8)idxn; ah = (u8)(idxn >> 8);
            }

            u8 test;
            if (mix)
            {
                test = interp ? ah : al;
            }
            else if (keyed)
            {
                test = al;
            }
            else
            {
                test = 1;
            }

            if (test)
                *dst = ah;
        }
    }
}

static s32 v6p_at(s64 aL, s64 aR, s32 d, s32 len, s64 zL, s64 zR)
{
    s64 den = (s64)(len - d) * zR + (s64)d * zL;
    if (den <= 0)
        den = 1;
    
    return (s32)(aL + (s64)d * zL * (aR - aL) / den);
}

static s16 v6p_step(s32 delta, s32 cw)
{
    if (cw == 16)
        return (s16)(delta >> 4);

    u16 mag = (u16)(delta < 0 ? -delta : delta);
    return (s16)(delta < 0 ? -(s32)(mag / (u16)cw) : (s32)(mag / (u16)cw));
}

static u16 v6p_ustep[64];

static void v6x3d_span_persp(int first, int end)
{
    u32 fl = v6x3d_ae98;
    int W = v6x3d_texw, H = v6x3d_texh;
    if (W < 1 || W > 8 || H < 1 || H > 8)
    {
        static int warned = 0;
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "v6x3d_span_persp: bad texture log2 dims W=%d H=%d (res %x)\n", W, H, v6x3d_res);
        return;
    }

    u8 *tex = alis.mem + v6x3d_res + 0x20;
    u8 *fog = alis.mem + v6x3d_fogbase;
    u8 *lut = fog - 0x10000;
    u8 *mix = (fl & 0x10) ? lut : NULL;
    int keyed = (fl & 1) == 0 && !mix;
    int blk = (fl & 0x20) != 0;
    int half = !blk && (fl & 0x40);
    u32 vmaskw = ((u32)((1u << H) - 1)) << 8;
    u32 mask = 0xFFFE00FFu | vmaskw;
    s32 pitch = alis.platform.width;
    for (int ri = first; ri < end; ri++)
    {
        sV6Row *row = &v6x3d_rows[ri];
        s16 rawlx = (s16)(row->lx >> 16);
        s16 rawrx = (s16)(row->rx >> 16);
        if (rawlx > v6xclipx2 || rawrx < v6xclipx1)
            continue;
        
        s32 len = rawrx - rawlx;
        if (len <= 0)
            continue;
        
        s32 bd = (s16)row->rband - (s16)row->lband;
        s32 mag = (s32)(((u32)(bd < 0 ? -bd : bd) / (u32)len) & 0xfffe);
        s16 dband = (s16)(bd < 0 ? -mag : mag);
        u16 band = row->lband;
        s16 lx = rawlx, rx = rawrx;
        if (lx < v6xclipx1)
        {
            band = (u16)(band + dband * (v6xclipx1 - lx));
            lx = v6xclipx1;
        }

        if (rx > v6xclipx2)
            rx = (s16)(v6xclipx2 + 1);
        
        s32 count = rx - lx;
        if (count <= 0)
            continue;
        
        s32 units;
        u8 *dst;
        if (blk)
        {
            lx = (s16)(lx & ~1);
            units = (u16)(rx - lx + 1) >> 1;
            dst = image.logic + row->rowoff * 2 + lx;
        }
        else if (half)
        {
            units = count >> 1;
            if (units == 0)
                continue;
            
            dst = image.logic + row->rowoff + lx;
        }
        else
        {
            units = count;
            dst = image.logic + row->rowoff + lx;
        }

        if (blk || half)
            dband = (s16)(dband * 2);

        s32 done, lenE;
        if (blk || half)
        {
            s32 l2 = rawlx & ~1;
            done = (s32)((u32)(lx - l2) >> 1);
            lenE = (s32)((u32)(rawrx - l2) >> 1);
        }
        else
        {
            done = lx - rawlx;
            lenE = len;
        }

        if (lenE == done)
            lenE = done + 1;

        s64 uL = (u32)row->lu15 >> 7, uR = (u32)row->ru15 >> 7;
        s64 vL = (u32)row->lv15 >> 7, vR = (u32)row->rv15 >> 7;
        s64 zL = (s32)(row->lz15 >> 8), zR = (s32)(row->rz15 >> 8);
        s32 u0 = v6p_at(uL, uR, done, lenE, zL, zR);
        s32 v0 = v6p_at(vL, vR, done, lenE, zL, zR);
        s32 chunk = 0;
        for (s32 left = units; left > 0; chunk++)
        {
            s32 cw = left & 0xf;
            if (cw == 0)
                cw = 16;
            
            s32 u1 = v6p_at(uL, uR, done + cw, lenE, zL, zR);
            s32 v1 = v6p_at(vL, vR, done + cw, lenE, zL, zR);
            s16 du = v6p_step(u1 - u0, cw);
            s16 dv = v6p_step(v1 - v0, cw);
            if (chunk < 64)
                v6p_ustep[chunk] = (u16)du;
            
            u16 uS  = (u16)((u16)u0 << (8 - W));
            u16 duS = (u16)(du << (8 - W));
            u32 uvb  = (((u32)band << 16) | (u16)v0) & mask;
            u32 step = ((u32)(u16)dband << 16) | (u16)dv;
            for (s32 nn = cw; nn > 0; nn--)
            {
                u32 idx = (((uvb >> 8) & 0xff) << W) | ((u32)uS >> (16 - W));
                u8 texel = tex[idx];
                u8 out = fog[((u32)(uvb >> 24) << 8) | texel];
                if (half)
                {
                    if (mix)
                    {
                        u8 m1 = mix[(dst[0] << 8) | out];
                        dst[0] = mix[(m1 << 8) | dst[-1]];
                        dst[1] = m1;
                    }
                    else if (keyed)
                    {
                        if (texel)
                        {
                            dst[2] = lut[(out << 8) | dst[-1]];
                            dst[3] = out;
                        }
                    }
                    else
                    {
                        dst[0] = lut[(out << 8) | dst[-1]];
                        dst[1] = out;
                    }

                    dst += 2;
                }
                else if (blk)
                {
                    if (mix)
                    {
                        u8 m = mix[(dst[1] << 8) | out];
                        dst[0] = dst[1] = dst[pitch] = dst[pitch + 1] = m;
                    }
                    else if (!keyed || texel != 0)
                    {
                        dst[0] = out; dst[1] = out;
                        dst[pitch] = out; dst[pitch + 1] = out;
                    }

                    dst += 2;
                }
                else
                {
                    if (!keyed || texel != 0)
                        *dst = mix ? mix[(*dst << 8) | out] : out;
                    dst++;
                }

                uvb += step;
                uS = (u16)(uS + duS);
                uvb &= mask;
            }

            band = (u16)(uvb >> 16);
            u0 = u1; v0 = v1;
            done += cw;
            left -= cw;
        }

        if (half && (count & 1))
        {
            u16 ax = chunk < 64 ? (u16)(v6p_ustep[chunk] << (8 - W)) : 0;
            u8 al = (u8)ax, ah = (u8)(ax >> 8);
            u8 test = mix ? ah : keyed ? al : 1;
            if (test)
                *dst = ah;
        }
    }
}

// span drawer dispatch
static void v6x3d_spans(int first, int end)
{
    u32 fl = v6x3d_ae98;
    if (v6x3d_pick)
    {
        v6x3d_pickhit = 0;
        s32 pr = v6x3d_picky;
        if (fl & 0x20)
            pr >>= 1;
        
        if (pr < first || pr > end)
            return;
        
        sV6Row *w = &v6x3d_rows[pr];
        s32 lx = (s16)(w->lx >> 16);
        s32 rx = (s16)(w->rx >> 16);
        if (lx == rx)
            return;
        
        s32 xmin = lx, xmax = rx;
        if (xmin > xmax)
        {
            s32 t = xmin;
            xmin = xmax;
            xmax = t;
        }

        s32 px = v6x3d_pickx;
        if (xmin > px || xmax < px)
            return;
        
        if ((fl & 1) || !(fl & 2))
        {
            v6x3d_pickhit = 1;
            return;
        }

        s32 len = xmax - xmin;
        s32 frac = ((px - xmin) << 8) / len;        // IDIV (len > 0)
        s32 uu = (((((s32)w->ru - w->lu) * frac) >> 8) + w->lu) >> 8;
        s32 vv = (((((s32)w->rv - w->lv) * frac) >> 8) + w->lv) >> 8;
        u32 idx = ((u32)vv << v6x3d_texw) + (u32)uu;
        v6x3d_pickhit = xread8(v6x3d_res + 0x20 + idx) != 0;
        return;
    }

    if (v6x3d_fogbase == 0)
    {
        static int warned = 0;
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "v6x3d_spans: no SHAD fog LUT (screen+0xb4) — spans dropped\n");
        return;
    }

    if (fl & 0x08)
    {
        static int warned = 0;
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "v6x3d_spans: bit3 reserved bank reached (flags %02x) — dropped\n", fl);
        return;
    }

    if ((fl & 6) == 0 && (fl & 0x70) == 0x40)
    {
        static u8 warned[256];
        if (!warned[fl & 0xff]++)
            ALIS_DEBUG(EDebugWarning, "v6x3d_spans: flat pair bank (flags %02x) is unreachable in DOS — spans dropped\n", fl);
        return;
    }

    if ((fl & 2) == 0)
    {
        v6x3d_span_flat(first, end);
    }
    else if (fl & 0x80)
    {
        v6x3d_span_persp(first, end);
    }
    else
    {
        v6x3d_span_tex(first, end);
    }
}

static void v6x3d_tridrv(u32 rec)
{
    sV6RastWS *w = &v6x3d_ws;
    u32 A = rec + 0x08, B = rec + 0x14, C = rec + 0x20;
    w->y = v6nd_y(A);
    w->n = 0;
    w->first = -1;
    w->cur = 0;
    w->end = 0;
    v6x3d_edgebuild(0, A, B);
    v6x3d_edgebuild(1, A, C);
    if (!v6x3d_segment())
    {
        v6x3d_orderfix();
        v6x3d_emit();
    }

    w->y += w->n;
    if ((s16)w->y <= v6xclipy2)
    {
        if (w->e[0].rows > w->e[1].rows)
        {
            v6x3d_edgebuild(1, C, B);
        }
        else
        {
            v6x3d_edgebuild(0, B, C);
        }

        if (!v6x3d_segment())
        {
            v6x3d_orderfix();
            v6x3d_emit();
        }
    }

    if (w->first != -1 && w->first < w->end)
        v6x3d_spans(w->first, w->end);
}

// QUAD driver
static void v6x3d_quaddrv(u32 rec)
{
    sV6RastWS *w = &v6x3d_ws;
    u32 A = rec + 0x08, B = rec + 0x14, C = rec + 0x20, D = rec + 0x2c;
    u32 ebx = B, ecx = C, edx = D;
    if (!(v6nd_y(ebx) > v6nd_y(edx)))
    {
        u32 t = ebx;
        ebx = edx;
        edx = t;
    }
    
    if (!(v6nd_y(ebx) > v6nd_y(ecx)))
    {
        u32 t = ebx;
        ebx = ecx;
        ecx = t;
    }
    
    u32 ymaxn = ebx;
    int swap_cd;
    if (v6nd_y(ecx) > v6nd_y(edx))
    {
        swap_cd = 0;
    }
    else if (v6nd_y(ecx) != v6nd_y(edx))
    {
        swap_cd = 1;
    }
    else if (v6nd_y(ecx) != v6nd_y(A))
    {
        swap_cd = 1;
    }
    else if (!(v6nd_x(A) < v6nd_x(ecx)))
    {
        swap_cd = 0;
    }
    else
    {
        swap_cd = (v6nd_x(ecx) < v6nd_x(edx));
    }

    if (swap_cd)
    {
        u32 t = ecx;
        ecx = edx;
        edx = t;
    }

    u32 mid = ecx;
    w->y = v6nd_y(A);
    w->n = 0;
    w->first = -1;
    w->cur = 0;
    w->end = 0;
    v6x3d_edgebuild(0, A, B);
    v6x3d_edgebuild(1, A, D);
    if (!v6x3d_segment())
    {
        v6x3d_orderfix();
        v6x3d_emit();
    }

    w->y += w->n;
    if ((s16)w->y > v6xclipy2)
        goto draw;
    
    if (w->e[1].rows == 0)
        v6x3d_edgebuild(1, w->e[1].lower, C);
    
    if (w->e[0].rows == 0)
        v6x3d_edgebuild(0, w->e[0].lower, C);
    
    if (!v6x3d_segment())
    {
        v6x3d_orderfix();
        v6x3d_emit();
    }

    v6x3d_rowterm(w->cur + 1);
    v6x3d_rowterm(w->cur + 2);
    w->y += w->n;
    if ((s16)w->y > v6xclipy2)
        goto draw;
    
    if (w->e[1].rows == 0)
    {
        if (w->e[0].rows == 0)
            goto draw;
        
        v6x3d_edgebuild(1, mid, ymaxn);
    }

    if (w->e[0].rows == 0)
        v6x3d_edgebuild(0, mid, ymaxn);
    
    if (!v6x3d_segment())
    {
        v6x3d_orderfix();
        v6x3d_emit();
    }

draw:
    
    if (w->first != -1 && w->first < w->end)
        v6x3d_spans(w->first, w->end);
}

static void v6x3d_drawface(u32 rec)
{
    u32 fl = xread8(rec + 0x3b);
    v6x3d_ae5f = 0;
    u32 m = xread32(rec + 0x40);
    if (xread32(m) != 0)
    {
        v6x3d_ae5f = xread8(m + 0x09);
        u8 b0a = xread8(m + 0x0a);
        if (b0a != 0)
        {
            fl |= 0x10;
            if (b0a == 0xff)
                return;
        }
    }

    u32 t = xread32(rec + 0x44);
    v6x3d_res   = t;
    v6x3d_texw  = xread8(t + 0x18);
    v6x3d_texw8 = (u8)(8 - v6x3d_texw);
    v6x3d_texh  = xread8(t + 0x19);
    if (xread8(t + 0x00) == 1)
    {
        fl |= 1;
        if (!(xread8(t + 0x1b) & 1))
            fl = (fl & ~1u) | 2;
    }
    else
    {
        fl &= ~2u;
    }

    if (xread8(t + 0x0f))
        fl |= 0x10;
    
    s16 sy1 = 0, sy2 = 0;
    if (fl & 0x20)
    {
        sy1 = v6xclipy1; sy2 = v6xclipy2;
        v6xclipy1 = (s16)(v6xclipy1 >> 1);
        v6xclipy2 = (s16)(v6xclipy2 >> 1);
        for (int k = 0; k < 4; k++)
        {
            u32 o = rec + 0x0c + (u32)k * 12;
            xwrite16(o, (s16)((s16)xread16(o) >> 1));
        }
    }

    v6x3d_ae98 = fl;
    v6x3d_ae9c = (fl & 6) | ((fl & 0x80) ? 8 : 0);
    if (xread8(rec + 0x3a) == 0)
    {
        v6x3d_tridrv(rec);
    }
    else
    {
        v6x3d_quaddrv(rec);
    }

    if (v6x3d_ae98 & 0x20)
    {
        for (int k = 0; k < 4; k++)
        {
            u32 o = rec + 0x0c + (u32)k * 12;
            xwrite16(o, (s16)((s16)xread16(o) << 1));
        }

        v6xclipy2 = sy2;
        v6xclipy1 = sy1;
    }
}

void v6x3d_flush(u32 scr)
{
    v6tf_cacheon = 0;
    u32 hbuf = xread32(scr + 0xb4);
    v6x3d_fogbase = hbuf ? xread32(hbuf) + 0x10000 : 0;
    v6x3d_bands = (u8)xread16(scr + 0x96);
    for (int g = 0x1ff; g >= 0; g--)
    {
        if (!v6x3d_slotdirty[g])
            continue;
        
        for (int i = 15; i >= 0; i--)
        {
            for (u32 rec = v6x3d_slothead[g * 16 + i]; rec > 1; rec = xread32(rec))
                v6x3d_drawface(rec);
        }
    }
}

static u32 v6x3d_pickrec;

static int v6x3d_pickrecur(u32 rec, s32 zmin)
{
    if (rec <= 1)
        return 0;
    if (xread32(rec) != 0)
    {
        if (v6x3d_pickrecur(xread32(rec), zmin))
            return 1;
    }
    
    v6x3d_pickhit = 0;
    if ((u32)(u16)xread16(rec + 0x38) < (u32)zmin)
        return 0;
    
    s16 px = (s16)v6x3d_pickx, py = (s16)v6x3d_picky;
    if (px < (s16)xread16(rec + 0x48))
        return 0;
    
    if (px > (s16)xread16(rec + 0x4c))
        return 0;
    
    if (py < (s16)xread16(rec + 0x4a))
        return 0;
    
    if (py > (s16)xread16(rec + 0x4e))
        return 0;
    
    xwrite8(rec + 0x3b, xread8(rec + 0x3b) & 0x7f);
    v6x3d_drawface(rec);
    if (v6x3d_pickhit)
    {
        v6x3d_pickrec = rec;
        return 1;
    }

    return 0;
}

int v6x3d_pickwalk(u32 scr, s32 px, s32 py, s32 zmin, s32 w3[3], u32 *elem, u32 *tag, u32 *depth)
{
    v6x3d_pick = 1;
    v6x3d_pickx = px;
    v6x3d_picky = py;

    s16 sx1 = v6xclipx1, sx2 = v6xclipx2, sy1 = v6xclipy1, sy2 = v6xclipy2;
    v6xclipx1 = v6xclipx2 = (s16)px;
    v6xclipy1 = v6xclipy2 = (s16)py;
    int hit = 0;
    for (u32 g = 0; g < 0x200 && !hit; g++)
    {
        if (!v6x3d_slotdirty[g])
            continue;
        
        for (int i = 0; i < 16 && !hit; i++)
        {
            u32 head = v6x3d_slothead[g * 16 + i];
            if (head == 0)
                continue;
            
            if (v6x3d_pickrecur(head, zmin))
                hit = 1;
        }
    }

    v6xclipx1 = sx1; v6xclipx2 = sx2; v6xclipy1 = sy1; v6xclipy2 = sy2;
    v6x3d_pick = 0;
    if (!hit)
        return 0;
    
    u32 rec = v6x3d_pickrec;
    u32 d16 = (u16)xread16(rec + 0x38);             // hit depth
    *depth = d16;
    *elem = xread32(rec + 0x3c);                    // owner elem
    u32 tg = 0;                                     // pick tag
    u32 mm = xread32(rec + 0x40);
    if (xread32(mm) != 0)                           // mat has texture
        tg = xread32(mm + 0x14);                    // explicit tag
    
    if (tg == 0)
        tg = xread32(xread32(rec + 0x44) + 8);      // texres+8
    
    *tag = tg;

    s32 num = (s32)(s16)xread16(scr + 0x4e) + (s32)d16;             // persp numerator
    s32 bx = (s32)(s16)(u16)((u16)px - (u16)xread16(scr + 0x62));   // 16-bit
    s32 camx = (s32)((s64)num * bx / (s16)xread16(scr + 0x48));     // sub+sext
    s32 by = (s32)(s16)(u16)(0 - (u16)((u16)py - (u16)xread16(scr + 0x64)));
    s32 camy = (s32)((s64)num * by / (s16)xread16(scr + 0x4c));     // Y flipped
    w3[0] = camx; w3[1] = (s32)d16; w3[2] = camy;                   // (x, depth, y-up)
    v6tf_build(scr, 0, &v6tf_camobj);
    v6tf_apply(w3, &v6tf_camobj, w3);                               // WORLD point
    return 1;
}

void v6x_skyfill(u32 s)
{
    s32 blitw = v6xclipx2 - v6xclipx1 + 1;
    s32 blith = v6xclipy2 - v6xclipy1 + 1;
    if (blitw <= 0 || blith <= 0)
        return;
    
    u32 entry = xread32(s + 0x118);
    if (entry == 0)
        return;
    
    u32 form = entry + xread32(entry);
    s32 W = (u16)xread16(form + 0x10);
    s32 H = (u16)xread16(form + 0x12);
    if (W <= 0)
        return;
    
    u32 sx = xread32(s + 0x0c), sy = xread32(s + 0x10);
    u16 syaw = xread16(s + 0x1c), sroll = xread16(s + 0x1a);
    v6tf_cacheon = 0;
    xwrite16(s + 0x1c, 0); xwrite16(s + 0x1a, 0);
    xwrite32(s + 0x0c, 0); xwrite32(s + 0x10, 0);
    s32 v[3] = { 0, xread32(s + 0x11c), 0 };
    v6tf_build(0, s, &v6tf_camobj);
    v6tf_apply(v, &v6tf_camobj, v);
    xwrite16(s + 0x1a, sroll); xwrite16(s + 0x1c, syaw);
    xwrite32(s + 0x10, sy); xwrite32(s + 0x0c, sx);
    s32 depth = v[1];
    if (depth == 0)
        depth = 1;

    s32 py = -((s32)((s64)v[2] * (s16)xread16(s + 0x4c) / depth)) + (s16)xread16(s + 0x64);
    if (py > 0x5dc0)
        py = 0x5dc0;
    
    if (py < -0x5dc0)
        py = -0x5dc0;
    
    s16 skytop = (s16)(py - (s16)xread16(form + 0x16));
    s32 ush = (((s32)(s16)syaw >> 4) * (s32)(s16)xread16(s + 0x120)) >> 8;
    s32 t = ush - (s16)xread16(form + 0x14) - v6xclipx1;
    s32 u0 = -(t % W);
    if (u0 < 0)
        u0 += W;
    
    s32 span1 = W - u0;
    if (span1 > blitw)
        span1 = blitw;
    
    s32 pitch = alis.platform.width;
    u8 *dst = image.logic + (s32)v6xclipy1 * pitch + v6xclipx1;
    s32 ntop = (s32)skytop - v6xclipy1;
    if (ntop > 0)
    {
        if (ntop > blith)
            ntop = blith;
        
        u8 c0 = xread8(form + 0x20);
        for (s32 r = 0; r < ntop; r++, dst += pitch)
            memset(dst, c0, (size_t)blitw);
    }

    s32 row0 = skytop > v6xclipy1 ? skytop : v6xclipy1;
    s32 rowend = (s32)skytop + H;
    if (rowend > v6xclipy2)
        rowend = v6xclipy2;
    
    s32 ntex = rowend - row0;
    if (ntex > 0)
    {
        s32 skip = v6xclipy1 - (s32)skytop;
        if (skip < 0)
            skip = 0;
        
        u32 src = form + 0x20 + (u32)(skip * W) + (u32)u0;
        for (s32 r = 0; r < ntex; r++, dst += pitch, src += (u32)W)
        {
            if (span1 > 0)
                memcpy(dst, alis.mem + src, (size_t)span1);
            
            s32 rest = blitw - span1;
            if (rest > 0)
                memcpy(dst + span1, alis.mem + src + span1 - (u32)W, (size_t)rest);
        }
    }

    s32 nbot = (s32)v6xclipy2 - (s32)skytop - H - 1;
    if (nbot > 0)
    {
        if (nbot > blith)
            nbot = blith;
        
        u8 cb = xread8(form + 0x20 + (u32)(W * H));
        for (s32 r = 0; r < nbot; r++, dst += pitch)
            memset(dst, cb, (size_t)blitw);
    }
}

void v6x3d_open(u32 scr)
{
    if (xread32(scr + 0xbc) == 0)
    {
        u16 node = alis.platform.kind != EPlatform3DO ? node_take(0xb) : 0;
        xwrite32(scr + 0xc0, 0x1d8c0);
        xwrite32(scr + 0xbc, io_malloc(0x1d8c0));
        if (node)
            scrnode_set(scr, 0xbc, node);
    }

    u32 h = xread32(scr + 0xbc);
    if (h == 0)
    {
        ALIS_DEBUG(EDebugWarning, "v6x3d_open: face arena alloc failed\n");
        return;
    }

    u32 a = xread32(h);
    v6x3d_arena = a;
    v6x3d_arenalim = a + 0x1d4c0;
    v6x3d_shl16 = a + 0x1d4c0;
    for (u32 i = 0; i < 256; i++)
        xwrite32(v6x3d_shl16 + i * 4, i << 16);

    for (int y = 0; y < 500; y++)
        v6x3d_rows[y].rowoff = y * alis.platform.width;

    memset(v6x3d_slotdirty, 1, sizeof(v6x3d_slotdirty));
    v6x3d_framereset(scr);
}

typedef struct {
    s32 ae3c, ae40, ae44, ae48, ae4c, ae50, ae54;
    s16 fogsel;
} sV6Lat;

static sV6Lat v6x3d_lat;
static u8  v6x3d_scale4;
static u32 v6x3d_b544;
static u8  v6x3d_lighton = 1;
static s32 v6x3d_b660[3];

static void v6x3d_gridland(u32 e, u32 s, u32 model, u32 mflags);

PACK_PUSH typedef struct PACK_ATTR {
    s16 sx;             // screen X
    s16 depth;          // camera depth int
    s16 sy;             // screen Y
    u8  u, v;
    s8  n[3];           // normal
    u8  band;           // fog/light band
} sV6Slot; PACK_POP

static u8      v6x3d_vbits[0x2000];                 // transformed bitset
static sV6Slot v6x3d_vcache[0x4000];                // vertex cache
static u32     v6x3d_vertbase, v6x3d_matbase;
static u32     v6x3d_b530;                          // current 3D element
static s16     v6x3d_b54c;                          // word[model+0x26]
static s32     v6x3d_near;                          // adaptive near dist
static u8      v6x3d_ae92;                          // fullbright flag
static s32     v6x3d_ae94;                          // saved depth (x4 world)
static u8      v6x3d_ae58, v6x3d_ae59, v6x3d_ae5a;  // wave params
static u8      v6x3d_ae5c, v6x3d_ae5d, v6x3d_ae5e;
u32            v6x3d_tick;

typedef struct { u16 i[4]; s8 n[3]; u8 mat; u16 flags; u8 uv[8]; u32 vmrec; } sV6Face;

static void v6x3d_wave(u32 mv)
{
    u8 f6 = xread8(mv + 6);
    if (f6 & 2)
    {
        s32 ph = ((s32)(v6x3d_tick << v6x3d_ae59) + (s16)xread16(mv) + (s16)xread16(mv + 2)) << v6x3d_ae5a;
        xwrite16(mv + 4, xread16(mv + 4) + (v6costab[ph & 0xfff] >> (15 - v6x3d_ae58)));
    }

    if (f6 & 4)
    {
        s32 ph = ((s32)(v6x3d_tick << v6x3d_ae5d) + 2 * (s16)xread16(mv + 4)) << v6x3d_ae5e;
        xwrite16(mv + 4, xread16(mv + 4) + (v6costab[ph & 0xfff] >> (15 - v6x3d_ae5c)));
    }
}

// per-vertex light + fog band -> slot->band
static void v6x3d_lightvtx(u32 mv, sV6Slot *sl)
{
    if (v6x3d_ae92)
    {
        sl->band = v6shad.identband;
        return;
    }

    if (v6x3d_scale4)
    {
        v6x3d_ae94 = sl->depth;
        sl->depth = (s16)((u16)sl->depth >> 2);
    }

    u8 f6 = xread8(mv + 6);
    s32 acc = v6x3dfog.baselevel + ((s32)(s8)xread8(mv + 0xb) << v6x3dfog.bandshift);

    for (int li = 0; li < v6x3d_nlights; li++)
    {
        sV6Light *rec = &v6x3d_light[li];
        if (!rec->active)
            continue;
        
        int camset = (f6 & 0x10) != 0;
        if (rec->kind == 1)
        {
            s32 *dir = camset ? rec->camdir : rec->objdir;
            s32 d = sl->n[0]*dir[0] + sl->n[1]*dir[1] + sl->n[2]*dir[2];
            if (f6 & 8)
            {
                d = d < 0 ? -d : d;
            }
            else
            {
                d = -d;
            }

            acc += (s32)(((s64)d * rec->intensity) >> 14);
            continue;
        }

        s32 *pos = camset ? rec->campos : rec->objpos;
        s32 dx = (s16)xread16(mv + 0) - pos[0];
        s32 dy = (s16)xread16(mv + 2) - pos[1];
        s32 dz = (s16)xread16(mv + 4) - pos[2];
        s32 manh = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy) + (dz < 0 ? -dz : dz);
        if (manh >= (s32)rec->range)
            continue;
        
        if (rec->spot)
        {
            s32 *pdir = camset ? rec->camdir : rec->objdir;
            s32 *ppos = camset ? rec->campos : rec->objpos;
            s32 ax = pdir[0] - ppos[0], ay = pdir[1] - ppos[1], az = pdir[2] - ppos[2];
            s32 a = ax*dx + ay*dy + az*dz;
            if (a <= 0)
                continue;
            
            if (a <= (((5 * manh - (s32)rec->range) >> 2) * rec->cone))
                continue;
        }

        s32 nd  = sl->n[0]*dx + sl->n[1]*dy;
        s32 ndz = sl->n[2]*dz;
        s32 num;
        switch (f6 & 0x38)
        {
        case 0x10: case 0x18:
            if (manh <= 200)
            {
                manh += 1;
                num = manh << 7;
                break;
            }

        default:
            nd += ndz;
            manh += 0x32;
            nd -= 0x1900;
            if (nd >= 0)
                goto nextlight;
            num = -nd;
            break;
        case 0x08: case 0x28:
            manh += 0x32;
            num = nd - 0x1900 + ndz;
            if (num < 0)
                num = -num;
            break;
        case 0x30: case 0x38:
            manh += 1;
            num = manh << 4;
            break;
        }

        if (manh <= (s32)rec->fstart)
        {
            acc += (s32)(((s64)rec->intensity * num / manh) >> 7);
        }
        else
        {
            acc += (s32)(((s64)((((manh - (s32)rec->fstart) * rec->slope + rec->int8fp) >> 8)) * num / manh) >> 7);
        }

nextlight: ;
    }

    s32 band, d = sl->depth - v6x3dfog.fogstart;
    if (v6x3dfog.fogslope != 0 && d >= 0)
    {
        if (v6x3dfog.attslope != 0)
        {
            s32 lim = v6x3dfog.bandclamp << 7;
            if (acc > lim)
                acc = lim;
            
            acc -= (((d * v6x3dfog.attslope) >> 8) * acc) >> 8;
        }

        acc += (d * v6x3dfog.fogslope) >> 9;
        band = acc >> 7;
        if ((u32)band >= (u32)v6x3dfog.bands)
            band = band < 0 ? 0 : v6x3dfog.bands - 1;
    }
    else
    {
        band = acc >> 7;
        if ((u32)band >= (u32)v6x3dfog.bandclamp)
            band = band < 0 ? 0 : v6x3dfog.bandclamp - 1;
    }

    sl->band = (u8)band;
    if (v6x3d_scale4)
        sl->depth = (s16)v6x3d_ae94;
}

static int v6x3d_project(u32 mv, sV6Slot *out, int *clippable, sV6Ctx *base)
{
    v6x3d_cntverts++;
    u8 f6 = xread8(mv + 6);
    u16 zsave = xread16(mv + 4);
    if (f6 & 6)
        v6x3d_wave(mv);
    s16 vx = (s16)xread16(mv + 0);
    s16 vy = (s16)xread16(mv + 2);
    s16 vz = (s16)xread16(mv + 4);
    sV6Ctx *c = ((f6 & 1) && base == &v6tf_objcam) ? &v6tf_parentctx : base;
    int ret = 1, clip = 0;

    s32 zf = (s32)c->m[1][0]*vx + (s32)c->m[1][1]*vy + (s32)c->m[1][2]*vz + ((c->t[1] + c->params[1]) << 15);
    c->params[8] = zf;
    s32 zi = zf >> 15;
    if (zi > c->params2[1])
        goto done;
    
    if (zi < -((s32)(u32)xread8(mv + 7) << 6))
        goto done;
    
    out->depth = (s16)(zi > 32000 ? 32000 : zi < -32000 ? -32000 : zi);

    if (c->flags & 0x80)
    {
        if (c->flags & 0x40)
        {
            if (zi < 0)
                goto done;
            
            out->sx = (s16)((((vx + c->t[0]) * c->scale) >> 8) + c->params[6]);
            out->sy = (s16)(-(((vz + c->t[2]) * c->scale) >> 8) + c->params[7]);
            ret = 0;
            goto done;
        }

        if (zi < 0)
            goto done;
        
        if (zi == 0)
            zi = 1;
        
        c->params[8] = zi;
        out->sx = (s16)(c->t[0] * c->params[0] / zi + c->params[6] + ((vx * c->scale) >> 8));
        out->sy = (s16)(-(c->t[2] * c->params[0] / zi) + c->params[7] - ((vz * c->scale) >> 8));
        ret = 0;
        goto done;
    }

    if (zi <= v6x3d_near)
    {
        clip = 1;
        goto done;
    }

    {
        s32 xf = (s32)c->m[0][0]*vx + (s32)c->m[0][1]*vy + (s32)c->m[0][2]*vz + (c->t[0] << 15);
        s32 sx = (s32)((s64)xf * c->params[0] / c->params[8]);
        if (sx > 16000 || sx < -16000)
            goto done;
        
        sx += c->params[6];
        out->sx = (s16)(sx > 24000 ? 24000 : sx < -24000 ? -24000 : sx);
        s32 yf = (s32)c->m[2][0]*vx + (s32)c->m[2][1]*vy + (s32)c->m[2][2]*vz + (c->t[2] << 15);
        s32 sy = (s32)(-((s64)yf * c->params[2] / c->params[8]));
        if (sy > 16000 || sy < -16000)
            goto done;
        
        sy += c->params[7];
        out->sy = (s16)(sy > 24000 ? 24000 : sy < -24000 ? -24000 : sy);
        ret = 0;
    }

done:
    
    if (f6 & 6)
        xwrite16(mv + 4, zsave);
    *clippable = clip;
    return ret;
}

static int v6x3d_tri_insert(const sV6Face *fr, sV6Slot *slotbase)
{
    sV6Slot *s0 = slotbase + fr->i[0];
    sV6Slot *s1 = slotbase + fr->i[1];
    sV6Slot *s2 = slotbase + fr->i[2];
    u8 rf = 6;
    if (s0->band == s1->band && s1->band == s2->band)
    {
        rf &= (u8)~4;
        if (s0->band == (u8)v6shad.solidband && v6shad.solidflag)
            rf &= (u8)~6;
    }

    s0->u = fr->uv[0]; s0->v = fr->uv[1];
    s1->u = fr->uv[2]; s1->v = fr->uv[3];
    s2->u = fr->uv[4]; s2->v = fr->uv[5];
    sV6Slot *p0 = s0, *p1 = s1, *p2 = s2, *t;
    if (p1->sy < p0->sy)
    {
        t = p0;
        p0 = p1;
        p1 = t;
    }

    if (p2->sy < p0->sy)
    {
        t = p0;
        p0 = p2;
        p2 = t;
    }

    if (p2->sy < p1->sy)
    {
        t = p1;
        p1 = p2;
        p2 = t;
    }

    if (p0->sy > v6xclipy2)
        return 0;
    
    s16 my = p1->sy > p2->sy ? p1->sy : p2->sy;
    if (my < v6xclipy1)
        return 0;
    
    s32 key = (u16)((u16)(p0->depth + p1->depth) >> 1);
    key = (u16)((u16)(key + p2->depth) >> 1);
    if (key > v6x3d_lat.ae3c)
        return 0;
    
    s32 ae88 = key;
    if (fr->flags & 0x2010)
    {
        if (fr->flags & 0x10)
        {
            key = p0->depth;
            if (p1->depth > key)
                key = p1->depth;
            
            if (p2->depth > key)
                key = p2->depth;
            
            if (fr->flags & 0x2000)
                key += 2 * v6x3d_b54c;
        }
        else
        {
            key -= v6x3d_b54c;
            if (key <= 0)
                key = 1;
        }
    }


    s16 x1 = s0->sx, x2 = s0->sx;
    if (s1->sx < x1)
        x1 = s1->sx;

    if (s2->sx < x1)
        x1 = s2->sx;

    if (s1->sx > x2)
        x2 = s1->sx;

    if (s2->sx > x2)
        x2 = s2->sx;

    if (x2 < v6xclipx1)
        return 0;
    
    if (x1 > v6xclipx2)
        return 0;
    
    u32 m = v6x3d_matbase + (u32)fr->mat * 0x20;
    if (xread32(m) == -1)
        return 0;

    if (key < 0)
        key = 0;

    if (key > 0x7fff)
        key = 0x7fff;

    u32 rec = v6x3d_slotinsert(key);
    xwrite32(rec + 0x04, fr->vmrec);
    xwrite16(rec + 0x48, x1);
    xwrite16(rec + 0x4a, p0->sy);
    xwrite16(rec + 0x4c, x2);
    xwrite16(rec + 0x4e, my);
    xwrite16(rec + 0x38, (u16)ae88);
    xwrite32(rec + 0x3c, v6x3d_b530);
    xwrite32(rec + 0x40, m);
    xwrite32(rec + 0x44, m + xread32(m));

    {
        u8 bl = rf;
        if ((u32)ae88 >= (u32)v6x3d_lat.ae48)
            bl &= (u8)~2;
        
        if (ae88 <= v6x3d_lat.ae4c)
            bl |= 0x40;
        
        if (ae88 <= v6x3d_lat.ae50)
            bl |= 0x20;
        
        if (fr->flags & 0x5000)
        {
            if (ae88 <= v6x3d_lat.ae40 && (fr->flags & 0x4000))
                bl |= 0x80;
            
            if (ae88 <= v6x3d_lat.ae44 && (fr->flags & 0x1000))
                bl |= 0x80;
        }

        if (ae88 <= v6x3d_lat.ae54)
            bl |= 0x80;
        
        xwrite8(rec + 0x3b, bl);
    }

    xwrite8(rec + 0x3a, 0);

    const sV6Slot *ns[3] = { p0, p1, p2 };
    for (int k = 0; k < 3; k++)
    {
        u32 o = rec + 0x08 + (u32)k * 12;
        xwrite16(o + 0, ns[k]->sx);
        xwrite16(o + 2, ns[k]->depth);
        xwrite16(o + 4, ns[k]->sy);
        xwrite8(o + 6, ns[k]->u);
        xwrite8(o + 7, ns[k]->v);
        xwrite8(o + 8, (u8)ns[k]->n[0]);
        xwrite8(o + 9, (u8)ns[k]->n[1]);
        xwrite8(o + 10, (u8)ns[k]->n[2]);
        xwrite8(o + 11, ns[k]->band);
    }

    return 1;
}

static void v6x3d_cliptri(u32 r, u16 flags);

static int v6x3d_pipeA(u32 r)
{
    u16 flags = xread16(r + 0xc);
    if (flags & 0x80)
        return 0;
    
    if (flags & 0x400)
    {
        u8 mb = xread8(v6x3d_matbase + (u32)xread8(r + 0xb) * 0x20 + 0xb);
        v6x3d_ae58 = v6x3d_ae5c = (u8)((mb & 7) * 2);
        v6x3d_ae59 = v6x3d_ae5d = (u8)(((mb >> 3) & 7) * 2);
        v6x3d_ae5a = v6x3d_ae5e = (u8)((mb >> 6) & 3);
    }

    u16 i0 = xread16(r + 0);
    u16 i1 = xread16(r + 2);
    u16 i2 = xread16(r + 4);
    if (i0 >= 0x4000 || i1 >= 0x4000 || i2 >= 0x4000)
        return 0;

    u32 v0 = v6x3d_vertbase + (u32)i0 * 0xC;
    s32 dot = (s8)xread8(r + 8)  * ((s16)xread16(v0 + 0) - v6x3d_b660[0]) + (s8)xread8(r + 9)  * ((s16)xread16(v0 + 2) - v6x3d_b660[1]) + (s8)xread8(r + 10) * ((s16)xread16(v0 + 4) - v6x3d_b660[2]);
    s32 mag;
    if (!(flags & 0xc04))
    {
        mag = -dot;
        if (mag < 0)
            return 0;
    }
    else
    {
        mag = dot < 0 ? -dot : dot;
    }

    if (flags & 0x1400)
    {
        v6x3d_near = v6tf_objcam.params2[0];
    }
    else
    {
        v6x3d_near = (s32)(((u32)(u16)v6x3d_lat.fogsel * (u32)mag) >> 22);
        if (v6x3d_near == 0)
            v6x3d_near = 1;
        
        if (v6x3d_near > v6tf_objcam.params2[0])
            v6x3d_near = v6tf_objcam.params2[0];
    }

    u16 idx[3] = { i0, i1, i2 };
    for (int k = 0; k < 3; k++)
    {
        u16 ix = idx[k];
        u32 mv = v6x3d_vertbase + (u32)ix * 0xC;
        sV6Slot *sl = &v6x3d_vcache[ix];
        if (!(v6x3d_vbits[ix >> 3] & (1 << (ix & 7))))
        {
            v6x3d_vbits[ix >> 3] |= (u8)(1 << (ix & 7));
            int clippable;
            if (v6x3d_project(mv, sl, &clippable, &v6tf_objcam))
            {
                v6x3d_vbits[ix >> 3] &= (u8)~(1 << (ix & 7));
                if (clippable)
                    v6x3d_cliptri(r, flags);
                
                return 0;
            }

            if (flags & 8)
            {
                sl->n[0] = (s8)xread8(r + 8);
                sl->n[1] = (s8)xread8(r + 9);
                sl->n[2] = (s8)xread8(r + 10);
                sl->band = xread8(r + 0xb);
            }
            else
            {
                sl->n[0] = (s8)xread8(mv + 8);
                sl->n[1] = (s8)xread8(mv + 9);
                sl->n[2] = (s8)xread8(mv + 10);
                sl->band = xread8(mv + 0xb);
            }

            v6x3d_lightvtx(mv, sl);
        }
        else if (flags & 8)
        {
            sl->n[0] = (s8)xread8(r + 8);
            sl->n[1] = (s8)xread8(r + 9);
            sl->n[2] = (s8)xread8(r + 10);
            v6x3d_lightvtx(mv, sl);
        }
    }

    if ((flags & 0x404) && !(flags & 0x800))
    {
        sV6Slot *s0 = &v6x3d_vcache[i0], *s1 = &v6x3d_vcache[i1], *s2 = &v6x3d_vcache[i2];
        s32 cross = (s32)(s16)(s1->sx - s0->sx) * (s16)(s2->sy - s0->sy) - (s32)(s16)(s2->sx - s0->sx) * (s16)(s1->sy - s0->sy);
        if (cross > 0)
            return 0;
    }

    sV6Face fr;
    fr.i[0] = i0; fr.i[1] = i1; fr.i[2] = i2; fr.i[3] = xread16(r + 6);
    fr.n[0] = (s8)xread8(r + 8); fr.n[1] = (s8)xread8(r + 9); fr.n[2] = (s8)xread8(r + 10);
    fr.mat = xread8(r + 0xb);
    fr.flags = flags;
    for (int k = 0; k < 8; k++)
        fr.uv[k] = xread8(r + 0x10 + k);

    fr.vmrec = r;
    return v6x3d_tri_insert(&fr, v6x3d_vcache);
}

PACK_PUSH typedef struct PACK_ATTR {
    sV6Slot s;
    s32 camx;
    s32 rawd;
    s32 camy;
} sV6CNode; PACK_POP

static sV6CNode v6x3d_cnode[5];
static sV6CNode v6x3d_cnode_ctr;
static s32 v6x3d_b6cc, v6x3d_b6d0;
static s32 v6x3d_b6d4;
static s32 v6x3d_b6dc, v6x3d_b6e0;
static u32 v6x3d_cscratch;

static int v6x3d_quad_insert(const sV6Face *fr, sV6Slot *slotbase)
{
    sV6Slot *A = slotbase + fr->i[0];
    sV6Slot *B = slotbase + fr->i[1];
    sV6Slot *C = slotbase + fr->i[2];
    sV6Slot *D = slotbase + fr->i[3];
    u8 rf = 6;
    if (A->band == B->band && B->band == C->band && C->band == D->band)
    {
        rf &= (u8)~4;
        if (A->band == (u8)v6shad.solidband && v6shad.solidflag)
            rf &= (u8)~6;
    }

    s16 y = A->sy;
    if (!(y > B->sy))
        y = B->sy;

    if (!(y > C->sy))
        y = C->sy;

    if (!(y > D->sy))
        y = D->sy;

    if (y < v6xclipy1)
        return 0;

    s16 maxy = y;
    sV6Slot *m = A; y = A->sy;
    if (!(y < B->sy))
    {
        y = B->sy;
        m = B;
    }

    if (!(y < C->sy))
    {
        y = C->sy;
        m = C;
    }

    if (!(y < D->sy))
    {
        y = D->sy;
        m = D;
    }

    if (y > v6xclipy2)
        return 0;
    
    s16 miny = y;
    A->u = fr->uv[0]; A->v = fr->uv[1];
    B->u = fr->uv[2]; B->v = fr->uv[3];
    C->u = fr->uv[4]; C->v = fr->uv[5];
    D->u = fr->uv[6]; D->v = fr->uv[7];

    sV6Slot *n0 = A, *n1 = B, *n2 = C, *n3 = D;
    if (m == B)
    {
        n0 = B;
        n1 = C;
        n2 = D;
        n3 = A;
    }
    else if (m == C)
    {
        n0 = C;
        n1 = D;
        n2 = A;
        n3 = B;
    }
    else if (m == D)
    {
        n0 = D;
        n1 = A;
        n2 = B;
        n3 = C;
    }

    s32 avg = ((s32)n0->depth + n1->depth + n2->depth + n3->depth) >> 2;
    if (avg > v6x3d_lat.ae3c)
        return 0;

    s32 ae88 = (u16)avg;
    u16 bp = (u16)avg;
    if (fr->flags & 0x2010)
    {
        if (fr->flags & 0x10)
        {
            s16 mx = n0->depth;
            if (!(mx > n1->depth))
                mx = n1->depth;

            if (!(mx > n2->depth))
                mx = n2->depth;

            if (!(mx > n3->depth))
                mx = n3->depth;

            bp = (u16)mx;
            if (fr->flags & 0x2000)
                bp = (u16)(bp + 2 * (u16)v6x3d_b54c);
        }
        else
        {
            bp = (u16)(bp - (u16)v6x3d_b54c);
            if (!((s16)bp > 0))
                bp = 1;
        }
    }

    s16 xd = n3->sx, xc = n2->sx, xb = n1->sx, xa = n0->sx, xt;
    if (!(xd > xc))
    {
        xt = xc;
        xc = xd;
        xd = xt;
    }

    if (!(xd > xb))
    {
        xt = xb;
        xb = xd;
        xd = xt;
    }

    if (!(xd > xa))
    {
        xt = xa;
        xa = xd;
        xd = xt;
    }

    if (xd < v6xclipx1)
        return 0;
    if (!(xc < xb))
    {
        xt = xb;
        xb = xc;
        xc = xt;
    }

    if (!(xc < xa))
    {
        xt = xa;
        xa = xc;
        xc = xt;
    }

    if (xc > v6xclipx2)
        return 0;
    
    u32 mmat = v6x3d_matbase + (u32)fr->mat * 0x20;
    if (xread32(mmat) == -1)
        return 0;
    
    s32 key = (u16)bp;
    if (key > 0x7fff)
        key = 0x7fff;
    
    u32 rec = v6x3d_slotinsert(key);
    xwrite32(rec + 0x04, fr->vmrec);
    xwrite16(rec + 0x48, xc);                       // bbox minX
    xwrite16(rec + 0x4c, xd);                       // bbox maxX
    xwrite16(rec + 0x4a, miny);
    xwrite16(rec + 0x4e, maxy);
    xwrite16(rec + 0x38, (u16)ae88);                // UNBIASED depth
    xwrite32(rec + 0x3c, v6x3d_b530);               // element
    xwrite32(rec + 0x40, mmat);                     // material
    xwrite32(rec + 0x44, mmat + xread32(mmat));     // texture resource
    {
        u8 bl = rf;
        if ((u32)ae88 >= (u32)v6x3d_lat.ae48)
            bl &= (u8)~2;
        
        if (ae88 <= v6x3d_lat.ae4c)
            bl |= 0x40;
        
        if (ae88 <= v6x3d_lat.ae50)
            bl |= 0x20;
        
        if (fr->flags & 0x5000)
        {
            if (ae88 <= v6x3d_lat.ae40 && (fr->flags & 0x4000))
                bl |= 0x80;
            
            if (ae88 <= v6x3d_lat.ae44 && (fr->flags & 0x1000))
                bl |= 0x80;
        }

        if (ae88 <= v6x3d_lat.ae54)
            bl |= 0x80;
        
        xwrite8(rec + 0x3b, bl);
    }

    const sV6Slot *ns[4] = { n0, n1, n2, n3 };
    for (int k = 0; k < 4; k++)
    {
        u32 o = rec + 0x08 + (u32)k * 12;
        xwrite16(o + 0, ns[k]->sx);
        xwrite16(o + 2, ns[k]->depth);
        xwrite16(o + 4, ns[k]->sy);
        xwrite8(o + 6, ns[k]->u);
        xwrite8(o + 7, ns[k]->v);
        xwrite8(o + 8, (u8)ns[k]->n[0]);
        xwrite8(o + 9, (u8)ns[k]->n[1]);
        xwrite8(o + 10, (u8)ns[k]->n[2]);
        xwrite8(o + 11, ns[k]->band);
    }

    xwrite8(rec + 0x3a, 1);
    return 1;
}

static void v6x3d_xform_full(u32 mv, sV6CNode *n)
{
    u8 f6 = xread8(mv + 6);
    u16 zsave = xread16(mv + 4);
    if (f6 & 6)
        v6x3d_wave(mv);
    
    s16 vx = (s16)xread16(mv + 0);
    s16 vy = (s16)xread16(mv + 2);
    s16 vz = (s16)xread16(mv + 4);
    sV6Ctx *c = (f6 & 1) ? &v6tf_parentctx : &v6tf_objcam;
    n->rawd = (s32)c->m[1][0]*vx + (s32)c->m[1][1]*vy + (s32)c->m[1][2]*vz + ((c->t[1] + c->params[1]) << 15);
    s32 d = n->rawd >> 15;
    n->s.depth = (s16)(d > 32000 ? 32000 : d < -32000 ? -32000 : d);
    n->camx = (s32)c->m[0][0]*vx + (s32)c->m[0][1]*vy + (s32)c->m[0][2]*vz + (c->t[0] << 15);
    n->s.sx = (s16)(n->camx >> 15);
    n->camy = (s32)c->m[2][0]*vx + (s32)c->m[2][1]*vy + (s32)c->m[2][2]*vz + (c->t[2] << 15);
    n->s.sy = (s16)(n->camy >> 15);
    if (f6 & 6)
        xwrite16(mv + 4, zsave);
}

static void v6x3d_clipedge(const sV6CNode *in, const sV6CNode *out, sV6CNode *o)
{
    s32 den = (s32)out->s.depth - in->s.depth;
    if (den == 0)
        den = 1;

    s32 t = (s32)((u32)(v6x3d_b6d4 - in->s.depth) << 16) / den;
    s64 lx = in->camx + (((s64)(out->camx - in->camx) * t) >> 16);
    s64 ly = in->camy + (((s64)(out->camy - in->camy) * t) >> 16);
    s32 sx = (s32)(lx * v6x3d_b6cc / ((s64)v6x3d_b6d4 << 15)) + v6x3d_b6dc;
    s32 sy = -(s32)(ly * v6x3d_b6d0 / ((s64)v6x3d_b6d4 << 15)) + v6x3d_b6e0;
    s32 du = ((s32)out->s.u - in->s.u) * t >> 16;
    s32 dv = ((s32)out->s.v - in->s.v) * t >> 16;
    s32 db = ((s32)out->s.band - in->s.band) * t >> 16;
    u8 iu = in->s.u, iv = in->s.v, ib = in->s.band;
    s8 nn[3] = { in->s.n[0], in->s.n[1], in->s.n[2] };
    o->s.depth = (s16)v6x3d_b6d4;
    o->s.sx = (s16)(sx > 24000 ? 24000 : sx < -24000 ? -24000 : sx);
    o->s.sy = (s16)(sy > 24000 ? 24000 : sy < -24000 ? -24000 : sy);
    o->s.u = (u8)(iu + du);
    o->s.v = (u8)(iv + dv);
    o->s.n[0] = nn[0]; o->s.n[1] = nn[1]; o->s.n[2] = nn[2];
    o->s.band = (u8)(ib + db);
}

static void v6x3d_projnode(sV6CNode *n)
{
    s32 sx = (s32)((s64)n->camx * v6x3d_b6cc / n->rawd) + v6x3d_b6dc;
    s32 sy = -(s32)((s64)n->camy * v6x3d_b6d0 / n->rawd) + v6x3d_b6e0;
    n->s.sx = (s16)(sx > 24000 ? 24000 : sx < -24000 ? -24000 : sx);
    n->s.sy = (s16)(sy > 24000 ? 24000 : sy < -24000 ? -24000 : sy);
}

#define V6CNODE_IX(nptr) ((u16)(((nptr) - v6x3d_cnode) * 2))

static void v6x3d_clipquad(u32 r, u16 flags)
{
    v6x3d_b6cc = v6tf_objcam.params[0];
    v6x3d_b6d0 = v6tf_objcam.params[2];
    v6x3d_b6d4 = v6x3d_near;
    v6x3d_b6dc = v6tf_objcam.params[6];
    v6x3d_b6e0 = v6tf_objcam.params[7];
    sV6Face trec, qrec;
    trec.n[0] = qrec.n[0] = (s8)xread8(r + 8);
    trec.n[1] = qrec.n[1] = (s8)xread8(r + 9);
    trec.n[2] = qrec.n[2] = (s8)xread8(r + 10);
    trec.mat  = qrec.mat  = xread8(r + 0xb);
    trec.flags = qrec.flags = flags;
    trec.vmrec = qrec.vmrec = 0;

    for (int i = 0; i < 4; i++)
    {
        u16 ix = xread16(r + (u32)i * 2);
        u32 mv = v6x3d_vertbase + (u32)ix * 0xC;
        sV6CNode *n = &v6x3d_cnode[i];
        v6x3d_xform_full(mv, n);
        n->s.u = xread8(r + 0x10 + (u32)i * 2);
        n->s.v = xread8(r + 0x11 + (u32)i * 2);
        if (flags & 8)
        {
            n->s.n[0] = (s8)xread8(r + 8);
            n->s.n[1] = (s8)xread8(r + 9);
            n->s.n[2] = (s8)xread8(r + 10);
        }
        else
        {
            n->s.n[0] = (s8)xread8(mv + 8);
            n->s.n[1] = (s8)xread8(mv + 9);
            n->s.n[2] = (s8)xread8(mv + 10);
        }

        n->s.band = xread8(mv + 0xb);
        v6x3d_lightvtx(mv, &n->s);
    }

    sV6CNode *a = &v6x3d_cnode[0], *b = &v6x3d_cnode[1], *c = &v6x3d_cnode[2], *d = &v6x3d_cnode[3], *t;
    s16 plane = (s16)v6x3d_b6d4;
#define V6ROT() do { t = d; d = c; c = b; b = a; a = t; } while (0)
    int found = 0, one = 0;
    if (plane < a->s.depth)
        found = 1;
    
    if (!found)
    {
        V6ROT();
        if (plane < a->s.depth)
            found = 1;
    }

    if (!found)
    {
        V6ROT();
        if (plane < a->s.depth)
            found = 1;
    }

    if (!found)
    {
        V6ROT();
        if (plane < a->s.depth)
        {
            found = 1;
            one = 1;
        }
    }

    if (!found)
        return;

    if (!one)
    {
        if (plane < d->s.depth)
        {
            V6ROT();
        }
        else if (!(plane < b->s.depth))
        {
            one = 1;
        }

        if (!one)
        {
            if (plane < d->s.depth)
            {
                V6ROT();
                goto three;
            }

            if (plane < c->s.depth)
                goto three;
            goto two;
        }
    }

    v6x3d_clipedge(a, b, b);
    v6x3d_clipedge(a, d, d);
    v6x3d_projnode(a);
    trec.i[0] = V6CNODE_IX(a); trec.i[1] = V6CNODE_IX(b);
    trec.i[2] = V6CNODE_IX(d); trec.i[3] = 0;
    trec.uv[0] = a->s.u; trec.uv[1] = a->s.v;
    trec.uv[2] = b->s.u; trec.uv[3] = b->s.v;
    trec.uv[4] = d->s.u; trec.uv[5] = d->s.v;
    trec.uv[6] = 0; trec.uv[7] = 0;
    v6x3d_tri_insert(&trec, (sV6Slot *)v6x3d_cnode);
    return;

two:

    v6x3d_clipedge(a, d, d);
    v6x3d_projnode(a);
    v6x3d_clipedge(b, c, c);
    v6x3d_projnode(b);
    qrec.i[0] = V6CNODE_IX(a); qrec.i[1] = V6CNODE_IX(b);
    qrec.i[2] = V6CNODE_IX(c); qrec.i[3] = V6CNODE_IX(d);
    qrec.uv[0] = a->s.u; qrec.uv[1] = a->s.v;
    qrec.uv[2] = b->s.u; qrec.uv[3] = b->s.v;
    qrec.uv[4] = c->s.u; qrec.uv[5] = c->s.v;
    qrec.uv[6] = d->s.u; qrec.uv[7] = d->s.v;
    v6x3d_quad_insert(&qrec, (sV6Slot *)v6x3d_cnode);
    return;

three:

    {
        sV6CNode *ex = &v6x3d_cnode[4];
        v6x3d_clipedge(c, d, ex);
        v6x3d_projnode(c);
        v6x3d_clipedge(a, d, d);
        v6x3d_projnode(a);
        v6x3d_projnode(b);

        qrec.i[0] = V6CNODE_IX(a); qrec.i[1] = V6CNODE_IX(b);
        qrec.i[2] = V6CNODE_IX(c); qrec.i[3] = V6CNODE_IX(d);
        qrec.uv[0] = a->s.u; qrec.uv[1] = a->s.v;
        qrec.uv[2] = b->s.u; qrec.uv[3] = b->s.v;
        qrec.uv[4] = c->s.u; qrec.uv[5] = c->s.v;
        qrec.uv[6] = d->s.u; qrec.uv[7] = d->s.v;
        trec.i[0] = V6CNODE_IX(c); trec.i[1] = V6CNODE_IX(d);
        trec.i[2] = V6CNODE_IX(ex); trec.i[3] = 0;
        trec.uv[0] = c->s.u; trec.uv[1] = c->s.v;
        trec.uv[2] = d->s.u; trec.uv[3] = d->s.v;
        trec.uv[4] = ex->s.u; trec.uv[5] = ex->s.v;
        trec.uv[6] = 0; trec.uv[7] = 0;
        v6x3d_tri_insert(&trec, (sV6Slot *)v6x3d_cnode);
        v6x3d_quad_insert(&qrec, (sV6Slot *)v6x3d_cnode);
    }
#undef V6ROT
}

static void v6x3d_cliptri(u32 r, u16 flags)
{
    v6x3d_b6cc = v6tf_objcam.params[0];
    v6x3d_b6d0 = v6tf_objcam.params[2];
    v6x3d_b6d4 = v6x3d_near;
    v6x3d_b6dc = v6tf_objcam.params[6];
    v6x3d_b6e0 = v6tf_objcam.params[7];
    sV6Face trec, qrec;
    trec.n[0] = qrec.n[0] = (s8)xread8(r + 8);
    trec.n[1] = qrec.n[1] = (s8)xread8(r + 9);
    trec.n[2] = qrec.n[2] = (s8)xread8(r + 10);
    trec.mat  = qrec.mat  = xread8(r + 0xb);
    trec.flags = qrec.flags = flags;
    trec.vmrec = qrec.vmrec = 0;
    for (int i2 = 0; i2 < 3; i2++)
    {
        u16 ix = xread16(r + (u32)i2 * 2);
        u32 mv = v6x3d_vertbase + (u32)ix * 0xC;
        sV6CNode *n = &v6x3d_cnode[i2];
        v6x3d_xform_full(mv, n);
        n->s.u = xread8(r + 0x10 + (u32)i2 * 2);
        n->s.v = xread8(r + 0x11 + (u32)i2 * 2);
        if (flags & 8)
        {
            n->s.n[0] = (s8)xread8(r + 8);
            n->s.n[1] = (s8)xread8(r + 9);
            n->s.n[2] = (s8)xread8(r + 10);
        }
        else
        {
            n->s.n[0] = (s8)xread8(mv + 8);
            n->s.n[1] = (s8)xread8(mv + 9);
            n->s.n[2] = (s8)xread8(mv + 10);
        }

        n->s.band = xread8(mv + 0xb);
        v6x3d_lightvtx(mv, &n->s);
    }

    sV6CNode *a = &v6x3d_cnode[0], *b = &v6x3d_cnode[1], *c = &v6x3d_cnode[2], *t;
    s16 plane = (s16)v6x3d_b6d4;
    sV6CNode *in1, *in2, *out;
    if (!(plane < a->s.depth))
    {
        t = a; a = b; b = c; c = t;
        if (!(plane < a->s.depth))
        {
            t = a; a = b; b = c; c = t;
            if (!(plane < a->s.depth))
                return;
            
            goto one;
        }
    }

    if (plane < b->s.depth)
    {
        if (plane < c->s.depth)
            return;
        
        in1 = a; in2 = b; out = c;
        goto two;
    }

    if (!(plane < c->s.depth))
        goto one;
    
    in1 = a; in2 = c; out = b;
    goto two;

one:

    trec.i[0] = V6CNODE_IX(c); trec.i[1] = V6CNODE_IX(b);
    trec.i[2] = V6CNODE_IX(a); trec.i[3] = 0;
    v6x3d_clipedge(a, b, b);
    v6x3d_clipedge(a, c, c);
    v6x3d_projnode(a);
    trec.uv[0] = c->s.u; trec.uv[1] = c->s.v;
    trec.uv[2] = b->s.u; trec.uv[3] = b->s.v;
    trec.uv[4] = a->s.u; trec.uv[5] = a->s.v;
    trec.uv[6] = 0; trec.uv[7] = 0;
    v6x3d_tri_insert(&trec, (sV6Slot *)v6x3d_cnode);
    return;
    
two:

    qrec.i[0] = V6CNODE_IX(in2); qrec.i[1] = V6CNODE_IX(in1);
    qrec.i[2] = V6CNODE_IX(out); qrec.i[3] = V6CNODE_IX(&v6x3d_cnode[3]);
    v6x3d_clipedge(out, in2, &v6x3d_cnode[3]);
    v6x3d_projnode(in2);
    v6x3d_clipedge(out, in1, out);
    v6x3d_projnode(in1);
    qrec.uv[0] = in2->s.u; qrec.uv[1] = in2->s.v;
    qrec.uv[2] = in1->s.u; qrec.uv[3] = in1->s.v;
    qrec.uv[4] = out->s.u; qrec.uv[5] = out->s.v;
    qrec.uv[6] = v6x3d_cnode[3].s.u; qrec.uv[7] = v6x3d_cnode[3].s.v;
    v6x3d_quad_insert(&qrec, (sV6Slot *)v6x3d_cnode);
}

static void v6x3d_pipeB_centered(u32 r, u16 flags)
{
    if (v6x3d_cscratch == 0)
    {
        s32 h = io_malloc(0x20);
        if (h == 0)
            return;
        
        v6x3d_cscratch = xread32(h);
    }

    u32 ctr = v6x3d_cscratch;
    u32 tmp = v6x3d_cscratch + 0x10;
    u32 mat = v6x3d_matbase + (u32)xread8(r + 0xb) * 0x20;
    if (xread8(mat + 8) & 0x10)
    {
        flags |= 8;
        xwrite16(r + 0xc, flags);
    }

    v6x3d_near = 1;
    u32 vA = v6x3d_vertbase + (u32)xread16(r + 0) * 0xC;
    u32 vB = v6x3d_vertbase + (u32)xread16(r + 2) * 0xC;
    s16 cx = (s16)(((s32)(s16)xread16(vA + 0) + (s16)xread16(vB + 0)) >> 1);
    s16 cz;
    if (v6x3d_b544 & 0x40)
    {
        u32 vC = v6x3d_vertbase + (u32)xread16(r + 4) * 0xC;
        cz = (s16)(((s32)(s16)xread16(vA + 4) + (s16)xread16(vC + 4)) >> 1);
    }
    else
    {
        cz = (s16)xread16(vA + 4);
    }

    s16 cy = (s16)xread16(vA + 2);
    xwrite16(ctr + 0, (u16)cx);
    xwrite16(ctr + 2, (u16)cy);
    xwrite16(ctr + 4, (u16)cz);
    xwrite16(ctr + 6, 0);
    
    v6x3d_xform_full(ctr, &v6x3d_cnode_ctr);
    v6x3d_cnode_ctr.s.depth = (s16)(v6x3d_cnode_ctr.s.depth - (s16)v6tf_objcam.params[1]);
    v6tf_light.t[0] = (s16)v6x3d_cnode_ctr.s.sx;
    v6tf_light.t[1] = (s16)v6x3d_cnode_ctr.s.depth;
    v6tf_light.t[2] = (s16)v6x3d_cnode_ctr.s.sy;

    s32 b548 = (v6tf_light.flags & 0xc0) ? 0x100 : v6tf_light.scale;
    for (int i = 0; i < 4; i++)
    {
        u16 ix = xread16(r + (u32)i * 2);
        if (ix >= 0x4000)
            return;
        
        u32 v = v6x3d_vertbase + (u32)ix * 0xC;
        sV6Slot *sl = &v6x3d_vcache[ix];
        xwrite16(tmp + 0, (u16)(s16)((((s32)(s16)xread16(v + 0) * b548) >> 8) - cx));
        xwrite16(tmp + 2, (u16)(s16)((s16)xread16(v + 2) - cy));
        xwrite16(tmp + 4, (u16)(s16)((((s32)(s16)xread16(v + 4) * b548) >> 8) - cz));
        xwrite16(tmp + 6, xread16(v + 6));
        int clippable;
        if (v6x3d_project(tmp, sl, &clippable, &v6tf_light))
            return;
        
        xwrite8(tmp + 8, xread8(v + 8));
        xwrite8(tmp + 9, xread8(v + 9));
        xwrite8(tmp + 10, xread8(v + 10));
        xwrite8(tmp + 11, xread8(v + 11));
        sl->n[0] = (s8)xread8(v + 8);
        sl->n[1] = (s8)xread8(v + 9);
        sl->n[2] = (s8)xread8(v + 10);
        sl->band = xread8(v + 11);
        u8 f6 = (u8)(xread8(tmp + 6) | 0x10);
        if (flags & 8)
            f6 |= 0x20;
        
        xwrite8(tmp + 6, f6);
        xwrite16(tmp + 0, (u16)v6x3d_cnode_ctr.s.sx);
        xwrite16(tmp + 2, (u16)v6x3d_cnode_ctr.s.depth);
        xwrite16(tmp + 4, (u16)v6x3d_cnode_ctr.s.sy);
        v6x3d_lightvtx(tmp, sl);
    }

    sV6Face fr;
    fr.i[0] = xread16(r + 0); fr.i[1] = xread16(r + 2);
    fr.i[2] = xread16(r + 4); fr.i[3] = xread16(r + 6);
    fr.n[0] = (s8)xread8(r + 8); fr.n[1] = (s8)xread8(r + 9);
    fr.n[2] = (s8)xread8(r + 10);
    fr.mat = xread8(r + 0xb);
    fr.flags = flags;
    for (int k = 0; k < 8; k++)
        fr.uv[k] = xread8(r + 0x10 + k);
    
    fr.vmrec = r;
    v6x3d_quad_insert(&fr, v6x3d_vcache);
}

static int v6x3d_pipeB(u32 r)
{
    u16 flags = xread16(r + 0xc);
    if (!(flags & 0x8000))
        return v6x3d_pipeA(r);
    
    if (flags & 0x200)
    {
        v6x3d_pipeB_centered(r, flags);
        return 0;
    }

    if (flags & 0x80)
        return 0;
    
    if (flags & 0x400)
    {
        u8 mb = xread8(v6x3d_matbase + (u32)xread8(r + 0xb) * 0x20 + 0xb);
        v6x3d_ae58 = v6x3d_ae5c = (u8)((mb & 7) * 2);
        v6x3d_ae59 = v6x3d_ae5d = (u8)(((mb >> 3) & 7) * 2);
        v6x3d_ae5a = v6x3d_ae5e = (u8)((mb >> 6) & 3);
    }

    u16 i0 = xread16(r + 0);
    u16 i1 = xread16(r + 2);
    u16 i2 = xread16(r + 4);
    u16 i3 = xread16(r + 6);
    if (i0 >= 0x4000 || i1 >= 0x4000 || i2 >= 0x4000 || i3 >= 0x4000)
        return 0;
    
    u32 v0 = v6x3d_vertbase + (u32)i0 * 0xC;
    s32 dot = (s8)xread8(r + 8)  * ((s16)xread16(v0 + 0) - v6x3d_b660[0]) + (s8)xread8(r + 9)  * ((s16)xread16(v0 + 2) - v6x3d_b660[1]) + (s8)xread8(r + 10) * ((s16)xread16(v0 + 4) - v6x3d_b660[2]);
    s32 mag;
    if (!(flags & 0xc04))
    {
        mag = -dot;
        if (mag < 0)
            return 0;
    }
    else
    {
        mag = dot < 0 ? -dot : dot;
    }

    if (flags & 0x1400)
    {
        v6x3d_near = v6tf_objcam.params2[0];
    }
    else
    {
        v6x3d_near = (s32)(((u32)(u16)v6x3d_lat.fogsel * (u32)mag) >> 22);
        if (v6x3d_near == 0)
            v6x3d_near = 1;
        
        if (v6x3d_near > v6tf_objcam.params2[0])
            v6x3d_near = v6tf_objcam.params2[0];
    }

    u16 idx[4] = { i0, i1, i2, i3 };
    for (int k = 0; k < 4; k++)
    {
        if (k == 3 && (flags & 0x404) && !(flags & 0x800))
        {
            sV6Slot *s0 = &v6x3d_vcache[i0], *s1 = &v6x3d_vcache[i1], *s2 = &v6x3d_vcache[i2];
            s32 cross = (s32)(s16)(s1->sx - s0->sx) * (s16)(s2->sy - s0->sy) - (s32)(s16)(s1->sy - s0->sy) * (s16)(s2->sx - s0->sx);
            if (cross > 0)
                return 0;
        }

        u16 ix = idx[k];
        u32 mv = v6x3d_vertbase + (u32)ix * 0xC;
        sV6Slot *sl = &v6x3d_vcache[ix];
        if (!(v6x3d_vbits[ix >> 3] & (1 << (ix & 7))))
        {
            v6x3d_vbits[ix >> 3] |= (u8)(1 << (ix & 7));
            int clippable;
            if (v6x3d_project(mv, sl, &clippable, &v6tf_objcam))
            {
                v6x3d_vbits[ix >> 3] &= (u8)~(1 << (ix & 7));
                if (clippable)
                    v6x3d_clipquad(r, flags);
                
                return 0;
            }

            if (flags & 8)
            {
                sl->n[0] = (s8)xread8(r + 8);
                sl->n[1] = (s8)xread8(r + 9);
                sl->n[2] = (s8)xread8(r + 10);
                sl->band = xread8(r + 0xb);
            }
            else
            {
                sl->n[0] = (s8)xread8(mv + 8);
                sl->n[1] = (s8)xread8(mv + 9);
                sl->n[2] = (s8)xread8(mv + 10);
                sl->band = xread8(mv + 0xb);
            }

            v6x3d_lightvtx(mv, sl);
        }
        else if (flags & 8)
        {
            sl->n[0] = (s8)xread8(r + 8);
            sl->n[1] = (s8)xread8(r + 9);
            sl->n[2] = (s8)xread8(r + 10);
            v6x3d_lightvtx(mv, sl);
        }
    }

    sV6Face fr;
    fr.i[0] = i0; fr.i[1] = i1; fr.i[2] = i2; fr.i[3] = i3;
    fr.n[0] = (s8)xread8(r + 8); fr.n[1] = (s8)xread8(r + 9);
    fr.n[2] = (s8)xread8(r + 10);
    fr.mat = xread8(r + 0xb);
    fr.flags = flags;
    for (int k = 0; k < 8; k++)
        fr.uv[k] = xread8(r + 0x10 + k);
    
    fr.vmrec = r;
    return v6x3d_quad_insert(&fr, v6x3d_vcache);
}

static void v6x3d_lights_activate(u32 e, u32 mflags)
{
    v6x3d_ae92 = (mflags & 0x80) ? 1 : 0;
    for (int li = 0; li < v6x3d_nlights; li++)
    {
        sV6Light *rec = &v6x3d_light[li];
        rec->active = 0;
        if (mflags & 0x80)
            continue;
        
        if (rec->kind == 1)
        {
            if (mflags & 0x10)
                continue;
            
            v6tf_build(0, e, &rec->ctx);
            s32 t[3] = { 0, 0, 0 };
            v6tf_apply(t, &rec->ctx, t);
            v6tf_apply(rec->dir, &rec->ctx, rec->objdir);
            rec->objdir[0] -= t[0];
            rec->objdir[1] -= t[1];
            rec->objdir[2] -= t[2];
            rec->active = 1;
        }
        else
        {
            if (mflags & 0x20)
                continue;
            
            v6tf_build(rec->elem, e, &rec->ctx);
            v6tf_apply(rec->pos, &rec->ctx, rec->objpos);
            if (rec->spot)
                v6tf_apply(rec->dir, &rec->ctx, rec->objdir);

            s32 d2 = 0;
            for (int i = 0; i < 3; i++)
            {
                s32 d = (s16)xread16(e + 0x42 + (u32)i * 2) - rec->objpos[i];
                d2 += (s32)((s64)d * d);
            }

            u32 r = (u32)((s16)xread16(e + 0x40) + rec->range);
            if ((u32)d2 <= r * r)
                rec->active = 1;
        }
    }
}

static void v6x3d_object(u32 e, u32 s)
{
    if (xread8(e + 0x23) & 0x70)
        if (!v6x3d_cull(e))
            return;

    u32 mref = xread32(e + 0x3c);
    if (mref == 0)
        return;
    
    u32 model = mref + xread32(mref);
    u32 mflags = xread32(model + 0x28);
    v6x3d_scale4 = (mflags & 8) ? 1 : 0;
    int sh = v6x3d_scale4 ? 2 : 0;

    v6tf_build(e, s, &v6tf_objcam);

    {
        s32 c[3];
        c[0] = (s16)xread16(e + 0x42);
        c[1] = (s16)xread16(e + 0x44);
        c[2] = (s16)xread16(e + 0x46);
        v6tf_apply(c, &v6tf_objcam, c);
        s64 dx = (s16)xread16(s + 0x42) - (s64)c[0];
        s64 dy = ((s64)((s16)xread16(s + 0x44)) << sh) - c[1];
        s64 dz = (s16)xread16(s + 0x46) - (s64)c[2];
        s64 d2 = dx*dx + dy*dy + dz*dz;
        s64 lim = (u16)xread16(e + 0x40) + ((s64)((u16)xread16(s + 0x40)) << sh);
        lim = lim * lim;
        if (d2 > lim)
            return;
    }

    v6x3d_lat.fogsel = v6costab[((u16)xread16(s + 0x176)) >> 4];
    v6x3d_lat.ae48 = xread32(s + 0x16a) << sh;
    v6x3d_lat.ae50 = (xread32(model + 0x68) != 0 ? xread32(s + 0x15a) : xread32(s + 0x152)) << sh;
    v6x3d_lat.ae4c = xread32(s + 0x156) << sh;
    v6x3d_lat.ae3c = xread32(s + 0x146) << sh;
    v6x3d_lat.ae44 = xread32(s + 0x14a) << sh;
    v6x3d_lat.ae40 = xread32(s + 0x162) << sh;
    v6x3d_lat.ae54 = xread32(s + 0x166) << sh;
    if ((mflags & 0x40) && (xread8(e + 0x25) & 0x40))
        v6x3d_lat.ae54 = v6x3d_lat.ae44 = v6x3d_lat.ae40 = v6x3d_lat.ae50 = v6x3d_lat.ae4c = -1;
    
    if (v6x3d_lighton == 0)
        v6x3d_lat.ae50 = -1;

    v6x3d_b54c = (s16)xread16(model + 0x26);
    v6x3d_ae92 = (mflags & 0x80) ? 1 : 0;

    if (xread32(model + 0x68) != 0)
    {
        if (xread8(model + 0x6c) == 1)
        {
            v6x3d_gridland(e, s, model, mflags);
        }
        else
        {
            static int warned = 0;
            if (!warned++)
                ALIS_DEBUG(EDebugWarning, "v6x3d: portal renderer (FUN_0000fc6d) not yet ported\n");
        }

        return;
    }

    v6tf_build(s, e, &v6tf_camobj);
    {
        s32 sv[3];
        sv[0] = xread32(s + 0x00);
        sv[1] = xread32(s + 0x04);
        sv[2] = xread32(s + 0x08);
        v6tf_apply(sv, &v6tf_camobj, v6x3d_b660);
    }

    v6tf_objcam.params[0] = (s16)xread16(s + 0x48);
    v6tf_objcam.params[1] = (s16)xread16(s + 0x4a);
    v6tf_objcam.params[2] = (s16)xread16(s + 0x4c);
    v6tf_objcam.params[5] = (s16)xread16(s + 0x52);
    v6tf_objcam.params2[0] = xread32(s + 0x142);
    v6tf_objcam.params2[1] = xread32(s + 0x146) + 0x1f4;
    v6tf_objcam.params[6] = (s16)xread16(s + 0x62);
    v6tf_objcam.params[7] = (s16)xread16(s + 0x64);
    if (mflags & 2)
    {
        u8 f = xread8(e + 0x25);
        xwrite8(e + 0x25, f | 0x8 | ((mflags & 0x40) ? 0x2 : 0));
        v6tf_build(e, s, &v6tf_light);
        xwrite8(e + 0x25, f);
        v6tf_light.params[0] = (s16)xread16(s + 0x48);
        v6tf_light.params[1] = (s16)xread16(s + 0x4a);
        v6tf_light.params[2] = (s16)xread16(s + 0x4c);
        v6tf_light.params[5] = (s16)xread16(s + 0x52);
        v6tf_light.params2[0] = xread32(s + 0x142);
        v6tf_light.params2[1] = xread32(s + 0x146) + 0x1f4;
        v6tf_light.params[6] = (s16)xread16(s + 0x62);
        v6tf_light.params[7] = (s16)xread16(s + 0x64);
    }

    if (mflags & 1)
    {
        v6tf_build(xread32(e + 0x8), s, &v6tf_parentctx);
        v6tf_parentctx.params[0] = (s16)xread16(s + 0x48);
        v6tf_parentctx.params[1] = (s16)xread16(s + 0x4a);
        v6tf_parentctx.params[2] = (s16)xread16(s + 0x4c);
        v6tf_parentctx.params[5] = (s16)xread16(s + 0x52);
        v6tf_parentctx.params2[0] = xread32(s + 0x142);
        v6tf_parentctx.params2[1] = xread32(s + 0x146) + 0x1f4;
        v6tf_parentctx.params[6] = (s16)xread16(s + 0x62);
        v6tf_parentctx.params[7] = (s16)xread16(s + 0x64);
    }

    v6x3d_lights_activate(e, mflags);
    v6x3d_vertbase = model + xread32(model + 0x38);
    v6x3d_matbase  = model + xread32(model + 0x44);
    v6x3d_b54c     = (s16)xread16(model + 0x26);
    v6x3d_b544     = mflags;
    v6x3d_ae92     = (mflags & 0x80) ? 1 : 0;
    {
        u32 nverts = (u16)xread16(model + 0x2c);
        u32 nbytes = (((nverts >> 5) + 1) * 4);
        if (nbytes > sizeof(v6x3d_vbits))
            nbytes = sizeof(v6x3d_vbits);
        
        memset(v6x3d_vbits, 0, nbytes);
    }

    {
        u16 na = xread16(model + 0x2e);
        u32 recA = model + xread32(model + 0x3c);
        for (u16 i = 0; i < na; i++, recA += 0x18)
            v6x3d_pipeA(recA);
        
        u16 nb = xread16(model + 0x30);
        u32 recB = model + xread32(model + 0x40);
        for (u16 i = 0; i < nb; i++, recB += 0x18)
            v6x3d_pipeB(recB);
    }
}

static u8 v6x3d_fbits[0x2000];

static void v6x3d_gridland(u32 e, u32 s, u32 model, u32 mflags)
{
    u32 listA = model + xread32(model + 0x3c);
    u32 listB = model + xread32(model + 0x40);
    v6x3d_vertbase = model + xread32(model + 0x38);
    v6x3d_matbase  = model + xread32(model + 0x44);
    v6x3d_b544     = mflags;

    v6tf_build(e, s, &v6tf_objcam);
    v6tf_objcam.params[0] = (s16)xread16(s + 0x48);
    v6tf_objcam.params[1] = (s16)xread16(s + 0x4a);
    v6tf_objcam.params[2] = (s16)xread16(s + 0x4c);
    v6tf_objcam.params[5] = (s16)xread16(s + 0x52);
    v6tf_objcam.params2[0] = xread32(s + 0x142);
    {
        s32 far146 = xread32(s + 0x146);
        if (v6x3d_scale4)
            far146 <<= 2;
        
        v6tf_objcam.params2[1] = far146 + 500;
    }

    v6tf_objcam.params[6] = (s16)xread16(s + 0x62);
    v6tf_objcam.params[7] = (s16)xread16(s + 0x64);

    v6tf_build(s, e, &v6tf_camobj);
    s32 C[3];
    C[0] = (s16)xread16(s + 0x42);
    C[1] = (s16)xread16(s + 0x44);
    C[2] = (s16)xread16(s + 0x46);
    if (mflags & 8)
        C[1] <<= 2;
    
    v6tf_apply(C, &v6tf_camobj, C);
    {
        s32 sv[3];
        sv[0] = xread32(s + 0x00);
        sv[1] = xread32(s + 0x04);
        sv[2] = xread32(s + 0x08);
        v6tf_apply(sv, &v6tf_camobj, v6x3d_b660);
    }

    s32 R = (u16)xread16(s + 0x40);
    if (mflags & 8)
        R <<= 2;
    
    s32 lo[3], hi[3];
    static const u8 orgo[3] = {0x50, 0x58, 0x60}, maxo[3] = {0x54, 0x5c, 0x64};
    static const u8 sho[3]  = {0x6d, 0x6e, 0x6f}, dimo[3] = {0x70, 0x74, 0x78};
    for (int a = 0; a < 3; a++)
    {
        s32 org = xread32(model + orgo[a]);
        s32 mx  = xread32(model + maxo[a]);
        u8  sh  = xread8(model + sho[a]);
        s32 dim = xread32(model + dimo[a]);
        if (C[a] - R > mx)
            return;
        
        lo[a] = (C[a] - R - org) >> sh;
        if (lo[a] < 1)
            lo[a] = 0;
        
        if (C[a] + R < org)
            return;
        
        if (lo[a] >= dim)
            lo[a] = dim - 1;
        
        hi[a] = (C[a] + R - org) >> sh;
        if (hi[a] >= dim)
            hi[a] = dim - 1;
        
        if (hi[a] < 0)
            hi[a] = 0;
    }

    u16 nA = xread16(model + 0x2e), nB = xread16(model + 0x30);
    {
        u32 nfb = (((u32)(u16)(nA + nB) >> 5) + 1) * 4;
        if (nfb > sizeof(v6x3d_fbits))
            nfb = sizeof(v6x3d_fbits);
        
        memset(v6x3d_fbits, 0, nfb);
        u32 nvb = (((u32)(u16)xread16(model + 0x2c) >> 5) + 1) * 4;
        if (nvb > sizeof(v6x3d_vbits))
            nvb = sizeof(v6x3d_vbits);
        
        memset(v6x3d_vbits, 0, nvb);
    }

    if (mflags & 2)
    {
        u8 f = xread8(e + 0x25);
        xwrite8(e + 0x25, f | 0x8 | ((mflags & 0x40) ? 0x2 : 0));
        v6tf_build(e, s, &v6tf_light);
        xwrite8(e + 0x25, f);
        v6tf_light.params[0] = (s16)xread16(s + 0x48);
        v6tf_light.params[1] = (s16)xread16(s + 0x4a);
        v6tf_light.params[2] = (s16)xread16(s + 0x4c);
        v6tf_light.params[5] = (s16)xread16(s + 0x52);
        v6tf_light.params2[0] = xread32(s + 0x142);
        v6tf_light.params2[1] = xread32(s + 0x146) + 500;
        v6tf_light.params[6] = (s16)xread16(s + 0x62);
        v6tf_light.params[7] = (s16)xread16(s + 0x64);
    }

    if (mflags & 1)
    {
        static int warned = 0;
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "v6x3d_gridland: model flag 1 parent frame hits the DOS stale-EDI bug — using the 13b58-correct form\n");
        
        v6tf_build(xread32(e + 0x8), s, &v6tf_parentctx);
        v6tf_parentctx.params[0] = (s16)xread16(s + 0x48);
        v6tf_parentctx.params[1] = (s16)xread16(s + 0x4a);
        v6tf_parentctx.params[2] = (s16)xread16(s + 0x4c);
        v6tf_parentctx.params[5] = (s16)xread16(s + 0x52);
        v6tf_parentctx.params2[0] = xread32(s + 0x142);
        v6tf_parentctx.params2[1] = xread32(s + 0x146) + 500;
        v6tf_parentctx.params[6] = (s16)xread16(s + 0x62);
        v6tf_parentctx.params[7] = (s16)xread16(s + 0x64);
    }

    v6x3d_lights_activate(e, mflags);

    u32 base = model + xread32(model + 0x68);
    u32 dim0 = xread32(model + 0x70);
    u32 dim1 = xread32(model + 0x74);
    u32 plane = dim1 * dim0 * 4;
    u32 rowst = dim0 * 4;
    for (s32 z = lo[2]; z <= hi[2]; z++)
    {
        for (s32 y = lo[1]; y <= hi[1]; y++)
        {
            u32 cp = base + plane * (u32)z + rowst * (u32)y + (u32)lo[0] * 4;
            for (s32 x = lo[0]; x <= hi[0]; x++, cp += 4)
            {
                u32 off = xread32(cp);
                if (off == 0)
                    continue;
                
                u32 fl = model + off;
                u16 n = xread16(fl);
                fl += 2;
                for (u16 i = 0; i < n; i++, fl += 2)
                {
                    u16 f = xread16(fl);
                    if (f == 0xffff)
                        continue;
                    
                    if (v6x3d_fbits[f >> 3] & (1 << (f & 7)))
                        continue;
                    
                    v6x3d_fbits[f >> 3] |= (u8)(1 << (f & 7));
                    if (f < nA)
                    {
                        v6x3d_pipeA(listA + (u32)f * 0x18);
                    }
                    else
                    {
                        v6x3d_pipeB(listB + (u32)(u16)(f - nA) * 0x18);
                    }
                }
            }
        }
    }
}

static u32 v6x3d_fake;

static void v6x3d_billboard(u32 e, u32 s)
{
    if (v6x3d_fake == 0)
    {
        s32 hf = io_malloc(0x200);
        if (hf == 0)
            return;

        v6x3d_fake = xread32(hf);
        u32 F0 = v6x3d_fake, M0 = F0 + 0x80, FR0 = F0 + 0x11c, V00 = F0 + 0x1b4;
        xwrite16(M0 + 0x2c, 4);                     // verts
        xwrite16(M0 + 0x2e, 0);                     // list A count
        xwrite16(M0 + 0x30, 1);                     // list B: ONE quad
        xwrite16(M0 + 0x32, 1);                     // texture slots
        xwrite32(M0 + 0x38, 0x134);                 // self-rel -> verts
        xwrite32(M0 + 0x40, 0x9c);                  // self-rel -> face rec
        xwrite32(M0 + 0x44, 0xb4);                  // self-rel -> materials
        xwrite32(M0 + 0x68, 0);                     // no cell grid
        xwrite8(M0 + 0x6c, 0);
        xwrite32(M0 + 0x28, 0);                     // model flags
        xwrite16(FR0 + 0, 0); xwrite16(FR0 + 2, 1); // quad 0,1,2,3
        xwrite16(FR0 + 4, 2); xwrite16(FR0 + 6, 3);
        xwrite8(FR0 + 8, 0); xwrite8(FR0 + 9, 0x80);// normal (0,-128,0): -Y
        xwrite8(FR0 + 10, 0);
        xwrite8(FR0 + 0xb, 0);                      // material 0
        xwrite16(FR0 + 0xc, 0x8802);                // default flags
        static const s8 vn[4][3] = { {-72,-72,72}, {72,-72,72},
                                     {72,-72,-72}, {-72,-72,-72} };
        for (int vi = 0; vi < 4; vi++)
        {
            u32 vv = V00 + (u32)vi * 0xC;
            xwrite8(vv + 6, 8);                     // vertex flag: two-sided light
            xwrite8(vv + 0xb, 0);
            xwrite8(vv + 8, (u8)vn[vi][0]);
            xwrite8(vv + 9, (u8)vn[vi][1]);
            xwrite8(vv + 10, (u8)vn[vi][2]);
        }
    }

    u32 F  = v6x3d_fake;            // fake elem
    u32 M  = F + 0x80;              // model hdr
    u32 FR = F + 0x11c;             // face record
    u32 TS = F + 0x134;             // texture slot
    u32 V0 = F + 0x1b4;             // 4 verts

    u32 res = xread32(e + 0x3c);
    if (res == 0)
        return;
    
    res += xread32(res);
    s32 w  = (s16)xread16(res + 0x10);
    s32 hh = (s16)xread16(res + 0x12);
    s32 hx = (s16)xread16(res + 0x14);
    s32 hy = (s16)xread16(res + 0x16);
    u32 rad = v6x_isqrt((u32)(w*w + hh*hh)) >> 1;
    xwrite16(M + 0x12, (u16)rad);                   // bounding radius
    xwrite16(M + 0x0c, (u16)(w / 2 - hx));          // centre offset
    xwrite16(M + 0x0e, 0);
    xwrite16(M + 0x10, (u16)(hh / 2 - hy));
    xwrite32(TS, res - TS);                         // self-rel res ptr
    xwrite32(TS + 8, 1);                            // texture slot valid

    u8 flip = xread8(e + 0x52);                     // b0 flipX, b1 flipY
    static const s32 cs[4][2] = { {0,0}, {1,0}, {1,1}, {0,1} };     // corner select
    for (int i = 0; i < 4; i++)
    {
        s32 X = cs[i][0] ? (w - 1 - hx) : -hx;
        s32 Y = cs[i][1] ? -(hh - 1 - hy) : hy;
        if (flip & 1)
            X = -X;
        
        if (flip & 2)
            Y = -Y;
        
        u32 v = V0 + (u32)i * 0xC;
        xwrite16(v + 0, (u16)X);
        xwrite16(v + 2, 0);
        xwrite16(v + 4, (u16)Y);
        xwrite8(v + 0xb, xread8(e + 0x54));
    }

    xwrite8(FR + 0x10, 0);            xwrite8(FR + 0x11, 0);
    xwrite8(FR + 0x12, (u8)(w - 1));  xwrite8(FR + 0x13, 0);
    xwrite8(FR + 0x14, (u8)(w - 1));  xwrite8(FR + 0x15, (u8)(hh - 1));
    xwrite8(FR + 0x16, 0);            xwrite8(FR + 0x17, (u8)(hh - 1));
    xwrite16(FR + 0xc, (u16)(0x8202 | ((flip & 0x20) ? 8 : 0)));

    for (u32 i = 0; i < 0x58; i += 4)
        xwrite32(F + i, xread32(e + i));
    
    xwrite32(F + 0x3c, M - 4);
    xwrite32(M - 4, 4);
    xwrite16(F + 0x40, (u16)rad);
    xwrite16(F + 0x42, xread16(M + 0x0c));
    xwrite16(F + 0x44, xread16(M + 0x0e));
    xwrite16(F + 0x46, xread16(M + 0x10));
    u32 mf = 0x42;
    if (flip & 0x08)
        mf |= 0x20;
    
    if (flip & 0x04)
        mf |= 0x10;
    
    if (flip & 0x10)
        mf |= 0x80;
    
    xwrite32(M + 0x28, mf);
    xwrite16(M + 0x26, 0);
    v6x3d_object(F, s);
    xwrite32(TS, 0);
}

void v6x_mode6(u32 s)
{
    u16 sav_x2 = xread16(s + 0x6a);                 // clip x2
    u16 sav_y2 = xread16(s + 0x6c);                 // clip y2
    u16 sav_w  = xread16(s + 0x7a);                 // buffer logical width
    u16 sav_cx = xread16(s + 0x62);                 // projection center X
    u16 sav_cy = xread16(s + 0x64);                 // projection center Y

    if (xread8(s + 0x59) & 0x20)                    // detail reduction
    {
        u8 clx = xread8(s + 0x136);
        v6xclipx2 -= (s16)((u16)(v6xclipx2 - v6xclipx1 + 1) >> clx);
        u16 w = (u16)(xread16(s + 0x6a) - xread16(s + 0x66) + 1);
        xwrite16(s + 0x6a, xread16(s + 0x6a) - (w >> clx));
        xwrite16(s + 0x7a, (u16)xread16(s + 0x7a) >> clx);
        s16 pcx = (s16)xread16(s + 0x62);
        xwrite16(s + 0x62, pcx - ((s16)(pcx - (s16)xread16(s + 0x66)) >> clx));

        u8 cly = xread8(s + 0x137);
        u16 gdy = (u16)(v6xclipy2 - v6xclipy1 + 1) >> cly;
        v6xclipy2 -= gdy;
        u16 h = (u16)(xread16(s + 0x6c) - xread16(s + 0x68) + 1);
        xwrite16(s + 0x6c, xread16(s + 0x6c) - (h >> cly));
        s16 pcy = (s16)xread16(s + 0x64);
        xwrite16(s + 0x64, pcy - ((s16)(pcy - (s16)xread16(s + 0x68)) >> cly));
    }

    v6xclipx1 = (s16)xread16(s + 0x66);
    v6xclipx2 = (s16)xread16(s + 0x6a);
    v6xclipy1 = (s16)xread16(s + 0x68);
    v6xclipy2 = (s16)xread16(s + 0x6c);

    u16 frnode = alis.platform.kind != EPlatform3DO ? node_take(0xb) : 0;
    v6x3d_framereset(s);
    v6x3d_heapmoved = 0;

    u32 head = xread32(s + 0x5e);
    if (head != 0)
    {
        for (u32 e = xread32(head + 0x38); e != 0; e = xread32(e + 0x38))
        {
            if (e == xread32(e + 0x38))
                break;

            if (xread8(e + 0x24) & 0x40)
                continue;

            if (xread8(e + 0x25) & 0x04)
                continue;

            u8 st = v6sp_state(e);
            if (st == 0x30)
            {
                v6x3d_b530 = e;
                v6x3d_object(e, s);
            }
            else if (st == 0x32)
            {
                v6x3d_b530 = e;
                v6x3d_billboard(e, s);
            }
            else if (st == 0x20)
            {
                if (xread16(e + 0x40) & 1)
                {
                    v6tf_build(e, s, &v6tf_objcam);
                    s32 X = v6tf_objcam.t[0];
                    s32 Y = v6tf_objcam.t[1];
                    s32 Z = v6tf_objcam.t[2];
                    s64 d2 = (s64)X*X + (s64)Y*Y + (s64)Z*Z;
                    if ((u64)d2 >> 32)
                    {
                        xwrite8(e + 0x47, 0);
                        xwrite8(e + 0x45, 0);
                        xwrite8(e + 0x51, 0);
                        v6snd_step(e);
                    }
                    else
                    {
                        s32 dist = (s32)v6x_isqrt((u32)d2);
                        u16 shift = ((u16)xread16(e + 0x42) >> 4) & 0xf;
                        if (shift)
                        {
                            s32 dd = dist;
                            if (dd == 0)
                                dd = 1;

                            s32 old = (u16)xread16(e + 0x4e);
                            xwrite16(e + 0x4e, (u16)dd);
                            s32 freq;
                            if (old == 0)
                            {
                                freq = (u16)xread16(e + 0x48);
                            }
                            else
                            {
                                s32 ab = dd < 0 ? -dd : dd;
                                s64 p = (s64)(old - dd) * ab;
                                s32 v = (s32)(p >> (16 - shift));
                                v += 0x100;
                                s64 f = (s64)v * (u16)xread16(e + 0x48);
                                freq = (s32)(f >> 8);
                                if (freq <= 0x64)
                                    freq = 0x64;
                                if (freq >= 0x7530)
                                    freq = 0x7530;
                            }

                            xwrite16(e + 0x4a, (u16)freq);
                        }

                        s32 fade = xread32(s + 0x172);
                        s32 vol1 = 0, vol2 = 0;
                        s32 A = (u16)xread16(e + 0x52) + fade - dist;
                        if (A > 0)
                        {
                            vol1 = (s8)xread8(e + 0x44);
                            vol2 = (s8)xread8(e + 0x46);
                            if (A < fade)
                            {
                                vol2 = (s32)((s64)A * vol2 / fade);
                                vol1 = (s32)((s64)A * vol1 / fade);
                            }
                        }

                        xwrite8(e + 0x47, (u8)vol2);
                        if (vol1 > 0x7f)
                            vol1 = 0x7f;
                        
                        xwrite8(e + 0x45, (u8)vol1);

                        s32 dead = (s16)xread16(e + 0x52) >> 2;
                        s32 pan = 0;
                        s32 lat = X;
                        int crossed;
                        if (lat < 0)
                        {
                            lat += dead;
                            crossed = (lat >= 0);
                        }
                        else
                        {
                            lat -= dead;
                            crossed = (lat < 0);
                        }

                        if (!crossed)
                        {
                            s64 p = (s64)lat * (s16)xread16(s + 0x48);
                            s32 dep = Y;
                            if (dep < 0)
                                dep = -dep;
                            dep++;
                            pan = (s32)(p / dep);
                            pan >>= 2;
                            if (pan <= -0x7f)
                                pan = -0x7f;
                            if (pan >=  0x7f)
                                pan =  0x7f;
                        }

                        xwrite8(e + 0x51, (u8)pan);
                        v6snd_step(e);
                    }
                }
            }
        }
    }

    node_free(frnode);
    xwrite16(s + 0x64, sav_cy);
    xwrite16(s + 0x62, sav_cx);
    xwrite16(s + 0x7a, sav_w);
    xwrite16(s + 0x6c, sav_y2);
    xwrite16(s + 0x6a, sav_x2);
}

s32 v6x_walkzoff;
s16 v6x_walkaoff;

void v6x_folscreen(u32 s)
{
    s16 id = (s16)xread16(s + 0x7e);
    if (id <= 0)
        return;

    u32 vram = xread32(alis.atent + (u16)id);
    if (vram == 0)
        return;

    u8 fl = xread8(s + 0x80);
    if (fl & 1)
    {
        s32 v = xread32(vram + 0x0c) + xread32(s + 0x82);
        if (v != xread32(s + 0x0c))
        {
            xwrite32(s + 0x0c, v);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }

        v = xread32(vram + 0x10) + xread32(s + 0x86);
        if (v != xread32(s + 0x10))
        {
            xwrite32(s + 0x10, v);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }

        v = xread32(vram + 0x14) + xread32(s + 0x8a) + v6x_walkzoff;
        if (v != xread32(s + 0x14))
        {
            xwrite32(s + 0x14, v);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }
    }

    if (fl & 2)
    {
        s16 a = (s16)(xread16(vram + 0x18) + xread16(s + 0x8e) + v6x_walkaoff);
        if (a != (s16)xread16(s + 0x18))
        {
            xwrite16(s + 0x18, a);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }

        a = (s16)(xread16(vram + 0x1a) + xread16(s + 0x90));
        if (a != (s16)xread16(s + 0x1a))
        {
            xwrite16(s + 0x1a, a);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }

        a = (s16)(xread16(vram + 0x1c) + xread16(s + 0x92));
        if (a != (s16)xread16(s + 0x1c))
        {
            xwrite16(s + 0x1c, a);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }
    }
}
