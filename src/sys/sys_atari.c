//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "config.h"

#if defined(ALIS_USE_NATIVE_ATARI)
#if !defined(ALIS_DSP_MIXER)
#error "native Atari build requires ALIS_DSP_MIXER (ISRs, audio, VBL cursor)"
#endif

#include <mint/osbind.h>
#include <mint/ostruct.h>
#include <mint/falcon.h>
#include <mint/cookie.h>
#include <gem.h>
#include <SDL/SDL.h>    // types only; native links no SDL

#include "sys.h"
#include "alis.h"
#include "audio.h"
#include "dsp_mixer.h"
#include "sys_atari_dma_sound.h"
#include "channel.h"
#include "image.h"
#include "mem.h"
#include "platform.h"
#include "utils.h"
#include "video.h"


#include "emu2149.h"
#include "math.h"

// ============================================================================
#pragma mark - Debug log
// ============================================================================

static FILE *logfile = NULL;

static void logopen(void)
{
#if defined(ALIS_NO_LOG) && ALIS_NO_LOG
    // dbglog fflushes every call (synchronous GEMDOS write): stalls CPU and audio ISRs.
    logfile = NULL;
#else
    // Use absolute path — CWD may not be writable or may differ from expectation
    logfile = fopen("C:\\alis_dbg.log", "w");
    if (!logfile)
        logfile = fopen("alis_dbg.log", "w");
    if (!logfile)
        logfile = fopen("A:\\alis_dbg.log", "w");
#endif
}

static void logclose(void)
{
    if (logfile) { fclose(logfile); logfile = NULL; }
}

void __attribute__((format(printf,1,2))) dbglog(const char *fmt, ...)
{
    if (!logfile) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(logfile, fmt, ap);
    va_end(ap);
    fflush(logfile);
}

// ============================================================================
#pragma mark - Constants and types
// ============================================================================

#define HZ200_TIMER     ((volatile u32 *)0x4BA)

// Falcon BLiTTER on/off. Defined here (not at the blitter block far below) because
// detect_machine() folds it into g_hw_blitter, which the planar call sites gate on.
#ifndef ALIS_USE_BLITTER
#define ALIS_USE_BLITTER 1
#endif

// Monitor types (from VgetMonitor)
#define MONITOR_MONO    0
#define MONITOR_TV      1
#define MONITOR_VGA     2
#define MONITOR_RGB     3

// Machine types (_MCH cookie >> 16)
#define MCH_ST          0
#define MCH_STE         1
#define MCH_TT          2
#define MCH_FALCON      3

const u32               k_event_ticks = (1000 / 120);
const u32               k_frame_ticks = (1000 / 50);

// SDL-typed globals shared with sys.c
extern SDL_keysym       button;
extern SDL_Event        event;
extern SDL_AudioSpec    *audio_spec;
extern SDL_Rect         dirty_rects[256];
extern SDL_Rect         dirty_mouse_rect;

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

volatile u8 dirty_pal = 1;   // written from Timer-C ISR context (itroutine fade steps)
u8 dirty_len = 0;

static u32 start_time = 0;   // for auto-exit timeout
static u32 render_calls = 0;
static u32 poll_calls = 0;

// ============================================================================
#pragma mark - Video state
// ============================================================================

static u8       old_conterm;         // saved conterm (keyclick) state
static s16      gem_ap_id = -1;     // GEM application ID
static s16      gem_vdi_handle = 0; // VDI workstation handle
static GRECT    gem_desk;           // Desktop rectangle
static int      gem_locked = 0;     // Whether we locked GEM
// Desktop palette is saved/restored via the VDI, not the CLUT: restoring only the CLUT leaves the
// VDI's logical palette holding ours (same as SDL-1.2 GEM_CommonSavePalette/RestorePalette).
static u16      vdi_oldpal[256][3]; // VDI intensities, 0..1000 per component
static int      vdi_oldncolors = 0;

static short    gem_scr_w = 0, gem_scr_h = 0;   // full screen, from v_opnvwk




static u16      machine_type;       // MCH_ST, MCH_STE, MCH_TT, MCH_FALCON
static u16      monitor_type;       // MONITOR_VGA, MONITOR_RGB, etc.
static u8       cpu_family;         // _CPU cookie: 0/20/30/40/60

// --audio= backend (0=auto, 1=DSP, 2=DMA). Here only 1 on a Falcon selects the DSP mixer;
// everything else uses DMA sound.
extern int atari_audio_backend;   // sys.c
static u8       video_planes;       // 4 (ST/STE 16 colors) or 8 (TT/Falcon 256 colors)
static u8       is_doubleline;      // TT_LOW: each line written twice

static void     *old_physbase;
// The VIDEL flip writes the logical base ($44E) directly (falcon_setbase_super); it must be restored
// too, or AES repaints the desktop into a freed game buffer.
static void     *old_logbase;
static long     old_vmode;          // Falcon: VsetMode(-1), ST/TT: Getrez()
static u16      old_palette[256*3]; // saved palette (Falcon format: R,G,B per entry)
static u16      old_st_palette[16]; // Falcon: the ST-compatible palette regs too (see restore)

static u8       *screen_mem[2];     // raw allocations
static u8       *screen_buf[2];     // 256-byte aligned screen buffers
static int      back_buf;           // which buffer we're drawing into
static u32      screen_size;        // size of one screen buffer in bytes
static u32      planar_pitch;       // bytes per scanline in planar format
static u32      screen_height;      // actual hw screen height (may be > game height)

// TT (TT_LOW 320x480): the VM composes into these two 320x200 buffers (TT-RAM
// preferred — 32-bit bus, no video-DMA contention), and sys_render expands the
// finished frame line-doubled into the ST-RAM screen_buf[] at publish time.
// NULL on every other machine (the VM then renders straight into screen_buf).
static u8       *compose_mem[2];    // raw allocations
static u8       *compose_buf[2];    // 256-byte aligned compose buffers

// Falcon single-buffer games (logic == physic): the VM draws into this off-screen canvas
// (the idle screen_buf[1]) and sys_render copies the dirty rects to the displayed
// screen_buf[0], so half-drawn frames are never shown. NULL otherwise.
// -DALIS_SINGLE_BUF_CANVAS=0 draws on screen as before.
static u8       *canvas;
#if !defined(ALIS_SINGLE_BUF_CANVAS)
#define ALIS_SINGLE_BUF_CANVAS 1
#endif
#if !defined(ALIS_CANVAS_FULL_PUBLISH)
#define ALIS_CANVAS_FULL_PUBLISH 0
#endif

// Runtime BLiTTER availability (Falcon only). The planar Blitter offloads poke $FFFF8A00
// directly; the TT has no BLiTTER, so they would bus-error there.
int             g_hw_blitter;

#if ALIS_FALCON_16BPP
// 16-bit (BPS16): the game blits directly into a 16-bit Videl framebuffer using
// pal16 (8-bit palette index -> Falcon RGB565). Shared with image_draw_16.c.
u16             pal16[256];
#endif

u8 mouse_pixels[16 * 16] =
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

// Decoded cursor dimensions (mouse_pixels is filled densely, row stride = width).
static u16 cursor_w = 16, cursor_h = 16;
#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
// Per-screen-buffer saved background under the cursor + its position. TWO entries are
// needed: each buffer carries its OWN cursor (from when it was last the front buffer)
// AND, after oldfen() copies physic->logic, a copy of the FRONT buffer's cursor. Both
// must be removed before affiscr composites, or the cursor bakes into the graphics.
static struct {
    u8  drawn;
    s16 x, y, w, h;
    u16 bg[32 * 16];   // 16bpp: pixel value; planar: plane words. 32 rows: the TT
                       // ISR cursor is drawn line-DOUBLED into the 320x480 screen
                       // buffer (2 screen rows per cursor row).
} cursor_save[2];
#endif

// ============================================================================
#pragma mark - Timing helpers
// ============================================================================

// Profiler tick readable in user mode ($4BA faults there): counted by the Timer-C ISR,
// Supexec $4BA fallback before it is installed (or with --no-timerc).
volatile u32 g_prof_isr_ticks = 0;

static u8 atari_timerc_installed;          // tentative decl; defined with the Timer-C state below

static u32  hz200_snap;
static long hz200_read_super(void) { hz200_snap = *HZ200_TIMER; return 0; }
static u32  hz200_read(void) { Supexec(hz200_read_super); return hz200_snap; }

u32 sys_ticks(void) { return hz200_read() * 5; }

void sys_delay(u32 ms)
{
    u32 t0 = sys_ticks();
    while (sys_ticks() - t0 < ms) ;
}

u32 sys_profile_ticks(void)
{
    if (atari_timerc_installed)
        return g_prof_isr_ticks;
    return hz200_read();
}

// ST-RAM allocation (Blitter/VIDEL-reachable; alt-RAM is off on the target). Used by
// both the 16bpp sprite-expansion cache and the planar sprite cache. Mode 0 = ST-RAM.
void *sys_stram_alloc(u32 size) { long p = Mxalloc((long)size, 0); return (p > 0) ? (void *)p : (void *)0; }
void  sys_stram_free(void *p)   { if (p) Mfree((long)p); }
// Largest free contiguous block per RAM type (Mxalloc(-1, mode)). TOS without Mxalloc returns
// an error; those machines are ST-RAM-only, so Malloc(-1) is the right answer.
u32   sys_stram_avail(void)
{
    long p = Mxalloc(-1L, MX_STRAM);
    if (p < 0) p = Malloc(-1L);            // no Mxalloc on this TOS -> all RAM is ST-RAM
    return p > 0 ? (u32)p : 0;
}

u32   sys_fastram_avail(void)
{
    long p = Mxalloc(-1L, MX_TTRAM);
    return p > 0 ? (u32)p : 0;             // negative = no Mxalloc, hence no alt-RAM either
}

#if defined(ALIS_PROFILE_DRAW)
// Draw-phase profiler: always read $4BA via Supexec.
u32 sys_profile_ticks_safe(void) { return hz200_read(); }

// One-shot VIDEL throttle benchmark (ST-RAM only). With the VIDEL displaying 16bpp,
// time writing the same number of words to the video-region ST-RAM back buffer using
// three access granularities. If movem >> move.w the throttle is per-bus-ACCESS
// (arbitration) and burst writes (Blitter / movem asm) beat it; if all ~equal the
// throttle is bandwidth-bound and only fewer pixels help.
static void fill_w(volatile u16 *p, u32 words) {   // move.w : 1 word / access
    u16 *a = (u16 *)p, *end = a + words;
    __asm__ volatile("1: move.w #0,(%0)+\n\t cmpa.l %1,%0\n\t blt.s 1b\n\t"
                     : "+a"(a) : "a"(end) : "memory","cc");
}
static void fill_l(volatile u16 *p, u32 words) {   // move.l : 2 words / access
    u32 *a = (u32 *)p, *end = a + words / 2;
    __asm__ volatile("1: move.l #0,(%0)+\n\t cmpa.l %1,%0\n\t blt.s 1b\n\t"
                     : "+a"(a) : "a"(end) : "memory","cc");
}
static void fill_m(volatile u16 *p, u32 words) {   // movem.l d0-d7 : 16 words / access
    u32 *a = (u32 *)p, *end = a + words / 2;
    __asm__ volatile(
        "moveq #0,%%d0\n\t moveq #0,%%d1\n\t moveq #0,%%d2\n\t moveq #0,%%d3\n\t"
        "moveq #0,%%d4\n\t moveq #0,%%d5\n\t moveq #0,%%d6\n\t moveq #0,%%d7\n\t"
        "1: movem.l %%d0-%%d7,(%0)\n\t lea 32(%0),%0\n\t cmpa.l %1,%0\n\t blt.s 1b\n\t"
        : "+a"(a) : "a"(end)
        : "d0","d1","d2","d3","d4","d5","d6","d7","memory","cc");
}
static u32 read_w(volatile u16 *p, u32 words) {   // sequential move.w reads (add.w (a)+)
    u16 *a = (u16 *)p, *end = a + words; u32 sum;
    __asm__ volatile("moveq #0,%%d0\n\t 1: add.w (%1)+,%%d0\n\t cmpa.l %2,%1\n\t blt.s 1b\n\t move.l %%d0,%0\n\t"
                     : "=d"(sum), "+a"(a) : "a"(end) : "d0","cc","memory");
    return sum;
}
static u32 g_cacr_snap;
static long cacr_read_super(void) { __asm__ volatile("movec %%cacr,%0" : "=d"(g_cacr_snap)); return 0; }
void sys_bench_bus(void)
{
    extern void dbglog(const char *fmt, ...);
    const u32 BUFW = 60000, REP = 4;       // 240000 words / test
    const u32 total = BUFW * REP;
    volatile u16 *buf = (volatile u16 *)image.logic;   // video-region ST-RAM (back buf, not shown)
    u32 t0, tw = 0, tl = 0, tm = 0, tr = 0;
    volatile u32 sink = 0;
    if (!buf) { dbglog("BUSBENCH: image.logic null, skipped\n"); return; }

    Supexec(cacr_read_super);
    t0 = sys_profile_ticks_safe(); for (u32 r = 0; r < REP; r++) fill_w(buf, BUFW); tw = sys_profile_ticks_safe() - t0;
    t0 = sys_profile_ticks_safe(); for (u32 r = 0; r < REP; r++) fill_l(buf, BUFW); tl = sys_profile_ticks_safe() - t0;
    t0 = sys_profile_ticks_safe(); for (u32 r = 0; r < REP; r++) fill_m(buf, BUFW); tm = sys_profile_ticks_safe() - t0;
    t0 = sys_profile_ticks_safe(); for (u32 r = 0; r < REP; r++) sink += read_w(buf, BUFW); tr = sys_profile_ticks_safe() - t0;

    // MB/s*100 for `bytes` moved in `ticks` @200Hz (5ms/tick). bytes = total words * 2.
    const u32 bytes = total * 2;
    #define MBPS100(ticks) ((ticks) ? (u32)(((unsigned long long)bytes * 100000ULL) \
                                             / ((unsigned long long)(ticks) * 5ULL * 1048576ULL)) : 0)
    u32 rw = MBPS100(tw), rl = MBPS100(tl), rm = MBPS100(tm), rr = MBPS100(tr);
    dbglog("BUSBENCH: %u KB/test to video ST-RAM (1 tick = 5ms)\n", bytes >> 10);
    dbglog("  CACR=%08lx  (instr-cache EI=%lu  data-cache ED=%lu)\n",
           (unsigned long)g_cacr_snap, (unsigned long)(g_cacr_snap & 1), (unsigned long)((g_cacr_snap >> 8) & 1));
    dbglog("  move.w  write (1 word/acc) : %u ms  %u.%02u MB/s\n", tw * 5, rw/100, rw%100);
    dbglog("  move.l  write (2 word/acc) : %u ms  %u.%02u MB/s\n", tl * 5, rl/100, rl%100);
    dbglog("  movem.l write (16word/acc) : %u ms  %u.%02u MB/s\n", tm * 5, rm/100, rm%100);
    dbglog("  move.w  READ  (seq)        : %u ms  %u.%02u MB/s\n", tr * 5, rr/100, rr%100);
    if (tm) dbglog("  -> movem write is %u.%ux faster than move.w write\n", tw / tm, (tw * 10 / tm) % 10);
    #undef MBPS100
}
#endif

#if defined(ALIS_DSP_MIXER)
// ---------------------------------------------------------------------------
// Timer-A MFP ISR — steady 50 Hz DSP audio feed.
//
// The DSP SSI ring under/overruns if fed at the irregular frame rate, so feed it
// at a steady cadence. Trampoline in sys_atari_isr.S (vector $134) calls
// atari_timera_c_callback.
// ---------------------------------------------------------------------------
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

// Set while alis_load_state rewrites alis/image/audio: the ISRs must not run on half-loaded state.
static volatile u8 g_isr_paused;
void sys_isr_pause(int on) { g_isr_paused = (u8)on; }

// Runs in supervisor (Timer-A ISR context). Issue the MFP software-EOI and mask
// Timer A (the feed is long + non-reentrant), drop CPU IPL to 5 so the ACIA
// (IPL 6) can still preempt — else mouse/keyboard ACIA overruns during the feed
// → phantom keypresses — run the feed, restore SR, drop any missed tick.
void atari_timera_c_callback(void)
{
    unsigned short saved_sr;

    MFP_ISRA  = (u8)~MFP_TIMERA_BIT;          // EOI: clear in-service
    MFP_IMRA &= (u8)~MFP_TIMERA_BIT;          // mask Timer A (no re-entry)

    __asm__ volatile(
        "move.w %%sr,%0\n\t"
        "move.w %0,%%d0\n\t"
        "andi.w #0xF8FF,%%d0\n\t"
        "ori.w  #0x0500,%%d0\n\t"             // IPL = 5
        "move.w %%d0,%%sr\n\t"
        : "=&d"(saved_sr) : : "d0", "cc", "memory");

    // Route any YM2149 writes made during this ISR (sys_psg_tick effects, mv2_chiprout
    // chip music) to direct hardware access instead of XBIOS Giaccess — see g_psg_isr_context.
    // Save/restore rather than clear, so a nested Timer-C ISR that also touches the YM stays safe.
    extern volatile u8 g_psg_isr_context;
    u8 _psg_saved = g_psg_isr_context;
    g_psg_isr_context = 1;
    if (g_isr_paused)
        ;
    else if (atari_dma_available)
        atari_dma_sound_tick_isr();            // STE/TT: mix one 50 Hz frame into the DMA ring
    else
        dsp_mixer_tick_isr();                  // Falcon: sequencer tick + 8-voice DSP feed
    g_psg_isr_context = _psg_saved;

    __asm__ volatile("move.w %0,%%sr" : : "d"(saved_sr) : "cc", "memory");

    MFP_IPRA  = (u8)~MFP_TIMERA_BIT;          // discard any pending Timer A
    MFP_IMRA |= MFP_TIMERA_BIT;               // re-enable Timer A
}

// ---------------------------------------------------------------------------
// Timer C (etv_timer $400) drives itroutine at ~50 Hz, so game timing and palette
// fades advance independent of render speed. The trampoline in sys_atari_isr.S is
// linked only with ALIS_DSP_MIXER, hence this guard.
// ---------------------------------------------------------------------------
extern void  atari_timerc_trampoline(void);   // sys_atari_isr.S
extern void *atari_old_etv_timer;             // sys_atari_isr.S (.bss)
extern int   atari_no_timerc;                 // sys.c (--no-timerc)

static int          atari_timerc_clock_div = 1;
static volatile u32 atari_timerc_subtick   = 0;
static u8           atari_timerc_installed  = 0;

#if !defined(ALIS_NATIVE_16BPP)
// Write the palette straight to the VIDEL CLUT ($FFFF9800, 256 longs, format 0xRRGG00BB).
// ISR-safe (no XBIOS); like the original `ittimer`/`io_hpalet`, fades stay smooth under load.
#define FALCON_CLUT_RGB(c) (((u32)(c)->r << 24) | ((u32)(c)->g << 16) | (u32)(c)->b)
static void falcon_clut_apply(const u32 *palette)
{
    volatile u32 *clut = (volatile u32 *)0xFFFF9800UL;
#if defined(ALIS_NATIVE_PLANAR)
    extern int g_planar_nplanes;
    if (image.flinepal && g_planar_nplanes <= 4) {
        // clinepal: re-pack the ×64-spaced banks into ×16 contiguous CLUT entries (see set_palette).
        for (int b = 0; b < 4; b++)
            for (int i = 0; i < 16; i++) {
                const sColorARGB *c = (const sColorARGB *)&palette[b * 64 + i];
                clut[b * 16 + i] = FALCON_CLUT_RGB(c);
            }
        return;
    }
#endif
    for (int i = 0; i < 256; i++) {
        const sColorARGB *c = (const sColorARGB *)&palette[i];
        clut[i] = FALCON_CLUT_RGB(c);
    }
}
#endif

// Called from the Timer-C trampoline (which chains TOS's etv_timer) every tick.
// etv_timer runs faster than the game tick, so divide down. The fitroutine guard
// keeps a tick that lands mid-itroutine from re-entering it.
void atari_timerc_c_callback(void)
{
    // Tick for sys_profile_ticks (user mode cannot read $4BA).
    g_prof_isr_ticks++;

    if (++atari_timerc_subtick < (u32)atari_timerc_clock_div)
        return;
    if (g_isr_paused)
        return;
    atari_timerc_subtick = 0;
    // Cursor mover is in the VBL queue, not here (IPL 6 blocks the ACIA).
    if (!image.fitroutine) {
        // itroutine runs the game tick (music/sound triggers) from ISR context — any YM write
        // it makes must go direct, not via XBIOS Giaccess (see g_psg_isr_context).
        extern volatile u8 g_psg_isr_context;
        u8 _psg_saved = g_psg_isr_context;
        g_psg_isr_context = 1;
        itroutine(0, NULL);
        g_psg_isr_context = _psg_saved;
    }
#if !defined(ALIS_NATIVE_16BPP)
    // Apply the palette here in the IRQ (Falcon only — TT/ST/SDL keep the main-loop set_palette).
    // itroutine just set dirty_pal if a fade step landed; push it to the CLUT now and consume it.
    extern volatile u8 dirty_pal;
    if (dirty_pal && machine_type == MCH_FALCON && host.pixelbuf.palette) {
        falcon_clut_apply((const u32 *)host.pixelbuf.palette);
        dirty_pal = 0;
    }
#endif
}

