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

//#ifdef _MSC_VER
# define a32 u32
//#else
//# include <stdatomic.h>
//# define a32 atomic_int
//#endif

#include "channel.h"
#include "config.h"

#define kNumMV1Channels (3)

typedef struct {
    
    u32 address;
    u32 fraction;
    
} sMV1MusicPosition;

typedef struct {
    
    u16 active;
    sMV1MusicPosition startsam;
    u32 speedsam;
    u32 endsam;
    u16 clearendsam;
    u32 instr;
    u16 data;
    u16 actflag;
    u8 unknown;

} sMV1Channel;

typedef struct {

    sMV1Channel channels[kNumMV1Channels];

    u32 tabfrq[128];
    u8 flag1;
    u8 flag2;
    u16 tempo;
    u16 effect;
    u16 volume;
    u16 prevvol;
    u16 basevol;
    u32 noteptr;
    u8 notedata;
    u16 mumax;
    u16 mucnt;
    u16 muspeed;

} sMV1Audio;

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
    
    s16 tvalue;
    u32 address1;
    s32 address2;
    u16 volume;
    s16 unknown4;
    u16 notedata;
    u16 unknownA;

} sChipChannel;

typedef struct {

    sAudioVoice voices[4]; //
    sChipChannel chipch[3];
    u32 tabfrq[0x358];
    u16 defvolins;
    u8 defvol[32];
    s16 trkval[36];
    s8 tabvol[0x4000 * 2];
    s16 prevmufreq;
    s16 prevmuvol;
    u16 mutype;
    u16 mufreq;
    u16 muchip;
    u16 muopl2;
    u16 muptr;
    u16 mumax;
    u16 mucnt;
    u16 mubufa;
    u16 mubufc;
    u16 muspeed;
    u16 muvolgen;
    u16 mubreak;
    u16 mutadata;
    u32 frqmod;
    u16 samples;
    s16 chipmixer;

} sMV2Audio;

typedef struct {

    s32 host_freq;
    u16 host_format;
    
    u16 musicId;
    a32 working;

    sChannel channels[4];
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

    a32 muflag;
    u16 mutaloop;
    s16 muadresse[0xffff];
    u32 smpidx;

    void (*soundrout)(void);
    
} sAudio;

extern sAudio audio;

void playsample(eChannelType type, u8 *address, s8 freq, u8 volume, u32 length, u16 loop, s8 priorson);
void playsound(eChannelType type, u8 pereson, u8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson);
void runson(eChannelType type, s8 pereson, s8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson);

void offsound(void);

// older music variant (atari st/amiga ishar and older)

void mv1_gomusic(void);
void mv1_offmusic(u32 much);

// newer music variant (ishar 1 falcon/dos and later)

void mv2_gomusic(void);
void mv2_offmusic(u32 much);

// ym

void io_canal(sChannel *channel, s16 index);

// soundrout

void mv1_soundrout(void);
void mv2_soundrout(void);
void mv2_chiprout(void);
