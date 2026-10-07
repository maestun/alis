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

#include "config.h"

#if ALIS_SDL_VER == 1

#include <SDL/SDL.h>

#include "sys.h"
#include "alis.h"
#include "audio.h"
#include "channel.h"
#include "image.h"
#include "mem.h"
#include "platform.h"
#include "utils.h"
#include "video.h"
#include "../audio/dsp_mixer.h"
#include "sys_atari_dma_sound.h"

#include "emu2149.h"
#include "math.h"

#include "../icons/icon8_data.h"

// ============================================================================
#pragma mark - Atari-only
// ============================================================================

#if defined(__atarist__)
# include <mint/osbind.h>
# include <mint/ostruct.h>
# include <mint/falcon.h>
# include <mint/cookie.h>
# include <stdarg.h>

# define SDL1_MCH_ST      0
# define SDL1_MCH_STE     1
# define SDL1_MCH_TT      2
# define SDL1_MCH_FALCON  3

# define ATARI_TIER_SLOW        0   // stock Falcon 030 @ 16MHz, low-spec
# define ATARI_TIER_MEDIUM      1   // TT 030 @ 32 MHz
# define ATARI_TIER_FAST        2   // 040 @ 25–40 MHz (Hades, CT60 040)
# define ATARI_TIER_FASTER   3   // 060 @ 50+ MHz (CT60)
# define ATARI_TIER_FASTEST    4   // Aranym / FPGA boards

static u16   sdl1_machine_type = SDL1_MCH_ST;
static u8    sdl1_cpu_family   = 0;
static u8    sdl1_cpu_tier     = ATARI_TIER_SLOW;
static u32   sdl1_bench_ticks  = 0;

#define SDL1_BENCH_ITERS   4000000UL

static volatile u32 sdl1_bench_result_ticks;

static long atari_bench(void)
{
    volatile u32 * const hz200 = (u32 *)0x4BAUL;
    u32 t0, t1;

    t0 = *hz200;
    while (*hz200 == t0) { /* spin */ }
    t0 = *hz200;

    u32 count = SDL1_BENCH_ITERS;
    __asm__ volatile (
        "1: subq.l  #1,%0\n\t"
        "   bne.s   1b\n\t"
        : "+d"(count) :: "cc"
    );

    t1 = *hz200;
    sdl1_bench_result_ticks = t1 - t0;
    return 0;
}

static u8 atari_cpu_tier(u32 ticks, u8 cpu_family)
{
    if (ticks < 5)
        return ATARI_TIER_FASTEST;
    if (cpu_family >= 60 || ticks <= 20)
        return ATARI_TIER_FASTER;
    if (cpu_family >= 40 || ticks <= 50)
        return ATARI_TIER_FAST;
    if (ticks <= 100)
        return ATARI_TIER_MEDIUM;
    return ATARI_TIER_SLOW;
}

static void atari_detect_machine(void)
{
    long cookie_mch = 0;
    long cookie_cpu = 0;

    if (Getcookie(C__MCH, &cookie_mch) != C_FOUND)
        cookie_mch = 0;  // plain ST
    sdl1_machine_type = (u16)(cookie_mch >> 16);

    if (Getcookie(C__CPU, &cookie_cpu) != C_FOUND)
        cookie_cpu = 0;  // 68000
    sdl1_cpu_family = (u8)cookie_cpu;

    sdl1_bench_result_ticks = 0;
    Supexec(atari_bench);
    sdl1_bench_ticks = sdl1_bench_result_ticks;

    sdl1_cpu_tier = atari_cpu_tier(sdl1_bench_ticks, sdl1_cpu_family);
}

static int atari_audio_rate(void)
{
    // Only hardware DMA rates: for an off-rate SDL plays the nearest legal one but
    // may report the requested rate, which skews isr_step (YM/SFX tick tempo).
    switch (sdl1_cpu_tier)
    {
        case ATARI_TIER_FASTEST:   return 25033;
        case ATARI_TIER_FASTER:    return 25033;
        case ATARI_TIER_FAST:      return 12517;
        case ATARI_TIER_MEDIUM:    return 12517;
        case ATARI_TIER_SLOW:      return  6258;
        default:                   return 12517;
    }
}

// STE/TT DMA sound supports only fixed hardware rates (6258/12517/25033/50066).
// Pick the one that matches the CPU tier exactly, so the mixer's resampling
// (driven by audio_spec->freq) and the actual playback rate agree.
static int atari_dma_rate(void)
{
    switch (sdl1_cpu_tier)
    {
        case ATARI_TIER_FASTEST:   return 25033;
        case ATARI_TIER_FASTER:    return 25033;
        case ATARI_TIER_FAST:      return 12517;
        case ATARI_TIER_MEDIUM:    return 12517;
        case ATARI_TIER_SLOW:      return  6258;
        default:                   return 12517;
    }
}

// Does this machine have the STE-style 8-bit DMA sound chip (STE/MegaSTE/TT)?
// (The Falcon has it too, but defaults to its DSP — see atari_use_dma_backend.)
static int atari_has_dma_sound(void)
{
    return sdl1_machine_type == SDL1_MCH_STE
        || sdl1_machine_type == SDL1_MCH_TT;
}

// We drive the audio hardware ourselves only on STE/MegaSTE/TT (DMA) and Falcon
// (DSP or DMA); everything else (ST, clones, GSXB, unknown _MCH) uses SDL audio.
static int atari_own_audio(void)
{
    return sdl1_machine_type == SDL1_MCH_STE
        || sdl1_machine_type == SDL1_MCH_TT
        || sdl1_machine_type == SDL1_MCH_FALCON;
}

// 1 = CPU-mixed DMA path. --audio: 0=auto (STE/TT→DMA, Falcon→DSP), 1=DSP
// (DMA off-Falcon), 2=DMA. Always 0 when SDL owns the audio.
extern int atari_audio_backend;
static int atari_use_dma_backend(void)
{
    if (!atari_own_audio()) return 0;                             // clones/ST → SDL
    if (atari_audio_backend == 2) return 1;                       // forced DMA
    if (atari_audio_backend == 1)                                 // forced DSP
        return (sdl1_machine_type == SDL1_MCH_FALCON) ? 0 : 1;    // no DSP off-Falcon
    return atari_has_dma_sound();                                 // auto
}

// Falcon DMA rates are 98340/(prescale+1). The CPU does all mixing on this path,
// so keep the rate low on a stock 030 (8195 Hz = Devconnect floor).
static int atari_dma_rate_machine(void)
{
    if (sdl1_machine_type == SDL1_MCH_FALCON)
    {
        switch (sdl1_cpu_tier)
        {
            case ATARI_TIER_FASTEST:
            case ATARI_TIER_FASTER:  return 16390;   // 060
            case ATARI_TIER_FAST:    return 12292;   // 040
            default:                 return  8195;   // stock 030 — Devconnect floor
        }
    }
    return atari_dma_rate();
}

#endif


const u32               k_event_ticks = (1000 / 120);  // poll keyboard 120 times per second
const u32               k_frame_ticks = (1000 / 50);   // 50 fps screen update

SDL_Surface             *surface;

extern SDL_keysym       button;
extern SDL_Event        event;
extern SDL_AudioSpec    *audio_spec;
extern SDL_Rect         dirty_rects[256];
extern SDL_Rect         dirty_mouse_rect;
extern volatile u8     dirty_pal;
extern u8              dirty_len;

extern u8               shift;
extern mouse_t          mouse;
extern bool             dirty_mouse;
extern u32              width;
extern u32              height;

extern int              audio_id;

extern u8               *vgalogic;
extern u8               *vgalogic_df;

extern u32              poll_ticks;
extern u32              frame_time;
extern u32              loop_time;

extern u32              isr_step;
extern u32              isr_counter;

u8                      *prev_pixels = NULL;
u8                      mouse_pixels[16 * 16] =
    { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
      0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
      0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
      0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
      0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
      0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
      0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
      0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
      0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
      0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
      0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,
      0,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,
      0,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
      0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0};
u8 mouse_bg_pixels[16 * 16];
u8 mouse_bg_restore = 0;

s32 surface_y_offset = 0;    // vertical centering when surface is taller than game buffer
s32 surface_stride = 0;      // surface pitch in pixels (surface->pitch / bpp)

// SDL granted SDL_DOUBLEBUF. Only then do SDL's Atari drivers page-swap on Vsync;
// otherwise rects are drawn into the visible screen. See sdl1_set_8bpp_mode().
static int use_flip = 0;

// `surface` is the render target, `display` the SDL screen (same when scale==1).
// scale>1: display is width*scale × height*scale*1.2, SoftStretched on flush.
// Always 1 on Atari, where the framebuffer is the display.
extern int   opt_scale;  // shared with sys_sdl2.c — set by --sN
SDL_Surface *display = NULL;
static s32   display_sx_num = 1, display_sx_den = 1;  // x scale = num/den
static s32   display_sy_num = 1, display_sy_den = 1;  // y scale = num/den (includes 1.2)

#if !defined(__TOS__) && !defined(__atarist__)
static u32 surface_palette[256]; // palette converted to native surface pixel format
#endif
SDL_Color hicolor8_palette[256]; // quantized 8 bit hicolor palette for Spectrum 512 and HAM videos

// 9-bit ST color LUT — one array populated at init based on surface bpp
static union { u32 p32[512]; u16 p16[512]; u8 p8[512]; } lut9;
static u8 lut9_bpp = 0;

void sys_audio_callback_S16MSB(void *userdata, u8 *stream, s32 len);
void sys_audio_callback_S8(void *userdata, u8 *stream, s32 len);

#if defined(__atarist__) || defined(__TOS__)

#include <signal.h>
#include <sys/time.h>
#include <mint/cookie.h>

extern void *atari_old_etv_timer;
extern void *atari_old_etv_term;   // BSS slot in sys_atari_isr.S
extern void atari_timerc_trampoline(void);

