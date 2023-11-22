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
#include "mem.h"

#include <SDL2/SDL.h>

extern SDL_AudioSpec *_audio_spec;

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

void f_gomusic(void);
void f_calculfrq(void);
void f_calculvol(void);
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
    
    u8 chipinst = 0;
    u8 opl2inst = 0;

    sAudioInstrument *instrument = audio.tabinst;
    for (s32 i = 0; i < 0x20; i++)
    {
        if (instrument->address != 0)
        {
            if (xread8(instrument->address - 0x10) == 5)
                chipinst += 1;
            
            else if (xread8(instrument->address - 0x10) == 6)
                opl2inst += 1;
        }
        
        instrument ++;
    }
    
    if (2 < chipinst)
        audio.muchip = 1;
    
    if (2 < opl2inst)
        audio.muopl2 = 1;
    
    audio.mutype = (2 >= chipinst);
    audio.mufreq = 4;// alis.vquality + 1;
    audio.muvol = (audio.muvolume >> 1) + 1;
    audio.mutemp = (u8)(((u32)audio.mutempo * 6) / 0x20);

    // init
    if (audio.mutype != 0)
    {
        audio.prevmuvol = 0;
        audio.prevmufreq = 0;
        // audio.muadresse = malloc(1024 * 1024);
        // audio.tabvol = newaddr;
        f_calculfrq();
        f_calculvol();
    }

    f_gomusic();
}

//s32 testsmplen = 4000;
//s32 testsmpat = 0;
//s16 testsmp[1024 * 1024 * 4];

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
    
    audio.smpidx = 0;
    audio.muflag = 1;
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

sAudioTrkfrq trkfrq_ste[5] = {
    { 0x0,  0x7D, 0x23AF4D0 },   // 6 khz
    { 0x1,  0xF9, 0x11D7A68 },   // 12 khz
    { 0x2, 0x1F3,  0x91A6F1 },   // 25 khz
    { 0x2, 0x1F3,  0x91A6F1 },   // 25 khz
    { 0x4, 0x3E7,  0x48D378 } }; // 50 khz

