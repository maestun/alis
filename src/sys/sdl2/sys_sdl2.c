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

#include <SDL2/SDL.h>

#include "../sys.h"
#include "alis.h"
#include "channel.h"
#include "image.h"
#include "utils.h"

u8 joystick0 = 0;
u8 joystick1 = 0;
u8 shift = 0;
SDL_Keysym button = { 0, 0, 0, 0 };

mouse_t _mouse;

SDL_Renderer *  _renderer;
SDL_Window *    _window;
SDL_Event       _event;
Uint32 *        _pixels;
SDL_Texture *   _texture;
float           _scale = 2;
float           _aspect_ratio = 1.2;
float           _scaleX;
float           _scaleY;
u32             _width = 320;
u32             _height = 200;

SDL_AudioSpec *_audio_spec;

extern u8       flinepal;
extern s16      firstpal[64];

void sys_audio_callback(void *userdata, Uint8 *stream, int len);

void sys_init(void) {
    _scaleX = _scale;
    _scaleY = _scale * _aspect_ratio;
    
    SDL_Init(SDL_INIT_VIDEO);
    _window = SDL_CreateWindow(kProgName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width * _scaleX, _height * _scaleY, SDL_WINDOW_RESIZABLE);
    _renderer = SDL_CreateRenderer(_window, -1, 0);
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
    SDL_RenderSetScale(_renderer, _scale, _scale);
    
    _pixels = malloc(_width * _height * sizeof(*_pixels));
    memset(_pixels, 0, _width * _height * sizeof(*_pixels));
    
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _width, _height);
    SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
    
    SDL_AudioSpec *desired_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));

    SDL_zero(*desired_spec);
    desired_spec->freq = 44100;
    desired_spec->format = AUDIO_U16;
    desired_spec->channels = 1;
    desired_spec->samples = 2646; // 60ms
    desired_spec->callback = sys_audio_callback;
    desired_spec->userdata = NULL;
    
    _audio_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    if (SDL_OpenAudio(desired_spec, _audio_spec) < 0 )
    {
      printf("Couldn't open audio: %s ", SDL_GetError());
      exit(-1);
    }

    free(desired_spec);

    SDL_PauseAudio(0);
}


u8 sys_poll_event(void) {
    u8 running = 1;
    SDL_PollEvent(&_event);

    // update mouse
    u32 bt = SDL_GetMouseState(&_mouse.x, &_mouse.y);
    _mouse.x /= _scaleX;
    _mouse.y /= _scaleY;
    _mouse.lb = SDL_BUTTON(bt) == SDL_BUTTON_LEFT;
    _mouse.rb = SDL_BUTTON(bt) == SDL_BUTTON_RIGHT;
    
    if (_mouse.lb)
    {
        debug(EDebugInfo, "\nx: %d, y: %d \n", _mouse.x, _mouse.y);
    }
    
    switch (_event.type) {
        case SDL_QUIT:
        {
            running = 0;
            break;
        }
        case SDL_KEYDOWN:
        {
            if (_event.key.keysym.mod == KMOD_RSHIFT || _event.key.keysym.mod == KMOD_LSHIFT)
            {
                shift = 1;
            }

            button = _event.key.keysym;
            break;
        }
		case SDL_WINDOWEVENT:
		{
			if (_event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				_scaleX = (float)_event.window.data1 / _width;
				_scaleY = (float)_event.window.data2 / _height;
				
				//SDL_RenderSetScale(_renderer, _scaleX, _scaleY);
			}
		}
    };

    return running;
}


