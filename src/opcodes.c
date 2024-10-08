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

#include "alis.h"
#include "audio.h"
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "screen.h"
#include "unpack.h"
#include "utils.h"
#include "video.h"

#define BIT_SCAN        (0)
#define BIT_INTER       (1)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// ============================================================================
#pragma mark - Experimental
// ============================================================================

int putdataidx = 0;

s16 putdata[][4] = {
    { 0x0000, 0x0000, 0xfffb, 0x0a },
    { 0x00a0, 0x0003, 0x002b, 0x10 },
    { 0x0000, 0x0000, 0x0000, 0x05 },
    { 0x0000, 0x0000, 0xfffb, 0x09 },
    { 0x0050, 0x0002, 0x0034, 0x0b },
    { 0x0052, 0x0002, 0x0028, 0x0b },
    { 0x0054, 0x0002, 0x0020, 0x0b },
    { 0x0000, 0x0000, 0xfffb, 0x0a },
    { 0x00be, 0x0003, 0x002b, 0x10 },
    { 0x0046, 0x0002, 0x0028, 0x13 },
    { 0x00c3, 0x0003, 0x002b, 0x10 },
    { 0x004b, 0x0002, 0x0028, 0x12 },
    { 0x00c8, 0x0003, 0x002b, 0x10 },
    { 0x0050, 0x0002, 0x0028, 0x13 },
    { 0x00cd, 0x0003, 0x002b, 0x10 },
    { 0x0055, 0x0002, 0x0028, 0x11 },
    { 0x00d2, 0x0003, 0x002b, 0x10 },
    { 0x005a, 0x0002, 0x0028, 0x12 },
    { 0x00d7, 0x0003, 0x002b, 0x10 },
    { 0x005f, 0x0002, 0x0028, 0x13 },
    { 0x00dc, 0x0003, 0x002b, 0x10 },
    { 0x0064, 0x0002, 0x0028, 0x14 },
    { 0x00e1, 0x0003, 0x002b, 0x10 },
    { 0x0069, 0x0002, 0x0028, 0x11 },
    { 0x00e6, 0x0003, 0x002b, 0x10 },
    { 0x006e, 0x0002, 0x0028, 0x13 },
    { 0x00eb, 0x0003, 0x002b, 0x10 },
    { 0x0073, 0x0002, 0x0028, 0x14 },
    { 0x00f0, 0x0003, 0x002b, 0x10 },
    { 0x0078, 0x0002, 0x0028, 0x13 },
    { 0x00f5, 0x0003, 0x002b, 0x10 },
    { 0x007d, 0x0002, 0x0028, 0x14 },
    { 0x00fa, 0x0003, 0x002b, 0x10 },
    { 0x0082, 0x0002, 0x0028, 0x13 },
    { 0x00ff, 0x0003, 0x002b, 0x10 },
    { 0x0087, 0x0002, 0x0028, 0x11 },
    { 0x0104, 0x0003, 0x002b, 0x10 },
    { 0x008c, 0x0002, 0x0028, 0x13 },
    { 0x0109, 0x0003, 0x002b, 0x10 },
    { 0x0091, 0x0002, 0x0028, 0x12 },
    { 0x010e, 0x0003, 0x002b, 0x10 },
    { 0x0096, 0x0002, 0x0028, 0x11 },
    { 0x0113, 0x0003, 0x002b, 0x10 },
    { 0x009b, 0x0002, 0x0028, 0x14 },
    { 0x0118, 0x0003, 0x002b, 0x10 },
    { 0x00a0, 0x0002, 0x0028, 0x11 },
    { 0x011d, 0x0003, 0x002b, 0x10 },
    { 0x00a5, 0x0002, 0x0028, 0x12 },
    { 0x0122, 0x0003, 0x002b, 0x10 },
    { 0x00aa, 0x0002, 0x0028, 0x13 },
    { 0x0127, 0x0003, 0x002b, 0x10 },
    { 0x00af, 0x0002, 0x0028, 0x11 },
    { 0x012c, 0x0003, 0x002b, 0x10 },
    { 0x00b4, 0x0002, 0x0028, 0x12 },
    { 0x0131, 0x0003, 0x002b, 0x10 },
    { 0x00b9, 0x0002, 0x0028, 0x14 },
    { 0x0136, 0x0003, 0x002b, 0x10 },
    { 0x00be, 0x0002, 0x0028, 0x13 },
    { 0x013b, 0x0003, 0x002b, 0x10 },
    { 0x00c3, 0x0002, 0x0028, 0x12 },
    { 0x0140, 0x0003, 0x002b, 0x10 },
    { 0x00c8, 0x0002, 0x0028, 0x11 },
    { 0x0145, 0x0003, 0x002b, 0x10 },
    { 0x00cd, 0x0002, 0x0028, 0x13 },
    { 0x014a, 0x0003, 0x002b, 0x10 },
    { 0x00d2, 0x0002, 0x0028, 0x12 },
    { 0x014f, 0x0003, 0x002b, 0x10 },
    { 0x00d7, 0x0002, 0x0028, 0x11 },
    { 0x0154, 0x0003, 0x002b, 0x10 },
    { 0x00dc, 0x0002, 0x0028, 0x13 },
    { 0x0159, 0x0003, 0x002b, 0x10 },
    { 0x00e1, 0x0002, 0x0028, 0x12 },
    { 0x015e, 0x0003, 0x002b, 0x10 },
    { 0x00e6, 0x0002, 0x0028, 0x13 },
    { 0x0163, 0x0003, 0x002b, 0x10 },
    { 0x00eb, 0x0002, 0x0028, 0x12 },
    { 0x0168, 0x0003, 0x002b, 0x10 },
    { 0x00f0, 0x0002, 0x0028, 0x13 },
    { 0x016d, 0x0003, 0x002b, 0x10 },
    { 0x00f5, 0x0002, 0x0028, 0x11 },
    { 0x0172, 0x0003, 0x002b, 0x10 },
    { 0x00fa, 0x0002, 0x0028, 0x13 },
    { 0x0177, 0x0003, 0x002b, 0x10 },
    { 0x00ff, 0x0002, 0x0028, 0x14 },
    { 0x017c, 0x0003, 0x002b, 0x10 },
    { 0x0104, 0x0002, 0x0028, 0x11 },
    { 0x0181, 0x0003, 0x002b, 0x10 },
    { 0x0109, 0x0002, 0x0028, 0x12 },
    { 0x0186, 0x0003, 0x002b, 0x10 },
    { 0x010e, 0x0002, 0x0028, 0x14 },
    { 0x018b, 0x0003, 0x002b, 0x10 },
    { 0x0113, 0x0002, 0x0028, 0x13 },
    { 0x0190, 0x0003, 0x002b, 0x10 },
    { 0x0118, 0x0002, 0x0028, 0x11 },
    { 0x0195, 0x0003, 0x002b, 0x10 },
    { 0x011d, 0x0002, 0x0028, 0x14 },
    { 0x019a, 0x0003, 0x002b, 0x10 },
    { 0x0122, 0x0002, 0x0028, 0x11 },
    { 0x019f, 0x0003, 0x002b, 0x10 },
    { 0x0127, 0x0002, 0x0028, 0x13 },
    { 0x01a4, 0x0003, 0x002b, 0x10 },
    { 0x012c, 0x0002, 0x0028, 0x11 },
    { 0x01a9, 0x0003, 0x002b, 0x10 },
    { 0x0131, 0x0002, 0x0028, 0x14 },
    { 0x01ae, 0x0003, 0x002b, 0x10 },
    { 0x0136, 0x0002, 0x0028, 0x13 },
    { 0x01b3, 0x0003, 0x002b, 0x10 },
    { 0x013b, 0x0002, 0x0028, 0x11 },
    { 0x01b8, 0x0003, 0x002b, 0x10 },
    { 0x0140, 0x0002, 0x0028, 0x14 },
    { 0x01bd, 0x0003, 0x002b, 0x10 },
    { 0x0145, 0x0002, 0x0028, 0x12 },
    { 0x01c2, 0x0003, 0x002b, 0x10 },
    { 0x014a, 0x0002, 0x0028, 0x11 },
    { 0x01c7, 0x0003, 0x002b, 0x10 },
    { 0x014f, 0x0002, 0x0028, 0x14 },
    { 0x01cc, 0x0003, 0x002b, 0x10 },
    { 0x0154, 0x0002, 0x0028, 0x11 },
    { 0x01d1, 0x0003, 0x002b, 0x10 },
    { 0x0159, 0x0002, 0x0028, 0x14 },
    { 0x01d6, 0x0003, 0x002b, 0x10 },
    { 0x015e, 0x0002, 0x0028, 0x12 },
    { 0x01db, 0x0003, 0x002b, 0x10 },
    { 0x0163, 0x0002, 0x0028, 0x14 },
    { 0x0106, 0x0000, 0x003e, 0x02 },
    { 0x0106, 0x0000, 0x003e, 0x03 },
    { 0x0106, 0x0000, 0x003e, 0x04 },
    { 0x0106, 0x0000, 0x003e, 0x03 },
    { 0x0106, 0x0000, 0x003e, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x04 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x04 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x04 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x02 }};

// TODO: Looks like some precomputed values
u8 tabatan[] = {
    0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x32, 0x33, 0x34, 0x35, 0x35, 0x36, 0x36, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3A, 0x3A, 0x3B, 0x3B, 0x3C, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x40,
    0x40, 0x40, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42, 0x43, 0x43, 0x43, 0x43, 0x44, 0x44, 0x44, 0x44, 0x45, 0x45, 0x45, 0x45, 0x46, 0x46, 0x46, 0x46, 0x47, 0x47, 0x47, 0x47, 0x47, 0x48, 0x48, 0x48,
    0x48, 0x48, 0x48, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C,
    0x4C, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F,
    0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
    0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52,
    0x52, 0x52, 0x52, 0x52, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53,
    0x53, 0x53, 0x53, 0x53, 0x53, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54,
    0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x54, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56,
    0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56,
    0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56,
    0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57,
    0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57,
    0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57,
    0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57,
    0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57,
    0x57, 0x57, 0x57, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x00, 0x00, 0x00, 0x00  // TODO four redundant entries?
};

s16 tabsin[] = {
    0x0000, 0x0008, 0x0011, 0x001a, 0x0023, 0x002c, 0x0035, 0x003e, 0x0047, 0x0050, 0x0058, 0x0061, 0x006a, 0x0073, 0x007b, 0x0084, 0x008d, 0x0095, 0x009e, 0x00a6, 0x00af, 0x00b7, 0x00bf,
    0x00c8, 0x00d0, 0x00d8, 0x00e0, 0x00e8, 0x00f0, 0x00f8, 0x0100, 0x0107, 0x010f, 0x0116, 0x011e, 0x0125, 0x012c, 0x0134, 0x013b, 0x0142, 0x0149, 0x014f, 0x0156, 0x015d, 0x0163, 0x016a,
    0x0170, 0x0176, 0x017c, 0x0182, 0x0188, 0x018d, 0x0193, 0x0198, 0x019e, 0x01a3, 0x01a8, 0x01ad, 0x01b2, 0x01b6, 0x01bb, 0x01bf, 0x01c4, 0x01c8, 0x01cc, 0x01d0, 0x01d3, 0x01d7, 0x01da,
    0x01dd, 0x01e1, 0x01e4, 0x01e6, 0x01e9, 0x01ec, 0x01ee, 0x01f0, 0x01f2, 0x01f4, 0x01f6, 0x01f8, 0x01f9, 0x01fb, 0x01fc, 0x01fd, 0x01fe, 0x01fe, 0x01ff, 0x01ff, 0x01ff, 0x0200
};

s16 tabcos[] = {
    0x0200, 0x01ff, 0x01ff, 0x01ff, 0x01fe, 0x01fe, 0x01fd, 0x01fc, 0x01fb, 0x01f9, 0x01f8, 0x01f6, 0x01f4, 0x01f2, 0x01f0, 0x01ee, 0x01ec, 0x01e9, 0x01e6, 0x01e4, 0x01e1, 0x01dd, 0x01da,
    0x01d7, 0x01d3, 0x01d0, 0x01cc, 0x01c8, 0x01c4, 0x01bf, 0x01bb, 0x01b6, 0x01b2, 0x01ad, 0x01a8, 0x01a3, 0x019e, 0x0198, 0x0193, 0x018d, 0x0188, 0x0182, 0x017c, 0x0176, 0x0170, 0x016a,
    0x0163, 0x015d, 0x0156, 0x014f, 0x0149, 0x0142, 0x013b, 0x0134, 0x012c, 0x0125, 0x011e, 0x0116, 0x010f, 0x0107, 0x0100, 0x00f8, 0x00f0, 0x00e8, 0x00e0, 0x00d8, 0x00d0, 0x00c8, 0x00bf,
    0x00b7, 0x00af, 0x00a6, 0x009e, 0x0095, 0x008d, 0x0084, 0x007b, 0x0073, 0x006a, 0x0061, 0x0058, 0x0050, 0x0047, 0x003e, 0x0035, 0x002c, 0x0023, 0x001a, 0x0011, 0x0008, 0x0000
};

u8 vstandard[256];

s16 px1;
s16 py1;
s16 pz1;

s16 px2;
s16 py2;
s16 pz2;

// ============================================================================
#pragma mark - Additions
// ============================================================================

void clivin(void);
void shrinkprog(s32 start, s32 length, u16 script_id);
void killent(u16 entidx);
void sviewtyp(void);
void putval(s16 d7w);

s32 multiform(s32 ent_vram, s32 ent_formedata, s32 formedata, s32 ent_baseform);
s32 monoform(s32 ent_vram, s32 ent_formedata, s32 formedata, s32 ent_baseform, s32 ent_wforme2x);
s32 traitfirm(s32 ent_vram, s32 formedata, s32 ent_baseform, s32 ent_wforme2x);
s32 monofirm(s32 ent_vram, s32 ent_formedata);
s16 multimat(s32 formeaddr, s32 baseform, s16 wforme);
s16 monomat(s32 formeaddr);
u32 traitmat(s32 baseform, s32 wforme);

static void cret(void);

s32 io_malloc(s32 rawsize);
void io_mfree(s32 addr);

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines
// ============================================================================

// Codopname no. 031 opcode 0x1e cstore
static void cstore(void) {
    readexec_opername_saveD7();
    cstore_continue();
}

// Codopname no. 032 opcode 0x1f ceval
static void ceval(void) {
    readexec_opername_saveD7();
}

// Codopname no. 033 opcode 0x20 cadd
static void cadd(void) {
    readexec_opername_saveD7();
    readexec_addname_swap();
}

// Codopname no. 034 opcode 0x21 csub
static void csub(void) {
    readexec_opername_saveD7();
    alis.varD7 *= -1;
    readexec_addname_swap();
}

// Codopname no. 037 opcode 0x24 cvprint
static void cvprint(void) {
    alis.charmode = 0;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

// Codopname no. 038 opcode 0x25 csprinti
static void csprinti(void) {
    alis.charmode = 0;
    for (u8 c = script_read8(); c != 0; c = script_read8())
    {
        put_char(c);
    }
}

// Codopname no. 039 opcode 0x26 csprinta
static void csprinta(void) {
    alis.charmode = 0;
    readexec_opername_saveD7();
    put_string();
}

// Codopname no. 040 opcode 0x27 clocate
static void clocate(void) {
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    s16 xlocate = alis.varD7;
    s16 ylocate = alis.varD6;
//    io_locate();
}

// Codopname no. 041 opcode 0x28 ctab
static void ctab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 042 opcode 0x29 cdim
// copy values to vram
// cdim(u16 offset, u8 counter, u8 value, optional u16[] values)
static void cdim(void) {
    
    s16 offset = script_read16();
    u8 counter = script_read8();
    u8 byte2 = script_read8();
    
    xwrite8(alis.script->vram_org + --offset, counter);
    xwrite8(alis.script->vram_org + --offset, byte2);

    if (alis.platform.version >= 30)
    {
        s16 counter16 = counter & 0xf;
        if (-1 < (s8)counter)
        {
            while (counter16-- > 0)
            {
                offset -= 2;
                xwrite16(alis.script->vram_org + offset, script_read16());
            }
            
            offset -= 4;
            xwrite32(alis.script->vram_org + offset, script_read32());
        }
        else
        {
            while (counter16-- > 0)
            {
                offset -= 4;
                xwrite32(alis.script->vram_org + offset, script_read32());
            }
            
            offset -= 4;
            xwrite32(alis.script->vram_org + offset, script_read32());
        }
    }
    else
    {
        // loop w/ counter, read words, store backwards
        while(counter--) {
            offset -= 2;
            xwrite16(alis.script->vram_org + offset, script_read16());
        }
    }
}

// Codopname no. 043 opcode 0x2a crandom
static void crandom(void) {
    readexec_opername();
    alis.random_number = alis.varD7;
    if(alis.random_number == 0) {
        alis.random_number = sys_random();
        // test key/joy input, if zero, random is set to $64 ???
    }
}

u32 save_loop_pc;

// cloopX(jmp_offset, addname(ram_offset))
// decrement value in RAM[offset2]
// if obtained value is zero, then jump with jmp_offset
static void cloop(s32 offset) {
    
    // NOTE: looks like the only code called is adir* and aloc*
    // we set zero flag using last called instructions there
    
    save_loop_pc = alis.script->pc;
    alis.varD7 = -1;
    readexec_addname_swap();
    if(!alis.sr.zero)
    {
        alis.script->pc = save_loop_pc;

        if(DEBUG_SCRIPT) {
             if (offset<0) {
                 debug(EDebugInfo, " [loop jmp up -0x%06x]", abs(offset));
             }
             else {
                 debug(EDebugInfo, " [loop jmp dn +0x%06x]", offset);
             }
           }

        script_jump(offset);
    }
}

// Codopname no. 044 opcode 0x2b cloop8
static void cloop8(void) {
    cloop((s8)script_read8());
}

// Codopname no. 045 opcode 0x2c cloop16
static void cloop16(void) {
    cloop((s16)script_read16());
}

// Codopname no. 046 opcode 0x2d cloop24
static void cloop24(void) {
    cloop(script_read24());
}

// Codopname no. 047 opcode 0x2e cswitch1
static void cswitch1(void) {
    readexec_opername();

    s16 test = 0;
    s16 addition = script_read8();
    if ((alis.script->pc & 1) != 0)
    {
        alis.script->pc ++;
    }
    
    do
    {
        test = script_read16();
        if (alis.varD7 == test)
        {
            alis.script->pc += script_read16();
            return;
        }
        
        alis.script->pc += 2;
    }
    while (alis.varD7 > test  && (--addition) > -1);
    
    if (addition > 0)
    {
        alis.script->pc += (addition * 4);
    }
}

// Codopname no. 048 opcode 0x2f cswitch2
static void cswitch2(void) {
    readexec_opername();
    
    s16 addition = script_read8();
    if ((alis.script->pc & 1) != 0)
    {
        alis.script->pc ++;
    }

    alis.varD7 += script_read16();
    if (alis.varD7 >= 0 && alis.varD7 <= addition)
    {
        alis.varD7 *= 2;
        alis.script->pc += alis.varD7;
        alis.script->pc += script_read16();
    }
    else
    {
        alis.script->pc += (addition * 2 + 2);
    }
}

// Codopname no. 052 opcode 0x33 cleave
static void cleave(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    if (alis.fseq == 0)
    {
        s16 vacc_offset = get_0x0c_vacc_offset(alis.script->vram_org);
        if (vacc_offset != 0)
        {
            set_0x0c_vacc_offset(alis.script->vram_org, 0);
            set_0x08_script_ret_offset(alis.script->vram_org, xread32(alis.script->vram_org + vacc_offset));
            set_0x0a_vacc_offset(alis.script->vram_org, vacc_offset + 4);
            return;
        }
    }
    else
    {
        alis.script->vacc_off = get_0x0c_vacc_offset(alis.script->vram_org);
        if (alis.script->vacc_off != 0)
        {
            set_0x0c_vacc_offset(alis.script->vram_org, 0);
            alis.script->pc = alis.script->pc_org + xpop32();
            return;
        }
    }
    
    cret();

    debug(EDebugVerbose, " (NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", alis.script->name, alis.script->vram_org, alis.finent, alis.script->vacc_off, alis.script->pc_org);
}

// Codopname no. 053 opcode 0x34 cprotect
static void cprotect(void) {
    readexec_opername();
    alis.vprotect = alis.varD7;
}

// Codopname no. 054 opcode 0x35 casleep
static void casleep(void) {
    readexec_opername();
    
    if (-1 < alis.varD7)
    {
        s32 vram = xread32(alis.atent + alis.varD7);
        if (vram)
        {
            set_0x04_cstart_csleep(vram, 0);
        }
    }
}

// Codopname no. 057 opcode 0x38 cscmov
static void cscmov(void) {
    readexec_opername_saveD7();
    s16 x = alis.varD7;
    readexec_opername_saveD7();
    s16 y = alis.varD7;
    readexec_opername_saveD7();
    s16 z = alis.varD7;
    
    u16 scridx = get_0x16_screen_id(alis.script->vram_org);
    if (alis.platform.version < 30)
    {
        set_scr_unknown0x2e(scridx, z);
        set_scr_unknown0x2c(scridx, y);
        set_scr_unknown0x2a(scridx, x);
        set_scr_state(scridx, get_scr_state(scridx) | 0x80);
    }
    else
    {
        if (z != 0)
        {
            set_scr_depz(scridx, get_scr_depz(scridx) + z);
            set_scr_state(scridx, get_scr_state(scridx) | 0x80);
        }

        if (y != 0)
        {
            set_scr_depy(scridx, get_scr_depy(scridx) + y);
            set_scr_state(scridx, get_scr_state(scridx) | 0x80);
        }

        if (x != 0)
        {
            set_scr_depx(scridx, get_scr_depx(scridx) + x);
            set_scr_state(scridx, get_scr_state(scridx) | 0x80);
        }
    }
}

// Codopname no. 058 opcode 0x39 cscset
static void cscset(void) {
    readexec_opername();
    s16 x = alis.varD7;
    readexec_opername();
    s16 y = alis.varD7;
    readexec_opername();
    s16 z = alis.varD7;
    
    u16 scridx = get_0x16_screen_id(alis.script->vram_org);

    if (alis.platform.version < 30)
    {
        set_scr_unknown0x2e(scridx, z - get_scr_depz(scridx));
        set_scr_unknown0x2c(scridx, y - get_scr_depy(scridx));
        set_scr_unknown0x2a(scridx, x - get_scr_depx(scridx));
        set_scr_state(scridx, get_scr_state(scridx) | 0x80);
    }
    else
    {
        if (z != get_scr_depz(scridx))
        {
            set_scr_depz(scridx, z);
            set_scr_state(scridx, get_scr_state(scridx) | 0x80);
        }

        if (y != get_scr_depy(scridx))
        {
            set_scr_depy(scridx, y);
            set_scr_state(scridx, get_scr_state(scridx) | 0x80);
        }

        if (x != get_scr_depx(scridx))
        {
            set_scr_depx(scridx, x);
            set_scr_state(scridx, get_scr_state(scridx) | 0x80);
        }
    }
}

// Codopname no. 059 opcode 0x3a cclipping
static void cclipping(void) {
    alis.fswitch = 0;
}

// Codopname no. 060 opcode 0x3b cswitching
static void cswitching(void) {
    alis.fswitch = 1;
}

void get_vector(s16 *x, s16 *y, s16 *z)
{
    readexec_opername();
    
    s16 index = alis.varD7;
    s32 frmidx = adresform(index);
    
    u8 test = xread8(frmidx);
    if (test == 0)
    {
        *x = (s16)xread8(frmidx + 4) + ((s16)xread8(frmidx + 7) >> 1);
        *z = (s16)xread8(frmidx + 5) + ((s16)xread8(frmidx + 8) >> 1);
        *y = (s16)xread8(frmidx + 6) + ((s16)xread8(frmidx + 9) >> 1);
    }
    else if ((char)test < 0)
    {
        // SYS_PrintError();
        alis.wcx = 0;
        alis.wcy = 0;
        alis.wcz = 0;
        return;
    }
    else
    {
        if (test == 1)
        {
            *x = xread16(frmidx + 4) + (xread16(frmidx + 0xa) >> 1);
            *z = xread16(frmidx + 6) + (xread16(frmidx + 0xc) >> 1);
            *y = xread16(frmidx + 8) + (xread16(frmidx + 0xe) >> 1);
        }
        else
        {
            *x = xread16(frmidx + 2);
            *z = xread16(frmidx + 4);
            *y = xread16(frmidx + 6);
        }
    }
    
    if (0x14 < get_0x2e_script_header_word_2(alis.script->vram_org) && get_0x03_xinv(alis.script->vram_org) != 0)
    {
        *x = -*x;
    }
}

// Codopname no. 061 opcode 0x3c cwlive
static void cwlive(void) {
    get_vector(&alis.wcx, &alis.wcy, &alis.wcz);
    
    alis.varD7 = -1;
    clivin();
}

// Codopname no. 062 opcode 0x3d cunload
static void cunload(void) {
    
    s16 id = script_read16();
    if (id != -1 && id != get_0x10_script_id(alis.script->vram_org))
    {
        s16 index = debprotf(id);
        if (index != -1)
        // while (index != -1)
        {
            u32 data_org = alis.atprog_ptr[index];
            u32 data_end = 0xfffffff;
            
            for (int i = 0; i < alis.nbprog; i++)
            {
                if (data_org < alis.atprog_ptr[i] && alis.atprog_ptr[i] < data_end)
                {
                    data_end = alis.atprog_ptr[i];
                }
            }
            
            if (data_end == 0xfffffff)
            {
                data_end = alis.finprog;
            }

            sAlisScriptData *script = alis.loaded_scripts[index];
            debug(EDebugVerbose, "\n (NAME: %s, ID: 0x%x ORG: 0x%x (0x%x) SZ: %d (%d)) \n", script->name, script->header.id, script->data_org, data_org, script->sz, data_end - data_org);
            
            // NOTE: interuptable delay to make everything work more like on original platforms
            sleep_interactive(&alis.unload_delay, alis.unload_delay - 100000);

            shrinkprog(data_org, data_end - data_org, id);
        }
    }
}

// Codopname no. 063 opcode 0x3e cwakeup
static void cwakeup(void) {
    readexec_opername();
 
    if (-1 < alis.varD7)
    {
        s32 vram = xread32(alis.atent + alis.varD7);
        if (vram)
        {
            set_0x04_cstart_csleep(vram, 1);
        }
    }
}

// Codopname no. 067 opcode 0x42 cstop
static void cstop(void) {
    // in real program, adds 4 to real stack pointer
    alis.script->running = 0;
    debug(EDebugInfo, "\n-- CSTOP --");
}

// Codopname no. 064 opcode 0x3f csleep
static void csleep(void) {
    set_0x04_cstart_csleep(alis.script->vram_org, 0);
    if (alis.fseq)
    {
        cstop();
    }
}

// Codopname no. 065 opcode 0x40 clive
static void clive(void) {
    alis.wcx = 0;
    alis.wcy = 0;
    alis.wcz = 0;
    
    alis.varD7 = -1;
    clivin();
}

void clivin(void)
{
    s16 id = script_read16();
    if (id != -1)
    {
        s16 index = debprotf(id);
        if (index != -1)
        {
            sAlisScriptData *prog = alis.loaded_scripts[index];
            sAlisScriptLive *script = script_live(prog);
            
            u8 *prev_vram = alis.mem + alis.script->vram_org;
            u8 *next = alis.mem + script->vram_org;

            if (alis.platform.version >= 30)
            {
                *(s32 *)(next + 0x00) = *(s32 *)(prev_vram + 0x00);
                *(s32 *)(next + 0x08) = *(s32 *)(prev_vram + 0x08);
                *(s32 *)(next + 0x10) = *(s32 *)(prev_vram + 0x10);

                *(s16 *)(next + 0x30) = *(s16 *)(prev_vram + 0x30);
                *(s16 *)(next + ALIS_SCR_WCX2) = *(s16 *)(prev_vram + ALIS_SCR_WCX2);
                *(s16 *)(next + ALIS_SCR_WCY2) = *(s16 *)(prev_vram + ALIS_SCR_WCY2);
                *(s16 *)(next + ALIS_SCR_WCZ2) = *(s16 *)(prev_vram + ALIS_SCR_WCZ2);

                *(s32 *)(next + ALIS_SCR_WCAX) = *(s32 *)(prev_vram + ALIS_SCR_WCAX);
                *(s32 *)(next + ALIS_SCR_WCAY) = *(s32 *)(prev_vram + ALIS_SCR_WCAY);
                *(s32 *)(next + ALIS_SCR_WCAZ) = *(s32 *)(prev_vram + ALIS_SCR_WCAZ);

                *(s16 *)(next + 0x40) = *(s16 *)(prev_vram + 0x40);
                *(s16 *)(next + 0x44) = *(s16 *)(prev_vram + 0x44);
                *(s16 *)(next + 0x48) = *(s16 *)(prev_vram + 0x48);
            }
            else
            {
                memcpy(next, prev_vram, 0xb);
            }

            xadd16(script->vram_org + ALIS_SCR_WCX, alis.wcx);
            xadd16(script->vram_org + ALIS_SCR_WCY, alis.wcz);
            xadd16(script->vram_org + ALIS_SCR_WCZ, alis.wcy);

            *(s16 *)(next - 0x28) = *(s16 *)(prev_vram - 0xe);
            *(s16 *)(next - 0x16) = *(s16 *)(prev_vram - 0x16);
            *(s16 *)(next - 0x22) = *(s16 *)(prev_vram - 0x22);
            *(s16 *)(next - 0x2a) = *(s16 *)(next - 0xe);

            // NOTE: just to show already running scripts
            
            debug(EDebugInfo, "\n");

            u16 tent = 0;
            u32 loop = alis.nbent;
            for (int i = 0; i < loop; i++)
            {
                tent = alis.atent_ptr[i].offset;

                sAlisScriptLive *s = alis.live_scripts[i];
                if (s && s->vram_org)
                {
                    s32 contextsize = get_context_size();
                    u32 datasize = contextsize + s->data->header.w_unknown5 + s->data->header.w_unknown7;
                    s32 vramsize = s->data->header.vram_alloc_sz;
                    s32 shrinkby = datasize + vramsize;
                    
                    debug(EDebugInfo, "%c[%s ID: %.2x(%.2x), %.2x, %.6x, %.6x] \n", script->vram_org == s->vram_org ? '*' : ' ', s->name, s->data->header.id, get_0x10_script_id(s->vram_org), xswap16(tent), s->vram_org, shrinkby);
                }
                else
                {
                    debug(EDebugInfo, " [ empty  ID: 00(00), %.2x, 000000, 000000] \n", xswap16(tent));
                    loop++;
                }
            }

            debug(EDebugInfo, " [finent %.6x] \n", alis.finent);

            if (alis.maxent < alis.nbent)
            {
                debug(EDebugFatal, "ERROR: Exceeded number of scripts slots!\n");
                return;
            }
        }
    }
    else
    {
        alis.varD7 = -1;
    }

    cstore_continue();
}

// Codopname no. 066 opcode 0x41 ckill
static void ckill(void) {
    readexec_opername_saveD7();
    if (alis.varD7 < 0)
        return;

     killent(alis.varD7);
}

// Codopname no. 068 opcode 0x43 cstopret
static void cstopret(void) {
    if (alis.fseq == 0)
    {
        cstop();
    }
    
    cret();
    alis.script->running = 0;
}

// Codopname no. 069 opcode 0x44 cexit
static void cexit(void) {
    if (alis.varD5 == 0)
    {
        exit(-1);
        return;
    }

    killent(alis.varD5);
    alis.script->running = 0;
}

// Codopname no. 070 opcode 0x45 cload
static void cload(void) {

    // NOTE: interuptable delay to make everything work more like on original platforms
    sleep_interactive(&alis.load_delay, alis.load_delay - 100000);

    // get script ID
    u16 id = script_read16();
    if(id != 0) {
        // not main script, depack and load into vm
        char path[kPathMaxLen] = {0};
        strcpy(path, alis.platform.path);
        char name[16] = {0};
        script_read_until_zero(name);
        strcpy(strrchr(name, '.') + 1, alis.platform.ext);
        strcat(path, name);
        script_load(strlower((char *)path));
        
        is_delay_script(name);
    }
    else
    {
        // NOTE: load main script and start game loop
        // we are dooing it elsewhere, shouldnt ever be reached;
        debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);

        readexec_opername_swap();
    }
}