void f_calculfrq(void)
{
    float ratio = 50000.0f / _audio_spec->freq;
    audio.frqmod = ratio * 0x48D378;
    audio.samples = 0x3E7 / ratio;

    s16 freq = audio.mufreq;
    if (freq < 1)
        freq = 1;
    
    if (freq > 4)
        freq = 4;
    
    if (freq != audio.prevmufreq)
    {
        sAudioTrkfrq *freqdata = &trkfrq_ste[(freq - 1)];
        audio.mutadata = freqdata->data;
        audio.mutaloop = audio.samples;
        audio.prevmufreq = freq;
        
        s32 index = 0;
        for (s32 i = 1; i < 0x358; i++, index++)
        {
            audio.tabfrq[index] = audio.frqmod / i;
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
        
//        memcpy(testsmp + testsmpat, audio.muadresse, (audio.mutaloop + 1) * 2);
//        testsmpat += audio.mutaloop + 1;
//
//        if (testsmpat > testsmplen * (audio.mutaloop + 1))
//        {
//            testsmpat = 0;
//            audio.mubreak = 1;
//            audio.muflag = 1;
//            FILE *f = fopen("/Users/gildor/Desktop/test.smp", "wb");
//            if (f == NULL)
//            {
//                return;
//            }
//
//            fwrite(testsmp, testsmplen * (audio.mutaloop + 1) * 2, 1, f);
//            fclose(f);
//        }
    }
}

u32 f_soundvoix(u32 noteat, sAudioVoice *voice)
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
    s32 tval = audio.tabinst[instidx].unknown;
    if (tval != 0)
    {
        tval += tval;
        
        s32 i = 0;
        for (; i < 0x24; i++)
        {
            if (newfreq == audio.trkval[i])
                break;
        }
        
        newfreq = audio.trkval[i + tval];
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
    s16 type = sample == 0 ? -1 : xread8(sample - 0x10);
    if (type == 2)
    {
        voice->sample = sample - 0x10;
        voice->startsam1 = sample + 0x10;
        voice->longsam1 = xpcread32(sample - 0xe) - 0x20;
        voice->volsam = 0x40; // audio.defvol[(audio.defvolins) + 1];
        voice->startsam2 = ((xpcread32(sample - 0xe) - 0x10) - xpcread32(sample - 4)) + sample;
        voice->longsam2 = xpcread32(sample - 4) - xpcread32(sample - 8);

//        char path[256];
//        memset(path, 0, 256);
//        {
//            sprintf(path, "/Users/gildor/Desktop/sample-a-%.4x.smp", instidx);
//            FILE *f = fopen(path, "wb");
//            if (f == NULL)
//            {
//                return;
//            }
//
//            fwrite(voice->startsam1 + alis.mem, voice->longsam1, 1, f);
//            fclose(f);
//        }
//        {
//            sprintf(path, "/Users/gildor/Desktop/sample-b-%.4x.smp", instidx);
//            FILE *f = fopen(path, "wb");
//            if (f == NULL)
//            {
//                return;
//            }
//
//            fwrite(voice->startsam2 + alis.mem, voice->longsam2, 1, f);
//            fclose(f);
//        }
    }
//    else if (type == 5) // YM2149
//    {
//
//    }
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

void f_checkcom(u32 noteat, sAudioVoice *voice)
{
//    u16 data = *(u16 *)(alis.mem + noteat + 1); // xpcread16(noteat + 1);
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
    
    u32 sample = voice->sample;
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
        s8 sam = xread8(startsam1 + test3 - 0x10);
//        s8 sam = xread8(startsam1 + longsam1);
//        if (alis.platform.kind == EPlatformAtari)
//        {
//            sam *= 2;
//            sam += 0x80;
//            if (sam == 0)
//                sam = 1;
//        }
        
//        volsam = ((volsam & 0xff00) | (u8)sam);
        
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

typedef union {
    struct __attribute__((packed)) {

        u32 address;
        u32 unknown;
    };

    struct __attribute__((packed)) {

        u16 a;
        u32 b;
        u16 c;
    };

} sAudioUnk01;

u8 xfsam;
u32 xmupnote2;

u32 xmu_acia;
// u32 xmubuf[1024 * 1024];
u32 xmupnoteptr;
u32 xmuprev_acia;
u16 xmuspeed;
u16 xmutemp;
void (* xmutimer_a)(void) = NULL;
void (* xmutimer_c)(void) = NULL;

u8 xmuval01 = 0;
u8 xmuval02 = 0;
u16 xmuval03 = 0;
u16 xmuval06 = 0x0405;
u16 xmuval07[4];
//u16 xmuval08[1024];
u8 xmuval09 = 0;
u16 xmuval10 = 0x6064;
u8 xmuval11;
u16 xmsp_command0[3];
u16 xmsp_command1[3];
u32 xmsp_command2[3];
u32 muactinstru[3];
u16 muactidata[3];
u16 xsmp_value;
u8 xmuval18;
u8 xmuval19;
u8 xmuval20;
//u8 xmuval21;
u8 xmuval22;
u16 xmuval23;
u8 xmuval24;
u16 xmuval25;
u32 xmuval26;
u32 xmuval27;
u8 xmuval28 = 0x7f;

u16 xmuvolume = 0;

u16 xmuvol = 0;

u8 xmu[1024];

sAudioUnk01 xmu240[3];
//u32 xmu242;
//u16 xmu248;
//u32 xmu24a;
//u32 xmu24e;

u8 xmu258;
u32 xmu259;

u32 xmu25c_smp_addr2[3];
u8 xmu268[3];

u8 xnotedata;
u16 xvolsam[] = {
    0xE74B, // lsl.w #0x3,D3w
    0xE54B, // lsl.w #0x2,D3w
    0xD643, // add.w d3w,d3w
    0x4E71, // nop
    0x4243, // clr.w D3w
    0x4243,
    0x4243,
    0x4243,
    0x4243,
    0x4243,
    0x4243
};

typedef struct {
    
    u8 interupt_mask_A;
    u8 interupt_ins_A;
    u8 interupt_pending_A;
    u8 interupt_enable_A;
    u8 timer_A_cont;
    u8 timer_data01;
    void (* timer_a)(void);
    u32 acia;
    void (* timer_c)(void);

} sTimerData;

sTimerData timerdata;
u32 interupt_mask_A;
u32 interupt_ins_A;
u32 interupt_pending_A;
u32 interupt_enable_A;
u32 timer_A_cont;
u32 timer_data01;
u32 acia;
u32 interupt_mask_A2;
u32 interupt_pending_A2;
u32 interupt_enable_A2;
u32 timer_A_cont2;

void (* timer_a)(void) = NULL;
void (* timer_c)(void) = NULL;

u32 xmutable1[] = {
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

u32 xmutable2[] = {
    0x080e090d, 0x0a0a0c0c,    0x080f0903, 0x0a0a0000,    0x080f0903, 0x0a0a0000,    0x080f0903, 0x0a0a0000,    0x080f0903, 0x0a0a0000,    0x080f0903, 0x0a0a0000,    0x080f0903, 0x0a0a0000,    0x080e090d, 0x0a0a0b0b,
    0x080e090d, 0x0a0a0b0b,    0x080e090d, 0x0a0a0b0b,    0x080e090d, 0x0a0a0b0b,    0x080e090d, 0x0a0a0b0b,    0x080e090d, 0x0a0a0b0b,    0x080e090d, 0x0a0a0b0b,    0x080e090d, 0x0a0a0a0a,    0x080e090d, 0x0a0a0a0a,
    0x080e090d, 0x0a0a0a0a,    0x080e090d, 0x0a0a0a0a,    0x080e090c, 0x0a0a0c0c,    0x080e090d, 0x0a0a0000,    0x080d090d, 0x0a0a0d0d,    0x080d090d, 0x0a0a0d0d,    0x080d090d, 0x0a0a0d0d,    0x080d090d, 0x0a0a0d0d,
    0x080d090d, 0x0a0a0d0d,    0x080d090d, 0x0a0a0d0d,    0x080e090c, 0x0a0a0b0b,    0x080e090c, 0x0a0a0b0b,    0x080e090c, 0x0a0a0b0b,    0x080e090c, 0x0a0a0b0b,    0x080e090c, 0x0a0a0b0b,    0x080e090c, 0x0a0a0b0b,
    0x080e090c, 0x0a0a0b0b,    0x080e090c, 0x0a0a0b0b,    0x080e090c, 0x0a0a0a0a,    0x080e090c, 0x0a0a0a0a,    0x080e090c, 0x0a0a0a0a,    0x080e090c, 0x0a0a0a0a,    0x080d090d, 0x0a0a0c0c,    0x080d090d, 0x0a0a0c0c,
    0x080e090c, 0x0a0a0909,    0x080e090c, 0x0a0a0909,    0x080e090c, 0x0a0a0505,    0x080e090c, 0x0a0a0000,    0x080e090c, 0x0a0a0000,    0x080e090b, 0x0a0a0b0b,    0x080e090b, 0x0a0a0b0b,    0x080e090b, 0x0a0a0b0b,
    0x080e090b, 0x0a0a0b0b,    0x080e090b, 0x0a0a0a0a,    0x080e090b, 0x0a0a0a0a,    0x080e090b, 0x0a0a0a0a,    0x080d090d, 0x0a0a0b0b,    0x080d090d, 0x0a0a0b0b,    0x080d090d, 0x0a0a0b0b,    0x080e090b, 0x0a0a0909,
    0x080e090b, 0x0a0a0909,    0x080e090b, 0x0a0a0909,    0x080d090c, 0x0a0a0c0c,    0x080d090d, 0x0a0a0a0a,    0x080e090b, 0x0a0a0707,    0x080e090b, 0x0a0a0000,    0x080e090b, 0x0a0a0000,    0x080d090d, 0x0a0a0909,
    0x080d090d, 0x0a0a0909,    0x080e090a, 0x0a0a0909,    0x080d090d, 0x0a0a0808,    0x080d090d, 0x0a0a0707,    0x080d090d, 0x0a0a0404,    0x080d090d, 0x0a0a0000,    0x080e090a, 0x0a0a0404,    0x080e0909, 0x0a0a0909,
    0x080e0909, 0x0a0a0909,    0x080d090c, 0x0a0a0b0b,    0x080e0909, 0x0a0a0808,    0x080e0909, 0x0a0a0808,    0x080e0909, 0x0a0a0707,    0x080e0908, 0x0a0a0808,    0x080e0909, 0x0a0a0101,    0x080c090c, 0x0a0a0c0c,
    0x080d090c, 0x0a0a0a0a,    0x080e0908, 0x0a0a0606,    0x080e0907, 0x0a0a0707,    0x080e0908, 0x0a0a0000,    0x080e0907, 0x0a0a0505,    0x080e0906, 0x0a0a0606,    0x080d090c, 0x0a0a0909,    0x080e0905, 0x0a0a0505,
    0x080e0904, 0x0a0a0404,    0x080d090c, 0x0a0a0808,    0x080d090b, 0x0a0a0b0b,    0x080e0900, 0x0a0a0000,    0x080d090c, 0x0a0a0606,    0x080d090c, 0x0a0a0505,    0x080d090c, 0x0a0a0202,    0x080c090c, 0x0a0a0b0b,
    0x080c090c, 0x0a0a0b0b,    0x080d090b, 0x0a0a0a0a,    0x080d090b, 0x0a0a0a0a,    0x080d090b, 0x0a0a0a0a,    0x080d090b, 0x0a0a0a0a,    0x080c090c, 0x0a0a0a0a,    0x080c090c, 0x0a0a0a0a,    0x080c090c, 0x0a0a0a0a,
    0x080d090b, 0x0a0a0909,    0x080d090b, 0x0a0a0909,    0x080d090a, 0x0a0a0a0a,    0x080d090a, 0x0a0a0a0a,    0x080d090a, 0x0a0a0a0a,    0x080c090c, 0x0a0a0909,    0x080c090c, 0x0a0a0909,    0x080c090c, 0x0a0a0909,
    0x080d090b, 0x0a0a0606,    0x080c090b, 0x0a0a0b0b,    0x080c090c, 0x0a0a0808,    0x080d090b, 0x0a0a0000,    0x080d090b, 0x0a0a0000,    0x080c090c, 0x0a0a0707,    0x080c090c, 0x0a0a0606,    0x080c090c, 0x0a0a0505,
    0x080c090c, 0x0a0a0303,    0x080c090c, 0x0a0a0101,    0x080c090b, 0x0a0a0a0a,    0x080d090a, 0x0a0a0505,    0x080d090a, 0x0a0a0404,    0x080d090a, 0x0a0a0202,    0x080d0909, 0x0a0a0808,    0x080d0909, 0x0a0a0808,
    0x080c090b, 0x0a0a0909,    0x080c090b, 0x0a0a0909,    0x080d0908, 0x0a0a0808,    0x080b090b, 0x0a0a0b0b,    0x080d0909, 0x0a0a0505,    0x080c090b, 0x0a0a0808,    0x080d0909, 0x0a0a0202,    0x080d0908, 0x0a0a0606,
    0x080c090b, 0x0a0a0707,    0x080d0907, 0x0a0a0707,    0x080c090b, 0x0a0a0606,    0x080c090a, 0x0a0a0909,    0x080b090b, 0x0a0a0a0a,    0x080c090b, 0x0a0a0202,    0x080c090b, 0x0a0a0000,    0x080c090a, 0x0a0a0808,
    0x080d0906, 0x0a0a0404,    0x080d0905, 0x0a0a0505,    0x080d0905, 0x0a0a0404,    0x080c0909, 0x0a0a0909,    0x080d0904, 0x0a0a0303,    0x080b090b, 0x0a0a0909,    0x080c090a, 0x0a0a0505,    0x080b090a, 0x0a0a0a0a,
    0x080c0909, 0x0a0a0808,    0x080b090b, 0x0a0a0808,    0x080c090a, 0x0a0a0000,    0x080c090a, 0x0a0a0000,    0x080c0909, 0x0a0a0707,    0x080b090b, 0x0a0a0707,    0x080c0909, 0x0a0a0606,    0x080b090b, 0x0a0a0606,
    0x080b090a, 0x0a0a0909,    0x080b090b, 0x0a0a0505,    0x080a090a, 0x0a0a0a0a,    0x080b090b, 0x0a0a0202,    0x080b090a, 0x0a0a0808,    0x080c0907, 0x0a0a0707,    0x080c0908, 0x0a0a0404,    0x080c0907, 0x0a0a0606,
    0x080b0909, 0x0a0a0909,    0x080c0906, 0x0a0a0606,    0x080a090a, 0x0a0a0909,    0x080c0907, 0x0a0a0303,    0x080b090a, 0x0a0a0505,    0x080b0909, 0x0a0a0808,    0x080b090a, 0x0a0a0303,    0x080a090a, 0x0a0a0808,
    0x080b090a, 0x0a0a0000,    0x080b0909, 0x0a0a0707,    0x080b0908, 0x0a0a0808,    0x080a090a, 0x0a0a0707,    0x080a0909, 0x0a0a0909,    0x080c0901, 0x0a0a0101,    0x080a090a, 0x0a0a0606,    0x080b0908, 0x0a0a0707,
    0x080a090a, 0x0a0a0505,    0x080a0909, 0x0a0a0808,    0x080a090a, 0x0a0a0202,    0x080a090a, 0x0a0a0101,    0x080a090a, 0x0a0a0000,    0x08090909, 0x0a0a0909,    0x080a0908, 0x0a0a0808,    0x080b0908, 0x0a0a0101,
    0x080a0909, 0x0a0a0606,    0x080b0907, 0x0a0a0404,    0x080a0909, 0x0a0a0505,    0x08090909, 0x0a0a0808,    0x080a0909, 0x0a0a0303,    0x080a0908, 0x0a0a0606,    0x080a0909, 0x0a0a0000,    0x08090909, 0x0a0a0707,
    0x08090908, 0x0a0a0808,    0x080a0908, 0x0a0a0404,    0x08090909, 0x0a0a0606,    0x080a0908, 0x0a0a0101,    0x08090909, 0x0a0a0505,    0x08090908, 0x0a0a0707,    0x08080908, 0x0a0a0808,    0x08090909, 0x0a0a0202,
    0x08090908, 0x0a0a0606,    0x08090909, 0x0a0a0000,    0x08090907, 0x0a0a0707,    0x08080908, 0x0a0a0707,    0x08090907, 0x0a0a0606,    0x08090908, 0x0a0a0202,    0x08080908, 0x0a0a0606,    0x08090906, 0x0a0a0606,
    0x08080907, 0x0a0a0707,    0x08080908, 0x0a0a0404,    0x08080907, 0x0a0a0606,    0x08080908, 0x0a0a0202,    0x08070907, 0x0a0a0707,    0x08080906, 0x0a0a0606,    0x08080907, 0x0a0a0404,    0x08070907, 0x0a0a0606,
    0x08080906, 0x0a0a0505,    0x08080906, 0x0a0a0404,    0x08070906, 0x0a0a0606,    0x08070907, 0x0a0a0404,    0x08080905, 0x0a0a0404,    0x08060906, 0x0a0a0606,    0x08070906, 0x0a0a0404,    0x08070905, 0x0a0a0505,
    0x08060906, 0x0a0a0505,    0x08060906, 0x0a0a0404,    0x08060905, 0x0a0a0505,    0x08060906, 0x0a0a0202,    0x08060905, 0x0a0a0404,    0x08050905, 0x0a0a0505,    0x08060905, 0x0a0a0202,    0x08050905, 0x0a0a0404,
    0x08050904, 0x0a0a0404,    0x08050905, 0x0a0a0202,    0x08040904, 0x0a0a0404,    0x08040904, 0x0a0a0303,    0x08040904, 0x0a0a0202,    0x08040903, 0x0a0a0303,    0x08030903, 0x0a0a0303,    0x08030903, 0x0a0a0202,
    0x08030902, 0x0a0a0202,    0x08020902, 0x0a0a0202,    0x08020902, 0x0a0a0101,    0x08010901, 0x0a0a0101,    0x08020901, 0x0a0a0000,    0x08010901, 0x0a0a0000,    0x08010900, 0x0a0a0000,    0x08000900, 0x0a0a0000
};

void o_gomusic(u16 mutempo, u16 muvolume);
void o_gettimers(void);
void o_mufce01(void);
void o_muroutine(void);
void o_offmusic(void);
void o_offmusic2(s16 d0w);
void o_clearsam(void);
void o_settimers(void);
void o_soundrout(void);
void o_soundrout2(void);
void o_tempmusic(u16 d0w);
void o_volmusic(u16 d0w);
void o_volmusic1(s16 vol);
void o_volmusic2(s16 d0w);
void o_volume(s16 vol);

void xgomusic(void)
{
    xfsam = 1;
    o_gomusic((audio.mutempo & 0x7f) << 3, (u16)(0x7f - (audio.muvolume & 0x7f)) >> 4 & 7);
}


u32 xmuidx = 0;
u8 xmubuffer[1024 * 1024 * 16];

void o_gomusic(u16 mutempo, u16 muvolume)
{
    memset(xmubuffer, 0, 1024 * 1024);
    
    xmu25c_smp_addr2[0] = 0;
    xmu25c_smp_addr2[1] = 0;
    xmu25c_smp_addr2[2] = 0;
    
    xmuvolume = audio.muvolume;
    xmuvolume <<= 8;
    
    xmutemp = mutempo;
    xmupnoteptr = audio.mupnote;
    xmuvol = muvolume;
    if (xmuval02 == 0)
    {
        if (xmuval01 == 0)
        {
            o_mufce01();
            
            xmuval23 = 0x6016;
            xmuval02 = 0xff;
            
            o_gettimers();
            
            timer_a = o_soundrout2;
            timer_c = o_soundrout;
            xmuval03 = xmuval06;
            
            o_volume(xmuvol);
            
//            for (int i = 0; i < 0xff; i++)
//            {
//                s32 value = xmubuf[i];
//
//                // write to YM2149
//                uRam00ff8800 = (u8)((u32)value >> 0x18);
//                uRam00ff8802 = (u8)((u32)value >> 0x10);
//                uRam00ff8804 = (u8)((u32)value >> 8);
//                uRam00ff8806 = (u8)value;
//                if (value == 0)
//                    break;
//            }
  
            // ignore acia stuff
//            xmuprev_acia = _acia;
//            _acia = &xmu_acia;
        }
        else
        {
            o_mufce01();

            xmuvol += 2;
            if (xmuvol == 2)
                xmuvol += 3;

            o_volmusic(xmuvol);

            xmuval23 = 0x2a78;
            xmuval02 = 0xff;

            timerdata.timer_c = timer_c;
            timer_c = o_soundrout;
            timer_a = o_soundrout2;
        }
        
//        interupt_ins_A = interupt_ins_A & 0xdf;
//        interupt_pending_A = interupt_pending_A & 0xdf;
//        timer_A_data = xmuval03._0_1_;
//        timer_A_cont = (u8)xmuval03;
//        interupt_enable_A = interupt_enable_A & 0xdf | 0x20;
//        interupt_mask_A = interupt_mask_A & 0xdf | 0x20;
//
    }
    else
    {
        xmuvol = xmuvolume;
        if (xmuval01 != 0 && (xmuvol = xmuvolume + 2) == 2)
        {
            xmuvol = xmuvolume + 3;
        }
        
        o_volmusic1(xmuvol);
    }

    for (int i = 0; i < 1024*1024; i ++)
    {
        o_muroutine();
        
        for (int m = 0; m < 4; m++)
        {
            o_soundrout();
        }
    }
}

void o_gettimers(void)
{
    timerdata.interupt_mask_A = interupt_mask_A;
    timerdata.interupt_ins_A = interupt_ins_A;
    timerdata.interupt_pending_A = interupt_pending_A;
    timerdata.interupt_enable_A = interupt_enable_A;
    timerdata.timer_A_cont = timer_A_cont;
    timerdata.timer_data01 = timer_data01;
    timerdata.timer_a = timer_a;
    timerdata.acia = acia;
    timerdata.timer_c = timer_c;
}

void o_mufce01(void)
{
    xmuval28 = 0x7f;
    xmsp_command0[0] = 0x6010;
    xmsp_command0[1] = 0x6010;
    xmsp_command0[2] = 0x6010;
    xmuval07[0] = 0;
    xmuval07[1] = 0;
    xmuval07[2] = 0;
    xmuval07[3] = 0;
}

void o_muroutine(void)
{
    if (audio.muattac == 0)
    {
        if (audio.muduree == 0)
        {
            if (audio.muchute != 0 && -1 < (s16)audio.muchute)
            {
                s16 sVar1 = xmuvolume - audio.dchute;
                if ((-1 < sVar1) && (sVar1 != 0))
                {
                    audio.muchute += -1;
                    xmuvolume = sVar1;
                    goto LAB_0001d218;
                }
            }
            
            o_offmusic();
            audio.mustate = 0;
            return;
        }
        
        audio.muduree --;
    }
    else
    {
        xmuvolume += audio.dattac;
        if (audio.maxvolume < audio.dattac)
        {
            audio.muattac = 1;
            xmuvolume = audio.maxvolume;
        }
        
        if (0x7fff < xmuvolume)
        {
            xmuvolume = 0x7fff;
        }
        
        audio.muattac --;
    }
    
LAB_0001d218:
    
    o_volmusic(xmuvolume >> 8);
    o_tempmusic(audio.mutempo);
}

void o_offmusic(void)
{
//    if (xmuval21 == 0)
//    {
////        __m68k_trap(3);
//    }
//    else
    {
        o_offmusic2(1);
    }
}

void o_offmusic2(s16 d0w)
{
//    u8 *puVar1;
//    u16 *extraout_A0;
//
//    if (*xmuval01 == 0)
//    {
//        return;
//    }
//
//    if (d0w < 0)
//    {
//        if (xmuval02 != 0)
//        {
//            puVar1 = (u8 *)o_volmusic1(xmuvol);
//            xmuval23 = 0x6016;
//            *puVar1 = 0;
//            return;
//        }
//    }
//    else
//    {
//        if (d0w == 0)
//        {
//            xmutimer_c = timer_c;
//            xmutimer_a = timer_a;
//            o_settimers();
//            return;
//        }
//
//        if (*xmuval01 != 0)
//        {
//            *xmuval01 = (u16)(*xmuval01) << 8;
//            timer_c = timerdata.timer_c;
//            timer_a = &xmuval22;
//            return;
//        }
//    }
//
//    o_settimers();
//    *extraout_A0 = 0;
//    o_clearsam();
}

void o_clearsam(void)
{
    xfsam = 0;
    xmuval24 = 0;
}

void o_settimers(void)
{
    interupt_mask_A2 = timerdata.interupt_mask_A;
    interupt_ins_A = timerdata.interupt_ins_A;
    interupt_pending_A2 = timerdata.interupt_pending_A;
    interupt_enable_A2 = timerdata.interupt_enable_A;
    timer_A_cont2 = timerdata.timer_A_cont;
    timer_data01 = timerdata.timer_data01;
    timer_a = timerdata.timer_a;
    acia = timerdata.acia;
    timer_c = timerdata.timer_c;
}

void o_soundrout(void)
{
    s32 iVar1;
    u8 bVar2;
    u16 lnotedata;
    s32 iVar4;
    u32 noteptr;
    u32 nextnoteptr;
    
    // 0x44d0e + 0x4fc6
    
    //    printf("\n0x%.6x\n", xmupnoteptr - 0x4fc6);
    
    xmuspeed -= xmutemp;
    if (((s16)xmuspeed < 0) || (xmuspeed == 0))
    {
        xmuspeed = 0;
        noteptr = xmupnoteptr;
        
        while (true)
        {
            if (noteptr == 0)
                noteptr = xmupnoteptr;
            
            xmupnoteptr = noteptr + 1;

            printf("  0x%.6x\n", 1 + xmupnoteptr - 0x4fc6);

            nextnoteptr = noteptr;
            bVar2 = xread8(noteptr);
            if ((s8)bVar2 < 0)
            {
                if (bVar2 != 0xff)
                {
                    xmuspeed = (bVar2 - 0x80) << 8;
                    break;
                }
                
                xnotedata = xread8(xmupnoteptr);
                nextnoteptr = noteptr + 2;
            }
            
            lnotedata = (u16)xnotedata;
            if (xnotedata == 0xff)
            {
                lnotedata = 0;
            }
            
            // u8 type = (((u8)lnotedata) << 2) >> 2;
            u8 type = lnotedata;
            type <<= 2;
            type >>= 2;
            
            //            printf("   type: 0x%.4x (0x%.2x, 0x%.2x)\n", lnotedata, type, type << 2);
            
            switch (type)
            {
                case 0:
                {
                    noteptr = audio.mupnote;
                    break;
                }
                case 1:
                {
                    u16 chidx = lnotedata >> 6;
                    if (xread8(nextnoteptr + 1) == 0)
                    {
                        noteptr = nextnoteptr + 2;
                        if ((xread8(nextnoteptr) == xmu268[chidx]) && (xmuval07[chidx] != 0))
                        {
                            xmuval07[chidx] = 0;
                            xmsp_command0[chidx] = 0x6010;
                            
                            xmuval09 -= 0x40;
                            if ((s16)((u16)xmuval09 << 8) < 0)
                            {
                                xmuval10 = 0x6064;
                                if (xmuval11 != 0)
                                {
                                    xmsp_command0[2] = 0x2a78;
                                }
                            }
                            else
                            {
                                xmuval28 += 0x3f;
                            }
                        }
                    }
                    else
                    {
                        // write bra.b at xmsp_command1 index for chanel lnotedata
                        // IE. skip clear
                        xmsp_command1[chidx] = 0x6002;
                        iVar1 = muactinstru[chidx];
                        xsmp_value = muactidata[chidx];
                        xmu240[chidx].address = iVar1;
                        iVar4 = xpcread32(iVar1 - 8);
                        if (iVar4 != 0)
                        {
                            iVar4 += iVar1;
                        }
                        
                        xmu25c_smp_addr2[chidx] = iVar4;
                        bVar2 = xread8(nextnoteptr);
                        xmu268[chidx] = bVar2;
                        noteptr = nextnoteptr + 2;
                        
                        xmsp_command2[chidx] = xmutable1[(u16)(xsmp_value + (u16)bVar2)];
                        lnotedata = (u16)(chidx * 2) >> 3;
                        
                        u32 unknown1 = xmuval07[chidx];
                        xmuval07[chidx] |= 0x80;
                        if (unknown1 == 0)
                        {
                            xmsp_command0[chidx] = 0x2a78;
                            if ((s8)xmuval09 < 0)
                            {
                            //    xmsp_command0[2] = 0x60c0;
                                xmuval10 = 0x4e71;
                            }
                            else
                            {
                                xmuval28 -= 0x3f;
                            }

                            xmuval09 += 0x40;
                        }
                    }
                    break;
                }
                case 2:
                    noteptr = nextnoteptr + 1;
                    u8 instidx = xread8(nextnoteptr);
                    u32 sample = audio.tabinst[instidx].address;
                    s32 tval = audio.tabinst[instidx].unknown;
                    
                    printf("   addr1: 0x%.6x, tv: 0x%4x\n", sample - 0x4fc6, ((u16)tval));
                    
                    lnotedata >>= 6;
                    
                    muactidata[lnotedata] = tval;
                    muactinstru[lnotedata] = sample;
                    break;
                    
                case 3:
                    lnotedata >>= 4;
                    iVar4 = xpcread32(muactinstru[lnotedata] - 4);
                    if (iVar4 != 0)
                    {
                        iVar4 += muactinstru[lnotedata];
                    }
                    
                    printf("   addr2: 0x%.6x\n", iVar4 - 0x4fc6);
                    
                    xmu25c_smp_addr2[lnotedata] = iVar4;
                    xmsp_command1[lnotedata] = 0x42b8;
                    noteptr = nextnoteptr;
                    break;
                    
                default:
                    sleep(0);
                    break;
            }
        }
    }

//    o_muroutine();

    for (int l = 0; l < 48; l++)
    {
        o_soundrout2();
    }
}

void advance(u32 *address, u32 *unknown, u32 *addvance)
{
    // NOTE: it would be much easier on big endian or using uint64
    
    uint64_t tadv = *addvance;
    tadv <<= 16;

    uint64_t test = *address;
    test <<= 32;
    test |= *unknown;
    test += tadv;

    *address = (u32)(test >> 32);
    *unknown = (u32)(test);
}

void o_soundrout2(void)
{
    s16 uVar11 = (s8)xmuval28;
    s16 test = 0;//(s8)xmuval28;
    
    u32 addr[] = { xmu25c_smp_addr2[0], xmu25c_smp_addr2[1], xmu25c_smp_addr2[2] };

    for (int c = 0; c < 3; c++)
    {
        // play chanel
        if (xmsp_command0[c] == 0x2a78)
        {
            // printf("   channel: %d addr: 0x%.6x\n", c, xmu240[c].address);
//            printf("   channel: %d addr: 0x%.6x\n", c, xmu240[c].address - 0x4fc6);
            
            s8 smpval = xread8(xmu240[c].address);
            if ((u8)smpval < 0xff)
            {
//                printf("   channel: %d adv: 0x%.6x\n", c, xmsp_command2[c]);
                test += smpval;

                smpval -= 0x40;
                uVar11 += smpval;
                advance(&(xmu240[c].address), &(xmu240[c].unknown), &(xmsp_command2[c]));
            }
            else
            {
                if (xmu25c_smp_addr2[c] != 0)
                {
                    if (xmsp_command1[c] == 0x42b8)
                        xmu25c_smp_addr2[c] = 0;
                    
//                    printf("   channel: %d adv: 0x%.6x\n", c, xmsp_command2[c]);
                    smpval = xread8(addr[c]) + 1;
                    test += smpval;

                    smpval -= 0x40;
                    uVar11 += smpval;

                    xmu240[c].address = addr[c];
                    advance(&(xmu240[c].address), &(xmu240[c].unknown), &(xmsp_command2[c]));
                }
                else
                {
                    xmsp_command0[c] = 0x6010;
                    xmuval07[c] = 0;
                    xmuval09 += -0x40;
                    if (xmuval09 < 0)
                    {
                        xmsp_command0[2] = 0x2a78;
                        xmuval10 = 0x6064;
                    }
                    else
                    {
                        xmuval28 += 0x3f;
                        uVar11 += 0x3f;
                    }
                }
            }
        }
    }
    
//    if (xmuval10 != 0x6064)
//    {
//        uVar11 = 0;
//    }
    
    // xmuvol contain asm code of what to do with val in uVar11
    uVar11 = test;
    switch (xmuvol)
    {
        case 0xE74B: // lsl.w #0x3,D3w
            uVar11 *= 8;
            break;
        case 0xE54B: // lsl.w #0x2,D3w
            uVar11 *= 4;
            break;
        case 0xD643: // add.w d3w,d3w
            uVar11 *= 2;
            break;
        case 0x4E71: // nop
            break;
        case 0x4243: // clr.w D3w
            uVar11 = 0;
            break;
    };
    
    xmubuffer[xmuidx ++] = uVar11 / 32;
    
    if (xmuidx == 1024 * 1024 * 4)
    {
        // signed 12 112 hz
        FILE *f = fopen("/Users/gildor/Desktop/test.smp", "wb");
        if (f == NULL)
        {
            return;
        }

        fwrite(xmubuffer, xmuidx, 1, f);
        fclose(f);

        xmuidx = 0;
    }

//    if (xmuval23 == 0x2a78)
//    {
//        // always 0
//        if (xmu258 == 0)
//        {
//            if (-1 < xmu259 - xmuval27)
//            {
//                xmu240[0].b = iVar3;
//                xmu240[1].b = iVar4;
//                {
//                    s16 sVar3 = xmuval25 - 1;
//                    if (sVar3 == 0)
//                    {
//                        xmuval25 = sVar3;
//                        o_volume(xmuvol);
//                        xmuval23 = 0x6018;
//                        d3w += 0x80;
//                    }
//                    else
//                    {
//                        if (sVar3 < 0)
//                            sVar3 = xmuval25;
//
//                        xmuval25 = sVar3;
//                        _xmu258 = xmuval26;
//                    }
//                    sVar3 = (d3w & 0x1fe) << 2;
//                    u32 uVar1 = xmutable2[sVar3 + 4];
//                    u32 uVar2 = xmutable2[sVar3];
//                    //                _YM2149_0 = CONCAT13((char)((uint)uVar2 >> 0x18),(int3)uVar1);
//                    //                _YM2149_4 = (char)((uint)uVar2 >> 8);
//                    //                _YM2149_6 = (char)uVar2;
//                    interupt_ins_A &= 0xdf;
//                }
//
//                return;
//            }
//        }
//        else
//        {
//            uVar11 += xmu258;
//        }
//
//        xmu258 += 1;
//    }
    
    s16 index = (s16)(test) << 1; // << 3;
    u32 uVar2 = xmutable2[index + 0];
    u32 uVar1 = xmutable2[index + 4];
//    _YM2149_0 = CONCAT13((char)((u32)uVar2 >> 0x18),(int3)uVar1);
//    _YM2149_4 = (char)((u32)uVar2 >> 8);
//    _YM2149_6 = (char)uVar2;
    interupt_ins_A &= 0xdf;
}

void o_tempmusic(u16 d0w)
{
    xmutemp = (d0w & 0x7f) << 3;
}

void o_volmusic(u16 d0w)
{
    o_volmusic2((u16)(0x7f - (d0w & 0x7f)) >> 4 & 7);
}

void o_volmusic1(s16 vol)
{
    xmuvol = xvolsam[vol];
}

void o_volmusic2(s16 vol)
{
    xmuvol = vol;
    if (xmuval01 < 0 && (vol += 2) == 2)
    {
        vol += 3;
    }
    
    xmuvol = xvolsam[vol];
}

void o_volume(s16 vol)
{
    xmuvol = xvolsam[vol];
}
