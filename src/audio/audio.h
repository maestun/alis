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

# define a32 u32

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

// One music tick of mixed samples: <= ~1001 at the fastest Atari rate (50 kHz).
#if defined(__atarist__) || defined(__TOS__)
#define kMuBufLen 1024
#else
#define kMuBufLen 0xffff
#endif

typedef struct {

    sAudioVoice voices[4];
    sChipChannel chipch[3];
    u32 tabfrq[0x358];
    u16 defvolins;
    u8 defvol[32];
    s16 trkval[36];
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
    u16 mutaloop;                   // <= kMuBufLen (set_mutaloop)
    s16 muadresse[kMuBufLen];
    u32 smpidx;

    void (*soundrout)(void);
    
} sAudio;

extern sAudio audio;

static inline void set_mutaloop(u32 n) { audio.mutaloop = (u16)(n < kMuBufLen ? n : kMuBufLen); }

void playsample(eChannelType type, u8 *address, s8 freq, u8 volume, u32 length, u16 loop, s8 priorson);
void playsound(eChannelType type, u8 pereson, u8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson);
void runson(eChannelType type, s8 pereson, s8 priorson, s16 volson, u16 freqson, u16 longson, s16 dvolson, s16 dfreqson);

void offsound(void);
void audio_relocate(u32 lo, u32 hi, s32 delta);

// older music variant (atari st/amiga ishar and older)

void mv1_gomusic(void);
void mv1_offmusic(u32 much);

// newer music variant (ishar 1 falcon/dos and later)

void mv2_gomusic(void);
void mv2_offmusic(u32 much);

// v1.0 ym music (manhattan dealers)

#define kMV0Loops 32

typedef struct {

    u8 on;
    u8 vflags;
    s8 vdelta;
    u8 vcnt;
    u8 vrate;
    u8 vnum;
    u8 vol;
    u8 pflags;
    u16 venv;
    u16 period;
    u8 busy;
    u8 dur;

} sMV0Channel;

typedef struct {

    sMV0Channel ch[3];
    u8 playing;
    u8 starting;
    u8 pending;
    u8 played;
    u8 cadence;
    u8 maxvol;
    u8 curvol;
    u8 tgtvol;
    u8 count;
    u8 divider;
    u8 rr;
    u8 transpose;
    u16 pfadein;
    u16 psustain;
    u16 pfadeout;
    u16 fadein;
    u16 sustain;
    u16 fadeout;
    u16 step;
    u16 rate;
    u32 songptr;
    u8 loopsp;
    u16 loopcnt[kMV0Loops];
    u32 loopptr[kMV0Loops];

} sMV0Audio;

extern sMV0Audio mv0a;

void mv0_gomusic(u32 addr, u8 vol, s16 fadein, s16 sustain, s16 fadeout);
void mv0_offmusic(void);
void mv0_cadence(u8 cadence);
void mv0_volume(u8 vol);
void mv0_reset(void);
void mv0_relocate(u32 lo, u32 hi, s32 delta);
void mv0_soundrout(void);

// ym

void io_canal(sChannel *channel, s16 index);

// soundrout

void mv1_soundrout(void);
void mv2_soundrout(void);
void mv2_chiprout(void);
void mv2_opl2rout(void);

// FLI speech queue (channel 3 only). Chunks must play back-to-back, so the
// mixer dequeues the next one when the current ends instead of being replaced.

#define FLI_AUDIO_QUEUE_SIZE 16  // power of 2 — head/tail use & mask

typedef struct {
    s8  *addr;
    u32  length;
    s16  freq;
} sFliAudioChunk;

extern volatile sFliAudioChunk fli_audio_queue[FLI_AUDIO_QUEUE_SIZE];
extern volatile u8 fli_audio_q_head;  // next slot to write
extern volatile u8 fli_audio_q_tail;  // next slot to read

// FLI chunks finished on channel 3. Video paces on it (the DAC runs at real
// time even when the emulated timer doesn't) to keep lip-sync.
extern volatile u32 fli_chunks_played;
