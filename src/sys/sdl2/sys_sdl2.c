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
float           _scale = 3; // TODO: as cli parameter
float           _aspect_ratio = 1.2;
float           _scaleX;
float           _scaleY;
u32             _width = 320;
u32             _height = 200;

extern u8       flinepal;
extern s16      firstpal[64];

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
            else
            {
                button = _event.key.keysym;
            }
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
        s16 *palentry = firstpal + 2 + (sizeof(u8 *) >> 1);
        s16 line = palentry[1];

        for (int y = 0; y < line; y++)
        {
            s32 px = y * buffer.w;
            for (int x = 0; x < buffer.w; x++, px++)
            {
                int index = buffer.data[px];
                _pixels[px] = (u32)(0xff000000 + (buffer.palette[index * 3 + 0] << 16) + (buffer.palette[index * 3 + 1] << 8) + (buffer.palette[index * 3 + 2] << 0));
            }
        }
        
        u8 *palette = *(u8 **)&(firstpal[2]);

        for (int y = line; y < buffer.h; y++)
        {
            s32 px = y * buffer.w;
            for (int x = 0; x < buffer.w; x++, px++)
            {
                int index = buffer.data[px];
                _pixels[px] = (u32)(0xff000000 + (palette[index * 3 + 0] << 16) + (palette[index * 3 + 1] << 8) + (palette[index * 3 + 2] << 0));
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
    SDL_CloseAudio();
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
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

    return button.sym;
}

u8 io_shiftkey(void) {
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
    if (shift && (!currentKeyStates[KMOD_RSHIFT] || !currentKeyStates[KMOD_LSHIFT]))
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
#pragma mark - AUDIO
// =============================================================================
typedef struct {
    s8 *    data;
    size_t  len;
    size_t  pos;
    u8      loop;
    double  volume;
    double  speed;
} sSampleContext;

typedef struct {
    double  frequency;
    double  duration;
    double  volume;
    size_t  sampleRate;
    size_t  position;
    u8      finished;
} sWaveContext;
static sWaveContext _wave_sample_ctx;

void play_sample_cb(void* userdata, Uint8* stream, int len) {
    sSampleContext * ctx = (sSampleContext *)userdata;

    size_t remaining_data = ctx->len - ctx->pos;
    size_t bytes_to_copy = SDL_min(len, remaining_data);

    for(int i = 0; i < bytes_to_copy; i++) {
        stream[i] = (Uint8)(ctx->data[ctx->pos + i] * ctx->volume);
    }
    ctx->pos += (size_t)(bytes_to_copy * ctx->speed);

    if (ctx->pos >= ctx->len) {
        
        if(ctx->loop) {
            ctx->pos = 0;
        }
        else {
            free(ctx);
            SDL_CloseAudio();
        }
    }
}

void sys_play_sample(s8* pcm_data, u32 pcm_len, u8 pcm_vol, u8 pcm_speed, u16 loop) {
    
    // Set up the audio context
    sSampleContext * ctx = (sSampleContext *)malloc(sizeof(sSampleContext));
    ctx->data = pcm_data;
    ctx->len = pcm_len;
    ctx->pos = 0;
    ctx->volume = 1; // max volume == 0x7f ?
    ctx->speed = 1 + (pcm_speed / 100.0);
    ctx->loop = (loop > 1);

    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 4100;
    spec.format = AUDIO_S8;
    spec.channels = 1;
    spec.samples = 512;
    spec.callback = play_sample_cb;
    spec.userdata = ctx;

    SDL_OpenAudio(&spec, &spec);
    SDL_PauseAudio(0);
}


void sineWaveCallback(void* userdata, Uint8* stream, int len)
{
    sWaveContext* waveContext = (sWaveContext*)userdata;
    double angularFrequency = 2.0 * M_PI * waveContext->frequency;
    double samplesPerCycle = waveContext->sampleRate / waveContext->frequency;
    double samplesPerWave = waveContext->duration * waveContext->sampleRate;
    double amplitude = waveContext->volume * 127.0;

    for (size_t i = 0; i < len; i++) {
        double wavePosition = (waveContext->position + i) / samplesPerCycle;
        double sample = sin(angularFrequency * wavePosition) * amplitude;

        stream[i] = (Uint8)(sample + 128);

        waveContext->position++;
        if (waveContext->position >= samplesPerWave) {
            waveContext->finished = 1;
            break;
        }
    }
}

void squareWaveCallback(void* userdata, Uint8* stream, int len)
{
    sWaveContext* waveContext = (sWaveContext*)userdata;

    double period = 1.0 / waveContext->frequency;
    double samplesPerCycle = waveContext->sampleRate * period;
    double samplesPerWave = waveContext->duration * waveContext->sampleRate;
    double amplitude = waveContext->volume * 127.0;

    for (size_t i = 0; i < len; i++) {
        double wavePosition = (waveContext->position + i) / samplesPerCycle;
        double sample = wavePosition - floor(wavePosition);
        sample = (sample < 0.5) ? amplitude : -amplitude;

        stream[i] = (Uint8)(sample + 128);

        waveContext->position++;
        if (waveContext->position >= samplesPerWave) {
            waveContext->finished = 1;
            break;
        }
    }
}

void sys_play_wave(u16 freq, u16 len, u8 vol, u8 is_square) {
    sWaveContext waveContext;
    waveContext.frequency = freq;
    waveContext.duration = len / 1000.0;
    waveContext.volume = vol;
    waveContext.sampleRate = 44100;
    waveContext.position = 0;
    waveContext.finished = 0;

    SDL_AudioSpec desiredSpec, obtainedSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_S8;
    desiredSpec.channels = 1;
    desiredSpec.samples = 4096;
    desiredSpec.callback = is_square ? squareWaveCallback : sineWaveCallback;
    desiredSpec.userdata = &waveContext;

    SDL_OpenAudio(&desiredSpec, &obtainedSpec);
    SDL_PauseAudio(0);
    while (!waveContext.finished) {
        SDL_Delay(100);
    }
    SDL_CloseAudio();
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
    
    return 0x456;
}


u16 sys_random(void) {
    return rand() & 0xffff;
}