static long atari_timerc_install_super(void)
{
    if (!atari_no_timerc)
        atari_old_etv_timer = (void *)Setexc(0x100, (long)atari_timerc_trampoline);
    return 0;
}
static long atari_timerc_uninstall_super(void)
{
    if (atari_old_etv_timer)
        Setexc(0x100, (long)atari_old_etv_timer);
    return 0;
}

// ---------------------------------------------------------------------------
// VBL-queue cursor mover. The Timer-C chain runs at IPL 6 and blocks the ACIA long
// enough to desync the IKBD; the VBL queue runs at IPL 4, vsync-aligned (as ScummVM).
// TOS keeps its dispatch state in callee-saved regs, so a plain C handler is safe.
// ---------------------------------------------------------------------------
#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
static void atari_mouse_vbl(void);
static void **atari_vbl_slot;

static long atari_vbl_install_super(void)
{
    s16    nvbls = *(volatile s16 *)0x454UL;   // nvbls
    void **queue = *(void ***)0x456UL;         // _vblqueue
    for (s16 i = 0; i < nvbls; i++) {
        if (queue[i] == NULL) {
            atari_vbl_slot = &queue[i];
            queue[i] = (void *)atari_mouse_vbl;
            return 1;
        }
    }
    return 0;                                  // queue full — cursor stays frame-locked
}
static long atari_vbl_remove_super(void)
{
    if (atari_vbl_slot) { *atari_vbl_slot = NULL; atari_vbl_slot = NULL; }
    return 0;
}
#endif

// Game-tick target: 60 Hz for DOS data (PIT), 50 Hz for Atari/Amiga. Set from sys_init's `pl`:
// alis.platform is filled only by the memory pre-flight (skipped by --no-mem-check / unpack).
static int atari_tick_target_hz = 50;

// Count etv_timer ISRs over one 200 Hz second and derive the divider for the tick target.
// Runs with clock_div huge so itroutine isn't called yet.
static void atari_timerc_calibrate(void)
{
    atari_timerc_clock_div = 0x10000;
    atari_timerc_subtick   = 0;

    u32 t0 = hz200_read();
    while (hz200_read() == t0) {}        // align to a 200 Hz edge
    t0 = hz200_read();

    atari_timerc_subtick = 0;
    while (hz200_read() - t0 < 200) {}   // count ISRs over one second
    u32 isr_count = atari_timerc_subtick;

    int target_hz = atari_tick_target_hz;
    int div = ((int)isr_count + target_hz / 2) / target_hz;
    if (div < 1)  div = 1;
    if (div > 64) div = 64;

    atari_timerc_clock_div = div;
    atari_timerc_subtick   = 0;
    // Measured rate, not the target (video.c and music tempo scaling read it).
    sys_timeclock_hz = (u32)isr_count / (u32)div;
    dbglog("Timer C: %d ISRs/s -> divider %d (target %d Hz, timeclock %u Hz)\n",
           (int)isr_count, div, target_hz, (unsigned)sys_timeclock_hz);
}

static void atari_timerc_install(void)
{
    atari_timerc_clock_div = 0x10000;   // huge: don't fire itroutine until calibrated
    atari_timerc_subtick   = 0;
    Supexec(atari_timerc_install_super);
    atari_timerc_installed = 1;
    if (!atari_no_timerc)
        atari_timerc_calibrate();
    dbglog("Timer C installed (itroutine @ ~50Hz), old_etv=%p\n", atari_old_etv_timer);
#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
    dbglog("VBL cursor mover installed=%ld\n", Supexec(atari_vbl_install_super));
#endif
}

static long atari_timera_install_super(void)
{
    MFP_TACR  = 0;                            // stop Timer A while we set up
    atari_old_timera = MFP_TIMERA_VEC;        // save the previous vector
    MFP_TIMERA_VEC = (u32)atari_timera_trampoline;
    MFP_IPRA  = (u8)~MFP_TIMERA_BIT;          // clear pending
    MFP_ISRA  = (u8)~MFP_TIMERA_BIT;          // clear in-service
    MFP_TADR  = 246;                          // 2457600 / (200*246) ≈ 49.95 Hz
    MFP_IERA |= MFP_TIMERA_BIT;               // enable Timer A interrupt
    MFP_IMRA |= MFP_TIMERA_BIT;               // unmask it
    MFP_TACR  = 7;                            // start: prescaler /200
    atari_timera_installed = 1;
    return 0;
}

static long atari_timera_uninstall_super(void)
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

// Fatal-signal cleanup (sys.c signals_handler, user mode): a dangling Timer-C, Timer-A
// or VBL hook would fire into freed memory and instantly re-crash.
void atari_timerc_emergency_uninstall(void)
{
#if defined(ALIS_DSP_MIXER)
    if (atari_timerc_installed)
        Supexec(atari_timerc_uninstall_super);
    if (atari_timera_installed)
        Supexec(atari_timera_uninstall_super);
#endif
#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
    Supexec(atari_vbl_remove_super);
#endif
}

// ============================================================================
#pragma mark - GEM lock/unlock
// ============================================================================

static int gem_init(void)
{
    short work_in[11], work_out[57];

    gem_ap_id = appl_init();
    if (gem_ap_id == -1)
        return 0;  // No AES — running from TOS command line, no lock needed

    // Open virtual VDI workstation
    gem_vdi_handle = graf_handle(&work_out[0], &work_out[1], &work_out[2], &work_out[3]);

    work_in[0] = 1;
    for (int i = 1; i < 10; i++) work_in[i] = 1;
    work_in[10] = 2;
    v_opnvwk(work_in, &gem_vdi_handle, work_out);
    gem_scr_w = (short)(work_out[0] + 1);
    gem_scr_h = (short)(work_out[1] + 1);

    // Read desktop dimensions
    wind_get(DESK, WF_WORKXYWH, &gem_desk.g_x, &gem_desk.g_y, &gem_desk.g_w, &gem_desk.g_h);

    // Save the desktop palette via the VDI. vq_color's 3rd arg 0 = "the values last REQUESTED via
    // vs_color", i.e. the VDI's logical palette, which is what the desktop redraw will use.
    vq_extnd(gem_vdi_handle, 1, work_out);
    {
        int bpp = work_out[4];
        vdi_oldncolors = (bpp > 8) ? 256 : (1 << bpp);
        if (vdi_oldncolors > 256) vdi_oldncolors = 256;
        for (int i = 0; i < vdi_oldncolors; i++) {
            short rgb[3];
            vq_color(gem_vdi_handle, i, 0, rgb);
            vdi_oldpal[i][0] = (u16)rgb[0];
            vdi_oldpal[i][1] = (u16)rgb[1];
            vdi_oldpal[i][2] = (u16)rgb[2];
        }
        dbglog("GEM palette saved: bpp=%d colors=%d (screen %dx%d)\n",
           bpp, vdi_oldncolors, (int)gem_scr_w, (int)gem_scr_h);
    }

    return 1;
}

// Put the desktop palette back. MUST run while the VDI workstation is still open, and AFTER the
// video mode has been restored (SDL1 order: restoreMode -> Vsync -> vs_color loop).
static void gem_restore_palette(void)
{
    if (gem_ap_id == -1 || gem_vdi_handle <= 0)
        return;

    for (int i = 0; i < vdi_oldncolors; i++)
        vs_color(gem_vdi_handle, i, (short *)vdi_oldpal[i]);
}

static void gem_lock_screen(void)
{
    if (gem_ap_id == -1 || gem_locked)
        return;

    // No AES lock (as ScummVM): BEG_MCTRL left no cursor/keys after exit.
    graf_mouse(M_OFF, NULL);
    dbglog("gem_lock: graf_mouse(M_OFF) only (ScummVM-style, no AES lock)\n");

    gem_locked = 1;
}

static void gem_unlock_screen(void)
{
    int rc;

    if (gem_ap_id == -1 || !gem_locked) {
        dbglog("gem_unlock: SKIPPED (ap_id=%d locked=%d) -> no FMD_FINISH, no desktop redraw\n",
               (int)gem_ap_id, gem_locked);
        return;
    }

    // One full-screen redraw broadcast, then the cursor.
    rc = form_dial(FMD_FINISH, 0, 0, 0, 0, 0, 0, gem_scr_w, gem_scr_h);
    dbglog("gem_unlock: form_dial(FMD_FINISH, fullscreen %dx%d) -> %d\n",
           (int)gem_scr_w, (int)gem_scr_h, rc);

    graf_mouse(M_ON, NULL);

    gem_locked = 0;
}



// Hand the screen back to GEM. Called LAST from main(), after all console output: FMD_FINISH
// repaints the desktop and anything printed afterwards would paint over it.
void sys_restore_desktop(void)
{
    dbglog("sys_restore_desktop: ap_id=%d vdi=%d locked=%d desk=%d,%d %dx%d\n",
           (int)gem_ap_id, (int)gem_vdi_handle, gem_locked,
           (int)gem_desk.g_x, (int)gem_desk.g_y, (int)gem_desk.g_w, (int)gem_desk.g_h);

    gem_unlock_screen();

    // Workstation stays open until here: gem_restore_palette() (called from sys_deinit) needs it.
    if (gem_vdi_handle > 0) {
        v_clsvwk(gem_vdi_handle);
        gem_vdi_handle = 0;
    }

    if (gem_ap_id != -1) {
        int rc = appl_exit();
        dbglog("sys_restore_desktop: appl_exit -> %d\n", rc);
        gem_ap_id = -1;
    }

    dbglog("sys_restore_desktop: done\n");
    logclose();          // the real end of the process — see sys_deinit
}

// ============================================================================
#pragma mark - Machine detection
// ============================================================================

static void detect_machine(void)
{
    long cookie_mch = 0;
    long cookie_cpu = 0;

    if (Getcookie(C__MCH, &cookie_mch) != C_FOUND)
        cookie_mch = 0;  // assume ST
    if (Getcookie(C__CPU, &cookie_cpu) != C_FOUND)
        cookie_cpu = 0;  // 68000

    machine_type = (u16)(cookie_mch >> 16);
    cpu_family   = (u8)cookie_cpu;       // 0/20/30/40/60 — sizes the Falcon DMA rate
    // Runtime flag must follow ALIS_USE_BLITTER (planar callers test g_hw_blitter).
    g_hw_blitter = (machine_type == MCH_FALCON) && ALIS_USE_BLITTER;

    switch (machine_type) {
        case MCH_TT:
            monitor_type = MONITOR_RGB;  // TT always RGB
            break;
        case MCH_FALCON:
            monitor_type = VgetMonitor();
            break;
        default:
            monitor_type = MONITOR_RGB;  // ST/STE
            break;
    }
}

static void save_video_state(void)
{
    old_physbase = Physbase();
    old_logbase  = Logbase();

    if (machine_type == MCH_FALCON) {
        old_vmode = VsetMode(-1);
        VgetRGB(0, 256, (u32 *)old_palette);
        // ST-compat desktop modes take colours from the ST palette regs, not the VIDEL CLUT.
        for (int i = 0; i < 16; i++)
            old_st_palette[i] = Setcolor(i, -1);
    } else if (machine_type == MCH_TT) {
        // Not EsetShift: its TT_MED -> TT_LOW switch is buggy (SDL1 ddc3766f); use Setscreen rez.
        old_vmode = Getrez();
        EgetPalette(0, 256, (short *)old_palette);
    } else {
        old_vmode = Getrez();
        for (int i = 0; i < 16; i++)
            ((u16 *)old_palette)[i] = Setcolor(i, -1);
    }
}

static void restore_video_state(void)
{
    // Early bail-out before save_video_state: nothing to restore.
    if (old_physbase == NULL)
        return;

    Vsync();
    if (machine_type == MCH_FALCON) {
        Setscreen(old_logbase, old_physbase, -1);   // BOTH bases: see old_logbase
        VsetMode(old_vmode);
        VsetRGB(0, 256, (u32 *)old_palette);        // VIDEL CLUT
        for (int i = 0; i < 16; i++)                // ST-compat palette regs (see save)
            Setcolor(i, old_st_palette[i]);
    } else if (machine_type == MCH_TT) {
        Setscreen(old_logbase, old_physbase, old_vmode);   // Getrez()-style mode; BOTH bases
        EsetPalette(0, 256, (short *)old_palette);
    } else {
        Setscreen(old_logbase, old_physbase, old_vmode);
        for (int i = 0; i < 16; i++)
            Setcolor(i, ((u16 *)old_palette)[i]);
    }
}

static int alloc_screen_buffers(void)
{
    for (int i = 0; i < 2; i++) {
        screen_mem[i] = (u8 *)Mxalloc(screen_size + 256, MX_STRAM);
        if (!screen_mem[i]) {
            fprintf(stderr, "Cannot allocate %d bytes for screen buffer %d\n", (int)screen_size, i);
            return 0;
        }
        screen_buf[i] = (u8 *)(((u32)screen_mem[i] + 255) & ~255UL);
        memset(screen_buf[i], 0, screen_size);
    }
    // Hardware will display screen_buf[0], so draw into screen_buf[1] first
    back_buf = 1;
    return 1;
}

static void free_screen_buffers(void)
{
    for (int i = 0; i < 2; i++) {
        if (screen_mem[i]) {
            Mfree(screen_mem[i]);
            screen_mem[i] = NULL;
        }
        screen_buf[i] = NULL;
        if (compose_mem[i]) {
            Mfree(compose_mem[i]);
            compose_mem[i] = NULL;
        }
        compose_buf[i] = NULL;
    }
}

// Video geometry only (no allocation, no Videl access); shared with sys_check_memory so its
// ST-RAM sizing matches the allocator. Returns 0 when this build can't run on this machine.
static int compute_video_geometry(void)
{
    // For all modes: planar pitch = (width / 8) * planes = width * planes / 8
    // 8 planes: pitch = width (320 pixels * 8 planes / 8 = 320 bytes)
    // 4 planes: pitch = width / 2 (320 pixels * 4 planes / 8 = 160 bytes)
    if (machine_type == MCH_FALCON) {
        is_doubleline = 0;
#if ALIS_FALCON_16BPP
        video_planes = 16;
        planar_pitch = width * 2;                 // 2 bytes/pixel
        screen_height = (monitor_type == MONITOR_VGA) ? 240 : height;
#else
        video_planes = 8;
        planar_pitch = width;                     // 8 planes: width bytes per line
        // VGA: 320x240, game uses the top 200 lines. RGB/TV: 320x200.
        screen_height = (monitor_type == MONITOR_VGA) ? 240 : height;
#endif
    } else if (machine_type == MCH_TT) {
#if ALIS_FALCON_16BPP
        // The 16bpp build renders 2 bytes/pixel — no TT video mode fits that,
        // and the planar compose buffers would be half the needed size.
        return 0;
#else
        video_planes = 8;
        is_doubleline = 1;
        planar_pitch = width;
        screen_height = 480;                      // TT_LOW is 320x480, we double each line
#endif
    } else {
        // ST/STE: 4 planes, 16 colors
        video_planes = 4;
        is_doubleline = 0;
        planar_pitch = width / 2;                 // 4 planes: 160 bytes per line
        screen_height = height;
    }

    screen_size = planar_pitch * screen_height;
    return 1;
}

// ============================================================================
#pragma mark - Startup memory pre-flight
// ============================================================================
// Size the VM arena from free RAM (as the originals did): the most that fits, between the game's
// floor and its preferred tier. Refuse with a readable message before the video takeover if even
// the floor doesn't fit. Hardware buffers must fit in ST-RAM; the heap (.TTP is -r -a) fits in
// leftover ST-RAM plus fast RAM. Overhead = heap outside the arena (sprite pool, caches, unpack
// buffers); games from Ishar 3 on also play films. Returns the arena size, 0 = refused.
u32 sys_size_arena(const sPlatform *pl, u32 floor, u32 pref)
{
    detect_machine();          // cookie reads only — safe before any takeover

    width  = pl->width;
    height = pl->height;
    if (!compute_video_geometry()) {
        printf("\nThis build cannot run on this machine's video hardware.\n");
        return 0;
    }

    // Hardware buffers, sized by the same code that allocates them:
    //   2 screen buffers (+256 each for the alignment slack alloc_screen_buffers adds)
    //   + the DMA sound ring (a few KB) + slack for TOS's own housekeeping.
    u32 st_need = 2u * (screen_size + 256u) + 16u * 1024u;

    u32 overhead = (pl->version >= 30 ? 1024u : 512u) * 1024u;

    u32 st_free   = sys_stram_avail();
    u32 fast_free = sys_fastram_avail();

    // Whatever ST-RAM the hardware buffers don't need is still usable for the heap.
    u32 heap_pool = fast_free + ((st_free > st_need) ? (st_free - st_need) : 0);

    u32 arena = heap_pool > overhead ? (heap_pool - overhead) & ~0xffffu : 0;
#if defined(ALIS_DEBUG_ARENA_KB)
    if (arena > ALIS_DEBUG_ARENA_KB * 1024u) arena = ALIS_DEBUG_ARENA_KB * 1024u;   // test low tiers
#endif
    if (arena > pref) arena = pref;
    if (st_free >= st_need && arena >= floor)
        return arena;

    u32 total_heap = floor + overhead;

    // Report totals including our program image, so they compare with the free memory the
    // desktop showed before launch (+1 KB for basepage and rounding).
    u32 image      = (u32)(_base->p_tlen + _base->p_dlen + _base->p_blen) + 1024u;
    u32 need_total = image + st_need + total_heap;
    u32 free_total = image + st_free + fast_free;   // == what was free before we loaded

    // name is the game ("Robinson's Requiem"), filled by script_guess_game; desc
    // is the platform ("Atari Falcon"). Prefer the game, fall back to the platform.
    printf("\nNot enough memory to run %s.\n\n", pl->name[0] ? pl->name : (pl->desc[0] ? pl->desc : "this game"));
    printf("       Needs: %5u KB free memory\n", need_total / 1024);
    printf("  This Atari: %5u KB\n\n", free_total / 1024);
    
    if (st_free < st_need) {
        // Totals may look fine; hardware-addressable ST-RAM specifically is short.
        printf("ST-RAM is short: %u KB free, %u KB needed.\n\n", st_free / 1024, st_need / 1024);
    }

    return 0;
}

static int setup_video_mode(void)
{
    if (!compute_video_geometry()) {
#if ALIS_FALCON_16BPP
        if (machine_type == MCH_TT)
            fprintf(stderr, "16bpp build unsupported on TT — use the planar build\n");
#endif
        return 0;
    }

    if (machine_type == MCH_FALCON) {
        u16 mode;
#if ALIS_FALCON_16BPP
        // 16-bit truecolor (linear chunky, no planar c2p). screen_buf[] hold the
        // Videl framebuffer in RGB565; the game renders 8-bit chunky elsewhere.
        mode = (monitor_type == MONITOR_VGA) ? (BPS16 | VERTFLAG) : BPS16;
        mode |= old_vmode & (VGA | PAL);
        if (!alloc_screen_buffers())
            return 0;
        dbglog("VsetMode(BPS16 0x%x) screen_buf[0]=%p\n", (unsigned)mode, (void*)screen_buf[0]);
        Setscreen(-1, screen_buf[0], -1);
        VsetMode(mode);
#else
        // VGA: 320x240 8-bit (BPS8|VERTFLAG), game uses top 200 lines. RGB/TV: 320x200 (BPS8).
        mode = (monitor_type == MONITOR_VGA) ? (BPS8 | VERTFLAG) : BPS8;
        mode |= old_vmode & (VGA | PAL);  // preserve VGA and PAL/NTSC flags

        if (!alloc_screen_buffers())
            return 0;

        dbglog("VsetMode(0x%x) screen_buf[0]=%p\n", (unsigned)mode, (void*)screen_buf[0]);
        Setscreen(-1, screen_buf[0], -1);
        VsetMode(mode);
#endif
    } else if (machine_type == MCH_TT) {
#if !ALIS_FALCON_16BPP
        if (!alloc_screen_buffers())
            return 0;

        // The VM composes off-screen at 320x200. Prefer TT-RAM (32-bit burst
        // reads, no video-DMA contention); Mxalloc mode 3 = MX_PREFTTRAM falls
        // back to ST-RAM by itself on a TT without FastRAM.
        u32 csize = planar_pitch * height;
        for (int i = 0; i < 2; i++) {
            compose_mem[i] = (u8 *)Mxalloc((long)(csize + 256), 3);
            if (!compose_mem[i]) {
                fprintf(stderr, "Cannot allocate %d bytes for compose buffer %d\n", (int)csize, i);
                return 0;
            }
            compose_buf[i] = (u8 *)(((u32)compose_mem[i] + 255) & ~255UL);
            memset(compose_buf[i], 0, csize);
        }
        dbglog("TT compose bufs: %p %p (%lu bytes each)\n",
               (void*)compose_buf[0], (void*)compose_buf[1], (unsigned long)csize);

        Setscreen(-1, screen_buf[0], TT_LOW >> 8);
#endif
    } else {
        // ST/STE: 4 planes, 16 colors
        if (!alloc_screen_buffers())
            return 0;

        Setscreen(-1, screen_buf[0], ST_LOW >> 8);
    }

    return 1;
}