// Codopname no. 071 opcode 0x46 cdefsc
// reads 35 bytes
static void cdefsc(void) {
    if (image.libsprit == 0)
        return;

    u16 scridx = script_read16();
    set_scr_state(scridx, 0x40);
    set_scr_numelem(scridx, script_read8());
    set_scr_screen_id(scridx, image.libsprit);
    
    u16 length = alis.platform.version == 10 ? 26 : 32;

    u8 *ptr = alis.mem + alis.basemain + scridx + 6;
    for (int i = 0; i < length; i++, ptr++)
        *ptr = script_read8();
    
    set_scr_to_next(scridx, 0);
    set_scr_unknown0x2a(scridx, 0);
    set_scr_unknown0x2c(scridx, 0);
    set_scr_unknown0x2e(scridx, 0);

    sSprite *sprite = SPRITE_VAR(image.libsprit);
    sprite->link = 0;
    sprite->numelem = get_scr_numelem(scridx);
    
    s16 x = get_scr_newx(scridx);
    s16 y = get_scr_newy(scridx);
    s16 w = get_scr_width(scridx);
    s16 h = get_scr_height(scridx);

    if (alis.platform.kind == EPlatformMac)
    {
        mac_update_pos(&x, &y);
        mac_update_pos(&w, &h);
    }
    
    sprite->newx = x;
    sprite->newy = y;
    sprite->newd = 0x7fff;
    sprite->depx = x + w;
    sprite->depy = y + h;

    image.libsprit = sprite->to_next;

    scadd(scridx);
    vectoriel(scridx);
}

// Codopname no. 072 opcode 0x47 cscreen
static void cscreen(void) {
    u16 screen_id = script_read16();
    if (screen_id != get_0x16_screen_id(alis.script->vram_org)) {
        set_0x16_screen_id(alis.script->vram_org, screen_id);
    }
}

// Codopname no. 073 opcode 0x48 cput
static void cput(void) {
    alis.flagmain = 0;
    image.invert_x = get_0x03_xinv(alis.script->vram_org);

    readexec_opername();
    
    image.depx = 0;
    image.depy = 0;
    image.depz = 0;
    image.numelem = 0;
    
    u16 idx = alis.varD7;

    put(idx);
}

// Codopname no. 074 opcode 0x49 cputnat
static void cputnat(void) {
    alis.flagmain = 0;
    image.invert_x = get_0x03_xinv(alis.script->vram_org);
    readexec_opername_saveD7();
    image.depx = alis.varD7;
    readexec_opername_saveD7();
    image.depy = alis.varD7;
    readexec_opername_saveD7();
    image.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    
    image.numelem = alis.varD6;
    u16 idx = alis.varD7;
    
    put(idx);
}

// Codopname no. 075 opcode 0x4a cerase
static void cerase(void) {
    u16 curidx = 0;
    u16 previdx = 0;

    if (searchtete(&curidx, &previdx))
    {
        s16 screenid = get_0x16_screen_id(alis.script->vram_org);

        do
        {
            killelem(&curidx, &previdx);
            if (curidx == 0)
            {
                alis.ferase = 0;
                return;
            }
        }
        while (screenid == SPRITE_VAR(curidx)->screen_id);
    }
    
    alis.ferase = 0;
}

// Codopname no. 076 opcode 0x4b cerasen
static void cerasen(void) {
    readexec_opername_saveD7();
    image.numelem = alis.varD7;
    
    u16 curidx = 0;
    u16 previdx = 0;

    while (searchelem(&curidx, &previdx))
    {
        killelem(&curidx, &previdx);
    }
    
    alis.ferase = 0;
}

// Codopname no. 077 opcode 0x4c cset
static void cset(void) {
    readexec_opername();
    xwrite16(alis.script->vram_org + ALIS_SCR_WCX, alis.varD7);
    readexec_opername();
    xwrite16(alis.script->vram_org + ALIS_SCR_WCY, alis.varD7);
    readexec_opername();
    xwrite16(alis.script->vram_org + ALIS_SCR_WCZ, alis.varD7);
}

// Codopname no. 078 opcode 0x4d cmov
static void cmov(void) {
    readexec_opername();
    xadd16(alis.script->vram_org + ALIS_SCR_WCX, alis.varD7);
    readexec_opername();
    xadd16(alis.script->vram_org + ALIS_SCR_WCY, alis.varD7);
    readexec_opername();
    xadd16(alis.script->vram_org + ALIS_SCR_WCZ, alis.varD7);
}

// Codopname no. 079 opcode 0x4e copensc
static void copensc(void) {
    u16 id = script_read16();
    *(alis.mem + alis.basemain + id) &= 0xbf;
    *(alis.mem + alis.basemain + id) |= 0x80;

    scbreak(id);
    scadd(id);
}

// Codopname no. 080 opcode 0x4f cclosesc
static void cclosesc(void) {
    u16 id = script_read16();
    *(alis.mem + alis.basemain + id) |= 0x40;
}

// Codopname no. 081 opcode 0x50 cerasall
static void cerasall(void) {
    u16 tmpidx = 0;
    u16 curidx = get_0x18_unknown(alis.script->vram_org);
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }
    
    alis.ferase = 0;
}

// Codopname no. 082 opcode 0x51 cforme
static void cforme(void) {
    readexec_opername();
    set_0x1a_cforme(alis.script->vram_org, alis.varD7);
}

// Codopname no. 083 opcode 0x52 cdelforme
static void cdelforme(void) {
    set_0x1a_cforme(alis.script->vram_org, -1);
}

u8 multifirm(s32 ent_vram, s32 ent_formedata, s32 formedata, s32 a4)
{
    u16 length = xread8(ent_formedata - 1);
    for (int i = 0; i < length; i++, ent_formedata += 2)
    {
        s32 forme = xread16(ent_formedata);
        if (-1 < forme)
        {
            if (!traitfirm(ent_vram, formedata, a4, forme + forme))
            {
                debug(EDebugInfo, "\n    Hit!");
                return 0;
            }
        }
    }
    
    debug(EDebugInfo, "\n    No hits!");
    return 1;
}

s32 traitfirm(s32 ent_vram, s32 formedata, s32 ent_baseform, s32 forme)
{
    s32 ent_formedata = xread16(ent_baseform + forme) + ent_baseform;
    s16 test = alis.platform.kind == EPlatformPC ? (s8)xread8(ent_formedata) : xread16(ent_formedata);
    if (test < 0)
    {
        return multifirm(ent_vram, ent_formedata + 2, formedata, ent_baseform);
    }
    
    return monofirm(ent_vram, ent_formedata + 2);
}

s32 monofirm(s32 ent_vram, s32 ent_formedata)
{
    s16 tmpx1, tmpy1, tmpz1, tmpx2, tmpy2, tmpz2;
    
    u16 result = xread16(ent_formedata) & alis.matmask;
    if (result == 0)
    {
        debug(EDebugInfo, "\n    Masked");
        return 1;
    }
    
    u8 bits = xread8(ent_formedata - 2);
    if (bits == 0)
    {
        tmpx1 = (s8)xread8(ent_formedata + 2);
        tmpy1 = (s8)xread8(ent_formedata + 3);
        tmpz1 = (s8)xread8(ent_formedata + 4);
        
        tmpx2 = (s8)xread8(ent_formedata + 5);
        tmpy2 = (s8)xread8(ent_formedata + 6);
        tmpz2 = (s8)xread8(ent_formedata + 7);
    }
    else if (bits == 1)
    {
        tmpx1 = xread16(ent_formedata + 2);
        tmpy1 = xread16(ent_formedata + 4);
        tmpz1 = xread16(ent_formedata + 6);

        tmpx2 = xread16(ent_formedata + 0x8);
        tmpy2 = xread16(ent_formedata + 0xa);
        tmpz2 = xread16(ent_formedata + 0xc);
    }
    else
    {
        debug(EDebugWarning, "Unexpected form data!");
        return 1;
    }

    alis.goodmat = xread16(ent_formedata);

    if (get_0x03_xinv(ent_vram) != 0)
    {
        tmpx1 = -tmpx1;
        tmpx2 = -tmpx2;
    }
    
    tmpx1 += xread16(ent_vram + ALIS_SCR_WCX);
    tmpy1 += xread16(ent_vram + ALIS_SCR_WCY);
    tmpz1 += xread16(ent_vram + ALIS_SCR_WCZ);

    tmpx2 += tmpx1;
    tmpy2 += tmpy1;
    tmpz2 += tmpz1;

    s16 tmp = tmpy1;
    if (tmpy2 <= tmpy1)
    {
        tmp = tmpy2;
        tmpy2 = tmpy1;
    }
    
    if ((py1 <= tmpy2) && (tmp <= py2))
    {
        tmp = tmpz1;
        if (tmpz2 <= tmpz1)
        {
            tmp = tmpz2;
            tmpz2 = tmpz1;
        }
        
        if ((pz1 <= tmpz2) && (tmp <= pz2))
        {
            tmp = tmpx1;
            if (tmpx2 <= tmpx1)
            {
                tmp = tmpx2;
                tmpx2 = tmpx1;
            }
            
            if ((px1 <= tmpx2) && (tmp <= px2))
            {
                debug(EDebugInfo, "\n    tx: %.3d, %.3d", tmpx1, tmpx2);
                debug(EDebugInfo, "    ty: %.3d, %.3d", tmpy1, tmpy2);
                debug(EDebugInfo, "    tz: %.3d, %.3d", tmpz1, tmpz2);

                return 0;
            }
        }
    }
    
    return 1;
}

s32 traitform(s32 ent_vram, s32 ent_formedata, s32 ent_baseform, s32 ent_wforme2x)
{
    s32 formedata = xread16(alis.baseform + ent_wforme2x) + alis.baseform;
    s16 test = alis.platform.kind == EPlatformPC ? (s8)xread16(formedata) : xread16(formedata);
    if (test < 0)
    {
        return multiform(ent_vram, ent_formedata, formedata + 2, ent_baseform);
    }
    
    return monoform(ent_vram, ent_formedata, formedata + 2, ent_baseform, ent_wforme2x);
}

s32 multiform(s32 ent_vram, s32 ent_formedata, s32 formedata, s32 ent_baseform)
{
    s8 length = xread8(formedata - 1);
    for (int i = 0; i < length; i++, formedata+=2)
    {
        s32 wforme = xread16(formedata);
        if (-1 < wforme)
        {
            if (!traitform(ent_vram, ent_formedata, ent_baseform, wforme + wforme))
            {
                debug(EDebugInfo, "\n    Hit!");
                return 0;
            }
        }
    }
    
    debug(EDebugInfo, "\n    No hits!");
    return 1;
}

s32 monoform(s32 ent_vram, s32 ent_formedata, s32 formedata, s32 ent_baseform, s32 ent_wforme2x)
{
    s16 frmx, frmy, frmz;
    
    u8 bits = xread8(formedata - 2);
    if (bits == 0)
    {
        frmx = (s8)xread8(formedata + 2);
        frmy = (s8)xread8(formedata + 3);
        frmz = (s8)xread8(formedata + 4);

        px2 = (s8)xread8(formedata + 5);
        py2 = (s8)xread8(formedata + 6);
        pz2 = (s8)xread8(formedata + 7);
        formedata += 8;
    }
    else if (bits == 1)
    {
        frmx = xread16(formedata + 2);
        frmy = xread16(formedata + 4);
        frmz = xread16(formedata + 6);

        px2 = xread16(formedata + 0x8);
        py2 = xread16(formedata + 0xa);
        pz2 = xread16(formedata + 0xc);
        formedata += 0xe;
    }
    else
    {
        debug(EDebugWarning, "Unexpected form data!");
        return 1;
    }

    if (get_0x03_xinv(alis.script->vram_org) != 0)
    {
        frmx = -frmx;
        px2 = -px2;
    }
    
    frmx += alis.wcx;
    frmy += alis.wcy;
    frmz += alis.wcz;

    px2 += frmx;
    py2 += frmy;
    pz2 += frmz;

    py1 = frmy;
    if (py2 <= frmy)
    {
        py1 = py2;
        py2 = frmy;
    }
    
    pz1 = frmz;
    if (pz2 <= frmz)
    {
        pz1 = pz2;
        pz2 = frmz;
    }
    
    px1 = frmx;
    if (px2 <= frmx)
    {
        px1 = px2;
        px2 = frmx;
    }
    
    debug(EDebugInfo, "\n    px: %.3d, %.3d", px1, px2);
    debug(EDebugInfo, "    py: %.3d, %.3d", py1, py2);
    debug(EDebugInfo, "    pz: %.3d, %.3d", pz1, pz2);
    
    s16 test = alis.platform.kind == EPlatformPC ? (s8)xread16(ent_formedata) : xread16(ent_formedata);     ent_formedata += 2;
    if (test < 0)
    {
        return multifirm(ent_vram, ent_formedata, formedata, ent_baseform);
    }
    
    return monofirm(ent_vram, ent_formedata);
}

// check for intersected object

void clipform(void) {
    
    alis.ptrent = alis.tablent;
    if (-1 < alis.wforme)
    {
        s32 val = get_0x14_script_org_offset(alis.script->vram_org);
        s32 addr = xread32(val + 0xe) + val;
        alis.baseform = xread32(addr + 6) + addr;
        s32 formedata = (u16)xread16(alis.baseform + (u32)alis.wforme * 2) + alis.baseform;
        s16 entidx = 0;

        debug(EDebugInfo, "\nChecking: %s (%.2x)", alis.script->name, alis.wforme);
        
        do
        {
            s32 ent_vram = xread32(alis.atent + entidx);
            if (xread16(ent_vram + 6) == xread16(alis.script->vram_org + 6) && -1 < get_0x1a_cforme(ent_vram) && alis.script->vram_org != ent_vram)
            {
                s32 ent_val = get_0x14_script_org_offset(ent_vram);
                s32 ent_addr = xread32(ent_val + 0xe) + ent_val;
                s32 ent_baseform = xread32(ent_addr + 6) + ent_addr;
                s32 ent_wforme2x = get_0x1a_cforme(ent_vram) * 2;
                s32 ent_formedata = xread16(ent_baseform + ent_wforme2x) + ent_baseform;
                
                sAlisScriptLive *sc = ENTSCR(entidx);
                debug(EDebugInfo, "\n  to: %s", sc->name);
                
                s32 result = 0;

                s16 test = alis.platform.kind == EPlatformPC ? (s8)xread16(formedata) : xread16(formedata);
                if (test < 0)
                {
                    result = multiform(ent_vram, ent_formedata, formedata + 2, ent_baseform);
                }
                else
                {
                    result = monoform(ent_vram, ent_formedata, formedata + 2, ent_baseform, ent_wforme2x);
                }
                
                debug(EDebugInfo, "    = %d", result);
                if (!result)
                {
                    if (alis.witmov != 0)
                    {
                        alis.fmitmov = 1;
                        return;
                    }

                    int index = (int)(alis.ptrent - alis.tablent);
                    alis.matent[index] = alis.goodmat;
                    alis.tablent[index] = entidx;
                    alis.ptrent ++;
                    formedata += 2;

                    if (alis.fallent == 0)
                    {
                        break;
                    }
                }
            }

            entidx = xread16(alis.atent + 4 + entidx);
        }
        while (entidx != 0);
    }

    if (alis.witmov == 0)
    {
        // NOTE: matent!!!
        int index = (int)(alis.ptrent - alis.tablent);
        alis.matent[index] = 0;
        alis.tablent[index] = -1;
        alis.ptrent++;
    }
}

// Codopname no. 088 opcode 0x57 csuccent
static void csuccent(void) {
    
    alis.varD7 = *alis.ptrent;
    
    if (-1 < *alis.ptrent)
        alis.ptrent ++;

    cstore_continue();
}

// Codopname no. 097 opcode 0x60 crstent
static void crstent(void) {
    alis.ptrent = alis.tablent;
    csuccent();
}

// Codopname no. 084 opcode 0x53 ctstmov
static void ctstmov(void) {
    readexec_opername();
    alis.wcx = xread16(alis.script->vram_org + ALIS_SCR_WCX) + alis.varD7;
    readexec_opername();
    alis.wcy = xread16(alis.script->vram_org + ALIS_SCR_WCY) + alis.varD7;
    readexec_opername();
    alis.wcz = xread16(alis.script->vram_org + ALIS_SCR_WCZ) + alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    alis.wforme = get_0x1a_cforme(alis.script->vram_org);
    clipform();
    crstent();
}

// Codopname no. 085 opcode 0x54 ctstset
static void ctstset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 086 opcode 0x55 cftstmov
static void cftstmov(void) {
    readexec_opername();
    alis.wcx = xread16(alis.script->vram_org + ALIS_SCR_WCX) + alis.varD7;
    readexec_opername();
    alis.wcy = xread16(alis.script->vram_org + ALIS_SCR_WCY) + alis.varD7;
    readexec_opername();
    alis.wcz = xread16(alis.script->vram_org + ALIS_SCR_WCZ) + alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    readexec_opername();
    alis.wforme = alis.varD7;
    clipform();
    crstent();
}

// Codopname no. 087 opcode 0x56 cftstset
static void cftstset(void) {
    readexec_opername();
    alis.wcx = alis.varD7;
    readexec_opername();
    alis.wcy = alis.varD7;
    readexec_opername();
    alis.wcz = alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    readexec_opername();
    alis.wforme = alis.varD7;
    clipform();
    crstent();
}

// Codopname no. 089 opcode 0x58 cpredent
static void cpredent(void) {
    if (alis.ptrent != alis.tablent)
    {
        alis.ptrent --;
        alis.varD7 = *alis.ptrent;
    }
    else
    {
        alis.varD7 = -1;
    }

    cstore_continue();
}

u8 calcnear(u32 source, u32 target, s32 *val)
{
    s32 wcx = xread16(target + ALIS_SCR_WCX) - xread16(source + ALIS_SCR_WCX);
    s32 wcy = xread16(target + ALIS_SCR_WCY) - xread16(source + ALIS_SCR_WCY);
    s32 wcz = xread16(target + ALIS_SCR_WCZ) - xread16(source + ALIS_SCR_WCZ);
    *val = (wcx * wcx) + (wcy * wcy) + (wcz * wcz);
    return (*val <= alis.valnorme && (alis.fview == 0 || *val <= ((s8)xread8(source + ALIS_SCR_WCX2) * wcx + (s8)xread8(source + ALIS_SCR_WCY2) * wcy + (s8)xread8(source + ALIS_SCR_WCZ2) * wcz) * (s32)alis.valchamp)) ? 1 : 0;
}

void trinorme(int bufidx)
{
    if (bufidx > 0)
    {
        bool result = false;

        do
        {
            s16 length = bufidx;
            for (int i = 0; i < length; i++, bufidx--)
            {
                s32 bufvalue = ((s32 *)alis.buffer)[bufidx - 1];
                if (bufvalue < ((s32 *)alis.buffer)[bufidx - 2])
                {
                    ((s32 *)alis.buffer)[bufidx - 1] = ((s32 *)alis.buffer)[bufidx - 2];
                    ((s32 *)alis.buffer)[bufidx - 2] = bufvalue;
                    u16 tblvalue = alis.tablent[bufidx - 1];
                    alis.tablent[bufidx - 1] = alis.tablent[bufidx - 2];
                    alis.tablent[bufidx - 2] = tblvalue;
                    result = true;
                }
            }
        }
        while (result);
    }
}

