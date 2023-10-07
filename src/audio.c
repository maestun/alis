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

#include "audio.h"
#include "alis.h"
#include "ayumi.h"
#include "channel.h"

sChannel channels[4] = {{.type = 0}, {.type = 0}, {.type = 0}, {.type = 0}};
sAudio audio = {};


void playsample(eChannelType type, u8 *address, s8 freq, u8 volume, u32 length, u16 loop)
{
    s32 channelidx = -1;
    for (s32 i = 0; i < 4; i ++)
    {
        if (channels[i].type == eChannelTypeNone)
        {
            channelidx = i;
            break;
        }
    }
    
    if (channelidx >= 0)
    {
        if (freq < 0x1 || 0x14 < freq)
            freq = 10;
        
        channels[channelidx].address = address;
        channels[channelidx].volume = volume;
        channels[channelidx].length = length;
        channels[channelidx].freq = freq * 1000;
        channels[channelidx].loop = loop;
        channels[channelidx].played = 0;
        channels[channelidx].type = type;
    }
}

#define WAVE_FORMAT_IEEE_FLOAT 3

s32 save_wave_file(const s8 *name, float *data, s32 sample_rate, s32 channel_count, s32 sample_count)
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

void update_ayumi_state(struct ayumi *ay, s32 *r)
{
    ayumi_set_tone(ay, 0, (r[1] << 8) | r[0]);
    ayumi_set_tone(ay, 1, (r[3] << 8) | r[2]);
    ayumi_set_tone(ay, 2, (r[5] << 8) | r[4]);
    ayumi_set_noise(ay, r[6]);
    ayumi_set_mixer(ay, 0, r[7] & 1, (r[7] >> 3) & 1, r[8] >> 4);
    ayumi_set_mixer(ay, 1, (r[7] >> 1) & 1, (r[7] >> 4) & 1, r[9] >> 4);
    ayumi_set_mixer(ay, 2, (r[7] >> 2) & 1, (r[7] >> 5) & 1, r[10] >> 4);
    ayumi_set_volume(ay, 0, r[8] & 0xf);
    ayumi_set_volume(ay, 1, r[9] & 0xf);
    ayumi_set_volume(ay, 2, r[10] & 0xf);
    ayumi_set_envelope(ay, (r[12] << 8) | r[11]);
    if (r[13] != 255)
    {
        ayumi_set_envelope_shape(ay, r[13]);
    }
}

void ayumi_render(struct ayumi *ay, sChannel *ch, float *sample_data)
{
    s32 frame_count = ch->length;
    double frame_rate = 50;
    double clock_rate = 2000000;
    s32 sample_rate = 44100;
    s32 volume = 1;

    s32 frame = 0;
    double isr_step = frame_rate / sample_rate;
    double isr_counter = 1;
    float *out = sample_data;

    s32 amplitude = 0;
    s32 frequency = 0;
    s32 tone_off = 0;
    s32 noise_off = 1;
    s32 envelope_on = 0;

    while (frame < frame_count)
    {
        isr_counter += isr_step;
        if (isr_counter >= 1)
        {
            isr_counter -= 1;
            
            amplitude = ch->volume >> 8;
            amplitude = amplitude >> 3;
            amplitude = amplitude & 0xf;
            
//            amplitude = (ch->volume >> 3) & 0xf;
            frequency = (ch->freq >> 3);

            ayumi_set_tone(ay, 0, frequency);
//            ayumi_set_tone(ay, 1, frequency);
//            ayumi_set_tone(ay, 2, frequency);
            ayumi_set_noise(ay, 0);
            ayumi_set_mixer(ay, 0, tone_off, noise_off, envelope_on);
//            ayumi_set_mixer(ay, 1, tone_off, noise_off, envelope_on);
//            ayumi_set_mixer(ay, 2, tone_off, noise_off, envelope_on);
            ayumi_set_volume(ay, 0, amplitude & 0xf);
//            ayumi_set_volume(ay, 1, amplitude & 0xf);
//            ayumi_set_volume(ay, 2, amplitude & 0xf);
//            ayumi_set_envelope(ay, 0);
//            if (r[13] != 255)
//            {
//                ayumi_set_envelope_shape(ay, r[13]);
//            }
            
            printf("\nvol: %d (%.4x, %.2x)", ch->volume, ch->volume, amplitude);
            
            ch->volume += ch->delta_volume;
            if (0x7fff < ch->volume)
            {
                ch->volume = 0x7fff;
            }

            ch->freq += ch->delta_freq;
            if (ch->freq < 0)
            {
                ch->freq = 0;
            }
            
            frame += 1;
        }

        ayumi_process(ay);

        if (0)
        {
            ayumi_remove_dc(ay);
        }

        out[0] = (float) (ay->right * volume * 1);
        out++;
//        out[0] = (float) (ay->left * volume);
//        out[1] = (float) (ay->right * volume);
//        out += 2;
    }
}

#include "emu2149.h"