void sys_render(pixelbuf_t buffer) {
    
    if (flinepal == 0)
    {
        for (int px = 0; px < buffer.w * buffer.h; px++)
        {
            int index = buffer.data[px];
            _pixels[px] = (u32)(0xff000000 + (buffer.palette[index * 3 + 0] << 16) + (buffer.palette[index * 3 + 1] << 8) + (buffer.palette[index * 3 + 2] << 0));
        }
    }
    else
    {
        s16 *palentry = firstpal;

        for (int i = 0; i < 3; i++)
        {
            s16 unk1 = palentry[0];
            if (unk1 == 0xff)
                break;

            s16 y1 = palentry[1];
            u8 *palette = *(u8 **)&(palentry[2]);
            
            palentry += 2 + (sizeof(u8 *) >> 1);
            s16 y2 = palentry[0] == 0xff ? 200 : min(palentry[1], 200);

            for (int y = y1; y < y2; y++)
            {
                s32 px = y * buffer.w;
                for (int x = 0; x < buffer.w; x++, px++)
                {
                    int index = buffer.data[px];
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
    
    // TODO: yield for 60 fps ?
}



void sys_deinit(void) {
 //   SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
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

u16 audiobuff[4096];
void sys_audio_callback(void *userdata, Uint8 *s, int length)
{
    // TODO: create chanels structure holding info about sounds to be played
    // type: (smaple, sound, noise, ...)
    // start: (playback start time, use it to calculate what to copy in to the stream)
    // frequency: we will have to do some interpolation to play at correct speed
    // ...
    
    memset(s, 0, length);
    
    u16 *strm = (u16 *)s;
    length /= 2;
    
    for (int i = 0; i < 4; i++)
    {
        sChannel *ch = &channels[i];
        
        switch (ch->type)
        {
            case eChannelTypeSample:
            {
                float ratio = (float)(ch->freq) / (float)(_audio_spec->freq);
                float volratio = ((float)(ch->volume) / 128.0) / 4;
                
                float smpidxf = 0;
                float x0, x1, x2, x3, t, r;
                
                int smpidx = 0;
                int played = 0;
                for (played = 0; played < length; played++)
                {
                    // interpolation

                    smpidxf = (ch->played + played) * ratio;
                    smpidx = (int)smpidxf;
                    if (smpidx >= ch->length)
                    {
                        if (ch->loop > 1)
                        {
                            length -= played;
                            ch->loop--;
                            ch->played = smpidx = played = 0;
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

                    x0 = (i0 >= 0) ? (s16)((ch->address[i0] | ch->address[i0] << 8) - 0x8000) : 0;
                    x1 = (i1 >= 0) ? (s16)((ch->address[i1] | ch->address[i1] << 8) - 0x8000) : 0;
                    x2 = (i2 >= 0) ? (s16)((ch->address[i2] | ch->address[i2] << 8) - 0x8000) : 0;
                    x3 = (i3 >= 0) ? (s16)((ch->address[i3] | ch->address[i3] << 8) - 0x8000) : 0;

                    t = smpidxf - (int)smpidxf;

                    // r = interpolate_hermite(x0, x1, x2, x3, t);
                    // r = interpolate_hermite_4pt_3ox(x0, x1, x2, x3, t);
                    r = interpolate_cubic(x0, x1, x2, x3, t);

                    s32 s0 = r * volratio;
                    s32 s1 = strm[played] - 0x8000;
                    strm[played] = (s0 + s1) + 0x8000;
                    
                    // no interpolation

//                  smpidx = (ch->played + played) * ratio;
//                  if (smpidx >= ch->length)
//                  {
//                      if (ch->loop > 1)
//                      {
//                          length -= played;
//                          ch->loop--;
//                          ch->played = smpidx = played = 0;
//                      }
//                      else
//                      {
//                          ch->type = eChannelTypeNone;
//                          break;
//                      }
//                  }
//
//                  s32 s0 = ((ch->address[smpidx] | ch->address[smpidx] << 8) - 0x8000) * volratio;
//                  s32 s1 = strm[played] - 0x8000;
//                  strm[played] = (s0 + s1) + 0x8000;
                }
                
                ch->played += played;
            }
                break;
                
            case eChannelTypeDing:
            {
//                double period = 1.0 / ch->freq;
//                double samplesPerCycle = (double)(_audio_spec->freq) * period;
//                double samplesPerWave = (ch->length / 1000.0) * (double)(_audio_spec->freq);
//                double amplitude = ((double)(ch->volume)) / 1.0;
//
//                for (size_t i = 0; i < length; i++)
//                {
//                    double wavePosition = (ch->played + i) / samplesPerCycle;
//                    double sample = wavePosition - floor(wavePosition);
//                    sample = (sample < 0.5) ? amplitude : -amplitude;
//
//                    strm[i] += (u16)(0xFF | (u16)(sample + 0x80) << 8);
//
//                    ch->played++;
//                    if (ch->played >= samplesPerWave)
//                    {
//                        ch->type = eChannelTypeNone;
//                        break;
//                    }
//                }
            }
                break;
                
            default:
                break;
        }
    }
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
}


u8 io_inkey(void)
{
    SDL_PumpEvents();
    
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (!currentKeyStates[button.scancode])
    {
        button.scancode = 0;
        button.sym = 0;
    }
    
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
    // wait for input
    
    u8 result = io_inkey();
    while (result == 0)
    {
        result = io_inkey();
    }
    
    // wait for user to release key
    
    u8 dummy;

    do
    {
        dummy = io_inkey();
    }
    while (dummy != 0);
    
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

char sys_get_key(void) {
    sys_poll_event();
    char result = io_inkey();
    if ((char)result != 0)
    {
        while (io_inkey() != 0)
        {
            sys_poll_event();
            sys_render(host.pixelbuf);
            usleep(100);
        }

        return result;
    }
    
    sys_render(host.pixelbuf);
    usleep(100);
    return sys_get_key();
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
        // append???
    }

    debug(EDebugInfo, " [%s][%.3x] ", path, mode);
    return fopen(strlower(path), flag);
}

//s16 FUN_OpenFile(char *path, short mode)
//{
//    char *_addr_file_name = path;
//    u16 w_file_open_mode = mode;
//    if ((mode & 0x100U) == 0 || sys_fexists(path))
//    {
//        if ((mode & 0x200) == 0)
//        {
//            u16 l_file_handle = SYS_FileOpen();
//            if (-1 < (int)l_file_handle)
//            {
//                if ((w_file_open_mode & 0x400) == 0)
//                {
//                    return w_file_open_mode;
//                }
//
//                SYS_SeekFile();
//                if (l_file_handle >= 0)
//                {
//                    return uVar1;
//                }
//            }
//
//            return SYS_PrintError();
//        }
//
//        if (sys_fexists(path))
//        {
//            return FUN_FileTruncate();
//        }
//    }
//
//    return FUN_CreateFile();
//}

int sys_fclose(FILE * fp) {
    return fclose(fp);
}

u8 sys_fexists(char * path) {
    u8 ret = 0;
    FILE * fp = sys_fopen(path, 0);
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
    // TODO: make it configureable?
    debug(EDebugInfo, "SIMULATED: %s", __FUNCTION__);
    // 0x456 = Atari STe / 1MB / Lowrez
    // 0x3f2 = Atari ST / 1MB / Lowrez
    
    // on PC, there are only 4 correct values (mono, cga, ega, vga?)
    // 0x7d0 + 0, 0x7d0 + 1, 0x7d0 + 2, 0x7d0 + 4
    
    // TODO: find all values
    switch (alis.platform.kind) {
            
        case EPlatformAmiga:        return 0xbb8;
//        case EPlatformAmigaAGA:     // ???
        case EPlatformAtari:        return 0x456;
        case EPlatformFalcon:       return 0x1388;
//        case EPlatformMac:          // ???
        case EPlatformPC:           return 0x7d4; // (I2 7d0 - 7d4, I3 0x7ee - 7f2)
        default:                    return 0x456;
    };
}


u16 sys_random(void) {
    return rand() & 0xffff;
}


