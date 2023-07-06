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

#include "config.h"
#include "platform.h"

long        get_file_size(char * file_name);
char *      strlower(char * str);
char *      strupper(char * str);
u32         reverse_bytes_32(s32 value);
// s16         extend_w(s8 value);
// s32         extend_l(s16 value);
//u32         read_big_endian(u8 * data, size_t sz);
// u16         fread16_old(FILE * fp, sPlatform pl);
// u32         fread32_old(FILE * fp, sPlatform pl);
u16         read16(const u8 *ptr);
u32         read24(const u8 *ptr);
u32         read32(const u8 *ptr);
// u16         swap16_old(u16 num, sPlatform pl);
// u32         swap32_old(u32 num, sPlatform pl);

u16         fread16(FILE * fp);
u32         fread32(FILE * fp);
u16         swap16(const u8 *);
// u32         swap24(const u8 *, u8 is_le);
u32         swap32(const u8 *);

int         is_host_le(void);
