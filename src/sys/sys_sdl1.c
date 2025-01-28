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

#include "emu2149.h"
#include "math.h"


const u32               k_event_ticks = (1000 / 120);  // poll keyboard 120 times per second
const u32               k_frame_ticks = (1000 / 50);   // 50 fps screen update

SDL_Surface             *surface;

extern SDL_keysym       button;
extern SDL_Event        event;
extern SDL_AudioSpec    *audio_spec;
extern SDL_Rect         dirty_rects[2048];
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

extern double           isr_step;
extern double           isr_counter;

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
u8                      mouse_bg_pixels[16 * 16];

u8                      dirty_pal = 1;
s32                     dirty_len = 0;
u32                     tmp_pal[256];

void sys_audio_callback(void *userdata, u8 *stream, s32 len);

// ============================================================================
#pragma mark - SDL1 System init
// ============================================================================

void sys_init(sPlatform *pl, int fullscreen, int mutesound) {

    dirty_mouse_rect = (SDL_Rect){0, 0, 16, 16};
    
    width = pl->width;
    height = pl->height;
    
    printf("  SDL initialization...\n");
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "   Unable to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }
    
    printf("  Video initialization...\n");
    
    SDL_WM_SetCaption(kProgName, "icon");
    surface = SDL_SetVideoMode(width, height, 0, SDL_SWSURFACE);
    if (surface == NULL) {
        fprintf(stderr, "   Could not create %.0dx%.0d window: %s\n", (width), (height), SDL_GetError());
        exit(-1);
    }
    
    prev_pixels = surface->pixels;

    dirty_mouse = 0;
    
    printf("  Audio initialization...\n");
    audio_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    
    SDL_AudioSpec *desired_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    
    memset(desired_spec, 0, sizeof(SDL_AudioSpec));
    desired_spec->freq = 12500;
    desired_spec->format = AUDIO_S16MSB;
    desired_spec->channels = 1;
    desired_spec->samples = desired_spec->freq / 20; // 20 ms
    desired_spec->callback = sys_audio_callback;
    desired_spec->userdata = NULL;
    desired_spec->size = desired_spec->samples;
    
    memcpy(audio_spec, desired_spec, sizeof(SDL_AudioSpec));
    
//    if (SDL_OpenAudio(desired_spec, audio_spec) != 0) {
    if (SDL_OpenAudio(desired_spec, 0) != 0) {
        fprintf(stderr, "   Could not open audio: %s\n", SDL_GetError());
        free(audio_spec);
        surface->pixels = prev_pixels;
        SDL_FreeSurface(surface);
        SDL_Quit();
        exit(-1);
    }
    
    // Extended information on obtained audio for tests and reports
    printf("  Opened audio device id %d successfully:\n", audio_id);
    printf("    Frequency:  %d\n", audio_spec->freq);
    printf("    Format:     0x%04x\n", audio_spec->format);
    printf("    Channels:   %d\n", audio_spec->channels);
    printf("    Samples:    %d\n", audio_spec->samples);
    printf("    Silence:    %d\n", audio_spec->silence);
    printf("    Padding:    %d\n", audio_spec->padding);
    printf("    Size:       %d\n", audio_spec->size);
    
    free(desired_spec);

    audio.host_freq = audio_spec->freq;
    audio.host_format = audio_spec->format;

//    sys_init_psg();
    
    isr_step = (double)50.0 / (double)(audio_spec->freq);
    isr_counter = 1;
    
    poll_ticks = SDL_GetTicks();
    frame_time = SDL_GetTicks();
    loop_time = SDL_GetTicks();

    SDL_PauseAudio(mutesound);
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
    if (wait > 0 && wait < 100)
        SDL_Delay(wait);
    *start = now;
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
    sys_sleep_until(&frame_time, k_frame_ticks * alis.ctiming);
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

    sys_sleep_until(&poll_ticks, k_frame_ticks);
    
    itroutine(20, NULL);
    
    SDL_PollEvent(&event);
    switch (event.type) {
            
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
    };
    
    // update mouse
    
    if (dirty_mouse_rect.x < host.pixelbuf.w && dirty_mouse_rect.y < host.pixelbuf.h)
    {
        dirty_rects[dirty_len] = dirty_mouse_rect;
        dirty_len++;
    }

    u32 bt = SDL_GetMouseState(&mouse.x, &mouse.y);
    
    if (mouse.x >= host.pixelbuf.w)
        mouse.x = host.pixelbuf.w;
    
    if (mouse.y >= host.pixelbuf.h)
        mouse.y = host.pixelbuf.h;

    mouse.lb = SDL_BUTTON(bt) == SDL_BUTTON_LEFT;
    mouse.rb = SDL_BUTTON(bt) == SDL_BUTTON_RIGHT;

    if (mouse.x < host.pixelbuf.w && mouse.y < host.pixelbuf.h)
    {
        s16 width = mouse.x + 16 > host.pixelbuf.w ? host.pixelbuf.w - mouse.x : 16;
        s16 height = mouse.y + 16 > host.pixelbuf.h ? host.pixelbuf.h - mouse.y : 16;

        dirty_rects[dirty_len] = dirty_mouse_rect = (SDL_Rect){ .x = mouse.x, .y = mouse.y, .w = width, .h = height };
        dirty_len++;
    }
    
    sys_render(host.pixelbuf);
}

