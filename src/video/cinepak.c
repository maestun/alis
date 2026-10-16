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

#include "cinepak.h"

static u32 rd16(const u8 *p) { return p[0] << 8 | p[1]; }
static u32 rd24(const u8 *p) { return (u32)p[0] << 16 | p[1] << 8 | p[2]; }
static u32 rd32(const u8 *p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | p[2] << 8 | p[3]; }

static u32 clamp8(int v) { return v < 0 ? 0 : v > 255 ? 255 : (u32)v; }

static u32 yuv(int y, int u, int v)
{
    return 0xff000000u | clamp8(y + v * 2) << 16 | clamp8(y - u / 2 - v) << 8 | clamp8(y + u * 2);
}

// Codebook chunk 0x20..0x27: +1 partial (flag words), +4 grey (no u/v)
static void cv_book(cvent *cb, int id, const u8 *p, const u8 *e)
{
    int n = (id & 4) ? 4 : 6;
    u32 flag = 0, mask = 0;
    for (int i = 0; i < 256; i++)
    {
        if ((id & 1) && !(mask >>= 1))
        {
            if (p + 4 > e)
                return;
            
            flag = rd32(p); p += 4; mask = 0x80000000u;
        }

        if ((id & 1) && !(flag & mask))
            continue;
        
        if (p + n > e)
            return;
        
        int u = n == 6 ? (s8)p[4] : 0, v = n == 6 ? (s8)p[5] : 0;
        for (int k = 0; k < 4; k++)
            cb[i].c[k] = yuv(p[k], u, v);
        
        p += n;
    }
}

static void put_v1(u32 *fb, int W, int H, int x, int y, const cvent *e)
{
    for (int dy = 0; dy < 4 && y + dy < H; dy++)
    {
        for (int dx = 0; dx < 4 && x + dx < W; dx++)
            fb[(y + dy) * W + x + dx] = e->c[(dy >> 1) * 2 + (dx >> 1)];
    }
}

static void put_v4(u32 *fb, int W, int H, int x, int y, const cvent *q[4])
{
    for (int dy = 0; dy < 4 && y + dy < H; dy++)
    {
        for (int dx = 0; dx < 4 && x + dx < W; dx++)
            fb[(y + dy) * W + x + dx] = q[(dy >> 1) * 2 + (dx >> 1)]->c[(dy & 1) * 2 + (dx & 1)];
    }
}

// Vector chunk 0x30 (key, V1/V4 flags), 0x31 (+skip flags), 0x32 (all V1)
static void cv_vec(const cvbook *b, int id, const u8 *p, const u8 *e, u32 *fb, int W, int H, int x1, int y1, int x2, int y2)
{
    u32 flag = 0, mask = 0;
    for (int y = y1; y < y2; y += 4)
        for (int x = x1; x < x2; x += 4)
        {
            if ((id & 1) && !(mask >>= 1))
            {
                if (p + 4 > e)
                    return;
                
                flag = rd32(p); p += 4; mask = 0x80000000u;
            }

            if ((id & 1) && !(flag & mask))
                continue;
            
            if (!(id & 2) && !(mask >>= 1))
            {
                if (p + 4 > e)
                    return;
                
                flag = rd32(p); p += 4; mask = 0x80000000u;
            }

            if ((id & 2) || !(flag & mask))
            {
                if (p >= e)
                    return;
                
                put_v1(fb, W, H, x, y, &b->v1[*p++]);
            }
            else
            {
                if (p + 4 > e)
                    return;
                
                const cvent *q[4] = { &b->v4[p[0]], &b->v4[p[1]], &b->v4[p[2]], &b->v4[p[3]] };
                put_v4(fb, W, H, x, y, q);
                p += 4;
            }
        }
}

void cinepak_frame(const u8 *p, u32 len, u32 *fb, int W, int H, cvbook *bk)
{
    if (len < 10)
        return;
    
    const u8 *e = p + len;
    int flags = p[0], ns = rd16(p + 8), y0 = 0;
    p += 10;
    for (int i = 0; i < ns && i < 32 && p + 12 <= e; )
    {
        int id = p[0];
        u32 sz = rd24(p + 1);
        if (sz < 4 || p + sz > e)
            return;
        
        if (id != 0x10 && id != 0x11)
        {
            p += sz;                    // e.g. the 3DO encoder's FE 000006 filler
            continue;
        }

        int y1 = rd16(p + 4), x1 = rd16(p + 6), y2 = rd16(p + 8), x2 = rd16(p + 10);
        if (y1 == 0)
        {
            y1 = y0;
            y2 += y0;
        }

        if (i && !(flags & 1))
            bk[i] = bk[i - 1];
        
        for (const u8 *c = p + 12, *se = p + sz; c + 4 <= se; )
        {
            int cid = c[0];
            u32 csz = rd24(c + 1);
            if (csz < 4 || c + csz > se)
                break;
            
            if (cid >= 0x20 && cid <= 0x27)
            {
                cv_book((cid & 2) ? bk[i].v1 : bk[i].v4, cid, c + 4, c + csz);
            }
            else if (cid >= 0x30 && cid <= 0x32)
            {
                cv_vec(&bk[i], cid, c + 4, c + csz, fb, W, H, x1, y1, x2, y2);
            }

            c += csz;
        }

        y0 = y2;
        p += sz;
        i++;
    }
}
