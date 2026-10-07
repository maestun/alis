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
#include "config.h"
#include "mem.h"
#include "dsp_mixer.h"


void mv1_initchannels(void);
void mv1_muroutine(void);
void mv1_checkcom(void);
void mv1_soundrout(void);
void mv1_playnote(void);
void mv1_tempmusic(u16 tempo);
void mv1_calcvolmusic(u16 vol);
void mv1_setvolmusic(s16 vol);
void mv1_stopmusic(void);


u32 mv1_tabfrq_st[] = {
    0x10000, 0x10000, 0x10000, 0x10000,
    0x10000, 0x10000, 0x10000, 0x10000,
    0x10000, 0x10000, 0x10000, 0x10000,
    0x00983, 0x00A14, 0x00AAD, 0x00B50,
    0x00BFC, 0x00CB2, 0x00D74, 0x00E40,
    0x00F19, 0x01000, 0x010F3, 0x011F5,
    0x01306, 0x01428, 0x0155B, 0x016A0,
    0x017F8, 0x01965, 0x01AE8, 0x01C81,
    0x01E33, 0x02000, 0x021E6, 0x023EA,
    0x0260D, 0x02850, 0x02AB6, 0x02D40,
    0x02FF1, 0x032CA, 0x035D0, 0x03903,
    0x03C67, 0x04000, 0x043CC, 0x047D4,
    0x04C1A, 0x050A0, 0x0556C, 0x05A80,
    0x05FE2, 0x06595, 0x06BA0, 0x07206,
    0x078CE, 0x08000, 0x08799, 0x08FA9,
    0x09834, 0x0A141, 0x0AAD8, 0x0B500,
    0x0BFC4, 0x0CB2B, 0x0D740, 0x0E40C,
    0x0F19C, 0x10000, 0x10F32, 0x11F53,
    0x13068, 0x14282, 0x155B0, 0x16A00,
    0x17F88, 0x19656, 0x1AE80, 0x1C818,
    0x1E338, 0x20000, 0x21E64, 0x23EA6,
    0x260D0, 0x28504, 0x2AB60, 0x2D400,
    0x2FF10, 0x32CAC, 0x35D00, 0x39030,
    0x3C670, 0x40000, 0x43CC8, 0x47D4C,
    0x4C1A0, 0x50A08, 0x556C0, 0x5A800,
    0x5FE20, 0x65958, 0x6BA00, 0x72060,
    0x78CE0, 0x80000, 0x87990, 0x8FA98,
    0x10000, 0x10000, 0x10000, 0x10000,
    0x10000, 0x10000, 0x10000, 0x10000,
    0x10000, 0x10000, 0x10000, 0x10000,
    0x10000, 0x10000, 0x10000, 0x10000,
    0x10000, 0x10000, 0x10000, 0x10000
};


sMV1Audio mv1a;

// Loop point of an instrument: the offset stored at `field`, relative to the sample start; 0 = none.
// Some instruments hold no loop offset there; the original then read stray memory, which outside
// the VM arena would fault here, so such a loop is dropped (the note plays once).
static u32 mv1_loop_addr(u32 instr, u32 field)
{
    s32 rel = xread32be(field);
    if (rel == 0)
        return 0;
    u32 loop = instr + rel;
    return loop < alis.platform.ram_sz ? loop : 0;
}

#ifdef ALIS_DSP_MIXER
// Per-channel note state latched for the DSP feed right after note parsing (a
// note can start and end within one mv1_checkcom). cap_seen latches across the
// 4 checkcoms; mv1_soundrout feeds and clears it.
static s8  *mv1_cap_addr[kNumMV1Channels];
static u32  mv1_cap_len [kNumMV1Channels];
static u32  mv1_cap_step[kNumMV1Channels];
static u32  mv1_cap_rep [kNumMV1Channels];   // loop length in bytes (tail loop)
static u8   mv1_cap_loop[kNumMV1Channels];
static u8   mv1_cap_seen[kNumMV1Channels];

static void mv1_step_state(int total);       // DSP-path analytic channel stepper

