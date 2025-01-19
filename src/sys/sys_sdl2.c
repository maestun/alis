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

#if ALIS_SDL_VER == 2

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

const u32               k_event_ticks = (1000000 / 120);  // poll keyboard 120 times per second
const u32               k_frame_ticks = (1000000 / 50);   // 50 fps screen update

extern u8               shift;
extern mouse_t          mouse;

extern SDL_Renderer    *renderer;
extern SDL_Window      *window;
extern SDL_Texture     *texture;
extern SDL_Cursor      *cursor;

extern SDL_Keysym      button;
extern SDL_Event       event;

extern SDL_sem         *render_sem;
extern SDL_TimerID     timer_id;

extern SDL_AudioSpec   *audio_spec;

extern bool            dirty_mouse;
extern float           scale;
extern float           aspect_ratio;
extern float           scale_x;
extern float           scale_y;
extern u32             width;
extern u32             height;

extern int             audio_id;

extern u8              fls_ham6;
extern u8              fls_s512;
extern u8              *vgalogic_df;

extern u8              failure;

extern u32             poll_ticks;
extern struct timeval   frame_time;
extern struct timeval   loop_time;

extern double          isr_step;
extern double          isr_counter;

u32                   *pixels;

void sys_audio_callback(void *userdata, u8 *stream, s32 len);

// ============================================================================
#pragma mark - SDL2 System init
// ============================================================================

