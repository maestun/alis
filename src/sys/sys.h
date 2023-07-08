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

#pragma once

#include "../config.h"
#include "../debug.h"


// =============================================================================
#pragma mark - LIFECYCLE
// =============================================================================
typedef void (*vmStep)(void);
void    sys_main(vmStep fStep);
void    sys_init(void);
u8      sys_poll_event(void);
void    sys_deinit(void);


// =============================================================================
#pragma mark - I/O
// =============================================================================
typedef struct {
    int x, y;
    u8 lb, rb;
    u8 enabled;
} mouse_t;
mouse_t sys_get_mouse(void);
void    sys_set_mouse(u16 x, u16 y);
void    sys_enable_mouse(u8 enable);

u8      io_inkey(void);
u8      io_shiftkey(void);
u8      io_joy(u8 port);
u8      io_joykey(u8 test);
char    sys_get_key(void);

// =============================================================================
#pragma mark - GFX
// =============================================================================
typedef struct {
    u16     w, h;
    u8      scale;
    u8 *    data;
    u8 *    palette;
} pixelbuf_t;
void    sys_render(pixelbuf_t buffer);


// =============================================================================
#pragma mark - FILE SYSTEM
// =============================================================================
int     sys_fclose(FILE * fp);
FILE *  sys_fopen(char * path, u16 mode);
u8      sys_fexists(char * path);


// =============================================================================
#pragma mark - AUDIO
// =============================================================================
void sys_play_sample(s8* pcm_data, u32 pcm_len, u8 pcm_vol, u8 pcm_speed, u16 loop);
void sys_play_wave(u16 freq, u16 len, u8 vol, u8 is_square);


// =============================================================================
#pragma mark - MISC
// =============================================================================
void    sys_set_time(u16 h, u16 m, u16 s);
time_t  sys_get_time(void);
u16     sys_get_model(void);
u16     sys_random(void);