// Scan from a VM sample start to the first ZERO byte (mv1_playnote's sample
// terminator), giving the full played length. Cached per channel by start
// address so a sustained/repeated note isn't re-scanned every soundrout.
static u32 mv1_scan_len(int c, u32 start, u32 ramsz)
{
    static u32 scan_key[kNumMV1Channels];
    static u32 scan_val[kNumMV1Channels];
    if (start == scan_key[c] && scan_val[c])
        return scan_val[c];

    const u8 *p = (const u8 *)(alis.mem + start);
    u32 maxscan = (ramsz > start) ? (ramsz - start) : 0;
    if (maxscan > 0x8000) maxscan = 0x8000;     // bound the scan
    u32 l = 0;
    while (l < maxscan && p[l] != 0) l++;
    scan_key[c] = start;
    scan_val[c] = l;
    return l;
}
// Set on each note-on (case 1) — MV1 re-strikes the SAME one-shot sample at a
// constant address for repeated notes, so the DSP voice must be told to restart
// even though its address is unchanged. Read+cleared in mv1_soundrout's feed.
static u8   mv1_retrig  [kNumMV1Channels];

static void mv1_dsp_capture(void)
{
    u32 ramsz = alis.platform.ram_sz;
    for (int c = 0; c < kNumMV1Channels; c++)
    {
        sMV1Channel *ch = &mv1a.channels[c];
        u32 start = ch->instr;            // stable note-on sample start
        u32 end   = ch->endsam;           // loop-point VM address (0 = one-shot)
        if (ch->active != 0x2a78 || !start) continue;

        // mv1_playnote always plays [instr → first-zero]; the full length is
        // that scan regardless of loop. For a looped note (endsam != 0), endsam
        // is the LOOP START it jumps back to after the terminator, so the tail
        // loop is [endsam → zero] = full - (endsam - instr).
        u32 len = mv1_scan_len(c, start, ramsz);
        u32 rep = len;
        u8  loop = 1;
        if (end > start && end <= ramsz)
        {
            u32 intro = end - start;                  // [instr, endsam)
            if (len > intro) { rep = len - intro; loop = 2; }   // tail [endsam, zero)
            else             { rep = len;         loop = 2; }   // degenerate → loop whole
        }

        if (len && start + len <= ramsz)
        {
            mv1_cap_addr[c] = (s8 *)(alis.mem + start);
            mv1_cap_len [c] = len;
            mv1_cap_rep [c] = rep;
            mv1_cap_step[c] = ch->speedsam;                       // 16.16 step
            mv1_cap_loop[c] = loop;
            mv1_cap_seen[c] = 1;                                  // latch latest active note
        }
    }
}
#endif


void mv1_gomusic(void)
{
    audio.soundrout = mv1_soundrout;
    
    mv1a.flag1 = 0;
    mv1a.flag2 = 0;
    mv1a.muspeed = 0;
    
    u32 ratio_fp = (9600UL << 16) / audio.host_freq;  // 16.16 fixed-point
    for (int i = 0; i < 128; i++)
    {
        mv1a.tabfrq[i] = (u32)((u64)mv1_tabfrq_st[i] * ratio_fp >> 16);
    }

    mv1a.mumax = audio.host_freq / 200;
    
    set_mutaloop(mv1a.mumax * 4);
    
    u16 mutempo = (audio.mutempo & 0x7f) << 3;
    u16 muvolume = (u16)(0x7f - (audio.muvolume & 0x7f)) >> 4 & 7;
    
    for (int c = 0; c < kNumMV1Channels; c++)
    {
        mv1a.channels[c].endsam = 0;
    }
    
    mv1a.basevol = audio.muvolume;
    mv1a.basevol <<= 8;
    
    mv1a.tempo = mutempo;
    mv1a.noteptr = audio.mupnote;
    mv1a.prevvol = muvolume;
    if (mv1a.flag2 == 0)
    {
        if (mv1a.flag1 == 0)
        {
            mv1_initchannels();
            
            mv1a.effect = 0x6016;
            mv1a.flag2 = 0xff;
            
            mv1_setvolmusic(mv1a.prevvol);
        }
        else
        {
            mv1_initchannels();

            mv1a.prevvol += 2;
            if (mv1a.prevvol == 2)
                mv1a.prevvol += 3;

            mv1_calcvolmusic(mv1a.prevvol);

            mv1a.effect = 0x2a78;
            mv1a.flag2 = 0xff;
        }
    }
    else
    {
        mv1a.prevvol = mv1a.basevol;
        if (mv1a.flag1 != 0 && (mv1a.prevvol = mv1a.basevol + 2) == 2)
        {
            mv1a.prevvol = mv1a.basevol + 3;
        }
        
        mv1_setvolmusic(mv1a.prevvol);
    }

    audio.smpidx = 0;
    audio.muflag = 1;
}

