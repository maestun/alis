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

#include "alis.h"
#include "audio.h"
#include "config.h"
#include "mem.h"
#include "../sys/sys.h"

#include "emu2149.h"
#include "emu8950.h"
#include "dsp_mixer.h"

void mv2_soundrout(void);
void mv2_calculfrq(void);
void mv2_calculvol(void);
u32 mv2_soundvoix(u32 noteat, sAudioVoice *voice);
void mv2_checkport(u32 noteat, sAudioVoice *voice);
void mv2_soundins(sAudioVoice *voice, s16 newfreq, u16 instidx);
void mv2_checkcom(u32 noteat, u16 *volsam);
void mv2_checkefft(sAudioVoice *voice);
void mv2_soundcal(sAudioVoice *voice, const bool first);
void mv2_soundcal_c(sAudioVoice *voice, const bool first);
void mv2_stopmusic(void);
void mv2_onmusic(void);

s16 f_volume(s16 volsam);
void mv2_chiprout(void);
s16 mv2_chipinstr(sChipChannel *chanel, s16 idx);
void mv2_chipcanal(sChipChannel *chanel, s32 idx);
u32 mv2_chipvoix(u32 noteat, sChipChannel *chanel);

void mv2_opl2rout(void);
void mv2_opl2_setinst(u32 sample, u8 channel);
u32 mv2_opl2voix(u32 noteat, sAudioVoice *voice, u8 voiceidx);

sAudioTrkfrq mv2_trkfrq[7] = {
    { 0xB, 0xA3, 0x1B989B4 },
    { 0x9, 0xC4, 0x16FF2C1 },
    { 0x7, 0xF5, 0x1265EDE },
    { 0x5, 0x149, 0xDB6E1E },
    { 0x4, 0x19E, 0xAE3684 },
    { 0x3, 0x1EB, 0x932DE7 },
    { 0x2, 0x2A5, 0x6ACCF1 } };

sAudioTrkfrq mv2_trkfrq_st[4] = {
    { 0x52,  0x95, 0x1DDBD2B },     // 7 khz
    { 0x3D,  0xC8, 0x1636423 },     // 10 khz
    { 0x31,  0xF8, 0x11D7A68 },     // 12 khz
    { 0x2E, 0x10A, 0x10BFFF8 } };   // 13 khz

sAudioTrkfrq mv2_trkfrq_ste[5] = {
    { 0x0,  0x7D, 0x23AF4D0 },   // 6 khz
    { 0x1,  0xF9, 0x11D7A68 },   // 12 khz
    { 0x2, 0x1F3,  0x91A6F1 },   // 25 khz
    { 0x2, 0x1F3,  0x91A6F1 },   // 25 khz
    { 0x4, 0x3E7,  0x48D378 } }; // 50 khz

sMV2Audio mv2a;

u8 chipdata[] = {
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x00, 0x00,
    0x02, 0x02, 0x00, 0x00,
    0x03, 0x03, 0x00, 0x00,
    0x04, 0x04, 0x00, 0x00,
    0x05, 0x05, 0x00, 0x00,
    0x06, 0x06, 0x00, 0x00,
    0x07, 0x07, 0xff, 0xff,
    0x08, 0x08, 0x00, 0x00,
    0x09, 0x09, 0x00, 0x00,
    0x0a, 0x0a, 0x00, 0x00,
    0x0b, 0x0b, 0x00, 0x00,
    0x0c, 0x0c, 0x00, 0x00,
};

u16 trkval[] = {
    0x0358, 0x0328, 0x02fa, 0x02d0, 0x02a6, 0x0280, 0x025c, 0x023a, 0x021a, 0x01fc, 0x01e0, 0x01c5, 0x01ac, 0x0194, 0x017d, 0x0168, 0x0153, 0x0140, 0x012e, 0x011d, 0x010d, 0x00fe, 0x00f0, 0x00e2, 0x00d6, 0x00ca, 0x00be, 0x00b4, 0x00aa, 0x00a0, 0x0097, 0x008f,
    0x0087, 0x007f, 0x0078, 0x0071, 0x0eee, 0x0e18, 0x0d4d, 0x0c8e, 0x0bda, 0x0b2f, 0x0a8f, 0x09f7, 0x0968, 0x08e1, 0x0861, 0x07e9, 0x0777, 0x070c, 0x06a7, 0x0647, 0x05ed, 0x0598, 0x0547, 0x04fc, 0x04b4, 0x0470, 0x0431, 0x03f4, 0x03bc, 0x0386, 0x0353, 0x0324,
    0x02f6, 0x02cc, 0x02a4, 0x027e, 0x025a, 0x0238, 0x0218, 0x01fa, 0x01de, 0x01c3, 0x01aa, 0x0192, 0x017b, 0x0166, 0x0152, 0x013f, 0x012d, 0x011c, 0x010c, 0x00fd, 0x00ef, 0x00e1, 0x00d5, 0x00c9, 0x00be, 0x00b3, 0x00a9, 0x009f, 0x0096, 0x008e, 0x0086, 0x007f,
    0x0077, 0x0071, 0x006a, 0x0064, 0x005f, 0x0059, 0x0054, 0x0050, 0x004b, 0x0047, 0x0043, 0x003f, 0x003c, 0x0038, 0x0035, 0x0032, 0x002f, 0x002d, 0x002a, 0x0028, 0x0026, 0x0024, 0x0022, 0x0020, 0x001e, 0x001c, 0x001b, 0x0019, 0x0018, 0x0016, 0x0015, 0x0014,
    0x0013, 0x0012, 0x0011, 0x0010,
 };

// The ALIS 2.x DOS driver (Bunny Bricks, Ishar 2 DOS): sequencer on the ~60 Hz game timer
// (with --native-timing), effect F sets the speed to its raw parameter.
static bool mv2_old_dos(void)
{
    return alis.platform.kind == EPlatformPC && alis.platform.version < 30;
}

