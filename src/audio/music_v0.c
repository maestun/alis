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
#include "mem.h"
#include "../sys/sys.h"
#if !defined(__TOS__) && !defined(__atarist__)
#include "emu2149.h"
#endif

// Tables from the Atari v1.0 interpreter. Durations are read with index & 31,
// past the 24 real entries into the period table, as the original does.
static const u8 mv0_tabdur[32] = {
    0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04, 0x06, 0x06, 0x08, 0x0c, 0x0c, 0x10,
    0x18, 0x18, 0x20, 0x30, 0x30, 0x00, 0x30, 0x19, 0x0e, 0xee, 0x0e, 0x18, 0x0d, 0x4d, 0x0c, 0x8e };

static const u16 mv0_tabfrq[96] = {
    0x0eee, 0x0e18, 0x0d4d, 0x0c8e, 0x0bda, 0x0b2f, 0x0a8f, 0x09f7, 0x0968, 0x08e1, 0x0861, 0x07e9,
    0x0777, 0x070c, 0x06a7, 0x0647, 0x05ed, 0x0598, 0x0547, 0x04fc, 0x04b4, 0x0470, 0x0431, 0x03f4,
    0x03bc, 0x0386, 0x0353, 0x0324, 0x02f6, 0x02cc, 0x02a4, 0x027e, 0x025a, 0x0238, 0x0218, 0x01fa,
    0x01de, 0x01c3, 0x01aa, 0x0192, 0x017b, 0x0166, 0x0152, 0x013f, 0x012d, 0x011c, 0x010c, 0x00fd,
    0x00ef, 0x00e1, 0x00d5, 0x00c9, 0x00be, 0x00b3, 0x00a9, 0x009f, 0x0096, 0x008e, 0x0086, 0x007f,
    0x0077, 0x0071, 0x006a, 0x0064, 0x005f, 0x0059, 0x0054, 0x0050, 0x004b, 0x0047, 0x0043, 0x003f,
    0x003c, 0x0038, 0x0035, 0x0032, 0x002f, 0x002d, 0x002a, 0x0028, 0x0026, 0x0024, 0x0022, 0x0020,
    0x001e, 0x001c, 0x001b, 0x0019, 0x0018, 0x0016, 0x0015, 0x0014, 0x0013, 0x0012, 0x0011, 0x0010 };

