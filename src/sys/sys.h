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

#include "config.h"
#include "debug.h"
#include "platform.h"


// =============================================================================
#pragma mark - Signals
// =============================================================================

void    sys_errors_init(void);
void    sys_errors_deinit(void);
void    signals_info(int signo);

// =============================================================================
#pragma mark - LIFECYCLE
// =============================================================================
typedef void (*vmStep)(void);
void    sys_main(vmStep fStep);
void    sys_init(sPlatform *pl, int fullscreen, int mutesound);
u8      sys_start(void);
void    sys_poll_event(void);
void    sys_deinit(void);
// Hand the screen back to GEM (native Atari only). Must be the last thing main() does:
// the AES repaint would overwrite any later console output.
void    sys_restore_desktop(void);

void    sys_init_timers(void);
void    sys_delay_loop(void);
void    sys_delay_frame(void);
void    sys_sleep_until_music_stops(void);

// Rate at which alis.timeclock advances, in Hz (set by sys_init, refined after
// Timer C calibration on Atari). video.c derives FLI audio rate and frame pacing from it.
extern u32 sys_timeclock_hz;

// Frame / palette-fade and music-sequencer rates (50 Hz, or the port's own with --native-timing).
extern int opt_native_timing;
u32 sys_pace_hz(void);
u32 sys_music_hz(void);

// Original game's interrupt rate: 50 Hz VBL (Atari/Amiga data), 60 Hz PIT (DOS data).
// Drives SFX envelope steps (isr_step), FLI pacing and audio rate, and OPL2 tempo.
extern u32 sys_sfx_tick_hz;
void    sys_sleep_interactive(s32 *loop, s32 intr);

void    sys_init_psg(void);
void    sys_deinit_psg(void);
void    sys_write_psg(u32 reg, u32 val);
u8      sys_read_psg(u32 reg);
void    sys_calc_psg_music(void);
void    sys_psg_tick(void);   // one 50/60 Hz tick of the YM tone-effect channels

void    sys_init_opl(void);
void    sys_deinit_opl(void);
void    sys_write_opl(u32 reg, u8 val);
void    sys_calc_opl_music(void);
void    sys_opl_frame_tick(void);   // end of an OPL music frame (capture mark, DSP feed, queue reset)

// Raw OPL register writes of the current frame (debug only; reset each frame).
extern int opl_capture_flag;        // --opl-capture: dump the register stream
extern u8  sys_opl_q_reg[];
extern u8  sys_opl_q_val[];
extern u16 sys_opl_q_count;

void    sys_wav_export_start(const char *path);
void    sys_wav_export_stop(void);

// =============================================================================
#pragma mark - I/O
// =============================================================================
typedef struct {
    int x, y;
    u8 lb, rb;
    u8 enabled;
    u8 consumed;
    u8 lb_clicked;
    u8 rb_clicked;
} mouse_t;

mouse_t sys_get_mouse(void);
mouse_t sys_consume_mouse(void);

void    sys_set_mouse(u16 x, u16 y);
void    sys_enable_mouse(u8 enable);

void    set_update_cursor(void);
void    sys_dirty_mouse(void);
// Integrated software-cursor hooks called from draw() (image.c): erase the cursor
// from image.logic before composition, draw it back after composition / before the
// buffer swap. Implemented by the native 16bpp backend; no-ops elsewhere.
void    sys_mouse_erase(void);   // before oldfen: remove image.logic's own cursor
void    sys_mouse_uncopy(void);  // after oldfen: remove the front cursor it copied in
void    sys_mouse_draw(void);    // before swap: draw the cursor into image.logic
void    sys_flip_wait(void);     // block until the VBL latched the last flip (native; no-op elsewhere)

u8      io_getkey(void);
u8      io_inkey(void);
u8      io_shiftkey(void);
u8      io_joy(u8 port);
u8      io_joy_raw(void);
u8      io_joykey(u8 test);