void mv2_gomusic(void)
{
    audio.muflag = 0;

    u8 chipinst = 0;
    u8 opl2inst = 0;

    sAudioInstrument *instrument = audio.tabinst;
    for (s32 i = 0; i < 0x20; i++)
    {
        if (instrument->address != 0)
        {
            u8 itype = xread8(instrument->address - 0x10);
            if (itype == 5)
                chipinst++;
            else if (itype == 6)
                opl2inst++;
        }
        
        instrument ++;
    }
    
    mv2a.muchip = 2 < chipinst;
    mv2a.muopl2 = 2 < opl2inst;
    mv2a.mutype = 2 >= chipinst;
    
    audio.muvol = (audio.muvolume >> 1) + 1;
    audio.mutemp = (u8)(((u32)audio.mutempo * 6) >> 5);

    // init
    if (mv2a.muopl2)
    {
        sys_init_opl();

        for (s32 i = 0; i < 4; i++)
        {
            mv2a.voices[i].freqsam = 0;
            mv2a.voices[i].startsam1 = 0;
            mv2a.voices[i].longsam1 = 0;
            mv2a.voices[i].volsam = 0;
            mv2a.voices[i].startsam2 = 0;
            mv2a.voices[i].longsam2 = 0;
            mv2a.voices[i].value = 0;
            mv2a.voices[i].type = 0;
            mv2a.voices[i].delta = 0;
        }

        mv2a.prevmuvol = 0;
        mv2a.prevmufreq = 0;
        mv2_calculfrq();
        mv2_calculvol();

        // mutaloop is sized for a 50 Hz tick; rescale to the music rate (60 Hz DOS with --native-timing).
        set_mutaloop((audio.mutaloop * 50UL) / sys_music_hz());
        audio.soundrout = mv2_opl2rout;
    }
    else if (mv2a.mutype != 0)
    {
        for (s32 i = 0; i < 4; i++)
        {
            mv2a.voices[i].freqsam = 0;
            mv2a.voices[i].startsam1 = 0;
            mv2a.voices[i].longsam1 = 0;
            mv2a.voices[i].volsam = 0;
            mv2a.voices[i].startsam2 = 0;
            mv2a.voices[i].longsam2 = 0;
            mv2a.voices[i].value = 0;
            mv2a.voices[i].type = 0;
            mv2a.voices[i].delta = 0;
        }

        mv2a.prevmuvol = 0;
        mv2a.prevmufreq = 0;
        mv2_calculfrq();
        mv2_calculvol();

        if (mv2_old_dos())
            set_mutaloop((audio.mutaloop * 50UL) / sys_music_hz());
        audio.soundrout = mv2_soundrout;
    }
    else
    {
        for (int i = 0; i < 0xc; i++)
        {
            chipdata[i * 4 + 2] = 0;
            chipdata[i * 4 + 3] = 0;
        }
        
        for (int i = 0; i < 3; i++)
        {
            memset(&mv2a.chipch[i], 0, sizeof(sChipChannel));
        }

        mv2a.chipmixer = -1;
        mv2a.prevmuvol = 0;
        mv2a.prevmufreq = 0;
        mv2_calculfrq();
        mv2_calculvol();
        
        audio.soundrout = mv2_chiprout;
    }

    for (s32 i = 0; i < 0x20; i++)
    {
        mv2a.defvol[i] = 0x40;
    }
    
    mv2a.mumax = xread8(audio.mupnote);
    audio.mupnote += 2;
    mv2a.mubreak = 0;
    mv2a.muptr = 0;
    mv2a.mucnt = 0;
    mv2a.mubufa = audio.muattac;
    mv2a.mubufc = audio.muchute;
    mv2a.muspeed = 1;
    
    audio.smpidx = 0;
    audio.muflag = 1;
}

void mv2_calculfrq(void)
{
    // Fixed-point: ratio = 50000 / host_freq (16.16)
    // frqmod = ratio * 0x48D378 = 50000 * 0x48D378 / host_freq
    // samples = 0x3E7 / ratio = 0x3E7 * host_freq / 50000
    mv2a.frqmod = (u32)((u64)50000 * 0x48D378 / audio.host_freq);
    mv2a.samples = (u32)((u64)0x3E7 * audio.host_freq / 50000);

    s16 freq = 4;
    if (freq < 1)
        freq = 1;
    
    if (freq > 4)
        freq = 4;
    
    if (freq != mv2a.prevmufreq)
    {
        sAudioTrkfrq *freqdata = &mv2_trkfrq_ste[(freq - 1)];
        mv2a.mutadata = freqdata->data;
        set_mutaloop(mv2a.samples + 1);
        mv2a.prevmufreq = freq;
        
        s32 index = 0;
        for (s32 i = 1; i < 0x358; i++, index++)
        {
            mv2a.tabfrq[index] = mv2a.frqmod / i;
        }
    }
}

void mv2_calculvol(void)
{
    // The original also built a volume table here; nothing reads it (mixers scale inline).
    if (mv2a.prevmuvol == 0)
        mv2a.prevmuvol = -1;
}

void mv2_soundrout(void)
{
    u16 prevmuspeed = mv2a.muspeed;
  
    if (audio.muflag == 0)
        return;
  
    u16 newvolgen = (u16)audio.muvol;
    if (mv2a.mubreak == 0)
    {
        if (audio.muattac == 0)
        {
            if (audio.muduree == 0)
                goto f_soundroutc;
            
            if (audio.muduree > 0)
                audio.muduree--;
        }
        else
        {
            audio.muattac--;
            u16 tmpvol = ((u32)audio.muvol * (u32)audio.muattac) / (u32)mv2a.mubufa;
            newvolgen = (tmpvol & 0xff00) | -((s8)tmpvol - audio.muvol);
        }
    }
    else
    {

f_soundroutc:

        if (audio.muchute != 0)
        {
            audio.muchute--;
            newvolgen = (u16)(((u32)audio.muvol * (u32)audio.muchute) / (u32)mv2a.mubufc);
        }
        else
        {
            mv2_stopmusic();
            return;
        }
    }
    
    mv2a.muvolgen = newvolgen;
    // Speed 0 (effect F00 = end of song): the original stops the music.
    if (mv2a.muspeed == 0)
    {
        mv2_stopmusic();
        return;
    }

    if (mv2a.muspeed != 0)
    {
        mv2a.muspeed--;
        if (mv2a.muspeed == 0 || (s16)prevmuspeed < 1)
        {
            mv2a.muspeed = audio.mutemp;

            if (-1 < (s16)(mv2a.mucnt - 0x40))
            {
                mv2a.mucnt = 0;
                mv2a.muptr++;
            }
            
            if (-1 < (s16)(mv2a.muptr - mv2a.mumax))
            {
                mv2a.muptr = 0;
            }
            
            u32 noteat = ((mv2a.mucnt * 0x10 + xread8(audio.mupnote + mv2a.muptr) * 0x400) + audio.mupnote + 0x84);
            noteat = mv2_soundvoix(noteat, &mv2a.voices[0]);
            noteat = mv2_soundvoix(noteat, &mv2a.voices[1]);
            noteat = mv2_soundvoix(noteat, &mv2a.voices[2]);
            noteat = mv2_soundvoix(noteat, &mv2a.voices[3]);
            
            mv2a.mucnt++;
        }
        
        mv2_checkefft(&mv2a.voices[0]);
        mv2_checkefft(&mv2a.voices[1]);
        mv2_checkefft(&mv2a.voices[2]);
        mv2_checkefft(&mv2a.voices[3]);
        
        // --- Sample mixing: DSP or CPU ---
#ifdef ALIS_DSP_MIXER
        if (dsp_mixer_available)
        {
            // DSP path: send voice state, DSP does all per-sample mixing.
            // No memset/soundcal needed — DSP mixes from scratch.
            for (int v = 0; v < 4; v++)
            {
                sAudioVoice *voice = &mv2a.voices[v];
                u32 freq = (voice->freqsam < 0)
                    ? mv2a.tabfrq[-voice->freqsam >> 2] >> 2
                    : mv2a.tabfrq[voice->freqsam];
                u32 lengthX = xread32(voice->sample + 2) - 0x11;
                u32 smpendX = voice->sample + 0x10 + lengthX;
                u16 volsam = f_volume(voice->volsam);

                // A stale voice can yield a wild offset that bus-errors the DSP
                // feed: out of VM range → NULL → silent.
                u32 offX = smpendX - 0x10 - voice->longsam1;
                u32 ramsz = alis.platform.ram_sz;
                s8 *maddr = (offX <= ramsz && lengthX <= ramsz - offX)
                          ? (s8 *)(alis.mem + offX) : NULL;

                dsp_mixer_update_music(v,
                    maddr,                                                 // current play position
                    lengthX,                                               // sample length (bytes)
                    freq,                                                  // 16.16 phase step (rel host_freq)
                    (s16)volsam,                                           // f_volume() result
                    (voice->longsam2 > 0) ? 2 : 1,                       // 1=one-shot, 2=loop
                    0,                                                   // MV2 restarts on address change only
                    lengthX                                              // loop whole sample (MV2's existing behavior)
                );
            }
        }
        else
#endif
        {
            // CPU fallback: original mixing path
            memset(audio.muadresse, 0, audio.mutaloop * 2);
            mv2_soundcal(&mv2a.voices[0], true);
            mv2_soundcal(&mv2a.voices[1], false);
            mv2_soundcal(&mv2a.voices[2], false);
            mv2_soundcal(&mv2a.voices[3], false);
        }
    }
}

