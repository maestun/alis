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

#include "sys.h"
#include "alis.h"
#include "audio.h"
#include "channel.h"
#include "../audio/dsp_mixer.h"
#include "../audio/opl_host.h"           // host-side OPL register -> FM block translator
#include "image.h"
#include "mem.h"
#include "platform.h"
#include "utils.h"

#include "emu2149.h"
#include "emu8950.h"
#include "math.h"
#include "utils.h"

// 0 No interpolation
// 1 cubic
// 2 hermite
// 3 hermite_4pt_3ox

#ifndef ALIS_SND_INTERPOLATE_TYPE
# define ALIS_SND_INTERPOLATE_TYPE 1
#endif

extern const u32 k_event_ticks;
extern const u32 k_frame_ticks;

#if ALIS_SDL_VER == 1

#include <SDL/SDL.h>

SDL_keysym button = { 0, 0, 0, 0 };
SDL_Rect dirty_rects[256];
SDL_Rect dirty_mouse_rect;
// The native build defines these in sys_atari.c.
#if !defined(ALIS_USE_NATIVE_ATARI)
volatile u8 dirty_pal = 1;
u8 dirty_len = 0;
#else
extern volatile u8 dirty_pal;   // defined in sys_atari.c (Timer-C ISR writes it)
extern u8 dirty_len;
#endif

#define SDLK_KP_0   SDLK_KP0
#define SDLK_KP_1   SDLK_KP1
#define SDLK_KP_2   SDLK_KP2
#define SDLK_KP_3   SDLK_KP3
#define SDLK_KP_4   SDLK_KP4
#define SDLK_KP_5   SDLK_KP5
#define SDLK_KP_6   SDLK_KP6
#define SDLK_KP_7   SDLK_KP7
#define SDLK_KP_8   SDLK_KP8
#define SDLK_KP_9   SDLK_KP9

#define SDL_GetKeyboardState    SDL_GetKeyState
#define sys_sleep(t) sys_delay(t); sys_poll_event();

#elif ALIS_SDL_VER == 2

#if defined(_MSC_VER)
# include "SDL.h"
# include "SDL_thread.h"
#elif __has_include(<SDL.h>)
# include <SDL.h>
# include <SDL_thread.h>
#else
# include <SDL2/SDL.h>
# include <SDL2/SDL_thread.h>
#endif

SDL_Renderer    *renderer;
SDL_Texture     *texture;
SDL_Window      *window;
SDL_Cursor      *cursor;
SDL_Keysym      button = { 0, 0, 0, 0 };
SDL_TimerID     timer_id;
SDL_sem         *render_sem = NULL;

#define sys_sleep(t) usleep(t)

#endif

SDL_Event       event;

u8              joystick0 = 0;
u8              joystick1 = 0;
u8              shift = 0;

// SDL stick #0, opened in sys_init (SDL2 also hot-plugs it). NULL if none.
SDL_Joystick   *sys_joy_handle = NULL;
#if ALIS_SDL_VER == 2
SDL_JoystickID  sys_joy_instance_id = -1;
#endif

mouse_t         mouse;

bool            dirty_mouse;
float           scale = 2;
float           aspect_ratio = 1.2;
float           scale_x;
float           scale_y;
int             opt_scale = 0;  // 0 = platform default (SDL2: 2, SDL1: 1); else 1..4 from --sN
int             atari_no_timerc = 0;  // --no-timerc: skip Timer C install on Atari (etv_term test only)
int             atari_audio_rate = 0;    // --audio-rate=N: DMA output rate (0 = auto from _CPU cookie, blind to clock speed)
int             atari_audio_stereo = 0;  // --audio-stereo: Falcon 8-bit stereo DMA instead of mono (the mix is mono)
int             atari_audio_backend = 2;  // --audio: 0=auto, 1=force DSP, 2=force DMA (Atari)
u32             width = 320;
u32             height = 200;

int             audio_id;
SDL_AudioSpec   *audio_spec;
#if !defined(__TOS__) && !defined(__atarist__)
PSG             *audio_psg;
OPL             *audio_opl;
#endif

#if defined(__TOS__) || defined(__atarist__)
// Host-side translator: decodes the OPL register stream into resolved per-channel
// FM blocks for the DSP synth (validated bit-exact vs emu8950, see tools/opl_*).
static opl_host_t *sys_opl_host = NULL;
#endif

extern u8       *vgalogic_df;

u8              failure;

u32             poll_ticks;
struct timeval  frame_time;
struct timeval  loop_time;

// 16.16 fixed point for ISR timing (no FPU on 68030)
u32             isr_step;    // 16.16 fixed-point: 50 * 65536 / host_freq
u32             isr_counter; // 16.16 fixed-point accumulator

// Rate at which alis.timeclock advances (set by each backend's sys_init).
// FLI audio uses it to lock speech rate to the frame interval.
u32             sys_timeclock_hz = 50;

// Original game's interrupt cadence (50 Hz Atari/Amiga VBL, 60 Hz DOS PIT). See sys.h.
u32             sys_sfx_tick_hz = 50;

// The games were made on the ST, so frames, palette fades and music run at its 50 Hz;
// --native-timing uses the port's own rates instead (DOS: VGA refresh, Mac: 60 Hz ticks).
int             opt_native_timing = 0;

u32 sys_pace_hz(void)
{
    if (opt_native_timing && alis.platform.kind == EPlatformPC)  return 70;
    if (opt_native_timing && alis.platform.kind == EPlatformMac) return 60;
    return 50;
}

u32 sys_music_hz(void)
{
    if (opt_native_timing && (alis.platform.kind == EPlatformPC || alis.platform.kind == EPlatformMac))
        return 60;
    return 50;
}

// FLI audio mixer-side logging (fli_dbg_log lives in video.c).
#ifndef ALIS_FLI_AUDIO_DEBUG
#define ALIS_FLI_AUDIO_DEBUG 0
#endif
#if ALIS_FLI_AUDIO_DEBUG
extern void fli_dbg_log(const char *fmt, ...);
#define FLI_DBG_MIX(...) fli_dbg_log(__VA_ARGS__)
#else
#define FLI_DBG_MIX(...) ((void)0)
#endif


u8 giaccess(s8 cmd, u8 data, u8 ch);
bool priorblanc(sChannel *channel);
u8 pblanc10(sChannel *a0, u8 d1b);

void sys_audio_callback_S16MSB(void *userdata, u8 *stream, s32 len);
void sys_audio_callback_S8(void *userdata, u8 *stream, s32 len);

// ============================================================================
#pragma mark - Signals
// ============================================================================

void signals_handler(int signo) {

    if (signo>0) {
        failure = signo;
    }
    else {
        failure = 1;
    }
    alis.state = eAlisStateStopped;

#if defined(__atarist__) || defined(__TOS__)
    // Unhook etv_timer first, before any teardown: TOS would otherwise keep
    // calling our trampoline from Timer C after the process is gone.
    extern void atari_timerc_emergency_uninstall(void);
    atari_timerc_emergency_uninstall();
#endif

    signal(signo, SIG_DFL);
}

void sys_errors_init(void) {

    failure = 0;
    signal(SIGSEGV, signals_handler);
#ifdef SIGBUS
    signal(SIGBUS,  signals_handler);  // MiNT memprot trap uses SIGBUS
#endif
    signal(SIGABRT, signals_handler);
    signal(SIGFPE,  signals_handler);
    signal(SIGINT,  signals_handler);
    signal(SIGTERM, signals_handler);
    signal(SIGILL,  signals_handler);

#if defined (_WIN32) || defined (__MINGW32__)
#endif
}

void sys_errors_deinit(void) {

    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE,  SIG_DFL);
    signal(SIGINT,  SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGILL,  SIG_DFL);
}