void playsound(eChannelType type, u8 pereson, u8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson)
{
    s32 channelidx = -1;
    for (s32 i = 0; i < 4; i ++)
    {
        if (channels[i].type == eChannelTypeNone)
        {
            channelidx = i;
            break;
        }
    }
    
    if (channelidx >= 0)
    {
        channels[channelidx].address = NULL;
        channels[channelidx].volume = volson << 8;
        channels[channelidx].length = longson;
        channels[channelidx].freq = freqson;
        channels[channelidx].loop = 0;
        channels[channelidx].played = 0;
        channels[channelidx].type = type;
        channels[channelidx].delta_freq = dfreqson;
        channels[channelidx].delta_volume = dvolson;
    }

    
//    s32 frame_count = longson;
//    double frame_rate = 50;
//    double clock_rate = 2000000;
//    s32 sample_rate = 44100;
//    struct ayumi ay;
//    if (!ayumi_configure(&ay, 1, clock_rate, sample_rate))
//    {
//        printf("ayumi_configure error (wrong sample rate?)\n");
//        return;
//    }
//
//    s32 sample_count = (s32)((sample_rate / frame_rate) * frame_count);
//    if (sample_count == 0)
//    {
//        printf("No frames\n");
//        return;
//    }
//
//    float *sample_data = (float *) malloc(sample_count * sizeof(float) * 2);
//    if (sample_data == NULL)
//    {
//        printf("Memory allocation error\n");
//        return;
//    }
//
//    float pan[3] = { .5, .5, .5 };
//
//    if (pan[0] >= 0)
//    {
//        ayumi_set_pan(&ay, 0, pan[0], 0);
//    }
//
//    if (pan[1] >= 0)
//    {
//        ayumi_set_pan(&ay, 1, pan[1], 0);
//    }
//
//    if (pan[2] >= 0)
//    {
//        ayumi_set_pan(&ay, 2, pan[2], 0);
//    }
//
//    ayumi_render(&ay, &(channels[channelidx]), sample_data);
//
//    if (!save_wave_file("/Users/gildor/Desktop/test.wav", sample_data, sample_rate, 1, sample_count))
//    {
//        printf("Save error\n");
//        return;
//    }
    
//    PSG *psg = PSG_new(3579545, 44100);
//    PSG_setClockDivider(psg, 1);
//    PSG_setVolumeMode(psg, 1); // YM style
//    PSG_reset(psg);
//    {
//        sChannel ch;
//        ch.address = NULL;
//        ch.volume = volson << 8;
//        ch.length = longson;
//        ch.freq = freqson;
//        ch.loop = 0;
//        ch.played = 0;
//        ch.type = type;
//        ch.delta_freq = dfreqson;
//        ch.delta_volume = dvolson;
//
//        s32 frame_count = ch.length;
//        double frame_rate = 50;
//        double clock_rate = 2000000;
//        s32 sample_rate = 44100;
//        s32 volume = 1;
//
//        s32 frame = 0;
//        double isr_step = frame_rate / sample_rate;
//        double isr_counter = 1;
//        float *out = sample_data;
//
//        s32 amplitude = 0;
//        s32 frequency = 0;
//        s32 tone_off = 0;
//        s32 noise_off = 1;
//        s32 envelope_on = 0;
//
//        while (frame < frame_count)
//        {
//            isr_counter += isr_step;
//            if (isr_counter >= 1)
//            {
//                isr_counter -= 1;
//
//                amplitude = ch.volume >> 8;
//                amplitude = amplitude >> 3;
//                amplitude = amplitude & 0xf;
//
//                frequency = (ch.freq >> 4);
//
//                PSG_writeReg(psg, 0x8, amplitude);
//                PSG_writeReg(psg, 0x0, frequency);
//                // -- TTT NNN
////                PSG_writeReg(psg, 0x7, 0b00001000);
//                PSG_writeReg(psg, 0x7, 0b11111110);
//
//
////                ayumi_set_tone(ay, 0, frequency);
////                ayumi_set_noise(ay, 0);
////                ayumi_set_mixer(ay, 0, tone_off, noise_off, envelope_on);
////                ayumi_set_volume(ay, 0, amplitude & 0xf);
//
//                ch.volume += ch.delta_volume;
//                if (0x7fff < ch.volume)
//                {
//                    ch.volume = 0x7fff;
//                }
//
//                ch.freq += ch.delta_freq;
//                if (ch.freq < 0)
//                {
//                    ch.freq = 0;
//                }
//
//                frame += 1;
//            }
//
//            s32 a = PSG_calc(psg);
//            out[0] = (float) (PSG_calc(psg) * .000125);
//            out++;
//            //        out[0] = (float) (ay->left * volume);
//            //        out[1] = (float) (ay->right * volume);
//            //        out += 2;
//        }
//
//        if (!save_wave_file("/Users/gildor/Desktop/test2.wav", sample_data, sample_rate, 1, sample_count))
//        {
//            printf("Save error\n");
//            return;
//        }
//    }

    // TODO: generate wave
//    canal[0] = 0x80;
//    canal[1] = bVar1;
//    canal[2] = typeson;
//    *(s16 *)(canal + 10) = freqson;
//    *(s16 *)(canal + 0xc) = dfreqson;
//    canal[6] = volson;
//    canal[7] = 0;
//    *(s16 *)(canal + 8) = dvolson;
//    *(s16 *)(canal + 4) = longson;
//    *(s16 *)(canal + 0xe) = pereson;
//    canal[0] = 2;
}
#include "mem.h"

