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

#include "image.h"

void export_script(sAlisScriptData * script);

void save_grayscale_bmp(const u8 *buffer, s32 width, s32 height, const char *filepath);
void save_palette(u8 *resourcedata, const char *filepath);

void export_graphics(u32 addr, const char *name);

void convert_mac_mono(u8 *tgt, u8 *bmpdata, s16 width, s16 height);
void convert_cga(u8 *tgt, u8 *bmpdata, s16 width, s16 height);
void convert_4bit_st(u8 *tgt, u8 *bmpdata, s16 width, s16 height);
void convert_4bit(u8 *tgt, u8 *bmpdata, s16 width, s16 height);
void convert_5bit_ami(u8 *tgt, u8 *bmpdata, s16 width, s16 height);


void save_audio_wav(const u8 *buffer, u32 length, u32 sampleRate, const char *filepath);

void export_audio(u32 addr, const char *name);
