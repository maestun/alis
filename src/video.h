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

typedef enum {
    
    eAlisVideoNone        = 0,
    eAlisVideoS512        = 1,
    eAlisVideoHAM6        = 2,
    eAlisVideoFLIC        = 3,
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
    s16 result;
    s16 batchframes;
    s16 waitclock;
    u32 basemain;
    eAlisVideo type;
} sFLICData;

void inifilm(void);
void runfilm(void);
void endfilm(void);

extern sFLICData bfilm;