// S512 (ST FLS video) shares the Falcon 16-bit fused surfaces with FLIC — couple the flags so
// enabling the 16-bit film path also gets the fast 16-bit S512 render (no separate build flag).
#if defined(ALIS_FLI_FALCON16) && !defined(ALIS_S512_FALCON16)
#define ALIS_S512_FALCON16 1
#endif

#if defined(ALIS_FLI_FALCON16)
// ---- Falcon 16-bit fused film path -------------------------------------------
// Like the CD original: switch the Videl to BPS16 for the film and let video.c's fused
// decoder (fli_decomp16) LUT-expand 8-bit indices straight to screen. Falcon only.
extern u16 *g_fli16_screen;      // video.c: fused decoder writes RGB565 to this target (NULL=planar)
extern u32  g_fli16_stride;      // video.c: screen row stride in pixels
static u8   *film16_mem   = NULL;
static u16  *film16_front = NULL; // displayed buffer
static u16  *film16_back  = NULL; // off-screen buffer (keyframes decode here, then flip)
static void *film16_base  = NULL; // physical screen base to restore on film end
static long  film16_mode  = -1;   // Videl mode to restore

int sys_film16_begin(u16 w, u16 h)
{
    if (machine_type != MCH_FALCON) return 0;   // Falcon only
    if (g_fli16_screen) return 1;               // already 16-bit (kept across a video sequence)

    // Two full-screen 16-bit surfaces (VGA doubles to 240 lines; the film fills the top h).
    // Front is displayed; keyframes decode into back off-screen then flip -> no visible wipe-in.
    s32 sh = (monitor_type == MONITOR_VGA) ? 240 : height;
    u32 sw = width;                              // 320 — screen row stride (pixels)
    u32 sz = sw * (u32)sh * 2;
    film16_mem = (u8 *)Mxalloc((long)(2 * sz + 256), MX_STRAM);
    if (film16_mem == NULL || (long)film16_mem <= 0) { film16_mem = NULL; return 0; }
    u16 *fb = (u16 *)(((u32)film16_mem + 255) & ~255UL);
    memset(fb, 0, 2 * sz);
    film16_front = fb;
    film16_back  = (u16 *)((u8 *)fb + sz);

    Vsync();
    film16_base = Physbase();
    film16_mode = VsetMode(-1);
    u16 mode = (u16)(BPS16 | (film16_mode & (VGA | PAL)));
    if (monitor_type == MONITOR_VGA) mode |= VERTFLAG;
    Setscreen(-1, film16_front, -1);
    VsetMode(mode);

    g_fli16_stride = sw;
    g_fli16_screen = film16_front;               // delta target = displayed front (arm last)
    dbglog("sys_film16_begin: %ux%u -> BPS16 front=%p back=%p stride=%u (saved mode=0x%lx base=%p)\n",
           (unsigned)w, (unsigned)h, (void*)film16_front, (void*)film16_back, (unsigned)sw, film16_mode, film16_base);
    return 1;
}

// Keyframe (self-contained full frame: BRUN/raw/black): aim the decoder at the OFF-SCREEN
// back buffer so the wipe-in isn't visible...
void sys_film16_keyframe(void)
{
    if (g_fli16_screen) g_fli16_screen = film16_back;
}
// ...then flip to it on vblank and make it the new front. Subsequent deltas modify this
// (now displayed) keyframe in place — deltas can't be double-buffered without a full copy.
void sys_film16_present(void)
{
    if (!g_fli16_screen) return;
    Setscreen(-1, film16_back, -1);              // flip to the freshly-decoded keyframe
    Vsync();                                     // hold until it is actually scanned out
    u16 *t = film16_front; film16_front = film16_back; film16_back = t;   // swap
    g_fli16_screen = film16_front;               // deltas now target the displayed keyframe
}

// Delta frame just decoded in place: hold it one vblank before the next in-place write.
void sys_film16_vsync(void)
{
    if (g_fli16_screen) Vsync();
}

void sys_film16_end(void)
{
    if (!g_fli16_screen) return;
    g_fli16_screen = NULL;                        // disarm the decoder first
    Vsync();
    Setscreen(-1, film16_base, -1);
    VsetMode((u16)film16_mode);
    if (film16_mem) { Mfree((long)film16_mem); film16_mem = NULL; }
    film16_front = film16_back = NULL;
    dbglog("sys_film16_end: restored mode=0x%lx base=%p\n", film16_mode, film16_base);
}
#endif // ALIS_FLI_FALCON16

// VM render targets: the ST-RAM screen buffers (video can only display ST-RAM; the VM's
// malloc lands in alt-RAM). TT: the compose buffers, line-doubled by sys_render.
u8 *sys_get_framebuffer(int index)
{
    if (index != 0 && index != 1)
        return NULL;
    if (compose_buf[index])
        return compose_buf[index];
#if ALIS_NATIVE_PLANAR && ALIS_SINGLE_BUF_CANVAS
    if (index == 0 && machine_type == MCH_FALCON && !alis.platform.dbl_buf)
        return canvas = screen_buf[1];
#endif
    return screen_buf[index];
}

// The buffer the VM renders into as index 0 — for mapping image.physic/logic
// back to a ping-pong slot (cursor_save indexing).
static inline u8 *vm_render_buf0(void)
{
    return compose_buf[0] ? compose_buf[0] : canvas ? canvas : screen_buf[0];
}

static void set_palette(u32 *palette, int ncolors)
{
    if (machine_type == MCH_FALCON) {
#if ALIS_FALCON_16BPP
        // Truecolor: no hardware CLUT — fold the palette into the RGB565 LUT the
        // chunky->16bpp expand uses. (Falcon BPS16 word = rrrrr ggggg g bbbbb.)
        for (int i = 0; i < ncolors; i++) {
            sColorARGB *c = (sColorARGB *)&palette[i];
            pal16[i] = (u16)(((c->r >> 3) << 11) | ((c->g >> 2) << 5) | (c->b >> 3));
        }
#else
        // Falcon VsetRGB expects 0x00RRGGBB per entry
        u32 falcon_pal[256];
#if defined(ALIS_NATIVE_PLANAR)
        extern int g_planar_nplanes;
        if (image.flinepal && g_planar_nplanes <= 4) {
            // clinepal: the per-row bank lives in framebuffer planes 4-5 (×16), so the
            // CLUT must hold each 16-colour bank contiguously at b*16. The game loads the
            // banks ×64-spaced (mpalet[b*64..]) — re-pack them to ×16 here. Only entries
            // 0..63 are indexable (max index = 3*16+15), so upload just those.
            for (int b = 0; b < 4; b++)
                for (int i = 0; i < 16; i++) {
                    sColorARGB *c = (sColorARGB *)&palette[b * 64 + i];
                    falcon_pal[b * 16 + i] = ((u32)c->r << 16) | ((u32)c->g << 8) | c->b;
                }
            VsetRGB(0, 64, falcon_pal);
        } else
#endif
        {
            for (int i = 0; i < ncolors; i++) {
                sColorARGB *c = (sColorARGB *)&palette[i];
                falcon_pal[i] = ((u32)c->r << 16) | ((u32)c->g << 8) | c->b;
            }
            VsetRGB(0, ncolors, falcon_pal);
        }
#endif
    } else if (machine_type == MCH_TT) {
        // TT uses 12-bit palette: 0x0RGB (4 bits per channel)
        u16 tt_pal[256];
        for (int i = 0; i < ncolors; i++) {
            sColorARGB *c = (sColorARGB *)&palette[i];
            tt_pal[i] = ((c->r >> 4) << 8) | ((c->g >> 4) << 4) | (c->b >> 4);
        }
        EsetPalette(0, ncolors, tt_pal);
    } else {
        // ST/STE: 12-bit palette, max 16 colors
        int n = ncolors > 16 ? 16 : ncolors;
        for (int i = 0; i < n; i++) {
            sColorARGB *c = (sColorARGB *)&palette[i];
            u16 stcol = ((c->r >> 4) << 8) | ((c->g >> 4) << 4) | (c->b >> 4);
            Setcolor(i, stcol);
        }
    }
}

#if ALIS_FALCON_16BPP
// Rebuild pal16 from the game palette; called at draw() start when dirty, before the fremap
// re-blit. g_pal16_gen bumps only on a real change: the 16bpp sprite cache re-expands on mismatch.
u32 g_pal16_gen = 1;
void sys_pal16_refresh(void)
{
    u32 *p = (u32 *)host.pixelbuf.palette;
    u8 changed = 0;
    for (int i = 0; i < 256; i++) {
        sColorARGB *c = (sColorARGB *)&p[i];
        u16 v = (u16)(((c->r >> 3) << 11) | ((c->g >> 2) << 5) | (c->b >> 3));
        if (pal16[i] != v) { pal16[i] = v; changed = 1; }
    }
    if (changed) g_pal16_gen++;
}
#endif // ALIS_FALCON_16BPP (pal16 refresh)

// ---------------------------------------------------------------------------
// Falcon BLiTTER ($FFFF8A00) — offload large solid fills. HOG mode runs the blit
// to completion synchronously (the CPU is held off the bus until it finishes).
// The registers are supervisor I/O, so each fill goes through Supexec; that setup
// cost only pays off above a size threshold (the callers in image_draw_16.c gate
// on area). The 030 data cache does NOT snoop the Blitter, and we read
// the framebuffer back (oldfen/tvtofen + cursor bg-save), so sys_blit_frame_sync()
// invalidates the data cache once per frame after composition.
#ifndef ALIS_USE_BLITTER
#define ALIS_USE_BLITTER 1
#endif

#if ALIS_USE_BLITTER
static u32 g_blt_dst;  static u16 g_blt_w, g_blt_h, g_blt_col;  static s16 g_blt_yinc;
static u8  g_blt_used;

static long blit_fill_super(void)
{
    volatile u8  *ctrl = (volatile u8  *)0xFFFF8A3CUL;
    volatile u16 *ht   = (volatile u16 *)0xFFFF8A00UL;
    while (*ctrl & 0x80) ;                          // wait out any in-flight blit
    for (int i = 0; i < 16; i++) ht[i] = g_blt_col; // halftone = solid fill colour
    *(volatile u16 *)0xFFFF8A28UL = 0xFFFF;         // endmask 1 (full word)
    *(volatile u16 *)0xFFFF8A2AUL = 0xFFFF;         // endmask 2
    *(volatile u16 *)0xFFFF8A2CUL = 0xFFFF;         // endmask 3
    *(volatile s16 *)0xFFFF8A2EUL = 2;              // dst x-inc = one 16bpp pixel
    *(volatile s16 *)0xFFFF8A30UL = g_blt_yinc;     // dst y-inc = row wrap
    *(volatile u32 *)0xFFFF8A32UL = g_blt_dst;      // dst address
    *(volatile u16 *)0xFFFF8A36UL = g_blt_w;        // x count (words/row)
    *(volatile u16 *)0xFFFF8A38UL = g_blt_h;        // y count (rows)
    *(volatile u8  *)0xFFFF8A3AUL = 1;              // HOP = halftone only
    *(volatile u8  *)0xFFFF8A3BUL = 3;              // LOP = replace (dst := halftone)
    *(volatile u8  *)0xFFFF8A3DUL = 0;              // skew / FXSR / NFSR = 0
    *ctrl = 0xC0;                                   // BUSY | HOG → run to completion
    return 0;
}

void sys_blit_fill_16(u16 *dst, int w, int h, u16 color)
{
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096)   // sanity: HOG mode hogs the bus
        return;
    g_blt_dst  = (u32)dst;
    g_blt_w    = (u16)w;
    g_blt_h    = (u16)h;
    g_blt_col  = color;
    g_blt_yinc = (s16)(host.pixelbuf.w * 2 - (w - 1) * 2);
    g_blt_used = 1;
    Supexec(blit_fill_super);
}

// Block copy of a 16bpp rectangle src -> framebuffer (HOP=source, LOP=replace).
// src lives in normal (cacheable) ST-RAM — the expanded sprite cache. The 030 data
// cache is write-through (no copyback bit in CACR), so CPU-written cache data is
// already in RAM for the Blitter to read. Used for the opaque background and long
// opaque runs; setup cost gates it to large copies.
static u32 g_blt_src, g_blt_cdst;  static u16 g_blt_cw, g_blt_ch;  static s16 g_blt_syinc, g_blt_dyinc;
static long blit_copy_super(void)
{
    volatile u8 *ctrl = (volatile u8 *)0xFFFF8A3CUL;
    while (*ctrl & 0x80) ;
    *(volatile s16 *)0xFFFF8A20UL = 2;              // src x-inc = one 16bpp pixel
    *(volatile s16 *)0xFFFF8A22UL = g_blt_syinc;    // src y-inc = src row wrap
    *(volatile u32 *)0xFFFF8A24UL = g_blt_src;      // src address
    *(volatile u16 *)0xFFFF8A28UL = 0xFFFF;         // endmasks (full words)
    *(volatile u16 *)0xFFFF8A2AUL = 0xFFFF;
    *(volatile u16 *)0xFFFF8A2CUL = 0xFFFF;
    *(volatile s16 *)0xFFFF8A2EUL = 2;              // dst x-inc
    *(volatile s16 *)0xFFFF8A30UL = g_blt_dyinc;    // dst y-inc = dst row wrap
    *(volatile u32 *)0xFFFF8A32UL = g_blt_cdst;     // dst address
    *(volatile u16 *)0xFFFF8A36UL = g_blt_cw;       // x count (words/row)
    *(volatile u16 *)0xFFFF8A38UL = g_blt_ch;       // y count (rows)
    *(volatile u8  *)0xFFFF8A3AUL = 2;              // HOP = source
    *(volatile u8  *)0xFFFF8A3BUL = 3;              // LOP = replace (dst := src)
    *(volatile u8  *)0xFFFF8A3DUL = 0;              // skew = 0
    *ctrl = 0xC0;                                   // BUSY | HOG
    return 0;
}
void sys_blit_copy_16(u16 *src, int src_pitch_w, u16 *dst, int w, int h)
{
    if (w <= 0 || h <= 0 || w > 4096 || h > 4096)
        return;
    g_blt_src   = (u32)src;
    g_blt_cdst  = (u32)dst;
    g_blt_cw    = (u16)w;
    g_blt_ch    = (u16)h;
    g_blt_syinc = (s16)(src_pitch_w   * 2 - (w - 1) * 2);
    g_blt_dyinc = (s16)(host.pixelbuf.w * 2 - (w - 1) * 2);
    g_blt_used  = 1;
    Supexec(blit_copy_super);
}

// 8-plane INTERLEAVED opaque copy: src (converted planar sprite, ST-RAM) -> framebuffer.
// One blit PER PLANE (8 total) inside a single Supexec. For plane p the words are at
// base + p*2, and consecutive words within a plane are one interleave stride (16 bytes)
// apart → src/dst x-inc = 16. Word-aligned only (skew=0): the caller gates on (px&15)==0.
// Used for the big OPAQUE background sprites (sky + cobblestone/grass strips) which the
// original ST engine kept word-aligned. src is write-through-cached → already in RAM.
static u32 g_pblt_src, g_pblt_dst;  static u16 g_pblt_cw, g_pblt_ch;  static s16 g_pblt_syinc, g_pblt_dyinc;
static int g_pblt_np = 8;
static long blit_copy_planar_super(void)
{
    volatile u8 *ctrl = (volatile u8 *)0xFFFF8A3CUL;
    for (int p = 0; p < g_pblt_np; p++) {
        while (*ctrl & 0x80) ;
        *(volatile s16 *)0xFFFF8A20UL = 16;                 // src x-inc = interleave stride
        *(volatile s16 *)0xFFFF8A22UL = g_pblt_syinc;       // src y-inc = src row wrap
        *(volatile u32 *)0xFFFF8A24UL = g_pblt_src + p * 2; // src address (plane p)
        *(volatile u16 *)0xFFFF8A28UL = 0xFFFF;             // endmasks (full words)
        *(volatile u16 *)0xFFFF8A2AUL = 0xFFFF;
        *(volatile u16 *)0xFFFF8A2CUL = 0xFFFF;
        *(volatile s16 *)0xFFFF8A2EUL = 16;                 // dst x-inc = interleave stride
        *(volatile s16 *)0xFFFF8A30UL = g_pblt_dyinc;       // dst y-inc = dst row wrap
        *(volatile u32 *)0xFFFF8A32UL = g_pblt_dst + p * 2; // dst address (plane p)
        *(volatile u16 *)0xFFFF8A36UL = g_pblt_cw;          // x count (chunks/row)
        *(volatile u16 *)0xFFFF8A38UL = g_pblt_ch;          // y count (rows)
        *(volatile u8  *)0xFFFF8A3AUL = 2;                  // HOP = source
        *(volatile u8  *)0xFFFF8A3BUL = 3;                  // LOP = replace (dst := src)
        *(volatile u8  *)0xFFFF8A3DUL = 0;                  // skew / FXSR / NFSR = 0
        *ctrl = 0xC0;                                       // BUSY | HOG → run to completion
    }
    while (*ctrl & 0x80) ;                                  // wait out the last plane
    return 0;
}
// src/dst_rowbytes = bytes per scanline of each buffer; chunks_w = 16px chunks/row; h = rows.
void sys_blit_copy_planar(u16 *src, int src_rowbytes, u16 *dst, int dst_rowbytes, int chunks_w, int h, int nplanes)
{
    if (chunks_w <= 0 || h <= 0 || chunks_w > 4096 || h > 4096)
        return;
    g_pblt_src   = (u32)src;
    g_pblt_dst   = (u32)dst;
    g_pblt_cw    = (u16)chunks_w;
    g_pblt_ch    = (u16)h;
    g_pblt_np    = (nplanes < 1) ? 1 : (nplanes > 8 ? 8 : nplanes);
    g_pblt_syinc = (s16)(src_rowbytes - (chunks_w - 1) * 16);
    g_pblt_dyinc = (s16)(dst_rowbytes - (chunks_w - 1) * 16);
    g_blt_used   = 1;
    Supexec(blit_copy_planar_super);
}

#if defined(ALIS_NATIVE_PLANAR)
// 8-plane INTERLEAVED solid fill: one Blitter halftone-fill PER PLANE (8 total) in a single
// Supexec. Plane p's halftone is 0xFFFF if colour bit p is set else 0x0000 (HOP=halftone,
// LOP=replace). End-masks em1/em3 trim the left/right boundary chunks to exact pixels;
// em2 = 0xFFFF for the interior. Used for large solid rectangles (draw_boxf).
static u32 g_pflt_dst;  static u16 g_pflt_cw, g_pflt_ch, g_pflt_col, g_pflt_em1, g_pflt_em3;  static s16 g_pflt_yinc;
static long blit_fill_planar_super(void)
{
    volatile u8  *ctrl = (volatile u8  *)0xFFFF8A3CUL;
    volatile u16 *ht   = (volatile u16 *)0xFFFF8A00UL;
    extern int g_planar_nplanes;
    for (int p = 0; p < g_planar_nplanes; p++) {            // skip unused high planes (they stay 0)
        u16 pv = ((g_pflt_col >> p) & 1) ? 0xFFFF : 0x0000;
        while (*ctrl & 0x80) ;
        for (int i = 0; i < 16; i++) ht[i] = pv;            // halftone = plane's solid value
        *(volatile u16 *)0xFFFF8A28UL = g_pflt_em1;         // endmask 1 (left edge)
        *(volatile u16 *)0xFFFF8A2AUL = 0xFFFF;             // endmask 2 (interior)
        *(volatile u16 *)0xFFFF8A2CUL = g_pflt_em3;         // endmask 3 (right edge)
        *(volatile s16 *)0xFFFF8A2EUL = 16;                 // dst x-inc = interleave stride
        *(volatile s16 *)0xFFFF8A30UL = g_pflt_yinc;        // dst y-inc = row wrap
        *(volatile u32 *)0xFFFF8A32UL = g_pflt_dst + p * 2; // dst (plane p)
        *(volatile u16 *)0xFFFF8A36UL = g_pflt_cw;          // x count (chunks/row)
        *(volatile u16 *)0xFFFF8A38UL = g_pflt_ch;          // y count (rows)
        *(volatile u8  *)0xFFFF8A3AUL = 1;                  // HOP = halftone only
        *(volatile u8  *)0xFFFF8A3BUL = 3;                  // LOP = replace
        *(volatile u8  *)0xFFFF8A3DUL = 0;                  // skew = 0
        *ctrl = 0xC0;                                       // BUSY | HOG
    }
    while (*ctrl & 0x80) ;
    return 0;
}
// dst = first chunk (plane 0); dst_rowbytes = framebuffer pitch; chunks_w/h = size; col = 8-bit
// colour; em1/em3 = left/right boundary end-masks (0xFFFF if that edge is chunk-aligned).
void sys_blit_fill_planar(u16 *dst, int dst_rowbytes, int chunks_w, int h, u16 col, u16 em1, u16 em3)
{
    if (chunks_w <= 0 || h <= 0 || chunks_w > 4096 || h > 4096)
        return;
    g_pflt_dst  = (u32)dst;
    g_pflt_cw   = (u16)chunks_w;
    g_pflt_ch   = (u16)h;
    g_pflt_col  = col;
    g_pflt_em1  = em1;
    g_pflt_em3  = em3;
    g_pflt_yinc = (s16)(dst_rowbytes - (chunks_w - 1) * 16);
    g_blt_used  = 1;
    Supexec(blit_fill_planar_super);
}
#endif // ALIS_NATIVE_PLANAR