void sviewtyp(void)
{
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);

    s16 id = alis.varD7;
    if (id < 0)
        id &= 0xff;
    
    s16 bufidx = 0;
    if (alis.fallent == 0)
    {
        ((s32 *)alis.buffer)[bufidx] = 0x7fffffff;
        alis.tablent[bufidx] = 0xffff;

        bufidx = 1;
    }
    
    u8 veradd = alis.platform.version >= 30 ? 0x30 : 0x6;

    s16 entidx = 0;
    u32 src_vram = alis.script->vram_org;

    do
    {
        u32 tgt_vram = xread32(alis.atent + entidx);

        if (id == get_0x10_script_id(tgt_vram) && xread16(tgt_vram + veradd) == xread16(src_vram + veradd) && src_vram != tgt_vram)
        {
            s32 value = 0;
            u8 result = calcnear(src_vram, tgt_vram, &value);
            if (!result)
            {
                if (alis.fallent == 0)
                {
                    if (((s32 *)alis.buffer)[bufidx - 1] <= value)
                        goto LAB_00015d44;
                    
                    bufidx --;
                }
                
                ((s32 *)alis.buffer)[bufidx] = value;
                alis.tablent[bufidx] = entidx;

                bufidx ++;
            }
        }
        
LAB_00015d44:
        
        entidx = xread16(alis.atent + 4 + entidx);
        if (entidx == 0)
        {
            alis.tablent[bufidx] = -1;
            if (alis.fallent != 0)
            {
                trinorme(bufidx);
            }
            
            alis.fallent = 0;
            crstent();
            return;
        }
    }
    while (true);
}

s16 multimat(s32 formeaddr, s32 baseform, s16 wforme)
{
    s16 length = xread8(formeaddr - 2);
    if (length == 0)
    {
        return wforme;
    }
    
    for (int i = 0; i < length; i++)
    {
        wforme = xread16(formeaddr + i * 2);
        if (-1 < wforme)
        {
            wforme = (s16)traitmat(baseform, wforme);
            if (wforme)
            {
                return wforme;
            }
        }
    }

    return wforme;
}

u32 traitmat(s32 baseform, s32 wforme)
{
    wforme *= 2;
    s16 formeaddr = xread32(baseform + wforme) + baseform;
    if (xread16(formeaddr) < 0)
    {
        return multimat(formeaddr + 2, baseform, wforme);
    }
    else
    {
        return monomat(formeaddr + 2);
    }
}

s16 monomat(s32 formeaddr)
{
    alis.goodmat = xread16(formeaddr);
    return alis.matmask & alis.goodmat;
}

s16 clipmat(s32 src_vram, s32 tgt_vram)
{
    s16 wforme = get_0x1a_cforme(tgt_vram);
    if (-1 < wforme && src_vram != tgt_vram)
    {
        s32 addr = xread32(get_0x14_script_org_offset(tgt_vram) + 0xe) + get_0x14_script_org_offset(tgt_vram);
        s32 baseform = xread32(addr + 6) + addr;
        s32 formeaddr = xread16(baseform + wforme * 2) + baseform;
        if (xread16(formeaddr) < 0)
        {
            return multimat(formeaddr + 2, baseform, wforme);
        }
        else
        {
            return monomat(formeaddr + 2);
        }
    }
    
    return wforme;
}

void sviewmat(void)
{
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);

    s16 bufidx = 0;
    if (alis.fallent == 0)
    {
        ((s32 *)alis.buffer)[bufidx] = 0x7fffffff;
        alis.tablent[bufidx] = 0xffff;

        bufidx = 1;
    }

    u8 veradd = alis.platform.version >= 30 ? 0x30 : 0x6;

    s16 entidx = 0;
    u32 src_vram = alis.script->vram_org;
    u32 tgt_vram = 0;

    do
    {
        tgt_vram = xread32(alis.atent + entidx);
        
        if (xread16(src_vram + veradd) == xread16(tgt_vram + veradd))
        {
            s16 result = clipmat(src_vram, tgt_vram);
            if (result)
            {
                s32 value = 0;
                u8 result = calcnear(src_vram, tgt_vram, &value);
                if (!result)
                {
                    if (alis.fallent == 0)
                    {
                        if (((s32 *)alis.buffer)[bufidx - 1] <= value)
                            goto LAB_00015c8a;

                        bufidx --;
                    }

                    ((s32 *)alis.buffer)[bufidx] = value;
                    alis.tablent[bufidx] = entidx;
                    bufidx++;
                }
            }
        }

LAB_00015c8a:

        entidx = xread16(alis.atent + 4 + entidx);
        if (entidx == 0)
        {
            alis.tablent[bufidx] = -1;
            if (alis.fallent != 0)
            {
                trinorme(bufidx);
            }
            
            alis.fallent = 0;
            crstent();
            return;
        }
    }
    while (true);
}

void sviewent(void)
{
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);

    if (alis.platform.version >= 30)
    {
        if (alis.fallent != 0)
        {
            alis.varD7 = -1;
        }

        s16 bufidx = 0;

        if (alis.fallent == 0)
        {
            ((s32 *)alis.buffer)[bufidx] = 0x7fffffff;
            alis.tablent[bufidx] = 0xffff;

            bufidx = 1;
        }
        
        s16 entidx = 0;
        u32 src_vram = alis.script->vram_org;

        do
        {
            u32 tgt_vram = xread32(alis.atent + entidx);

            if ((alis.varD7 < 0 || alis.varD7 == get_0x0e_script_ent(tgt_vram)) && xread16(tgt_vram + 0x30) == xread16(src_vram + 0x30) && src_vram != tgt_vram)
            {
                s32 value = 0;
                u8 result = calcnear(src_vram, tgt_vram, &value);
                if (!result)
                {
                    if (alis.fallent == 0)
                    {
                        if (((s32 *)alis.buffer)[bufidx - 1] <= value)
                            goto cnearen2;
                        
                        bufidx --;
                    }
                    
                    ((s32 *)alis.buffer)[bufidx] = value;
                    alis.tablent[bufidx] = entidx;

                    bufidx ++;
                }
            }
            
cnearen2:
            
            entidx = xread16(alis.atent + 4 + entidx);
            if (entidx == 0)
            {
                alis.tablent[bufidx] = -1;
                if (alis.fallent != 0)
                {
                    trinorme(bufidx);
                }
                
                alis.fallent = 0;
                crstent();
                return;
            }
        }
        while (true);
    }
    else
    {
        s16 entidx = alis.varD7;
        s16 tabidx = 0;
        
        u32 src_vram = alis.script->vram_org;
        u32 tgt_vram = xread32(alis.atent + entidx);
        
        if (xread16(tgt_vram + 6) == xread16(src_vram + 6))
        {
            s32 tmp;
            u8 result = calcnear(src_vram, tgt_vram, &tmp);
            if (!result)
            {
                alis.tablent[tabidx++] = entidx;
            }
        }
        
        alis.tablent[tabidx] = -1;
        alis.fallent = 0;
        crstent();
    }
}

// Codopname no. 090 opcode 0x59 cnearent
void cnearent(void)
{
    alis.fview = 0;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    sviewent();
}

// Codopname no. 091 opcode 0x5a cneartyp
static void cneartyp(void) {
    alis.fview = 0;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    if (alis.platform.kind == EPlatformPC)
    {
        alis.varD7 = script_read16();
    }
    else
    {
        readexec_opername();
    }
    
    sviewtyp();
}

// Codopname no. 092 opcode 0x5b cnearmat
static void cnearmat(void) {
    alis.fview = 0;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    sviewmat();
}

// Codopname no. 093 opcode 0x5c cviewent
static void cviewent(void) {
    alis.fview = 1;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    alis.valchamp = alis.varD7;
    readexec_opername();
    sviewent();
}

// Codopname no. 094 opcode 0x5d cviewtyp
static void cviewtyp(void) {
    alis.fview = 1;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    alis.valchamp = alis.varD7;
    if (alis.platform.kind == EPlatformPC)
    {
        alis.varD7 = script_read16();
    }
    else
    {
        readexec_opername();
    }

    sviewtyp();
}

// Codopname no. 095 opcode 0x5e cviewmat
static void cviewmat(void) {
    alis.fview = 1;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    alis.valchamp = alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    sviewmat();
}

// Codopname no. 096 opcode 0x5f corient
static void corient(void) {
    readexec_opername();
    alis.varD7 *= 3;

    int addr = get_0x20_set_vect(alis.script->vram_org) == 0 ? alis.vstandard + 1 : get_0x20_set_vect(alis.script->vram_org) + get_0x14_script_org_offset(alis.script->vram_org) + 1;
    xwrite8(alis.script->vram_org + ALIS_SCR_WCX2, xread8(addr + 0 + alis.varD7));
    xwrite8(alis.script->vram_org + ALIS_SCR_WCY2, xread8(addr + 1 + alis.varD7));
    xwrite8(alis.script->vram_org + ALIS_SCR_WCZ2, xread8(addr + 2 + alis.varD7));
}

// Codopname no. 098 opcode 0x61 csend
static void csend(void) {
    s16 length = script_read8();

    readexec_opername();

    s16 entry = alis.varD7;
    if (entry != -1)
    {
        s32 vram = xread32(alis.atent + entry);
        if (vram != 0)
        {
            if ((get_0x24_scan_inter(vram) & 1) == 0)
            {
                s16 offset = xread16(get_0x14_script_org_offset(vram) + 0x16);
                s16 limit = alis.platform.version >= 30 ? -0x3f : -0x35;
                s16 old_val = get_0x1c_scan_clr(vram);
                s16 new_val;

                while (true)
                {
                    readexec_opername();
                    
                    new_val = old_val + 2;
                    if (limit < new_val)
                    {
                        new_val -= offset;
                    }
                    
                    if (new_val == get_0x1e_scan_clr(vram))
                    {
                        break;
                    }
                    
                    xwrite16(vram + old_val, alis.varD7);
                    
                    old_val = new_val;
                    length--;
                    
                    if (length == -1)
                    {
                        set_0x1c_scan_clr(vram, new_val);
                        set_0x24_scan_inter(vram, get_0x24_scan_inter(vram) | 0x80);
                        return;
                    }
                }
                
                set_0x1c_scan_clr(vram, new_val);
            }
        }
    }
    
    while ((length--) != -1) {
        readexec_opername();
    }
}

// Codopname no. 103 opcode 0x66 cscanclr
static void cscanclr(void) {
    set_0x1e_scan_clr(alis.script->vram_org, get_0x1c_scan_clr(alis.script->vram_org));
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0x7f);
}

// Codopname no. 099 opcode 0x62 cscanon
static void cscanon(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfe);
}

// Codopname no. 100 opcode 0x63 cscanoff
static void cscanoff(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 1);
    cscanclr();
}

// Codopname no. 101 opcode 0x64 cinteron
static void cinteron(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfd);
}

// Codopname no. 102 opcode 0x65 cinteroff
static void cinteroff(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 2);
}

// Codopname no. 104 opcode 0x67 callentity
static void callentity(void) {
    alis.fallent = 1;
}

// Codopname no. 105 opcode 0x68 cpalette
static void cpalette(void) {
    alis.flagmain = 0;

    readexec_opername();
    s16 palidx = alis.varD7;
    if (palidx < 0)
    {
        restorepal(palidx, 0);
    }
    else
    {
        s32 addr = adresdes(palidx);
        u8 *paldata = alis.mem + addr + xread32(addr);
        topalette(paldata, 0);
    }
}

// Codopname no. 106 opcode 0x69 cdefcolor
static void cdefcolor(void) {
    readexec_opername();
    readexec_opername_saveD6();

    u8 *rawcolor = (u8 *)&(alis.varD6);
    
    u8 r, g, b;
    if (alis.platform.kind == EPlatformAmiga || alis.platform.kind == EPlatformAmigaAGA)
    {
        r = (rawcolor[1] & 0b00001111) << 4;
        g = (rawcolor[0] >> 4) << 4;
        b = (rawcolor[0] & 0b00001111) << 4;
    }
    else
    {
        r = (rawcolor[1] & 0b00000111) << 5;
        g = (rawcolor[0] >> 4) << 5;
        b = (rawcolor[0] & 0b00000111) << 5;
    }
    
    s16 index = alis.varD7 * 3;
    image.mpalet[index + 0] = r;
    image.mpalet[index + 1] = g;
    image.mpalet[index + 2] = b;
    
    setmpalet();
    set_update_cursor();
}

// Codopname no. 107 opcode 0x6a ctiming
static void ctiming(void) {
    readexec_opername();
    alis.ctiming = (u8)(alis.varD7 & 0xff);
}

// Codopname no. 108 opcode 0x6b czap
static void czap(void) {
    u8 pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    u8 priorson = alis.varD7;
    readexec_opername();
    s16 volson = alis.varD7 << 8;
    readexec_opername();
    u16 freqson = alis.varD7;
    readexec_opername();
    u16 longson = alis.varD7;
    if (longson == 0)
    {
        return;
    }
    
    readexec_opername();
    u16 dfreqson = alis.varD7;

    runson(eChannelTypeDingZap, pereson, priorson, volson, freqson, longson, 0, dfreqson);
}

// Codopname no. 109 opcode 0x6c cexplode
static void cexplode(void) {
    u8 pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    u8 priorson = alis.varD7;
    readexec_opername();
    s16 volson = alis.varD7 << 8;
    readexec_opername();
    u16 freqson = alis.varD7;
    readexec_opername();
    u16 longson = alis.varD7;
    if (longson == 0)
    {
        return;
    }
    
    u32 dvolson = ((u32)volson) / (u32)longson;
    if (dvolson == 0)
    {
        dvolson = 1;
    }

    runson(eChannelTypeExplode, pereson, priorson, volson, freqson, longson, -(s16)dvolson, 0);
}

// Codopname no. 110 opcode 0x6d cding
static void cding(void) {
    u8 pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    u8 priorson = alis.varD7;
    readexec_opername();
    s16 volson = alis.varD7 << 8;
    readexec_opername();
    u16 freqson = alis.varD7;
    readexec_opername();
    u16 longson = alis.varD7;
    if (longson == 0)
    {
        return;
    }
    
    u32 dvolson = ((u32)volson) / (u32)longson;
    if (dvolson == 0)
    {
        dvolson = 1;
    }

    runson(eChannelTypeDingZap, pereson, priorson, volson, freqson, longson, -(s16)dvolson, 0);
}

// Codopname no. 111 opcode 0x6e cnoise
static void cnoise(void) {
    u8 pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    u8 priorson = alis.varD7;
    readexec_opername();
    s16 volson = alis.varD7 << 8;
    readexec_opername();
    u16 freqson = alis.varD7;
    readexec_opername();
    u16 longson = alis.varD7;
    if (longson == 0)
    {
        return;
    }
    
    u32 dvolson = ((u32)volson) / (u32)longson;
    if (dvolson == 0)
    {
        dvolson = 1;
    }

    runson(eChannelTypeNoise, pereson, priorson, volson, freqson, longson, -(s16)dvolson, 0);
}

// Codopname no. 112 opcode 0x6f cinitab
static void cinitab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 113 opcode 0x70 cfopen
static void cfopen(void) {
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    
    u16 mode;

    if(*(alis.mem + alis.script->pc) == 0xff)
    {
        script_jump(1);
        readexec_opername_swap();
        readexec_opername();

        strcat(path, alis.sd7);
        mode = alis.varD7;
    }
    else
    {
        script_read_until_zero((path + strlen(alis.platform.path)));
        mode = script_read16();
    }

    afopen((char *)path, mode);
//    alis.fp = sys_fopen((char *)path, mode);
    if(alis.fp == NULL) {
        alis_error(ALIS_ERR_FOPEN, path);
    }
}

// Codopname no. 114 opcode 0x71 cfclose
static void cfclose(void) {
    if(sys_fclose(alis.fp) < 0) {
        alis_error(ALIS_ERR_FCLOSE);
    }
    
    alis.fp = NULL;
}

// Codopname no. 115 opcode 0x72 cfcreat
static void cfcreat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 116 opcode 0x73 cfdel
static void cfdel(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 117 opcode 0x74 cfreadv
static void cfreadv(void) {
    fread(alis.buffer, 2, 1, alis.fp);
    alis.varD7 = xpcswap16(*(s16 *)alis.buffer);
    debug(EDebugInfo, " 0x%04x", alis.varD7);
    cstore_continue();
}

// Codopname no. 118 opcode 0x75 cfwritev
static void cfwritev(void) {
    readexec_opername();
    s16 val = xpcswap16(alis.varD7);
    debug(EDebugInfo, " 0x%04x", alis.varD7);
    fwrite(&val, 2, 1, alis.fp);
}

// Codopname no. 119 opcode 0x76 cfwritei
static void cfwritei(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 120 opcode 0x77 cfreadb
static void cfreadb(void) {
    
    s32 addr = (s16)script_read16();
    if (addr == 0)
    {
        addr = (s16)script_read16();
        addr += alis.basemain;
    }
    else
    {
        addr += alis.script->vram_org;
    }
    
    u32 length = 0;

    if (alis.platform.version >= 30)
    {
        if ((s8)xread8(addr - 1) < 0)
        {
            addr = xread32(addr);
            addr = xread32(addr);
            if (addr < 1)
            {
                return;
            }
        }
        
        length = script_read32();
        
        if (alis.typepack == 0)
        {
            fread(alis.mem + addr, length, 1, alis.fp);
        }
        else
        {
            debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);

//            u8 *unpacked_buffer = alis.mem + addr;
//            u32 unpacked_size = unpack_script_fp(alis.fp, &unpacked_buffer);
//            if (alis.wordpack != 0)
//            {
//                 unmixword();
//            }
        }
    }
    else
    {
        length = script_read16();
        
        fread(alis.mem + addr, length, 1, alis.fp);
    }
    
    // NOTE: *.fic files in all platforms are identical, for PC we have to do byteswaping
    
    if (alis.platform.kind == EPlatformPC)
    {
        u8 bytes = *(alis.mem + addr - 2);
        if (bytes == 2)
        {
            for (int i = 0; i < length; i += 2)
            {
                u16 *val = (u16 *)(alis.mem + addr + i);
                *val = (*val <<  8) | (*val >>  8);
            }
        }
    }
}

void writeswap(u32 addr, u32 length)
{
    if (alis.platform.kind == EPlatformPC)
    {
        u8 bytes = *(alis.mem + addr - 2);
        if (bytes == 2)
        {
            u8 *data = malloc(length);
            u16 *tgt = (u16 *)data;
            
            for (int i = 0; i < length; i += 2, tgt ++)
            {
                u16 *val = (u16 *)(alis.mem + addr + i);
                *tgt = (*val <<  8) | (*val >>  8);
            }
            
            fwrite(data, length, 1, alis.fp);
            free(data);
            return;
        }
    }
    
    fwrite(alis.mem + addr, length, 1, alis.fp);
}

// Codopname no. 121 opcode 0x78 cfwriteb
static void cfwriteb(void) {
    s32 addr = (s16)script_read16();
    if (addr == 0)
    {
        addr = (s16)script_read16();
        addr += alis.basemain;
    }
    else
    {
        addr += alis.script->vram_org;
    }
    
    if (alis.platform.version >= 30)
    {
        u32 length = script_read32();

        if (-1 < (s8)xread8(addr - 1))
        {
            writeswap(addr, length);
        }
        else
        {
            u32 addr1 = xread32(addr);
            u32 addr2 = xread32(addr1);
            if (0 < addr2)
            {
                writeswap(addr2, length);
            }
        }
    }
    else
    {
        u16 length = script_read16();
        writeswap(addr, length);
    }
}

s16 adptsin[2];

void plot(s16 x0, s16 y0)
{
    image.logic[x0 + y0 * alis.platform.width] = image.inkcolor;
}

void SYS_PutPixel(void)
{
    adptsin[0] = alis.poldx;
    adptsin[1] = alis.poldy;
    
    plot(alis.poldx, alis.poldy);
}

// Codopname no. 122 opcode 0x79 cplot
static void cplot(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    alis.poldx = alis.varD7;
    alis.poldy = alis.varD6;
    
    SYS_PutPixel();
}

void io_drawline(s16 x0, s16 y0, s16 x1, s16 y1)
{
    s32 dx = abs(x1 - x0);
    s32 sx = x0 < x1 ? 1 : -1;
    s32 dy = -abs(y1 - y0);
    s32 sy = y0 < y1 ? 1 : -1;
    s32 error = dx + dy;
    
    while (true)
    {
        plot(x0, y0);
        if (x0 == x1 && y0 == y1)
            break;
            
        s32 e2 = 2 * error;
        if (e2 >= dy)
        {
            if (x0 == x1)
                break;
                
            error += dy;
            x0 += sx;
        }
        
        if (e2 <= dx)
        {
            if (y0 == y1)
                break;
            
            error += dx;
            y0 += sy;
        }
    }
}

void io_box(s16 x1,s16 y1,s16 x2,s16 y2)
{
    if (alis.platform.kind == EPlatformMac)
    {
        mac_update_pos(&x1, &y1);
        mac_update_pos(&x2, &y2);
    }

    io_drawline(x1, y1, x2, y1);
    io_drawline(x2, y1, x2, y2);
    io_drawline(x2, y2, x1, y2);
    io_drawline(x1, y2, x1, y1);
}

void io_boxf(s16 x1,s16 y1,s16 x2,s16 y2)
{
    if (alis.platform.kind == EPlatformMac)
    {
        mac_update_pos(&x1, &y1);
        mac_update_pos(&x2, &y2);
    }
    
    s16 tmpx = min(x1, x2);
    x2 = max(x1, x2);
    x1 = tmpx;
    tmpx = x2 - x1;
    for (s16 y = y1; y <= y2; y++)
    {
        memset(image.logic + x1 + y * alis.platform.width, image.inkcolor, tmpx);
    }
}

// Codopname no. 123 opcode 0x7a cdraw
static void cdraw(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 oldx = alis.poldx;
    s16 oldy = alis.poldy;
    
    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;
    
    io_drawline(oldx, oldy, alis.poldx, alis.poldy);
}

// Codopname no. 124 opcode 0x7b cbox
static void cbox(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 oldx = alis.poldx;
    s16 oldy = alis.poldy;

    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;

    io_box(oldx, oldy, alis.poldx, alis.poldy);
}

// Codopname no. 125 opcode 0x7c cboxf
static void cboxf(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 oldx = alis.poldx;
    s16 oldy = alis.poldy;

    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;

    io_boxf(oldx, oldy, alis.poldx, alis.poldy);
}

// Codopname no. 126 opcode 0x7d cink
static void cink(void) {
    readexec_opername();
    image.inkcolor = alis.varD7;
}

// Codopname no. 127 opcode 0x7e cpset
static void cpset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 128 opcode 0x7f cpmove
static void cpmove(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 129 opcode 0x80 cpmode
static void cpmode(void) {
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);

    readexec_opername();
    image.line_a_mode = alis.varD7;
}

// Codopname no. 130 opcode 0x81 cpicture
static void cpicture(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 131 opcode 0x82 cxyscroll
static void cxyscroll(void) {
    readexec_opername();
    readexec_opername();
}

// Codopname no. 132 opcode 0x83 clinking
static void clinking(void) {
    readexec_opername();
    set_0x2a_clinking(alis.script->vram_org, alis.varD7);
}

// Codopname no. 133 opcode 0x84 cmouson
static void cmouson(void) {
    sys_enable_mouse(1);
}

// Codopname no. 134 opcode 0x85 cmousoff
static void cmousoff(void) {
    sys_enable_mouse(0);
}

// Codopname no. 135 opcode 0x86 cmouse
// 0x86 - 14d62
static void cmouse(void) {
    mouse_t mouse = sys_get_mouse();
    
    if (alis.platform.kind == EPlatformMac)
    {
        mouse.x /= 1.5;
        mouse.y /= 1.5;
    }
   
    alis.varD7 = mouse.x;
    cstore_continue();
    
    alis.varD7 = mouse.y;
    cstore_continue();
    
    alis.varD7 = mouse.lb ? 1 : (mouse.rb ? 2 : 0);
    cstore_continue();
    
    mouse.lb = 0;
    
    alis.butmouse = 0;
    alis.cbutmouse = 0;
}

// Codopname no. 136 opcode 0x87 cdefmouse
static void cdefmouse(void) {
    alis.flagmain = 0;
    readexec_opername();
    
    if (alis.platform.kind != EPlatformMac)
    {
        u32 addr = adresdes(alis.varD7);
        u8 *ptr = alis.mem + addr + xread32(addr);
        if (*ptr == 0 || *ptr == 0x10 || *ptr == 0x14)
        {
            alis.desmouse = ptr;
            set_update_cursor();
        }
    }
}

// Codopname no. 137 opcode 0x88 csetmouse
static void csetmouse(void) {
    readexec_opername();
    u16 x = alis.varD7;
    readexec_opername();
    u16 y = alis.varD6;
    sys_set_mouse(x, y);
}

// Codopname no. 138 opcode 0x89 cdefvect
static void cdefvect(void) {
    s16 offset = script_read8();
    offset += offset * 2;
    offset += 3;
    
    alis.script->pc += offset;
}

// Codopname no. 139 opcode 0x8a csetvect
static void csetvect(void) {
    s16 value = script_read16();
    set_0x20_set_vect(alis.script->vram_org, value);
}

u8 *deb_approach(s32 offset, s16 *wcx, s16 *wcy, s16 *wcz)
{
    offset = xread32(alis.atent + offset);
    *wcx = xread16(offset + ALIS_SCR_WCX) - xread16(alis.script->vram_org + ALIS_SCR_WCX);
    *wcy = xread16(offset + ALIS_SCR_WCY) - xread16(alis.script->vram_org + ALIS_SCR_WCY);
    *wcz = xread16(offset + ALIS_SCR_WCZ) - xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    
    u8 *address = get_0x20_set_vect(alis.script->vram_org) == 0 ? vstandard : alis.mem + get_0x20_set_vect(alis.script->vram_org) + get_0x14_script_org_offset(alis.script->vram_org);

    alis.varD7 = *address;
    xwrite8(alis.script->vram_org + ALIS_SCR_ADDR, *address);
    return address + 1;
}

s32 calscal(s8 *address, s16 wcx, s16 wcy, s16 wcz)
{
    s32 result = 0;
    if (address[0] != 0)
        result += address[0] * wcx;
    
    if (address[1] != 0)
        result += address[1] * wcy;
    
    if (address[2] != 0)
        result += address[2] * wcz;

    return result;
}

// Codopname no. 141 opcode 0x8c capproach
static void capproach(void) {
    readexec_opername();
    if (alis.varD7 != -1)
    {
        s16 wcx, wcy, wcz;
        u8 *addr0 = deb_approach(alis.varD7, &wcx, &wcy, &wcz);
        u8 *addr1 = 0;

        s32 limit = 0x80000000;
        s8 index = alis.varD6;
 
        do
        {
            s32 test = calscal((s8 *)addr0, wcx, wcy, wcz);
            if (limit < test)
            {
                index = (s8)alis.varD7;
                limit = test;
                addr1 = addr0;
            }
            
            addr0 += 3;
            alis.varD7 --;
        }
        while (alis.varD7 != -1);
        
        xwrite8(alis.script->vram_org + ALIS_SCR_ADDR, xread8(alis.script->vram_org + ALIS_SCR_ADDR) - index);
        xwrite8(alis.script->vram_org + ALIS_SCR_WCX2, addr1[0]);
        xwrite8(alis.script->vram_org + ALIS_SCR_WCY2, addr1[1]);
        xwrite8(alis.script->vram_org + ALIS_SCR_WCZ2, addr1[2]);
    }
}

// Codopname no. 142 opcode 0x8d cescape
static void cescape(void) {
    readexec_opername();
    if (alis.varD7 != -1)
    {
        s16 wcx, wcy, wcz;
        u8 *addr0 = deb_approach(alis.varD7, &wcx, &wcy, &wcz);
        u8 *addr1 = 0;

        s32 limit = 0x7fffffff;
        s8 index = alis.varD6;

        do
        {
            s16 test = calscal((s8 *)addr0, wcx, wcy, wcz);
            if (test < limit)
            {
                index = (s8)alis.varD7;
                limit = test;
                addr1 = addr0;
            }

            addr0 += 3;
            alis.varD7 --;
        }
        while (alis.varD7 != -1);
        
        xwrite8(alis.script->vram_org + ALIS_SCR_ADDR, xread8(alis.script->vram_org + ALIS_SCR_ADDR) - index);
        xwrite8(alis.script->vram_org + ALIS_SCR_WCX2, addr1[0]);
        xwrite8(alis.script->vram_org + ALIS_SCR_WCY2, addr1[1]);
        xwrite8(alis.script->vram_org + ALIS_SCR_WCZ2, addr1[2]);
    }
}

// Codopname no. 143 opcode 0x8e cvtstmov
static void cvtstmov(void) {
    alis.wcx = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2) + xread16(alis.script->vram_org + ALIS_SCR_WCX);
    alis.wcy = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCY2) + xread16(alis.script->vram_org + ALIS_SCR_WCY);
    alis.wcz = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCZ2) + xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    readexec_opername();
    alis.matmask = alis.varD7;
    alis.wforme = get_0x1a_cforme(alis.script->vram_org);
    clipform();
    crstent();
}

