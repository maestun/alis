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

#include <SDL2/SDL.h>

#include "emu2149.h"

void mv2_soundrout(void);
void mv2_calculfrq(void);
void mv2_calculvol(void);
u32 mv2_soundvoix(u32 noteat, sAudioVoice *voice);
void mv2_checkport(u32 noteat, sAudioVoice *voice);
void mv2_soundins(sAudioVoice *voice, s16 newfreq, u16 instidx);
void mv2_checkcom(u32 noteat, u16 *volsam);
void mv2_checkefft(sAudioVoice *voice);
void mv2_soundcal(sAudioVoice *voice);
void mv2_stopmusic(void);
void mv2_onmusic(void);

void mv2_chiprout(void);
s16 mv2_chipinstr(sChipChannel *chanel, s16 idx);
void mv2_chipcanal(sChipChannel *chanel, s32 idx);
u32 mv2_chipvoix(u32 noteat, sChipChannel *chanel);

extern SDL_AudioSpec *_audio_spec;
extern PSG *_psg;

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
            if (xread8(instrument->address - 0x10) == 5)
                chipinst++;
            
            else if (xread8(instrument->address - 0x10) == 6)
                opl2inst++;
        }
        
        instrument ++;
    }
    
    mv2a.muchip = 2 < chipinst;
    mv2a.muopl2 = 2 < opl2inst;
    mv2a.mutype = 2 >= chipinst;
    
//    mv2a.mutype = 4;// alis.vquality + 1;
    audio.muvol = (audio.muvolume >> 1) + 1;
    audio.mutemp = (u8)(((u32)audio.mutempo * 6) / 0x20);

    // init
    if (mv2a.mutype != 0)
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

        // mv2a.tabvol = newaddr;
        mv2a.prevmuvol = 0;
        mv2a.prevmufreq = 0;
        mv2_calculfrq();
        mv2_calculvol();
        
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
    float ratio = 50000.0f / _audio_spec->freq;
    mv2a.frqmod = ratio * 0x48D378;
    mv2a.samples = 0x3E7 / ratio;

    s16 freq = 4; // mv2a.mutype;
    if (freq < 1)
        freq = 1;
    
    if (freq > 4)
        freq = 4;
    
    if (freq != mv2a.prevmufreq)
    {
        sAudioTrkfrq *freqdata = &mv2_trkfrq_ste[(freq - 1)];
        mv2a.mutadata = freqdata->data;
        audio.mutaloop = mv2a.samples + 1;
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
    if (mv2a.prevmuvol == 0)
    {
        mv2a.prevmuvol = -1;
        
//        // ST
//        s16 *tabvolptr = (s16 *)(mv2_audio->tabvol + 0x4000);
//        for (s32 x = 0x40; x != 0; x--)
//        {
//            s8 v = 0x3f;
//            for (s32 y = 0x7f; y != -1; y--, v--)
//            {
//                tabvolptr --;
//                *tabvolptr = ((s16)((s8)v * x >> 6) + 0x40) * 4;
//            }
//        }
        // ste/falcon
        s8 *tabvolptr = (s8 *)(mv2a.tabvol + 0x4000);
        for (s32 x = 0x40; x > 0; x--)
        {
            for (s32 y = 0xff; y > -1; y--)
            {
                tabvolptr --;
                *tabvolptr = (s8)(((s8)y * x) / 0x80);
            }
        }
    }
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
        
        memset(audio.muadresse, 0, audio.mutaloop * 2);
        
        mv2_soundcal(&mv2a.voices[0]);
        mv2_soundcal(&mv2a.voices[1]);
        mv2_soundcal(&mv2a.voices[2]);
        mv2_soundcal(&mv2a.voices[3]);
    }
}

u32 mv2_soundvoix(u32 noteat, sAudioVoice *voice)
{
    u32 notedata = xpcread32(noteat);
    if (notedata != 0)
    {
        voice->value = (s16)(notedata >> 0x10);
        voice->type = (s8)(notedata >> 8);
        voice->delta = (s8)notedata;
    }
    
    u32 nextat = (noteat + 2);
    s16 newfreq = xpcread16(noteat);
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
    
//    mv2_audio->defvolins = instidx;
    voice->freqsam = newfreq;
    s16 type = sample == 0 ? -1 : xread8(sample - 0x10);
    if (type == 2)
    {
        voice->sample = sample - 0x10;
        voice->startsam1 = sample + 0x10;
        voice->longsam1 = xread32(sample - 0xe) - 0x20;
        voice->volsam = 0x40; // mv2_audio->defvol[(mv2_audio->defvolins) + 1];
        voice->startsam2 = ((xread32(sample - 0xe) - 0x10) - xread32(sample - 4)) + sample;
        voice->longsam2 = xread32(sample - 4) - xread32(sample - 8);
    }
//    else if (type == 5) // YM3812
//    {
//
//    }
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
            u32 newval = (audio.mutempo * (data & 0x1f)) / 0x20;
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
//    mv2a.mubreak = 0;
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
    
    // Falcon
//    s16 volume = ((u16)(volsam * ((u16)(mv2_audio->muvolgen + 1) >> 1)) >> 6) - 1;
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

void mv2_soundcal(sAudioVoice *voice)
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

    u16 prevlongsam1;
    
    u8 volsam2 = volsam >> 8;
    
    float volsamf = (float)volsam2 / (float)0x3f;
    volsamf *= 64;

    u16 frqto = 0;
    bool frqnxt;
    
    u32 sample = voice->sample;
    u32 lengthX = xread32(sample + 2) - 0x11;
    u32 smpbegX = sample + 0x10;
    u32 smpendX = smpbegX + lengthX;
    
    for (int index = 0; index < audio.mutaloop; index ++)
    {
        frqnxt = frqto < frqlo;
        frqto -= frqlo;
        
        prevlongsam1 = (u16)longsam1;
        
        longsam1 -= (frqnxt + frqhi);
        if (prevlongsam1 < frqhi || (frqnxt && prevlongsam1 == frqhi))
        {
            frqlo = freq & 0xffff;
            frqhi = freq >> 0x10;
            longsam1 = longsam2;
            startsam1 = startsam2;
            if (longsam2 == 0)
                break;
        }
        
        s32 test3 = smpendX - (startsam1 + longsam1);
        s8 sam = xread8(startsam1 + test3 - 0x10);
//        s8 sam = xread8(startsam1 + longsam1);

        int total = audio.muadresse[index] + (sam * volsamf);
        if (total < -32768)
            total = -32768;
        
        if (total > 32767)
            total = 32767;
        
        audio.muadresse[index] = total;
    }

    voice->longsam1 = longsam1;
    voice->startsam1 = startsam1;
}

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

//        u16 *cht = (u16 *)chipdata;
//        for (int i = 0; i < 13; i++)
//            printf("\n%.4x %.4x", cht[i * 2 + 0], cht[i * 2 + 1]);
//        printf("\n");
        
        for (int i = 0; i < 13; i++)
        {
            PSG_writeReg(_psg, chipdata[i * 4], chipdata[4 * i + 2]);
        }
        
        memset(audio.muadresse, 0, audio.mutaloop * 2);
        for (int index = 0; index < audio.mutaloop; index ++)
        {
            audio.muadresse[index] = PSG_calc(_psg) * 4;
        }
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
        while (xread8(address) != 0xff)
        {
            u8 type = xread8(address); address ++;
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
                    u16 uVar4 = rotl(rot, result);
                    chipdata[0x2e]     = (s8)uVar4;
                    chipdata[0x32]     = (s8)(uVar4 >> 8);
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
        u32 test = xpcread16(noteat); noteat += 2;
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
