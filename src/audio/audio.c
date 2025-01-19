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
#include "channel.h"
#include "mem.h"

sAudio audio = {};

void playsample(eChannelType type, u8 *address, s8 freq, u8 volume, u32 length, u16 loop, s8 priorson)
{
    char cson = CHAR_MAX;
    sChannel *canal = NULL;
    
    for (int i = 0; i < 4; i++)
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
        for (int i = 0; i < 4; i++)
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
        return;

//    if ((s8)type < '\0')
//    {
//        gosound(canal);
//    }
//    else
    {
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
}

void offsound(void)
{
    for (int i = 0; i < 4; i++)
    {
        audio.channels[i].type = eChannelTypeNone;
        audio.channels[i].volume = 0;
        audio.channels[i].freq = 0;
        audio.channels[i].curson = 0x80;
        audio.channels[i].pere = 0;
        audio.channels[i].state = 0;
        audio.channels[i].played = 0;
    }

    for (int i = 0; i < 3; i++)
    {
        io_canal(&(audio.channels[i]), i);
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