// Codopname no. 144 opcode 0x8f cvftstmov
static void cvftstmov(void) {
    alis.wcx = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2) + xread16(alis.script->vram_org + ALIS_SCR_WCX);
    alis.wcy = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCY2) + xread16(alis.script->vram_org + ALIS_SCR_WCY);
    alis.wcz = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCZ2) + xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    readexec_opername();
    alis.matmask = alis.varD7;
    readexec_opername();
    alis.wforme = alis.varD7;
    clipform();
    crstent();
}

// Codopname no. 145 opcode 0x90 cvmov
static void cvmov(void) {
    xadd16(alis.script->vram_org + ALIS_SCR_WCX, (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2));
    xadd16(alis.script->vram_org + ALIS_SCR_WCY, (s8)xread8(alis.script->vram_org + ALIS_SCR_WCY2));
    xadd16(alis.script->vram_org + ALIS_SCR_WCZ, (s8)xread8(alis.script->vram_org + ALIS_SCR_WCZ2));
}

// Codopname no. 146 opcode 0x91 cdefworld
static void cdefworld(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    s16 offset = script_read16();
    u8 counter = 5;
    while(counter--) {
        xwrite8(alis.script->vram_org + offset, script_read8());
    }
}

// Codopname no. 147 opcode 0x92 cworld
static void cworld(void) {
    xwrite8(alis.script->vram_org + 0xffde, script_read8());
    xwrite8(alis.script->vram_org + 0xffdf, script_read8());
}

// Codopname no. 148 opcode 0x93 cfindmat
static void cfindmat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 149 opcode 0x94 cfindtyp
static void cfindtyp(void) {
    
    if (alis.platform.kind == EPlatformPC)
    {
        alis.varD7 = script_read16();
    }
    else
    {
        readexec_opername();
    }
    
    if (alis.varD7 < 0)
    {
        alis.varD7 &= 0xff;
    }

    s16 id = alis.varD7;
    s16 entidx = 0;
    s32 tabidx = 0;
    
    do
    {
        u32 tgt_vram = xread32(alis.atent + entidx);
        if (id == get_0x10_script_id(tgt_vram) && alis.script->vram_org != tgt_vram)
        {
            alis.matent[tabidx] = 0;
            alis.tablent[tabidx] = entidx;
            tabidx ++;
            
            if (alis.fallent == 0)
            {
                break;
            }
        }
        
        entidx = xread16(alis.atent + 4 + entidx);
    }
    while (entidx != 0);
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    crstent();
}

void music(void) {
    if (alis.platform.version <= 10)
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        return;
    }
    
    u16 scriptId = alis.flagmain ? alis.main->data->header.id : alis.script->data->header.id;
    audio.musicId = scriptId;
    
    readexec_opername();
    s16 idx = alis.varD7;
    s32 addr = adresmus(idx);

    u8 type = xread8(alis.script->data->data_org + addr);
    if (type == 0 || type == 3)
    {
        audio.muvolume = 0;

        audio.mupnote = alis.script->data->data_org + addr + 6;
        readexec_opername();
        audio.maxvolume = alis.varD7 << 8;
        readexec_opername();
        audio.mutempo = (char)alis.varD7;
        readexec_opername();
        audio.muattac = alis.varD7 + 1;
        readexec_opername();
        audio.muduree = alis.varD7 + 1;
        readexec_opername();
        audio.muchute = alis.varD7 + 2;
        if (audio.muattac != 0)
        {
            audio.dattac = audio.maxvolume / audio.muattac;
        }
        
        if (audio.muchute != 0)
        {
            audio.dchute = audio.maxvolume / audio.muchute;
        }

        u8 type = xread8(audio.mupnote - 6);
        if (type == 0)
        {
            mv1_gomusic();
        }
        else
        {
            debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
        }

        audio.mustate = 1;
    }
    else if (type == 4)
    {
        audio.muvolume = 0;

        audio.mupnote = alis.script->data->data_org + addr + 6;
        readexec_opername();
        audio.maxvolume = (alis.varD7 << 8);
        readexec_opername();
        audio.mutempo = alis.varD7;
        readexec_opername();
        audio.muattac = alis.varD7 + 1;
        readexec_opername();
        audio.muduree = alis.varD7;
        readexec_opername();
        audio.muchute = alis.varD7 + 1;

        audio.dattac = 0;
        audio.dchute = 0;
        if (audio.muattac != 0)
        {
            audio.dattac = audio.maxvolume / audio.muattac;
        }
        if (audio.muchute != 0)
        {
            audio.dchute = audio.maxvolume / audio.muchute;
        }

        audio.muvolume = audio.maxvolume >> 8;
        
        if (audio.fmusic)
        {
            mv2_gomusic();
        }
        
        audio.mustate = 1;
    }
    else
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        
        debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    }
}

// Codopname no. 150 opcode 0x95 cmusic
static void cmusic(void) {
    alis.flagmain = 0;
    music();
}

// Codopname no. 151 opcode 0x96 cdelmusic
static void cdelmusic(void) {
    if (alis.platform.version <= 10)
        return;
    
    readexec_opername();
    
    if (alis.platform.version < 21)
    {
        mv1_offmusic(alis.varD7);
    }
    else
    {
        mv2_offmusic(alis.varD7);
    }
}

// Codopname no. 152 opcode 0x97 ccadence
static void ccadence(void) {
    readexec_opername();
    if (alis.varD7 == 0)
        alis.varD7 = 1;

    audio.mutempo = (u8)alis.varD7;
    audio.mutemp = (u8)((audio.mutempo * 6) / 0x20);
}

// Codopname no. 153 opcode 0x98 csetvolum
static void csetvolum(void) {
    readexec_opername();
    audio.muvolume = alis.varD7;
    *(u8 *)&audio.maxvolume = alis.varD7;
    audio.muvol = ((audio.muvolume) >> 1) + 1;

    if (audio.muattac != 0)
    {
        audio.dattac = audio.maxvolume / audio.muattac;
    }
    
    if (audio.muchute != 0)
    {
        audio.dchute = audio.maxvolume / audio.muchute;
    }
}

// Codopname no. 154 opcode 0x99 cxinv
static void cxinv(void) {
    u8 xinv = get_0x03_xinv(alis.script->vram_org);
    BIT_CHG(xinv, 0);
    set_0x03_xinv(alis.script->vram_org, xinv);
}

// Codopname no. 155 opcode 0x9a cxinvon
static void cxinvon(void) {
    set_0x03_xinv(alis.script->vram_org, 1);
}

// Codopname no. 156 opcode 0x9b cxinvoff
static void cxinvoff(void) {
    set_0x03_xinv(alis.script->vram_org, 0);
}

// Codopname no. 157 opcode 0x9c clistent
static void clistent(void) {
    s16 entidx = 0;
    s16 tabidx = 0;
    while ((entidx = xread16(alis.atent + 4 + entidx)) != 0)
    {
        alis.matent[tabidx] = 0;
        alis.tablent[tabidx] = entidx;
        tabidx ++;
    }
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    crstent();
}

static void sound(void) {
    readexec_opername();
    u8 index = alis.varD7;
    readexec_opername();
    u8 priorson = alis.varD7;
    readexec_opername();
    u8 volson = alis.varD7;
    readexec_opername();
    u16 loopsam = alis.varD7;
    readexec_opername();
    u8 speedsam = alis.varD7;
    
    s32 addr = adresmus(index);
    s8 type = xread8(alis.script->data->data_org + addr);
    if (type == 1 || type == 2)
    {
        if (speedsam == 0)
            speedsam = xread8(alis.script->data->data_org + addr + 1);

        u32 longsam = xread32(alis.script->data->data_org + addr + 2) - 0x10;
        if (alis.platform.kind == EPlatformPC && (alis.platform.uid == EGameColorado || alis.platform.uid == EGameWindsurfWilly || alis.platform.uid == EGameMadShow || alis.platform.uid == EGameLeFeticheMaya))
            longsam = xpcread32(alis.script->data->data_org + addr + 2) - 0x10;

        u32 startsam = alis.script->data->data_org + addr + 0x10;
        playsample(eChannelTypeSample, alis.mem + startsam, speedsam, volson, longsam, loopsam);
    }
}

// Codopname no. 158 opcode 0x9d csound
static void csound(void) {
    alis.flagmain = 0;
    sound();
}

// Codopname no. 159 opcode 0x9e cmsound
static void cmsound(void) {
    alis.flagmain = 1;
    sound();
}

// Codopname no. 160 opcode 0x9f credon
static void credon(void) {
    set_0x25_credon_credoff(alis.script->vram_org, 0x0);
}

// Codopname no. 161 opcode 0xa0 credoff
static void credoff(void) {
    set_0x25_credon_credoff(alis.script->vram_org, alis.platform.version < 30 ? 0xff : 0x80);
}

// Codopname no. 162 opcode 0xa1 cdelsound
static void cdelsound(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    for (int i = 0; i < 4; i++)
    {
        channels[i].type = eChannelTypeNone;
    }

    for (int i = 0; i < 3; i++)
    {
        channels[i].type = eChannelTypeNone;
        channels[i].volume = 0;
        channels[i].freq = 0;
        channels[i].curson = 0x80;
        channels[i].state = 0;
        channels[i].played = 0;
        io_canal(&(channels[i]), i);
    }
}

// Codopname no. 163 opcode 0xa2 cwmov
static void cwmov(void) {
    s16 x, y, z;
    get_vector(&x, &y, &z);
    xadd16(alis.script->vram_org + ALIS_SCR_WCX, x);
    xadd16(alis.script->vram_org + ALIS_SCR_WCY, y);
    xadd16(alis.script->vram_org + ALIS_SCR_WCZ, z);
}

// Codopname no. 164 opcode 0xa3 cwtstmov
static void cwtstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 165 opcode 0xa4 cwftstmov
static void cwftstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 166 opcode 0xa5 ctstform
static void ctstform(void) {
    alis.wcx = xread16(alis.script->vram_org + ALIS_SCR_WCX);
    alis.wcy = xread16(alis.script->vram_org + ALIS_SCR_WCY);
    alis.wcz = xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    readexec_opername();
    alis.matmask = alis.varD7;
    readexec_opername();
    alis.wforme = alis.varD7;
    clipform();
    crstent();
}

// Codopname no. 167 opcode 0xa6 cxput
static void cxput(void) {
    alis.flagmain = 0;
    image.invert_x = get_0x03_xinv(alis.script->vram_org) ^ 1;
    readexec_opername();
    image.depx = 0;
    image.depy = 0;
    image.depz = 0;
    image.numelem = 0;
    
    u16 idx = alis.varD7;
    put(idx);
}

// Codopname no. 168 opcode 0xa7 cxputat
static void cxputat(void) {
    alis.flagmain = 0;
    image.invert_x = get_0x03_xinv(alis.script->vram_org) ^ 1;

    readexec_opername_saveD7();
    image.depx = alis.varD7;
    readexec_opername_saveD7();
    image.depy = alis.varD7;
    readexec_opername_saveD7();
    image.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    u16 idx = alis.varD7;
    image.numelem = alis.varD6;
    put(idx);
}

// Codopname no. 169 opcode 0xa8 cmput
static void cmput(void) {
    alis.flagmain = 1;
    image.invert_x = get_0x03_xinv(alis.script->vram_org);
    readexec_opername();
    u16 idx = alis.varD7;
    image.depx = 0;
    image.depy = 0;
    image.depz = 0;
    image.numelem = 0;
    put(idx);
}

// Codopname no. 170 opcode 0xa9 cmputat
static void cmputat(void) {
    alis.flagmain = 1;
    image.invert_x = get_0x03_xinv(alis.script->vram_org);
    readexec_opername_saveD7();
    image.depx = alis.varD7;
    readexec_opername_saveD7();
    image.depy = alis.varD7;
    readexec_opername_saveD7();
    image.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    image.numelem = alis.varD6;
    put(alis.varD7);
}

// Codopname no. 171 opcode 0xaa cmxput
static void cmxput(void) {
    alis.flagmain = 1;
    image.invert_x = get_0x03_xinv(alis.script->vram_org) ^ 1;
    readexec_opername();
    image.depx = 0;
    image.depy = 0;
    image.depz = 0;
    image.numelem = 0;
    put(alis.varD7);
}

// Codopname no. 172 opcode 0xab cmxputat
static void cmxputat(void) {
    alis.flagmain = 1;
    image.invert_x = get_0x03_xinv(alis.script->vram_org) ^ 1;
    readexec_opername_saveD7();
    image.depx = alis.varD7;
    readexec_opername_saveD7();
    image.depy = alis.varD7;
    readexec_opername_saveD7();
    image.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    image.numelem = alis.varD6;
    put(alis.varD7);
}

// Codopname no. 173 opcode 0xac cmmusic
static void cmmusic(void) {
    alis.flagmain = 1;
    music();
}

// Codopname no. 174 opcode 0xad cmforme
static void cmforme(void) {
    readexec_opername();
    set_0x1a_cforme(alis.script->vram_org, alis.varD7);
}

// Codopname no. 175 opcode 0xae csettime
static void csettime(void) {
    readexec_opername();
    u16 h = alis.varD7;
    readexec_opername();
    u16 m = alis.varD7;
    readexec_opername();
    u16 s = alis.varD7;
    sys_set_time(h, m, s);
}

// Codopname no. 176 opcode 0xaf cgettime
static void cgettime(void) {
    time_t t = sys_get_time();
    alis.varD7 = t << 16 & 0xff;
    cstore_continue();
    alis.varD7 = t << 8 & 0xff;
    cstore_continue();
    alis.varD7 = t << 0 & 0xff;
    cstore_continue();
}