u32 mv2_soundvoix(u32 noteat, sAudioVoice *voice)
{
    u32 notedata = xread32be(noteat);
    if (notedata != 0)
    {
        voice->value = (s16)(notedata >> 0x10);
        voice->type = (s8)(notedata >> 8);
        voice->delta = (s8)notedata;
    }
    
    u32 nextat = (noteat + 2);
    s16 newfreq = xread16be(noteat);
    if (newfreq != 0)
    {
        u16 instidx = xread8(nextat);
        instidx &= 0xf0;

        if (BIT_CHK(newfreq, 0xc))
        {
            newfreq &= 0xfff;
            instidx |= 0x100;
        }
        
        instidx >>= 1;
        instidx -= 8;
        instidx >>= 3;

        mv2_checkport(nextat, voice);
        mv2_soundins(voice, newfreq, instidx);
    }
    
    mv2_checkcom(nextat, &voice->volsam);
    return nextat + 2;
}

void mv2_checkport(u32 noteat, sAudioVoice *voice)
{
    u8 d2b = xread8(noteat) & 0xf;
    if (d2b == 3 && voice->freqsam != 0 && -1 < (s16)(voice->freqsam - voice->value))
    {
        d2b = -xread8(noteat + 1);
        voice->delta = d2b;
    }
}

void mv2_soundins(sAudioVoice *voice, s16 newfreq, u16 instidx)
{
    u32 sample = audio.tabinst[instidx].address;
    s32 tval = audio.tabinst[instidx].unknown;
    if (tval != 0)
    {
        tval += tval;
        
        s32 i = 0;
        for (; i < 0x24; i++)
        {
            if (newfreq == mv2a.trkval[i])
                break;
        }
        
        newfreq = mv2a.trkval[i + tval];
    }
    
    if ((s16)(newfreq - 0x71U) < 0)
    {
        newfreq = 0x71;
    }
    
    if (0x357 < newfreq)
    {
        newfreq = 0x357;
    }
    
    voice->freqsam = newfreq;
    s16 type = sample == 0 ? -1 : xread8(sample - 0x10);
    if (type == 2)
    {
        voice->sample = sample - 0x10;
        voice->startsam1 = sample + 0x10;
        voice->longsam1 = xread32(sample - 0xe) - 0x20;
        voice->volsam = 0x40;
        voice->startsam2 = ((xread32(sample - 0xe) - 0x10) - xread32(sample - 4)) + sample;
        voice->longsam2 = xread32(sample - 4) - xread32(sample - 8);
    }
    else if (type == 5 || type == 6)
    {
        // OPL2: NOP
    }
    else
    {
        voice->startsam1 = 0;
        voice->longsam1 = 0;
        voice->volsam = 0;
        voice->startsam2 = 0;
        voice->longsam2 = 0;
        voice->value = 0;
        voice->type = 0;
        voice->delta = 0;
        voice->loopsam = 0;
    }
}

void mv2_checkcom(u32 noteat, u16 *volsam)
{
    u16 data = xread8(noteat + 1);

    u8 type = xread8(noteat) & 0xf;
    switch (type)
    {
        case 0xb:
        {
            mv2a.muptr = data - 1;
            mv2a.mucnt = 0x40;
            break;
        }
        case 0xc:
        {
            if (0x40 < data)
                data = 0;
            
            *volsam = data;
            break;
        }
        case 0xd:
        {
            mv2a.mucnt = 0x40;
            break;
        }
        case 0xf:
        {
            // Bunny Bricks' driver skips F00 (its song then loops); later engines stop on speed 0.
            if (data == 0 && alis.platform.kind == EPlatformPC && alis.platform.uid == EGameBunnyBricks)
                break;

            u32 newval = mv2_old_dos() ? data : (audio.mutempo * (data & 0x1f)) >> 5;
            audio.mutemp = (u8)newval;
            mv2a.muspeed = (u16)newval;
            break;
        }
    };
}

void mv2_checkefft(sAudioVoice *voice)
{
    u16 newval = voice->value;
    u16 delta = (u16)voice->delta;
    u8 type = voice->type & 0xf;
    switch (type)
    {
        case 0x1:
            {
                newval = voice->freqsam - delta;
                if ((s16)(newval - 0x71) < 0)
                {
                    newval = 0x71;
                    voice->value = 0;
                    voice->type = 0;
                    voice->delta = 0;
                }
                
                voice->freqsam = newval;
                break;
            }
            
        case 0x2:
            {
                newval = voice->freqsam + delta;
                if (-1 < (s16)(newval - 0x357))
                {
                    newval = 0x357;
                    voice->value = 0;
                    voice->type = 0;
                    voice->delta = 0;
                }
                
                voice->freqsam = newval;
                break;
            }
            
        case 0x3:
            {
                if ((s16)delta < 0)
                {
                    u16 newfreq = voice->freqsam + delta;
                    if ((s16)(newfreq - newval) < 0)
                    {
                        newfreq = newval;
                    }
                    
                    voice->freqsam = newfreq;
                }
                else
                {
                    u16 newfreq = voice->freqsam + delta;
                    if (-1 < (s16)(newfreq - newval))
                    {
                        newfreq = newval;
                    }
                    
                    voice->freqsam = newfreq;
                }
                break;
            }

        case 0x5:
        case 0x6:
        case 0xa:
            {
                if (delta >> 4 == 0)
                {
                    voice->volsam -= (delta & 0xf);
                    if ((s16)voice->volsam < 0)
                    {
                        voice->volsam = 0;
                    }
                }
                else
                {
                    voice->volsam = (delta >> 4) + voice->volsam;
                    if (0x40 < voice->volsam)
                    {
                        voice->volsam = 0x40;
                    }
                }
                break;
            }
    };
}

void mv2_offmusic(u32 much)
{
    audio.muchute = much;
    if (audio.muchute == 0)
    {
        mv2_stopmusic();
    }
    else
    {
        audio.muvol = mv2a.muvolgen;
        mv2a.mubufc = audio.muchute;
        mv2a.mubreak = 1;
    }
}