// Volume envelopes: records of (flags: 1 tone / 2 noise / 0 end, delta, rate, count).
static const u8 mv0_tabins[] = {
    0x01, 0x0f, 0x02, 0x01, 0x01, 0xff, 0x16, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x0f, 0x02, 0x01, 0x01, 0xff, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x0f, 0x02, 0x01, 0x01, 0xff, 0x08, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x0f, 0x02, 0x01, 0x01, 0xff, 0x04, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x0f, 0x02, 0x01, 0x01, 0xff, 0x02, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x0f, 0x01, 0x01, 0x02, 0xff, 0x0b, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x0f, 0x01, 0x01, 0x02, 0xff, 0x07, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x0f, 0x01, 0x01, 0x02, 0xff, 0x04, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x0f, 0x01, 0x01, 0x02, 0xff, 0x02, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x0f, 0x01, 0x01, 0x02, 0xff, 0x01, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x0f, 0x02, 0x01, 0x03, 0xff, 0x0a, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x0f, 0x02, 0x01, 0x03, 0xff, 0x04, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x0f, 0x02, 0x01, 0x01, 0x00, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x0f, 0x02, 0x01, 0x01, 0x00, 0xa0, 0x02, 0x01, 0xff, 0x08, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x04, 0x0a, 0x01, 0x01, 0x10, 0x05, 0x01, 0xff, 0x10, 0x05, 0x01, 0xff, 0x04, 0x0a,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x38, 0x06, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x38, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const u8 mv0_insoff[17] = { 0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 172, 192, 200 };

sMV0Audio mv0a;

static void mv0_start(void);

// Channel taken by a YM sound effect.
static int mv0_sfx(int i)
{
    u8 t = audio.channels[i].type;
    return t == eChannelTypeDingZap || t == eChannelTypeNoise || t == eChannelTypeExplode;
}

static void mv0_wmix(int i)
{
    sMV0Channel *c = &mv0a.ch[i];
    u8 m = sys_read_psg(7) | (9 << i);
    u8 f = c->vflags | c->pflags;
    if (f & 1)
        m &= ~(1 << i);
    if (f & 2)
    {
        m &= ~(8 << i);
        sys_write_psg(6, c->period >> 7);
    }
    sys_write_psg(7, m);
}

static void mv0_canal(int i)
{
    sMV0Channel *c = &mv0a.ch[i];
    if (c->vflags && --c->vcnt == 0)
    {
        c->vol += c->vdelta;
        if (c->vol & 0x80)
            c->vol = 0;
        if (--c->vnum == 0)
        {
            const u8 *r = mv0_tabins + c->venv;
            c->vflags = r[0];
            c->vdelta = r[1];
            c->vrate = r[2];
            c->vnum = r[3];
            c->venv += 4;
        }
        c->vcnt = c->vrate;
        sys_write_psg(8 + i, ((c->vol * (mv0a.curvol + 1)) >> 4) & 0xf);
        mv0_wmix(i);
    }

    // No instrument has a pitch envelope: it only latches the period once.
    if (c->pflags)
    {
        c->pflags = 0;
        sys_write_psg(i * 2, c->period & 0xff);
        sys_write_psg(i * 2 + 1, (c->period >> 8) & 0xf);
        mv0_wmix(i);
    }
}

static void mv0_end(void)
{
    mv0a.playing = 0;
    if (mv0a.pending && !mv0a.starting && audio.mupnote)
        mv0_start();
}

// Bit 6: tied note, its length sums the following bit 5 events of the same pitch.
static u32 mv0_tie(u32 p, sMV0Channel *c)
{
    u32 q = p;
    u8 dur = mv0_tabdur[xread8(q++) & 31];
    u8 note = xread8(q++);
    for (int guard = 0; guard < 0x4000; guard++)
    {
        u8 b = xread8(q++);
        if (b == 0)
            continue;

        if (b & 0x80)
        {
            if (b == 0x80 || b == 0x84)
                q += 2;
            else if (b == 0x81 || b == 0x83 || b == 0x85)
                q++;
            else if (b != 0x82 && b != 0x86)
                break;
            continue;
        }

        u8 d = xread8(q++);
        u8 n = xread8(q++);
        if (!(b & 0x20) || n != note)
            continue;

        dur += mv0_tabdur[d & 31];
        if (b & 0x40)
            continue;

        c->dur = dur;
        c->busy = 1;
        break;
    }

    return p + 1;
}

static u32 mv0_note(u32 p, u8 cmd)
{
    if (cmd & 0x20)
        return p + 2;

    mv0a.played = 1;

    int i = 0;
    while (i < 3 && mv0a.ch[i].busy)
        i++;
    if (i == 3)
    {
        i = 0;
        while (i < 3 && mv0a.ch[i].busy != 2)
            i++;
    }
    if (i == 3)
        i = mv0a.rr < 2 ? mv0a.rr : 2;

    sMV0Channel *c = &mv0a.ch[i];
    if (cmd & 0x40)
    {
        p = mv0_tie(p, c);
    }
    else
    {
        c->dur = mv0_tabdur[xread8(p++) & 31];
        c->busy = 1;
    }

    u8 n = xread8(p++) - mv0a.transpose - 12;
    if (!mv0_sfx(i))
    {
        c->period = mv0_tabfrq[n < 96 ? n : 95];
        c->vflags = 1;
        c->vnum = 1;
        c->vcnt = 1;
        c->vdelta = 0;
        c->vol = 0;
        c->pflags = 1;
        u8 ins = (cmd - 1) & 31;
        c->venv = mv0_insoff[ins > 15 ? 16 : ins];
        c->on = 1;
    }

    mv0a.rr = i + 1 < 3 ? i + 1 : 0;
    return p;
}

// Plays the events up to the next 0 that follows a note.
static void mv0_events(void)
{
    mv0a.played = 0;
    u32 p = mv0a.songptr;
    for (int guard = 0; guard < 0x4000; guard++)
    {
        u8 b = xread8(p++);
        if (b == 0)
        {
            if (mv0a.played)
                break;
            continue;
        }

        if (b < 0x80)
        {
            p = mv0_note(p, b);
            continue;
        }

        switch (b)
        {
            case 0xff:
                mv0_end();
                return;
            case 0x80:
            case 0x84:
                p += 2;
                break;
            case 0x81:
                mv0a.divider = (u8)(0xff - xread8(p++)) >> 5;
                break;
            case 0x83:
                mv0a.transpose = xread8(p++);
                break;
            case 0x85:
            {
                // The original never pops this stack: kept as is.
                u8 sp = mv0a.loopsp < kMV0Loops ? mv0a.loopsp++ : kMV0Loops - 1;
                mv0a.loopcnt[sp] = xread8(p++);
                mv0a.loopptr[sp] = p;
                break;
            }
            case 0x86:
                if (mv0a.loopsp && --mv0a.loopcnt[mv0a.loopsp - 1])
                    p = mv0a.loopptr[mv0a.loopsp - 1];
                break;
            default:
                break;
        }
    }

    mv0a.songptr = p;
}

static void mv0_start(void)
{
    mv0a.playing = 0;
    mv0a.starting = 1;
    mv0a.songptr = audio.mupnote;
    mv0a.divider = 1;
    mv0a.transpose = 0;
    mv0a.loopsp = 0;
    mv0a.rr = 0;
    for (int i = 0; i < 3; i++)
        mv0a.ch[i].busy = 0;

    mv0_events();
    mv0_events();
    mv0a.count = mv0a.divider;
    mv0a.starting = 0;

    mv0a.tgtvol = mv0a.maxvol;
    mv0a.curvol = 0;
    mv0a.fadein = mv0a.pfadein;
    mv0a.step = mv0a.rate = mv0a.fadein >> 4;
    mv0a.sustain = mv0a.psustain;
    mv0a.fadeout = mv0a.pfadeout;
    if (mv0a.cadence)
        mv0a.divider = mv0a.cadence;

    mv0a.pending--;
    mv0a.playing = 1;

    set_mutaloop(audio.host_freq / 50);
    audio.soundrout = mv0_soundrout;
    audio.muflag = 1;
}

// One 100 Hz tick: volume fades, then the sequencer.
static void mv0_tick(void)
{
    if (mv0a.fadein)
    {
        if (--mv0a.step == 0)
        {
            if ((s8)mv0a.curvol < (s8)mv0a.tgtvol)
                mv0a.curvol++;
            mv0a.step = mv0a.rate;
        }
        mv0a.fadein--;
    }
    else if (mv0a.sustain)
    {
        if (--mv0a.sustain == 0)
            mv0a.step = mv0a.rate = mv0a.fadeout >> 4;
    }
    else if (mv0a.fadeout)
    {
        if (--mv0a.step == 0)
        {
            if (mv0a.curvol)
                mv0a.curvol--;
            mv0a.step = mv0a.rate;
        }
        if (--mv0a.fadeout == 0)
        {
            mv0_end();
            return;
        }
    }

    if (--mv0a.count == 0)
    {
        int ended = 0;
        for (int i = 0; i < 3; i++)
        {
            sMV0Channel *c = &mv0a.ch[i];
            if (c->busy)
            {
                c->busy = 2;
                if (--c->dur == 0)
                {
                    c->busy = 0;
                    ended = 1;
                }
            }
        }
        if (ended)
            mv0_events();
        mv0a.count = mv0a.divider;
    }
}

// The original runs at 100 Hz, soundrout at 50 Hz.
void mv0_soundrout(void)
{
    for (int t = 0; t < 2; t++)
    {
        if (mv0a.playing)
            mv0_tick();
        for (int i = 0; i < 3; i++)
        {
            if (mv0a.ch[i].on && !mv0_sfx(i))
                mv0_canal(i);
        }
    }

#if !defined(__TOS__) && !defined(__atarist__)
    // As sys_calc_psg_music, saturated: 3 loud channels overflow at x4.
    extern PSG *audio_psg;
    for (int i = 0; i < audio.mutaloop; i++)
    {
        s32 v = PSG_calc(audio_psg) * 4;
        audio.muadresse[i] = v > 0x7fff ? 0x7fff : v;
    }
#endif
}

// cmusic: song at addr (data after the 3-byte header); queued while one plays.
void mv0_gomusic(u32 addr, u8 vol, s16 fadein, s16 sustain, s16 fadeout)
{
    audio.mupnote = addr + 3;
    mv0a.maxvol = vol;
    mv0a.pfadein = fadein + 0x10;
    mv0a.psustain = sustain;
    mv0a.pfadeout = fadeout + 0x10;
    mv0a.pending = 1;
    if (!mv0a.playing)
        mv0_start();
}

// cdelmusic: fade out, no queued song.
void mv0_offmusic(void)
{
    mv0a.fadein = 0;
    mv0a.sustain = 0;
    mv0a.pending = 0;
}

void mv0_cadence(u8 cadence)
{
    mv0a.cadence = cadence + 1;
}

void mv0_volume(u8 vol)
{
    mv0a.tgtvol = vol;
    mv0a.curvol = vol;
}

void mv0_reset(void)
{
    memset(&mv0a, 0, sizeof(mv0a));
}

void mv0_relocate(u32 lo, u32 hi, s32 delta)
{
    u32 a = mv0a.songptr;
    mv0a.songptr = a < lo ? a : a < hi ? 0 : a + delta;
    for (int i = 0; i < kMV0Loops; i++)
    {
        a = mv0a.loopptr[i];
        mv0a.loopptr[i] = a < lo ? a : a < hi ? 0 : a + delta;
    }
    if (mv0a.songptr == 0)
    {
        mv0a.playing = mv0a.pending = 0;
        for (int i = 0; i < 3; i++)
        {
            if (mv0a.ch[i].on && !mv0_sfx(i))
                sys_write_psg(8 + i, 0);
            mv0a.ch[i].on = 0;
        }
    }
}
