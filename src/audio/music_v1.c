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


void mv1_gomusic(void)
{
    audio.soundrout = mv1_soundrout;
    
    mv1a.flag1 = 0;
    mv1a.flag2 = 0;
    mv1a.muspeed = 0;
    
    float ratio = (48.0 * 200.0) / audio.host_freq;
    for (int i = 0; i < 128; i++)
    {
        mv1a.tabfrq[i] = mv1_tabfrq_st[i] * ratio;
    }

    mv1a.mumax = 48.0 / ratio;
    
    audio.mutaloop = mv1a.mumax * 4;
    
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
        audio.dchute = (u16)((u32)(s32)audio.muvolume / (u32)audio.muchute);
    }
}

void mv1_stopmusic(void)
{
    audio.muflag = 0;
    
//    if (*mv1a.flag1 != 0)
//        *mv1a.flag1 = (u16)(*mv1a.flag1) << 8;
}

void mv1_soundrout(void)
{
    memset(audio.muadresse, 0, audio.mutaloop);

    mv1a.mucnt = 0;
    
    mv1_muroutine();
    
    for (int m = 0; m < 4; m++)
    {
        mv1_checkcom();
    }
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
                        s32 instre = xread32be(instr - 8);
                        if (instre != 0)
                        {
                            instre += instr;
                        }
                        
                        channel->endsam = instre;
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
                    u8 instidx = xread8(nextnoteptr);
                    u32 sample = audio.tabinst[instidx].address;
                    s32 tval = audio.tabinst[instidx].unknown;
                    
                    channel->data = tval;
                    channel->instr = sample;
                    break;
                }
                case 3:
                {
                    s32 instre = xread32be(channel->instr - 4);
                    if (instre != 0)
                    {
                        instre += channel->instr;
                    }
                    
                    channel->endsam = instre;
                    channel->clearendsam = 0x42b8;
                    noteptr = nextnoteptr;
                    break;
                }
            }
        }
    }

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