void f_gomusic(void);
void f_calculfrq(void);
void f_calculvol(void);
void f_soundrout(void);
u32 f_soundvoix(u32 noteat, sAudioVoice *voice);
void f_checkport(u32 noteat, sAudioVoice *voice);
void f_soundins(sAudioVoice *voice, s16 d0w, u16 d1w);
void f_checkcom(u32 noteat, sAudioVoice *voice);
void f_checkefft(sAudioVoice *voice);
void f_soundcalSTE1(sAudioVoice *voice, u8 *muaddress);
void f_soundcalSTE2(sAudioVoice *voice, u8 *muaddress);
void f_soundcal(sAudioVoice *voice);

void playmusic(void)
{
    audio.muflag = 0;
    audio.tinstrum = audio.tabinst;
    
    u8 result = 0;
    
    sAudioInstrument *instrument = audio.tabinst;
    for (s32 i = 0; i < 0x20; i++)
    {
        if ((instrument->address != 0) && xread8(instrument->address - 0x10) == 5)
            result += 1;
        
        instrument ++;
    }
    
    if (2 < result)
    {
        audio.muchip = 1;
    }
    
    audio.mutype = (2 >= result);
    audio.mufreq = 4;// alis.vquality + 1;
    audio.muvol = (audio.muvolume >> 1) + 1;
    audio.mutemp = (u8)(((u32)audio.mutempo * 6) / 0x20);

    // init
    if (audio.mutype != 0)
    {
        audio.prevmuvol = 0;
        audio.prevmufreq = 0;
        audio.muadresse = malloc(1024 * 1024);
        // audio.tabvol = newaddr;
        f_calculfrq();
        f_calculvol();
    }

    f_gomusic();
}

int smpat = 0;
s8 testsmp[1024 * 1024 * 4];

void f_gomusic(void)
{
    for (s32 i = 0; i < 0x20; i++)
    {
        audio.defvol[i] = 0x40;
    }
    
    audio.mumax = xread8(audio.mupnote);
    audio.mupnote += 2;
    audio.mubreak = 0;
    audio.muptr = 0;
    audio.mucnt = 0;
    audio.mubufa = audio.muattac;
    audio.mubufc = audio.muchute;
    audio.muspeed = 1;
    
    for (s32 i = 0; i < 4; i++)
    {
        audio.voices[i].freqsam = 0;
        audio.voices[i].startsam1 = 0;
        audio.voices[i].longsam1 = 0;
        audio.voices[i].volsam = 0;
        audio.voices[i].startsam2 = 0;
        audio.voices[i].longsam2 = 0;
        audio.voices[i].value = 0;
        audio.voices[i].type = 0;
        audio.voices[i].delta = 0;
    }
    
    audio.muflag = 1;
    
    for (s32 i = 0; i < 8000; i++)
    {
        f_soundrout();
    }
    
    printf("\n%d\n", smpat);
}

sAudioTrkfrq trkfrq[7] = {
    { 0xB, 0xA3, 0x1B989B4 },
    { 0x9, 0xC4, 0x16FF2C1 },
    { 0x7, 0xF5, 0x1265EDE },
    { 0x5, 0x149, 0xDB6E1E },
    { 0x4, 0x19E, 0xAE3684 },
    { 0x3, 0x1EB, 0x932DE7 },
    { 0x2, 0x2A5, 0x6ACCF1 } };

sAudioTrkfrq trkfrq_st[4] = {
    { 0x52,  0x95, 0x1DDBD2B },
    { 0x3D,  0xC8, 0x1636423 },
    { 0x31,  0xF8, 0x11D7A68 },
    { 0x2E, 0x10A, 0x10BFFF8 } };

sAudioTrkfrq trkfrq_ste[4] = {
    // 0    125
    { 0x0,  0x7D, 0x23AF4D0 },
    { 0x1,  0xF9, 0x11D7A68 },
    { 0x2, 0x1F3,  0x91A6F1 },
    { 0x2, 0x1F3,  0x91A6F1 } };

void f_calculfrq(void)
{
    s16 freq = audio.mufreq;
    if (freq < 1)
        freq = 1;
    
    if (freq > 4)
        freq = 4;
    
    if (freq != audio.prevmufreq)
    {
        sAudioTrkfrq *freqdata = &trkfrq_ste[(freq - 1)];
        audio.mutadata = freqdata->data;
        audio.mutaloop = freqdata->loop;
        audio.prevmufreq = freq;
        
        s32 index = 0;
        for (s32 i = 1; i < 0x358; i++, index++)
        {
            audio.tabfrq[index] = freqdata->addr / i;
        }
    }
}

