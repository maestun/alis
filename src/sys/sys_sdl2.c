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

#ifdef _MSC_VER
# include "SDL.h"
# include "SDL_thread.h"
#else
# include <SDL2/SDL.h>
# include <SDL2/SDL_thread.h>
#endif

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

#define ALIS_SND_INTERPOLATE_TYPE 1


u8 joystick0 = 0;
u8 joystick1 = 0;
u8 shift = 0;
SDL_Keysym button = { 0, 0, 0, 0 };

mouse_t _mouse;

SDL_Renderer *  _renderer;
SDL_TimerID     _timerID;
SDL_Window *    _window;
SDL_Event       _event;
Uint32 *        _pixels;
SDL_Texture *   _texture;
SDL_Cursor *    _cursor;
bool            _update_cursor;
float           _scale = 2;
float           _aspect_ratio = 1.2;
float           _scaleX;
float           _scaleY;
u32             _width = 320;
u32             _height = 200;
SDL_sem *       _render_sem = NULL;

int             _audio_id;
SDL_AudioSpec   *_audio_spec;
PSG             *_psg;

extern u8       fls_ham6;
extern u8       fls_s512;
extern u8       *vgalogic_df;

u8              global_failure;

double isr_step;
double isr_counter;

u8 giaccess(s8 cmd, u8 data, u8 ch);
bool priorblanc(sChannel *channel);
u8 pblanc10(sChannel *a0, u8 d1b);

void sys_audio_callback(void *userdata, Uint8 *stream, int len);

// ============================================================================
#pragma mark - Signals
// ============================================================================

