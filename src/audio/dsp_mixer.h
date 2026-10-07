// =========================================================================
// DSP56001 8-Channel Audio Mixer — Host API for Falcon
// =========================================================================
//
// Offloads ALL sample mixing (SFX + music voices) to the DSP56001.
// CPU only does: music sequencing + channel state updates.
//
// Slots 0-3: SFX channels (from audio.channels[])
// Slots 4-7: Music voices (from MV2 voice state)
//

#pragma once

#include "config.h"

#if defined(__atarist__) || defined(__TOS__)

#include "channel.h"

// DSP path selection (shared by ALL translation units so the Timer-A feed in
// sys_sdl1.c and the driver in dsp_mixer.c agree). The legacy hand-written SFX
// mixer is opt-in via ALIS_DSP_USE_SFXMIXER; otherwise default to the tracker.
#if !defined(ALIS_DSP_USE_SFXMIXER) && !defined(ALIS_DSP_USE_TRACKER)
#define ALIS_DSP_USE_TRACKER
#endif

// Initialize DSP mixer: load program, lock DSP.
// Returns 0 on success, -1 if DSP not available.
int dsp_mixer_init(void);

// Release DSP.
void dsp_mixer_close(void);

// Update one MV2 music voice (0-3 → DSP voices 4-7). Called from
// mv2_soundrout's DSP branch each sequencer tick.
//   address : host pointer to the voice's current 8-bit signed PCM data
//   length  : sample length in bytes
//   step    : 16.16 fixed-point phase increment relative to audio.host_freq
//             (playback_hz = host_freq * step / 65536)
//   volume  : f_volume() result (0..~0x3F00)
//   loop    : 1 = one-shot, 2 = loop
//   retrigger : non-zero restarts even at the same address (MV1 re-strikes the
//             same sample); 0 restarts only on an address change (MV2).
//   replen  : looped tail length in bytes ([length-replen, length)); == length
//             loops the whole sample. Ignored for one-shots.
void dsp_mixer_update_music(int voice, s8 *address, u32 length, u32 step,
                            s16 volume, u8 loop, int retrigger, u32 replen);

// Stub: zero-fills output (the DSP drives the DAC directly).
void dsp_mixer_mix(s16 *output, u16 num_samples, u16 sample_rate);

// Main-loop service: program swaps, wedge recovery (NOT interrupt context).
void dsp_mixer_poll(void);

// Per-tick DSP feed from the 50 Hz Timer A ISR (supervisor): the tracker's SSI
// ring needs a steadier cadence than the main loop. Tracker / FM programs only.
void dsp_mixer_tick_isr(void);

// Is DSP mixer active?
extern u8 dsp_mixer_available;

// ---- OPL2 FM music synth on the DSP56001 (host-port FM-block feed) ----
// Separate from the sample mixer above: the host translates the OPL register
// stream into resolved per-channel FM blocks (opl_host) and pushes them here once
// per music frame; the DSP applies them and renders the FM additively.
#if defined(ALIS_DSP_MIXER)
#include "opl_block.h"
extern int dsp_opl_available;                  // 1 once the DSP FM synth is loaded
void dsp_opl_feed(const opl_block_t *blocks);  // push 9 resolved FM blocks (per frame)
int  dsp_opl_fm_init(void);                    // load the FM-only DSP program (intro/death)
void dsp_opl_fm_frame(void);                   // trigger one rendered frame (per music frame)
#else
#define dsp_opl_available 0
#define dsp_opl_feed(b)     ((void)0)
#define dsp_opl_fm_init()   (-1)
#define dsp_opl_fm_frame()  ((void)0)
#endif

#else
// Non-Falcon stubs
#define dsp_mixer_init()    (-1)
#define dsp_mixer_close()   ((void)0)
#define dsp_mixer_tick_isr() ((void)0)
#define dsp_mixer_available 0
#define dsp_opl_available 0
#define dsp_opl_feed(b)     ((void)0)
#define dsp_opl_fm_init()   (-1)
#define dsp_opl_fm_frame()  ((void)0)
#endif