void atari_etv_term_handler(void);                    // forward decl
static long atari_timerc_uninstall_supexec(void);  // forward decl

// --no-timerc: skip the Timer C ($400) hook to test etv_term ($408) in isolation.
extern int atari_no_timerc;

// Re-entry guard for MiNT's p_term loop.
volatile u32 atari_etv_term_fired = 0;

// Set at the end of sys_deinit: etv_term then only chains to SDL's handler.
volatile u32 atari_cleanup_done = 0;

// Vectors saved before SDL_Init, restored in etv_term in case SDL can't unhook
// them at termination (a stale vector under MiNT memprot panics). As ScummVM.
static void *atari_old_timera_vec = NULL;  // MFP Timer A ($134, vec 0x4D)
static void *atari_old_kbdvec     = NULL;  // IKBD ACIA byte handler
static void *atari_old_mousevec   = NULL;  // IKBD mouse packet handler
static int   atari_hw_vectors_saved = 0;

static long atari_save_hw_vectors_supexec(void)
{
    atari_old_timera_vec = (void *)Setexc(0x4D, -1);

    _KBDVECS *kbdvecs = Kbdvbase();
    atari_old_kbdvec   = (void *)((unsigned long *)kbdvecs)[-1];
    atari_old_mousevec = (void *)kbdvecs->mousevec;

    atari_hw_vectors_saved = 1;
    return 0;
}

static long atari_restore_hw_vectors_supexec(void)
{
    if (!atari_hw_vectors_saved) return 0;

    // Timer A first — silences any pending SDMA IRQ at the source.
    Setexc(0x4D, (long)atari_old_timera_vec);

    _KBDVECS *kbdvecs = Kbdvbase();
    ((unsigned long *)kbdvecs)[-1] = (unsigned long)atari_old_kbdvec;
    kbdvecs->mousevec = (void(*)(void *))atari_old_mousevec;

    atari_hw_vectors_saved = 0;
    return 0;
}

// etv_term ($408) handler, called in user mode at termination (even SIGKILL).
// Order (as ScummVM critical_restore): stop audio DMA (it reads our soon-freed
// memory), restore HW vectors, silence YM, unhook Timer C, SDL_Quit last.
void atari_etv_term_handler(void)
{
    // Re-entry guard for MiNT's do_exit p_term loop.
    if (atari_etv_term_fired)
        return;
    atari_etv_term_fired = 1;

    void *sdl_old_term = atari_old_etv_term;
    atari_old_etv_term = NULL;

    // Normal shutdown already cleaned up; repeating it could fault. Just chain.
    if (atari_cleanup_done) {
        if (sdl_old_term)
            Setexc(0x102, (long)sdl_old_term);
        return;
    }

    // Abnormal exit: clean up here, then restore (not call) SDL's chain.
    Buffoper(0);
    Supexec(atari_restore_hw_vectors_supexec);
    sys_deinit_psg();
    if (atari_old_etv_timer)
        Supexec(atari_timerc_uninstall_supexec);
    SDL_Quit();

    if (sdl_old_term)
        Setexc(0x102, (long)sdl_old_term);
}

static int atari_timerc_clock_div = 1;
static volatile u32 atari_timerc_subtick = 0;
static u8 mint_timer_active = 0;

void atari_timerc_c_callback(void)
{
    if (++atari_timerc_subtick < (u32)atari_timerc_clock_div)
        return;
    
    atari_timerc_subtick = 0;

    if (!image.fitroutine)
        itroutine(0, NULL);
}

// ---- MFP Timer A: 50 Hz audio feed (Falcon DSP tracker / STE-TT DMA mix) ----
// Timer A is free for apps; we own vector $134 and program the MFP directly,
// as Simplet's Init_Music_IT. 50 Hz = 2.4576 MHz / (prescale 200 * data 246).
#if defined(ALIS_DSP_MIXER) && defined(ALIS_DSP_USE_TRACKER) && (defined(__atarist__) || defined(__TOS__))

extern void atari_timera_trampoline(void);   // sys_atari_isr.S

#define MFP_IERA  (*(volatile u8 *)0xFFFFFA07)
#define MFP_IPRA  (*(volatile u8 *)0xFFFFFA0B)
#define MFP_ISRA  (*(volatile u8 *)0xFFFFFA0F)
#define MFP_IMRA  (*(volatile u8 *)0xFFFFFA13)
#define MFP_TACR  (*(volatile u8 *)0xFFFFFA19)
#define MFP_TADR  (*(volatile u8 *)0xFFFFFA1F)
#define MFP_TIMERA_BIT 0x20                   // channel 13 = bit 5 of the A regs
#define MFP_TIMERA_VEC (*(volatile u32 *)0x134UL)

static u32 atari_old_timera = 0;
static u8  atari_timera_installed = 0;

// Timer A ISR body. Lower IPL during the long feed so the ACIA isn't overrun:
// (1) EOI Timer A so lower MFP channels (ACIA) can interrupt, (2) mask Timer A
// against re-entry, (3) drop IPL to 5, (4) restore on exit.
void atari_timera_c_callback(void)
{
    unsigned short saved_sr;

    MFP_ISRA  = (u8)~MFP_TIMERA_BIT;          // EOI: clear in-service
    MFP_IMRA &= (u8)~MFP_TIMERA_BIT;          // mask Timer A (no re-entry)

    __asm__ volatile(
        "move.w %%sr,%0\n\t"                  // save SR (entered at IPL 6)
        "move.w %0,%%d0\n\t"
        "andi.w #0xF8FF,%%d0\n\t"             // clear IPL field
        "ori.w  #0x0500,%%d0\n\t"             // IPL = 5 → only the ACIA/MFP
        "move.w %%d0,%%sr\n\t"                //   (IPL 6) preempts, not VBL/HBL
        : "=&d"(saved_sr) : : "d0", "cc", "memory");

    // YM writes during the feed (sys_psg_tick, mv2_chiprout) must go direct, not via XBIOS
    // Giaccess (not reentrant from an ISR → intermittent reset). See g_psg_isr_context.
    extern volatile u8 g_psg_isr_context;
    u8 _psg_saved = g_psg_isr_context;
    g_psg_isr_context = 1;
    if (atari_dma_available)
        atari_dma_sound_tick_isr();           // STE/TT: software-mix one frame into DMA buf
    else
        dsp_mixer_tick_isr();                 // Falcon: sequencer tick + 8-voice DSP feed
    g_psg_isr_context = _psg_saved;

    __asm__ volatile("move.w %0,%%sr" : : "d"(saved_sr) : "cc", "memory");

    // Drop a tick that went pending during a long feed, else it re-fires at
    // once and back-to-back feeds starve the CPU.
    MFP_IPRA  = (u8)~MFP_TIMERA_BIT;          // discard any pending Timer A
    MFP_IMRA |= MFP_TIMERA_BIT;               // re-enable Timer A
}

static long atari_timera_install_supexec(void)
{
    MFP_TACR  = 0;                            // stop Timer A while we set up
    atari_old_timera = MFP_TIMERA_VEC;        // save the previous vector
    MFP_TIMERA_VEC = (u32)atari_timera_trampoline;
    MFP_IPRA  = (u8)~MFP_TIMERA_BIT;          // clear any pending
    MFP_ISRA  = (u8)~MFP_TIMERA_BIT;          // clear in-service
    MFP_TADR  = 246;                          // 2457600 / (200*246) ≈ 49.95 Hz
    MFP_IERA |= MFP_TIMERA_BIT;               // enable Timer A interrupt
    MFP_IMRA |= MFP_TIMERA_BIT;               // unmask it
    MFP_TACR  = 7;                            // start: prescaler /200
    atari_timera_installed = 1;
    return 0;
}

static long atari_timera_uninstall_supexec(void)
{
    MFP_TACR  = 0;                            // stop Timer A
    MFP_IERA &= (u8)~MFP_TIMERA_BIT;          // disable + mask its interrupt
    MFP_IMRA &= (u8)~MFP_TIMERA_BIT;
    if (atari_old_timera)
        MFP_TIMERA_VEC = atari_old_timera;    // restore previous vector
    atari_timera_installed = 0;
    return 0;
}
#endif /* ALIS_DSP_MIXER */

static long atari_timerc_install_supexec(void)
{
    if (!atari_no_timerc)
        atari_old_etv_timer = (void *)Setexc(0x100, (long)atari_timerc_trampoline);
    // etv_term ($408) is set in user mode (atari_timerc_install): only the
    // trap path updates MiNT's per-process p_term entry.
    return 0;
}

static long atari_timerc_uninstall_supexec(void)
{
    // Only Timer C ($400): etv_term ($408) must stay hooked so MiNT's do_exit
    // still reaches our handler.
    if (atari_old_etv_timer)
        Setexc(0x100, (long)atari_old_etv_timer);
    return 0;
}

static u32 atari_calibrate_hz200_snapshot;
static long atari_read_hz200_supexec(void)
{
    atari_calibrate_hz200_snapshot = *(volatile u32 *)0x4BAUL;
    return 0;
}

static u32 atari_read_hz200(void)
{
    Supexec(atari_read_hz200_supexec);
    return atari_calibrate_hz200_snapshot;
}

static void atari_timerc_calibrate(void)
{
    atari_timerc_clock_div = 0x10000;
    atari_timerc_subtick   = 0;

    u32 t0 = atari_read_hz200();
    while (atari_read_hz200() == t0) {}
    t0 = atari_read_hz200();

    atari_timerc_subtick = 0;
    while (atari_read_hz200() - t0 < 200) {}
    u32 isr_count = atari_timerc_subtick;

    int target_hz = alis.platform.is_little_endian ? 60 : 50;
    int div = ((int)isr_count + target_hz / 2) / target_hz;
    if (div < 1)
        div = 1;
    if (div > 64)
        div = 64;

    atari_timerc_clock_div = div;
    atari_timerc_subtick   = 0;

    // Publish the actual rate (the divider may not hit the target) for FLI pacing.
    sys_timeclock_hz = (u32)isr_count / (u32)div;

    printf("  Atari Timer C: %d ISRs/s -> divider %d (target %d Hz, actual %u Hz)\n",
           (int)isr_count, div, target_hz, (unsigned)sys_timeclock_hz);
}

