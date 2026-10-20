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

#include "alis.h"
#include "image.h"
#include "mem.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct { s16 m[9]; s32 t[3]; } mat3d;
typedef struct { mat3d m; s32 fx, fy, cx, cy, near; } view3d;
typedef struct { s32 x, z, y, valid; } pt3d;            // screen x, depth, screen y

typedef struct cel3d {
    u32 img;
    s32 p[8];
    u32 pixc;
    u8 accw;
    s32 depth;
    u32 owner, aux;
    struct cel3d *next;
} cel3d;

#define CELMAX  0x140
#define TILEMAX 0x400
#if defined(ALIS_USE_NATIVE_ATARI)
#define FARMUL  1
#else
#define FARMUL  2
#endif
#define CELCAP  (CELMAX * FARMUL * FARMUL)
#define TILECAP (TILEMAX * FARMUL * FARMUL)
#define BUCKETS 0x280
#define TEXMAX  128

// big tables on the heap at first use (not in every build's .bss)
static s32 *costab;                                     // [0x1000]
static s32 *recip;                                      // [0x8000] 0x40000 / i
static const u32 pixctab[15] = {
    0, 0x03000300, 0x08000800, 0x02000200, 0x10001000, 0x0b000b00, 0x18001800, 0x01000100,
    0x13001300, 0x0a000a00, 0x1b001b00, 0x05000500, 0x12001200, 0x09000900, 0x1a001a00 };

static cel3d *pool;                                     // [CELCAP]
static cel3d *bucket[BUCKETS];
static int ncel, maxbucket = -1, drewland;
static pt3d *cache;                                     // [TILECAP]
static s32 (*spans)[2];                                 // [TILECAP]
// decoded cels kept across frames; sig checks the source is unchanged (scripts may move or unload)
#define TEXBYTES 0x80000
static struct { u32 img, len; u8 sig[48]; s32 w, h; u16 *px; } tex[TEXMAX];
static u32 texbytes;

// '--far' doubles the view distance, 3DOs cel and tile limits grow with the area
static s32 viewfar(u32 scr)  { return (s32)xread32(scr + 0x146) * (image.ddrawdist ? FARMUL : 1); }
static int celmax(void)      { return image.ddrawdist ? CELCAP : CELMAX; }
static int tilemax(void)     { return image.ddrawdist ? TILECAP : TILEMAX; }
static int ntex;

static inline s32 mulw(s32 a, s32 b) { return (s32)((u32)a * (u32)b); }
static inline s32 sdiv(s32 d, s32 n) { return d ? n / d : 0; }

static int tables(void)
{
    static int done;
    if (done)
        return 1;

    costab = calloc(0x1000, sizeof(s32));
    recip  = calloc(0x8000, sizeof(s32));
    pool   = calloc(CELCAP, sizeof(cel3d));
    cache  = calloc(TILECAP, sizeof(pt3d));
    spans  = calloc(TILECAP, sizeof(*spans));
    if (!costab || !recip || !pool || !cache || !spans)
    {
        alis_fatal = "Out of memory for the 3D tables.";
        return 0;
    }
    done = 1;
    // rotation recurrence: cos() is an fcos the 68040/060 lack (Line-F under plain TOS)
    double c = 1, s = 0;
    const double C = 0.99999882345170187925, S = 0.00153398018628476550;   // cos, sin(pi / 2048)
    for (int i = 0; i < 0x1000; i++)
    {
        costab[i] = (s32)(4096 * c);
        double t = c * C - s * S;
        s = s * C + c * S;
        c = t;
    }
    
    costab[2048] = -4095;
    costab[2532] = -3017;
    for (int i = 1; i < 0x8000; i++)
        recip[i] = 0x40000 / i;
    return 1;
}

static void rot(mat3d *m, int a, int b, s32 ang)
{
    if (ang == 0)
        return;
    
    s32 c = costab[ang & 0xfff], s = costab[(ang + 0xc00) & 0xfff];
    for (int i = 0; i < 3; i++)
    {
        s32 x = m->m[a * 3 + i], y = m->m[b * 3 + i];
        m->m[a * 3 + i] = (s16)((x * c - y * s) >> 12);
        m->m[b * 3 + i] = (s16)((y * c + x * s) >> 12);
    }

    s32 x = m->t[a], y = m->t[b];
    m->t[a] = (mulw(x, c) - mulw(y, s)) >> 12;
    m->t[b] = (mulw(y, c) + mulw(x, s)) >> 12;
}

static void cammat(mat3d *m, u32 scr, int inv)
{
    memset(m, 0, sizeof(*m));
    m->m[0] = m->m[4] = m->m[8] = 0x1000;
    s32 ax = (s16)xread16(scr + 0x18), az = (s16)xread16(scr + 0x1c);
    if (!inv)
    {
        rot(m, 1, 2, ax >> 4);
        rot(m, 0, 1, az >> 4);
        for (int i = 0; i < 3; i++)
            m->t[i] += xread32(scr + 0xc + i * 4);
        
        return;
    }

    for (int i = 0; i < 3; i++)
        m->t[i] -= xread32(scr + 0xc + i * 4);
    
    rot(m, 0, 1, -az >> 4);
    rot(m, 1, 2, -ax >> 4);
}

static void xform(const mat3d *m, s32 v[3])
{
    s32 x = v[0], y = v[1], z = v[2];
    for (int r = 0; r < 3; r++)
        v[r] = m->t[r] + ((m->m[r * 3] * x + m->m[r * 3 + 1] * y + m->m[r * 3 + 2] * z) >> 12);
}

static void project(const view3d *v, s32 x, s32 y, s32 z, pt3d *o)
{
    const s16 *m = v->m.m;
    s32 d = v->m.t[1] + ((x * m[3] + y * m[4] + z * m[5]) >> 12);
    s32 px = v->m.t[0] + ((x * m[0] + y * m[1] + z * m[2]) >> 12);
    s32 py = v->m.t[2] + ((x * m[6] + y * m[7] + z * m[8]) >> 12);
    o->z = d;
    o->valid = 1;
    if (d >= v->near)
    {
        s32 r = recip[d < 0x8000 ? d : 0x7fff];
        o->x = v->cx + (mulw(mulw(r, v->fx), px) >> 18);
        o->y = v->cy - (mulw(mulw(r, v->fy), py) >> 18);
        return;
    }

    s32 div = d < 1 ? v->near : d;
    o->x = v->cx + sdiv(div, mulw(v->fx, px));
    o->y = v->cy - sdiv(div, mulw(v->fy, py));
    if (o->y > 300)
        o->y = 300;
    
    if (o->y < -0x32)
        o->y = -0x32;
    
    if (o->x > 0x280)
        o->x = 0x280;
    
    if (o->x < -0x140)
        o->x = -0x140;
}