void f_calculvol(void)
{
    if (audio.prevmuvol == 0)
    {
        audio.prevmuvol = -1;
        
//        // ST
//        s16 *tabvolptr = (s16 *)(audio.tabvol + 0x4000);
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
        s8 *tabvolptr = (s8 *)(audio.tabvol + 0x4000);
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

void f_soundrout(void)
{
    u32 uVar1;
    s16 sVar5;
  
    u16 prevmuspeed = audio.muspeed;
  
    if (audio.muflag == 0)
        goto f_soundrout5;
  
    u16 wVar4 = (u16)audio.muvol;
    if (audio.mubreak == 0)
    {
//        if (audio.muattac == 0)
//        {
//            if (audio.muduree == 0)
//                goto f_soundroutc;
//
//            if (-1 < audio.muduree)
//            {
//                audio.muduree--;
//            }
//        }
//        else
//        {
//            audio.muattac -= 1;
//            uVar1 = ((u32)audio.muvol * (u32)audio.muattac) / (u32)audio.mubufa & 0xffff;
////            wVar4 = (u16)CONCAT31((int3)(uVar1 >> 8),-((s8)uVar1 - audio.muvol));
//            wVar4 = (uVar1 & 0xffffff00) | -((s8)uVar1 - audio.muvol);
//        }
        
f_soundrout1:
        
        audio.muvolgen = wVar4;
        if (audio.muspeed != 0)
        {
            audio.muspeed -= 1;
            if (audio.muspeed == 0 || (s16)prevmuspeed < 1)
            {
                audio.muspeed = audio.mutemp;

                sVar5 = audio.mucnt;
                if (-1 < (s16)(audio.mucnt + -0x40))
                {
                    sVar5 = 0;
                    audio.muptr += 1;
                }
                
                audio.mucnt = sVar5 + 1;
                if (-1 < (s16)(audio.muptr - audio.mumax))
                {
                    audio.muptr = 0;
                }
                
                u32 noteat = ((sVar5 * 0x10 + xread8(audio.mupnote + audio.muptr) * 0x400) + audio.mupnote + 0x84);
                noteat = f_soundvoix(noteat, &audio.voices[0]);
                noteat = f_soundvoix(noteat, &audio.voices[1]);
                noteat = f_soundvoix(noteat, &audio.voices[2]);
                noteat = f_soundvoix(noteat, &audio.voices[3]);
            }
            
            f_checkefft(&audio.voices[0]);
            f_checkefft(&audio.voices[1]);
            f_checkefft(&audio.voices[2]);
            f_checkefft(&audio.voices[3]);
            
            memset(audio.muadresse, 0, 1024);
            
            f_soundcal(&audio.voices[0]);
            f_soundcal(&audio.voices[1]);
            f_soundcal(&audio.voices[2]);
            f_soundcal(&audio.voices[3]);
            
            memcpy(testsmp + smpat, audio.muadresse, audio.mutaloop + 1);
            smpat += audio.mutaloop + 1;
            
            if (smpat > 750 * (audio.mutaloop + 1))
            {
                smpat = 0;
                audio.mubreak = 1;
                audio.muflag = 1;
                FILE *f = fopen("/Users/gildor/Desktop/test.smp", "wb");
                if (f == NULL)
                {
                    return;
                }
                
                fwrite(testsmp, 750 * (audio.mutaloop + 1), 1, f);
                fclose(f);
            }
            
            goto f_soundrout5;
        }
    }
    else
    {

f_soundroutc:

        if (audio.muchute != 0)
        {
            audio.muchute -= 1;
            wVar4 = (u16)(((u32)audio.muvol * (u32)audio.muchute) / (u32)audio.mubufc);
            goto f_soundrout1;
        }
    }
    
    f_stopmusic();
    
f_soundrout5:
    
//    psVar10 = audio.muadresse;
//    audio.muvar = 0;
//    if (digit1._24_2_ != 0)
//    {
//        audio.muvar = 1;
//        if (audio.muflag == 0)
//        {
//            f_digitmov((sAudioVoice *)digit1,muadresse);
//        }
//        else
//        {
//            f_digitadd((sAudioVoice *)digit1,muadresse);
//        }
//    }
//
//    if (digit2._24_2_ != 0)
//    {
//        muvar += 2;
//        if (muflag == 0)
//        {
//            f_digitmov((sAudioVoice *)digit2,psVar10 + 1);
//        }
//        else
//        {
//            f_digitadd((sAudioVoice *)digit2,psVar10 + 1);
//        }
//    }
//
//    if (digit3._24_2_ != 0)
//    {
//        muvar += 4;
//        if ((muflag == 0) && ((muvar & 1) == 0))
//        {
//            f_digitmov((sAudioVoice *)digit3,psVar10);
//        }
//        else
//        {
//            f_digitadd((sAudioVoice *)digit3,psVar10);
//        }
//    }
//
//    if (digit4._24_2_ != 0)
//    {
//        muvar += 8;
//        if ((muflag == 0) && ((muvar & 2) == 0))
//        {
//            f_digitmov((sAudioVoice *)digit4,psVar10 + 1);
//        }
//        else
//        {
//            f_digitadd((sAudioVoice *)digit4,psVar10 + 1);
//        }
//    }
//
//    uVar7 = muvar;
//    if (muflag == 0)
//    {
//        if (((muvar & 4) == 0) && (psVar9 = psVar10, uVar6 = mutaloop, (muvar & 1) == 0))
//        {
//            do
//            {
//                *psVar9 = 0;
//                if (false) break;
//                uVar6 -= 1;
//                psVar9 = psVar9 + 2;
//            } while (uVar6 != 0xffff);
//        }
//
//        if (((uVar7 & 8) == 0) && ((uVar7 & 2) == 0))
//        {
//            psVar10 = psVar10 + 1;
//            uVar7 = mutaloop;
//            do
//            {
//                *psVar10 = 0;
//                psVar10 = psVar10 + 2;
//                if (false) break;
//                uVar7 -= 1;
//            }
//            while (uVar7 != 0xffff);
//        }
//    }
//
//    psVar10 = muadresse;
//    muadresse = DAT_000163b8;
//    DAT_000163b8 = psVar10;
//    psVar9 = DAT_000163b8;
//    DAT_000163b8._3_1_ = SUB41(psVar10,0);
//    DAT_ffff8907 = DAT_000163b8._3_1_;
//    DAT_000163b8._2_1_ = (u8)((u32)psVar10 >> 8);
//    DAT_ffff8905 = DAT_000163b8._2_1_;
//    DAT_000163b8._1_1_ = (u8)((u32)psVar10 >> 0x10);
//    DAT_ffff8903 = DAT_000163b8._1_1_;
//    psVar10 = psVar10 + (mutaloop + 1) * 2;
//    DAT_ffff8913 = (s8)psVar10;
//    DAT_ffff8911 = (s8)((u32)psVar10 >> 8);
//    DAT_ffff890f = (s8)((u32)psVar10 >> 0x10);
//    DAT_000163b8 = psVar9;
    sleep(0);
}

u32 f_soundvoix(u32 noteat, sAudioVoice *voice)
{
    u32 uVar1 = xread32(noteat);
    if (uVar1 != 0)
    {
        voice->value = (s16)(uVar1 >> 0x10);
        voice->type = (s8)(uVar1 >> 8);
        voice->delta = (s8)uVar1;
    }
    
    u32 nextat = (noteat + 2);
    s16 d0w = xread16(noteat);
    if (d0w != 0)
    {
        u16 instidx = xread8(nextat);
        instidx &= 0xf0;

        if (BIT_CHK(d0w, 0xc))
        {
            d0w &= 0xfff;
            instidx |= 0x100;
        }
        
        instidx >>= 1;
        instidx -= 8;
        instidx >>= 3;

        f_checkport(nextat, voice);
        f_soundins(voice, d0w, instidx);
    }
    
    f_checkcom(nextat, voice);
    return nextat + 2;
}

void f_checkport(u32 noteat, sAudioVoice *voice)
{
    u8 d2b = xread8(noteat) & 0xf;
    if (d2b == 3 && voice->freqsam != 0 && -1 < (s16)(voice->freqsam - voice->value))
    {
        d2b = -xread8(noteat + 1);
        voice->delta = d2b;
    }
}

void f_soundins(sAudioVoice *voice, s16 d0w, u16 instidx)
{

    u32 sample = audio.tabinst[instidx].address;
    s32 iVar1 = audio.tabinst[instidx].unknown;
    if (iVar1 != 0)
    {
        iVar1 += iVar1;
        
        s32 i = 0;
        for (; i < 0x24; i++)
        {
            if (d0w == audio.trkval[i])
                break;
        }
        
        d0w = audio.trkval[i + iVar1];
    }
    
    if ((s16)(d0w - 0x71U) < 0)
    {
        d0w = 0x71;
    }
    
    if (0x357 < d0w)
    {
        d0w = 0x357;
    }
    
//    audio.defvolins = instidx;
    voice->freqsam = d0w;
    if (sample && xread8(sample - 0x10) == 2)
    {
        voice->startsam1 = sample + 0x10;
        voice->longsam1 = xread32(sample - 0xe) - 0x20;
        voice->volsam = 0x40; // audio.defvol[(audio.defvolins) + 1];
        voice->startsam2 = ((xread32(sample - 0xe) + -0x10) - xread32(sample - 4)) + sample;
        voice->longsam2 = xread32(sample - 4) - xread32(sample - 8);
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

void f_checkcom(u32 noteat, sAudioVoice *voice)
{
//    u16 data = *(u16 *)(alis.mem + noteat + 1); // xread16(noteat + 1);
    u16 data = xread8(noteat + 1);

    u8 type = xread8(noteat) & 0xf;
    switch (type)
    {
        case 0xb:
        {
            audio.muptr = data - 1;
            audio.mucnt = 0x40;
            break;
        }
        case 0xc:
        {
            if (0x40 < data)
                data = 0;
            
//            s16 test0 = xread8(noteat);
//            s16 test1 = test0 >> 4;
            
            voice->volsam = data;
//            audio.defvol[xread8(noteat) >> 4] = (s8)data;
            break;
        }
        case 0xd:
        {
            audio.mucnt = 0x40;
            break;
        }
        case 0xf:
        {
            u32 newval = (audio.mutempo * (data & 0x1f)) / 0x20;
            audio.mutemp = (u8)newval;
            audio.muspeed = (u16)newval;
            
            if (newval == 0)
            {
                sleep(0);
            }
            break;
        }
    };
}

void f_checkefft(sAudioVoice *voice)
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

void f_offmusic(void)
{
    if (audio.muchute == 0)
    {
        f_stopmusic();
    }
    else
    {
        audio.muvol = audio.muvolgen;
        audio.mubufc = audio.muchute;
        audio.mubreak = 1;
    }
}

void f_stopmusic(void)
{
    audio.muflag = 0;
    audio.mubreak = 0;
}

void f_onmusic(void)
{
    audio.mubufa = audio.muattac;
    audio.muflag = 1;
}

s16 f_volume(s16 volsam)
{
    // ST/STE
    s16 volume = ((u16)(volsam * audio.muvolgen) >> 6) - 1;
    
    // Falcon
//    s16 volume = ((u16)(volsam * ((u16)(audio.muvolgen + 1) >> 1)) >> 6) - 1;
    return volume < 0 ? 0 : volume << 8;
}

// a1 - 0x16
void f_updatevoice(sAudioVoice *a1, u32 a0, u32 d0)
{
    if (a1->loopsam == 0)
    {
        a1->startsam1 = 0;
        a1->longsam1 = 0;
        a1->volsam = 0;
        a1->startsam2 = 0;
        a1->longsam2 = 0;
        a1->value = 0;
        a1->type = 0;
        a1->delta = 0;
        a1->loopsam = 0;
        
//        FUN_00011d72();
    }
    else
    {
        a1->startsam1 = a0;
        a1->longsam1 = d0;
    }
}

void f_soundcalSTE1(sAudioVoice *voice, u8 *muaddress)
{
    u32 freq;
    s16 freqsam = voice->freqsam;
    if (freqsam < 0)
    {
        freqsam = -freqsam;
        freqsam = freqsam >> 2;
        freq = audio.tabfrq[freqsam] >> 2;
    }
    else
    {
        freq = audio.tabfrq[freqsam];
    }
    
    s16 volume = f_volume(voice->volsam);
    if (volume > 256)
    {
        sleep(0);
    }

    s32 startsam1 = voice->startsam1;
    s32 longsam1 = voice->longsam1;
    if (longsam1 != 0)
      longsam1--;

    u16 frqlo = freq & 0xffff;
    u16 frqhi = (u16)(freq >> 0x10);

    s32 startsam2 = voice->startsam2;
    s32 longsam2 = voice->longsam2 + -1;
    if (longsam2 < 1)
    {
        longsam2 = 0;
        freq = 0;
    }

    u16 frqhi2 = (u16)(freq >> 0x10);

    s8 cVar2;
    u8 uVar6;
    u16 uVar4;
    u16 uVar13;
    u16 uVar14;

    u16 uVar12 = 0;

    s16 loop = audio.mutaloop;
    if (0 < voice->loopsam)
    {
        if (audio.mutype != 0)
        {
            for (; loop > -1; loop --)
            {
                uVar13 = uVar12 - frqlo;
                uVar4 = (u16)longsam1;
                longsam1 -= ((uVar12 < frqlo) + frqhi);
                if (uVar4 < frqhi || (uVar12 < frqlo && uVar4 == frqhi))
                {
                    voice->loopsam --;
                    longsam1 = longsam2;
                    startsam1 = startsam2;
                    frqhi = frqhi2;
                }
                
                uVar6 = (u8)((u16)volume >> 8);
                muaddress[0] = *(s8 *)(audio.tabvol + xread8(startsam1 + longsam1));
                uVar14 = uVar13 - frqlo;
                uVar12 = (u16)longsam1;
                longsam1 -= ((uVar13 < frqlo) + frqhi);
                if (uVar12 < frqhi || (uVar13 < frqlo && uVar12 == frqhi))
                {
                    voice->loopsam --;
                    longsam1 = longsam2;
                    startsam1 = startsam2;
                    frqhi = frqhi2;
                }
                
                // u32 t = (((u16)uVar6 << 8) | (xread8(startsam1 + longsam1)));
                muaddress[2] = *(s8 *)(audio.tabvol + (((u16)uVar6 << 8) | (xread8(startsam1 + longsam1))));
                uVar12 = uVar14 - frqlo;
                uVar4 = (u16)longsam1;
                longsam1 -= ((uVar14 < frqlo) + frqhi);
                if (uVar4 < frqhi || (uVar14 < frqlo && uVar4 == frqhi))
                {
                    voice->loopsam --;
                    longsam1 = longsam2;
                    startsam1 = startsam2;
                    frqhi = frqhi2;
                }
                
                volume = ((u16)uVar6 << 8) | (xread8(startsam1 + longsam1));
                muaddress[4] = *(s8 *)(audio.tabvol + freqsam);
                muaddress = muaddress + 6;
            }
        }
        else
        {
            for (; loop > -1; loop --)
            {
                uVar13 = uVar12 - frqlo;
                uVar4 = (u16)longsam1;
                longsam1 -= ((uVar12 < frqlo) + frqhi);
                if (uVar4 < frqhi || (uVar12 < frqlo && uVar4 == frqhi))
                {
                    voice->loopsam --;
                    longsam1 = longsam2;
                    startsam1 = startsam2;
                    frqhi = frqhi2;
                }
                cVar2 = xread8(startsam1 + longsam1);
                muaddress[0] = cVar2;
                muaddress[1] = cVar2;

                uVar14 = uVar13 - frqlo;
                uVar12 = (u16)longsam1;
                longsam1 -= ((uVar13 < frqlo) + frqhi);
                if (uVar12 < frqhi || (uVar13 < frqlo && uVar12 == frqhi))
                {
                    voice->loopsam --;
                    longsam1 = longsam2;
                    startsam1 = startsam2;
                    frqhi = frqhi2;
                }
                cVar2 = xread8(startsam1 + longsam1);
                muaddress[2] = cVar2;
                muaddress[3] = cVar2;

                uVar12 = uVar14 - frqlo;
                uVar4 = (u16)longsam1;
//                longsam1 = (longsam1 & 0xffff0000) | (uVar4 - ((uVar14 < uVar13) + uVar9));
                longsam1 -= ((uVar14 < frqlo) + frqhi);
                if (uVar4 < frqhi || (uVar14 < frqlo && uVar4 == frqhi))
                {
                    voice->loopsam --;
                    longsam1 = longsam2;
                    startsam1 = startsam2;
                    frqhi = frqhi2;
                }
                
                cVar2 = xread8(startsam1 + longsam1);
                muaddress[4] = cVar2;
                muaddress[5] = cVar2;
                muaddress = muaddress + 6;
            }
        }
        
        f_updatevoice(voice, startsam1, longsam1);
    }
    else
    {
        for (; loop > -1; loop --)
        {
            uVar12 -= frqlo;
            uVar4 = (u16)longsam1;
//            longsam1 = (longsam1 & 0xffff0000) | (uVar4 - (bVar18 + uVar9));
            longsam1 -= ((uVar12 < frqlo) + frqhi);
            if (uVar4 < frqhi || ((uVar12 < frqlo) && frqlo == frqhi))
            {
                longsam1 = longsam2;
                startsam1 = startsam2;
                
                if (longsam2 == 0)
                    break;
            }
            // 014140 61 00 01 a6     bsr.w      f_volume                                         int f_volume(short d2w)

            u8 sample = xread8(startsam1 + longsam1);
            volume = (volume & 0xff00) | sample;
            *muaddress = *(s8 *)(audio.tabvol + volume);
            muaddress += 2;
        }

        for (; loop > -1; loop --)
        {
            *muaddress = 0;
            muaddress += 2;
        }
        
        voice->longsam1 = longsam1;
        voice->startsam1 = startsam1;
    }
}

void f_soundcalSTE2(sAudioVoice *voice, u8 *muaddress)
{
    u32 freq = audio.tabfrq[voice->freqsam];
    s32 startsam1 = voice->startsam1;
    s32 longsam1 = voice->longsam1;
    if (longsam1 != 0)
        longsam1 --;
    
    s32 startsam2 = voice->startsam2;
    s32 longsam2 = voice->longsam2 - 1;
    s16 volume = f_volume(voice->volsam);
    u32 uVar11 = 0;
    u16 frqlo = freq & 0xffff;
    u16 frqhi = (u16)(freq >> 0x10);
    if (longsam2 < 1)
    {
        longsam2 = 0;
        freq = 0;
    }
    
    u16 frqhi2 = (u16)(freq >> 0x10);
    s16 loop = audio.mutaloop;
    
    s8 value;
    u8 volhi;
    u16 uVar4;

    if (0 < (s16)voice->loopsam)
    {
        u16 uVar12;
        u16 uVar13;

        for (; loop > -1; loop --)
        {
            uVar13 = (u16)frqlo;
            uVar12 = uVar11 - uVar13;
            uVar4 = (u16)longsam1;
            longsam1 -= ((uVar11 < uVar13) + frqhi);
            if (uVar4 < frqhi || (uVar11 < uVar13 && uVar4 == frqhi))
            {
                voice->loopsam --;
                frqlo = freq & 0xffff;
                longsam1 = longsam2;
                startsam1 = startsam2;
                frqhi = frqhi2;
            }
            
            volhi = (u8)((u16)volume >> 8);
            muaddress[0] += *(s8 *)(audio.tabvol + xread8(startsam1 + longsam1));
            uVar4 = (u16)frqlo;
            uVar13 = uVar12 - uVar4;
            uVar11 = (u16)longsam1;
            longsam1 -= ((uVar12 < uVar4) + frqhi);
            if (uVar11 < frqhi || (uVar12 < uVar4 && uVar11 == frqhi))
            {
                voice->loopsam --;
                frqlo = freq & 0xffff;
                longsam1 = longsam2;
                startsam1 = startsam2;
                frqhi = frqhi2;
            }
            
            muaddress[2] += *(s8 *)(audio.tabvol + (volhi << 8 | xread8(startsam1 + longsam1)));
            uVar12 = (u16)frqlo;
            uVar11 = uVar13 - uVar12;
            uVar4 = (u16)longsam1;
            longsam1 -= ((uVar13 < uVar12) + frqhi);
            if (uVar4 < frqhi || (uVar13 < uVar12 && uVar4 == frqhi))
            {
                voice->loopsam --;
                frqlo = freq & 0xffff;
                longsam1 = longsam2;
                startsam1 = startsam2;
                frqhi = frqhi2;
            }
            
            value = *(s8 *)(audio.tabvol + (volhi << 8 | xread8(startsam1 + longsam1)));
            volume = (volhi << 8 | value);
            muaddress[4] += value;
            muaddress = muaddress + 6;
        }
        
        f_updatevoice(voice, startsam1, longsam1);
    }
    else
    {
        for (; loop > -1; loop --)
        {
            uVar11 -= frqlo;
            uVar4 = (u16)longsam1;
            longsam1 -= ((uVar11 < uVar4) + frqhi);
            if (uVar4 < frqhi || ((uVar11 < uVar4) && uVar4 == frqhi))
            {
                frqlo = freq & 0xffff;
                longsam1 = longsam2;
                startsam1 = startsam2;
                frqhi = frqhi2;
                if (longsam2 == 0)
                {
                    break;
                }
            }
            
            volhi = (u8)((u16)volume >> 8);
            value = *(s8 *)(audio.tabvol + (volhi << 8 | xread8(startsam1 + longsam1)));
            volume = volhi << 8 | value;
            *muaddress += value;
            muaddress = muaddress + 2;
        }
        
        voice->longsam1 = longsam1;
        voice->startsam1 = startsam1;
    }
}

void f_soundcal(sAudioVoice *voice)
{
    u32 freq;
    s16 freqsam = voice->freqsam;
    if (freqsam < 0)
    {
        freqsam = -freqsam;
        freqsam = freqsam >> 2;
        freq = audio.tabfrq[freqsam] >> 2;
    }
    else
    {
        freq = audio.tabfrq[freqsam];
    }

    u32 startsam1 = voice->startsam1;
    u32 longsam1 = voice->longsam1;
    u32 volsam = f_volume(voice->volsam);
    u32 startsam2 = voice->startsam2;
    u32 longsam2 = voice->longsam2 - 1;
    u16 uVar8 = 0;
    u32 frqlo = freq & 0xffff;
    u16 frqhi = (u16)(freq >> 0x10);
    if ((s32)longsam2 < 1)
    {
        longsam2 = 0;
        freq = 0;
    }

    u16 prevlongsam1;
    bool bVar12;

    for (int index = 0; index < audio.mutaloop + 1; index ++)
    {
        prevlongsam1 = (u16)frqlo;
        bVar12 = uVar8 < prevlongsam1;
        uVar8 -= prevlongsam1;
        prevlongsam1 = (u16)longsam1;
        longsam1 = ((longsam1 & 0xffff0000) | (prevlongsam1 - (bVar12 + frqhi)));
        if (prevlongsam1 < frqhi || (bVar12 && prevlongsam1 == frqhi))
        {
            frqlo = freq & 0xffff;
            frqhi = (u16)(freq >> 0x10);
            longsam1 = longsam2;
            startsam1 = startsam2;
            if (longsam2 == 0)
                break;
        }

        s8 sam = xread8(startsam1 + longsam1);

        sam *= 2;
        sam += 0x80;
        if (sam == 0)
            sam = 1;
        
        volsam = ((volsam & 0xff00) | (u8)sam);
        audio.muadresse[index] += audio.tabvol[volsam];
    }

    voice->longsam1 = longsam1;
    voice->startsam1 = startsam1;
}