// =============================================================================
#pragma mark - GFX
// =============================================================================
typedef struct {
    u16     w, h;
    u16     surface_h;      // SDL surface height if > h (e.g. 240 for a 200-row game); 0 = h
    u16     surface_y_off;  // top centering pad (rows); data points this far into the alloc
    u8      scale;
    u8 *    data;
    u32 *   palette;
} pixelbuf_t;
void    sys_render(pixelbuf_t buffer);

// Falcon BLiTTER offload (native only; no-ops elsewhere). sys_blit_frame_sync()
// invalidates the 030 data cache once per frame so the CPU sees Blitter writes.
void    sys_blit_fill_16(u16 *dst, int w, int h, u16 color);
// Block-copy a 16bpp rectangle (src in cacheable ST-RAM, e.g. the expanded-sprite
// cache) to the framebuffer via the Blitter. src_pitch_w = src row stride in words.
void    sys_blit_copy_16(u16 *src, int src_pitch_w, u16 *dst, int w, int h);
// 8-plane INTERLEAVED opaque copy (per-plane Blitter): src/dst row strides in BYTES,
// chunks_w = number of 16px chunks per row, h = rows. Word-aligned only (skew=0).
void    sys_blit_copy_planar(u16 *src, int src_rowbytes, u16 *dst, int dst_rowbytes, int chunks_w, int h, int nplanes);
// 8-plane solid rectangle fill (per-plane Blitter halftone). em1/em3 = left/right edge masks.
void    sys_blit_fill_planar(u16 *dst, int dst_rowbytes, int chunks_w, int h, u16 col, u16 em1, u16 em3);
void    sys_blit_frame_sync(void);
// 1 if a BLiTTER exists at $FFFF8A00. The TT has none (bus error), so every
// sys_blit_*_planar size gate must check this too.
extern int g_hw_blitter;
// ST-RAM (Blitter-readable) allocation for the sprite-expansion cache.
void *  sys_stram_alloc(u32 size);
void    sys_stram_free(void *p);

// Largest free block in ST-RAM (screens, DMA sound, Blitter sources) / fast RAM
// (0 if none), for sys_size_arena. The VM heap may use either (.TTP is -r -a).
u32     sys_stram_avail(void);
u32     sys_fastram_avail(void);

#if defined(ALIS_USE_NATIVE_ATARI)
// Startup memory pre-flight; call before sys_init so a refusal needs no video restore.
// Returns the VM arena size in [floor, pref] that fits this machine, 0 to refuse.
u32     sys_size_arena(const sPlatform *pl, u32 floor, u32 pref);
// Hold off the game/audio/cursor ISRs while alis_load_state rewrites their state.
void    sys_isr_pause(int on);
// Savestates: take the async cursor out of the screen buffers while they are saved/replaced.
void    sys_cursor_hold(int on);
#else
static inline void sys_isr_pause(int on) { (void)on; }
static inline void sys_cursor_hold(int on) { (void)on; }
#endif


// =============================================================================
#pragma mark - FILE SYSTEM
// =============================================================================
int     sys_fclose(FILE * fp);
FILE *  sys_fopen(char * path, u16 mode);
u8      sys_fexists(char * path);


// =============================================================================
#pragma mark - MISC
// =============================================================================
void    sys_set_time(u16 h, u16 m, u16 s);
time_t  sys_get_time(void);

// Free bytes on the drive holding `path`, capped at `cap` (0 if unknown).
u32     sys_free_bytes(const char *path, u32 cap);

// Monotonic tick counter for lightweight profiling. Units are backend-specific
// (SDL: ms; native Atari: 200 Hz / 5 ms) — only ratios are meaningful.
u32     sys_profile_ticks(void);

u32     sys_ticks(void);          // milliseconds
void    sys_delay(u32 ms);

// Displayable buffer for image.physic (0) / logic (1), or NULL to malloc them.
// Native Atari returns ST-RAM screens (VIDEL can't scan out fast RAM).
u8 *    sys_get_framebuffer(int index);

u16     sys_get_model(void);
u16     sys_random(void);

// =============================================================================
#pragma mark - SYNC
// =============================================================================
void    sys_lock_renderer(void);
void    sys_unlock_renderer(void);


