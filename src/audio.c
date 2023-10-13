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
void f_soundins(sAudioVoice *voice, s16 newfreq, u16 instidx);
void f_checkcom(u32 noteat, sAudioVoice *voice);
void f_checkefft(sAudioVoice *voice);
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

int testsmplen = 4000;
int testsmpat = 0;
s16 testsmp[1024 * 1024 * 4];

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
    
    for (s32 i = 0; i < testsmplen + 1000; i++)
    {
        f_soundrout();
    }
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
    { 0x52,  0x95, 0x1DDBD2B },     // 7 khz
    { 0x3D,  0xC8, 0x1636423 },     // 10 khz
    { 0x31,  0xF8, 0x11D7A68 },     // 12 khz
    { 0x2E, 0x10A, 0x10BFFF8 } };   // 13 khz

sAudioTrkfrq trkfrq_ste[4] = {
    { 0x0,  0x7D, 0x23AF4D0 },   // 6 khz
    { 0x1,  0xF9, 0x11D7A68 },   // 12 khz
    { 0x2, 0x1F3,  0x91A6F1 },   // 25 khz
    { 0x2, 0x1F3,  0x91A6F1 } }; // 25 khz

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
            audio.tabfrq[index] = freqdata->frqmod / i;
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
    u16 prevmuspeed = audio.muspeed;
  
    if (audio.muflag == 0)
        return;
  
    u16 newvolgen = (u16)audio.muvol;
    if (audio.mubreak == 0)
    {
        if (audio.muattac == 0)
        {
            if (audio.muduree == 0)
            {
                goto f_soundroutc;
            }
            
            if (audio.muduree > 0)
            {
                audio.muduree--;
            }
        }
        else
        {
            audio.muattac -= 1;
            u16 tmpvol = ((u32)audio.muvol * (u32)audio.muattac) / (u32)audio.mubufa;
            newvolgen = (tmpvol & 0xff00) | -((s8)tmpvol - audio.muvol);
        }
    }
    else
    {

f_soundroutc:

        if (audio.muchute != 0)
        {
            audio.muchute -= 1;
            newvolgen = (u16)(((u32)audio.muvol * (u32)audio.muchute) / (u32)audio.mubufc);
        }
        else
        {
            f_stopmusic();
            return;
        }
    }
    
    audio.muvolgen = newvolgen;
    if (audio.muspeed != 0)
    {
        audio.muspeed -= 1;
        if (audio.muspeed == 0 || (s16)prevmuspeed < 1)
        {
            audio.muspeed = audio.mutemp;

            s16 prevmucnt = audio.mucnt;
            if (-1 < (s16)(audio.mucnt - 0x40))
            {
                prevmucnt = 0;
                audio.muptr += 1;
            }
            
            audio.mucnt = prevmucnt + 1;
            if (-1 < (s16)(audio.muptr - audio.mumax))
            {
                audio.muptr = 0;
            }
            
            u32 noteat = ((prevmucnt * 0x10 + xread8(audio.mupnote + audio.muptr) * 0x400) + audio.mupnote + 0x84);
            noteat = f_soundvoix(noteat, &audio.voices[0]);
            noteat = f_soundvoix(noteat, &audio.voices[1]);
            noteat = f_soundvoix(noteat, &audio.voices[2]);
            noteat = f_soundvoix(noteat, &audio.voices[3]);
        }
        
        f_checkefft(&audio.voices[0]);
        f_checkefft(&audio.voices[1]);
        f_checkefft(&audio.voices[2]);
        f_checkefft(&audio.voices[3]);
        
        memset(audio.muadresse, 0, 1024 * 1024);
        
        f_soundcal(&audio.voices[0]);
        f_soundcal(&audio.voices[1]);
        f_soundcal(&audio.voices[2]);
        f_soundcal(&audio.voices[3]);
        
        memcpy(testsmp + testsmpat, audio.muadresse, (audio.mutaloop + 1) * 2);
        testsmpat += audio.mutaloop + 1;
        
        if (testsmpat > testsmplen * (audio.mutaloop + 1))
        {
            testsmpat = 0;
            audio.mubreak = 1;
            audio.muflag = 1;
            FILE *f = fopen("/Users/gildor/Desktop/test.smp", "wb");
            if (f == NULL)
            {
                return;
            }
            
            fwrite(testsmp, testsmplen * (audio.mutaloop + 1) * 2, 1, f);
            fclose(f);
        }
    }
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
    s16 newfreq = xread16(noteat);
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

        f_checkport(nextat, voice);
        f_soundins(voice, newfreq, instidx);
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

