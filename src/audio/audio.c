//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of s8ge, to any person obtaining a copy
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

#include <stdio.h>
#include <stdlib.h>

#include "audio.h"
#include "alis.h"
#include "channel.h"
#include "mem.h"
#include "video.h"
#include "../sys/sys.h"
#include <string.h>

sAudio audio = {};
s16 muright[kMuBufLen];
u8 mustereo;

volatile sFliAudioChunk fli_audio_queue[FLI_AUDIO_QUEUE_SIZE] = {{0}};
volatile u8 fli_audio_q_head = 0;
volatile u8 fli_audio_q_tail = 0;
volatile u32 fli_chunks_played = 0;

// v6 (Deus DOS) voice manager
sV6Voice v6voices[V6SND_CHANNELS] = {0};

// v6 init ALL channels (SFX + music)
void v6snd_init(void)
{
    for (int i = 0; i < V6SND_CHANNELS; i++)
    {
        v6voices[i].prio = -0x80;
        v6voices[i].unk06 = 0xffff;
        v6voices[i].state = 0;
    }
}

static s16 v6snd_alloc(u32 slot)
{
    s16 idx;
    s8 minprio = v6voices[0].prio;
    s16 minidx = 0;
    for (s16 i = 0;;)
    {
        if (v6voices[i].state == 0)
        {
            idx = i;
            break;
        }
        
        if (++i >= V6SND_SFXCHANNELS)
        {
            if (minprio >= (s8)xread8(slot + 0x45))
                return -1;
            
            idx = minidx;
            break;
        }
        
        if (minprio >= v6voices[i].prio)
        {
            minprio = v6voices[i].prio;
            minidx = i;
        }
    }
    
    sV6Voice *ch = &v6voices[idx];
    ch->state = 0;
    ch->prio = (s8)xread8(slot + 0x45);
    ch->voiceid = (u32)xread32(slot + 0x54);
    ch->unk06 = (u16)xread16(slot + 0x2a);
    return idx;
}

static void v6snd_check(u32 slot, sV6Voice *ch)
{
    xwrite8(slot + 0x23, xread8(slot + 0x23) | 1);
    if (xread8(slot + 0x23) & 0x80)
        return;
    
    if (ch->state != 0 && (u32)xread32(slot + 0x54) == ch->voiceid)
        return;
    
    xwrite8(slot + 0x23, 2);
}

static void v6snd_trigger(u32 slot, sV6Voice *ch)
{
    u32 vol = xread8(slot + 0x47);
    s32 pan = (s8)xread8(slot + 0x51);
    ch->lvol = (u8)((vol * (u32)(0x80 - pan)) >> 9);
    ch->rvol = (u8)((vol * (u32)(0x80 + pan)) >> 9);
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        ch->lvol = ch->rvol = (u8)((vol > 0x7f ? 0x7f : vol) * 2);  // 3DO: amplitude 0x7fff * vol / 127
    
    u16 freq = (u16)xread16(slot + 0x4a);
    ch->freq = freq;
    u32 step = ((u32)freq << 16) / 22050;
    ch->stepfrac = (step & 0xffff) << 16;
    ch->stepint = step >> 16;
    ch->unk30 = (u16)xread16(slot + 0x42);
    ch->fade36 = 0;
    ch->unk34 = 0;
}

static void v6snd_start(u32 slot, sV6Voice *ch)
{
    ch->state = 0;
    u32 p = (u32)xread32(slot + 0x3c);
    u32 desc = p + (u32)xread32(p);
    u32 data = desc + 0x20;
    ch->sample0 = data;
    ch->loopstart = data;
    ch->pos = data;
    ch->frac = 0;
    u32 end = data + (u32)xread32(desc + 0x10);
    ch->end = end;
    ch->loopend = end;
    s16 loop = (s16)xread16(slot + 0x4c);
    ch->loopcnt = loop;
    if (loop != -1)
        xwrite16(slot + 0x4c, 0);
    
    ch->type = 1;
    ch->state = 1;
}

void v6snd_stopslot(u32 slot)
{
    s8 chidx = (s8)xread8(slot + 0x50);
    if (chidx != -1)
    {
        sV6Voice *ch = &v6voices[(u8)chidx];
        if ((u32)xread32(slot + 0x54) == ch->voiceid)
            ch->state = 0;
    }
    
    xwrite8(slot + 0x50, 0xff);
    if ((s16)xread16(slot + 0x4c) != -1)
        xwrite16(slot + 0x40, 0);
}

