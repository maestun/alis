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
#include "audio.h"
#include "channel.h"
#include "image.h"
#include "utils.h"

#include "emu2149.h"


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
float           _scale = 2;
float           _aspect_ratio = 1.2;
float           _scaleX;
float           _scaleY;
u32             _width = 320;
u32             _height = 200;

SDL_AudioSpec   *_audio_spec;
PSG             *_psg;

extern u8       fls_ham6;
extern u8       fls_s512;
extern u8       *vgalogic_df;

extern u8       flinepal;
extern s16      firstpal[64];

double isr_step;
double isr_counter;

u8 giaccess(s8 cmd, u8 data, u8 ch);
bool priorblanc(sChannel *channel);
u8 pblanc10(sChannel *a0, u8 d1b);

void sys_audio_callback(void *userdata, Uint8 *stream, int len);

void sys_init(void) {
    _scaleX = _scale;
    _scaleY = _scale * _aspect_ratio;
    
    SDL_Init(SDL_INIT_VIDEO);
    _timerID = SDL_AddTimer(20, itroutine, NULL);
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
    desired_spec->samples = desired_spec->freq / 50; // 20 ms
    desired_spec->callback = sys_audio_callback;
    desired_spec->userdata = NULL;
    
    _audio_spec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    if (SDL_OpenAudio(desired_spec, _audio_spec) < 0 )
    {
        printf("Couldn't open audio: %s ", SDL_GetError());
        exit(-1);
    }

    free(desired_spec);
    
    _psg = PSG_new(3579545, _audio_spec->freq);
    PSG_setClockDivider(_psg, 1);
    PSG_setVolumeMode(_psg, 1); // YM style
    PSG_setQuality(_psg, 1);
    PSG_reset(_psg);

    isr_step = (double)50.0 / (double)(_audio_spec->freq);
    isr_counter = 1;

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
        SDL_RenderClear(_renderer);
        SDL_RenderCopy(_renderer, _texture, NULL, NULL);
        SDL_RenderPresent(_renderer);
        return;
    }

    if (fls_s512)
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
        return;
    }

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
    
    SDL_PauseAudio(1);

    free(_audio_spec);

    SDL_RemoveTimer(_timerID);
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

static u32 samplelength = 0;
static u8 *samplebuffer[1024 * 1024 * 4];

bool priorblanc(sChannel *channel)
{
    u8 result = 1;
    if (channel->curson < channels[0].curson)
        result = pblanc10(&(channels[0]), result);

    if (channel->curson < channels[1].curson)
        result = pblanc10(&(channels[1]), result);

    if (channel->curson < channels[2].curson)
        result = pblanc10(&(channels[2]), result);
    
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
                sChannel *ch = &channels[i];
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
            
        case EPlatformAtari:        return 1110;
        case EPlatformPC:           return 2000;
        case EPlatformAmiga:        return 3000;
        case EPlatformMac:          return 4000;
        case EPlatformFalcon:       return 5000;
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