void mv2_stopmusic(void)
{
    audio.muflag = 0;
}

void mv2_onmusic(void)
{
    mv2a.mubufa = audio.muattac;
    audio.muflag = 1;
}

s16 f_volume(s16 volsam)
{
    // ST/STE
    s16 volume = ((u16)(volsam * mv2a.muvolgen) >> 6) - 1;
    
    return volume < 0 ? 0 : volume << 8;
}

// a1 - 0x16
void f_updatevoice(sAudioVoice *voice, u32 smpstart, u32 smplength)
{
    if (voice->loopsam == 0)
    {
        voice->startsam1 = 0;
        voice->longsam1 = 0;
        voice->volsam = 0;
        voice->startsam2 = 0;
        voice->longsam2 = 0;
        voice->value = 0;
        voice->type = 0;
        voice->delta = 0;
        voice->loopsam = 0;
    }
    else
    {
        voice->startsam1 = smpstart;
        voice->longsam1 = smplength;
    }
}

// Pure-C reference mixer (the original decompiled body): non-Atari fallback and
// the ALIS_MV2_MIX_VERIFY oracle for the asm loop. Keep it unchanged.
void mv2_soundcal_c(sAudioVoice *voice, const bool first)
{
    u32 freq;
    s16 freqsam = voice->freqsam;
    if (freqsam < 0)
    {
        freqsam = -freqsam;
        freqsam = freqsam >> 2;
        freq = mv2a.tabfrq[freqsam] >> 2;
    }
    else
    {
        freq = mv2a.tabfrq[freqsam];
    }

    u32 startsam1 = voice->startsam1;
    u32 longsam1 = voice->longsam1;
    u16 volsam = f_volume(voice->volsam);
    u32 startsam2 = voice->startsam2;
    u32 longsam2 = voice->longsam2 - 1;
    u16 frqlo = freq & 0xffff;
    u16 frqhi = freq >> 0x10;
    if ((s32)longsam2 < 1)
    {
        longsam2 = 0;
        freq = 0;
    }

    u8 volsam2 = volsam >> 8;
    s16 vol_fixed = (s16)((u32)volsam2);

    u16 frqto = 0;

    u32 sample = voice->sample;
    u32 lengthX = xread32(sample + 2) - 0x11;
    u32 smpendX = sample + 0x10 + lengthX;
    if (!sample || smpendX > alis.finmem)
        return;   // no sample, or a stale header (script data moved)

    u8 * const mem_base = alis.mem;
    s16 * const dst = audio.muadresse;
    const u32 mutaloop = audio.mutaloop;
    const u32 end_adj = smpendX - 0x10;
    
    u32 step_full = ((u32)frqhi << 16) | frqlo;

    if (first)
    {
        for (u32 index = 0; index < mutaloop; index++)
        {
            u32 acc_before = ((u32)(u16)longsam1 << 16) | frqto;
            u32 acc_after;
            int phase_switch = __builtin_sub_overflow(acc_before, step_full, &acc_after);

            frqto = (u16)acc_after;
            longsam1 = ((longsam1 >> 16) - (u32)phase_switch) << 16 | (u32)(acc_after >> 16);

            if (phase_switch)
            {
                frqlo = freq & 0xffff;
                frqhi = freq >> 0x10;
                step_full = ((u32)frqhi << 16) | frqlo;
                longsam1 = longsam2;
                startsam1 = startsam2;
                if (longsam2 == 0)
                    break;
            }

            dst[index] = (s16)((s8)mem_base[end_adj - longsam1] * vol_fixed);
        }
    }
    else
    {
        for (u32 index = 0; index < mutaloop; index++)
        {
            u32 acc_before = ((u32)(u16)longsam1 << 16) | frqto;
            u32 acc_after;
            s32 phase_switch = __builtin_sub_overflow(acc_before, step_full, &acc_after);

            frqto = (u16)acc_after;
            longsam1 = ((longsam1 >> 16) - (u32)phase_switch) << 16 | (u32)(acc_after >> 16);

            if (phase_switch)
            {
                frqlo = freq & 0xffff;
                frqhi = freq >> 0x10;
                step_full = ((u32)frqhi << 16) | frqlo;
                longsam1 = longsam2;
                startsam1 = startsam2;
                if (longsam2 == 0)
                    break;
            }

            s16 scaled_val = (s16)((s8)mem_base[end_adj - longsam1] * vol_fixed);
            int total = dst[index] + scaled_val;
            if (total < -32768)
                total = -32768;

            if (total > 32767)
                total = 32767;

            dst[index] = (s16)total;
        }
    }

    voice->longsam1 = longsam1;
    voice->startsam1 = startsam1;
}

// ============================================================================
// Register-pinned asm version of mv2_soundcal_c (same semantics; avoids ST-RAM
// stack spills). The reload's step reset is omitted: freq is 0 only when
// longsam2==0, which breaks on the first reload anyway.
// ============================================================================

#if defined(ALIS_MV2_MIX_ASM) && ALIS_MV2_MIX_ASM && defined(ALIS_USE_NATIVE_ATARI)

struct mv2_asm_ctx { u32 longsam2; u32 startsam2; u32 startsam1; };