void atari_timerc_emergency_uninstall(void)
{
    // Restore only Timer C ($400); leave etv_term ($408) installed so
    // MiNT's do_exit can still walk our handler for final cleanup.
    if (atari_old_etv_timer || atari_old_etv_term)
    {
        Supexec(atari_timerc_uninstall_supexec);
        atari_old_etv_timer = NULL;
    }
}

static void atari_timerc_install(void)
{
    atari_timerc_clock_div = 0x10000;
    atari_timerc_subtick = 0;
    Supexec(atari_timerc_install_supexec);

    // etv_term in user mode so MiNT's do_exit walks our handler.
    atari_old_etv_term = (void *)Setexc(0x102, (long)atari_etv_term_handler);

    atexit(atari_timerc_emergency_uninstall);

    // No SIGTERM/SIGINT handlers here: sys.c's signals_handler stops the VM so
    // the normal sys_deinit path runs.

    if (atari_no_timerc) {
        printf("  Atari Timer C: SKIPPED (--no-timerc); etv_term hook only\n");
    } else {
        atari_timerc_calibrate();
    }
}

static void atari_timerc_uninstall(void)
{
    atari_timerc_emergency_uninstall();
}

#else /* !Atari: SDL_AddTimer path */

static SDL_TimerID timer_id = NULL;

#endif

// ============================================================================
#pragma mark - SDL1 System init
// ============================================================================

u32 sys_profile_ticks(void) { return SDL_GetTicks(); }
u32 sys_ticks(void) { return SDL_GetTicks(); }
void sys_delay(u32 ms) { SDL_Delay(ms); }
u8 *sys_get_framebuffer(int index) { (void)index; return NULL; }  // VM mallocs the render buffers

#if defined(__TOS__) || defined(__atarist__)
// SDL c2p-converts dirty rects into the screen; without SDL_DOUBLEBUF that's the
// displayed buffer with no Vsync, so the frame visibly assembles. Ask for it and
// fall back if not granted (no second ST-RAM screen, or GEM).
static SDL_Surface *sdl1_set_8bpp_mode(s32 w, s32 h)
{
    SDL_Surface *s = SDL_SetVideoMode(w, h, 0, SDL_HWSURFACE|SDL_HWPALETTE|SDL_DOUBLEBUF);
    if (s == NULL)
        s = SDL_SetVideoMode(w, h, 0, SDL_HWSURFACE|SDL_HWPALETTE);
    return s;
}
#endif

void sys_init(sPlatform *pl, int fullscreen, int mutesound) {
    dirty_mouse_rect = (SDL_Rect){ .x = host.pixelbuf.w, .y = host.pixelbuf.h, .w = 0, .h = 0 };

    width = pl->width;
    height = pl->height;

#if defined(__atarist__) || defined(__TOS__)
    // Save HW vectors before SDL_Init so etv_term can restore them.
    Supexec(atari_save_hw_vectors_supexec);

    // Detect the machine first: it decides whether SDL or we own the audio.
    atari_detect_machine();
    printf("  machine_type=%d cpu_family=%d bench_ticks=%u tier=%d own_audio=%d\n",
           (int)sdl1_machine_type, (int)sdl1_cpu_family, (unsigned)sdl1_bench_ticks,
           (int)sdl1_cpu_tier, atari_own_audio());
#endif

    printf("  SDL initialization...\n");
    // STE/TT/Falcon: we own the audio hardware, so skip SDL audio (it causes
    // ACIA underruns). Other Ataris use SDL's audio drivers.
    {
        Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
#if !defined(__atarist__) && !defined(__TOS__)
        sdl_flags |= SDL_INIT_TIMER;
        sdl_flags |= SDL_INIT_AUDIO;
#elif !(defined(ALIS_DSP_MIXER))
        sdl_flags |= SDL_INIT_AUDIO;          // non-DSP Atari build: always SDL audio
#else
        if (!atari_own_audio())               // clones/ST → let SDL drive audio
            sdl_flags |= SDL_INIT_AUDIO;
#endif
        if (SDL_Init(sdl_flags) < 0) {
            fprintf(stderr, "   Unable to initialize SDL: %s\n", SDL_GetError());
            exit(-1);
        }
    }

    {
        extern SDL_Joystick *sys_joy_handle;
        // Polled via SDL_JoystickGet* in io_joy; we don't need events queued.
        SDL_JoystickEventState(SDL_IGNORE);
        if (SDL_NumJoysticks() > 0) {
            sys_joy_handle = SDL_JoystickOpen(0);
            if (sys_joy_handle) {
                printf("  Joystick: %s\n", SDL_JoystickName(0));
            }
        }
    }
    
    printf("  Video initialization...\n");
    
    SDL_WM_SetCaption(kProgName, "icon");

    // XBIOS switches the screen mode to 320×200 — no headroom to scale into,
    // so force scale=1. GEM driver runs inside the desktop at higher
    // resolution and benefits from scaling.
#if defined(__TOS__) || defined(__atarist__)
    {
        const char *drv = SDL_VideoDriverName((char[16]){0}, 16);
        if (drv && strcmp(drv, "gem") != 0) {
            opt_scale = 1;
        }
    }
#endif

    if (opt_scale < 1) opt_scale = 1;
    if (opt_scale > 4) opt_scale = 4;

    // ×1.2 aspect correction only when scaling.
    s32 disp_w, disp_h;
    if (opt_scale > 1) {
        disp_w = (s32)width  * opt_scale;
        disp_h = (s32)height * opt_scale * 12 / 10;
        display_sx_num = opt_scale;        display_sx_den = 1;
        display_sy_num = opt_scale * 12;   display_sy_den = 10;
    } else {
        disp_w = (s32)width;
        disp_h = (s32)height;
        display_sx_num = 1;  display_sx_den = 1;
        display_sy_num = 1;  display_sy_den = 1;
    }

#if defined(__TOS__) || defined(__atarist__)
# if defined(ALIS_FALCON_16BPP) && ALIS_FALCON_16BPP
    // 16-bit Videl truecolor surface (chunky, no c2p).
    display = SDL_SetVideoMode(disp_w, disp_h, 16, SDL_HWSURFACE);
    if (display == NULL)  // fall back to native if the driver won't give 16bpp
        display = sdl1_set_8bpp_mode(disp_w, disp_h);
# else
    display = sdl1_set_8bpp_mode(disp_w, disp_h);
# endif
#else
    display = SDL_SetVideoMode(disp_w, disp_h, 32, SDL_SWSURFACE);
#endif
    if (display == NULL) {
        fprintf(stderr, "   Could not create %dx%d window: %s\n", disp_w, disp_h, SDL_GetError());
        exit(-1);
    }

    if (opt_scale > 1) {
        // Memory back buffer at native resolution, same pixel format as display.
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
                                       display->format->BitsPerPixel,
                                       display->format->Rmask, display->format->Gmask,
                                       display->format->Bmask, display->format->Amask);
        if (surface == NULL) {
            fprintf(stderr, "   Could not create %dx%d back buffer: %s\n", (int)width, (int)height, SDL_GetError());
            exit(-1);
        }
        if (display->format->BitsPerPixel == 8 && display->format->palette) {
            SDL_SetColors(surface, display->format->palette->colors, 0, 256);
        }
    } else {
        surface = display;  // alias — no scaling
    }

    // The scaled path goes through SDL_SoftStretch and never uses SDL_DOUBLEBUF.
    use_flip = (display == surface) && ((display->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF);

    surface_y_offset = surface->pitch ? (surface->offset / surface->pitch) : 0;
    prev_pixels = (u8 *)surface->pixels - surface->offset;
    surface_stride = surface->pitch / (surface->format->BitsPerPixel / 8);

    host.pixelbuf.surface_h     = (u16)surface->h;
    host.pixelbuf.surface_y_off = (u16)surface_y_offset;

    printf("  Surface: %dx%d pitch=%d offset=%d (requested %dx%d, y_offset=%d)\n", surface->w, surface->h, surface->pitch, surface->offset, (int)width, (int)height, (int)surface_y_offset);
    printf("  Present: %s\n", use_flip ? "SDL_DOUBLEBUF page flip (vsynced)" : "single buffer (no vsync)");
    printf("  Surface format: BPP=%d Rmask=0x%08x Gmask=0x%08x Bmask=0x%08x Amask=0x%08x\n", surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);

    switch (surface->format->BitsPerPixel) {
        case 32: {
#if defined(__TOS__) || defined(__atarist__)
            image.pal_format = EPalARGB;
#else
            image.pal_format = EPalABGR;
#endif
            break;
        }
            
        case 16: {
            image.pal_format = EPal565;
            break;
        }
            
        case 8: {
            image.pal_format = EPalRGBA32;
            
            for (int i = 0; i < 256; i++) {
                hicolor8_palette[i].r = (i & 0b11000000);
                hicolor8_palette[i].g = (i & 0b00111000) << 2;
                hicolor8_palette[i].b = (i & 0b00000111) << 5;
            }

            break;
        }
            
        default: {
            image.pal_format = EPalARGB;
            break;
        }
    }
    
    // Build 9-bit ST color LUT for S512/HAM video modes
    lut9_bpp = surface->format->BitsPerPixel;
    for (int c = 0; c < 512; c++) {
        u8 r = ((c >> 6) & 7) << 5;
        u8 g = ((c >> 3) & 7) << 5;
        u8 b = ( c       & 7) << 5;
        switch (lut9_bpp) {
            case 32:
#if !defined(__TOS__) && !defined(__atarist__)
                lut9.p32[c] = ((u32)b << 24) | ((u32)g << 16) | ((u32)r << 8);
#else
                lut9.p32[c] = ((u32)r << 16) | ((u32)g << 8) | b;
#endif
                break;
            case 16:
                lut9.p16[c] = (u16)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
                break;
            case 8:
                lut9.p8[c] = (u8)(((r >> 6) << 6) | ((g >> 5) << 3) | (b >> 5));
                break;
        }
    }

    // set icon
    SDL_RWops *rw = SDL_RWFromConstMem(alis_icon8_bmp, (int)alis_icon8_bmp_len);
    if (rw) {
        SDL_Surface *icon_surface = SDL_LoadBMP_RW(rw, 1);
        if (icon_surface) {
            SDL_WM_SetIcon(icon_surface, NULL);
            SDL_FreeSurface(icon_surface);
        }
    }

    dirty_mouse = 0;
    
    printf("  Audio initialization...\n");

    audio_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));

    SDL_AudioSpec *desired_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));

    memset(desired_spec, 0, sizeof(SDL_AudioSpec));

