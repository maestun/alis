// =========================================================================
// Atari STE / Mega STE / TT030 / Falcon 8-bit DMA Sound — host output backend
// =========================================================================
// See sys_atari_dma_sound.h for the design.

#include "sys_atari_dma_sound.h"

#if defined(__atarist__) || defined(__TOS__)

#include <string.h>
#include <stdio.h>
#include <mint/osbind.h>

#include "audio.h"

// 8-bit signed PCM producer (shared with the SDL path). Fills `len` bytes of
// `stream` with freshly mixed audio (music sequencer tick + SFX scan).
extern void sys_audio_callback_S8(void *userdata, u8 *stream, s32 len);

// ---- Falcon XBIOS sound calls (not in this mint's osbind.h) ---------------
static inline long Locksnd(void) {
    register long r __asm__("d0");
    __asm__ volatile("move.w #128,-(%%sp)\n\ttrap #14\n\taddq.l #2,%%sp"
                     : "=r"(r) : : "d1","d2","a0","a1","a2","cc","memory");
    return r;
}
static inline long Unlocksnd(void) {
    register long r __asm__("d0");
    __asm__ volatile("move.w #129,-(%%sp)\n\ttrap #14\n\taddq.l #2,%%sp"
                     : "=r"(r) : : "d1","d2","a0","a1","a2","cc","memory");
    return r;
}
static inline long Sndstatus(short reset) {
    register long r __asm__("d0");
    __asm__ volatile("move.w %1,-(%%sp)\n\tmove.w #140,-(%%sp)\n\ttrap #14\n\taddq.l #4,%%sp"
                     : "=r"(r) : "r"(reset) : "d1","d2","a0","a1","a2","cc","memory");
    return r;
}
static inline long Setmode(short mode) {
    register long r __asm__("d0");
    __asm__ volatile("move.w %1,-(%%sp)\n\tmove.w #132,-(%%sp)\n\ttrap #14\n\taddq.l #4,%%sp"
                     : "=r"(r) : "r"(mode) : "d1","d2","a0","a1","a2","cc","memory");
    return r;
}
static inline long Devconnect(short src, short dst, short srcclk, short prescale, short protocol) {
    register long r __asm__("d0");
    __asm__ volatile("move.w %5,-(%%sp)\n\tmove.w %4,-(%%sp)\n\tmove.w %3,-(%%sp)\n\t"
                     "move.w %2,-(%%sp)\n\tmove.w %1,-(%%sp)\n\tmove.w #139,-(%%sp)\n\t"
                     "trap #14\n\tlea 12(%%sp),%%sp"
                     : "=r"(r) : "r"(src),"r"(dst),"r"(srcclk),"r"(prescale),"r"(protocol)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return r;
}
static inline long Setbuffer(short region, void *start, void *end) {
    register long r __asm__("d0");
    __asm__ volatile("move.l %3,-(%%sp)\n\tmove.l %2,-(%%sp)\n\tmove.w %1,-(%%sp)\n\t"
                     "move.w #131,-(%%sp)\n\ttrap #14\n\tlea 12(%%sp),%%sp"
                     : "=r"(r) : "r"(region),"r"(start),"r"(end)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return r;
}
static inline long Buffoper(short mode) {
    register long r __asm__("d0");
    __asm__ volatile("move.w %1,-(%%sp)\n\tmove.w #136,-(%%sp)\n\ttrap #14\n\taddq.l #4,%%sp"
                     : "=r"(r) : "r"(mode) : "d1","d2","a0","a1","a2","cc","memory");
    return r;
}

// ---- STE / TT DMA sound registers (byte-wide, supervisor space) ----------
#define DMA_CTRL    (*(volatile u8 *)0xFFFF8901UL)  // b0 = play, b1 = loop
#define DMA_START_H (*(volatile u8 *)0xFFFF8903UL)  // frame start (24-bit)
#define DMA_START_M (*(volatile u8 *)0xFFFF8905UL)
#define DMA_START_L (*(volatile u8 *)0xFFFF8907UL)
#define DMA_CNT_H   (*(volatile u8 *)0xFFFF8909UL)  // current play addr (RO)
#define DMA_CNT_M   (*(volatile u8 *)0xFFFF890BUL)
#define DMA_CNT_L   (*(volatile u8 *)0xFFFF890DUL)
#define DMA_END_H   (*(volatile u8 *)0xFFFF890FUL)  // frame end (24-bit)
#define DMA_END_M   (*(volatile u8 *)0xFFFF8911UL)
#define DMA_END_L   (*(volatile u8 *)0xFFFF8913UL)
#define DMA_MODE    (*(volatile u8 *)0xFFFF8921UL)  // b1-0 rate, b7 = mono

#define DMA_NFRAMES  4          // ring length in ~20 ms frames (≈60 ms ahead)

int atari_dma_available = 0;

// Falcon crossbar (only programmed on Falcon — STE/TT wire DMA→DAC directly)
#define FAL_8920    (*(volatile u8  *)0xFFFF8920UL)  // DAC track enable
#define FAL_8930    (*(volatile u16 *)0xFFFF8930UL)  // source / clock select
#define FAL_8932    (*(volatile u16 *)0xFFFF8932UL)  // destination source-select
#define FAL_8935    (*(volatile u8  *)0xFFFF8935UL)  // output prescale (rate)
#define FAL_8937    (*(volatile u8  *)0xFFFF8937UL)  // matrix/PSG enable

static u8  *dma_buf      = NULL;   // ST-RAM ring buffer
static u32  dma_total    = 0;      // ring size in bytes
static u32  dma_frame    = 0;      // refill chunk in bytes (one 50 Hz frame)
static u32  dma_write    = 0;      // producer offset (always frame-aligned)
static u8   dma_mode     = 0;      // value written to DMA_MODE ($8921)
static u8   dma_is_falcon = 0;     // Falcon: route the crossbar, rate via $8935
static u8   dma_prescale = 3;      // Falcon $8935 prescale → 98340/(presc+1) Hz

// Map a requested rate to the nearest hardware-legal DMA rate bits.
//   00 = 6258 Hz, 01 = 12517 Hz, 10 = 25033 Hz, 11 = 50066 Hz
static u8 dma_rate_bits(int rate)
{
    if (rate >= 37550) return 3;       // → 50066
    if (rate >= 18775) return 2;       // → 25033
    if (rate >=  9388) return 1;       // → 12517
    return 0;                          // → 6258
}

// ---- supervisor thunks (hardware regs live in $FFFFxxxx) ------------------

// STE / Mega STE / TT: program the DMA registers directly (DMA→DAC is wired,
// $8921 carries rate + mono). The Falcon goes through XBIOS instead (below).
static long dma_start_super(void)
{
    u32 base = (u32)dma_buf;
    u32 end  = base + dma_total;

    DMA_CTRL    = 0;                                  // stop while we set up
    DMA_MODE    = dma_mode;                           // rate + mono in $8921
    DMA_START_H = (u8)(base >> 16); DMA_START_M = (u8)(base >> 8); DMA_START_L = (u8)base;
    DMA_END_H   = (u8)(end  >> 16); DMA_END_M   = (u8)(end  >> 8); DMA_END_L   = (u8)end;
    DMA_CTRL    = 0x03;                               // play + loop
    return 0;
}

// Falcon: enable the YM2149 (PSG) mix into the DAC ($8937 bit0) on top of the
// XBIOS-configured matrix. Runs in supervisor (the XBIOS setup is user mode).
static u8 fal_mode_read;

static long fal_psg_on_super(void)
{
    FAL_8937 = 0x03;                                  // matrix (bit1) + PSG (bit0) → DAC
    fal_mode_read = DMA_MODE;
    return 0;
}

static long dma_stop_super(void)
{
    DMA_CTRL = 0;                                     // stop playback
    return 0;
}

static u8 dma_isr_driven = 0;     // 1 = refill runs from the 50 Hz Timer-A ISR

// Producer: keep the ring filled to within one frame of the play cursor. MUST
// run in supervisor (reads the DMA address counter at $FFFF89xx). The music
// sequencer tick happens inside sys_audio_callback_S8, so this MUST be paced at
// a steady 50 Hz (Timer-A ISR) — pacing it off the main loop slaves the music
// tempo to the frame rate.
static void dma_refill(void)
{
    u32 base = (u32)dma_buf;
    u32 cur  = ((u32)DMA_CNT_H << 16) | ((u32)DMA_CNT_M << 8) | (u32)DMA_CNT_L;
    u32 cur_off = cur - base;
    if (cur_off >= dma_total) cur_off = 0;            // guard vs a transient read

    // filled_ahead = bytes already mixed that the cursor has not yet played.
    // Fill frames while we can do so without lapping the cursor, leaving one
    // frame of margin so the hardware always has fresh data in front of it.
    for (int guard = 0; guard < DMA_NFRAMES; guard++)
    {
        u32 filled_ahead = (dma_write + dma_total - cur_off) % dma_total;
        if (filled_ahead > dma_total - 2 * dma_frame)
            break;                                    // buffer full enough

        sys_audio_callback_S8(NULL, dma_buf + dma_write, (s32)dma_frame);
        dma_write += dma_frame;
        if (dma_write >= dma_total) dma_write -= dma_total;
    }
}

static long dma_refill_super(void) { dma_refill(); return 0; }

// ---- public API -----------------------------------------------------------

int atari_dma_sound_init(int rate, int channels, int is_falcon)
{
    if (channels < 1) channels = 1;
    if (channels > 2) channels = 2;

    // One 50 Hz frame, rounded up to an even byte count (DMA addrs must be even).
    u32 frame = (u32)((rate / 50) * channels);
    frame = (frame + 1) & ~1u;
    if (frame == 0) frame = 2;

    dma_frame = frame;
    dma_total = frame * DMA_NFRAMES;
    dma_write = 0;
    dma_mode  = (u8)(dma_rate_bits(rate) | (channels == 1 ? 0x80 : 0x00));
    dma_is_falcon = (u8)(is_falcon ? 1 : 0);
    // Falcon rate = 98340/(prescale+1) → prescale = 98340/rate - 1 (1,3,7,...).
    {
        int presc = (rate > 0) ? (98340 / rate) - 1 : 3;
        if (presc < 1)  presc = 1;
        if (presc > 11) presc = 11;
        dma_prescale = (u8)presc;
    }

    // DMA can only fetch from ST-RAM (mode 0 = MX_STRAM). Mxalloc returns an
    // even (word-aligned) block, so every frame offset stays even.
    dma_buf = (u8 *)Mxalloc((long)dma_total, 0);
    if (!dma_buf)
        return -1;

    memset(dma_buf, 0, dma_total);                    // start on silence

    long setmode_rc = 0;
    if (dma_is_falcon)
    {
        // Falcon: let XBIOS set up the crossbar + clock (by hand via $8930
        // the wrong clock gets selected: 2x playback rate). Devconnect wires
        // DMA-Play → DAC at 25.175 MHz internal / prescale, Setmode 0 = 8-bit
        // stereo, 2 = 8-bit mono, Buffoper 3 = play+repeat.
        Locksnd();
        Sndstatus(1);                                 // reset the matrix
        setmode_rc = Setmode(channels == 1 ? 2 : 0);
        Devconnect(0 /*DMAPLAY*/, 8 /*DAC*/, 0 /*CLK25M int*/,
                   dma_prescale, 0 /*handshake*/);
        Setbuffer(0 /*SR_PLAY*/, dma_buf, dma_buf + dma_total);
        Buffoper(3);                                  // play + repeat (loop)
        Supexec(fal_psg_on_super);                    // mix YM2149 into the DAC
    }
    else
    {
        Supexec(dma_start_super);                     // STE/TT direct registers
    }
    atari_dma_available = 1;

    FILE *log = fopen("dma_log.txt", "w");
    if (log) {
        if (dma_is_falcon)
            fprintf(log, "Falcon Setmode rc=%ld, $8921 after setup=0x%02X\n", setmode_rc, fal_mode_read);
        fprintf(log, "DMA sound: buf=%p total=%lu frame=%lu mode=0x%02X rate=%d ch=%d "
                "falcon=%d presc=%u xbios=%d\n",
                (void *)dma_buf, (unsigned long)dma_total, (unsigned long)dma_frame,
                (unsigned)dma_mode, rate, channels, (int)dma_is_falcon,
                (unsigned)dma_prescale, dma_is_falcon ? 1 : 0);
        fclose(log);
    }
    return 0;
}

void atari_dma_sound_poll(void)
{
    // No-op once the Timer-A ISR is driving the refill (the normal case) —
    // calling it here too would race the ISR on dma_write. Kept as the path
    // for any build that can't install the 50 Hz timer.
    if (!atari_dma_available || dma_isr_driven) return;
    Supexec(dma_refill_super);
}

// Called from the 50 Hz Timer-A ISR (already supervisor — no Supexec).
void atari_dma_sound_tick_isr(void)
{
    if (!atari_dma_available) return;
    dma_refill();
}

void atari_dma_sound_set_isr_driven(int on)
{
    dma_isr_driven = (u8)(on ? 1 : 0);
}

void atari_dma_sound_close(void)
{
    if (!atari_dma_available) return;
    atari_dma_available = 0;
    if (dma_is_falcon)
    {
        Buffoper(0);          // stop play
        Unlocksnd();          // release the audio subsystem
    }
    else
    {
        Supexec(dma_stop_super);
    }
    if (dma_buf) { Mfree(dma_buf); dma_buf = NULL; }
}

#endif /* __atarist__ || __TOS__ */
