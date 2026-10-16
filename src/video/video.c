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
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "screen.h"
#include "../sys/sys.h"
#include "utils.h"
#include "video.h"


#if ALIS_SDL_VER < 2
# include <SDL/SDL.h>
#elif defined(_MSC_VER)
# include "SDL.h"
#elif __has_include(<SDL.h>)
# include <SDL.h>
#else
# include <SDL2/SDL.h>
#endif

u8 *endframe = NULL;

sFLICData bfilm;

// Wall-clock pacing for runfilm; reset per film, anchored on the first audio chunk.
static u32 fli_pace_anchor   = 0;
static u32 fli_pace_total_ms = 0;


u16 pal[16] = {
    0x0000,
    0x0019,
    0x0011,
    0x0811,
    0x0089,
    0x0881,
    0x0081,
    0x0088,
    0x0018,
    0x0012,
    0x0889,
    0x0812,
    0x0181,
    0x0181,
    0x0181,
    0x0181,
};

extern void dbglog(const char *fmt, ...);   // flushing debug log (no-op unless logfile open)

// Film composition buffer (FLS ping-pong frames / FLIC chunky frame), allocated per film.
u8 *pvgalogic = NULL;
u8 *vgalogic = NULL;
u8 *vgalogic_df = NULL;
u32 vgalogic_sz = 0;

int vgalogic_alloc(void)
{
    if (!pvgalogic && !(pvgalogic = calloc(1, kVgaLogicSize)))
        return 0;
    
    if (vgalogic_sz < kVgaLogicSize)
        vgalogic_sz = kVgaLogicSize;
    
    vgalogic = pvgalogic + 0x40;
    vgalogic_df = pvgalogic + 0xdf00 + 319;
    return 1;
}

int vgalogic_fit(u32 need)
{
    if (!pvgalogic || need <= vgalogic_sz)
        return pvgalogic != NULL;
    
    u8 *p = calloc(1, need);
    if (!p)
        return 0;
    
    memcpy(p, pvgalogic, vgalogic_sz);
    vgalogic = p + (vgalogic - pvgalogic);
    vgalogic_df = p + (vgalogic_df - pvgalogic);
    free(pvgalogic);
    pvgalogic = p;
    vgalogic_sz = need;
    return 1;
}

void vgalogic_free(void)
{
    free(pvgalogic);
    pvgalogic = vgalogic = vgalogic_df = NULL;
    vgalogic_sz = 0;
}

u8 g_s512_ending = 0;   // S512 film reached its end — sys_render flushes the final frame then → None

void endfilm(void)
{
    flstofen(-1);
    bfilm.type = eAlisVideoNone;
    // 16-bit film path stays in 16-bit mode holding the last frame (as the original);
    // sys_render restores planar on the first game draw.
}

void inifilm(void)
{
    bfilm.playing = 0;

    if (!vgalogic_alloc())
    {
        dbglog("inifilm: no memory for the film buffer — film skipped\n");
        bfilm.type = eAlisVideoNone;
        return;
    }

    // Free channel 3 (a game sound may hold it; the queue only primes when idle) and
    // reset the queue. type = None first so the mixer skips it meanwhile.
    audio.channels[3].type   = eChannelTypeNone;
    audio.channels[3].curson = 0x80;
    fli_audio_q_head = 0;
    fli_audio_q_tail = 0;

    g_s512_ending = 0;   // fresh film — cancel any pending end-flush from a previous one

    switch (alis.platform.kind)
    {
        case EPlatformAtari:
            bfilm.type = eAlisVideoS512;
            break;
        case EPlatformAmiga:
            bfilm.type = eAlisVideoHAM6;
            break;
        default:
            bfilm.type = eAlisVideoFLIC;
            break;
    };
    
#if ALIS_SDL_VER == 1 && !defined(ALIS_NATIVE_PLANAR)
    memset(image.physic, 0, 64000);
    memset(image.logic, 0, 64000);
#endif
    // Native planar: no clear (the display is one of these buffers; the first frame is a keyframe).

    if (alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga)
    {
        flstofen(0);
    }
    else
    {
        flitofen();   // reads the FLC header (0xAF11) -> bfilm.width/height set
#if defined(ALIS_FLI_FALCON16)
        // Falcon: 16-bit Videl mode, decode straight to screen. No-op elsewhere.
        extern int sys_film16_begin(u16 w, u16 h);
        sys_film16_begin(bfilm.width, bfilm.height);
#endif
    }
}