static long dcache_clear_super(void)
{
    u32 cacr;
    __asm__ volatile("movec %%cacr,%0" : "=d"(cacr));
    cacr |= 0x0800;   // CD: clear (invalidate) the 030 data cache
    __asm__ volatile("movec %0,%%cacr" : : "d"(cacr) : "memory");
    return 0;
}

void sys_blit_frame_sync(void)
{
    if (g_blt_used) {
        Supexec(dcache_clear_super);   // make Blitter writes visible to CPU reads
        g_blt_used = 0;
    }
}
#else
void sys_blit_fill_16(u16 *dst, int w, int h, u16 color) { (void)dst; (void)w; (void)h; (void)color; }
void sys_blit_copy_16(u16 *src, int spw, u16 *dst, int w, int h) { (void)src; (void)spw; (void)dst; (void)w; (void)h; }
void sys_blit_copy_planar(u16 *src, int srb, u16 *dst, int drb, int cw, int h, int nplanes) { (void)src; (void)srb; (void)dst; (void)drb; (void)cw; (void)h; (void)nplanes; }
void sys_blit_fill_planar(u16 *dst, int drb, int cw, int h, u16 col, u16 em1, u16 em3) { (void)dst; (void)drb; (void)cw; (void)h; (void)col; (void)em1; (void)em3; }
void sys_blit_frame_sync(void) {}
#endif // ALIS_USE_BLITTER

// ============================================================================
#pragma mark - SDL audio callback (shared with sys.c)
// ============================================================================

void sys_audio_callback_S16MSB(void *userdata, u8 *stream, s32 len);
void sys_audio_callback_S8(void *userdata, u8 *stream, s32 len);

// ============================================================================
#pragma mark - System init
// ============================================================================

// --- Raw IKBD keyboard (sys_atari_ikbd.S) -----------------------------------------
// Hook our scancode handler into kbdvec (Kbdvbase()-4) so io_inkey/io_shiftkey
// read make/break scancodes directly, bypassing the AES-desynced Bconin path.
extern void atari_ikbd_kbdvec(void);
extern void atari_ikbd_mousevec(void);
extern volatile u8 g_ikbd_pressed[128];
extern volatile u8 g_ikbd_cur;
extern volatile s16 g_mouse_dx, g_mouse_dy;   // IKBD relative deltas (sys_atari_ikbd.S)
extern volatile u8  g_mouse_buttons;          // bit0=right, bit1=left
extern void atari_ikbd_joyvec(void);
static void *ikbd_old_kbdvec;
static void *ikbd_old_mousevec;
static void *ikbd_old_joyvec;

static long ikbd_install_super(void)
{
    _KBDVECS *kv = Kbdvbase();
    ikbd_old_kbdvec     = (void *)((long *)kv)[-1];
    ((long *)kv)[-1]    = (long)atari_ikbd_kbdvec;
    ikbd_old_mousevec   = (void *)kv->mousevec;   // IKBD relative-mouse packets
    kv->mousevec        = (void *)atari_ikbd_mousevec;
    ikbd_old_joyvec     = (void *)kv->joyvec;     // IKBD joystick packets
    kv->joyvec          = (void *)atari_ikbd_joyvec;
    return 0;
}
static long ikbd_uninstall_super(void)
{
    _KBDVECS *kv = Kbdvbase();
    if (ikbd_old_kbdvec)
        ((long *)kv)[-1] = (long)ikbd_old_kbdvec;
    if (ikbd_old_mousevec)
        kv->mousevec = ikbd_old_mousevec;
    if (ikbd_old_joyvec)
        kv->joyvec = ikbd_old_joyvec;
    return 0;
}
void atari_ikbd_init(void)   { Supexec(ikbd_install_super); }
void atari_ikbd_deinit(void) { if (ikbd_old_kbdvec) Supexec(ikbd_uninstall_super); }


// Abnormal-termination cleanup — defined next to sys_deinit, installed at the end of sys_init.
extern void *atari_old_etv_term;          // BSS slot in sys_atari_isr.S
void atari_etv_term_handler(void);
static void atari_etv_term_atexit(void);

void sys_init(sPlatform *pl, int fullscreen, int mutesound) {

    logopen();
    dbglog("=== ALIS native Atari backend ===\n");

    dirty_mouse_rect = (SDL_Rect){0, 0, 16, 16};

    width = pl->width;
    height = pl->height;

    // DOS data ticks at 60 Hz (PIT), Atari/Amiga at 50 Hz; little_endian = DOS data (as sys_sdl1.c).
    // From the 200 Hz Timer C, 60 Hz rounds to div 3 = 66.67 Hz; -DALIS_DOS_TICK_HZ=50 for exact 50.
#ifndef ALIS_DOS_TICK_HZ
#define ALIS_DOS_TICK_HZ 60
#endif
    atari_tick_target_hz = pl->is_little_endian ? ALIS_DOS_TICK_HZ : 50;

    dbglog("sys_init: requested %dx%d (tick target %d Hz)\n",
           (int)width, (int)height, atari_tick_target_hz);

    // Before audio (picks channel count); cookie reads only.
    detect_machine();
    dbglog("Machine: type=%d monitor=%d\n", (int)machine_type, (int)monitor_type);

    // --- Audio init FIRST, before any video mode changes ---
    audio_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));

    SDL_AudioSpec *desired_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));

    memset(desired_spec, 0, sizeof(SDL_AudioSpec));
    // Default spec (muted path and DSP): 12517 Hz, the STE DMA rate.
    desired_spec->freq = 12517;
    desired_spec->format = AUDIO_S16MSB;
    desired_spec->channels = (machine_type == MCH_ST) ? 1 : 2;

    if (mutesound) {
        dbglog("Audio MUTED (-m).\n");
        goto audio_mute;
    }

    // Falcon --audio=dsp: the DSP does all mixing and owns the codec (dsp_mixer_init is
    // self-sufficient); fed at 50 Hz by the Timer-A ISR.
    if (machine_type == MCH_FALCON && atari_audio_backend == 1) {
        int dsp_rc = dsp_mixer_init();
        dbglog("DSP mixer init=%d available=%d (opt-in via --audio=dsp)\n", dsp_rc, (int)dsp_mixer_available);
        if (dsp_mixer_available) {
            audio_spec->freq = desired_spec->freq;  // keep downstream isr_step sane
            // 0 would silence music.
            audio.host_freq   = desired_spec->freq;
            audio.host_format = desired_spec->format;
            Supexec(atari_timera_install_super);    // steady 50 Hz feed via MFP Timer A
            dbglog("Native DSP audio active (Timer-A feed).\n");
            goto audio_done;
        }
        dbglog("DSP init failed — falling back to DMA.\n");
    }

    // STE/TT/Falcon DMA sound (default): CPU mixes into an ST-RAM ring the chip loops, refilled
    // by the 50 Hz Timer-A ISR. STE/TT: 12517 Hz mono. Falcon: 8-bit mono (--audio-stereo: stereo,
    // the S8 callback duplicates the mix) at a crossbar rate 98340/(prescale+1) picked by CPU class
    // (_CPU gives family only); --audio-rate=N overrides.
    if (machine_type == MCH_TT || machine_type == MCH_STE || machine_type == MCH_FALCON) {
        const int is_fal   = (machine_type == MCH_FALCON);
        extern int atari_audio_rate;
        const int fal_auto = (cpu_family >= 60) ? 24585 : (cpu_family >= 40) ? 12292 : 8195;
        const int fal_rate = atari_audio_rate > 0 ? atari_audio_rate : fal_auto;
        const int dma_rate = is_fal ? fal_rate : (atari_audio_rate > 0 ? atari_audio_rate : 12517);
        extern int atari_audio_stereo;
        const int dma_ch   = is_fal && atari_audio_stereo ? 2 : 1;
        if (atari_dma_sound_init(dma_rate, dma_ch, is_fal) == 0) {
            audio_spec->freq     = dma_rate;
            audio_spec->format   = AUDIO_S8;
            audio_spec->channels = dma_ch;
            audio_spec->samples  = dma_rate / 50;
            audio.host_freq   = dma_rate;
            audio.host_format = AUDIO_S8;
            atari_dma_sound_set_isr_driven(1);
            Supexec(atari_timera_install_super);   // steady 50 Hz refill via MFP Timer A
            dbglog("Native %s DMA audio active (%d Hz %s, Timer-A feed).\n",
                   is_fal ? "Falcon" : "STE/TT", dma_rate, dma_ch == 2 ? "stereo" : "mono");
            goto audio_done;
        }
        dbglog("DMA sound init failed.\n");
    }
    dbglog("No DSP/DMA sound — audio muted.\n");

audio_mute:
    // No audio HW + ISR; host_freq must stay non-zero (music_v2 divides by it).
    audio_spec->freq     = desired_spec->freq;
    audio_spec->format   = desired_spec->format;
    audio_spec->channels = desired_spec->channels;
    audio.host_freq      = desired_spec->freq;
    audio.host_format    = desired_spec->format;

audio_done:
    free(desired_spec);

    // --- Now do GEM + video setup ---
    gem_init();
    dbglog("GEM init: ap_id=%d vdi=%d\n", (int)gem_ap_id, (int)gem_vdi_handle);

    save_video_state();
    dbglog("Saved video state: physbase=%p logbase=%p vmode=0x%lx\n",
           old_physbase, old_logbase, (unsigned long)old_vmode);

    gem_lock_screen();
    dbglog("GEM locked=%d\n", gem_locked);

    if (!setup_video_mode()) {
        dbglog("setup_video_mode FAILED\n");
        gem_unlock_screen();
        restore_video_state();
        logclose();
        exit(-1);
    }

    image.pal_format = EPalARGB;
    dirty_mouse = 0;

    dbglog("Video OK: %dx%d planes=%d screen_h=%d pitch=%d\n",
         (int)width, (int)height, (int)video_planes,
         (int)screen_height, (int)planar_pitch);
    dbglog("screen_buf[0]=%p screen_buf[1]=%p back_buf=%d\n",
         (void*)screen_buf[0], (void*)screen_buf[1], back_buf);

    sys_sfx_tick_hz = (u32)atari_tick_target_hz;
    isr_step = (sys_sfx_tick_hz << 16) / audio_spec->freq;  // 16.16 fixed-point
    isr_counter = 0x10000;  // = 1.0 in 16.16

    poll_ticks = sys_ticks();
    frame_time = poll_ticks;
    loop_time = poll_ticks;

    atari_ikbd_init();   // take over the keyboard scancode vector

    // Crash safety net once all vector hooks are live: MiNT runs etv_term ($408) even on SIGKILL,
    // before freeing our memory. Setexc from USER mode, not Supexec (as sys_sdl1.c).
    {
        atari_old_etv_term = (void *)Setexc(0x102, (long)atari_etv_term_handler);
        atexit(atari_etv_term_atexit);
        dbglog("etv_term handler installed, old=%p\n", atari_old_etv_term);
    }

#if defined(ALIS_DSP_MIXER)
    // itroutine only bumps timeclock until the game sets a fade, so installing pre-game is safe.
    // Seed the rate so video.c never sees the 50 default on DOS data before calibration.
    sys_timeclock_hz = (u32)atari_tick_target_hz;
    atari_timerc_install();
#endif

    dbglog("sys_init COMPLETE\n");
}

void sys_init_timers(void) {
    dbglog("sys_init_timers called\n");
    poll_ticks = sys_ticks();
    frame_time = sys_ticks();
    loop_time = sys_ticks();
}

// ============================================================================
#pragma mark - Timing
// ============================================================================

void sys_sleep_until(u32 *start, s32 len)
{
    u32 now = sys_ticks();
    s32 wait = len - (now - *start);
    if (wait > 0 && wait <= len)
        sys_delay(wait);

    *start = sys_ticks();
}

void sys_sleep_interactive(s32 *loop, s32 intr)
{
    u32 prev = sys_ticks();
    while (*loop > intr || (*loop > 0 && io_inkey() == 0))
    {
        sys_poll_event();
        sys_delay(k_event_ticks);

        u32 now = sys_ticks();
        *loop -= now - prev;
        prev = now;
    }

    *loop = 0;
}

void sys_delay_loop(void)
{
#if !defined(ALIS_NO_LOOP_DELAY)
    sys_sleep_until(&loop_time, 6);
#endif
}

void sys_delay_frame(void)
{
    sys_sleep_until(&frame_time, k_frame_ticks * 50 / sys_pace_hz() * alis.ctiming);

    // Single-buffer games drawing into the displayed buffer: start compositing at vblank so it
    // races ahead of the beam (as the original). frame_time was reset first, so only the phase
    // changes. Not needed with the canvas: only finished frames are published.
    if (alis.fswitch == 0 && !canvas)
        Vsync();
}

// DSP/DMA audio is ISR-driven: nothing to wait for.
void sys_sleep_until_music_stops(void) {}

// ============================================================================
#pragma mark - Main loop
// ============================================================================

u8 sys_start(void) {
    dbglog("sys_start: entering alis_thread\n");
    alis.state = eAlisStateRunning;
    alis_thread(NULL);
    dbglog("sys_start: alis_thread returned\n");
    return 0;
}

// ============================================================================
#pragma mark - Input
// ============================================================================

void sys_poll_event(void) {

    poll_calls++;
    if (poll_calls <= 3)
        dbglog("sys_poll_event #%d\n", (int)poll_calls);

    sys_sleep_until(&poll_ticks, k_frame_ticks);

    // itroutine runs from Timer C when installed; synchronous fallback otherwise.
#if defined(ALIS_DSP_MIXER)
    if (!atari_timerc_installed)
        itroutine(20, NULL);
#else
    itroutine(20, NULL);  // big-endian Atari = 20
#endif

    // Keys come from the IKBD handler (sys_atari_ikbd.S); here only the global quit key (Undo).
    if (g_ikbd_pressed[0x61]) {
        ALIS_DEBUG(EDebugSystem, "INTERRUPT: Quit by user request.\n");
        alis.state = eAlisStateStopped;
    }

    // No Pause/F11/F12 on Atari keyboards: Shift+F1 = save (F11), Shift+F2 = load (F12),
    // Shift+F10 = quit (Pause). Raw scancodes (F1=0x3b + shift), edge-triggered.
    {
        static u8 hk_f1, hk_f2, hk_f10;
        u8 sh  = (g_ikbd_pressed[0x2a] || g_ikbd_pressed[0x36]) ? 1 : 0;   // L/R shift
        u8 f1  = sh && g_ikbd_pressed[0x3b];
        u8 f2  = sh && g_ikbd_pressed[0x3c];
        u8 f10 = sh && g_ikbd_pressed[0x44];

        if (f1 && !hk_f1) {
            ALIS_DEBUG(EDebugSystem, "INTERRUPT: Save state (Shift+F1).\n");
            alis.state = eAlisStateSave;
        }
        if (f2 && !hk_f2) {
            ALIS_DEBUG(EDebugSystem, "INTERRUPT: Load state (Shift+F2).\n");
            alis.state = eAlisStateLoad;
        }
        if (f10 && !hk_f10) {
            ALIS_DEBUG(EDebugSystem, "INTERRUPT: Quit by user request (Shift+F10).\n");
            alis.state = eAlisStateStopped;
        }
        hk_f1 = f1; hk_f2 = f2; hk_f10 = f10;
    }

    // Mouse buttons only; position is integrated by atari_mouse_vbl.
    {
        // g_mouse_down latches presses since the last poll, so a click shorter than a frame
        // still registers. and.b to memory is one instruction: atomic against the IKBD ISR.
        extern volatile u8 g_mouse_down;
        u8 down = g_mouse_down;
        __asm__ volatile("and.b %1,%0" : "+m"(g_mouse_down) : "d"((u8)~down));
        u8  btn = g_mouse_buttons;
        u8 lb = (btn & 2) ? 1 : 0;           // packet bit1 = left button
        u8 rb = (btn & 1) ? 1 : 0;           // packet bit0 = right button
        if ((lb && !mouse.lb) || (down & 2)) mouse.lb_clicked = 1;
        if ((rb && !mouse.rb) || (down & 1)) mouse.rb_clicked = 1;
        mouse.lb = lb;
        mouse.rb = rb;
    }

    sys_render(host.pixelbuf);
}

// ============================================================================
#pragma mark - Rendering
// ============================================================================

// Page-flip the Falcon display base (supervisor). TOS's VBL reloads the hardware base from
// _v_bas_ad ($44E), so write both; latched at vblank, hence the caller Vsyncs.
//   $FF8201 = bits16-23, $FF8203 = 8-15, $FF820D = 0-7
static u8 *g_flip_base;
static long videl_setbase_super(void)
{
    u32 a = (u32)g_flip_base;
    *(volatile u32 *)0x44EUL   = a;            // _v_bas_ad: TOS VBL keeps the base here
    *(volatile u8 *)0xFFFF8201 = (u8)(a >> 16);
    *(volatile u8 *)0xFFFF8203 = (u8)(a >> 8);
    *(volatile u8 *)0xFFFF820D = (u8)(a);
    return 0;
}

// Read the live base back (supervisor) for the log.
static volatile u32 g_dbg_hwbase, g_dbg_vbas;
static long videl_readbase_super(void)
{
    g_dbg_hwbase = ((u32)*(volatile u8 *)0xFFFF8201 << 16)
                 | ((u32)*(volatile u8 *)0xFFFF8203 << 8)
                 |  (u32)*(volatile u8 *)0xFFFF820D;
    g_dbg_vbas   = *(volatile u32 *)0x44EUL;
    return 0;
}

#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
// Integrated software cursor — called from draw() (image.c), mirroring the original
// engine's mousefen/mouseput. Both operate on image.logic (the buffer being composed
// this frame, which the swap then makes the front buffer):
//   sys_mouse_erase  — runs BEFORE composition: restore the saved background (remove
//                      the previous cursor). Any staleness is overwritten by the
//                      composition that immediately follows → no stuck imprint.
//   sys_mouse_draw   — runs AFTER composition, BEFORE the swap: save the fresh
//                      background and draw the cursor, so the presented frame already
//                      contains it → no flicker.
// Per-buffer save (cursor_save[2]) keeps the two ping-pong buffers independent.
#if defined(ALIS_NATIVE_PLANAR)
// The cursor pre-converted to 8-plane words (16px/row → 8 plane-words), rebuilt by
// sys_dirty_mouse when the cursor sprite changes. Drawn word-at-a-time (cookie-cut),
// and the background under it is saved/restored as plane-words — no per-pixel loops.
// For planar, cursor_save: x = first chunk index, y = top row, w = #chunks (1-2), h = rows;
// bg = the saved plane-words (h * w*8).
static u16 cursor_planar[16][8];
#endif
// Restore the saved background for cursor entry `bi` into framebuffer `fb`.
static void cursor_restore_into(u16 *fb, int bi)
{
#if defined(ALIS_NATIVE_PLANAR)
    int pw = host.pixelbuf.w >> 1;            // plane-words per row
    s16 cx0 = cursor_save[bi].x, oy = cursor_save[bi].y;
    s16 nw = cursor_save[bi].w * 8, h = cursor_save[bi].h;
    const u16 *bg = cursor_save[bi].bg;
    for (int y = 0; y < h; y++) {
        u16 *drow = fb + (s32)(oy + y) * pw + (s32)cx0 * 8;
        for (int c = 0; c < nw; c++) drow[c] = *bg++;
    }
#else
    int pitch = host.pixelbuf.w;
    s16 ox = cursor_save[bi].x, oy = cursor_save[bi].y;
    for (int y = 0; y < cursor_save[bi].h; y++)
        for (int x = 0; x < cursor_save[bi].w; x++)
            fb[(oy + y) * pitch + (ox + x)] = cursor_save[bi].bg[y * 16 + x];
#endif
}