void oval(void);
void getval(void)
{
    u8 c;
    char *ptr = alis.sd7;
    while (1)
    {
        c = getchar();
        if (c == 0xd)
        {
            break;
        }
        
        if (c == 8)
        {
            if (ptr != alis.sd7)
            {
                ptr --;
                *ptr = 0;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else if ((c == 0x2d) || ((-1 < (char)(c - 0x30) && (c < 0x3a))))
        {
            *ptr++ = c;
            putchar(c);
        }
    }
    
    *ptr = 0;
    
    oval();
}

// Codopname no. 177 opcode 0xb0 cvinput
static void cvinput(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    alis.charmode = 0;
    getval();
    cstore_continue();
}

void getstring(void)
{
    u8 c;
    char *ptr = alis.sd7;
    while (1)
    {
        c = getchar();
        if (c == '\r')
        {
            break;
        }
        
        if (c == '\b')
        {
            if (ptr != alis.sd7)
            {
                ptr --;
                *ptr = 0;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else
        {
            *ptr++ = c;
            putchar(c);
        }
    }
    
    *ptr = 0;
}

// Codopname no. 178 opcode 0xb1 csinput
static void csinput(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    alis.charmode = 0;
    getstring();
    cstore_continue();
}

// Codopname no. 179 opcode 0xb2 casleepfar
static void casleepfar(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
    readexec_opername();
// ...
}

// Codopname no. 180 opcode 0xb3 casleepon
static void casleepon(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfb); // TODO check if byte ptr is OK: btr dword ptr es:[ebp-24h], 2
}

// Codopname no. 181 opcode 0xb4 casleepoff
static void casleepoff(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 4); // TODO check if byte ptr is OK: bts dword ptr es:[ebp-24h], 2
}

// Codopname no. 182 opcode 0xb5 crunfilm
static void crunfilm(void) {
    readexec_opername();
    bfilm.batchframes = alis.varD7;
    runfilm();
    alis.varD7 = bfilm.frame;
    cstore_continue();
    alis.varD7 = bfilm.result;
    cstore_continue();
}

void printd0(s16 d0w)
{
    char *ptr = alis.sd7;
    valtostr(alis.sd7, d0w);

    if(DEBUG_SCRIPT) {
       debug(EDebugInfo, " [\"%s\"]", alis.sd7);
    }
    
    while (*ptr != 0)
    {
        put_char(*ptr++);
    }
}

void putval(s16 d7w)
{
    printd0(d7w);
}

// Codopname no. 183 opcode 0xb6 cvpicprint
static void cvpicprint(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    alis.charmode = 1;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

// Codopname no. 184 opcode 0xb7 cspicprint
static void cspicprint(void) {
    alis.charmode = 1;
    readexec_opername_swap();
    put_string();
}

// Codopname no. 185 opcode 0xb8 cvputprint
static void cvputprint(void) {
    alis.charmode = 2;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

// Codopname no. 186 opcode 0xb9 csputprint
static void csputprint(void) {
    alis.charmode = 2;
    readexec_opername_swap();
    put_string();
}

// Codopname no. 187 opcode 0xba cfont
static void cfont(void) {
    readexec_opername();
    alis.foasc = alis.varD7;
    readexec_opername();
    alis.fonum = alis.varD7;
    readexec_opername();
    alis.folarg = (u8)alis.varD7;
    readexec_opername();
    alis.fohaut = (u8)alis.varD7;
    readexec_opername();
    alis.fomax = alis.varD7;
}

// Codopname no. 188 opcode 0xbb cpaper
static void cpaper(void) {
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    readexec_opername();
    
    // TODO: ...
}

// Codopname no. 189 opcode 0xbc ctoblack
// fade-out to black
static void ctoblack(void) {
    readexec_opername_saveD6();
    
    s16 duration = alis.varD6;
    toblackpal(duration);
}

s16 subcol(s16 change, s16 component)
{
    s16 result = (component & 0xf) - (change & 0xf);
    if (result < 0)
        return 0;
    
    if (result < 8)
        return result;
    
    return 7;
}

s16 addcol(s16 change, s16 component)
{
    s16 result = (component & 0xf) + (change & 0xf);
    if (result < 0)
        return 0;
    
    if (result < 8)
        return result;
    
    return 7;
}

// Codopname no. 190 opcode 0xbd cmovcolor
static void cmovcolor(void) {
    readexec_opername();
    readexec_opername_saveD6();

    s16 index = alis.varD7;
    s16 change = alis.varD6;

    if (alis.platform.bpp == 4)
    {
        if (change < 0)
        {
            change = -change;
 
            image.mpalet[index * 3 + 0] = subcol(change       , image.mpalet[index * 3 + 0] >> 5) << 5;
            image.mpalet[index * 3 + 0] = subcol(change >> 4  , image.mpalet[index * 3 + 0] >> 5) << 5;
            image.mpalet[index * 3 + 0] = subcol(change >> 8  , image.mpalet[index * 3 + 0] >> 5) << 5;
        }
        else
        {
            image.mpalet[index * 3 + 0] = addcol(change       , image.mpalet[index * 3 + 0] >> 5) << 5;
            image.mpalet[index * 3 + 0] = addcol(change >> 4  , image.mpalet[index * 3 + 0] >> 5) << 5;
            image.mpalet[index * 3 + 0] = addcol(change >> 8  , image.mpalet[index * 3 + 0] >> 5) << 5;
        }
        
        image.ftopal = 0xff;
        image.thepalet = 0;
        image.defpalet = 0;

        set_update_cursor();
    }
}

// Codopname no. 191 opcode 0xbe ctopalet
// fade-in to palette
static void ctopalet(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    alis.flagmain = 0;
   
    u16 palidx = alis.varD7;
    u16 duration = alis.varD6;
    
    if (palidx < 0)
    {
        restorepal(palidx, duration);
    }
    else
    {
        s32 addr = adresdes(palidx);
        u8 *paldata = alis.mem + addr + xread32(addr);
        topalette(paldata, duration);
    }
}

// Codopname no. 192 opcode 0xbf cnumput
static void cnumput(void) {
    readexec_opername();
    image.depx = alis.varD7;
    readexec_opername();
    image.depy = alis.varD7;
    readexec_opername();
    image.depz = alis.varD7;
    readexec_opername();
    image.numelem = alis.varD7;
}

// Codopname no. 193 opcode 0xc0 cscheart
static void cscheart(void) {
    readexec_opername();
    readexec_opername_saveD6();

    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scr_unknown0x0a(screen_id, alis.varD7);
        set_scr_unknown0x0c(screen_id, alis.varD6);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 194 opcode 0xc1 cscpos
static void cscpos(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        if (alis.platform.kind == EPlatformMac)
        {
            mac_update_pos(&alis.varD7, &alis.varD6);
        }
        
        set_scr_newx(screen_id, alis.varD7);
        set_scr_newy(screen_id, alis.varD6);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 195 opcode 0xc2 cscsize
static void cscsize(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        if (alis.platform.version < 30)
        {
            if (alis.platform.kind == EPlatformMac)
            {
                alis.varD7--;
                alis.varD6--;
                mac_update_pos(&alis.varD7, &alis.varD6);
                alis.varD7++;
                alis.varD6++;
            }
            
            set_scr_width(screen_id, alis.varD7);
            set_scr_height(screen_id, alis.varD6);
        }
        else
        {
            set_scr_width(screen_id, alis.varD7 - 1);
            set_scr_height(screen_id, alis.varD6 - 1);

            scdosprite(screen_id);
        }

        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 196 opcode 0xc3 cschoriz
static void cschoriz(void) {
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        readexec_opername();
        set_scr_unknown0x20(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scr_unknown0x21(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scr_unknown0x22(screen_id, (u8)alis.varD7);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 197 opcode 0xc4 cscvertic
static void cscvertic(void) {
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        readexec_opername();
        set_scr_unknown0x23(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scr_unknown0x24(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scr_unknown0x25(screen_id, (u8)alis.varD7);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 198 opcode 0xc5 cscreduce
static void cscreduce(void) {
    readexec_opername();
    readexec_opername_saveD6();

    u16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scr_creducing(screen_id, alis.varD7);
        
        readexec_opername();
        set_scr_clinking(screen_id, (alis.varD6 - 1) << 8 | alis.varD7);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 199 opcode 0xc6 cscscale
static void cscscale(void) {
    readexec_opername();
    s8 scale = alis.varD7;
    u16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scr_credon_off(screen_id, scale);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 200 opcode 0xc7 creducing
static void creducing(void) {
    readexec_opername();
    set_0x26_creducing(alis.script->vram_org, alis.varD7);
    readexec_opername();
    set_0x27_creducing(alis.script->vram_org, alis.varD7);
}

// Codopname no. 201 opcode 0xc8 cscmap
static void cscmap(void) {
    readexec_opername();
    u16 ent = get_0x0e_script_ent(alis.script->vram_org);
    s16 val = script_read16();
    if (val == 0)
    {
        ent = 0;
        val = script_read16();
    }
    
    if (get_0x16_screen_id(alis.script->vram_org) != 0)
    {
        s32 tgtaddr = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
        xwrite16(tgtaddr + 0x42, val);
        xwrite16(tgtaddr + 0x40, ent);
    }
}

// Codopname no. 202 opcode 0xc9 cscdump
static void cscdump(void) {
    debug(EDebugInfo, "MISSING (NOP?): %s", __FUNCTION__);
}

// Codopname no. 203 opcode 0xca cfindcla
static void cfindcla(void) {
    readexec_opername();
    s32 tabidx = 0;
    s16 entidx = 0;
    
    u8 veradd = alis.platform.version >= 30 ? 0x4c : 0xc;

    do
    {
        u32 tgt_vram = xread32(alis.atent + entidx);
        if ((s8)alis.varD7 == (s8)xread8(tgt_vram + veradd) && alis.script->vram_org != tgt_vram)
        {
            alis.matent[tabidx] = 0;
            alis.tablent[tabidx] = entidx;
            tabidx ++;
            
            if (alis.fallent == 0)
            {
                break;
            }
        }
        
        entidx = xread16(alis.atent + entidx + 4);
    }
    while (entidx != 0);
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    
    crstent();
}

// Codopname no. 204 opcode 0xcb cnearcla
static void cnearcla(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 205 opcode 0xcc cviewcla
static void cviewcla(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 206 opcode 0xcd cinstru
static void cinstru(void) {
    alis.flagmain = 0;
    readexec_opername();
    s16 tabidx = alis.varD7;
    readexec_opername();
    s16 scridx = alis.varD7;
    readexec_opername();
    s16 instidx = alis.varD7;

    s32 addr;
    u8 type = 0;

    if (scridx < 0)
    {
        instidx = 0;
        addr = 0;
    }
    else
    {
        addr = adresmus(scridx);
        type = xread8(alis.script->data->data_org + addr);
        if (type != 1 && type != 2 && type != 5 && type != 6)
            return;
        
        addr += 0x10;
    }
    
    audio.tabinst[tabidx].address = alis.script->data->data_org + addr;
    audio.tabinst[tabidx].unknown = instidx;
}

// Codopname no. 207 opcode 0xce cminstru
static void cminstru(void) {
    alis.flagmain = 1;
    readexec_opername();
    s16 tabidx = alis.varD7;
    readexec_opername();
    s16 scridx = alis.varD7;
    readexec_opername();
    s16 instidx = alis.varD7;

    s32 addr;

    if (scridx < 0)
    {
        instidx = 0;
        addr = 0;
    }
    else
    {
        addr = adresmus(scridx);
        u8 type = xread8(alis.script->data->data_org + addr);
        if (type != 1 && type != 2 && type != 5 && type != 6)
            return;
        
        addr += 0x10;
    }
    
    audio.tabinst[tabidx].address = addr;
    audio.tabinst[tabidx].unknown = instidx;
}

// Codopname no. 208 opcode 0xcf cordspr
static void cordspr(void) {
    readexec_opername();
    set_0x2b_cordspr(alis.script->vram_org, (u8)alis.varD7);
}

// Codopname no. 209 opcode 0xd0 calign
static void calign(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    u8 bit;

    readexec_opername();

    if (((u8)alis.varD7 & 0x10) == 0)
    {
        bit = 4;
    }
    else if (((u8)alis.varD7 & 0x8) == 0)
    {
        bit = 3;
    }
    else if (((u8)alis.varD7 & 0x4) == 0)
    {
        bit = 2;
    }
    else if (((u8)alis.varD7 & 0x2) == 0)
    {
        bit = 1;
    }
    else
    {
        bit = 0;
    }

    set_0x2c_calign(alis.script->vram_org, bit);
    
    readexec_opername();
    set_0x2d_calign(alis.script->vram_org, alis.varD7);
}

void rescmode(s16 at, u8 value)
{
    SPRITE_VAR(at)->numelem = value;
}

s16 starbuff[8];

// Codopname no. 210 opcode 0xd1 cbackstar
static void cbackstar(void) {
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    
    readexec_opername();
    starbuff[0] = alis.varD7;
    readexec_opername();
    starbuff[1] = alis.varD7;
    readexec_opername();
    starbuff[2] = alis.varD7;
    readexec_opername();
    starbuff[3] = alis.varD7;
    readexec_opername();
    starbuff[4] = alis.varD7;
    readexec_opername();
    starbuff[5] = alis.varD7;
    starbuff[6] = 0;

    switch (alis.platform.uid) {
            
        case EGameColorado:
        {
            if (-1 < (s16)get_0x16_screen_id(alis.script->vram_org))
            {
                u8 value = xread8(alis.basemain + 1);
                value &= 0xf7;
                value &= 0xef;
                if (alis.varD7 != 0)
                    value |= 0x18;

                xwrite8(alis.basemain + 1, value);
            }
            break;
        }

        case EGameManhattanDealers0:
        case EGameManhattanDealers1:
        case EGameMadShow:
        case EGameWindsurfWilly:
        case EGameLeFeticheMaya:
        {
            // NOP
            break;
        }
            
        case EGameStarblade:
        {
            // TODO: starblade implementation differs
            break;
        }
        
        case EGameStormMaster:
        case EGameTarghan0:
        case EGameTarghan1:
        case EGameCrystalsOfArborea0:
        case EGameCrystalsOfArborea1:
        case EGameMetalMutant:
        case EGameTransartica:
        case EGameBostonBombClub:
        case EGameBunnyBricks:
        case EGameIshar_1:
        case EGameIshar_2:
        case EGameIshar_3:
        case EGameRobinsonsRequiem0:
        case EGameRobinsonsRequiem1:
        {
            if (-1 < (s16)get_0x16_screen_id(alis.script->vram_org))
            {
                u8 value = xread8(alis.basemain + 1);
                value &= 0xf7;
                value &= 0xef;
                if (alis.varD7 != 0)
                {
                    value |= 0x18;

                    s16 at = xread16(alis.basemain + get_0x16_screen_id(alis.script->vram_org) + 2);
                    rescmode(at, value);
                }
                
                xwrite8(alis.basemain + get_0x16_screen_id(alis.script->vram_org) + 1, value);
            }
            break;
        }
            
        default:
            break;
    }
    
//    if (-1 < (s16)get_0x16_screen_id(alis.script->vram_org))
//    {
//        if (alis.platform.uid == EGameColorado) // colorado and likely other older games
//        {
//            u8 value = xread8(alis.basemain + 1);
//            value &= 0xf7;
//            value &= 0xef;
//            if (starbuff[0] != 0)
//                value |= 0x18;
//
//            xwrite8(alis.basemain + 1, value);
//        }
//        else
//        {
//            u8 value = xread8(alis.basemain + get_0x16_screen_id(alis.script->vram_org) + 1);
//            value &= 0xf7;
//            value &= 0xef;
//            if (starbuff[0] != 0)
//            {
//                value |= 0x18;
//
//                s16 at = xread16(alis.basemain + get_0x16_screen_id(alis.script->vram_org) + 2);
//                rescmode(at, value);
//            }
//            
//            xwrite8(alis.basemain + get_0x16_screen_id(alis.script->vram_org) + 1, value);
//        }
//    }
}

// Codopname no. 211 opcode 0xd2 cstarring
static void cstarring(void) {
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    
    readexec_opername();
    starbuff[0] = alis.varD7;
    readexec_opername();
    starbuff[1] = alis.varD7;
    readexec_opername();
    starbuff[2] = alis.varD7;
    readexec_opername();
    
    // not used in colorado
    if (alis.platform.uid != EGameColorado)
    {
        starbuff[3] = alis.varD7;
        readexec_opername();
        starbuff[4] = alis.varD7;
        readexec_opername();
        starbuff[5] = alis.varD7;
        readexec_opername();
        starbuff[6] = alis.varD7;
        readexec_opername();
        starbuff[7] = alis.varD7;
    }
}

// Codopname no. 212 opcode 0xd3 cengine
static void cengine(void) {
    readexec_opername();
    readexec_opername();
    readexec_opername();
    readexec_opername();
}

// Codopname no. 213 opcode 0xd4 cautobase
static void cautobase(void) {
    debug(EDebugInfo, "MISSING (NOP?): %s", __FUNCTION__);
}

// Codopname no. 214 opcode 0xd5 cquality
static void cquality(void) {
    readexec_opername();
    alis.vquality = alis.varD7;
}

// Codopname no. 215 opcode 0xd6 chsprite
static void chsprite(void) {
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);

    readexec_opername();
    set_0x2f_chsprite(alis.script->vram_org, (s8)alis.varD7);
    if ((s8)alis.varD7 == 9)
    {
        s16 entmov = get_0x0e_script_ent(alis.script->vram_org);
        readexec_opername();
        s8 citmov = (s8)alis.varD7;
        if (citmov == 0)
        {
            citmov = 1;
        }
        
        s8 citmov0 = citmov;
        readexec_opername();
        s16 mitmov = alis.varD7;
        readexec_opername();
        s16 spritmov = get_0x18_unknown(alis.script->vram_org);
        s16 sitmov = 0xff;
        s16 ritmov = 0;
        s16 fitmov = 1;
    }
    else
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        
        if (get_0x2f_chsprite(alis.script->vram_org) != 0)
        {
            return;
        }
        
        s16 fitmov = 0;
        s16 entmov = 0xffff;
        s16 spritmov = 0;
    }
}

// Codopname no. 216 opcode 0xd7 cselpalet
static void cselpalet(void) {
    readexec_opername();
    alis.varD7 &= 0x3; // 4 palettes: 0...3
    image.thepalet = alis.varD7;
    image.defpalet = 1;
}

// Codopname no. 217 opcode 0xd8 clinepalet
static void clinepalet(void) {
    readexec_opername();
    readexec_opername_saveD6();

    if (alis.platform.bpp != 8 && !(alis.platform.uid == EGameMetalMutant && alis.platform.kind == EPlatformPC))
    {
        setlinepalet();
    }
}

// Codopname no. 218 opcode 0xd9 cautomode
static void cautomode(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 219 opcode 0xda cautofile
static void cautofile(void) {
    readexec_opername_swap();
    
    if (alis.platform.version >= 30)
    {
        int i = 0;
        for (; i < 0xc; i++)
            alis.autoname[i] = alis.sd7[i];
        
        alis.autoname[i] = 0;
    }
}

// Codopname no. 220 opcode 0xdb ccancel
static void ccancel(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 221 opcode 0xdc ccancall
static void ccancall(void) {
    alis.ferase = 1;
    
    u16 tmpidx = 0;
    u16 curidx = get_0x18_unknown(alis.script->vram_org);
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }
    
    alis.ferase = 0;
}

// Codopname no. 222 opcode 0xdd ccancen
static void ccancen(void) {
    alis.ferase = 1;
    cerasen();
}

// Codopname no. 223 opcode 0xde cblast
static void cblast(void) {
    alis.ferase = 1;
    ckill();
}

void iniback(int addr)
{
    u32 size = (((u16)(xread16(addr + 0x12) + 1U) >> 4) * 8) * (u16)((image.backy2 - image.backy1) + 1) + 0x10;
    u32 baddr = io_malloc(size);
    if (baddr == 0)
    {
        image.sback = 0;
    }
    else
    {
        s32 bdes = xread32(baddr);
        image.backlarg = ((u16)(xread16(addr + 0x12) + 1U) >> 4) << 2;
        image.backmap = alis.mem + (bdes + 10);
        image.backdes = bdes;
        image.backaddr = baddr;
        xwrite32(bdes, 4);
        
        s16 *bdata = (s16 *)(alis.mem + xread32(bdes) + bdes);
        bdata[0] = 0x200;
        bdata[1] = image.backx2 - image.backx1;
        bdata[2] = image.backy2 - image.backy1;
    }
}

// Codopname no. 224 opcode 0xdf cscback
void cscback(void)
{
    s8 oldsback = image.sback;
    
    readexec_opername();
    image.sback = alis.varD7;
    readexec_opername();
    image.backprof = alis.varD7;
    readexec_opername();
    image.backx1 = alis.varD7;
    readexec_opername();
    image.backy1 = alis.varD7;
    readexec_opername();
    image.backx2 = alis.varD7;
    readexec_opername();
    image.backy2 = alis.varD7;
    
    if (alis.platform.version >= 30)
    {
        s32 tgtaddr = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
        if (oldsback != image.sback)
        {
            if (image.sback == 0)
            {
                io_mfree(image.backaddr);
                
                image.cback = 0;
                image.pback = 0;
                
                xwrite8(tgtaddr + 1, xread8(tgtaddr + 1) & 0xfb);
                for (s16 elemidx = xread16(tgtaddr + 2); elemidx != 0; elemidx = SPRITE_VAR(elemidx)->link)
                {
                    sSprite *sprite = SPRITE_VAR(elemidx);
                    if (image.backsprite == sprite->link)
                    {
                        sprite->link = SPRITE_VAR(image.backsprite)->link;
                        return;
                    }
                }
            }
            else
            {
                iniback(tgtaddr);
                
                if (image.sback != 0)
                {
                    sSprite *backsprite = SPRITE_VAR(image.backsprite);
                    backsprite->screen_id = get_0x16_screen_id(alis.script->vram_org);
                    backsprite->newx = image.backx1;
                    backsprite->newy = image.backy1;
                    backsprite->newd = image.backprof;
                    backsprite->cordspr = 0x80;
                    backsprite->numelem = 0x80;
                    backsprite->clinking = 0xffff;
                    backsprite->chsprite = 0xff;
                    backsprite->state = 0;
                    backsprite->newad = image.backdes;
                    
                    s16 elemidx = xread16(tgtaddr + 0x02);
                    xwrite8(tgtaddr + 0x01, xread16(tgtaddr + 0x01) | 4);
                    image.wback = 0;
                    
                    rangesprite(image.backsprite, 0xffff, elemidx);
                    elemidx = SPRITE_VAR(elemidx)->link;
                    
                    do
                    {
                        if (elemidx == image.backsprite)
                            return;
                        
                        sSprite *sprite = SPRITE_VAR(elemidx);
                        sprite->state = 2;
                        elemidx = sprite->link;
                    }
                    while (elemidx != 0);
                }
            }
        }
    }
}

// Codopname no. 225 opcode 0xe0 cscrolpage
static void cscrolpage(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername();
    image.spag = (s8)alis.varD7;
    readexec_opername();
    image.pagdx = alis.varD7;
    readexec_opername();
    image.pagdy = alis.varD7;
    readexec_opername();
    image.pagcount = alis.varD7;
    if (image.spag != 0)
    {
        u8 *screen = (alis.mem + alis.basemain + get_0x16_screen_id(alis.script->vram_org));
        *screen |= 0x20;
    }
}

// Codopname no. 226 opcode 0xe1 cmatent
static void cmatent(void) {
    int index = (int)(alis.ptrent - alis.tablent) - 1;
    alis.varD7 = alis.matent[index];
    cstore_continue();
}

// Codopname no. 227 opcode 0xe2 cshrink
static void cshrink(void) {
    
    // TODO: this is I3 implementation I2 and older differs verify if its ok!!!
    // delete bitmap and shift following data
    
    readexec_opername();
    readexec_opername_saveD6();
    
    alis.flagmain = 0;
    
    s16 width;

    u32 addr = adresdes(alis.varD7);
    addr += xread32(addr);
    u8 *data = alis.mem + addr;
    if (*data == 0 || *data == 2)
    {
        width = (read16(data + 2) + 1) >> 1;
    }
    else if (*data == 0x1a || *data == 0x18)
    {
        width = read16(data + 2) + 1;
    }
    else if (*data == 0x16 || *data == 0x14 || *data == 0x1e || *data == 0x1c)
    {
        width = read16(data + 2) + 1;
    }
    else if (*data == 0x12 || *data == 0x10)
    {
        width = (read16(data + 2) + 1) >> 1;
    }
    else
    {
        return;
    }
    
    s16 height = read16(data + 4);
    if (height != 0)
    {
        height++;
        
        s32 bits = width * height;
        
        u32 offset = get_0x14_script_org_offset(alis.script->vram_org);
        s32 l = xread32(offset + 0xe);
        s16 e = xread16(offset + l + 4);
        s32 a = xread32(offset + l) + l;

        for (s32 i = 0; i < e; i++, a += 4)
        {
            s32 t = xread32(offset + a);
            s32 addr2 = a + offset + t;
            if (addr < addr2)
            {
                t -= bits;
                xwrite32(offset + a, t);
            }
        }
        
        xwrite32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0x6, xread32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0x6) - bits);
        xwrite32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0xc, xread32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0xc) - bits);
        xwrite16(addr + 2, 0xf);
        xwrite16(addr + 4, 0);
        
        debug(EDebugVerbose, " (NAME: %s, ID: 0x%x) ", alis.script->name, alis.script->data->header.id);
        shrinkprog(addr + 6, bits, 0);
        alis.script->data->sz -= bits;

        debug(EDebugVerbose, "\n (NAME: %s, ID: 0x%x SZ: %d) \n", alis.script->name, alis.script->data->header.id, alis.script->data->sz);
    }
}

// Codopname no. 228 opcode 0xe3 cdefmap
static void cdefmap(void) {
    if (alis.platform.uid == EGameTransartica)
    {
        u32 mapram = alis.script->vram_org;
        
        s16 offset = script_read16();
        if (offset == 0)
        {
            offset = script_read16();
            mapram = alis.basemain;
        }
        
        mapram += offset;
        
        readexec_opername();
        xwrite8(mapram - 0x50, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x4e, alis.varD7);
        xwrite16(mapram - 0x4c, get_0x0e_script_ent(alis.script->vram_org));
        readexec_opername();
        xwrite16(mapram - 0x4a, alis.varD7);
        xwrite16(mapram - 0x48, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x48, alis.varD7 - 1 + xread16(mapram - 0x48));
        readexec_opername();
        xwrite16(mapram - 0x44, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x42, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x40, alis.varD7);
        xwrite8(mapram - 0x4f, 0);
        xwrite32(mapram - 0x2c, 4);
        xwrite8(mapram - 0x28, 0x7f);
    }
    else if (alis.platform.uid == EGameRobinsonsRequiem0 || alis.platform.uid == EGameRobinsonsRequiem1)
    {
        debug(EDebugWarning, "CHECK: %s", __FUNCTION__);

        u32 mapram = alis.script->vram_org;
        s16 offset = script_read16();
        if (offset == 0)
        {
            offset = script_read16();
            mapram = alis.basemain;
        }
        
        mapram += offset;

        readexec_opername();
        xwrite8(mapram - 0x400, (s8)alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3fe, alis.varD7);
        xwrite16(mapram - 0x3fc, get_0x0e_script_ent(alis.script->vram_org));
        readexec_opername();
        xwrite16(mapram - 0x3fa, alis.varD7);
        xwrite16(mapram - 0x3f8, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3f8, alis.varD7 - 1 + xread16(mapram - 0x3f8));
        readexec_opername();
        if (alis.varD7 == 0)
            alis.varD7 = 1;

        xwrite16(mapram - 0x3f4, alis.varD7);
        readexec_opername();
        if (alis.varD7 == 0)
            alis.varD7 = 1;

        xwrite16(mapram - 0x3f2, alis.varD7);
        readexec_opername();
        if (alis.varD7 == 0)
            alis.varD7 = 1;

        xwrite16(mapram - 0x3f0, alis.varD7);
        xwrite8(mapram - 0x3ff, 0);
        xwrite32(mapram - 0x3dc, 4);
        xwrite8(mapram - 0x3d8, 0x7f);
        
        s32 vala, valb;
        if ((s8)xread8(mapram - 1) < 0)
        {
            valb = xread32(mapram - 6);
            vala = xread32(mapram - 10);
        }
        else
        {
            valb = xread32(mapram - 4);
            vala = xread32(mapram - 6);
        }
        
        s32 ratio = (s16)(vala / (valb & 0xffff));
        xwrite16(mapram - 0x3c6, ratio);
        xwrite16(mapram - 0x3cc, ratio);
        xwrite16(mapram - 0x3c4, (u16)valb);
        xwrite16(mapram - 0x3ca, (u16)valb >> 1);
        xwrite16(mapram - 0x3b6, 0);
        
        for (int i = 0; i < 3; i++)
        {
            s32 result = -1;
            u16 count = xread16(mapram - (0x3f4 - i * 2));

            do
            {
                result++;
                count >>= 1;
            }
            while (count != 0);
            
            xwrite16(mapram - (0x3c0 - i * 2), result);
        }

        if (xread8(mapram - 0x3d8) == '\n')
        {
            s32 scrid = get_0x16_screen_id(alis.script->vram_org);
            if (scrid != 0)
            {
                xwrite16(alis.basemain + scrid + 0x40, get_0x0e_script_ent(alis.script->vram_org));
                xwrite16(alis.basemain + scrid + 0x42, mapram - alis.script->vram_org);
            }
        }
    }
    else
    {
        debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
    }
}

// Codopname no. 229 opcode 0xe4 csetmap
static void csetmap(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    u32 mapram = alis.script->vram_org;
    
    s16 offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        mapram = alis.basemain;
    }
    
    mapram += offset;
    
    if (alis.platform.uid == EGameTransartica)
    {
        readexec_opername();
        xwrite16(mapram - 0x3e, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3c, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3a, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x38, alis.varD7);
        xwrite16(mapram - 0x26, (u16)alis.varD7 - 1U);
        xwrite16(mapram - 0x3e, ((u16)(alis.varD7 - 1U) >> 1) + xread16(mapram - 0x3e));
        readexec_opername();
        xwrite16(mapram - 0x36, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x34, alis.varD7);
        xwrite16(mapram - 0x24, (u16)alis.varD7 - 1U);
        xwrite16(mapram - 0x3a, ((u16)(alis.varD7 - 1U) >> 1) + xread16(mapram - 0x3a));
    }
    else if (alis.platform.uid == EGameRobinsonsRequiem0 || alis.platform.uid == EGameRobinsonsRequiem1)
    {
        readexec_opername();
        xwrite16(mapram - 0x3ee, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3ec, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3ea, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3e8, alis.varD7);
        xwrite16(mapram - 0x3d6, (u16)alis.varD7 - 1U);
        xwrite16(mapram - 0x3ee, ((u16)(alis.varD7 - 1U) >> 1) + xread16(mapram - 0x3ee));
        readexec_opername();
        xwrite16(mapram - 0x3e6, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3e4, alis.varD7);
        xwrite16(mapram - 0x3d4, (u16)alis.varD7 - 1U);
        xwrite16(mapram - 0x3ea, ((u16)(alis.varD7 - 1U) >> 1) + xread16(mapram - 0x3ea));
    }
}

void putmap(s16 spridx, s32 bitmap)
{
    sSprite *sprite = SPRITE_VAR(spridx);
    sprite->data = bitmap;
    sprite->flaginvx = image.invert_x;
    sprite->clinking = get_0x2a_clinking(alis.script->vram_org);
    sprite->script_ent = (get_0x2c_calign(alis.script->vram_org) << 8) | get_0x2d_calign(alis.script->vram_org);
    sprite->cordspr = get_0x2b_cordspr(alis.script->vram_org);
    sprite->chsprite = get_0x2f_chsprite(alis.script->vram_org);
    sprite->credon_off = get_0x25_credon_credoff(alis.script->vram_org);
    if (-1 < (s8)sprite->credon_off)
    {
        sprite->creducing = get_0x27_creducing(alis.script->vram_org);
        sprite->credon_off = get_0x26_creducing(alis.script->vram_org);
        if ((s8)sprite->credon_off < 0)
        {
            u32 scridx = get_0x16_screen_id(alis.script->vram_org) + alis.basemain;
            sprite->creducing = 0;
            sprite->credon_off = xread8(alis.basemain + scridx + 0x1f);
        }
    }
    
    sprite->depx = image.oldcx + image.depx;
    sprite->depy = image.oldcy + image.depy;
    sprite->depz = image.oldcz + image.depz;
    
    if (alis.fmuldes == 0)
    {
        u16 newidx;
        u16 oldidx;

        if (!searchelem(&newidx, &oldidx))
        {
            return;
        }
        
        do
        {
            sSprite *cursprite = SPRITE_VAR(newidx);
            while (cursprite->state == 0)
            {
                killelem(&newidx, &oldidx);
                if (newidx == 0)
                {
                    alis.fadddes = 0;
                    return;
                }
                
                if (!testnum(&newidx))
                {
                    alis.fadddes = 0;
                    return;
                }
            }
        }
        while (nextnum(&newidx, &oldidx));
    }
    
    alis.fadddes = 0;
}

void putmap2(s16 spridx, s32 bitmap)
{
    sSprite *sprite = SPRITE_VAR(spridx);
    sprite->data = bitmap;
    sprite->flaginvx = image.invert_x;
    sprite->sprite_0x28 = get_0x28_unknown(alis.script->vram_org);
    sprite->script_ent = get_0x0e_script_ent(alis.script->vram_org);
    sprite->clinking = get_0x2a_clinking(alis.script->vram_org);
    sprite->cordspr = get_0x2b_cordspr(alis.script->vram_org);
    sprite->chsprite = get_0x2f_chsprite(alis.script->vram_org);
    sprite->creducing = get_0x27_creducing(alis.script->vram_org);
    sprite->credon_off = get_0x25_credon_credoff(alis.script->vram_org);
    if ((s8)sprite->credon_off == - 0x80)
    {
        sprite->newzoomx = get_0x38_unknown(alis.script->vram_org);
        sprite->newzoomy = get_0x36_unknown(alis.script->vram_org);
        sprite->creducing = 0;
    }
    else
    {
        sprite->creducing = get_0x27_creducing(alis.script->vram_org);
        sprite->credon_off = get_0x26_creducing(alis.script->vram_org);
    }
    
    sprite->depx = image.oldcx + image.depx;
    sprite->depy = image.oldcy + image.depy;
    sprite->depz = image.oldcz + image.depz;
    
    if (alis.fmuldes == 0)
    {
        u16 newidx;
        u16 oldidx;

        if (!searchelem(&newidx, &oldidx))
        {
            return;
        }
        
        do
        {
            sSprite *cursprite = SPRITE_VAR(newidx);
            while (cursprite->state == 0)
            {
                killelem(&newidx, &oldidx);
                if (newidx == 0)
                {
                    alis.fadddes = 0;
                    return;
                }
                
                if (!testnum(&newidx))
                {
                    alis.fadddes = 0;
                    return;
                }
            }
        }
        while (nextnum(&newidx, &oldidx));
    }
    
    alis.fadddes = 0;
}

// Codopname no. 230 opcode 0xe5 cputmap
static void cputmap(void) {
    u32 mapram = alis.script->vram_org;
    
    s16 offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        mapram = alis.basemain;
    }
    
    mapram += offset;
    
    if (alis.platform.uid == EGameTransartica)
    {
        readexec_opername();
        xwrite16(mapram - 0x32, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x30, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x2e, alis.varD7);
        readexec_opername();
        image.numelem = (u8)alis.varD7;
        u32 uVar3 = xread16(mapram - 0x2e);
        u16 uVar1 = xread16(mapram - 0x40);
        u16 uVar2 = uVar3 % uVar1;
        xwrite16(mapram - 0x22, (s16)uVar2);
        alis.fmuldes = 0;
        
        u16 newidx = 0;
        u16 oldidx = 0;
        if (!searchelem(&newidx, &oldidx))
        {
            createlem(&newidx, &oldidx);
        }
        
        sSprite *sprite = SPRITE_VAR(newidx);
        if (-1 < sprite->state)
        {
            sprite->state = 2;
        }
        
        putmap(newidx, mapram - 0x2c);
        
        if (searchelem(&newidx, &oldidx))
        {
            sSprite *sprite = SPRITE_VAR(newidx);
            sprite->credon_off = 0xff;
            sprite->flaginvx = 0;
            sprite->chsprite = 0;
            sprite->depx = xread16(mapram - 0x3e);
            sprite->depy = xread16(mapram - 0x3c);
            sprite->depz = xread16(mapram - 0x3a);
        }
    }
    else if (alis.platform.uid == EGameRobinsonsRequiem0 || alis.platform.uid == EGameRobinsonsRequiem1)
    {
        readexec_opername();
        xwrite16(mapram - 0x3e2, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3e0, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3de, alis.varD7);
        readexec_opername();
        image.numelem = (u8)alis.varD7;
        u32 uVar3 = xread16(mapram - 0x3de);
        u16 uVar1 = xread16(mapram - 0x3f0);
        u16 uVar2 = uVar3 % uVar1;
        xwrite16(mapram - 0x3d2, (s16)uVar2);
        alis.fmuldes = 0;
        
        u16 newidx = 0;
        u16 oldidx = 0;
        if (!searchelem(&newidx, &oldidx))
        {
            createlem(&newidx, &oldidx);
        }
        
        sSprite *sprite = SPRITE_VAR(newidx);
        if (-1 < sprite->state)
        {
            sprite->state = 2;
        }
        
        putmap2(newidx, mapram - 0x3dc);
        
        if (searchelem(&newidx, &oldidx))
        {
            sSprite *sprite = SPRITE_VAR(newidx);
            sprite->credon_off = 0xff;
            sprite->flaginvx = 0;
            sprite->chsprite = 0;
            sprite->depx = xread16(mapram - 0x3ee);
            sprite->depy = xread16(mapram - 0x3ec);
            sprite->depz = xread16(mapram - 0x3ea);
        }
    }
}

// Codopname no. 231 opcode 0xe6 csavepal
static void csavepal(void) {
    readexec_opername();
    savepal(alis.varD7);
}

// Codopname no. 232 opcode 0xe7 csczoom
static void csczoom(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

s32 texmap(u32 a0, s16 d0w, s16 d1w)
{
    s32 DAT_0001cd3c = ((s32)d1w + (s32)d0w * xread16(a0 - 0x3ca)) * 2;
    return DAT_0001cd3c;
}

// Codopname no. 233 opcode 0xe8 ctexmap
static void ctexmap(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    alis.flagmain = 0;
    
    u32 addr = alis.script->vram_org;

    s16 offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        addr = alis.basemain;
    }
    
    addr += offset;

    readexec_opername_saveD7();
    alis.varD6 = alis.varD7;
    alis.varD7 = offset;
    
    u32 a0 = addr;
    
    addr += (alis.varD6 * 0x20) - 0xc00;
    
    readexec_opername();
    xwrite8(addr + 1, (char)alis.varD7);
    readexec_opername();
    if (0 < xread8(addr + 1))
    {
        xwrite32(addr + 4, adresdes(alis.varD7));
    }
    
    readexec_opername();
    xwrite16(addr + 10, alis.varD7);
    readexec_opername();
    xwrite8(addr + 8, (char)alis.varD7);
    xwrite8(addr + 9, (char)alis.varD7 + 1);
    readexec_opername();
    xwrite16(addr + 0xe, alis.varD7);
    readexec_opername();
    s16 uVar2 = alis.varD7;
    char cVar4 = (char)uVar2;
    xwrite8(addr + 0x14, cVar4);
    if (cVar4 == 0)
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
    }
    else if (cVar4 < 0)
    {
        readexec_opername();
        xwrite16(addr + 0x10, alis.varD7);
        readexec_opername();
        xwrite16(addr + 0x12, alis.varD7);
        readexec_opername();
        xwrite16(addr + 0x16, alis.varD7);
        readexec_opername();
        xwrite16(addr + 0x1a, alis.varD7);
        s32 iVar1 = texmap(a0, xread16(addr + 0x10), xread16(addr + 0x12));
        xwrite32(addr + 0x10, iVar1);
        if (iVar1 < 0)
        {
            xwrite8(addr + 0x14, 0);
        }
    }
    else if (cVar4 == 2)
    {
        readexec_opername();
        xwrite16(addr + 0x16, alis.varD7);
        readexec_opername();
        readexec_opername();
        readexec_opername();
        xwrite16(addr + 0x1a, alis.varD7);
    }
    else
    {
        readexec_opername();
        xwrite32(addr + 0x10, adresdes(alis.varD7));
        xwrite16(addr + 0x1c, xread16(alis.script->vram_org - 0xe));
        readexec_opername();
        xwrite16(addr + 0x16, alis.varD7);
        readexec_opername();
        xwrite8(addr + 0x15, (char)alis.varD7);
        readexec_opername();
        xwrite16(addr + 0x1a, alis.varD7);
    }
    
    // TODO: more code here in Falcon CD version
}

// Codopname no. 234 opcode 0xe9 calloctab
static void calloctab(void) {
    s16 offset = (s16)script_read16();
    if (offset == 0)
    {
        offset = (s16)script_read16();
    }
    
    s8 index = xread8(alis.basemain + offset - 1);
    if (index < 0)
    {
        index = (index & 0xf) * -4;
        s32 size = xread32(alis.basemain + offset - 6 + index);
        xwrite32(alis.basemain + offset, io_malloc(size));
    }
}

// Codopname no. 235 opcode 0xea cfreetab
static void cfreetab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 236 opcode 0xeb cscantab
static void cscantab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 237 opcode 0xec cneartab
static void cneartab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 238 opcode 0xed cscsun
static void cscsun(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 239 opcode 0xee cdarkpal
static void cdarkpal(void) {
    readexec_opername();
    s16 darkpar = alis.varD7;
    readexec_opername();
    s16 darkpav = alis.varD7;
    readexec_opername();
    s16 darkpab = alis.varD7;
    readexec_opername();
    s16 darkpac = alis.varD7;
    readexec_opername();
    s16 darkpan = alis.varD7;
    
    s16 val0 = 0x3c;
    s16 val1 = 0x3f;
    s16 val2 = 0x40;
    if (alis.platform.bpp == 8)
    {
        val0 = 0x100;
        val1 = 0xff;
        val2 = 0x100;
    }

    if (darkpac < val0)
    {
        s16 length = darkpac + darkpan;
        if (val1 < length)
        {
            length = val2;
        }
        
        length -= darkpac;

        for (int i = darkpac; i <= length; i++)
        {
            image.dkpalet[i * 3 + 0] = darkpar;
            image.dkpalet[i * 3 + 1] = darkpav;
            image.dkpalet[i * 3 + 2] = darkpab;
        }

        image.dkpalet[0] = 0;
        image.dkpalet[1] = 0;
        image.dkpalet[2] = 0;
        
        for (int i = 3; i < 0xb6; i++)
        {
            if (image.dkpalet[i] != 0x100)
            {
                image.fdarkpal = 1;
                return;
            }
        }

        image.fdarkpal = 0;
    }
}

// Codopname no. 240 opcode 0xef cscdark
static void cscdark(void) {
    alis.flagmain = 0;
    
    u32 mem = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
    
    readexec_opername();
    xwrite16(mem + 0x98, alis.varD7);
    
    readexec_opername();
    u32 sprite = adresdes(alis.varD7);
    xwrite32(mem + 0x9a, xread32(sprite) + sprite);

    readexec_opername();
    xwrite16(mem + 0x9e, alis.varD7);

    xwrite8(mem, xread8(mem) | 0x80);

    s16 uVar1 = xread16(xread32(mem + 0x9a) + 4) + 1;
    xwrite16(mem + 0xa0, uVar1);
    if (xread16(mem + 0x9e) < 0)
    {
        xwrite16(mem + 0x9e, uVar1 >> 1);
    }
    
    alis.basedark = xread16(mem + 0x9e);
    alis.ptrdark = xread32(mem + 0x9a) + 6;
}

// Codopname no. 241 opcode 0xf0 caset
static void caset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 242 opcode 0xf1 camov
static void camov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 243 opcode 0xf2 cscaset
static void cscaset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 244 opcode 0xf3 cscamov
static void cscamov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 245 opcode 0xf4 cscfollow
static void cscfollow(void) {
    u32 screen = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
    readexec_opername();
    xwrite16(screen + 0x60, alis.varD7);
    readexec_opername();
    xwrite8(screen + 0x84, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x86, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x88, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x8a, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x8c, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x8e, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x90, alis.varD7);
}

// Codopname no. 246 opcode 0xf5 cscview
static void cscview(void) {
    u32 screen = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
    readexec_opername();
    xwrite16(screen + 0x64, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x66, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x68, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x6a, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x6c, alis.varD7);
    readexec_opername();
    xwrite16(screen + 0x6e, alis.varD7);
    xwrite8(screen, xread8(screen) | 0x80);
}

// Codopname no. 247 opcode 0xf6 cfilm
static void cfilm(void) {
    memset(&bfilm, 0, sizeof(bfilm));
    readexec_opername();
    bfilm.id = alis.varD7;
    readexec_opername();
    s32 addr = adresdes(alis.varD7) + 0x20 + 4;
    bfilm.addr1 = alis.mem + addr;
    bfilm.addr2 = alis.mem + addr;
    readexec_opername();
    bfilm.waitclock = alis.varD7;
    bfilm.basemain = alis.basemain;
    inifilm();
    alis.varD7 = bfilm.frames;
    cstore_continue();
}

void calcnewax(s16 d4w)
{
    s16 newax = d4w - xread16(alis.script->vram_org + ALIS_SCR_WCAX);
    if (newax != 0)
    {
        newax >>= 2;
        if (newax < 0)
        {
            if (newax < -0xf)
                newax = -0xf;
        }
        else
        {
            if (newax == 0)
                newax = 1;

            if (0xf < newax)
                newax = 0xf;
        }
        
        newax += xread16(alis.script->vram_org + ALIS_SCR_WCAX);
        if (newax < 0x4c)
        {
            if (newax < -0x4b)
                newax = -0x4b;
        }
        else
        {
            newax = 0x4b;
        }
        
        xwrite16(alis.script->vram_org + ALIS_SCR_WCAX, newax);
    }
}

s16 unknownwm01[256];

s16 varwcx;
s16 varwcy;
s16 varwcz;
s16 varwcz2;
s32 unknownwm02;
s32 unknownwm03;
s16 walkmap1;

u8 walkmap3;
u16 walkmap4;

s16 varmatmask;

u16 atstmap(s32 addr, s32 vram, s16 wcx2, s16 wcy2, s16 wcz2);

s16 walkmap(s32 addr)
{
    alis.matmask = 0;
    if (varmatmask < 0)
    {
        alis.matmask = varmatmask;
    }
    
    s16 sVar4 = tabsin[xread16(alis.script->vram_org + ALIS_SCR_WCAZ)];
    s16 sVar5 = tabcos[xread16(alis.script->vram_org + ALIS_SCR_WCAZ)];
    s16 wcz2 = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCY2);
    if (varmatmask != 0)
    {
        unknownwm02 = (s32)-xread16(addr - 0x3f4) * sVar4 * 0x80;
        unknownwm03 = (s32) xread16(addr - 0x3f4) * sVar5 * 0x80;
    }
    
    s32 wcx = xread32(alis.script->vram_org + ALIS_SCR_WCX);
    s32 wcy = xread32(alis.script->vram_org + ALIS_SCR_WCY);

    s32 iVar1 = wcx + ((s32)sVar5 * (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2) - (s32)sVar4 * (s32)wcz2) * 0x80;
    s32 iVar2 = wcy + ((s32)sVar5 * (s32)wcz2 + (s32)sVar4 * (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2)) * 0x80;
    varwcz2 = xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    varwcx = (s16)((u32)iVar1 >> 0x10);
    sVar4 = varwcx;
    varwcy = (s16)((u32)iVar2 >> 0x10);
    sVar5 = varwcy;
    varwcx = iVar1;
    varwcy = iVar2;
    varwcz = atstmap(addr, alis.script->vram_org, sVar4, sVar5, wcz2);
    sVar4 = varwcz - varwcz2;
    if (alis.matmask != 0)
    {
        sVar5 = atstmap(addr, alis.script->vram_org,
                        (s16)((u32)(wcx + unknownwm03 + unknownwm02) >> 0x10),
                        (s16)((u32)(wcy + unknownwm02 + unknownwm03) >> 0x10), sVar4);
        if (0xfe < sVar5)
        {
            sVar4 = sVar5 - varwcz2;
            varwcz = sVar5;
        }
        
        sVar5 = atstmap(addr, alis.script->vram_org,
                        (s16)((u32)(wcx + (unknownwm02 - unknownwm03)) >> 0x10),
                        (s16)((u32)(wcy + (unknownwm03 - unknownwm02)) >> 0x10), sVar4);
        if (0xfe < sVar5)
        {
            sVar4 = sVar5 - varwcz2;
            varwcz = sVar5;
        }
        
        sVar5 = atstmap(addr, alis.script->vram_org,
                        (s16)((u32)(wcx + unknownwm02 * 2) >> 0x10),
                        (s16)((u32)(wcy + unknownwm03 * 2) >> 0x10),sVar4);
        if (0xfe < sVar5)
        {
            sVar4 = sVar5 - varwcz2;
            varwcz = sVar5;
        }
        
        sVar5 = atstmap(addr, alis.script->vram_org,
                        (s16)((u32)(wcx + unknownwm03 + unknownwm02 * 2) >> 0x10),
                        (s16)((u32)(wcy + unknownwm02 + unknownwm03 * 2) >> 0x10), sVar4);
        if (0xfe < sVar5)
        {
            sVar4 = sVar5 - varwcz2;
            varwcz = sVar5;
        }
        
        sVar5 = atstmap(addr, alis.script->vram_org,
                        (s16)((u32)(wcx + (unknownwm02 * 2 - unknownwm03)) >> 0x10),
                        (s16)((u32)(wcy + (unknownwm03 * 2 - unknownwm02)) >> 0x10), sVar4);
        if (0xfe < sVar5)
        {
            sVar4 = sVar5 - varwcz2;
            varwcz = sVar5;
        }
    }
    
    if (0 < varmatmask)
    {
        sVar5 = sVar4;
        if (sVar4 < 0)
        {
            sVar5 = -sVar4;
        }
        
        if (sVar5 <= walkmap1)
        {
            sVar5 = atstmap(addr, alis.script->vram_org,
                            (s16)((u32)(wcx + unknownwm02) >> 0x10),
                            (s16)((u32)(wcy + unknownwm03) >> 0x10), sVar4);
            sVar5 = ((s16)(sVar5) - varwcz2) * 4;
            sVar5 = atstmap(addr, alis.script->vram_org,
                            (s16)((u32)(wcx + unknownwm02 * 2) >> 0x10),
                            (s16)((u32)(wcy + unknownwm03 * 2) >> 0x10), sVar5);
            sVar4 = ((s16)(sVar5) - varwcz2) * 2;
            sVar5 = atstmap(addr, alis.script->vram_org,
                            (s16)((u32)(wcx + unknownwm02 * 4) >> 0x10),
                            (s16)((u32)(wcy + unknownwm03 * 4) >> 0x10), sVar4);
            u16 uVar6 = ((s16)(sVar5) - varwcz2) + sVar4 + sVar5;
            u16 uVar3 = uVar6;
            if ((s16)uVar6 < 0)
            {
                uVar3 = -uVar6;
            }
            
            if (0x3c0 < uVar3)
            {
                uVar6 = 0;
            }
            
            sVar5 = (s16)(((s32)(s16)uVar6 * (s32)(s16)varmatmask >> 2) / 0xc);
            sVar4 = sVar5;
            if (sVar5 < 0)
            {
                sVar4 = -sVar5;
            }
            
            if ((u8)sVar4 < walkmap3)
            {
                sVar5 = 0;
            }
            
            calcnewax(sVar5);
        }
    }
    
    u16 uVar6 = varwcz - varwcz2;
    if ((s32)((u32)uVar6 << 0x10) < 0)
    {
        uVar6 = -uVar6;
    }
    
    if (walkmap1 < (s16)uVar6)
    {
        sVar5 = atstmap(addr, alis.script->vram_org, xread16(alis.script->vram_org + ALIS_SCR_WCX), xread16(alis.script->vram_org + ALIS_SCR_WCY), sVar4);
        xwrite16(alis.script->vram_org + ALIS_SCR_WCZ, sVar5);
    }
    else
    {
        xwrite32(alis.script->vram_org + ALIS_SCR_WCX, varwcx);
        xwrite32(alis.script->vram_org + ALIS_SCR_WCY, varwcy);
        xwrite16(alis.script->vram_org + ALIS_SCR_WCZ, varwcz);
    }
    
    return varwcz - varwcz2;
}

// Codopname no. 248 opcode 0xf7 cwalkmap
static void cwalkmap(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    s16 offset = script_read16();
    u32 vram = alis.script->vram_org;
    if (offset == 0)
    {
        offset = script_read16();
        vram = alis.basemain;
    }
    
    readexec_opername();
    walkmap1 = alis.varD7;
    readexec_opername();
    varmatmask = alis.varD7;
    readexec_opername_saveD6();
    walkmap4 = alis.varD6;
    
    alis.varD7 = walkmap(vram + offset);
    
    cstore_continue();
}

void testaim(s16 *wcx2, s16 *wcy2, s16 *wcz2, s16 wcax, s16 wcaz)
{
    s32 sinax = tabsin[wcax];
    s32 sinaz = tabsin[wcaz];
    s32 cosax = tabcos[wcax];
    s32 cosaz = tabcos[wcaz];
    
    *wcz2 = (*wcy2 * sinax + *wcz2 * cosax) * 0x80;
    s16 tmpx2 = (cosaz * *wcx2 - (s16)(sinaz * cosax >> 9) * *wcy2) * 0x80;
    s16 tmpy2 = ((s16)(cosaz * cosax >> 9) * *wcy2 + sinaz * *wcx2) * 0x80;
    *wcx2 = tmpx2;
    *wcy2 = tmpy2;
}

u16 atstmap(s32 addr, s32 vram, s16 wcx2, s16 wcy2, s16 wcz2)
{
    u16 newd7 = 0;
    u16 result = 0;
    
    u16 valx = wcx2 >> ((u16)xread16(addr - 0x3c0) & 0x3f);
    u16 valy = wcy2 >> ((u16)xread16(addr - 0x3c0) & 0x3f);
    if ((valy <= (u16)xread16(addr - 0x292)) && (valx <= (u16)xread16(addr - 0x294)))
    {
        u32 newaddr = ((s32)(s16)valy + (s32)(s16)valy + *(s32 *)(unknownwm01 + (s16)(valx * 4)));
        newd7 = (u16)xread16(newaddr);
        result = newd7 & 0xff;
        
        s16 offset = ((newd7 & 0x3f00) >> 3) - 0xc00;
        s8 test = (s8)xread8(addr + 0x14 + offset);
        if (test != 0)
        {
            if (test < 0)
            {
                newaddr += xread32(addr + 0x10 + offset);
                s16 sVar3 = xread8(newaddr + 1) - result;
                offset = xread16(addr + 0x16 + offset) - sVar3;
                if ((sVar3 <= offset) || ((s16)(xread8(newaddr + 1) - offset) <= xread16(vram + ALIS_SCR_WCZ)))
                {
                    newd7 = (u16)xread16(newaddr);
                    result = newd7 & 0xff;
                }
            }
            else if ((alis.matmask != 0) && (xread16(addr + 0x1a + offset) != 0))
            {
                if ((s8)xread8(addr + 0x14 + offset) == 1)
                {
                    if (xread32(addr + 0x10 + offset) != 0)
                    {
                        result += 0xff;
                    }
                }
                else
                {
                    result += xread16(addr + 0x16 + offset);
                }
            }
        }
    }
    
    alis.varD7 = newd7;
    
    return result;
}

// Codopname no. 249 opcode 0xf8 catstmap
static void catstmap(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    s16 offset = script_read16();
    u32 addr = alis.script->vram_org;
    if (offset == 0)
    {
        offset = script_read16();
        addr = alis.basemain;
    }
    
    addr += offset;
    
    readexec_opername();
    s16 wcx2 = alis.varD7;
    readexec_opername();
    s16 wcy2 = alis.varD7;
    readexec_opername_saveD6();
    s16 wcz2 = alis.varD6;
    
    u32 vram = alis.script->vram_org;
    s16 wcax = xread16(vram + ALIS_SCR_WCAX);
    s16 wcaz = xread16(vram + ALIS_SCR_WCAZ);
    
    testaim(&wcx2, &wcy2, &wcz2, wcax, wcaz);
    
    alis.matmask = 0;
    
    atstmap(addr, vram, (s16)((u32)(xread32(vram + ALIS_SCR_WCX) + wcx2) >> 0x10),
                        (s16)((u32)(xread32(vram + ALIS_SCR_WCY) + wcy2) >> 0x10),
                        (s16)wcz2);
    
    cstore_continue();
}

// Codopname no. 250 opcode 0xf9 cavtstmov
static void cavtstmov(void) {
    u32 vram = alis.script->vram_org;

    s16 wcx2 = (s8)xread8(vram + ALIS_SCR_WCX2);
    s16 wcy2 = (s8)xread8(vram + ALIS_SCR_WCY2);
    s16 wcz2 = (s8)xread8(vram + ALIS_SCR_WCZ2);

    if (alis.platform.version >= 31)
    {
        debug(EDebugWarning, "CHECK: %s", __FUNCTION__);

        s16 wcax = xread16(vram + ALIS_SCR_WCAX);
        s16 wcaz = xread16(vram + ALIS_SCR_WCAZ);
        
        testaim(&wcx2, &wcy2, &wcz2, wcax, wcaz);

        alis.wcx = (xread32(vram + ALIS_SCR_WCX) + wcx2) >> 0x10;
        alis.wcy = (xread32(vram + ALIS_SCR_WCY) + wcy2) >> 0x10;
        alis.wcz = (xread32(vram + ALIS_SCR_WCZ) + wcz2) >> 0x10;
    }
    else
    {
        alis.wcx = xread32(vram + ALIS_SCR_WCX) + wcx2;
        alis.wcy = xread32(vram + ALIS_SCR_WCY) + wcy2;
        alis.wcz = xread32(vram + ALIS_SCR_WCZ) + wcz2;
    }

    readexec_opername();
    alis.matmask = alis.varD7;
    alis.wforme = get_0x1a_cforme(vram);
    clipform();
    crstent();
}

// Codopname no. 251 opcode 0xfa cavmov
static void cavmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void getangle(s16 *atx, s16 *aty)
{
    s16 uVar2;
    s16 uVar3;

    s16 natx;
    s16 naty = *aty;
    if (atx == 0)
    {
getangle3:
        natx = 0;
        if (*aty == 0)
        {
            naty = 0;
        }
        else if (*aty < 0)
        {
            natx = 0xb4;
        }
    }
    else
    {
        uVar2 = *atx;
        if (*atx < 0)
        {
            uVar2 = -(*atx);
        }
        
        if (aty != 0)
        {
            uVar3 = *aty;
            if (aty < 0)
            {
                uVar3 = -(*aty);
            }
            
            if (uVar3 < uVar2)
            {
                natx = 0x5a;
                uVar2 = (((s32)uVar2 << 5) / uVar3);
                if (0x73f < uVar2 || (natx = tabatan[uVar2]) != 0)
                {
                    if (aty < 0)
                    {
                        natx = 0xb4 - natx;
                    }
                    
                    if (-1 < *atx)
                    {
                        natx = -natx;
                    }
                    
                    naty = (*atx * 0x200) / tabsin[natx] & 0xffff;
                    goto getangle4;
                }
                
                goto getangle3;
            }
            
            natx = 0x5a;
            uVar2 = (((s32)uVar3 << 5) / uVar2);
            if (0x73f < uVar2 || (natx = tabatan[uVar2]) != 0)
            {
                if (aty < 0)
                {
                    natx = -natx;
                }
                
                natx -= 0x5a;
                if (*atx < 0)
                {
                    natx = -natx;
                }
                
                naty = (*aty * 0x200) / tabcos[natx] & 0xffff;
                goto getangle4;
            }
        }
        
        naty = *atx;
        natx = 0xffa6;
        if (*atx < 0)
        {
            natx = 0x5a;
        }
    }
    
getangle4:
    
    if (naty < 0)
    {
        naty = -naty;
    }
    
    *atx = natx;
    *aty = naty;
}

void polarang(s16 *cx, s16 *cy, s16 *cz, s16 *oldcx, s16 *oldcy, s16 *oldcz)
{
    *cx -= *oldcx;
    *cy -= *oldcy;
    *cz -= *oldcz;
    
    getangle(cx, cy);
    
    s16 ncx = 0;

    if (*cz != 0)
    {
        ncx = -(*cz);
        getangle(&ncx, cy);
    }
    
    *cz = *cy;
    *cy = ncx;
}

// Codopname no. 252 opcode 0xfb caim
static void caim(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    // TODO: on Atari ST values for the first call should be: 0x19e0, 0x560, 0x5 but aren't
    // we should call it at afe12 NOT on aff24
    // INVESTIGATE!!!

    readexec_opername();
    s16 cx = alis.varD7;
    readexec_opername();
    s16 cy = alis.varD7;
    readexec_opername();
    s16 cz = alis.varD7;

//    s16 cx = 0x19e0;
//    s16 cy = 0x560;
//    s16 cz = 0x5;
    
    s16 oldcx = xread16(alis.script->vram_org + ALIS_SCR_WCX);
    s16 oldcy = xread16(alis.script->vram_org + ALIS_SCR_WCY);
    s16 oldcz = xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    
    polarang(&cx, &cy, &cz, &oldcx, &oldcy, &oldcz);
    
    if (cz == 0)
    {
        cx = xread16(alis.script->vram_org + ALIS_SCR_WCAZ);
    }
    
    readexec_opername();
    u16 uVar4 = alis.varD7;
    
    if (0xb3 >= uVar4)
    {
        cy = xread16(alis.script->vram_org + ALIS_SCR_WCAZ);
        
        u16 uVar1 = cx - cy;
        if ((s16)uVar1 < 0)
        {
            if ((s16)uVar1 < -0xb3)
            {
                uVar1 += 0x168;
                if ((s16)uVar1 < 0)
                {
                    if ((s16)uVar1 <= (s16)-uVar4)
                        uVar1 = -uVar4;
                    
                    cx = cy + uVar1;
                }
                    
                if ((s16)uVar4 <= (s16)uVar1)
                    uVar1 = uVar4;
                
                cx = cy + uVar1;
            }
            else
            {
                if ((s16)uVar1 <= (s16)-uVar4)
                    uVar1 = -uVar4;
                
                cx = cy + uVar1;
            }
        }
        else
        {
            if (0xb3 < (s16)uVar1)
            {
                uVar1 -= 0x168;
                if ((s16)uVar1 < 0)
                {
                    if ((s16)uVar1 <= (s16)-uVar4)
                        uVar1 = -uVar4;
                    
                    cx = cy + uVar1;
                }
            }
            else
            {
                if ((s16)uVar4 <= (s16)uVar1)
                {
                    uVar1 = uVar4;
                }
                
                cx = cy + uVar1;
            }
        }
    }
    
    if (cx < 0x168)
    {
        if (cx < -0x167)
        {
            cx += 0x168;
        }
    }
    else
    {
        cx += -0x168;
    }
    
    alis.varD7 = cy;
    cstore_continue();
    
    alis.varD7 = cx;
    cstore_continue();
    
    alis.varD7 = cz;
    cstore_continue();
}

// Codopname no. 253 opcode 0xfc cpointpix
static void cpointpix(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 254 opcode 0xfd cchartmap
static void cchartmap(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    // TODO: verify and clean
    
    u32 vram = alis.script->vram_org;

    s16 offset = script_read16();
    if (offset == 0)
    {
        vram = alis.basemain;
        offset = script_read16();
    }

    u32 addr = vram + offset;

    readexec_opername();
    s16 value = alis.varD7;
    
    s32 iVar4;
    if (value == 0)
    {
        iVar4 = 0;
    }
    else
    {
        if (value != 1)
        {
            if ((value == 2) || (value == 3))
            {
                xwrite16(vram - 0x3b6, value);
                readexec_opername();
                return;
            }
            
            if (value == 4)
            {
                xwrite16(vram - 0x3b4, value);
                readexec_opername();
                return;
            }
            
            if (value == 5)
            {
                xwrite16(vram - 0x3b2, value);
                readexec_opername();
                return;
            }
            
            readexec_opername();
            iVar4 = alis.varD7;
            
            value --;
            if (value < 0)
            {
                value = 0;
            }
            
            s32 iVar1 = xread32(addr - 0x3ba);
            s32 iVar3 = (s16)(xread16(alis.script->vram_org) - value) / xread16(iVar1 - 0x3f4);
            s16 outval0 = (s16)iVar3;
            if (iVar3 < 0)
            {
                outval0 = 0;
            }
            
            if (xread16(iVar1 - 0x3e8) <= outval0)
            {
                outval0 = xread16(iVar1 - 0x3e8);
            }
            
            iVar3 = (s16)(xread16(alis.script->vram_org) + value) / xread16(iVar1 - 0x3f4);
            s16 outval1 = (s16)iVar3;
            if (iVar3 < 0)
            {
                outval1 = 0;
            }
            
            if (xread16(iVar1 - 0x3e8) <= outval1)
            {
                outval1 = xread16(iVar1 - 0x3e8);
            }
            
            iVar3 = (s16)(xread16(alis.script->vram_org + 8) - value) / xread16(iVar1 - 0x3f2);
            s16 outval2 = (s16)iVar3;
            if (iVar3 < 0)
            {
                outval2 = 0;
            }
            
            if (xread16(iVar1 - 0x3e6) <= outval2)
            {
                outval2 = xread16(iVar1 - 0x3e6);
            }
            
            iVar3 = (s16)(xread16(alis.script->vram_org + 8) + value) / xread16(iVar1 - 0x3f2);
            s16 outval3 = (s16)iVar3;
            if (iVar3 < 0)
            {
                outval3 = 0;
            }
            
            if (xread16(iVar1 - 0x3e6) <= outval3)
            {
                outval3 = xread16(iVar1 - 0x3e6);
            }
            
            u32 rot = ((xread16(addr - 0x3c0) - 4) - xread16(iVar1 - 0x3c0)) & 0x3f;
            u16 outval0u = outval0 >> rot;
            u16 outval1u = outval1 >> rot;
            rot = (xread16(addr - 0x3be) - xread16(iVar1 - 0x3be)) & 0x3f;
            outval2 >>= rot;
            outval3 = (outval3 >> rot) - outval2;
            u16 uVar7 = ~(0xffffU >> (outval0u - (outval0u & 0xfff0) & 0x3f));
            u16 count = outval1u | 0xf;
            outval1u = ~(-1 << (-(outval1u - count) & 0x3f));
            count = (u16)(count - (outval0u & 0xfff0)) >> 4;
            if (count <= 0)
            {
                uVar7 |= outval1u;
                outval1u = 0;
            }
            
            if (outval1u != 0)
            {
                count--;
            }
            
            u32 addr2 = addr;
            if ((s8)xread8(addr - 1) < 0)
            {
                addr2 = xread32(addr);
                addr2 = xread32(addr2);
            }
            
            addr2 += outval2 * 2 + (outval0u >> 4) * xread16(addr - 0x3c4);
            s16 offset2 = xread16(addr - 0x3c4);
            
            do
            {
                xwrite16(addr2, xread16(addr2) & uVar7);
                u32 addr3 = addr2 + offset2;

                for (int i = 0; i < count; i++)
                {
                    xwrite16(addr3, 0);
                    addr3 = addr3 + offset2;
                }
                
                if (outval1u != 0)
                {
                    xwrite16(addr3, xread16(addr3) & outval1u);
                }
                
                addr2++;
                if ((outval3 --) == -1)
                {
                    return;
                }
            }
            while (true);
        }
        
        iVar4 = -1;
    }
    
    s16 loops = ((s16)(xread16(addr - 0x3c6) * xread16(addr - 0x3c4)) >> 1);
    if ((s8)xread8(addr - 1) < 0)
    {
        addr = xread32(addr);
        addr = xread32(addr);
    }

    for (int i = 0; i < loops; i++, addr += 2)
    {
        xwrite16(addr, (s16)iVar4);
    }
    
    vram = alis.script->vram_org;
    offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        vram = alis.basemain;
    }
    
    xwrite32(addr - 0x3ba, vram + offset);
}

// Codopname no. 255 opcode 0xfe cscsky
static void cscsky(void) {
    alis.flagmain = 0;
    
    u32 mem = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
    
    readexec_opername();
    xwrite16(mem + 0xa2, alis.varD7);

    readexec_opername();
    xwrite32(mem + 0xa4, adresdes(alis.varD7));

    readexec_opername();
    xwrite16(mem + 0xa8, alis.varD7);
    
    u32 sprite = xread32(mem + 0xa4);
    u32 spdata = xread32(sprite) + sprite;
    if (xread8(spdata) == 3)
    {
        sprite = sprite + xread16(spdata + 2) * 4;
        spdata = xread32(sprite) + sprite;
    }

    xwrite16(mem + 0xa8, (((((((xread16(mem + 0x6c) * 4) >> 3) * 0x168) / (xread16(spdata + 2) + 1)) + 0x80U & 0xff00) * (xread16(spdata + 2) + 1)) / 0x168));
    xwrite8(mem, xread8(mem) | 0x80);
}

// Codopname no. 256 opcode 0xff czoom
static void czoom(void) {
    debug(EDebugWarning, "I3 SPECIFIC ?: %s", __FUNCTION__);
    
//    readexec_opername();
//    s16 tmp0 = alis.varD7;
//    alis.script->context->_0x36 = tmp0;
    
//    readexec_opername();
//    s16 tmp1 = alis.varD7;
//    alis.script->context->_0x38 = tmp1;
}

// Codopname no. 055 opcode 0x36 cclock
static void cclock(void)    {
    readexec_opername();
    set_0x3a_wait_cycles(alis.script->vram_org, alis.varD7);
    set_0x3e_wait_time(alis.script->vram_org, alis.varD7 + alis.timeclock);
}

// Codopname no. 002 opcode 0x01 cesc1
static void cesc1(void)     {
//    u16 code = script_read8() | 0x100;
//    sAlisOpcode opcode = opcodes[code];
//    debug(EDebugInfo, " %s", opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
//    return opcode.fptr();
    readexec_codesc1name();
}

// Codopname no. 003 opcode 0x02 cesc2
// The function is not yet found in ALIS interpreters (no confirmation that the codesc1 table
// has been completed), restored for a reason. This does not affect the code if it does not exist.
static void cesc2(void)     {
    debug(EDebugWarning, "CHECK: ", __FUNCTION__);
    readexec_codesc2name();
}

// Codopname no. 004 opcode 0x03 cesc3
// The function is not yet found in ALIS interpreters (no confirmation that tables codesc1 and codesc2
// have been completed), restored for a reason. This does not affect the code if it does not exist.
static void cesc3(void)     {
    debug(EDebugWarning, "CHECK: ", __FUNCTION__);
    readexec_codesc3name();
}

// ============================================================================
#pragma mark - Stub routines
// ============================================================================

// ALIS uses two types of stubs.
// The first, called cnul, is put in place of missing functions (opcodes)
// in tables tcodop and tcodesc1. It does nothing.
//
// The second, unnamed one (we called it pnul), has address 0x0000 and
// is put in place of missing functions (opcodes) in tables toper, tstore and tadd.
// It outputs a message that a null pointer has been called and then exits the program.
//
// As the engine evolved, opcodes that were sent to these stubs in earlier versions
// could get their own implementation and a new name in later ones.

// Codopname no. 001 opcode 0x00 -> cnul
// Codopname no. 056 opcode 0x37 -> cnul
// Codopname no. 140 opcode 0x8b -> cnul
// Codesc1name no. 01 opcode 0x00 -> cnul
void cnul(void)      {
    debug(EDebugWarning, "WARNING: NULL opcode called");
}

// Opername no. 31 opcode 0x3c -> pnul
// Opername no. 32 opcode 0x3e -> pnul
// Storename no. 01 opcode 0x00 -> pnul
// Storename no. 02 opcode 0x02 -> pnul
// Storename no. 03 opcode 0x04 -> pnul
// Addname no. 01 opcode 0x00 -> pnul
// Addname no. 02 opcode 0x02 -> pnul
// Addname no. 03 opcode 0x04 -> pnul
void pnul(void)      {
    debug(EDebugFatal, "\nERROR: NULL code pointer called\n");
    if (!VM_IGNORE_ERRORS) {
        debug(EDebugFatal, " The ALIS VM has been stopped.");
        alis.running = 0;
    }
}

// ============================================================================
#pragma mark - Unimplemented codops
// ============================================================================

// Codopname no. 005 opcode 0x04 cbreakpt
static void cbreakpt(void)  {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 035 opcode 0x22 cmul
static void cmul(void)      {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 036 opcode 0x23 cdiv
static void cdiv(void)      {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 012 opcode 0x0b cjsrabs
static void cjsrabs(void)   {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 013 opcode 0x0c cjmpabs
static void cjmpabs(void)   {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 014 opcode 0x0d cjsrind16
static void cjsrind16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 015 opcode 0x0e cjsrind24
static void cjsrind24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 016 opcode 0x0f cjmpind16
static void cjmpind16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 017 opcode 0x10 cjmpind24
static void cjmpind24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}


// ============================================================================
#pragma mark - Flow control - Subroutines
// ============================================================================

// Codopname no. 018 opcode 0x11 cret
static void cret(void) {
    
    if (alis.fseq == 0 && alis.saversp <= alis.script->vacc_off)
    {
        cstop();
    }
    else
    {
        alis.script->pc = alis.script->pc_org + xpeek32();
        if (alis.script->vacc_off == get_0x0c_vacc_offset(alis.script->vram_org))
        {
            set_0x0c_vacc_offset(alis.script->vram_org, 0);
        }

        if (alis.script->vacc_off >= -0x34)
        {
            debug(EDebugError, " VACC out of bounds!!! ");
        }
        
        alis.script->vacc_off += sizeof(s32);
    }
}

static void cjsr(s32 offset) {

    // save return **OFFSET**, not ADDRESS
    // TODO: dans la vm originale on empile la 'vraie' adresse du PC en 32 bits
    // Là on est en 64 bits, donc j'empile l'offset
    // TODO: peut-on stocker une adresse de retour *virtuelle* ?
    // Sinon ça oblige à créer une pile virtuelle d'adresses
    //   dont la taille est platform-dependent
    xpush32((u32)(alis.script->pc - alis.script->pc_org));
    script_jump(offset);
}

// Codopname no. 006 opcode 0x05 cjsr8
static void cjsr8(void) {
    // read byte, extend sign
    s16 offset = (s8)script_read8();

    if(DEBUG_SCRIPT) {
         if (offset<0) {
             debug(EDebugInfo, " [call up -0x%02x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [call dn +0x%02x]", offset);
         }
       }

    cjsr(offset);
}

// Codopname no. 007 opcode 0x06 cjsr16
static void cjsr16(void) {
    s16 offset = script_read16();

    if(DEBUG_SCRIPT) {
         if (offset<0) {
             debug(EDebugInfo, " [call up -0x%04x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [call dn +0x%04x]", offset);
         }
       }

    cjsr(offset);
}

// Codopname no. 008 opcode 0x07 cjsr24
static void cjsr24(void) {
    s32 offset = script_read24();

    if(DEBUG_SCRIPT) {
         if (offset<0) {
             debug(EDebugInfo, " [call up -0x%06x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [call dn +0x%06x]", offset);
         }
       }

    cjsr(offset);
}


// ============================================================================
#pragma mark - Flow control - Jump
// ============================================================================

// Codopname no. 009 opcode 0x08 cjmp8
static void cjmp8(void) {
    s16 offset = (s8)script_read8();

    if(DEBUG_SCRIPT) {
         if (offset<0) {
             debug(EDebugInfo, " [jmp up -0x%02x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [jmp dn +0x%02x]", offset);
         }
       }

    script_jump(offset);
}

// Codopname no. 010 opcode 0x09 cjmp16
static void cjmp16(void) {
    s16 offset = script_read16();

    if(DEBUG_SCRIPT) {
         if (offset<0) {
             debug(EDebugInfo, " [jmp up -0x%04x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [jmp dn +0x%04x]", offset);
         }
       }

    script_jump(offset);
}

// Codopname no. 011 opcode 0x0a cjmp24
static void cjmp24(void) {
    s32 offset = script_read24();

    if(DEBUG_SCRIPT) {
         if (offset<0) {
             debug(EDebugInfo, " [jmp up -0x%06x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [jmp dn +0x%06x]", offset);
         }
       }

    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if zero
// ============================================================================

// Codopname no. 019 opcode 0x12 cbz8
static void cbz8(void) {
    s16 offset = alis.varD7 ? 1 : (s8)script_read8();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 != 0) {
           debug(EDebugInfo, " [d7<>0 => no jmp +1]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7==0 => jmp up -0x%02x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7==0 => jmp dn +0x%02x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 020 opcode 0x13 cbz16
static void cbz16(void) {
    s16 offset = alis.varD7 ? 2 : script_read16();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 != 0) {
           debug(EDebugInfo, " [d7<>0 => no jmp +2]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7==0 => jmp up -0x%04x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7==0 => jmp dn +0x%04x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 021 opcode 0x14 cbz24
static void cbz24(void) {
    s32 offset = alis.varD7 ? 3 : script_read24();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 != 0) {
           debug(EDebugInfo, " [d7<>0 => no jmp +3]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7==0 => jmp up -0x%06x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7==0 => jmp dn +0x%06x]", offset);
         }
       }
    }

    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if non-zero
// ============================================================================

// Codopname no. 022 opcode 0x15 cbnz8
static void cbnz8(void) {
    s16 offset = alis.varD7 == 0 ? 1 : (s8)script_read8();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 == 0) {
           debug(EDebugInfo, " [d7==0 => no jmp +1]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7<>0 => jmp up -0x%02x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7<>0 => jmp dn +0x%02x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 023 opcode 0x16 cbnz16
static void cbnz16(void) {
    s16 offset = alis.varD7 == 0 ? 2 : script_read16();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 == 0) {
           debug(EDebugInfo, " [d7==0 => no jmp +2]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7<>0 => jmp up -0x%04x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7<>0 => jmp dn +0x%04x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 024 opcode 0x17 cbnz24
static void cbnz24(void) {
    s32 offset = alis.varD7 == 0 ? 3 : script_read24();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 == 0) {
           debug(EDebugInfo, " [d7==0 => no jmp +3]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7<>0 => jmp up -0x%06x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7<>0 => jmp dn +0x%06x]", offset);
         }
       }
    }

    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if equal
// ============================================================================

// Codopname no. 025 opcode 0x18 cbeq8
static void cbeq8(void) {
    s16 offset = alis.varD7 == alis.varD6 ? 1 : (s8)script_read8();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 == alis.varD6) {
           debug(EDebugInfo, " [d7==d6 => no jmp +1]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7<>d6 => jmp up -0x%02x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7<>d6 => jmp dn +0x%02x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 026 opcode 0x19 cbeq16
static void cbeq16(void) {
    s16 offset = alis.varD7 == alis.varD6 ? 2 : script_read16();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 == alis.varD6) {
           debug(EDebugInfo, " [d7==d6 => no jmp +2]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7<>d6 => jmp up -0x%04x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7<>d6 => jmp dn +0x%04x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 027 opcode 0x1a cbeq24
static void cbeq24(void) {
    s32 offset = alis.varD7 == alis.varD6 ? 3 : script_read24();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 == alis.varD6) {
           debug(EDebugInfo, " [d7==d6 => no jmp +3]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7<>d6 => jmp up -0x%06x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7<>d6 => jmp dn +0x%06x]", offset);
         }
       }
    }

    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if not equal
// ============================================================================

// Codopname no. 028 opcode 0x1b cbne8
static void cbne8(void) {
    s16 offset = alis.varD7 != alis.varD6 ? 1 : (s8)script_read8();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 != alis.varD6) {
           debug(EDebugInfo, " [d7<>d6 => no jmp +1]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7==d6 => jmp up -0x%02x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7==d6 => jmp dn +0x%02x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 029 opcode 0x1c cbne16
static void cbne16(void) {
    s16 offset = alis.varD7 != alis.varD6 ? 2 : script_read16();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 != alis.varD6) {
           debug(EDebugInfo, " [d7<>d6 => no jmp +2]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7==d6 => jmp up -0x%04x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7==d6 => jmp dn +0x%04x]", offset);
         }
       }
    }

    script_jump(offset);
}

// Codopname no. 030 opcode 0x1d cbne24
static void cbne24(void) {
    s32 offset = alis.varD7 != alis.varD6 ? 3 : script_read24();

    if(DEBUG_SCRIPT) {
       if (alis.varD7 != alis.varD6) {
           debug(EDebugInfo, " [d7<>d6 => no jmp +3]");
       }
       else {
         if (offset<0) {
             debug(EDebugInfo, " [d7==d6 => jmp up -0x%06x]", abs(offset));
         }
         else {
             debug(EDebugInfo, " [d7==d6 => jmp dn +0x%06x]", offset);
         }
       }
    }

    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Start
// ============================================================================
static void cstart(s32 offset) {
    if (alis.fseq == 0)
    {
        if (get_0x0c_vacc_offset(alis.script->vram_org) == 0)
        {
            s16 vacc_offset = get_0x0a_vacc_offset(alis.script->vram_org) - 4;
            set_0x0c_vacc_offset(alis.script->vram_org, vacc_offset);
            set_0x0a_vacc_offset(alis.script->vram_org, vacc_offset);
            xwrite32(alis.script->vram_org + vacc_offset, (u32)(get_0x08_script_ret_offset(alis.script->vram_org) - alis.script->pc_org));
            set_0x08_script_ret_offset(alis.script->vram_org, offset + alis.script->pc);
        }
        else
        {
            set_0x0a_vacc_offset(alis.script->vram_org, get_0x0c_vacc_offset(alis.script->vram_org));
            set_0x08_script_ret_offset(alis.script->vram_org, offset + alis.script->pc);
        }
        
        set_0x04_cstart_csleep(alis.script->vram_org, 1);
        set_0x01_wait_count(alis.script->vram_org, 1);
        
        if (alis.platform.version >= 30)
        {
            set_0x3e_wait_time(alis.script->vram_org, 0);
        }
    }
    else
    {
        if (get_0x0c_vacc_offset(alis.script->vram_org) == 0)
        {
            xpush32((u32)(alis.script->pc - alis.script->pc_org));
            set_0x0c_vacc_offset(alis.script->vram_org, alis.script->vacc_off);
        }
        else
        {
            alis.script->vacc_off = get_0x0c_vacc_offset(alis.script->vram_org);
        }

        alis.script->pc += offset;
    }
}

// Codopname no. 049 opcode 0x30 cstart8
static void cstart8(void) {
    // read byte, extend sign to word, then to long
    cstart((s8)script_read8());
}

// Codopname no. 050 opcode 0x31 cstart16
static void cstart16(void) {
    cstart((s16)script_read16());
}

// Codopname no. 051 opcode 0x32 cstart24
static void cstart24(void) {
    cstart(script_read24());
}

// ============================================================================
#pragma mark - Additions
// ============================================================================

void killent(u16 killent)
{
    // make sure music playback is stopped
    
    sAlisScriptLive *script = ENTSCR(killent);
    if (script->data->header.id == audio.musicId)
    {
        audio.working = 1;

        for (int count = 0; count < 1000 && audio.muflag; count++)
        {
            usleep(1000);
        }

        audio.muflag = 0;
        audio.musicId = 0xffff;

        while (audio.working)
        {
            usleep(1000);
        }
    }
    
    s32 vram = xread32(alis.atent + killent);
    if (vram == 0)
    {
        alis.ferase = 0;
        return;
    }
    
    alis.script = ENTSCR(killent);
    cerasall();
    alis.script = ENTSCR(alis.varD5);

    s32 shrinkby = xread32(vram - 0x14);
    s32 contextsize = get_context_size();
    u32 datasize = xread16(shrinkby + 0x16) + xread16(shrinkby + 0x12) + contextsize;
    u32 target = vram - datasize;
    shrinkby = xread16(shrinkby + 0x14) + datasize;
    u32 source = target + shrinkby;

    // copy work mem to freed space
    
    do
    {
        *(u16 *)(alis.mem + target) = *(u16 *)(alis.mem + source);
        target += 2;
        source += 2;
    }
    while (source < alis.finent);

    alis.finent -= shrinkby;
    
    u16 curent = 0;
    u16 prevent = 0;
    u16 nextent;

    while (true)
    {
        nextent = xread16(alis.atent + 4 + prevent);
        if (killent == nextent)
        {
            curent = prevent;
        }
        
        if (nextent == 0)
        {
            break;
        }
        
        prevent = nextent;
        if (vram <= xread32(alis.atent + nextent))
        {
            sAlisScriptLive *curscript = ENTSCR(nextent);
            curscript->vram_org -= shrinkby;
            xwrite32(alis.atent + nextent, xread32(alis.atent + nextent) - shrinkby);
        }
    }

    xwrite16(alis.atent + curent + 4, xread16(alis.atent + 4 + killent));
    xwrite16(alis.atent + killent + 4, alis.dernent);
    xwrite32(alis.atent + killent, 0);
    alis.dernent = killent;
    alis.nbent --;

    // if we deleted currently running script, set previous script and restart loop
    if (alis.varD5 == killent)
    {
        alis.varD5 = curent;
        alis.restart_loop = 1;
        cstop();
    }
}

void shrinkprog(s32 start, s32 length, u16 id)
{
    u8 *target = alis.mem;
    u8 *source = alis.mem + length;
    
    // copy scripts to freed space
    for (s32 i = start; i < alis.finprog - length; i++)
    {
        target[i] = source[i];
    }
    
    debug(EDebugInfo, "\nFreeing range %.6x - %.6x", start, start + length);

    alis.finprog -= length;
    
    if (id != 0)
    {
        s8 idx = -1;
        for (s32 i = 0; i < alis.nbprog; i++)
        {
            sAlisScriptData *scr = alis.loaded_scripts[i];
            if (scr->header.id == id)
            {
                idx = i;
                debug(EDebugInfo, "\nRemoved prog: %s at: %.6x ", scr->name, scr->data_org);
            }
            else
            {
                debug(EDebugInfo, "\nSkipped prog: %s at: %.6x ", scr->name, scr->data_org);
            }

            if (idx > 0)
            {
                alis.loaded_scripts[i] = i < alis.maxprog ? alis.loaded_scripts[i + 1] : 0;
            }
        }

        u8 found = 0;
        for (s32 i = 0; i < alis.nbprog; i++)
        {
            s32 location = alis.atprog_ptr[i];
            if (start == location)
            {
                found = 1;
            }
            
            if (found)
            {
                alis.atprog_ptr[i] = i < alis.maxprog ? alis.atprog_ptr[i + 1] : 0;
            }
        }

        alis.dernprog -= 4;
        alis.nbprog --;
    }

    debug(EDebugInfo, "\nShifting range %.6x - %.6x", alis.atprog, alis.dernprog);

    for (int i = 0; i < alis.nbprog; i++)
    {
        sAlisScriptData *script = alis.loaded_scripts[i];
        debug(EDebugInfo, "\nChecking prog: %s at: %.6x ", script->name, script->data_org);
            
        if (start <= script->data_org)
        {
            debug(EDebugInfo, "pre-shrinked: %.6x", script->data_org);

            script->data_org -= length;
            alis.atprog_ptr[i] -= length;
            debug(EDebugInfo, "shrinked to: %.6x", script->data_org);
        }
        else
        {
            debug(EDebugInfo, "OK");
        }
    }

    debug(EDebugInfo, "\n");

    if (id != 0)
    {
        s32 contextsize = get_context_size();

        u32 entidx = 0;
        u32 prevent = 0;
        
        while ((entidx = xread16(alis.atent + 4 + entidx)))
        {
            sAlisScriptLive *script = ENTSCR(entidx);
            debug(EDebugInfo, "\nChecking script: %s at: %.6x ", script->name, script->data->data_org);

            if (id == get_0x10_script_id(script->vram_org))
            {
                killent(entidx);
                entidx = prevent; //alis.varD5;
                debug(EDebugInfo, "removed");
            }
            else if (start <= get_0x14_script_org_offset(script->vram_org))
            {
                script->pc -= length;
                script->pc_org -= length;
                set_0x14_script_org_offset(script->vram_org, get_0x14_script_org_offset(script->vram_org) - length);
                set_0x08_script_ret_offset(script->vram_org, get_0x08_script_ret_offset(script->vram_org) - length);
                
                u32 org_offset = get_0x14_script_org_offset(script->vram_org);
                script->vacc_off = -contextsize - xread16(org_offset + 0x16);
                debug(EDebugInfo, " [va %.4x]", (s16)alis.script->vacc_off);

                // NOTE: no longer needed, we are calculating proper value using script start location
//                while (get_0x0a_vacc_offset(script->vram_org) < script->vacc_off)
//                {
//                    script->vacc_off -= 4;
//                    xsub32(script->vram_org + script->vacc_off, length);
//                    debug(EDebugInfo, " [%.8x => va %.4x + %.6x (%.6x)]", xread32(script->vram_org + script->vacc_off), (s16)script->vacc_off, script->vram_org, script->vacc_off + script->vram_org);
//                }
            }
            else
            {
                debug(EDebugInfo, "OK");
            }
            
            prevent = entidx;
        }
    }
    
    debug(EDebugInfo, "\n");

    // move addresses to sprite data to match actual new locations
    
    for (s16 scidx = screen.ptscreen; scidx != 0; scidx = get_scr_to_next(scidx))
    {
        s16 spridx = get_scr_screen_id(scidx);
        if (spridx != 0)
        {
            while ((spridx = SPRITE_VAR(spridx)->link) != 0)
            {
                sSprite *sprite = SPRITE_VAR(spridx);
                if (start < sprite->newad)
                    sprite->newad -= length;

                if (start < sprite->data)
                    sprite->data -= length;
            }
        }
    }
}

s32 io_malloc(s32 rawsize)
{
    s32 size = (rawsize - 1U | 3) + 9;
    s32 blockloc = alis.finmem - size;
    if (alis.finprog < blockloc)
    {
        alis.finmem = blockloc;
        xwrite32(blockloc, size);

        if (alis.platform.version < 30)
        {
            return blockloc;
        }
        
        for (int i = 0; i < 0xf; i++)
        {
            if (xread32(alis.tabptr + i * 8) == 0)
            {
                xwrite32(alis.tabptr + i * 8 + 0, blockloc + 8);
                xwrite32(alis.tabptr + i * 8 + 4, blockloc + 8);

                // NOTE: silmarils originaly used 32 bit pointer to pointer table allocated in (alis.mem) block
                return alis.tabptr + i * 8;
            }
        }
    }
    
    return 0;
}

void io_mfree(s32 addr)
{
    s32 blockloc = xread32(addr + 4);
    xwrite32(addr + 0, 0);
    xwrite32(addr + 4, 0);
    
    if (alis.finmem == blockloc - 8)
    {
        alis.finmem += xread32(alis.finmem);
    }
}

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines pointer table (256 values)
// ============================================================================
sAlisOpcode opcodes[] = {
    DECL_OPCODE(0x00, cnul,         "null"),
    DECL_OPCODE(0x01, cesc1,        "TODO: add desc"),
    DECL_OPCODE(0x02, cesc2,        "TODO: add desc"),
    DECL_OPCODE(0x03, cesc3,        "TODO: add desc"),
    DECL_OPCODE(0x04, cbreakpt,     "TODO: add desc"),
    DECL_OPCODE(0x05, cjsr8,        "jump to sub-routine with 8-bit offset"),
    DECL_OPCODE(0x06, cjsr16,       "jump to sub-routine with 16-bit offset"),
    DECL_OPCODE(0x07, cjsr24,       "jump to sub-routine with 24-bit offset"),
    DECL_OPCODE(0x08, cjmp8,        "jump (8-bit offset)"),
    DECL_OPCODE(0x09, cjmp16,       "jump (16-bit offset)"),
    DECL_OPCODE(0x0a, cjmp24,       "jump (24-bit offset)"),
    DECL_OPCODE(0x0b, cjsrabs,      "[N/I] jump to sub-routine w/ absolute addr"),
    DECL_OPCODE(0x0c, cjmpabs,      "[N/I] jump to absolute addr"),
    DECL_OPCODE(0x0d, cjsrind16,    "[N/I] jump to sub-routine w/ indirect (16-bit offset)"),
    DECL_OPCODE(0x0e, cjsrind24,    "[N/I] jump to sub-routine w/ indirect (24-bit offset)"),
    DECL_OPCODE(0x0f, cjmpind16,    "[N/I] jump w/ indirect (16-bit offset)"),
    DECL_OPCODE(0x10, cjmpind24,    "[N/I] jump w/ indirect (24-bit offset)"),
    DECL_OPCODE(0x11, cret,         "return from sub-routine"),
    DECL_OPCODE(0x12, cbz8,         "branch if zero with 8-bit offset"),
    DECL_OPCODE(0x13, cbz16,        "branch if zero with 16-bit offset"),
    DECL_OPCODE(0x14, cbz24,        "branch if zero with 24-bit offset"),
    DECL_OPCODE(0x15, cbnz8,        "branch if non-zero with 8-bit offset"),
    DECL_OPCODE(0x16, cbnz16,       "branch if non-zero with 16-bit offset"),
    DECL_OPCODE(0x17, cbnz24,       "branch if non-zero with 24-bit offset"),
    DECL_OPCODE(0x18, cbeq8,        "branch if equal with 8-bit offset"),
    DECL_OPCODE(0x19, cbeq16,       "branch if equal with 16-bit offset"),
    DECL_OPCODE(0x1a, cbeq24,       "branch if equal with 24-bit offset"),
    DECL_OPCODE(0x1b, cbne8,        "branch if non-equal with 8-bit offset"),
    DECL_OPCODE(0x1c, cbne16,       "branch if non-equal with 16-bit offset"),
    DECL_OPCODE(0x1d, cbne24,       "branch if non-equal with 24-bit offset"),
    DECL_OPCODE(0x1e, cstore,       "store expression"),
    DECL_OPCODE(0x1f, ceval,        "start expression evaluation"),
    DECL_OPCODE(0x20, cadd,         "TODO: add desc"),
    DECL_OPCODE(0x21, csub,         "TODO: add desc"),
    DECL_OPCODE(0x22, cmul,         "[N/I]"),
    DECL_OPCODE(0x23, cdiv,         "[N/I]"),
    DECL_OPCODE(0x24, cvprint,      "TODO: add desc"),
    DECL_OPCODE(0x25, csprinti,     "TODO: add desc"),
    DECL_OPCODE(0x26, csprinta,     "TODO: add desc"),
    DECL_OPCODE(0x27, clocate,      "TODO: add desc"),
    DECL_OPCODE(0x28, ctab,         "TODO: add desc"),
    DECL_OPCODE(0x29, cdim,         "TODO: add desc"),
    DECL_OPCODE(0x2a, crandom,      "generate a random number"),
    DECL_OPCODE(0x2b, cloop8,       "TODO: add desc"),
    DECL_OPCODE(0x2c, cloop16,      "TODO: add desc"),
    DECL_OPCODE(0x2d, cloop24,      "TODO: add desc"),
    DECL_OPCODE(0x2e, cswitch1,     "TODO: add desc"),
    DECL_OPCODE(0x2f, cswitch2,     "TODO: add desc"),
    DECL_OPCODE(0x30, cstart8,      "TODO: add desc"),
    DECL_OPCODE(0x31, cstart16,     "TODO: add desc"),
    DECL_OPCODE(0x32, cstart24,     "TODO: add desc"),
    DECL_OPCODE(0x33, cleave,       "TODO: add desc"),
    DECL_OPCODE(0x34, cprotect,     "TODO: add desc"),
    DECL_OPCODE(0x35, casleep,      "TODO: add desc"),
    DECL_OPCODE(0x36, cclock,       "TODO: add desc"),
    DECL_OPCODE(0x37, cnul,         "null"),
    DECL_OPCODE(0x38, cscmov,       "TODO: add desc"),
    DECL_OPCODE(0x39, cscset,       "TODO: add desc"),
    DECL_OPCODE(0x3a, cclipping,    "TODO: add desc"),
    DECL_OPCODE(0x3b, cswitching,   "TODO: add desc"),
    DECL_OPCODE(0x3c, cwlive,       "TODO: add desc"),
    DECL_OPCODE(0x3d, cunload,      "TODO: add desc"),
    DECL_OPCODE(0x3e, cwakeup,      "TODO: add desc"),
    DECL_OPCODE(0x3f, csleep,       "TODO: add desc"),
    DECL_OPCODE(0x40, clive,        "TODO: add desc"),
    DECL_OPCODE(0x41, ckill,        "TODO: add desc"),
    DECL_OPCODE(0x42, cstop,        "TODO: add desc"),
    DECL_OPCODE(0x43, cstopret,     "TODO: add desc"),
    DECL_OPCODE(0x44, cexit,        "TODO: add desc"),
    DECL_OPCODE(0x45, cload,        "Load and depack a script, set into vm"),
    DECL_OPCODE(0x46, cdefsc,       "Define Scene ??"),
    DECL_OPCODE(0x47, cscreen,      "TODO: add desc"),
    DECL_OPCODE(0x48, cput,         "TODO: add desc"),
    DECL_OPCODE(0x49, cputnat,      "TODO: add desc"),
    DECL_OPCODE(0x4a, cerase,       "TODO: add desc"),
    DECL_OPCODE(0x4b, cerasen,      "TODO: add desc"),
    DECL_OPCODE(0x4c, cset,         "TODO: add desc"),
    DECL_OPCODE(0x4d, cmov,         "TODO: add desc"),
    DECL_OPCODE(0x4e, copensc,      "TODO: add desc"),
    DECL_OPCODE(0x4f, cclosesc,     "TODO: add desc"),
    DECL_OPCODE(0x50, cerasall,     "TODO: add desc"),
    DECL_OPCODE(0x51, cforme,       "TODO: add desc"),
    DECL_OPCODE(0x52, cdelforme,    "TODO: add desc"),
    DECL_OPCODE(0x53, ctstmov,      "TODO: add desc"),
    DECL_OPCODE(0x54, ctstset,      "TODO: add desc"),
    DECL_OPCODE(0x55, cftstmov,     "TODO: add desc"),
    DECL_OPCODE(0x56, cftstset,     "TODO: add desc"),
    DECL_OPCODE(0x57, csuccent,     "TODO: add desc"),
    DECL_OPCODE(0x58, cpredent,     "TODO: add desc"),
    DECL_OPCODE(0x59, cnearent,     "TODO: add desc"),
    DECL_OPCODE(0x5a, cneartyp,     "TODO: add desc"),
    DECL_OPCODE(0x5b, cnearmat,     "TODO: add desc"),
    DECL_OPCODE(0x5c, cviewent,     "TODO: add desc"),
    DECL_OPCODE(0x5d, cviewtyp,     "TODO: add desc"),
    DECL_OPCODE(0x5e, cviewmat,     "TODO: add desc"),
    DECL_OPCODE(0x5f, corient,      "TODO: add desc"),
    DECL_OPCODE(0x60, crstent,      "TODO: add desc"),
    DECL_OPCODE(0x61, csend,        "TODO: add desc"),
    DECL_OPCODE(0x62, cscanon,      "TODO: add desc"),
    DECL_OPCODE(0x63, cscanoff,     "TODO: add desc"),
    DECL_OPCODE(0x64, cinteron,     "TODO: add desc"),
    DECL_OPCODE(0x65, cinteroff,    "TODO: add desc"),
    DECL_OPCODE(0x66, cscanclr,     "TODO: add desc"),
    DECL_OPCODE(0x67, callentity,   "TODO: add desc"),
    DECL_OPCODE(0x68, cpalette,     "TODO: add desc"),
    DECL_OPCODE(0x69, cdefcolor,    "TODO: add desc"),
    DECL_OPCODE(0x6a, ctiming,      "TODO: add desc"),
    DECL_OPCODE(0x6b, czap,         "TODO: add desc"),
    DECL_OPCODE(0x6c, cexplode,     "TODO: add desc"),
    DECL_OPCODE(0x6d, cding,        "TODO: add desc"),
    DECL_OPCODE(0x6e, cnoise,       "TODO: add desc"),
    DECL_OPCODE(0x6f, cinitab,      "TODO: add desc"),
    DECL_OPCODE(0x70, cfopen,       "TODO: add desc"),
    DECL_OPCODE(0x71, cfclose,      "TODO: add desc"),
    DECL_OPCODE(0x72, cfcreat,      "TODO: add desc"),
    DECL_OPCODE(0x73, cfdel,        "TODO: add desc"),
    DECL_OPCODE(0x74, cfreadv,      "TODO: add desc"),
    DECL_OPCODE(0x75, cfwritev,     "TODO: add desc"),
    DECL_OPCODE(0x76, cfwritei,     "TODO: add desc"),
    DECL_OPCODE(0x77, cfreadb,      "TODO: add desc"),
    DECL_OPCODE(0x78, cfwriteb,     "TODO: add desc"),
    DECL_OPCODE(0x79, cplot,        "TODO: add desc"),
    DECL_OPCODE(0x7a, cdraw,        "TODO: add desc"),
    DECL_OPCODE(0x7b, cbox,         "TODO: add desc"),
    DECL_OPCODE(0x7c, cboxf,        "TODO: add desc"),
    DECL_OPCODE(0x7d, cink,         "TODO: add desc"),
    DECL_OPCODE(0x7e, cpset,        "TODO: add desc"),
    DECL_OPCODE(0x7f, cpmove,       "TODO: add desc"),
    DECL_OPCODE(0x80, cpmode,       "TODO: add desc"),
    DECL_OPCODE(0x81, cpicture,     "TODO: add desc"),
    DECL_OPCODE(0x82, cxyscroll,    "TODO: add desc"),
    DECL_OPCODE(0x83, clinking,     "TODO: add desc"),
    DECL_OPCODE(0x84, cmouson,      "display mouse cursor"),
    DECL_OPCODE(0x85, cmousoff,     "hide mouse cursor"),
    DECL_OPCODE(0x86, cmouse,       "get mouse status (x, y, buttons) and store"),
    DECL_OPCODE(0x87, cdefmouse,    "TODO: define mouse sprite ???"),
    DECL_OPCODE(0x88, csetmouse,    "set mouse position"),
    DECL_OPCODE(0x89, cdefvect,     "TODO: add desc"),
    DECL_OPCODE(0x8a, csetvect,     "TODO: add desc"),
    DECL_OPCODE(0x8b, cnul,         "null"),
    DECL_OPCODE(0x8c, capproach,    "TODO: add desc"),
    DECL_OPCODE(0x8d, cescape,      "TODO: add desc"),
    DECL_OPCODE(0x8e, cvtstmov,     "TODO: add desc"),
    DECL_OPCODE(0x8f, cvftstmov,    "TODO: add desc"),
    DECL_OPCODE(0x90, cvmov,        "TODO: add desc"),
    DECL_OPCODE(0x91, cdefworld,    "TODO: add desc"),
    DECL_OPCODE(0x92, cworld,       "TODO: add desc"),
    DECL_OPCODE(0x93, cfindmat,     "TODO: add desc"),
    DECL_OPCODE(0x94, cfindtyp,     "TODO: add desc"),
    DECL_OPCODE(0x95, cmusic,       "TODO: add desc"),
    DECL_OPCODE(0x96, cdelmusic,    "TODO: add desc"),
    DECL_OPCODE(0x97, ccadence,     "TODO: add desc"),
    DECL_OPCODE(0x98, csetvolum,    "TODO: add desc"),
    DECL_OPCODE(0x99, cxinv,        "TODO: add desc"),
    DECL_OPCODE(0x9a, cxinvon,      "TODO: add desc"),
    DECL_OPCODE(0x9b, cxinvoff,     "TODO: add desc"),
    DECL_OPCODE(0x9c, clistent,     "TODO: add desc"),
    DECL_OPCODE(0x9d, csound,       "TODO: add desc"),
    DECL_OPCODE(0x9e, cmsound,      "TODO: add desc"),
    DECL_OPCODE(0x9f, credon,       "TODO: add desc"),
    DECL_OPCODE(0xa0, credoff,      "TODO: add desc"),
    DECL_OPCODE(0xa1, cdelsound,    "TODO: add desc"),
    DECL_OPCODE(0xa2, cwmov,        "TODO: add desc"),
    DECL_OPCODE(0xa3, cwtstmov,     "TODO: add desc"),
    DECL_OPCODE(0xa4, cwftstmov,    "TODO: add desc"),
    DECL_OPCODE(0xa5, ctstform,     "TODO: add desc"),
    DECL_OPCODE(0xa6, cxput,        "TODO: add desc"),
    DECL_OPCODE(0xa7, cxputat,      "TODO: add desc"),
    DECL_OPCODE(0xa8, cmput,        "TODO: add desc"),
    DECL_OPCODE(0xa9, cmputat,      "TODO: add desc"),
    DECL_OPCODE(0xaa, cmxput,       "TODO: add desc"),
    DECL_OPCODE(0xab, cmxputat,     "TODO: add desc"),
    DECL_OPCODE(0xac, cmmusic,      "TODO: add desc"),
    DECL_OPCODE(0xad, cmforme,      "TODO: add desc"),
    DECL_OPCODE(0xae, csettime,     "set current time"),
    DECL_OPCODE(0xaf, cgettime,     "get current time"),
    DECL_OPCODE(0xb0, cvinput,      "TODO: add desc"),
    DECL_OPCODE(0xb1, csinput,      "TODO: add desc"),
    DECL_OPCODE(0xb2, casleepfar,   "TODO: add desc"),
    DECL_OPCODE(0xb3, casleepon,    "TODO: add desc"),
    DECL_OPCODE(0xb4, casleepoff,   "TODO: add desc"),
    DECL_OPCODE(0xb5, crunfilm,     "TODO: add desc"),
    DECL_OPCODE(0xb6, cvpicprint,   "TODO: add desc"),
    DECL_OPCODE(0xb7, cspicprint,   "TODO: add desc"),
    DECL_OPCODE(0xb8, cvputprint,   "TODO: add desc"),
    DECL_OPCODE(0xb9, csputprint,   "TODO: add desc"),
    DECL_OPCODE(0xba, cfont,        "TODO: add desc"),
    DECL_OPCODE(0xbb, cpaper,       "TODO: add desc"),
    DECL_OPCODE(0xbc, ctoblack,     "fade-out screen to black"),
    DECL_OPCODE(0xbd, cmovcolor,    "TODO: add desc"),
    DECL_OPCODE(0xbe, ctopalet,     "fade-in screen to palette"),
    DECL_OPCODE(0xbf, cnumput,      "TODO: add desc"),
    DECL_OPCODE(0xc0, cscheart,     "TODO: add desc"),
    DECL_OPCODE(0xc1, cscpos,       "TODO: add desc"),
    DECL_OPCODE(0xc2, cscsize,      "TODO: add desc"),
    DECL_OPCODE(0xc3, cschoriz,     "TODO: add desc"),
    DECL_OPCODE(0xc4, cscvertic,    "TODO: add desc"),
    DECL_OPCODE(0xc5, cscreduce,    "TODO: add desc"),
    DECL_OPCODE(0xc6, cscscale,     "TODO: add desc"),
    DECL_OPCODE(0xc7, creducing,    "TODO: add desc"),
    DECL_OPCODE(0xc8, cscmap,       "TODO: add desc"),
    DECL_OPCODE(0xc9, cscdump,      "TODO: add desc"),
    DECL_OPCODE(0xca, cfindcla,     "TODO: add desc"),
    DECL_OPCODE(0xcb, cnearcla,     "TODO: add desc"),
    DECL_OPCODE(0xcc, cviewcla,     "TODO: add desc"),
    DECL_OPCODE(0xcd, cinstru,      "TODO: add desc"),
    DECL_OPCODE(0xce, cminstru,     "TODO: add desc"),
    DECL_OPCODE(0xcf, cordspr,      "TODO: add desc"),
    DECL_OPCODE(0xd0, calign,       "TODO: add desc"),
    DECL_OPCODE(0xd1, cbackstar,    "TODO: add desc"),
    DECL_OPCODE(0xd2, cstarring,    "TODO: add desc"),
    DECL_OPCODE(0xd3, cengine,      "TODO: add desc"),
    DECL_OPCODE(0xd4, cautobase,    "TODO: add desc"),
    DECL_OPCODE(0xd5, cquality,     "TODO: add desc"),
    DECL_OPCODE(0xd6, chsprite,     "TODO: add desc"),
    DECL_OPCODE(0xd7, cselpalet,    "TODO: add desc"),
    DECL_OPCODE(0xd8, clinepalet,   "TODO: add desc"),
    DECL_OPCODE(0xd9, cautomode,    "TODO: add desc"),
    DECL_OPCODE(0xda, cautofile,    "TODO: add desc"),
    DECL_OPCODE(0xdb, ccancel,      "TODO: add desc"),
    DECL_OPCODE(0xdc, ccancall,     "TODO: add desc"),
    DECL_OPCODE(0xdd, ccancen,      "TODO: add desc"),
    DECL_OPCODE(0xde, cblast,       "TODO: add desc"),
    DECL_OPCODE(0xdf, cscback,      "TODO: add desc"),
    DECL_OPCODE(0xe0, cscrolpage,   "TODO: add desc"),
    DECL_OPCODE(0xe1, cmatent,      "TODO: add desc"),
    DECL_OPCODE(0xe2, cshrink,      "Delete bitmap and shift following data"),
    DECL_OPCODE(0xe3, cdefmap,      "TODO: add desc"),
    DECL_OPCODE(0xe4, csetmap,      "TODO: add desc"),
    DECL_OPCODE(0xe5, cputmap,      "TODO: add desc"),
    DECL_OPCODE(0xe6, csavepal,     "TODO: add desc"),
    DECL_OPCODE(0xe7, csczoom,      "TODO: add desc"),
    DECL_OPCODE(0xe8, ctexmap,      "TODO: add desc"),
    DECL_OPCODE(0xe9, calloctab,    "TODO: add desc"),
    DECL_OPCODE(0xea, cfreetab,     "TODO: add desc"),
    DECL_OPCODE(0xeb, cscantab,     "TODO: add desc"),
    DECL_OPCODE(0xec, cneartab,     "TODO: add desc"),
    DECL_OPCODE(0xed, cscsun,       "TODO: add desc"),
    DECL_OPCODE(0xee, cdarkpal,     "TODO: add desc"),
    DECL_OPCODE(0xef, cscdark,      "TODO: add desc"),
    DECL_OPCODE(0xf0, caset,        "TODO: add desc"),
    DECL_OPCODE(0xf1, camov,        "TODO: add desc"),
    DECL_OPCODE(0xf2, cscaset,      "TODO: add desc"),
    DECL_OPCODE(0xf3, cscamov,      "TODO: add desc"),
    DECL_OPCODE(0xf4, cscfollow,    "TODO: add desc"),
    DECL_OPCODE(0xf5, cscview,      "TODO: add desc"),
    DECL_OPCODE(0xf6, cfilm,        "TODO: add desc"),
    DECL_OPCODE(0xf7, cwalkmap,     "TODO: add desc"),
    DECL_OPCODE(0xf8, catstmap,     "TODO: add desc"),
    DECL_OPCODE(0xf9, cavtstmov,    "TODO: add desc"),
    DECL_OPCODE(0xfa, cavmov,       "TODO: add desc"),
    DECL_OPCODE(0xfb, caim,         "TODO: add desc"),
    DECL_OPCODE(0xfc, cpointpix,    "TODO: add desc"),
    DECL_OPCODE(0xfd, cchartmap,    "TODO: add desc"),
    DECL_OPCODE(0xfe, cscsky,       "TODO: add desc"),
    DECL_OPCODE(0xff, czoom,        "TODO: add desc")
};
