// =========================================================================
// Atari STE / Mega STE / TT030 / Falcon 8-bit DMA Sound — host output backend
// =========================================================================
//
// Bypasses SDL: the CPU mixes ALIS's channels (sys_audio_callback_S8, the
// chip's 8-bit signed format) into an ST-RAM ring buffer that the DMA hardware
// replays in loop mode, topped up ahead of the play cursor by the 50 Hz
// Timer-A ISR. Used on STE/TT and by default on the Falcon (crossbar routed
// via XBIOS); plain ST has no DMA sound.

#pragma once

#include "config.h"

#if defined(__atarist__) || defined(__TOS__)

// 1 once the DMA buffer is allocated and the hardware is looping.
extern int atari_dma_available;

// Allocate the ST-RAM ring buffer, program the DMA sound registers for
// `rate` Hz (snapped to a hardware-legal rate by the caller) and `channels`
// (1 = mono, 2 = stereo), and start it looping. Returns 0 on success, -1 on
// failure (no ST-RAM / unsupported). All hardware access runs in supervisor.
//
// is_falcon: 0 = STE/Mega STE/TT (direct $8921 rate, DMA→DAC is wired) ;
//            1 = Falcon (route the crossbar DAC←DMA-Play and set the rate via
//            the $8935 prescale — rate must be a Falcon rate 98340/(presc+1)).
int atari_dma_sound_init(int rate, int channels, int is_falcon);

// Top up the ring buffer ahead of the play cursor. Main-loop fallback; becomes
// a no-op once the 50 Hz Timer-A ISR is driving the refill (see below).
void atari_dma_sound_poll(void);

// Top up the ring buffer from the 50 Hz Timer-A ISR (already in supervisor).
// This is what keeps the music sequencer ticking at a steady 50 Hz regardless
// of the rendering frame rate.
void atari_dma_sound_tick_isr(void);

// Tell the driver the Timer-A ISR is now driving the refill (poll() turns off).
void atari_dma_sound_set_isr_driven(int on);

// Stop the DMA and free the buffer.
void atari_dma_sound_close(void);

#endif
