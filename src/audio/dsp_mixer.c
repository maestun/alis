// =========================================================================
// DSP56001 Audio Mixer — Host-side for Falcon (SSI direct output)
// =========================================================================
//
// DSP outputs directly to the DAC via SSI. CPU sends channel params + sample
// windows via the host port.
//

#include "config.h"

#if defined(ALIS_DSP_MIXER) && (defined(__atarist__) || defined(__TOS__))

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <SDL/SDL.h>
#include <mint/basepage.h>
#include "dsp_mixer.h"
#include "audio.h"
#include "channel.h"

// The feed reads audio.channels[] directly; no SDL audio callback on this path.
extern sAudio audio;
extern void sys_psg_tick(void);   // YM tone-effect tick (sys.c) — run on the DSP path

// Default to the Simplet tracker; the hand-written SFX mixer is opt-in (ALIS_DSP_USE_SFXMIXER).
#if !defined(ALIS_DSP_USE_SFXMIXER) && !defined(ALIS_DSP_USE_TRACKER)
#define ALIS_DSP_USE_TRACKER
#endif

// ---- Falcon DSP host interface registers (supervisor mode access) ----
// NB: data bytes are at $FFA205-$FFA207, NOT $FFA204-$FFA206!
// Writing the LOW byte ($FFA207) triggers the actual transfer.
#define HI_ICR      (*(volatile u8  *)0xFFA200)
#define HI_CVR      (*(volatile u8  *)0xFFA201)
#define HI_ISR      (*(volatile u8  *)0xFFA202)
#define HI_DATA_H   (*(volatile u8  *)0xFFA205)
#define HI_DATA_M   (*(volatile u8  *)0xFFA206)
#define HI_DATA_L   (*(volatile u8  *)0xFFA207)

#define HI_ISR_RXDF  0x01
#define HI_ISR_TXDE  0x02

// Falcon audio matrix
#define FALCON_CROSSBAR   (*(volatile u16 *)0xFFF8932)
#define FALCON_DMA_CTRL   (*(volatile u8  *)0xFFF8901)

#define DSP_NUM_CHANNELS 8
#define DSP_MAX_WINDOW   28   // Fits in internal Y, matches num_samples=32 limit

u8 dsp_mixer_available = 0;

// Which DSP program is currently resident. Sampled music and OPL2 music are
// mutually exclusive, so the DSP holds ONE program at a time and we hot-swap it
// when the music type changes (sampled/chip → tracker, OPL → FM+SFX).
#define DSPPROG_NONE    0
#define DSPPROG_TRACKER 1
#define DSPPROG_FM      2
static int dsp_current_prog = DSPPROG_NONE;
static int dsp_setup_done   = 0;      // Dsp_Lock/Locksnd/Mxalloc done once (not per swap)
static int dsp_dac_muted    = 0;      // FM mode: DAC muted only during a program swap
static int fm_silenced      = 0;      // FM mode: slots muted because OPL music stopped
static void dsp_opl_fm_silence(void); // defined with the FM loaders
extern void mv2_opl2rout(void);       // OPL2-music soundrout (music_v2.c) — the FM trigger
static void dsp_audio_select_for_music(void);   // defined after the loaders