void mv1_initchannels(void)
{
    for (int c = 0; c < kNumMV1Channels; c++)
    {
        mv1a.channels[c].active = 0x6010;
        mv1a.channels[c].actflag = 0;
    }
}

void mv1_muroutine(void)
{
    if (audio.muattac == 0)
    {
        if (audio.muduree == 0)
        {
            if (audio.muchute != 0 && -1 < (s16)audio.muchute)
            {
                s16 vol = mv1a.basevol - audio.dchute;
                if ((-1 < vol) && (vol != 0))
                {
                    audio.muchute += -1;
                    mv1a.basevol = vol;
                    mv1_calcvolmusic(mv1a.basevol >> 8);
                    mv1_tempmusic(audio.mutempo);
                    return;
                }
            }
            
            mv1_stopmusic();
            audio.mustate = 0;
            return;
        }
        
        audio.muduree --;
    }
    else
    {
        mv1a.basevol += audio.dattac;
        if (audio.maxvolume < audio.dattac)
        {
            audio.muattac = 1;
            mv1a.basevol = audio.maxvolume;
        }
        
        if (0x7fff < mv1a.basevol)
        {
            mv1a.basevol = 0x7fff;
        }
        
        audio.muattac --;
    }
    
    mv1_calcvolmusic(mv1a.basevol >> 8);
    mv1_tempmusic(audio.mutempo);
}

void mv1_offmusic(u32 much)
{
    audio.muchute = much + 0x10;
    audio.muattac = 0;
    audio.muduree = 1;
    
    if (audio.muchute != 0)
    {
        audio.dchute = (u16)((u32)mv1a.basevol / (u32)audio.muchute);   // live volume (music() zeroes muvolume)
    }
}

void mv1_stopmusic(void)
{
    audio.muflag = 0;
    
}

void mv1_soundrout(void)
{
    mv1a.mucnt = 0;   // 4 x mv1_checkcom() render all mutaloop samples: no clear needed
    
    mv1_muroutine();

#ifdef ALIS_DSP_MIXER
    // Clear the per-channel latch for this soundrout; each mv1_checkcom() then
    // captures its active channels (mv1_dsp_capture) before its render loop.
    if (dsp_mixer_available)
        for (int c = 0; c < kNumMV1Channels; c++) mv1_cap_seen[c] = 0;
#endif

    for (int m = 0; m < 4; m++)
    {
        mv1_checkcom();
    }

#ifdef ALIS_DSP_MIXER
    // Feed the latched voices to the DSP sample mixer (music voices 4-6;
    // voice 7 = silence — MV1 has only 3 channels). mv1a.volume is a global
    // 0..32 gain scaled to the f_volume 0..0x3F00 range.
    if (dsp_mixer_available)
    {
        s16 vol = (s16)(mv1a.volume * 504);        // 32 → 0x3F00 (full)
        for (int c = 0; c < 4; c++)
        {
            if (c >= kNumMV1Channels)
            {
                // No such MV1 channel — keep this DSP voice silent.
                dsp_mixer_update_music(c, NULL, 0, 0, 0, 1, 0, 0);
                continue;
            }

            int retrig = mv1_retrig[c];
            mv1_retrig[c] = 0;

            if (mv1_cap_seen[c])
            {
                // Fresh note geometry this soundrout — (re)feed the voice. A
                // note-on (retrig) forces a restart even at the same address;
                // otherwise the host driver continues the sustained sample.
                dsp_mixer_update_music(c, mv1_cap_addr[c], mv1_cap_len[c],
                                       mv1_cap_step[c], vol, mv1_cap_loop[c],
                                       retrig, mv1_cap_rep[c]);
            }
            else if (mv1a.channels[c].active != 0x2a78)
            {
                // Channel has gone inactive — stop the voice once.
                dsp_mixer_update_music(c, NULL, 0, 0, 0, 1, 0, 0);
            }
            // else: active but no fresh capture — keep the voice playing.
        }
    }
#endif
}

