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
#include "image.h"
#include "mem.h"
#include "platform.h"
#include "utils.h"

#include "emu2149.h"
#include "math.h"

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

SDL_keysym      button = { 0, 0, 0, 0 };
SDL_Rect        dirty_rects[2048];
SDL_Rect        dirty_mouse_rect;

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
#define sys_sleep(t) SDL_Delay(t); sys_poll_event();

#elif ALIS_SDL_VER == 2

#ifdef _MSC_VER
# include "SDL.h"
# include "SDL_thread.h"
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

mouse_t         mouse;

bool            dirty_mouse;
float           scale = 2;
float           aspect_ratio = 1.2;
float           scale_x;
float           scale_y;
u32             width = 320;
u32             height = 200;

int             audio_id;
SDL_AudioSpec   *audio_spec;
#if !defined(__TOS__) && !defined(__atarist__)
PSG             *audio_psg;
#endif

extern u8       *vgalogic_df;

u8              failure;

u32             poll_ticks;
struct timeval  frame_time;
struct timeval  loop_time;

double          isr_step;
double          isr_counter;

//static u32      samplelength = 0;
//static u8       *samplebuffer[1024 * 1024 * 4];


u8 giaccess(s8 cmd, u8 data, u8 ch);
bool priorblanc(sChannel *channel);
u8 pblanc10(sChannel *a0, u8 d1b);

void sys_audio_callback(void *userdata, u8 *stream, s32 len);

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

    signal(signo, SIG_DFL);
//  raise(signo);
}