// ============================================================================
#pragma mark - SDL1 Video
// ============================================================================

void setHAMto32bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u32 *)pixels)[px] = (u32)(0xff000000 + (rawcolor[0] << 16) + (rawcolor[1] << 8) + (rawcolor[2]));
}

void setHAMto16bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u16 *)pixels)[px] = (u16)((rawcolor[0] >> 3) << 11) + ((rawcolor[1] >> 2) << 5) + (rawcolor[2] >> 3);
}

void setHAMto8bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u8 *)pixels)[px] = (u8)(((rawcolor[0] >> 6) << 6) + ((rawcolor[1] >> 5) << 3) + (((rawcolor[2] >> 5))));
}

void set9to32bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u32 *)pixels)[px] = (u32)(0xff000000 + (((rawcolor[0] & 0b00000111) << 5) << 16) + (((rawcolor[1] >> 4) << 5) << 8) + (((rawcolor[1] & 0b00000111) << 5)));
}

void set9to16bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u16 *)pixels)[px] = (u16)((((rawcolor[0] & 0b00000111) << 2) << 11) + (((rawcolor[1] >> 4) << 3) << 5) + (((rawcolor[1] & 0b00000111) << 2)));
}

void set9to8bit(void *pixels, u32 px, u8 *rawcolor) {
    ((u8 *)pixels)[px] = (u8)((((rawcolor[0] & 0b00000111) >> 1) << 6) + ((rawcolor[1] >> 4) << 3) + (((rawcolor[1] & 0b00000111))));
}