// Runs the per-sample DDA over [dst, dst+mutaloop) into dst. Returns final
// longsam1; writes final startsam1 into ctx->startsam1. `first` picks the
// write vs accumulate+clamp body (chosen once, outside the hot loop).
__attribute__((noinline))
static u32 mv2_soundcal_asm(s16 *dst, u32 mutaloop, const u8 *read_base,
                            s16 vol_fixed, u32 step_full, u32 longsam1,
                            int first, struct mv2_asm_ctx *ctx)
{
    u32 ls1 = longsam1;
    u32 frq = 0;                       // frqto (starts 0)
    u16 flo = (u16)(step_full & 0xffff);
    u16 fhi = (u16)(step_full >> 16);
    u16 vol = (u16)vol_fixed;
    u32 cnt = mutaloop - 1;            // dbf counter (mutaloop>=1 guaranteed by caller)
    u32 s1, s2;

    if (first)
    {
        __asm__ volatile (
        "0: sub.w   %[flo],%[frq]              \n"  // frqto -= frqlo, X = borrow
        "   subx.w  %[fhi],%[ls1]              \n"  // ls1_lo -= frqhi - X ; C = phase_switch
        "   bcs     3f                         \n"  // sample stepped -> reload
        "1: move.l  %[ls1],%[s1]               \n"  // s1 = longsam1
        "   neg.l   %[s1]                      \n"  // s1 = -longsam1  (index = end_adj-longsam1)
        "   move.b  %[rb]@(0,%[s1]:l),%[s1]    \n"  // s1 = (s8) sample byte
        "   ext.w   %[s1]                      \n"
        "   muls.w  %[vol],%[s1]               \n"  // * vol_fixed (product fits s16)
        "   move.w  %[s1],%[dp]@+              \n"  // dst[i] = scaled
        "   dbf     %[cnt],0b                  \n"
        "   bra     9f                         \n"
        "3: move.l  %[ctx]@(4),%[s1]           \n"  // s1 = startsam2
        "   move.l  %[s1],%[ctx]@(8)           \n"  // ctx->startsam1 = startsam2
        "   move.l  %[ctx]@,%[ls1]             \n"  // longsam1 = longsam2 ; sets Z
        "   beq     9f                         \n"  // longsam2==0 -> break (no write)
        "   bra     1b                         \n"  // else write with new longsam1
        "9:                                    \n"
        : [ls1]"+d"(ls1), [frq]"+d"(frq), [cnt]"+d"(cnt), [dp]"+a"(dst),
          [s1]"=&d"(s1), [s2]"=&d"(s2)
        : [flo]"d"(flo), [fhi]"d"(fhi), [vol]"d"(vol),
          [rb]"a"(read_base), [ctx]"a"(ctx)
        : "cc", "memory"
        );
    }
    else
    {
        __asm__ volatile (
        "0: sub.w   %[flo],%[frq]              \n"
        "   subx.w  %[fhi],%[ls1]              \n"
        "   bcs     3f                         \n"
        "1: move.l  %[ls1],%[s1]               \n"
        "   neg.l   %[s1]                      \n"
        "   move.b  %[rb]@(0,%[s1]:l),%[s1]    \n"
        "   ext.w   %[s1]                      \n"
        "   muls.w  %[vol],%[s1]               \n"  // s1 = scaled (32-bit, fits s16)
        "   move.w  %[dp]@,%[s2]               \n"  // s2 = dst[i]
        "   ext.l   %[s2]                      \n"
        "   add.l   %[s2],%[s1]                \n"  // total = dst[i] + scaled
        "   cmp.l   #32767,%[s1]               \n"  // clamp high
        "   ble     4f                         \n"
        "   move.l  #32767,%[s1]               \n"
        "4: cmp.l   #-32768,%[s1]              \n"  // clamp low
        "   bge     5f                         \n"
        "   move.l  #-32768,%[s1]              \n"
        "5: move.w  %[s1],%[dp]@+              \n"  // dst[i] = total
        "   dbf     %[cnt],0b                  \n"
        "   bra     9f                         \n"
        "3: move.l  %[ctx]@(4),%[s1]           \n"
        "   move.l  %[s1],%[ctx]@(8)           \n"
        "   move.l  %[ctx]@,%[ls1]             \n"
        "   beq     9f                         \n"
        "   bra     1b                         \n"
        "9:                                    \n"
        : [ls1]"+d"(ls1), [frq]"+d"(frq), [cnt]"+d"(cnt), [dp]"+a"(dst),
          [s1]"=&d"(s1), [s2]"=&d"(s2)
        : [flo]"d"(flo), [fhi]"d"(fhi), [vol]"d"(vol),
          [rb]"a"(read_base), [ctx]"a"(ctx)
        : "cc", "memory"
        );
    }

    (void)frq; (void)cnt; (void)s1; (void)s2;
    return ls1;
}

#if defined(ALIS_MV2_MIX_VERIFY) && ALIS_MV2_MIX_VERIFY
// Bit-exact harness counters (updated from the mixer tick — which may run in the
// audio ISR — so plain volatiles, no dbglog here). Reported from a main-loop
// safe point (dsp_mixer_poll, [mv2mix]).
#define MV2_MIX_VERIFY_MAX 4096
volatile unsigned long mv2_mix_calls = 0;
volatile unsigned long mv2_mix_bad   = 0;
volatile long          mv2_mix_bi[6];   // last mismatch: index, asm, C, first, asm_ls1^C_ls1, asm_ss1^C_ss1
static s16 mv2_mix_scratch[MV2_MIX_VERIFY_MAX];
static s16 mv2_mix_predst [MV2_MIX_VERIFY_MAX];
#endif

// Computes the mixer setup exactly as mv2_soundcal_c(), then runs the pinned asm
// loop (or, flag-off / non-Atari, defers to the pure-C oracle).
void mv2_soundcal(sAudioVoice *voice, const bool first)
{
    // --- setup: identical to mv2_soundcal_c() ---
    u32 freq;
    s16 freqsam = voice->freqsam;
    if (freqsam < 0)
    {
        freqsam = -freqsam;
        freqsam = freqsam >> 2;
        freq = mv2a.tabfrq[freqsam] >> 2;
    }
    else
    {
        freq = mv2a.tabfrq[freqsam];
    }

    u32 startsam1 = voice->startsam1;
    u32 longsam1 = voice->longsam1;
    u16 volsam = f_volume(voice->volsam);
    u32 startsam2 = voice->startsam2;
    u32 longsam2 = voice->longsam2 - 1;
    u16 frqlo = freq & 0xffff;
    u16 frqhi = freq >> 0x10;
    if ((s32)longsam2 < 1)
    {
        longsam2 = 0;
        freq = 0;
    }

    u8 volsam2 = volsam >> 8;
    s16 vol_fixed = (s16)((u32)volsam2);

    u32 sample = voice->sample;
    u32 lengthX = xread32(sample + 2) - 0x11;
    u32 smpendX = sample + 0x10 + lengthX;
    if (!sample || smpendX > alis.finmem)
        return;   // no sample, or a stale header (script data moved)

    u8 * const mem_base = alis.mem;
    s16 * const dst = audio.muadresse;
    const u32 mutaloop = audio.mutaloop;
    const u32 end_adj = smpendX - 0x10;

    u32 step_full = ((u32)frqhi << 16) | frqlo;
    (void)freq;   // step reset on reload is a no-op (see header)

    if (mutaloop == 0)
    {
        voice->longsam1 = longsam1;
        voice->startsam1 = startsam1;
        return;
    }

    const u8 *read_base = mem_base + end_adj;   // read_base[-(s32)longsam1] == mem_base[end_adj-longsam1]

#if defined(ALIS_MV2_MIX_VERIFY) && ALIS_MV2_MIX_VERIFY
    if (mutaloop <= MV2_MIX_VERIFY_MAX)
    {
        // Snapshot pre-mix dst, then run the trusted C oracle LIVE (keeps
        // muadresse correct), then run the asm into a scratch and compare.
        for (u32 i = 0; i < mutaloop; i++) mv2_mix_predst[i] = dst[i];

        mv2_soundcal_c(voice, first);          // trusted result -> live dst + voice
        u32 c_ls1 = voice->longsam1;
        u32 c_ss1 = voice->startsam1;

        for (u32 i = 0; i < mutaloop; i++) mv2_mix_scratch[i] = mv2_mix_predst[i];
        struct mv2_asm_ctx ctx = { longsam2, startsam2, startsam1 };
        u32 a_ls1 = mv2_soundcal_asm(mv2_mix_scratch, mutaloop, read_base,
                                     vol_fixed, step_full, longsam1, first, &ctx);
        u32 a_ss1 = ctx.startsam1;

        mv2_mix_calls++;
        int bad = 0; long bidx = -1; long ba = 0, bc = 0;
        for (u32 i = 0; i < mutaloop; i++)
        {
            if (mv2_mix_scratch[i] != dst[i])
            {
                if (!bad) { bidx = (long)i; ba = mv2_mix_scratch[i]; bc = dst[i]; }
                bad = 1;
            }
        }
        if (a_ls1 != c_ls1 || a_ss1 != c_ss1) bad = 1;
        if (bad)
        {
            mv2_mix_bad++;
            mv2_mix_bi[0] = bidx; mv2_mix_bi[1] = ba; mv2_mix_bi[2] = bc;
            mv2_mix_bi[3] = first ? 1 : 0;
            mv2_mix_bi[4] = (long)(a_ls1 ^ c_ls1);
            mv2_mix_bi[5] = (long)(a_ss1 ^ c_ss1);
        }
        return;   // live dst already holds the trusted C result
    }
    // mutaloop too large for the scratch buffers: fall through to plain asm.
#endif

    struct mv2_asm_ctx ctx = { longsam2, startsam2, startsam1 };
    longsam1 = mv2_soundcal_asm(dst, mutaloop, read_base,
                                vol_fixed, step_full, longsam1, first, &ctx);

    voice->longsam1 = longsam1;
    voice->startsam1 = ctx.startsam1;
}