void signals_info(int signo) {

    if (signo == 0) return;

    printf("\n");
    switch (signo) {
        case SIGSEGV:
        {
            ALIS_DEBUG(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGSEGV: Invalid memory access (segmentation fault).\n");
            break;
        }
        case SIGABRT:
        {
            ALIS_DEBUG(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGABRT: Abnormal termination.\n");
            break;
        }
        case SIGFPE:
        {
            ALIS_DEBUG(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGFPE: Erroneous arithmetic operation.\n");
            break;
        }
        case SIGINT:
        {
            ALIS_DEBUG(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGINT: External interrupt.\n");
            break;
        }
        case SIGTERM:
        {
            ALIS_DEBUG(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGTERM: Termination request.\n");
            break;
        }
        case SIGILL:
        {
            ALIS_DEBUG(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGILL: Invalid program image.\n");
            break;
        }
        default:
            if (disalis) {
                ALIS_DEBUG(EDebugFatal, "FATAL ERROR: An unidentified error %d occurred.\n", signo);
            }
            else {
                ALIS_DEBUG(EDebugFatal, "An unidentified error %d occurred.\n", signo);
            }
    }
    
    ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the host system queue...\n");
}


// ============================================================================
#pragma mark - Audio
// ============================================================================

// Atari: PSG writes go to the real YM2149 (mixed with DMA at the speaker), so
// no software synthesis; emu2149 is excluded from the build (ATARI_EXCLUDES).

#if defined(__TOS__) || defined(__atarist__)
# include <mint/osbind.h>   /* Giaccess */
#endif

void sys_init_psg(void)
{
#if !defined(__TOS__) && !defined(__atarist__)
    // ST YM2149: 2 MHz, no internal divider. Quality=0 keeps tiny tone
    // periods audible (cexplode/cnoise rely on noise leaking through).
    audio_psg = PSG_new(2000000, audio_spec->freq);
    PSG_setClockDivider(audio_psg, 0);
    PSG_setVolumeMode(audio_psg, 1);
    PSG_setQuality(audio_psg, 0);
    PSG_reset(audio_psg);
#endif
    /* Real YM2149 needs no init — TOS already drives it for keyboard
     * click etc. and our writes layer on top. */
}

void sys_deinit_psg(void)
{
    // Zero the channel volumes (regs 8-10): the real chip latches and would keep
    // humming after exit. Reg 7 holds port bits TOS needs, so leave it.
    sys_write_psg(8,  0);
    sys_write_psg(9,  0);
    sys_write_psg(10, 0);
#if !defined(__TOS__) && !defined(__atarist__)
    PSG_delete(audio_psg);
    audio_psg = NULL;
#endif
}

// Nonzero while YM writes run in ISR context. XBIOS Giaccess isn't reentrant
// (preempting a main-loop trap corrupts its state → reset), so ISRs hit the
// YM2149 registers directly; the main loop keeps Giaccess (user-mode safe).
volatile u8 g_psg_isr_context = 0;

#if defined(__TOS__) || defined(__atarist__)
// Direct YM2149 access: $FFFF8800 = register-select / read-data, $FFFF8802 = write-data.
// Mask to IPL 7 so the select+data pair can't be split by another MFP source.
static inline void psg_hw_write(u8 reg, u8 val)
{
    volatile u8 *sel = (volatile u8 *)0xFFFF8800UL;
    volatile u8 *dat = (volatile u8 *)0xFFFF8802UL;
    unsigned short sr;
    __asm__ volatile("move.w %%sr,%0\n\tori.w #0x0700,%%sr" : "=d"(sr) : : "cc", "memory");
    *sel = reg; *dat = val;
    __asm__ volatile("move.w %0,%%sr" : : "d"(sr) : "cc", "memory");
}
static inline u8 psg_hw_read(u8 reg)
{
    volatile u8 *sel = (volatile u8 *)0xFFFF8800UL;
    unsigned short sr; u8 v;
    __asm__ volatile("move.w %%sr,%0\n\tori.w #0x0700,%%sr" : "=d"(sr) : : "cc", "memory");
    *sel = reg; v = *sel;
    __asm__ volatile("move.w %0,%%sr" : : "d"(sr) : "cc", "memory");
    return v;
}
#endif

void sys_write_psg(u32 reg, u32 val)
{
#if defined(__TOS__) || defined(__atarist__)
    if (g_psg_isr_context) { psg_hw_write((u8)(reg & 0x0f), (u8)val); return; }
    /* Giaccess(data, reg). Bit 7 of reg = write-enable. Registers
     * 0..15. Routed through XBIOS so it works in user mode without
     * Super(). */
    Giaccess((u8)val, (u8)((reg & 0x0f) | 0x80));
#else
    PSG_writeReg(audio_psg, reg, val);
#endif
}

u8 sys_read_psg(u32 reg)
{
#if defined(__TOS__) || defined(__atarist__)
    if (g_psg_isr_context) return psg_hw_read((u8)(reg & 0x0f));
    /* Bit 7 clear → read register. */
    return (u8)Giaccess(0, (u8)(reg & 0x0f));
#else
    return PSG_readReg(audio_psg, reg);
#endif
}

void sys_calc_psg_music(void)
{
#if !defined(__TOS__) && !defined(__atarist__)
    memset(audio.muadresse, 0, audio.mutaloop * 2);
    for (int index = 0; index < audio.mutaloop; index ++)
    {
        audio.muadresse[index] = PSG_calc(audio_psg) * 4;
    }
#endif
}

void sys_init_opl(void)
{
#if !defined(__TOS__) && !defined(__atarist__)
    if (audio_opl)
        return; // already initialized

    audio_opl = OPL_new(3579545, audio_spec->freq);
    OPL_setChipType(audio_opl, 2); // YM3812
    OPL_reset(audio_opl);
#else
    if (!sys_opl_host)                            // Atari: FM-synth on the DSP
        sys_opl_host = opl_host_new();
#endif
}

void sys_deinit_opl(void)
{
#if !defined(__TOS__) && !defined(__atarist__)
    if (audio_opl)
    {
        OPL_delete(audio_opl);
        audio_opl = NULL;
    }
#else
    if (sys_opl_host)
    {
        opl_host_delete(sys_opl_host);
        sys_opl_host = NULL;
    }
#endif
}

// ============================================================================
// OPL2 → DSP bridge
// ----------------------------------------------------------------------------
// Desktop: OPL writes go to emu8950. Atari (no AdLib, emu8950 too slow): writes
// feed opl_host, and sys_opl_frame_tick() pushes the resolved FM blocks to the
// DSP synth once per music frame.
//
// --opl-capture dumps the register stream: 16-byte header {"OPLC", u32 freq,
// u32 frame_samples, u32 reserved}, then 0x00,reg,val = write; 0x01 = one frame.
// ============================================================================
#define SYS_OPL_QUEUE_MAX 256                     // max OPL writes buffered per frame
int opl_capture_flag = 0;                         // set by --opl-capture

u8  sys_opl_q_reg[SYS_OPL_QUEUE_MAX];             // queued register writes
u8  sys_opl_q_val[SYS_OPL_QUEUE_MAX];             //   (host → DSP, drained per frame)
u16 sys_opl_q_count = 0;

static FILE *sys_opl_cap = NULL;                  // capture file (NULL = off)

static void sys_opl_capture_open_if_needed(void)
{
    if (!opl_capture_flag || sys_opl_cap)
        return;
    sys_opl_cap = fopen("opl_capture.bin", "wb");
    opl_capture_flag = 0;                         // open once
    if (sys_opl_cap)
    {
        u32 freq = audio_spec ? (u32)audio_spec->freq : 0;
        u32 fsmp = (u32)audio.mutaloop;
        u32 rsv  = 0;
        fwrite("OPLC", 1, 4, sys_opl_cap);
        fwrite(&freq, 4, 1, sys_opl_cap);
        fwrite(&fsmp, 4, 1, sys_opl_cap);
        fwrite(&rsv,  4, 1, sys_opl_cap);
    }
}

void sys_write_opl(u32 reg, u8 val)
{
    sys_opl_capture_open_if_needed();
    if (sys_opl_cap)
    {
        unsigned char rec[3] = { 0x00, (unsigned char)reg, val };
        fwrite(rec, 1, 3, sys_opl_cap);
    }

#if !defined(__TOS__) && !defined(__atarist__)
    if (audio_opl)
        OPL_writeReg(audio_opl, reg, val);        // desktop: emu8950 (the reference)
#else
    if (sys_opl_host)                              // Atari: feed the FM-block translator
        opl_host_write_reg(sys_opl_host, reg, val);
    if (sys_opl_q_count < SYS_OPL_QUEUE_MAX)        // (raw queue kept for capture/debug)
    {
        sys_opl_q_reg[sys_opl_q_count] = (u8)reg;
        sys_opl_q_val[sys_opl_q_count] = val;
        sys_opl_q_count++;
    }
#endif
}

// Called once per music frame (end of mv2_opl2rout), after that frame's writes
// and render. Marks the frame boundary in the capture and drains the queue.
void sys_opl_frame_tick(void)
{
    if (sys_opl_cap)
    {
        unsigned char rec = 0x01;
        fwrite(&rec, 1, 1, sys_opl_cap);
    }
#if defined(__TOS__) || defined(__atarist__)
    // Resolve the 9 channels into FM blocks, apply (FM_Receive), then
    // trigger one rendered frame (FM_Frame) into the DSP SampleBuffer.
    if (sys_opl_host && dsp_opl_available)
    {
        opl_block_t blocks[9];
        for (int ch = 0; ch < 9; ch++)
            opl_host_get_block(sys_opl_host, ch, &blocks[ch]);
        dsp_opl_feed(blocks);
        dsp_opl_fm_frame();
    }
#endif
    sys_opl_q_count = 0;
}

// WAV export for music debugging - accumulates float samples, writes on stop
extern s32 save_wave_file(const char *name, float *data, s32 sample_rate, s32 channel_count, s32 sample_count);

static float *wav_export_buf = NULL;
static u32 wav_export_samples = 0;
static u32 wav_export_capacity = 0;
static char wav_export_path[256] = {0};
static u8 wav_export_active = 0;

void sys_wav_export_start(const char *path)
{
    if (wav_export_active)
        return;

    // Pre-allocate for ~60 seconds at host sample rate
    u32 rate = audio_spec ? audio_spec->freq : 44100;
    wav_export_capacity = rate * 60;
    wav_export_buf = (float *)malloc(wav_export_capacity * sizeof(float));
    if (!wav_export_buf)
        return;

    wav_export_samples = 0;
    strncpy(wav_export_path, path, sizeof(wav_export_path) - 1);
    wav_export_active = 1;

    ALIS_DEBUG(EDebugWarning, "WAV export started: %s\n", path);
}

void sys_wav_export_stop(void)
{
    if (!wav_export_active || !wav_export_buf)
        return;

    u32 rate = audio_spec ? audio_spec->freq : 44100;
    save_wave_file(wav_export_path, wav_export_buf, rate, 1, wav_export_samples);

    free(wav_export_buf);
    wav_export_buf = NULL;
    wav_export_active = 0;

    ALIS_DEBUG(EDebugWarning, "WAV export stopped: %u samples written to %s\n", wav_export_samples, wav_export_path);
}

static void sys_wav_export_write(s16 *samples, int count)
{
    if (!wav_export_active || !wav_export_buf)
        return;

    for (int i = 0; i < count && wav_export_samples < wav_export_capacity; i++)
    {
        wav_export_buf[wav_export_samples++] = (float)samples[i] / 32768.0f;
    }
}

void sys_calc_opl_music(void)
{
#if !defined(__TOS__) && !defined(__atarist__)
    memset(audio.muadresse, 0, audio.mutaloop * 2);
    for (int index = 0; index < audio.mutaloop; index ++)
    {
        audio.muadresse[index] = OPL_calc(audio_opl);
    }

    sys_wav_export_write(audio.muadresse, audio.mutaloop);
#endif
}

float interpolate_cubic(float x0, float x1, float x2, float x3, float t)
{
    float a0, a1, a2, a3;
    a0 = x3 - x2 - x0 + x1;
    a1 = x0 - x1 - a0;
    a2 = x2 - x0;
    a3 = x1;
    return (a0 * (t * t * t)) + (a1 * (t * t)) + (a2 * t) + (a3);
}

float interpolate_hermite_4pt_3ox(float x0, float x1, float x2, float x3, float t)
{
    float c0 = x1;
    float c1 = .5F * (x2 - x0);
    float c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
    float c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
    return (((((c3 * t) + c2) * t) + c1) * t) + c0;
}

float interpolate_hermite(float x0, float x1, float x2, float x3, float t)
{
    float diff = x1 - x2;
    float c1 = x2 - x0;
    float c3 = x3 - x0 + 3 * diff;
    float c2 = -(2 * diff + c1 + c3);
    return 0.5f * ((c3 * t + c2) * t + c1) * t + x1;
}

bool priorblanc(sChannel *channel)
{
    u8 result = 1;
    if (channel->curson < audio.channels[0].curson)
        result = pblanc10(&(audio.channels[0]), result);

    if (channel->curson < audio.channels[1].curson)
        result = pblanc10(&(audio.channels[1]), result);

    if (channel->curson < audio.channels[2].curson)
        result = pblanc10(&(audio.channels[2]), result);
    
    return result == 0;
}

u8 pblanc10(sChannel *channel, u8 d1b)
{
    return ((channel->type & 2) != 0) ? 0 : d1b;
}

void io_canal(sChannel *channel, s16 index)
{
    // Mirrors Falcon RRQ asm io_canal (rrq-falcon.asm:0xD3EE).
    // Both early-returns are required: index > 2 would clobber reg 6
    // (noise period); sample channels (type & 0x80) must not touch PSG.
    if (index > 2) return;
    if (channel->type & 0x80) return;

    giaccess(index + 0x88, (channel->volume >> 11) & 0xf, index);
    giaccess(index * 2 + 0x80, (channel->freq >> 3) & 0xff, index);
    giaccess(index * 2 + 0x81, ((channel->freq >> 3) >> 8) & 0xf, index);
    
    u8 mixer = giaccess(7, 0, index);

    if (index < 3)
    {
        mixer = mixer | 1 << (index & 0x1f) | 1 << ((index + 3) & 0x1f);
        if ((channel->type & 1) != 0)
        {
            mixer &= ~(1 << (index & 0x1f));
        }
        
        if ((channel->type & 2) != 0)
        {
            mixer &= ~(1 << ((index + 3) & 0x1f));
            if (!priorblanc(channel))
            {
                giaccess(0x86, (u8)((u16)channel->freq >> 10), index);
            }
        }
    }

    giaccess(0x87, mixer, index);
}

// One 50/60 Hz tick of the YM effect channels (DingZap/Noise/Explode), same as
// the audio callback's isr_counter step. The Falcon DSP path bypasses that
// callback, so dsp_mixer_tick_isr() calls this from the Timer-A ISR.
void sys_psg_tick(void)
{
    for (int i = 0; i < 3; i++)
    {
        sChannel *ch = &audio.channels[i];
        if (ch->type != eChannelTypeDingZap
            && ch->type != eChannelTypeNoise
            && ch->type != eChannelTypeExplode)
            continue;

        do
        {
            if (ch->played < ch->length)
            {
                ch->played++;

                s32 vol = (s32)ch->delta_volume + (s32)ch->volume;
                if (-1 < vol)
                {
                    if (0x7fff < vol) vol = 0x7fff;
                    ch->volume = (s16)vol;
                    u32 freq = ch->delta_freq + ch->freq;
                    if (-1 < (s32)(freq << 0x10))
                    {
                        ch->freq = freq;
                        io_canal(ch, i);
                        break;
                    }
                }
            }

            ch->type   = eChannelTypeNone;
            ch->volume = 0;
            ch->freq   = 0;
            ch->curson = 0x80;
            ch->state  = 0;
            ch->played = 0;
            io_canal(ch, i);
        }
        while (false);
    }
}

#if !defined(__TOS__) && !defined(__atarist__)
// OPL2 sound effect support: use OPL2 channels 4-5 for effects (0-3 reserved for music)
u8 opl_sfx_initialized[2] = {0, 0};
u8 opl_sfx_type[2] = {0, 0};

static void io_canal_opl_init(u8 opl_ch, u8 chan_type)
{
    static const u8 mod_off[] = {0x09, 0x0A};
    static const u8 car_off[] = {0x0C, 0x0D};
    u8 idx = opl_ch - 4;
    u8 mod = mod_off[idx];
    u8 car = car_off[idx];

    if (chan_type == eChannelTypeNoise || chan_type == eChannelTypeExplode)
    {
        // Noise/explosion: FM mode with high feedback + high multipliers
        // Creates inharmonic, noise-like timbre
        OPL_writeReg(audio_opl, 0x20 + mod, 0x20 | 0x07); // Sustain=1, Mul=7
        OPL_writeReg(audio_opl, 0x40 + mod, 0x00);
        OPL_writeReg(audio_opl, 0x60 + mod, 0xF0);         // AR=max, DR=0
        OPL_writeReg(audio_opl, 0x80 + mod, 0xF0);         // SL=max, RR=0
        OPL_writeReg(audio_opl, 0xE0 + mod, 0x00);         // Sine

        OPL_writeReg(audio_opl, 0x20 + car, 0x20 | 0x0E);  // Sustain=1, Mul=14
        OPL_writeReg(audio_opl, 0x40 + car, 0x00);
        OPL_writeReg(audio_opl, 0x60 + car, 0xF0);
        OPL_writeReg(audio_opl, 0x80 + car, 0xF0);
        OPL_writeReg(audio_opl, 0xE0 + car, 0x00);

        // FM mode (connection=0) with max feedback (7) → noisy output
        OPL_writeReg(audio_opl, 0xC0 + opl_ch, 0x0E);      // FB=7, CNT=0
    }
    else
    {
        // Tone (czap, cding): additive sine, from Ishar 2 DOS
        OPL_writeReg(audio_opl, 0x20 + mod, 0x21);  // Sustain=1, Mul=1
        OPL_writeReg(audio_opl, 0x40 + mod, 0x00);
        OPL_writeReg(audio_opl, 0x60 + mod, 0xF0);
        OPL_writeReg(audio_opl, 0x80 + mod, 0xF0);
        OPL_writeReg(audio_opl, 0xE0 + mod, 0x00);

        OPL_writeReg(audio_opl, 0x20 + car, 0x21);
        OPL_writeReg(audio_opl, 0x40 + car, 0x00);
        OPL_writeReg(audio_opl, 0x60 + car, 0xF0);
        OPL_writeReg(audio_opl, 0x80 + car, 0xF0);
        OPL_writeReg(audio_opl, 0xE0 + car, 0x00);

        OPL_writeReg(audio_opl, 0xC0 + opl_ch, 0x01); // FB=0, CNT=1 (additive)
    }

    opl_sfx_initialized[idx] = 1;
    opl_sfx_type[idx] = chan_type;
}

static void io_canal_opl(sChannel *channel, s16 index)
{
    if (!audio_opl)
        sys_init_opl();

    u8 opl_ch = 4 + (index % 2);
    u8 idx = opl_ch - 4;

    // Re-init if channel type changed
    if (!opl_sfx_initialized[idx] || opl_sfx_type[idx] != channel->type)
        io_canal_opl_init(opl_ch, channel->type);

    // Volume: channel volume 0-0x7FFF → OPL2 TL 0x3F(silent)-0x00(max)
    u8 vol_4bit = (channel->volume >> 11) & 0xf;
    u8 tl = 0x3f - ((vol_4bit * 0x3f) / 0xf);

    static const u8 mod_off_v[] = {0x09, 0x0A};
    static const u8 car_off_v[] = {0x0C, 0x0D};

    if (opl_sfx_type[idx] == eChannelTypeNoise || opl_sfx_type[idx] == eChannelTypeExplode)
    {
        // FM mode: only carrier TL controls output volume
        OPL_writeReg(audio_opl, 0x40 + car_off_v[idx], tl);
    }
    else
    {
        // Additive mode: both operators output, update both
        OPL_writeReg(audio_opl, 0x40 + mod_off_v[idx], tl);
        OPL_writeReg(audio_opl, 0x40 + car_off_v[idx], tl);
    }

    if (channel->type == eChannelTypeNone || channel->volume == 0)
    {
        OPL_writeReg(audio_opl, 0xB0 + opl_ch, 0x00);
        opl_sfx_initialized[idx] = 0;
        return;
    }

    // Frequency: from Ishar 2 DOS FUN_1000_0af5
    s32 freq_idx = (channel->freq >> 3) + 0x140;
    if (freq_idx < 1) freq_idx = 1;
    u16 fnum = 0xD6D8 / freq_idx;
    if (fnum > 0x3FF) fnum = 0x3FF;

    OPL_writeReg(audio_opl, 0xA0 + opl_ch, fnum & 0xFF);
    OPL_writeReg(audio_opl, 0xB0 + opl_ch, 0x20 | 0x1C | ((fnum >> 8) & 0x03));
}
#endif
u8 giaccess(s8 cmd, u8 data, u8 ch)
{
    u8 regval = cmd & 0xf;
    if ((regval < 0xe) && (cmd < 0))
    {
        sys_write_psg(regval, data);
    }

    return sys_read_psg(regval);
}

#if ALIS_SND_INTERPOLATE_TYPE == 0
// (s8)sample * volratio, low 16 bits (all the mixers keep): per channel, rebuilt on volume change.
static s16 vol_tab[4][256];
static s32 vol_tab_v[4] = { -1, -1, -1, -1 };

static const s16 *vol_table(int ch, s32 volratio)
{
    if (vol_tab_v[ch] != volratio) {
        for (int k = 0; k < 256; k++)
            vol_tab[ch][k] = (s16)((s8)k * volratio);
        vol_tab_v[ch] = volratio;
    }
    return vol_tab[ch];
}

// The plain part of the sample loop: mix until the buffer is full or until the next source step
// reaches `end` (loop / chunk splice / end of sound), which the caller's full loop body handles.
// Returns the buffer index it stopped at; state is as at the top of that iteration.
#define MIX_RUN(name, T, MIX)                                                                      \
static __attribute__((noinline))                                                                   \
int name(T *buf, int bi, int len, u8 adv, const s16 *vt, const s8 *addr, u32 end, u32 ratio,        \
                u32 *accp, int *smpidxp, u32 *playedp, s32 *s0p)                                    \
{                                                                                                  \
    u32 acc = *accp, played = *playedp;                                                            \
    int smpidx = *smpidxp;                                                                         \
    s32 s0 = *s0p;                                                                                 \
    for (; bi < len; bi += adv, played++, acc = (acc & 0xFFFF) + ratio) {                          \
        u32 add = acc >> 16;                                                                       \
        if (add) {                                                                                 \
            if ((u32)(smpidx + add) >= end)                                                         \
                break;                                                                             \
            smpidx += add;                                                                         \
            s0 = vt[(u8)addr[smpidx]];                                                             \
        }                                                                                          \
        T v = MIX;                                                                                 \
        buf[bi] = v;                                                                               \
        if (adv >= 2) buf[bi + 1] = v;                                                             \
    }                                                                                              \
    *accp = acc; *playedp = played; *smpidxp = smpidx; *s0p = s0;                                  \
    return bi;                                                                                     \
}
MIX_RUN(mix_run_s16, s16, (s16)(u16)(s0 + buf[bi]))
typedef int MixRunS8(s8 *buf, int bi, int len, u8 adv, const s16 *vt, const s8 *addr, u32 end, u32 ratio,
                     u32 *accp, int *smpidxp, u32 *playedp, s32 *s0p);
#if defined(__m68k__) && !defined(__mcoldfire__)
MixRunS8 mix_run_s8, mix_run_s8_store;   // sys_atari_mix.S; _store: the buffer is still zero
#if defined(ALIS_MIX_VERIFY)
// Run the C version on copies next to the asm and log any difference.
MIX_RUN(mix_run_s8_c, s8, (s8)((s16)(s0 + ((s16)buf[bi] << 8)) >> 8))
static int mix_check(MixRunS8 *fn, s8 *buf, int bi, int len, u8 adv, const s16 *vt, const s8 *addr, u32 end,
                     u32 ratio, u32 *accp, int *smpidxp, u32 *playedp, s32 *s0p)
{
    extern void dbglog(const char *fmt, ...);
    static s8 copy[8192];
    static u32 calls, bad;
    u32 acc = *accp, played = *playedp; int smp = *smpidxp; s32 s0 = *s0p;
    if (len > (int)sizeof(copy)) return fn(buf, bi, len, adv, vt, addr, end, ratio, accp, smpidxp, playedp, s0p);
    memcpy(copy, buf, len);
    int ri = mix_run_s8_c(copy, bi, len, adv, vt, addr, end, ratio, &acc, &smp, &played, &s0);
    int ra = fn(buf, bi, len, adv, vt, addr, end, ratio, accp, smpidxp, playedp, s0p);
    calls++;
    if (ri != ra || acc != *accp || smp != *smpidxp || played != *playedp || (s16)s0 != (s16)*s0p || memcmp(copy, buf, len)) {
        if (bad++ < 8) dbglog("[mix] #%lu BAD %s bi %d/%d acc %lx/%lx smp %d/%d\n", (unsigned long)calls,
                              fn == mix_run_s8_store ? "store" : "add", ri, ra,
                              (unsigned long)acc, (unsigned long)*accp, smp, *smpidxp);
    } else if ((calls & 1023) == 0)
        dbglog("[mix] %lu calls, %lu bad\n", (unsigned long)calls, (unsigned long)bad);
    return ra;
}
static int mix_run_s8_v(s8 *b, int bi, int l, u8 adv, const s16 *vt, const s8 *a, u32 e, u32 r, u32 *ac, int *sm, u32 *pl, s32 *s0)
{ return mix_check(mix_run_s8, b, bi, l, adv, vt, a, e, r, ac, sm, pl, s0); }
static int mix_run_s8_store_v(s8 *b, int bi, int l, u8 adv, const s16 *vt, const s8 *a, u32 e, u32 r, u32 *ac, int *sm, u32 *pl, s32 *s0)
{ return mix_check(mix_run_s8_store, b, bi, l, adv, vt, a, e, r, ac, sm, pl, s0); }
#define mix_run_s8 mix_run_s8_v
#define mix_run_s8_store mix_run_s8_store_v
#endif
#else
MIX_RUN(mix_run_s8, s8, (s8)((s16)(s0 + ((s16)buf[bi] << 8)) >> 8))
#define mix_run_s8_store mix_run_s8
#endif
#undef MIX_RUN
#endif

// sys_audio_callback_S16MSB — 16-bit signed big-endian stereo/mono output.
// Used when SDL's negotiated native format is AUDIO_S16MSB (Falcon DMA,
// desktop SDL, most common path).
void sys_audio_callback_S16MSB(void *userdata, u8 *s, s32 buffer_length)
{
    memset(s, 0, buffer_length);

    u32 ratio = 0;
    s32 volratio = 0;

#if ALIS_SND_INTERPOLATE_TYPE > 0
    int a0, a1, a2, a3;
    int x0, x1, x2, x3;
    float t, r;
#endif

    u16 *audio_buffer = (u16 *)s;
    buffer_length >>= 1;

    int smpidx = 0;
    u8 adv = audio_spec->channels;

    // Write one mono sample to every channel of the frame (idx frame-aligned).
    #define WRITE_AUDIO_FRAME(idx, sample) do {                 \
        u16 __wfs = (u16)(sample);                               \
        audio_buffer[(idx)] = __wfs;                             \
        if (adv >= 2) audio_buffer[(idx) + 1] = __wfs;           \
    } while (0)

    // handle music

    if (audio.muflag > 0)
    {
        int smprem = buffer_length / adv;

        int buflen = audio.mutaloop;
        int len = (buffer_length + buflen - 1) / buflen;

        if (audio.smpidx)
        {
            int smpcopy = min(audio.smpidx, smprem);
            s16 *music_buffer = (audio.muadresse + (buflen - audio.smpidx));

            int music_index = 0;
            for (int audio_index = 0; music_index < smpcopy; audio_index+=adv, music_index++)
                WRITE_AUDIO_FRAME(audio_index, music_buffer[music_index]);

            smpidx += music_index;
            smprem -= music_index;

            audio.smpidx -= music_index;
        }

        for (int i = 0; i < len && smprem; i++)
        {
            audio.soundrout();

            int smpcopy = min(buflen, smprem);

            int music_index = 0;
            for (int audio_index = smpidx * adv; music_index < smpcopy; audio_index+=adv, music_index++)
                WRITE_AUDIO_FRAME(audio_index, audio.muadresse[music_index]);

            smpidx += music_index;
            smprem -= music_index;

            audio.smpidx = buflen - music_index;
        }
    }
    
    if (audio.muflag == 0)
        audio.working = 0;

    // handle sounds

    if (audio.fsound)
    {
        u32 prev_counter = isr_counter;
        u32 add = 0;
        s32 s0, s1;
        u32 accumulator;   // 16.16, masked to 16 bits per sample: fits in 32 bits (ratio < 2^31)
        
        for (int i = 0; i < 4; i++)
        {
            isr_counter = prev_counter;
            
            sChannel *ch = &audio.channels[i];
            switch (ch->type)
            {
                case eChannelTypeSample:
                {
                    add = 0;
                    volratio = ch->volume >> 1;
#if ALIS_SND_INTERPOLATE_TYPE == 0
                    const s16 *vt = vol_table(i, volratio);
#endif

                    // addr/length change when the next FLI chunk is spliced in.
                    s8 *addr   = ch->address;
                    u32 length = ch->length;
                    u8  loop   = ch->loop;
                    u32 played = ch->played;

                    ratio = (ch->freq << 16) / audio_spec->freq;
                    u64 start = (u64)played * (u64)ratio;
                    smpidx = start >> 16;
                    accumulator = (u32)start & 0xFFFF;

#if ALIS_SND_INTERPOLATE_TYPE > 0
                    x0 = (smpidx - 1 >= 0) ? (s16)(addr[smpidx - 1] * 256) : loop > 1 ? (s16)(addr[length - 1] * 256) : 0;
                    x1 = (s16)(addr[smpidx] * 256);
                    x2 = (smpidx + 1 < length) ? (s16)(addr[smpidx + 1] * 256) : loop > 1 ? (s16)(addr[0] * 256) : 0;
                    x3 = (smpidx + 2 < length) ? (s16)(addr[smpidx + 2] * 256) : loop > 1 ? (s16)(addr[0] * 256) : 0;
# if ALIS_SND_INTERPOLATE_TYPE == 1
                    a0 = x3 - x2 - x0 + x1;
                    a1 = x0 - x1 - a0;
                    a2 = x2 - x0;
# endif
#else
                    s0 = vt[(u8)addr[smpidx]];
#endif

                    int buffer_index = 0;
                    while (buffer_index < buffer_length)
                    {
#if ALIS_SND_INTERPOLATE_TYPE == 0
                        buffer_index = mix_run_s16(audio_buffer, buffer_index, buffer_length, adv, vt, addr, length + 1,
                                              ratio, &accumulator, &smpidx, &played, &s0);
                        if (buffer_index >= buffer_length)
                            break;
#endif
                        if ((add = accumulator >> 16) > 0)
                        {
                            smpidx += add;
#if ALIS_SND_INTERPOLATE_TYPE > 0
                            // Shift window by 'add' samples so it tracks smpidx when downsampling.
                            for (u32 step = 1; step <= add; step++) {
                                x0 = x1; x1 = x2; x2 = x3;
                                s32 idx = (s32)smpidx - (s32)add + (s32)step + 2;
                                if (idx < (s32)length) {
                                    x3 = (s16)(addr[idx] * 256);
                                } else if (loop > 1) {
                                    s32 wrapped = idx - (s32)length;
                                    x3 = (wrapped >= 0 && wrapped < (s32)length)
                                         ? (s16)(addr[wrapped] * 256)
                                         : 0;
                                } else {
                                    x3 = 0;
                                }
                            }
#else
                            s0 = vt[(u8)addr[smpidx]];
#endif

                            if (smpidx >= length + 1)
                            {
                                if (loop > 1)
                                {
                                    loop--;
                                    played = smpidx = accumulator = 0;
#if ALIS_SND_INTERPOLATE_TYPE > 0
                                    // Re-prime the window at the loop start.
                                    x0 = (s16)(addr[length - 1] * 256);
                                    x1 = (s16)(addr[0] * 256);
                                    x2 = (length > 1) ? (s16)(addr[1] * 256) : 0;
                                    x3 = (length > 2) ? (s16)(addr[2] * 256) : 0;
#endif
                                }
                                else if (i == 3 && fli_audio_q_head != fli_audio_q_tail)
                                {
                                    // Splice in the next queued FLI chunk.
                                    if (i == 3) fli_chunks_played++;
                                    u8 t = fli_audio_q_tail;
                                    addr   = fli_audio_queue[t].addr;
                                    length = fli_audio_queue[t].length;
                                    ch->address = addr;
                                    ch->length  = length;
                                    ch->freq    = fli_audio_queue[t].freq;
                                    fli_audio_q_tail = (u8)((t + 1) & (FLI_AUDIO_QUEUE_SIZE - 1));
                                    ratio = (ch->freq << 16) / audio_spec->freq;
                                    played = smpidx = accumulator = 0;
#if ALIS_SND_INTERPOLATE_TYPE > 0
                                    // Carry the old tail in x0 so the splice doesn't click.
                                    x0 = x3;
                                    x1 = (length > 0) ? (s16)(addr[0] * 256) : 0;
                                    x2 = (length > 1) ? (s16)(addr[1] * 256) : 0;
                                    x3 = (length > 2) ? (s16)(addr[2] * 256) : 0;
#endif
                                    FLI_DBG_MIX("S16MSB splice: chunk=%u freq=%d len=%u ratio=0x%x q_used=%u\n",
                                                fli_chunks_played, (int)ch->freq, length, ratio,
                                                (unsigned)((fli_audio_q_head - fli_audio_q_tail) & (FLI_AUDIO_QUEUE_SIZE - 1)));
                                }
                                else
                                {
                                    if (i == 3) {
                                        fli_chunks_played++;
                                        FLI_DBG_MIX("S16MSB end: chunk=%u (queue empty, channel terminating)\n",
                                                    fli_chunks_played);
                                    }
                                    ch->type = eChannelTypeNone;
                                    ch->curson = 0x80;
                                    break;
                                }
                            }

# if ALIS_SND_INTERPOLATE_TYPE == 1
                            a0 = x3 - x2 - x0 + x1;
                            a1 = x0 - x1 - a0;
                            a2 = x2 - x0;
# endif
                        }

#if ALIS_SND_INTERPOLATE_TYPE > 0
                        t = (accumulator / 65536.0) - add;
# if ALIS_SND_INTERPOLATE_TYPE == 1
                        r = ((a0 * (t * t * t)) + (a1 * (t * t)) + (a2 * t) + x1) / 256.0;
# elif ALIS_SND_INTERPOLATE_TYPE == 2
                        r = interpolate_hermite(x0, x1, x2, x3, t);
# else
                        r = interpolate_hermite_4pt_3ox(x0, x1, x2, x3, t);
# endif
                        s0 = r * volratio;
#endif
                        WRITE_AUDIO_FRAME(buffer_index, s0 + audio_buffer[buffer_index]);
                        accumulator &= 0xFFFF;
                        buffer_index += adv; played++; accumulator += ratio;
                    }

                    // Main thread may have replaced the chunk directly; then `played` is stale.
                    if (ch->address == addr) {
                        ch->played = played;
                        ch->loop   = loop;
                    }
                    break;
                }

                case eChannelTypeDingZap:
                case eChannelTypeNoise:
                case eChannelTypeExplode:
                {
                    if (i < 3)
                    {
                        // Count frames, not samples: tick rate must not depend on channel count.
                        for (int p = 0; p < buffer_length; p += adv)
                        {
                            isr_counter += isr_step;
                            if (isr_counter >= 0x10000)
                            {
                                isr_counter -= 0x10000;

                                do
                                {
                                    if (ch->played < ch->length)
                                    {
                                        ch->played ++;

                                        s32 vol = (s32)ch->delta_volume + (s32)ch->volume;
                                        if (-1 < vol)
                                        {
                                            if (0x7fff < vol)
                                            {
                                                vol = 0x7fff;
                                            }

                                            ch->volume = (s16)vol;
                                            u32 freq = ch->delta_freq + ch->freq;
                                            if (-1 < (s32)(freq << 0x10))
                                            {
                                                ch->freq = freq;
#if !defined(__TOS__) && !defined(__atarist__)
                                                if (alis.platform.kind == EPlatformPC)
                                                    io_canal_opl(ch, i);
                                                else
#endif
                                                    io_canal(ch, i);
                                                break;
                                            }
                                        }
                                    }

                                    ch->type = eChannelTypeNone;
                                    ch->volume = 0;
                                    ch->freq = 0;
                                    ch->curson = 0x80;
                                    ch->state = 0;
                                    ch->played = 0;
#if !defined(__TOS__) && !defined(__atarist__)
                                    if (alis.platform.kind == EPlatformPC)
                                        io_canal_opl(ch, i);
                                    else
#endif
                                        io_canal(ch, i);
                                }
                                while (false);
                            }

#if !defined(__TOS__) && !defined(__atarist__)
                            if (alis.platform.kind == EPlatformPC && audio_opl)
                            {
                                s0 = OPL_calc(audio_opl);
                                s1 = audio_buffer[p] - 0x8000;
                                audio_buffer[p] = (s0 + s1) + 0x8000;
                            }
                            else
                            {
                                s0 = PSG_calc(audio_psg);
                                s1 = audio_buffer[p] - 0x8000;
                                audio_buffer[p] = (s0 + s1) + 0x8000;
                            }
#endif
                        }
                    }
                    break;
                }
                    
                default:
                    break;
            }
        }
    }
    
    #undef WRITE_AUDIO_FRAME

#if !defined(__TOS__) && !defined(__atarist__)
    // The OPL/PSG paths above write only the first channel; duplicate it.
    for (int o = 1; o < audio_spec->channels; o++)
    {
        for (int buffer_index = 0; buffer_index < buffer_length; buffer_index+=adv)
            audio_buffer[buffer_index + o] = audio_buffer[buffer_index];
    }
#endif
}

// sys_audio_callback_S8 — 8-bit signed output (STE/TT DMA). Narrows the s16 mix
// inline to avoid SDL's S16→S8 converter. Twin of S16MSB: keep the two in sync.

void sys_audio_callback_S8(void *userdata, u8 *s, s32 buffer_length)
{
    memset(s, 0, buffer_length);
    int fresh = 1;   // buffer still all zero: the first sample channel can store instead of mix

    u32 ratio = 0;
    s32 volratio = 0;

#if ALIS_SND_INTERPOLATE_TYPE > 0
    int a0, a1, a2, a3;
    int x0, x1, x2, x3;
    float t, r;
#endif

    // S8 output: one byte per sample, buffer_length is already the sample
    // count (no `>>= 1` like the S16MSB variant).
    s8 *audio_buffer = (s8 *)s;

    int smpidx = 0;
    u8 adv = audio_spec->channels;

    // Narrow an s16 sample to s8 and write it to every channel of the frame.
    #define WRITE_AUDIO_FRAME(idx, sample_s16) do {              \
        s8 __wfs = (s8)((s16)(sample_s16) >> 8);                  \
        audio_buffer[(idx)] = __wfs;                               \
        if (adv >= 2) audio_buffer[(idx) + 1] = __wfs;             \
    } while (0)

    // handle music — mixer output is s16 in audio.muadresse

    if (audio.muflag > 0)
    {
        int smprem = buffer_length / adv;

        int buflen = audio.mutaloop;
        int len = (buffer_length + buflen - 1) / buflen;

        // Chip music plays on the real YM and never fills muadresse: skip the
        // silent copy but keep calling soundrout() for sequencer timing.
        extern void mv2_chiprout(void);
        int chip_only = (audio.soundrout == mv2_chiprout);

        if (audio.smpidx)
        {
            int smpcopy = min(audio.smpidx, smprem);

            if (!chip_only)
            {
                fresh = 0;
                s16 *music_buffer = (audio.muadresse + (buflen - audio.smpidx));
                int music_index = 0;
                for (int audio_index = 0; music_index < smpcopy; audio_index+=adv, music_index++)
                    WRITE_AUDIO_FRAME(audio_index, music_buffer[music_index]);
            }

            smpidx += smpcopy;
            smprem -= smpcopy;

            audio.smpidx -= smpcopy;
        }

        for (int i = 0; i < len && smprem; i++)
        {
            audio.soundrout();

            int smpcopy = min(buflen, smprem);

            if (!chip_only)
            {
                fresh = 0;
                int music_index = 0;
                for (int audio_index = smpidx * adv; music_index < smpcopy; audio_index+=adv, music_index++)
                    WRITE_AUDIO_FRAME(audio_index, audio.muadresse[music_index]);
            }

            smpidx += smpcopy;
            smprem -= smpcopy;

            audio.smpidx = buflen - smpcopy;
        }
    }

    if (audio.muflag == 0)
        audio.working = 0;

    // handle sounds

    if (audio.fsound)
    {
        u32 prev_counter = isr_counter;
        u32 add = 0;
        s32 s0, s1;
        u32 accumulator;   // 16.16, masked to 16 bits per sample: fits in 32 bits (ratio < 2^31)

        for (int i = 0; i < 4; i++)
        {
            isr_counter = prev_counter;

            sChannel *ch = &audio.channels[i];
            switch (ch->type)
            {
                case eChannelTypeSample:
                {
                    add = 0;
                    volratio = ch->volume >> 1;
#if ALIS_SND_INTERPOLATE_TYPE == 0
                    const s16 *vt = vol_table(i, volratio);
#endif

                    // addr/length change when the next FLI chunk is spliced in.
                    s8 *addr   = ch->address;
                    u32 length = ch->length;
                    u8  loop   = ch->loop;
                    u32 played = ch->played;

                    ratio = (ch->freq << 16) / audio_spec->freq;
                    u64 start = (u64)played * (u64)ratio;
                    smpidx = start >> 16;
                    accumulator = (u32)start & 0xFFFF;

#if ALIS_SND_INTERPOLATE_TYPE > 0
                    x0 = (smpidx - 1 >= 0) ? (s16)(addr[smpidx - 1] * 256) : loop > 1 ? (s16)(addr[length - 1] * 256) : 0;
                    x1 = (s16)(addr[smpidx] * 256);
                    x2 = (smpidx + 1 < length) ? (s16)(addr[smpidx + 1] * 256) : loop > 1 ? (s16)(addr[0] * 256) : 0;
                    x3 = (smpidx + 2 < length) ? (s16)(addr[smpidx + 2] * 256) : loop > 1 ? (s16)(addr[0] * 256) : 0;
# if ALIS_SND_INTERPOLATE_TYPE == 1
                    a0 = x3 - x2 - x0 + x1;
                    a1 = x0 - x1 - a0;
                    a2 = x2 - x0;
# endif
#else
                    s0 = vt[(u8)addr[smpidx]];
#endif

                    int buffer_index = 0;
                    while (buffer_index < buffer_length)
                    {
#if ALIS_SND_INTERPOLATE_TYPE == 0
                        buffer_index = (fresh ? mix_run_s8_store : mix_run_s8)(audio_buffer, buffer_index, buffer_length, adv, vt, addr, length + 1,
                                              ratio, &accumulator, &smpidx, &played, &s0);
                        if (buffer_index >= buffer_length)
                            break;
#endif
                        if ((add = accumulator >> 16) > 0)
                        {
                            smpidx += add;
#if ALIS_SND_INTERPOLATE_TYPE > 0
                            // Shift window by 'add' samples so it tracks smpidx when downsampling.
                            for (u32 step = 1; step <= add; step++) {
                                x0 = x1; x1 = x2; x2 = x3;
                                s32 idx = (s32)smpidx - (s32)add + (s32)step + 2;
                                if (idx < (s32)length) {
                                    x3 = (s16)(addr[idx] * 256);
                                } else if (loop > 1) {
                                    s32 wrapped = idx - (s32)length;
                                    x3 = (wrapped >= 0 && wrapped < (s32)length)
                                         ? (s16)(addr[wrapped] * 256)
                                         : 0;
                                } else {
                                    x3 = 0;
                                }
                            }
#else
                            s0 = vt[(u8)addr[smpidx]];
#endif

                            if (smpidx >= length + 1)
                            {
                                if (loop > 1)
                                {
                                    loop--;
                                    played = smpidx = accumulator = 0;
#if ALIS_SND_INTERPOLATE_TYPE > 0
                                    x0 = (s16)(addr[length - 1] * 256);
                                    x1 = (s16)(addr[0] * 256);
                                    x2 = (length > 1) ? (s16)(addr[1] * 256) : 0;
                                    x3 = (length > 2) ? (s16)(addr[2] * 256) : 0;
#endif
                                }
                                else if (i == 3 && fli_audio_q_head != fli_audio_q_tail)
                                {
                                    // Splice in the next queued FLI chunk.
                                    if (i == 3) fli_chunks_played++;
                                    u8 t = fli_audio_q_tail;
                                    addr   = fli_audio_queue[t].addr;
                                    length = fli_audio_queue[t].length;
                                    ch->address = addr;
                                    ch->length  = length;
                                    ch->freq    = fli_audio_queue[t].freq;
                                    fli_audio_q_tail = (u8)((t + 1) & (FLI_AUDIO_QUEUE_SIZE - 1));
                                    ratio = (ch->freq << 16) / audio_spec->freq;
                                    played = smpidx = accumulator = 0;
                                    FLI_DBG_MIX("S8 splice: chunk=%u freq=%d len=%u ratio=0x%x q_used=%u\n",
                                                fli_chunks_played, (int)ch->freq, length, ratio,
                                                (unsigned)((fli_audio_q_head - fli_audio_q_tail) & (FLI_AUDIO_QUEUE_SIZE - 1)));
#if ALIS_SND_INTERPOLATE_TYPE > 0
                                    x0 = x3;   // tail of previous chunk (avoids click)
                                    x1 = (length > 0) ? (s16)(addr[0] * 256) : 0;
                                    x2 = (length > 1) ? (s16)(addr[1] * 256) : 0;
                                    x3 = (length > 2) ? (s16)(addr[2] * 256) : 0;
#endif
                                }
                                else
                                {
                                    if (i == 3) {
                                        fli_chunks_played++;
                                        FLI_DBG_MIX("S8 end: chunk=%u (queue empty, channel terminating)\n",
                                                    fli_chunks_played);
                                    }
                                    ch->type = eChannelTypeNone;
                                    ch->curson = 0x80;
                                    break;
                                }
                            }

# if ALIS_SND_INTERPOLATE_TYPE == 1
                            a0 = x3 - x2 - x0 + x1;
                            a1 = x0 - x1 - a0;
                            a2 = x2 - x0;
# endif
                        }

#if ALIS_SND_INTERPOLATE_TYPE > 0
                        t = (accumulator / 65536.0) - add;
# if ALIS_SND_INTERPOLATE_TYPE == 1
                        r = ((a0 * (t * t * t)) + (a1 * (t * t)) + (a2 * t) + x1) / 256.0;
# elif ALIS_SND_INTERPOLATE_TYPE == 2
                        r = interpolate_hermite(x0, x1, x2, x3, t);
# else
                        r = interpolate_hermite_4pt_3ox(x0, x1, x2, x3, t);
# endif
                        s0 = r * volratio;
#endif
                        // Widen the existing s8 sample so the mix matches S16MSB.
                        s16 existing_as_s16 = (s16)audio_buffer[buffer_index] << 8;
                        WRITE_AUDIO_FRAME(buffer_index, s0 + existing_as_s16);
                        accumulator &= 0xFFFF;
                        buffer_index += adv; played++; accumulator += ratio;
                    }

                    // Main thread may have replaced the chunk directly; then `played` is stale.
                    if (ch->address == addr) {
                        ch->played = played;
                        ch->loop   = loop;
                    }
                    fresh = 0;
                    break;
                }

                case eChannelTypeDingZap:
                case eChannelTypeNoise:
                case eChannelTypeExplode:
                {
                    if (i < 3)
                    {
                        // Count frames, not samples: tick rate must not depend on channel count.
                        for (int p = 0; p < buffer_length; p += adv)
                        {
                            isr_counter += isr_step;
                            if (isr_counter >= 0x10000)
                            {
                                isr_counter -= 0x10000;

                                do
                                {
                                    if (ch->played < ch->length)
                                    {
                                        ch->played ++;

                                        s32 vol = (s32)ch->delta_volume + (s32)ch->volume;
                                        if (-1 < vol)
                                        {
                                            if (0x7fff < vol)
                                            {
                                                vol = 0x7fff;
                                            }

                                            ch->volume = (s16)vol;
                                            u32 freq = ch->delta_freq + ch->freq;
                                            if (-1 < (s32)(freq << 0x10))
                                            {
                                                ch->freq = freq;
#if !defined(__TOS__) && !defined(__atarist__)
                                                if (alis.platform.kind == EPlatformPC)
                                                    io_canal_opl(ch, i);
                                                else
#endif
                                                    io_canal(ch, i);
                                                break;
                                            }
                                        }
                                    }

                                    ch->type = eChannelTypeNone;
                                    ch->volume = 0;
                                    ch->freq = 0;
                                    ch->curson = 0x80;
                                    ch->state = 0;
                                    ch->played = 0;
#if !defined(__TOS__) && !defined(__atarist__)
                                    if (alis.platform.kind == EPlatformPC)
                                        io_canal_opl(ch, i);
                                    else
#endif
                                        io_canal(ch, i);
                                }
                                while (false);
                            }

#if !defined(__TOS__) && !defined(__atarist__)
                            // Unreachable in practice (S8 is Atari-only); kept compiling.
                            if (alis.platform.kind == EPlatformPC && audio_opl)
                            {
                                s0 = OPL_calc(audio_opl);
                                s1 = ((s16)audio_buffer[p] << 8) - 0x8000;
                                s16 mixed = (s16)((s0 + s1) + 0x8000);
                                audio_buffer[p] = (s8)(mixed >> 8);
                            }
                            else
                            {
                                s0 = PSG_calc(audio_psg);
                                s1 = ((s16)audio_buffer[p] << 8) - 0x8000;
                                s16 mixed = (s16)((s0 + s1) + 0x8000);
                                audio_buffer[p] = (s8)(mixed >> 8);
                            }
#endif
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    #undef WRITE_AUDIO_FRAME

#if !defined(__TOS__) && !defined(__atarist__)
    // The OPL/PSG paths above write only the first channel; duplicate it.
    for (int o = 1; o < audio_spec->channels; o++)
    {
        for (int buffer_index = 0; buffer_index < buffer_length; buffer_index+=adv)
            audio_buffer[buffer_index + o] = audio_buffer[buffer_index];
    }
#endif
}

// =============================================================================
#pragma mark - I/O
// =============================================================================

#if defined(__atarist__) || defined(__TOS__)
#include <mint/osbind.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <sys/statvfs.h>
#endif

u32 sys_free_bytes(const char *path, u32 cap)
{
    unsigned long long n = 0;
#if defined(__atarist__) || defined(__TOS__)
    // Dfree drive: 0 = current, 1 = A:, ... Use the path's drive letter if it has one.
    _DISKINFO di;
    int drive = (path && path[0] && path[1] == ':') ? ((path[0] & 0x1f)) : 0;
    if (Dfree(&di, drive) == 0)
        n = (unsigned long long)di.b_free * di.b_secsiz * di.b_clsiz;
#elif defined(_WIN32)
    ULARGE_INTEGER avail;
    if (GetDiskFreeSpaceExA(path, &avail, NULL, NULL))
        n = avail.QuadPart;
#else
    struct statvfs st;
    if (statvfs(path, &st) == 0)
        n = (unsigned long long)st.f_bavail * st.f_frsize;
#endif
    return n < cap ? (u32)n : cap;
}

mouse_t sys_get_mouse(void) {
    return mouse;
}

mouse_t sys_consume_mouse(void) {
    mouse_t snap = mouse;
    mouse.lb_clicked = 0;
    mouse.rb_clicked = 0;
    return snap;
}

void sys_set_mouse(u16 x, u16 y) {
    mouse.x = x;
    mouse.y = y;
}

bool rect_equals(const SDL_Rect *x, const SDL_Rect *y)
{
    return x && y && x->x == y->x && x->y == y->y && x->w == y->w && x->h == y->h;
}

void sys_enable_mouse(u8 enable) {
    
#if ALIS_SDL_VER == 1
    s16 width = mouse.x + 16 > host.pixelbuf.w ? host.pixelbuf.w - mouse.x : 16;
    s16 height = mouse.y + 16 > host.pixelbuf.h ? host.pixelbuf.h - mouse.y : 16;

    SDL_Rect new_dirty_rect = (SDL_Rect){ .x = mouse.x, .y = mouse.y, .w = width, .h = height };
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
#endif

    mouse.enabled = enable;

    if (!mouse.enabled)
        alis.desmouse = NULL;
    
    dirty_mouse = true;
}

void set_update_cursor(void) {
    
    dirty_mouse = true;
}

#if defined(ALIS_USE_NATIVE_ATARI)
extern volatile u8 g_ikbd_pressed[128];   // held-key state (sys_atari_ikbd.S)
extern volatile u8 g_ikbd_cur;            // last key pressed

// Atari ST (US) keyboard scancode -> ASCII for io_inkey's printable default case
// (0 = none). Special keys (ESC/arrows/F/modifiers) are handled in the switch.
static const u8 atari_sc_ascii[128] = {
    [0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',[0x07]='6',[0x08]='7',[0x09]='8',[0x0a]='9',[0x0b]='0',
    [0x0c]='-',[0x0d]='=',[0x0e]=0x08,[0x0f]=0x09,
    [0x10]='q',[0x11]='w',[0x12]='e',[0x13]='r',[0x14]='t',[0x15]='y',[0x16]='u',[0x17]='i',[0x18]='o',[0x19]='p',
    [0x1a]='[',[0x1b]=']',[0x1c]=0x0d,
    [0x1e]='a',[0x1f]='s',[0x20]='d',[0x21]='f',[0x22]='g',[0x23]='h',[0x24]='j',[0x25]='k',[0x26]='l',
    [0x27]=';',[0x28]='\'',[0x29]='`',[0x2b]='\\',
    [0x2c]='z',[0x2d]='x',[0x2e]='c',[0x2f]='v',[0x30]='b',[0x31]='n',[0x32]='m',
    [0x33]=',',[0x34]='.',[0x35]='/',[0x39]=' ',[0x53]=0x7f,
    [0x4a]='-',[0x4e]='+',[0x65]='/',[0x66]='*',
    [0x67]='7',[0x68]='8',[0x69]='9',[0x6a]='4',[0x6b]='5',[0x6c]='6',
    [0x6d]='1',[0x6e]='2',[0x6f]='3',[0x70]='0',[0x71]='.',[0x72]=0x0d,
};
static const u8 atari_sc_ascii_shift[128] = {
    [0x02]='!',[0x03]='@',[0x04]='#',[0x05]='$',[0x06]='%',[0x07]='^',[0x08]='&',[0x09]='*',[0x0a]='(',[0x0b]=')',
    [0x0c]='_',[0x0d]='+',[0x0e]=0x08,[0x0f]=0x09,
    [0x10]='Q',[0x11]='W',[0x12]='E',[0x13]='R',[0x14]='T',[0x15]='Y',[0x16]='U',[0x17]='I',[0x18]='O',[0x19]='P',
    [0x1a]='{',[0x1b]='}',[0x1c]=0x0d,
    [0x1e]='A',[0x1f]='S',[0x20]='D',[0x21]='F',[0x22]='G',[0x23]='H',[0x24]='J',[0x25]='K',[0x26]='L',
    [0x27]=':',[0x28]='"',[0x29]='~',[0x2b]='|',
    [0x2c]='Z',[0x2d]='X',[0x2e]='C',[0x2f]='V',[0x30]='B',[0x31]='N',[0x32]='M',
    [0x33]='<',[0x34]='>',[0x35]='?',[0x39]=' ',
    // Numpad is shift-independent (shift is read separately via io_shiftkey).
    [0x4a]='-',[0x4e]='+',[0x65]='/',[0x66]='*',
    [0x67]='7',[0x68]='8',[0x69]='9',[0x6a]='4',[0x6b]='5',[0x6c]='6',
    [0x6d]='1',[0x6e]='2',[0x6f]='3',[0x70]='0',[0x71]='.',[0x72]=0x0d,
};
#endif

#if defined(ALIS_TRACE_KEYS)
#include "video.h"
#endif

u8 io_inkey(void)
{
#if defined(ALIS_USE_NATIVE_ATARI)
    // Scancodes from the IKBD handler (sys_atari_ikbd.S). Report the last key only
    // while it's held (like SDL key state) so repeated polls see a stable value.
    {
        // A press shorter than the poll interval (slow machines) is still reported once.
        extern volatile u8 g_ikbd_new;
        u8 sc = g_ikbd_cur;
        if (sc == 0 || sc >= 128 || (!g_ikbd_pressed[sc] && !g_ikbd_new)) {
            g_ikbd_cur = 0;
            return 0;
        }
#if defined(ALIS_TRACE_KEYS)
        {   // key the game sees: log changes and latched (already released) reports
            extern void dbglog(const char *fmt, ...);
            static u8 last_sc;
            u8 held = g_ikbd_pressed[sc] != 0;
            if (sc != last_sc || !held)
                dbglog("KEY sc=%02x held=%d new=%d t=%u script=%s film=%d/%d play=%d\n", sc, held,
                       g_ikbd_new != 0, (unsigned)alis.timeclock, alis.script ? alis.script->name : "?",
                       (int)bfilm.frame, (int)bfilm.frames, (int)bfilm.playing);
            last_sc = sc;
        }
#endif
        g_ikbd_new = 0;
        switch (sc) {
            case 0x01: return 0x1b;                          // ESC
            case 0x48: return 0xc8;                          // UP
            case 0x50: return 0xd0;                          // DOWN
            case 0x4b: return 0xcb;                          // LEFT
            case 0x4d: return 0xcd;                          // RIGHT
            case 0x3b: return 0xbb; case 0x3c: return 0xbc;  // F1 F2
            case 0x3d: return 0xbd; case 0x3e: return 0xbe;  // F3 F4
            case 0x3f: return 0xbf; case 0x40: return 0xc0;  // F5 F6
            case 0x41: return 0xc1; case 0x42: return 0xc2;  // F7 F8
            case 0x43: return 0xc3; case 0x44: return 0xc4;  // F9 F10
            case 0x2a: case 0x36: case 0x1d: case 0x38: case 0x3a:
                return 0;                                    // shift/ctrl/alt/caps
            default: {
                u8 shifted = g_ikbd_pressed[0x2a] || g_ikbd_pressed[0x36];
                u8 ch = (shifted ? atari_sc_ascii_shift : atari_sc_ascii)[sc];
                return ch ? ch : sc;
            }
        }
    }
#else
#if ALIS_SDL_VER == 1
    SDL_PumpEvents();

    // VM busy loops (e.g. pause-on-ESC) never reach sys_poll_event, so drain
    // KEYDOWN here or `button` never changes. Other events (KEYUP drives the
    // F11/F12 save states) stay queued for sys_poll_event.
    {
        SDL_Event ev;
        while (SDL_PeepEvents(&ev, 1, SDL_GETEVENT,
                              SDL_EVENTMASK(SDL_KEYDOWN)) > 0)
        {
            SDLKey k = ev.key.keysym.sym;
            if (k == SDLK_PAUSE)
            {
                ALIS_DEBUG(EDebugSystem, "INTERRUPT: Quit by user request.\n");
                alis.state = eAlisStateStopped;
            }
            else if (k != SDLK_F11 && k != SDLK_F12)
            {
                button = ev.key.keysym;
            }
        }
    }

    const u8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (!currentKeyStates[button.sym])
#else
    const u8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (!currentKeyStates[button.scancode])
#endif
    {
        button.scancode = 0;
        button.sym = 0;
    }
    
    ALIS_DEBUG(EDebugInfo, " [\"%s\"]", SDL_GetKeyName(button.sym));
    switch (button.sym) {
        case SDLK_ESCAPE:       return 0x1b;
            
        case SDLK_UP:           return 0xc8;
        case SDLK_DOWN:         return 0xd0;
        case SDLK_LEFT:         return 0xcb;
        case SDLK_RIGHT:        return 0xcd;
        
        case SDLK_KP_0:         return '0';
        case SDLK_KP_1:         return '1';
        case SDLK_KP_2:         return '2';
        case SDLK_KP_3:         return '3';
        case SDLK_KP_4:         return '4';
        case SDLK_KP_5:         return '5';
        case SDLK_KP_6:         return '6';
        case SDLK_KP_7:         return '7';
        case SDLK_KP_8:         return '8';
        case SDLK_KP_9:         return '9';
        case SDLK_KP_PLUS:      return '+';
        case SDLK_KP_MINUS:     return '-';
            
        case SDLK_F1:           return 0xbb;
        case SDLK_F2:           return 0xbc;
        case SDLK_F3:           return 0xbd;
        case SDLK_F4:           return 0xbe;
        case SDLK_F5:           return 0xbf;
        case SDLK_F6:           return 0xc0;
        case SDLK_F7:           return 0xc1;
        case SDLK_F8:           return 0xc2;
        case SDLK_F9:           return 0xc3;
        case SDLK_F10:          return 0xc4;
            
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
        case SDLK_LCTRL:
        case SDLK_RCTRL:
        case SDLK_LALT:
        case SDLK_RALT:
        case SDLK_CAPSLOCK:
        case SDLK_MODE:         return 0;

        default:
        {
            const u8 *keys = SDL_GetKeyboardState(NULL);
#if ALIS_SDL_VER == 1
            u8 shifted = (SDL_GetModState() & KMOD_SHIFT) || keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT];
#else
            u8 shifted = (SDL_GetModState() & KMOD_SHIFT) || keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
#endif
            if (shifted)
            {
                switch (button.sym) {
                    case SDLK_1:            return '!';
                    case SDLK_2:            return '@';
                    case SDLK_3:            return '#';
                    case SDLK_4:            return '$';
                    case SDLK_5:            return '%';
                    case SDLK_6:            return '^';
                    case SDLK_7:            return '&';
                    case SDLK_8:            return '*';
                    case SDLK_9:            return '(';
                    case SDLK_0:            return ')';
                    case SDLK_MINUS:        return '_';
                    case SDLK_EQUALS:       return '+';
                    default: break;
                }
            }
            return button.sym;
        }
    }
#endif // ALIS_USE_NATIVE_ATARI
}

u8 io_shiftkey(void) {

#if defined(ALIS_USE_NATIVE_ATARI)
    // Read modifiers straight from the IKBD held-key state (sys_atari_ikbd.S).
    // Encoding: 1=RShift 2=LShift 4=Ctrl 8=Alt.
    shift = 0;
    if (g_ikbd_pressed[0x36]) shift |= 1;
    if (g_ikbd_pressed[0x2a]) shift |= 2;
    if (g_ikbd_pressed[0x1d]) shift |= 4;
    if (g_ikbd_pressed[0x38]) shift |= 8;
    return shift;
#else
    const u8 *keys = SDL_GetKeyboardState(NULL);
    u16 mod = SDL_GetModState();

#if ALIS_SDL_VER == 1
    if (keys[SDLK_RSHIFT])  mod |= KMOD_RSHIFT;
    if (keys[SDLK_LSHIFT])  mod |= KMOD_LSHIFT;
    if (keys[SDLK_RCTRL] || keys[SDLK_LCTRL])  mod |= KMOD_CTRL;
    if (keys[SDLK_RALT]  || keys[SDLK_LALT])   mod |= KMOD_ALT;
#else
    if (keys[SDL_SCANCODE_RSHIFT])  mod |= KMOD_RSHIFT;
    if (keys[SDL_SCANCODE_LSHIFT])  mod |= KMOD_LSHIFT;
    if (keys[SDL_SCANCODE_RCTRL] || keys[SDL_SCANCODE_LCTRL])  mod |= KMOD_CTRL;
    if (keys[SDL_SCANCODE_RALT]  || keys[SDL_SCANCODE_LALT])   mod |= KMOD_ALT;
#endif

    shift = (mod & KMOD_RSHIFT) ? shift |  1 : shift & 0xfe;
    shift = (mod & KMOD_LSHIFT) ? shift |  2 : shift & 0xfd;
    shift = (mod & KMOD_CTRL)   ? shift |  4 : shift & 0xfb;
    shift = (mod & KMOD_ALT)    ? shift |  8 : shift & 0xf7;
    shift = (mod & KMOD_CAPS)   ? shift | 16 : shift & 0xef;
    return shift;
#endif // ALIS_USE_NATIVE_ATARI
}

u8 io_getkey(void) {
    
    u8 result = 0;
    while (alis.state && (result = io_inkey()) == 0) { sys_sleep(k_event_ticks); }
    while (alis.state && io_inkey() != 0) { sys_sleep(k_event_ticks); }
    return result;
}

// Atari joystick byte: bit0=UP bit1=DOWN bit2=LEFT bit3=RIGHT bit7=FIRE. Like the
// original, merge SDL joystick + keyboard, and the mouse button as fire on
// joystick1 only (debut_pack mouse-as-joystick).
#if defined(ALIS_USE_NATIVE_ATARI)
// IKBD joystick packets (joyvec) + held keys; fire on joystick1 = either mouse button
// (right = the joystick's own fire while the mouse is on).
extern volatile u8 g_joy0, g_joy1, g_mouse_buttons;

static u8 sys_joy_keys(void) {
    const volatile u8 *k = g_ikbd_pressed;
    u8 b = 0;
    if (k[0x48] || k[0x68] || k[0x67] || k[0x69]) b |= 0x01;   // up, kp 8/7/9
    if (k[0x50] || k[0x6e] || k[0x6d] || k[0x6f]) b |= 0x02;   // down, kp 2/1/3
    if (k[0x4b] || k[0x6a] || k[0x67] || k[0x6d]) b |= 0x04;   // left, kp 4/7/1
    if (k[0x4d] || k[0x6c] || k[0x69] || k[0x6f]) b |= 0x08;   // right, kp 6/9/3
    return b;
}

static void sys_joy_refresh(void) {
    u8 keys = sys_joy_keys();
    joystick0 = g_joy0 | keys;
    joystick1 = g_joy1 | keys | ((g_mouse_buttons & 3) ? 0x80 : 0);
#if defined(ALIS_DEBUG_AUTOWALK)
    // Test runs: keep the camera moving (turn left, walk, turn right, walk), 1.5 s per step.
    static const u8 walk[] = { 0x04, 0x04, 0x01, 0x08, 0x08, 0x01, 0x01, 0x05 };
    joystick1 |= walk[(sys_ticks() / 1500) % sizeof walk];
#endif
}
#else
#define SYS_JOY_AXIS_DEAD   8000

static u8 sys_joy_compose(int with_mouse_fire) {
    u8 b = 0;

    if (sys_joy_handle) {
#if ALIS_SDL_VER == 1
        SDL_JoystickUpdate();
#endif
        s16 ax = SDL_JoystickGetAxis(sys_joy_handle, 0);
        s16 ay = SDL_JoystickGetAxis(sys_joy_handle, 1);
        if (ay < -SYS_JOY_AXIS_DEAD) b |= 0x01;
        if (ay >  SYS_JOY_AXIS_DEAD) b |= 0x02;
        if (ax < -SYS_JOY_AXIS_DEAD) b |= 0x04;
        if (ax >  SYS_JOY_AXIS_DEAD) b |= 0x08;

        if (SDL_JoystickNumHats(sys_joy_handle) > 0) {
            u8 h = SDL_JoystickGetHat(sys_joy_handle, 0);
            if (h & SDL_HAT_UP)    b |= 0x01;
            if (h & SDL_HAT_DOWN)  b |= 0x02;
            if (h & SDL_HAT_LEFT)  b |= 0x04;
            if (h & SDL_HAT_RIGHT) b |= 0x08;
        }

        int nb = SDL_JoystickNumButtons(sys_joy_handle);
        for (int i = 0; i < nb; i++) {
            if (SDL_JoystickGetButton(sys_joy_handle, i)) { b |= 0x80; break; }
        }
    }

    // Arrows + numpad as compass directions (7/9/1/3 set both bits). Shift is
    // not fire: games gating on ojoykey's fire bit would break (Metal Mutant).
    const u8 *keys = SDL_GetKeyboardState(NULL);
#if ALIS_SDL_VER == 1
    if (keys[SDLK_UP]    || keys[SDLK_KP8] || keys[SDLK_KP7] || keys[SDLK_KP9]) b |= 0x01;
    if (keys[SDLK_DOWN]  || keys[SDLK_KP2] || keys[SDLK_KP1] || keys[SDLK_KP3]) b |= 0x02;
    if (keys[SDLK_LEFT]  || keys[SDLK_KP4] || keys[SDLK_KP7] || keys[SDLK_KP1]) b |= 0x04;
    if (keys[SDLK_RIGHT] || keys[SDLK_KP6] || keys[SDLK_KP9] || keys[SDLK_KP3]) b |= 0x08;
#else
    if (keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_KP_8] || keys[SDL_SCANCODE_KP_7] || keys[SDL_SCANCODE_KP_9]) b |= 0x01;
    if (keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_KP_2] || keys[SDL_SCANCODE_KP_1] || keys[SDL_SCANCODE_KP_3]) b |= 0x02;
    if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_KP_4] || keys[SDL_SCANCODE_KP_7] || keys[SDL_SCANCODE_KP_1]) b |= 0x04;
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_KP_6] || keys[SDL_SCANCODE_KP_9] || keys[SDL_SCANCODE_KP_3]) b |= 0x08;
#endif

    if (with_mouse_fire && mouse.lb) b |= 0x80;
    return b;
}

static void sys_joy_refresh(void) {
    joystick0 = sys_joy_compose(0);
    joystick1 = sys_joy_compose(1);
}
#endif

u8 io_joy(u8 port) {
    sys_joy_refresh();
    return port ? joystick0 : joystick1;
}

u8 io_joykey(u8 test) {
    sys_joy_refresh();

    u8 result = 0;

    if (test == 0)
    {
        result = (joystick1 & 0x80) != 0;
        if ((shift & 4) != 0)
            result = result | 2;

        if ((shift & 8) != 0)
            result = result | 4;

        if (button.sym == -0x1f)
            result = result | 0x80;
    }

    return result;
}

// =============================================================================
#pragma mark - FILE SYSTEM
// =============================================================================

FILE * sys_fopen(char * path, u16 mode) {

    char flag[8] = "rb";
    
    if (mode & 0x100)
    {
        // create if necessary
        strcat(flag, "wb");
    }
    
    if (mode & 0x200)
    {
        // truncate if necessary
        strcpy(flag, "wb");
    }
    
    if (mode & 0x400)
    {
        // append
        strcpy(flag, "ab");
    }

    ALIS_DEBUG(EDebugInfo, " [%s][%.3x] ", path, mode);
    return fopen(strlower(path), flag);
}

int sys_fclose(FILE * fp) {
    return fclose(fp);
}

u8 sys_fexists(char * path) {
    u8 ret = 0;
    FILE * fp = fopen(strlower(path), "rb");
    if(fp) {
        ret = 1;
        sys_fclose(fp);
    }
    return ret;
}

// =============================================================================
#pragma mark - MISC
// =============================================================================

void sys_set_time(u16 h, u16 m, u16 s) {
    
}

time_t sys_get_time(void) {
    return 0;
}

u16 sys_get_model(void) {

    // Developer/author mode (-a): hand the startup test a sub-1000 model exactly once, then behave
    // normally so the script's platform discrimination still sees the true machine. See config.h.
    if (authormode)
    {
        authormode = 0;
        return kAuthorModel;
    }

    // 1010 = Atari ST / 1MB / lowrez
    // 1110 = Atari STe / 1MB / lowrez
    // 1111 = Atari STe / 1MB / mono
    
    // PC, 16-bit engines: 2000 + video adapter (0 CGA, 1 Tandy, 2 EGA, 4 VGA); scripts branch on
    // omodel % 10. The CGA-art games (version <= 11) and the 32-bit engines (version >= 30, where
    // the digit is a CPU class) keep 2000.
    
    // TODO: find all values
    switch (alis.platform.kind) {
            
        // Memory class (1000 = 512K, 1010 = 1MB, 1020 = more) + 100 for STE (v20+ engines).
        case EPlatformAtari:
            if (alis.memclass)
                return alis.memclass + (alis.platform.version < 20 ? 0 : 100);
            return alis.platform.version < 20 ? 1010 : 1120;
        case EPlatformPC:           return alis.platform.version > 11 && alis.platform.version < 30 ? 2004 : 2000;
        case EPlatformAmiga:        return 3000;
        case EPlatformMac:          return 4001;
        case EPlatformFalcon:       return 5000;    // NOTE: 256 color mac also use fo extension but 4000 model value
        case EPlatformAmigaAGA:     return 6000;
            
        // TODO: Amstrad CPC / 3DO / Jaguar values unknown
            
        default:                    return 4000;
    };
}


u16 sys_random(void) {
    return rand() & 0xffff;
}
