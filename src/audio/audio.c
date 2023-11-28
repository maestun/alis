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

// #include "emu2149.h"

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