#else  // !ALIS_MV2_MIX_ASM || !native: pure C

void mv2_soundcal(sAudioVoice *voice, const bool first)
{
    mv2_soundcal_c(voice, first);
}

#endif

#pragma mark Atari STE chipmusic

void mv2_chiprout(void)
{
    u16 prevmuspeed = mv2a.muspeed;
    
    if (audio.muflag == 0)
        return;
    
    if (mv2a.mutype != 0)
        return;
    
    u16 newvolgen = (u16)audio.muvol;
    if (mv2a.mubreak == 0)
    {
        if (audio.muattac == 0)
        {
            if (audio.muduree == 0)
                goto f_chiprouttc;
            
            audio.muduree--;
        }
        else
        {
            audio.muattac--;

            u16 tmpvol = ((u32)audio.muvol * (u32)audio.muattac) / (u32)mv2a.mubufa;
            newvolgen = (tmpvol & 0xff00) | -((s8)tmpvol - audio.muvol);
        }
    }
    else
    {
        
f_chiprouttc:
        
        if (audio.muchute != 0)
        {
            audio.muchute--;
            newvolgen = (u16)(((u32)audio.muvol * (u32)audio.muchute) / (u32)mv2a.mubufc);
        }
        else
        {
            mv2_stopmusic();
            return;
        }
    }
    
    mv2a.muvolgen = newvolgen;
    // Speed 0 (effect F00 = end of song): the original stops the music.
    if (mv2a.muspeed == 0)
    {
        mv2_stopmusic();
        return;
    }

    if (mv2a.muspeed != 0)
    {
        mv2a.muspeed--;
        
        if (mv2a.muspeed == 0 || (s16)(prevmuspeed < 1))
        {
            mv2a.muspeed = audio.mutemp;

            if (-1 < (s16)(mv2a.mucnt - 0x40))
            {
                mv2a.mucnt = 0;
                mv2a.muptr++;
            }
            
            if (-1 < (s16)(mv2a.muptr - mv2a.mumax))
            {
                mv2a.muptr = 0;
            }
            
            u32 noteat = (mv2a.mucnt * 0x10 + xread8(audio.mupnote + mv2a.muptr) * 0x400) + audio.mupnote + 0x84;
            if (mv2a.muchip != 0)
            {
                noteat += 4;
            }

            noteat = mv2_chipvoix(noteat, &mv2a.chipch[0]);
            noteat = mv2_chipvoix(noteat, &mv2a.chipch[1]);
            noteat = mv2_chipvoix(noteat, &mv2a.chipch[2]);
            
            mv2a.mucnt++;
        }
        
        mv2_chipcanal(&mv2a.chipch[0], 0);
        mv2_chipcanal(&mv2a.chipch[1], 1);
        mv2_chipcanal(&mv2a.chipch[2], 2);
        
        chipdata[0x1e] = mv2a.chipmixer;
        
        for (int i = 0; i < 13; i++)
        {
            sys_write_psg(chipdata[i * 4], chipdata[4 * i + 2]);
        }
        
#if !defined(__TOS__) && !defined(__atarist__)
        sys_calc_psg_music();
#endif
    }
}

s16 mv2_chipinstr(sChipChannel *chanel, s16 idx)
{
    if ((s16)(idx + 0x18) < 0)
        idx = -0x18;

    if (-1 < (s16)(idx - 0xa8))
        idx = 0xa8;
    
    s16 result = trkval[48 + idx / 2];
    s16 alt = trkval[48 + (s8)chanel->unknown4];

    s16 newuA = chanel->unknownA;
    if (chanel->notedata != 0)
    {
        if (chanel->notedata < 0)
        {
            newuA += chanel->notedata;
            result += newuA;
            if ((s16)(result - alt) < 0)
            {
                chanel->notedata = 0;
                chanel->tvalue = chanel->unknown4;
                chanel->unknown4 = 0;
                newuA = 0;
                result = alt;
            }
            
            chanel->unknownA = newuA;
        }
        else
        {
            newuA += chanel->notedata;
            result += newuA;
            if (-1 < (s16)(result - alt))
            {
                chanel->notedata = 0;
                chanel->tvalue = chanel->unknown4;
                chanel->unknown4 = 0;
                newuA = 0;
                result = alt;
            }
            
            chanel->unknownA = newuA;
        }
    }
    
    return result;
}

u32 rotl(u8 rotate, u32 value)
{
    if ((rotate &= sizeof(value) * 8 - 1) == 0)
        return value;
    
    return (value << rotate) | (value >> (sizeof(value) * 8 - rotate));
}