void sys_errors_init(void) {

    failure = 0;
    signal(SIGSEGV, signals_handler);
    signal(SIGABRT, signals_handler);
    signal(SIGFPE,  signals_handler);
    signal(SIGINT,  signals_handler);
    signal(SIGTERM, signals_handler);
    signal(SIGILL,  signals_handler);

#if defined (_WIN32) || defined (__MINGW32__)
//   _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
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

void sys_init_psg(void)
{
#if !defined(__TOS__) && !defined(__atarist__)
    audio_psg = PSG_new(2000000, audio_spec->freq);
    PSG_setClockDivider(audio_psg, 1);
    PSG_setVolumeMode(audio_psg, 1); // YM style
    PSG_setQuality(audio_psg, 1);
    PSG_reset(audio_psg);
#endif
}

void sys_deinit_psg(void)
{
#if !defined(__TOS__) && !defined(__atarist__)
    PSG_delete(audio_psg);
#endif
}

void sys_write_psg(u32 reg, u32 val)
{
#if defined(__TOS__) || defined(__atarist__)
    // TODO:
#else
    PSG_writeReg(audio_psg, reg, val);
#endif
}

u8 sys_read_psg(u32 reg)
{
    return
#if defined(__TOS__) || defined(__atarist__)
    // TODO:
    0;
#else
    PSG_readReg(audio_psg, reg);
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
    giaccess(index + 0x88, (channel->volume >> 11) & 0xf, index);
    giaccess(index * 2 + 0x80, max(8, channel->freq >> 3), index);
    giaccess(index * 2 + 0x81, max(1, ((channel->freq >> 3) >> 8)) & 0xf, index);
    
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

u8 giaccess(s8 cmd, u8 data, u8 ch)
{
    u8 regval = cmd & 0xf;
    if ((regval < 0xe) && (cmd < 0))
    {
        sys_write_psg(regval, data);
    }

    return sys_read_psg(regval);
}

void sys_audio_callback(void *userdata, u8 *s, s32 buffer_length)
{
    // TODO: create channels structure holding info about sounds to be played
    // type: (smaple, sound, noise, ...)
    // start: (playback start time, use it to calculate what to copy in to the stream)
    // frequency: we will have to do some interpolation to play at correct speed
    // ...

    memset(s, 0, buffer_length);

    u32 ratio = 0;
    s32 volratio = 0;
    
    bool update_ym = false;

#if ALIS_SND_INTERPOLATE_TYPE > 0
    int a0, a1, a2, a3;
    int x0, x1, x2, x3;
    float t, r;
#endif

    int smpidx = 0;

    // handle music
    
    if (audio.muflag > 0)
    {
        int smprem = buffer_length;

        int buflen = audio.mutaloop * 2;
        float lenf = (float)buffer_length / (float)buflen;
        int len = ceil(lenf);
        
        if (audio.smpidx)
        {
            int smpcopy = min(audio.smpidx, smprem);
            
            memcpy(s, (u8 *)audio.muadresse + (buflen - audio.smpidx), smpcopy);

            smpidx += smpcopy;
            smprem -= smpcopy;
            
            audio.smpidx -= smpcopy;
        }
        
        for (int i = 0; i < len && smprem; i++)
        {
            audio.soundrout();
            
            int smpcopy = min(buflen, smprem);
            
            memcpy(s + smpidx, audio.muadresse, smpcopy);
            
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
        u16 *audio_buffer = (u16 *)s;
        buffer_length >>= 1;
        
        double prev_counter = isr_counter;
        u32 add = 0;
        s32 s0, s1;
        s32 accumulator, prevsmpidx;
        
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

                    ratio = (ch->freq << 16) / audio_spec->freq;
                    accumulator = ch->played * ratio;
                    smpidx = accumulator >> 16;
                    accumulator &= 0xFFFF;

#if ALIS_SND_INTERPOLATE_TYPE > 0
                    x0 = (smpidx - 1 >= 0) ? (s16)(ch->address[smpidx - 1] * 256) : ch->loop > 1 ? (s16)(ch->address[ch->length - 1] * 256) : 0;
                    x1 = (s16)(ch->address[smpidx] * 256);
                    x2 = (smpidx + 1 < ch->length) ? (s16)(ch->address[smpidx + 1] * 256) : ch->loop > 1 ? (s16)(ch->address[0] * 256) : 0;
                    x3 = (smpidx + 2 < ch->length) ? (s16)(ch->address[smpidx + 2] * 256) : ch->loop > 1 ? (s16)(ch->address[0] * 256) : 0;
# if ALIS_SND_INTERPOLATE_TYPE == 1
                    a0 = x3 - x2 - x0 + x1;
                    a1 = x0 - x1 - a0;
                    a2 = x2 - x0;
# endif
#else
                    s0 = (s8)ch->address[smpidx] * volratio;
#endif
                    
                    for (int buffer_index = 0; buffer_index < buffer_length; buffer_index++, ch->played++, accumulator+=ratio)
                    {
                        if ((add = accumulator >> 16))
                        {
                            smpidx += add;
#if ALIS_SND_INTERPOLATE_TYPE > 0
                            if (smpidx < ch->length - 3)
                            {
                                x0 = x1;
                                x1 = x2;
                                x2 = x3;
                                x3 = (s16)(ch->address[smpidx + 2] * 256);
                            }
                            else
                            {
                                x0 = x1;
                                x1 = x2;
                                x2 = x3;
                                x3 = 0;
#endif
                                if (smpidx >= ch->length + 1)
                                {
                                    if (ch->loop > 1)
                                    {
                                        ch->loop--;
                                        ch->played = smpidx = accumulator = 0;
                                    }
                                    else
                                    {
                                        ch->type = eChannelTypeNone;
                                        ch->curson = 0x80;
                                        break;
                                    }
                                }
#if ALIS_SND_INTERPOLATE_TYPE > 0
                            }

# if ALIS_SND_INTERPOLATE_TYPE == 1
                            a0 = x3 - x2 - x0 + x1;
                            a1 = x0 - x1 - a0;
                            a2 = x2 - x0;
# endif
#else
                            s0 = (s8)ch->address[smpidx] * volratio;
#endif
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
                        audio_buffer[buffer_index] = (s0 + audio_buffer[buffer_index]);
                        accumulator &= 0xFFFF;
                    }
                    break;
                }
                    
                case eChannelTypeDingZap:
                case eChannelTypeNoise:
                case eChannelTypeExplode:
                {
                    if (i < 3)
                    {
                        for (int p = 0; p < buffer_length; p++)
                        {
                            isr_counter += isr_step;
                            if (isr_counter >= 1)
                            {
                                isr_counter--;
                                
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
                                    io_canal(ch, i);
                                }
                                while (false);
                            }
                            
#if !defined(__TOS__) && !defined(__atarist__)
                            s0 = PSG_calc(audio_psg);
                            s1 = audio_buffer[p] - 0x8000;
                            audio_buffer[p] = (s0 + s1) + 0x8000;
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
    
//    // NOTE: just for checking
//
//    u8 *src = (u8 *)s;
//    u8 *tgt = (u8 *)samplebuffer + samplelength;
//
//    for (int x = 0; x < length; x++)
//        *tgt++ = *src++;
//
//    samplelength += length;
//
//    if (samplelength >= 1024 * 1024 * .3)
//    {
//        FILE *f = fopen("/Users/gildor/Desktop/test.smp", "wb");
//        if (f == NULL)
//        {
//            return;
//        }
//
//        fwrite(samplebuffer, samplelength, 2, f);
//        fclose(f);
//
//        samplelength = 0;
//    }
}

// =============================================================================
#pragma mark - I/O
// =============================================================================

mouse_t sys_get_mouse(void) {
    return mouse;
}

void sys_set_mouse(u16 x, u16 y) {
    mouse.x = x;
    mouse.y = y;
}

void sys_enable_mouse(u8 enable) {
    mouse.enabled = enable;

    if (!mouse.enabled)
        alis.desmouse = NULL;
    
    dirty_mouse = true;
}

void set_update_cursor(void) {
    
    dirty_mouse = true;
}

u8 io_inkey(void)
{
#if ALIS_SDL_VER == 1
    SDL_PumpEvents();
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
        
        case SDLK_KP_0:         return 48;
        case SDLK_KP_1:         return 49;
        case SDLK_KP_2:         return 50;
        case SDLK_KP_3:         return 51;
        case SDLK_KP_4:         return 52;
        case SDLK_KP_5:         return 53;
        case SDLK_KP_6:         return 54;
        case SDLK_KP_7:         return 55;
        case SDLK_KP_8:         return 56;
        case SDLK_KP_9:         return 57;
            
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

        default:                return button.sym;
    }
}

u8 io_shiftkey(void) {
    if (shift && !(SDL_GetModState() & KMOD_SHIFT))
    {
        shift = 0;
    }

    return shift;
}

u8 io_getkey(void) {
    
    u8 result = 0;
    while (alis.state && (result = io_inkey()) == 0) { sys_sleep(k_event_ticks); }
    while (alis.state && io_inkey() != 0) { sys_sleep(k_event_ticks); }
    return result;
}

u8 io_joy(u8 port) {
    return port ? joystick0 : joystick1;
}

u8 io_joykey(u8 test) {
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

    // 1010 = Atari ST / 1MB / lowrez
    // 1110 = Atari STe / 1MB / lowrez
    // 1111 = Atari STe / 1MB / mono
    
    // on PC, there are only 4 correct values (mono, cga, ega, vga?)
    // 2000 + [0 - 4]
    
    // TODO: find all values
    switch (alis.platform.kind) {
            
        case EPlatformAtari:        return alis.platform.version < 20 ? 1010 : 1110;
        case EPlatformPC:           return 2000;
        case EPlatformAmiga:        return 3000;
        case EPlatformMac:          return 4001;
        case EPlatformFalcon:       return 5000;    // NOTE: 256 color mac also use fo extension but 4000 model value
        case EPlatformAmigaAGA:     return 6000;
            
        // TODO: find correct values
        // case EPlatformAmstradCPC:   return 4000;
        // case EPlatform3DO:          return 4000;
        // case EPlatformJaguar:       return 4000;
            
        default:                    return 4000;
    };
}


u16 sys_random(void) {
    return rand() & 0xffff;
}