// ---- XBIOS calls (supervisor mode handled by trap) ----
static inline long Dsp_GetWordSize(void) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w #103,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #2,%%sp" : "=r"(retval) : : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
static inline long Dsp_Lock(void) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w #104,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #2,%%sp" : "=r"(retval) : : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
static inline void Dsp_Unlock(void) {
    __asm__ volatile("move.w #105,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #2,%%sp" : : : "d0","d1","d2","a0","a1","a2","cc","memory");
}
static inline void Dsp_ExecProg(const void *codeptr, long codesize, long ability) {
    __asm__ volatile("move.w %2,-(%%sp)\n\t"
                     "move.l %1,-(%%sp)\n\t"
                     "move.l %0,-(%%sp)\n\t"
                     "move.w #109,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 12(%%sp),%%sp"
                     : : "r"(codeptr), "r"(codesize), "r"((short)ability)
                     : "d0","d1","d2","a0","a1","a2","cc","memory");
}
// Dsp_Reserve (XBIOS #107): reserve X and Y memory for our DSP program.
// Must be called between Dsp_Lock and Dsp_ExecProg — without it
// Dsp_ExecProg may silently refuse to load. Values are in 24-bit words.
static inline long Dsp_Reserve(long xreserve, long yreserve) {
    register long retval __asm__("d0");
    __asm__ volatile("move.l %2,-(%%sp)\n\t"
                     "move.l %1,-(%%sp)\n\t"
                     "move.w #107,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 10(%%sp),%%sp" : "=r"(retval) : "r"(xreserve), "r"(yreserve)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Dsp_ExecBoot: load bootstrap program (XBIOS #110)
// codeptr = byte array (3 bytes per word), codesize = word count, ability = short
static inline void Dsp_ExecBoot(const void *codeptr, long codesize, short ability) {
    __asm__ volatile("move.w %2,-(%%sp)\n\t"
                     "move.l %1,-(%%sp)\n\t"
                     "move.l %0,-(%%sp)\n\t"
                     "move.w #110,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 12(%%sp),%%sp"
                     : : "r"(codeptr), "r"(codesize), "r"(ability)
                     : "d0","d1","d2","a0","a1","a2","cc","memory");
}
// Dsp_DoBlock: bidirectional block transfer (XBIOS #96) — 3-byte packed format
static inline void Dsp_DoBlock(const void *data_in, long size_in, void *data_out, long size_out) {
    __asm__ volatile("move.l %3,-(%%sp)\n\t"
                     "move.l %2,-(%%sp)\n\t"
                     "move.l %1,-(%%sp)\n\t"
                     "move.l %0,-(%%sp)\n\t"
                     "move.w #96,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 18(%%sp),%%sp"
                     : : "r"(data_in), "r"(size_in), "r"(data_out), "r"(size_out)
                     : "d0","d1","d2","a0","a1","a2","cc","memory");
}
// Dsp_BlkUnpacked: transfer using 32-bit longs (XBIOS #98)
// Same format as MP2 player uses. One long per DSP word.
// Can be called with size_in=0 or size_out=0 for one-directional transfers.
static inline void Dsp_BlkUnpacked(const long *data_in, long size_in, long *data_out, long size_out) {
    __asm__ volatile("move.l %3,-(%%sp)\n\t"
                     "move.l %2,-(%%sp)\n\t"
                     "move.l %1,-(%%sp)\n\t"
                     "move.l %0,-(%%sp)\n\t"
                     "move.w #98,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 18(%%sp),%%sp"
                     : : "r"(data_in), "r"(size_in), "r"(data_out), "r"(size_out)
                     : "d0","d1","d2","a0","a1","a2","cc","memory");
}
// Dsp_BlkWords: bidirectional block transfer (XBIOS #123) — 32-bit long format
// Each DSP word is one long (24-bit value in lower bits).
static inline void Dsp_BlkWords(const long *data_in, long size_in, long *data_out, long size_out) {
    __asm__ volatile("move.l %3,-(%%sp)\n\t"
                     "move.l %2,-(%%sp)\n\t"
                     "move.l %1,-(%%sp)\n\t"
                     "move.l %0,-(%%sp)\n\t"
                     "move.w #123,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 18(%%sp),%%sp"
                     : : "r"(data_in), "r"(size_in), "r"(data_out), "r"(size_out)
                     : "d0","d1","d2","a0","a1","a2","cc","memory");
}
// Supexec: run function in supervisor mode (XBIOS #38)
static inline void Supexec(void (*func)(void)) {
    __asm__ volatile("move.l %0,-(%%sp)\n\t"
                     "move.w #38,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #6,%%sp"
                     : : "r"(func)
                     : "d0","d1","d2","a0","a1","a2","cc","memory");
}

// ---- XBIOS audio matrix calls (MP2/DUMP set_matrix sequence) ----
// Locksnd (XBIOS #128): take exclusive ownership of the audio subsystem
static inline long Locksnd(void) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w #128,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #2,%%sp" : "=r"(retval) : : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Sndstatus (XBIOS #140): SND_RESET=1 → reset the audio matrix
static inline long Sndstatus(short reset) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w %1,-(%%sp)\n\t"
                     "move.w #140,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #4,%%sp" : "=r"(retval) : "r"(reset) : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Setmode (XBIOS #132): STEREO16=1 → 16-bit stereo
static inline long Setmode(short mode) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w %1,-(%%sp)\n\t"
                     "move.w #132,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #4,%%sp" : "=r"(retval) : "r"(mode) : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Soundcmd (XBIOS #130): cmd=ADDERIN(4) data=MATIN(2)
static inline long Soundcmd(short cmd, short data) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w %2,-(%%sp)\n\t"
                     "move.w %1,-(%%sp)\n\t"
                     "move.w #130,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #6,%%sp" : "=r"(retval) : "r"(cmd), "r"(data) : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Dsptristate (XBIOS #137): tristate DSP send/receive to enable matrix routing
static inline long Dsptristate(short dspxmit, short dsprecv) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w %2,-(%%sp)\n\t"
                     "move.w %1,-(%%sp)\n\t"
                     "move.w #137,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #6,%%sp" : "=r"(retval) : "r"(dspxmit), "r"(dsprecv) : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Unlocksnd (XBIOS #129): release the audio subsystem
static inline long Unlocksnd(void) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w #129,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #2,%%sp" : "=r"(retval) : : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Devconnect (XBIOS #139): wire a source into the matrix (clock + frame
// sync + destination). src 0=DMA-Play, 1=DSP-XMT, 2=Ext-In, 3=ADC.
// dst is a bitmask: 1=DMA-Rec, 2=DSP-RX, 4=Ext-Out, 8=DAC. srcclk:
// 0=25.175MHz internal, 1=Ext, 2=32MHz internal. prescaler: 0..11.
// protocol: 0=Atari handshake. Adds the source non-destructively.
static inline long Devconnect(short src, short dst, short srcclk, short prescale, short protocol) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w %5,-(%%sp)\n\t"
                     "move.w %4,-(%%sp)\n\t"
                     "move.w %3,-(%%sp)\n\t"
                     "move.w %2,-(%%sp)\n\t"
                     "move.w %1,-(%%sp)\n\t"
                     "move.w #139,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 12(%%sp),%%sp" : "=r"(retval)
                     : "r"(src), "r"(dst), "r"(srcclk), "r"(prescale), "r"(protocol)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Setbuffer (XBIOS #131): point a DMA play/record frame at a memory range
// region: 0=playback (SR_PLAY), 1=record (SR_RECORD)
static inline long Setbuffer(short region, void *startaddr, void *endaddr) {
    register long retval __asm__("d0");
    __asm__ volatile("move.l %3,-(%%sp)\n\t"
                     "move.l %2,-(%%sp)\n\t"
                     "move.w %1,-(%%sp)\n\t"
                     "move.w #131,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "lea 12(%%sp),%%sp" : "=r"(retval) : "r"(region), "r"(startaddr), "r"(endaddr)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Buffoper (XBIOS #136): control DMA play/record engines
// mode bits: 1=play enable, 2=play repeat, 4=record enable, 8=record repeat
static inline long Buffoper(short mode) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w %1,-(%%sp)\n\t"
                     "move.w #136,-(%%sp)\n\t"
                     "trap #14\n\t"
                     "addq.l #4,%%sp" : "=r"(retval) : "r"(mode) : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Mxalloc (GEMDOS #68 / TOS 1.04+ / FreeMiNT): allocate memory with type
// mode: 0=ST-RAM, 1=TT-RAM, 2=ST-RAM preferred, 3=TT-RAM preferred
static inline void *sys_Mxalloc(long amount, short mode) {
    register long retval __asm__("d0");
    __asm__ volatile("move.w %2,-(%%sp)\n\t"
                     "move.l %1,-(%%sp)\n\t"
                     "move.w #68,-(%%sp)\n\t"
                     "trap #1\n\t"
                     "addq.l #8,%%sp" : "=r"(retval) : "r"(amount), "r"(mode)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return (void *)retval;
}
// Mfree (GEMDOS #73 / $49): free memory previously allocated by Malloc/Mxalloc/Pexec.
static inline long sys_Mfree(void *addr) {
    register long retval __asm__("d0");
    __asm__ volatile("move.l %1,-(%%sp)\n\t"
                     "move.w #73,-(%%sp)\n\t"
                     "trap #1\n\t"
                     "addq.l #6,%%sp" : "=r"(retval) : "r"(addr)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}
// Pexec (GEMDOS #75 / $4B): load/run a TOS program.
// mode 3 = load + relocate + don't run main → returns BASEPAGE *
static inline long sys_Pexec(short mode, const char *file, const char *cmd, const char *env) {
    register long retval __asm__("d0");
    __asm__ volatile("move.l %4,-(%%sp)\n\t"
                     "move.l %3,-(%%sp)\n\t"
                     "move.l %2,-(%%sp)\n\t"
                     "move.w %1,-(%%sp)\n\t"
                     "move.w #75,-(%%sp)\n\t"
                     "trap #1\n\t"
                     "lea 16(%%sp),%%sp" : "=r"(retval)
                     : "r"(mode), "r"(file), "r"(cmd), "r"(env)
                     : "d1","d2","a0","a1","a2","cc","memory");
    return retval;
}

// ---- Host port I/O (called from supervisor mode only) ----
// GCC m68k -O3 can miscompile functions with volatile I/O + loops.
// Force O0 on all supervisor-mode DSP functions.
#define DSP_FUNC __attribute__((noinline, optimize("O0")))

static inline void hi_write(long val) {
    while (!(HI_ISR & HI_ISR_TXDE)) ;
    HI_DATA_H = (u8)((u32)val >> 16);
    HI_DATA_M = (u8)((u32)val >> 8);
    HI_DATA_L = (u8)((u32)val);
}

static inline long hi_read(void) {
    while (!(HI_ISR & HI_ISR_RXDF)) ;
    long val = ((long)(s8)HI_DATA_H << 16) | ((u32)HI_DATA_M << 8) | HI_DATA_L;
    return val;
}

// ---- DSP program binary ----
#if __has_include("dsp_mixer_bin.h")
#include "dsp_mixer_bin.h"
#else
static const u8 dsp_mixer_program[] = {0};
static const u32 dsp_mixer_program_size = 0;
#endif

// Simplet's DSP Soundtracker (ABSTRACT/DHS). HCV-driven 2+2x voice mixer.
// Sections at P:$0 (jmp Start), $10 (SSI Tx ISR), $26/$28 (HCV vectors),
// $40 (main). lod_to_h.py merges into a single blob starting at P:$0000,
// so we bootstrap with load address $0000 (vs $40 for dsp_mixer_program).
#if __has_include("dsp_tracker_bin.h")
#include "dsp_tracker_bin.h"
#else
static const u8 dsp_tracker_program[] = {0};
static const u32 dsp_tracker_program_size = 0;
#endif

// Program installed by dsp_bootstrap_super (set before calling it).
static const u8  *g_boot_program      = dsp_mixer_program;
static u32        g_boot_program_size = 0;  // set in init based on which path
static u16        g_boot_load_addr    = 0x0040;

// NoCrew DSP bootstrap loader (extracted from dump171/src/dsp_fix.s).
// 512 DSP words = 1536 bytes. The Falcon boot ROM loads this at P:$0000
// and jumps to its loader at P:$0040 which then reads the user binary
// from the host port and runs it. Same blob DUMP and MP2 use.
#include "dsp_nocrew_bootstrap.h"

// ---- Channel state ----
typedef struct {
    u8   active;
    s8  *address;
    u32  length;
    u32  played;
    u16  freq;
    s16  volume;
    u8   loop;
} sDSPChannel;

static sDSPChannel dsp_channels[DSP_NUM_CHANNELS];

// Debug log
static FILE *dsp_mixlog = NULL;
static int dsp_mix_calls = 0;
static int dsp_poll_calls = 0;

#if defined(ALIS_DSP_USE_SFXMIXER)   // legacy SFX mixer: host<->DSP block transfers
// ---- Send buffer for supervisor-mode transfer ----
#define DSP_SENDBUF_MAX 8192
static long dsp_sendbuf[DSP_SENDBUF_MAX];
static long dsp_send_count = 0;

// ---- Supervisor-mode transfer routine ----
// Sends dsp_send_count words. If num_samples > 0, reads back:
//   num_samples mixed output words → dsp_recvbuf[0..num_samples-1]
//   8 position words → dsp_recvbuf[num_samples..num_samples+7]
static long dsp_recvbuf[2048];
static long dsp_recv_count = 0;

static volatile long dsp_xfer_status = 0;  // 0=idle, 1=sending, 2=receiving, -1=TX timeout, -2=RX timeout
static volatile long dsp_xfer_progress = 0;

// Transfer using 32-bit long access to $FFA204 (NoCrew/MP2 player method)
// This is atomic and works correctly in Hatari unlike byte-by-byte access.
DSP_FUNC static void dsp_transfer_32bit(void) {
    volatile u8  *isr  = (volatile u8  *)0xFFA202;
    volatile u32 *data = (volatile u32 *)0xFFA204;

    long *sp = dsp_sendbuf;
    long count = dsp_send_count;
    long num_samples = sp[0];

    dsp_xfer_status = 1;

    // Send all data to DSP via 32-bit writes
    for (long i = 0; i < count; i++) {
        long timeout = 500000;
        while (!(*isr & 0x02)) {
            if (--timeout == 0) { dsp_xfer_status = -1; dsp_xfer_progress = i; return; }
        }
        *data = (u32)sp[i];
    }

    dsp_xfer_status = 2;

    // Read back via 32-bit reads
    if (num_samples > 0) {
        long total_recv = num_samples + DSP_NUM_CHANNELS;
        if (total_recv > 2048) total_recv = 2048;
        for (long i = 0; i < total_recv; i++) {
            long timeout = 500000;
            while (!(*isr & 0x01)) {
                if (--timeout == 0) { dsp_xfer_status = -2; dsp_xfer_progress = i; dsp_recv_count = i; return; }
            }
            u32 v = *data;
            // Sign-extend 24-bit value (bit 23 is sign)
            dsp_recvbuf[i] = (v & 0x800000) ? (long)(v | 0xFF000000) : (long)v;
        }
        dsp_recv_count = total_recv;
    }
    dsp_xfer_status = 0;
}

// TX-only: send data, don't wait for response
DSP_FUNC static void dsp_tx_only_super(void) {
    volatile u8 *isr = (volatile u8 *)0xFFA202;
    volatile u8 *dh  = (volatile u8 *)0xFFA205;
    volatile u8 *dm  = (volatile u8 *)0xFFA206;
    volatile u8 *dl  = (volatile u8 *)0xFFA207;

    long *sp = dsp_sendbuf;
    long count = dsp_send_count;

    for (long i = 0; i < count; i++) {
        long timeout = 500000;
        while (!(*isr & 0x02)) {
            if (--timeout == 0) { dsp_xfer_status = -1; dsp_xfer_progress = i; return; }
        }
        *dh = (u8)((u32)sp[i] >> 16);
        *dm = (u8)((u32)sp[i] >> 8);
        *dl = (u8)((u32)sp[i]);
    }
    dsp_xfer_status = 0;
    dsp_xfer_progress = count;
}

// RX-only: read response from DSP
DSP_FUNC static void dsp_rx_only_super(void) {
    volatile u8 *isr = (volatile u8 *)0xFFA202;
    volatile u8 *dh  = (volatile u8 *)0xFFA205;
    volatile u8 *dm  = (volatile u8 *)0xFFA206;
    volatile u8 *dl  = (volatile u8 *)0xFFA207;

    long total_recv = dsp_recv_count;  // expected count, set by caller
    for (long i = 0; i < total_recv; i++) {
        long timeout = 500000;
        while (!(*isr & 0x01)) {
            if (--timeout == 0) { dsp_xfer_status = -2; dsp_xfer_progress = i; dsp_recv_count = i; return; }
        }
        long h = *dh, m = *dm, l = *dl;
        dsp_recvbuf[i] = (h << 16) | (m << 8) | l;
    }
    dsp_xfer_status = 0;
}

#endif

// ---- Audio matrix ----
// Snapshot of $FF8930/32/34 for the init log (dsp_read_matrix_super).
static volatile u16 dsp_8930 = 0;
static volatile u16 dsp_8932 = 0;
static volatile u16 dsp_8934 = 0;

DSP_FUNC static void dsp_read_matrix_super(void) {
    dsp_8930 = *(volatile u16 *)0xFFFF8930;
    dsp_8932 = *(volatile u16 *)0xFFFF8932;
    dsp_8934 = *(volatile u16 *)0xFFFF8934;
}

static volatile u8 *dsp_dma_rec_buf_addr = NULL;

// SFX-mixer matrix: DUMP's (dspmod_init) values + a dummy DMA-record to keep
// the matrix clock running.
DSP_FUNC static void dsp_dump_matrix_setup_super(void) {
    u32 start = (u32)(uintptr_t)dsp_dma_rec_buf_addr;
    u32 end   = start + 0x1000;

    *(volatile u16 *)0xFFFF8930 = 0x0090;   // source side — DUMP-observed value
    *(volatile u16 *)0xFFFF8932 = 0x200b;   // DSP-Send → DAC + matrix
    *(volatile u16 *)0xFFFF8934 = 0x0101;   // prescaler / play+rec sync — DUMP value
    *(volatile u8  *)0xFFFF8900 = 0x00;     // no DMA IRQ
    *(volatile u8  *)0xFFFF8936 = 0x00;     // record 1 track
    *(volatile u8  *)0xFFFF8921 = 0x40;     // 16-bit DMA
    *(volatile u8  *)0xFFFF8901 = 0x80;     // select RECORD frame
    *(volatile u8  *)0xFFFF8907 = (u8)(start);
    *(volatile u8  *)0xFFFF8905 = (u8)(start >> 8);
    *(volatile u8  *)0xFFFF8903 = (u8)(start >> 16);
    *(volatile u8  *)0xFFFF8913 = (u8)(end);
    *(volatile u8  *)0xFFFF8911 = (u8)(end >> 8);
    *(volatile u8  *)0xFFFF890F = (u8)(end >> 16);
    *(volatile u8  *)0xFFFF8901 = 0xb0;     // record enable + repeat
}

// ---- Alive-probe: confirm DSP is actually running our code ----
static volatile long dsp_probe_ready = 0;
static volatile long dsp_probe_value = 0;

DSP_FUNC static void dsp_alive_probe_super(void) {
    volatile u8 *isr = (volatile u8 *)0xFFA202;
    volatile u8 *dh  = (volatile u8 *)0xFFA205;
    volatile u8 *dm  = (volatile u8 *)0xFFA206;
    volatile u8 *dl  = (volatile u8 *)0xFFA207;

    // Wait up to ~1 M loop iterations for RXDF (DSP wrote a word to us).
    long timeout = 1000000;
    while (!(*isr & 0x01)) {                 // RXDF = bit 0 of ISR
        if (--timeout == 0) { dsp_probe_ready = 0; dsp_probe_value = 0; return; }
    }
    long h = *dh, m = *dm, l = *dl;
    dsp_probe_value = (h << 16) | (m << 8) | l;
    dsp_probe_ready = 1;
}

// ---- Direct DSP bootstrap loader (NoCrew method) ----
// Resets DSP via YM2149 Port A, then writes 512 words of program.
// Debug state from bootstrap (set in supervisor, read from user)
static volatile u8  dbg_pa_before = 0;
static volatile u8  dbg_pa_after = 0;
static volatile u8  dbg_isr_before = 0;
static volatile u8  dbg_isr_after_reset = 0;
static volatile u8  dbg_isr_after_write = 0;
static volatile u8  dbg_icr_before = 0;
static volatile u8  dbg_icr_after = 0;
static volatile long dbg_bootstrap_entered = 0;
static volatile long dbg_boot_fail = 0;   // 0=ok, else stage that timed out
static volatile long dsp_echo_result;

DSP_FUNC static void dsp_bootstrap_super(void) {
    volatile u8  *ym_sel = (volatile u8  *)0xFFFF8800;
    volatile u8  *ym_wr  = (volatile u8  *)0xFFFF8802;
    volatile u8  *isr    = (volatile u8  *)0xFFA202;
    volatile u8  *dh     = (volatile u8  *)0xFFA205;
    volatile u8  *dm     = (volatile u8  *)0xFFA206;
    volatile u8  *dl     = (volatile u8  *)0xFFA207;
    volatile u32 *data32 = (volatile u32 *)0xFFA204;

    *(volatile u8 *)0xFFFF8937 = 0;   // mute DAC during DSP reset+reload (matrix re-enables it)
    dbg_bootstrap_entered = 1;
    dbg_boot_fail = 0;
    volatile u8 *icr = (volatile u8 *)0xFFA200;
    dbg_icr_before = *icr;
    dbg_isr_before = *isr;

    // --- NoCrew bootstrap: matches MP2/DUMP working sequence ---

    // 1) Reset DSP via YM2149 Port A bit 4 (1 = RESET, 0 = RUN).
    *ym_sel = 0x0E;
    u8 pa = *ym_sel;
    dbg_pa_before = pa;
    *ym_wr = pa & ~0x10;                   // ensure running
    *ym_wr = (pa & ~0x10) | 0x10;          // hold in reset
    for (volatile int i = 0; i < 20000; i++) ;
    *ym_wr = pa & ~0x10;                   // release — DSP enters host boot
    for (volatile int i = 0; i < 100000; i++) ;
    dbg_isr_after_reset = *isr;
    *ym_sel = 0x0E;
    dbg_pa_after = *ym_sel;

    // 2) Send the 512-word NoCrew bootstrap_code blob. The Atari boot
    //    ROM reads exactly 512 words, loads them at P:$0000, then
    //    JMPs to P:$0000. That code is a custom loader at P:$0040 that
    //    reads a section stream from the host port next.
    for (int w = 0; w < 512; w++) {
        long t = 500000;
        while (!(*isr & 0x02)) { if (--t == 0) { dbg_boot_fail = 1; return; } }
        *dh = nocrew_bootstrap_code[w * 3 + 0];
        *dm = nocrew_bootstrap_code[w * 3 + 1];
        *dl = nocrew_bootstrap_code[w * 3 + 2];
    }

    // 3) Send the program (g_boot_*) as one P-memory section. NoCrew section
    //    format: type (0=P, 1=X, 2=Y/L), load address, length in words, data.
    //    Type 3 with no payload = launch.
    #define SEND_WORD(v) do { \
        long _t = 500000; \
        while (!(*isr & 0x02)) { if (--_t == 0) { dbg_boot_fail = 2; return; } } \
        *dh = (u8)(((u32)(v)) >> 16); \
        *dm = (u8)(((u32)(v)) >> 8);  \
        *dl = (u8)(((u32)(v)));       \
    } while (0)

    SEND_WORD(0);                          // section type 0 = P memory
    SEND_WORD(g_boot_load_addr);           // load address (program-specific)
    SEND_WORD(g_boot_program_size);        // word count

    for (u32 w = 0; w < g_boot_program_size; w++) {
        long t = 500000;
        while (!(*isr & 0x02)) { if (--t == 0) { dbg_boot_fail = 3; return; } }
        *dh = g_boot_program[w * 3 + 0];
        *dm = g_boot_program[w * 3 + 1];
        *dl = g_boot_program[w * 3 + 2];
    }

    // 4) Launch: type 3 as a 32-bit write to $FFA204 (as MP2 does), not as
    //    bytes to $FFA205-7; the loader may rely on the long write.
    { long t = 500000;
      while (!(*isr & 0x02)) { if (--t == 0) { dbg_boot_fail = 4; return; } } }
    *data32 = 3;
    #undef SEND_WORD

    dbg_isr_after_write = *isr;
    dbg_icr_after = *icr;
    dbg_bootstrap_entered = 2;
}

// ---- Debug: read ISR from supervisor mode ----
static volatile u8 debug_isr;
DSP_FUNC static void read_isr_super(void) { debug_isr = *(volatile u8 *)0xFFA202; }

// ---- Echo test via direct register access in supervisor mode ----
// Uses NoCrew/dspbench register conventions: $FFA202=ISR, $FFA205-7=data
DSP_FUNC static void echo_test_super(void) {
    volatile u8 *isr = (volatile u8 *)0xFFA202;
    volatile u8 *dh  = (volatile u8 *)0xFFA205;
    volatile u8 *dm  = (volatile u8 *)0xFFA206;
    volatile u8 *dl  = (volatile u8 *)0xFFA207;

    // Send $123456: wait TXDE (bit 1), write H/M/L (L triggers send)
    while (!(*isr & 0x02)) ;
    *dh = 0x12;
    *dm = 0x34;
    *dl = 0x56;

    // Receive: wait RXDF (bit 0), read H/M/L (L acknowledges)
    long timeout = 500000;
    while (!(*isr & 0x01)) {
        if (--timeout == 0) { dsp_echo_result = -1; return; }
    }
    long h = *dh, m = *dm, l = *dl;
    dsp_echo_result = (h << 16) | (m << 8) | l;
}

#if defined(ALIS_DSP_USE_SFXMIXER)
// ---- DSP command sender ----
// Batch send: fill dsp_sendbuf with words, set dsp_send_count, call Supexec once
DSP_FUNC static void dsp_send_batch_super(void) {
    volatile u8  *isr  = (volatile u8  *)0xFFA202;
    volatile u32 *data = (volatile u32 *)0xFFA204;
    long *sp = dsp_sendbuf;
    long count = dsp_send_count;

    for (long i = 0; i < count; i++) {
        long timeout = 500000;
        while (!(*isr & 0x02)) {
            if (--timeout == 0) { dsp_xfer_status = -(i+1); return; }
        }
        *data = (u32)sp[i];
    }
    dsp_xfer_status = 0;
}

// Send a single word
static void dsp_send(long word) {
    dsp_sendbuf[0] = word;
    dsp_send_count = 1;
    Supexec(dsp_send_batch_super);
}

// Send multiple words in one Supexec call
static void dsp_send_batch(long *words, long count) {
    if (count > DSP_SENDBUF_MAX) count = DSP_SENDBUF_MAX;
    memcpy(dsp_sendbuf, words, count * sizeof(long));
    dsp_send_count = count;
    Supexec(dsp_send_batch_super);
}

// ---- Sample slot management ----
#define DSP_NUM_SLOTS 4
#define DSP_SLOT_SIZE 8192  // max samples per slot

typedef struct {
    s8  *address;           // host-side sample address (for comparison)
    u32  length;            // sample length
} sDSPSlot;

static sDSPSlot dsp_slots[DSP_NUM_SLOTS];

// Upload sample data to DSP slot — via Dsp_BlkUnpacked (XBIOS, send-only)
static void dsp_upload_sample(int slot, s8 *data, u32 length) {
    if (slot < 0 || slot >= DSP_NUM_SLOTS) return;
    if (length > DSP_SLOT_SIZE) length = DSP_SLOT_SIZE;
    if (length > DSP_SENDBUF_MAX - 3) length = DSP_SENDBUF_MAX - 3;

    dsp_slots[slot].address = data;
    dsp_slots[slot].length = length;

    // Pack: CMD + slot + length + data
    long *sp = dsp_sendbuf;
    *sp++ = 1;
    *sp++ = slot;
    *sp++ = (long)length;
    for (u32 i = 0; i < length; i++) {
        *sp++ = (s32)data[i] << 16;
    }
    long count = (long)(sp - dsp_sendbuf);

    if (dsp_mixlog) {
        fprintf(dsp_mixlog, "upload: slot=%d len=%lu words=%ld\n",
                slot, (unsigned long)length, count);
        fflush(dsp_mixlog);
    }

    // Send via Dsp_BlkUnpacked (XBIOS handles sync)
    static long dummy_out;
    Dsp_BlkUnpacked(dsp_sendbuf, count, &dummy_out, 0);

    if (dsp_mixlog) {
        fprintf(dsp_mixlog, "  upload done\n");
        fflush(dsp_mixlog);
    }
}

// Find or allocate a slot for a sample
static int dsp_find_slot(s8 *address, u32 length) {
    // Already loaded?
    for (int i = 0; i < DSP_NUM_SLOTS; i++) {
        if (dsp_slots[i].address == address && dsp_slots[i].length == length)
            return i;
    }
    // Find empty slot
    for (int i = 0; i < DSP_NUM_SLOTS; i++) {
        if (dsp_slots[i].address == NULL)
            return i;
    }
    // Evict slot 0 (simple LRU)
    return 0;
}

#endif

// ---- Public API ----

static u8 *g_dma_buf  = NULL;
// =========================================================================
// Simplet DSP Soundtracker host-side feed (ALIS_DSP_USE_TRACKER)
// =========================================================================
// Port of Simplet's SndTrack_IT / Play_Voice host driver; loaded with the
// NoCrew bootstrap (Dsp_ExecProg freezes). Per frame, 32-bit I/O at $FFA204:
//   trigger HCV $26 (CVR = 0x93)
//   push Length (984/492)
//   push Active_Mask (bit v = voice v active this frame)
//   for each active voice, in order 0..7:
//       push Volume      (vol*($7fffff/64*2)/voices)
//       push Frequence   (($800000/49169*428*8363)/period)
//       read sample-byte count from DSP (poll RXDF)
//       advance position by count (loop via repeat_length)
//       push packet count (bytes/6 + 1)
//       per packet push 2 overlapping longwords from (pos-1), advance 6
// The DSP clears the output window and mixes only the active voices.
#ifdef ALIS_DSP_USE_TRACKER

#define TRK_VOICES   8       // voices 0-3 = SFX channels, 4-7 = MV2 music voices
#define TRK_NB_SUP   ((TRK_VOICES - 2) / 2)   // = 3 supplementary pairs

// Output rate. TRK_HALF_RATE=1: 24585 Hz, halving the DSP's SSI load at the cost
// of bandwidth and top pitch. Length, $FF8935 and the period math derive from it.
#ifndef TRK_HALF_RATE
#define TRK_HALF_RATE 0
#endif
#if TRK_HALF_RATE
#define TRK_OUTPUT_HZ    24585
#define TRK_LENGTH       492     // output samples per frame (24585/50)
#define TRK_SND_PRESCALE 0x03    // $FF8935 → 24585 Hz
#else
#define TRK_OUTPUT_HZ    49169
#define TRK_LENGTH       984     // output samples per frame (reference SplLen)
#define TRK_SND_PRESCALE 0x01    // $FF8935 → 49169 Hz
#endif

// Always feed: an idle ring isn't reliably silent.

#define TRK_PERIOD   428     // Amiga period for the test tone (C-3 = 8363 Hz)
#define TRK_VOL_DIV  4       // per-voice headroom (4 voices share each stereo side)
// DSP per-voice buffer is 1400 words: 3 header + ~cnt+6 sample words ⇒ cnt ≤ 1391.
// 1380 leaves margin so a garbage count can't overflow into the next voice.
#define TRK_CNT_MAX  1380
// Host-port spin bound: only trips if the DSP wedges (then we re-bootstrap).
// Writes are polled, not blasted, so a wedge can't hang the CPU on DTACK.
#define TRK_TIMEOUT  300000
#define TRK_BLAST    0

// Test-tone sample (one looped sample shared by all voices).
static u8  *trk_mod_buf    = NULL;
static s8  *trk_sample     = NULL;
static u32  trk_sample_len = 0;       // bytes
static u32  trk_sample_rep = 0;       // loop length in bytes
static u32  trk_vpos[TRK_VOICES];     // per-voice byte position

// Reference Play_Voice formulas, evaluated once (all operands constant):
//   volume    = vol(0..64) * ($7fffff/64*2) / num_voices
//   frequency = ($800000/output_hz*428*8363) / period   (relative replay step)
static long trk_vol_word  = (64L * (0x7fffffL / 64 * 2)) / TRK_VOICES;
static long trk_freq_word = ((0x800000L / TRK_OUTPUT_HZ) * 428L * 8363L) / TRK_PERIOD;

// Feed mode: 1 = live game SFX (audio.channels[0..3]); 0 = the static test
// sample (sine/mod) used to validate the chain.
#define TRK_FEED_GAME 1

// Diagnostic: log audio.channels[] transitions + music state to dsp_sfx.txt.
#define TRK_SFX_LOG 0

// freq_w = TRK_FREQ_NUM / period. Scales with the output rate so a given sample
// rate maps to the same absolute pitch at either output rate.
#define TRK_FREQ_NUM  ((0x800000L / TRK_OUTPUT_HZ) * 428L * 8363L)
// Smallest period whose freq_w stays a positive 24-bit step (≤ $7FFFFF). At
// half rate this is ~146 (vs ~73), i.e. the top octave isn't representable.
#define TRK_PERIOD_MIN (int)(TRK_FREQ_NUM / 0x7FFFFFL + 1)

// Per-voice playback state for the live SFX feed.
static s8   trk_silence[8];          // zeros for idle voices
static s8  *trk_vaddr[TRK_VOICES];   // sample currently playing (NULL = idle)
static u32  trk_vlen [TRK_VOICES];   // bytes
static u32  trk_vrep [TRK_VOICES];   // loop length in bytes
static u16  trk_vloopcnt[TRK_VOICES];// remaining loop count (mixer: loop>1 loops)
static u8   trk_vdone[TRK_VOICES];   // reached its end (one-shot / count expired)
static u32  trk_vgen [TRK_VOICES];   // last retrigger generation played (music voices)

// MV2 music voice state (4 voices → DSP voices 4-7). Written by
// dsp_mixer_update_music() from mv2_soundrout's DSP branch each tick.
static s8  *trk_music_addr[4];
static u32  trk_music_len [4];
static u32  trk_music_step[4];       // 16.16 phase step relative to host_freq
static s16  trk_music_vol [4];
static u8   trk_music_loop[4];       // 1 = one-shot, 2 = loop
static u32  trk_music_rep [4];       // loop length in bytes (looped tail at sample end)
static u32  trk_music_gen [4];       // retrigger counter (bumped on each note re-strike)
static volatile u32 trk_music_upd = 0;   // diagnostic: update_music call count
static volatile long trk_dbg_maxcnt = 0; // diagnostic: largest DSP count seen
static volatile long trk_dbg_clamps = 0; // diagnostic: count clamps (anomalies)
static volatile int  trk_wedged   = 0;   // DSP stopped responding → recover
static volatile long trk_dbg_wedges = 0; // diagnostic: wedge/recover events

// Game playback rate (Hz) → Amiga period for Play_Voice's freq formula.
// replay_hz = 428*8363/period, so period = 3579364/hz. Clamp period to
// TRK_PERIOD_MIN (≈ output rate) so the resulting freq stays a positive
// 24-bit step.
static int trk_hz_to_period(long hz)
{
    long p;
    if (hz < 50) hz = 50;
    p = 3579364L / hz;                 // 428 * 8363
    if (p < TRK_PERIOD_MIN) p = TRK_PERIOD_MIN;
    if (p > 32767)          p = 32767;
    return (int)p;
}

// Connection check — the DSP's Start blocks at Conct_Get waiting for the
// host to send ONE word, then replies with its liveness constant
// (asm56k `#12345678` is DECIMAL → 0x00BC614E → 24-bit $BC614E). Must run
// in supervisor mode ($FFA2xx is protected I/O) and must send before it
// reads, or the DSP never unblocks.
static volatile long trk_probe_value = 0;

DSP_FUNC static void dsp_tracker_probe_super(void)
{
    volatile u8 *isr = (volatile u8 *)0xFFA202;
    volatile u8 *dh  = (volatile u8 *)0xFFA205;
    volatile u8 *dm  = (volatile u8 *)0xFFA206;
    volatile u8 *dl  = (volatile u8 *)0xFFA207;
    long t;

    t = 1000000;
    while (!(*isr & 0x02)) { if (--t == 0) { trk_probe_value = -1; return; } }
    *dh = 0; *dm = 0; *dl = 0;                 // the word Conct_Get expects

    t = 1000000;
    while (!(*isr & 0x01)) { if (--t == 0) { trk_probe_value = -2; return; } }
    trk_probe_value = ((long)*dh << 16) | ((u32)*dm << 8) | *dl;
}

// Simplet's exact audio matrix (Init_Sound).
// DSP-Xmit is the SSI clock master → DAC via the matrix with handshaking,
// so NO DMA play/record is needed to keep the clock ticking.
DSP_FUNC static void dsp_simplet_matrix_super(void)
{
    *(volatile u8  *)0xFFFF8901 = 0x00;     // stop any DMA playback
    *(volatile u8  *)0xFFFF8920 = 0x0F;     // DAC on track 0
    *(volatile u16 *)0xFFFF8930 = 0x0091;   // DSP-Xmit + DMA-Play, internal 25.175MHz
    *(volatile u16 *)0xFFFF8932 = 0x2213;   // DAC/DMA-Rec/Ext-Out ← DSP-Xmit (handshake); DSP-Rec ← DMA-Play
    *(volatile u8  *)0xFFFF8935 = TRK_SND_PRESCALE;   // output rate (49169 or 24585 Hz)
    // %11 = matrix (DSP→DAC) AND PSG-Yamaha mixed to the DAC. Simplet used %10
    // (matrix only, PSG off) — but ALIS needs the YM2149 for chip music
    // (mv2_chiprout) and effects (cexplode/cding/cnoise), so keep bit0 set.
    *(volatile u8  *)0xFFFF8937 = 0x03;
}

// Per-frame feed — faithful Play_Voice. 32-bit longword host-port I/O.
// Feed-word scratch: max packets = TRK_CNT_MAX/6+1, two longwords each. File
// static (not on the ISR stack) — kept out of the pump's frame.
static u32 trk_wbuf[2 * (TRK_CNT_MAX / 6 + 2)];

// Pure-C reference for computing one voice's DSP feed words (no DSP I/O). This
// is the harness oracle AND the non-Atari fallback. Word values + order are the
// contract the optimized version must reproduce.
DSP_FUNC static void dsp_emit_packets_c(const s8 *smp, long a, long slen,
                                        long packets, int loop, u32 *out)
{
    for (long k = 0; k < packets; k++) {
        u32 w0, w1;
        if (a >= 0 && a + 6 < slen) {
            const u8 *p = (const u8 *)smp + a;
            w0 = *(const u32 *)p;
            w1 = *(const u32 *)(p + 3);
        } else {
            u8 pk[7];
            for (int j = 0; j < 7; j++) {
                long ix = a + j;
                if (loop) { while (ix >= slen) ix -= slen; while (ix < 0) ix += slen; pk[j] = (u8)smp[ix]; }
                else      { pk[j] = (ix >= 0 && ix < slen) ? (u8)smp[ix] : 0; }
            }
            w0 = ((u32)pk[0] << 24) | ((u32)pk[1] << 16) | ((u32)pk[2] << 8) | pk[3];
            w1 = ((u32)pk[3] << 24) | ((u32)pk[4] << 16) | ((u32)pk[5] << 8) | pk[6];
        }
        *out++ = w0; *out++ = w1;
        a += 6;
        if (loop) { while (a >= slen) a -= slen; }
    }
}

// Register-pinned dsp_emit_packets_c (avoids ST-RAM stack spills): asm for the
// contiguous run, C for packets crossing the loop seam / sample end.
DSP_FUNC static void dsp_emit_packets(const s8 *smp, long a, long slen,
                                      long packets, int loop, u32 *out)
{
#if defined(ALIS_USE_NATIVE_ATARI)
    long k = 0;
    while (k < packets) {
        if (a >= 0 && a + 6 < slen) {
            long run = (slen - a - 1) / 6;       // packets whose window is fully in [0,slen)
            if (run > packets - k) run = packets - k;
            if (run < 1) run = 1;                // outer test guarantees >=1 contiguous
            const u8 *p = (const u8 *)smp + a;
            long n = run;
            __asm__ volatile (
                "0: move.l %0@,%%d0      \n"
                "   move.l %0@(3),%%d1   \n"
                "   move.l %%d0,%1@+     \n"
                "   move.l %%d1,%1@+     \n"
                "   lea    %0@(6),%0     \n"
                "   subq.l #1,%2         \n"
                "   bne.b  0b            \n"
                : "+a"(p), "+a"(out), "+d"(n)
                : : "d0", "d1", "cc", "memory");
            a += 6 * run;
            k += run;
            if (loop) { while (a >= slen) a -= slen; }
        } else {
            u8 pk[7];
            for (int j = 0; j < 7; j++) {
                long ix = a + j;
                if (loop) { while (ix >= slen) ix -= slen; while (ix < 0) ix += slen; pk[j] = (u8)smp[ix]; }
                else      { pk[j] = (ix >= 0 && ix < slen) ? (u8)smp[ix] : 0; }
            }
            *out++ = ((u32)pk[0] << 24) | ((u32)pk[1] << 16) | ((u32)pk[2] << 8) | pk[3];
            *out++ = ((u32)pk[3] << 24) | ((u32)pk[4] << 16) | ((u32)pk[5] << 8) | pk[6];
            a += 6;
            if (loop) { while (a >= slen) a -= slen; }
            k++;
        }
    }
#else
    dsp_emit_packets_c(smp, a, slen, packets, loop, out);
#endif
}

// Drain feed words to the DSP port with the per-word TXDE handshake. Returns 1
// if the DSP wedged (TXDE never set) so the caller can re-bootstrap.
DSP_FUNC static int dsp_feed_words(volatile u8 *isr, volatile u32 *data,
                                   const u32 *w, long n)
{
    if (n <= 0) return 0;
#if defined(ALIS_USE_NATIVE_ATARI)
    // Register-pinned (gcc reloads args from the ST-RAM stack every iteration).
    int wedged = 0;
    long t, tmp;
    __asm__ volatile (
        "0: move.l %[to],%[t]       \n"   /* t = TRK_TIMEOUT */
        "1: move.b %[isr]@,%[tmp]   \n"   /* poll TXDE (bit 1 of *isr)   */
        "   btst   #1,%[tmp]        \n"
        "   bne.b  2f               \n"   /*   set -> ready, go write    */
        "   subq.l #1,%[t]          \n"
        "   bne.b  1b               \n"
        "   moveq  #1,%[wed]        \n"   /*   timeout -> wedged, stop   */
        "   bra.b  3f               \n"
        "2: move.l %[w]@+,%[data]@  \n"   /* *data = *w++                */
        "   subq.l #1,%[n]          \n"
        "   bne.b  0b               \n"
        "3:                         \n"
        : [w]"+a"(w), [n]"+d"(n), [wed]"+d"(wedged), [t]"=&d"(t), [tmp]"=&d"(tmp)
        : [isr]"a"(isr), [data]"a"(data), [to]"i"((long)TRK_TIMEOUT)
        : "cc", "memory");
    return wedged;
#else
    for (long i = 0; i < n; i++) {
        long t = TRK_TIMEOUT;
        while (!(*isr & 0x02)) { if (--t == 0) return 1; }
        *data = w[i];
    }
    return 0;
#endif
}

#if defined(ALIS_DSP_PUMP_VERIFY) && ALIS_DSP_PUMP_VERIFY
static u32 trk_wbuf_c[2 * (TRK_CNT_MAX / 6 + 2)];   // C-reference words for compare
static volatile unsigned long dsp_pump_calls = 0, dsp_pump_bad = 0;
static volatile long dsp_pump_bi[6];   // last mismatch: v, nbad, first, packets, a, slen
static volatile u32  dsp_pump_bw[2];   // last mismatch: asm word, C word
#endif

DSP_FUNC static void dsp_tracker_pump_super(void)
{
    volatile u8  *isr  = (volatile u8  *)0xFFA202;
    volatile u8  *cvr  = (volatile u8  *)0xFFA201;
    volatile u32 *data = (volatile u32 *)0xFFA204;

    // Send Length, the 8-bit active-voice mask, then feed data for the set bits in
    // voice order. The DSP clears its window and mixes only those voices (mask 0 =
    // silence); per-voice phase persists in fixed DSP slots.
    // Runs at the lowered IPL set by atari_timera_c_callback (ACIA must preempt):
    // don't raise it. On a TXDE stall flag a wedge and bail; the main loop re-bootstraps.
    #define TXW(val) do { \
        long _t = TRK_TIMEOUT; \
        while (!(*isr & 0x02)) { if (--_t == 0) { trk_wedged = 1; goto done; } } \
        *data = (u32)(val); \
    } while (0)

    // ---- PASS 1: per-voice state + feed params. Restart detection must run for all
    // 8 voices every frame, or a stop→restart at the same address won't retrigger.
    s8       *f_smp [TRK_VOICES];
    long      f_slen[TRK_VOICES], f_srep[TRK_VOICES], f_vol[TRK_VOICES], f_freq[TRK_VOICES];
    int       f_loop[TRK_VOICES];
    sChannel *f_gc  [TRK_VOICES];
    u32       active_mask = 0;

    for (int v = 0; v < TRK_VOICES; v++) {
        s8  *smp;
        long slen, srep, vol_w, freq_w;
        int  loop, playing;
        sChannel *gc = NULL;

#if TRK_FEED_GAME
        if (v < 4) {
            // Voices 0-3: game SFX channels. Idle / finished → silent.
            gc = &audio.channels[v];
            int wants = (audio.fsound && gc->state >= 0
                         && gc->type == eChannelTypeSample
                         && gc->address != NULL && gc->length > 0);
            if (wants) {
                if (trk_vaddr[v] != gc->address) {  // new sample → restart
                    trk_vaddr[v]    = gc->address;
                    trk_vlen[v]     = gc->length;
                    trk_vrep[v]     = gc->length;   // loop whole sample
                    trk_vloopcnt[v] = gc->loop;     // mixer semantics: loop>1 repeats
                    trk_vpos[v]     = 0;
                    trk_vdone[v]    = 0;
                }
            } else {
                trk_vaddr[v] = NULL;                // idle → next sample is "new"
                trk_vdone[v] = 0;
            }
            playing = (trk_vaddr[v] != NULL && !trk_vdone[v]);
            if (playing) {
                smp  = trk_vaddr[v];
                slen = (long)trk_vlen[v];
                srep = (long)trk_vrep[v];
                loop = (trk_vloopcnt[v] > 1);
                { long gv = gc->volume;             // SFX volume is 0..127
                  if (gv < 0)   gv = 0;
                  if (gv > 127) gv = 127;
                  vol_w = (gv * (0x7FFFFFL / 127L)) / TRK_VOL_DIV; }
                freq_w = TRK_FREQ_NUM / trk_hz_to_period(gc->freq);
            } else {
                smp = trk_silence; slen = (long)sizeof(trk_silence); srep = slen;
                loop = 1; vol_w = 0; freq_w = 0;
            }
        } else {
            // Voices 4-7: MV2 music voices (trk_music_* set by the sequencer).
            int m = v - 4;
            int wants = (audio.muflag > 0 && trk_music_addr[m] != NULL
                         && trk_music_len[m] > 0 && trk_music_vol[m] > 0
                         && trk_music_step[m] > 0);
            if (wants) {
                // Restart on an address change OR a fresh retrigger generation
                // (MV1 re-strikes the same one-shot sample at a constant address
                // for each repeated note — the generation bump forces the replay).
                if (trk_vaddr[v] != trk_music_addr[m]
                    || trk_vgen[v] != trk_music_gen[m]) {
                    trk_vaddr[v]    = trk_music_addr[m];
                    trk_vlen[v]     = trk_music_len[m];
                    trk_vrep[v]     = trk_music_rep[m];     // looped tail length
                    trk_vloopcnt[v] = (trk_music_loop[m] >= 2) ? 0xFFFF : 1;
                    trk_vpos[v]     = 0;
                    trk_vdone[v]    = 0;
                    trk_vgen[v]     = trk_music_gen[m];
                }
            } else {
                trk_vaddr[v] = NULL;
                trk_vdone[v] = 0;
            }
            playing = (trk_vaddr[v] != NULL && !trk_vdone[v]);
            if (playing) {
                u32 hz;
                smp  = trk_vaddr[v];
                slen = (long)trk_vlen[v];
                srep = (long)trk_vrep[v];
                loop = (trk_vloopcnt[v] > 1);
                { long mv = trk_music_vol[m];           // f_volume 0..~0x3F00
                  if (mv < 0) mv = 0;
                  if (mv > 0x3F00) mv = 0x3F00;
                  vol_w = (mv * (0x7FFFFFL / 0x3F00L)) / TRK_VOL_DIV; }
                // playback_hz = host_freq * (16.16 step) / 65536
                hz = (u32)(((unsigned long long)audio.host_freq
                            * trk_music_step[m]) >> 16);
                freq_w = TRK_FREQ_NUM / trk_hz_to_period((long)hz);
            } else {
                smp = trk_silence; slen = (long)sizeof(trk_silence); srep = slen;
                loop = 1; vol_w = 0; freq_w = 0;
            }
        }
#else
        smp  = trk_sample;                      // static test signal (validation)
        slen = (long)trk_sample_len;
        srep = (long)trk_sample_rep;
        if (srep <= 0) srep = slen;
        loop = 1; playing = 1;
        vol_w = trk_vol_word; freq_w = trk_freq_word;
#endif
        if (!smp || slen <= 0) {                // safety: never read NULL
            smp = trk_silence; slen = (long)sizeof(trk_silence); srep = slen; loop = 1;
        }

        if (playing) active_mask |= (1u << v);
        f_smp[v] = smp; f_slen[v] = slen; f_srep[v] = srep;
        f_vol[v] = vol_w; f_freq[v] = freq_w; f_loop[v] = loop; f_gc[v] = gc;
    }

    // Trigger Soundtrack_Rout (vector $26 → CVR = 0x80 | ($26 >> 1) = 0x93).
    // Wait for the DSP to have accepted the previous command (HC clear). A
    // long stall = wedge → flag + bail (never trigger-anyway: that would queue
    // two frames into the single HC bit → off-by-one desync).
    { long _t = TRK_TIMEOUT;
      while (*cvr & 0x80) { if (--_t == 0) { trk_wedged = 1; goto done; } } }
    *cvr = 0x93;

    TXW(TRK_LENGTH);                       // SplLen
    TXW(active_mask);                      // 8-bit active-voice bitmask (DSP gates Rx + Mix on it)

    // ---- PASS 2: feed the active voices in voice order (the DSP's mask-gated Receive).
    for (int v = 0; v < TRK_VOICES; v++) {
        if (!(active_mask & (1u << v)))
            continue;
        s8  *smp  = f_smp[v];
        long slen = f_slen[v], srep = f_srep[v], vol_w = f_vol[v], freq_w = f_freq[v];
        int  loop = f_loop[v];
        sChannel *gc = f_gc[v];
        long a, cnt = 0, packets, k;

        TXW(vol_w);
        TXW(freq_w);

        // WaitDSP: read the sample-byte count (32-bit, mask to 24). A long
        // stall = wedge → flag + bail (no mid-frame abandon-without-reset).
        { long _t = TRK_TIMEOUT;
          while (!(*isr & 0x01)) { if (--_t == 0) { trk_wedged = 1; goto done; } }
          cnt = (long)((*data) & 0xFFFFFF); }
        // Clamp: a desynced handshake can return a garbage count (→ a push loop
        // of millions of packets = freeze).
        if (cnt < 0) cnt = 0;
        if (cnt > TRK_CNT_MAX) { cnt = TRK_CNT_MAX; trk_dbg_clamps++; }
        if (cnt > trk_dbg_maxcnt) trk_dbg_maxcnt = cnt;

        a = (long)trk_vpos[v];             // read window starts at the old pos
        { long np = a + cnt;               // advance the cursor
          while (np >= slen) {             // mixer semantics: loop>1 repeats,
              if (trk_vloopcnt[v] > 1) {   // decrementing the count each wrap
                  trk_vloopcnt[v]--;
                  np -= srep;
              } else {                     // one-shot / count expired → stop
                  trk_vdone[v] = 1;
                  np = slen;
                  // Free the game channel as the software mixer does, or the
                  // picker never reclaims it.
                  if (gc) { gc->type = eChannelTypeNone; gc->curson = (s8)0x80; }
                  break;
              }
          }
          if (np < 0) np = 0;
          trk_vpos[v] = (u32)np; }

        packets = cnt / 6 + 1;             // samples sent in packets of 6 bytes
        TXW(packets);

        // Two overlapping longwords per packet from (pos-1), advance 6.
        // Looping samples wrap; one-shots read 0 past the end.
        a -= 1;
        // Normalize into [0, slen) for loops so the per-byte wrap stays
        // bounded (≤1 step). One-shots keep `a` as-is (0-pad past the end).
        if (loop) { while (a >= slen) a -= slen; while (a < 0) a += slen; }
        // Compute all feed words, then drain them to the port in one flat loop.
        dsp_emit_packets(smp, a, slen, packets, loop, trk_wbuf);
#if defined(ALIS_DSP_PUMP_VERIFY) && ALIS_DSP_PUMP_VERIFY
        // Word-stream oracle: recompute with the pure-C reference and compare.
        // Pure (no DSP I/O), so it re-runs safely in the ISR; report from the
        // periodic logger (dbglog/fprintf is not ISR-safe here).
        dsp_emit_packets_c(smp, a, slen, packets, loop, trk_wbuf_c);
        { long nb = 0, first = -1;
          for (long i = 0; i < 2 * packets; i++)
              if (trk_wbuf[i] != trk_wbuf_c[i]) { if (first < 0) first = i; nb++; }
          dsp_pump_calls++;
          if (nb) { dsp_pump_bad++;
              dsp_pump_bi[0]=v; dsp_pump_bi[1]=nb; dsp_pump_bi[2]=first;
              dsp_pump_bi[3]=packets; dsp_pump_bi[4]=a; dsp_pump_bi[5]=slen;
              dsp_pump_bw[0]=trk_wbuf[first]; dsp_pump_bw[1]=trk_wbuf_c[first]; } }
#endif
        if (dsp_feed_words(isr, data, trk_wbuf, 2 * packets)) { trk_wedged = 1; goto done; }
    }

done:
    #undef TXW
    return;
}

// Re-bootstrap a wedged DSP (it stopped responding mid-feed → a wait timed
// out). Runs from the main loop (dsp_mixer_poll), NOT the ISR: re-loads the
// tracker program, restores the matrix, re-confirms the liveness probe. A full
// reset means no leftover desync. One attempt per wedge; if it can't come back
// the feed stays disabled (silent) rather than freezing or retry-storming.
static void dsp_tracker_recover(void)
{
    trk_wedged = 0;                         // one attempt per wedge event
    dsp_mixer_available = 0;                // stop the ISR feed during recovery
    trk_dbg_wedges++;

    g_boot_program      = dsp_tracker_program;
    g_boot_program_size = dsp_tracker_program_size;
    g_boot_load_addr    = 0x0000;
    Supexec(dsp_bootstrap_super);
    if (dbg_boot_fail || dbg_bootstrap_entered != 2)
        return;                             // bootstrap failed → stay silent

    for (volatile long i = 0; i < 200000; i++) ;
    Supexec(dsp_simplet_matrix_super);
    Supexec(dsp_tracker_probe_super);
    if ((trk_probe_value & 0xFFFFFF) != 0xBC614E)
        return;                             // probe failed → stay silent

    // Fresh start: drop any in-flight cursors so we don't resume mid-sample.
    memset(trk_vpos,  0, sizeof(trk_vpos));
    memset(trk_vaddr, 0, sizeof(trk_vaddr));
    memset(trk_vdone, 0, sizeof(trk_vdone));
    memset(trk_vgen,  0, sizeof(trk_vgen));
    memset(trk_music_gen, 0, sizeof(trk_music_gen));
    memset(trk_music_rep, 0, sizeof(trk_music_rep));
    dsp_mixer_available = 1;                // resume feeding
}

// Diagnostic test signal. 1 = synthesized sine (clean, unambiguous tone —
// use this to confirm the DSP→DAC chain is faithful). 0 = first non-empty
// sample from test.mod (arbitrary instrument; sounds "strange" looped).
#define TRK_TEST_SINE 1

// 256-byte buffer holding an integer number of sine cycles, so looping the
// whole buffer has no seam click. 8 cycles / 256 bytes = 32 bytes/cycle →
// at period 428 (8363 Hz) the pitch is 8363/32 ≈ 261 Hz (middle C).
static s8 trk_tone[256];

static void trk_build_sine(int cycles)
{
    for (int i = 0; i < 256; i++) {
        double ph = 2.0 * 3.14159265358979 * (double)cycles * (double)i / 256.0;
        trk_tone[i] = (s8)(sin(ph) * 100.0);
    }
    trk_sample     = trk_tone;
    trk_sample_len = 256;
    trk_sample_rep = 256;
}

// Load test.mod and pick its first non-empty sample as the looped test
// tone. Falls back to a synthesized square wave if the file is missing.
static int trk_load_test_sample(FILE *dlog)
{
#if TRK_TEST_SINE
    trk_build_sine(8);
    if (dlog) fprintf(dlog, "Test signal: synthesized sine, 8 cycles/256 bytes "
                      "(~261 Hz @ period 428)\n");
    return 0;
#else
    static s8 square[256];

    FILE *f = fopen("test.mod", "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (sz >= 1084) {
            trk_mod_buf = (u8 *)sys_Mxalloc(sz, 0);
            if (trk_mod_buf && (long)fread(trk_mod_buf, 1, sz, f) == sz) {
                u8 *m = trk_mod_buf;
                int maxpat = 0;
                for (int i = 0; i < 128; i++) if (m[952+i] > maxpat) maxpat = m[952+i];
                long off = 1084 + (long)(maxpat + 1) * 1024;
                for (int s = 0; s < 31; s++) {
                    u8 *h = m + 20 + s * 30;
                    u32 L  = (((u32)h[22] << 8) | h[23]) * 2;
                    u32 rl = (((u32)h[28] << 8) | h[29]) * 2;
                    if (L >= 4) {
                        trk_sample     = (s8 *)(m + off);
                        trk_sample_len = L;
                        trk_sample_rep = (rl > 2) ? rl : L;
                        fclose(f);
                        if (dlog) fprintf(dlog, "test.mod sample %d: off=%ld len=%lu rep=%lu\n",
                                          s + 1, off, (unsigned long)trk_sample_len,
                                          (unsigned long)trk_sample_rep);
                        return 0;
                    }
                    off += L;
                }
            }
        }
        fclose(f);
    }

    // Fallback: 256-byte square wave so the chain can still be exercised.
    for (int i = 0; i < 256; i++) square[i] = (i < 128) ? 100 : -100;
    trk_sample     = square;
    trk_sample_len = 256;
    trk_sample_rep = 256;
    if (dlog) fprintf(dlog, "test.mod unavailable — using 256-byte square wave\n");
    return 0;
#endif /* TRK_TEST_SINE */
}

#endif /* ALIS_DSP_USE_TRACKER */

// Per-tick feed from the 50 Hz Timer A ISR (already supervisor: no Supexec).
// The DSP's SSI ring (~3700 samples) needs this steady cadence.
void dsp_mixer_tick_isr(void)
{
    if (dsp_current_prog == DSPPROG_FM) {
        // OPL mode: the only place soundrout() ticks. mv2_opl2rout → sys_opl_frame_tick
        // → dsp_opl_feed + dsp_opl_fm_frame.
        sys_psg_tick();
        if (dsp_dac_muted) { *(volatile u8 *)0xFFFF8937 = 0x03; dsp_dac_muted = 0; }  // DAC on
        if (audio.fmusic && audio.muflag > 0 && audio.soundrout) {
            fm_silenced = 0;
            audio.soundrout();          // FM music: feeds blocks + FM_Frame (FM + SFX)
        } else {
            // OPL music stopped: silence the FM notes ONCE (no stuck note), but keep
            // rendering each frame so sample SFX still play (DAC stays on).
            if (!fm_silenced) { dsp_opl_fm_silence(); fm_silenced = 1; }
            dsp_opl_fm_frame();         // render silent FM + the SFX voices
        }
        return;
    }
#ifdef ALIS_DSP_USE_TRACKER
    // YM effect channels (no audio callback on this path); runs even while the DSP is wedged.
    sys_psg_tick();

    if (!dsp_mixer_available || trk_wedged) return;   // skip until recovered
    // The only place soundrout() ticks; music updates trk_music_* via dsp_mixer_update_music().
    if (audio.fmusic && audio.muflag > 0 && audio.soundrout)
        audio.soundrout();
    dsp_tracker_pump_super();          // feed all 8 voices (SFX + music)
#endif
}

int dsp_mixer_init(void)
{
    FILE *dlog = fopen("dsp_init.txt", "w");

    // One-time DSP setup (lock + STRAM buffer). Guarded so a music-type swap can
    // re-enter dsp_mixer_init() to re-bootstrap the tracker without re-locking.
    if (!dsp_setup_done) {
        long ws = (short)Dsp_GetWordSize();
        if (dlog) { fprintf(dlog, "Dsp_GetWordSize=%ld\n", ws); fflush(dlog); }
        if (ws != 3) { if (dlog) fclose(dlog); return -1; }
        long lr = (short)Dsp_Lock();
        long sr = (short)Locksnd();
        if (dlog) { fprintf(dlog, "Dsp_Lock=%ld Locksnd=%ld\n", lr, sr); fflush(dlog); }
        // STRAM buffer used for dummy DMA-record. The Falcon audio matrix master
        // clock only ticks while something is driving the DMA chain.
        g_dma_buf = (u8 *)sys_Mxalloc(8000, 0);
        if (dlog) { fprintf(dlog, "Mxalloc(STRAM,8000)=%p\n", g_dma_buf); fflush(dlog); }
        if (!g_dma_buf) { if (dlog) fclose(dlog); return -1; }
        dsp_setup_done = 1;
    }

#if defined(ALIS_DSP_USE_TRACKER)  /* Simplet DSP Soundtracker path */

    // ===== Simplet's tracker (the DSP audio path) =====
    // NoCrew-bootstrap dsp_tracker_program at P:$0 (sections at $0/$10/
    // $26/$28/$40), set up Simplet's matrix, then the 50 Hz Timer A ISR
    // feeds per-frame voice params + sample bytes via HCV $26 (Soundtrack_Rout).
    g_boot_program      = dsp_tracker_program;
    g_boot_program_size = dsp_tracker_program_size;
    g_boot_load_addr    = 0x0000;
    if (dlog) { fprintf(dlog, "NoCrew bootstrap %lu tracker words at P:$0...\n",
                       (unsigned long)dsp_tracker_program_size); fflush(dlog); }
    Supexec(dsp_bootstrap_super);
    if (dlog) {
        fprintf(dlog, "  bootstrap done (entered=%ld fail=%ld pa_after=%02X isr_after=%02X)\n",
                dbg_bootstrap_entered, dbg_boot_fail, dbg_pa_after, dbg_isr_after_write);
        fflush(dlog);
    }
    // Bootstrap timed out → DSP host port never responded. Leave the mixer
    // disabled so the per-frame pump never runs; game proceeds silently
    // rather than hanging.
    if (dbg_boot_fail || dbg_bootstrap_entered != 2) {
        if (dlog) { fprintf(dlog, "FAILED: bootstrap stage %ld — DSP audio disabled.\n",
                           dbg_boot_fail); fclose(dlog); }
        dsp_mixer_available = 0;
        return -1;
    }
    for (volatile long i = 0; i < 200000; i++) ;

    // Audio matrix: Simplet's EXACT setup (its DSP is the SSI clock master
    // → DAC with handshaking, so no DMA play/record is needed).
    Supexec(dsp_simplet_matrix_super);
    Supexec(dsp_read_matrix_super);
    if (dlog) { fprintf(dlog, "Simplet matrix: $8930=%04X $8932=%04X $8934=%04X\n",
                       dsp_8930, dsp_8932, dsp_8934); fflush(dlog); }

    // Liveness probe (see dsp_tracker_probe_super).
    Supexec(dsp_tracker_probe_super);
    if (dlog) {
        fprintf(dlog, "Tracker liveness probe: got=$%06lX (expect $BC614E; "
                "-1=TX timeout, -2=RX timeout)\n", trk_probe_value & 0xFFFFFF);
        fflush(dlog);
    }

    // Only engage the per-frame feed when the DSP is confirmed running our
    // code. Otherwise every pump would burn host-port timeouts each frame
    // (sluggish, silent). Confirmed-dead → silent, full-speed game.
    if ((trk_probe_value & 0xFFFFFF) == 0xBC614E) {
        memset(trk_vpos,    0, sizeof(trk_vpos));
        memset(trk_vaddr,   0, sizeof(trk_vaddr));
        memset(trk_vlen,    0, sizeof(trk_vlen));
        memset(trk_vrep,     0, sizeof(trk_vrep));
        memset(trk_vloopcnt, 0, sizeof(trk_vloopcnt));
        memset(trk_vdone,    0, sizeof(trk_vdone));
        memset(trk_vgen,     0, sizeof(trk_vgen));
        memset(trk_music_gen, 0, sizeof(trk_music_gen));
        memset(trk_music_rep, 0, sizeof(trk_music_rep));
        memset(trk_silence, 0, sizeof(trk_silence));
#if TRK_FEED_GAME
        if (dlog) fprintf(dlog, "Tracker init done. Live SFX feed: "
                          "audio.channels[0..3] → DSP voices.\n");
#else
        trk_load_test_sample(dlog);        // static test tone (all voices)
        if (dlog) fprintf(dlog, "Tracker init done. vol=$%06lX freq=$%06lX len=%lu. "
                          "Feeding static test tone.\n", trk_vol_word & 0xFFFFFF,
                          trk_freq_word & 0xFFFFFF, (unsigned long)trk_sample_len);
#endif
        if (dlog) fclose(dlog);
        dsp_mixer_available = 1;
        dsp_current_prog = DSPPROG_TRACKER;
        return 0;
    }

    dsp_mixer_available = 0;
    if (dlog) { fprintf(dlog, "Liveness probe failed — DSP audio disabled (game runs silent).\n");
        fclose(dlog); }
    return -1;

#else  /* ALIS_DSP_USE_SFXMIXER — legacy hand-written DSP SFX mixer */

    // ===== Our 8-channel sample mixer =====
    // NoCrew-bootstrap OUR dsp_mixer.asm code, set up matrix DUMP-style,
    // then dsp_mixer_poll() (from the main loop) feeds it UPLOAD+PLAY/STOP
    // commands as game channels change state.
    g_boot_program      = dsp_mixer_program;
    g_boot_program_size = dsp_mixer_program_size;
    g_boot_load_addr    = 0x0040;
    if (dlog) { fprintf(dlog, "NoCrew bootstrap %lu user words at P:$40...\n",
                       (unsigned long)dsp_mixer_program_size); fflush(dlog); }
    Supexec(dsp_bootstrap_super);
    if (dlog) {
        fprintf(dlog, "  bootstrap done (entered=%ld pa_after=%02X isr_after=%02X)\n",
                dbg_bootstrap_entered, dbg_pa_after, dbg_isr_after_write);
        fflush(dlog);
    }
    for (volatile long i = 0; i < 200000; i++) ;

    // DUMP-style matrix + DMA-record. Reuses g_dma_buf for the
    // record window.
    dsp_dma_rec_buf_addr = g_dma_buf;
    Supexec(dsp_dump_matrix_setup_super);
    Supexec(dsp_read_matrix_super);
    if (dlog) { fprintf(dlog, "Matrix: $8930=%04X $8932=%04X $8934=%04X\n",
                       dsp_8930, dsp_8932, dsp_8934); fflush(dlog); }

    memset(dsp_channels, 0, sizeof(dsp_channels));
    memset(dsp_slots, 0, sizeof(dsp_slots));
    dsp_mixer_available = 1;

    dsp_mixlog = fopen("dsp_mix.txt", "w");
    if (dsp_mixlog) fprintf(dsp_mixlog, "SFX mixer started\n");

    if (dlog) { fprintf(dlog, "Init done. dsp_mixer_available=1, poll active.\n"); fclose(dlog); }
    return 0;
#endif
}


// Release the DSP cleanly, or the next run inherits a wedged DSP.
void dsp_mixer_close(void)
{
    if (dsp_mixlog) { fclose(dsp_mixlog); dsp_mixlog = NULL; }

#if defined(ALIS_DSP_USE_TRACKER)
    // Tracker path: halt audio DMA so the matrix master clock stops, then
    // let Dsp_Unlock reset the DSP. Without halting DMA the SSI keeps
    // pulling samples and the next run inherits a wedged matrix.
    if (dsp_mixer_available) Buffoper(0);
#else
    // Stop channels + DMA so the next run starts with a clean matrix.
    if (dsp_mixer_available) {
        for (int ch = 0; ch < DSP_NUM_CHANNELS; ch++) {
            long stop_cmd[2] = { 3, ch };
            static long dummy;
            Dsp_BlkUnpacked(stop_cmd, 2, &dummy, 0);
        }
        Buffoper(0);
    }
#endif

    Dsp_Unlock();
    Unlocksnd();
    dsp_mixer_available = 0;
}

static int dsp_update_count = 0;

void dsp_mixer_update_music(int voice, s8 *address, u32 length, u32 step,
                            s16 volume, u8 loop, int retrigger, u32 replen)
{
#ifdef ALIS_DSP_USE_TRACKER
    if (voice < 0 || voice >= 4) return;
    trk_music_addr[voice] = address;
    trk_music_len [voice] = length;
    trk_music_rep [voice] = (replen && replen <= length) ? replen : length;
    trk_music_step[voice] = step;
    trk_music_vol [voice] = volume;
    trk_music_loop[voice] = loop;
    if (retrigger) trk_music_gen[voice]++;
    trk_music_upd++;
#else
    (void)voice; (void)address; (void)length; (void)step; (void)volume; (void)loop;
    (void)retrigger; (void)replen;
#endif
}

static int dsp_mix_call_count = 0;

void dsp_mixer_mix(s16 *output, u16 num_samples, u16 sample_rate)
{
    dsp_mix_call_count++;
    // Log first calls to verify we reach here
    if (dsp_mix_call_count <= 3 && dsp_mixlog) {
        fprintf(dsp_mixlog, "mix#%d output=%p n=%u\n", dsp_mix_call_count, (void*)output, num_samples);
        fflush(dsp_mixlog);
    }
    if (output && num_samples > 0)
        memset(output, 0, num_samples * sizeof(s16));
    (void)sample_rate;
}

// Main-loop service (never the ISR): DSP program swaps, wedge recovery, harness
// reports. SFX-mixer path: emit upload+play / stop for audio.channels[0..3] changes.
void dsp_mixer_poll(void)
{
    // Swap the resident DSP program if the music type changed (OPL ↔ sampled).
    // Safe context (main loop, not the ISR) for the slow re-bootstrap.
    if (dsp_setup_done)
        dsp_audio_select_for_music();

#if defined(ALIS_MV2_MIX_VERIFY) && ALIS_MV2_MIX_VERIFY
    // CPU sample-mixer (mv2_soundcal) bit-exact harness: the mixer tick (which
    // can run in the audio ISR) only accumulates counters; report here, in the
    // main-loop safe context, ~once/second to the dbglog file.
    { extern void dbglog(const char *fmt, ...);
      extern volatile unsigned long mv2_mix_calls, mv2_mix_bad;
      extern volatile long mv2_mix_bi[6];
      static unsigned long mv_last = 0; static int mv_n = 0;
      if ((++mv_n % 50) == 0 && (mv2_mix_calls != mv_last)) {
          mv_last = mv2_mix_calls;
          dbglog("[mv2mix] calls=%lu bad=%lu last{idx=%ld asm=%ld C=%ld first=%ld dLS=%08lx dSS=%08lx}\n",
                 mv2_mix_calls, mv2_mix_bad, mv2_mix_bi[0], mv2_mix_bi[1],
                 mv2_mix_bi[2], mv2_mix_bi[3],
                 (unsigned long)mv2_mix_bi[4], (unsigned long)mv2_mix_bi[5]); }
    }
#endif

#if defined(ALIS_DSP_USE_TRACKER)
    // Simplet tracker: the per-frame feed is driven by the Timer A ISR. The
    // main-loop poll handles (a) recovering a wedged DSP — heavy re-bootstrap,
    // done here in a safe context, not the ISR — and (b) logging.
    if (trk_wedged) dsp_tracker_recover();
#if defined(ALIS_DSP_PUMP_VERIFY) && ALIS_DSP_PUMP_VERIFY
    // Word-stream harness report: main-loop safe context, ~once/second, to the
    // dbglog file (the ISR only accumulates the counters).
    { extern void dbglog(const char *fmt, ...);
      static unsigned long pv_last = 0; static int pv_n = 0;
      if ((++pv_n % 50) == 0 && (dsp_pump_calls != pv_last)) {
          pv_last = dsp_pump_calls;
          dbglog("[dsppump] calls=%lu bad=%lu last{v=%ld nb=%ld first=%ld pk=%ld a=%ld slen=%ld asm=%08lx C=%08lx}\n",
                 dsp_pump_calls, dsp_pump_bad, dsp_pump_bi[0], dsp_pump_bi[1],
                 dsp_pump_bi[2], dsp_pump_bi[3], dsp_pump_bi[4], dsp_pump_bi[5],
                 (unsigned long)dsp_pump_bw[0], (unsigned long)dsp_pump_bw[1]); }
    }
#endif
#if TRK_SFX_LOG
    {
        static FILE *slog = NULL;
        static int   opened = 0;
        static int   last_state[4];
        static int   last_type[4];
        static void *last_addr[4];
        if (!opened) {
            slog = fopen("dsp_sfx.txt", "w");
            opened = 1;
            if (slog) { fprintf(slog, "fsound=%d (logging channel transitions)\n",
                                audio.fsound); fflush(slog); }
        }
        for (int v = 0; v < 4; v++) {
            sChannel *gc = &audio.channels[v];
            if (gc->state != last_state[v] || (int)gc->type != last_type[v]
                || (void *)gc->address != last_addr[v]) {
                last_state[v] = gc->state;
                last_type[v]  = (int)gc->type;
                last_addr[v]  = (void *)gc->address;
                if (slog) {
                    fprintf(slog, "ch%d state=%d type=0x%02X addr=%p len=%lu "
                            "freq=%d vol=%d loop=%d fsound=%d\n",
                            v, gc->state, (unsigned)gc->type, (void *)gc->address,
                            (unsigned long)gc->length, (int)gc->freq,
                            (int)gc->volume, gc->loop, audio.fsound);
                    fflush(slog);
                }
            }
        }

        // ---- Music diagnostics: once a second, dump sequencer + voices ----
        static int mcount = 0;
        static u32 last_upd = 0;
        if (slog && (++mcount % 50) == 0) {
            fprintf(slog, "MUSIC fmusic=%d muflag=%d soundrout=%p host_freq=%d "
                    "upd=%lu (+%lu/s) maxcnt=%ld clamps=%ld wedges=%ld avail=%d\n",
                    audio.fmusic, (int)audio.muflag, (void *)audio.soundrout,
                    (int)audio.host_freq, (unsigned long)trk_music_upd,
                    (unsigned long)(trk_music_upd - last_upd),
                    trk_dbg_maxcnt, trk_dbg_clamps, trk_dbg_wedges,
                    (int)dsp_mixer_available);
            last_upd = trk_music_upd;
            trk_dbg_maxcnt = 0;            // reset so each line shows this second's peak
            for (int m = 0; m < 4; m++)
                fprintf(slog, "  mv%d addr=%p len=%lu step=%lu vol=%d loop=%d\n",
                        m, (void *)trk_music_addr[m], (unsigned long)trk_music_len[m],
                        (unsigned long)trk_music_step[m], (int)trk_music_vol[m],
                        trk_music_loop[m]);
            fflush(slog);
        }
    }
#endif
    return;
#else
    if (!dsp_mixer_available) return;

    static long dummy_out;

    for (int slot = 0; slot < 4; slot++) {
        sChannel *ch = &audio.channels[slot];
        sDSPChannel *d = &dsp_channels[slot];

        int wants_play = (audio.fsound
                          && ch->state >= 0
                          && ch->type == eChannelTypeSample
                          && ch->address != NULL
                          && ch->length > 0);

        if (wants_play) {
            if (d->active && d->address == ch->address)
                continue;  // already playing this sample on this slot

            int dsp_slot = dsp_find_slot(ch->address, ch->length);
            u32 upload_len = ch->length;
            if (upload_len > DSP_SLOT_SIZE) upload_len = DSP_SLOT_SIZE;
            if (upload_len > DSP_SENDBUF_MAX - 9) upload_len = DSP_SENDBUF_MAX - 9;

            // Build one Dsp_BlkUnpacked transaction: optional UPLOAD + PLAY
            long *sp = dsp_sendbuf;

            if (dsp_slots[dsp_slot].address != ch->address || dsp_slots[dsp_slot].length != upload_len) {
                dsp_slots[dsp_slot].address = ch->address;
                dsp_slots[dsp_slot].length = upload_len;
                *sp++ = 1;                    // CMD_UPLOAD
                *sp++ = dsp_slot;
                *sp++ = (long)upload_len;
                for (u32 i = 0; i < upload_len; i++)
                    *sp++ = (s32)ch->address[i] << 16;
            }

            *sp++ = 2;                                          // CMD_PLAY
            *sp++ = slot;                                       // DSP channel = slot 0..3
            *sp++ = dsp_slot;
            *sp++ = 0x010000;                                   // step (1.0 fixed-point)
            *sp++ = ((s32)ch->volume << 8) & 0x7FFFFF;
            *sp++ = ch->loop;

            long count = (long)(sp - dsp_sendbuf);
            Dsp_BlkUnpacked(dsp_sendbuf, count, &dummy_out, 0);

            d->active = 1;
            d->address = ch->address;
            d->length = ch->length;
            dsp_poll_calls++;

            if (dsp_mixlog && dsp_poll_calls < 20) {
                fprintf(dsp_mixlog, "PLAY ch%d slot%d len=%lu vol=%d\n",
                        slot, dsp_slot, (unsigned long)upload_len, ch->volume);
                fflush(dsp_mixlog);
            }
        } else if (d->active) {
            long stop_cmd[2] = { 3, slot };                     // CMD_STOP
            Dsp_BlkUnpacked(stop_cmd, 2, &dummy_out, 0);
            d->active = 0;
            d->address = NULL;
            dsp_poll_calls++;
        }
    }

    dsp_update_count++;
    if ((dsp_update_count % 200) == 1 && dsp_mixlog) {
        fprintf(dsp_mixlog, "poll t=%d  s0 st=%d ty=%d  s1 st=%d ty=%d  s2 st=%d ty=%d  s3 st=%d ty=%d\n",
                dsp_update_count,
                audio.channels[0].state, audio.channels[0].type,
                audio.channels[1].state, audio.channels[1].type,
                audio.channels[2].state, audio.channels[2].type,
                audio.channels[3].state, audio.channels[3].type);
        fflush(dsp_mixlog);
    }
#endif
}

// ============================================================================
// OPL2 FM music synth — host-port FM-block sender.
// sys_opl_frame_tick hands us 9 resolved per-channel opl_block_t's per music
// frame. We send only the CHANGED ones to the DSP's FM_Receive Host Command:
//   word 0 : N (changed-channel count) ; per channel: 27 words
//     B_CH,B_KEY,B_CON,B_FNUM,B_BLK, op0[ML,PM,AM,EG,AR,DR,SL,RR,FB,TLL,RKS], op1[..]
// matching FM_Receive/ApplyBlock on the DSP (validated in tools/dsp_sim).
// ============================================================================
#include "opl_block.h"
#include <string.h>

#define FM_HCV_VEC  0x2A      // DSP host-command vector P:$2A -> CVR = 0x80|($2A/2) = 0x95

int dsp_opl_available = 0;    // set to 1 once the merged DSP program (w/ FM_Receive) is up

static opl_block_t fm_last[9];          // last-sent block per channel (change detection)
static long fm_sendbuf[9*27 + 1];       // staged words: [0]=N, then N*27
static long fm_send_count = 0;

// Trigger FM_Receive and blast the staged words. Supervisor (protected $FFA2xx I/O).
DSP_FUNC static void dsp_opl_send_super(void)
{
    volatile u8  *cvr  = (volatile u8  *)0xFFA201;
    volatile u8  *isr  = (volatile u8  *)0xFFA202;
    volatile u32 *data = (volatile u32 *)0xFFA204;

    long t = 200000;
    while ((*cvr & 0x80) && --t) ;       // wait for any pending host command to clear
    *cvr = (u8)(0x80 | (FM_HCV_VEC >> 1));

    for (long i = 0; i < fm_send_count; i++) {
        t = 500000;
        while (!(*isr & 0x02)) { if (--t == 0) return; }   // wait HTDE (transmit ready)
        *data = (u32)fm_sendbuf[i];
    }
}

void dsp_opl_feed(const opl_block_t *blocks)
{
    if (!dsp_opl_available) return;

    long *p = fm_sendbuf + 1;            // [0] reserved for N
    int n = 0;
    for (int ch = 0; ch < 9; ch++) {
        const opl_block_t *b = &blocks[ch];
        if (memcmp(b, &fm_last[ch], sizeof *b) == 0) continue;   // unchanged -> skip
        fm_last[ch] = *b;
        *p++ = ch; *p++ = b->key; *p++ = b->connection; *p++ = b->fnum; *p++ = b->blk;
        for (int op = 0; op < 2; op++) {
            *p++ = b->op[op].ML; *p++ = b->op[op].PM; *p++ = b->op[op].AM; *p++ = b->op[op].EG;
            *p++ = b->op[op].AR; *p++ = b->op[op].DR; *p++ = b->op[op].SL; *p++ = b->op[op].RR;
            *p++ = b->op[op].FB; *p++ = (long)b->op[op].tll; *p++ = (long)b->op[op].rks;
        }
        n++;
    }
    if (n == 0) return;                  // nothing changed -> no host command this frame
    fm_sendbuf[0] = n;
    fm_send_count = 1 + n * 27;
    dsp_opl_send_super();                 // direct: called from the supervisor Timer-A ISR
}

// ---- FM-only DSP program loader (NoCrew, 3 sections) + init + per-frame trigger ----
// Loads dsp_opl_fm (vectors+ISR at P:$0, code at EXTERNAL P:$200, tables at Y:$1000)
// — three sections kept separate so the load never clobbers the loader at $40-$1FF.
#include "dsp_opl_fm_bin.h"

// FM can't keep up at 24585 Hz (code runs from external P): $FF8935 0x07 ≈ 12292 Hz.
// Lower further (0x0B/0x0F) if it underruns.
#ifndef OPL_FM_PRESCALE
#define OPL_FM_PRESCALE 0x07
#endif
#ifndef OPL_FM_LENGTH
#define OPL_FM_LENGTH   246           // ~12292/50 samples per frame
#endif
#ifndef OPL_FM_OUTHZ
#define OPL_FM_OUTHZ    12292         // output rate (must match OPL_FM_PRESCALE)
#endif
#define OPL_FM_WINMAX   512           // per-voice window cap (must match DSP WINMAX)
#define OPL_FM_SFX_GAIN 16            // game vol (0..127) → DSP vol scale (tune by ear)

// Per-SFX-voice resampler position (12.12 source-sample units) + last sample addr.
static u32  fm_sfx_pos[4];
static s8  *fm_sfx_addr[4];

// Reset DSP + send the 512-word NoCrew bootstrap + the 3 program sections + launch.
DSP_FUNC static void dsp_opl_fm_boot_super(void)
{
    volatile u8  *ym_sel = (volatile u8  *)0xFFFF8800;
    volatile u8  *ym_wr  = (volatile u8  *)0xFFFF8802;
    volatile u8  *isr    = (volatile u8  *)0xFFA202;
    volatile u8  *dh     = (volatile u8  *)0xFFA205;
    volatile u8  *dm     = (volatile u8  *)0xFFA206;
    volatile u8  *dl     = (volatile u8  *)0xFFA207;
    volatile u32 *data32 = (volatile u32 *)0xFFA204;

    *(volatile u8 *)0xFFFF8937 = 0;   // mute DAC during DSP reset+reload (matrix re-enables it)
    dbg_bootstrap_entered = 1; dbg_boot_fail = 0;
    *ym_sel = 0x0E; u8 pa = *ym_sel;
    *ym_wr = pa & ~0x10;
    *ym_wr = (pa & ~0x10) | 0x10;
    for (volatile int i = 0; i < 20000; i++) ;
    *ym_wr = pa & ~0x10;
    for (volatile int i = 0; i < 100000; i++) ;
    *ym_sel = 0x0E;

    for (int w = 0; w < 512; w++) {
        long t = 500000;
        while (!(*isr & 0x02)) { if (--t == 0) { dbg_boot_fail = 1; return; } }
        *dh = nocrew_bootstrap_code[w*3]; *dm = nocrew_bootstrap_code[w*3+1]; *dl = nocrew_bootstrap_code[w*3+2];
    }
    #define SW(v) do { long _t=500000; \
        while(!(*isr&0x02)){if(--_t==0){dbg_boot_fail=2;return;}} \
        *dh=(u8)((u32)(v)>>16); *dm=(u8)((u32)(v)>>8); *dl=(u8)((u32)(v)); } while(0)
    #define SECTION(type,addr,buf,size) do { SW(type); SW(addr); SW(size); \
        for (u32 _w=0; _w<(u32)(size); _w++) { long _t=500000; \
            while(!(*isr&0x02)){if(--_t==0){dbg_boot_fail=3;return;}} \
            *dh=(buf)[_w*3]; *dm=(buf)[_w*3+1]; *dl=(buf)[_w*3+2]; } } while(0)
    SECTION(0, DSP_OPL_FM_PLO_ADDR, dsp_opl_fm_plo, DSP_OPL_FM_PLO_SIZE);  // P low  (internal, vectors+ISR)
    SECTION(0, DSP_OPL_FM_PHI_ADDR, dsp_opl_fm_phi, DSP_OPL_FM_PHI_SIZE);  // P high (EXTERNAL, code)
    SECTION(2, DSP_OPL_FM_Y_ADDR,   dsp_opl_fm_y,   DSP_OPL_FM_Y_SIZE);    // Y      (EXTERNAL, tables)
    { long t = 500000; while (!(*isr & 0x02)) { if (--t == 0) { dbg_boot_fail = 4; return; } } }
    *data32 = 3;                          // launch
    #undef SW
    #undef SECTION
    dbg_bootstrap_entered = 2;
}

// Audio matrix: DSP is SSI clock master → DAC (same as the tracker), FM output rate.
DSP_FUNC static void dsp_opl_fm_matrix_super(void)
{
    *(volatile u8  *)0xFFFF8901 = 0x00;
    *(volatile u8  *)0xFFFF8920 = 0x0F;
    *(volatile u16 *)0xFFFF8930 = 0x0091;
    *(volatile u16 *)0xFFFF8932 = 0x2213;
    *(volatile u8  *)0xFFFF8935 = OPL_FM_PRESCALE;
    *(volatile u8  *)0xFFFF8937 = 0x03;
}

// Trigger FM_Frame (HCV $26 → CVR 0x93) and push: frame length, the SFX voice count
// N, then per active SFX voice (VOL, STEP, PHASE, window-length + resampled window of
// source samples). The DSP renders FM then resample-mixes these voices on top.
DSP_FUNC static void dsp_opl_fm_frame_super(void)
{
    volatile u8  *cvr  = (volatile u8 *)0xFFA201;
    volatile u8  *isr  = (volatile u8 *)0xFFA202;
    volatile u32 *data = (volatile u32 *)0xFFA204;
    long t;
    #define FSEND(v) do { long _t=500000; while(!(*isr&0x02)){if(--_t==0)return;} *data=(u32)(long)(v); } while(0)

    t = 200000; while ((*cvr & 0x80) && --t) ;
    *cvr = 0x93;
    FSEND(OPL_FM_LENGTH);

    // Decide which of the 4 SFX channels are actively playing a sample.
    int active[4], nact = 0;
    for (int v = 0; v < 4; v++) {
        sChannel *gc = &audio.channels[v];
        if (gc->address != fm_sfx_addr[v]) { fm_sfx_addr[v] = gc->address; fm_sfx_pos[v] = 0; }
        int wants = (audio.fsound && gc->state >= 0 && gc->type == eChannelTypeSample
                     && gc->address != NULL && gc->length > 0 && gc->freq > 0);
        if (wants && (fm_sfx_pos[v] >> 12) < gc->length) active[nact++] = v;
    }
    FSEND(nact);

    for (int k = 0; k < nact; k++) {
        int v = active[k];
        sChannel *gc = &audio.channels[v];
        long step  = ((long)gc->freq << 12) / OPL_FM_OUTHZ;   // 12.12 source samples/output
        if (step < 1) step = 1;
        u32  pos   = fm_sfx_pos[v];
        u32  ws    = pos >> 12;
        long phase = (long)(pos & 0xFFF);
        long wlen  = ((phase + (long)OPL_FM_LENGTH * step) >> 12) + 2;
        if (wlen > OPL_FM_WINMAX) wlen = OPL_FM_WINMAX;
        long avail = (long)gc->length - (long)ws;             // real samples left
        long vol   = (long)gc->volume * OPL_FM_SFX_GAIN;
        FSEND(vol); FSEND(step); FSEND(phase); FSEND(wlen);
        s8 *src = gc->address + ws;
        for (long i = 0; i < wlen; i++)
            FSEND(i < avail ? (long)src[i] : 0L);             // zero-pad past the sample end
        fm_sfx_pos[v] = pos + (u32)((long)OPL_FM_LENGTH * step);
    }
    #undef FSEND
}

void dsp_opl_fm_frame(void) { if (dsp_opl_available) dsp_opl_fm_frame_super(); }

// Mute all FM slots (HCV $2C → CVR 0x96). Called from the supervisor Timer-A ISR
// when OPL music stops, so the FM render falls silent while SFX keep playing.
static void dsp_opl_fm_silence(void)
{
    if (!dsp_opl_available) return;
    volatile u8 *cvr = (volatile u8 *)0xFFA201;
    long t = 200000;
    while ((*cvr & 0x80) && --t) ;
    *cvr = (u8)(0x80 | (0x2C >> 1));         // FM_Silence vector $2C
    memset(fm_last, 0, sizeof fm_last);      // force a full re-send when music resumes
}

// Liveness probe: send one word, read the reply. Start replies #12345678 ($BC614E)
// once it has actually executed (in external P). -1=TX timeout, -2=RX timeout.
static volatile long fm_probe_value;
DSP_FUNC static void dsp_opl_fm_probe_super(void)
{
    volatile u8 *isr = (volatile u8 *)0xFFA202;
    volatile u8 *dh  = (volatile u8 *)0xFFA205;
    volatile u8 *dm  = (volatile u8 *)0xFFA206;
    volatile u8 *dl  = (volatile u8 *)0xFFA207;
    long t = 1000000;
    while (!(*isr & 0x02)) { if (--t == 0) { fm_probe_value = -1; return; } }
    *dh = 0; *dm = 0; *dl = 0;
    t = 1000000;
    while (!(*isr & 0x01)) { if (--t == 0) { fm_probe_value = -2; return; } }
    fm_probe_value = ((long)*dh << 16) | ((u32)*dm << 8) | *dl;
}

int dsp_opl_fm_init(void)
{
    FILE *dlog = fopen("dsp_opl_fm.txt", "w");
    // One-time DSP setup, guarded (a swap re-enters this to re-bootstrap the FM program).
    if (!dsp_setup_done) {
        if ((short)Dsp_GetWordSize() != 3) { if (dlog) { fprintf(dlog,"no DSP\n"); fclose(dlog);} return -1; }
        Dsp_Lock(); Locksnd();
        if (!g_dma_buf) g_dma_buf = (u8 *)sys_Mxalloc(8000, 0);
        if (!g_dma_buf) { if (dlog){fprintf(dlog,"Mxalloc fail\n");fclose(dlog);} return -1; }
        dsp_setup_done = 1;
    }
    if (dlog) { fprintf(dlog, "Loading FM program: P_LO %d@$%X, P_HI %d@$%X, Y %d@$%X\n",
        DSP_OPL_FM_PLO_SIZE, DSP_OPL_FM_PLO_ADDR, DSP_OPL_FM_PHI_SIZE, DSP_OPL_FM_PHI_ADDR,
        DSP_OPL_FM_Y_SIZE, DSP_OPL_FM_Y_ADDR); fflush(dlog); }
    Supexec(dsp_opl_fm_boot_super);
    if (dlog) { fprintf(dlog, "bootstrap: entered=%ld fail=%ld (0=ok,1=boot,2=hdr,3=data,4=launch)\n",
        dbg_bootstrap_entered, dbg_boot_fail); fflush(dlog); }
    if (dbg_boot_fail || dbg_bootstrap_entered != 2) { dsp_opl_available = 0;
        if (dlog) { fprintf(dlog, "LOAD FAILED\n"); fclose(dlog); } return -1; }
    for (volatile long i = 0; i < 200000; i++) ;
    // Liveness: confirm Start actually ran (executes from external P + launch went to $0).
    Supexec(dsp_opl_fm_probe_super);
    if (dlog) fprintf(dlog, "liveness probe: got=$%06lX (expect $BC614E; "
        "-1=TX timeout, -2=RX timeout = DSP not running our code)\n", fm_probe_value & 0xFFFFFF);
    if ((fm_probe_value & 0xFFFFFF) != 0xBC614E) {
        if (dlog) { fprintf(dlog, "DSP NOT ALIVE — external-P execution or launch failed. "
            "FM disabled.\n"); fclose(dlog); }
        dsp_opl_available = 0;
        return -1;
    }
    Supexec(dsp_opl_fm_matrix_super);     // re-enables the DAC ($8937=0x03)
    dsp_dac_muted = 0;
    memset(fm_last, 0, sizeof fm_last);   // force a full re-send of all channels after a swap
    memset(fm_sfx_pos, 0, sizeof fm_sfx_pos);
    memset(fm_sfx_addr, 0, sizeof fm_sfx_addr);
    fm_silenced = 0;
    dsp_opl_available = 1;
    dsp_current_prog = DSPPROG_FM;
    if (dlog) { fprintf(dlog, "FM program ALIVE + up (rate prescale $%02X, length %d/frame). "
        "dsp_opl_available=1\n", OPL_FM_PRESCALE, OPL_FM_LENGTH); fclose(dlog); }
    return 0;
}

// ---- Music-type → DSP-program selector (called from the main loop, NOT an ISR) ----
// Sampled/chip music ⇒ tracker program; OPL2 music ⇒ FM+SFX program. Re-bootstraps
// the DSP on a change. The re-bootstrap is slow (~ms) but music transitions are rare.
DSP_FUNC static void dsp_mute_super(void) { *(volatile u8 *)0xFFFF8937 = 0; }

static void dsp_audio_select_for_music(void)
{
    int want = (audio.soundrout == mv2_opl2rout) ? DSPPROG_FM : DSPPROG_TRACKER;
    if (want == dsp_current_prog) return;
    // Mute the DAC before the old program loops its last frame + across the reload.
    Supexec(dsp_mute_super);
    dsp_dac_muted = 1;
    // Stop the ISR feed while we reload (clear flags + prog so dsp_mixer_tick_isr skips).
    dsp_current_prog   = DSPPROG_NONE;
    dsp_mixer_available = 0;
    dsp_opl_available   = 0;
    if (want == DSPPROG_FM) dsp_opl_fm_init();
    else                    dsp_mixer_init();
}

#endif // ALIS_DSP_MIXER && __atarist__