void signals_info(int signo) {
    switch (signo) {
        case 0:
        {
            break;
        }
        printf("\n");
        case SIGSEGV:
        {
            debug(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGSEGV: Invalid memory access (segmentation fault).\n");
            break;
        }
        case SIGABRT:
        {
            debug(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGABRT: Abnormal termination.\n");
            break;
        }
        case SIGFPE:
        {
            debug(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGFPE: Erroneous arithmetic operation.\n");
            break;
        }
        case SIGINT:
        {
            debug(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGINT: External interrupt.\n");
            break;
        }
        case SIGTERM:
        {
            debug(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGTERM: Termination request.\n");
            break;
        }
        case SIGILL:
        {
            debug(EDebugFatal, disalis ? "FATAL ERROR: %s" : "%s", "SIGILL: Invalid program image.\n");
            break;
        }
        default:
            if (disalis) {
                debug(EDebugFatal, "FATAL ERROR: An unidentified error %d occurred.\n", signo);
            }
            else {
                debug(EDebugFatal, "An unidentified error %d occurred.\n", signo);
            } 
        }
    debug(EDebugSystem, "A STOP signal has been sent to the host system queue...\n");
}

void signals_handler(int signo) {

    if (signo>0) { 
        global_failure = signo;
    }
    else {
        global_failure = 1;
    }
    alis.state = eAlisStateStopped;

    signal(signo, SIG_DFL);
    raise(signo);
}

void sys_errors_init(void) {

    global_failure = 0;
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

// ============================================================================
#pragma mark - System init
// ============================================================================

void sys_init(sPlatform *pl, int fullscreen) {

    _width = pl->width;
    _height = pl->height;
    
    _scaleX = _scale;
    _scaleY = _scale * _aspect_ratio;
    
    printf("  SDL initialization...\n");
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0) {
        fprintf(stderr, "   Unable to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }
    
    _timerID = SDL_AddTimer(20, itroutine, NULL);
    if (!_timerID) {
        fprintf(stderr, "   Could not create timer: %s\n", SDL_GetError());
        exit(-1);
    }
    
    printf("  Video initialization...\n");
    _window = SDL_CreateWindow(kProgName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width * _scaleX, _height * _scaleY, SDL_WINDOW_RESIZABLE | fullscreen * SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (_window == NULL) {
        fprintf(stderr, "   Could not create %.0fx%.0f window: %s\n", (_width * _scaleX), (_height * _scaleY), SDL_GetError());
        exit(-1);
    }
    
    _renderer = SDL_CreateRenderer(_window, -1, 0);
    if (_renderer == NULL) {
        fprintf(stderr, "   Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(_window);
        exit(-1);
    }
    
    SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
    SDL_RenderSetScale(_renderer, _scale, _scale);
    SDL_RenderSetLogicalSize(_renderer, _width, _height * _aspect_ratio);
    
    _pixels = malloc(_width * _height * sizeof(*_pixels));
    memset(_pixels, 0, _width * _height * sizeof(*_pixels));
    
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _width, _height);
    if (_texture == NULL) {
        fprintf(stderr, "   Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(_renderer);
        SDL_DestroyWindow(_window);
        exit(-1);
    }
    
    SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
    
    _cursor = NULL;
    _update_cursor = 0;
    
    printf("  Audio initialization...\n");
    _audio_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    
    SDL_AudioSpec *desired_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    
    SDL_zero(*desired_spec);
    desired_spec->freq = 44100;
    desired_spec->format = AUDIO_S16;
    desired_spec->channels = 1;
    desired_spec->samples = desired_spec->freq / 50; // 20 ms
    desired_spec->callback = sys_audio_callback;
    desired_spec->userdata = NULL;
    desired_spec->size = desired_spec->samples * 2;
    
    if ((_audio_id = SDL_OpenAudioDevice(NULL, 0, desired_spec, _audio_spec, 0)) <= 0 ) {
        fprintf(stderr, "   Could not open audio: %s\n", SDL_GetError());
        exit(-1);
    }
    
    // Extended information on obtained audio for tests and reports
    printf("  Opened audio device id %d successfully:\n", _audio_id);
    printf("    Frequency:  %d\n", _audio_spec->freq);
    printf("    Format:     0x%04x => %d bits per sample\n", _audio_spec->format, (int) SDL_AUDIO_BITSIZE(_audio_spec->format));
    printf("    Channels:   %d\n", _audio_spec->channels);
    printf("    Samples:    %d\n", _audio_spec->samples);
    printf("    Silence:    %d\n", _audio_spec->silence);
    printf("    Padding:    %d\n", _audio_spec->padding);
    printf("    Size:       %d\n", _audio_spec->size);
    
    free(desired_spec);
    
    _psg = PSG_new(2000000, _audio_spec->freq);
    PSG_setClockDivider(_psg, 1);
    PSG_setVolumeMode(_psg, 1); // YM style
    PSG_setQuality(_psg, 1);
    PSG_reset(_psg);
    
    isr_step = (double)50.0 / (double)(_audio_spec->freq);
    isr_counter = 1;
    
    SDL_PauseAudioDevice(_audio_id, 0);
    
    _render_sem = SDL_CreateSemaphore(1);
    if (_render_sem == NULL) {
        fprintf(stderr, "   Could not create semaphore: %s\n", SDL_GetError());
        exit(-1);
    };
}

// ============================================================================
#pragma mark - Main system loop
// ============================================================================

u8 sys_start(void) {

    if (global_failure) return -1;

    alis.state = eAlisStateRunning;

    SDL_Thread *thread = SDL_CreateThread(alis_thread, "ALIS", (void *)NULL);
    if (thread == NULL) {
        printf("Could not create thread! %s\n", SDL_GetError());
        return 1;
    }
    
    while (alis.state && !global_failure) {

        // wait so image is drawn and keyboard is polled at 120 fps
        usleep(kControlsTicks);
        
        sys_render(host.pixelbuf);
        
        SDL_PollEvent(&_event);

        // update mouse
        u32 bt = SDL_GetMouseState(&_mouse.x, &_mouse.y);
        
        float newx, newy;
        SDL_RenderWindowToLogical(_renderer, _mouse.x, _mouse.y, &newx, &newy);
        
        _mouse.x = newx;
        _mouse.y = newy / _aspect_ratio;
        _mouse.lb = SDL_BUTTON(bt) == SDL_BUTTON_LEFT;
        _mouse.rb = SDL_BUTTON(bt) == SDL_BUTTON_RIGHT;
        
        switch (_event.type) {
                
            case SDL_QUIT:
            {
                printf("\n");
                debug(EDebugSystem, "SDL2: Quit.\n");
                debug(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
                alis.state = eAlisStateStopped;
                break;
            }
            case SDL_KEYUP:
            {
                switch (_event.key.keysym.sym)
                {
                    case SDLK_LALT:
                        printf("\n");
                        debug(EDebugSystem, "INTERRUPT: User debug label.\n");
                        break;

                    case SDLK_F11:
                        alis.state = eAlisStateSave;
                        break;

                    case SDLK_F12:
                        alis.state = eAlisStateLoad;
                        break;
                        
                    default:
                        break;
                }

                break;
            }
            case SDL_KEYDOWN:
            {
                if (_event.key.keysym.mod == KMOD_RSHIFT || _event.key.keysym.mod == KMOD_LSHIFT)
                {
                    shift = 1;
                }

                if (_event.key.keysym.sym == SDLK_F11 || _event.key.keysym.sym == SDLK_F12)
                {
                    break;
                }

                if (_event.key.keysym.sym == SDLK_PAUSE)
                {
                    printf("\n");
                    debug(EDebugSystem, "INTERRUPT: Quit by user request.\n");
                    debug(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
                    alis.state = eAlisStateStopped;
                }
                else
                {
                    button = _event.key.keysym;
                }
                
                break;
            }
            case SDL_WINDOWEVENT:
            {
                if (_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    SDL_RenderGetScale(_renderer, &_scaleX, &_scaleY);
                    _scaleY *= _aspect_ratio;
                    
                    _update_cursor = true;
                }
                
                break;
            }
        };
    }

    signals_info(global_failure);
    if (!global_failure) {
         debug(EDebugInfo, "Waiting for the VM kernel thread to finish...\n");
         SDL_WaitThread(thread, NULL);
         } 
         else {
                debug(EDebugSystem, "Ignoring the crashed VM kernel thread...\n");
              }

    return 0;
}

// ============================================================================
#pragma mark - Video
// ============================================================================

void sys_render(pixelbuf_t buffer) {
    
    SDL_SemWait(_render_sem);
    
    if (_update_cursor)
    {
        _update_cursor = false;
        sys_update_cursor();
    }

    if (fls_ham6)
    {
        u8 *bitmap = vgalogic_df + 0xa0;
    
        // Amiga HAM bitplanes
        
        u32 px = 0;
        u32 planesize = (buffer.w * buffer.h) / 8;
        u8 c0, c1, c2, c3, c4, c5;
        u8 r = 0;
        u8 g = 0;
        u8 b = 0;

        for (s32 h = 0; h < buffer.h; h++)
        {
            u8 *tgt = buffer.data + (h * buffer.w);
            for (s32 w = 0; w < buffer.w; w++, tgt++, px++)
            {
                s32 idx = (w + h * buffer.w) / 8;
                c0 = *(bitmap + idx);
                c1 = *(bitmap + (idx += planesize));
                c2 = *(bitmap + (idx += planesize));
                c3 = *(bitmap + (idx += planesize));
                c4 = *(bitmap + (idx += planesize));
                c5 = *(bitmap + (idx += planesize));

                int bit = 7 - (w % 8);
                
                int control = ((c4 >> bit) & 1) << 0 | ((c5 >> bit) & 1) << 1;
                int index = ((c0 >> bit) & 1) | ((c1 >> bit) & 1) << 1 | ((c2 >> bit) & 1) << 2 | ((c3 >> bit) & 1) << 3;

                switch (control) {
                    case 0:
                        r = buffer.palette[index * 3 + 0];
                        g = buffer.palette[index * 3 + 1];
                        b = buffer.palette[index * 3 + 2];
                        break;
                    case 1:
                        b = index << 4;
                        break;
                    case 2:
                        r = index << 4;
                        break;
                    case 3:
                        g = index << 4;
                        break;
                }

                _pixels[px] = (u32)(0xff000000 + (r << 16) + (g << 8) + (b << 0));
            }
        }

        SDL_UpdateTexture(_texture, NULL, _pixels, _width * sizeof(*_pixels));

        // render
        SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255);
        SDL_RenderClear(_renderer);
        SDL_RenderCopy(_renderer, _texture, NULL, NULL);
        SDL_RenderPresent(_renderer);
    }
    else if (fls_s512)
    {
        u8 *bitmap = vgalogic_df + 0xa0;

        // Atari ST Spectrum 512 bitplanes
        
        u16 curpal[16];
        memcpy(curpal, vgalogic_df + 32000, 32);

        // s16 pallines = 0xc5 - fls_pallines;

        u16 *palette = (u16 *)(vgalogic_df + 32000 + 32);

        // ST scanline width 48 + 320 + 44 (412)
        // change palette every 412 / 48 ?
        float pxs = 9.6;

        int limit0 = -4;
        int limit1 = 156;
        
        u32 px = 0;
        u32 at = 0;

        u32 palidx;
        u8 *rawcolor;
        u8 r, g, b;
        
        for (int y = 0; y < buffer.h; y++, palette += 16)
        {
            int lpx = 0;
            
            for (int x = 0; x < buffer.w; x+=16, at+=8)
            {
                for (int dpx = 0; dpx < 8; dpx++, lpx++, px++)
                {
                    // handle palette for the fist 8 pixels
                    
                    if (lpx >= limit0)
                    {
                        if (lpx == limit1)
                        {
                            palette += 16;
                        }
                        
                        palidx = lpx < limit1 ? (lpx - limit0) / pxs : (lpx - limit1) / pxs;
                        if (palidx < 16)
                        {
                            curpal[palidx] = palette[palidx];
                        }
                    }
                    
                    u32 rot = (7 - dpx);
                    u32 mask = 1 << rot;
                    
                    rawcolor = (u8 *)&(curpal[(((bitmap[at + 0] & mask) >> rot) << 0) | (((bitmap[at + 2] & mask) >> rot) << 1) | (((bitmap[at + 4] & mask) >> rot) << 2) | (((bitmap[at + 6] & mask) >> rot) << 3)]);
                    r = (rawcolor[0] & 0b00000111) << 5;
                    g = (rawcolor[1] >> 4) << 5;
                    b = (rawcolor[1] & 0b00000111) << 5;
                    _pixels[px] = (u32)(0xff000000 + (r << 16) + (g << 8) + (b << 0));
                }
                
                for (int dpx = 0; dpx < 8; dpx++, lpx++, px++)
                {
                    // handle palette for the second 8 pixels
                    
                    if (lpx >= limit0)
                    {
                        if (lpx == limit1)
                        {
                            palette += 16;
                        }
                        
                        palidx = lpx < limit1 ? (lpx - limit0) / pxs : (lpx - limit1) / pxs;
                        if (palidx < 16)
                        {
                            curpal[palidx] = palette[palidx];
                        }
                    }
                    
                    u32 rot = (7 - dpx);
                    u32 mask = 1 << rot;
                    
                    rawcolor = (u8 *)&(curpal[(((bitmap[at + 1] & mask) >> rot) << 0) | (((bitmap[at + 3] & mask) >> rot) << 1) | (((bitmap[at + 5] & mask) >> rot) << 2) | (((bitmap[at + 7] & mask) >> rot) << 3)]);
                    r = (rawcolor[0] & 0b00000111) << 5;
                    g = (rawcolor[1] >> 4) << 5;
                    b = (rawcolor[1] & 0b00000111) << 5;
                    _pixels[px] = (u32)(0xff000000 + (r << 16) + (g << 8) + (b << 0));
                }
            }
            
            palette += 16;
            memcpy(curpal, palette, 32);
        }

        SDL_UpdateTexture(_texture, NULL, _pixels, _width * sizeof(*_pixels));

        // render
        SDL_RenderClear(_renderer);
        SDL_RenderCopy(_renderer, _texture, NULL, NULL);
        SDL_RenderPresent(_renderer);
    }
    else
    {
        if (image.flinepal == 0)
        {
            int index;
            for (int px = 0; px < buffer.w * buffer.h; px++)
            {
                index = buffer.data[px];
                _pixels[px] = (u32)(0xff000000 + (buffer.palette[index * 3 + 0] << 16) + (buffer.palette[index * 3 + 1] << 8) + (buffer.palette[index * 3 + 2] << 0));
            }
        }
        else
        {
            s16 *palentry = image.firstpal;
            
            for (int i = 0; i < 3; i++)
            {
                s16 unk1 = palentry[0];
                if (unk1 == 0xff)
                    break;
                
                s16 y1 = palentry[1];
                u8 *palette = *(u8 **)&(palentry[2]);
                
                palentry += 2 + (sizeof(u8 *) >> 1);
                s16 y2 = palentry[0] == 0xff ? _height : min(palentry[1], _height);
                
                int index;
                for (int y = y1; y < y2; y++)
                {
                    s32 px = y * buffer.w;
                    for (int x = 0; x < buffer.w; x++, px++)
                    {
                        index = buffer.data[px];
                        _pixels[px] = (u32)(0xff000000 + (palette[index * 3 + 0] << 16) + (palette[index * 3 + 1] << 8) + (palette[index * 3 + 2] << 0));
                    }
                }
            }
        }
        
        SDL_UpdateTexture(_texture, NULL, _pixels, _width * sizeof(*_pixels));
        
        // render
        SDL_RenderClear(_renderer);
        SDL_RenderCopy(_renderer, _texture, NULL, NULL);
        SDL_RenderPresent(_renderer);
    }
    
    SDL_SemPost(_render_sem);
}

// ============================================================================
#pragma mark - System deinit
// ============================================================================

void sys_deinit(void) {
    
    SDL_RemoveTimer(_timerID);
    SDL_PauseAudioDevice(_audio_id, 1);
    SDL_CloseAudioDevice(_audio_id);
    SDL_DestroySemaphore(_render_sem);
    
    SDL_Delay(20);   // 20ms fail-safe delay to make sure that sound buffer is empty

    PSG_delete(_psg);
    free(_audio_spec);

//   SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

// ============================================================================
#pragma mark - Audio
// ============================================================================

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

static u32 samplelength = 0;
static u8 *samplebuffer[1024 * 1024 * 4];

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
    giaccess(index * 2 + 0x80, channel->freq >> 3, index);
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

u8 giaccess(s8 cmd, u8 data, u8 ch)
{
    u8 regval = cmd & 0xf;
    if ((regval < 0xe) && (cmd < 0))
    {
        PSG_writeReg(_psg, regval, data);
    }

    return PSG_readReg(_psg, regval);
}

void sys_audio_callback(void *userdata, Uint8 *s, int length)
{
    // TODO: create channels structure holding info about sounds to be played
    // type: (smaple, sound, noise, ...)
    // start: (playback start time, use it to calculate what to copy in to the stream)
    // frequency: we will have to do some interpolation to play at correct speed
    // ...

    memset(s, 0, length);

    float ratio = 0;
    float volratio = 0;

    int amplitude = 0;
    int frequency = 0;
    int tone_off = 0;
    int noise_off = 1;
    int envelope_on = 0;
    
    bool update_ym = false;

#if ALIS_SND_INTERPOLATE_TYPE > 0
    float smpidxf = 0;
    float x0, x1, x2, x3, t, r;
#endif

    int smpidx = 0;

    // handle music
    
    if (audio.muflag > 0)
    {
        int smprem = length;

        int buflen = audio.mutaloop * 2;
        float lenf = (float)length / (float)buflen;
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
        u16 *strm = (u16 *)s;
        length /= 2;
        
        for (int p = 0; p < length; p++)
        {
            isr_counter += isr_step;
            if (isr_counter >= 1)
            {
                isr_counter--;
                update_ym = true;
            }
            else
            {
                update_ym = false;
            }
            
            for (int i = 0; i < 4; i++)
            {
                sChannel *ch = &audio.channels[i];
                switch (ch->type)
                {
                    case eChannelTypeSample:
                    {
                        ratio = (float)(ch->freq) / (float)(_audio_spec->freq);
                        volratio = ((float)(ch->volume) / 128.0) / 4;
                        
#if ALIS_SND_INTERPOLATE_TYPE > 0
                        // interpolation
                        
                        smpidxf = ch->played * ratio;
                        smpidx = (int)smpidxf;
                        if (smpidx >= ch->length)
                        {
                            if (ch->loop > 1)
                            {
                                ch->loop--;
                                ch->played = smpidx = 0;
                            }
                            else
                            {
                                ch->type = eChannelTypeNone;
                                break;
                            }
                        }
                        
                        int i1 = (int)smpidxf;
                        if (i1 >= ch->length)
                            i1 = (ch->loop > 1) ? 0 : -1;
                        
                        int i2 = i1 + 1;
                        if (i2 >= ch->length)
                            i2 = (ch->loop > 1) ? 0 : -1;
                        
                        int i3 = i2 + 1;
                        if (i3 >= ch->length)
                            i3 = (ch->loop > 1) ? 0 : -1;
                        
                        int i0 = i1 - 1;
                        if (i0 < 0)
                            i0 = (ch->loop > 1) ? ch->length -1 : -1;
                        
                        
                        //                        s8 sam = xread8(startsam1 + test3 - 0x10);
                        //                        int total = audio.muadresse[index] + sam;
                        //                        if (total < -32768)
                        //                            total = -32768;
                        //
                        //                        if (total > 32767)
                        //                            total = 32767;
                        
                        //                        x0 = (i0 >= 0) ? (s16)((ch->address[i0] | ch->address[i0] << 8) - 0x8000) : 0;
                        //                        x1 = (i1 >= 0) ? (s16)((ch->address[i1] | ch->address[i1] << 8) - 0x8000) : 0;
                        //                        x2 = (i2 >= 0) ? (s16)((ch->address[i2] | ch->address[i2] << 8) - 0x8000) : 0;
                        //                        x3 = (i3 >= 0) ? (s16)((ch->address[i3] | ch->address[i3] << 8) - 0x8000) : 0;
                        x0 = (i0 >= 0) ? (s16)(ch->address[i0] * 256) : 0;
                        x1 = (i1 >= 0) ? (s16)(ch->address[i1] * 256) : 0;
                        x2 = (i2 >= 0) ? (s16)(ch->address[i2] * 256) : 0;
                        x3 = (i3 >= 0) ? (s16)(ch->address[i3] * 256) : 0;
                        
                        t = smpidxf - (int)smpidxf;
                        
# if ALIS_SND_INTERPOLATE_TYPE == 1
                        r = interpolate_cubic(x0, x1, x2, x3, t);
# elif ALIS_SND_INTERPOLATE_TYPE == 2
                        r = interpolate_hermite(x0, x1, x2, x3, t);
# else // 3
                        r = interpolate_hermite_4pt_3ox(x0, x1, x2, x3, t);
# endif
                        
                        s32 s0 = r * volratio;
#else
                        // no interpolation
                        
                        smpidx = ch->played * ratio;
                        if (smpidx >= ch->length)
                        {
                            if (ch->loop > 1)
                            {
                                ch->loop--;
                                ch->played = smpidx = 0;
                            }
                            else
                            {
                                ch->type = eChannelTypeNone;
                                break;
                            }
                        }
                        
                        //                        s32 s0 = ((ch->address[smpidx] | ch->address[smpidx] << 8) - 0x8000) * volratio;
                        s32 s0 = (ch->address[smpidx] * 256) * volratio;
#endif
                        s32 s1 = strm[p] - 0x8000;
                        strm[p] = (s0 + s1) + 0x8000;
                        
                        ch->played ++;
                    }
                        break;
                        
                    case eChannelTypeDingZap:
                    case eChannelTypeNoise:
                    case eChannelTypeExplode:
                    {
                        if (i < 3 && update_ym)
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
                                    u16 freq = ch->delta_freq + ch->freq;
                                    if (-1 < (s32)((u32)freq << 0x10))
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
                    }
                        break;
                        
                    default:
                        break;
                }
                
                s32 s0 = PSG_calc(_psg);
                s32 s1 = strm[p] - 0x8000;
                
                s32 val = (strm[p] + ((s0 + s1) + 0x8000)) / 2;
                strm[p] = val;
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
    return _mouse;
}

void sys_set_mouse(u16 x, u16 y) {
    _mouse.x = x;
    _mouse.y = y;
}

void sys_enable_mouse(u8 enable) {
    _mouse.enabled = enable;

    if (!_mouse.enabled)
        alis.desmouse = NULL;
    
    _update_cursor = true;
}

void set_update_cursor(void) {
    
    _update_cursor = true;
}

void sys_update_cursor(void) {
    
    if (alis.desmouse == NULL)
    {
        SDL_ShowCursor(_mouse.enabled);
        return;
    }
    
    u8 type = alis.desmouse[0];
    u16 width = read16(alis.desmouse + 2) + 1;
    u16 height = read16(alis.desmouse + 4) + 1;
    
    u32 *pixels = malloc(width * height * sizeof(u32));

    switch (type)
    {
        case 0x00:
        case 0x02:
        {
            u8 *palette = host.pixelbuf.palette;

            if (image.flinepal)
            {
                s16 *palentry = image.firstpal;

                for (int i = 0; i < 3; i++)
                {
                    s16 unk1 = palentry[0];
                    if (unk1 == 0xff)
                        break;

                    palette = *(u8 **)&(palentry[2]);
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
                    pixels[px] = color == clear ? 0 : (u32)(0xff000000 + (palette[color * 3 + 0] << 16) + (palette[color * 3 + 1] << 8) + (palette[color * 3 + 2] << 0));
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
                    int index = palidx + color;
                    pixels[px] = color == clear ? 0 : (u32)(0xff000000 + (host.pixelbuf.palette[index * 3 + 0] << 16) + (host.pixelbuf.palette[index * 3 + 1] << 8) + (host.pixelbuf.palette[index * 3 + 2] << 0));
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
                pixels[px] = color == clear ? 0 : (u32)(0xff000000 + (host.pixelbuf.palette[color * 3 + 0] << 16) + (host.pixelbuf.palette[color * 3 + 1] << 8) + (host.pixelbuf.palette[color * 3 + 2] << 0));
            }

            break;
        }
            
        default:
        {
            break;
        }
    };
    
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, width * 4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!surface)
    {
        printf("Failed to create mouse surface: %s\n", SDL_GetError());
        return;
    }
    
    SDL_Surface *scaledSurface = SDL_CreateRGBSurface(0, width * _scaleX, height * _scaleY, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
    if (SDL_BlitScaled(surface, NULL, scaledSurface, NULL) < 0)
    {
        printf("Failed to scale mouse cursor: %s\n", SDL_GetError());

        SDL_FreeSurface(scaledSurface);
        scaledSurface = NULL;
    }
    else
    {
        SDL_FreeSurface(surface);

        surface = scaledSurface;
        width = surface->w;
        height = surface->h;
    }
    
    _cursor = SDL_CreateColorCursor(surface, 0, 0);
    
    SDL_FreeSurface(surface);
    
    SDL_SetCursor(_cursor);
    SDL_ShowCursor(_mouse.enabled);

    free(pixels);
}

u8 io_inkey(void)
{
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (!currentKeyStates[button.scancode])
    {
        button.scancode = 0;
        button.sym = 0;
    }
    debug(EDebugInfo, " [\"%s\"]", SDL_GetKeyName(button.sym));
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
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (shift && !currentKeyStates[SDL_SCANCODE_LSHIFT] && !currentKeyStates[SDL_SCANCODE_RSHIFT])
    {
        shift = 0;
    }

    return shift;
}

u8 io_getkey(void) {
    
    u8 result = 0;
    while (alis.state && (result = io_inkey()) == 0) { usleep(kControlsTicks); }
    while (alis.state && io_inkey() != 0) { usleep(kControlsTicks); }
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

    debug(EDebugInfo, " [%s][%.3x] ", path, mode);
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

// =============================================================================
#pragma mark - SYNC
// =============================================================================

void sys_lock_renderer(void) {
    
    SDL_SemWait(_render_sem);
}

void sys_unlock_renderer(void) {
    
    SDL_SemPost(_render_sem);
}