void runfilm(void)
{
    u32 basemain = bfilm.basemain;
    image.fenx1 = read16le(alis.mem + basemain + 0xe);
    image.fenx2 = read16le(alis.mem + basemain + 0x12) + image.fenx1;
    image.clipl = read16le(alis.mem + basemain + 0x12) + 1;
    image.feny1 = read16le(alis.mem + basemain + 0x10);
    image.feny2 = read16le(alis.mem + basemain + 0x14) + image.feny1;
    image.cliph = read16le(alis.mem + basemain + 0x14) + 1;
    image.wloglarg = image.loglarg;
    image.wlogx1 = 0;
    image.wlogy1 = 0;
    image.clipx1 = image.fenx1;
    image.clipy1 = image.feny1;
    image.clipx2 = image.fenx2;
    image.clipy2 = image.feny2;

    // Reset wall-clock pacing state so this cutscene anchors fresh
    // (lazy — actual anchor sets on first audio chunk).
    fli_pace_anchor   = 0;
    fli_pace_total_ms = 0;

    bfilm.playing = 1;

    while (true)
    {
        u32 prevclock = alis.timeclock;
        u8  q_head_before = fli_audio_q_head;
        u32 chunks_target = fli_chunks_played;
        s16 result = (alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga) ? flstofen(0) : flitofen();
#if defined(ALIS_FLI_FALCON16)
        // 16-bit FLIC decodes on screen in place: wait a vblank. Not for FLS (sys_render flips).
        if (!(alis.platform.kind == EPlatformAtari || alis.platform.kind == EPlatformAmiga))
        {
            extern void sys_film16_vsync(void);
            sys_film16_vsync();
        }
#endif

        // Did flstofen/flitofen enqueue audio chunk(s) for this frame?
        u8 chunks_enqueued = (u8)((fli_audio_q_head - q_head_before)
                                  & (FLI_AUDIO_QUEUE_SIZE - 1));
        chunks_target += chunks_enqueued;

        if (0 < bfilm.waitclock)
        {
            // Audio frames: pace by chunk duration (wall clock; timeclock on native Atari).
            // Silent frames: timeclock.
            int use_wallclock_pace = chunks_enqueued;
#if defined(ALIS_USE_NATIVE_ATARI)
            use_wallclock_pace = 0;
#endif
            if (use_wallclock_pace)
            {
                // Anchor on the first chunk; accumulate each chunk's duration.
                if (fli_pace_anchor == 0)
                {
                    fli_pace_anchor   = sys_ticks();
                    fli_pace_total_ms = 0;
                }

                for (u8 k = 0; k < chunks_enqueued; k++)
                {
                    u8 idx = (u8)((q_head_before + k) & (FLI_AUDIO_QUEUE_SIZE - 1));
                    s16 cf = fli_audio_queue[idx].freq;
                    u32 cl = fli_audio_queue[idx].length;
                    if (cf > 0)
                        fli_pace_total_ms += (cl * 1000UL) / (u32)cf;
                }

                while (sys_ticks() - fli_pace_anchor < fli_pace_total_ms)
                {
                    sys_delay(1);
                }
            }
            else
            {
                // Interval = waitclock / file-format tick rate (FLI 60 Hz, FLS 50 Hz).
                u32 thz = sys_timeclock_hz ? sys_timeclock_hz : 50;
                u32 src = (alis.platform.kind == EPlatformAtari ||
                           alis.platform.kind == EPlatformAmiga) ? 50UL : 60UL;
                s16 index = (s16)(((u32)(u16)bfilm.waitclock * thz) / src);
                if (index == 0)
                    index = 1;
                while (alis.timeclock < (u32)(index + prevclock))
                {
                    sys_delay(1);
                }
            }
        }
        
        if (result == 0)
        {
            // S512 data exhausted: sys_render still has to draw the last frame, then ends the film.
            if (bfilm.type == eAlisVideoS512 || bfilm.type == eAlisVideoHAM6)
                g_s512_ending = 1;
            break;
        }

        bfilm.batchframes--;
        if (bfilm.batchframes == 0)
        {
            bfilm.result = 0;

#if ALIS_SDL_VER == 1
            // some versions of game do not run cendfilm
            if (bfilm.frame + 1 >= bfilm.frames && bfilm.type == eAlisVideoFLIC)
            {
                bfilm.type = eAlisVideoCleanup;
            }

            // Same for S512 (no cleanup frame needed).
            else if (bfilm.frame + 1 >= bfilm.frames && bfilm.type == eAlisVideoS512)
            {
                bfilm.type = eAlisVideoNone;
            }
#endif

            return;
        }

        if (bfilm.batchframes < 0)
        {
            bfilm.batchframes++;
        }
    }

    if (bfilm.sfp != NULL)
    {
        fli_stream_rewind();   // window mode: addr2 may be stale after compaction
    }
    else
    {
        bfilm.addr1 = bfilm.addr2;
    }

    bfilm.result = -0x7ffc;
}