void v6snd_step(u32 slot)
{
    if (xread8(slot + 0x47) != 0)
    {
        s8 chidx = (s8)xread8(slot + 0x50);

        sV6Voice *ch = NULL;
        bool needAlloc = true;
        if (chidx != -1)
        {
            ch = &v6voices[(u8)chidx];
            if ((u32)xread32(slot + 0x54) == ch->voiceid)
                needAlloc = false;
        }

        if (needAlloc)
        {
            if (xread16(slot + 0x4c) == 0)
            {
                v6snd_stopslot(slot);
                xwrite8(slot + 0x23, xread8(slot + 0x23) & 0xfd);
                return;
            }

            s16 a = v6snd_alloc(slot);
            if (a == -1)
            {
                v6snd_stopslot(slot);
                xwrite8(slot + 0x23, xread8(slot + 0x23) & 0xfd);
                return;
            }

            xwrite8(slot + 0x50, (u8)a);
            xwrite8(slot + 0x23, xread8(slot + 0x23) | 0x80);
            ch = &v6voices[a];
        }

        v6snd_check(slot, ch);
        if (xread8(slot + 0x23) & 0x81)
        {
            v6snd_trigger(slot, ch);
            xwrite8(slot + 0x23, xread8(slot + 0x23) & 0xfe);
        }

        if (xread8(slot + 0x23) & 0x80)
        {
            v6snd_start(slot, ch);
            xwrite8(slot + 0x23, xread8(slot + 0x23) & 0x7f);
        }

        if ((xread8(slot + 0x23) & 0x02) == 0)
        {
            return;
        }
    }

    v6snd_stopslot(slot);
    xwrite8(slot + 0x23, xread8(slot + 0x23) & 0xfd);
}

void v6snd_delsound(u32 slot, u16 arg)
{
    s8 chidx = (s8)xread8(slot + 0x50);
    if (chidx != -1)
    {
        if (arg == 0)
        {
            v6snd_stopslot(slot);
            xwrite8(slot + 0x50, 0xff);
        }
        else
        {
            v6voices[(u8)chidx].fade36 = arg;
        }
    }
}

void v6snd_stopchan(s16 idx)
{
    if (idx < 0)
    {
        for (int i = 0; i < V6SND_SFXCHANNELS; i++)
            v6voices[i].state = 0;
    }
    else if (idx < V6SND_SFXCHANNELS)
    {
        v6voices[idx].state = 0;
    }
}

// 3DO cplaycd: the audio thread (launchme 0xb120) plays a TRACKn AIFF at full volume,
// loops times (0 = once, negative = forever) unless stopped.
static struct { s16 *pcm; u32 len, rate; char name[kPathMaxLen]; } cdtrk;
static volatile u32 cdpos, cdfrac;
static volatile s32 cdloops;
static volatile u8 cdon;

static u32 be32(const u8 *p) { return (u32)p[0] << 24 | (u32)p[1] << 16 | p[2] << 8 | p[3]; }