void mv2_chipcanal(sChipChannel *chanel, s32 idx)
{
    s32 idx1 = idx * 8;
    s32 idx2 = 0x22 + idx * 4;
    
    s16 tval = chanel->tvalue;
    s32 address = chanel->address1;
    if (address != 0)
    {
        u8 type;
        while ((type = xread8(address)) != 0xff)
        {
            address ++;
            if (type == 0xf5)
            {
                mv2a.chipmixer |= 1 << idx;
                mv2a.chipmixer |= 1 << (idx + 3);

                type = xread8(address); address ++;
            }
            else {
                if (type == 0xfd)
                {
                    mv2a.chipmixer |= 1 << idx;
                    mv2a.chipmixer &= ~(1 << (idx + 3));

                    type = xread8(address); address ++;
                }
                if (type == 0xfe)
                {
                    mv2a.chipmixer &= ~(1 << idx);
                    mv2a.chipmixer |= 1 << (idx + 3);

                    type = xread8(address); address ++;
                }
                if (type == 0xfc)
                {
                    mv2a.chipmixer &= ~(1 << idx);
                    mv2a.chipmixer &= ~(1 << (idx + 3));

                    type = xread8(address); address ++;
                }
            }
            
            if (type == 0xfb)
            {
                chipdata[0x1a] = xread8(address); address ++;
                type = xread8(address); address ++;
            }
            
            if (type != 0xfa)
            {
                if (type == 0xf9)
                {
                    chipdata[idx2]     = 0x10;
                    chipdata[0x32]     = 0;
                    chipdata[0x2e]     = xread8(address); address ++;
                    chipdata[idx1 + 6] = xread8(address); address ++;
                    chipdata[idx1 + 2] = xread8(address); address ++;
                }
                else if (type == 0xf8)
                {
                    chipdata[idx2]     = 0x10;
                    chipdata[idx1 + 6] = xread8(address); address ++;
                    chipdata[idx1 + 2] = xread8(address); address ++;
                    s16 result = mv2_chipinstr(chanel, (s8)(xread8(address) + (s8)tval) * 2); address ++;
                    chipdata[0x2e]     = (s8)(result);
                    chipdata[0x32]     = (s8)(result >> 8);
                }
                else if (type == 0xf7)
                {
                    chipdata[idx2]     = 0x10;
                    chipdata[0x32]     = xread8(address); address ++;
                    chipdata[0x2e]     = xread8(address); address ++;
                    s16 result = mv2_chipinstr(chanel, (s8)(xread8(address) + (s8)tval) * 2); address ++;
                    chipdata[idx1 + 2] = (s8)(result);
                    chipdata[idx1 + 6] = (s8)(result >> 8);
                }
                else if (type == 0xf6)
                {
                    chipdata[idx2]     = 0x10;
                    s16 result = mv2_chipinstr(chanel, (s8)(xread8(address) + (s8)tval) * 2); address ++;
                    chipdata[idx1 + 2] = (s8)(result);
                    chipdata[idx1 + 6] = (s8)(result >> 8);
                    u16 rot = xread8(address); address ++;
                    u16 rotres = rotl(rot, result);
                    chipdata[0x2e]     = (s8)rotres;
                    chipdata[0x32]     = (s8)(rotres >> 8);
                }
                else
                {
                    u8 add = xread8(address); address ++;
                    chipdata[idx2]     = (s8)(((u32)type * ((u16)(mv2a.muvolgen * 10) >> 6)) / 0xf);
                    s16 result = mv2_chipinstr(chanel, (s8)(xread8(address) + (s8)tval) * 2); address ++;
                    result += add;
                    chipdata[idx1 + 2] = (s8)(result);
                    chipdata[idx1 + 6] = (s8)(result >> 8);
                }
                
                chanel->address1 = address;
                break;
            }
            
            address = chanel->address2 + xread8(address);
        }
    }
}

u32 mv2_chipvoix(u32 noteat, sChipChannel *chanel)
{
    u32 address = 0;
    
    do
    {
        u32 test = xread16be(noteat); noteat += 2;
        if (test == 0)
        {
            goto chipvoixend;
        }
        
        int i = 0;
        for (; i < 36; i++)
        {
            u16 b = trkval[i];
            if (test == b)
                break;
        }
        
        s16 instidx = (((xread8(noteat) & 0xf0) >> 1) - 8) >> 3;
        address = audio.tabinst[instidx].address;
        
        s16 tval = audio.tabinst[instidx].unknown + i;

        if ((xread8(noteat) & 0xf) == 3)
        {
            chanel->unknown4 = tval;
            u16 notedata = (u16)xread8(noteat + 1);
            if ((s16)(chanel->tvalue - chanel->unknown4) < 0)
            {
                notedata = -notedata;
            }
            
            chanel->notedata = notedata;
            chanel->tvalue = chanel->tvalue;
            goto chipvoixend;
        }
        
        chanel->unknown4 = 0;
        chanel->notedata = 0;
        chanel->unknownA = 0;
        chanel->tvalue = tval;
    }
    while (address == 0);
    
    if (xread8(address - 0x10) == 5)
    {
        chanel->address1 = address;
        chanel->address2 = address;

    chipvoixend:
        
        mv2_checkcom(noteat, &chanel->volume);
    }
    
    return noteat + 2;
}

#pragma mark - OPL2 music

// OPL2 operator register offsets per channel (modulator, carrier)
static const u8 opl2_op_offset[9][2] = {
    { 0x00, 0x03 }, // ch 0
    { 0x01, 0x04 }, // ch 1
    { 0x02, 0x05 }, // ch 2
    { 0x08, 0x0B }, // ch 3
    { 0x09, 0x0C }, // ch 4
    { 0x0A, 0x0D }, // ch 5
    { 0x10, 0x13 }, // ch 6
    { 0x11, 0x14 }, // ch 7
    { 0x12, 0x15 }, // ch 8
};

static void mv2_opl2_write_op(u8 op_offset, const u8 *data, u8 global_vol, u8 scale_vol)
{
    // 13-byte operator parameter block
    u8 ksl   = data[0];
    u8 mul   = data[1];
    u8 ar    = data[3];
    u8 sl    = data[4];
    u8 eg    = data[5];
    u8 dr    = data[6];
    u8 rr    = data[7];
    u8 tl    = data[8];
    u8 am    = data[9];
    u8 vib   = data[10];
    u8 ksr   = data[11];

    // scale TL by global volume
    u8 adjusted_tl = tl;
    if (scale_vol)
    {
        adjusted_tl = 0x3f - (u8)(((u16)global_vol * (u16)(0x3f - tl)) >> 6);
    }

    // Register 0x40: KSL(2) + TL(6) - KSL shifted left 6
    u8 reg40 = ((ksl & 0x03) << 6) | (adjusted_tl & 0x3f);
    // Register 0x20: AM(1) VIB(1) EG(1) KSR(1) MUL(4)
    u8 reg20 = (mul & 0x0f) | ((eg & 1) << 5) | ((am & 1) << 7) | ((vib & 1) << 6) | ((ksr & 1) << 4);
    // Register 0x60: AR(4) + DR(4)
    u8 reg60 = ((ar & 0x0f) << 4) | (dr & 0x0f);
    // Register 0x80: SL(4) + RR(4)
    u8 reg80 = ((sl & 0x0f) << 4) | (rr & 0x0f);

    sys_write_opl(0x40 + op_offset, reg40);
    sys_write_opl(0x20 + op_offset, reg20);
    sys_write_opl(0x60 + op_offset, reg60);
    sys_write_opl(0x80 + op_offset, reg80);
    // NOTE: Waveform (0xE0) is written separately in mv2_opl2_setinst
}