void mv1_checkcom(void)
{
    mv1a.muspeed -= mv1a.tempo;
    if (((s16)mv1a.muspeed < 0) || (mv1a.muspeed == 0))
    {
        mv1a.muspeed = 0;

        u32 nextnoteptr;
        u32 noteptr = mv1a.noteptr;
        
        while (true)
        {
            if (noteptr == 0)
            {
                noteptr = mv1a.noteptr;
            }
            
            mv1a.noteptr = noteptr + 1;

            nextnoteptr = noteptr;
            u8 speed = xread8(noteptr);
            if ((s8)speed < 0)
            {
                if (speed != 0xff)
                {
                    mv1a.muspeed = (speed - 0x80) << 8;
                    break;
                }
                
                mv1a.notedata = xread8(mv1a.noteptr);
                nextnoteptr = noteptr + 2;
            }
            
            u16 notedata = (u16)mv1a.notedata;
            if (mv1a.notedata == 0xff)
            {
                notedata = 0;
            }
            
            u8 type = notedata;
            type <<= 2;
            type >>= 2;
            
            u16 chidx = notedata >> 6;
            sMV1Channel *channel = &(mv1a.channels[chidx]);
            
            switch (type)
            {
                case 0:
                {
                    noteptr = audio.mupnote;
                    break;
                }
                case 1:
                {
                    if (xread8(nextnoteptr + 1) == 0)
                    {
                        noteptr = nextnoteptr + 2;
                        if ((xread8(nextnoteptr) == channel->unknown) && (channel->actflag != 0))
                        {
                            channel->actflag = 0;
                            channel->active = 0x6010;
                        }
                    }
                    else
                    {
                        channel->clearendsam = 0x6002;
                        s32 instr = channel->instr;
                        u16 data = channel->data;
                        channel->startsam.address = instr;
#ifdef ALIS_DSP_MIXER
                        // Note-on always rewinds the sample to instr → the DSP
                        // voice must restart even if its address is unchanged.
                        if ((chidx & 3) < kNumMV1Channels)
                            mv1_retrig[chidx & 3] = 1;
#endif
                        channel->endsam = mv1_loop_addr(instr, instr - 8);
                        u8 unkn = xread8(nextnoteptr);
                        channel->unknown = unkn;
                        noteptr = nextnoteptr + 2;
                        
                        channel->speedsam = mv1a.tabfrq[(u16)(data + (u16)unkn)];
                        u32 flag = channel->actflag;
                        channel->actflag |= 0x80;
                        if (flag == 0)
                        {
                            channel->active = 0x2a78;
                        }
                    }
                    break;
                }
                case 2:
                {
                    noteptr = nextnoteptr + 1;
                    u8 instidx = xread8(nextnoteptr) & 0x7f;
                    u32 sample = audio.tabinst[instidx].address;
                    s32 tval = audio.tabinst[instidx].unknown;
                    
                    channel->data = tval;
                    channel->instr = sample;
                    break;
                }
                case 3:
                {
                    channel->endsam = mv1_loop_addr(channel->instr, channel->instr - 4);
                    channel->clearendsam = 0x42b8;
                    noteptr = nextnoteptr;
                    break;
                }
                default:
                {
                    // Not music data (the original would jump through garbage): stop instead of spinning.
                    mv1_stopmusic();
                    for (int c = 0; c < kNumMV1Channels; c++)
                    {
                        mv1a.channels[c].active = 0x6010;
                        mv1a.channels[c].actflag = 0;
                    }
                    return;
                }
            }
        }
    }

#ifdef ALIS_DSP_MIXER
    // Capture note state NOW — after parsing, before mv1_playnote can advance a
    // short sample to its end and deactivate the channel within this checkcom.
    if (dsp_mixer_available)
        mv1_dsp_capture();
#endif

    // Channel state must advance even on the DSP path: MV1 clears active/actflag
    // at sample end, and note-on only re-activates a channel when actflag==0.
#ifdef ALIS_DSP_MIXER
    if (dsp_mixer_available)
    {
        // DSP owns the audio: advance channel state analytically, no mixing.
        mv1_step_state(mv1a.mumax);
    }
    else
#endif
    for (int l = 0; l < mv1a.mumax; l++)
    {
        mv1_playnote();
    }
}

void mv1_advance(u32 *address, u32 *fraction, u32 *addvance)
{
    u64 adv = *addvance;
    adv <<= 16;

    u64 addr = *address;
    addr <<= 32;
    addr |= *fraction;
    addr += adv;

    *address = (u32)(addr >> 32);
    *fraction = (u32)(addr);
}

