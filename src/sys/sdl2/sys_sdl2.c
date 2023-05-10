//
//  sys_sdl2.c
//  alis
//

#include "../sys.h"
#include "experimental.h"
#include <SDL2/SDL.h>

u8 joystick0 = 0;
u8 joystick1 = 0;
u8 shift = 0;
u16 button = 0;

mouse_t _mouse;

SDL_Renderer *  _renderer;
SDL_Window *    _window;
SDL_Event       _event;
Uint32 *        _pixels;
SDL_Texture *   _texture;

int width = 320;
int height = 200;

u8 io_inkey(void)
{
    return button;
}

u8 io_shiftkey(void)
{
    return shift;
}

u8 io_getkey(void)
{
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

u8 io_joy(u8 port)
{
    return port ? joystick0 : joystick1;
}

u8 io_joykey(u8 test)
{
    u8 result = 0;
    
    if (test == 0)
    {
        result = (joystick1 & 0x80) != 0;
        if ((shift & 4) != 0)
            result = result | 2;

        if ((shift & 8) != 0)
            result = result | 4;

        if (button == -0x1f)
            result = result | 0x80;
    }
    
    return result;
}

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
            if (_event.key.keysym.mod == KMOD_RSHIFT || _event.key.keysym.mod == KMOD_LSHIFT)
            {
                shift = 1;
            }
            else
            {
                button = _event.key.keysym.sym;
            }
            break;
        }
        case SDL_KEYUP:
        {
            if (_event.key.keysym.mod == KMOD_RSHIFT || _event.key.keysym.mod == KMOD_LSHIFT)
            {
                shift = 0;
            }
            else if (button == _event.key.keysym.sym)
            {
                button = 0;
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
FILE * sys_fopen(char * path, u16 mode) {
    // TODO: mode...
    printf(" (%s) ", path);
    return fopen(path, "r");
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
    debug(EDebugWarning, "/* %s SIMULATED */", __FUNCTION__);
    return 0x456; // Atari STe / 1MB / Lowrez
}


u16 sys_random(void) {
    return rand() & 0xffff;
}


