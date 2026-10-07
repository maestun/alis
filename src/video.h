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

#include <stdio.h>   // FILE — streaming-film handle

typedef enum {
    
    eAlisVideoNone        = 0,
    eAlisVideoS512        = 1,
    eAlisVideoHAM6        = 2,
    eAlisVideoFLIC        = 3,
#if ALIS_SDL_VER == 1
    eAlisVideoCleanup     = 10,
#endif
} eAlisVideo;

typedef struct {
    s16 id;
    u8  playing;
    u8 *addr1;
    u8 *addr2;
    u8 *endptr;
    u8 *delptr;
    s16 frame;
    s16 frames;
    u16 width;
    u16 height;
    s16 result;
    s16 batchframes;
    s16 waitclock;
    u32 basemain;
    eAlisVideo type;
    u8  ham_pal[16][3];     // raw RGB palette for HAM6 (bypasses pal_format system)

    // Film streaming — large films that don't fit in RAM (see fli_stream_* in
    // video.c). NULL sfp = whole-film-in-RAM mode (all fields below unused).
    FILE *sfp;         // open film file; owned by the streamer, closed by fli_stream_close
    u8   *sbuf;        // sliding-window base (also stored in delptr, freed by cdelfilm)
    u8   *sfill;       // end of valid (read) data in the window
    u32   sbuf_size;   // window capacity
    u32   sremain;     // file bytes not yet read into the window
    u32   sfile_size;  // total film file size (rewind)
} sFLICData;

void inifilm(void);
void runfilm(void);
void endfilm(void);

// Film streaming (video.c). fli_stream_open takes ownership of fp and puts the
// film into sliding-window mode (bfilm.sbuf = window, primed lazily by
// flitofen); fli_stream_close closes the file and clears the streaming state —
// it does NOT free the window (bfilm.delptr owns it; cdelfilm frees).
int  fli_stream_open(FILE *fp, u32 file_size);
void fli_stream_close(void);

extern sFLICData bfilm;

// S512/HAM dirty line tracking: one byte per scanline (0=clean, bit 0/1 = left/right half).
// Reset and set by fls_decomp().
extern u8 fls_dirty_lines[200];   // current frame
extern u8 fls_prev_dirty[200];    // previous frame dirty (double-buffer compensation)
extern u8 fls_dirty_x1[200];     // min dirty byte offset per line (0-159)
extern u8 fls_dirty_x2[200];     // max dirty byte offset per line (0-159)
extern u8 fls_prev_x1[200];      // previous frame X ranges
extern u8 fls_prev_x2[200];

// Film composition buffer: allocated by inifilm, released by the backend after the last frame.
#define kVgaLogicSize 128400
extern u8 *pvgalogic, *vgalogic, *vgalogic_df;
int  vgalogic_alloc(void);
void vgalogic_free(void);