void f_soundins(sAudioVoice *voice, s16 newfreq, u16 instidx)
{

    u32 sample = audio.tabinst[instidx].address;
    s32 iVar1 = audio.tabinst[instidx].unknown;
    if (iVar1 != 0)
    {
        iVar1 += iVar1;
        
        s32 i = 0;
        for (; i < 0x24; i++)
        {
            if (newfreq == audio.trkval[i])
                break;
        }
        
        newfreq = audio.trkval[i + iVar1];
    }
    
    if ((s16)(newfreq - 0x71U) < 0)
    {
        newfreq = 0x71;
    }
    
    if (0x357 < newfreq)
    {
        newfreq = 0x357;
    }
    
//    audio.defvolins = instidx;
    voice->freqsam = newfreq;
    if (sample && xread8(sample - 0x10) == 2)
    {
        voice->startsam1 = sample + 0x10;
        voice->longsam1 = xread32(sample - 0xe) - 0x20;
        voice->volsam = 0x40; // audio.defvol[(audio.defvolins) + 1];
        voice->startsam2 = ((xread32(sample - 0xe) + -0x10) - xread32(sample - 4)) + sample;
        voice->longsam2 = xread32(sample - 4) - xread32(sample - 8);
        
        voice->length1 = sample; // xread32(sample - 0xe) - 0x20;
        voice->length2 = xread32(sample - 4) - xread32(sample - 8);

        char path[256];
        memset(path, 0, 256);
        {
            sprintf(path, "/Users/gildor/Desktop/sample-a-%.4x.smp", instidx);
            FILE *f = fopen(path, "wb");
            if (f == NULL)
            {
                return;
            }
            
            fwrite(voice->startsam1 + alis.mem, voice->longsam1, 1, f);
            fclose(f);
        }
        {
            sprintf(path, "/Users/gildor/Desktop/sample-b-%.4x.smp", instidx);
            FILE *f = fopen(path, "wb");
            if (f == NULL)
            {
                return;
            }
            
            fwrite(voice->startsam2 + alis.mem, voice->longsam2, 1, f);
            fclose(f);
        }
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
        
//        FUN_00011d72();
    }
    else
    {
        voice->startsam1 = smpstart;
        voice->longsam1 = smplength;
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
    
    u32 sample = voice->length1 - 0x10;
    u32 lengthX = xread32(sample + 2) - 0x11;
    u32 smpbegX = sample + 0x10;
    u32 smpendX = smpbegX + lengthX;
    
    for (int index = 0; index < audio.mutaloop + 1; index ++)
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
        s32 test4 = (lengthX - longsam1);
        test4 -= 0x10;

        s8 sam = xread8(startsam1 + test3 - 0x10);
//        s8 sam = xread8(startsam1 + longsam1);
        sam *= 2;
        sam += 0x80;
        if (sam == 0)
            sam = 1;
        
        volsam = ((volsam & 0xff00) | (u8)sam);
        
        int total = audio.muadresse[index] + (sam * volsamf);
        if (total < -32768)
            total = -32768;
        
        if (total > 32767)
            total = 32767;
        
        audio.muadresse[index] = total; // sam * 128;// volsamf; // audio.tabvol[volsam];
    }

    voice->longsam1 = longsam1;
    voice->startsam1 = startsam1;
}