// --- Async-cursor shared state (see atari_mouse_vbl below) ---
static volatile u8  g_cursor_lock;                 // main loop holds it around copy/uncopy + flip
static u8 *volatile g_cursor_front;                // displayed buffer the ISR may draw on (NULL: none/TT)
static volatile s16 g_cursor_drawn_x[2] = {-1,-1};
static volatile s16 g_cursor_drawn_y[2] = {-1,-1};

// Before oldfen(): erase image.logic's OWN cursor (from when it was last the front
// buffer). image.logic is untouched since then, so its saved background is fresh.
// Takes g_cursor_lock: from here through sys_mouse_uncopy(), oldfen() copies front
// rects whose cursor position must not change under it (the ISR pauses).
void sys_mouse_erase(void)
{
#if defined(ALIS_NATIVE_PLANAR)
    if (is_doubleline)
        return;   // TT: compose buffers never carry a cursor (ISR paints the screen
                  // buffers); the cursor_save slots belong to screen_buf[] there.
#endif
    g_cursor_lock = 1;
    if (image.logic == NULL)
        return;
    int L = (image.logic == vm_render_buf0()) ? 0 : 1;
    if (!cursor_save[L].drawn)
        return;
    cursor_restore_into((u16 *)image.logic, L);
    cursor_save[L].drawn = 0;
}

// After oldfen() copied image.physic -> image.logic: erase the FRONT buffer's cursor
// that the copy dragged into image.logic. Valid because logic == physic at the copied
// rects. Leave the front entry's `drawn` set — that cursor still lives in the front
// buffer and is erased there when the front later becomes the compositing buffer.
void sys_mouse_uncopy(void)
{
#if defined(ALIS_NATIVE_PLANAR)
    if (is_doubleline)
        return;   // TT: see sys_mouse_erase (lock was never taken)
#endif
    if (image.logic == NULL) {
        g_cursor_lock = 0;
        return;
    }
    int P = (image.physic == vm_render_buf0()) ? 0 : 1;
    if (cursor_save[P].drawn)
        cursor_restore_into((u16 *)image.logic, P);
    g_cursor_lock = 0;   // releases the erase→oldfen→uncopy critical section
}

// Draw the cursor at (mx,my) into `fb`, saving the background into slot `bi`.
// ISR-safe: pure memory RMW, no OS calls. Shared by the per-frame sys_mouse_draw
// hook (compose buffer) and the VBL cursor mover atari_mouse_vbl (front buffer).
// `fb_h` = the buffer's row count (200 compose / 480 TT screen); `doubled` writes
// each cursor row twice (TT line-doubled screen), my then being an absolute
// screen row. cursor_save[bi].h records SCREEN rows, so cursor_restore_into
// works unchanged for both.
static void cursor_draw_into(u16 *fb, int bi, s16 mx, s16 my, s16 fb_h, int doubled)
{
    int pitch = host.pixelbuf.w;
    s16 mw = cursor_w, mh = cursor_h;
    s16 rows = doubled ? (s16)(mh * 2) : mh;
    if (mx + mw > pitch)   mw = pitch - mx;
    if (my + rows > fb_h)  rows = fb_h - my;
    if (mx < 0 || my < 0 || mw <= 0 || rows <= 0) {
        cursor_save[bi].drawn = 0;
        return;
    }
    mh = rows;    // from here on, mh = SCREEN rows written (2x cursor rows if doubled)
    int L = bi;   // keep the body's existing slot naming
#if defined(ALIS_NATIVE_PLANAR)
    // Word-level: save the 1-2 affected chunks' plane-words, then cookie-cut the cursor
    // (longword-shifted) into them. No per-pixel loops.
    int pw  = host.pixelbuf.w >> 1;
    int s   = mx & 15;
    int cx0 = mx >> 4, cx1 = (mx + mw - 1) >> 4, nch = cx1 - cx0 + 1;
    u16 *bg = cursor_save[L].bg;
    for (int y = 0; y < mh; y++) {
        u16 *drow = fb + (s32)(my + y) * pw + (s32)cx0 * 8;
        const u16 *cw = cursor_planar[doubled ? (y >> 1) : y];
        for (int c = 0; c < nch * 8; c++) *bg++ = drow[c];        // save bg (plane-words)
        u16 un0 = 0, dl[8];
        for (int p = 0; p < 8; p++) { dl[p] = (u16)(cw[p] >> s); un0 |= dl[p]; }
        if (un0) { u16 nm = (u16)~un0;
            for (int p = 0; p < 8; p++) drow[p] = (u16)((drow[p] & nm) | dl[p]); }
        if (nch == 2) {                                          // spill into the next chunk
            u16 un1 = 0, dt[8]; u16 *dc = drow + 8;
            for (int p = 0; p < 8; p++) { dt[p] = (u16)(cw[p] << (16 - s)); un1 |= dt[p]; }
            if (un1) { u16 nm = (u16)~un1;
                for (int p = 0; p < 8; p++) dc[p] = (u16)((dc[p] & nm) | dt[p]); }
        }
    }
    cursor_save[L].x = (s16)cx0; cursor_save[L].y = my;
    cursor_save[L].w = (s16)nch; cursor_save[L].h = mh;
#else
    const u8 *src = mouse_pixels;
    for (int y = 0; y < mh; y++) {
        for (int x = 0; x < mw; x++) {
            u16 *dst = &fb[(my + y) * pitch + (mx + x)];
            cursor_save[L].bg[y * 16 + x] = *dst;
            u8 idx = src[y * cursor_w + x];
            if (idx) *dst = pal16[idx];
        }
    }
    cursor_save[L].x = mx; cursor_save[L].y = my;
    cursor_save[L].w = mw; cursor_save[L].h = mh;
#endif
    cursor_save[L].drawn = 1;
}

// Before the swap: draw the cursor into image.logic (the just-composed buffer that the
// swap makes the front), saving its background.
void sys_mouse_draw(void)
{
#if defined(ALIS_NATIVE_PLANAR)
    if (is_doubleline)
        return;   // TT: the ISR paints the cursor on the screen buffers instead
#endif
    if (image.logic == NULL)
        return;
    int L = (image.logic == vm_render_buf0()) ? 0 : 1;

    if (!mouse.enabled) {
        cursor_save[L].drawn = 0;
        return;
    }

    cursor_draw_into((u16 *)image.logic, L, mouse.x, mouse.y, (s16)host.pixelbuf.h, 0);
    g_cursor_drawn_x[L] = mouse.x;
    g_cursor_drawn_y[L] = mouse.y;
}

// Savestates write and replace the screen buffers wholesale. on: erase the cursor from both
// buffers, forget the saved backgrounds and keep the ISR off them; off: let it draw again.
void sys_cursor_hold(int on)
{
    if (!on) {
        g_cursor_lock = 0;
        return;
    }
    g_cursor_lock = 1;
    for (int s = 0; s < 2; s++) {
        u8 *fb = sys_get_framebuffer(s);
#if defined(ALIS_NATIVE_PLANAR)
        if (is_doubleline)
            fb = screen_buf[s];
#endif
        if (cursor_save[s].drawn && fb)
            cursor_restore_into((u16 *)fb, s);
        cursor_save[s].drawn = 0;
    }
}

// ---------------------------------------------------------------------------
// VBL-driven cursor (as the original, independent of frame rate): (a) integrates
// the IKBD deltas (sole consumer, last-seen pattern: the IKBD ISR only adds), and
// (b) erases/redraws the cursor on the FRONT buffer. draw() holds g_cursor_lock
// around the oldfen-copy/uncopy window and the flip; the handler skips those ticks.
// ---------------------------------------------------------------------------
// Gate diagnostics, dumped from sys_render (dbglog is not ISR-safe).
volatile u32 g_mcur_tick, g_mcur_lock, g_mcur_nofront, g_mcur_gate, g_mcur_same, g_mcur_draw;

static void atari_mouse_vbl(void)
{
    g_mcur_tick++;
    // (a) integrate deltas at tick rate
    static s16 seen_x, seen_y;
    s16 ax = g_mouse_dx, ay = g_mouse_dy;      // single s16 reads — atomic vs IKBD ISR
    s16 dx = (s16)(ax - seen_x), dy = (s16)(ay - seen_y);
    seen_x = ax; seen_y = ay;
    if (dx || dy) {
        mouse.x += dx;
        mouse.y += dy;
        if (mouse.x < 0) mouse.x = 0;
        if (mouse.y < 0) mouse.y = 0;
        if (mouse.x >= host.pixelbuf.w) mouse.x = host.pixelbuf.w - 1;
        if (mouse.y >= host.pixelbuf.h) mouse.y = host.pixelbuf.h - 1;
    }

    // (b) move the on-screen cursor
    if (g_cursor_lock || g_isr_paused) {
        g_mcur_lock++;
        return;
    }
    u8 *front = g_cursor_front;
    if (front == NULL) {
        g_mcur_nofront++;
        return;
    }
    if (!mouse.enabled || alis.fswitch == 0) {
        g_mcur_gate++;
        return;
    }
    // TT: the front is a line-doubled 320x480 SCREEN buffer (slots indexed by
    // screen_buf, absolute screen row = border + 2*game_y). Falcon: the front is
    // a 320x200 compose/screen buffer (slots indexed by the vm bufs).
    int F, dbl;
    s16 sy, fbh;
#if defined(ALIS_NATIVE_PLANAR)
    if (is_doubleline) {
        dbl = 1;
        F   = (front == screen_buf[0]) ? 0 : 1;
        sy  = (s16)(((screen_height - 2 * height) / 2) + mouse.y * 2);
        fbh = (s16)screen_height;
    } else
#endif
    {
        dbl = 0;
        F   = (front == vm_render_buf0()) ? 0 : 1;
        sy  = mouse.y;
        fbh = (s16)host.pixelbuf.h;
    }
    if (cursor_save[F].drawn && g_cursor_drawn_x[F] == mouse.x && g_cursor_drawn_y[F] == mouse.y) {
        g_mcur_same++;
        return;                                 // already drawn there
    }
    if (cursor_save[F].drawn) {
        cursor_restore_into((u16 *)front, F);
        cursor_save[F].drawn = 0;
    }
    cursor_draw_into((u16 *)front, F, mouse.x, sy, fbh, dbl);
    g_mcur_draw++;
    g_cursor_drawn_x[F] = mouse.x;
    g_cursor_drawn_y[F] = mouse.y;
}
#else
void sys_mouse_erase(void)  {}
void sys_mouse_uncopy(void) {}
void sys_mouse_draw(void)   {}
void sys_flip_wait(void)    {}
#endif // ALIS_NATIVE_16BPP || ALIS_NATIVE_PLANAR

// The buffer sys_render last published. Film paths move the VIDEL base themselves
// and return before the publish block, so it is invalidated when a film ends.
static u8 *last_shown = NULL;

#if ALIS_NATIVE_PLANAR
// ============================================================================
// Native Spectrum-512 (S512 / FLS) film render — ST 4-bit videos (.ao/.do data).
//
// S512 shows >16 colours by rewriting the ST palette as the beam scans a line
// (palcntr += 102/pixel; the slot _pi = palcntr>>10 is refreshed as it advances,
// so the 16-colour palette morphs across the row). The Falcon CLUT can't change
// mid-scanline, so every pixel is baked to an 8-bit RGB233 index into a fixed
// 256-colour CLUT (s512_lut9 quantises the ST 9-bit colour to RGB233).
//
// Per 16-px chunk: inverse-Kalms the 4 plane words -> 16 indices, apply the
// morphing llut -> 16 8-bit pixels, Kalms-8 -> 8 plane words. The two magic-mask
// transposes replace the SDL renderer's per-pixel bit-twiddling, so we produce
// 8-plane output for ~the cost SDL spends producing chunky. Palette advancement
// and the half-line dirty logic mirror sys_sdl1.c render_s512.
// ============================================================================

// inverse-Kalms: 4 plane words (MSB=pixel0) -> 16 chunky indices. Reverse of
// kalms16_4's magic-mask steps.
static inline void s512_planar_to_chunky16(const u16 *plane, u8 *out)
{
    u32 d1 = ((u32)plane[1]<<16) | plane[0];
    u32 d0 = ((u32)plane[3]<<16) | plane[2];
    u32 d7;
    d7=((d1>>2)^d0)&0x33333333u; d0^=d7; d1^=(d7<<2);
    { u32 s1=(d1<<16)|(d1>>16), s0=(d0<<16)|(d0>>16);
      u32 n0=(d0&0xffff0000u)|(s1&0xffffu), n1=(s0&0xffff0000u)|(d1&0xffffu); d0=n0; d1=n1; }
    d7=((d1>>1)^d0)&0x55555555u; d0^=d7; d1^=(d7<<1);
    d7=((d1>>8)^d0)&0x00ff00ffu; d0^=d7; d1^=(d7<<8);
    out[0]=(d0>>28)&0xf; out[4]=(d0>>24)&0xf; out[1]=(d0>>20)&0xf; out[5]=(d0>>16)&0xf;
    out[2]=(d0>>12)&0xf; out[6]=(d0>>8)&0xf;  out[3]=(d0>>4)&0xf;  out[7]=d0&0xf;
    out[8]=(d1>>28)&0xf; out[12]=(d1>>24)&0xf;out[9]=(d1>>20)&0xf; out[13]=(d1>>16)&0xf;
    out[10]=(d1>>12)&0xf;out[14]=(d1>>8)&0xf; out[11]=(d1>>4)&0xf; out[15]=d1&0xf;
}

// Kalms-8: 16 chunky (8-bit) pixels -> 8 plane words (same as opcodes.c kalms16_8).
static inline void s512_chunky16_to_planar(const u8 *in, u16 *out)
{
    u32 d0=((u32)in[0]<<24)|((u32)in[1]<<16)|((u32)in[2]<<8)|in[3];
    u32 d1=((u32)in[4]<<24)|((u32)in[5]<<16)|((u32)in[6]<<8)|in[7];
    u32 d2=((u32)in[8]<<24)|((u32)in[9]<<16)|((u32)in[10]<<8)|in[11];
    u32 d3=((u32)in[12]<<24)|((u32)in[13]<<16)|((u32)in[14]<<8)|in[15];
    u32 d7;
    d7=((d1>>4)^d0)&0x0f0f0f0fu; d0^=d7; d1^=(d7<<4);
    d7=((d3>>4)^d2)&0x0f0f0f0fu; d2^=d7; d3^=(d7<<4);
    d7=((d2>>8)^d0)&0x00ff00ffu; d0^=d7; d2^=(d7<<8);
    d7=((d3>>8)^d1)&0x00ff00ffu; d1^=d7; d3^=(d7<<8);
    d7=((d2>>1)^d0)&0x55555555u; d0^=d7; d2^=(d7<<1);
    d7=((d3>>1)^d1)&0x55555555u; d1^=d7; d3^=(d7<<1);
    { u32 s2=(d2<<16)|(d2>>16), s0=(d0<<16)|(d0>>16);
      u32 n0=(d0&0xffff0000u)|(s2&0xffffu), n2=(s0&0xffff0000u)|(d2&0xffffu); d0=n0; d2=n2; }
    { u32 s3=(d3<<16)|(d3>>16), s1=(d1<<16)|(d1>>16);
      u32 n1=(d1&0xffff0000u)|(s3&0xffffu), n3=(s1&0xffff0000u)|(d3&0xffffu); d1=n1; d3=n3; }
    d7=((d2>>2)^d0)&0x33333333u; d0^=d7; d2^=(d7<<2);
    d7=((d3>>2)^d1)&0x33333333u; d1^=d7; d3^=(d7<<2);
    out[0]=(u16)d3; out[1]=(u16)(d3>>16); out[2]=(u16)d1; out[3]=(u16)(d1>>16);
    out[4]=(u16)d2; out[5]=(u16)(d2>>16); out[6]=(u16)d0; out[7]=(u16)(d0>>16);
}

#define ST_PAL9(v) (((((v)>>8)&7)<<6) | ((((v)>>4)&7)<<3) | ((v)&7))

static u8 s512_lut9[512];       // ST 9-bit colour -> RGB233 8-bit index
#if defined(ALIS_S512_FALCON16)
// Falcon 16-bit fused path, one heap block per S512 film (s512_build_luts16 / s512_free_luts16):
static u16 *s512_pal2565;       // [4096] raw ST palette word (&0xfff) -> RGB565
static u32 *s512_spread;        // [256] planar byte b: bit k -> bit0 of nibble (7-k)
static u8  s512_seg[2][18];     // seg[half][k] = first column (0..160) where pi>=k; seeds 408 / 510
#endif
static u8 s512_luts_ready = 0;
static u8 s512_clut_set = 0;
static u8 s512_was_active = 0;    // an S512 film rendered since the last film-end
static u8 s512_pending_clear = 0; // clear planes 4-7 on the first game frame after the film

// Film ping-pong state (shared FLIC/S512): film_shown = the buffer currently
// scanned out (NULL at film start); reset when a film ends (see sys_render).
static u8 *film_shown = NULL;
#if defined(ALIS_FLI_SPAN_C2P)
static u16 film_prev_c0[256], film_prev_c1[256];   // per-row dirty chunk span [c0,c1), 2-frame catch-up
#else
static u8  film_prev_drow[256];
#endif
static u8  film_prev_valid = 0;

static void s512_build_luts(void)
{
    // Quantise the ST 3-3-3 colour to RGB 2-3-3 (matches sys_sdl1.c lut9.p8).
    for (int c = 0; c < 512; c++) {
        u8 r2 = (c >> 7) & 3, g3 = (c >> 3) & 7, b3 = c & 7;
        s512_lut9[c] = (u8)((r2 << 6) | (g3 << 3) | b3);
    }
#if defined(ALIS_S512_FALCON16)
    // Palette-change columns: pi = (seed + col*102) >> 10 becomes k at the first column where
    // seed + col*102 >= k*1024. Precomputed so the render walks pixel-runs between changes
    // instead of recomputing pi (a variable asr) every pixel. seed 408 = left half, 510 = right.
    for (int s = 0; s < 2; s++) {
        int seed = s ? 510 : 408;
        for (int k = 0; k <= 17; k++) {
            int x = 0; long thr = (long)k * 1024;
            while (x < 160 && (seed + (long)x * 102) < thr) x++;
            s512_seg[s][k] = (u8)x;
        }
    }
#endif
    s512_luts_ready = 1;
}

#if defined(ALIS_S512_FALCON16)
static int s512_build_luts16(void)
{
    if (s512_pal2565)
        return 1;
    u16 *blk = (u16 *)malloc(4096 * sizeof(u16) + 256 * sizeof(u32));
    if (!blk)
        return 0;
    // ST 3-3-3 colour (9-bit index) -> RGB565: expand each component to 5/6 bits.
    u16 lut16[512];
    for (int c = 0; c < 512; c++) {
        u8 r = (c >> 6) & 7, g = (c >> 3) & 7, b = c & 7;
        u16 r5 = (r << 2) | (r >> 1), g6 = (g << 3) | g, b5 = (b << 2) | (b >> 1);
        lut16[c] = (u16)((r5 << 11) | (g6 << 5) | b5);
    }
    u32 *spread = (u32 *)(blk + 4096);
    for (int v = 0; v < 4096; v++) blk[v] = lut16[ST_PAL9(v)];   // folds ST_PAL9 in
    for (int b = 0; b < 256; b++) {
        u32 s = 0;
        for (int k = 0; k < 8; k++) if (b & (1 << k)) s |= 1u << ((7 - k) * 4);
        spread[b] = s;                             // bit k (pixel 7-k in the byte) -> nibble (7-k)
    }
    s512_spread = spread;
    s512_pal2565 = blk;
    return 1;
}

static void s512_free_luts16(void)
{
    free(s512_pal2565);
    s512_pal2565 = NULL;
    s512_spread = NULL;
}
#endif

// Zero the high 4 bitplanes (planes 4-7) of an 8-plane framebuffer. The S512 video writes all 8
// planes (256-colour RGB233), but the 4-plane games that show these videos only draw planes 0-3 —
// on real ST the video is itself 4-plane, so clearing 4-7 restores exactly that state and the
// game's 16-colour sprites read correct indices again. Interleaved layout: each 16-px group is 8
// plane-words; planes 4-7 are the upper 8 bytes (two longs) of every 16-byte group.
static void s512_clear_high_planes(u8 *buf)
{
    u32 *p = (u32 *)buf;
    for (u32 i = 0; i < 16000; i += 4) {   // 64000 bytes / 4 = 16000 longs, 4 longs per group
        p[i + 2] = 0;                      // planes 4,5
        p[i + 3] = 0;                      // planes 6,7
    }
}

// Supervisor CLUT upload: write all 256 VIDEL entries from g_s512_clut_src (FALCON_CLUT_RGB
// format 0xRRGG__BB). Runs via Supexec so the $FFFF9800 writes are supervisor-legal.
static const u32 *g_s512_clut_src;
static long s512_clut_super(void)
{
    volatile u32 *clut = (volatile u32 *)0xFFFF9800UL;
    const sColorARGB *p = (const sColorARGB *)g_s512_clut_src;
    for (int i = 0; i < 256; i++)
        clut[i] = ((u32)p[i].r << 24) | ((u32)p[i].g << 16) | (u32)p[i].b;
    return 0;
}