#if defined(__atarist__)
    desired_spec->freq = atari_audio_rate();
#else
    desired_spec->freq = 12517;
#endif
    desired_spec->format = AUDIO_S16MSB;
#if defined(FORCE_MONO_AUDIO)
    desired_spec->channels = 1;
#elif defined(__atarist__)
    desired_spec->channels = (sdl1_machine_type == SDL1_MCH_ST) ? 1 : 2;
#else
    desired_spec->channels = 2;
#endif
    desired_spec->samples = desired_spec->freq / 20; // 20 ms
    desired_spec->callback = sys_audio_callback_S16MSB;
    desired_spec->userdata = NULL;

#if defined(ALIS_DSP_MIXER) && (defined(__atarist__) || defined(__TOS__))
    // DMA sound (STE/TT, or Falcon with --audio=dma): 8-bit signed at a fixed
    // hardware rate. SDL_OpenAudio is skipped, so these values are final.
    if (atari_use_dma_backend())
    {
        desired_spec->freq     = atari_dma_rate_machine();
        desired_spec->format   = AUDIO_S8;
        // Falcon DMA is stereo-only; mono on STE/TT halves the mix cost.
        desired_spec->channels = (sdl1_machine_type == SDL1_MCH_FALCON) ? 2 : 1;
        desired_spec->samples  = desired_spec->freq / 50;
        desired_spec->callback = sys_audio_callback_S8;
    }
#endif

    printf("  Audio desired: freq=%d format=0x%04x channels=%d samples=%d\n", (int)desired_spec->freq, (unsigned)desired_spec->format, (int)desired_spec->channels, (int)desired_spec->samples);

#if defined(ALIS_DSP_MIXER) && (defined(__atarist__) || defined(__TOS__))
    // Own-audio machines: audio_spec = desired_spec, no SDL_OpenAudio.
    if (atari_own_audio()) {
        memcpy(audio_spec, desired_spec, sizeof(SDL_AudioSpec));
        printf("    Own audio backend (DSP/DMA): SDL_OpenAudio skipped.\n");
    } else
#endif
    if (SDL_OpenAudio(desired_spec, audio_spec) != 0) {
        fprintf(stderr, "   Could not open audio: %s\n", SDL_GetError());
        free(audio_spec);
        surface->pixels = prev_pixels;
        SDL_FreeSurface(surface);
        SDL_Quit();
        exit(-1);
    }

    if (audio_spec->format != desired_spec->format || audio_spec->channels != desired_spec->channels || audio_spec->freq != desired_spec->freq) {
        printf("    Non-native audio:\n");
        if (audio_spec->format != desired_spec->format)
            printf("      format:   desired=0x%04x, native=0x%04x\n", (unsigned)desired_spec->format, (unsigned)audio_spec->format);
        if (audio_spec->channels != desired_spec->channels)
            printf("      channels: desired=%d, native=%d\n", (int)desired_spec->channels, (int)audio_spec->channels);
        if (audio_spec->freq != desired_spec->freq)
            printf("      freq:     desired=%d, native=%d\n", (int)desired_spec->freq, (int)audio_spec->freq);

        if (audio_spec->format == desired_spec->format)
        {
            // Same format: SDL inserted no converter, and the mixer and isr_step
            // use audio_spec's freq/channels, so just accept them.
            printf("    Format matches — accepting SDL's native rate/channels (no converter).\n");
        }
        else
        {
            SDL_CloseAudio();

            if (audio_spec->format == AUDIO_S8)
            {
                printf("    Using native S8 callback — no SDL converter.\n");
                desired_spec->format   = AUDIO_S8;
                desired_spec->channels = audio_spec->channels;
                desired_spec->freq     = audio_spec->freq;
                desired_spec->callback = sys_audio_callback_S8;
                if (SDL_OpenAudio(desired_spec, audio_spec) != 0) {
                    printf("    SDL_OpenAudio S8 retry FAILED: %s\n", SDL_GetError());
                    fprintf(stderr, "   SDL_OpenAudio S8 retry FAILED: %s\n", SDL_GetError());
                    free(audio_spec);
                    surface->pixels = prev_pixels;
                    SDL_FreeSurface(surface);
                    SDL_Quit();
                    exit(-1);
                }

                printf("    S8 path OK; callback writes freq=%d format=0x%04x channels=%d\n", (int)audio_spec->freq, (unsigned)audio_spec->format, (int)audio_spec->channels);
            }
            else
            {
                // Format we don't have a native callback for (rare on
                // Atari, e.g. SDL emulator giving S16LSB). Last resort.
                printf("    Falling back to SDL converter path (S16MSB callback).\n");
                desired_spec->callback = sys_audio_callback_S16MSB;
                if (SDL_OpenAudio(desired_spec, NULL) != 0) {
                    fprintf(stderr, "   SDL_OpenAudio retry FAILED: %s\n", SDL_GetError());
                    free(audio_spec);
                    surface->pixels = prev_pixels;
                    SDL_FreeSurface(surface);
                    SDL_Quit();
                    exit(-1);
                }

                memcpy(audio_spec, desired_spec, sizeof(SDL_AudioSpec));
                printf("    Converter path OK; callback writes freq=%d format=0x%04x channels=%d\n", (int)audio_spec->freq, (unsigned)audio_spec->format, (int)audio_spec->channels);
            }
        }
    }
    
#if defined(__atarist__) || defined(__TOS__)
    // Log requested vs obtained rate: isr_step is only right if SDL reports
    // the rate the hardware really plays.
    if (!atari_own_audio()) {
        FILE *alog = fopen("audio_log.txt", "w");
        if (alog) {
            fprintf(alog, "SDL audio: tier=%d requested=%d obtained=%d fmt=0x%04x ch=%d tick_hz=%d\n",
                    (int)sdl1_cpu_tier, (int)desired_spec->freq, (int)audio_spec->freq,
                    (unsigned)audio_spec->format, (int)audio_spec->channels,
                    (int)(alis.platform.is_little_endian ? 60 : 50));
            fclose(alog);
        }
    }
#endif

    free(desired_spec);

    audio.host_freq = audio_spec->freq;
    audio.host_format = audio_spec->format;

#if !defined(__TOS__) && !defined(__atarist__)
    sys_init_psg();
#endif

#if defined(ALIS_DSP_MIXER) && (defined(__atarist__) || defined(__TOS__))
    // Own audio backends: CPU-mixed DMA, or the Falcon DSP. The YM2149 bypasses
    // the matrix, so it's audible either way.
    if (atari_use_dma_backend())
    {
        // CPU-mixed DMA path. STE/TT wire DMA→DAC directly; the Falcon needs
        // its crossbar routed (is_falcon=1). Do NOT call dsp_mixer_init() on a
        // non-Falcon — the DSP XBIOS calls it makes don't exist there.
        int is_falcon = (sdl1_machine_type == SDL1_MCH_FALCON);
        int dma_rc = atari_dma_sound_init(audio_spec->freq, audio_spec->channels, is_falcon);
        FILE *dlog = fopen("dma_log.txt", "a");    // init wrote the header ("w")
        if (dlog) { fprintf(dlog, "DMA: backend=%d falcon=%d init=%d available=%d\n",
                            atari_audio_backend, is_falcon, dma_rc, atari_dma_available); }
        // Refill from the 50 Hz Timer-A ISR so music tempo is independent of frame rate.
#ifdef ALIS_DSP_USE_TRACKER
        if (atari_dma_available) {
            atari_dma_sound_set_isr_driven(1);
            Supexec(atari_timera_install_supexec);
            if (dlog) fprintf(dlog, "DMA: Timer A refill installed (50 Hz).\n");
        }
#endif
        if (dlog) fclose(dlog);
    }
    else if (atari_own_audio())   // Falcon DSP. Clones/ST fall through to SDL audio.
    {
        FILE *dlog = fopen("dsp_log.txt", "w");
        if (dlog) { fprintf(dlog, "DSP: calling init (after SDL_OpenAudio)...\n"); fflush(dlog); }
        // Load the tracker initially; the main-loop poll (dsp_audio_select_for_music)
        // hot-swaps to the FM+SFX program when OPL2 music plays, and back. The 50 Hz
        // Timer A drives whichever program is resident (tracker feed, or OPL sequencer tick).
        int dsp_rc = dsp_mixer_init();
        if (dlog) fprintf(dlog, "DSP: init returned %d, available=%d\n", dsp_rc, dsp_mixer_available);
        if (dsp_mixer_available) {
            Supexec(atari_timera_install_supexec);
            if (dlog) fprintf(dlog, "DSP: Timer A feed installed (50 Hz).\n");
        }
        if (dlog) fclose(dlog);
    }
#endif

    // 60 Hz PIT for DOS (little-endian) data, 50 Hz VBL otherwise.
    sys_sfx_tick_hz = alis.platform.is_little_endian ? 60 : 50;

    isr_step = (sys_sfx_tick_hz * 65536UL) / (u32)audio_spec->freq;
    isr_counter = 0x10000;
    
    poll_ticks = SDL_GetTicks();
    frame_time = SDL_GetTicks();
    loop_time = SDL_GetTicks();

