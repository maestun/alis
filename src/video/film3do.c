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
#include "audio.h"
#include "cinepak.h"
#include "image.h"
#include "video.h"
#include "../sys/sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG(a, b, c, d) ((u32)(a) << 24 | (u32)(b) << 16 | (u32)(c) << 8 | (u32)(d))

static char fname[kPathMaxLen];

static u32 rd32(const u8 *p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | p[2] << 8 | p[3]; }

static void show(const u32 *fb, int fw, int fh)
{
    if (!image.rgbphysic)
        return;
    
    int W = alis.platform.width, H = alis.platform.height;
    int w = fw < W ? fw : W, h = fh < H ? fh : H;
    sys_lock_renderer();
    for (int y = 0; y < h; y++)
        memcpy(image.rgbphysic + y * W, fb + y * fw, w * sizeof(u32));
    
    sys_unlock_renderer();
}

// Any new key or click stops the film (the 3DO player stops on the pad's X button).
static int skipped(u8 *armed)
{
    u8 k = io_inkey();
    mouse_t m = sys_consume_mouse();
    if (!k && !m.lb && !m.rb)
        *armed = 1;
    
    return *armed && (k || m.lb_clicked || m.rb_clicked);
}

void film3do_open(const char *name)
{
    strncpy(fname, name, sizeof(fname) - 1);
    fname[sizeof(fname) - 1] = 0;
}

void film3do_run(s16 mode)
{
    FILE *f = fopen(fname, "rb");
    if (!f)
        return;

    cvbook *bk = calloc(32, sizeof(cvbook));
    u32 *fb = NULL, cap = 0;
    u8 *buf = NULL;
    int fw = 0, fh = 0, pcm = 0;
    s16 freq = 22050;
    u32 t0 = 0, stall = 0;
    u8 armed = 0, hd[16];

    audio.channels[3].type = eChannelTypeNone;
    fli_audio_q_head = fli_audio_q_tail = 0;

    while (bk && fread(hd, 16, 1, f) == 1)
    {
        u32 type = rd32(hd), sz = rd32(hd + 4), tm = rd32(hd + 8);
        if (sz < 16)
            break;
        
        sz -= 16;
        if (type != TAG('F','I','L','M') && type != TAG('S','N','D','S'))
        {
            fseek(f, sz, SEEK_CUR);
            continue;
        }

        if (sz > cap)
        {
            u8 *nb = realloc(buf, sz);
            if (!nb)
                break;
            
            buf = nb;
            cap = sz;
        }

        if (sz < 12 || fread(buf, sz, 1, f) != 1)
            break;
        
        if (!t0)
            t0 = sys_ticks();

        u32 sub = rd32(buf);
        if (sub == TAG('S','H','D','R'))
        {
            // +0x18 bits, +0x1c rate, +0x20 channels, +0x24 compression
            pcm = rd32(buf + 0x18) == 8 && rd32(buf + 0x20) == 1 && rd32(buf + 0x24) == TAG('N','O','N','E');
            freq = (s16)(rd32(buf + 0x1c) > 32767 ? 32767 : rd32(buf + 0x1c));
        }
        else if (sub == TAG('S','S','M','P') && pcm)
        {
            // Wait while the queue drains; once it stalls (muted audio) just drop
            u32 n = rd32(buf + 4), done = 0;
            if (n > sz - 8)
                n = sz - 8;
            
            while ((done += fli_audio_push((s8 *)buf + 8 + done, n - done, freq)) < n && stall < 1000)
            {
                u8 tail = fli_audio_q_tail;
                if (skipped(&armed))
                    goto stop;
                
                sys_delay(10);
                stall = tail == fli_audio_q_tail ? stall + 10 : 0;
            }
        }
        else if (sub == TAG('F','H','D','R'))
        {
            fh = rd32(buf + 0xc);
            fw = rd32(buf + 0x10);
            free(fb);
            fb = calloc((size_t)fw * fh, sizeof(u32));
        }
        else if (sub == TAG('F','R','M','E') && fb)
        {
            u32 n = rd32(buf + 8);
            if (n > sz - 12)
                n = sz - 12;
            
            cinepak_frame(buf + 12, n, fb, fw, fh, bk);
            while (sys_ticks() - t0 < tm * 1000 / 240)
            {
                if (skipped(&armed))
                    goto stop;
                
                sys_delay(2);
            }

            show(fb, fw, fh);
            if (skipped(&armed))
                break;
        }
    }

stop:
    audio.channels[3].type = eChannelTypeNone;
    fli_audio_q_head = fli_audio_q_tail = 0;
    if (mode < 0 && image.rgbphysic)
    {
        size_t n = (size_t)alis.platform.width * alis.platform.height;
        sys_lock_renderer();
        for (size_t i = 0; i < n; i++)
            image.rgbphysic[i] = image.rgblogic[i] = 0xff000000u;
        
        sys_unlock_renderer();
    }

    fclose(f);
    free(fb);
    free(buf);
    free(bk);
}