// Fixed 256-colour CLUT for S512: index i is RGB233 -> back to RGB bytes
// (matches sys_sdl1.c hicolor8_palette). Built into buffer.palette + applied once.
static void s512_set_clut(pixelbuf_t buffer)
{
    for (int i = 0; i < 256; i++) {
        sColorARGB *c = (sColorARGB *)&buffer.palette[i];
        c->a = 0;
        c->r = (u8)(i & 0xC0);
        c->g = (u8)((i & 0x38) << 2);
        c->b = (u8)((i & 0x07) << 5);
    }
    // VIDEL CLUT is supervisor-only: Supexec. Not falcon_clut_apply (its clinepal branch would
    // re-pack a flat RGB233 CLUT). TT/ST: XBIOS.
    if (machine_type == MCH_FALCON) {
        g_s512_clut_src = (const u32 *)buffer.palette;
        Supexec(s512_clut_super);
    } else {
        set_palette(buffer.palette, 256);
    }
}

// Decode the whole S512 frame into the 8-plane back buffer. Only lines dirty this
// or last frame are redrawn (fls_prev_dirty gives the 2-frame ping-pong catch-up).
static void s512_render_planar(u8 *back)
{
    extern u8 *vgalogic_df;
    if (!s512_luts_ready) s512_build_luts();

    u8  *bitmap  = vgalogic_df + 0xA0;
    // 17+ entries: pi reaches 16 on the last pixels of a half-line (written, never read for output).
    u16  curpal[24];
    memcpy(curpal, vgalogic_df + 32000, 32);
    u16 *palette = (u16 *)(vgalogic_df + 32000 + 32);
    u8   llut[24];
    for (int c = 0; c < 16; c++) llut[c] = s512_lut9[ST_PAL9(curpal[c])];

    // FLS films leave bfilm.height at 0 (the ST format doesn't set it); the frame is always the
    // full 200-line screen (SDL uses buffer.h, not bfilm.height).
    s32 h = bfilm.height > 0 ? bfilm.height : 200;
    if (h > 200) h = 200;
    u32 at = 0;
    for (s32 y = 0; y < h; y++, palette += 16) {
        u8 halves = fls_dirty_lines[y] | fls_prev_dirty[y];
        if (!halves) {                                  // whole line unchanged: advance palette state
            at += 160; palette += 32;
            memcpy(curpal, palette, 32);
            for (int c = 0; c < 16; c++) llut[c] = s512_lut9[ST_PAL9(curpal[c])];
            continue;
        }
        u16 *pw = (u16 *)(back + (u32)y * planar_pitch);
        s32 palcntr, prev_pi;

        // Left half — columns 0..159 (10 chunks of 16 px). palcntr seed 408.
        if (halves & 1) {
            palcntr = 408; prev_pi = -1;
            for (int ch = 0; ch < 10; ch++, at += 8, pw += 8) {
                u16 pl[4];
                pl[0]=(bitmap[at]  <<8)|bitmap[at+1]; pl[1]=(bitmap[at+2]<<8)|bitmap[at+3];
                pl[2]=(bitmap[at+4]<<8)|bitmap[at+5]; pl[3]=(bitmap[at+6]<<8)|bitmap[at+7];
                u8 idx[16], out8[16];
                s512_planar_to_chunky16(pl, idx);
                for (int i = 0; i < 16; i++) {
                    s32 pi = palcntr >> 10;
                    if (pi != prev_pi) {
                        curpal[pi] = palette[pi];
                        llut[pi] = s512_lut9[ST_PAL9(curpal[pi])];
                        prev_pi = pi;
                    }
                    out8[i] = llut[idx[i]];
                    palcntr += 102;
                }
                s512_chunky16_to_planar(out8, pw);
            }
        } else {
            at += 80; pw += 80;
            memcpy(curpal, palette, 32);
            for (int c = 0; c < 16; c++) llut[c] = s512_lut9[ST_PAL9(curpal[c])];
        }
        palette += 16;

        // Right half — columns 160..319 (10 chunks). palcntr seed 510.
        if (halves & 2) {
            palcntr = 510; prev_pi = -1;
            for (int ch = 0; ch < 10; ch++, at += 8, pw += 8) {
                u16 pl[4];
                pl[0]=(bitmap[at]  <<8)|bitmap[at+1]; pl[1]=(bitmap[at+2]<<8)|bitmap[at+3];
                pl[2]=(bitmap[at+4]<<8)|bitmap[at+5]; pl[3]=(bitmap[at+6]<<8)|bitmap[at+7];
                u8 idx[16], out8[16];
                s512_planar_to_chunky16(pl, idx);
                for (int i = 0; i < 16; i++) {
                    s32 pi = palcntr >> 10;
                    if (pi != prev_pi) {
                        curpal[pi] = palette[pi];
                        llut[pi] = s512_lut9[ST_PAL9(curpal[pi])];
                        prev_pi = pi;
                    }
                    out8[i] = llut[idx[i]];
                    palcntr += 102;
                }
                s512_chunky16_to_planar(out8, pw);
            }
        } else {
            at += 80;
            memcpy(curpal, palette, 32);
            for (int c = 0; c < 16; c++) llut[c] = s512_lut9[ST_PAL9(curpal[c])];
        }
        palette += 16;

        memcpy(curpal, palette, 32);
        for (int c = 0; c < 16; c++) llut[c] = s512_lut9[ST_PAL9(curpal[c])];
    }

    memcpy(fls_prev_dirty, fls_dirty_lines, 200);
}

#if defined(ALIS_S512_FALCON16)
// ============================================================================
// Falcon 16-bit fused S512 render — writes RGB565 straight to the on-screen BPS16
// framebuffer (no chunky->planar c2p, no CLUT). Same index selection as
// s512_render_planar.
//
// The per-scanline palette morph (pi = (seed + col*102)>>10, seed 408/510) only
// changes pi every ~10 pixels, so emit pixel-runs between the precomputed change
// columns (s512_seg), loading one palette entry per run.
// ============================================================================
// Prevent gcc from rewriting the rebuild loops into libc memcpy/memset calls (the -ffixed-a4/a5
// build spills those to slow ST-RAM) or Duff-unrolling them.
#define S512_16_ATTR __attribute__((optimize("no-tree-loop-distribute-patterns,no-unroll-loops,no-peel-loops")))

// Reload all 16 base palette entries (line-start / not-dirty state) via the direct pal->565 LUT.
static inline void s512_rebuild(u16 *llut16, const u16 *palette)
{
    for (int c = 0; c < 16; c++) llut16[c] = s512_pal2565[palette[c] & 0xfff];
}

// Fused half-line: transpose 10 planar chunks to chunky nibbles via s512_spread (kept in
// registers — no idx160 round-trip through ST-RAM), and emit each pixel straight to the 16-bit
// screen, loading a palette entry only at the precomputed morph-change columns (s512_seg). The
// nibble order from s512_spread is sequential (pixel i = nibble i), so we shift them out 4 bits
// at a time.
static inline void s512_emit_half(const u8 *bm, u16 *scr, const u16 *palette,
                                  const u8 *sg, u16 *llut16)
{
    int col = 0, next_k = 0, bound = sg[0];
    for (int ch = 0; ch < 10; ch++) {
        u16 p0=(bm[0]<<8)|bm[1], p1=(bm[2]<<8)|bm[3], p2=(bm[4]<<8)|bm[5], p3=(bm[6]<<8)|bm[7];
        bm += 8;
        u32 hi = s512_spread[p0>>8]   | (s512_spread[p1>>8]<<1)   | (s512_spread[p2>>8]<<2)   | (s512_spread[p3>>8]<<3);
        u32 lo = s512_spread[p0&0xff] | (s512_spread[p1&0xff]<<1) | (s512_spread[p2&0xff]<<2) | (s512_spread[p3&0xff]<<3);
        for (int p = 0; p < 8; p++) {
            while (col == bound) { llut16[next_k] = s512_pal2565[palette[next_k] & 0xfff]; next_k++; bound = sg[next_k]; }
            scr[col++] = llut16[hi & 0xf]; hi >>= 4;
        }
        for (int p = 0; p < 8; p++) {
            while (col == bound) { llut16[next_k] = s512_pal2565[palette[next_k] & 0xfff]; next_k++; bound = sg[next_k]; }
            scr[col++] = llut16[lo & 0xf]; lo >>= 4;
        }
    }
}

static S512_16_ATTR void s512_render_16(u16 *screen, u32 stride)
{
    if (!s512_luts_ready) s512_build_luts();
    if (!s512_build_luts16()) return;

    u8 *bitmap = vgalogic_df + 0xA0;
    u16 llut16[24];                                    // [16] tolerated (pi reaches 16); never read for output
    s512_rebuild(llut16, (const u16 *)(vgalogic_df + 32000));
    const u16 *palette = (const u16 *)(vgalogic_df + 32000 + 32);

    s32 h = bfilm.height > 0 ? bfilm.height : 200;
    if (h > 200) h = 200;
    u32 at = 0;
    for (s32 y = 0; y < h; y++, palette += 16) {
        u8 halves = fls_dirty_lines[y] | fls_prev_dirty[y];
        if (!halves) {                                 // whole line unchanged: advance palette state
            at += 160; palette += 32;
            s512_rebuild(llut16, palette);
            continue;
        }
        u16 *row = screen + (u32)y * stride;

        if (halves & 1) {                              // left half — columns 0..159, seed 408
            s512_emit_half(bitmap + at, row, palette, s512_seg[0], llut16);
            at += 80;
        } else {
            at += 80;
            s512_rebuild(llut16, palette);
        }
        palette += 16;

        if (halves & 2) {                              // right half — columns 160..319, seed 510
            s512_emit_half(bitmap + at, row + 160, palette, s512_seg[1], llut16);
            at += 80;
        } else {
            at += 80;
            s512_rebuild(llut16, palette);
        }
        palette += 16;

        s512_rebuild(llut16, palette);
    }

    memcpy(fls_prev_dirty, fls_dirty_lines, 200);
}
#endif // ALIS_S512_FALCON16

// ============================================================================
// Native HAM6 (Amiga Hold-And-Modify) film render — Amiga 6-plane videos (.do data).
//
// HAM6 planes are stored CONTIGUOUSLY (Amiga style): c0..c5 at bitmap + k*planesize
// (planesize = w*h/8). Per pixel: planes 0-3 = 4-bit index, planes 4-5 = 2-bit control.
//   ctrl 0: take r,g,b from the 16-entry ham_pal[index]
//   ctrl 1/2/3: HOLD the previous pixel, MODIFY B/R/G to (index<<4)
// r,g,b carry left-to-right and reset each row, so the decode is inherently sequential —
// we batch only the plane extraction (inverse-Kalms) and the planar output (Kalms-8), and
// quantise the running RGB to the same fixed 256-colour RGB233 CLUT as S512.
// ============================================================================
static void ham6_render_planar(u8 *back)
{
    extern u8 *vgalogic_df;
    if (!s512_luts_ready) s512_build_luts();

    u8 *bitmap = vgalogic_df + 0xA0;
    u32 planesize = (320u * 200u) >> 3;                 // 8000 bytes per Amiga bitplane
    u8 *c0 = bitmap,        *c1 = c0 + planesize, *c2 = c1 + planesize,
       *c3 = c2 + planesize, *c4 = c3 + planesize, *c5 = c4 + planesize;
    u8 (*ham_pal)[3] = bfilm.ham_pal;

    // Precompute the RGB233 index for each of the 16 palette entries so ctrl==0 pixels are a single
    // lookup, and carry the packed RGB233 value directly (no per-pixel r/g/b split + re-quantise).
    u8 pal233[16];
    for (int i = 0; i < 16; i++)
        pal233[i] = (u8)(((ham_pal[i][0] >> 6) << 6) | ((ham_pal[i][1] >> 5) << 3) | (ham_pal[i][2] >> 5));

    for (s32 y = 0; y < 200; y++) {
        if (!(fls_dirty_lines[y] | fls_prev_dirty[y])) continue;   // clean row (2-frame catch-up) → keep back buffer's copy
        u8 cur = 0;                                      // packed RGB233, resets each row (HAM start)
        u16 *pw = (u16 *)(back + (u32)y * planar_pitch);
        u32 off = (u32)y * 40;                           // 40 bytes/row per bitplane
        for (int ch = 0; ch < 20; ch++, off += 2, pw += 8) {
            u16 pi[4] = { (u16)((c0[off]<<8)|c0[off+1]), (u16)((c1[off]<<8)|c1[off+1]),
                          (u16)((c2[off]<<8)|c2[off+1]), (u16)((c3[off]<<8)|c3[off+1]) };
            u16 pc[4] = { (u16)((c4[off]<<8)|c4[off+1]), (u16)((c5[off]<<8)|c5[off+1]), 0, 0 };
            u8 idx[16], ctl[16], out8[16];
            s512_planar_to_chunky16(pi, idx);            // 4 index planes  -> 16 indices
            s512_planar_to_chunky16(pc, ctl);            // 2 control planes -> 16 ctrl (0..3)
            for (int i = 0; i < 16; i++) {
                u8 c = ctl[i], id = idx[i];
                if      (c == 0) cur = pal233[id];               // set from palette
                else if (c == 1) cur = (u8)((cur & 0xF8) | (id >> 1));         // modify B (3-bit)
                else if (c == 2) cur = (u8)((cur & 0x3F) | ((id >> 2) << 6));  // modify R (2-bit)
                else             cur = (u8)((cur & 0xC7) | ((id >> 1) << 3));  // modify G (3-bit)
                out8[i] = cur;
            }
            s512_chunky16_to_planar(out8, pw);
        }
    }
    memcpy(fls_prev_dirty, fls_dirty_lines, 200);
}
#endif // ALIS_NATIVE_PLANAR

// Set by the game flip below; consumed by sys_flip_wait() at the top of the next
// draw(). Main-loop only (no ISR access), so no volatile needed.
static u8 g_flip_pending;

// Block until the VBL has latched the most recent flip. Called by draw() right
// before the first write into the new back buffer, so it is usually a no-op.
void sys_flip_wait(void)
{
    if (g_flip_pending) {
        g_flip_pending = 0;
#if defined(ALIS_PROFILE_DRAW)
        extern u32 g_prof_frame_vsync;
        u32 _pf_v0 = sys_profile_ticks_safe();
        Vsync();
        g_prof_frame_vsync += sys_profile_ticks_safe() - _pf_v0;
#else
        Vsync();
#endif
    }
}

// TT film playback at half vertical res (decode+c2p even source lines only, quad-copy to
// screen): the TT's 030 can't do full-res 256-colour film at speed. TT only.
#if defined(ALIS_USE_NATIVE_ATARI) && !defined(ALIS_FLI_TT_HALFRES)
#define ALIS_FLI_TT_HALFRES 1
#endif
// Scanline mode (needs half-res): write 2 of every 4 screen rows, the rest stay black,
// halving the ST-RAM write. Disable with -DALIS_FLI_TT_SCANLINE=0.
#if defined(ALIS_FLI_TT_HALFRES) && !defined(ALIS_FLI_TT_SCANLINE)
#define ALIS_FLI_TT_SCANLINE 1
#endif
#if defined(ALIS_FLI_TT_SCANLINE) && (ALIS_FLI_TT_SCANLINE + 0 == 0)
#undef ALIS_FLI_TT_SCANLINE
#endif
#if defined(ALIS_FLI_TT_HALFRES)
static u8 g_tt_film_hr = 0;      // set by the FLIC branch for a TT half-res film frame
#endif

// TT_LOW (320x480, the TT's only 8bpp mode): expand the 320x200 compose frame into the
// ST-RAM display buffer, each line written twice with movem.l bursts (the access pattern
// contended ST-RAM handles best). Row pitch hard-wired to 320 bytes (8 planes x 320 px).
static void tt_line_double_copy(const u8 *src, u8 *dst, u32 lines)
{
    // End-pointer compare: d0-d7 carry pixel data, no register left for dbra (a4/a5 -ffixed).
    const u8 *end = src + lines * 320u;
    __asm__ volatile (
    "1:\n\t"
        ".rept 10\n\t"
        "movem.l (%[s])+,%%d0-%%d7\n\t"
        "movem.l %%d0-%%d7,(%[d])\n\t"
        "movem.l %%d0-%%d7,320(%[d])\n\t"
        "lea 32(%[d]),%[d]\n\t"
        ".endr\n\t"
        "lea 320(%[d]),%[d]\n\t"
        "cmpa.l %[e],%[s]\n\t"
        "blt 1b"       /* word branch — the unrolled body is past short-branch reach */
        : [s]"+a"(src), [d]"+a"(dst)
        : [e]"a"(end)
        : "d0","d1","d2","d3","d4","d5","d6","d7","memory","cc");
}

#if defined(ALIS_FLI_TT_HALFRES)
// Half-res video variant: each compose row is written to FOUR screen rows (quad),
// so `lines` half-res compose rows fill 4*lines screen rows — same 400-line region
// as the full-res 200-row double-copy, at half the vertical detail. Used only for
// TT film playback (see the FLIC branch); the game keeps the double-copy.
static void tt_line_quad_copy(const u8 *src, u8 *dst, u32 lines)
{
    const u8 *end = src + lines * 320u;
    __asm__ volatile (
    "1:\n\t"
        ".rept 10\n\t"
        "movem.l (%[s])+,%%d0-%%d7\n\t"
        "movem.l %%d0-%%d7,(%[d])\n\t"
        "movem.l %%d0-%%d7,320(%[d])\n\t"
        "movem.l %%d0-%%d7,640(%[d])\n\t"
        "movem.l %%d0-%%d7,960(%[d])\n\t"
        "lea 32(%[d]),%[d]\n\t"
        ".endr\n\t"
        "lea 960(%[d]),%[d]\n\t"
        "cmpa.l %[e],%[s]\n\t"
        "blt 1b"
        : [s]"+a"(src), [d]"+a"(dst)
        : [e]"a"(end)
        : "d0","d1","d2","d3","d4","d5","d6","d7","memory","cc");
}

#if defined(ALIS_FLI_TT_SCANLINE)
// Scanline variant: write each compose row to only TWO of its four screen rows and
// skip the other two (which are pre-blacked once at film start), HALVING the ST-RAM
// screen write — the dominant TT film cost. Same 400-line region; the gaps read as
// black scanlines. `dst` advances 4 rows/compose-row (2 written, 2 left black).
static void tt_line_scanline_copy(const u8 *src, u8 *dst, u32 lines)
{
    const u8 *end = src + lines * 320u;
    __asm__ volatile (
    "1:\n\t"
        ".rept 10\n\t"
        "movem.l (%[s])+,%%d0-%%d7\n\t"
        "movem.l %%d0-%%d7,(%[d])\n\t"
        "movem.l %%d0-%%d7,320(%[d])\n\t"
        "lea 32(%[d]),%[d]\n\t"
        ".endr\n\t"
        "lea 960(%[d]),%[d]\n\t"       /* +320(rept)+960 = 1280 = 4 rows: 2 written, 2 skipped */
        "cmpa.l %[e],%[s]\n\t"
        "blt 1b"
        : [s]"+a"(src), [d]"+a"(dst)
        : [e]"a"(end)
        : "d0","d1","d2","d3","d4","d5","d6","d7","memory","cc");
}
#endif
#endif /* ALIS_FLI_TT_HALFRES */

// ---------------------------------------------------------------------------
// TT dirty-rect publish: copy only dirty_rects[] (complete: SDL1 presents only those).
// The published buffer is TWO frames stale, so each frame's rects go to BOTH buffers'
// pending lists. Saturation (dirty_len 0xff), overflow, films and each buffer's first
// publish fall back to a full copy. -DALIS_TT_FULL_PUBLISH=1 disables rect mode.
// ---------------------------------------------------------------------------
#if defined(ALIS_TT_FULL_PUBLISH) && ALIS_TT_FULL_PUBLISH
#define TT_DIRTY_PUBLISH 0
#else
#define TT_DIRTY_PUBLISH 1
#endif

#define TT_PEND_MAX 96
typedef struct { s16 x, y, w, h; } tt_rect;
static tt_rect tt_pend[2][TT_PEND_MAX];
static u8      tt_pend_n[2];
static u8      tt_pend_full[2] = { 1, 1 };    // first publish of each buffer = full

static void tt_pend_add(s16 x, s16 y, s16 w, s16 h)
{
    if (w <= 0 || h <= 0) return;
    for (int b = 0; b < 2; b++) {
        if (tt_pend_full[b]) continue;
        if (tt_pend_n[b] >= TT_PEND_MAX) { tt_pend_full[b] = 1; continue; }
        tt_pend[b][tt_pend_n[b]++] = (tt_rect){ x, y, w, h };
    }
}