#if defined(ALIS_DSP_MIXER) && (defined(__atarist__) || defined(__TOS__))
    // Own-audio machines never opened SDL audio; SDL_PauseAudio would hang.
    if (!atari_own_audio())
        SDL_PauseAudio(mutesound);
#else
    SDL_PauseAudio(mutesound);
#endif

#if defined(__atarist__) || defined(__TOS__)
    sys_timeclock_hz = alis.platform.is_little_endian ? 60 : 50;   // calibration overwrites
    atari_timerc_install();
#else
    {
        u32 timer_ms = alis.platform.is_little_endian ? 17 : 20;
        timer_id = SDL_AddTimer(timer_ms, itroutine, NULL);
        sys_timeclock_hz = 1000 / timer_ms;   // 17ms → 58.82 Hz, 20ms → 50 Hz
    }
#endif
}

void sys_init_timers(void) {
    
    poll_ticks = SDL_GetTicks();
    frame_time = SDL_GetTicks();
    loop_time = SDL_GetTicks();
}

void sys_sleep_until(u32 *start, s32 len)
{
    u32 now = SDL_GetTicks();
    s32 wait = len - (now - *start);
    if (wait > 0 && wait <= len)
        SDL_Delay(wait);

    *start = SDL_GetTicks();
}

void sys_sleep_interactive(s32 *loop, s32 intr)
{
    u32 prev = SDL_GetTicks();
    while (*loop > intr || (*loop > 0 && io_inkey() == 0))
    {
#if ALIS_USE_THREADS <= 0
        sys_poll_event();
#endif

        SDL_Delay(k_event_ticks);

        u32 now = SDL_GetTicks();
        *loop -= now - prev;
        prev = now;
    }

    *loop = 0;
}

void sys_delay_loop(void)
{
    sys_sleep_until(&loop_time, 6);
}

void sys_delay_frame(void)
{
    sys_sleep_until(&frame_time, k_frame_ticks * 50 / sys_pace_hz() * alis.ctiming);
}

void sys_sleep_until_music_stops(void)
{
    if (SDL_GetAudioStatus() != SDL_AUDIO_PLAYING)
        return;

    audio.working = 1;

    for (int count = 0; count < 10 && audio.muflag; count++)
    {
        SDL_Delay(10);
    }

    audio.muflag = 0;
    audio.musicId = 0xffff;

    for (int count = 0; count < 10 && audio.working; count++)
    {
        SDL_Delay(10);
    }
}

// ============================================================================
#pragma mark - SDL1 Main system loop
// ============================================================================

u8 sys_start(void) {

    alis.state = eAlisStateRunning;
    alis_thread(NULL);
    return 0;
}

void sys_poll_event(void) {

    while (SDL_PollEvent(&event)) {
        switch (event.type) {

            case SDL_MOUSEBUTTONDOWN:
            {
                if (event.button.button == SDL_BUTTON_LEFT)  { mouse.lb = 1; mouse.lb_clicked = 1; }
                if (event.button.button == SDL_BUTTON_RIGHT) { mouse.rb = 1; mouse.rb_clicked = 1; }
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                    if (event.button.button == SDL_BUTTON_LEFT)  mouse.lb = 0;
                    if (event.button.button == SDL_BUTTON_RIGHT) mouse.rb = 0;
                break;
            }
            case SDL_QUIT:
            {
                printf("\n");
                ALIS_DEBUG(EDebugSystem, "SDL: Quit.\n");
                ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
                alis.state = eAlisStateStopped;
                break;
            }
            case SDL_KEYUP:
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_LALT:
                        printf("\n");
                        ALIS_DEBUG(EDebugSystem, "INTERRUPT: User debug label.\n");
                        break;

                    case SDLK_F11:
                        alis.state = eAlisStateSave;
                        break;

                    case SDLK_F12:
                        alis.state = eAlisStateLoad;
                        break;

                    // Atari equivalents: a real ST/Falcon keyboard has no Pause and stops at F10.
                    // Shifted F-keys ARE the Atari F11-F20, so Shift+F1/F2 == F11/F12, and
                    // Shift+F10 quits (this build's Pause binding is unreachable on hardware).
                    case SDLK_F1:
                        if (SDL_GetModState() & KMOD_SHIFT)
                            alis.state = eAlisStateSave;
                        break;

                    case SDLK_F2:
                        if (SDL_GetModState() & KMOD_SHIFT)
                            alis.state = eAlisStateLoad;
                        break;

                    case SDLK_F10:
                        if (SDL_GetModState() & KMOD_SHIFT) {
                            printf("\n");
                            ALIS_DEBUG(EDebugSystem, "INTERRUPT: Quit by user request (Shift+F10).\n");
                            alis.state = eAlisStateStopped;
                        }
                        break;
                        
                    default:
                        break;
                }

                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_F11 || event.key.keysym.sym == SDLK_F12)
                {
                    break;
                }

                if (event.key.keysym.sym == SDLK_PAUSE)
                {
                    printf("\n");
                    ALIS_DEBUG(EDebugSystem, "INTERRUPT: Quit by user request.\n");
                    ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
                    alis.state = eAlisStateStopped;
                }
                else
                {
                    button = event.key.keysym;
                }
                
                break;
            }
        };
    }

    // update mouse

    SDL_GetMouseState(&mouse.x, &mouse.y);

    // Reported coords are in screen space; map back to game space.
    if (opt_scale > 1) {
        mouse.x = (s32)mouse.x * display_sx_den / display_sx_num;
        mouse.y = (s32)mouse.y * display_sy_den / display_sy_num;
    }

    if (mouse.x >= host.pixelbuf.w)
        mouse.x = host.pixelbuf.w;

    if (mouse.y >= host.pixelbuf.h)
        mouse.y = host.pixelbuf.h;

    if (mouse.enabled && mouse.x < host.pixelbuf.w && mouse.y < host.pixelbuf.h)
    {
        s16 width = mouse.x + 16 > host.pixelbuf.w ? host.pixelbuf.w - mouse.x : 16;
        s16 height = mouse.y + 16 > host.pixelbuf.h ? host.pixelbuf.h - mouse.y : 16;

        SDL_Rect new_dirty_rect = (SDL_Rect){ .x = mouse.x, .y = mouse.y, .w = width, .h = height };
        if (!(new_dirty_rect.x == dirty_mouse_rect.x && new_dirty_rect.y == dirty_mouse_rect.y && new_dirty_rect.w == dirty_mouse_rect.w && new_dirty_rect.h == dirty_mouse_rect.h))
        {
            if (dirty_len > 0xfc)
            {
                dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                dirty_len = 0xff;
            }
            else
            {
                if (dirty_mouse_rect.w > 0 && dirty_mouse_rect.h > 0)
                {
                    dirty_rects[dirty_len] = dirty_mouse_rect;
                    dirty_len++;
                }
                
                dirty_rects[dirty_len] = dirty_mouse_rect = new_dirty_rect;
                dirty_len++;
            }
        }
    }
    
    sys_render(host.pixelbuf);
}

// ============================================================================
#pragma mark - SDL1 Video
// ============================================================================

void setHAMto32bit(void *pixels, u32 px, u8 *rawcolor) {
#if !defined(__TOS__) && !defined(__atarist__)
    ((u32 *)pixels)[px] = (rawcolor[2] << 24) | (rawcolor[1] << 16) | (rawcolor[0] << 8);
#else
    ((u32 *)pixels)[px] = (u32)((rawcolor[0] << 16) | (rawcolor[1] << 8) | rawcolor[2]);
#endif
}

void setHAMto16bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u16 *)pixels)[px] = (u16)(((rawcolor[0] >> 3) << 11) | ((rawcolor[1] >> 2) << 5) | (rawcolor[2] >> 3));
}

void setHAMto8bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u8 *)pixels)[px] = (u8)(((rawcolor[0] >> 6) << 6) | ((rawcolor[1] >> 5) << 3) | (rawcolor[2] >> 5));
}

void set9to32bit(void *pixels, u32 px, u8 *rawcolor) {
#if !defined(__TOS__) && !defined(__atarist__)
    ((u32 *)pixels)[px] = (u32)((((rawcolor[0] & 0x07) << 5) << 8) | (((rawcolor[1] >> 4) << 5) << 16) | ((u32)((rawcolor[1] & 0x07) << 5) << 24));
#else
    ((u32 *)pixels)[px] = (u32)((((rawcolor[0] & 0b00000111) << 5) << 16) | (((rawcolor[1] >> 4) << 5) << 8) | (((rawcolor[1] & 0b00000111) << 5)));
#endif
}

void set9to16bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u16 *)pixels)[px] = (u16)((((rawcolor[0] & 0b00000111) << 2) << 11) + (((rawcolor[1] >> 4) << 3) << 5) + (((rawcolor[1] & 0b00000111) << 2)));
}

void set9to8bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u8 *)pixels)[px] = (u8)((((rawcolor[0] & 0b00000111) >> 1) << 6) + ((rawcolor[1] >> 4) << 3) + (((rawcolor[1] & 0b00000111))));
}