void sys_render(pixelbuf_t buffer) {

    if (dirty_len == 0 && dirty_pal == 0)
        return;
    
    if (dirty_mouse)
    {
        dirty_mouse = false;
        sys_dirty_mouse();
    }

    if (bfilm.type == eAlisVideoHAM6)
    {
        u8 *bitmap = vgalogic_df + 0xa0;
        u32 *pixels = surface->pixels;

        // Amiga HAM bitplanes

        SDL_Color sdl_palette[256];

        void (*to_pixels)(void *, u32, u8 *) = &setHAMto32bit;

        switch (surface->format->BitsPerPixel)
        {
            case 32:
            {
                to_pixels = &setHAMto32bit;
                break;
            }
            case 16:
            {
                to_pixels = &setHAMto16bit;
                break;
            }
            case 8:
            {
                for (int i = 0; i < 256; i++)
                {
                    sdl_palette[i].r = (i & 0b11000000);
                    sdl_palette[i].g = (i & 0b00111000) << 2;
                    sdl_palette[i].b = (i & 0b00000111) << 5;
                }

                SDL_SetPalette(surface, SDL_PHYSPAL, sdl_palette, 0, 256);

                to_pixels = &setHAMto8bit;
                break;
            }
        };
        
        u32 px = 0;
        u32 planesize = (buffer.w * buffer.h) >> 3;

        s32 index;
        s32 control;

        u8 rawcolor[3];

        u8 *c0 = bitmap;
        u8 *c1 = c0 + planesize;
        u8 *c2 = c1 + planesize;
        u8 *c3 = c2 + planesize;
        u8 *c4 = c3 + planesize;
        u8 *c5 = c4 + planesize;

        for (s32 y = 0; y < buffer.h; y++)
        {
            for (s32 x = 0; x < buffer.w; x+=8, c0++, c1++, c2++, c3++, c4++, c5++)
            {
                for (s32 bit = 7; bit >= 0; bit--, px++)
                {
                    index = (((*c0 >> bit) & 1) | ((*c1 >> bit) & 1) << 1 | ((*c2 >> bit) & 1) << 2 | ((*c3 >> bit) & 1) << 3) << 2;
                    control = ((*c4 >> bit) & 1) << 0 | ((*c5 >> bit) & 1) << 1;
                    switch (control) {
                        case 0:
                            rawcolor[0] = buffer.palette[index + 0];
                            rawcolor[1] = buffer.palette[index + 1];
                            rawcolor[2] = buffer.palette[index + 2];
                            break;
                        case 1:
                            rawcolor[2] = index << 2;
                            break;
                        case 2:
                            rawcolor[0] = index << 2;
                            break;
                        case 3:
                            rawcolor[1] = index << 2;
                            break;
                    }

                    to_pixels(pixels, px, rawcolor);
                }
            }
        }

        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 1;
    }
    else if (bfilm.type == eAlisVideoS512)
    {
        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 1;

        SDL_Color sdl_palette[256];

        void (*to_pixels)(void *, u32, u8 *) = &set9to32bit;

        switch (surface->format->BitsPerPixel)
        {
            case 32:
            {
                to_pixels = &set9to32bit;
                break;
            }
            case 16:
            {
                to_pixels = &set9to16bit;
                break;
            }
            case 8:
            {
                for (int i = 0; i < 256; i++)
                {
                    sdl_palette[i].r = (i & 0b11000000);
                    sdl_palette[i].g = (i & 0b00111000) << 2;
                    sdl_palette[i].b = (i & 0b00000111) << 5;
                }

                SDL_SetPalette(surface, SDL_PHYSPAL, sdl_palette, 0, 256);

                to_pixels = &set9to8bit;
                break;
            }
        };
        
        u8 *bitmap = vgalogic_df + 0xa0;
        u8 *pixels = surface->pixels;

        // Atari ST Spectrum 512 bitplanes
        
        u16 curpal[32];
        memcpy(curpal, vgalogic_df + 32000, 32);
        
        u16 *palette = (u16 *)(vgalogic_df + 32000 + 32);
        
        int palcntr = 0;
        
        u32 px = 0;
        u32 at = 0;
        
        s32 palidx, mask;
        u8 *rawcolor;
        
        for (int y = 0; y < buffer.h; y++, palette += 16)
        {
            palcntr = 408;
            
            for (int x = 0; x < 10; x++, at+=8)
            {
                for (int i = at; i < at + 2; i++)
                {
                    for (int rot = 7; rot >= 0; rot--, palcntr+=102, px++)
                    {
                        palidx = palcntr >> 10;
                        curpal[palidx] = palette[palidx];
                        mask = 1 << rot;
                        rawcolor = (u8 *)&(curpal[((bitmap[i + 0] & mask) >> rot) | (((bitmap[i + 2] & mask) >> rot) << 1) | (((bitmap[i + 4] & mask) >> rot) << 2) | (((bitmap[i + 6] & mask) >> rot) << 3)]);
                        to_pixels(pixels, px, rawcolor);
                    }
                }
            }
            
            palcntr = 510;
            palette += 16;
            
            for (int x = 10; x < 20; x++, at+=8)
            {
                for (int i = at; i < at + 2; i++)
                {
                    for (int rot = 7; rot >= 0; rot--, palcntr+=102, px++)
                    {
                        palidx = palcntr >> 10;
                        curpal[palidx] = palette[palidx];
                        mask = 1 << rot;
                        rawcolor = (u8 *)&(curpal[((bitmap[i + 0] & mask) >> rot) | (((bitmap[i + 2] & mask) >> rot) << 1) | (((bitmap[i + 4] & mask) >> rot) << 2) | (((bitmap[i + 6] & mask) >> rot) << 3)]);
                        to_pixels(pixels, px, rawcolor);
                    }
                }
            }
            
            palette += 16;
            memcpy(curpal, palette, 32);
        }

        dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
        dirty_len = 1;
    }
    else
    {
        if (bfilm.type == eAlisVideoFLIC)
        {
            memcpy(buffer.data, vgalogic, host.pixelbuf.w * host.pixelbuf.h);
            dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
            dirty_len = 1;
            dirty_pal = 1;
        }
        
        if (mouse.enabled && dirty_mouse_rect.x < host.pixelbuf.w && dirty_mouse_rect.y < host.pixelbuf.h)
        {
            s16 width = dirty_mouse_rect.x + 16 > host.pixelbuf.w ? host.pixelbuf.w - dirty_mouse_rect.x : 16;
            s16 height = dirty_mouse_rect.y + 16 > host.pixelbuf.h ? host.pixelbuf.h - dirty_mouse_rect.y : 16;

            u8 *bgr = mouse_bg_pixels;
            u8 *src = mouse_pixels;
            u8 *tgt = buffer.data + dirty_mouse_rect.x + dirty_mouse_rect.y * buffer.w;
            for (int y = 0; y < height; y++, src += 16, bgr += width, tgt += buffer.w)
            {
                memcpy(bgr, tgt, width);
                
                for (int x = 0; x < width; x++)
                {
                    if (src[x])
                        tgt[x] = src[x];
                }
            }
        }
        
        if (dirty_pal)
        {
            switch (surface->format->BitsPerPixel)
            {
                case 32:
                {
                    for (int i = 0; i < 1024; i+=4)
                        tmp_pal[i>>2] = (u32)(0xff000000 + (buffer.palette[i + 0] << 16) + (buffer.palette[i + 1] << 8) + (buffer.palette[i + 2] << 0));

                    dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                    dirty_len = 1;
                    break;
                }
                case 16:
                {
                    u16 *tmppal16 = (u16 *)tmp_pal;
                    for (int i = 0; i < 1024; i+=4)
                        tmppal16[i>>2] = (u16)(((buffer.palette[i + 0] >> 3) << 11) + ((buffer.palette[i + 1] >> 2) << 5) + ((buffer.palette[i + 2] >> 3) << 0));

                    dirty_rects[0] = (SDL_Rect){ .x = 0, .y = 0, .w = host.pixelbuf.w, .h = host.pixelbuf.h };
                    dirty_len = 1;
                    break;
                }
                case 8:
                {
                    SDL_SetPalette(surface, SDL_PHYSPAL, (SDL_Color *)buffer.palette, 0, 256);
                    break;
                }
            };
            
            dirty_pal = 0;
        }
        
        if (surface->format->BitsPerPixel == 8)
        {
            surface->pixels = buffer.data;
        }
        else
        {
            for (int i = 0; i < dirty_len; i++)
            {
                SDL_Rect r = dirty_rects[i];
                switch (surface->format->BitsPerPixel)
                {
                    case 32:
                    {
                        u32 *pixels = surface->pixels;
                        
                        int px = r.x + r.y * buffer.w;
                        int po = buffer.w - r.w;
                        for (int y = r.y; y < r.y + r.h; y++, px += po)
                        {
                            for (int x = 0; x < r.w; x++, px++)
                            {
                                pixels[px] = tmp_pal[buffer.data[px]];
                            }
                        }
                        
                        break;
                    }
                    case 16:
                    {
                        u16 *tmppal16 = (u16 *)tmp_pal;
                        u16 *pixels = surface->pixels;

                        int px = r.x + r.y * buffer.w;
                        int po = buffer.w - r.w;
                        for (int y = r.y; y < r.y + r.h; y++, px += po)
                        {
                            for (int x = 0; x < r.w; x++, px++)
                            {
                                pixels[px] = tmppal16[buffer.data[px]];
                            }
                        }
                        break;
                    }
                };
            }
        }
    }
    
    SDL_UpdateRects(surface, abs(dirty_len), dirty_rects);
    dirty_len = 0;
    
    if (mouse.enabled && dirty_mouse_rect.x < host.pixelbuf.w && dirty_mouse_rect.y < host.pixelbuf.h)
    {
        s16 width = dirty_mouse_rect.x + 16 > host.pixelbuf.w ? host.pixelbuf.w - dirty_mouse_rect.x : 16;
        s16 height = dirty_mouse_rect.y + 16 > host.pixelbuf.h ? host.pixelbuf.h - dirty_mouse_rect.y : 16;

        u8 *bgr = mouse_bg_pixels;
        u8 *tgt = buffer.data + dirty_mouse_rect.x + dirty_mouse_rect.y * buffer.w;
        for (int y = 0; y < height; y++, bgr += width, tgt += buffer.w)
        {
            memcpy(tgt, bgr, width);
        }
    }
}

// ============================================================================
#pragma mark - SDL1 System deinit
// ============================================================================

void sys_deinit(void) {
    
    SDL_Delay(20);   // 20ms fail-safe delay to make sure that sound buffer is empty

    // NOTE: make sure cursor is visible when quiting
    SDL_ShowCursor(1);

    sys_deinit_psg();

    free(audio_spec);
    
    surface->pixels = prev_pixels;
    SDL_FreeSurface(surface);

    SDL_Quit();
}

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
                    int index = palidx + color;
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
