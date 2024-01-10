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

#pragma once

#include "channel.h"
#include "config.h"

typedef struct {
    
    u32 address;
    u32 unknown;

} sAudioInstrument;

typedef struct {
    
    u16 data;
    u16 loop;
    u32 frqmod;

} sAudioTrkfrq;

typedef struct {
    
    u16 freqsam;
    u32 startsam1;
    u32 longsam1;
    u16 volsam;
    u32 startsam2;
    u32 longsam2;
    s16 value;
    u8 type;
    u8 delta;
    u16 loopsam;
    u16 unknown1;
    u16 unknown2;
    u16 unknown3;
    
    u32 sample;

} sAudioVoice;

typedef struct {

    sAudioInstrument tabinst[128];
    
    u8 fsound;
    u8 fmusic;
    
    u32 mupnote;
    u8 muvolume;
    u16 maxvolume;
    u8 muvol;
    u8 mustate;
    u8 mutempo;
    u8 mutemp;
    u16 muattac;
    u16 muchute;
    s16 muduree;
    u16 dattac;
    u16 dchute;

    u16 muflag;
    u16 mutaloop;
    s16 muadresse[0xffff];
    u32 smpidx;

    void (*soundrout)(void);
    
} sAudio;

extern sAudio audio;

void playsample(eChannelType type, u8 *address, s8 freq, u8 volume, u32 length, u16 loop);
void playsound(eChannelType type, u8 pereson, u8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson);
void runson(eChannelType type, s8 pereson, s8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson);

// older music variant (atari st/amiga ishar and older)

void mv1_gomusic(void);
void mv1_offmusic(u32 much);

// newer music variant (ishar 1 falcon/dos and later)

void mv2_gomusic(void);
void mv2_offmusic(u32 much);

// ym

void io_canal(sChannel *channel, s16 index);