// S512 renderer — extracted to its own function to reduce register pressure
// and avoid GCC ICE in push_reload at -O3 on m68k.
static void __attribute__((noinline)) render_s512(pixelbuf_t buffer)
{
    #define ST_PAL9(v) ((((v) >> 8) & 7) << 6 | (((v) >> 4) & 7) << 3 | ((v) & 7))

    if (lut9_bpp == 8 && bfilm.frame == 1) {
        SDL_SetPalette(surface, SDL_PHYSPAL, hicolor8_palette, 0, 256);
        if (display != surface)
            SDL_SetPalette(display, SDL_PHYSPAL, hicolor8_palette, 0, 256);
    }

    u8 *bitmap = vgalogic_df + 0xa0;
    u16 curpal[32];
    memcpy(curpal, vgalogic_df + 32000, 32);
    u16 *palette = (u16 *)(vgalogic_df + 32000 + 32);
    int palcntr = 0;
    u32 at = 0;
    s32 prev_pi = -1;

    #define BP4_IDX(b0,b1,b2,b3, bit) \
        (((b0>>bit)&1) | (((b1>>bit)&1)<<1) | (((b2>>bit)&1)<<2) | (((b3>>bit)&1)<<3))

    #define S512_PIX(row, col, b0, b2, b4, b6, bit) do { \
        s32 _pi = palcntr >> 10; \
        if (_pi != prev_pi) { \
            curpal[_pi] = palette[_pi]; \
            llut[_pi] = glut[ST_PAL9(curpal[_pi])]; \
            prev_pi = _pi; \
        } \
        (row)[(col)] = llut[BP4_IDX(b0,b2,b4,b6, bit)]; \
        palcntr += 102; \
    } while(0)

    #define S512_8PIX(row, col, i) do { \
        u8 _b0 = bitmap[(i)], _b2 = bitmap[(i)+2], _b4 = bitmap[(i)+4], _b6 = bitmap[(i)+6]; \
        S512_PIX(row, (col)+0, _b0,_b2,_b4,_b6, 7); \
        S512_PIX(row, (col)+1, _b0,_b2,_b4,_b6, 6); \
        S512_PIX(row, (col)+2, _b0,_b2,_b4,_b6, 5); \
        S512_PIX(row, (col)+3, _b0,_b2,_b4,_b6, 4); \
        S512_PIX(row, (col)+4, _b0,_b2,_b4,_b6, 3); \
        S512_PIX(row, (col)+5, _b0,_b2,_b4,_b6, 2); \
        S512_PIX(row, (col)+6, _b0,_b2,_b4,_b6, 1); \
        S512_PIX(row, (col)+7, _b0,_b2,_b4,_b6, 0); \
    } while(0)

    #define S512_REBUILD_LLUT() do { \
        for (int _c = 0; _c < 16; _c++) \
            llut[_c] = glut[ST_PAL9(curpal[_c])]; \
    } while(0)

    #define S512_RENDER_LOOP(type, lut) do { \
        type const *glut = (lut); \
        type llut[16]; \
        S512_REBUILD_LLUT(); \
        for (int y = 0; y < buffer.h; y++, palette += 16) \
        { \
            u8 _halves = fls_dirty_lines[y] | fls_prev_dirty[y]; \
            if (!_halves) { \
                at += 160; \
                palette += 16; \
                palette += 16; \
                memcpy(curpal, palette, 32); \
                S512_REBUILD_LLUT(); \
                continue; \
            } \
            type *row = (type *)((u8 *)surface->pixels \
                    + (y + surface_y_offset) * surface->pitch); \
            u32 col = 0; \
            if (_halves & 1) { \
                palcntr = 408; prev_pi = -1; \
                for (int x = 0; x < 10; x++, at += 8) \
                    for (int i = at; i < at + 2; i++, col += 8) \
                        S512_8PIX(row, col, i); \
            } else { \
                at += 80; col = 160; \
                memcpy(curpal, palette, 32); \
                S512_REBUILD_LLUT(); \
            } \
            palette += 16; \
            if (_halves & 2) { \
                palcntr = 510; prev_pi = -1; \
                for (int x = 10; x < 20; x++, at += 8) \
                    for (int i = at; i < at + 2; i++, col += 8) \
                        S512_8PIX(row, col, i); \
            } else { \
                at += 80; \
                memcpy(curpal, palette, 32); \
                S512_REBUILD_LLUT(); \
            } \
            palette += 16; \
            memcpy(curpal, palette, 32); \
            S512_REBUILD_LLUT(); \
        } \
    } while(0)

    switch (lut9_bpp) {
        case 32: S512_RENDER_LOOP(u32, lut9.p32); break;
        case 16: S512_RENDER_LOOP(u16, lut9.p16); break;
        case 8:  S512_RENDER_LOOP(u8,  lut9.p8);  break;
    }

    /* Build tight dirty rects from per-line X ranges */
    {
        dirty_len = 0;
        s16 ry = -1;
        for (int l = 0; l <= 200 && dirty_len < 254; l++) {
            u8 d = (l < 200) ? (fls_dirty_lines[l] | fls_prev_dirty[l]) : 0;
            if (d) {
                u8 x1, x2;
                if (fls_dirty_lines[l] && fls_prev_dirty[l]) {
                    x1 = fls_dirty_x1[l] < fls_prev_x1[l] ? fls_dirty_x1[l] : fls_prev_x1[l];
                    x2 = fls_dirty_x2[l] > fls_prev_x2[l] ? fls_dirty_x2[l] : fls_prev_x2[l];
                } else if (fls_dirty_lines[l]) {
                    x1 = fls_dirty_x1[l]; x2 = fls_dirty_x2[l];
                } else {
                    x1 = fls_prev_x1[l]; x2 = fls_prev_x2[l];
                }
                s16 px1 = (x1 / 8) * 16;
                s16 px2 = ((x2 / 8) + 1) * 16 - 1;
                if (px2 >= (s16)host.pixelbuf.w) px2 = host.pixelbuf.w - 1;
                if (ry >= 0 && l == ry + dirty_rects[dirty_len].h &&
                    px1 <= dirty_rects[dirty_len].x + dirty_rects[dirty_len].w &&
                    px2 >= dirty_rects[dirty_len].x - 1) {
                    s16 ox1 = dirty_rects[dirty_len].x;
                    s16 ox2 = ox1 + dirty_rects[dirty_len].w - 1;
                    if (px1 < ox1) ox1 = px1;
                    if (px2 > ox2) ox2 = px2;
                    dirty_rects[dirty_len].x = ox1;
                    dirty_rects[dirty_len].w = ox2 - ox1 + 1;
                    dirty_rects[dirty_len].h++;
                } else {
                    if (ry >= 0) dirty_len++;
                    ry = l;
                    dirty_rects[dirty_len] = (SDL_Rect){ .x = px1, .y = l, .w = px2 - px1 + 1, .h = 1 };
                }
            } else if (ry >= 0) {
                dirty_len++;
                ry = -1;
            }
        }
        if (ry >= 0) dirty_len++;
        if (dirty_len == 0) {
            dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
            dirty_len = 1;
        }
        memcpy(fls_prev_dirty, fls_dirty_lines, 200);
        memcpy(fls_prev_x1, fls_dirty_x1, 200);
        memcpy(fls_prev_x2, fls_dirty_x2, 200);
    }

    #undef S512_RENDER_LOOP
    #undef S512_REBUILD_LLUT
    #undef S512_8PIX
    #undef S512_PIX
    #undef BP4_IDX
    #undef ST_PAL9
}

void sys_render(pixelbuf_t buffer) {

#if !defined(__TOS__) && !defined(__atarist__)
    // Desktop: convert full palette to native surface pixel format.
    // Uses a separate array so buffer.palette (internal format) is never modified.
    {
        u8 r, g, b;
        for (int i = 0; i < 256; i++) {
            PAL_READ_RGB(buffer.palette, i, r, g, b);
            surface_palette[i] = SDL_MapRGB(surface->format, r, g, b);
        }
    }
#endif

    if (dirty_mouse)
    {
        dirty_mouse = false;
        sys_dirty_mouse();
    }

    if (bfilm.type == eAlisVideoHAM6)
    {
        u8 *bitmap = vgalogic_df + 0xa0;
        u8 (*ham_pal)[3] = bfilm.ham_pal;
        u32 planesize = (buffer.w * buffer.h) >> 3;

        if (lut9_bpp == 8 && bfilm.frame == 1) {
            SDL_SetPalette(surface, SDL_PHYSPAL, hicolor8_palette, 0, 256);
            if (display != surface)
                SDL_SetPalette(display, SDL_PHYSPAL, hicolor8_palette, 0, 256);
        }

        u8 *c0 = bitmap;
        u8 *c1 = c0 + planesize;
        u8 *c2 = c1 + planesize;
        u8 *c3 = c2 + planesize;
        u8 *c4 = c3 + planesize;
        u8 *c5 = c4 + planesize;

        // Unrolled bitplane extract for 6 planes: reads bytes once, constant shifts.
        // Produces index (4-bit, planes 0-3) and control (2-bit, planes 4-5).
        #define HAM_EXTRACT(b0,b1,b2,b3,b4,b5, bit, idx, ctrl) \
            idx  = ((b0>>bit)&1) | (((b1>>bit)&1)<<1) | (((b2>>bit)&1)<<2) | (((b3>>bit)&1)<<3); \
            ctrl = ((b4>>bit)&1) | (((b5>>bit)&1)<<1);

        // HAM decode: update r,g,b from index+control
        #define HAM_DECODE(idx, ctrl) \
            if (ctrl == 0) { r = ham_pal[idx][0]; g = ham_pal[idx][1]; b = ham_pal[idx][2]; } \
            else { u8 _v = (u8)(idx << 4); \
                   if (ctrl == 1) b = _v; else if (ctrl == 2) r = _v; else g = _v; }

        // Full loop macro — one per bpp, zero per-pixel branching on pixel format
        #define HAM_RENDER_LOOP(type, WRITE_PIX) do { \
            u8 r = 0, g = 0, b = 0; \
            s32 idx, ctrl; \
            for (s32 y = 0; y < buffer.h; y++) { \
                type *row = (type *)((u8 *)surface->pixels + y * surface->pitch); \
                r = g = b = 0; \
                for (s32 x = 0; x < buffer.w; x += 8, c0++, c1++, c2++, c3++, c4++, c5++) { \
                    u8 p0=*c0, p1=*c1, p2=*c2, p3=*c3, p4=*c4, p5=*c5; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 7, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+0] = WRITE_PIX; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 6, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+1] = WRITE_PIX; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 5, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+2] = WRITE_PIX; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 4, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+3] = WRITE_PIX; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 3, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+4] = WRITE_PIX; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 2, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+5] = WRITE_PIX; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 1, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+6] = WRITE_PIX; \
                    HAM_EXTRACT(p0,p1,p2,p3,p4,p5, 0, idx, ctrl) HAM_DECODE(idx,ctrl) row[x+7] = WRITE_PIX; \
                } \
            } \
        } while(0)