void mv2_opl2_setinst(u32 sample, u8 channel)
{
    if (channel >= 9)
        return;

    const u8 *instdata = (const u8 *)(alis.mem + sample);

    // Instrument: 0-1 header (channel override), 2-14 modulator, 15-27 carrier,
    // 28/29 modulator/carrier waveform (reg 0xE0).
    const u8 *mod_data = instdata + 2;
    const u8 *car_data = instdata + 15;

    u8 mod_offset = opl2_op_offset[channel][0];
    u8 car_offset = opl2_op_offset[channel][1];

    u8 global_vol = mv2a.muvolgen;

    // Connection/algorithm from modulator block byte 12
    u8 cnt = mod_data[12];
    u8 connection = cnt ^ 1;

    // Key-off first
    if (channel < 6)
        sys_write_opl(0xB0 + channel, 0);

    // Write modulator operator registers
    // Volume scaling: in FM mode (connection=0), only carrier is scaled
    // In additive mode (connection=1), both are scaled
    mv2_opl2_write_op(mod_offset, mod_data, global_vol, connection);

    // Write carrier operator registers (always volume-scaled)
    if (car_offset != 0xFF)
        mv2_opl2_write_op(car_offset, car_data, global_vol, 1);

    // Waveform select registers (0xE0) - from bytes 28-29 after both operator blocks
    sys_write_opl(0xE0 + mod_offset, instdata[28] & 0x03);
    if (car_offset != 0xFF)
        sys_write_opl(0xE0 + car_offset, instdata[29] & 0x03);

    // Feedback/Connection register (0xC0) - only for melody channels (< 7)
    if (channel < 7)
    {
        u8 feedback = mod_data[2]; // FB field from modulator block
        u8 regC0 = connection | ((feedback & 0x07) << 1);
        sys_write_opl(0xC0 + channel, regC0);
    }
}

static void mv2_opl2_noteon(u8 channel, s16 freqsam)
{
    if (channel >= 9 || freqsam <= 0)
        return;

    // Convert freqsam (Atari ST YM2149 period) to OPL2 F-Number + Block
    // YM2149 freq = 2MHz / (16 * period) = 125000 / period
    // OPL2 freq = 49716 * F-Number / 2^(20-Block)
    // So: F-Number * 2^Block = 125000 * 2^20 / (49716 * period) = 0x283986 / period
    // NOTE: The DOS original used 0xF041A0 with its own period table, but
    // our VM uses Atari ST periods which are ~6x smaller, so we need 0x283986.
    u32 raw = 0x283986 / (u32)freqsam;

    // Decompose into F-Number (10 bits, 0-1023) and Block (3 bits, 0-7)
    // Shift right until F-Number fits in 10 bits, counting Block
    s32 block = 0;
    while (raw >= 1024 && block < 7)
    {
        raw >>= 1;
        block++;
    }

    u16 fnum = raw & 0x3FF;

    // Write F-Number low byte to register 0xA0+channel
    sys_write_opl(0xA0 + channel, fnum & 0xFF);

    // Write F-Number high 2 bits + Block + Key-On to register 0xB0+channel
    u8 regB0 = ((fnum >> 8) & 0x03) | ((block & 0x07) << 2) | 0x20;
    sys_write_opl(0xB0 + channel, regB0);
}

static void mv2_opl2_noteoff(u8 channel)
{
    if (channel >= 9)
        return;

    // Key-off: clear the key-on bit in 0xB0 register
    sys_write_opl(0xB0 + channel, 0);
}

u32 mv2_opl2voix(u32 noteat, sAudioVoice *voice, u8 voiceidx)
{
    u32 notedata = xread32be(noteat);
    if (notedata != 0)
    {
        voice->value = (s16)(notedata >> 0x10);
        voice->type = (s8)(notedata >> 8);
        voice->delta = (s8)notedata;
    }

    u32 nextat = (noteat + 2);
    s16 newfreq = xread16be(noteat);
    if (newfreq != 0)
    {
        u16 instidx = xread8(nextat);
        instidx &= 0xf0;

        if (BIT_CHK(newfreq, 0xc))
        {
            newfreq &= 0xfff;
            instidx |= 0x100;
        }

        instidx >>= 1;
        instidx -= 8;
        instidx >>= 3;

        u32 sample = audio.tabinst[instidx].address;
        if (sample != 0 && xread8(sample - 0x10) == 6)
        {
            // Determine OPL2 channel
            u8 channel = voiceidx; // default: voice index = channel
            u8 *instdata = alis.mem + sample;
            if (instdata[0] == 1)
            {
                channel = instdata[1]; // override channel
            }

            // Clamp frequency
            if ((s16)(newfreq - 0x71U) < 0)
                newfreq = 0x71;
            if (0x357 < newfreq)
                newfreq = 0x357;

            voice->freqsam = newfreq;

            // Program instrument on OPL2 channel
            mv2_opl2_setinst(sample, channel);

            // Play note
            mv2_opl2_noteon(channel, newfreq);

            voice->volsam = 0x40;
            voice->loopsam = channel; // store OPL2 channel for later use
        }
        else
        {
            // Note with no valid instrument: note off
            if ((newfreq & 0xfff) == 0)
            {
                mv2_opl2_noteoff(voice->loopsam);
            }
        }
    }
    else if ((newfreq & 0xfff) == 0 && notedata == 0)
    {
        // Silent: nothing to do
    }

    mv2_checkcom(nextat, &voice->volsam);
    return nextat + 2;
}

void mv2_opl2rout(void)
{
    u16 prevmuspeed = mv2a.muspeed;

    if (audio.muflag == 0)
        return;

    u16 newvolgen = (u16)audio.muvol;
    if (mv2a.mubreak == 0)
    {
        if (audio.muattac == 0)
        {
            if (audio.muduree == 0)
                goto f_opl2routc;

            if (audio.muduree > 0)
                audio.muduree--;
        }
        else
        {
            audio.muattac--;
            u16 tmpvol = ((u32)audio.muvol * (u32)audio.muattac) / (u32)mv2a.mubufa;
            newvolgen = (tmpvol & 0xff00) | -((s8)tmpvol - audio.muvol);
        }
    }
    else
    {

f_opl2routc:

        if (audio.muchute != 0)
        {
            audio.muchute--;
            newvolgen = (u16)(((u32)audio.muvol * (u32)audio.muchute) / (u32)mv2a.mubufc);
        }
        else
        {
            mv2_stopmusic();
            return;
        }
    }

    mv2a.muvolgen = newvolgen;
    // Speed 0 (effect F00 = end of song): the original stops the music.
    if (mv2a.muspeed == 0)
    {
        mv2_stopmusic();
        return;
    }

    if (mv2a.muspeed != 0)
    {
        mv2a.muspeed--;
        if (mv2a.muspeed == 0 || (s16)prevmuspeed < 1)
        {
            mv2a.muspeed = audio.mutemp;

            s16 prevmucnt = mv2a.mucnt;
            if (-1 < (s16)(mv2a.mucnt - 0x40))
            {
                mv2a.mucnt = 0;
                mv2a.muptr++;
            }

            if (-1 < (s16)(mv2a.muptr - mv2a.mumax))
            {
                mv2a.muptr = 0;
            }

            u32 noteat = ((mv2a.mucnt * 0x10 + xread8(audio.mupnote + mv2a.muptr) * 0x400) + audio.mupnote + 0x84);
            noteat = mv2_opl2voix(noteat, &mv2a.voices[0], 0);
            noteat = mv2_opl2voix(noteat, &mv2a.voices[1], 1);
            noteat = mv2_opl2voix(noteat, &mv2a.voices[2], 2);
            noteat = mv2_opl2voix(noteat, &mv2a.voices[3], 3);

            mv2a.mucnt++;
        }

        // Render OPL2 into the music buffer (no-op on Atari: the DSP synthesizes).
        sys_calc_opl_music();

        // End of OPL frame: capture mark + DSP FM feed.
        sys_opl_frame_tick();
    }
}