void sys_init(sPlatform *pl, int fullscreen, int mutesound) {

    width = pl->width;
    height = pl->height;
    
    scale_x = scale;
    scale_y = scale * aspect_ratio;
    
    printf("  SDL initialization...\n");
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0) {
        fprintf(stderr, "   Unable to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }
    
    timer_id = SDL_AddTimer(20, itroutine, NULL);
    if (!timer_id) {
        fprintf(stderr, "   Could not create timer: %s\n", SDL_GetError());
        exit(-1);
    }
    
    printf("  Video initialization...\n");
    window = SDL_CreateWindow(kProgName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * scale_x, height * scale_y, SDL_WINDOW_RESIZABLE | fullscreen * SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (window == NULL) {
        fprintf(stderr, "   Could not create %.0fx%.0f window: %s\n", (width * scale_x), (height * scale_y), SDL_GetError());
        exit(-1);
    }
    
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        fprintf(stderr, "   Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        exit(-1);
    }
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderSetScale(renderer, scale, scale);
    SDL_RenderSetLogicalSize(renderer, width, height * aspect_ratio);
    
    pixels = malloc(width * height * sizeof(*pixels));
    memset(pixels, 0, width * height * sizeof(*pixels));
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == NULL) {
        fprintf(stderr, "   Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        exit(-1);
    }
    
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    
    cursor = NULL;
    dirty_mouse = 0;
    
    printf("  Audio initialization...\n");
    audio_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    
    SDL_AudioSpec *desired_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    
    SDL_zero(*desired_spec);
    desired_spec->freq = 44100;
    desired_spec->format = AUDIO_S16;
    desired_spec->channels = 1;
    desired_spec->samples = desired_spec->freq / 50; // 20 ms
    desired_spec->callback = sys_audio_callback;
    desired_spec->userdata = NULL;
    desired_spec->size = desired_spec->samples * 2;
    
    if ((audio_id = SDL_OpenAudioDevice(NULL, 0, desired_spec, audio_spec, 0)) <= 0 ) {
        fprintf(stderr, "   Could not open audio: %s\n", SDL_GetError());
        exit(-1);
    }
    
    // Extended information on obtained audio for tests and reports
    printf("  Opened audio device id %d successfully:\n", audio_id);
    printf("    Frequency:  %d\n", audio_spec->freq);
    printf("    Format:     0x%04x => %d bits per sample\n", audio_spec->format, (int) SDL_AUDIO_BITSIZE(audio_spec->format));
    printf("    Channels:   %d\n", audio_spec->channels);
    printf("    Samples:    %d\n", audio_spec->samples);
    printf("    Silence:    %d\n", audio_spec->silence);
    printf("    Padding:    %d\n", audio_spec->padding);
    printf("    Size:       %d\n", audio_spec->size);
    
    free(desired_spec);
    
    audio.host_freq = audio_spec->freq;
    audio.host_format = audio_spec->format;

    sys_init_psg();
    
    isr_step = (double)50.0 / (double)(audio_spec->freq);
    isr_counter = 1;
    
    SDL_PauseAudioDevice(audio_id, mutesound);
    
    render_sem = SDL_CreateSemaphore(1);
    if (render_sem == NULL) {
        fprintf(stderr, "   Could not create semaphore: %s\n", SDL_GetError());
        exit(-1);
    };
    
    poll_ticks = SDL_GetTicks();
    gettimeofday(&frame_time, NULL);
    gettimeofday(&loop_time, NULL);
}

void sys_init_timers(void) {
    
    poll_ticks = SDL_GetTicks();
    gettimeofday(&frame_time, NULL);
    gettimeofday(&loop_time, NULL);
}

void sys_sleep_until(struct timeval *start, s32 len)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    
    s64 wait = len - ((s64)(now.tv_sec * 1000000LL + now.tv_usec) - (s64)(start->tv_sec * 1000000LL + start->tv_usec));
    if (wait > 0)
        usleep((u32)wait);
    
    gettimeofday(&now, NULL);
    *start = now;
}

void sys_sleep_interactive(s32 *loop, s32 intr)
{
    if (intr > 0)
    {
        *loop *= 1000;
        intr *= 1000;
        
        struct timeval now, prev;
        gettimeofday(&prev, NULL);
        
        while (*loop > intr || (*loop > 0 && io_inkey() == 0))
        {
            usleep(k_event_ticks);
            
            gettimeofday(&now, NULL);
            *loop -= ((now.tv_sec * 1000000LL) + now.tv_usec) - ((prev.tv_sec * 1000000LL) + prev.tv_usec);
            prev = now;
        }
        
        *loop = 0;
    }
}

void sys_delay_loop(void)
{
    sys_sleep_until(&loop_time, 4000);
}

void sys_delay_frame(void)
{
    sys_sleep_until(&frame_time, k_frame_ticks * alis.ctiming);
}

void sys_sleep_until_music_stops(void)
{
    if (SDL_GetAudioDeviceStatus(audio_id) != SDL_AUDIO_PLAYING)
        return;
    
    audio.working = 1;

    for (int count = 0; count < 100 && audio.muflag; count++)
        usleep(10000);

    audio.muflag = 0;
    audio.musicId = 0xffff;

    for (int count = 0; count < 100 && audio.working; count++)
        usleep(10000);
}

// ============================================================================
#pragma mark - SDL2 Main system loop
// ============================================================================

u8 sys_start(void) {

    if (failure) return -1;

    alis.state = eAlisStateRunning;

    SDL_Thread *thread = SDL_CreateThread(alis_thread, "ALIS", (void *)NULL);
    if (thread == NULL) {
        printf("Could not create thread! %s\n", SDL_GetError());
        return 1;
    }
    
    while (alis.state && !failure) {

        // wait so image is drawn and keyboard is polled at 120 fps
        usleep(k_event_ticks);
        sys_poll_event();
    }

    signals_info(failure);
    if (!failure) {
        ALIS_DEBUG(EDebugInfo, "Waiting for the VM kernel thread to finish...\n");
        SDL_WaitThread(thread, NULL);
    }
    else {
        ALIS_DEBUG(EDebugSystem, "Ignoring the crashed VM kernel thread...\n");
    }

    return 0;
}

void sys_poll_event(void) {
    
    sys_render(host.pixelbuf);
    
    SDL_PollEvent(&event);

    // update mouse
    u32 bt = SDL_GetMouseState(&mouse.x, &mouse.y);
    
    float newx, newy;
    SDL_RenderWindowToLogical(renderer, mouse.x, mouse.y, &newx, &newy);
    
    mouse.x = newx;
    mouse.y = newy / aspect_ratio;
    mouse.lb = SDL_BUTTON(bt) == SDL_BUTTON_LEFT;
    mouse.rb = SDL_BUTTON(bt) == SDL_BUTTON_RIGHT;
    
    switch (event.type) {
            
        case SDL_QUIT:
        {
            printf("\n");
            ALIS_DEBUG(EDebugSystem, "SDL2: Quit.\n");
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
                    
                default:
                    break;
            }

            break;
        }
        case SDL_KEYDOWN:
        {
            if (event.key.keysym.mod == KMOD_RSHIFT || event.key.keysym.mod == KMOD_LSHIFT)
            {
                shift = 1;
            }

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
        case SDL_WINDOWEVENT:
        {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                SDL_RenderGetScale(renderer, &scale_x, &scale_y);
                scale_y *= aspect_ratio;
                
                dirty_mouse = true;
            }
            
            break;
        }
    };
}

// ============================================================================
#pragma mark - SDL2 RENDER
// ============================================================================