#if defined(__TOS__) || defined(__atarist__)
        #define HAM_PIX32 (((u32)r << 16) | ((u32)g << 8) | b)
#else
        #define HAM_PIX32 (((u32)b << 24) | ((u32)g << 16) | ((u32)r << 8))
#endif
        #define HAM_PIX16 ((u16)(((r>>3)<<11)|((g>>2)<<5)|(b>>3)))
        #define HAM_PIX8  ((u8)(((r>>6)<<6)|((g>>5)<<3)|(b>>5)))

        switch (lut9_bpp) {
            case 32: HAM_RENDER_LOOP(u32, HAM_PIX32); break;
            case 16: HAM_RENDER_LOOP(u16, HAM_PIX16); break;
            case 8:  HAM_RENDER_LOOP(u8,  HAM_PIX8);  break;
        }

        #undef HAM_RENDER_LOOP
        #undef HAM_EXTRACT
        #undef HAM_DECODE
        #undef HAM_PIX32
        #undef HAM_PIX16
        #undef HAM_PIX8

        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 1;
    }
    else if (bfilm.type == eAlisVideoS512)
    {
        render_s512(buffer);
    }
    else
    {
        u8 *data = buffer.data;
        if (bfilm.type == eAlisVideoFLIC)
        {
            data = vgalogic;
            
            if (image.ftopal)
                dirty_pal = 1;
        }
        else if (bfilm.type == eAlisVideoCleanup)
        {
            data = vgalogic;
            
            if (image.ftopal)
                dirty_pal = 1;

            bfilm.type = eAlisVideoNone;
        }
        
        if (dirty_len == 0 && dirty_pal == 0)
            return;

        if (surface->format->BitsPerPixel == 8)
        {
            if (dirty_pal)
            {
                SDL_SetPalette(surface, SDL_PHYSPAL, (SDL_Color *)buffer.palette, 0, 256);
                if (display != surface)
                    SDL_SetPalette(display, SDL_PHYSPAL, (SDL_Color *)buffer.palette, 0, 256);
                dirty_pal = 0;
            }

            surface->pixels = data - surface_y_offset * surface_stride;
        }
        else
        {
            if (dirty_pal)
            {
                dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                dirty_len = 1;
                dirty_pal = 0;
           }

            // 4x unrolled scanline blit: palette lookup with reduced loop overhead
            // Advances dst/src by w bytes after the row
            #define BLIT_ROW_32(dst, src, pal, w) do { \
                u8 *_s4 = (src) + ((w) & ~3); \
                u8 *_se = (src) + (w); \
                for (; (src) < _s4; (src) += 4, (dst) += 4) { \
                    (dst)[0] = (pal)[(src)[0]]; (dst)[1] = (pal)[(src)[1]]; \
                    (dst)[2] = (pal)[(src)[2]]; (dst)[3] = (pal)[(src)[3]]; } \
                for (; (src) < _se; (src)++, (dst)++) \
                    *(dst) = (pal)[*(src)]; \
            } while(0)

            #define BLIT_ROW_16(dst, src, pal, w) do { \
                u8 *_s4 = (src) + ((w) & ~3); \
                u8 *_se = (src) + (w); \
                for (; (src) < _s4; (src) += 4, (dst) += 4) { \
                    (dst)[0] = (pal)[(src)[0]]; (dst)[1] = (pal)[(src)[1]]; \
                    (dst)[2] = (pal)[(src)[2]]; (dst)[3] = (pal)[(src)[3]]; } \
                for (; (src) < _se; (src)++, (dst)++) \
                    *(dst) = (pal)[*(src)]; \
            } while(0)

            for (int i = 0; i < dirty_len; i++)
            {
                SDL_Rect r = dirty_rects[i];
                u8 *src = data + r.x + r.y * buffer.w;
                s32 src_skip = buffer.w - r.w;
                s32 dst_skip = surface_stride - r.w;
                s16 y_end;

                switch (surface->format->BitsPerPixel)
                {
                    case 32:
                    {
#if !defined(__TOS__) && !defined(__atarist__)
                        u32 *pal_base = surface_palette;
#else
                        u32 *pal_base = (u32 *)buffer.palette;
#endif
                        u32 *pal32;
                        u32 *dst = (u32 *)surface->pixels + r.x + (r.y + surface_y_offset) * surface_stride;
                        if (image.flinepal)
                        {
                            int stride = 2 + (sizeof(u8 *) >> 1);
                            s16 *pal_cur = image.firstpal;
                            s16 *pal_nxt = pal_cur + stride;
                            s16 y_start = r.y;

                            while (pal_nxt[0] != 0xff && r.y >= pal_nxt[1])
                            {
                                pal_cur = pal_nxt;
                                pal_nxt = pal_cur + stride;
                            }

                            while (y_start < r.y + r.h)
                            {
                                y_end = r.y + r.h;
                                if (pal_nxt[0] != 0xff && pal_nxt[1] < y_end)
                                    y_end = pal_nxt[1];

                                pal32 = pal_base + pal_cur[2];
                                for (int y = y_start; y < y_end; y++, dst += dst_skip, src += src_skip)
                                    BLIT_ROW_32(dst, src, pal32, r.w);

                                y_start = y_end;
                                if (pal_nxt[0] != 0xff) {
                                    pal_cur = pal_nxt;
                                    pal_nxt = pal_cur + stride;
                                }
                            }
                        }
                        else
                        {
                            for (int y = r.y; y < r.y + r.h; y++, dst += dst_skip, src += src_skip)
                                BLIT_ROW_32(dst, src, pal_base, r.w);
                        }

                        break;
                    }
                    case 16:
                    {
                        // Palette is stored as u16 RGB565 at u16 stride (via sColor565 writes)
                        u16 *pal_base = (u16 *)buffer.palette;
                        u16 *pal16;
                        u16 *dst = (u16 *)surface->pixels + r.x + (r.y + surface_y_offset) * surface_stride;
                        if (image.flinepal)
                        {
                            int stride = 2 + (sizeof(u8 *) >> 1);
                            s16 *pal_cur = image.firstpal;
                            s16 *pal_nxt = pal_cur + stride;
                            s16 y_start = r.y;

                            while (pal_nxt[0] != 0xff && r.y >= pal_nxt[1]) {
                                pal_cur = pal_nxt;
                                pal_nxt = pal_cur + stride;
                            }

                            while (y_start < r.y + r.h)
                            {
                                y_end = r.y + r.h;
                                if (pal_nxt[0] != 0xff && pal_nxt[1] < y_end)
                                    y_end = pal_nxt[1];

                                pal16 = pal_base + pal_cur[2];
                                for (int y = y_start; y < y_end; y++, dst += dst_skip, src += src_skip)
                                    BLIT_ROW_16(dst, src, pal16, r.w);

                                y_start = y_end;
                                if (pal_nxt[0] != 0xff) {
                                    pal_cur = pal_nxt;
                                    pal_nxt = pal_cur + stride;
                                }
                            }
                        }
                        else
                        {
                            for (int y = r.y; y < r.y + r.h; y++, dst += dst_skip, src += src_skip)
                                BLIT_ROW_16(dst, src, pal_base, r.w);
                        }

                        break;
                    }
                };
            }

            #undef BLIT_ROW_32
            #undef BLIT_ROW_16
        }

        // Cursor drawn straight into the surface: its indices (0..15) refer to
        // the base palette, not the flinepal slice.
        if (mouse.enabled && dirty_mouse_rect.x < host.pixelbuf.w && dirty_mouse_rect.y < host.pixelbuf.h)
        {
            s16 mx = dirty_mouse_rect.x;
            s16 my = dirty_mouse_rect.y;
            s16 mw = mx + 16 > host.pixelbuf.w ? host.pixelbuf.w - mx : 16;
            s16 mh = my + 16 > host.pixelbuf.h ? host.pixelbuf.h - my : 16;
            u8 *msrc = mouse_pixels;

            if (surface->format->BitsPerPixel == 32)
            {
#if !defined(__TOS__) && !defined(__atarist__)
                u32 *pal_base = surface_palette;
#else
                u32 *pal_base = (u32 *)buffer.palette;
#endif
                u32 *mdst = (u32 *)surface->pixels + mx + (my + surface_y_offset) * surface_stride;
                for (int y = 0; y < mh; y++, mdst += surface_stride, msrc += 16)
                    for (int x = 0; x < mw; x++)
                        if (msrc[x]) mdst[x] = pal_base[msrc[x]];
            }
            else if (surface->format->BitsPerPixel == 16)
            {
                u16 *pal_base = (u16 *)buffer.palette;
                u16 *mdst = (u16 *)surface->pixels + mx + (my + surface_y_offset) * surface_stride;
                for (int y = 0; y < mh; y++, mdst += surface_stride, msrc += 16)
                    for (int x = 0; x < mw; x++)
                        if (msrc[x]) mdst[x] = pal_base[msrc[x]];
            }
            else if (surface->format->BitsPerPixel == 8)
            {
                // 8bpp: surface->pixels aliases buffer.data, so cursor
                // writes corrupt the chunky buffer for the next frame.
                // Save background here, restore after SDL_UpdateRects.
                u8 *bgr = mouse_bg_pixels;
                u8 *mdst = (u8 *)surface->pixels + mx + (my + surface_y_offset) * surface_stride;
                for (int y = 0; y < mh; y++, mdst += surface_stride, msrc += 16, bgr += mw)
                {
                    memcpy(bgr, mdst, mw);
                    for (int x = 0; x < mw; x++)
                        if (msrc[x]) mdst[x] = msrc[x];
                }
            }
        }
    }

    if (dirty_len == 0xff)
        dirty_len = 1;

    // Page flip: the back page was shown two presents ago, so also hand SDL the
    // previous frame's rects. Capture them before the merge pass (which rewrites
    // dirty_rects) and only on presented frames.
    if (use_flip)
    {
        static SDL_Rect prev_rects[256];
        static int      prev_len = 0;
        int             cur_len  = dirty_len;

        if (cur_len + prev_len > 254)
        {
            // No room to carry both frames -- repaint the whole screen, and
            // record that as the previous frame so the next one is correct too.
            dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
            dirty_len = 1;
            prev_rects[0] = dirty_rects[0];
            prev_len = 1;
        }
        else
        {
            // Appending leaves the first cur_len entries untouched, so they can
            // be saved as this frame's list afterwards.
            for (int i = 0; i < prev_len; i++)
                dirty_rects[dirty_len++] = prev_rects[i];

            memcpy(prev_rects, dirty_rects, (size_t)cur_len * sizeof(SDL_Rect));
            prev_len = cur_len;
        }
    }