static u32 fogpixc(u32 scr, u32 depth, s32 shade)
{
    u32 fade = 0x100;
    u16 mode = xread16(scr + 0x94);
    if (mode != 100 && mode != 101)
        return 0x05000500;
    
    u32 fog = alis.fog3do > 0xff ? 0xff : alis.fog3do;
    u32 far = (u32)viewfar(scr);
    if (far < depth)
        depth = far;
    
    u32 f = depth * recip[far < 0x8000 ? far : 0x7fff] >> 4;
    f = f * f >> 5;
    u32 nf = 0x800000 - f;
    u32 g = fade * ((f * fog >> 23) + (u32)((s32)((0x28 - shade) * nf) >> 23)) >> 8;
    if (g > 0xff)
        g = 0xff;
    
    u32 k = fade * (nf >> 15), kk = k >> 8;
    u32 pix = kk <= 0x80 ? pixctab[k >> 12] : kk <= 0x100 ? pixctab[((kk - 0x80) >> 5) + 8] : kk < 0x140 ? pixctab[13] : pixctab[14];
    return (g >> 3) << 1 | (g >> 3) << 17 | pix | 0x400040;
}

static cel3d *newcel(int b, u32 img, u32 pixc, s32 depth, u32 owner, u32 aux)
{
    cel3d *c = &pool[ncel++];
    c->next = bucket[b];
    bucket[b] = c;
    if (b > maxbucket)
        maxbucket = b;
    
    c->img = img;
    c->pixc = pixc;
    c->accw = xread8(img + 0x1b) & 8;
    c->depth = depth;
    c->owner = owner;
    c->aux = aux;
    return c;
}

static void billboard(u32 img, const pt3d *p, s32 scale, u32 scr, s16 yoff, u32 tag)
{
    if (ncel >= celmax())
        return;
    
    if (scale > 0x280000)
        scale = 0x200000;
    
    s32 b = p->z >> 5;
    if (b < 0 || b >= BUCKETS)
        return;
    
    u32 pixc = (xread8(img + 0x1b) & 4) ? 0x05000500 : fogpixc(scr, b << 5, 0);
    cel3d *c = newcel(b, img, pixc, p->z, tag, (u32)(s32)yoff);
    s32 w = mulw(scale, (u16)xread16(img + 0x10)) >> 19;
    s32 top = mulw(scale, (u16)xread16(img + 0x12) + yoff) >> 18;
    s32 bot = mulw(scale, yoff) >> 18;
    s32 p8[8] = { p->x - w, p->y - top, p->x + w, p->y - top, p->x + w, p->y - bot, p->x - w, p->y - bot };
    memcpy(c->p, p8, sizeof(p8));
}

static void texquad(const pt3d *a, const pt3d *b, const pt3d *c, const pt3d *d, u32 img, s32 shade, u32 scr, u8 minh)
{
    if (ncel >= celmax())
        return;
    
    s32 z = a->z;
    if (z < b->z)
        z = b->z;
    
    if (z < c->z)
        z = c->z;
    
    if (z < d->z)
        z = d->z;
    
    if (z < 0 || (z >> 5) >= BUCKETS)
        return;
    
    u32 pixc = (xread8(img + 0x1b) & 4) ? 0x05000500 : fogpixc(scr, (z >> 5) << 5, shade);
    cel3d *q = newcel(z >> 5, img, pixc, z, scr, (u32)minh << 16);
    s32 p8[8] = { a->x, a->y, d->x, d->y, c->x, c->y, b->x, b->y };
    memcpy(q->p, p8, sizeof(p8));
}

static inline s32 mirror(s32 i, s32 n)
{
    return i < 0 ? -i : i >= n ? 2 * n - 2 - i : i;
}

static void sprcel(u32 img, const pt3d *p, s32 scale, u32 scr, u32 e, s32 hx, s32 hy)
{
    if (ncel >= celmax())
        return;
    
    if (scale > 0x280000)
        scale = 0x200000;
    
    s32 b = p->z >> 5;
    if (b < 0 || b >= BUCKETS)
        return;
    
    u32 pixc = (xread8(img + 0x1b) & 4) ? 0x05000500 : fogpixc(scr, b << 5, 0);
    cel3d *c = newcel(b, img, pixc, p->z, e, 0);
    s32 ox = (s16)xread16(img + 0x14) - hx, oy = (s16)xread16(img + 0x16) - hy;
    s32 w = (u16)xread16(img + 0x10), h = (u16)xread16(img + 0x12);
    s32 l = p->x - (mulw(scale, ox) >> 18), r = p->x + (mulw(scale, w - ox) >> 18);
    s32 t = p->y - (mulw(scale, oy) >> 18), bt = p->y + (mulw(scale, h - oy) >> 18);
    if (xread8(e + 0x25) & 1)
    {
        s32 k = l; l = r; r = k;
        c->accw = 1;
    }

    s32 p8[8] = { l, t, r, t, r, bt, l, bt };
    memcpy(c->p, p8, sizeof(p8));
}

static void mat7778(mat3d *m, u32 e0, u32 cam)
{
    memset(m, 0, sizeof(*m));
    m->m[0] = m->m[4] = m->m[8] = 0x1000;
    for (u32 e = e0; e; e = xread32(e + 8))
    {
        u8 f = xread8(e + 0x25);
        rot(m, 1, 2, (s16)xread16(((f & 2) && cam ? cam : e) + 0x18) >> 4);
        rot(m, 2, 0, (s16)xread16(((f & 4) && cam ? cam : e) + 0x1a) >> 4);
        rot(m, 0, 1, (s16)xread16(((f & 8) && cam ? cam : e) + 0x1c) >> 4);
        for (int i = 0; i < 3; i++)
            m->t[i] += xread32(e + 0xc + i * 4);
    }

    u32 chain[256];
    int n = 0;
    for (u32 c = cam; c && n < 256; c = xread32(c + 8))
        chain[n++] = c;
    
    u8 f = e0 ? xread8(e0 + 0x25) : 0;
    while (n--)
    {
        u32 c = chain[n];
        if (!(f & 0x30))
        {
            for (int i = 0; i < 3; i++)
                m->t[i] -= xread32(c + 0xc + i * 4);
            
            rot(m, 0, 1, -(s16)xread16(c + 0x1c) >> 4);
        }

        if (!(f & 0x20))
            rot(m, 2, 0, -(s16)xread16(c + 0x1a) >> 4);
        
        if (!(f & 0x10))
            rot(m, 1, 2, -(s16)xread16(c + 0x18) >> 4);
    }
}

static void elemmat(mat3d *m, u32 e)
{
    mat7778(m, e, 0);
}

// move an entity by d turned by itd own angles
void r3do_polar(u32 vram, s32 dx, s32 dy, s32 dz)
{
    if (!tables())
        return;
    mat3d m;
    mat7778(&m, vram, xread32(vram + 8));
    for (int r = 0; r < 3; r++)
    {
        s32 v = (m.m[r * 3] * dx + m.m[r * 3 + 1] * dy + m.m[r * 3 + 2] * dz) >> 12;
        xwrite32(vram + 0xc + r * 4, m.t[r] + v + (v < 0));
    }
}