static int cd_load(const char *path)
{
    if (cdtrk.pcm && !strcmp(cdtrk.name, path))
        return 1;
    
    FILE *f = sys_fopen(path, "rb");
    if (!f)
        return 0;
    
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    u8 *b = malloc(n);
    if (!b || fread(b, n, 1, f) != 1 || n < 12 || memcmp(b, "FORM", 4) || memcmp(b + 8, "AIFF", 4))
    {
        fclose(f);
        free(b);
        return 0;
    }
    
    fclose(f);
    
    u32 rate = 0, frames = 0, ok = 0;
    const u8 *pcm = NULL;
    for (long o = 12; o + 8 <= n; )
    {
        u32 sz = be32(b + o + 4);
        const u8 *c = b + o + 8;
        if (!memcmp(b + o, "COMM", 4) && sz >= 18)
        {
            frames = be32(c + 2);
            ok = c[0] == 0 && c[1] == 1 && c[6] == 0 && c[7] == 16;   // mono, 16-bit
            s32 e = (c[8] << 8 | c[9]) - 16383 - 31;                  // 80-bit float rate
            rate = e < 0 ? be32(c + 10) >> -e : be32(c + 10);
        }
        else if (!memcmp(b + o, "SSND", 4) && sz >= 8)
        {
            pcm = c + 8 + be32(c);
        }
        
        o += 8 + ((sz + 1) & ~1);
    }
    
    if (!ok || !pcm || !rate || pcm + frames * 2 > b + n)
    {
        ALIS_DEBUG(EDebugWarning, "cplaycd: unsupported AIFF %s\n", path);
        free(b);
        return 0;
    }
    
    s16 *d = malloc(frames * 2);
    for (u32 i = 0; d && i < frames; i++)
    {
        d[i] = (s16)(pcm[i * 2] << 8 | pcm[i * 2 + 1]);
    }
    
    free(b);
    if (!d)
    {
        return 0;
    }
    
    cdon = 0;
    free(cdtrk.pcm);
    cdtrk.pcm = d;
    cdtrk.len = frames;
    cdtrk.rate = rate;
    strncpy(cdtrk.name, path, sizeof(cdtrk.name) - 1);
    return 1;
}

void cdtrack_play(const char *path, s8 loops)
{
    if (cdon || !cd_load(path))
        return;
    
    cdpos = cdfrac = 0;
    cdloops = loops ? loops : 1;
    cdon = 1;
}

void cdtrack_stop(void)
{
    cdon = 0;
}

// Savestate tail: on flag, repeats left, file name (playback restarts from the top)
void cdtrack_save(FILE *fp)
{
    char nm[32] = {0};
    const char *b = strrchr(cdtrk.name, '/');
    strncpy(nm, b ? b + 1 : cdtrk.name, sizeof(nm) - 1);
    s32 l = cdloops;
    u8 on = cdon;
    fwrite(&on, 1, 1, fp);
    fwrite(&l, 4, 1, fp);
    fwrite(nm, sizeof(nm), 1, fp);
}

void cdtrack_load(FILE *fp)
{
    char nm[32], path[kPathMaxLen];
    s32 l;
    u8 on;
    cdon = 0;
    if (fread(&on, 1, 1, fp) != 1 || fread(&l, 4, 1, fp) != 1 || fread(nm, sizeof(nm), 1, fp) != 1 || !on)
        return;
    
    nm[sizeof(nm) - 1] = 0;
    snprintf(path, sizeof(path), "%s%s", alis.platform.path, nm);
    cdtrack_play(path, l < 0 ? -1 : (s8)l);
}

static void cd_render(s16 *buf, s32 frames, s32 channels, s32 outrate)
{
    u32 step = (u32)(((u64)cdtrk.rate << 16) / outrate);
    for (s32 f = 0; f < frames && cdon; f++)
    {
        s32 smp = cdtrk.pcm[cdpos];
        for (s32 c = 0; c < channels; c++)
        {
            s32 v = buf[f * channels + c] + smp;
            buf[f * channels + c] = (s16)(v < -32768 ? -32768 : v > 32767 ? 32767 : v);
        }
        
        cdfrac += step;
        cdpos += cdfrac >> 16;
        cdfrac &= 0xffff;
        if (cdpos >= cdtrk.len)
        {
            cdpos = 0;
            if (cdloops > 0 && --cdloops == 0)
                cdon = 0;
        }
    }
}

void v6snd_render(s16 *buf, s32 frames, s32 channels, s32 outrate)
{
    if (cdon)
        cd_render(buf, frames, channels, outrate);

    if (!audio.fsound)
    {
        return;
    }

    for (int i = 0; i < V6SND_SFXCHANNELS; i++)
    {
        sV6Voice *ch = &v6voices[i];
        if (ch->state == 0 || ch->type != 1)
        {
            continue;
        }
        
        ch->state = 2;

        u32 step = ((u32)ch->freq << 16) / (u32)outrate;
        u32 stepfrac = (step & 0xffff) << 16;
        u32 stepint = step >> 16;

        for (s32 f = 0; f < frames; f++)
        {
            s32 smp = (s32)*(alis.mem + ch->pos) - 0x80;
            s32 l = buf[f * channels] + smp * ch->lvol;
            l = l < -32768 ? -32768 : l > 32767 ?  32767 : l;
            
            buf[f * channels] = (s16)l;
            if (channels > 1)
            {
                s32 r = buf[f * channels + 1] + smp * ch->rvol;
                r = r < -32768 ? -32768 : r > 32767 ? 32767 : r;
                
                buf[f * channels + 1] = (s16)r;
            }

            u32 oldfrac = ch->frac;
            ch->frac += stepfrac;
            ch->pos += stepint + (ch->frac < oldfrac ? 1 : 0);
            if (ch->pos >= ch->end)
            {
                ch->loopcnt--;
                if (ch->loopcnt == 0)
                {
                    ch->state = 0;
                    break;
                }
                
                if (ch->loopcnt < 0)
                {
                    ch->loopcnt++;
                }
                
                ch->end = ch->loopend;
                ch->pos = ch->loopstart;
                ch->frac = 0;
            }
        }
    }
}

