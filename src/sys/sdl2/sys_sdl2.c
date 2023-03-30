//
//  sys_sdl2.c
//  alis
//

#include "../sys.h"
#include "experimental.h"
#include <SDL2/SDL.h>

u8 _button_count = 0;
u16 _buttons[16];

mouse_t _mouse;

SDL_Renderer *  _renderer;
SDL_Window *    _window;
SDL_Event       _event;
Uint32 *        _pixels;
SDL_Texture *   _texture;

int width = 320;
int height = 200;

void sys_init() {
    SDL_Init(SDL_INIT_VIDEO);
    _window = SDL_CreateWindow(kProgName,
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               640, 480, 0);
    _renderer = SDL_CreateRenderer(_window, -1, 0);
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
    SDL_RenderSetScale(_renderer, 2, 2);
    
    _pixels = malloc(width * height * sizeof(*_pixels));
    memset(_pixels, 0, width * height * sizeof(*_pixels));
    
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
}


u8 sys_poll_event() {
    u8 running = 1;
    SDL_PollEvent(&_event);
    
    // update mouse
    u32 bt = SDL_GetMouseState(&_mouse.x, &_mouse.y);
    _mouse.lb = SDL_BUTTON(bt) == SDL_BUTTON_LEFT;
    _mouse.rb = SDL_BUTTON(bt) == SDL_BUTTON_RIGHT;
    
    switch (_event.type) {
        case SDL_QUIT:
        {
            running = 0;
            break;
        }
        case SDL_KEYDOWN:
        {
            _buttons[_button_count] = _event.key.keysym.scancode;
            _button_count = (_button_count + 1) % 16;
            break;
        }
        case SDL_KEYUP:
        {
            u8 found = 0;
            u8 count = _button_count;
            for (s32 i = 0; i < count; i++)
            {
                if (found)
                {
                    _buttons[i - 1] = _buttons[i];
                }
                else if (_buttons[_button_count] == _event.key.keysym.scancode)
                {
                    _button_count = (_button_count - 1 < 0) ? 15 : _button_count - 1;
                    found = 1;
                }
            }
            break;
        }
    };
    
    return running;
}


void sys_render(pixelbuf_t buffer) {
    
    for (int px = 0; px < buffer.w * buffer.h; px++)
    {
        int index = buffer.data[px];
        _pixels[px] = (u32)(0xff000000 + (buffer.palette[index * 3 + 0] << 16) + (buffer.palette[index * 3 + 1] << 8) + (buffer.palette[index * 3 + 2] << 0));
    }
    
    SDL_UpdateTexture(_texture, NULL, _pixels, width * sizeof(*_pixels));

    // render
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, NULL, NULL);
    SDL_RenderPresent(_renderer);
    
    // TODO: yield for 60 fps ?
}



void sys_deinit() {
 //   SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}


// =============================================================================
#pragma mark - I/O
// =============================================================================
mouse_t sys_get_mouse() {
    return _mouse;
}

void sys_set_mouse(u16 x, u16 y) {
    _mouse.x = x;
    _mouse.y = y;
}

void sys_enable_mouse(u8 enable) {
    _mouse.enabled = enable;
}



// =============================================================================
#pragma mark - FILE SYSTEM
// =============================================================================
FILE * sys_fopen(char * path) {
    return fopen(path, "r");
}

int sys_fclose(FILE * fp) {
    return fclose(fp);
}

u8 sys_fexists(char * path) {
    u8 ret = 0;
    FILE * fp = sys_fopen(path);
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
    debug(EDebugWarning, "/* %s SIMULATED */", __FUNCTION__);
    return 0x456; // Atari STe / 1MB / Lowrez
}


u16 sys_random(void) {
    return rand() & 0xffff;
}