// Collect this frame's change list. The native cursor is drawn into the compose
// buffers by the draw() hooks, OUTSIDE the dirty_rects mechanism — cover it with
// both saved-background rects (old positions, per compose slot) plus the current
// position.
static void tt_pend_frame(void)
{
    if (dirty_len == 0xff) {
        tt_pend_full[0] = tt_pend_full[1] = 1;
    } else {
        for (int i = 0; i < dirty_len; i++)
            tt_pend_add(dirty_rects[i].x, dirty_rects[i].y, dirty_rects[i].w, dirty_rects[i].h);
    }
#if defined(ALIS_NATIVE_PLANAR)
    for (int s = 0; s < 2; s++)   // planar cursor_save: x = chunk index, w = chunks
        if (cursor_save[s].drawn)
            tt_pend_add(cursor_save[s].x * 16, cursor_save[s].y, cursor_save[s].w * 16, cursor_save[s].h);
    if (mouse.enabled)
        tt_pend_add(mouse.x, mouse.y, cursor_w ? cursor_w : 16, cursor_h ? cursor_h : 16);
#endif
}

// Clamped, 16px-chunk-aligned rect for dirty publishing.
typedef struct { s16 c0, c1, y0, y1; } chunk_rect;

// Adds a rect to q[0..n) unless it lies inside one already there; drops the ones it covers.
// Keeps *area (chunk-aligned pixels) in sync. Returns the new count.
static int chunk_rect_add(chunk_rect *q, int n, u32 *area, s16 x, s16 y, s16 w, s16 h)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > 320)         w = 320 - (s32)x;
    if (y + h > (s16)height) h = (s16)height - y;
    if (w <= 0 || h <= 0) return n;

    s16 c0 = x >> 4, c1 = (s16)(x + w - 1) >> 4, y1 = (s16)(y + h - 1);
    for (int j = 0; j < n; j++) {
        if (c0 >= q[j].c0 && c1 <= q[j].c1 && y >= q[j].y0 && y1 <= q[j].y1)
            return n;
        if (q[j].c0 >= c0 && q[j].c1 <= c1 && q[j].y0 >= y && q[j].y1 <= y1) {
            *area -= (u32)(q[j].c1 - q[j].c0 + 1) * 16u * (u32)(q[j].y1 - q[j].y0 + 1);
            q[j--] = q[--n];
        }
    }
    q[n] = (chunk_rect){ c0, c1, y, y1 };
    *area += (u32)(c1 - c0 + 1) * 16u * (u32)(y1 - y + 1);
    return n + 1;
}

// Line-double one rect from the 320x200 compose buffer into the (already
// border-offset) 320x400 destination. Planar layout: align x to 16px chunks
// (16 bytes each); rows are contiguous within the chunk span. memcpy is the
// mint movem-based one; the second read of src costs little (TT-RAM).
static void tt_line_double_rect(const u8 *src, u8 *dst, s16 x, s16 y, s16 w, s16 h)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > 320)        w = 320 - (s32)x;
    if (y + h > (s16)height) h = (s16)height - y;
    if (w <= 0 || h <= 0) return;

    s16 c0 = x >> 4, c1 = (s16)(x + w - 1) >> 4;
    u32 wb = (u32)(c1 - c0 + 1) * 16u;
    const u8 *s = src + (u32)y * 320u + (u32)c0 * 16u;
    u8 *d = dst + (u32)y * 640u + (u32)c0 * 16u;
    for (s16 r = 0; r < h; r++, s += 320u, d += 640u) {
        memcpy(d, s, wb);
        memcpy(d + 320, s, wb);
    }
}

// Expand `composed` (an image.physic/logic compose buffer) into the back display
// buffer, flip the display ping-pong, and return the new video base for the
// caller to program. Used by both the game flip and the film paths (films pass
// full=1: whole-frame content, and they cover path-switch transitions). Waits
// out a still-pending flip first (possible after a path switch). The 40 spare lines
// above and below the doubled 400 stay black from the alloc-time memset.
static u8 *tt_publish(u8 *composed, int full)
{
    if (g_flip_pending) {
        g_flip_pending = 0;
        Vsync();
    }
    u8 *base = screen_buf[back_buf];
    u8 *dst  = base + (u32)((screen_height - 2 * height) / 2) * planar_pitch;

    // This buffer carried an ISR cursor when it was last the front: restore its
    // background BEFORE copying fresh content (the published rects may not cover
    // it, and its saved bg goes stale under the copy either way). The game-flip
    // caller holds g_cursor_lock; film callers set g_cursor_front = NULL first.
    {
        int ts = (base == screen_buf[0]) ? 0 : 1;
        if (cursor_save[ts].drawn) {
            cursor_restore_into((u16 *)base, ts);
            cursor_save[ts].drawn = 0;
        }
    }

#if defined(ALIS_FLI_TT_HALFRES)
    // Half-res TT film frame: `composed` holds height/2 rows (even source lines);
    // quad-copy each to 4 screen rows to fill the same centered 400-line region.
    // Full copy (dirty-publish is moot for the full-motion cutscenes this targets).
    if (g_tt_film_hr) {
        g_tt_film_hr = 0;
#if defined(ALIS_FLI_TT_SCANLINE)
        tt_line_scanline_copy(composed, dst, (u32)height >> 1);   // 2 of 4 rows; gaps pre-blacked
#else
        tt_line_quad_copy(composed, dst, (u32)height >> 1);
#endif
        back_buf ^= 1;
        return base;
    }
#endif

#if TT_DIRTY_PUBLISH
    if (full) {
        tt_pend_full[0] = tt_pend_full[1] = 1;   // both buffers need the full frame
    } else {
        tt_pend_frame();                          // append this frame's rects to both
    }
    if (tt_pend_full[back_buf]) {
        tt_line_double_copy(composed, dst, height);
    } else {
        // Absorb contained rects: the 2-frame accumulation otherwise copies an
        // every-frame 3D viewport twice.
        chunk_rect q[TT_PEND_MAX];
        int n = 0;
        u32 area = 0;
        for (int i = 0; i < tt_pend_n[back_buf]; i++) {
            tt_rect *r = &tt_pend[back_buf][i];
            n = chunk_rect_add(q, n, &area, r->x, r->y, r->w, r->h);
        }
        // Area gate: past ~70% of the frame the sequential unrolled-movem full
        // copy beats scattered per-rect memcpy rows (and bounds any remaining
        // partial-overlap double-copy).
        if (area >= (u32)(320 * (s32)height * 7 / 10)) {
            tt_line_double_copy(composed, dst, height);
        } else {
            for (int i = 0; i < n; i++)
                tt_line_double_rect(composed, dst, (s16)(q[i].c0 * 16), q[i].y0,
                                    (s16)((q[i].c1 - q[i].c0 + 1) * 16), (s16)(q[i].y1 - q[i].y0 + 1));
        }
    }
    tt_pend_full[back_buf] = 0;
    tt_pend_n[back_buf] = 0;
#else
    (void)full;
    tt_line_double_copy(composed, dst, height);
#endif

    back_buf ^= 1;
    return base;
}

#if ALIS_NATIVE_PLANAR
// Copy this frame's dirty rects (widened to 16px chunks) from the canvas to the displayed
// buffer; the full frame after a base change (film) or when the rects cover most of it. -DALIS_CANVAS_FULL_PUBLISH=1 always copies the full frame.
static void canvas_publish(void)
{
    u8 *src = image.physic;
    u8 *dst = screen_buf[0] + (src - canvas);
    int full = last_shown != screen_buf[0] || dirty_len == 0xff || ALIS_CANVAS_FULL_PUBLISH;

    if (!full && dirty_len == 0)
        return;

    chunk_rect q[256];
    int n = 0;
    u32 area = 0, bytes = 0;
    if (!full) {
        for (int i = 0; i < dirty_len; i++)
            n = chunk_rect_add(q, n, &area, dirty_rects[i].x, dirty_rects[i].y, dirty_rects[i].w, dirty_rects[i].h);
        full = area >= (u32)(320 * (s32)height * 7 / 10);
    }

    if (full) {
        n = 0;
        q[n++] = (chunk_rect){ 0, 319 >> 4, 0, (s16)height - 1 };
    }

    // trsfen moves only the planes in use (and uses the Blitter on big aligned rects).
    s16 fx1 = image.fenx1, fy1 = image.feny1, fx2 = image.fenx2, fy2 = image.feny2;
    for (int i = 0; i < n; i++) {
        image.fenx1 = q[i].c0 * 16;
        image.fenx2 = q[i].c1 * 16 + 15;
        image.feny1 = q[i].y0;
        image.feny2 = q[i].y1;
        trsfen(src, dst);
        bytes += (u32)(q[i].c1 - q[i].c0 + 1) * 16 * (u32)(q[i].y1 - q[i].y0 + 1);
    }
    image.fenx1 = fx1; image.feny1 = fy1; image.fenx2 = fx2; image.feny2 = fy2;

#if defined(ALIS_PROFILE_DRAW)
    {
        static u32 frames, fulls, rects, copied;
        frames++; fulls += full; rects += n; copied += bytes;
        if (frames == 100) {
            dbglog("canvas/100f: full=%u rects=%u KB=%u\n", fulls, rects, copied / 1024);
            frames = fulls = rects = copied = 0;
        }
    }
#endif

    if (last_shown != screen_buf[0]) {
        g_flip_base = screen_buf[0];
        Supexec(videl_setbase_super);
        g_cursor_front = screen_buf[0];
        last_shown = screen_buf[0];
    }
}
#endif

