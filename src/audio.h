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
    
    u32 length1;
    u32 length2;

} sAudioVoice;

typedef struct {
    
    sAudioInstrument tabinst[128];
    sAudioInstrument *tinstrum;
    
    u32 tabfrq[0x358];
    
    sAudioVoice voices[4];
    
    u16 defvolins;
    u8 defvol[32];
    s16 trkval[36];
    
    s8 tabvol[0x4000 * 2];
    
    s16 prevmufreq;
    s16 prevmuvol;
    
    u16 muflag;
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
    u16 mutype;
    u16 mufreq;
    u16 muchip;
    u16 dattac;
    u16 dchute;
    
    u16 muptr;
    u16 mumax;
    u16 mucnt;
    u16 mubufa;
    u16 mubufc;
    u16 muvar;
    u16 muspeed;
    u16 muvolgen;
    u16 mubreak;
    u16 mutadata;
    u16 mutaloop;
    s16 *muadresse;
    
} sAudio;

extern sAudio audio;

void playsample(eChannelType type, u8 *address, s8 freq, u8 volume, u32 length, u16 loop);
void playsound(eChannelType type, u8 pereson, u8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson);
void playmusic(void);

void f_offmusic(void);
void f_stopmusic(void);
void f_onmusic(void);