#ifdef ALIS_DSP_MIXER
// DSP-path replacement for the mv1_playnote() loop: track position only, to clear
// active/actflag at sample end. The only zero byte is the terminator at
// instr+scan_len, so advance `total` samples in O(1) with the same loop-back /
// deactivate transitions. Exact for speedsam<65536 (the usual case).
static void mv1_step_state(int total)
{
    u32 ramsz = alis.platform.ram_sz;
    for (int c = 0; c < kNumMV1Channels; c++)
    {
        sMV1Channel *channel = &mv1a.channels[c];
        if (channel->active != 0x2a78) continue;

        u32 slen = mv1_scan_len(c, channel->instr, ramsz);
        u64 step = (u64)channel->speedsam << 16;
        if (!slen || !step) continue;                 // nothing to advance / no length
        u32 zero_addr = channel->instr + slen;        // terminator byte address

        u64 pos = ((u64)channel->startsam.address << 32) | channel->startsam.fraction;
        int remaining = total;
        int guard = total + 8;                        // backstop vs degenerate loops

        while (remaining > 0 && guard-- > 0)
        {
            u32 addr = (u32)(pos >> 32);
            if (addr < zero_addr)
            {
                u64 need = ((u64)zero_addr << 32) - pos;
                u64 k = (need + step - 1) / step;     // samples until addr == zero_addr
                if ((u64)remaining < k) { pos += step * (u64)remaining; remaining = 0; break; }
                pos += step * k;
                remaining -= (int)k;
                if (remaining <= 0) break;            // landed on terminator; handle next tick
            }
            // Terminator reached (one read iteration): loop back or deactivate.
            if (channel->endsam != 0)
            {
                u32 loopback = channel->endsam;
                if (channel->clearendsam == 0x42b8)
                    channel->endsam = 0;              // type-3: loop once, then stop
                pos = ((u64)loopback << 32) | (pos & 0xFFFFFFFFu);
                pos += step;                          // advance one step from the loop point
                remaining -= 1;
            }
            else
            {
                channel->active  = 0x6010;
                channel->actflag = 0;
                remaining = 0;
                break;
            }
        }

        channel->startsam.address  = (u32)(pos >> 32);
        channel->startsam.fraction = (u32)(pos & 0xFFFFFFFFu);
    }
    mv1a.mucnt += total;                              // keep parity with the per-sample path
}
#endif

void mv1_playnote(void)
{
    s16 mix = 0;
    u32 addr[] = { mv1a.channels[0].endsam, mv1a.channels[1].endsam, mv1a.channels[2].endsam };

    for (int c = 0; c < kNumMV1Channels; c++)
    {
        sMV1Channel *channel = &(mv1a.channels[c]);

        if (channel->active == 0x2a78)
        {
            s8 smp = xread8(channel->startsam.address);
            if (smp != 0)
            {
                mix += smp;
                mv1_advance(&(channel->startsam.address), &(channel->startsam.fraction), &(channel->speedsam));
            }
            else
            {
                if (channel->endsam != 0)
                {
                    if (channel->clearendsam == 0x42b8)
                        channel->endsam = 0;
                    
                    smp = xread8(addr[c]) + 1;
                    mix += smp;

                    channel->startsam.address = addr[c];
                    mv1_advance(&(channel->startsam.address), &(channel->startsam.fraction), &(channel->speedsam));
                }
                else
                {
                    channel->active = 0x6010;
                    channel->actflag = 0;
                }
            }
        }
    }

    if (mv1a.effect == 0x2a78)
    {
        ALIS_DEBUG(EDebugWarning, "MISSING MUSIC EFFECT: %s", __FUNCTION__);
    }

    mix *= mv1a.volume;
    audio.muadresse[mv1a.mucnt++] = mix;
}

void mv1_tempmusic(u16 temp)
{
    mv1a.tempo = (temp & 0x7f) << 3;
}

void mv1_calcvolmusic(u16 vol)
{
    vol = ((0x7f - (vol & 0x7f)) >> 4 & 7);
    if (mv1a.flag1 < 0 && (vol += 2) == 2)
    {
        vol += 3;
    }
    
    mv1a.prevvol = vol;
    mv1a.volume = (8 >> vol) << 2;
}

void mv1_setvolmusic(s16 vol)
{
    mv1a.volume = (8 >> vol) << 2;
}