void sys_render(pixelbuf_t buffer) {

    render_calls++;

#if defined(ALIS_MEM_WATCH)
    // Free-RAM low-water mark (largest free block per pool), logged when it drops.
    if ((render_calls % 25) == 0) {
        static u32 st_min = 0xffffffffu, fast_min = 0xffffffffu;
        u32 st = sys_stram_avail(), fast = sys_fastram_avail();
        if (st < st_min || fast < fast_min) {
            if (st < st_min) st_min = st;
            if (fast < fast_min) fast_min = fast;
            dbglog("memwatch: frame %d ST free %u KB (min %u)  fast free %u KB (min %u)\n",
                   (int)render_calls, st / 1024, st_min / 1024, fast / 1024, fast_min / 1024);
        }
    }
#endif

#if defined(ALIS_PROFILE_DRAW) && defined(ALIS_BENCH_BUS)
    { extern void sys_bench_bus(void); static u8 _bench_done = 0;
      if (!_bench_done && render_calls > 3) { _bench_done = 1; sys_bench_bus(); } }
#endif

    if (render_calls <= 5)
        dbglog("render #%d: dirty_len=%d dirty_pal=%d buf=%p\n",
             (int)render_calls, (int)dirty_len, (int)dirty_pal, (void*)buffer.data);

#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
    // Async-cursor gate diagnostics (see atari_mouse_vbl).
    {
        extern volatile u32 g_mcur_tick, g_mcur_lock, g_mcur_nofront, g_mcur_gate, g_mcur_same, g_mcur_draw;
        static u32 _mc_last; static int _mc_n;
        if (_mc_n < 20 && g_mcur_tick - _mc_last >= 250) {   // ~every 5s of ticks
            _mc_last = g_mcur_tick; _mc_n++;
            dbglog("[mcur] ticks=%u lock=%u nofront=%u gate=%u same=%u draw=%u (fswitch=%d en=%d front=%p)\n",
                   g_mcur_tick, g_mcur_lock, g_mcur_nofront, g_mcur_gate, g_mcur_same, g_mcur_draw,
                   (int)alis.fswitch, (int)mouse.enabled, (void *)g_cursor_front);
        }
    }
#endif

    if (dirty_mouse) {
        dirty_mouse = false;
        sys_dirty_mouse();
    }

#if ALIS_NATIVE_PLANAR
    // FLIC film frame (8-bit games, .IO/.FO data): video.c decompressed the full-screen 8-bit
    // chunky frame into vgalogic. Convert it straight into the live 8-plane framebuffer (for a
    // 320-wide frame the c2p output stride cpr*16 == planar_pitch, so it lands row-for-row).
    // Then fall through to the existing flip + palette path to publish it.
    {
        extern u8 *vgalogic;
        extern void chunky8_to_planar(const u8 *idx8, s32 w, s32 h, int flip, u8 *dst);
        // draw() steps aside during films, so image.physic/logic are the two stable screen buffers
        // and WE own the ping-pong (film_shown / film_prev_* are file-scope, reset when a film ends).

#if defined(ALIS_FLI_FALCON16)
        // 16-bit mode persists across a video sequence (gaps hold the last frame instead of
        // flashing a stale planar buffer); the deferred restore below returns to planar on the
        // first real game draw. A planar-only film type must restore planar now.
        if (g_fli16_screen && bfilm.type != eAlisVideoNone
            && bfilm.type != eAlisVideoFLIC && bfilm.type != eAlisVideoCleanup
#if defined(ALIS_S512_FALCON16)
            && bfilm.type != eAlisVideoS512     // S512 has its own 16-bit fused path — keep 16-bit mode
#endif
           )
            sys_film16_end();
#endif

        // S512 / FLS films (ST 4-bit videos) — own decoder (per-scanline palette morph).
        if (bfilm.type == eAlisVideoS512) {
#if defined(ALIS_S512_FALCON16)
            // Falcon 16-bit fused path: render RGB565 into the off-screen FLIC 16-bit surface, flip.
            if (machine_type == MCH_FALCON) {
                if (!g_fli16_screen) sys_film16_begin(width, height);
                if (g_fli16_screen) {
                    extern u8 g_s512_ending;
                    // No s512_was_active: planar buffers untouched, no high-plane clear needed.
                    s512_render_16(film16_back, g_fli16_stride);   // dirty|prev-dirty lines, 2-frame catch-up
                    g_flip_base = (u8 *)film16_back;               // direct VIDEL base write (cheaper than Setscreen XBIOS)
                    Supexec(videl_setbase_super);
                    Vsync();
                    { u16 *t = film16_front; film16_front = film16_back; film16_back = t; }
                    g_fli16_screen = film16_front;                 // keep armed (deferred restore ends it)
                    g_cursor_front = NULL;
                    dirty_len = 0;
                    if (g_s512_ending) { bfilm.type = eAlisVideoNone; g_s512_ending = 0; }
                    return;
                }
            }
#endif
            if (film_shown == NULL) film_shown = image.physic;
            u8 *back = (film_shown == image.physic) ? image.logic : image.physic;
            s512_was_active = 1;
#if defined(ALIS_PROFILE_DRAW)
            u32 _s5_0 = sys_profile_ticks_safe();
#endif
            s512_render_planar(back);                  // writes dirty|prev-dirty lines only
            // CLUT after rendering, right before the flip: the slow first frame would otherwise
            // show the previous image under the new palette.
            if (!s512_clut_set) { s512_set_clut(buffer); s512_clut_set = 1; }
#if defined(ALIS_PROFILE_DRAW)
            {
                extern u32 g_flic_decode;
                static u32 s5_n, s5_dec, s5_dec0;
                s5_dec += sys_profile_ticks_safe() - _s5_0;
                if (++s5_n >= 50) {
                    dbglog("S512/50f: render=%u ticks(%ums) decode=%u(%ums)\n",
                           s5_dec, s5_dec * 5, g_flic_decode - s5_dec0, (g_flic_decode - s5_dec0) * 5);
                    s5_n = s5_dec = 0; s5_dec0 = g_flic_decode;
                }
            }
#endif
            g_cursor_front = NULL;   // films own the screen; no ISR cursor until the game flips again
            g_flip_base = is_doubleline ? tt_publish(back, 1) : back;   // films: full frame
            Supexec(videl_setbase_super);
            Vsync();
            film_shown = back;
            dirty_len = 0;
            // Final frame flushed (runfilm hit end-of-data): now drop to None so the game resumes.
            // The film-end reset below (next call) does the ping-pong reset + deferred plane clear.
            { extern u8 g_s512_ending;
              if (g_s512_ending) { bfilm.type = eAlisVideoNone; g_s512_ending = 0; } }
            return;
        }

        // HAM6 / Amiga Hold-And-Modify films. Same fixed RGB233 CLUT + ping-pong/flip as S512;
        // the decoder renders the whole frame (no usable per-line dirty), so no catch-up needed.
        if (bfilm.type == eAlisVideoHAM6) {
            extern u8 g_s512_ending;
            static s16 last_ham_frame = -1;
            if (film_shown == NULL) { film_shown = image.physic; last_ham_frame = -1; }
            u8 *back = (film_shown == image.physic) ? image.logic : image.physic;
            // Full HAM decode is expensive; don't repeat it when the film frame hasn't advanced
            // (the game keeps calling sys_render between crunfilm ticks). Keep the current display.
            if (bfilm.frame == last_ham_frame && !g_s512_ending) { dirty_len = 0; return; }
            int _h6_first = (last_ham_frame < 0);        // first render of this film
            last_ham_frame = bfilm.frame;
            s512_was_active = 1;
            // First frame: force every row (both ping-pong buffers get a full frame over 2 frames via
            // the catch-up), so no never-dirtied row is left showing stale buffer content.
            if (_h6_first) memset(fls_dirty_lines, 3, 200);
            { static int _h6_dbg = 0;
              if (_h6_dbg < 40) { dbglog("HAM6 render: frame=%d back=%p film_shown=%p pal0=%d,%d,%d\n",
                    (int)bfilm.frame, (void*)back, (void*)film_shown,
                    bfilm.ham_pal[0][0], bfilm.ham_pal[0][1], bfilm.ham_pal[0][2]); _h6_dbg++; } }
#if defined(ALIS_PROFILE_DRAW)
            u32 _h6_0 = sys_profile_ticks_safe();
#endif
            ham6_render_planar(back);
            if (!s512_clut_set) { s512_set_clut(buffer); s512_clut_set = 1; }
#if defined(ALIS_PROFILE_DRAW)
            { static u32 h6_n, h6_r; h6_r += sys_profile_ticks_safe() - _h6_0;
              if (++h6_n >= 50) { dbglog("HAM6/50f: render=%u ticks(%ums)\n", h6_r, h6_r*5); h6_n = h6_r = 0; } }
#endif
            g_cursor_front = NULL;   // films own the screen; no ISR cursor until the game flips again
            g_flip_base = is_doubleline ? tt_publish(back, 1) : back;   // films: full frame
            Supexec(videl_setbase_super);
            Vsync();
            film_shown = back;
            { extern u8 g_s512_ending;
              if (g_s512_ending) { bfilm.type = eAlisVideoNone; g_s512_ending = 0; } }
            dirty_len = 0;
            return;
        }

        if (bfilm.type == eAlisVideoFLIC || bfilm.type == eAlisVideoCleanup) {
#if defined(ALIS_FLI_FALCON16)
            // 16-bit fused path: fli_decomp16 already wrote the screen. Cleanup frame: retire
            // the film but stay 16-bit (see the deferred restore).
            if (g_fli16_screen) {
                if (bfilm.type == eAlisVideoCleanup) bfilm.type = eAlisVideoNone;
                dirty_len = 0;
                return;
            }
#endif
            s32 w = bfilm.width, h = bfilm.height, hc = h < 256 ? h : 256;
            // Write ONLY the back buffer (never the one being scanned out → no visible redraw), then
            // point the VIDEL at it. Because the back buffer sat out the previous frame, apply BOTH
            // the previous and current frame's dirty rows (2-frame catch-up) so it's coherent; a
            // keyframe (dirty_len==0xff) marks all rows.
            if (film_shown == NULL) {
                film_shown = image.physic;   // physic is live when a film starts
#if defined(ALIS_FLI_TT_SCANLINE)
                if (is_doubleline) {
                    // Scanline mode never writes the "off" rows, so black the whole video
                    // region in BOTH screen buffers once now -> the gaps stay black.
                    u32 _off = (u32)((screen_height - 2 * height) / 2) * planar_pitch;
                    u32 _sz  = (u32)(2 * height) * planar_pitch;
                    memset(screen_buf[0] + _off, 0, _sz);
                    memset(screen_buf[1] + _off, 0, _sz);
                }
#endif
            }
            u8 *back = (film_shown == image.physic) ? image.logic : image.physic;

#if defined(ALIS_FLI_SPAN_C2P)
            /* Span-based c2p: convert only each row's dirty chunk range (from FLI's dirty
             * rects), unioned with the previous frame's span (2-frame catch-up keeps the
             * alternating back buffer coherent). Over-converting a gap between disjoint
             * spans is harmless. */
            s32 cpr = (w + 15) >> 4;
            static u16 cur_c0[256], cur_c1[256];
            if (dirty_len == 0xff) {
                for (s32 y = 0; y < hc; y++) { cur_c0[y] = 0; cur_c1[y] = (u16)cpr; }
            } else {
                for (s32 y = 0; y < hc; y++) { cur_c0[y] = (u16)cpr; cur_c1[y] = 0; }  // empty span
                for (int i = 0; i < (int)dirty_len; i++) {
                    s32 ry = dirty_rects[i].y - image.feny1;
                    if (ry < 0 || ry >= hc) continue;
                    s32 rx0 = dirty_rects[i].x - image.fenx1;
                    s32 rx1 = rx0 + dirty_rects[i].w;
                    if (rx0 < 0) rx0 = 0;
                    if (rx1 > w) rx1 = w;
                    if (rx1 <= rx0) continue;
                    u16 c0 = (u16)(rx0 >> 4);
                    u16 c1 = (u16)((rx1 + 15) >> 4);
                    if (c1 > (u16)cpr) c1 = (u16)cpr;
                    if (c0 < cur_c0[ry]) cur_c0[ry] = c0;
                    if (c1 > cur_c1[ry]) cur_c1[ry] = c1;
                }
            }
#if defined(ALIS_PROFILE_DRAW)
            u32 _fp_rows = 0, _fp_c0 = sys_profile_ticks_safe();
#endif
#if defined(ALIS_FLI_TT_HALFRES)
            if (is_doubleline) {
                // TT half-res: c2p only EVEN source rows into the top half of the compose
                // buffer (row k = source row 2k); tt_publish quad-copies them to screen.
                for (s32 k = 0, y = 0; y < hc; k++, y += 2) {
                    s32 a0 = cur_c0[y], a1 = cur_c1[y];
                    if (film_prev_valid) {
                        if (film_prev_c0[y] < a0) a0 = film_prev_c0[y];
                        if (film_prev_c1[y] > a1) a1 = film_prev_c1[y];
                    }
                    if (a1 > a0) {
                        s32 px0 = a0 * 16;
                        s32 pxend = a1 * 16; if (pxend > w) pxend = w;
                        chunky8_to_planar(vgalogic + (u32)y * w + px0, pxend - px0, 1, 0,
                                          back + (u32)k * planar_pitch + px0);
#if defined(ALIS_PROFILE_DRAW)
                        _fp_rows++;
#endif
                    }
                }
                g_tt_film_hr = 1;   // tt_publish: quad-copy hc/2 compose rows for this frame
            } else
#endif
            for (s32 y = 0; y < hc; y++) {
                s32 a0 = cur_c0[y], a1 = cur_c1[y];
                if (film_prev_valid) {
                    if (film_prev_c0[y] < a0) a0 = film_prev_c0[y];
                    if (film_prev_c1[y] > a1) a1 = film_prev_c1[y];
                }
                if (a1 > a0) {
                    s32 px0 = a0 * 16;
                    s32 pxend = a1 * 16; if (pxend > w) pxend = w;
                    chunky8_to_planar(vgalogic + (u32)y * w + px0, pxend - px0, 1, 0,
                                      back + (u32)y * planar_pitch + px0);
#if defined(ALIS_PROFILE_DRAW)
                    _fp_rows++;
#endif
                }
            }
#if defined(ALIS_PROFILE_DRAW)
            u32 _fp_c2p = sys_profile_ticks_safe() - _fp_c0;
            int _fp_full = (dirty_len == 0xff);
#endif
#if defined(ALIS_FLI_SPAN_VERIFY)
            /* Oracle: a correct span+catch-up must leave `back` byte-identical to a
             * full-frame conversion of the current vgalogic. Any mismatch = a stale
             * chunk (coverage/catch-up bug or a dirty-rect the decoder missed). */
            {
                static u8 flispan_ref[64000];
                static u32 flispan_calls = 0, flispan_bad = 0;
                s32 sz = planar_pitch * hc;
                if (sz > (s32)sizeof(flispan_ref)) sz = sizeof(flispan_ref);
                for (s32 y = 0; y < hc; y++)
                    chunky8_to_planar(vgalogic + (u32)y * w, w, 1, 0, flispan_ref + (u32)y * planar_pitch);
                u32 diff = 0; s32 firstoff = -1;
                for (s32 i = 0; i < sz; i++)
                    if (flispan_ref[i] != back[i]) { if (firstoff < 0) firstoff = i; diff++; }
                flispan_calls++;
                if (diff) { flispan_bad++;
                    dbglog("[flispan] BAD call=%lu frame=%d diff=%lu firstoff=%ld dl=%u\n",
                           (unsigned long)flispan_calls, (int)bfilm.frame, (unsigned long)diff,
                           (long)firstoff, (unsigned)dirty_len); }
                else if ((flispan_calls & 0x3f) == 0)
                    dbglog("[flispan] OK %lu calls (bad=%lu)\n", (unsigned long)flispan_calls, (unsigned long)flispan_bad);
            }
#endif
            memcpy(film_prev_c0, cur_c0, (size_t)hc * sizeof(u16));
            memcpy(film_prev_c1, cur_c1, (size_t)hc * sizeof(u16));
            film_prev_valid = 1;
#else /* whole-row c2p (original) */
            u8 cur_drow[256];
            if (dirty_len == 0xff) {
                memset(cur_drow, 1, (size_t)hc);
            } else {
                memset(cur_drow, 0, (size_t)hc);
                for (int i = 0; i < (int)dirty_len; i++) {
                    s32 ry = dirty_rects[i].y - image.feny1;
                    if (ry >= 0 && ry < hc) cur_drow[ry] = 1;
                }
            }
#if defined(ALIS_PROFILE_DRAW)
            u32 _fp_rows = 0, _fp_c0 = sys_profile_ticks_safe();
#endif
            for (s32 y = 0; y < hc; y++)
                if (cur_drow[y] || (film_prev_valid && film_prev_drow[y])) {
                    chunky8_to_planar(vgalogic + (u32)y * w, w, 1, 0, back + (u32)y * planar_pitch);
#if defined(ALIS_PROFILE_DRAW)
                    _fp_rows++;
#endif
                }
#if defined(ALIS_PROFILE_DRAW)
            u32 _fp_c2p = sys_profile_ticks_safe() - _fp_c0;
            int _fp_full = (dirty_len == 0xff);
#endif
            memcpy(film_prev_drow, cur_drow, (size_t)hc);
            film_prev_valid = 1;
#endif /* ALIS_FLI_SPAN_C2P */

            // Palette: fli_palette already aliased image.mpalet into buffer.palette (via ampalet).
            if (image.ftopal) {
                image.ftopal = 0;
                dirty_pal = 1;
                if (machine_type != MCH_FALCON || !atari_timerc_installed) {
                    set_palette(buffer.palette, 256);   // ST/TT (Timer-C ISR does the Falcon CLUT)
                    dirty_pal = 0;
                }
            }

            // Publish: point the VIDEL at the freshly-drawn back buffer and wait for the latch.
            g_cursor_front = NULL;   // films own the screen; no ISR cursor until the game flips again
            g_flip_base = is_doubleline ? tt_publish(back, 1) : back;   // films: full frame
            Supexec(videl_setbase_super);
#if defined(ALIS_PROFILE_DRAW)
            u32 _fp_v0 = sys_profile_ticks_safe();
#endif
            Vsync();
            film_shown = back;

#if defined(ALIS_PROFILE_DRAW)
            {
                extern u32 g_flic_decode;                // RLE-decode ticks accumulated in runfilm
                static u32 fp_n, fp_rows, fp_c2p, fp_vs, fp_full, fp_dec0;
                fp_c2p += _fp_c2p;  fp_rows += _fp_rows;  fp_full += _fp_full;
                fp_vs  += sys_profile_ticks_safe() - _fp_v0;
                if (++fp_n >= 50) {
                    dbglog("FLIC/50f: rows=%u/f c2p=%u ticks(%ums) vsync=%u(%ums) decode=%u(%ums) full=%u\n",
                           fp_rows / 50, fp_c2p, fp_c2p * 5, fp_vs, fp_vs * 5,
                           g_flic_decode - fp_dec0, (g_flic_decode - fp_dec0) * 5, fp_full);
                    fp_n = fp_rows = fp_c2p = fp_vs = fp_full = 0; fp_dec0 = g_flic_decode;
                }
            }
#endif

            if (bfilm.type == eAlisVideoCleanup) {
                bfilm.type = eAlisVideoNone;
                film_shown = NULL; film_prev_valid = 0; s512_clut_set = 0;   // reset for next film
                last_shown = NULL;      // re-anchor the ping-pong (see declaration)
                image.fphytolog = 1;    // equalise both buffers before the game repaints
                vgalogic_free();
            }
            dirty_len = 0;
            return;                                    // film owns the frame — skip normal flip/render
        }
    }

#if defined(ALIS_FLI_FALCON16)
    // Deferred restore: still 16-bit with no film (gap between videos, or game resumed).
    // Hold the last frame until the game draws, then restore planar and publish below.
    if (g_fli16_screen && bfilm.type == eAlisVideoNone) {
        if (dirty_len == 0) return;      // idle (between videos) -> stay 16-bit (hold last frame)
        sys_film16_end();                 // first real game draw -> restore planar, publish below
    }
#endif

    // A film just ended (S512 has no Cleanup frame): reset the ping-pong and CLUT. The high-plane
    // clear waits for the first game draw (the last video frame is still on screen).
    if (bfilm.type == eAlisVideoNone && film_shown != NULL) {
        film_shown = NULL; film_prev_valid = 0; s512_clut_set = 0;
        // Publish selector is stale (film moved the base): force a publish, and copy
        // physic->logic so the buffer not painted during the film can't alternate into view.
        last_shown = NULL;
        image.fphytolog = 1;
        if (s512_was_active) { s512_pending_clear = 1; s512_was_active = 0; }
        vgalogic_free();
#if defined(ALIS_S512_FALCON16)
        s512_free_luts16();
#endif
    }

    // First game draw after an S512 film: zero the high planes both buffers inherited from the
    // 8-plane video.
    if (s512_pending_clear && dirty_len != 0) {
        s512_clear_high_planes(image.physic);
        s512_clear_high_planes(image.logic);
        s512_pending_clear = 0;
    }
#endif

#if ALIS_NATIVE_PLANAR
    if (canvas)
        canvas_publish();
#endif

    // Flip before the dirty early-return: draw() swaps physic/logic every frame, and the
    // display must track the swap or the next frame draws into the on-screen buffer.
    u8 publish = !canvas && image.physic != last_shown;
    // TT: single-buffer games (fswitch==0) never swap physic/logic, so the
    // compose buffer must be re-published whenever new content landed in it.
    if (is_doubleline && dirty_len != 0)
        publish = 1;
    if (publish) {
        // Falcon/ST: bare base write. TT: tt_publish line-doubles the compose buffer first.
        g_cursor_lock = 1;   // front is about to change under the cursor ISR
        g_flip_base = is_doubleline ? tt_publish(image.physic, 0) : image.physic;
        Supexec(videl_setbase_super);
        // New front = cursor mover's target (TT: the published screen buffer, cursor-free).
        g_cursor_front = g_flip_base;
        g_cursor_lock = 0;
        // Deferred Vsync: the retired buffer must not be written before the VBL latches the
        // new base; sys_flip_wait() blocks on that only when draw() first writes it.
        g_flip_pending = 1;
        last_shown = image.physic;

        if (render_calls <= 8) {
            Supexec(videl_readbase_super);   // read the live base after the latch
            dbglog("render #%d: physic=%p logic=%p hwbase=%06lx v_bas=%06lx\n",
                   (int)render_calls, (void*)image.physic, (void*)image.logic,
                   (unsigned long)g_dbg_hwbase, (unsigned long)g_dbg_vbas);
        }
    }

    if (dirty_len == 0 && dirty_pal == 0)
        return;

    if (dirty_len == 0xff)
        dirty_len = 1;

    // Update hardware palette
    if (dirty_pal) {
#if defined(ALIS_NATIVE_16BPP)
        set_palette(buffer.palette, 256);
        // 16bpp: draw() consumes dirty_pal (it must re-blit with the fresh pal16).
#else
        // Falcon: the Timer-C ISR writes the CLUT (atari_timerc_c_callback). Here only for
        // TT/ST or Falcon with --no-timerc.
        if (machine_type != MCH_FALCON || !atari_timerc_installed) {
            set_palette(buffer.palette, 256);
            dirty_pal = 0;
        }
#endif
    }

    dirty_len = 0;

#if ALIS_NATIVE_PLANAR
    // Planar blitter call counts
    extern void planar_dump_profile(void);
    planar_dump_profile();
#endif
}

// ============================================================================
#pragma mark - Abnormal-termination cleanup (etv_term / PROCTERM)
// ============================================================================
// On a crash or kill our Timer C/A, kbdvec, mousevec and VBL hooks (and audio DMA) would
// outlive our memory. MiNT runs etv_term ($408) before reclaiming it, even on SIGKILL.
// Mirrors sys_sdl1.c's atari_etv_term_handler and ScummVM's critical_restore.
volatile u32 atari_etv_term_fired = 0;   // re-entry guard for MiNT's p_term loop
volatile u32 atari_cleanup_done   = 0;   // set at the end of sys_deinit (normal path already ran)

void atari_etv_term_handler(void)
{
    if (atari_etv_term_fired)
        return;
    atari_etv_term_fired = 1;

    // Take our entry out of the chain first, so a fault below falls through to the next handler
    // instead of recursing into us.
    void *old_term = atari_old_etv_term;
    atari_old_etv_term = NULL;

    if (atari_cleanup_done) {
        // Normal shutdown already did everything; repeating it here would risk faulting.
        if (old_term)
            Setexc(0x102, (long)old_term);
        return;
    }

    // Abnormal exit. Same order as sys_deinit: silence the interrupt SOURCES first, then release
    // the vectors they use — a source left live while its vector is restored fires into the old
    // handler with our state half gone.
#if defined(ALIS_DSP_MIXER)
    if (atari_timera_installed)
        Supexec(atari_timera_uninstall_super);   // MFP Timer A ($134): the audio feed
    if (atari_dma_available)
        atari_dma_sound_close();                 // stop the DMA chip reading our ST-RAM ring
    else if (dsp_mixer_available)
        dsp_mixer_close();
#endif
    sys_deinit_psg();                            // YM latches its registers; silence it explicitly

#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
    Supexec(atari_vbl_remove_super);             // VBL cursor mover
#endif
#if defined(ALIS_DSP_MIXER)
    if (atari_timerc_installed)
        Supexec(atari_timerc_uninstall_super);   // etv_timer ($400)
#endif
    atari_ikbd_deinit();                         // kbdvec + mousevec

    if (old_term)
        Setexc(0x102, (long)old_term);
}

// atexit twin: covers a plain exit()/abort() that never reaches sys_deinit but does run the C
// runtime's exit handlers (etv_term covers the rest).
static void atari_etv_term_atexit(void)
{
    if (!atari_cleanup_done)
        atari_etv_term_handler();
}

void sys_deinit(void) {

#if defined(ALIS_DSP_MIXER)
    if (atari_timerc_installed)
        Supexec(atari_timerc_uninstall_super);   // restore etv_timer ($400)
#endif
#if defined(ALIS_NATIVE_16BPP) || defined(ALIS_NATIVE_PLANAR)
    Supexec(atari_vbl_remove_super);             // unhook the VBL cursor mover
#endif

    atari_ikbd_deinit();   // restore the original keyboard vector

    sys_delay(20);

#if defined(ALIS_DSP_MIXER)
    if (atari_timera_installed)
        Supexec(atari_timera_uninstall_super);   // stop the 50 Hz feed ISR first
    if (atari_dma_available)
        atari_dma_sound_close();   // STE/TT: stop the DMA loop + free the ST-RAM ring
    else if (dsp_mixer_available)
        dsp_mixer_close();     // hand the codec/DSP back before tearing down audio
#endif

    sys_deinit_psg();

    free(audio_spec);

    restore_video_state();
    // Did the restore actually land? Compare against the "Saved video state" line at startup.
    dbglog("video restored: physbase=%p logbase=%p mode=0x%lx (saved base=%p mode=0x%lx)\n",
           (void *)Physbase(), (void *)Logbase(),
           (machine_type == MCH_FALCON) ? (unsigned long)VsetMode(-1) : 0UL,
           old_physbase, (unsigned long)old_vmode);
    Vsync();                 // SDL1 order: restoreMode -> vsync -> palette
    gem_restore_palette();   // VDI logical palette, while the workstation is still open
    free_screen_buffers();

    // From here the etv_term handler must do nothing but hand off down the chain.
    atari_cleanup_done = 1;

    dbglog("sys_deinit complete, total render calls=%d\n", (int)render_calls);
    // logclose() is in sys_restore_desktop, called by main() after this.
}

// ============================================================================
#pragma mark - Mouse cursor
// ============================================================================

// Decode alis.desmouse into mouse_pixels[] (color indices; 0 = transparent).
// Ported from sys_sdl1.c sys_dirty_mouse.
void sys_dirty_mouse(void) {

    if (alis.desmouse == NULL)
        return;

    u8  type   = alis.desmouse[0];
    u16 width  = read16(alis.desmouse + 2) + 1;
    u16 height = read16(alis.desmouse + 4) + 1;
    if (width > 16)  width = 16;
    if (height > 16) height = 16;
    cursor_w = width;
    cursor_h = height;

#if defined(ALIS_NATIVE_PLANAR)
    // A cursor converted to interleaved planar at load (convert_sprites_inplace) already has the
    // layout the word-at-a-time draw wants: copy its leftmost chunk. Per-resource test: on DOS
    // data only some scripts are converted; others take the decode below.
    { extern int sprite_is_planar(const u8 *bitmap);
    if (sprite_is_planar(alis.desmouse)) {
        const u16 *pd = (const u16 *)(alis.desmouse + 8);
        s32 cpr = (cursor_w + 15) >> 4;          // = 1 for a <=16px cursor
        // sp plane-words/chunk (4 for a 16-colour game, else 8); zero-fill the rest.
        int sp = (alis.desmouse[6] == 4) ? 4 : 8;
        int hh = cursor_h > 16 ? 16 : cursor_h;  // cursor_planar is [16][8] — never overrun it
        for (int y = 0; y < hh; y++) {
            for (int p = 0; p < sp; p++)
                cursor_planar[y][p] = pd[(s32)y * cpr * sp + p];
            for (int p = sp; p < 8; p++)
                cursor_planar[y][p] = 0;
        }
        return;
    } }
#endif

    switch (type)
    {
        case 0x00:
        case 0x02:
        {
            s8 palidx = 0;
            if (image.flinepal) {
                s16 *palentry = image.firstpal;
                for (int i = 0; i < 3; i++) {
                    if (palentry[0] == 0xff) break;
                    palidx = palentry[2];
                    palentry += 2 + (sizeof(u8 *) >> 1);
                }
            }
            u8 clear = type == 0 ? 0 : (u8)-1;
            u8 *at = alis.desmouse + 6;
            s32 px = 0;
            for (s32 h = 0; h < height; h++)
                for (s32 w = 0; w < width; w++, px++) {
                    u8 color = *(at + (w / 2) + h * (width / 2));
                    color = (w % 2 == 0) ? ((color & 0xf0) >> 4) : (color & 0x0f);
                    mouse_pixels[px] = color == clear ? 0 : color + palidx;
                }
            break;
        }
        case 0x10:
        case 0x12:
        {
            u8 palidx = alis.desmouse[6];
            u8 clear  = alis.desmouse[0] == 0x10 ? alis.desmouse[7] : (u8)-1;
            u8 *at = alis.desmouse + 8;
            s32 px = 0;
            for (s32 h = 0; h < height; h++)
                for (s32 w = 0; w < width; w++, px++) {
                    u8 color = *(at + (w / 2) + h * (width / 2));
                    color = (w % 2 == 0) ? ((color & 0xf0) >> 4) : (color & 0x0f);
                    mouse_pixels[px] = color == clear ? 0 : color + palidx;
                }
            break;
        }
        case 0x14:
        case 0x16:
        {
            u8 clear = 0;
            if (alis.platform.uid == EGameIshar_2)
                clear = alis.desmouse[0] == 0x14 ? alis.desmouse[7] : (u8)-1;
            u8 *at = alis.desmouse + 8;
            for (int px = 0; px < width * height; px++) {
                u8 color = at[px];
                mouse_pixels[px] = color == clear ? 0 : color;
            }
            break;
        }
        default:
            break;
    }

#if defined(ALIS_NATIVE_PLANAR)
    // Pre-convert the decoded cursor to 8-plane words (drawn word-at-a-time by sys_mouse_draw).
    for (int y = 0; y < cursor_h; y++) {
        u16 cw[8] = { 0 };
        for (int x = 0; x < cursor_w; x++) {
            u8 idx = mouse_pixels[y * cursor_w + x];
            for (int p = 0; p < 8; p++) cw[p] |= (u16)(((idx >> p) & 1) << (15 - x));
        }
        for (int p = 0; p < 8; p++) cursor_planar[y][p] = cw[p];
    }
#endif
}

// ============================================================================
#pragma mark - Renderer sync (no-op, single threaded)
// ============================================================================

void sys_lock_renderer(void) {}
void sys_unlock_renderer(void) {}

#endif // ALIS_USE_NATIVE_ATARI