#if defined(ALIS_DIRTY_MERGE)
    // Merge touching/overlapping rects until stable: SDL's Atari drivers c2p
    // each rect separately, and fewer, larger rects favour the full-row path.
    if (dirty_len > 1) {
        int changed = 1;
        while (changed) {
            changed = 0;
            for (int __i = 0; __i < (int)dirty_len; __i++) {
                SDL_Rect *__a = &dirty_rects[__i];
                int __ax2 = __a->x + __a->w;
                int __ay2 = __a->y + __a->h;
                for (int __j = __i + 1; __j < (int)dirty_len; __j++) {
                    SDL_Rect *__b = &dirty_rects[__j];
                    int __bx2 = __b->x + __b->w;
                    int __by2 = __b->y + __b->h;
                    // Touch/overlap: left edge of one within 1 of right edge
                    // of the other on both axes.
                    if (__a->x <= __bx2 && __b->x <= __ax2 &&
                        __a->y <= __by2 && __b->y <= __ay2)
                    {
                        // Union bbox into rect __i, remove rect __j.
                        int __ux  = (__a->x < __b->x) ? __a->x : __b->x;
                        int __uy  = (__a->y < __b->y) ? __a->y : __b->y;
                        int __ux2 = (__ax2 > __bx2) ? __ax2 : __bx2;
                        int __uy2 = (__ay2 > __by2) ? __ay2 : __by2;
                        __a->x = __ux;
                        __a->y = __uy;
                        __a->w = __ux2 - __ux;
                        __a->h = __uy2 - __uy;
                        __ax2 = __ux2;
                        __ay2 = __uy2;
                        // Swap last rect into __j's slot and shrink.
                        dirty_rects[__j] = dirty_rects[dirty_len - 1];
                        dirty_len--;
                        __j--;
                        changed = 1;
                    }
                }
            }
        }
    }
#endif

    if (display != surface) {
        // Stretch each dirty rect from back buffer to scaled display.
        SDL_Rect scaled_rects[256];
        for (int i = 0; i < dirty_len; i++) {
            SDL_Rect dst;
            dst.x = (Sint16)((s32)dirty_rects[i].x * display_sx_num / display_sx_den);
            dst.y = (Sint16)((s32)dirty_rects[i].y * display_sy_num / display_sy_den);
            dst.w = (Uint16)((s32)dirty_rects[i].w * display_sx_num / display_sx_den);
            dst.h = (Uint16)((s32)dirty_rects[i].h * display_sy_num / display_sy_den);
            SDL_SoftStretch(surface, &dirty_rects[i], display, &dst);
            scaled_rects[i] = dst;
        }
        SDL_UpdateRects(display, dirty_len, scaled_rects);
    } else {
        SDL_UpdateRects(surface, dirty_len, dirty_rects);
    }
    dirty_len = 0;

    // 8bpp only: restore the saved background under the cursor so
    // next frame's drawing into buffer.data sees pristine pixels.
    if (mouse.enabled && surface->format->BitsPerPixel == 8 && dirty_mouse_rect.x < host.pixelbuf.w && dirty_mouse_rect.y < host.pixelbuf.h)
    {
        s16 mx = dirty_mouse_rect.x;
        s16 my = dirty_mouse_rect.y;
        s16 mw = mx + 16 > host.pixelbuf.w ? host.pixelbuf.w - mx : 16;
        s16 mh = my + 16 > host.pixelbuf.h ? host.pixelbuf.h - my : 16;
        u8 *bgr = mouse_bg_pixels;
        u8 *mdst = (u8 *)surface->pixels + mx + (my + surface_y_offset) * surface_stride;
        for (int y = 0; y < mh; y++, mdst += surface_stride, bgr += mw)
            memcpy(mdst, bgr, mw);
    }
}

// ============================================================================
#pragma mark - SDL1 System deinit
// ============================================================================

void sys_restore_desktop(void) { }   // GEM/desktop handover: native Atari backend only

void sys_deinit(void) {

    {
        extern SDL_Joystick *sys_joy_handle;
        if (sys_joy_handle) { SDL_JoystickClose(sys_joy_handle); sys_joy_handle = NULL; }
    }

    // Stop the script-clock timer first so its callback can't fire
    // after we've torn down audio / video state.
#if defined(__atarist__) || defined(__TOS__)
    atari_timerc_uninstall();
#else
    if (timer_id) {
        SDL_RemoveTimer(timer_id);
        timer_id = NULL;
    }
#endif

    SDL_Delay(20);   // 20ms fail-safe delay to make sure that sound buffer is empty

    // NOTE: make sure cursor is visible when quiting
    SDL_ShowCursor(1);

#if defined(ALIS_DSP_MIXER) && (defined(__atarist__) || defined(__TOS__))
    // Stop the Timer A feed FIRST so the ISR can't poke a half-torn-down
    // DSP, then stop DSP channels and release Locksnd/Dsp_Lock so the next
    // program gets a clean audio matrix. Safe no-ops if init never ran.
#ifdef ALIS_DSP_USE_TRACKER
    if (atari_timera_installed)
        Supexec(atari_timera_uninstall_supexec);
#endif
    if (atari_use_dma_backend())
        atari_dma_sound_close();   // stop DMA + free ST-RAM ring (no-op if unused)
    else if (atari_own_audio())
        dsp_mixer_close();         // Falcon DSP. Clones/ST: SDL_Quit closes SDL audio.
#endif
    sys_deinit_psg();

    free(audio_spec);

    surface->pixels = prev_pixels;
    SDL_FreeSurface(surface);  // back buffer (or aliased display)

    SDL_Quit();

#if defined(__atarist__) || defined(__TOS__)
    // Mark cleanup complete so atari_etv_term_handler (etv_term) becomes
    // a no-op when MiNT walks p_term during process termination.
    atari_cleanup_done = 1;
#endif
}

// Integrated-cursor hooks (used by the native 16bpp backend). SDL draws the cursor
// in its own sys_render overlay, so these are no-ops here.
void sys_mouse_erase(void)  {}
void sys_mouse_uncopy(void) {}
void sys_mouse_draw(void)   {}
void sys_flip_wait(void)    {}

void sys_dirty_mouse(void) {

    SDL_ShowCursor(0);
    if (alis.desmouse == NULL)
    {
        return;
    }
    
    u8 type = alis.desmouse[0];
    u16 width = read16(alis.desmouse + 2) + 1;
    u16 height = read16(alis.desmouse + 4) + 1;

    switch (type)
    {
        case 0x00:
        case 0x02:
        {
            s8 palidx = 0;
            if (image.flinepal)
            {
                s16 *palentry = image.firstpal;

                for (int i = 0; i < 3; i++)
                {
                    s16 unk1 = palentry[0];
                    if (unk1 == 0xff)
                        break;

                    palidx = palentry[2];
                    palentry += 2 + (sizeof(u8 *) >> 1);
                }
            }
            
            // ST image
            
            u8 color;
            u8 clear = type == 0 ? 0 : -1;
            
            u8 *at = alis.desmouse + 6;

            s32 px = 0;
            for (s32 h = 0; h < height; h++)
            {
                for (s32 w = 0; w < width; w++, px++)
                {
                    s16 wh = w / 2;
                    color = *(at + wh + h * (width / 2));
                    color = w % 2 == 0 ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                    mouse_pixels[px] = color == clear ? 0 : color + palidx;
                }
            }
            
            break;
        }
            
        case 0x10:
        case 0x12:
        {
            // 4 bit image
            u8 palidx = alis.desmouse[6];
            u8 clear = alis.desmouse[0] == 0x10 ? alis.desmouse[7] : -1;
            u8 *at = alis.desmouse + 8;
            u8 color;

            s32 px = 0;
            for (s32 h = 0; h < width; h++)
            {
                for (s32 w = 0; w < height; w++, px++)
                {
                    s16 wh = w / 2;
                    color = *(at + wh + h * (width / 2));
                    color = w % 2 == 0 ? ((color & 0b11110000) >> 4) : (color & 0b00001111);
                    mouse_pixels[px] = color == clear ? 0 : color + palidx;
                }
            }
            break;
        }

            
        case 0x14:
        case 0x16:
        {
            // 8 bit image
            
            u8 color;

            // Ishar 1 & 3
            u8 clear = 0;

            if (alis.platform.uid == EGameIshar_2)
            {
                // Ishar 2
                clear = alis.desmouse[0] == 0x14 ? alis.desmouse[7] : -1;
            }
            
            u8 *at = alis.desmouse + 8;

            for (int px = 0; px < width * height; px++)
            {
                color = at[px];
                mouse_pixels[px] = color == clear ? 0 : color;
            }

            break;
        }
            
        default:
        {
            break;
        }
    };
}

// =============================================================================
#pragma mark - SDL1 SYNC
// =============================================================================

void sys_lock_renderer(void) {
}

void sys_unlock_renderer(void) {
}

#endif