static int landmap(u32 scr, u32 *L, u32 *H)
{
    s16 owner = xread16(scr + 0xc4);
    u32 vram = owner ? xread32(alis.atent + (u16)owner) : 0;
    if (!vram)
        return 0;
    
    *L = vram + (s16)xread16(scr + 0xc6);
    if (xread32(*L - 0xc00) == 0)
        return 0;
    
    *H = *L;
    if (xread8(*L - 2) & 0x80)
        *H = xread32(*L) + xread32(xread32(*L));
    
    return 1;
}

// probe box against the land of mode-5 screen
s32 r3do_maptest(u32 scr, s32 X, s32 Y, const s32 mn[3], const s32 mx[3], int walking, u32 *mat)
{
    u32 L, H;
    if (!landmap(scr, &L, &H))
        return 0;
    
    u32 S = L - 0xc00;
    s32 xs = (s16)xread16(L - 0x3c0), ys = (s16)xread16(L - 0x3be), hs = (s16)xread16(L - 0x3b8);
    s32 nx = (s16)xread16(L - 0x3cc), ny = (s16)xread16(L - 0x3ca);
    s32 ix = X >> xs, iy = Y >> ys;
    s32 cx[4] = { ix, ix + 1, ix + 1, ix }, cy[4] = { iy, iy, iy + 1, iy + 1 };
    u16 c[4];
    s32 avg = 0;
    for (int i = 0; i < 4; i++)
    {
        c[i] = xread16(H + (mirror(cx[i], nx) * ny + mirror(cy[i], ny)) * 2);
        avg += c[i] & 0xff;
    }

    avg >>= 2;
    if (walking)
    {
        u32 slot = S + ((c[0] >> 8) & 0x3f) * 0x14, img = slot + xread32(slot);
        s32 x0 = ix * (1 << xs), x1 = (ix + 1) * (1 << xs);
        s32 y0 = iy * (1 << xs), y1 = (iy + 1) * (1 << xs);
        if ((x0 < mx[0] && x1 > mn[0] && y0 < mx[1] && y1 > mn[1] && mx[2] > 0 && avg * (1 << hs) > mn[2]) || mx[2] < 0 || mn[2] < 0)
        {
            *mat = (u16)xread16(img + 8) | (u32)xread16(img + 0xa) << 16;
            return -3;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        u32 slot = S + ((c[i] >> 8) & 0x3f) * 0x14;
        if (!xread32(slot + 0xc) || xread8(slot + 9) != 2)
            continue;
        
        u32 obj = slot + 0xc + xread32(slot + 0xc);
        s32 r = (u16)xread16(obj + 0x10), ht = (u16)xread16(obj + 0x12);
        s32 ox = cx[i] * (1 << xs), oy = cy[i] * (1 << xs), oz = (c[i] & 0xff) * (1 << hs);
        if (ox - 2 * r <= mx[0] && ox + 2 * r >= mn[0] && oy - 2 * r <= mx[1] && oy + 2 * r >= mn[1] && oz <= mx[2] && oz + ht >= mn[2])
        {
            *mat = (u16)xread16(obj + 8) | (u32)xread16(obj + 0xa) << 16;
            return xread32(slot + 0x10);
        }
    }

    return 0;
}

u16 r3do_cellat(u32 vram, s32 dx, s32 dy, s32 dz, u32 L, int mode)
{
    u32 H = L;
    if (xread8(L - 2) & 0x80)
        H = xread32(L) + xread32(xread32(L));
    
    s32 xs = (s16)xread16(L - 0x3c0), ys = (s16)xread16(L - 0x3be);
    s32 nx = (s16)xread16(L - 0x3cc), ny = (s16)xread16(L - 0x3ca);
    r3do_polar(vram, dx, dy, dz);
    s32 X = 0, Y = 0;
    for (u32 e = vram; e; e = xread32(e + 8))
    {
        X += xread32(e + 0xc);
        Y += xread32(e + 0x10);
    }

    s32 ix = X >> xs, iy = Y >> ys;
    s32 m0 = mirror(ix, nx), n0 = mirror(iy, ny), m1 = mirror(ix + 1, nx), n1 = mirror(iy + 1, ny);
    u16 best = xread16(H + (m0 * ny + n0) * 2);
    if (mode == 2)
        return best;
    
    u16 k[3] = { xread16(H + (m1 * ny + n0) * 2), xread16(H + (m1 * ny + n1) * 2), xread16(H + (m0 * ny + n1) * 2) };
    for (int i = 0; i < 3; i++)
    {
        if (mode == 0 ? (k[i] & 0xff) > (best & 0xff) : (k[i] & 0xff) < (best & 0xff))
            best = k[i];
    }
    
    return best;
}

static void sprites(u32 scr, const view3d *v, s32 x0, s32 x1, s32 y0, s32 y1)
{
    u32 node = xread32(scr + 0x5e);
    if (!node)
        return;

    for (u32 e = xread32(node + 0x38); e; )
    {
        u32 next = xread32(e + 0x38);
        if (xread8(e + 0x22) == 0x32 && !(xread8(e + 0x24) & 0x40))
        {
            mat3d m;
            elemmat(&m, e);
            if (m.t[0] < x1 && x0 < m.t[0] && m.t[1] < y1 && y0 < m.t[1])
            {
                pt3d p;
                project(v, m.t[0], m.t[1], m.t[2], &p);
                s16 az = 0;
                for (u32 k = e; k; k = xread32(k + 8))
                    az += (s16)xread16(k + 0x1c);
                
                s32 n = (s8)xread8(e + 0x2e);
                u32 vi = 0;
                if (n)
                {
                    vi = (u32)((n * ((u16)(az - (s16)xread16(scr + 0x1c)) + 0x8000 & 0xffff) + 0x8000) >> 16) * 4 & 0xffff;
                    if ((s32)vi >= n * 4)
                        vi = 0;
                }

                if (xread16(e + 0x1e) == 0)
                    xwrite16(e + 0x1e, 0x100);
                
                u32 img = 0, mv = xread32(e + 0x50);
                s32 hx = 0, hy = 0;
                if (!mv)
                {
                    u32 r = xread32(e + 0x3c) + vi;
                    img = r + xread32(r);
                }
                else
                {
                    u32 r = mv + vi, t6 = r + xread32(r);
                    if (t6 >= alis.platform.ram_sz - 0x20 || xread8(t6) != 6)
                        goto skip;
                    
                    s16 k = (s16)xread16(t6 + 0xe);
                    img = r + k * 4 + xread32(r + k * 4);
                    xwrite8(e + 0x25, xread8(t6 + 0xc) == 1 ? xread8(e + 0x25) | 1 : xread8(e + 0x25) & 0xfe);
                    xwrite16(e + 0x54, xread16(t6 + 0x12));
                    xwrite16(e + 0x56, xread16(t6 + 0x1a));
                    hx = (s16)xread16(e + 0x54);
                    hy = (s16)xread16(e + 0x56);
                }

                if (img)
                    sprcel(img, &p, sdiv(p.z, (s16)xread16(e + 0x1e) * v->fy * 0x400), scr, e, hx, hy);
            }
        }

skip:
        if (next == e)
            break;
        
        e = next;
    }
}


void r3do_land(u32 scr)
{
    if (!tables())
        return;
    memset(bucket, 0, sizeof(bucket));
    ncel = 0;
    maxbucket = -1;
    drewland = 0;

    s16 owner = xread16(scr + 0xc4);
    if (owner == 0)
        return;
    
    u32 vram = xread32(alis.atent + (u16)owner);
    if (vram == 0)
        return;
    
    u32 L = vram + (s16)xread16(scr + 0xc6);
    u32 slots = L - 0xc00;
    if (xread32(slots) == 0)
        return;
    
    u32 H = L;
    if (xread8(L - 2) & 0x80)
        H = xread32(L) + xread32(xread32(L));
    
    int sh = (s8)xread8(L - 0x3bf);
    s32 cc = (s16)xread16(L - 0x3cc), rc = (s16)xread16(L - 0x3ca);
    for (int i = 0; i < tilemax(); i++)
        cache[i].valid = 0;

    mat3d tocam;
    view3d v;
    cammat(&tocam, scr, 0);
    cammat(&v.m, scr, 1);
    v.cx = (u16)xread16(scr + 0x62);
    v.cy = (u16)xread16(scr + 0x64);
    v.fx = (s16)xread16(scr + 0x48);
    v.fy = (s16)xread16(scr + 0x4c);
    v.near = (s32)xread32(scr + 0x142);
    s32 far = viewfar(scr);

    // view triangle (eye, far right, far left) in world space
    s32 half = sdiv(v.fx, v.cx * far);
    s32 p1[3] = { half, far, 0 }, p2[3] = { -half, far, 0 }, p3[3] = { 0, 0, 0 };
    xform(&tocam, p1);
    xform(&tocam, p2);
    xform(&tocam, p3);
    s32 ymin = p1[1], ymax = p1[1], xmin = p1[0], xmax = p1[0];
    if (p2[1] < ymin)
        ymin = p2[1];
    
    if (p3[1] < ymin)
        ymin = p3[1];
    
    if (p2[1] > ymax)
        ymax = p2[1];
    
    if (p3[1] > ymax)
        ymax = p3[1];
    
    if (p2[0] < xmin)
        xmin = p2[0];
    
    if (p3[0] < xmin)
        xmin = p3[0];
    
    if (p2[0] > xmax)
        xmax = p2[0];
    
    if (p3[0] > xmax)
        xmax = p3[0];
    
    s32 d1x = p1[0] - p3[0], d1y = p1[1] - p3[1];
    s32 d2x = p2[0] - p3[0], d2y = p2[1] - p3[1];
    s32 d3x = p2[0] - p1[0], d3y = p2[1] - p1[1];

    // per tile row, columns triangle crosses at the sampled lines
    s32 r0 = ymin >> sh, r1 = ymax >> sh, eyerow = p3[1] >> sh;
    s32 e4 = xmin >> sh, e8 = xmax >> sh;
    if (r1 - r0 + 1 > tilemax())
        return;
    
    s32 ex1 = 0, ex2 = 0, ex3 = 0, prevlo = 0, prevhi = 0;
    int first = 1;
    for (s32 r = r0; r <= r1; r++)
    {
        for (int pass = (r == eyerow) ? 0 : 1; pass < 2; pass++)
        {
            s32 y = pass == 0 ? p3[1] : r == r1 ? r * (1 << sh) : (r + 1) * (1 << sh);
            if (d1y)
                ex1 = sdiv(d1y, mulw(d1x, y - p1[1])) + p1[0];
            
            if (d2y)
                ex2 = sdiv(d2y, mulw(d2x, y - p2[1])) + p2[0];
            
            if (d3y)
                ex3 = sdiv(d3y, mulw(d3x, y - p2[1])) + p2[0];
            
            int ok1 = ex1 >= xmin && ex1 <= xmax && d1y;
            int ok2 = ex2 >= xmin && ex2 <= xmax && d2y;
            int ok3 = ex3 >= xmin && ex3 <= xmax && d3y;
            s32 a = ex2, b = ex3;
            if (ok1 && !ok2)
            {
                a = ex1;
            }
            else if (ok1)
            {
                a = ex1, b = ex2;
            }

            s32 lo = a < b ? a : b, hi = a < b ? b : a;
            if (ok1 && ok2 && ok3)
            {
                if (ex3 < lo)
                    lo = ex3;
                
                if (ex3 > hi)
                    hi = ex3;
            }

            lo >>= sh;
            hi >>= sh;
            if (lo == hi)
                hi++, lo--;
            
            s32 rawlo = lo, rawhi = hi;
            if (!first)
            {
                if (hi < prevhi)
                    hi = prevhi;
                
                if (prevlo < lo)
                    lo = prevlo;
            }

            first = 0;
            spans[r - r0][0] = lo;
            spans[r - r0][1] = hi;
            if (lo < e4)
                e4 = lo;
            
            if (hi > e8)
                e8 = hi;
            
            prevlo = rawlo;
            prevhi = rawhi;
        }
    }

    s32 f0 = e8 - e4 + 2;
    if (f0 * (r1 + 2 - r0) > tilemax())
        return;
    
    drewland = 1;
    sprites(scr, &v, (e4 - 1) * (1 << sh), (e8 + 1) * (1 << sh), (r0 - 1) * (1 << sh), (r1 + 1) * (1 << sh));
    s32 camx = xread32(scr + 0xc), camy = xread32(scr + 0x10), camz = xread32(scr + 0x14);
    s32 clx1 = (u16)xread16(scr + 0x66), cly1 = (u16)xread16(scr + 0x68);
    s32 clx2 = (u16)xread16(scr + 0x6a), cly2 = (u16)xread16(scr + 0x6c);
    s32 bscale = (s16)xread16(scr + 0x4c) * 0x2a800;
    for (s32 r = r0; r <= r1; r++)
    {
        s32 rowa = f0 * (r - r0), rowb = f0 * (r + 1 - r0);
        s32 rm0 = mirror(r, rc), rm1 = mirror(r + 1, rc);
        s32 y0 = r * (1 << sh), y1 = (r + 1) * (1 << sh);
        for (s32 col = spans[r - r0][0]; col <= spans[r - r0][1]; col++)
        {
            pt3d *q00 = &cache[col - e4 + rowa + 1], *q10 = &cache[col + 1 - e4 + rowa + 1];
            pt3d *q11 = &cache[col + 1 - e4 + rowb + 1], *q01 = &cache[col - e4 + rowb + 1];
            s32 cm0 = mirror(col, cc), cm1 = mirror(col + 1, cc);
            u16 c00 = xread16(H + (rc * cm0 + rm0) * 2), c10 = xread16(H + (rc * cm1 + rm0) * 2);
            u16 c11 = xread16(H + (rc * cm1 + rm1) * 2), c01 = xread16(H + (rc * cm0 + rm1) * 2);
            s32 h00 = c00 & 0xff, h10 = c10 & 0xff, h11 = c11 & 0xff, h01 = c01 & 0xff;
            s32 x0 = col * (1 << sh), x1 = (col + 1) * (1 << sh);
            u32 slot = slots + ((c00 >> 8) & 0x3f) * 0x14;
            u32 obj = xread32(slot + 0xc) ? slot + 0xc + xread32(slot + 0xc) : 0;
            int front = mulw(y0 - camy, h00 - h01) + mulw(h00 - h10, x0 - camx) + (h00 - camz) * (1 << sh) < 0 || mulw(h01 - h11, x1 - camx) + mulw(h10 - h11, y1 - camy) + (h11 - camz) * (1 << sh) < 0;
            if (!q00->valid)
                project(&v, x0, y0, h00, q00);
            
            if (!front)
            {
                if (obj)
                    billboard(obj, q00, sdiv(q00->z, bscale), scr, (s16)xread16(slot + 0xa), xread32(slot + 0x10));
                
                continue;
            }

            if (!q10->valid)
                project(&v, x1, y0, h10, q10);
            
            if (!q11->valid)
                project(&v, x1, y1, h11, q11);
            
            if (!q01->valid)
                project(&v, x0, y1, h01, q01);
            
            if (obj)
                billboard(obj, q00, sdiv(q00->z, bscale), scr, (s16)xread16(slot + 0xa), xread32(slot + 0x10));
            
            if ((q00->y < cly1 && q10->y < cly1 && q11->y < cly1 && q01->y < cly1) || (q00->y > cly2 && q10->y > cly2 && q11->y > cly2 && q01->y > cly2) || (q00->x < clx1 && q10->x < clx1 && q11->x < clx1 && q01->x < clx1) || (q00->x > clx2 && q10->x > clx2 && q11->x > clx2 && q01->x > clx2))
                continue;
            
            if (q00->x == q10->x && q11->x == q01->x && q00->y == q10->y && q11->y == q01->y)
                continue;
            
            if (xread32(slot))
            {
                s32 shade = ((c00 >> 14) + (c10 >> 14) + (c11 >> 14) + (c01 >> 14)) >> 1;
                s32 minh = h00 < h10 ? h00 : h10;
                if (h11 < minh)
                    minh = h11;
                
                if (h01 < minh)
                    minh = h01;
                
                texquad(q00, q10, q11, q01, slot + xread32(slot), shade * 10 - 0x14, scr, minh);
            }
        }
    }
}

static void texsig(u32 img, u32 len, u8 *sig)
{
    if (len < 16 || img + len > alis.platform.ram_sz)
        len = 16;

    memcpy(sig, alis.mem + img, 32);
    memcpy(sig + 32, alis.mem + img + len - 16, 16);
}

static void texflush(void)
{
    for (int i = 0; i < ntex; i++)
        free(tex[i].px);

    ntex = 0;
    texbytes = 0;
}

static const u16 *texget(u32 img, s32 *w, s32 *h)
{
    u32 len = read32(alis.mem + img + 4);
    u8 sig[48];
    texsig(img, len, sig);
    int slot = -1;
    for (int i = 0; i < ntex; i++)
    {
        if (tex[i].img != img)
            continue;

        if (tex[i].len == len && memcmp(tex[i].sig, sig, sizeof(sig)) == 0)
        {
            *w = tex[i].w;
            *h = tex[i].h;
            return tex[i].px;
        }

        // stale: same address, different cel
        free(tex[i].px);
        texbytes -= (u32)(tex[i].w * tex[i].h) * 2;
        slot = i;
        break;
    }

    u16 *px = cel_decode(img, w, h);
    if (!px)
    {
        if (slot >= 0)
            tex[slot] = tex[--ntex];

        return NULL;
    }

    u32 sz = (u32)(*w * *h) * 2;
    if (slot < 0 && (ntex >= TEXMAX || texbytes + sz > TEXBYTES))
        texflush();

    if (slot < 0)
        slot = ntex++;

    tex[slot].img = img;
    tex[slot].len = len;
    memcpy(tex[slot].sig, sig, sizeof(sig));
    tex[slot].w = *w;
    tex[slot].h = *h;
    tex[slot].px = px;
    texbytes += sz;
    return px;
}

static inline u32 rgb5(u32 r, u32 g, u32 b)
{
    return 0xff000000u | ((r << 3 | r >> 2) << 16) | ((g << 3 | g >> 2) << 8) | (b << 3 | b >> 2);
}

static float edge(const s32 *p, int a, int b)
{
    return hypotf((float)(p[b] - p[a]), (float)(p[b + 1] - p[a + 1]));
}

// MapCel corners as p = a + e*u + f*v + g*u*v, plus the ~1.5 pixel overlap per texture axis
// (to keep neighbouring quads from showing seams)
typedef struct { float ax, ay, ex, ey, fx, fy, gx, gy, k2, eu, ev; } celmap;

static void celsetup(const cel3d *c, celmap *m)
{
    m->ax = c->p[0], m->ay = c->p[1];
    m->ex = c->p[2] - m->ax, m->ey = c->p[3] - m->ay;
    m->fx = c->p[6] - m->ax, m->fy = c->p[7] - m->ay;
    m->gx = m->ax - c->p[2] + c->p[4] - c->p[6], m->gy = m->ay - c->p[3] + c->p[5] - c->p[7];
    m->k2 = m->gx * m->fy - m->gy * m->fx;
    m->eu = 1.5f / fmaxf(fminf(edge(c->p, 0, 2), edge(c->p, 6, 4)), 1.0f);
    m->ev = 1.5f / fmaxf(fminf(edge(c->p, 0, 6), edge(c->p, 2, 4)), 1.0f);
}

static void celbox(const cel3d *c, s32 *x1, s32 *y1, s32 *x2, s32 *y2)
{
    *x1 = *x2 = c->p[0];
    *y1 = *y2 = c->p[1];
    for (int i = 2; i < 8; i += 2)
    {
        if (c->p[i] < *x1)
            *x1 = c->p[i];
        
        if (c->p[i] > *x2)
            *x2 = c->p[i];
        
        if (c->p[i + 1] < *y1)
            *y1 = c->p[i + 1];
        
        if (c->p[i + 1] > *y2)
            *y2 = c->p[i + 1];
    }
}

// texel index under pixel (xx, yy), or -1
static s32 texel(float u, float v, s32 w, s32 h)
{
    s32 tx = (s32)(u * w), ty = (s32)(v * h);
    tx = tx < 0 ? 0 : tx >= w ? w - 1 : tx;
    ty = ty < 0 ? 0 : ty >= h ? h - 1 : ty;
    return ty * w + tx;
}

// barycentric weights of (px,py) for b and c in triangle abc
static float bary(float px, float py, const s32 *a, const s32 *b, const s32 *c, float *lb, float *lc)
{
    float d = (float)(b[0] - a[0]) * (c[1] - a[1]) - (float)(c[0] - a[0]) * (b[1] - a[1]);
    if (fabsf(d) < 1e-6f)
        return 0;
    *lb = ((px - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (py - a[1])) / d;
    *lc = ((b[0] - a[0]) * (py - a[1]) - (px - a[0]) * (b[1] - a[1])) / d;
    return d;
}

// pixel within e screen pixels of triangle abc (weights lb, lc; d = signed area x2)
static int intri(const s32 *a, const s32 *b, const s32 *c, float d, float lb, float lc, float e)
{
    float k = e / fabsf(d);
    float la = 1 - lb - lc;
    return la >= -k * hypotf(c[0] - b[0], c[1] - b[1]) && lb >= -k * hypotf(c[0] - a[0], c[1] - a[1]) && lc >= -k * hypotf(b[0] - a[0], b[1] - a[1]);
}

// non-convex quad leaves pixels with no bilinear solution, which 3DOs forward mapping still covers: map those through the quad's two triangles instead
static s32 celtri(const cel3d *c, float px, float py, s32 w, s32 h, float e)
{
    const s32 *p = c->p;
    float lb, lc, d = bary(px, py, p, p + 2, p + 4, &lb, &lc);
    if (d != 0 && (d > 0 || c->accw) && intri(p, p + 2, p + 4, d, lb, lc, e))
        return texel(lb + lc, lc, w, h);
    
    d = bary(px, py, p, p + 4, p + 6, &lb, &lc);
    if (d != 0 && (d > 0 || c->accw) && intri(p, p + 4, p + 6, d, lb, lc, e))
        return texel(lb, lb + lc, w, h);
    
    return -1;
}

// texel index under pixel (xx, yy), or -1
static s32 celtexel(const cel3d *c, const celmap *m, s32 w, s32 h, s32 xx, s32 yy)
{
    float hx = xx + 0.5f - m->ax, hy = yy + 0.5f - m->ay;
    float k1 = m->ex * m->fy - m->ey * m->fx + hx * m->gy - hy * m->gx;
    float k0 = hx * m->ey - hy * m->ex;
    float u, v;
    if (fabsf(m->k2) < 1e-4f)
    {
        if (fabsf(k1) < 1e-6f)
            return celtri(c, xx + 0.5f, yy + 0.5f, w, h, 1.5f);
        
        v = -k0 / k1;
    }
    else
    {
        float disc = k1 * k1 - 4 * k0 * m->k2;
        if (disc < 0)
            return celtri(c, xx + 0.5f, yy + 0.5f, w, h, 1.5f);
        
        disc = sqrtf(disc);
        v = (-k1 - disc) / (2 * m->k2);
        if (v < -m->ev || v > 1 + m->ev)
            v = (-k1 + disc) / (2 * m->k2);
    }

    float dx = m->ex + m->gx * v, dy = m->ey + m->gy * v;
    u = fabsf(dx) > fabsf(dy) ? (hx - m->fx * v) / dx : (hy - m->fy * v) / dy;
    if (!(u >= -m->eu && u <= 1 + m->eu && v >= -m->ev && v <= 1 + m->ev))
        return celtri(c, xx + 0.5f, yy + 0.5f, w, h, 1.5f);
    
    float wind = (m->ex + m->gx * v) * (m->fy + m->gy * u) - (m->ey + m->gy * v) * (m->fx + m->gx * u);
    if (wind < 0 && !c->accw)
        return -1;
    
    return texel(u, v, w, h);
}

// pixc as a per-channel table (5-bit in, 8-bit out, already shifted into ARGB; alpha in red)
static u32 pixlut[3][32], pixlutc = 1;

// 3DO PIXC per channel: min(31, c * mf / df + av) (halved if bit 0); df is a power of two
static void pixc_lut(u32 pixc)
{
    if (pixc == pixlutc)
        return;

    static const u8 dsh[4] = { 4, 1, 2, 3 };
    pixlutc = pixc;
    u32 mf = ((pixc >> 10) & 7) + 1, sh = dsh[(pixc >> 8) & 3];
    u32 av = ((pixc >> 6) & 3) == 1 ? (pixc >> 1) & 0x1f : 0;
    for (u32 i = 0; i < 32; i++)
    {
        u32 v = ((i * mf) >> sh) + av;
        if (pixc & 1)
            v >>= 1;

        if (v > 31)
            v = 31;

        u32 e = v << 3 | v >> 2;
        pixlut[0][i] = 0xff000000u | e << 16;
        pixlut[1][i] = e << 8;
        pixlut[2][i] = e;
    }
}

// one quad edge stepped per scanline: x in 24.8, u/v in texels 16.16
typedef struct { s32 y0, y1, x, dx, u, du, v, dv; } celedge;

__attribute__((noinline)) static int celedge_set(celedge *e, const s32 *a, const s32 *b, s32 ua, s32 va, s32 ub, s32 vb, s32 ystart)
{
    if (a[1] == b[1])
        return 0;

    if (a[1] > b[1])
    {
        const s32 *t = a; a = b; b = t;
        s32 k = ua; ua = ub; ub = k;
        k = va; va = vb; vb = k;
    }

    s32 dy = b[1] - a[1];
    e->y0 = a[1];
    e->y1 = b[1];
    e->dx = (s32)(((s64)(b[0] - a[0]) << 8) / dy);
    e->du = (ub - ua) / dy;
    e->dv = (vb - va) / dy;
    // first scanline centre at or below the top vertex, or the clip top
    s32 y = ystart > a[1] ? ystart : a[1];
    s32 k = y - a[1];
    e->x = (a[0] << 8) + e->dx * k + e->dx / 2;
    e->u = ua + e->du * k + e->du / 2;
    e->v = va + e->dv * k + e->dv / 2;
    return 1;
}

// texture row pointers of the cel being drawn (asm span loop indexes them instead of multiplying)
#define CELROWS 512
static const u16 *celrows[CELROWS];

#if defined(ALIS_USE_NATIVE_ATARI) && defined(__m68k__) && !defined(__mcoldfire__)
// n >= 1 texels, u/v in range: d[i] = pixc(tex[v][u]) unless transparent
static void celrun(u32 *d, s32 u, s32 du, s32 v, s32 dv, s32 n)
{
    const u16 *const *rows = celrows;
    const u32 *lut = &pixlut[0][0];
    s32 t, c, o;
    const u16 *row;
    n--;
    __asm__ volatile (
    "0: move.l  %[v],%[t]                  \n"
    "   swap    %[t]                       \n"
    "   move.l  %[rows]@(0,%[t]:w:4),%[row] \n"  // row = rows[v >> 16]
    "   move.l  %[u],%[t]                  \n"
    "   swap    %[t]                       \n"
    "   move.w  %[row]@(0,%[t]:w:2),%[t]   \n"  // texel, 0 = transparent
    "   beq     1f                         \n"
    "   move.w  %[t],%[c]                  \n"
    "   and.w   #31,%[c]                   \n"
    "   move.l  %[lut]@(256,%[c]:w:4),%[o] \n"  // blue
    "   lsr.w   #5,%[t]                    \n"
    "   move.w  %[t],%[c]                  \n"
    "   and.w   #31,%[c]                   \n"
    "   or.l    %[lut]@(128,%[c]:w:4),%[o] \n"  // green
    "   lsr.w   #5,%[t]                    \n"
    "   and.w   #31,%[t]                   \n"
    "   or.l    %[lut]@(0,%[t]:w:4),%[o]   \n"  // red + alpha
    "   move.l  %[o],%[d]@                 \n"
    "1: addq.l  #4,%[d]                    \n"
    "   add.l   %[du],%[u]                 \n"
    "   add.l   %[dv],%[v]                 \n"
    "   dbf     %[n],0b                    \n"
    : [d]"+a"(d), [u]"+d"(u), [v]"+d"(v), [n]"+d"(n),
      [t]"=&d"(t), [c]"=&d"(c), [o]"=&d"(o), [row]"=&a"(row)
    : [rows]"a"(rows), [lut]"a"(lut), [du]"d"(du), [dv]"d"(dv)
    : "cc", "memory"
    );
}
#endif

static inline void celpix(u32 *d, const u16 *px, s32 w, s32 h, s32 u, s32 v)
{
    s32 tu = u < 0 ? 0 : u >= w << 16 ? w - 1 : u >> 16;
    s32 tv = v < 0 ? 0 : v >= h << 16 ? h - 1 : v >> 16;
    u16 t = px[tv * w + tu];
    if (t)
        *d = pixlut[0][(t >> 10) & 0x1f] | pixlut[1][(t >> 5) & 0x1f] | pixlut[2][t & 0x1f];
}

// one textured span [xs, xe) on row y, clipped to [cx1, cx2]; the ends (overlap pixels) are
// clamped, the rest runs unclamped
__attribute__((optimize("no-unroll-loops"))) static void celspan(const u16 *px, s32 w, s32 h, s32 y, s32 xs, s32 xe, s32 u, s32 v, s32 du, s32 dv, s32 cx1, s32 cx2)
{
    u32 U = (u32)w << 16, V = (u32)h << 16;
    if (xs < cx1)
    {
        u += du * (cx1 - xs);
        v += dv * (cx1 - xs);
        xs = cx1;
    }

    if (xe > cx2 + 1)
        xe = cx2 + 1;

    s32 n = xe - xs;
    u32 *d = image.rgblogic + y * alis.platform.width + xs;
    for (int k = 0; k < 2 && n > 0 && ((u32)u >= U || (u32)v >= V); k++, n--, d++, u += du, v += dv)
        celpix(d, px, w, h, u, v);

    for (int k = 0; k < 2 && n > 0 && ((u32)(u + du * (n - 1)) >= U || (u32)(v + dv * (n - 1)) >= V); k++, n--)
        celpix(d + n - 1, px, w, h, u + du * (n - 1), v + dv * (n - 1));

    if (n <= 0)
        return;

    if ((u32)u >= U || (u32)v >= V || (u32)(u + du * (n - 1)) >= U || (u32)(v + dv * (n - 1)) >= V)
    {
        for (; n > 0; n--, d++, u += du, v += dv)
            celpix(d, px, w, h, u, v);

        return;
    }

#if defined(ALIS_USE_NATIVE_ATARI) && defined(__m68k__) && !defined(__mcoldfire__)
    if (h <= CELROWS)
    {
        celrun(d, u, du, v, dv, n);
        return;
    }
#endif

    for (; n > 0; n--, d++, u += du, v += dv)
    {
        u16 t = px[(v >> 16) * w + (u >> 16)];
        if (t)
            *d = pixlut[0][(t >> 10) & 0x1f] | pixlut[1][(t >> 5) & 0x1f] | pixlut[2][t & 0x1f];
    }
}

// cel being drawn
static struct { const u16 *px; s32 w, h, cx1, cy1, cx2, cy2, ytop, ybot, ymin, ymax; } cc;

// span between edges l and r (any order) on row y, plus the overlap row above/below the cel
__attribute__((noinline)) static void celrow(const celedge *l, const celedge *r, s32 y)
{
    if (l->x > r->x)
    {
        const celedge *t = l; l = r; r = t;
    }

    // 1 pixel overlap all round hides T-junction cracks between terrain quads (old 1.5 px margin)
    s32 xs = ((l->x - 128 + 255) >> 8) - 1, xe = ((r->x - 128 + 255) >> 8) + 1;
    s32 span = (r->x - l->x) >> 4;
    s32 du = span > 0 ? (s32)(((r->u - l->u) << 4) / span) : 0;
    s32 dv = span > 0 ? (s32)(((r->v - l->v) << 4) / span) : 0;
    s32 f = (xs << 8) + 128 - l->x;
    s32 u = l->u + (du >> 8) * f, v = l->v + (dv >> 8) * f;
    celspan(cc.px, cc.w, cc.h, y, xs, xe, u, v, du, dv, cc.cx1, cc.cx2);
    if (y == cc.ymin && y - 1 >= cc.cy1 && cc.ymin == cc.ytop)
        celspan(cc.px, cc.w, cc.h, y - 1, xs, xe, u, v, du, dv, cc.cx1, cc.cx2);

    if (y == cc.ymax - 1 && y + 1 <= cc.cy2 && cc.ymax == cc.ybot)
        celspan(cc.px, cc.w, cc.h, y + 1, xs, xe, u, v, du, dv, cc.cx1, cc.cx2);
}

static inline void celstep(celedge *e)
{
    e->x += e->dx;
    e->u += e->du;
    e->v += e->dv;
}

static const u8 celcu[4] = { 0, 1, 1, 0 }, celcv[4] = { 0, 0, 1, 1 };

// edge from corner i to corner n, starting at row y; 0 if horizontal
static int celedge_at(celedge *e, const s32 *p, int i, int n, s32 y)
{
    s32 U = (cc.w << 16) - 1, V = (cc.h << 16) - 1;
    return celedge_set(e, p + i * 2, p + n * 2, celcu[i] ? U : 0, celcv[i] ? V : 0, celcu[n] ? U : 0, celcv[n] ? V : 0, y);
}

// convex quad: one left and one right chain from the top corner
__attribute__((noinline, optimize("no-unroll-loops"))) static void celconvex(const s32 *p)
{
    int top = 0;
    for (int i = 1; i < 4; i++)
    {
        if (p[i * 2 + 1] < p[top * 2 + 1])
            top = i;
    }

    celedge a, b;
    int ai = top, bi = top;
    s32 y = cc.ymin;
    a.y1 = b.y1 = y - 0x8000;
    while (y < cc.ymax)
    {
        // next edge on each chain once the current one ends (horizontal ones are skipped)
        for (int k = 0; k < 4 && y >= a.y1; k++)
        {
            int n = (ai + 3) & 3;
            if (!celedge_at(&a, p, ai, n, y) || a.y1 <= y)
                a.y1 = y - 0x8000;

            ai = n;
        }

        for (int k = 0; k < 4 && y >= b.y1; k++)
        {
            int n = (bi + 1) & 3;
            if (!celedge_at(&b, p, bi, n, y) || b.y1 <= y)
                b.y1 = y - 0x8000;

            bi = n;
        }

        if (y >= a.y1 || y >= b.y1)
            return;

        s32 end = a.y1 < b.y1 ? a.y1 : b.y1;
        if (end > cc.ymax)
            end = cc.ymax;

        for (; y < end; y++)
        {
            celrow(&a, &b, y);
            celstep(&a);
            celstep(&b);
        }
    }
}

// concave or bowtie quad: even-odd over all four edges
__attribute__((noinline, optimize("no-unroll-loops"))) static void celgeneral(const s32 *p)
{
    celedge e[4];
    int ne = 0;
    for (int i = 0; i < 4; i++)
        ne += celedge_at(&e[ne], p, i, (i + 1) & 3, cc.ymin);

    for (s32 y = cc.ymin; y < cc.ymax; y++)
    {
        celedge *x[4];
        int nx = 0;
        for (int i = 0; i < ne; i++)
        {
            if (y < e[i].y0 || y >= e[i].y1)
                continue;

            int k = nx++;
            while (k > 0 && x[k - 1]->x > e[i].x)
            {
                x[k] = x[k - 1];
                k--;
            }

            x[k] = &e[i];
        }

        for (int k = 0; k + 1 < nx; k += 2)
            celrow(x[k], x[k + 1], y);

        for (int i = 0; i < ne; i++)
        {
            if (y >= e[i].y0 && y < e[i].y1)
                celstep(&e[i]);
        }
    }
}

// 3DO MapCel quad, scanline filled; u/v exact on the edges, linear across each span. Pixel
// centres decide coverage, so shared edges leave no seams.
__attribute__((noinline)) static void drawcel(const cel3d *c, s32 cx1, s32 cy1, s32 cx2, s32 cy2)
{
    s32 w, h;
    const u16 *px = texget(c->img, &w, &h);
    if (!px)
        return;

    const s32 *p = c->p;
    s32 area = 0, ymin = p[1], ymax = p[1];
    int pos = 0, neg = 0;
    for (int i = 0; i < 4; i++)
    {
        const s32 *a = p + i * 2, *b = p + ((i + 1) & 3) * 2, *d = p + ((i + 2) & 3) * 2;
        area += a[0] * b[1] - b[0] * a[1];
        s32 z = (b[0] - a[0]) * (d[1] - b[1]) - (b[1] - a[1]) * (d[0] - b[0]);
        pos |= z > 0;
        neg |= z < 0;
        if (a[1] < ymin)
            ymin = a[1];

        if (a[1] > ymax)
            ymax = a[1];
    }

    if (area == 0 || (area < 0 && !c->accw))
        return;

    cc.ytop = ymin;
    cc.ybot = ymax;
    cc.ymin = ymin < cy1 ? cy1 : ymin;
    cc.ymax = ymax > cy2 + 1 ? cy2 + 1 : ymax;
    if (cc.ymin >= cc.ymax)
        return;

    cc.px = px;
    cc.w = w;
    cc.h = h;
    cc.cx1 = cx1;
    cc.cy1 = cy1;
    cc.cx2 = cx2;
    cc.cy2 = cy2;
    pixc_lut(c->pixc);
    for (s32 i = 0; i < h && i < CELROWS; i++)
        celrows[i] = px + i * w;

    if (pos && neg)
    {
        celgeneral(p);
    }
    else
    {
        celconvex(p);
    }
}

u32 r3do_pick(u32 scr, s32 x, s32 y, s32 w3[3], u32 *aux, u32 *img)
{
    cel3d *best = NULL;
    s32 bw = 0x7fffffff, bh = 0x7fffffff;
    int hit = 0;
    for (int b = 0; b <= maxbucket && !hit; b++)
    {
        for (cel3d *c = bucket[b]; c; c = c->next)
        {
            s32 x1, y1, x2, y2;
            celbox(c, &x1, &y1, &x2, &y2);
            if (x < x1 || x > x2 || y < y1 || y > y2)
                continue;
            
            s32 w, h;
            const u16 *px = texget(c->img, &w, &h);
            celmap m;
            celsetup(c, &m);
            s32 t = px ? celtexel(c, &m, w, h, x, y) : -1;
            hit = t >= 0 && px[t];
            if (hit && (!best || c->depth < best->depth || (c->depth == best->depth && (x2 - x1 < bw || y2 - y1 < bh))))
            {
                best = c;
                bw = x2 - x1;
                bh = y2 - y1;
            }
        }
    }
    
    if (!best)
        return 0;
    
    s32 d = best->depth;
    s32 v[3] = { sdiv((s16)xread16(scr + 0x48), mulw(d, x - (u16)xread16(scr + 0x62))), d, sdiv((s16)xread16(scr + 0x4c), mulw(d, (u16)xread16(scr + 0x64) - y)) };
    mat3d m;
    cammat(&m, scr, 0);
    xform(&m, v);
    if ((s32)best->aux >> 16)
        v[2] = (s32)best->aux >> 16;
    
    memcpy(w3, v, sizeof(v));
    *aux = best->aux;
    *img = best->img;
    return best->owner;
}

// nothing to draw since the last r3do_land
int r3do_empty(void)
{
    return !drewland && maxbucket < 0;
}

__attribute__((noinline)) static void fogfill(u32 col, s32 x1, s32 y1, s32 x2, s32 y2)
{
    s32 W = alis.platform.width;
    for (s32 yy = y1; yy <= y2; yy++)
    {
        u32 *d = image.rgblogic + yy * W + x1;
        for (s32 n = x2 - x1 + 1; n > 0; n--)
            *d++ = col;
    }
}

void r3do_flush(u32 scr)
{
    if (!image.rgblogic || !tables())
        return;
    
    s32 W = alis.platform.width, H = alis.platform.height;
    s32 x1 = (s16)xread16(scr + 0x66), y1 = (s16)xread16(scr + 0x68);
    s32 x2 = (s16)xread16(scr + 0x6a), y2 = (s16)xread16(scr + 0x6c);
    if (x1 < 0)
        x1 = 0;
    
    if (y1 < 0)
        y1 = 0;
    
    if (x2 >= W)
        x2 = W - 1;
    
    if (y2 >= H)
        y2 = H - 1;
    
    if (drewland)
    {
        u32 g = (alis.fog3do > 0xff ? 0xff : alis.fog3do) >> 3;
        fogfill(rgb5(g, g, g), x1, y1, x2, y2);
    }

    for (int b = maxbucket; b >= 0; b--)
    {
        for (cel3d *c = bucket[b]; c; c = c->next)
            drawcel(c, x1, y1, x2, y2);
    }
}