void sys_render(pixelbuf_t buffer) {
    
    SDL_SemWait(render_sem);
    
    if (dirty_mouse)
    {
        dirty_mouse = false;
        sys_dirty_mouse();
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
                        r = buffer.palette[(index << 2) + 0];
                        g = buffer.palette[(index << 2) + 1];
                        b = buffer.palette[(index << 2) + 2];
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

                pixels[px] = (u32)(0xff000000 + (r << 16) + (g << 8) + (b << 0));
            }
        }
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

        u32 palidx, iat, rot, mask;
        u8 *rawcolor;
        
        for (int y = 0; y < buffer.h; y++, palette += 16)
        {
            int lpx = 0;
            
            for (int x = 0; x < buffer.w; x+=16, at+=8)
            {
                for (int i = 0; i < 2; i++)
                {
                    iat = at + i;
                    for (int dpx = 0; dpx < 8; dpx++, lpx++, px++)
                    {
                        if (lpx >= limit0)
                        {
                            if (lpx == limit1)
                                palette += 16;
                            
                            palidx = lpx < limit1 ? (lpx - limit0) / pxs : (lpx - limit1) / pxs;
                            if (palidx < 16)
                                curpal[palidx] = palette[palidx];
                        }
                        
                        rot = (7 - dpx);
                        mask = 1 << rot;
                        
                        rawcolor = (u8 *)&(curpal[(((bitmap[iat + 0] & mask) >> rot) << 0) | (((bitmap[iat + 2] & mask) >> rot) << 1) | (((bitmap[iat + 4] & mask) >> rot) << 2) | (((bitmap[iat + 6] & mask) >> rot) << 3)]);
                        pixels[px] = (u32)(0xff000000 + (((rawcolor[0] & 0b00000111) << 5) << 16) + (((rawcolor[1] >> 4) << 5) << 8) + (((rawcolor[1] & 0b00000111) << 5) << 0));
                    }
                }
            }
            
            palette += 16;
            memcpy(curpal, palette, 32);
        }
    }
    else
    {
        int index;
        for (int px = 0; px < buffer.w * buffer.h; px++)
        {
            index = buffer.data[px];
            pixels[px] = (u32)(0xff000000 + (buffer.palette[index * 4 + 0] << 16) + (buffer.palette[index * 4 + 1] << 8) + (buffer.palette[index * 4 + 2] << 0));
        }
    }

    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(*pixels));
    
    // render
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
    SDL_SemPost(render_sem);
}

// ============================================================================
#pragma mark - SDL2 System deinit
// ============================================================================

void sys_deinit(void) {
    
    SDL_RemoveTimer(timer_id);
    SDL_PauseAudioDevice(audio_id, 1);
    SDL_CloseAudioDevice(audio_id);
    SDL_DestroySemaphore(render_sem);
    
    SDL_Delay(20);   // 20ms fail-safe delay to make sure that sound buffer is empty

    sys_deinit_psg();

    free(audio_spec);

//   SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// =============================================================================
#pragma mark - SDL2 SYNC
// =============================================================================

void sys_lock_renderer(void) {
    
    SDL_SemWait(render_sem);
}

void sys_unlock_renderer(void) {
    
    SDL_SemPost(render_sem);
}

// =============================================================================
#pragma mark - SDL2 MOUSE
// =============================================================================

void sys_dirty_mouse(void) {
    
    if (alis.desmouse == NULL)
    {
        SDL_ShowCursor(mouse.enabled);
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

                    u16 paloff = palentry[2];
                    palette = host.pixelbuf.palette + palentry[2] * 4;
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
                    pixels[px] = color == clear ? 0 : (u32)(0xff000000 + (palette[color * 4 + 0] << 16) + (palette[color * 4 + 1] << 8) + (palette[color * 4 + 2] << 0));
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
                    pixels[px] = color == clear ? 0 : (u32)(0xff000000 + (host.pixelbuf.palette[index * 4 + 0] << 16) + (host.pixelbuf.palette[index * 4 + 1] << 8) + (host.pixelbuf.palette[index * 4 + 2] << 0));
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
                pixels[px] = color == clear ? 0 : (u32)(0xff000000 + (host.pixelbuf.palette[color * 4 + 0] << 16) + (host.pixelbuf.palette[color * 4 + 1] << 8) + (host.pixelbuf.palette[color * 4 + 2] << 0));
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
    
    SDL_Surface *scaledSurface = SDL_CreateRGBSurface(0, width * scale_x, height * scale_y, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
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
    
    cursor = SDL_CreateColorCursor(surface, 0, 0);
    
    SDL_FreeSurface(surface);
    
    SDL_SetCursor(cursor);
    SDL_ShowCursor(mouse.enabled);

    free(pixels);
}

#endif