void playsample(eChannelType type, u8 *address, s8 freq, u8 volume, u32 length, u16 loop, s8 priorson)
{
    char cson = CHAR_MAX;
    sChannel *canal = NULL;

    // While a film plays, channel 3 is reserved for FLI speech (SFX would chop it).
    int max_chan = bfilm.playing ? 3 : 4;

    for (int i = 0; i < max_chan; i++)
    {
        if (priorson == audio.channels[i].curson)
        {
            canal = &audio.channels[i];
            cson = priorson;
            break;
        }
    }

    if (canal == NULL)
    {
        for (int i = 0; i < max_chan; i++)
        {
            if (audio.channels[i].curson < cson)
            {
                canal = &audio.channels[i];
                cson = audio.channels[i].curson;
            }
        }
    }
    
    if ((priorson >= 0 && cson < 0 && cson != -0x80) || priorson < cson)
        return;

    if (freq < 0x1 || 0x14 < freq)
        freq = 10;

    canal->address = (s8 *)address;
    canal->volume = volume;
    canal->length = length;
    canal->freq = freq * 1000;
    canal->loop = loop;
    canal->played = 0;
    canal->type = type;
    canal->curson = priorson;
}

void runson(eChannelType type, s8 pereson, s8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson)
{
    char cson = CHAR_MAX;
    sChannel *canal = NULL;

    if (priorson < 0)
    {
        for (int i = 0; i < 3; i++)
        {
            if (priorson == audio.channels[i].curson)
            {
                canal = &audio.channels[i];
                cson = priorson;
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            if (pereson == audio.channels[i].pere && priorson == audio.channels[i].curson)
            {
                canal = &audio.channels[i];
                cson = priorson;
                break;
            }
        }
    }
    
    if (canal == NULL)
    {
        for (int i = 0; i < 3; i++)
        {
            if (audio.channels[i].curson < cson)
            {
                canal = &audio.channels[i];
                cson = audio.channels[i].curson;
            }
        }
    }
    
    if ((priorson >= 0 && cson < 0 && cson != -0x80) || priorson < cson)
    {
        return;
    }
    
    canal->state = -0x80;
    canal->curson = priorson;
    canal->type = type;
    canal->freq = freqson;
    canal->delta_freq = dfreqson;
    canal->volume = volson;
    canal->delta_volume = dvolson;
    canal->length = longson;
    canal->pere = pereson;
    canal->state = 2;
}

static void channel_off(int i)
{
    audio.channels[i].type = eChannelTypeNone;
    audio.channels[i].volume = 0;
    audio.channels[i].freq = 0;
    audio.channels[i].curson = 0x80;
    audio.channels[i].pere = 0;
    audio.channels[i].state = 0;
    audio.channels[i].played = 0;

    if (i < 3)
        io_canal(&(audio.channels[i]), i);
}

void offsound(void)
{
    for (int i = 0; i < 4; i++)
        channel_off(i);
}

extern sMV1Audio mv1a;
extern sMV2Audio mv2a;

// VM address after script data moved: >= hi shifts by delta, [lo, hi) was removed (0), below lo stays.
static inline u32 moved(u32 a, u32 lo, u32 hi, s32 delta)
{
    return a < lo ? a : a < hi ? 0 : a + delta;
}

static inline void move_ptr(u32 *a, u32 lo, u32 hi, s32 delta) { *a = moved(*a, lo, hi, delta); }

// The original engine leaves audio pointing at the old location after shrinkprog (garbage
// sound); our mixers derive read pointers from sample headers, so stale ones crash.
void audio_relocate(u32 lo, u32 hi, s32 delta)
{
    for (int i = 0; i < 128; i++)
        move_ptr(&audio.tabinst[i].address, lo, hi, delta);

    if (audio.mupnote >= lo && audio.mupnote < hi)
        audio.muflag = 0;
    
    move_ptr(&audio.mupnote, lo, hi, delta);
    if (alis.platform.version == 10)
        mv0_relocate(lo, hi, delta);
    
    move_ptr(&mv1a.noteptr, lo, hi, delta);

    for (int i = 0; i < kNumMV1Channels; i++)
    {
        sMV1Channel *ch = &mv1a.channels[i];
        move_ptr(&ch->instr, lo, hi, delta);
        move_ptr(&ch->endsam, lo, hi, delta);
        move_ptr(&ch->startsam.address, lo, hi, delta);
        if (ch->startsam.address == 0 && ch->instr == 0)
            ch->actflag = 0;
    }

    for (int i = 0; i < 4; i++)
    {
        sAudioVoice *v = &mv2a.voices[i];
        u32 sample = v->sample;
        move_ptr(&v->sample, lo, hi, delta);
        move_ptr(&v->startsam1, lo, hi, delta);
        move_ptr(&v->startsam2, lo, hi, delta);
        if (sample && !v->sample)
            memset(v, 0, sizeof(*v));
    }

    // v6 voices in the freed block stop, moved ones restart as state 1
    for (int i = 0; alis.platform.version >= 60 && i < V6SND_CHANNELS; i++)
    {
        sV6Voice *v = &v6voices[i];
        if (v->state == 0 || v->sample0 < lo)
        {
            continue;
        }
        
        if (v->sample0 < hi)
        {
            v->state = 0;
            continue;
        }
        
        v->sample0 += delta;
        v->loopstart += delta;
        v->loopend += delta;
        v->end += delta;
        v->pos += delta;
        v->state = 1;
    }

    for (int i = 0; i < 4; i++)
    {
        sChannel *ch = &audio.channels[i];
        u8 *a = (u8 *)ch->address;
        if (a < alis.mem + lo || a >= alis.mem + alis.finmem)
        {
            continue;
        }
        
        u32 at = moved((u32)(a - alis.mem), lo, hi, delta);
        if (at)
        {
            ch->address = (s8 *)(alis.mem + at);
        }
        else
        {
            channel_off(i);
        }
    }
}

#define WAVE_FORMAT_IEEE_FLOAT 3

s32 save_wave_file(const char *name, float *data, s32 sample_rate, s32 channel_count, s32 sample_count)
{
    s8 header[58];
    s32 sample_size = sizeof(float);
    s32 data_size = sample_size * channel_count * sample_count;
    s32 pad = data_size & 1;
    memcpy(&header[0], "RIFF", 4);
    *((s32*) (header + 4)) = 50 + data_size + pad;
    memcpy(&header[8], "WAVE", 4);
    memcpy(&header[12], "fmt ", 4);
    *((s32*) (header + 16)) = 18;
    *((s16*) (header + 20)) = WAVE_FORMAT_IEEE_FLOAT;
    *((s16*) (header + 22)) = (s16) channel_count;
    *((s32*) (header + 24)) = sample_rate;
    *((s32*) (header + 28)) = sample_rate * sample_size * channel_count;
    *((s16*) (header + 32)) = (s16) (sample_size * channel_count);
    *((s16*) (header + 34)) = (s16) (8 * sample_size);
    *((s16*) (header + 36)) = 0;
    memcpy(&header[38], "fact", 4);
    *((s32*) (header + 42)) = 4;
    *((s32*) (header + 46)) = channel_count * sample_count;
    memcpy(&header[50], "data", 4);
    *((s32*) (header + 54)) = data_size;
    FILE *f = fopen(name, "wb");
    if (f == NULL)
    {
        return 0;
    }
    
    fwrite(header, 1, sizeof(header), f);
    fwrite(data, 1, data_size, f);
    if (pad)
    {
        fwrite(&pad, 1, 1, f);
    }
    
    fclose(f);
    return 1;
}
