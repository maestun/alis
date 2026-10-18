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

// VM-core translation unit: opt into hot-state register pinning (see alis.h).
#define ALIS_VM_CORE
#include "alis.h"
#include "audio.h"
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "deus3d.h"
#include "rrq3d.h"
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

s16 tables[] = {
    
    0x0000, 0x0008, 0x0011, 0x001A, 0x0023, 0x002C, 0x0035, 0x003E, 0x0047, 0x0050, 0x0058, 0x0061, 0x006A, 0x0073, 0x007B, 0x0084, 0x008D, 0x0095, 0x009E, 0x00A6, 0x00AF, 0x00B7, 0x00BF, 0x00C8, 0x00D0, 0x00D8, 0x00E0, 0x00E8, 0x00F0, 0x00F8, 0x0100, 0x0107, 0x010F, 0x0116, 0x011E,
    0x0125, 0x012C, 0x0134, 0x013B, 0x0142, 0x0149, 0x014F, 0x0156, 0x015D, 0x0163, 0x016A, 0x0170, 0x0176, 0x017C, 0x0182, 0x0188, 0x018D, 0x0193, 0x0198, 0x019E, 0x01A3, 0x01A8, 0x01AD, 0x01B2, 0x01B6, 0x01BB, 0x01BF, 0x01C4, 0x01C8, 0x01CC, 0x01D0, 0x01D3, 0x01D7, 0x01DA, 0x01DD,
    0x01E1, 0x01E4, 0x01E6, 0x01E9, 0x01EC, 0x01EE, 0x01F0, 0x01F2, 0x01F4, 0x01F6, 0x01F8, 0x01F9, 0x01FB, 0x01FC, 0x01FD, 0x01FE, 0x01FE, 0x01FF, 0x01FF, 0x01FF, 0x0200, 0x01FF, 0x01FF, 0x01FF, 0x01FE, 0x01FE, 0x01FD, 0x01FC, 0x01FB, 0x01F9, 0x01F8, 0x01F6, 0x01F4, 0x01F2, 0x01F0,
    0x01EE, 0x01EC, 0x01E9, 0x01E6, 0x01E4, 0x01E1, 0x01DD, 0x01DA, 0x01D7, 0x01D3, 0x01D0, 0x01CC, 0x01C8, 0x01C4, 0x01BF, 0x01BB, 0x01B6, 0x01B2, 0x01AD, 0x01A8, 0x01A3, 0x019E, 0x0198, 0x0193, 0x018D, 0x0188, 0x0182, 0x017C, 0x0176, 0x0170, 0x016A, 0x0163, 0x015D, 0x0156, 0x014F,
    0x0149, 0x0142, 0x013B, 0x0134, 0x012C, 0x0125, 0x011E, 0x0116, 0x010F, 0x0107, 0x0100, 0x00F8, 0x00F0, 0x00E8, 0x00E0, 0x00D8, 0x00D0, 0x00C8, 0x00BF, 0x00B7, 0x00AF, 0x00A6, 0x009E, 0x0095, 0x008D, 0x0084, 0x007B, 0x0073, 0x006A, 0x0061, 0x0058, 0x0050, 0x0047, 0x003E, 0x0035,
    0x002C, 0x0023, 0x001A, 0x0011, 0x0008, 0x0000, 0xFFF8, 0xFFEF, 0xFFE6, 0xFFDD, 0xFFD4, 0xFFCB, 0xFFC2, 0xFFB9, 0xFFB0, 0xFFA8, 0xFF9F, 0xFF96, 0xFF8D, 0xFF85, 0xFF7C, 0xFF73, 0xFF6B, 0xFF62, 0xFF5A, 0xFF51, 0xFF49, 0xFF41, 0xFF38, 0xFF30, 0xFF28, 0xFF20, 0xFF18, 0xFF10, 0xFF08,
    0xFF00, 0xFEF9, 0xFEF1, 0xFEEA, 0xFEE2, 0xFEDB, 0xFED4, 0xFECC, 0xFEC5, 0xFEBE, 0xFEB7, 0xFEB1, 0xFEAA, 0xFEA3, 0xFE9D, 0xFE96, 0xFE90, 0xFE8A, 0xFE84, 0xFE7E, 0xFE78, 0xFE73, 0xFE6D, 0xFE68, 0xFE62, 0xFE5D, 0xFE58, 0xFE53, 0xFE4E, 0xFE4A, 0xFE45, 0xFE41, 0xFE3C, 0xFE38, 0xFE34,
    0xFE30, 0xFE2D, 0xFE29, 0xFE26, 0xFE23, 0xFE1F, 0xFE1C, 0xFE1A, 0xFE17, 0xFE14, 0xFE12, 0xFE10, 0xFE0E, 0xFE0C, 0xFE0A, 0xFE08, 0xFE07, 0xFE05, 0xFE04, 0xFE03, 0xFE02, 0xFE02, 0xFE01, 0xFE01, 0xFE01, 0xFE00, 0xFE01, 0xFE01, 0xFE01, 0xFE02, 0xFE02, 0xFE03, 0xFE04, 0xFE05, 0xFE07,
    0xFE08, 0xFE0A, 0xFE0C, 0xFE0E, 0xFE10, 0xFE12, 0xFE14, 0xFE17, 0xFE1A, 0xFE1C, 0xFE1F, 0xFE23, 0xFE26, 0xFE29, 0xFE2D, 0xFE30, 0xFE34, 0xFE38, 0xFE3C, 0xFE41, 0xFE45, 0xFE4A, 0xFE4E, 0xFE53, 0xFE58, 0xFE5D, 0xFE62, 0xFE68, 0xFE6D, 0xFE73, 0xFE78, 0xFE7E, 0xFE84, 0xFE8A, 0xFE90,
    0xFE96, 0xFE9D, 0xFEA3, 0xFEAA, 0xFEB1, 0xFEB7, 0xFEBE, 0xFEC5, 0xFECC, 0xFED4, 0xFEDB, 0xFEE2, 0xFEEA, 0xFEF1, 0xFEF9, 0xFF00, 0xFF08, 0xFF10, 0xFF18, 0xFF20, 0xFF28, 0xFF30, 0xFF38, 0xFF41, 0xFF49, 0xFF51, 0xFF5A, 0xFF62, 0xFF6B, 0xFF73, 0xFF7C, 0xFF85, 0xFF8D, 0xFF96, 0xFF9F,
    0xFFA8, 0xFFB0, 0xFFB9, 0xFFC2, 0xFFCB, 0xFFD4, 0xFFDD, 0xFFE6, 0xFFEF, 0xFFF8, 0x0000, 0x0008, 0x0011, 0x001A, 0x0023, 0x002C, 0x0035, 0x003E, 0x0047, 0x0050, 0x0058, 0x0061, 0x006A, 0x0073, 0x007B, 0x0084, 0x008D, 0x0095, 0x009E, 0x00A6, 0x00AF, 0x00B7, 0x00BF, 0x00C8, 0x00D0,
    0x00D8, 0x00E0, 0x00E8, 0x00F0, 0x00F8, 0x0100, 0x0107, 0x010F, 0x0116, 0x011E, 0x0125, 0x012C, 0x0134, 0x013B, 0x0142, 0x0149, 0x014F, 0x0156, 0x015D, 0x0163, 0x016A, 0x0170, 0x0176, 0x017C, 0x0182, 0x0188, 0x018D, 0x0193, 0x0198, 0x019E, 0x01A3, 0x01A8, 0x01AD, 0x01B2, 0x01B6,
    0x01BB, 0x01BF, 0x01C4, 0x01C8, 0x01CC, 0x01D0, 0x01D3, 0x01D7, 0x01DA, 0x01DD, 0x01E1, 0x01E4, 0x01E6, 0x01E9, 0x01EC, 0x01EE, 0x01F0, 0x01F2, 0x01F4, 0x01F6, 0x01F8, 0x01F9, 0x01FB, 0x01FC, 0x01FD, 0x01FE, 0x01FE, 0x01FF, 0x01FF, 0x01FF, 0x0200, 0x01FF, 0x01FF, 0x01FF, 0x01FE,
    0x01FE, 0x01FD, 0x01FC, 0x01FB, 0x01F9, 0x01F8, 0x01F6, 0x01F4, 0x01F2, 0x01F0, 0x01EE, 0x01EC, 0x01E9, 0x01E6, 0x01E4, 0x01E1, 0x01DD, 0x01DA, 0x01D7, 0x01D3, 0x01D0, 0x01CC, 0x01C8, 0x01C4, 0x01BF, 0x01BB, 0x01B6, 0x01B2, 0x01AD, 0x01A8, 0x01A3, 0x019E, 0x0198, 0x0193, 0x018D,
    0x0188, 0x0182, 0x017C, 0x0176, 0x0170, 0x016A, 0x0163, 0x015D, 0x0156, 0x014F, 0x0149, 0x0142, 0x013B, 0x0134, 0x012C, 0x0125, 0x011E, 0x0116, 0x010F, 0x0107, 0x0100, 0x00F8, 0x00F0, 0x00E8, 0x00E0, 0x00D8, 0x00D0, 0x00C8, 0x00BF, 0x00B7, 0x00AF, 0x00A6, 0x009E, 0x0095, 0x008D,
    0x0084, 0x007B, 0x0073, 0x006A, 0x0061, 0x0058, 0x0050, 0x0047, 0x003E, 0x0035, 0x002C, 0x0023, 0x001A, 0x0011, 0x0008, 0x0000, 0xFFF8, 0xFFEF, 0xFFE6, 0xFFDD, 0xFFD4, 0xFFCB, 0xFFC2, 0xFFB9, 0xFFB0, 0xFFA8, 0xFF9F, 0xFF96, 0xFF8D, 0xFF85, 0xFF7C, 0xFF73, 0xFF6B, 0xFF62, 0xFF5A,
    0xFF51, 0xFF49, 0xFF41, 0xFF38, 0xFF30, 0xFF28, 0xFF20, 0xFF18, 0xFF10, 0xFF08, 0xFF00, 0xFEF9, 0xFEF1, 0xFEEA, 0xFEE2, 0xFEDB, 0xFED4, 0xFECC, 0xFEC5, 0xFEBE, 0xFEB7, 0xFEB1, 0xFEAA, 0xFEA3, 0xFE9D, 0xFE96, 0xFE90, 0xFE8A, 0xFE84, 0xFE7E, 0xFE78, 0xFE73, 0xFE6D, 0xFE68, 0xFE62,
    0xFE5D, 0xFE58, 0xFE53, 0xFE4E, 0xFE4A, 0xFE45, 0xFE41, 0xFE3C, 0xFE38, 0xFE34, 0xFE30, 0xFE2D, 0xFE29, 0xFE26, 0xFE23, 0xFE1F, 0xFE1C, 0xFE1A, 0xFE17, 0xFE14, 0xFE12, 0xFE10, 0xFE0E, 0xFE0C, 0xFE0A, 0xFE08, 0xFE07, 0xFE05, 0xFE04, 0xFE03, 0xFE02, 0xFE02, 0xFE01, 0xFE01, 0xFE01,
    0xFE00, 0xFE01, 0xFE01, 0xFE01, 0xFE02, 0xFE02, 0xFE03, 0xFE04, 0xFE05, 0xFE07, 0xFE08, 0xFE0A, 0xFE0C, 0xFE0E, 0xFE10, 0xFE12, 0xFE14, 0xFE17, 0xFE1A, 0xFE1C, 0xFE1F, 0xFE23, 0xFE26, 0xFE29, 0xFE2D, 0xFE30, 0xFE34, 0xFE38, 0xFE3C, 0xFE41, 0xFE45, 0xFE4A, 0xFE4E, 0xFE53, 0xFE58,
    0xFE5D, 0xFE62, 0xFE68, 0xFE6D, 0xFE73, 0xFE78, 0xFE7E, 0xFE84, 0xFE8A, 0xFE90, 0xFE96, 0xFE9D, 0xFEA3, 0xFEAA, 0xFEB1, 0xFEB7, 0xFEBE, 0xFEC5, 0xFECC, 0xFED4, 0xFEDB, 0xFEE2, 0xFEEA, 0xFEF1, 0xFEF9, 0xFF00, 0xFF08, 0xFF10, 0xFF18, 0xFF20, 0xFF28, 0xFF30, 0xFF38, 0xFF41, 0xFF49,
    0xFF51, 0xFF5A, 0xFF62, 0xFF6B, 0xFF73, 0xFF7C, 0xFF85, 0xFF8D, 0xFF96, 0xFF9F, 0xFFA8, 0xFFB0, 0xFFB9, 0xFFC2, 0xFFCB, 0xFFD4, 0xFFDD, 0xFFE6, 0xFFEF, 0xFFF8, 0x0000, 0x0008, 0x0011, 0x001A, 0x0023, 0x002C, 0x0035, 0x003E, 0x0047, 0x0050, 0x0058, 0x0061, 0x006A, 0x0073, 0x007B,
    0x0084, 0x008D, 0x0095, 0x009E, 0x00A6, 0x00AF, 0x00B7, 0x00BF, 0x00C8, 0x00D0, 0x00D8, 0x00E0, 0x00E8, 0x00F0, 0x00F8, 0x0100, 0x0107, 0x010F, 0x0116, 0x011E, 0x0125, 0x012C, 0x0134, 0x013B, 0x0142, 0x0149, 0x014F, 0x0156, 0x015D, 0x0163, 0x016A, 0x0170, 0x0176, 0x017C, 0x0182,
    0x0188, 0x018D, 0x0193, 0x0198, 0x019E, 0x01A3, 0x01A8, 0x01AD, 0x01B2, 0x01B6, 0x01BB, 0x01BF, 0x01C4, 0x01C8, 0x01CC, 0x01D0, 0x01D3, 0x01D7, 0x01DA, 0x01DD, 0x01E1, 0x01E4, 0x01E6, 0x01E9, 0x01EC, 0x01EE, 0x01F0, 0x01F2, 0x01F4, 0x01F6, 0x01F8, 0x01F9, 0x01FB, 0x01FC, 0x01FD,
    0x01FE, 0x01FE, 0x01FF, 0x01FF, 0x01FF, 0x0200, 0x01FF, 0x01FF, 0x01FF, 0x01FE, 0x01FE, 0x01FD, 0x01FC, 0x01FB, 0x01F9, 0x01F8, 0x01F6, 0x01F4, 0x01F2, 0x01F0, 0x01EE, 0x01EC, 0x01E9, 0x01E6, 0x01E4, 0x01E1, 0x01DD, 0x01DA, 0x01D7, 0x01D3, 0x01D0, 0x01CC, 0x01C8, 0x01C4, 0x01BF,
    0x01BB, 0x01B6, 0x01B2, 0x01AD, 0x01A8, 0x01A3, 0x019E, 0x0198, 0x0193, 0x018D, 0x0188, 0x0182, 0x017C, 0x0176, 0x0170, 0x016A, 0x0163, 0x015D, 0x0156, 0x014F, 0x0149, 0x0142, 0x013B, 0x0134, 0x012C, 0x0125, 0x011E, 0x0116, 0x010F, 0x0107, 0x0100, 0x00F8, 0x00F0, 0x00E8, 0x00E0,
    0x00D8, 0x00D0, 0x00C8, 0x00BF, 0x00B7, 0x00AF, 0x00A6, 0x009E, 0x0095, 0x008D, 0x0084, 0x007B, 0x0073, 0x006A, 0x0061, 0x0058, 0x0050, 0x0047, 0x003E, 0x0035, 0x002C, 0x0023, 0x001A, 0x0011, 0x0008, 0x0000, 0xFFF8, 0xFFEF, 0xFFE6, 0xFFDD, 0xFFD4, 0xFFCB, 0xFFC2, 0xFFB9, 0xFFB0,
    0xFFA8, 0xFF9F, 0xFF96, 0xFF8D, 0xFF85, 0xFF7C, 0xFF73, 0xFF6B, 0xFF62, 0xFF5A, 0xFF51, 0xFF49, 0xFF41, 0xFF38, 0xFF30, 0xFF28, 0xFF20, 0xFF18, 0xFF10, 0xFF08, 0xFF00, 0xFEF9, 0xFEF1, 0xFEEA, 0xFEE2, 0xFEDB, 0xFED4, 0xFECC, 0xFEC5, 0xFEBE, 0xFEB7, 0xFEB1, 0xFEAA, 0xFEA3, 0xFE9D,
    0xFE96, 0xFE90, 0xFE8A, 0xFE84, 0xFE7E, 0xFE78, 0xFE73, 0xFE6D, 0xFE68, 0xFE62, 0xFE5D, 0xFE58, 0xFE53, 0xFE4E, 0xFE4A, 0xFE45, 0xFE41, 0xFE3C, 0xFE38, 0xFE34, 0xFE30, 0xFE2D, 0xFE29, 0xFE26, 0xFE23, 0xFE1F, 0xFE1C, 0xFE1A, 0xFE17, 0xFE14, 0xFE12, 0xFE10, 0xFE0E, 0xFE0C, 0xFE0A,
    0xFE08, 0xFE07, 0xFE05, 0xFE04, 0xFE03, 0xFE02, 0xFE02, 0xFE01, 0xFE01, 0xFE01, 0xFE00, 0xFE01, 0xFE01, 0xFE01, 0xFE02, 0xFE02, 0xFE03, 0xFE04, 0xFE05, 0xFE07, 0xFE08, 0xFE0A, 0xFE0C, 0xFE0E, 0xFE10, 0xFE12, 0xFE14, 0xFE17, 0xFE1A, 0xFE1C, 0xFE1F, 0xFE23, 0xFE26, 0xFE29, 0xFE2D,
    0xFE30, 0xFE34, 0xFE38, 0xFE3C, 0xFE41, 0xFE45, 0xFE4A, 0xFE4E, 0xFE53, 0xFE58, 0xFE5D, 0xFE62, 0xFE68, 0xFE6D, 0xFE73, 0xFE78, 0xFE7E, 0xFE84, 0xFE8A, 0xFE90, 0xFE96, 0xFE9D, 0xFEA3, 0xFEAA, 0xFEB1, 0xFEB7, 0xFEBE, 0xFEC5, 0xFECC, 0xFED4, 0xFEDB, 0xFEE2, 0xFEEA, 0xFEF1, 0xFEF9,
    0xFF00, 0xFF08, 0xFF10, 0xFF18, 0xFF20, 0xFF28, 0xFF30, 0xFF38, 0xFF41, 0xFF49, 0xFF51, 0xFF5A, 0xFF62, 0xFF6B, 0xFF73, 0xFF7C, 0xFF85, 0xFF8D, 0xFF96, 0xFF9F, 0xFFA8, 0xFFB0, 0xFFB9, 0xFFC2, 0xFFCB, 0xFFD4, 0xFFDD, 0xFFE6, 0xFFEF, 0xFFF8, 0x0000, 0x0008, 0x0011, 0x001A, 0x0023,
    0x002C, 0x0035, 0x003E, 0x0047, 0x0050, 0x0058, 0x0061, 0x006A, 0x0073, 0x007B, 0x0084, 0x008D, 0x0095, 0x009E, 0x00A6, 0x00AF, 0x00B7, 0x00BF, 0x00C8, 0x00D0, 0x00D8, 0x00E0, 0x00E8, 0x00F0, 0x00F8, 0x0100, 0x0107, 0x010F, 0x0116, 0x011E, 0x0125, 0x012C, 0x0134, 0x013B, 0x0142,
    0x0149, 0x014F, 0x0156, 0x015D, 0x0163, 0x016A, 0x0170, 0x0176, 0x017C, 0x0182, 0x0188, 0x018D, 0x0193, 0x0198, 0x019E, 0x01A3, 0x01A8, 0x01AD, 0x01B2, 0x01B6, 0x01BB, 0x01BF, 0x01C4, 0x01C8, 0x01CC, 0x01D0, 0x01D3, 0x01D7, 0x01DA, 0x01DD, 0x01E1, 0x01E4, 0x01E6, 0x01E9, 0x01EC,
    0x01EE, 0x01F0, 0x01F2, 0x01F4, 0x01F6, 0x01F8, 0x01F9, 0x01FB, 0x01FC, 0x01FD, 0x01FE, 0x01FE, 0x01FF, 0x01FF, 0x01FF, 0x0200, 0x01FF, 0x01FF, 0x01FF, 0x01FE, 0x01FE, 0x01FD, 0x01FC, 0x01FB, 0x01F9, 0x01F8, 0x01F6, 0x01F4, 0x01F2, 0x01F0, 0x01EE, 0x01EC, 0x01E9, 0x01E6, 0x01E4,
    0x01E1, 0x01DD, 0x01DA, 0x01D7, 0x01D3, 0x01D0, 0x01CC, 0x01C8, 0x01C4, 0x01BF, 0x01BB, 0x01B6, 0x01B2, 0x01AD, 0x01A8, 0x01A3, 0x019E, 0x0198, 0x0193, 0x018D, 0x0188, 0x0182, 0x017C, 0x0176, 0x0170, 0x016A, 0x0163, 0x015D, 0x0156, 0x014F, 0x0149, 0x0142, 0x013B, 0x0134, 0x012C,
    0x0125, 0x011E, 0x0116, 0x010F, 0x0107, 0x0100, 0x00F8, 0x00F0, 0x00E8, 0x00E0, 0x00D8, 0x00D0, 0x00C8, 0x00BF, 0x00B7, 0x00AF, 0x00A6, 0x009E, 0x0095, 0x008D, 0x0084, 0x007B, 0x0073, 0x006A, 0x0061, 0x0058, 0x0050, 0x0047, 0x003E, 0x0035, 0x002C, 0x0023, 0x001A, 0x0011, 0x0008,
    0x0000, 0xFFF8, 0xFFEF, 0xFFE6, 0xFFDD, 0xFFD4, 0xFFCB, 0xFFC2, 0xFFB9, 0xFFB0, 0xFFA8, 0xFF9F, 0xFF96, 0xFF8D, 0xFF85, 0xFF7C, 0xFF73, 0xFF6B, 0xFF62, 0xFF5A, 0xFF51, 0xFF49, 0xFF41, 0xFF38, 0xFF30, 0xFF28, 0xFF20, 0xFF18, 0xFF10, 0xFF08, 0xFF00, 0xFEF9, 0xFEF1, 0xFEEA, 0xFEE2,
    0xFEDB, 0xFED4, 0xFECC, 0xFEC5, 0xFEBE, 0xFEB7, 0xFEB1, 0xFEAA, 0xFEA3, 0xFE9D, 0xFE96, 0xFE90, 0xFE8A, 0xFE84, 0xFE7E, 0xFE78, 0xFE73, 0xFE6D, 0xFE68, 0xFE62, 0xFE5D, 0xFE58, 0xFE53, 0xFE4E, 0xFE4A, 0xFE45, 0xFE41, 0xFE3C, 0xFE38, 0xFE34, 0xFE30, 0xFE2D, 0xFE29, 0xFE26, 0xFE23,
    0xFE1F, 0xFE1C, 0xFE1A, 0xFE17, 0xFE14, 0xFE12, 0xFE10, 0xFE0E, 0xFE0C, 0xFE0A, 0xFE08, 0xFE07, 0xFE05, 0xFE04, 0xFE03, 0xFE02, 0xFE02, 0xFE01, 0xFE01, 0xFE01, 0xFE00, 0xFE01, 0xFE01, 0xFE01, 0xFE02, 0xFE02, 0xFE03, 0xFE04, 0xFE05, 0xFE07, 0xFE08, 0xFE0A, 0xFE0C, 0xFE0E, 0xFE10,
    0xFE12, 0xFE14, 0xFE17, 0xFE1A, 0xFE1C, 0xFE1F, 0xFE23, 0xFE26, 0xFE29, 0xFE2D, 0xFE30, 0xFE34, 0xFE38, 0xFE3C, 0xFE41, 0xFE45, 0xFE4A, 0xFE4E, 0xFE53, 0xFE58, 0xFE5D, 0xFE62, 0xFE68, 0xFE6D, 0xFE73, 0xFE78, 0xFE7E, 0xFE84, 0xFE8A, 0xFE90, 0xFE96, 0xFE9D, 0xFEA3, 0xFEAA, 0xFEB1,
    0xFEB7, 0xFEBE, 0xFEC5, 0xFECC, 0xFED4, 0xFEDB, 0xFEE2, 0xFEEA, 0xFEF1, 0xFEF9, 0xFF00, 0xFF08, 0xFF10, 0xFF18, 0xFF20, 0xFF28, 0xFF30, 0xFF38, 0xFF41, 0xFF49, 0xFF51, 0xFF5A, 0xFF62, 0xFF6B, 0xFF73, 0xFF7C, 0xFF85, 0xFF8D, 0xFF96, 0xFF9F, 0xFFA8, 0xFFB0, 0xFFB9, 0xFFC2, 0xFFCB,
    0xFFD4, 0xFFDD, 0xFFE6, 0xFFEF, 0xFFF8, 0x0000, 0x0008, 0x0011, 0x001A, 0x0023, 0x002C, 0x0035, 0x003E, 0x0047, 0x0050, 0x0058, 0x0061, 0x006A, 0x0073, 0x007B, 0x0084, 0x008D, 0x0095, 0x009E, 0x00A6, 0x00AF, 0x00B7, 0x00BF, 0x00C8, 0x00D0, 0x00D8, 0x00E0, 0x00E8, 0x00F0, 0x00F8,
    0x0100, 0x0107, 0x010F, 0x0116, 0x011E, 0x0125, 0x012C, 0x0134, 0x013B, 0x0142, 0x0149, 0x014F, 0x0156, 0x015D, 0x0163, 0x016A, 0x0170, 0x0176, 0x017C, 0x0182, 0x0188, 0x018D, 0x0193, 0x0198, 0x019E, 0x01A3, 0x01A8, 0x01AD, 0x01B2, 0x01B6, 0x01BB, 0x01BF, 0x01C4, 0x01C8, 0x01CC,
    0x01D0, 0x01D3, 0x01D7, 0x01DA, 0x01DD, 0x01E1, 0x01E4, 0x01E6, 0x01E9, 0x01EC, 0x01EE, 0x01F0, 0x01F2, 0x01F4, 0x01F6, 0x01F8, 0x01F9, 0x01FB, 0x01FC, 0x01FD, 0x01FE, 0x01FE, 0x01FF, 0x01FF, 0x01FF, 0x0200
};

s16 *tabsin = tables + 720;
s16 *tabcos = tables + 810;


s16 px1;
s16 py1;
s16 pz1;

s16 px2;
s16 py2;
s16 pz2;

// ============================================================================
#pragma mark - Additions
// ============================================================================

// Terminator of the found-entity list
#define ENTLIST_END (alis.platform.version >= 60 ? 0 : -1)

void clivin(void);
void shrinkprog(s32 start, s32 length, u16 script_id);
void v6_unloadprog(const char *name, s32 mode);
void v6_wakeall(u16 n);
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
static s16 getprot(void);

// ============================================================================
#pragma mark - Stub routines
// ============================================================================
// Originally ALIS uses two types of stubs, and also has individual empty functions
// (see the N/I section below on the latter).
//
// The address of the first stub, called cnul, is put in place of the non-implemented
// functions (opcodes) in the pointer tables tcodop and tcodesc1.
// It does nothing (the function cnul contain only retn).
//
// The address 0x0000 of the second stub, unnamed one (we called it pnul), is put in
// place of the non-implemented functions (opcodes) in the pointer tables toper, tstore and tadd.
// It outputs a message that a null pointer has been called and then exits the program.
//
// As the engine evolved, opcodes that were sent to these stubs in earlier versions
// could get their own implementation and even a new name in later ones and vice versa
// (implemented functions could be removed from the engine, and pointers to cnul/pnul
// were placed in their place in the corresponding tables).

// Codopname no. 001 opcode 0x00 -> cnul
// Codopname no. 056 opcode 0x37 -> cnul
// Codopname no. 140 opcode 0x8b -> cnul
// Codesc1name no. 01 opcode 0x00 -> cnul
void cnul(void)
{
#if VM_IGNORE_ERRORS == 0
    printf("\n");
    ALIS_DEBUG(EDebugFatal, disalis ? "ERROR: %s" : "%s", "[N/I] cnul: NULL opcode called\n");
    ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
    alis.state = eAlisStateStopped;
#else
    ALIS_DEBUG(EDebugError, "[N/I] cnul: NULL opcode called");
#endif
}

// Opername no. 31 opcode 0x3c -> pnul
// Opername no. 32 opcode 0x3e -> pnul
// Storename no. 01 opcode 0x00 -> pnul
// Storename no. 02 opcode 0x02 -> pnul
// Storename no. 03 opcode 0x04 -> pnul
// Addname no. 01 opcode 0x00 -> pnul
// Addname no. 02 opcode 0x02 -> pnul
// Addname no. 03 opcode 0x04 -> pnul
void pnul(void)
{
    printf("\n");
    ALIS_DEBUG(EDebugFatal, disalis ? "ERROR: %s" : "%s", "[N/I] pnul: NULL code pointer called\n");
#if VM_IGNORE_ERRORS == 0
    ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
    alis.state = eAlisStateStopped;
#endif
}

// Ishar 3 Korean (IBM PC):
// tcodop_E3: cdefmap => map_cnul
// tcodop_E4: csetmap => map_cnul
// tcodop_E5: cputmap => map_cnul
// tcodop_E8: ctexmap => map_cnul
// tcodop_F7: cwalkmap => map_cnul
// tcodop_F8: catstmap => map_cnul
// tcodop_FD: cchartmap => map_cnul
void map_cnul(void)
{
#if VM_IGNORE_ERRORS == 0
    printf("\n");
    ALIS_DEBUG(EDebugFatal, disalis ? "ERROR: %s" : "%s", "[N/I] cnul for map opcodes: NULL opcode called\n");
    ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
    alis.state = eAlisStateStopped;
#else
    ALIS_DEBUG(EDebugError, "[N/I] cnul for map opcodes: NULL opcode called");
#endif
}

// ============================================================================
#pragma mark - Codesc1, Codesc2, Codesc3 routines
// ============================================================================

// Codopname no. 002 opcode 0x01 cesc1
static void cesc1(void)
{
    if (alis.platform.version == 10)
        return;
    
    readexec_codesc1name();
}

// ============================================================================
#pragma mark - Restored non-implemented [N/I] codops (opcodes)
// ============================================================================
// See next section for details on non-implemented opcodes
// ============================================================================

// Codopname no. 003 opcode 0x02 cesc2
// The function is not yet found in ALIS interpreters (no confirmation that the codesc1 table
// has been completed), restored for a reason. This does not affect the code if it does not exist.
static void cesc2(void)
{
    if (alis.platform.version == 10 || alis.platform.version >= 60)
        return;

    readexec_codesc2name();
}

// Codopname no. 004 opcode 0x03 cesc3
// The function is not yet found in ALIS interpreters (no confirmation that tables codesc1 and codesc2
// have been completed), restored for a reason. This does not affect the code if it does not exist.
static void cesc3(void)
{
    if (alis.platform.version == 10 || alis.platform.version >= 60)
        return;

    readexec_codesc3name();
}

// ============================================================================
#pragma mark - Non-implemented [N/I] codops (opcodes)
// ============================================================================
// In the original interpreters, these opcodes are empty (their functions contain
// only retn). If found, it should be documented in which game and on which platform
// the implemented function is detected!
//
// As the engine evolved, opcodes that were empty in early versions could get
// their implementation later and vice versa (become empty). This is observed even
// in different versions of the same games, for example, on different platforms.
//
// Note: [N/I] functions are not to be confused with [MISSING] functions, whose
// code is implemented in the original engine, but not yet implemented by us here.
// ============================================================================

// Codopname no. 005 opcode 0x04 cbreakpt
// [N/I]: Robinson's Requiem (IBM PC)
static void cbreakpt(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 012 opcode 0x0b cjsrabs
// [N/I]: Robinson's Requiem (IBM PC)
static void cjsrabs(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 013 opcode 0x0c cjmpabs
// [N/I]: Robinson's Requiem (IBM PC)
static void cjmpabs(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 014 opcode 0x0d cjsrind16
// [N/I]: Robinson's Requiem (IBM PC)
static void cjsrind16(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 015 opcode 0x0e cjsrind24
// [N/I]: Robinson's Requiem (IBM PC). NOTE: in the DOS VM these indirect ops are bare
// RET (no-ops), so reaching one normally means a PREVIOUS opcode consumed the wrong
// number of script bytes and we are mis-aligned. Keep the warning as a drift indicator.
static void cjsrind24(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 016 opcode 0x0f cjmpind16
// [N/I]: Robinson's Requiem (IBM PC)
static void cjmpind16(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 017 opcode 0x10 cjmpind24
// [N/I]: Robinson's Requiem (IBM PC)
static void cjmpind24(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 035 opcode 0x22 cmul
// [N/I]: Robinson's Requiem (IBM PC)
static void cmul(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 036 opcode 0x23 cdiv
// [N/I]: Robinson's Requiem (IBM PC)
static void cdiv(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 202 opcode 0xc9 cscdump
// [N/I]: Robinson's Requiem (IBM PC)
static void cscdump(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I] (NOP?): %s", __FUNCTION__);
}

// v6 dispatch target for cscdump
static void cscdump_v6(void)
{
    image.numelem = getprot();
    readexec_opername();
    readexec_opername();
    readexec_opername();
    readexec_opername();
    readexec_opername(); image.depx = alis.varD7;
    readexec_opername(); image.depy = alis.varD7;
    readexec_opername(); image.depz = alis.varD7;
    
    // TODO: FUN_0000761f — put the type-0x39 credits scroll sprite (render not yet implemented).
}

// Codopname no. 213 opcode 0xd4 cautobase
// [N/I]: Robinson's Requiem (IBM PC)
static void cautobase(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I] (NOP?): %s", __FUNCTION__);
}

// Codopname no. 236 opcode 0xeb cscantab
// [N/I]: Robinson's Requiem (IBM PC)
static void cscantab(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// Codopname no. 237 opcode 0xec cneartab
// [N/I]: Robinson's Requiem (IBM PC)
static void cneartab(void)
{
    ALIS_DEBUG(EDebugWarning, "[N/I]: %s", __FUNCTION__);
}

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines
// ============================================================================

// Codopname no. 031 opcode 0x1e cstore
static void cstore(void)
{
    readexec_opername_saveD7();
    cstore_continue();
}

// Codopname no. 032 opcode 0x1f ceval
static void ceval(void)
{
    readexec_opername_saveD7();
}

// Codopname no. 033 opcode 0x20 cadd
static void cadd(void)
{
    readexec_opername_saveD7();
    readexec_addname_swap();
}

// Codopname no. 034 opcode 0x21 csub
static void csub(void)
{
    readexec_opername_saveD7();
    alis.varD7 *= -1;
    readexec_addname_swap();
}

// Codopname no. 037 opcode 0x24 cvprint
static void cvprint(void)
{
    alis.charmode = 0;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

// Codopname no. 038 opcode 0x25 csprinti
static void csprinti(void)
{
    alis.charmode = 0;
    for (u8 c = script_read8(); c != 0; c = script_read8())
    {
        put_char(c);
    }
}

// Codopname no. 039 opcode 0x26 csprinta
static void csprinta(void)
{
    if (alis.platform.version == 10)
        return;
    
    alis.charmode = 0;
    readexec_opername_saveD7();
    put_string();
}

// Codopname no. 040 opcode 0x27 clocate
static void clocate(void)
{
    alis.charmode = 0;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    alis.xlocate = (u8)(alis.varD7 & 0xff);
    alis.ylocate = (u8)(alis.varD6 & 0xff);
}

// Codopname no. 041 opcode 0x28 ctab
static void ctab(void)
{
    alis.charmode = 0;
    readexec_opername_saveD7();
    alis.xlocate = (u8)(alis.varD7 & 0xff);
}

// Codopname no. 042 opcode 0x29 cdim
static void cdim(void)
{
    
    s16 offset = script_read16();
    u8 counter = script_read8();
    u8 byte2 = script_read8();
    
    xwrite8(alis.script->vram_org + --offset, counter);
    xwrite8(alis.script->vram_org + --offset, byte2);

    if (alis.platform.version >= 60)
    {
        offset -= 2;
        xwrite16(alis.script->vram_org + offset, script_read16());

        for (s16 i = 0; i < (s8)counter + 1; i++)
        {
            offset -= 4;
            xwrite32(alis.script->vram_org + offset, script_read32());
        }
    }
    else if (alis.platform.version >= 30)
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
        while(counter--)
        {
            offset -= 2;
            xwrite16(alis.script->vram_org + offset, script_read16());
        }
    }
}

// Codopname no. 043 opcode 0x2a crandom
static void crandom(void)
{
    readexec_opername();
    alis.random_number = alis.varD7;
    if(alis.random_number == 0)
    {
        alis.random_number = sys_random();
        // test key/joy input, if zero, random is set to $64 ???
    }
}

static s16 read_off24_v6(void);

static s32 read_off24(void)
{
    if (alis.platform.version >= 60)
        return read_off24_v6();
    if (alis.platform.is_little_endian)
    {
        s16 o = (s16)script_read16();
        script_read8();
        return o;
    }

    s32 hi = (s8)script_read8();
    return (s32)((u32)hi << 16 | (u16)script_read16());
}

u32 save_loop_pc;

static void cloop(s32 offset)
{
    // NOTE: looks like the only code called is adir* and aloc*
    // we set zero flag using last called instructions there
    
    save_loop_pc = alis.script->pc;
    alis.varD7 = -1;
    readexec_addname_swap();
    if(!alis.sr.zero)
    {
        alis.script->pc = save_loop_pc;

#ifndef NDEBUG
        if (disalis)
        {
             if (offset<0)
             {
                 ALIS_DEBUG(EDebugInfo, " [loop jmp up -0x%06x]", abs(offset));
             }
             else
             {
                 ALIS_DEBUG(EDebugInfo, " [loop jmp dn +0x%06x]", offset);
             }
           }
#endif
        alis.script->pc += offset;
    }
}

// Codopname no. 044 opcode 0x2b cloop8
static void cloop8(void)
{
    cloop((s8)script_read8());
}

// Codopname no. 045 opcode 0x2c cloop16
static void cloop16(void)
{
    cloop((s16)script_read16());
}

// Codopname no. 046 opcode 0x2d cloop24
static void cloop24(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        s16 o = (s16)script_read16();
        script_read8();
        cloop(o);
        return;
    }

    cloop(read_off24());
}

// Codopname no. 047 opcode 0x2e cswitch1
static void cswitch1(void)
{
    readexec_opername();

    s16 val = (s16)alis.varD7;
    s16 test = 0;
    s16 addition = script_read8();
    if ((alis.script->pc & 1) != 0)
    {
        alis.script->pc ++;
    }
    
    do
    {
        test = script_read16();
        if (val == test)
        {
            alis.script->pc += script_read16();
            return;
        }
        
        alis.script->pc += 2;
    }
    while (val > test  && (--addition) > -1);
    
    if (addition > 0)
    {
        alis.script->pc += (addition * 4);
    }
}

// Codopname no. 048 opcode 0x2f cswitch2
static void cswitch2(void)
{
    readexec_opername();
    
    s16 addition = script_read8();
    if ((alis.script->pc & 1) != 0)
    {
        alis.script->pc ++;
    }

    s16 val = (s16)(alis.varD7 + script_read16());
    alis.varD7 = val;
    if (val >= 0 && val <= addition)
    {
        alis.varD7 = val * 2;
        alis.script->pc += alis.varD7;
        alis.script->pc += script_read16();
    }
    else
    {
        alis.script->pc += (addition * 2 + 2);
    }
}

// Codopname no. 052 opcode 0x33 cleave
static void cleave(void)
{
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

    ALIS_DEBUG(EDebugVerbose, " (NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", alis.script->name, alis.script->vram_org, alis.finent, alis.script->vacc_off, alis.script->pc_org);
}

// v6 dispatch target for cleave
static void cleave_v6(void)
{
    if (alis.fseq != 0)
    {
        xwrite32(alis.script->vram_org - 0x80, alis.script->pc - alis.script->pc_org);
        xwrite16(alis.script->vram_org - 0x7c, alis.script->vacc_off);
    }
}

// Codopname no. 053 opcode 0x34 cprotect
static void cprotect(void)
{
    readexec_opername();
    alis.vprotect = alis.varD7;
}

// Codopname no. 054 opcode 0x35 casleep
static void casleep(void)
{
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
static void cscmov(void)
{
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
static void cscset(void)
{
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
static void cclipping(void)
{
    alis.fswitch = 0;
}

// Codopname no. 060 opcode 0x3b cswitching
// [N/I]: Robinson's Requiem, Storm Master (IBM PC)
// [N/I]: Ishar 3 Korean (IBM PC)
// TODO: the PC original never sets fswitch, but this port must or nothing is shown.
static void cswitching(void)
{
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
static void cwlive(void)
{
    get_vector(&alis.wcx, &alis.wcy, &alis.wcz);
    
    alis.varD7 = -1;
    clivin();
}

// Codopname no. 062 opcode 0x3d cunload
static void cunload(void)
{
    s16 id;
    s16 index;

    if (alis.platform.version >= 60)
    {
        readexec_opername_swap();
        char name[16] = {0};
        strncpy(name, alis.sd7, sizeof(name) - 1);
        readexec_opername();
        v6_unloadprog(name, alis.platform.kind == EPlatform3DO ? 0 : (s32)alis.varD7);
        return;
    }

    id = script_read16();
    if (id == -1 || id == get_0x10_script_id(alis.script->vram_org))
        return;
    index = debprotf(id);
    if (index == -1)
        return;

    {
            u32 data_org = ATPROG(index);
            u32 data_end = 0xfffffff;
            
            for (int i = 0; i < alis.nbprog; i++)
            {
                if (data_org < ATPROG(i) && ATPROG(i) < data_end)
                {
                    data_end = ATPROG(i);
                }
            }
            
            if (data_end == 0xfffffff)
            {
                data_end = alis.finprog;
            }

            sAlisScriptData *script = alis.loaded_scripts[index];
            ALIS_DEBUG(EDebugVerbose, "\n (NAME: %s, ID: 0x%x ORG: 0x%x (0x%x) SZ: %d (%d)) \n", script->name, script->header.id, script->data_org, data_org, script->sz, data_end - data_org);
            
            // NOTE: interuptable delay to make everything work more like on original platforms
            sys_sleep_interactive(&alis.unload_delay, alis.unload_delay - 100);

            shrinkprog(data_org, data_end - data_org, id);
    }
}

// Codopname no. 063 opcode 0x3e cwakeup
static void cwakeup(void)
{
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
static void cstop(void)
{
    // in real program, adds 4 to real stack pointer
    alis.script->running = 0;
    ALIS_DEBUG(EDebugInfo, "\n-- CSTOP --");
}

// Codopname no. 064 opcode 0x3f csleep
static void csleep(void)
{
    set_0x04_cstart_csleep(alis.script->vram_org, 0);
    if (alis.fseq)
    {
        cstop();
    }
}

// Codopname no. 065 opcode 0x40 clive
static void clive(void)
{
    alis.wcx = 0;
    alis.wcy = 0;
    alis.wcz = 0;
    
    alis.varD7 = -1;
    clivin();
}

void clivin(void)
{
    sAlisScriptData *prog = NULL;

    if (alis.platform.version >= 60)
    {
        readexec_opername_swap();
        prog = node_prog(node_find(alis.sd7));
    }
    else
    {
        s16 id = script_read16();
        if (id == -1)
        {
            alis.varD7 = -1;
        }
        else
        {
            s16 index = debprotf(id);
            if (index != -1)
                prog = alis.loaded_scripts[index];
        }
    }

    if (prog)
    {
        sAlisScriptLive *script = script_live(prog);
        if (script == NULL)
        {
            alis.varD7 = 0;
            return;
        }

        u8 *prev_vram = alis.mem + alis.script->vram_org;
        u8 *next = alis.mem + script->vram_org;

        if (alis.platform.version < 60)
        {
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
            else if (alis.platform.version == 10)
            {
                // position and velocity (DOS skips bytes 6-7), byte 8 is not inherited
                memcpy(next, prev_vram, alis.platform.kind == EPlatformPC ? 6 : 8);
                memcpy(next + 9, prev_vram + 9, 3);
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
        }

        // NOTE: just to show already running scripts
        
        ALIS_DEBUG(EDebugInfo, "\n");

        u16 tent = 0;
        u32 loop = alis.nbent;
        if (loop > MAX_SCRIPTS)
            loop = MAX_SCRIPTS;

        for (int i = 0; i < loop && i < MAX_SCRIPTS; i++)
        {
            tent = xread16(alis.atent + i * ENT_STRIDE + 4);

            sAlisScriptLive *s = alis.live_scripts[i];
            if (s && s->vram_org && s->data)
            {
                s32 contextsize = get_context_size();
                u32 datasize = contextsize + s->data->header.w_unknown5 + s->data->header.w_unknown7;
                s32 vramsize = s->data->header.vram_alloc_sz;
                s32 shrinkby = datasize + vramsize;
                
                ALIS_DEBUG(EDebugInfo, "%c[%s ID: %.2x(%.2x), %.2x, %.6x, %.6x] \n", script->vram_org == s->vram_org ? '*' : ' ', s->name, s->data->header.id, get_0x10_script_id(s->vram_org), xswap16(tent), s->vram_org, shrinkby);
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, " [ empty  ID: 00(00), %.2x, 000000, 000000] \n", xswap16(tent));
                loop++;
            }
        }

        ALIS_DEBUG(EDebugInfo, " [finent %.6x] \n", alis.finent);

        if (alis.maxent < alis.nbent)
        {
            ALIS_DEBUG(EDebugFatal, "ERROR: Exceeded number of scripts slots!\n");
            return;
        }
    }
    else if (alis.platform.version >= 60)
    {
        alis.varD7 = 0;
    }

    cstore_continue();
}

// Codopname no. 066 opcode 0x41 ckill
static void ckill(void)
{
    readexec_opername_saveD7();
    if (alis.platform.version >= 60)
    {
        if (alis.varD7 <= 0)
            return;
        
        alis.ferase = 1;
    }
    else if (alis.varD7 < 0)
    {
        return;
    }

     killent(alis.varD7);
}

// Codopname no. 068 opcode 0x43 cstopret
static void cstopret(void)
{

    if (alis.fseq == 0)
    {
        cstop();
        if (alis.platform.version == 10) // v1.0 just stops
            return;
    }

    cret();
    alis.script->running = 0;
}

// v6 dispatch target for cstopret
static void cstopret_v6(void)
{
    if (alis.fseq == 0)
    {
        cstop();
        return;
    }

    cret();
    alis.script->running = 0;
}

// Codopname no. 069 opcode 0x44 cexit
static void cexit(void)
{

    if (alis.varD5 == 0)
    {
        // Quit: unwind to main(), which tears down on the main thread (cexit runs on the VM
        // worker thread; sys_deinit owns the IRQ-quiesce ordering).
        alis.cstopret = 1;
        alis.state = eAlisStateStopped;
        alis.script->running = 0;
        return;
    }

    killent(alis.varD5);
    alis.script->running = 0;
}

// v6 dispatch target for cexit
static void cexit_v6(void)
{
    if (alis.script->vram_org == alis.basemain)
    {
        alis.cstopret = 1;
        alis.state = eAlisStateStopped;
        alis.script->running = 0;
        return;
    }

    alis.ferase = 1;
    killent(xread16(alis.script->vram_org + 0x3a));
    alis.script->running = 0;
}

// Codopname no. 070 opcode 0x45 cload
static void cload(void)
{

    // NOTE: interuptable delay to make everything work more like on original platforms
    sys_sleep_interactive(&alis.load_delay, alis.load_delay - 100);

    if (alis.platform.version >= 60)
    {
        readexec_opername_swap();

        char name[24] = {0};
        strncpy(name, alis.sd7, sizeof(name) - 4);

        u16 node = node_find(name);
        if (node == 0)
        {
            strcat(name, ".");
            strcat(name, alis.platform.ext);

            char path[kPathMaxLen] = {0};
            strcpy(path, alis.platform.path);
            strcat(path, name);

            sAlisScriptData *loaded = script_load(strlower((char *)path));
            node = loaded ? loaded->node : 0;
            is_delay_script(name);
        }

        v6_wakeall(node);
        return;
    }

    // get script ID
    u16 id = script_read16();
    if(id != 0)
    {
        // not main script, unpack and load into vm
        char path[kPathMaxLen] = {0};
        strcpy(path, alis.platform.path);
        char name[16] = {0};
        script_read_until_zero(name);
        strcpy(strrchr(name, '.') + 1, alis.platform.ext);
        strcat(path, name);
        sAlisScriptData *loaded = script_load(path);
        if (!loaded && alis_fatal)
        {
            // Out of memory: the original stops with a fatal error too.
            alis.cstopret = 1;
            alis.state = eAlisStateStopped;
            alis.script->running = 0;
            return;
        }

        is_delay_script(name);
    }
    else
    {
        // NOTE: load main script and start game loop
        // we are dooing it elsewhere, shouldnt ever be reached;
        ALIS_DEBUG(EDebugWarning, "STUBBED: %s", __FUNCTION__);

        if (alis.platform.version == 10)
        {
            char name[16] = {0};
            script_read_until_zero(name);
        }
        else
        {
            readexec_opername_swap();
        }
    }
}

void cdefsc(void)
{
    if (alis.platform.version >= 60)
    {
        s16 scene = alis.platform.version >= 60 ? (s16)xread16(alis.script->vram_org - 0x64)
                                                : get_0x16_screen_id(alis.script->vram_org);
        readexec_opername();
        xwrite8(alis.basemain + (u16)scene + 0x5a, alis.varD7 & 0xff);
        return;
    }

    s16 scridx = script_read16();
    set_scr_state(scridx, get_scr_state(scridx) | 0x40);
    set_scr_numelem(scridx, script_read8());
    set_scr_screen_id(scridx, image.libsprit);

    u16 length = alis.platform.version == 10 ? 26 : 32;

    u8 *ptr = alis.mem + alis.basemain + scridx + 6;
    for (int i = 0; i < length; i++, ptr++)
        *ptr = script_read8();

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
    
    set_scr_to_next(scridx, 0);

    scdosprite(scridx);

    if (alis.platform.version != 10)
    {
        set_scr_unknown0x2a(scridx, 0);
        set_scr_unknown0x2c(scridx, 0);
        set_scr_unknown0x2e(scridx, 0);
    }

    image.libsprit = sprite->to_next;

    scadd(scridx);
    vectoriel(scridx);
}

// Codopname no. 072 opcode 0x47 cscreen
static void cscreen_v6x(u16 id);

static void cscreen(void)
{
    u16 screen_id = script_read16();
    if (alis.platform.version >= 60)
    {
        cscreen_v6x(screen_id);
        return;
    }

    if (screen_id != get_0x16_screen_id(alis.script->vram_org))
    {
        set_0x16_screen_id(alis.script->vram_org, screen_id);
    }
}

// Codopname no. 073 opcode 0x48 cput
static s16 getprot(void);
static void v6_put_entry(int xflip, int frommain, int mzoom, int at);

static void cput(void)
{
    if (alis.platform.version >= 60)
    {
        v6_put_entry(0, 0, 0, 0);       // DOS OPCODE_CPUT 00006ecc
        return;
    }

    alis.flagmain = 0;
    image.invert_x = get_0x03_xinv(alis.script->vram_org);

    s16 elem = 0;

    readexec_opername();

    image.depx = 0;
    image.depy = 0;
    image.depz = 0;
    image.numelem = elem;

    u16 idx = alis.varD7;

    put(idx);
}

// Codopname no. 074 opcode 0x49 cputnat
static void cputnat(void)
{
    if (alis.platform.version >= 60)
    {
        v6_put_entry(0, 0, 0, 1);       // DOS OPCODE_CPUTNAT 00006f8d
        return;
    }

    alis.flagmain = 0;
    image.invert_x = get_0x03_xinv(alis.script->vram_org);

    readexec_opername_saveD7();
    image.depx = alis.varD7;
    readexec_opername_saveD7();
    image.depy = alis.varD7;
    image.depz = 0;
    if (alis.platform.version != 10)
    {
        readexec_opername_saveD7();
        image.depz = alis.varD7;
    }

    readexec_opername_saveD7();
    readexec_opername_saveD6();

    image.numelem = alis.varD6;
    u16 idx = alis.varD7;
    
    put(idx);
}

static s16 getprot(void)
{
    s16 ax = (s16)script_read16();
    u16 uax = (u16)ax;

    if (uax == 0xffff || uax == 0xfffe)
    {
        return ax;
    }
    
    if (uax == 0xfffa)
    {
        return (s16)script_read16();
    }

    if (uax == 0xfffb)
    {
        readexec_opername();
        return ax;
    }

    if (uax == 0xfffd || uax == 0xfffc)
    {
        s16 w = (s16)script_read16();
        u32 ctx = alis.script->vram_org + (u16)w;
        if ((s8)xread8(ctx - 1) >= 0)
            readexec_opername();
        
        readexec_opername();
        return ax;
    }

    if (uax > 0xff00)
        return ax;

    u32 ctx = alis.script->vram_org + uax;
    if ((s8)xread8(ctx - 1) >= 0)
        readexec_opername();
    
    return ax;
}

typedef struct {
    s32 elem;
    u32 limit;
    s32 aux;
} sV6Prot;

static void getprot_x(sV6Prot *p)
{
    u32 vram = alis.script->vram_org;
    u16 ax = (u16)script_read16();

    if (ax == 0xffff)
    {
        p->elem = -1; p->aux = -1; p->limit = 0;
        return;
    }

    if (ax == 0xfffd)
    {
        u32 elem = vram + (u16)script_read16();
        u16 size = (u16)xread16(elem - 4);
        if ((s8)xread8(elem - 1) >= 0)
        {
            alis.v6tabcell = 0;
            readexec_opername();
            if (alis.v6tabcell != 0)
                elem = alis.v6tabcell;
        }

        readexec_opername_saveD7();
        s32 aux = (u16)alis.varD7 * V6_SLOT_STRIDE;
        p->elem = (s32)elem;
        p->limit = elem + size - 8;
        if (aux > (s32)size)
        {
            ALIS_DEBUG(EDebugError, "getprot_x: slot index out of range (perror 0x27)\n");
            aux = 0;
        }

        p->aux = aux;
        return;
    }

    if (ax == 0xfffc)
    {
        u32 elem = vram + (u16)script_read16();
        u16 size = (u16)xread16(elem - 4);
        if ((s8)xread8(elem - 1) >= 0)
        {
            alis.v6tabcell = 0;
            readexec_opername();
            if (alis.v6tabcell != 0)
                elem = alis.v6tabcell;
        }

        readexec_opername_saveD7();
        s32 desc = adresdes((s16)alis.varD7);
        s32 aux = 0;
        u8 count = xread8(elem + 0x33);
        for (u8 i = 0; i < count; i++)
        {
            if (xread32(elem + aux + 0x3c) == desc)
                break;
            
            aux += V6_SLOT_STRIDE;
        }

        p->elem = (s32)elem;
        p->aux = aux;
        p->limit = elem + size - 8;
        return;
    }

    if (ax == 0xfffe)
    {
        p->elem = (s32)vram;
        p->limit = vram + V6_SLOT_STRIDE;
        p->aux = 0;
        return;
    }

    if (ax == 0xfffb)
    {
        readexec_opername_saveD7();
        s16 id = alis.varD7;
        if (id == 0)
        {
            p->elem = 0; p->limit = 0; p->aux = 0;
            return;
        }

        u32 elem = xread32(alis.atent + (u16)id);
        p->elem = (s32)elem;
        p->limit = elem + V6_SLOT_STRIDE;
        p->aux = 0;
        return;
    }

    if (ax == 0xfffa)
    {
        u32 elem = alis.basemain + (u16)script_read16();
        p->elem = (s32)elem;
        p->limit = elem + V6_SLOT_STRIDE;
        p->aux = 0;
        return;
    }

    if (ax > 0xff00)
    {
        p->elem = (s16)ax;
        p->aux = -1; p->limit = 0;
        return;
    }

    u32 elem = vram + ax;
    u16 size = (u16)xread16(elem - 4);
    if ((s8)xread8(elem - 1) >= 0)
    {
        alis.v6tabcell = 0;
        readexec_opername();
        if (alis.v6tabcell != 0)
            elem = alis.v6tabcell;
    }

    p->elem = (s32)elem;
    p->aux = -1;
    p->limit = elem + size - 8;
}

// reads and drops one element selector
void v6_skipprot(void)
{
    sV6Prot p;
    getprot_x(&p);
}

static void v6_put_entry(int xflip, int frommain, int mzoom, int at)
{
    alis.flagmain = frommain;
    u8 inv = xread8(alis.script->vram_org + (frommain ? 0x24 : 0x25));
    v6put.invert = xflip ? (inv ^ 1) : inv;
    v6put.scale = mzoom ? 0 : 0x100;
    v6put.rotate = 0;
    v6put.rotb1 = 0;
    v6put.rotb2 = 0;
    v6put.ftext = 0;

    sV6Prot p;
    getprot_x(&p);
    v6put.limit = p.limit;
    v6put.basesprite = (u32)p.elem;

    readexec_opername_saveD7();
    s32 desc = adresdes((s16)alis.varD7);
    v6put.desc = (desc == 0xf) ? 0 : (u32)desc;

    if (v6put.scale == 0)
    {
        readexec_opername_saveD7();
        v6put.rotate = alis.varD7;
        readexec_opername_saveD7();
        v6put.scale = alis.varD7;
        readexec_opername_saveD7();
        v6put.rotb1 = (u8)alis.varD7;
        v6put.rotb2 = (u8)((u16)alis.varD7 >> 8);
    }

    if (at)
    {
        readexec_opername_saveD7();
        v6put.depx = (s16)alis.varD7;
        readexec_opername_saveD7();
        v6put.depy = (s16)alis.varD7;
        readexec_opername_saveD7();
        v6put.depz = (s16)alis.varD7;
    }
    else
    {
        v6put.depx = 0;
        v6put.depy = 0;
        v6put.depz = 0;
    }

    v6put.angx = 0;
    v6put.angy = 0;
    v6put.angz = 0;

    if (p.elem > 0)
        put_v6((u32)p.elem);
}

#define V6MAT_MAX 128
static u16 v6mat_ent[V6MAT_MAX];
static u32 v6mat_id[V6MAT_MAX];
static int v6mat_n;

// Codopname opcode 0x37 — v6 TWEEN/ANIM slot driver
static void canim_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    v6put.basesprite = (u32)p.elem;
    s32 elem = p.elem;
    s32 slot = elem;
    u8 usable = 1;

    u8 count = xread8(elem + 0x33);
    if (count != 0)
    {
        v6put.numcreated = count;
        slot = elem + (u32)count * V6_SLOT_STRIDE;
        slot -= V6_SLOT_STRIDE;
        if (xread8(slot + 0x22) != 0x3a)
        {
            if (p.limit != 0 && (u32)(slot + 2 * V6_SLOT_STRIDE) > p.limit)
            {
                ALIS_DEBUG(EDebugWarning, "canim v6: elem %x slot array full (limit=%x) - anim suppressed\n", elem, p.limit);
                usable = 0;
            }
            else
            {
                slot += V6_SLOT_STRIDE;
                xwrite8(slot + 0x22, 0x3a);
                xwrite32(slot + 0x40, -1);
                xwrite32(slot + 0x50, 0);
                xwrite32(slot + 0x3c, 0);
                v6put.numcreated++;
                xwrite8(elem + 0x33, count + 1);
                if (xread32(slot + 0x34) == 0)
                {
                    u16 world = get_0x16_screen_id(alis.script->vram_org);
                    xwrite16(slot + 0x2c, world);
                    u32 w = alis.basemain + world;
                    s32 head = xread32(w + 0x38);
                    if (head <= 0 || (u32)head >= alis.platform.ram_sz)
                    {
                        ALIS_DEBUG(EDebugWarning, "canim v6: world %04x has no sprite list (head=%x) - not linking\n", world, head);
                        goto canim_linked;
                    }

                    xwrite32(w + 0x38, slot);
                    xwrite32(head + 0x34, slot);
                    xwrite32(slot + 0x34, w);
                    xwrite32(slot + 0x38, head);
                    xwrite32(slot + 0x8, alis.script->vram_org);
                    xwrite8(slot + 0x23, 0x80);
canim_linked: ;
                }
            }
        }
    }

    readexec_opername_saveD7();
    s32 desc = adresdes((s16)alis.varD7);
    if (usable)
    {
        if (desc != xread32(slot + 0x3c))
            xwrite32(slot + 0x40, -1);
        
        xwrite32(slot + 0x3c, desc);
    }

    readexec_opername_saveD7();
    if (usable)
        xwrite32(slot + 0x44, (s32)alis.varD7);
    
    readexec_opername_saveD7();
    if (usable)
        xwrite16(slot + 0x54, alis.varD7);
    
    readexec_opername_saveD7();
    s32 op4 = (s32)alis.varD7;
    if (usable)
        xwrite32(slot + 0x48, op4);

    s32 result;
    if (usable && xread16(slot + 0x54) != 0)
    {
        result = v6anim_step((u32)slot);
    }
    else if (op4 == -1 && desc != 0 && xread8(desc + xread32(desc)) == 2)
    {
        result = (u16)xread16(desc + xread32(desc) + 0xa);
    }
    else
    {
        result = usable ? xread32(slot + 0x40) : 0;
    }

    alis.varD7 = result;
    cstore_continue();
}

// Codopname no. 056 opcode 0x37: v6 tween driver, no-op for older games.
static void canim(void)
{
    if (alis.platform.version >= 60)
    {
        canim_v6();
        return;
    }

    cnul();
}

static void v6_erase_entry(u8 ferase)
{
    alis.ferase = ferase;
    sV6Prot p;
    getprot_x(&p);
    if (p.elem < 0)
    {
        killallelem_v6(alis.script->vram_org);
    }
    else if (p.elem > 0)
    {
        eraselem_v6((u32)p.elem);
    }
}

// Codopname no. 075 opcode 0x4a cerase
static void cerase(void)
{
    if (alis.platform.version >= 60)
    {
        v6_erase_entry(1);
        return;
    }

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
static void cerasen(void)
{
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

// v6 dispatch target for cerasen
static void cerasen_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    if (p.aux != -1)
    {
        s32 slot = p.elem + p.aux;
        readexec_opername_saveD7();
        xadd32(slot + 0xc, (s32)alis.varD7);
        readexec_opername_saveD7();
        xadd32(slot + 0x10, (s32)alis.varD7);
        readexec_opername_saveD7();
        xadd32(slot + 0x14, (s32)alis.varD7);
        return;
    }

    readexec_opername_saveD7();
    s16 dx = alis.varD7;
    readexec_opername_saveD7();
    s16 dy = alis.varD7;
    readexec_opername_saveD7();
    s16 dz = alis.varD7;
    s32 slot = p.elem;
    u8 count = xread8(slot + 0x33);
    for (u8 i = 0; i < count; i++)
    {
        if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
        {
            xadd32(slot + 0xc, dx);
            xadd32(slot + 0x10, dy);
            xadd32(slot + 0x14, dz);
            if (xread8(slot + 0x22) == 0x30)
            {
                xadd16(slot + 0x48, dx);
                xadd16(slot + 0x4a, dy);
                xadd16(slot + 0x4c, dz);
            }

            xwrite8(slot + 0x23, xread8(slot + 0x23) | 1);
        }

        slot += V6_SLOT_STRIDE;
    }
}

// Codopname no. 077 opcode 0x4c cset
static void cset(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername();
        xwrite32(alis.script->vram_org + 0x0c, (s32)alis.varD7);
        readexec_opername();
        xwrite32(alis.script->vram_org + 0x10, (s32)alis.varD7);
        readexec_opername();
        xwrite32(alis.script->vram_org + 0x14, (s32)alis.varD7);
        xwrite8(alis.script->vram_org + 0x45, 1);
        return;
    }

    readexec_opername();
    xwrite16(alis.script->vram_org + ALIS_SCR_WCX, alis.varD7);
    readexec_opername();
    xwrite16(alis.script->vram_org + ALIS_SCR_WCY, alis.varD7);
    readexec_opername();
    xwrite16(alis.script->vram_org + ALIS_SCR_WCZ, alis.varD7);
}

// Codopname no. 078 opcode 0x4d cmov
static void cmov(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername();
        xadd32(alis.script->vram_org + 0x0c, (s32)alis.varD7);
        readexec_opername();
        xadd32(alis.script->vram_org + 0x10, (s32)alis.varD7);
        readexec_opername();
        xadd32(alis.script->vram_org + 0x14, (s32)alis.varD7);
        xwrite8(alis.script->vram_org + 0x45, 1);
        return;
    }

    readexec_opername();
    xadd16(alis.script->vram_org + ALIS_SCR_WCX, alis.varD7);
    readexec_opername();
    xadd16(alis.script->vram_org + ALIS_SCR_WCY, alis.varD7);
    readexec_opername();
    xadd16(alis.script->vram_org + ALIS_SCR_WCZ, alis.varD7);
}

// Codopname no. 079 opcode 0x4e copensc

// append screen to the end of the ptscreen display chain
static void v6_scadd(u16 id)
{
    xwrite16(alis.basemain + id + 0x5c, 0);
    if (screen.ptscreen == 0)
    {
        screen.ptscreen = id;
        return;
    }

    u16 cur = screen.ptscreen;
    u16 nxt;
    while ((nxt = xread16(alis.basemain + cur + 0x5c)) != 0)
        cur = nxt;
    
    xwrite16(alis.basemain + cur + 0x5c, id);
}

// unlink screen from the ptscreen chain
static void v6_scdel(u16 id)
{
    u16 cur = screen.ptscreen;
    if (cur == 0)
        return;
    
    if (cur == id)
    {
        screen.ptscreen = xread16(alis.basemain + id + 0x5c);
        return;
    }

    do
    {
        u16 nxt = xread16(alis.basemain + cur + 0x5c);
        if (nxt == id)
        {
            xwrite16(alis.basemain + cur + 0x5c, xread16(alis.basemain + id + 0x5c));
            return;
        }

        cur = nxt;
    }
    while (cur != 0);
}

// refresh the screen's buffer/bounds fields
static void v6_screfresh(u32 s)
{
    xwrite32(s + 0x72, 0);
    xwrite16(s + 0x76, 0);
    xwrite16(s + 0x78, 0);
    s16 w = alis.platform.width;
    s16 h = alis.platform.height;
    xwrite16(s + 0x7a, w);
    xwrite16(s + 0x7c, h);
    s16 x1 = (s16)xread16(s + 0x66);
    s16 y1 = (s16)xread16(s + 0x68);
    s16 x2 = (s16)xread16(s + 0x6a);
    s16 y2 = (s16)xread16(s + 0x6c);
    if (x1 < 0)
        x1 = 0;
    
    if (y1 < 0)
        y1 = 0;
    
    if (x2 > w - 1)
        x2 = w - 1;
    
    if (y2 > h - 1)
        y2 = h - 1;
    
    xwrite16(s + 0x66, x1);
    xwrite16(s + 0x68, y1);
    xwrite16(s + 0x6a, x2);
    xwrite16(s + 0x6c, y2);
    xwrite16(s + 0x6e, x2 - x1 + 1);
    xwrite16(s + 0x70, y2 - y1 + 1);
}

static void cscreen_v6x(u16 id)
{
    xwrite16(alis.script->vram_org - 0x64, id);     // define-reg
    u32 s = alis.basemain + id;
    if (xread8(s + 0x5b) != 0)                      // already defined
        return;

    xwrite8(s + 0x5a, 0);                           // projection mode
    xwrite16(s + 0x66, 0);                          // clip x1
    xwrite16(s + 0x68, 0);                          // clip y1
    s16 w = alis.platform.width;
    xwrite16(s + 0x6e, w);
    xwrite16(s + 0x6a, w - 1);                      // clip x2
    s16 h = alis.platform.height;
    xwrite16(s + 0x70, h);
    xwrite16(s + 0x6c, h - 1);                      // clip y2
    xwrite32(s + 0xb4, 0);
    xwrite8(s + 0x136, 0);
    xwrite8(s + 0x137, 0);
    xwrite8(s + 0x138, 0);
    xwrite32(s + 0x132, 0);
    xwrite8(s + 0x139, 0);
    xwrite8(s + 0x13a, 0);
    xwrite8(s + 0x59, 0);
    xwrite8(s + 0x58, 0x40);                        // defined, hidden
    xwrite32(s + 0x72, 0);
    xwrite16(s + 0x76, 0);
    xwrite16(s + 0x78, 0);
    xwrite16(s + 0x7a, w);
    xwrite16(s + 0x7c, h);
    xwrite8(s + 0x13b, 0);                          // background colour
    v6_scadd(id);
    xwrite8(s + 0x5b, 1);                           // defined flag
}

static void copensc(void)
{
    if (alis.platform.version >= 60)
    {
        u16 id = script_read16();
        u32 s = alis.basemain + id;
        xwrite8(s + 0x58, xread8(s + 0x58) & 0xbf);
        xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        v6_scdel(id);
        v6_scadd(id);
        u8 mode = xread8(s + 0x5a);
        if (mode == 5)
        {
            if (alis.platform.uid != EGameRobinsonsRequiem3DO)
                ALIS_DEBUG(EDebugWarning, "copensc v6: mode-5 open hook (FUN_000438f0) not yet ported\n");
        }
        else if (mode == 6)
        {
            v6x3d_open(s);
            v6ips_warm = 8;
        }

        return;
    }

    u16 id = script_read16();
    *(alis.mem + alis.basemain + id) &= 0xbf;
    *(alis.mem + alis.basemain + id) |= 0x80;

    scbreak(id);
    scadd(id);
    
    if (alis.platform.version >= 31)
    {
        if ((xread8(alis.basemain + 1 + id) & 2) != 0)
        {
            openland(id);
        }
    }
}

// Codopname no. 080 opcode 0x4f cclosesc
static void cclosesc(void)
{
    u16 id = script_read16();
    if (alis.platform.version >= 60)
    {
        u32 s = alis.basemain + id;
        xwrite8(s + 0x58, xread8(s + 0x58) | 0x40); // OR 0x40
        return;
    }

    *(alis.mem + alis.basemain + id) |= 0x40;
}

// Codopname no. 081 opcode 0x50 cerasall
static void cerasall(void)
{
    u16 tmpidx = 0;
    u16 curidx = get_0x18_unknown(alis.script->vram_org);
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }
    
    alis.ferase = 0;
}

// Codopname no. 082 opcode 0x51 cforme
static void cforme(void)
{
    readexec_opername();
    set_0x1a_cforme(alis.script->vram_org, alis.varD7);
}

// v6 dispatch target for cforme
static void cforme_v6(void)
{
    v6put.fadddes = 1;
    cput();
    v6put.fadddes = 0;
}

// Codopname no. 083 opcode 0x52 cdelforme
static void cdelforme(void)
{
    set_0x1a_cforme(alis.script->vram_org, -1);
}

// v6 dispatch target for cdelforme
static void cdelforme_v6(void)
{
    v6put.fadddes = 1;
    cputnat();
    v6put.fadddes = 0;
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
                ALIS_DEBUG(EDebugInfo, "\n    Hit!");
                return 0;
            }
        }
    }
    
    ALIS_DEBUG(EDebugInfo, "\n    No hits!");
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
        ALIS_DEBUG(EDebugInfo, "\n    Masked");
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
        ALIS_DEBUG(EDebugWarning, "Unexpected form data!");
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
                ALIS_DEBUG(EDebugInfo, "\n    tx: %.3d, %.3d", tmpx1, tmpx2);
                ALIS_DEBUG(EDebugInfo, "    ty: %.3d, %.3d", tmpy1, tmpy2);
                ALIS_DEBUG(EDebugInfo, "    tz: %.3d, %.3d", tmpz1, tmpz2);

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
                ALIS_DEBUG(EDebugInfo, "\n    Hit!");
                return 0;
            }
        }
    }
    
    ALIS_DEBUG(EDebugInfo, "\n    No hits!");
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
        ALIS_DEBUG(EDebugWarning, "Unexpected form data!");
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
    
    ALIS_DEBUG(EDebugInfo, "\n    px: %.3d, %.3d", px1, px2);
    ALIS_DEBUG(EDebugInfo, "    py: %.3d, %.3d", py1, py2);
    ALIS_DEBUG(EDebugInfo, "    pz: %.3d, %.3d", pz1, pz2);
    
    s16 test = alis.platform.kind == EPlatformPC ? (s8)xread16(ent_formedata) : xread16(ent_formedata);     ent_formedata += 2;
    if (test < 0)
    {
        return multifirm(ent_vram, ent_formedata, formedata, ent_baseform);
    }
    
    return monofirm(ent_vram, ent_formedata);
}

// v1.0 forms: header byte 0 < 0 composite (byte 1 = count, then sub-form indices),
// 0 = box of 6 signed bytes, > 0 = box of 6 words (x, y, z, dx, dy, dz)

static u32 formbase_v10(u32 vram)
{
    u32 addr = get_0x14_script_org_offset(vram);
    if (alis.platform.kind == EPlatformPC)
    {
        addr += (u16)xread16(addr + 0xe);
        addr += (u16)xread16(addr + 6);
    }
    else
    {
        addr += (u32)xread32(addr + 0xe);
        addr += (u32)xread32(addr + 6);
    }

    return addr;
}

static inline u32 formaddr_v10(u32 base, s16 idx) { return base + (s16)xread16(base + idx * 2); }

// 68k tests the whole header word, x86 only its first byte
static inline s16 formhdr_v10(u32 at) { return alis.platform.kind == EPlatformPC ? (s8)xread8(at) : xread16(at); }

static void formbox_v10(u32 at, s16 hdr, u32 vram, s16 px, s16 py, s16 pz, u8 bug, s16 *b)
{
    s16 x1, y1, z1, x2, y2, z2;
    if (hdr == 0)
    {
        x1 = (s8)xread8(at + 2); y1 = (s8)xread8(at + 3); z1 = (s8)xread8(at + 4);
        x2 = (s8)xread8(at + 5); y2 = (s8)xread8(at + 6); z2 = (s8)xread8(at + 7);
    }
    else
    {
        x1 = xread16(at + 2); y1 = xread16(at + 4); z1 = xread16(at + 6);
        x2 = xread16(at + 8); y2 = xread16(at + 10); z2 = xread16(at + 12);
    }

    u8 inv = get_0x03_xinv(vram) != 0;
    if (inv)
        x1 = -x1;
    
    x1 += px; y1 += py; z1 += pz;
    if (inv)
    {
        // original 68k bug: word box negates x1 again instead of dx
        if (bug && hdr != 0)
        {
            x1 = -x1;
        }
        else
        {
            x2 = -x2;
        }
    }
    
    x2 += x1; y2 += y1; z2 += z1;
    
    s16 t;
    if (y2 <= y1)
    {
        t = y1;
        y1 = y2;
        y2 = t;
    }
    
    if (z2 <= z1)
    {
        t = z1;
        z1 = z2;
        z2 = t;
    }
    
    if (x2 <= x1)
    {
        t = x1;
        x1 = x2;
        x2 = t;
    }
    
    b[0] = x1; b[1] = y1; b[2] = z1; b[3] = x2; b[4] = y2; b[5] = z2;
}

static u8 formhit_ent_v10(u32 at, u32 base, u32 vram, const s16 *own, int depth)
{
    s16 hdr = formhdr_v10(at);
    if (hdr < 0)
    {
        if (depth > 16)
            return 0;
        
        u8 cnt = xread8(at + 1);
        for (int i = 0; i < cnt; i++)
        {
            s16 idx = xread16(at + 2 + i * 2);
            if (idx >= 0 && formhit_ent_v10(formaddr_v10(base, idx), base, vram, own, depth + 1))
                return 1;
        }
        
        return 0;
    }
    
    s16 b[6];
    formbox_v10(at, hdr, vram, xread16(vram + 0), xread16(vram + 2), xread16(vram + 4), 0, b);
    return !(b[4] < own[1] || b[1] > own[4] || b[5] < own[2] || b[2] > own[5] || b[3] < own[0] || b[0] > own[3]);
}

static u8 formhit_own_v10(u32 at, u32 base, u32 ent_form, u32 ent_base, u32 ent_vram, int depth)
{
    s16 hdr = formhdr_v10(at);
    if (hdr < 0)
    {
        if (depth > 16)
            return 0;
        
        u8 cnt = xread8(at + 1);
        for (int i = 0; i < cnt; i++)
        {
            s16 idx = xread16(at + 2 + i * 2);
            if (idx >= 0 && formhit_own_v10(formaddr_v10(base, idx), base, ent_form, ent_base, ent_vram, depth + 1))
                return 1;
        }
        
        return 0;
    }
    
    s16 own[6];
    formbox_v10(at, hdr, alis.script->vram_org, alis.wcx, alis.wcy, alis.wcz, alis.platform.kind != EPlatformPC, own);
    return formhit_ent_v10(ent_form, ent_base, ent_vram, own, 0);
}

static void clipform_v10(void)
{
    alis.ptrent = alis.tablent;
    if (alis.wforme >= 0)
    {
        u32 vram = alis.script->vram_org;
        u32 base = formbase_v10(vram);
        u32 form = formaddr_v10(base, alis.wforme);
        s16 entidx = 0;
        
        do
        {
            u32 ent_vram = xread32(alis.atent + entidx);
            s16 ent_forme = get_0x1a_cforme(ent_vram);
            if ((xread8(ent_vram + 0xc) & (u8)alis.matmask) && xread16(ent_vram + 6) == xread16(vram + 6) && ent_forme >= 0 && ent_vram != vram)
            {
                u32 ent_base = formbase_v10(ent_vram);
                if (formhit_own_v10(form, base, formaddr_v10(ent_base, ent_forme), ent_base, ent_vram, 0))
                {
                    ALIS_DEBUG(EDebugInfo, "\n  hit: entity %.4x", entidx);
                    int index = (int)(alis.ptrent - alis.tablent);
                    if (index >= 127)
                        break;
                    
                    alis.matent[index] = 0;
                    alis.tablent[index] = entidx;
                    alis.ptrent++;
                    if (alis.fallent == 0)
                        break;
                }
            }
            
            entidx = xread16(alis.atent + 4 + entidx);
        }
        while (entidx != 0);
    }
    
    int index = (int)(alis.ptrent - alis.tablent);
    alis.matent[index] = 0;
    alis.tablent[index] = -1;
    alis.ptrent++;
}

// check for intersected object

void clipform(void)
{
    
    if (alis.platform.version == 10)
    {
        clipform_v10();
        return;
    }
    
    alis.ptrent = alis.tablent;
    if (-1 < alis.wforme)
    {
        s32 val = get_0x14_script_org_offset(alis.script->vram_org);
        s32 addr = xread32(val + 0xe) + val;
        alis.baseform = xread32(addr + 6) + addr;
        s32 formedata = (u16)xread16(alis.baseform + (u32)alis.wforme * 2) + alis.baseform;
        s16 entidx = (s16)alis_first_ent();     // v6: skip the null sentinel

        ALIS_DEBUG(EDebugInfo, "\nChecking: %s (%.2x)", alis.script->name, alis.wforme);
        
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
                ALIS_DEBUG(EDebugInfo, "\n  to: %s", sc->name);
                
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
                
                ALIS_DEBUG(EDebugInfo, "    = %d", result);
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
        alis.tablent[index] = ENTLIST_END;
        alis.ptrent++;
    }
}

// Codopname no. 088 opcode 0x57 csuccent
static void csuccent(void)
{

    alis.varD7 = *alis.ptrent;

    if (alis.platform.version >= 60 ? (*alis.ptrent != 0) : (-1 < *alis.ptrent))
        alis.ptrent ++;

    cstore_continue();
}

// Codopname no. 097 opcode 0x60 crstent
static void crstent(void)
{
    alis.ptrent = alis.tablent;
    csuccent();
}

static void v6_tst_from_movevec(void)
{
    u32 vram = alis.script->vram_org;
    alis.v6tstdx = (s16)xread16(vram + 0x26);
    alis.v6tstdy = (s16)xread16(vram + 0x28);
    alis.v6tstdz = (s16)xread16(vram + 0x2a);
}

static void v6_tst_addr_operand(void)
{
    sV6Prot p;
    getprot_x(&p);
    alis.v6tstarg = (s32)p.elem;
}

#include "v6atantab.h"

// the synthetic probe shape DAT_000522fc (elem-record layout, host form)
typedef struct {
    u32 parent;             // +0x08 entity vram (world-position chain root)
    s32 lx, ly, lz;         // +0x0c/10/14 local pos
    u8  type;               // +0x22: 0x33 sphere / 0x35 box
    u8  flip;               // +0x25 bit0 = X-flip
    u16 selfid;             // +0x2a own elem id (self-skip)
    u16 scroff;             // +0x2c screen offset (world list to scan)
    s32 bmin[3];            // box +0x40/44/48 min x/y/z
    s32 bext[3];            // box +0x4c/50/54 extents
    u16 srad;               // sphere +0x40 radius
    s16 scen[3];            // sphere +0x42/44/46 center
} sV6Probe;
static sV6Probe v6probe;

static u8 v6ts_stop;                    // first-hit latch
static u8 v6wk_cbinner;                 // 0 = FUN_00009ed0, 1 = 0x9d38
static u8 v6wk_cbstep;                  // 0 = 0x9c2c (form), 1 = FUN_00009d68 (elem)
static s8 v6wk_d9;                      // saved step count
static u8 v6wk_db;                      // descending intent
static s32 v6wk_bc, v6wk_c0, v6wk_c8;   // bc/c0/c8
static s32 v6wk_dc[3];                  // dc/e0/e4 saved position
static s32 v6ts_boxmin[3], v6ts_boxmax[3];

// reset the found stack
static void v6ts_reset(void)
{
    v6mat_n = 0;
    alis.ptrent = alis.tablent;
    v6ts_stop = 0;
}

int v6ts_push(u16 id, u32 mat)
{
    if (v6mat_n < V6MAT_MAX - 1)
    {
        alis.tablent[v6mat_n] = (s16)id;
        v6mat_id[v6mat_n] = mat;
        v6mat_n++;
    }

    if (alis.fallent == 0)
    {
        v6ts_stop = 1;
        return 1;
    }

    return 0;
}

// terminator at the cursor
static void v6ts_term(void)
{
    if (v6mat_n < V6MAT_MAX)
    {
        alis.tablent[v6mat_n] = 0;
        v6mat_id[v6mat_n] = 0;
    }
}

static void v6ts_chainpos(u32 parent, s32 lx, s32 ly, s32 lz, s32 out[3])
{
    out[0] = lx; out[1] = ly; out[2] = lz;
    for (u32 e = parent; e != 0; e = xread32(e + 0x8))
    {
        out[0] += xread32(e + 0x0c);
        out[1] += xread32(e + 0x10);
        out[2] += xread32(e + 0x14);
    }
}

static void v6ts_worldbox(u32 parent, s32 lx, s32 ly, s32 lz, u8 flip, const s32 bmin[3], const s32 bext[3])
{
    s32 pos[3];
    v6ts_chainpos(parent, lx, ly, lz, pos);
    if (flip & 1)
    {
        v6ts_boxmax[0] = pos[0] - bmin[0];
        v6ts_boxmin[0] = v6ts_boxmax[0] - bext[0];
    }
    else
    {
        v6ts_boxmin[0] = pos[0] + bmin[0];
        v6ts_boxmax[0] = v6ts_boxmin[0] + bext[0];
    }

    v6ts_boxmin[2] = pos[2] + bmin[2];
    v6ts_boxmax[2] = v6ts_boxmin[2] + bext[2];
    v6ts_boxmin[1] = pos[1] + bmin[1];
    v6ts_boxmax[1] = v6ts_boxmin[1] + bext[1];
}

static int v6ts_aabb(u32 cand)
{
    s32 pos[3];
    v6ts_chainpos(xread32(cand + 0x8), xread32(cand + 0x0c), xread32(cand + 0x10), xread32(cand + 0x14), pos);
    s32 cmin, cmax;
    if (xread8(cand + 0x25) & 1)
    {
        cmax = pos[0] - xread32(cand + 0x40);
        cmin = cmax - xread32(cand + 0x4c);
    }
    else
    {
        cmin = pos[0] + xread32(cand + 0x40);
        cmax = cmin + xread32(cand + 0x4c);
    }

    if (cmin > v6ts_boxmax[0] || cmax < v6ts_boxmin[0])
        return 0;
    
    cmin = pos[2] + xread32(cand + 0x48);
    cmax = cmin + xread32(cand + 0x54);
    if (cmin > v6ts_boxmax[2] || cmax < v6ts_boxmin[2])
        return 0;
    
    cmin = pos[1] + xread32(cand + 0x44);
    cmax = cmin + xread32(cand + 0x50);
    if (cmin > v6ts_boxmax[1] || cmax < v6ts_boxmin[1])
        return 0;
    
    return 1;
}

static int v6wk_walking;

static void v6ts_maptest(u32 parent, s32 lx, s32 ly, s32 lz, u16 scroff)
{
    u32 scr = alis.basemain + (s16)xread16(alis.script->vram_org - 0x64);
    if (xread8(scr + 0x5a) != 5 || xread32(scr + 0x5e) != alis.basemain + scroff)
        return;
    
    s32 pos[3];
    v6ts_chainpos(parent, lx, ly, lz, pos);
    u32 mat = 0;
    s32 r = r3do_maptest(scr, pos[0], pos[1], v6ts_boxmin, v6ts_boxmax, v6wk_walking, &mat);
    if (r)
        v6ts_push((u16)r, mat);
}

static void v6ts_scan_box(u32 parent, s32 lx, s32 ly, s32 lz, u8 flip, const s32 bmin[3], const s32 bext[3], u16 selfid, u16 scroff)
{
    int is3do = alis.platform.uid == EGameRobinsonsRequiem3DO;
    v6ts_worldbox(parent, lx, ly, lz, flip, bmin, bext);
    u32 e = alis.basemain + scroff;
    for (;;)
    {
        e = xread32(e + 0x38);
        if (e == 0 || e == xread32(e + 0x38))
            break;
        if ((alis.v6tstmask & (u16)xread16(e + 0x28)) == 0)
            continue;
        
        if (!is3do && (xread8(e + 0x24) & 0x40))
            continue;
        
        if (!is3do && (xread8(e + 0x25) & 0x04))
            continue;
        
        if (xread8(e + 0x22) != 0x35)
            continue;
        
        if ((u16)xread16(e + 0x2a) == selfid)
            continue;
        
        if (!v6ts_aabb(e))
            continue;
        
        u32 mat = (u16)xread16(e + 0x26) | ((u32)(u16)xread16(e + 0x28) << 16);
        if (v6ts_push((u16)xread16(e + 0x2a), mat))
            return;
    }

    if (is3do)
        v6ts_maptest(parent, lx, ly, lz, scroff);
}

static void v6ts_scan_sphere3do(u16 selfid, u16 scroff)
{
    for (u32 e = alis.basemain + scroff; ; )
    {
        e = xread32(e + 0x38);
        if (e == 0 || e == xread32(e + 0x38))
            break;
        
        if ((alis.v6tstmask & (u16)xread16(e + 0x28)) == 0 || (u16)xread16(e + 0x2a) == selfid)
            continue;
        
        if (xread8(e + 0x22) != 0x33 && xread8(e + 0x22) != 0x30)
            continue;
        
        if (v6ts_push((u16)xread16(e + 0x2a), (u16)xread16(e + 0x26)))
            return;
    }
}

static void v6ts_scan_shape(void)
{
    if (v6probe.type == 0x33)
    {
        v6col_sphere_scan(v6probe.parent, v6probe.srad, v6probe.scen[0], v6probe.scen[1], v6probe.scen[2], v6probe.selfid, v6probe.scroff);
        return;
    }

    v6ts_scan_box(v6probe.parent, v6probe.lx, v6probe.ly, v6probe.lz, v6probe.flip, v6probe.bmin, v6probe.bext, v6probe.selfid, v6probe.scroff);
}

static void v6ts_scan_elem(u32 probe)
{
    u8 t = xread8(probe + 0x22);
    if (alis.platform.uid == EGameRobinsonsRequiem3DO && (t == 0x33 || t == 0x30))
    {
        if (t == 0x33)
        {
            v6ts_scan_sphere3do((u16)xread16(probe + 0x2a), (u16)xread16(probe + 0x2c));
            return;
        }

        t = 0x35;
    }

    if (t == 0x33)
    {
        v6col_sphere_scan(probe, (u16)xread16(probe + 0x40), (s16)xread16(probe + 0x42), (s16)xread16(probe + 0x44), (s16)xread16(probe + 0x46), (u16)xread16(probe + 0x2a), (u16)xread16(probe + 0x2c));
        return;
    }

    if (t == 0x30)
    {
        u32 res = xread32(probe + 0x3c);
        res += xread32(res);
        v6col_sphere_scan(probe, (u16)xread16(res + 0x9a), (s16)xread16(res + 0x94), (s16)xread16(res + 0x96), (s16)xread16(res + 0x98), (u16)xread16(probe + 0x2a), (u16)xread16(probe + 0x2c));
        return;
    }

    s32 bmin[3] = { xread32(probe + 0x40), xread32(probe + 0x44), xread32(probe + 0x48) };
    s32 bext[3] = { xread32(probe + 0x4c), xread32(probe + 0x50), xread32(probe + 0x54) };
    v6ts_scan_box(xread32(probe + 0x8), xread32(probe + 0xc), xread32(probe + 0x10), xread32(probe + 0x14), xread8(probe + 0x25), bmin, bext, (u16)xread16(probe + 0x2a), (u16)xread16(probe + 0x2c));
}

static void v6ts_scan_ownelems(void)
{
    u32 vram = alis.script->vram_org;
    u32 head = v6_elemhead(vram);
    if (head == 0)
        return;
    
    u32 e = xread32(head + 4);
    if (e == 0 || e == xread32(e + 4))
        return;
    
    u8 count = xread8(e + 0x33);
    for (u8 i = 0; i < count; i++, e += V6_SLOT_STRIDE)
    {
        if (xread8(e + 0x22) == 0x35)
        {
            v6ts_scan_elem(e);
            if (v6ts_stop)
                break;
        }
    }
}

static void v6_polar(u32 vram, s32 dx, s32 dy, s32 dz)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        r3do_polar(vram, dx, dy, dz);
        return;
    }

    sV6Ctx c;
    v6tf_build(vram, xread32(vram + 0x8), &c);
    s32 v[3] = { dx, dy, dz };
    v6tf_apply(v, &c, v);
    xwrite32(vram + 0x0c, v[0]);
    xwrite32(vram + 0x10, v[1]);
    xwrite32(vram + 0x14, v[2]);
}

static void v6wk_polar_add(u32 vram)
{
    v6_polar(vram, alis.v6tstdx, alis.v6tstdy, alis.v6tstdz);
}

#define V6TS_OWN_ELEMS 1u
static void v6ts_driver(u32 elemprobe)
{
    u32 vram = alis.script->vram_org;
    v6ts_reset();
    s32 n = (s32)(s8)alis.v6tstflag;
    if (n > 0)
    {
        for (; n > 0; n--)
        {
            s32 sx = xread32(vram + 0x0c), sy = xread32(vram + 0x10), sz = xread32(vram + 0x14);
            if (alis.v6tstpolar == 0)
            {
                xwrite32(vram + 0x0c, sx + alis.v6tstdx);
                xwrite32(vram + 0x10, sy + alis.v6tstdy);
                xwrite32(vram + 0x14, sz + alis.v6tstdz);
            }
            else
            {
                v6wk_polar_add(vram);
            }

            if (elemprobe == 0)
            {
                v6ts_scan_shape();
            }
            else if (elemprobe == V6TS_OWN_ELEMS)
            {
                v6ts_scan_ownelems();
            }
            else
            {
                v6ts_scan_elem(elemprobe);
            }

            if (v6mat_n != 0)
            {
                xwrite32(vram + 0x0c, sx);
                xwrite32(vram + 0x10, sy);
                xwrite32(vram + 0x14, sz);
                break;
            }
        }
    }
    else
    {
        n = (n == 0) ? 1 : -n;
        s32 ox = xread32(vram + 0x0c), oy = xread32(vram + 0x10), oz = xread32(vram + 0x14);
        for (; n > 0; n--)
        {
            if (alis.v6tstpolar)
            {
                v6wk_polar_add(vram);
            }
            else
            {
                xwrite32(vram + 0x0c, xread32(vram + 0x0c) + alis.v6tstdx);
                xwrite32(vram + 0x10, xread32(vram + 0x10) + alis.v6tstdy);
                xwrite32(vram + 0x14, xread32(vram + 0x14) + alis.v6tstdz);
            }

            if (elemprobe == 0)
            {
                v6ts_scan_shape();
            }
            else if (elemprobe == V6TS_OWN_ELEMS)
            {
                v6ts_scan_ownelems();
            }
            else
            {
                v6ts_scan_elem(elemprobe);
            }

            if (v6mat_n != 0)
                break;
        }

        xwrite32(vram + 0x0c, ox);
        xwrite32(vram + 0x10, oy);
        xwrite32(vram + 0x14, oz);
    }

    v6ts_term();
}

static void v6ts_shape_and_scan(s32 form)
{
    u32 vram = alis.script->vram_org;
    if (form < 0)
    {
        v6wk_cbinner = 1;
        v6ts_driver(V6TS_OWN_ELEMS);
        alis.fallent = 0;
        return;
    }

    s32 desc = adresdes((s16)form);
    u32 fd = (u32)desc + xread32(desc);             // resolved form data
    v6probe.parent = vram;
    v6probe.scroff = (u16)xread16(vram - 0x16);     // entity's screen offset
    v6probe.selfid = (u16)xread16(vram + 0x3a);     // entity's elem id
    v6probe.lx = v6probe.ly = v6probe.lz = 0;
    v6probe.flip = 0;
    u8 ft = xread8(fd);                             // form type
    if (ft == 7)
    {
        v6probe.type = 0x33;
        v6probe.scen[0] = (s16)xread16(fd + 0x0c);
        v6probe.scen[1] = (s16)xread16(fd + 0x0e);
        v6probe.scen[2] = (s16)xread16(fd + 0x10);
        v6probe.srad = (u16)xread16(fd + 0x12);
    }
    else if (ft == 8)
    {
        v6probe.type = 0x33;
        v6probe.scen[0] = (s16)xread16(fd + 0x94);
        v6probe.scen[1] = (s16)xread16(fd + 0x96);
        v6probe.scen[2] = (s16)xread16(fd + 0x98);
        v6probe.srad = (u16)xread16(fd + 0x9a);
    }
    else
    {
        v6probe.type = 0x35;
        v6probe.bmin[0] = (s16)xread16(fd + 0x0c);  v6probe.bext[0] = (s16)xread16(fd + 0x12);
        v6probe.bmin[1] = (s16)xread16(fd + 0x0e);  v6probe.bext[1] = (s16)xread16(fd + 0x14);
        v6probe.bmin[2] = (s16)xread16(fd + 0x10);  v6probe.bext[2] = (s16)xread16(fd + 0x16);
        v6probe.flip = xread8(vram + 0x24);         // invert bits
    }

    v6wk_cbinner = 0;
    v6ts_driver(0);
    alis.fallent = 0;
}

static int v6wk_atan2(s32 a, s32 b, s32 *angle, s32 *mag)
{
    if (a == 0 && b == 0)
        return 0;
    
    u16 cx = 0; int neg = 0;
    if (a < 0)
    {
        cx = (u16)(cx + 0x800);
        a = -a;
        b = -b;
    }

    if (b < 0)
    {
        neg = !neg;
        b = -b;
    }

    if (a < b)
    {
        cx = (u16)(cx + 0x400);
        if (neg)
            cx = (u16)(cx - 0x800);
        
        neg = !neg;
        s32 t = a; a = b; b = t;
    }

    u32 q = (u32)(((u64)(u32)b << 10) / (u32)a);
    s32 ax = (s16)v6atantab[q > 1024 ? 1024 : q];
    if (neg)
        ax = -ax;
    
    ax = (s32)((u16)(ax + cx));
    ax &= 0xfff; ax <<= 4;
    *angle = ax;
    *mag = (s32)v6x_isqrt((u32)((u32)a * (u32)a + (u32)b * (u32)b));
    return 1;
}

static s32 v6wk_polarize_pitch(s32 dx, s32 dy, s32 dz)
{
    s32 ang = 0, mag = 0;
    if (!v6wk_atan2(dy, dx, &ang, &mag))
        mag = 0;
    
    s32 pitch = 0, dist = 0;
    if (!v6wk_atan2(mag, dz, &pitch, &dist))
        pitch = 0;
    
    return pitch;
}

void v6x_focangle(u32 scr)
{
    s32 dxl = (s16)xread16(scr + 0x62) - (s16)xread16(scr + 0x66);
    if (dxl < 0)
        dxl = -dxl;
    
    s32 dxr = (s16)xread16(scr + 0x66) + (s16)xread16(scr + 0x6e) - (s16)xread16(scr + 0x62);
    if (dxr < 0)
        dxr = -dxr;
    
    s32 dx = dxl > dxr ? dxl : dxr;
    s32 dyt = (s16)xread16(scr + 0x64) - (s16)xread16(scr + 0x68);
    if (dyt < 0)
        dyt = -dyt;
    
    s32 dyb = (s16)xread16(scr + 0x68) + (s16)xread16(scr + 0x70) - (s16)xread16(scr + 0x64);
    if (dyb < 0)
        dyb = -dyb;
    
    s32 dy = dyt > dyb ? dyt : dyb;
    u32 r = v6x_isqrt((u32)(u16)dx * (u16)dx + (u32)(u16)dy * (u16)dy);
    s32 f = (s16)xread16(scr + 0x48) < (s16)xread16(scr + 0x4c) ? (s16)xread16(scr + 0x48) : (s16)xread16(scr + 0x4c);
    if (f < 0)
        f = -f;
    
    s32 ang = 0, mag = 0;
    v6wk_atan2(f, (s32)r, &ang, &mag);
    xwrite16(scr + 0x176, (u16)ang);
}

// steppers ground-probe callback dispatch
static void v6wk_probe(void)
{
    if (v6wk_cbstep == 0)
    {
        v6ts_shape_and_scan(alis.v6walkb8);
    }
    else
    {
        v6wk_cbinner = 0;
        if (alis.v6walkb8 > 0)
        {
            v6ts_driver((u32)alis.v6walkb8);
        }
        else
        {
            v6ts_driver(V6TS_OWN_ELEMS);
        }

        alis.fallent = 0;
    }
}

static int v6wk_blocking(void)
{
    if (v6mat_n == 0)
        return 0;
    
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        return 0;
    
    u16 w = (u16)(v6mat_id[v6mat_n - 1] >> 16);
    return (s16)(w & alis.v6tstmask) < 0;
}

static s32 v6wk_stepper(void)
{
    u32 vram = alis.script->vram_org;
    s32 cc = alis.v6walkcc, c4 = alis.v6walkc4;
    if (c4 <= cc)
        cc = c4;
    
    s32 d0 = alis.v6walkd0;
    int is3do = alis.platform.uid == EGameRobinsonsRequiem3DO;
    s32 fall = is3do ? 2 * c4 : c4;
    if (is3do)
    {
        if (alis.v6walkmode != 1)
            xwrite32(vram + 0x14, xread32(vram + 0x14) - d0);
        
        v6wk_walking = 1;
    }
    else
    {
        switch (alis.v6walkmode)
        {
            case 0: xwrite32(vram + 0x14, xread32(vram + 0x14) - d0); break;    // body-height bias
            case 1: v6x_walkzoff = d0; break;                                   // camera-Z, then walk
            case 2: v6x_walkaoff = (s16)d0; break;                              // camera-pitch, then walk
            case 3: v6x_walkzoff = d0; return 0;                                // set only, NO probe
            case 4: v6x_walkaoff = (s16)d0; return 0;                           // set only, NO probe
            default: return 0;
        }
    }

    v6wk_dc[0] = xread32(vram + 0x0c);              // save start position
    v6wk_dc[1] = xread32(vram + 0x10);
    v6wk_dc[2] = xread32(vram + 0x14);
    v6wk_db = 0;
    s32 zt = (s16)xread16(vram + 0x2a);             // move-vector Z
    if (zt <= 0)
    {
        if (zt == 0)
            zt = -1;
        
        zt = -zt; v6wk_db = 1;                      // descending intent
    }

    v6wk_c8 = zt;                                   // step tolerance >= 1

    u8 d8 = alis.v6walkd8;                          // DEC-BYTE loop counter
    v6wk_d9 = (s8)alis.v6walkd8;

    for (;;)                                        // outer per-step loop
    {
        alis.v6tstdz = -v6wk_c8;                    // test: move + step down
        alis.v6tstflag = 0;
        v6wk_probe();
        if (v6mat_n != 0)
        {
            // contact within c8 below the moved position
            if (v6wk_blocking())
                break;
            
            alis.v6tstflag = 1; alis.v6tstdz = 0;   // try APPLY flat
            v6wk_probe();
            if (v6mat_n != 0)                       // still blocked -> climb
            {
                alis.v6tstdz = 0; alis.v6tstflag = 0;
                s32 up;
                for (;;)
                {
                    up = cc + alis.v6tstdz;
                    if (up > c4)
                        goto out;                   // blocked wall -> stop
                    
                    alis.v6tstdz = up;
                    v6wk_probe();
                    if (v6mat_n == 0)
                        break;                      // free above
                }

                v6wk_bc = alis.v6tstdz;             // FREE / HIT bounds
                v6wk_c0 = alis.v6tstdz - cc;
                goto bisect;
            }

            // applied flat -> next step
        }
        else
        {
            // nothing at -c8: coarse-descend
            alis.v6tstdz = 0; alis.v6tstflag = 0;
            int ground = 0;
            for (;;)
            {
                s32 depth = cc - alis.v6tstdz;      // negative
                if (depth > fall)                   // drop exceeds limit
                {
                    // fellThrough
                    if (v6wk_db != 1)
                    {
                        alis.v6tstflag = 1;         // APPLY at deepest free
                        v6wk_probe();
                    }

                    // cursor NOT advanced, next pair cleared
                    if (v6mat_n < V6MAT_MAX - 1)
                    {
                        alis.tablent[v6mat_n] = -2;
                        v6mat_id[v6mat_n] = 0;
                        alis.tablent[v6mat_n + 1] = 0;
                        v6mat_id[v6mat_n + 1] = 0;
                    }

                    goto out;
                }

                alis.v6tstdz = -depth;
                v6wk_probe();
                if (v6mat_n != 0)
                {
                    ground = 1;
                    break;
                }
            }

            if (ground)
            {
                if (v6wk_blocking())
                    break;
                
                v6wk_c0 = alis.v6tstdz;             // HIT bound
                v6wk_bc = alis.v6tstdz + cc;        // FREE bound
                goto bisect;
            }
        }

    step:
        if (--d8 != 0)
            continue;                               // next step this frame
        
        break;

    bisect:
        alis.v6tstflag = 0;
        for (;;)
        {
            alis.v6tstdz = (v6wk_bc + v6wk_c0) >> 1;    // SAR midpoint
            if (alis.v6tstdz - v6wk_c0 <= v6wk_c8)      // converged
                break;
            
            v6wk_probe();
            if (v6mat_n == 0)
            {
                v6wk_bc = alis.v6tstdz;
            }
            else
            {
                v6wk_c0 = alis.v6tstdz;
            }
        }

        alis.v6tstdz = v6wk_bc;                     // APPLY at free bound
        alis.v6tstflag = 1;
        v6wk_probe();
        if (v6mat_n == 0)                           // landed
            goto step;
        
        break;
    }

out:;
    s32 ret = 0;
    if (v6wk_d9 <= 0)                               // test-only walk:
    {
        xwrite32(vram + 0x0c, v6wk_dc[0]);          // restore start position
        xwrite32(vram + 0x10, v6wk_dc[1]);
        xwrite32(vram + 0x14, v6wk_dc[2]);
    }

    if (is3do)
    {
        if (alis.v6walkmode == 0)
            xwrite32(vram + 0x14, xread32(vram + 0x14) + d0);
        v6wk_walking = 0;
        return 0;
    }

    if (alis.v6walkmode == 0)
    {
        xwrite32(vram + 0x14, xread32(vram + 0x14) + d0);   // undo bias
        return 0;
    }

    ret = v6wk_polarize_pitch(xread32(vram + 0x0c) - v6wk_dc[0], xread32(vram + 0x10) - v6wk_dc[1], xread32(vram + 0x14) - v6wk_dc[2]);
    return ret;                                     // pitch angle
}

static void v6_tstscan(void)
{
    v6ts_shape_and_scan(alis.v6tstarg);
}

static void v6_tstscan_elem(void)
{
    v6wk_cbinner = 0;
    if (alis.v6tstarg > 0)
    {
        v6ts_driver((u32)alis.v6tstarg);
    }
    else
    {
        v6ts_driver(V6TS_OWN_ELEMS);
    }

    alis.fallent = 0;
}

// Codopname no. 084 opcode 0x53 ctstmov
static void ctstmov(void)
{
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

// v6 dispatch target for ctstmov
static void ctstmov_v6(void)
{
    readexec_opername_saveD7();
    alis.v6tstdx = (s32)alis.varD7;
    readexec_opername_saveD7();
    alis.v6tstdy = (s32)alis.varD7;
    readexec_opername_saveD7();
    alis.v6tstdz = (s32)alis.varD7;
    readexec_opername_saveD7();
    alis.v6tstflag = (u8)alis.varD7;            // DL, byte
    readexec_opername_saveD7();
    alis.v6tstmask = (u16)alis.varD7;           // DX, word
    readexec_opername_saveD7();
    alis.v6tstarg = alis.varD7;                 // scan argument
    v6_tstscan();
    crstent();
}

// Codopname no. 085 opcode 0x54 ctstset
static void ctstset(void)
{
    if (alis.platform.version == 10)
    {
        readexec_opername();
        alis.wcx = alis.varD7;
        readexec_opername();
        alis.wcy = alis.varD7;
        readexec_opername();
        alis.wcz = alis.varD7;
        readexec_opername();
        alis.matmask = alis.varD7;
        alis.wforme = get_0x1a_cforme(alis.script->vram_org);
        clipform();
        crstent();
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// v6 dispatch target for ctstset
static void ctstset_v6(void)
{
    u32 vram = alis.script->vram_org;
    readexec_opername_saveD7();
    alis.v6tstdx = (s32)alis.varD7 - xread32(vram + 0x0c);
    readexec_opername_saveD7();
    alis.v6tstdy = (s32)alis.varD7 - xread32(vram + 0x10);
    readexec_opername_saveD7();
    alis.v6tstdz = (s32)alis.varD7 - xread32(vram + 0x14);
    alis.v6tstflag = 0;
    readexec_opername_saveD7();
    alis.v6tstmask = (u16)alis.varD7;
    readexec_opername_saveD7();
    alis.v6tstarg = alis.varD7;
    v6_tstscan();
    crstent();
}

// Codopname no. 086 opcode 0x55 cftstmov
static void cftstmov(void)
{
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

// v6 dispatch target for cftstmov
static void cftstmov_v6(void)
{
    readexec_opername_saveD7(); alis.v6tstdx = (s32)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstdy = (s32)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstdz = (s32)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstflag = (u8)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    v6_tst_addr_operand();
    v6_tstscan_elem();
    crstent();
}

// Codopname no. 087 opcode 0x56 cftstset
static void cftstset(void)
{
    if (alis.platform.version == 10)
    {
        // v1.0 handler is relative, same as cftstmov
        cftstmov();
        return;
    }
    
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

// v6 dispatch target for cftstset
static void cftstset_v6(void)
{
    u32 vram = alis.script->vram_org;
    readexec_opername_saveD7(); alis.v6tstdx = (s32)alis.varD7 - xread32(vram + 0x0c);
    readexec_opername_saveD7(); alis.v6tstdy = (s32)alis.varD7 - xread32(vram + 0x10);
    readexec_opername_saveD7(); alis.v6tstdz = (s32)alis.varD7 - xread32(vram + 0x14);
    alis.v6tstflag = 0;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    v6_tst_addr_operand();
    v6_tstscan_elem();
    crstent();
}

// Codopname no. 089 opcode 0x58 cpredent
static void cpredent(void)
{
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
    if (alis.platform.version == 10)
    {
        // v1.0: 16-bit deltas; view drops the y term (68k adds it to d7) and truncates the dot to 16 bits
        s16 dx = wcx, dy = wcy, dz = wcz;
        *val = dx * dx + dy * dy + dz * dz;
        if (*val > (s32)alis.valnorme)
            return 1;

        if (alis.fview == 0)
            return 0;

        alis.varD7 += (s8)xread8(source + ALIS_SCR_WCY2) * dy;
        s16 dot = (s8)xread8(source + ALIS_SCR_WCX2) * dx + (s8)xread8(source + ALIS_SCR_WCZ2) * dz;
        return *val <= dot * (s32)alis.valchamp ? 0 : 1;
    }

    *val = (wcx * wcx) + (wcy * wcy) + (wcz * wcz);
    return (*val <= alis.valnorme && (alis.fview == 0 || *val <= ((s8)xread8(source + ALIS_SCR_WCX2) * wcx + (s8)xread8(source + ALIS_SCR_WCY2) * wcy + (s8)xread8(source + ALIS_SCR_WCZ2) * wcz) * (s32)alis.valchamp)) ? 0 : 1;
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

static void findent(u8 bytyp, u8 near)
{
    u32 src_vram = alis.script->vram_org;
    s16 tabidx = 0;
    s16 entidx = 0;
    s32 value;
    
    do
    {
        u32 tgt_vram = xread32(alis.atent + entidx);
        if ((bytyp ? alis.varD7 == (s16)get_0x10_script_id(tgt_vram) : (xread8(tgt_vram + 0xc) & (u8)alis.matmask) != 0)
            && (!near || xread16(tgt_vram + 6) == xread16(src_vram + 6)) && src_vram != tgt_vram
            && (!near || !calcnear(src_vram, tgt_vram, &value)) && tabidx < 127)
        {
            alis.tablent[tabidx++] = entidx;
            if (alis.fallent == 0)
                break;
        }
    }
    while ((entidx = xread16(alis.atent + 4 + entidx)));
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    crstent();
}

void sviewtyp(void)
{
    if (alis.platform.version == 10)
    {
        findent(1, 1);
        return;
    }
    
    s16 id = alis.varD7;
    if (id < 0)
        id &= 0xff;
    
    s16 bufidx = 0;
    s32 *buffer32 = (s32 *)alis.buffer;
    
    if (alis.fallent == 0)
    {
        buffer32[bufidx] = 0x7fffffff;
        alis.tablent[bufidx] = 0xffff;

        bufidx = 1;
    }
    
    u8 veradd = alis.platform.version >= 30 ? 0x30 : 0x6;

    s16 entidx = (s16)alis_first_ent();     // v6: skip the null sentinel
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
                if (alis.fallent || buffer32[bufidx - 1] > value)
                {
                    if (alis.fallent == 0)
                    {
                        bufidx --;
                    }
                    
                    buffer32[bufidx] = value;
                    alis.tablent[bufidx] = entidx;
                    
                    bufidx ++;
                }
            }
        }
    }
    while ((entidx = xread16(alis.atent + 4 + entidx)));
    
    alis.tablent[bufidx] = ENTLIST_END;
    if (alis.fallent != 0)
    {
        trinorme(bufidx);
    }
    
    alis.fallent = 0;
    crstent();
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
    if (alis.platform.version == 10)
    {
        findent(0, 1);
        return;
    }
    
    s16 bufidx = 0;
    if (alis.fallent == 0)
    {
        ((s32 *)alis.buffer)[bufidx] = 0x7fffffff;
        alis.tablent[bufidx] = 0xffff;

        bufidx = 1;
    }

    u8 veradd = alis.platform.version >= 30 ? 0x30 : 0x6;

    s16 entidx = (s16)alis_first_ent();     // v6: skip the null sentinel
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
            alis.tablent[bufidx] = ENTLIST_END;
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
        
        s16 entidx = (s16)alis_first_ent();     // v6: skip the null sentinel
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
                alis.tablent[bufidx] = ENTLIST_END;
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
        
        alis.tablent[tabidx] = ENTLIST_END;
        alis.fallent = 0;
        crstent();
    }
}

static void near_v6(int kind)
{
    int is3do = alis.platform.uid == EGameRobinsonsRequiem3DO;
    if (is3do)
    {
        readexec_opername_saveD7();
    }
    else
    {
        readexec_opername();
    }

    u32 r2 = (u32)alis.varD7 * (u32)alis.varD7;
    char name[8];
    s32 key = 0;
    if (kind == 0)
    {
        readexec_opername_swap();
        memset(name, 0, sizeof(name));
        strncpy(name, alis.sd7, sizeof(name));
    }
    else
    {
        if (is3do)
        {
            readexec_opername_saveD7();
        }
        else
        {
            readexec_opername();
        }

        key = kind == 1 && alis.fallent ? -1 : (s32)alis.varD7;
    }

    u32 src = alis.script->vram_org;
    s16 world = (s16)xread16(src - 0x16);
    s32 tabidx = 0;
    s16 entidx = (s16)alis_first_ent();
    do
    {
        u32 tgt = xread32(alis.atent + entidx);
        int match = tgt != 0 && (s16)xread16(tgt - 0x16) == world;
        if (match && kind == 0)
        {
            match = memcmp(alis.mem + tgt + 0x3c, name, 8) == 0;
        }
        else if (match && kind == 1)
        {
            match = key <= 0 || entidx == key;
        }
        else if (match)
        {
            match = (s16)xread16(tgt + (is3do ? 0x48 : 0x46)) == (s16)key;
        }

        if (match && kind == 1 && !is3do)
            match = tgt != src;
        if (match)
        {
            u64 d2 = 0;
            for (int i = 0; is3do && i < 3; i++)
            {
                s64 d = (s64)(s32)xread32(tgt + 0xc + i * 4) - (s32)xread32(src + 0xc + i * 4);
                d2 += (u64)(d * d);
            }

            if (d2 <= r2)
            {
                alis.matent[tabidx] = 0;
                alis.tablent[tabidx] = entidx;
                tabidx++;
                if (alis.fallent == 0)
                    break;
            }
        }

        entidx = xread16(alis.atent + 4 + entidx);
    }
    while (entidx != 0);
    alis.tablent[tabidx] = ENTLIST_END;
    alis.fallent = 0;
    crstent();
}

// Codopname no. 090 opcode 0x59 cnearent
void cnearent(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(1);
        return;
    }

    alis.fview = 0;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    sviewent();
}

// Codopname no. 091 opcode 0x5a cneartyp
static void cneartyp(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(0);
        return;
    }

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
static void cnearmat(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(2);
        return;
    }

    alis.fview = 0;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    sviewmat();
}

// Codopname no. 093 opcode 0x5c cviewent
static void cviewent(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(1);
        return;
    }

    alis.fview = 1;
    readexec_opername();
    alis.valnorme = (u32)alis.varD7 * (u32)alis.varD7;
    readexec_opername();
    alis.valchamp = alis.varD7;
    readexec_opername();
    sviewent();
}

// Codopname no. 094 opcode 0x5d cviewtyp
static void cviewtyp(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(0);
        return;
    }

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
static void cviewmat(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(2);
        return;
    }

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
static void corient(void)
{
    if (alis.platform.version >= 60)
        return;
    
    readexec_opername();
    alis.varD7 *= 3;

    int addr = get_0x20_set_vect(alis.script->vram_org) == 0 ? alis.vstandard + 1 : get_0x20_set_vect(alis.script->vram_org) + get_0x14_script_org_offset(alis.script->vram_org) + 1;
    xwrite8(alis.script->vram_org + ALIS_SCR_WCX2, xread8(addr + 0 + alis.varD7));
    xwrite8(alis.script->vram_org + ALIS_SCR_WCY2, xread8(addr + 1 + alis.varD7));
    xwrite8(alis.script->vram_org + ALIS_SCR_WCZ2, xread8(addr + 2 + alis.varD7));
}

// Codopname no. 098 opcode 0x61 csend
static void csend(void)
{
    s16 length = script_read8();

    do
    {
        readexec_opername();

        s16 entry = alis.varD7;
        if (entry == -1)
            break;

        s32 vram = xread32(alis.atent + entry);
        if (vram == 0)
            break;

        if ((get_0x24_scan_inter(vram) & 1) != 0)
            break;

        s16 limit = alis.platform.version >= 30 ? -0x3f : -0x35;

        s16 offset = xread16(get_0x14_script_org_offset(vram) + 0x16);
        s16 scanclr1 = get_0x1c_scan_clr(vram);
        s16 scanclr2;

        while (true)
        {
            readexec_opername();

            scanclr2 = scanclr1 + 2;
            if (limit < scanclr2)
            {
                scanclr2 -= offset;
            }

            if (scanclr2 == get_0x1e_scan_clr(vram))
            {
                length--;
                if (alis.platform.version == 10)
                    scanclr2 = get_0x1c_scan_clr(vram);
                break;
            }

            xwrite16(vram + scanclr1, alis.varD7);

            scanclr1 = scanclr2;

            if ((--length) == -1)
            {
                set_0x1c_scan_clr(vram, scanclr2);
                set_0x24_scan_inter(vram, get_0x24_scan_inter(vram) | 0x80);
                return;
            }
        }

        set_0x1c_scan_clr(vram, scanclr2);
    }
    while (false);

    while ((length--) != -1)
    {
        readexec_opername();
    }
}

static void v6_enqueue(s16 id, s32 val)
{
    if (id <= 0)
        return;

    u32 vram = xread32(alis.atent + id);
    if (vram == 0)                                  // dead entity
        return;

    if (alis.platform.kind != EPlatform3DO && (xread8(V6_FARSLEEP(vram)) & V6_DORMANT))
        return;                                     // 3DO has no test

    if ((get_0x24_scan_inter(vram) & 1) != 0)
        return;

    u16 qsize = (u16)xread16(get_0x14_script_org_offset(vram) + 0x36);
    s16 tail = get_0x1e_scan_clr(vram);
    s16 next = tail + 4;
    if (next >= -0x40)
        next -= qsize;

    if (next != get_0x1c_scan_clr(vram))
    {
        xwrite32(vram + tail, val);
        tail = next;
    }

    set_0x1e_scan_clr(vram, tail);
    set_0x24_scan_inter(vram, get_0x24_scan_inter(vram) | 0x80);
}

// v6 dispatch target for csend
static void csend_v6(void)
{
    s16 count = script_read8();
    readexec_opername();
    s16 id = alis.varD7;
    for (s16 i = 0; i <= count; i++)
    {
        readexec_opername();
        v6_enqueue(id, alis.varD7);
    }
}

// Codopname no. 103 opcode 0x66 cscanclr
static void cscanclr(void)
{
    set_0x1e_scan_clr(alis.script->vram_org, get_0x1c_scan_clr(alis.script->vram_org));
    if (alis.platform.version != 10)
        set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0x7f);
}

// v6 dispatch target for cscanclr
static void cscanclr_v6(void)
{
    set_0x1c_scan_clr(alis.script->vram_org, get_0x1e_scan_clr(alis.script->vram_org));
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0x7f);
}

// Codopname no. 099 opcode 0x62 cscanon
static void cscanon(void)
{
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfe);
}

// Codopname no. 100 opcode 0x63 cscanoff
static void cscanoff(void)
{
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 1);
    if (alis.platform.version != 10)
        cscanclr();
}

// Codopname no. 101 opcode 0x64 cinteron
static void cinteron(void)
{
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfd);
}

// Codopname no. 102 opcode 0x65 cinteroff
static void cinteroff(void)
{
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 2);
}

// Codopname no. 104 opcode 0x67 callentity
static void callentity(void)
{
    alis.fallent = 1;
}

// Codopname no. 105 opcode 0x68 cpalette
static void cpalette(void)
{
    alis.flagmain = 0;

    readexec_opername();
    s16 palidx = alis.varD7;
    if (alis.platform.version == 10)
    {
        // keeps the BIOS CGA palette
        u8 *paldata = alis.mem + adresdes(palidx);
        if (alis.platform.kind != EPlatformPC && paldata[0] == 0xfe)
            topalette(paldata, 0);
    }
    else if (dos_pal_active())
    {
        s32 addr = palidx < 0 ? 0 : adresdes(palidx);
        dos_cpalette(palidx, palidx < 0 ? NULL : alis.mem + addr + xread32(addr));
    }
    else if (palidx < 0)
    {
        if (alis.platform.version < 60 || palidx > -3)
            restorepal(palidx, 0);
    }
    else
    {
        s32 addr = adresdes(palidx);
        u8 *paldata = alis.mem + addr + xread32(addr);
        if (alis.platform.version >= 60 && paldata[0] != 3)
            return;
        
        topalette(paldata, 0);
    }
}

// Codopname no. 106 opcode 0x69 cdefcolor
static void cdefcolor(void)
{
    readexec_opername();
    readexec_opername_saveD6();

    // big-endian color word, as RGB9/RGB12 expect
    u8 color[2] = { (u8)(alis.varD6 >> 8), (u8)alis.varD6 };
    u8 *rawcolor = color;
    s16 index = alis.varD7;

    if (dos_pal_active())
    {
        dos_cdefcolor(index, (u16)alis.varD6);
        return;
    }

    if (alis.platform.kind == EPlatformAmiga || alis.platform.kind == EPlatformAmigaAGA)
    {
        PAL_WRITE_RGB_AT(image.mpalet, index, RGB12(rawcolor));
    }
    else
    {
        PAL_WRITE_RGB_AT(image.mpalet, index, RGB9(rawcolor));
    }
    
    setmpalet();
    set_update_cursor();
}

// Codopname no. 107 opcode 0x6a ctiming
static void ctiming(void)
{
    readexec_opername();
    alis.ctiming = (u8)(alis.varD7 & 0xff);
}

static void ymsfx(eChannelType type, u8 vol, int zap)
{
    u8 pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    u8 priorson = alis.varD7;
    readexec_opername();
    u16 freqson = alis.varD7;
    readexec_opername();
    u16 longson = alis.varD7;
    s16 dfreqson = 0;
    if (zap)
    {
        readexec_opername();
        dfreqson = alis.varD7;
    }
    
    if (longson == 0)
    {
        return;
    }
    
    s16 volson = vol << 8;
    s16 dvolson = 0;
    if (dfreqson == 0)
    {
        dvolson = volson / longson;
        if (dvolson == 0)
            dvolson = 1;
    }
    
    runson(type, pereson, priorson, volson, freqson, longson, -dvolson, dfreqson);
}

// Deus DOS czap/cexplode/cding/cnoise only read their operands
static int psg_ops_dos_v6(int operands)
{
    if (alis.platform.version < 60 || alis.platform.uid == EGameRobinsonsRequiem3DO)
        return 0;
    
    while (operands--)
        readexec_opername();
    
    return 1;
}

// Codopname no. 108 opcode 0x6b czap
static void czap(void)
{
    if (psg_ops_dos_v6(5))
    {
        return;
    }

    if (alis.platform.version == 10)
    {
        ymsfx(eChannelTypeDingZap, 0xf, 1);
        return;
    }

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
static void cexplode(void)
{
    if (psg_ops_dos_v6(4))
    {
        return;
    }

    if (alis.platform.version == 10)
    {
        ymsfx(eChannelTypeExplode, 0xf, 0);
        return;
    }

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
static void cding(void)
{
    if (psg_ops_dos_v6(4))
    {
        return;
    }

    if (alis.platform.version == 10)
    {
        ymsfx(eChannelTypeDingZap, 0xf, 0);
        return;
    }

    u8 pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    u8 priorson = alis.varD7;
    readexec_opername();
    s16 volson = alis.varD7 << 8;
    readexec_opername();
    u16 freqson = alis.varD7 >> 1;
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
static void cnoise(void)
{
    if (psg_ops_dos_v6(4))
    {
        return;
    }

    if (alis.platform.version == 10)
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        return;
    }

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
static void cinitab(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        s16 off = (s16)script_read16();
        script_read16();
        xwrite8(alis.script->vram_org + off, script_read8());
        return;
    }

    if (alis.platform.version >= 60)
    {
        s16 off = (s16)script_read16();
        s16 count = (s16)script_read16();
        for (s32 i = 0; i < count - 1; i++)
            xwrite8(alis.script->vram_org + off + i, script_read8());
        
        return;
    }

    if (alis.platform.version == 10)
    {
        // copy inline bytes into the script variables (DOS copies one byte less)
        s16 offset = script_read16();
        u16 length = script_read16();
        if (alis.platform.kind == EPlatformPC && length)
            length--;
        
        for (u16 i = 0; i < length; i++)
            xwrite8(alis.script->vram_org + offset + i, script_read8());
        
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 113 opcode 0x70 cfopen
static void cfopen(void)
{
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    
    u16 mode;

    if(*(alis.mem + alis.script->pc) == 0xff)
    {
        alis.script->pc += 1;
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
    if(alis.fp == NULL)
    {
        alis_error(ALIS_ERR_FOPEN, path);
    }
}

// Codopname no. 114 opcode 0x71 cfclose
static void cfclose(void)
{
    if(sys_fclose(alis.fp) < 0)
    {
        alis_error(ALIS_ERR_FCLOSE);
    }
    
    alis.fp = NULL;
}

// Codopname no. 115 opcode 0x72 cfcreat
static void cfcreat(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        char name[kPathMaxLen] = {0}, path[kPathMaxLen] = {0};
        script_read_until_zero(name);
        script_read16();
        snprintf(path, sizeof(path), "%s%s", alis.platform.path, name);
        FILE *f = fopen(path, "ab");
        if (f)
            fclose(f);
        
        return;
    }

    if (alis.platform.version >= 60)
    {
        char path[kPathMaxLen] = {0};
        alis.script->pc++;
        readexec_opername_swap();
        readexec_opername();
        snprintf(path, sizeof(path), "%s%s", alis.platform.path, alis.sd7);
        FILE *f = fopen(path, "wb");
        if (f)
            fclose(f);
        
        return;
    }

    if (alis.platform.version == 10)
    {
        char name[kPathMaxLen] = {0};
        script_read_until_zero(name);
        script_read16();
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 116 opcode 0x73 cfdel
static void cfdel(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        char name[kPathMaxLen] = {0}, path[kPathMaxLen] = {0};
        if (*(alis.mem + alis.script->pc) == 0xff)
        {
            alis.script->pc++;
            readexec_opername_swap();
            strncpy(name, alis.sd7, sizeof(name) - 1);
        }
        else
        {
            script_read_until_zero(name);
        }

        snprintf(path, sizeof(path), "%s%s", alis.platform.path, name);
        remove(path);
        return;
    }

    if (alis.platform.version >= 60)
    {
        char path[kPathMaxLen] = {0};
        alis.script->pc++;
        readexec_opername_swap();
        snprintf(path, sizeof(path), "%s%s", alis.platform.path, alis.sd7);
        remove(path);
        return;
    }

    if (alis.platform.version == 10)
    {
        char name[kPathMaxLen] = {0};
        script_read_until_zero(name);
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 117 opcode 0x74 cfreadv
static void cfreadv(void)
{
    fread(alis.buffer, 2, 1, alis.fp);
    alis.varD7 = xswap16be(*(s16 *)alis.buffer);
    ALIS_DEBUG(EDebugInfo, " 0x%04x", alis.varD7);
    cstore_continue();
}

// v6 dispatch target for cfreadv
static void cfreadv_v6(void)
{
    u8 b[4] = { 0, 0, 0, 0 };
    fread(b, 1, 4, alis.fp);
    alis.varD7 = (s32)(((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8)  | (u32)b[3]);
    ALIS_DEBUG(EDebugInfo, " 0x%08x", alis.varD7);
    cstore_continue();
}

// Codopname no. 118 opcode 0x75 cfwritev
static void cfwritev(void)
{
    readexec_opername();
    if (alis.platform.version >= 60)
    {
        u32 v = (u32)alis.varD7;
        u8 b[4] = { (u8)(v >> 24), (u8)(v >> 16), (u8)(v >> 8), (u8)v };
        ALIS_DEBUG(EDebugInfo, " 0x%08x", alis.varD7);
        fwrite(b, 1, 4, alis.fp);
        return;
    }

    s16 val = xswap16be(alis.varD7);
    ALIS_DEBUG(EDebugInfo, " 0x%04x", alis.varD7);
    fwrite(&val, 2, 1, alis.fp);
}

// Codopname no. 119 opcode 0x76 cfwritei
static void cfwritei(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        char name[kPathMaxLen] = {0};
        static int warned;
        script_read_until_zero(name);
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "cfwritei 3DO: '%s' ignored (00034b28 not ported)\n", name);

        return;
    }

    if (alis.platform.version >= 60)
    {
        u8 *str = alis.mem + alis.script->pc;
        u32 len = (u32)strlen((char *)str) + 1;
        alis.script->pc += len;
        if (alis.fp)
            fwrite(str, len, 1, alis.fp);

        return;
    }

    if (alis.platform.version == 10)
    {
        char name[kPathMaxLen] = {0};
        script_read_until_zero(name);
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void unmixword(s32 tgt_addr, u32 tgt_len)
{
    u8 tgt_byte;
    u8 tmp_byte;
    u8 tmp_xchg;
    s32 tmp_offset_lsr4 = 0;
    s32 tmp_offset_tune;
    s32 tmp_offset;
    u8 bitcheck;
    
    u32 bufpackend = (tgt_len >> 5);
    
    u16 len = (tgt_len >> 5) + 4;
    u8 *tmp_buffer = malloc(len);
    memset(tmp_buffer, 0, len);
    
    s32 half_longpack = tgt_len >> 1;
    s32 tmp_offset2 = 1;

    tmp_buffer[0] = 1;

    do
    {
        tmp_byte = xread8(tgt_addr + tmp_offset2);
        tmp_offset = tmp_offset2;
        
        while (true)
        {
            tmp_xchg = tmp_byte;
            if (half_longpack <= tmp_offset)
            {
                do
                {
                    tmp_offset = (tmp_offset - half_longpack) * 2 + 1;
                    tmp_xchg = xread8(tgt_addr + tmp_offset);
                    xwrite8(tgt_addr + tmp_offset, tmp_byte);
                    tmp_byte = tmp_xchg;
                }
                while (half_longpack <= tmp_offset);
                
                tmp_offset_lsr4 = tmp_offset >> 4;
                tmp_buffer[tmp_offset_lsr4] |= 1 << ((u16)(tmp_offset >> 1) & 7);
                if (tmp_offset == tmp_offset2)
                {
                    break;
                }
            }
            
            tmp_offset *= 2;
            tmp_byte = xread8(tgt_addr + tmp_offset);
            xwrite8(tgt_addr + tmp_offset, tmp_xchg);
        }
        
        while (tmp_buffer[tmp_offset_lsr4] == 0xff)
        {
            tmp_offset_lsr4++;
            if (bufpackend <= tmp_offset_lsr4)
            {
                free(tmp_buffer);
                return;
            }
        }
        
        tgt_byte = tmp_buffer[tmp_offset_lsr4];
        tmp_offset_tune = -1;
        
        do
        {
            tmp_offset_tune++;
            bitcheck = tgt_byte & 1;
            tgt_byte >>= 1;
        }
        while (bitcheck != 0);
        
        tmp_offset2 = (tmp_offset_tune + tmp_offset_lsr4 * 8) * 2 + 1;
    }
    while (tmp_offset2 < half_longpack);
    
    free(tmp_buffer);
}

// s16 offset from vram; 0 means "next s16 is from basemain" (DOS 000068b2, 3DO 0001e4d8)
u32 read_vaddr(void)
{
    u32 base = alis.script->vram_org;
    s16 offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        base = alis.basemain;
    }

    return base + offset;
}

// .fic data is big-endian: on PC swap u16 cells (flag 2), and on v6 also u32 (flag 4)
static u8 fic_cellsize(u32 cell)
{
    if (alis.platform.kind != EPlatformPC)
        return 0;
    u8 flag = xread8(cell - 2);
    if (alis.platform.version >= 60)
        return flag & 0xf;
    return flag == 2 ? 2 : 0;
}

static void fic_swap(u8 *p, u32 length, u8 size)
{
    if (size != 2 && size != 4)
        return;
    for (u32 i = 0; i + size <= length; i += size)
        for (u8 l = 0, r = size - 1; l < r; l++, r--)
        {
            u8 t = p[i + l];
            p[i + l] = p[i + r];
            p[i + r] = t;
        }
}

static s32 v6_tabdata(u32 cell)
{
    if ((s8)xread8(cell - 2) >= 0)
        return cell;
    
    s32 blk = xread32(cell);
    return blk > 0 ? blk + xread32(blk) : 0;
}

// Codopname no. 120 opcode 0x77 cfreadb
static void cfreadb(void)
{
    u32 cell = read_vaddr();
    s32 addr = cell;
    u32 length = 0;

    if (alis.platform.version >= 30)
    {
        if (alis.platform.version >= 60)
        {
            if ((addr = v6_tabdata(cell)) == 0)
                return;
        }
        else if ((s8)xread8(addr - 1) < 0)
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
            u8 *unpacked_buffer = alis.mem + addr;
            unpack_script_fp(alis.fp, unpacked_buffer, length);

            if (alis.wordpack != 0)
            {
                unmixword(addr, length);
            }
        }
    }
    else
    {
        length = script_read16();
        
        fread(alis.mem + addr, length, 1, alis.fp);
    }
    
    fic_swap(alis.mem + addr, length, fic_cellsize(cell));
}

void writeswap(u32 addr, u32 length, u32 cell)
{
    u8 size = fic_cellsize(cell);
    if (size == 2 || size == 4)
    {
        u8 *data = malloc(length);
        memcpy(data, alis.mem + addr, length);
        fic_swap(data, length, size);
        fwrite(data, length, 1, alis.fp);
        free(data);
        return;
    }
    
    fwrite(alis.mem + addr, length, 1, alis.fp);
}

// Codopname no. 121 opcode 0x78 cfwriteb
static void cfwriteb(void)
{
    u32 cell = read_vaddr();

    if (alis.platform.version >= 60)
    {
        s32 data = v6_tabdata(cell);
        if (data == 0)
            return;

        u32 length = script_read32();
        writeswap(data, length, cell);
    }
    else if (alis.platform.version >= 30)
    {
        u32 length = script_read32();

        if (-1 < (s8)xread8(cell - 1))
        {
            writeswap(cell, length, cell);
        }
        else
        {
            u32 addr1 = xread32(cell);
            u32 addr2 = xread32(addr1);
            if (0 < addr2)
            {
                writeswap(addr2, length, cell);
            }
        }
    }
    else
    {
        u16 length = script_read16();
        writeswap(cell, length, cell);
    }
}

s16 adptsin[2];
void SYS_PutPixel(void)
{
    adptsin[0] = alis.poldx;
    adptsin[1] = alis.poldy;
    
    draw_pixel(alis.poldx, alis.poldy);
}

static int pen_ops_nodraw(void)
{
    return alis.platform.version >= 60 || (alis.platform.kind == EPlatformPC && alis.platform.version >= 30);
}

// Codopname no. 122 opcode 0x79 cplot
static void cplot(void)
{
    if (pen_ops_nodraw())
    {
        readexec_opername();
        readexec_opername();
        return;
    }

    readexec_opername();
    readexec_opername_saveD6();
    
    alis.poldx = alis.varD7;
    alis.poldy = alis.varD6;
    
    SYS_PutPixel();
}

// Codopname no. 123 opcode 0x7a cdraw
static void cdraw(void)
{
    readexec_opername();
    readexec_opername_saveD6();
    if (pen_ops_nodraw())
        return;
    
    s16 oldx = alis.poldx;
    s16 oldy = alis.poldy;
    
    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;
    
    draw_line(oldx, oldy, alis.poldx, alis.poldy);
}

// Codopname no. 124 opcode 0x7b cbox
static void cbox(void)
{
    readexec_opername();
    readexec_opername_saveD6();
    if (pen_ops_nodraw())
        return;

    s16 oldx = alis.poldx;
    s16 oldy = alis.poldy;

    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;

    draw_box(oldx, oldy, alis.poldx, alis.poldy);
}

// Codopname no. 125 opcode 0x7c cboxf
static void cboxf(void)
{
    readexec_opername();
    readexec_opername_saveD6();
    if (pen_ops_nodraw())
        return;

    s16 oldx = alis.poldx;
    s16 oldy = alis.poldy;

    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;

    draw_boxf(oldx, oldy, alis.poldx, alis.poldy);
}

// Codopname no. 126 opcode 0x7d cink
static void cink(void)
{
    readexec_opername();
    image.inkcolor = alis.varD7;
}

// Codopname no. 127 opcode 0x7e cpset
static void cpset(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername();
        readexec_opername();
        return;
    }

    // pre-v6 pen position
    readexec_opername();
    alis.poldx = alis.varD7;
    readexec_opername();
    alis.poldy = alis.varD7;
}

// Codopname no. 128 opcode 0x7f cpmove
static void cpmove(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername();
        readexec_opername();
        return;
    }

    // pre-v6 pen position
    readexec_opername();
    alis.poldx += alis.varD7;
    readexec_opername();
    alis.poldy += alis.varD7;
}

// Codopname no. 129 opcode 0x80 cpmode
static void cpmode(void)
{
    readexec_opername();
    image.line_a_mode = alis.varD7;
}

// Codopname no. 130 opcode 0x81 cpicture
static void cpicture(void)
{
    if (alis.platform.version == 10)
    {
        alis.flagmain = 0;
        readexec_opername();
        image.depx = 0;
        image.depy = 0;
        image.depz = 0;
        image.invert_x = 0;
        picture_v10(alis.varD7);
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// v6 dispatch target for cpicture
static void cpicture_v6(void)
{
    u16 world = (u16)script_read16();
    if (alis.platform.version >= 60)
    {
        s16 scr = (s16)xread16(alis.script->vram_org - 0x64);
        if (scr != 0)
        {
            u32 s = alis.basemain + (u16)scr;
            xwrite32(s + 0x5e, alis.basemain + world);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }
    }
}

// Codopname no. 131 opcode 0x82 cxyscroll
static void cxyscroll(void)
{
    readexec_opername();
    readexec_opername();
}

// Codopname no. 132 opcode 0x83 clinking
static void clinking(void)
{
    readexec_opername();
    if (alis.platform.version >= 60)
    {
        xwrite16(V6_CLINKING(alis.script->vram_org), alis.varD7);
    }
    else
    {
        set_0x2a_clinking(alis.script->vram_org, alis.varD7);
    }
}

// Codopname no. 133 opcode 0x84 cmouson
static void cmouson(void)
{
    sys_enable_mouse(1);
}

// Codopname no. 134 opcode 0x85 cmousoff
static void cmousoff(void)
{
    sys_enable_mouse(0);
}

// Codopname no. 135 opcode 0x86 cmouse
// 0x86 - 14d62
static void cmouse(void)
{
    // Consume (not just read) so the sticky lb_clicked/rb_clicked latches are cleared.
    mouse_t mouse = sys_consume_mouse();

    if (alis.platform.kind == EPlatformMac)
    {
        mouse.x /= 1.5;
        mouse.y /= 1.5;
    }

    alis.varD7 = mouse.x;
    cstore_continue();

    alis.varD7 = mouse.y;
    cstore_continue();

    // Quick clicks (down+up landed between two script polls) show up
    // here through the *_clicked latches consumed above.
    u8 lb_seen = mouse.lb || mouse.lb_clicked;
    u8 rb_seen = mouse.rb || mouse.rb_clicked;
    alis.varD7 = lb_seen ? 1 : (rb_seen ? 2 : 0);
    cstore_continue();
}

// Codopname no. 136 opcode 0x87 cdefmouse
static void cdefmouse(void)
{
    alis.flagmain = 0;
    readexec_opername();

    if (alis.platform.version >= 60)
    {
        u32 addr = adresdes(alis.varD7);
        if (addr != 0xf)
        {
            u32 desc = addr + xread32(addr);
            alis.desmouse = alis.mem + desc;
            v6x_defmouse(desc);
        }

        return;
    }

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
static void csetmouse(void)
{
    readexec_opername();
    u16 x = alis.varD7;
    if (alis.platform.version == 10 && alis.platform.kind == EPlatformPC)
        return;
    
    readexec_opername();
    u16 y = alis.varD6;
    sys_set_mouse(x, y);
}

// Codopname no. 138 opcode 0x89 cdefvect
static void cdefvect(void)
{

    s16 offset = script_read8();
    offset += offset * 2;
    offset += 3;

    alis.script->pc += offset;
}

// v6 dispatch target for cdefvect
static void cdefvect_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    if (p.aux != -1)
    {
        s32 slot = p.elem + p.aux;
        readexec_opername_saveD7();
        xwrite16(slot + 0x18, alis.varD7 << 8);
        readexec_opername_saveD7();
        xwrite16(slot + 0x1a, alis.varD7 << 8);
        readexec_opername_saveD7();
        xwrite16(slot + 0x1c, alis.varD7 << 8);
        return;
    }

    readexec_opername_saveD7();
    s16 ax = alis.varD7 << 8;
    readexec_opername_saveD7();
    s16 bx = alis.varD7 << 8;
    readexec_opername_saveD7();
    s16 dx = alis.varD7 << 8;
    s32 slot = p.elem;
    u8 count = xread8(slot + 0x33);
    for (u8 i = 0; i < count; i++)
    {
        if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
        {
            xwrite16(slot + 0x18, ax);
            xwrite16(slot + 0x1a, bx);
            xwrite16(slot + 0x1c, dx);
            xwrite8(slot + 0x23, xread8(slot + 0x23) | 1);
        }

        slot += V6_SLOT_STRIDE;
    }
}

// Codopname no. 139 opcode 0x8a csetvect
static void csetvect(void)
{

    s16 value = script_read16();
    set_0x20_set_vect(alis.script->vram_org, value);
}

// v6 dispatch target for csetvect
static void csetvect_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    if (p.aux != -1)
    {
        s32 slot = p.elem + p.aux;
        readexec_opername_saveD7();
        xadd16(slot + 0x18, alis.varD7 << 8);
        readexec_opername_saveD7();
        xadd16(slot + 0x1a, alis.varD7 << 8);
        readexec_opername_saveD7();
        xadd16(slot + 0x1c, alis.varD7 << 8);
        return;
    }

    readexec_opername_saveD7();
    s16 ax = alis.varD7 << 8;
    readexec_opername_saveD7();
    s16 bx = alis.varD7 << 8;
    readexec_opername_saveD7();
    s16 dx = alis.varD7 << 8;
    s32 slot = p.elem;
    u8 count = xread8(slot + 0x33);
    for (u8 i = 0; i < count; i++)
    {
        if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
        {
            xadd16(slot + 0x18, ax);
            xadd16(slot + 0x1a, bx);
            xadd16(slot + 0x1c, dx);
            xwrite8(slot + 0x23, xread8(slot + 0x23) | 1);
        }

        slot += V6_SLOT_STRIDE;
    }
}

u8 *deb_approach(s32 offset, s16 *wcx, s16 *wcy, s16 *wcz)
{
    offset = xread32(alis.atent + offset);
    *wcx = xread16(offset + ALIS_SCR_WCX) - xread16(alis.script->vram_org + ALIS_SCR_WCX);
    *wcy = xread16(offset + ALIS_SCR_WCY) - xread16(alis.script->vram_org + ALIS_SCR_WCY);
    *wcz = xread16(offset + ALIS_SCR_WCZ) - xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    
    u8 *address = get_0x20_set_vect(alis.script->vram_org) == 0 ? alis.mem + alis.vstandard : alis.mem + get_0x20_set_vect(alis.script->vram_org) + get_0x14_script_org_offset(alis.script->vram_org);

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
static void capproach(void)
{
    if (alis.platform.version >= 60)
        return;
    
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
static void cescape(void)
{
    if (alis.platform.version >= 60)
        return;
    
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
static void cvtstmov(void)
{
    alis.wcx = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2) + xread16(alis.script->vram_org + ALIS_SCR_WCX);
    alis.wcy = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCY2) + xread16(alis.script->vram_org + ALIS_SCR_WCY);
    alis.wcz = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCZ2) + xread16(alis.script->vram_org + ALIS_SCR_WCZ);
    readexec_opername();
    alis.matmask = alis.varD7;
    alis.wforme = get_0x1a_cforme(alis.script->vram_org);
    clipform();
    crstent();
}

// v6 dispatch target for cvtstmov
static void cvtstmov_v6(void)
{
    v6_tst_from_movevec();
    readexec_opername_saveD7(); alis.v6tstflag = (u8)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstarg = alis.varD7;
    v6_tstscan();
    crstent();
}

// Codopname no. 144 opcode 0x8f cvftstmov
static void cvftstmov(void)
{
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

// v6 dispatch target for cvftstmov
static void cvftstmov_v6(void)
{
    v6_tst_from_movevec();
    readexec_opername_saveD7(); alis.v6tstflag = (u8)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    v6_tst_addr_operand();
    v6_tstscan_elem();
    crstent();
}

// Codopname no. 145 opcode 0x90 cvmov
static void cvmov(void)
{
    if (alis.platform.version >= 60)
    {
        u32 v = alis.script->vram_org;
        xwrite32(v + 0x0c, xread32(v + 0x0c) + (s16)xread16(v + 0x26));
        xwrite32(v + 0x10, xread32(v + 0x10) + (s16)xread16(v + 0x28));
        xwrite32(v + 0x14, xread32(v + 0x14) + (s16)xread16(v + 0x2a));
        xwrite8(v + 0x45, 1);
        return;
    }

    xadd16(alis.script->vram_org + ALIS_SCR_WCX, (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2));
    xadd16(alis.script->vram_org + ALIS_SCR_WCY, (s8)xread8(alis.script->vram_org + ALIS_SCR_WCY2));
    xadd16(alis.script->vram_org + ALIS_SCR_WCZ, (s8)xread8(alis.script->vram_org + ALIS_SCR_WCZ2));
}

static u32 defworld_list_head = 0;
u32 v6_worldhead(void) { return defworld_list_head; }

// Codopname no. 146 opcode 0x91 cdefworld
static void cdefworld(void)
{
    s16 offset = script_read16();

    if (alis.platform.version >= 60)
    {
        u32 base = alis.script->vram_org + (u16)offset;
        if (alis.platform.version >= 60)
        {
            set_0x16_screen_id(alis.script->vram_org, (u16)offset);
        }
        else
        {
            set_0x22_cworld(alis.script->vram_org, offset);
        }

        // insert into the global world list + initialise the struct fields
        xwrite32(base + 0xb4, defworld_list_head);
        defworld_list_head = base;
        xwrite32(base + 0xb0, 0);
        xwrite32(base + 0x38, base + 0x58);
        xwrite32(base + 0x34, 0);
        xwrite8 (base + 0x22, 0);
        xwrite32(base + 0x8c, base);
        xwrite32(base + 0x90, base + 0x58);
        xwrite8 (base + 0x7a, 0);

        // TODO: verify destination base + 0x58 + offset against a non-empty world
        u16 count = script_read16();
        u32 dst = base + 0x58 + (u16)offset;
        while (count--)
        {
            xwrite16(dst, script_read16());
            dst += 2;
        }
    }
    else
    {
        u8 counter = alis.platform.version == 10 && alis.platform.kind == EPlatformPC ? 5 : 6;
        while(counter--)
        {
            xwrite8(alis.script->vram_org + offset, script_read8());
        }
    }
}

// Codopname no. 147 opcode 0x92 cworld
static void cworld(void)
{
    if (alis.platform.version >= 60)
    {
        u16 world = (u16)script_read16();
        set_0x16_screen_id(alis.script->vram_org, world);
        return;
    }

    xwrite8(alis.script->vram_org + 0xffde, script_read8());
    xwrite8(alis.script->vram_org + 0xffdf, script_read8());
}

// Codopname no. 148 opcode 0x93 cfindmat
static void cfindmat(void)
{
    if (alis.platform.version == 10)
    {
        readexec_opername();
        alis.matmask = alis.varD7;
        findent(0, 0);
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// v6 dispatch target for cfindmat
static void cfindmat_v6(void)
{
    u16 world = (u16)script_read16();
    s32 tabidx = 0;
    s16 entidx = (s16)alis_first_ent();
    do
    {
        u32 tgt_vram = xread32(alis.atent + entidx);
        u16 w = alis.platform.version >= 60 ? get_0x16_screen_id(tgt_vram) : get_0x22_cworld(tgt_vram);
        if (tgt_vram != 0 && w == world)
        {
            alis.matent[tabidx] = 0;
            alis.tablent[tabidx] = entidx;
            tabidx++;
            if (alis.fallent == 0)
                break;
        }

        entidx = xread16(alis.atent + entidx + 4);
    }
    while (entidx != 0);

    alis.tablent[tabidx] = ENTLIST_END;
    alis.fallent = 0;
    crstent();
}

// Codopname no. 149 opcode 0x94 cfindtyp
static void cfindtyp(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername_swap();
        char name[8];
        memset(name, 0, sizeof(name));
        strncpy(name, alis.sd7, sizeof(name));

        s32 tabidx = 0;
        s16 entidx = (s16)alis_first_ent();
        do
        {
            u32 tgt_vram = xread32(alis.atent + entidx);
            if (tgt_vram != 0 && tgt_vram != alis.script->vram_org && memcmp(alis.mem + tgt_vram + 0x3c, name, 8) == 0)
            {
                alis.matent[tabidx] = 0;
                alis.tablent[tabidx] = entidx;
                tabidx++;
                if (alis.fallent == 0)
                    break;
            }

            entidx = xread16(alis.atent + 4 + entidx);
        }
        while (entidx != 0);

        alis.tablent[tabidx] = ENTLIST_END;
        alis.fallent = 0;
        crstent();
        return;
    }

    if (alis.platform.kind == EPlatformPC)
    {
        alis.varD7 = script_read16();
    }
    else
    {
        readexec_opername();
    }
    
    if (alis.platform.version == 10 && alis.platform.kind != EPlatformPC)
    {
        findent(1, 0);
        return;
    }
    
    if (alis.varD7 < 0)
    {
        alis.varD7 &= 0xff;
    }

    s16 id = alis.varD7;
    s16 entidx = (s16)alis_first_ent();     // v6: skip the null sentinel (task #10)
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
    
    alis.tablent[tabidx] = ENTLIST_END;
    alis.fallent = 0;
    crstent();
}

void music(void)
{
    if (alis.platform.version <= 10)
    {
        readexec_opername();
        s16 idx = alis.varD7;
        s32 addr = (idx < 0 || alis.platform.kind == EPlatformPC) ? 0 : adresmus(idx);
        if (idx < 0)
            mv0_offmusic();

        s16 op[4];
        for (int i = 0; i < 4; i++)
        {
            readexec_opername();
            op[i] = alis.varD7;
        }

        if (addr && xread8(addr) == 0)
            mv0_gomusic(addr, op[0], op[1], op[2], op[3]);
        return;
    }
    
    u16 scriptId = alis.flagmain ? alis.main->data->header.id : alis.script->data->header.id;
    audio.musicId = scriptId;
    
    readexec_opername();
    s16 idx = alis.varD7;
    s32 addr = adresmus(idx);

    u8 type = xread8(addr);
    if (type == 0 || type == 3)
    {
        audio.muvolume = 0;

        audio.mupnote = addr + 6;
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
            ALIS_DEBUG(EDebugWarning, "STUBBED: %s", __FUNCTION__);
        }

        audio.mustate = 1;
    }
    else if (type == 4)
    {
        audio.muvolume = 0;

        audio.mupnote = addr + 6;
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
        
        ALIS_DEBUG(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    }
}

// Codopname no. 150 opcode 0x95 cmusic
static void cmusic(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        for (int i = 0; i < 6; i++)
            readexec_opername();

        return;
    }

    alis.flagmain = 0;
    music();
}

// Codopname no. 151 opcode 0x96 cdelmusic
static void cdelmusic(void)
{
    if (alis.platform.version <= 10)
    {
        mv0_offmusic();
        return;
    }
    
    readexec_opername();
    
    // Stop the engine that plays: version-21 games (Boston Bomb Club, Bunny Bricks) still use MV1.
    if (audio.soundrout == mv1_soundrout)
    {
        mv1_offmusic(alis.varD7);
    }
    else
    {
        mv2_offmusic(alis.varD7);
    }
}

// Codopname no. 152 opcode 0x97 ccadence
static void ccadence(void)
{
    readexec_opername();
    if (alis.platform.version == 10)
    {
        mv0_cadence(alis.varD7);
        return;
    }

    if (alis.varD7 == 0)
        alis.varD7 = 1;

    audio.mutempo = (u8)alis.varD7;
    audio.mutemp = (u8)((audio.mutempo * 6) / 0x20);
}

// Codopname no. 153 opcode 0x98 csetvolum
static void csetvolum(void)
{
    readexec_opername();
    if (alis.platform.version == 10)
    {
        mv0_volume(alis.varD7);
        return;
    }

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

// v6 dispatch target for csetvolum
static void csetvolum_v6(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        readexec_opername();
        audio.muvolume = (s16)alis.varD7;
        *(u8 *)&audio.maxvolume = (u8)alis.varD7;
        audio.muvol = (audio.muvolume >> 1) + 1;
        return;
    }

    // TODO: port the full v6 music-volume routine
    readexec_opername();
    s16 vol = alis.varD7;
    readexec_opername();
    readexec_opername();
    readexec_opername();

    audio.muvolume = vol;
    *(u8 *)&audio.maxvolume = vol;
    audio.muvol = ((audio.muvolume) >> 1) + 1;
    if (audio.muattac != 0)
        audio.dattac = audio.maxvolume / audio.muattac;
    
    if (audio.muchute != 0)
        audio.dchute = audio.maxvolume / audio.muchute;
}

// Codopname no. 154 opcode 0x99 cxinv
static void cxinv(void)
{
    u8 xinv = get_0x03_xinv(alis.script->vram_org);
    BIT_CHG(xinv, 0);
    set_0x03_xinv(alis.script->vram_org, xinv);
}

// Codopname no. 155 opcode 0x9a cxinvon
static void cxinvon(void)
{
    set_0x03_xinv(alis.script->vram_org, 1);
}

// Codopname no. 156 opcode 0x9b cxinvoff
static void cxinvoff(void)
{
    set_0x03_xinv(alis.script->vram_org, 0);
}

// Codopname no. 157 opcode 0x9c clistent
static void clistent(void)
{
    s16 entidx = alis_first_ent();
    s16 tabidx = 0;
    if (alis.platform.version == 10)
    {
        // v1.0 includes entity 0 and keeps fallent
        do
        {
            alis.tablent[tabidx++] = entidx;
        }
        while ((entidx = xread16(alis.atent + 4 + entidx)) != 0 && tabidx < 127);
        
        alis.tablent[tabidx] = -1;
        crstent();
        return;
    }
    
    while ((entidx = xread16(alis.atent + 4 + entidx)) != 0)
    {
        alis.matent[tabidx] = 0;
        alis.tablent[tabidx] = entidx;
        tabidx ++;
    }
    
    alis.tablent[tabidx] = ENTLIST_END;
    alis.fallent = 0;
    crstent();
}

static void sound(void)
{
    // v1.0: priority (0 = 4, odd = interruptible), index; one digi channel at ~8 kHz
    if (alis.platform.version == 10)
    {
        readexec_opername();
        u8 prio = alis.varD7;
        readexec_opername();
        if (alis.platform.kind == EPlatformPC)
            return;

        s32 addr = adresmus(alis.varD7);
        sChannel *ch = &audio.channels[3];
        if (xread8(addr) != 1 || (ch->type == eChannelTypeSample && !(ch->curson & 1)))
            return;

        u16 len = (xread8(addr + 1) << 8) | xread8(addr + 2);
        if (len < 2)
            return;

        ch->type = eChannelTypeNone;
        ch->address = (s8 *)(alis.mem + addr + 3);
        ch->length = len - 1;
        ch->freq = 7979;
        ch->volume = 0xff;
        ch->loop = 0;
        ch->played = 0;
        ch->curson = prio ? prio : 4;
        ch->type = eChannelTypeSample;
        return;
    }

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
    s8 type = xread8(addr);
    if (type == 1 || type == 2)
    {
        if (speedsam == 0)
            speedsam = xread8(addr + 1);

        u32 longsam = xread32(addr + 2) - 0x10;
        if (alis.platform.kind == EPlatformPC && (alis.platform.uid == EGameColorado || alis.platform.uid == EGameWindsurfWilly || alis.platform.uid == EGameMadShow || alis.platform.uid == EGameLeFeticheMaya))
            longsam = xread32be(addr + 2) - 0x10;

        u32 startsam = addr + 0x10;
        playsample(eChannelTypeSample, alis.mem + startsam, speedsam, volson, longsam, loopsam, priorson);
    }
}

static void v6_sound_tail(s32 slot)
{
    int w = (slot > 0);
    if (!w)
    {
        for (int i = 0; i < 7; i++)
            readexec_opername_saveD7();
        
        ALIS_DEBUG(EDebugWarning, "v6 sound: elem %d is a sentinel/null - operands consumed, writes skipped\n", slot);
        return;
    }

    readexec_opername_saveD7();
    xwrite16(slot + 0x40, alis.varD7);
    readexec_opername_saveD7();
    xwrite8(slot + 0x44, (u8)alis.varD7);
    xwrite8(slot + 0x45, (u8)alis.varD7);
    readexec_opername_saveD7();
    xwrite8(slot + 0x46, (u8)alis.varD7);
    xwrite8(slot + 0x47, (u8)alis.varD7);
    readexec_opername_saveD7();
    xwrite16(slot + 0x4c, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(slot + 0x48, alis.varD7);
    xwrite16(slot + 0x4a, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(slot + 0x42, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(slot + 0x52, alis.varD7);

    s32 pan = xread32(slot + 0x0c) >> 2;
    if (pan > 0x7f)
        pan = 0x7f;

    if (pan < -0x7f)
        pan = -0x7f;

    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        pan = 0;

    xwrite8(slot + 0x51, (u8)pan);

    if (xread8(slot + 0x22) != 0x20)
    {
        xwrite8(slot + 0x50, 0xff);
        alis.v6soundseq++;
        xwrite32(slot + 0x54, alis.v6soundseq);
    }

    if (v6put.desc != 0)
    {
        v6put.defstate = 0x20;
        v6put.invert = xread8(alis.script->vram_org + 0x25);
        v6put.depx = v6put.depy = v6put.depz = 0;
        v6put.angx = v6put.angy = v6put.angz = 0;
        v6put.fadddes = 0;
        put_v6((u32)slot);
        xwrite8(slot + 0x23, xread8(slot + 0x23) | 0x80);
    }

    if ((xread16(slot + 0x40) & 3) == 0)
        v6snd_step((u32)slot);
}

static void v6_sound_form(void)
{
    readexec_opername_saveD7();
    s16 form = alis.varD7;
    v6put.desc = (form == -1) ? 0 : (u32)adresdes(form);
}

static void csound_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    v6put.basesprite = (u32)p.elem;
    v6put.limit = p.limit;
    v6_sound_form();
    v6_sound_tail(p.elem);
}

static void cmsound_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    v6put.basesprite = (u32)p.elem;
    v6put.limit = p.limit;
    readexec_opername_swap();

    sAlisScriptData *found = NULL;
    for (int i = 0; i < alis.nbprog && !found; i++)
    {
        sAlisScriptData *d = alis.loaded_scripts[i];
        if (d && strncmp(alis.sd7, (char *)(alis.mem + d->data_org + 8), 8) == 0)
            found = d;
    }

    if (!found)
    {
        for (int i = 0; i < 8; i++)
            readexec_opername_saveD7();
        
        return;
    }

    readexec_opername_saveD7();
    s16 form = alis.varD7;
    u32 base = found->data_org;
    if (form == -1)
    {
        v6put.desc = 0;
    }
    else if (alis.platform.uid == EGameRobinsonsRequiem3DO || (u16)form < (u16)xread16(base + 0x3e))
    {
        v6put.desc = base + 0x40 + (u16)form * 4;
    }
    else
    {
        ALIS_DEBUG(EDebugError, "cmsound: form %d out of range in %s\n", form, found->name);
        v6put.desc = 0xf;
    }

    v6_sound_tail(p.elem);
}

// Codopname no. 158 opcode 0x9d csound
static void csound(void)
{
    if (alis.platform.version >= 60)
    {
        csound_v6();
        return;
    }

    alis.flagmain = 0;
    sound();
}

// Codopname no. 159 opcode 0x9e cmsound
static void cmsound(void)
{
    if (alis.platform.version >= 60)
    {
        cmsound_v6();
        return;
    }

    alis.flagmain = 1;
    sound();
}

// Codopname no. 160 opcode 0x9f credon
static void credon(void)
{
    // v1.0: cding with volume
    if (alis.platform.version == 10)
    {
        readexec_opername();
        ymsfx(eChannelTypeDingZap, alis.varD7, 0);
        return;
    }

    set_0x25_credon_credoff(alis.script->vram_org, 0x0);
}

// Codopname no. 161 opcode 0xa0 credoff
static void credoff(void)
{
    // v1.0: cexplode with volume
    if (alis.platform.version == 10)
    {
        readexec_opername();
        ymsfx(eChannelTypeExplode, alis.varD7, 0);
        return;
    }

    set_0x25_credon_credoff(alis.script->vram_org, alis.platform.version < 30 ? 0xff : 0x80);
}

// Codopname no. 162 opcode 0xa1 cdelsound
static void cdelsound(void)
{
    offsound();
}

// v6 dispatch target for cdelsound
static void cdelsound_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    readexec_opername_saveD7();
    s16 arg = alis.varD7;
    if (arg != -1)
    {
        alis.ferase = 0;
        if (p.elem > 0)
        {
            v6snd_delsound((u32)p.elem, (u16)arg);
            eraselem_v6((u32)p.elem);
        }
        else
        {
            ALIS_DEBUG(EDebugWarning, "cdelsound: elem %d is a sentinel/null - skipped\n", p.elem);
        }
    }
    else
    {
        v6snd_stopchan(-1);
    }
}

// Codopname no. 163 opcode 0xa2 cwmov
static void cwmov(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername_saveD7();
        s32 px = (s32)alis.varD7;
        readexec_opername_saveD7();
        s32 op2 = (s32)alis.varD7;

        s16 scr = (s16)xread16(alis.script->vram_org - 0x64);
        u32 s = alis.basemain + (u16)scr;
        u32 world = xread32(s + 0x5e);

        s32 py = 0;
        s32 pz = op2;
        u8 mode = xread8(s + 0x5a);
        if (mode != 0)
        {
            pz = (s16)(-op2 + (s16)xread16(s + 0x64));
            if (mode != 1 && mode != 6)
            {
                s32 t = pz; pz = py; py = t;
            }
        }

        s32 pye = 0x7fffffff, pze = 1;
        if (mode == 2)
        {
            pye = 1;
            pze = 0x7fffffff;
        }

        s32 pxmin = px, pxmax = px + 1;
        s32 pymin = py, pymax = py + pye;
        s32 pzmin = pz, pzmax = pz + pze;

        u16 result = 0;
        v6mat_n = 0;
        if (world != 0)
        {
            u32 sp = xread32(world + 0x38);
            while (sp != 0 && sp != xread32(sp + 0x38))
            {
                u32 cand = sp;
                sp = xread32(sp + 0x38);
                if (xread16(cand + 0x28) == 0)
                    continue;
                
                if (xread8(cand + 0x24) & 0x40)
                    continue;
                
                if (xread8(cand + 0x25) & 0x4)
                    continue;
                
                if (v6sp_state(cand) != 0x35)
                    continue;
                
                if (xread16(cand + 0x2a) == 0)
                    continue;
                
                s32 cx = 0, cy = 0, cz = 0;
                for (u32 p = cand; p != 0; p = v6sp_parent(p))
                {
                    cx += v6sp_depx(p);
                    cy += v6sp_depy(p);
                    cz += v6sp_depz(p);
                }

                s32 cxmin, cxmax;
                if (xread8(cand + 0x25) & 1)
                {
                    cxmax = cx - xread32(cand + 0x40);
                    cxmin = cxmax - xread32(cand + 0x4c);
                }
                else
                {
                    cxmin = cx + xread32(cand + 0x40);
                    cxmax = cxmin + xread32(cand + 0x4c);
                }

                if (cxmin > pxmax || cxmax < pxmin)
                    continue;
                
                s32 czmin = cz + xread32(cand + 0x48);
                s32 czmax = czmin + xread32(cand + 0x54);
                if (czmin > pzmax || czmax < pzmin)
                    continue;
                
                s32 cymin = cy + xread32(cand + 0x44);
                s32 cymax = cymin + xread32(cand + 0x50);
                if (cymin > pymax || cymax < pymin)
                    continue;
                
                if (v6mat_n < V6MAT_MAX)
                {
                    v6mat_ent[v6mat_n] = xread16(cand + 0x2a);
                    v6mat_id[v6mat_n] = (u16)xread16(cand + 0x26) | ((u32)(u16)xread16(cand + 0x28) << 16);
                    v6mat_n++;
                }

                result = xread16(cand + 0x26);
                break;
            }
        }

        alis.varD7 = (s16)result;
        cstore_continue();
        return;
    }

    s16 x, y, z;
    get_vector(&x, &y, &z);
    xadd16(alis.script->vram_org + ALIS_SCR_WCX, x);
    xadd16(alis.script->vram_org + ALIS_SCR_WCY, y);
    xadd16(alis.script->vram_org + ALIS_SCR_WCZ, z);
}

// Codopname no. 164 opcode 0xa3 cwtstmov
static void cwtstmov(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// v6 dispatch target for cwtstmov
static void cwtstmov_v6(void)
{
    alis.v6tstdx = alis.v6tstdy = alis.v6tstdz = 0;
    alis.v6tstflag = 0;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstarg = alis.varD7;
    v6_tstscan();
    crstent();
}

// Codopname no. 165 opcode 0xa4 cwftstmov
static void cwftstmov(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// v6 dispatch target for cwftstmov
static void cwftstmov_v6(void)
{
    alis.v6tstdx = alis.v6tstdy = alis.v6tstdz = 0;
    alis.v6tstflag = 0;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    v6_tst_addr_operand();
    v6_tstscan_elem();
    crstent();
}

// Codopname no. 166 opcode 0xa5 ctstform
static void ctstform(void)
{
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

// v6 dispatch target for ctstform
static void ctstform_v6(void)
{
    alis.v6tstpolar = 1;
    v6_tst_from_movevec();
    readexec_opername_saveD7(); alis.v6tstflag = (u8)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    v6_tst_addr_operand();
    v6_tstscan_elem();
    alis.v6tstpolar = 0;
    crstent();
}

// Codopname no. 167 opcode 0xa6 cxput
static void cxput(void)
{
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

// v6 dispatch target for cxput
static void cxput_v6(void)
{
    v6_put_entry(1, 0, 0, 0);
}

// Codopname no. 168 opcode 0xa7 cxputat
static void cxputat(void)
{
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

// v6 dispatch target for cxputat
static void cxputat_v6(void)
{
    v6_put_entry(1, 0, 0, 1);
}

// Codopname no. 169 opcode 0xa8 cmput
static void cmput(void)
{
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

// v6 dispatch target for cmput
static void cmput_v6(void)
{
    v6_put_entry(0, 1, 0, 0);
}

// Codopname no. 170 opcode 0xa9 cmputat
static void cmputat(void)
{
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

// v6 dispatch target for cmputat
static void cmputat_v6(void)
{
    v6_put_entry(0, 1, 0, 1);
}

// Codopname no. 171 opcode 0xaa cmxput
static void cmxput(void)
{
    alis.flagmain = 1;
    image.invert_x = get_0x03_xinv(alis.script->vram_org) ^ 1;
    readexec_opername();
    image.depx = 0;
    image.depy = 0;
    image.depz = 0;
    image.numelem = 0;
    put(alis.varD7);
}

// v6 dispatch target for cmxput
static void cmxput_v6(void)
{
    v6_put_entry(0, 0, 1, 0);
}

// Codopname no. 172 opcode 0xab cmxputat
static void cmxputat(void)
{
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

// v6 dispatch target for cmxputat
static void cmxputat_v6(void)
{
    v6_put_entry(0, 0, 1, 1);
}

// Codopname no. 173 opcode 0xac cmmusic
static void cmmusic(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        for (int i = 0; i < 6; i++)
            readexec_opername();
        
        return;
    }

    alis.flagmain = 1;
    music();
}

// Codopname no. 174 opcode 0xad cmforme
static void cmforme(void)
{
    readexec_opername();
    set_0x1a_cforme(alis.script->vram_org, alis.varD7);
}

// Codopname no. 175 opcode 0xae csettime
static void csettime(void)
{
    readexec_opername();
    u16 h = alis.varD7;
    readexec_opername();
    u16 m = alis.varD7;
    readexec_opername();
    u16 s = alis.varD7;
    sys_set_time(h, m, s);
}

// Codopname no. 176 opcode 0xaf cgettime
static void cgettime(void)
{
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
    while (alis.state && (c = io_getkey()) != '\r')
    {
        if (c == '\b')
        {
            if (ptr != alis.sd7)
            {
                ptr --;
                *ptr = 0;
                put_char('\b');
                put_char(' ');
                put_char('\b');
            }
        }
        else if ((c == 0x2d) || ((-1 < (char)(c - 0x30) && (c < 0x3a))))
        {
            *ptr++ = c;
            put_char(c);
        }
    }
    
    *ptr = 0;
    
    oval();
}

// Codopname no. 177 opcode 0xb0 cvinput
static void cvinput(void)
{
    alis.charmode = 0;
    getval();
    cstore_continue();
}

void getstring(void)
{
    u8 c;
    char *ptr = alis.sd7;
    while (alis.state && (c = io_getkey()) != '\r')
    {
        if (c == '\b')
        {
            if (ptr != alis.sd7)
            {
                ptr --;
                *ptr = 0;
                put_char('\b');
                put_char(' ');
                put_char('\b');
            }
        }
        else
        {
            *ptr++ = c;
            put_char(c);
        }
    }
    
    *ptr = 0;
}

// Codopname no. 178 opcode 0xb1 csinput
static void csinput(void)
{
    alis.charmode = 0;
    getstring();
    cstore_continue();
}

void sendmess(u32 vram, u16 idx)
{
    if ((get_0x24_scan_inter(vram) & 1) == 0)
    {
        s16 val = get_0x1c_scan_clr(vram) + 2;
        if (-0x3f < val)
        {
            val -= xread16(get_0x14_script_org_offset(vram) + 0x16);
        }
        
        if (val == get_0x1e_scan_clr(vram))
        {
            set_0x1c_scan_clr(vram, val);
        }
        else
        {
            xwrite16(vram + get_0x1c_scan_clr(vram), idx);
            set_0x1c_scan_clr(vram, val);
            set_0x24_scan_inter(vram, get_0x24_scan_inter(vram) | 0x80);
        }
    }
}

// Codopname no. 179 opcode 0xb2 casleepfar
static void casleepfar_v6(void)
{
    readexec_opername();
    u32 r2 = (u32)alis.varD7 * (u32)alis.varD7;
    u32 me = alis.script->vram_org;
    s16 ent = (s16)alis_first_ent();
    do
    {
        u32 vram = xread32(alis.atent + ent);
        if (vram == 0 || (get_0x24_scan_inter(vram) & 4) != 0)
            continue;
        s64 dx = (s64)(s32)xread32(vram + 0x0c) - (s32)xread32(me + 0x0c);
        s64 dy = (s64)(s32)xread32(vram + 0x10) - (s32)xread32(me + 0x10);
        s64 dz = (s64)(s32)xread32(vram + 0x14) - (s32)xread32(me + 0x14);
        s64 sx = (s64)(s32)dx * (s32)dx, sy = (s64)(s32)dy * (s32)dy, sz = (s64)(s32)dz * (s32)dz;
        u64 d2 = (u64)sx + (u64)sy + (u64)sz;
        u8 fl = xread8(V6_FARSLEEP(vram));
        if ((u32)(d2 >> 32) == 0 && (u32)d2 < r2)
        {
            if (fl & 2)
            {
                xwrite8(V6_FARSLEEP(vram), fl & 0xfd);
                v6_enqueue((s16)xread16(vram + 0x3a), (s32)0xffff8001);
                v6_enqueue((s16)xread16(vram + 0x3a), (u16)xread16(me + 0x3a));
            }
        }
        else if (!(fl & 2))
        {
            xwrite8(V6_FARSLEEP(vram), fl | 2);
        }
    }
    while ((ent = xread16(alis.atent + 4 + ent)));
}

static void casleepfar(void)
{
    if (alis.platform.version >= 60)
    {
        casleepfar_v6();
        return;
    }

    readexec_opername();
    alis.valnorme = (alis.varD7 & 0xffff) * (alis.varD7 & 0xffff);

    s16 ent = (s16)alis_first_ent();

    do
    {
        u32 vram = xread32(alis.atent + ent);
        if (vram != 0 && (get_0x24_scan_inter(vram) & 4) == 0)
        {
            s32 xdif = xread16(vram) - xread16(alis.script->vram_org);
            s32 ydif = xread16(vram + 0x8) - xread16(alis.script->vram_org + 0x8);
            s32 zdif = xread16(vram + 0x10) - xread16(alis.script->vram_org + 0x10);
            
            if ((u32)(zdif * zdif + ydif * ydif + xdif * xdif) < alis.valnorme)
            {
                if (get_0x04_cstart_csleep(vram) == 0)
                {
                    set_0x04_cstart_csleep(vram, 1);
                    sendmess(vram, 0x8001);
                    sendmess(vram, get_0x0e_script_ent(alis.script->vram_org));
                }
            }
            else
            {
                if (get_0x04_cstart_csleep(vram) != 0)
                {
                    set_0x04_cstart_csleep(vram, 0);
                    sendmess(vram, 0x8000);
                    sendmess(vram, get_0x0e_script_ent(alis.script->vram_org));
                }
            }
        }
    }
    while ((ent = xread16(alis.atent + 4 + ent)));
}

// Codopname no. 180 opcode 0xb3 casleepon
static void casleepon(void)
{
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfb);
}

// Codopname no. 181 opcode 0xb4 casleepoff
static void casleepoff(void)
{
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 4);
}

// Codopname no. 182 opcode 0xb5 crunfilm
static void crunfilm(void)
{
    readexec_opername();
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        film3do_run(alis.varD7);
        alis.varD7 = 0;
        cstore_continue();
        alis.varD7 = 0;
        cstore_continue();
        return;
    }

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

#ifndef NDEBUG
    if (disalis)
    {
       ALIS_DEBUG(EDebugInfo, " [\"%s\"]", alis.sd7);
    }
#endif
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
static void cvpicprint(void)
{
    alis.charmode = 1;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

// Codopname no. 184 opcode 0xb7 cspicprint
static void cspicprint(void)
{
    alis.charmode = 1;
    readexec_opername_swap();
    put_string();
}

// Codopname no. 185 opcode 0xb8 cvputprint
static void cvputprint(void)
{
    if (alis.platform.version >= 60)
    {
        char num[12];
        alis.charmode = 2;
        readexec_opername();
        snprintf(num, sizeof(num), "%d", alis.varD7);
        v6x_textput((const u8 *)num);
        return;
    }

    alis.charmode = 2;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

// Codopname no. 186 opcode 0xb9 csputprint
static void csputprint(void)
{
    if (alis.platform.version >= 60)
    {
        alis.charmode = 2;
        readexec_opername_swap();
        v6x_textput((const u8 *)alis.sd7);
        return;
    }

    alis.charmode = 2;
    readexec_opername_swap();
    put_string();
}

// Codopname no. 187 opcode 0xba cfont
static void cfont(void)
{

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

// v6 dispatch target for cfont
static void cfont_v6(void)
{
    readexec_opername();
    s16 index = alis.varD7;
    if (alis.platform.version >= 60)
    {
        xwrite8(alis.script->vram_org - 0x66, (u8)index);
    }

    if (index >= 0 && index < (s16)(sizeof(alis.v6fonts) / sizeof(alis.v6fonts[0])) && alis.v6fonts[index].valid)
    {
        alis.foasc   = alis.v6fonts[index].foasc;
        alis.fonum   = alis.v6fonts[index].fonum;
        alis.fomax   = alis.v6fonts[index].fomax;
        alis.folarg  = alis.v6fonts[index].folarg;
        alis.fohaut  = alis.v6fonts[index].fohaut;
        alis.fomain  = alis.v6fonts[index].usemain;
    }
}

// Codopname no. 188 opcode 0xbb cpaper
static void cpaper(void)
{
    ALIS_DEBUG(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    readexec_opername();
    
    // TODO: ...
}

// Codopname no. 189 opcode 0xbc ctoblack
// fade-out to black
static void ctoblack(void)
{
    readexec_opername_saveD6();
    
    s16 duration = alis.varD6;
    if (dos_pal_active())
    {
        dos_ctoblack(duration);
    }
    else
    {
        toblackpal(duration);
    }
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
static void cmovcolor(void)
{
    readexec_opername();
    readexec_opername_saveD6();

    s16 index = alis.varD7;
    s16 change = alis.varD6;

    if (alis.platform.bpp == 4)
    {
        u8 r, g, b;
        if (change < 0)
        {
            change = -change;
 
            PAL_READ_RGB(image.mpalet, index, r, g, b);
            r = subcol(change     , r >> 5) << 5;
            g = subcol(change >> 4, g >> 5) << 5;
            b = subcol(change >> 8, b >> 5) << 5;
            PAL_WRITE_RGB(image.mpalet, index, r, g, b);
        }
        else
        {
            PAL_READ_RGB(image.mpalet, index, r, g, b);
            r = addcol(change     , r >> 5) << 5;
            g = addcol(change >> 4, g >> 5) << 5;
            b = addcol(change >> 8, b >> 5) << 5;
            PAL_WRITE_RGB(image.mpalet, index, r, g, b);
        }
        
        image.ftopal = 0xff;
        image.thepalet = 0;
        image.defpalet = 0;

        set_update_cursor();
    }
}

// Codopname no. 191 opcode 0xbe ctopalet
// fade-in to palette
static void ctopalet(void)
{
    readexec_opername();
    readexec_opername_saveD6();
    
    alis.flagmain = 0;
   
    s16 palidx = alis.varD7;   // negative = restore saved palette (original: tst.w / bmi)
    u16 duration = alis.varD6;
    
    if (dos_pal_active())
    {
        s32 addr = palidx < 0 ? 0 : adresdes(palidx);
        dos_ctopalet(palidx, (s16)duration, palidx < 0 ? NULL : alis.mem + addr + xread32(addr));
    }
    else if (palidx < 0)
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
static void cnumput(void)
{
    if (alis.platform.version >= 60)
    {
        alis.flagmain = 0;
        v6put.angx = 0;
        v6put.angy = 0;
        v6put.angz = 0;
        v6put.invert = 0;
        v6put.scale = 0x100;
        v6put.rotb1 = 0;
        v6put.rotate = 0;
        v6put.rotb2 = 0;
        sV6Prot p;
        getprot_x(&p);
        v6put.basesprite = (u32)p.elem;
        v6put.limit = p.limit;
        v6put.numcreated = 0;
        if (p.elem > 0)
            xwrite32((u32)p.elem + 0x3c, 0);
        
        readexec_opername_saveD7();
        v6put.depx = (s16)alis.varD7;
        readexec_opername_saveD7();
        v6put.depy = (s16)alis.varD7;
        readexec_opername_saveD7();
        v6put.depz = (s16)alis.varD7;
        readexec_opername_saveD7();
        v6put.textattr = (u8)alis.varD7;
        return;
    }

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
static void cscheart(void)
{
    readexec_opername();
    readexec_opername_saveD6();

    if (alis.platform.version >= 60)
    {
        s16 scr = (s16)xread16(alis.script->vram_org - 0x64);
        if (scr != 0)
        {
            u32 s = alis.basemain + (u16)scr;
            xwrite16(s + 0x62, alis.varD7);
            xwrite16(s + 0x64, alis.varD6);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }

        return;
    }

    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scr_unknown0x0a(screen_id, alis.varD7);
        set_scr_unknown0x0c(screen_id, alis.varD6);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 194 opcode 0xc1 cscpos
static void cscpos(void)
{
    readexec_opername();
    readexec_opername_saveD6();

    if (alis.platform.version >= 60)
    {
        s16 scr = (s16)xread16(alis.script->vram_org - 0x64);
        if (scr != 0)
        {
            u32 s = alis.basemain + (u16)scr;
            s16 x = alis.varD7, y = alis.varD6;
            xwrite16(s + 0x66, x);
            xwrite16(s + 0x68, y);
            xwrite16(s + 0x6a, x + (s16)xread16(s + 0x6e) - 1);
            xwrite16(s + 0x6c, y + (s16)xread16(s + 0x70) - 1);
            v6_screfresh(s);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }

        return;
    }

    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        if (alis.platform.kind == EPlatformMac)
        {
            s16 t7 = (s16)alis.varD7, t6 = (s16)alis.varD6;
            mac_update_pos(&t7, &t6);
            alis.varD7 = t7; alis.varD6 = t6;
        }

        set_scr_newx(screen_id, alis.varD7);
        set_scr_newy(screen_id, alis.varD6);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 195 opcode 0xc2 cscsize
static void cscsize(void)
{
    readexec_opername();
    readexec_opername_saveD6();

    if (alis.platform.version >= 60)
    {
        s16 scr = (s16)xread16(alis.script->vram_org - 0x64);
        if (scr != 0)
        {
            u32 s = alis.basemain + (u16)scr;
            s16 w = alis.varD7, h = alis.varD6;
            xwrite16(s + 0x6e, w);
            xwrite16(s + 0x70, h);
            xwrite16(s + 0x6a, w + (s16)xread16(s + 0x66) - 1);
            xwrite16(s + 0x6c, h + (s16)xread16(s + 0x68) - 1);
            v6_screfresh(s);
            xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
        }

        return;
    }

    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        if (alis.platform.version < 30)
        {
            if (alis.platform.kind == EPlatformMac)
            {
                s16 t7 = (s16)(alis.varD7 - 1), t6 = (s16)(alis.varD6 - 1);
                mac_update_pos(&t7, &t6);
                alis.varD7 = t7 + 1; alis.varD6 = t6 + 1;
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
static void cschoriz(void)
{
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

// v6 dispatch target for cschoriz
static void cschoriz_v6(void)
{
    s16 scene = alis.platform.version >= 60 ? (s16)xread16(alis.script->vram_org - 0x64) : get_0x16_screen_id(alis.script->vram_org);
    readexec_opername();
    u32 addr = alis.basemain + (u16)scene + 0x59;
    u8 dl = (u8)alis.varD7;
    if (alis.varD7 & 0x100)
    {
        xwrite8(addr, xread8(addr) | dl);
    }
    else
    {
        xwrite8(addr, xread8(addr) & (u8)~dl);
    }
}

// Codopname no. 197 opcode 0xc4 cscvertic
static void cscvertic(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        readexec_opername();
        return;
    }

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
static void cscreduce(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        static int warned;
        readexec_opername();
        readexec_opername();
        if (!warned++)
            ALIS_DEBUG(EDebugWarning, "cscreduce 3DO: entity wake/sleep by distance not ported\n");
        return;
    }

    readexec_opername();
    readexec_opername_saveD6();

    u16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scr_creducing(screen_id, alis.varD7);
        
        readexec_opername();
        set_scr_redmax(screen_id, (u8)(alis.varD6 - 1));
        set_scr_redshift(screen_id, (u8)alis.varD7);
        set_scr_state(screen_id, get_scr_state(screen_id) | 0x80);
    }
}

// Codopname no. 199 opcode 0xc6 cscscale
static void cscscale(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        readexec_opername();
        u16 id = (u16)xread16(alis.script->vram_org - 0x64);
        if (id == 0)
            return;

        u32 scr = alis.basemain + id;
        for (int i = 0; i < 3; i++)
        {
            readexec_opername();
            xwrite16(scr + 0x1e + i * 2, (s16)alis.varD7);
        }

        xwrite8(scr + 0x58, xread8(scr + 0x58) | 0x80);
        return;
    }

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
static void creducing(void)
{
    readexec_opername();
    set_0x26_creducing(alis.script->vram_org, alis.varD7);
    readexec_opername();
    set_0x27_creducing(alis.script->vram_org, alis.varD7);
}

// v6 dispatch target for creducing
static void creducing_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    readexec_opername_saveD7();
    s32 v = (s32)alis.varD7;
    u8 setbits = (u8)v;
    u8 clrmask = (u8)~(u8)(v >> 16);

    if (p.aux != -1)
    {
        s32 slot = p.elem + p.aux;
        xwrite8(slot + 0x25, (xread8(slot + 0x25) | setbits) & clrmask);
        xwrite8(slot + 0x23, (xread8(slot + 0x23) & 0x8f) | 1);
        soundatten_v6((u32)slot);
        return;
    }

    s32 slot = p.elem;
    u8 count = xread8(slot + 0x33);
    for (u8 i = 0; i < count; i++)
    {
        if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
        {
            xwrite8(slot + 0x25, (xread8(slot + 0x25) | setbits) & clrmask);
            xwrite8(slot + 0x23, (xread8(slot + 0x23) & 0x8f) | 1);
            soundatten_v6((u32)slot);
        }

        slot += V6_SLOT_STRIDE;
    }
}

// Codopname no. 201 opcode 0xc8 cscmap
static void cscmap(void)
{
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

// Codopname no. 203 opcode 0xca cfindcla
static void cfindcla(void)
{
    readexec_opername();
    s32 tabidx = 0;
    s16 entidx = (s16)alis_first_ent();

    if (alis.platform.version >= 60)
    {
        do
        {
            u32 tgt_vram = xread32(alis.atent + entidx);
            if (tgt_vram != 0 && (s16)alis.varD7 == xread16(tgt_vram + 0x46))
            {
                alis.matent[tabidx] = 0;
                alis.tablent[tabidx] = entidx;
                tabidx++;
                if (alis.fallent == 0)
                    break;
            }

            entidx = xread16(alis.atent + entidx + 4);
        }
        while (entidx != 0);

        alis.tablent[tabidx] = ENTLIST_END;
        alis.fallent = 0;
        crstent();
        return;
    }

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
    
    alis.tablent[tabidx] = ENTLIST_END;
    alis.fallent = 0;
    
    crstent();
}

// Codopname no. 204 opcode 0xcb cnearcla
static void cnearcla(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(2);
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 205 opcode 0xcc cviewcla
static void cviewcla(void)
{
    if (alis.platform.version >= 60)
    {
        near_v6(2);
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 206 opcode 0xcd cinstru
static void cinstru(void)
{
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
        type = xread8(addr);
        if (type != 1 && type != 2 && type != 5 && type != 6)
            return;
        
        addr += 0x10;
    }
    
    audio.tabinst[tabidx].address = addr;
    audio.tabinst[tabidx].unknown = instidx;
}

// Codopname no. 207 opcode 0xce cminstru
static void cminstru(void)
{
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
        u8 type = xread8(addr);
        if (type != 1 && type != 2 && type != 5 && type != 6)
            return;
        
        addr += 0x10;
    }
    
    audio.tabinst[tabidx].address = addr;
    audio.tabinst[tabidx].unknown = instidx;
}

// Codopname no. 208 opcode 0xcf cordspr
static void cordspr(void)
{
    readexec_opername();
    set_0x2b_cordspr(alis.script->vram_org, (u8)alis.varD7);
}

// Codopname no. 209 opcode 0xd0 calign
static void calign(void)
{
    u8 bit;

    readexec_opername();

    if ((u8)alis.varD7 & 0x10)
    {
        bit = 4;
    }
    else if ((u8)alis.varD7 & 0x8)
    {
        bit = 3;
    }
    else if ((u8)alis.varD7 & 0x4)
    {
        bit = 2;
    }
    else if ((u8)alis.varD7 & 0x2)
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

// v6 dispatch target for calign
static void calign_v6(void)
{
    alis.flagmain = 0;
    v6put.invert = xread8(alis.script->vram_org + 0x25);
    sV6Prot p;
    getprot_x(&p);
    v6put.limit = p.limit;
    v6put.basesprite = (u32)p.elem;
    v6put.desc = 0;
    v6put.defstate = 0x10;
    s32 e = p.elem;

    readexec_opername_saveD7();
    xwrite8(e + 0x56, (u8)alis.varD7);
    readexec_opername_saveD7();
    xwrite16(e + 0x48, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(e + 0x4a, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(e + 0x4c, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(e + 0x4e, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(e + 0x40, alis.varD7);
    if (alis.varD7 == -1)
        xwrite8(e + 0x56, 1);
    
    readexec_opername_saveD7();
    u32 vol = (u32)(s32)alis.varD7;
    if (vol >= 0x100)
        vol = 0xff;
    
    xwrite8(e + 0x53, (u8)vol);
    readexec_opername_saveD7();
    xwrite16(e + 0x54, 0xff);
    xwrite8(e + 0x50, 0);
    xwrite8(e + 0x51, 0x7f);
    xwrite8(e + 0x52, 0);
    readexec_opername_saveD7();
    v6put.depx = (s32)alis.varD7;
    readexec_opername_saveD7();
    v6put.depy = (s32)alis.varD7;
    readexec_opername_saveD7();
    v6put.depz = (s32)alis.varD7;
    v6put.angx = 0;
    v6put.angy = 0;
    v6put.angz = 0;
    v6put.fadddes = 0;
    if (p.elem > 0)
        put_v6((u32)p.elem);
}

void rescmode(s16 at, u8 value)
{
    SPRITE_VAR(at)->numelem = value;
}

s16 starbuff[8];

// Codopname no. 210 opcode 0xd1 cbackstar
static void cbackstar(void)
{
    ALIS_DEBUG(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    
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

    switch (alis.platform.uid)
    {
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
        case EGameCrystalsOfArborea2:
        case EGameMetalMutant:
        case EGameTransarctica:
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
}

// Codopname no. 211 opcode 0xd2 cstarring
static void cstarring(void)
{
    ALIS_DEBUG(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    
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
// [N/I]: Robinson's Requiem, Ishar 3, Storm Master (IBM PC)
static void cengine(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        return;
    
    if (alis.platform.kind == EPlatformPC)
    {
        ALIS_DEBUG(EDebugWarning, "[N/I] %s: %s", alis.platform.desc, __FUNCTION__);
        return;
    }

    readexec_opername();
    readexec_opername();
    readexec_opername();
    readexec_opername();
}

// Codopname no. 214 opcode 0xd5 cquality
static void cquality(void)
{
    readexec_opername();
    alis.vquality = alis.varD7;
}

// Codopname no. 215 opcode 0xd6 chsprite
static void chsprite(void)
{
    ALIS_DEBUG(EDebugWarning, "STUBBED: %s", __FUNCTION__);

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
static void cselpalet(void)
{
    readexec_opername();
    if (dos_pal_active())
    {
        dos_cselpalet(alis.varD7);
        return;
    }

    alis.varD7 &= 0x3; // 4 palettes: 0...3
    image.thepalet = alis.varD7;
    image.defpalet = 1;   // the original only records the bank; palette writes select it
}

// Codopname no. 217 opcode 0xd8 clinepalet
static void clinepalet(void)
{
    readexec_opername();
    readexec_opername_saveD6();

    if (alis.platform.bpp < 8 && !dos_pal_active() && !(alis.platform.uid == EGameMetalMutant && alis.platform.kind == EPlatformPC))
    {
        setlinepalet();
    }
}

// Codopname no. 218 opcode 0xd9 cautomode
static void cautomode(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        readexec_opername();
        return;
    }
    
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 219 opcode 0xda cautofile
static void cautofile(void)
{
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
static void ccancel(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// v6 dispatch target for ccancel
static void ccancel_v6(void)
{
    v6_erase_entry(0);
}

// Codopname no. 221 opcode 0xdc ccancall
static void ccancall(void)
{
    alis.ferase = 1;

    u16 tmpidx = 0;
    u16 curidx = get_0x18_unknown(alis.script->vram_org);
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }

    alis.ferase = 0;
}

// v6 dispatch target for ccancall
static void ccancall_v6(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        return;

    sV6Prot p1, p2;
    getprot_x(&p1);
    getprot_x(&p2);
    if (p2.elem != 0 && p1.elem > 0)
        xwrite32((u32)p1.elem + 0x8, (u32)p2.elem);
}

// Codopname no. 222 opcode 0xdd ccancen
static void ccancen(void)
{
    alis.ferase = 1;
    cerasen();
}

// v6 dispatch target for ccancen
static void ccancen_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    if (p.aux != -1)
    {
        u32 tgt = (u32)p.elem + (u32)p.aux;
        readexec_opername_saveD7();
        s32 mvx = (s32)alis.varD7;
        readexec_opername_saveD7();
        s32 mvy = (s32)alis.varD7;
        readexec_opername_saveD7();
        s32 mvz = (s32)alis.varD7;
        xwrite32(tgt + 0x0c, mvx);
        xwrite32(tgt + 0x10, mvy);
        xwrite32(tgt + 0x14, mvz);
        return;
    }

    readexec_opername_saveD7();
    s32 x = (s32)alis.varD7;
    readexec_opername_saveD7();
    s32 y = (s32)alis.varD7;
    readexec_opername_saveD7();
    s32 z = (s32)alis.varD7;
    if (p.elem > 0)
    {
        u8 count = xread8((u32)p.elem + 0x33);
        u32 sp = (u32)p.elem;
        for (u8 i = 0; i < count; i++, sp += V6_SLOT_STRIDE)
        {
            if (v6sp_parent(sp) != alis.script->vram_org)
                continue;
            
            v6sp_set_depx(sp, x);
            v6sp_set_depy(sp, y);
            v6sp_set_depz(sp, z);
            if (v6sp_state(sp) == 0x30)
            {
                xwrite16(sp + 0x48, (s16)x);
                xwrite16(sp + 0x4a, (s16)y);
                xwrite16(sp + 0x4c, (s16)z);
            }

            v6sp_set_flags(sp, v6sp_flags(sp) | 1);
        }
    }
}

// Codopname no. 223 opcode 0xde cblast
static void cblast(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername_saveD7();
        alis.ferase = 0;
        if (alis.varD7 > 0)
            killent(alis.varD7);
        
        return;
    }

    alis.ferase = 1;
    ckill();
}

void iniback(int addr)
{
#if !defined(ALIS_NATIVE_16BPP)
    // Background cache: full-screen shadow laid out like image.logic, so behind-sprites use the
    // blitters' absolute addressing and backsprite is a plain rect copy. (The original packs a tight
    // rect buffer, only consistent when backy1==0.) Never cleared, as in the original.
    (void)addr;
    if (image.bgcache_alloc)
        free(image.bgcache_alloc);
    image.bgcache_alloc = (u8 *)malloc(image.buffer_alloc_size);
    if (image.bgcache_alloc == NULL)
    {
        image.sback = 0;
        image.backmap = NULL;
    }
    else
    {
        memset(image.bgcache_alloc, 0, image.buffer_alloc_size);
        image.backmap = image.bgcache_alloc + host.pixelbuf.surface_h * alis.platform.width;
        image.backlarg = alis.platform.width;
        image.backdes = 0;
        image.backaddr = 0;
    }
#else
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

        // Bitmap header for the flattened backsprite (planar/16bpp path): read back by
        // destofen with read16 (= read16be for a BE title); byte 0 is the sprite format.
        // Store in game/platform byte order (matching the original move.w at 0xbd82).
        u8 *bhdr = alis.mem + xread32(bdes) + bdes;     // = mem + bdes + 4
        s16 bw = image.backx2 - image.backx1;
        s16 bh = image.backy2 - image.backy1;
        bhdr[0] = 0x02; bhdr[1] = 0x00;                 // format word 0x0200 (fmt=02, flags=00)
        if (alis.platform.is_little_endian)
        {
            bhdr[2] = (u8)(bw & 0xff); bhdr[3] = (u8)(bw >> 8);
            bhdr[4] = (u8)(bh & 0xff); bhdr[5] = (u8)(bh >> 8);
        }
        else
        {
            bhdr[2] = (u8)(bw >> 8); bhdr[3] = (u8)(bw & 0xff);
            bhdr[4] = (u8)(bh >> 8); bhdr[5] = (u8)(bh & 0xff);
        }
    }
#endif
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
#if !defined(ALIS_NATIVE_16BPP)
                if (image.bgcache_alloc)
                {
                    free(image.bgcache_alloc);
                    image.bgcache_alloc = NULL;
                    image.backmap = NULL;
                }
#else
                io_mfree(image.backaddr);
#endif

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
                    // fenetre31 requires newf >= 0, which cscback never sets: force it drawable.
                    backsprite->newf = 0;

                    s16 elemidx = xread16(tgtaddr + 0x02);
                    // asm: bset.b #2,(0x1,A2) — byte RMW of numelem; a 16-bit one clobbers its other flags.
                    xwrite8(tgtaddr + 0x01, xread8(tgtaddr + 0x01) | 4);
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
static void cscrolpage(void)
{
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
static void cmatent(void)
{
    if (alis.platform.version >= 60)
    {
        alis.varD7 = (v6mat_n > 0) ? (s16)(v6mat_id[v6mat_n - 1] & 0xffff) : 0;
        cstore_continue();
        return;
    }

    int index = (int)(alis.ptrent - alis.tablent) - 1;
    alis.varD7 = alis.matent[index];
    cstore_continue();
}

// Codopname no. 227 opcode 0xe2 cshrink
static void cshrink(void)
{
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
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
        // Converted planar sprite (byte-6 stamp 4/8): storage is 2 extra header bytes + h*cpr*sp*2,
        // not width*height. Gated: on other builds bitmap[6] is a real header field.
        if ((*data == 0x14 || *data == 0x16) && (data[6] == 4 || data[6] == 8))
        {
            s32 cpr = (width + 15) >> 4;
            bits = 2 + (s32)height * cpr * (data[6] * 2);
        }
#endif

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
        
        ALIS_DEBUG(EDebugVerbose, " (NAME: %s, ID: 0x%x) ", alis.script->name, alis.script->data->header.id);
        shrinkprog(addr + 6, bits, 0);
        alis.script->data->sz -= bits;

        ALIS_DEBUG(EDebugVerbose, "\n (NAME: %s, ID: 0x%x SZ: %d) \n", alis.script->name, alis.script->data->header.id, alis.script->data->sz);
    }
}

// index of the highest set bit, 0 for 0 (3DO lsrs loop on the sign-extended word)
static s16 hibit(s16 v)
{
    s16 n = -1;
    u32 u = (u32)(s32)v;
    do
    {
        n++;
        u >>= 1;
    } while (u);

    return n;
}

// ARM unrolled 32-bit divide: x/0 yields all ones
static u32 udiv_arm(u32 num, u32 div)
{
    return div ? num / div : 0xffffffff;
}

// 3DO OPCODE_CDEFMAP_0xe3 @0001e82c
static void cdefmap_3do(void)
{
    u32 vram = alis.script->vram_org;
    u32 map = read_vaddr();

    readexec_opername();
    xwrite8(map - 0x400, alis.varD7);
    readexec_opername();
    xwrite16(map - 0x3fe, alis.varD7);
    xwrite16(map - 0x3fc, xread16(vram + 0x3a));
    readexec_opername();
    xwrite16(map - 0x3fa, alis.varD7);
    xwrite16(map - 0x3f8, alis.varD7);
    readexec_opername();
    xwrite16(map - 0x3f8, xread16(map - 0x3f8) + alis.varD7 - 1);
    readexec_opername();
    xwrite16(map - 0x3f4, alis.varD7);
    readexec_opername();
    xwrite16(map - 0x3f2, alis.varD7);
    readexec_opername();
    xwrite16(map - 0x3f0, alis.varD7);
    xwrite8(map - 0x3ff, 0);
    xwrite16(map - 0x3dc, 4);
    xwrite16(map - 0x3da, 0);
    xwrite8(map - 0x3d8, 0x7f);

    // the negative branch never loads r8, which still holds 0x7f from above
    u32 num = 0x7f, div;
    if ((s8)xread8(map - 1) < 0)
    {
        div = xread32(map - 6);
    }
    else
    {
        div = (u16)xread16(map - 4);
        num = xread32(map - 8);
    }

    xwrite16(map - 0x3c4, num);
    xwrite16(map - 0x3ca, udiv_arm(num, div));
    xwrite16(map - 0x3cc, udiv_arm(xread32(map - 0xc), num));
    xwrite16(vram - 0x3b6, 0);
    xwrite16(map - 0x3c0, hibit(xread16(map - 0x3f4)));
    xwrite16(map - 0x3be, hibit(xread16(map - 0x3f2)));
    xwrite16(map - 0x3b8, hibit(xread16(map - 0x3f0)));
    // the original's "-0x3d8 == 10" screen binding can't fire: -0x3d8 was just set to 0x7f
}

// Codopname no. 228 opcode 0xe3 cdefmap
// Ishar 3 Korean (IBM PC): cdefmap => map_cnul
static void cdefmap(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        cdefmap_3do();
        return;
    }

    if (alis.platform.uid == EGameTransarctica)
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
        
        u32 vala, valb;
        if ((s8)xread8(mapram - 1) < 0)
        {
            valb = xread32(mapram - 6);
            vala = xread32(mapram - 10);
        }
        else
        {
            valb = (u16)xread16(mapram - 4);
            vala = (u16)xread16(mapram - 6);
        }

        u16 ratio = vala / (u16)valb;
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
        ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
    }
}

// Codopname no. 229 opcode 0xe4 csetmap
// Ishar 3 Korean (IBM PC): csetmap => map_cnul
static void csetmap(void)
{
    u32 mapram = alis.script->vram_org;
    
    s16 offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        mapram = alis.basemain;
    }
    
    mapram += offset;
    
    if (alis.platform.uid == EGameTransarctica)
    {
        readexec_opername();
        xwrite16(mapram - 0x3e, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3c, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3a, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x38, alis.varD7);
        xwrite16(mapram - 0x26, (u16)alis.varD7 - 1);
        xwrite16(mapram - 0x3e, ((u16)(alis.varD7 - 1) >> 1) + xread16(mapram - 0x3e));
        readexec_opername();
        xwrite16(mapram - 0x36, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x34, alis.varD7);
        xwrite16(mapram - 0x24, (u16)alis.varD7 - 1);
        xwrite16(mapram - 0x3a, ((u16)(alis.varD7 - 1) >> 1) + xread16(mapram - 0x3a));
    }
    else if (alis.platform.uid == EGameRobinsonsRequiem0 || alis.platform.uid == EGameRobinsonsRequiem1 || alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        readexec_opername();
        xwrite16(mapram - 0x3ee, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3ec, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3ea, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3e8, alis.varD7);
        xwrite16(mapram - 0x3d6, (u16)alis.varD7 - 1);
        xwrite16(mapram - 0x3ee, ((u16)(alis.varD7 - 1) >> 1) + xread16(mapram - 0x3ee));
        readexec_opername();
        xwrite16(mapram - 0x3e6, alis.varD7);
        readexec_opername();
        xwrite16(mapram - 0x3e4, alis.varD7);
        xwrite16(mapram - 0x3d4, (u16)alis.varD7 - 1);
        xwrite16(mapram - 0x3ea, ((u16)(alis.varD7 - 1) >> 1) + xread16(mapram - 0x3ea));
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
    
    // TODO: credon_off >= 0 path (creducing from 0x26/0x27).
    
    sprite->chsprite = get_0x2f_chsprite(alis.script->vram_org);
    if ((s8)get_0x25_credon_credoff(alis.script->vram_org) == -0x80)
    {
        sprite->newzoomx = get_0x36_unknown(alis.script->vram_org);
        sprite->newzoomy = get_0x38_unknown(alis.script->vram_org);
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
// Ishar 3 Korean (IBM PC): cputmap => map_cnul
static void cputmap(void)
{
    // Resolve map base address
    u32 map_base = alis.script->vram_org;

    s16 offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        map_base = alis.basemain;
    }

    map_base += offset;

    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        readexec_opername();
        xwrite16(map_base - 0x3e2, alis.varD7);
        readexec_opername();
        xwrite16(map_base - 0x3e0, alis.varD7);
        readexec_opername();
        xwrite16(map_base - 0x3de, alis.varD7);
        readexec_opername();
        u16 pos = xread16(map_base - 0x3de), w = xread16(map_base - 0x3f0);
        xwrite16(map_base - 0x3d2, w ? pos % w : pos);
        return;
    }

    if (alis.platform.uid == EGameTransarctica)
    {
        // Read map parameters
        readexec_opername();
        xwrite16(map_base - 0x32, alis.varD7);
        readexec_opername();
        xwrite16(map_base - 0x30, alis.varD7);
        readexec_opername();
        xwrite16(map_base - 0x2e, alis.varD7);
        readexec_opername();
        image.numelem = (u8)alis.varD7;

        // Compute wrapped column index
        u32 map_pos = xread16(map_base - 0x2e);
        u16 map_width = xread16(map_base - 0x40);
        u16 col_idx = map_pos % map_width;
        xwrite16(map_base - 0x22, (s16)col_idx);
        alis.fmuldes = 0;

        // Find or create sprite element
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

        putmap(newidx, map_base - 0x2c);

        // Set up sprite display properties
        if (searchelem(&newidx, &oldidx))
        {
            sSprite *sprite = SPRITE_VAR(newidx);
            sprite->credon_off = -1;
            sprite->flaginvx = 0;
            sprite->chsprite = 0;
            sprite->depx = xread16(map_base - 0x3e);
            sprite->depy = xread16(map_base - 0x3c);
            sprite->depz = xread16(map_base - 0x3a);
        }
    }
    else if (alis.platform.uid == EGameRobinsonsRequiem0 || alis.platform.uid == EGameRobinsonsRequiem1)
    {
        // Read map parameters
        readexec_opername();
        xwrite16(map_base - 0x3e2, alis.varD7);
        readexec_opername();
        xwrite16(map_base - 0x3e0, alis.varD7);
        readexec_opername();
        xwrite16(map_base - 0x3de, alis.varD7);
        readexec_opername();
        image.numelem = (u8)alis.varD7;

        // Compute wrapped column index
        u32 map_pos = xread16(map_base - 0x3de);
        u16 map_width = xread16(map_base - 0x3f0);
        u16 col_idx = map_pos % map_width;
        xwrite16(map_base - 0x3d2, (s16)col_idx);
        alis.fmuldes = 0;

        // Find or create sprite element
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

        putmapin(newidx, map_base - 0x3dc);

        // Set up sprite display properties
        if (searchelem(&newidx, &oldidx))
        {
            sSprite *sprite = SPRITE_VAR(newidx);
            sprite->credon_off = -1;
            sprite->flaginvx = 0;
            sprite->chsprite = 0;
            sprite->depx = xread16(map_base - 0x3ee);
            sprite->depy = xread16(map_base - 0x3ec);
            sprite->depz = xread16(map_base - 0x3ea);
        }
    }
}

// Codopname no. 231 opcode 0xe6 csavepal
static void csavepal(void)
{
    readexec_opername();
    if (dos_pal_active())
    {
        dos_csavepal(alis.varD7);
    }
    else
    {
        savepal(alis.varD7);
    }
}

// Codopname no. 232 opcode 0xe7 csczoom
static void csczoom(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        if (xread16(alis.script->vram_org - 0x64) != 0)
        {
            for (int i = 0; i < 5; i++)
                readexec_opername();
        }
        
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 233 opcode 0xe8 ctexmap
// Ishar 3 Korean (IBM PC): ctexmap => map_cnul
static void ctexmap(void)
{
    alis.flagmain = 0;

    // Resolve scene base address
    u32 scene_addr = alis.script->vram_org;

    s16 offset = script_read16();
    if (offset == 0)
    {
        offset = script_read16();
        scene_addr = alis.basemain;
    }

    scene_addr += offset;

    // Compute texture entry address from slot index
    readexec_opername_saveD6();

    u32 entry = scene_addr + (alis.varD6 * 0x20) + (s16)0xf400;

    // Read exists flag (param 1) - stored at entry[8] on DOS, entry[1] on ST
    readexec_opername();
    if (alis.platform.is_little_endian)
    {
        xwrite8(entry + 8, (char)alis.varD7);
    }
    else
    {
        xwrite8(entry + 1, (char)alis.varD7);
    }

    // Resolve texture resource address if exists flag > 0
    readexec_opername();
    s8 exists_flag = alis.platform.is_little_endian ? (s8)xread8(entry + 8) : (s8)xread8(entry + 1);
    if (exists_flag > 0)
    {
        xwrite32(entry + 4, adresdes(alis.varD7));

        {
            u32 res_addr = xread32(entry + 4);
            if (res_addr != 0)
            {
                u32 bitmap_addr = res_addr + xread32(res_addr);

                u8 fmt = xread8(bitmap_addr);

#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
                // Terrain textures may have been planarized at load; the 3D rasterizer samples chunky.
                { extern int deplanar_8bit_inplace(u32 bitmap_addr);
                  deplanar_8bit_inplace(bitmap_addr); }
#endif

                // 0x14/0x16 and 0x1C/0x1E carry an 8-byte header; 0x00/0x02 carry 6.
                u32 pixel_offset = (fmt == 0x1C || fmt == 0x1E ||
                                    fmt == 0x14 || fmt == 0x16) ? 8 : 6;

                // Compute yshift = bit count of x_mask (matches DOS: INC CL; SHR AX,1; JNZ)
                u16 x_mask = (u16)xread16(bitmap_addr + 2);
                u8 yshift = 0;
                {
                    u16 tmp = x_mask;
                    do
                    {
                        yshift++;
                        tmp >>= 1;
                    }
                    while (tmp);
                }
                
                xwrite8(entry, yshift);

                // Initialize yorigin to 0
                xwrite16(entry + 2, 0);

                // Scan texture rows for first fully opaque row (no zero pixels).
                // Stores yorigin at entry[2] to skip transparent top rows.
                s16 y_mask = (s16)xread16(bitmap_addr + 4);
                u32 pixel_ptr = bitmap_addr + pixel_offset;
                s16 rows_rem = y_mask;
                u8 found_opaque = 0;

                while (rows_rem >= 0)
                {
                    s16 cols = (s16)x_mask;
                    u8 has_zero = 0;
                    while (cols >= 0)
                    {
                        if (xread8(pixel_ptr) == 0)
                            has_zero = 1;
                        
                        pixel_ptr++;
                        cols--;
                    }

                    if (!has_zero)
                    {
                        found_opaque = 1;
                        break;
                    }

                    rows_rem--;
                }

                if (found_opaque)
                {
                    s16 yorigin = y_mask - rows_rem;
                    xwrite16(entry + 2, yorigin);

                    if (yorigin != 0)
                    {
                        // Adjust bitmap y_mask to snap to power-of-2 within opaque region
                        u16 available = y_mask - yorigin;
                        static const u16 pow2m1[] = {1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767};
                        for (int i = 0; i < 15; i++)
                        {
                            if (pow2m1[i] == available)
                                break;  // exact match, keep original y_mask
                            
                            if (pow2m1[i] > available)
                            {
                                if (i > 0)
                                    xwrite16(bitmap_addr + 4, pow2m1[i - 1] + yorigin);
                                
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (alis.platform.is_little_endian && exists_flag == 0)
    {
        // DOS: explicitly clear texture fields for no-texture types
        xwrite32(entry + 4, 0);
        xwrite8(entry, 0);
        xwrite16(entry + 2, 0);
    }

    // Read texture parameters
    readexec_opername();
    xwrite16(entry + 10, alis.varD7);
    readexec_opername();
    if (alis.platform.is_little_endian)
    {
        // DOS: param 4 stores only at entry[9]
        xwrite8(entry + 9, (char)alis.varD7);
    }
    else
    {
        // ST/Amiga: param 4 stores at entry[8] and entry[9]=param+1
        xwrite8(entry + 8, (char)alis.varD7);
        xwrite8(entry + 9, (char)alis.varD7 + 1);
    }

    readexec_opername();
    xwrite16(entry + 0xe, alis.varD7);

    // Read mapping type and dispatch
    readexec_opername();
    char type = (char)alis.varD7;
    xwrite8(entry + 0x14, type);
    if (type == 0)
    {
        // Type 0: no mapping, skip 4 operands
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
    }
    else if (type < 0)
    {
        // Negative type: grid-based terrain texture mapping
        readexec_opername();
        xwrite16(entry + 0x10, alis.varD7);
        readexec_opername();
        xwrite16(entry + 0x12, alis.varD7);
        readexec_opername();
        xwrite16(entry + 0x16, alis.varD7);
        readexec_opername();
        xwrite16(entry + 0x1a, alis.varD7);
        s32 tex_offset = calctop(scene_addr, xread16(entry + 0x10), xread16(entry + 0x12));
        xwrite32(entry + 0x10, tex_offset);
        if (tex_offset < 0)
        {
            xwrite8(entry + 0x14, 0);
        }
    }
    else if (type == 2)
    {
        // Type 2: simple texture with scale
        readexec_opername();
        xwrite16(entry + 0x16, alis.varD7);
        readexec_opername();
        readexec_opername();
        readexec_opername();
        xwrite16(entry + 0x1a, alis.varD7);
    }
    else
    {
        // Type 1+: sprite-based texture mapping
        readexec_opername();
        xwrite32(entry + 0x10, adresdes(alis.varD7));
        xwrite16(entry + 0x1c, xread16(alis.script->vram_org - 0xe));
        readexec_opername();
        xwrite16(entry + 0x16, alis.varD7);
        readexec_opername();
        xwrite8(entry + 0x15, (char)alis.varD7);
        readexec_opername();
        xwrite16(entry + 0x1a, alis.varD7);
    }

    if (alis.platform.bpp == 8)
    {
        // Histogram analysis for 8-bit textures (types 0x1c/0x1e)
        // Computes the two most frequent pixel colors for terrain dithering (entry[8], entry[9])
        if (!alis.platform.is_little_endian && (s8)xread8(entry + 8) <= 0 && (s8)xread8(entry + 1) > 0)
        {
            u32 res_addr = xread32(entry + 4);
            u32 bitmap_addr = res_addr + xread32(res_addr);
            u8 bmp_type = xread8(bitmap_addr);
            
            if (bmp_type == 0x1c || bmp_type == 0x1e)
            {
                u16 histogram[256] = {0};
                
                u32 width = xread16(bitmap_addr + 2) + 1;
                u32 height = xread16(bitmap_addr + 4) + 1;
                u32 total_pixels = width * height;
                
                // Count pixel occurrences
                u32 pixel_addr = bitmap_addr + 8;
                for (u32 i = 0; i < total_pixels; i++)
                {
                    u8 color = xread8(pixel_addr + i);
                    histogram[color]++;
                }
                
                // Pass 1: find most frequent non-zero color
                u16 max_count = 0;
                u8 winner1 = 0;
                for (int i = 1; i < 256; i++)
                {
                    if (histogram[i] >= max_count)
                    {
                        max_count = histogram[i];
                        winner1 = (u8)i;
                    }
                }

                xwrite8(entry + 8, winner1);
                
                // Pass 2: find second most frequent, excluding first winner
                max_count = 0;
                u8 winner2 = 0;
                for (int i = 1; i < 256; i++)
                {
                    if (histogram[i] >= max_count && i != winner1)
                    {
                        max_count = histogram[i];
                        winner2 = (u8)i;
                    }
                }

                xwrite8(entry + 9, winner2);
            }
        }
    }
}

// v6 dispatch target for ctexmap
static void ctexmap_v6(void)
{
    alis.v6tstpolar = 1;
    u32 vram = alis.script->vram_org;
    alis.v6tstdx = (s16)xread16(vram + 0x26);
    alis.v6tstdy = (s16)xread16(vram + 0x28);
    readexec_opername_saveD7();
    alis.v6walkmode = (u8)alis.varD7;
    readexec_opername_saveD7();
    alis.v6walkc4 = (s32)alis.varD7;
    alis.v6walkcc = (s32)alis.varD7;
    readexec_opername_saveD7();
    alis.v6walkd0 = (s32)alis.varD7;
    readexec_opername_saveD7();
    alis.v6walkd8 = (u8)alis.varD7;
    readexec_opername_saveD7();
    alis.v6tstmask = (u16)alis.varD7;
    {
        sV6Prot p;
        getprot_x(&p);
        alis.v6walkb8 = p.elem;
        if (p.elem > 0 && xread8(p.elem + 0x22) == 0x33)
            alis.v6walkcc = (u16)xread16(p.elem + 0x40);
    }

    v6wk_cbstep = 1;
    alis.varD7 = v6wk_stepper();
    alis.v6tstpolar = 0;
    cstore_continue();
    crstent();
}

// Codopname no. 234 opcode 0xe9 calloctab
static void calloctab_v6(u32 cell)
{
    u32 size = xread32(cell - 8 - xread8(cell - 1) * 4) + 0x800 + 8;
    s32 h = io_malloc(size);
    if (h == 0)
        return;

    u32 blk = xread32(h);
    memset(alis.mem + blk, 0, size);
    u32 pad = ((blk + 8 + 0x7ff) & ~0x7ffu) - (blk + 8);
    xwrite32(blk, 8 + pad);
    xwrite32(blk + 4, size);
    xwrite32(cell, blk);
}

static u32 tabcell_flag(u32 cell)
{
    return xread8(cell - (alis.platform.version >= 60 ? 2 : 1));
}

static void calloctab(void)
{
    u32 cell = read_vaddr();
    if ((s8)tabcell_flag(cell) >= 0 || xread32(cell) != 0)
        return;
    
    if (alis.platform.version >= 60)
    {
        calloctab_v6(cell);
        return;
    }

    s32 size = xread32(cell - 6 - (xread8(cell - 1) & 0xf) * 4);
    xwrite32(cell, io_malloc(size));
}

// Codopname no. 235 opcode 0xea cfreetab
static void cfreetab(void)
{
    if (alis.platform.version < 30)
    {
        ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
        return;
    }

    u32 cell = read_vaddr();
    u32 h = xread32(cell);
    if ((s8)tabcell_flag(cell) >= 0 || h == 0)
        return;
    xwrite32(cell, 0);
    if (alis.platform.version < 60)
    {
        io_mfree(h);
        return;
    }

    for (int i = 0; i < 0x10; i++)
    {
        if (xread32(alis.tabptr + i * 8) == h)
        {
            io_mfree(alis.tabptr + i * 8);
            return;
        }
    }
}

// Codopname no. 238 opcode 0xed cscsun
static void cscsun(void)
{
    if (alis.platform.version < 60)
    {
        ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
        return;
    }

    sV6Prot p;
    getprot_x(&p);
    u8 v[3];
    for (int i = 0; i < 3; i++)
    {
        readexec_opername();
        v[i] = alis.varD7;
    }

    if (p.elem <= 0)
        return;
    u32 slot = p.elem + (p.aux == -1 ? 0 : p.aux);
    u8 n = p.aux == -1 ? xread8(p.elem + 0x33) : 1;
    int is3do = alis.platform.uid == EGameRobinsonsRequiem3DO;
    for (; n; n--, slot += V6_SLOT_STRIDE)
    {
        if (is3do)
        {
            xwrite8(slot + 0x30, v[0]);
            xwrite8(slot + 0x2e, v[1]);
            xwrite8(slot + 0x2f, v[2]);
            continue;
        }

        xwrite8(slot + 0x2e, v[0]);
        xwrite8(slot + 0x30, v[1]);
        xwrite8(slot + 0x2f, v[2]);
        if (p.aux == -1)
            xwrite8(slot + 0x23, xread8(slot + 0x23) | 1);
    }
}

// Codopname no. 239 opcode 0xee cdarkpal
static void cdarkpal(void)
{
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

    if (alis.platform.uid == EGameDeus)
    {
        s32 first = (u16)darkpac, last = (u16)darkpan;
        if (last > 0xff)
            last = 0xff;

        if (last - first + 1 <= 0)
            return;

        for (s32 i = first; i <= last; i++)
        {
            image.dkpalet[i * 3 + 0] = darkpar;
            image.dkpalet[i * 3 + 1] = darkpav;
            image.dkpalet[i * 3 + 2] = darkpab;
        }

        image.dkpalet[0] = image.dkpalet[1] = image.dkpalet[2] = 0;
        image.fdarkpal = 0;
        for (int i = 3; i < 3 + 0x2fd; i++)
        {
            if (image.dkpalet[i] != 0x100)
            {
                image.fdarkpal = 1;
                break;
            }
        }
        
        return;
    }

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

        for (int i = darkpac; i < length; i++)
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

void v6_scdark_precalc(u32 scr)
{
    u8 mode = xread8(scr + 0x94);
    if (mode == 1)
    {
        s32 range = xread32(scr + 0x146);
        s32 near = range - (s32)((u32)range >> 2);
        xwrite32(scr + 0x16e, near);
        xwrite16(scr + 0xae, (u16)xread16(scr + 0x96) >> 1);
        s32 den = range - near;
        if (den == 0)
            den = 1;
        
        xwrite32(scr + 0xaa, 0x10000 / den);
    }
    else
    {
        s32 range = xread32(scr + 0x146);
        xwrite32(scr + 0x16e, (u32)range >> 2);
        xwrite16(scr + 0xae, xread16(scr + 0x96));
        xwrite32(scr + 0xaa, 0);
    }

    s32 den = xread32(scr + 0x146) - xread32(scr + 0x16e);
    if (den == 0)
        den = 1;
    
    xwrite32(scr + 0xa6, ((s32)(s16)xread16(scr + 0xa0) << 9) / den);
}

// Codopname no. 240 opcode 0xef cscdark
static void cscdark(void)
{
    alis.flagmain = 0;

    u32 screen = alis.basemain + get_0x16_screen_id(alis.script->vram_org);

    // Read darkness config value
    readexec_opername();
    xwrite16(screen + 0x98, alis.varD7);

    // Resolve darkness palette resource address
    readexec_opername();
    u32 dark_addr = adresdes(alis.varD7);
    xwrite32(screen + 0x9a, xread32(dark_addr) + dark_addr);

    // Read darkness level
    readexec_opername();
    xwrite16(screen + 0x9e, alis.varD7);

    // Mark screen as having darkness enabled
    xwrite8(screen, xread8(screen) | 0x80);

    // Compute darkness table height from resource header
    s16 dark_height = xread16(xread32(screen + 0x9a) + 4) + 1;
    xwrite16(screen + 0xa0, dark_height);
    if (xread16(screen + 0x9e) < 0)
    {
        // Negative dark level: default to half table height
        xwrite16(screen + 0x9e, dark_height >> 1);
    }

    // Set global darkness base and pointer
    alis.basedark = xread16(screen + 0x9e);
    alis.ptrdark = xread32(screen + 0x9a) + (alis.platform.bpp == 4 ? 6 : 8);
}

// v6 dispatch target for cscdark
static void cscdark_v6(void)
{
    u32 scr = alis.basemain + (u16)xread16(alis.script->vram_org - 0x64);

    readexec_opername_saveD7();
    xwrite16(scr + 0x94, alis.varD7);
    readexec_opername_saveD7();
    s32 form = (s16)alis.varD7;
    if (form >= 0)
        form = adresdes(alis.varD7);
    
    xwrite32(scr + 0xb0, form);
    readexec_opername_saveD7();
    xwrite16(scr + 0x96, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(scr + 0x98, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(scr + 0x9a, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(scr + 0x9c, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(scr + 0x9e, alis.varD7);
    readexec_opername_saveD7();
    xwrite32(scr + 0xa2, (s32)alis.varD7);
    readexec_opername_saveD7();
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        alis.fog3do = alis.varD7;
    
    xwrite16(scr + 0xa0, alis.varD7 - (s16)xread32(scr + 0xa2));

    s32 cx = (s16)xread16(scr + 0x96);
    if (xread8(scr + 0x94) == 1)
        cx = (s32)((u32)cx >> 1);
    
    xwrite32(scr + 0xa2, xread32(scr + 0xa2) * cx);
    xwrite16(scr + 0xa0, (s16)(xread16(scr + 0xa0) * (s16)cx));
    v6_scdark_precalc(scr);

    readexec_opername_saveD7();
    xwrite16(scr + 0xb8, alis.varD7);
    readexec_opername_saveD7();
    xwrite16(scr + 0xba, alis.varD7);

    if (alis.platform.kind == EPlatform3DO || (xread16(scr + 0x94) & 0x200) == 0)
    {
        s32 desc = xread32(scr + 0xb0);
        desc += xread32(desc);
        if (xread8(desc) == 3)
        {
            if (alis.platform.kind == EPlatform3DO)
                node_take(0xb);

            v6shad_build3(scr, (u32)desc);
        }
        else
        {
            xwrite32(scr + 0xb4, desc);
            if (alis.platform.uid == EGameRobinsonsRequiem3DO)
                goto done;

            s32 d2 = desc + xread32(desc);
            u16 cx2 = xread16(d2 + 0x12);
            xwrite16(scr + 0x96, cx2);
            if ((s16)xread16(scr + 0x98) < 0)
            {
                cx2 >>= 1;
                xwrite16(scr + 0x98, cx2);
            }

            ALIS_DEBUG(EDebugWarning, "cscdark v6: non-type-3 land init partially ported\n");
        }
    }

done:
    xwrite8(scr + 0x58, xread8(scr + 0x58) | 0x80);
}

// Codopname no. 241 opcode 0xf0 caset
static void caset(void)
{
    if (alis.platform.version < 30)
    {
        ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
        return;
    }

    u32 vram = alis.script->vram_org;
    readexec_opername();
    xwrite16(vram + 0x18, alis.varD7);
    readexec_opername();
    xwrite16(vram + 0x20, alis.varD7);
    readexec_opername();
    xwrite16(vram + 0x28, alis.varD7);
}

// v6 dispatch target for caset
static void caset_v6(void)
{
    u32 vram = alis.script->vram_org;
    readexec_opername_saveD7();
    xwrite16(vram + 0x18, (s16)(alis.varD7 << 8));
    readexec_opername_saveD7();
    xwrite16(vram + 0x1a, (s16)(alis.varD7 << 8));
    readexec_opername_saveD7();
    xwrite16(vram + 0x1c, (s16)(alis.varD7 << 8));
    xwrite8(vram + 0x45, 1);
}

// Codopname no. 242 opcode 0xf1 camov
static void camov(void)
{
    if (alis.platform.version < 30)
    {
        ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
        return;
    }

    u32 vram = alis.script->vram_org;
    readexec_opername();
    xadd16(vram + 0x18, alis.varD7);
    readexec_opername();
    xadd16(vram + 0x20, alis.varD7);
    readexec_opername();
    xadd16(vram + 0x28, alis.varD7);
}

// v6 dispatch target for camov
static void camov_v6(void)
{
    u32 vram = alis.script->vram_org;
    readexec_opername_saveD7();
    xadd16(vram + 0x18, (s16)(alis.varD7 << 8));
    readexec_opername_saveD7();
    xadd16(vram + 0x1a, (s16)(alis.varD7 << 8));
    readexec_opername_saveD7();
    xadd16(vram + 0x1c, (s16)(alis.varD7 << 8));
    xwrite8(vram + 0x45, 1);
}

void camovnat(void)
{
    u32 vram = alis.script->vram_org;
    readexec_opername_saveD7();
    xadd16(vram + 0x18, (s16)alis.varD7);
    readexec_opername_saveD7();
    xadd16(vram + 0x1a, (s16)alis.varD7);
    readexec_opername_saveD7();
    xadd16(vram + 0x1c, (s16)alis.varD7);
    xwrite8(vram + 0x45, 1);
}

void casetnat(void)
{
    u32 vram = alis.script->vram_org;
    readexec_opername_saveD7();
    xwrite16(vram + 0x18, (s16)alis.varD7);
    readexec_opername_saveD7();
    xwrite16(vram + 0x1a, (s16)alis.varD7);
    readexec_opername_saveD7();
    xwrite16(vram + 0x1c, (s16)alis.varD7);
    xwrite8(vram + 0x45, 1);
}

void csetvectnat(void)
{
    sV6Prot p;
    getprot_x(&p);
    if (p.aux != -1)
    {
        s32 slot = p.elem + p.aux;
        readexec_opername_saveD7();
        xadd16(slot + 0x18, (s16)alis.varD7);
        readexec_opername_saveD7();
        xadd16(slot + 0x1a, (s16)alis.varD7);
        readexec_opername_saveD7();
        xadd16(slot + 0x1c, (s16)alis.varD7);
        return;
    }

    readexec_opername_saveD7();
    s16 ax = alis.varD7;
    readexec_opername_saveD7();
    s16 bx = alis.varD7;
    readexec_opername_saveD7();
    s16 dx = alis.varD7;
    s32 slot = p.elem;
    u8 count = xread8(slot + 0x33);
    for (u8 i = 0; i < count; i++)
    {
        if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
        {
            xadd16(slot + 0x18, ax);
            xadd16(slot + 0x1a, bx);
            xadd16(slot + 0x1c, dx);
            xwrite8(slot + 0x23, xread8(slot + 0x23) | 1);
        }

        slot += V6_SLOT_STRIDE;
    }
}

void cdefvectnat(void)
{
    sV6Prot p;
    getprot_x(&p);
    if (p.aux != -1)
    {
        s32 slot = p.elem + p.aux;
        readexec_opername_saveD7();
        xwrite16(slot + 0x18, (s16)alis.varD7);
        readexec_opername_saveD7();
        xwrite16(slot + 0x1a, (s16)alis.varD7);
        readexec_opername_saveD7();
        xwrite16(slot + 0x1c, (s16)alis.varD7);
        return;
    }

    readexec_opername_saveD7();
    s16 ax = alis.varD7;
    readexec_opername_saveD7();
    s16 bx = alis.varD7;
    readexec_opername_saveD7();
    s16 dx = alis.varD7;
    s32 slot = p.elem;
    u8 count = xread8(slot + 0x33);
    for (u8 i = 0; i < count; i++)
    {
        if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
        {
            xwrite16(slot + 0x18, ax);
            xwrite16(slot + 0x1a, bx);
            xwrite16(slot + 0x1c, dx);
            xwrite8(slot + 0x23, xread8(slot + 0x23) | 1);
        }

        slot += V6_SLOT_STRIDE;
    }
}

// Codopname no. 243 opcode 0xf2 cscaset
static void cscaset(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 244 opcode 0xf3 cscamov
static void cscamov(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// v6 screen camera

static u32 v6_defscr(void)
{
    return alis.basemain + (s16)xread16(alis.script->vram_org - 0x64);
}

static void v6_readxyz(s32 v[3])
{
    for (int i = 0; i < 3; i++)
    {
        readexec_opername();
        v[i] = alis.varD7;
    }
}

static void v6_scdirty(u32 s)
{
    xwrite8(s + 0x58, xread8(s + 0x58) | 0x80);
}

// add to camera position (s32 +0xc/+0x10/+0x14)
static void cscmov_v6(void)
{
    s32 v[3];
    v6_readxyz(v);
    u32 s = v6_defscr();
    for (int i = 0; i < 3; i++)
    {
        if (v[i])
        {
            xwrite32(s + 0xc + i * 4, xread32(s + 0xc + i * 4) + v[i]);
            v6_scdirty(s);
        }
    }
}

// set camera position
static void cscset_v6(void)
{
    s32 v[3];
    v6_readxyz(v);
    u32 s = v6_defscr();
    for (int i = 0; i < 3; i++)
    {
        if (xread32(s + 0xc + i * 4) != v[i])
        {
            xwrite32(s + 0xc + i * 4, v[i]);
            v6_scdirty(s);
        }
    }
}

// add to camera angles
static void cscamov_v6(void)
{
    s32 v[3];
    v6_readxyz(v);
    u32 s = v6_defscr();
    for (int i = 0; i < 3; i++)
        xwrite16(s + 0x18 + i * 2, xread16(s + 0x18 + i * 2) + (v[i] << 8));
    v6_scdirty(s);
}

// set camera angles
static void cscaset_v6(void)
{
    s32 v[3];
    v6_readxyz(v);
    u32 s = v6_defscr();
    for (int i = 0; i < 3; i++)
        xwrite16(s + 0x18 + i * 2, v[i] << 8);
    v6_scdirty(s);
}

// map reference of the screen
static void cscmap_v6(void)
{
    readexec_opername();
    s16 owner = xread16(alis.script->vram_org + 0x3a);
    s16 off = script_read16();
    if (off == 0)
    {
        owner = 0;
        off = script_read16();
    }

    if ((s16)xread16(alis.script->vram_org - 0x64) != 0)
    {
        u32 s = v6_defscr();
        xwrite16(s + 0xc6, off);
        xwrite16(s + 0xc4, owner);
    }
}

// Codopname no. 245 opcode 0xf4 cscfollow
static void cscfollow(void)
{

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

// v6 dispatch target for cscfollow
static void cscfollow_v6(void)
{
    u32 screen = alis.basemain + (u16)xread16(alis.script->vram_org - 0x64);
    readexec_opername_saveD7();
    s16 id = alis.varD7;
    readexec_opername_saveD7();
    u16 flw = (u16)alis.varD7;
    readexec_opername_saveD7();
    s32 ox = (s32)alis.varD7;
    readexec_opername_saveD7();
    s32 oy = (s32)alis.varD7;
    readexec_opername_saveD7();
    s32 oz = (s32)alis.varD7;
    readexec_opername_saveD7();
    s16 ax = alis.varD7;
    readexec_opername_saveD7();
    s16 ay = alis.varD7;
    readexec_opername_saveD7();
    s16 az = alis.varD7;
    xwrite16(screen + 0x92, az);
    xwrite16(screen + 0x90, ay);
    xwrite16(screen + 0x8e, ax);
    xwrite32(screen + 0x8a, oz);
    xwrite32(screen + 0x86, oy);
    xwrite32(screen + 0x82, ox);
    xwrite8(screen + 0x80, (u8)flw);
    if (!(flw & 0x100))
    {
        xwrite16(screen + 0x8e, (s16)(xread16(screen + 0x8e) << 8));
        xwrite16(screen + 0x90, (s16)(xread16(screen + 0x90) << 8));
        xwrite16(screen + 0x92, (s16)(xread16(screen + 0x92) << 8));
    }

    xwrite16(screen + 0x7e, id);
    xwrite8(screen + 0x58, xread8(screen + 0x58) | 0x80);
}

// Codopname no. 246 opcode 0xf5 cscview
static void cscview(void)
{
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
static void cfilm(void)
{
    fli_stream_close();   // stale streaming film — don't leak the file handle
    memset(&bfilm, 0, sizeof(bfilm));
    alis.filmnode = 0;
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

void headland(s16 d4w)
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

s32 walkcx;
s32 walkcy;
s16 walkcz;
s16 walkocz;
s32 walkdx;
s32 walkdy;
s16 walkhaut;

u8 walkmap3;
u16 walkdim;

s16 walktete;

void landalti(s32 addr, s32 vram, s16 world_x, s16 world_y, s16 *out_alt, s16 *out_cell);

s16 walkland(s32 scene_addr)
{
    // Set material collision mask from walktete sign
    alis.matmask = 0;
    if (walktete < 0)
    {
        alis.matmask = walktete;
    }

    // Read azimuth trig values and walk offset from VRAM
    s16 sin_az = tabsin[xread16(alis.script->vram_org + ALIS_SCR_WCAZ)];
    s16 cos_az = tabcos[xread16(alis.script->vram_org + ALIS_SCR_WCAZ)];
    s16 offset_x = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCX2);
    s16 offset_y = (s8)xread8(alis.script->vram_org + ALIS_SCR_WCY2);

    // Compute forward walk direction vectors
    if (walktete != 0)
    {
        walkdx = (s32)-xread16(scene_addr - 0x3f4) * sin_az * 0x80;
        walkdy = (s32) xread16(scene_addr - 0x3f4) * cos_az * 0x80;
    }

    // Compute new world position by rotating walk offset by azimuth
    s32 world_x = xread32_wc(alis.script->vram_org, ALIS_SCR_WCX);
    s32 world_y = xread32_wc(alis.script->vram_org, ALIS_SCR_WCY);

    walkcx = world_x + ((s32)cos_az * offset_x - (s32)sin_az * (s32)offset_y) * 0x80;
    walkcy = world_y + ((s32)cos_az * (s32)offset_y + (s32)sin_az * offset_x) * 0x80;
    walkocz = xread16(alis.script->vram_org + ALIS_SCR_WCZ);

    // Query terrain altitude at new position
    s16 probe_z, probe_cell;
    landalti(scene_addr, alis.script->vram_org, walkcx >> 0x10, walkcy >> 0x10, &walkcz, &probe_cell);
    s16 delta_z = walkcz - walkocz;

    // Multi-point collision probes around walk path
    if (alis.matmask != 0)
    {
        landalti(scene_addr, alis.script->vram_org, ((u32)(world_x + walkdy + walkdx) >> 0x10), ((u32)(world_y + walkdx + walkdy) >> 0x10), &probe_z, &probe_cell);
        if (0xfe < probe_z)
        {
            delta_z = probe_z - walkocz;
            walkcz = probe_z;
        }

        landalti(scene_addr, alis.script->vram_org, ((u32)(world_x + (walkdx - walkdy)) >> 0x10), ((u32)(world_y + (walkdy - walkdx)) >> 0x10), &probe_z, &probe_cell);
        if (0xfe < probe_z)
        {
            delta_z = probe_z - walkocz;
            walkcz = probe_z;
        }

        landalti(scene_addr, alis.script->vram_org, ((u32)(world_x + walkdx * 2) >> 0x10), ((u32)(world_y + walkdy * 2) >> 0x10), &probe_z, &probe_cell);
        if (0xfe < probe_z)
        {
            delta_z = probe_z - walkocz;
            walkcz = probe_z;
        }

        landalti(scene_addr, alis.script->vram_org, ((u32)(world_x + walkdy + walkdx * 2) >> 0x10), ((u32)(world_y + walkdx + walkdy * 2) >> 0x10), &probe_z, &probe_cell);
        if (0xfe < probe_z)
        {
            delta_z = probe_z - walkocz;
            walkcz = probe_z;
        }

        landalti(scene_addr, alis.script->vram_org, ((u32)(world_x + (walkdx * 2 - walkdy)) >> 0x10), ((u32)(world_y + (walkdy * 2 - walkdx)) >> 0x10), &probe_z, &probe_cell);
        if (0xfe < probe_z)
        {
            delta_z = probe_z - walkocz;
            walkcz = probe_z;
        }
    }

    // Look-ahead slope estimation for head bobbing
    if (0 < walktete)
    {
        s16 abs_delta = delta_z;
        if (delta_z < 0)
        {
            abs_delta = -delta_z;
        }

        if (abs_delta <= walkhaut)
        {
            // Sample altitude at 1x, 2x, 4x walk steps with decreasing weights
            s16 sample_z;
            landalti(scene_addr, alis.script->vram_org, (s16)((u32)(world_x + walkdx) >> 0x10), (s16)((u32)(world_y + walkdy) >> 0x10), &sample_z, &probe_cell);
            s16 weighted_1x = sample_z = (sample_z - walkocz) * 4;
            landalti(scene_addr, alis.script->vram_org, (s16)((u32)(world_x + walkdx * 2) >> 0x10), (s16)((u32)(world_y + walkdy * 2) >> 0x10), &sample_z, &probe_cell);
            sample_z = (sample_z - walkocz) * 2;
            landalti(scene_addr, alis.script->vram_org, (s16)((u32)(world_x + walkdx * 4) >> 0x10), (s16)((u32)(world_y + walkdy * 4) >> 0x10), &sample_z, &probe_cell);
            u16 slope_sum = ((s16)(sample_z) - walkocz) + weighted_1x + sample_z;
            u16 abs_slope = slope_sum;
            if ((s16)slope_sum < 0)
            {
                abs_slope = -slope_sum;
            }

            // Clamp extreme slopes to zero
            if (0x3c0 < abs_slope)
            {
                slope_sum = 0;
            }

            // Scale slope by walktete factor
            if (alis.platform.is_little_endian)
            {
                // DOS: uses high byte of walktete (typically 0 for values < 256, disabling head-bob)
                s16 wt = (s16)((u16)walktete >> 8);
                sample_z = (s16)(((s32)(s16)slope_sum * (s32)wt >> 2) / 0xc);
            }
            else
            {
                sample_z = (s16)(((s32)(s16)slope_sum * (s32)(s16)walktete >> 2) / 0xc);
            }

            s16 abs_head = sample_z;
            if (sample_z < 0)
            {
                abs_head = -sample_z;
            }

            // Apply dead zone threshold
            if ((u8)abs_head < walkmap3)
            {
                sample_z = 0;
            }

            headland(sample_z);
        }
    }

    // Final altitude check: reject move if height change exceeds walkhaut
    u16 abs_delta_z = walkcz - walkocz;
    if ((s32)((u32)abs_delta_z << 0x10) < 0)
    {
        abs_delta_z = -abs_delta_z;
    }

    if (walkhaut < (s16)abs_delta_z)
    {
        // Move rejected: keep old XY, update Z at current position
        landalti(scene_addr, alis.script->vram_org, xread16(alis.script->vram_org + ALIS_SCR_WCX), xread16(alis.script->vram_org + ALIS_SCR_WCY), &probe_z, &probe_cell);
        xwrite16(alis.script->vram_org + ALIS_SCR_WCZ, probe_z);
    }
    else
    {
        // Move accepted: update full world position
        xwrite32_wc(alis.script->vram_org, ALIS_SCR_WCX, walkcx);
        xwrite32_wc(alis.script->vram_org, ALIS_SCR_WCY, walkcy);
        xwrite16(alis.script->vram_org + ALIS_SCR_WCZ, walkcz);
    }

    return walkcz - walkocz;
}

// Codopname no. 248 opcode 0xf7 cwalkmap
// Ishar 3 Korean (IBM PC): cwalkmap => map_cnul
static void cwalkmap(void)
{
    s16 offset = script_read16();
    u32 vram = alis.script->vram_org;
    if (offset == 0)
    {
        offset = script_read16();
        vram = alis.basemain;
    }
    
    readexec_opername();
    walkhaut = alis.varD7;
    readexec_opername();
    walktete = alis.varD7;
    readexec_opername_saveD6();
    walkdim = alis.varD6;
    
    alis.varD7 = walkland(vram + offset);
    
    cstore_continue();
}

// v6 dispatch target for cwalkmap
static void cwalkmap_v6(void)
{
    alis.v6tstpolar = 1;
    u32 vram = alis.script->vram_org;
    alis.v6tstdx = (s16)xread16(vram + 0x26);
    alis.v6tstdy = (s16)xread16(vram + 0x28);
    readexec_opername_saveD7();
    alis.v6walkmode = (u8)alis.varD7;
    readexec_opername_saveD7();
    alis.v6walkc4 = (s32)alis.varD7;
    alis.v6walkcc = (s32)alis.varD7;
    readexec_opername_saveD7();
    alis.v6walkd0 = (s32)alis.varD7;
    readexec_opername_saveD7();
    alis.v6walkd8 = (u8)alis.varD7;
    readexec_opername_saveD7();
    alis.v6tstmask = (u16)alis.varD7;
    readexec_opername_saveD7();
    alis.v6walkb8 = (s32)alis.varD7;
    if (alis.varD7 != -1)
    {
        s32 desc = adresdes(alis.varD7);
        u32 fdsc = (u32)desc + xread32(desc);
        if (xread8(fdsc) == 7)
            alis.v6walkcc = (u16)xread16(fdsc + 0x12);
    }

    v6wk_cbstep = 0;
    alis.varD7 = v6wk_stepper();
    alis.v6tstpolar = 0;
    cstore_continue();
    crstent();
}

sVector polarmov(s16 wcx2, s16 wcy2, s16 wcz2, s16 wcax, s16 wcaz)
{
    s32 sinax = tabsin[wcax];
    s32 sinaz = tabsin[wcaz];
    s32 cosax = tabcos[wcax];
    s32 cosaz = tabcos[wcaz];
    
    sVector result;
    if (alis.platform.is_little_endian)
    {
        // DOS: only wcy2 terms get *0x80
        result.x = cosaz * (int)wcx2 - (int)(s16)(sinaz * cosax >> 9) * (int)wcy2 * 0x80;
        result.y = (int)(s16)((int)cosaz * cosax >> 9) * (int)wcy2 * 0x80 + sinaz * (int)wcx2;
    }
    else
    {
        // m68k: everything * 0x80
        result.x = (cosaz * (int)wcx2 - (int)(s16)(sinaz * cosax >> 9) * (int)wcy2) * 0x80;
        result.y = ((int)(s16)((int)cosaz * cosax >> 9) * (int)wcy2 + sinaz * (int)wcx2) * 0x80;
    }

    result.z = ((int)wcy2 * sinax + (int)wcz2 * cosax) * 0x80;
    
    return result;
}

void landalti(s32 addr, s32 vram, s16 world_x, s16 world_y, s16 *out_alt, s16 *out_cell)
{
    // Convert world coordinates to grid coordinates
    u16 grid_x = world_x >> ((u16)xread16(addr - 0x3c0) & 0x3f);
    u16 grid_y = world_y >> ((u16)xread16(addr - 0x3c0) & 0x3f);

    // Bounds check against grid dimensions
    if ((grid_y <= (u16)xread16(addr - 0x292)) && (grid_x <= (u16)xread16(addr - 0x294)))
    {
        // Read terrain cell data
        u32 cell_addr = ((s32)(s16)grid_y + (s32)(s16)grid_y + xread32(image.atlland + (s16)(grid_x * 4)));
        *out_cell = (u16)xread16(cell_addr);
        *out_alt = *out_cell & 0xff;

        // Compute terrain type offset (6-bit type from cell, scaled)
        s16 terrain_offset = ((*out_cell & 0x3f00) >> 3) - 0xc00;
        s8 terrain_flag = (s8)xread8(addr + 0x14 + terrain_offset);
        if (terrain_flag != 0)
        {
            if (terrain_flag < 0)
            {
                // Overhang/bridge: follow redirect and test altitude
                cell_addr += xread32(addr + 0x10 + terrain_offset);
                s16 alt_diff = (xread16(cell_addr) & 0xff) - *out_alt;
                terrain_offset = xread16(addr + 0x16 + terrain_offset) - alt_diff;
                if ((alt_diff <= terrain_offset) || ((s16)((xread16(cell_addr) & 0xff) - terrain_offset) <= xread16(vram + ALIS_SCR_WCZ)))
                {
                    *out_cell = xread16(cell_addr);
                    *out_alt = *out_cell & 0xff;
                }
            }
            else if ((alis.matmask != 0) && (xread16(addr + 0x1a + terrain_offset) != 0))
            {
                // Material modifier
                if ((s8)xread8(addr + 0x14 + terrain_offset) == 1)
                {
                    if (xread32(addr + 0x10 + terrain_offset) != 0)
                    {
                        out_alt += 0xff; // NOTE: adds to pointer, not value — matches original code
                    }
                }
                else
                {
                    *out_alt += xread16(addr + 0x16 + terrain_offset);
                }
            }
        }
    }
}

// Codopname no. 249 opcode 0xf8 catstmap
// Ishar 3 Korean (IBM PC): catstmap => map_cnul
void v6_mapcell(int mode)
{
    s16 off = script_read16();
    u32 L = alis.script->vram_org;
    if (off == 0)
    {
        off = script_read16();
        L = alis.basemain;
    }

    L += off;
    if (mode < 0)
    {
        readexec_opername();
        mode = alis.varD7;
    }

    s32 d[3];
    for (int i = 0; i < 3; i++)
    {
        readexec_opername();
        d[i] = (s16)alis.varD7;
    }

    u32 vram = alis.script->vram_org;
    s32 pos[3] = { xread32(vram + 0xc), xread32(vram + 0x10), xread32(vram + 0x14) };
    alis.varD7 = r3do_cellat(vram, d[0], d[1], d[2], L, mode);
    cstore_continue();
    for (int i = 0; i < 3; i++)
        xwrite32(vram + 0xc + i * 4, pos[i]);
}

static void catstmap(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        v6_mapcell(0);
        return;
    }

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
    
    sVector vec = polarmov(wcx2, wcy2, wcz2, wcax, wcaz);
    
    alis.matmask = 0;
    
    s16 d2w;
    s16 t7 = (s16)alis.varD7;
    landalti(addr, vram, (s16)((u32)(xread32_wc(vram, ALIS_SCR_WCX) + vec.x) >> 0x10), (s16)((u32)(xread32_wc(vram, ALIS_SCR_WCY) + vec.y) >> 0x10), &d2w, &t7);
    alis.varD7 = t7;
    cstore_continue();
}

// Codopname no. 250 opcode 0xf9 cavtstmov
static void cavtstmov(void)
{
    u32 vram = alis.script->vram_org;

    s16 wcx2 = (s8)xread8(vram + ALIS_SCR_WCX2);
    s16 wcy2 = (s8)xread8(vram + ALIS_SCR_WCY2);
    s16 wcz2 = (s8)xread8(vram + ALIS_SCR_WCZ2);

    if (alis.platform.version >= 31)
    {
        s16 wcax = xread16(vram + ALIS_SCR_WCAX);
        s16 wcaz = xread16(vram + ALIS_SCR_WCAZ);
        
        sVector vec = polarmov(wcx2, wcy2, wcz2, wcax, wcaz);
        alis.wcx = (xread32_wc(vram, ALIS_SCR_WCX) + vec.x) >> 0x10;
        alis.wcy = (xread32_wc(vram, ALIS_SCR_WCY) + vec.y) >> 0x10;
        alis.wcz = (xread32_wc(vram, ALIS_SCR_WCZ) + vec.z) >> 0x10;
    }
    else
    {
        alis.wcx = xread32_wc(vram, ALIS_SCR_WCX) + wcx2;
        alis.wcy = xread32_wc(vram, ALIS_SCR_WCY) + wcy2;
        alis.wcz = xread32_wc(vram, ALIS_SCR_WCZ) + wcz2;
    }

    readexec_opername();
    alis.matmask = alis.varD7;
    alis.wforme = get_0x1a_cforme(vram);
    clipform();
    crstent();
}

// v6 dispatch target for cavtstmov
static void cavtstmov_v6(void)
{
    alis.v6tstpolar = 1;
    v6_tst_from_movevec();
    readexec_opername_saveD7(); alis.v6tstflag = (u8)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstmask = (u16)alis.varD7;
    readexec_opername_saveD7(); alis.v6tstarg = alis.varD7;
    v6_tstscan();
    alis.v6tstpolar = 0;
    crstent();
}

// Codopname no. 251 opcode 0xfa cavmov
static void cavmov(void)
{
    if (alis.platform.version >= 60)
    {
        u32 v = alis.script->vram_org;
        v6_polar(v, (s16)xread16(v + 0x26), (s16)xread16(v + 0x28), (s16)xread16(v + 0x2a));
        xwrite8(v + 0x45, 1);
        return;
    }

    if (alis.platform.version >= 31)
    {
        u32 vram = alis.script->vram_org;
        s16 wcx2 = (s8)xread8(vram + ALIS_SCR_WCX2);
        s16 wcy2 = (s8)xread8(vram + ALIS_SCR_WCY2);
        s16 wcz2 = (s8)xread8(vram + ALIS_SCR_WCZ2);
        s16 wcax = xread16(vram + ALIS_SCR_WCAX);
        s16 wcaz = xread16(vram + ALIS_SCR_WCAZ);
        
        sVector vec = polarmov(wcx2, wcy2, wcz2, wcax, wcaz);

        if (alis.platform.is_little_endian)
        {
            // DOS: add only integer part (high 16 bits of polarmov result)
            xadd16(vram + ALIS_SCR_WCX, (s16)(vec.x >> 16));
            xadd16(vram + ALIS_SCR_WCY, (s16)(vec.y >> 16));
            xadd16(vram + ALIS_SCR_WCZ, (s16)(vec.z >> 16));
        }
        else
        {
            // m68k: full 32-bit add to position
            xadd32(vram + ALIS_SCR_WCX, vec.x);
            xadd32(vram + ALIS_SCR_WCY, vec.y);
            xadd32(vram + ALIS_SCR_WCZ, vec.z);
        }
    }
}

void getangle(s16 *atx, s16 *aty)
{
    s16 angle;
    s16 magnitude = *aty;

    if (*atx == 0)
    {
        // X component is zero: angle is 0° or 180° (0xb4)
        angle = 0;
        if (*aty == 0)
        {
            magnitude = 0;
        }
        else if (*aty < 0)
        {
            angle = 0xb4;
        }
    }
    else
    {
        s16 abs_x = *atx;
        if (*atx < 0)
            abs_x = -(*atx);

        if (*aty != 0)
        {
            s16 abs_y = *aty;
            if (*aty < 0)
                abs_y = -(*aty);

            if (abs_y < abs_x)
            {
                // |x| > |y|: compute atan(|x|/|y|), angle near 90° (0x5a)
                angle = 0x5a;
                abs_x = (((s32)abs_x << 5) / abs_y);  // reused as atan ratio
                if ((u16)abs_x >= sizeof(tabatan) || (angle = tabatan[abs_x]) != 0)
                {
                    if (*aty < 0)
                        angle = 0xb4 - angle;

                    if (-1 < *atx)
                        angle = -angle;

                    // Recover magnitude via sin
                    magnitude = (*atx * 0x200) / tabsin[angle] & 0xffff;
                }
                else
                {
                    // Degenerate atan: treat as X ≈ 0
                    angle = 0;
                    if (*aty == 0)
                        magnitude = 0;
                    
                    else if (*aty < 0)
                        angle = 0xb4;
                }
            }
            else
            {
                // |y| >= |x|: compute atan(|y|/|x|), angle near 0°
                angle = 0x5a;
                abs_x = (((s32)abs_y << 5) / abs_x);  // reused as atan ratio
                if ((u16)abs_x >= sizeof(tabatan) || (angle = tabatan[abs_x]) != 0)
                {
                    if (*aty < 0)
                        angle = -angle;

                    angle -= 0x5a;
                    if (*atx < 0)
                        angle = -angle;

                    // Recover magnitude via cos
                    magnitude = (*aty * 0x200) / tabcos[angle] & 0xffff;
                }
                else
                {
                    // Degenerate atan: treat as Y ≈ 0
                    magnitude = *atx;
                    angle = *atx < 0 ? 0x5a : (s16)0xffa6;
                }
            }
        }
        else
        {
            // Y component is zero: angle is ±90° (0x5a / 0xffa6)
            magnitude = *atx;
            angle = *atx < 0 ? 0x5a : (s16)0xffa6;
        }
    }

    if (magnitude < 0)
        magnitude = -magnitude;

    *atx = angle;
    *aty = magnitude;
}

void polarang(s16 *cx, s16 *cy, s16 *cz, s16 *cur_x, s16 *cur_y, s16 *cur_z)
{
    // Compute deltas from current position to target
    *cx -= *cur_x;
    *cy -= *cur_y;
    *cz -= *cur_z;

    // Horizontal angle: getangle returns azimuth in *cx, horizontal distance in *cy
    getangle(cx, cy);

    // Vertical angle: if there is a height difference, compute elevation
    s16 elevation = 0;
    if (*cz != 0)
    {
        elevation = -(*cz);
        getangle(&elevation, cy);
    }

    // Output: *cx = azimuth, *cy = elevation, *cz = distance
    *cz = *cy;
    *cy = elevation;
}

// Codopname no. 252 opcode 0xfb caim
static void caim(void)
{
    // Read target world position
    readexec_opername();
    s16 cx = alis.varD7;
    readexec_opername();
    s16 cy = alis.varD7;
    readexec_opername();
    s16 cz = alis.varD7;

    // Read current world position
    s16 cur_x = xread16(alis.script->vram_org + ALIS_SCR_WCX);
    s16 cur_y = xread16(alis.script->vram_org + ALIS_SCR_WCY);
    s16 cur_z = xread16(alis.script->vram_org + ALIS_SCR_WCZ);

    // Compute polar angle from current position to target
    // After this: cx = target azimuth, cz = elevation (0 = same plane)
    polarang(&cx, &cy, &cz, &cur_x, &cur_y, &cur_z);

    // If target is at same altitude, keep current azimuth
    if (cz == 0)
    {
        cx = xread16(alis.script->vram_org + ALIS_SCR_WCAZ);
    }

    // Read max turn rate per step (degrees)
    readexec_opername();
    u16 max_turn = alis.varD7;

    // Clamp angular change to max_turn (0xb3 = 179°, 0x168 = 360°)
    if (0xb3 >= max_turn)
    {
        cy = xread16(alis.script->vram_org + ALIS_SCR_WCAZ);

        u16 angle_diff = cx - cy;
        if ((s16)angle_diff < 0)
        {
            if ((s16)angle_diff < -0xb3)
            {
                // Wrapped past -180°, adjust by +360°
                angle_diff += 0x168;
                if ((s16)angle_diff < 0)
                {
                    if ((s16)angle_diff <= (s16)-max_turn)
                        angle_diff = -max_turn;

                    cx = cy + angle_diff;
                }

                if ((s16)max_turn <= (s16)angle_diff)
                    angle_diff = max_turn;

                cx = cy + angle_diff;
            }
            else
            {
                // Negative turn within half-circle
                if ((s16)angle_diff <= (s16)-max_turn)
                    angle_diff = -max_turn;

                cx = cy + angle_diff;
            }
        }
        else
        {
            if (0xb3 < (s16)angle_diff)
            {
                // Wrapped past +180°, adjust by -360°
                angle_diff -= 0x168;
                if ((s16)angle_diff < 0)
                {
                    if ((s16)angle_diff <= (s16)-max_turn)
                        angle_diff = -max_turn;

                    cx = cy + angle_diff;
                }
            }
            else
            {
                // Positive turn within half-circle
                if ((s16)max_turn <= (s16)angle_diff)
                {
                    angle_diff = max_turn;
                }

                cx = cy + angle_diff;
            }
        }
    }

    // Normalize azimuth to [-359°, +359°] range
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

    // Store results: current azimuth, new azimuth, elevation
    alis.varD7 = cy;
    cstore_continue();

    alis.varD7 = cx;
    cstore_continue();

    alis.varD7 = cz;
    cstore_continue();
}

static void caim_tail(s32 dx, s32 dy, s32 dz)
{
    u32 vram = alis.script->vram_org;
    s32 b1 = dx, a1 = dy;
    v6wk_atan2(dy, dx, &b1, &a1);
    s32 bearing = -(s32)(s16)b1;
    s32 b2 = dz, a2 = a1;
    v6wk_atan2(a1, dz, &b2, &a2);
    s32 dist = a2;
    s32 rx = b2, rz = bearing;
    if (dist == 0)
    {
        rz = (s16)xread16(vram + 0x1c);
        rx = (s16)xread16(vram + 0x18);
    }

    readexec_opername_saveD7();
    s32 rate = alis.varD7 << 8;
    s32 d, m;
    d = rx - (s32)(s16)xread16(vram + 0x18);
    if (d <= -0x8000)
        d += 0x10000;
    
    if (d >= 0x8000)
        d -= 0x10000;
    
    if (d < 0)
    {
        m = -d;
        if (m >= rate)
            m = rate;
        
        d = -m;
    }
    else
    {
        if (d >= rate)
            d = rate;
    }

    u16 nx = (u16)((u16)d + (u16)xread16(vram + 0x18));
    d = rz - (s32)(s16)xread16(vram + 0x1c);
    if (d <= -0x8000)
        d += 0x10000;
    
    if (d >= 0x8000)
        d -= 0x10000;
    
    if (d < 0)
    {
        m = -d;
        if (m >= rate)
            m = rate;
        d = -m;
    }
    else
    {
        if (d >= rate)
            d = rate;
    }

    u16 nz = (u16)((u16)d + (u16)xread16(vram + 0x1c));
    alis.varD7 = ((s32)(s16)nx) >> 8;
    cstore_continue();
    alis.varD7 = ((s32)(s16)nz) >> 8;
    cstore_continue();
    alis.varD7 = dist;
    cstore_continue();
}

// v6 dispatch target for caim
static void caim_v6(void)
{
    u32 vram = alis.script->vram_org;
    readexec_opername_saveD7();
    s32 tx = alis.varD7;
    readexec_opername_saveD7();
    s32 ty = alis.varD7;
    readexec_opername_saveD7();
    s32 tz = alis.varD7;
    caim_tail(tx - xread32(vram + 0x0c), ty - xread32(vram + 0x10), tz - xread32(vram + 0x14));
}

static u32 prot_elem(const sV6Prot *p)
{
    return (u32)(p->elem + (p->aux == -1 ? 0 : p->aux));
}

static void v6_xform_point(u32 src, u32 dst, s32 v[3])
{
    sV6Ctx c;
    v6tf_build(src, dst, &c);
    v6tf_apply(v, &c, v);
}

// v6 escape 0x17: getglobal(sel, x, y, z) -> world x, y, z
void cgetglobal(void)
{
    sV6Prot p;
    getprot_x(&p);
    s32 v[3];
    readexec_opername_saveD7(); v[0] = alis.varD7;
    readexec_opername_saveD7(); v[1] = alis.varD7;
    readexec_opername_saveD7(); v[2] = alis.varD7;
    v6_xform_point(prot_elem(&p), 0, v);
    alis.varD7 = v[0]; cstore_continue();
    alis.varD7 = v[1]; cstore_continue();
    alis.varD7 = v[2]; cstore_continue();
}

// v6 escape 0x18: getlocal(sel, x, y, z, sel2): point of sel in sel2's frame
void cgetlocal(void)
{
    sV6Prot p;
    getprot_x(&p);
    s32 v[3];
    readexec_opername_saveD7(); v[0] = alis.varD7;
    readexec_opername_saveD7(); v[1] = alis.varD7;
    readexec_opername_saveD7(); v[2] = alis.varD7;
    sV6Prot p2;
    getprot_x(&p2);
    v6_xform_point(prot_elem(&p), prot_elem(&p2), v);
    alis.varD7 = v[0]; cstore_continue();
    alis.varD7 = v[1]; cstore_continue();
    alis.varD7 = v[2]; cstore_continue();
}

// v6 escape 0x23: caim at a target point from an element's world origin
void cobjaim(void)
{
    sV6Prot p;
    getprot_x(&p);
    s32 t[3];
    readexec_opername_saveD7(); t[0] = alis.varD7;
    readexec_opername_saveD7(); t[1] = alis.varD7;
    readexec_opername_saveD7(); t[2] = alis.varD7;
    s32 g[3] = { 0, 0, 0 };
    v6_xform_point(prot_elem(&p), 0, g);
    caim_tail(t[0] - g[0], t[1] - g[1], t[2] - g[2]);
}

// Codopname no. 253 opcode 0xfc cpointpix
static void cpointpix(void)
{
    readexec_opername();
    readexec_opername_saveD6();
    
    pointpix(alis.varD7, alis.varD6);
    
    alis.varD7 = image.cxtstpix;
    cstore_continue();
    alis.varD7 = image.cytstpix;
    cstore_continue();
    alis.varD7 = image.cztstpix;
    cstore_continue();
    alis.varD7 = image.dtstpix;
    cstore_continue();
    alis.varD7 = image.ntstpix;
    cstore_continue();
    alis.varD7 = image.etstpix;
    cstore_continue();
}

// 3DO: the first operand is dropped; x/y pick among the land cels
static void cpointpix_3do(void)
{
    readexec_opername_saveD7();
    readexec_opername_saveD7();
    s32 px = alis.varD7;
    readexec_opername_saveD7();
    s32 py = alis.varD7;
    u32 scr = alis.basemain + (u16)xread16(alis.script->vram_org - 0x64);
    s32 w3[3];
    u32 aux, img;
    u32 elem = r3do_pick(scr, px, py, w3, &aux, &img);
    alis.varD7 = 0;
    if (!elem)
    {
        for (int k = 0; k < 6; k++)
            cstore_continue();
        
        return;
    }

    s32 dx = w3[0] - xread32(scr + 0x0c), dy = w3[1] - xread32(scr + 0x10), dz = w3[2] - xread32(scr + 0x14);
    u32 dist = v6x_isqrt((u32)(dx * dx) + (u32)(dy * dy) + (u32)(dz * dz));
    for (int k = 0; k < 3; k++)
    {
        alis.varD7 = w3[k];
        cstore_continue();
    }

    alis.varD7 = (s32)dist;
    cstore_continue();

    s32 tag = (s16)xread16(img + 8);
    if (tag == 0)
        tag = (s32)((u32)xread16(elem + 0x28) << 16 | xread16(elem + 0x26));
    alis.varD7 = tag;
    cstore_continue();

    if (elem == scr || xread8(elem + 0x22) == 0x7f)
    {
        alis.varD7 = (aux & 0xffff) ? (s32)(aux & 0xffff) : -2;
    }
    else
    {
        alis.varD7 = (u16)xread16(elem + 0x2a);
    }

    cstore_continue();
}

// v6 dispatch target for cpointpix
static void cpointpix_v6(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        cpointpix_3do();
        return;
    }

    readexec_opername_saveD7();
    s32 px = alis.varD7;                        // pick X
    readexec_opername_saveD7();
    s32 py = alis.varD7;                        // pick Y
    readexec_opername_saveD7();
    s32 zmin = alis.varD7;                      // min depth
    u32 vram = alis.script->vram_org;
    u32 scr = alis.basemain + (u16)xread16(vram - 0x64);
    s32 w3[3] = { 0, 0, 0 };
    u32 elem = 0, tg = 0, dep = 0;
    int hit = 0;
    if (v6x3d_heapmoved == 0)                   // stale-lists gate
    {
        hit = v6x3d_pickwalk(scr, px, py, zmin, w3, &elem, &tg, &dep);
    }
    
    if (hit)
    {
        s32 dx = w3[0] - xread32(scr + 0x0c);   // camera delta
        s32 dy = w3[1] - xread32(scr + 0x10);
        s32 dz = w3[2] - xread32(scr + 0x14);
        u32 d2 = (u32)(dx * dx) + (u32)(dy * dy) + (u32)(dz * dz);  // wraps
        u32 dist = v6x_isqrt(d2);
        alis.varD7 = w3[0]; cstore_continue();  // world X
        alis.varD7 = w3[1]; cstore_continue();  // world Y
        alis.varD7 = w3[2]; cstore_continue();  // world Z
        alis.varD7 = (s32)dist; cstore_continue();
        if ((u16)tg == 0)
            tg = xread32(elem + 0x26);          // elem tag
        
        alis.varD7 = (s32)tg; cstore_continue();
        alis.varD7 = (u16)xread16(elem + 0x2a); // owner entity id
        cstore_continue();
    }
    else
    {
        alis.varD7 = 0;
        for (int k = 0; k < 6; k++)
            cstore_continue();
    }
}

// Codopname no. 254 opcode 0xfd cchartmap
// Ishar 3 Korean (IBM PC): cchartmap => map_cnul

static void cchartmap(void)
{
    u32 vram = alis.script->vram_org;
    s16 offset = script_read16();
    if (offset == 0)
    {
        vram = alis.basemain;
        offset = script_read16();
    }

    u32 addr = vram + offset;
    s32 fill_value;

    readexec_opername();
    s16 value = alis.varD7;

    if (value == 0)
    {
        fill_value = 0;
    }
    else
    {
        if (value != 1)
        {
            // Sub-command 2/3: set chart type
            if (value == 2 || value == 3)
            {
                readexec_opername();
                xwrite16(addr - 0x3b6, alis.varD7);
                return;
            }

            // Sub-command 4: set chart color base
            if (value == 4)
            {
                readexec_opername();
                xwrite16(addr - (alis.platform.is_little_endian ? 0x3ae : 0x3b4), alis.varD7);
                return;
            }

            if (value != 5)
            {
                // Sub-command >= 6: clear rectangular area in chart bitmap
                readexec_opername();
                s16 radius = (s16)alis.varD7 - 1;
                if (radius < 0)
                    radius = 0;

                // Compute clipped tile bounds from extent
                s32 entity_data = xread32(addr - 0x3ba);

                s16 pos_x = alis.wcx;
                s16 pos_y = alis.wcy;
                s16 cell_w = xread16(entity_data - 0x3f4);
                s16 cell_h = xread16(entity_data - 0x3f2);
                s16 max_cols = xread16(entity_data - 0x3e8);
                s16 max_rows = xread16(entity_data - 0x3e6);

                s32 div_result = (int)(s16)(pos_x - radius) / (int)(s16)cell_w;
                s16 min_col = (s16)div_result;
                if (div_result < 0)
                    min_col = 0;
                if (max_cols <= min_col)
                    min_col = max_cols;

                div_result = (int)(s16)(pos_x + radius) / (int)(s16)cell_w;
                s16 max_col = (s16)div_result;
                if (div_result < 0)
                    max_col = 0;
                if (max_cols <= max_col)
                    max_col = max_cols;

                div_result = (int)(s16)(pos_y - radius) / (int)(s16)cell_h;
                s16 min_row = (s16)div_result;
                if (div_result < 0)
                    min_row = 0;
                if (max_rows <= min_row)
                    min_row = max_rows;

                div_result = (int)(s16)(pos_y + radius) / (int)(s16)cell_h;
                s16 max_row = (s16)div_result;
                if (div_result < 0)
                    max_row = 0;
                if (max_rows <= max_row)
                    max_row = max_rows;

                // Shift tile bounds to bitmap coordinates
                u16 shift_x = (xread16(addr - 0x3c0) - 4) - xread16(entity_data - 0x3c0);
                u16 min_x = min_col >> (shift_x & 0x3f);
                u16 max_x = max_col >> (shift_x & 0x3f);
                u16 shift_y = xread16(addr - 0x3be) - xread16(entity_data - 0x3be);
                s16 start_row = min_row >> (shift_y & 0x3f);
                s16 row_count = (max_row >> (shift_y & 0x3f)) - start_row;

                // Compute bitmasks for left and right edges of the rectangle
                u16 left_mask = ~(0xffffU >> (min_x - (min_x & 0xfff0) & 0x3f));
                u16 aligned_max = max_x | 0xf;
                u16 right_mask = ~(-1 << (-(max_x - aligned_max) & 0x3f));
                u16 word_span = (u16)(aligned_max - (min_x & 0xfff0)) >> 4;
                s16 mid_words = word_span - 1;
                if (mid_words < 0)
                {
                    left_mask = right_mask | left_mask;
                    right_mask = 0;
                }

                if (right_mask != 0)
                {
                    mid_words = word_span - 2;
                }

                // Resolve data pointer and compute starting position
                u32 data_base = addr;
                if ((s8)xread8(addr - 1) < 0)
                    data_base = xread32(xread32(addr));

                s16 stride = xread16(addr - 0x3c4);
                u32 row_ptr = (data_base + start_row * 2 + (u32)(min_x >> 4) * (u16)stride);

                // Clear rectangle row by row using bitmasks
                do
                {
                    xwrite16(row_ptr, left_mask & xread16(row_ptr));
                    u32 word_ptr = row_ptr + stride;
                    s16 mid_count = mid_words;
                    if (-1 < mid_words)
                    {
                        do
                        {
                            xwrite16(word_ptr, 0);
                            word_ptr += stride;
                            mid_count--;
                        }
                        while (mid_count != -1);
                    }

                    if (right_mask != 0)
                        xwrite16(word_ptr, right_mask & xread16(word_ptr));

                    row_ptr += 2;
                    row_count--;
                }
                while (row_count != -1);

                return;
            }

            // Sub-command 5: set chart color count
            readexec_opername();
            xwrite16(addr - (alis.platform.is_little_endian ? 0x3b0 : 0x3b2), alis.varD7);
            return;
        }

        fill_value = -1;
    }

    // Fill entire chart bitmap with fill_value (0 or -1), then link entity
    s16 loops = ((s16)(xread16(addr - 0x3c6) * xread16(addr - 0x3c4)) >> 1);
    u32 data_ptr = addr;
    if ((s8)xread8(data_ptr - 1) < 0)
    {
        data_ptr = xread32(data_ptr);
        data_ptr = xread32(data_ptr);
    }

    for (int i = 0; i < loops; i++, data_ptr += 2)
    {
        xwrite16(data_ptr, (s16)fill_value);
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
static void cscsky(void)
{
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

// v6 dispatch target for cscsky
static void cscsky_v6(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        // 3DO: mode, sky form, angle (3DO land view draws no sky, fog fill)
        for (int i = 0; i < 3; i++)
            readexec_opername();
        
        return;
    }

    u32 scr = alis.basemain + (u16)xread16(alis.script->vram_org - 0x64);
    readexec_opername_saveD7();
    xwrite16(scr + 0x116, (u16)alis.varD7);     // sky mode
    readexec_opername_saveD7();
    s16 form = (s16)alis.varD7;
    xwrite16(scr + 0x122, (u16)form);           // sky form number
    xwrite16(scr + 0x124, xread16(alis.script->vram_org + 0x3a));
    u32 entry = (u32)adresdes(form);
    xwrite32(scr + 0x118, entry);
    readexec_opername_saveD7();
    xwrite32(scr + 0x11c, (u32)alis.varD7);     // param dword
    readexec_opername_saveD7();
    s32 scale = alis.varD7;
    u32 res = entry + xread32(entry);
    xwrite16(scr + 0x120, (u16)((s32)((s16)xread16(res + 0x10) * scale) / 0x1000));
    xwrite8(scr + 0x58, xread8(scr + 0x58) | 0x80); // redraw
}

// Codopname no. 256 opcode 0xff czoom
static void czoom(void)
{
    readexec_opername();
    xwrite16(alis.script->vram_org - 0x36, alis.varD7);

    readexec_opername();
    xwrite16(alis.script->vram_org - 0x38, alis.varD7);
}

// v6 dispatch target for czoom
static void czoom_v6(void)
{
    sV6Prot p;
    getprot_x(&p);
    if (p.aux != -1)
    {
        readexec_opername_saveD7();
        xwrite16(p.elem + p.aux + 0x1e, alis.varD7);
        return;
    }

    readexec_opername_saveD7();
    s32 slot = p.elem;
    u8 count = xread8(slot + 0x33);
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        if (count == 0)
        {
            xwrite16(slot + 0x1e, alis.varD7);
            return;
        }

        for (; count; count--, slot += V6_SLOT_STRIDE)
        {
            if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
                xwrite16(slot + 0x1e, alis.varD7);
        }
        return;
    }

    for (u8 i = 0; i < count; i++)
    {
        if (xread32(slot + 0x8) == (s32)alis.script->vram_org)
            xwrite16(slot + 0x1e, alis.varD7);
        slot += V6_SLOT_STRIDE;
    }
}

// Codopname no. 055 opcode 0x36 cclock
static void cclock(void)
{
    readexec_opername();
    set_0x3a_wait_cycles(alis.script->vram_org, alis.varD7);
    set_0x3e_wait_time(alis.script->vram_org, alis.varD7 + alis.timeclock);
}

// ============================================================================
#pragma mark - Flow control - Subroutines
// ============================================================================

// Codopname no. 018 opcode 0x11 cret
static void cret(void)
{
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
            ALIS_DEBUG(EDebugError, " VACC out of bounds!!! ");
        }

        alis.script->vacc_off += sizeof(s32);
    }
}

// v6 dispatch target for cret
static void cret_v6(void)
{
    if (alis.fseq == 0 && (s16)alis.script->vacc_off >= (s16)alis.saversp)
    {
        cstop();
        return;
    }

    s16 di = alis.script->vacc_off;
    alis.script->pc = alis.script->pc_org + xread32(alis.script->vram_org + di);
    if (di == (s16)xread16(alis.script->vram_org - 0x7c))
        xwrite16(alis.script->vram_org - 0x7c, 0);
    
    alis.script->vacc_off = di + 4;
}

static void cjsr(s32 offset)
{
    // save return **OFFSET**, not ADDRESS
    // TODO: dans la vm originale on empile la 'vraie' adresse du PC en 32 bits
    // Là on est en 64 bits, donc j'empile l'offset
    // TODO: peut-on stocker une adresse de retour *virtuelle* ?
    // Sinon ça oblige à créer une pile virtuelle d'adresses
    //   dont la taille est platform-dependent
    xpush32((u32)(alis.script->pc - alis.script->pc_org));
    alis.script->pc += offset;
}

// Codopname no. 006 opcode 0x05 cjsr8
static void cjsr8(void)
{
    // read byte, extend sign
    s16 offset = (s8)script_read8();

#ifndef NDEBUG
    if (disalis)
    {
         if (offset<0)
         {
             ALIS_DEBUG(EDebugInfo, " [call up -0x%02x]", abs(offset));
         }
         else
         {
             ALIS_DEBUG(EDebugInfo, " [call dn +0x%02x]", offset);
         }
    }
#endif
    cjsr(offset);
}

// Codopname no. 007 opcode 0x06 cjsr16
static void cjsr16(void)
{
    s16 offset = script_read16();

#ifndef NDEBUG
    if (disalis)
    {
         if (offset<0)
         {
             ALIS_DEBUG(EDebugInfo, " [call up -0x%04x]", abs(offset));
         }
         else
         {
             ALIS_DEBUG(EDebugInfo, " [call dn +0x%04x]", offset);
         }
       }
#endif
    cjsr(offset);
}

// Codopname no. 008 opcode 0x07 cjsr24
static void cjsr24(void)
{
    s32 offset = read_off24();

#ifndef NDEBUG
    if (disalis)
    {
         if (offset<0)
         {
             ALIS_DEBUG(EDebugInfo, " [call up -0x%06x]", abs(offset));
         }
         else
         {
             ALIS_DEBUG(EDebugInfo, " [call dn +0x%06x]", offset);
         }
       }
#endif
    cjsr(offset);
}


// ============================================================================
#pragma mark - Flow control - Jump
// ============================================================================

// Codopname no. 009 opcode 0x08 cjmp8
static void cjmp8(void)
{
    s16 offset = (s8)script_read8();

#ifndef NDEBUG
    if (disalis)
    {
         if (offset<0)
         {
             ALIS_DEBUG(EDebugInfo, " [jmp up -0x%02x]", abs(offset));
         }
         else
         {
             ALIS_DEBUG(EDebugInfo, " [jmp dn +0x%02x]", offset);
         }
       }
#endif
    alis.script->pc += offset;
}

// Codopname no. 010 opcode 0x09 cjmp16
static void cjmp16(void)
{
    s16 offset = script_read16();

#ifndef NDEBUG
    if (disalis)
    {
         if (offset<0)
         {
             ALIS_DEBUG(EDebugInfo, " [jmp up -0x%04x]", abs(offset));
         }
         else
         {
             ALIS_DEBUG(EDebugInfo, " [jmp dn +0x%04x]", offset);
         }
       }
#endif
    alis.script->pc += offset;
}

// Codopname no. 011 opcode 0x0a cjmp24
static void cjmp24(void)
{
    s32 offset = read_off24();

#ifndef NDEBUG
    if (disalis)
    {
         if (offset<0)
         {
             ALIS_DEBUG(EDebugInfo, " [jmp up -0x%06x]", abs(offset));
         }
         else
         {
             ALIS_DEBUG(EDebugInfo, " [jmp dn +0x%06x]", offset);
         }
       }
#endif
    alis.script->pc += offset;
}

// v6 dispatch target for cjmp24
static s16 read_off24_v6(void)
{
    if (alis.platform.is_little_endian)
    {
        s16 offset = script_read16();
        script_read8();
        return offset;
    }

    script_read8();
    return script_read16();
}

static void cjmp24_v6(void)
{
    s16 offset = read_off24_v6();
    if (disalis)
        ALIS_DEBUG(EDebugInfo, offset < 0 ? " [jmp up -0x%04x]" : " [jmp dn +0x%04x]", abs(offset));
    
    alis.script->pc += offset;
}


// ============================================================================
#pragma mark - Flow control - Branch if zero
// ============================================================================

// value tested by cbz/cbnz
static s32 bztest(void)
{
    return alis.platform.uid == EGameRobinsonsRequiem3DO ? alis.varD7 : (s16)alis.varD7;
}

// Codopname no. 019 opcode 0x12 cbz8
static void cbz8(void)
{
    s32 d7 = bztest();
    s16 offset = d7 ? 1 : (s8)script_read8();

#ifndef NDEBUG
    if (disalis)
    {
        if (d7 != 0)
        {
            ALIS_DEBUG(EDebugInfo, " [d7<>0 => no jmp +1]");
        }
        else
        {
            if (offset<0)
            {
                ALIS_DEBUG(EDebugInfo, " [d7==0 => jmp up -0x%02x]", abs(offset));
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, " [d7==0 => jmp dn +0x%02x]", offset);
            }
        }
    }
#endif
    alis.script->pc += offset;
}

// Codopname no. 020 opcode 0x13 cbz16
static void cbz16(void)
{
    s32 d7 = bztest();
    s16 offset = d7 ? 2 : script_read16();

#ifndef NDEBUG
    if (disalis)
    {
        if (d7 != 0)
        {
            ALIS_DEBUG(EDebugInfo, " [d7<>0 => no jmp +2]");
        }
        else
        {
            if (offset<0)
            {
                ALIS_DEBUG(EDebugInfo, " [d7==0 => jmp up -0x%04x]", abs(offset));
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, " [d7==0 => jmp dn +0x%04x]", offset);
            }
        }
    }
#endif
    alis.script->pc += offset;
}

// Codopname no. 021 opcode 0x14 cbz24
static void cbz24(void)
{
    s32 d7 = bztest();
    s32 offset = d7 ? 3 : read_off24();

#ifndef NDEBUG
    if (disalis)
    {
        if (d7 != 0)
        {
            ALIS_DEBUG(EDebugInfo, " [d7<>0 => no jmp +3]");
        }
        else
        {
            if (offset<0)
            {
                ALIS_DEBUG(EDebugInfo, " [d7==0 => jmp up -0x%06x]", abs(offset));
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, " [d7==0 => jmp dn +0x%06x]", offset);
            }
        }
    }
#endif
    alis.script->pc += offset;
}

// v6 dispatch target for cbz24
static void cbz24_v6(void)
{
    s32 d7 = bztest();
    if (d7 == 0)
    {
        alis.script->pc += read_off24_v6();
    }
    else
    {
        alis.script->pc += 3;
    }
}


// ============================================================================
#pragma mark - Flow control - Branch if non-zero
// ============================================================================

// Codopname no. 022 opcode 0x15 cbnz8
static void cbnz8(void)
{
    s32 d7 = bztest();
    s16 offset = d7 == 0 ? 1 : (s8)script_read8();

#ifndef NDEBUG
    if (disalis)
    {
        if (d7 == 0)
        {
            ALIS_DEBUG(EDebugInfo, " [d7==0 => no jmp +1]");
        }
        else
        {
            if (offset<0)
            {
                ALIS_DEBUG(EDebugInfo, " [d7<>0 => jmp up -0x%02x]", abs(offset));
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, " [d7<>0 => jmp dn +0x%02x]", offset);
            }
        }
    }
#endif
    alis.script->pc += offset;
}

// Codopname no. 023 opcode 0x16 cbnz16
static void cbnz16(void)
{
    s32 d7 = bztest();
    s16 offset = d7 == 0 ? 2 : script_read16();

#ifndef NDEBUG
    if (disalis)
    {
        if (d7 == 0)
        {
            ALIS_DEBUG(EDebugInfo, " [d7==0 => no jmp +2]");
        }
        else
        {
            if (offset<0)
            {
                ALIS_DEBUG(EDebugInfo, " [d7<>0 => jmp up -0x%04x]", abs(offset));
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, " [d7<>0 => jmp dn +0x%04x]", offset);
            }
        }
    }
#endif
    alis.script->pc += offset;
}

// Codopname no. 024 opcode 0x17 cbnz24
static void cbnz24(void)
{
    s32 d7 = bztest();
    s32 offset = d7 == 0 ? 3 : read_off24();

#ifndef NDEBUG
    if (disalis)
    {
        if (d7 == 0)
        {
            ALIS_DEBUG(EDebugInfo, " [d7==0 => no jmp +3]");
        }
        else
        {
            if (offset<0)
            {
                ALIS_DEBUG(EDebugInfo, " [d7<>0 => jmp up -0x%06x]", abs(offset));
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, " [d7<>0 => jmp dn +0x%06x]", offset);
            }
        }
    }
#endif
    alis.script->pc += offset;
}

// v6 dispatch target for cbnz24
static void cbnz24_v6(void)
{
    s32 d7 = bztest();
    if (d7 != 0)
    {
        alis.script->pc += read_off24_v6();
    }
    else
    {
        alis.script->pc += 3;
    }
}


// ============================================================================
#pragma mark - Flow control - Branch if equal
// ============================================================================

static int beqtest(void)
{
    return (alis.platform.uid == EGameRobinsonsRequiem3DO) ? alis.varD7 == alis.varD6 : (s16)alis.varD7 == (s16)alis.varD6;
}

static void cbcond(int jump, int bits)
{
    s32 offset;
    if (bits == 8)
    {
        offset = (s8)script_read8();
    }
    else if (bits == 16)
    {
        offset = (s16)script_read16();
    }
    else
    {
        offset = read_off24();
    }

    if (jump)
        alis.script->pc += offset;
    
#ifndef NDEBUG
    if (disalis)
        ALIS_DEBUG(EDebugInfo, jump ? " [jmp %+d]" : " [no jmp]", offset);
#endif
}

// Codopname no. 025 opcode 0x18 cbeq8
static void cbeq8(void) { cbcond(beqtest(), 8); }

// Codopname no. 026 opcode 0x19 cbeq16
static void cbeq16(void) { cbcond(beqtest(), 16); }

// Codopname no. 027 opcode 0x1a cbeq24
static void cbeq24(void) { cbcond(beqtest(), 24); }

// Codopname no. 028 opcode 0x1b cbne8
static void cbne8(void) { cbcond(!beqtest(), 8); }

// Codopname no. 029 opcode 0x1c cbne16
static void cbne16(void) { cbcond(!beqtest(), 16); }

// Codopname no. 030 opcode 0x1d cbne24
static void cbne24(void) { cbcond(!beqtest(), 24); }


// ============================================================================
#pragma mark - Flow control - Start
// ============================================================================
static void cstart(s32 offset)
{
    if (alis.platform.version >= 60)
    {
        u32 vram = alis.script->vram_org;
        s16 marker = (s16)xread16(vram - 0x7c);
        if (alis.fseq == 0)
        {
            if (marker == 0)
            {
                marker = (s16)alis.saversp;
                xwrite16(vram - 0x7c, marker);
                xwrite32(vram - 0x80, get_0x08_script_ret_offset(vram) - alis.script->pc_org);
            }

            s16 slot = marker - 4;
            alis.saversp = (u16)slot;
            xwrite32(vram + slot, xread32(vram - 0x80));
            set_0x08_script_ret_offset(vram, alis.script->pc + offset);
            set_0x04_cstart_csleep(vram, 1);
            set_0x3e_wait_time(vram, 0);
        }
        else
        {
            if (marker == 0)
            {
                xwrite32(vram - 0x80, alis.script->pc - alis.script->pc_org);
                marker = alis.script->vacc_off;
                xwrite16(vram - 0x7c, marker);
            }

            s16 slot = marker - 4;
            alis.script->vacc_off = slot;
            xwrite32(vram + slot, xread32(vram - 0x80));
            alis.script->pc += offset;
        }

        return;
    }

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
        
        if (alis.platform.version != 10)
        {
            set_0x04_cstart_csleep(alis.script->vram_org, 1);
            set_0x01_wait_count(alis.script->vram_org, 1);
        }
        
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
static void cstart8(void)
{
    // read byte, extend sign to word, then to long
    cstart((s8)script_read8());
}

// Codopname no. 050 opcode 0x31 cstart16
static void cstart16(void)
{
    cstart((s16)script_read16());
}

// Codopname no. 051 opcode 0x32 cstart24
static void cstart24(void)
{
    cstart(read_off24());
}

// ============================================================================
#pragma mark - Additions
// ============================================================================

static void v6_relocate_vram(u32 blockstart, u32 shrinkby)
{
    for (u32 w = defworld_list_head; w != 0; w = xread32(w + 0xb4))
    {
        u32 node = w;
        for (;;)
        {
            u32 next = xread32(node + 0x38);
            if (next >= blockstart)
                xwrite32(node + 0x38, next - shrinkby);
            
            u32 prev = xread32(node + 0x34);
            if (prev >= blockstart)
                xwrite32(node + 0x34, prev - shrinkby);
            
            u32 par = xread32(node + 0x8);
            if (par >= blockstart)
                xwrite32(node + 0x8, par - shrinkby);
            
            if (node == next)
                break;
            
            node = next;
        }
    }

    for (int i = 0; i < MAX_SCRIPTS; i++)
    {
        sAlisScriptLive *s = alis.live_scripts[i];
        if (s == NULL || s->vram_org < blockstart)
            continue;
        
        u32 node = v6_elemhead(s->vram_org);
        for (;;)
        {
            u32 next = v6el_next(node);
            if (next >= blockstart)
                v6el_set_next(node, next - shrinkby);
            
            u32 prev = v6el_prev(node);
            if (prev >= blockstart)
                v6el_set_prev(node, prev - shrinkby);
            
            if (node == next)
                break;
            
            node = next;
        }
    }
}

void killent(u16 killent)
{
    s32 vram = xread32(alis.atent + killent);
    if (vram == 0)
    {
        if (alis.platform.version < 60)
            alis.ferase = 0;

        return;
    }

    // make sure music playback is stopped (a dormant v6 entity has no program)
    sAlisScriptLive *script = ENTSCR(killent);
    if (script->data && script->data->header.id == audio.musicId)
    {
        sys_sleep_until_music_stops();
    }
    
    if (alis.platform.version >= 60)
    {
        killallelem_v6((u32)vram);
    }
    else
    {
        ALIS_SET_SCRIPT(ENTSCR(killent));
        cerasall();
        ALIS_SET_SCRIPT(ENTSCR(alis.varD5));
    }

    s32 shrinkby = xread32(vram - 0x14);
    s32 contextsize = get_context_size();
    u32 datasize;
    u32 target;
    if (alis.platform.version >= 60)
    {
        datasize = xread16(alis.atent + killent + 6);
        target = vram - datasize;
        shrinkby = xread32(alis.atent + killent + 8);
    }
    else
    {
        datasize = xread16(shrinkby + 0x16) + xread16(shrinkby + 0x12) + contextsize;
        target = vram - datasize;
        shrinkby = xread16(shrinkby + 0x14) + datasize;
    }

    u32 source = target + shrinkby;

    // rebase every v6 slot/elem pointer that lives above the freed block before copy
    if (alis.platform.version >= 60)
        v6_relocate_vram(target, (u32)shrinkby);

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
    xwrite32(alis.atent + killent, 0);
    if (alis.platform.version >= 60)
    {
        node_free(killent);
        alis.live_scripts[killent / ENT_STRIDE] = NULL;
    }
    else
    {
        xwrite16(alis.atent + killent + 4, alis.dernent);
        alis.dernent = killent;
    }

    alis.nbent --;

    // if we deleted currently running script, set previous script and restart loop
    if (alis.varD5 == killent)
    {
        alis.varD5 = curent;
        alis.restart_loop = 1;
        cstop();
    }
}

static void v6_relocate_prog(u32 start, u32 length, u32 fin)
{
    u32 end = start + length;
    for (u32 w = defworld_list_head; w != 0; w = xread32(w + 0xb4))
    {
        for (u32 e = xread32(w + 0x38); e != xread32(e + 0x38); e = xread32(e + 0x38))
        {
            if ((xread8(e + 0x24) & 0x40) || v6sp_state(e) == '8')
                continue;
            u32 p = xread32(e + 0x50);
            if ((v6sp_state(e) == '0' || v6sp_state(e) == ':') && p >= end && p < fin)
                xwrite32(e + 0x50, p - length);
            p = xread32(e + 0x3c);
            if (p >= end && p < fin)
                xwrite32(e + 0x3c, p - length);
        }
    }

    for (int i = 0; i < 16; i++)
    {
        u32 g = alis.v6xfonts[i].glyphslot;
        if (g >= end && g < fin)
        {
            alis.v6xfonts[i].glyphslot = g - length;
        }
        else if (g >= start && g < end)
        {
            alis.v6xfonts[i].glyphslot = 0;
        }
    }
}

static void v6_dormant(u16 ent, u32 org, u32 end)
{
    sAlisScriptLive *s = ENTSCR(ent);
    u32 vram = s->vram_org;
    u8 fl = xread8(V6_FARSLEEP(vram));
    if (fl & V6_DORMANT)
        return;
    
    xwrite8(V6_FARSLEEP(vram), fl | V6_DORMANT);
    for (u32 e = v6el_next(v6_elemhead(vram)); e != v6el_next(e); e = v6el_next(e))
    {
        u32 sp = e;
        for (u8 i = xread8(e + 0x33); i != 0; i--, sp += V6_SLOT_STRIDE)
        {
            u32 p = xread32(sp + 0x3c);
            if (p >= org && p < end)
                xwrite8(sp + 0x24, xread8(sp + 0x24) | 0x40);
        }
    }

    s->data = NULL;
}

static void v6_wake(u16 ent, sAlisScriptData *prog)
{
    sAlisScriptLive *s = ENTSCR(ent);
    u32 vram = s->vram_org;
    u8 fl = xread8(V6_FARSLEEP(vram));
    if (!(fl & V6_DORMANT))
        return;
    
    xwrite8(V6_FARSLEEP(vram), fl & ~V6_DORMANT);
    s32 d = (s32)(prog->data_org - get_0x14_script_org_offset(vram));
    set_0x14_script_org_offset(vram, prog->data_org);
    set_0x08_script_ret_offset(vram, get_0x08_script_ret_offset(vram) + d);
    s->pc += d;
    s->pc_org += d;
    s->data = prog;
    s->name = prog->name;
    for (u32 e = v6el_next(v6_elemhead(vram)); e != v6el_next(e); e = v6el_next(e))
    {
        u32 sp = e;
        for (u8 i = xread8(e + 0x33); i != 0; i--, sp += V6_SLOT_STRIDE)
        {
            if (!(xread8(sp + 0x24) & 0x40))
                continue;
            
            if (xread32(sp + 0x3c) != 0)
                xwrite32(sp + 0x3c, xread32(sp + 0x3c) + d);
            
            if (v6sp_state(sp) == '0' || v6sp_state(sp) == ':')
                xwrite32(sp + 0x50, xread32(sp + 0x50) + d);
            
            xwrite8(sp + 0x24, xread8(sp + 0x24) & 0xbf);
        }
    }
}

void v6_wakeall(u16 n)
{
    sAlisScriptData *prog = node_prog(n);
    if (prog == NULL || alis.platform.kind == EPlatform3DO)
        return;

    for (u16 e = xread16(alis.atent + 4); e != 0; e = xread16(alis.atent + e + 4))
    {
        if (memcmp(alis.mem + xread32(alis.atent + e) + 0x3c, alis.mem + alis.atent + n + 6, 8) == 0)
            v6_wake(e, prog);
    }
}

void v6_unloadprog(const char *name, s32 mode)
{
    if ((mode & 0x100) && alis.platform.kind != EPlatform3DO)
    {
        for (u16 n = xread16(alis.atent + NODE_BLKHEAD + 0xe); n != NODE_BLKTAIL; n = xread16(alis.atent + n + 0xe))
        {
            u8 t = xread8(alis.atent + n + 0x15);
            if (t != 2 && t != 0xc && t != 0xd)
                continue;
            
            char nm[9] = {0};
            memcpy(nm, alis.mem + alis.atent + n + 6, 8);
            if (memcmp(nm, "main\0\0\0\0", 8) == 0)
                continue;
            
            v6_unloadprog(nm, (u8)mode);
            if (alis.restart_loop)
                return;
        }

        return;
    }

    u8 m = (u8)mode;
    u16 n = node_find(name);
    if (n == 0)
        return;
    
    if (alis.platform.kind != EPlatform3DO)
        v6x3d_heapmoved = 1;

    sAlisScriptData *prog = node_prog(n);
    u32 org = prog ? prog->data_org : 0;
    u32 end = alis.finprog;
    for (int i = 0; prog && i < alis.nbprog; i++)
    {
        if (ATPROG(i) > org && ATPROG(i) < end)
            end = ATPROG(i);
    }
    
    for (u16 e = xread16(alis.atent + 4); e != 0; e = xread16(alis.atent + e + 4))
    {
        if (memcmp(alis.mem + xread32(alis.atent + e) + 0x3c, alis.mem + alis.atent + n + 6, 8) != 0)
            continue;
        
        alis.ferase = 0;
        if (m == 2)
        {
            if (prog)
                v6_dormant(e, org, end);
            continue;
        }

        if (m != 1)
            alis.ferase = 1;
        
        killent(e);
        if (alis.restart_loop)
            return;
    }

    if (prog)
    {
        // NOTE: interuptable delay to make everything work more like on original platforms
        sys_sleep_interactive(&alis.unload_delay, alis.unload_delay - 100);
        ALIS_DEBUG(EDebugVerbose, "\n (UNLOAD: %s at 0x%x, 0x%x bytes) \n", prog->name, org, end - org);
        shrinkprog(org, end - org, prog->header.id);
    }

    extern void v6text_free(u16 n);
    v6text_free(n);
    node_free(n);
}

void shrinkprog(s32 start, s32 length, u16 id)
{
    s32 oldfin = alis.finprog;
    if (alis.platform.version >= 60)
        v6_relocate_prog(start, length, oldfin);

    // Shift [start+length, finprog) down by `length` to close the gap.
    if (alis.finprog - length > start)
        memmove(alis.mem + start, alis.mem + start + length, (size_t)(alis.finprog - length - start));

    ALIS_DEBUG(EDebugInfo, "\nFreeing range %.6x - %.6x", start, start + length);

    alis.finprog -= length;

    // Address-keyed caches and the cursor pointer must follow the move.
#if defined(ALIS_NATIVE_PLANAR)
    planar_tab_flush();
#endif
#if defined(ALIS_NATIVE_16BPP)
    sprite_cache_flush();
#endif
#if defined(ALIS_RRQ_ASM_ZOOM) && ALIS_RRQ_ASM_ZOOM && !defined(ALIS_NO_ZOOM_TRIM)
    zoom_rows_flush();
#endif
    if (alis.desmouse)
    {
        u8 *lo = alis.mem + start, *hi = lo + length;
        if (alis.desmouse >= hi)
        {
            alis.desmouse -= length;
        }
        else if (alis.desmouse >= lo)
        {
            alis.desmouse = NULL;
        }
    }

    audio_relocate(start, start + length, -length);
    
    if (id != 0)
    {
        s8 idx = -1;
        for (s32 i = 0; i < alis.nbprog; i++)
        {
            sAlisScriptData *scr = alis.loaded_scripts[i];
            if (alis.platform.version >= 60 ? scr->data_org == (u32)start : scr->header.id == id)
            {
                idx = i;
                ALIS_DEBUG(EDebugInfo, "\nRemoved prog: %s at: %.6x ", scr->name, scr->data_org);
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, "\nSkipped prog: %s at: %.6x ", scr->name, scr->data_org);
            }

            if (idx >= 0)
            {
                alis.loaded_scripts[i] = i < alis.maxprog ? alis.loaded_scripts[i + 1] : 0;
            }
        }

        u8 found = 0;
        for (s32 i = 0; i < alis.nbprog; i++)
        {
            s32 location = ATPROG(i);
            if (start == location)
            {
                found = 1;
            }
            
            if (found)
            {
                ATPROG_SET(i, i < alis.maxprog ? ATPROG(i + 1) : 0);
            }
        }

        alis.dernprog -= 4;
        alis.nbprog --;
    }

    ALIS_DEBUG(EDebugInfo, "\nShifting range %.6x - %.6x", alis.atprog, alis.dernprog);

    for (int i = 0; i < alis.nbprog; i++)
    {
        sAlisScriptData *script = alis.loaded_scripts[i];
        ALIS_DEBUG(EDebugInfo, "\nChecking prog: %s at: %.6x ", script->name, script->data_org);
            
        if (start <= script->data_org)
        {
            ALIS_DEBUG(EDebugInfo, "pre-shrinked: %.6x", script->data_org);

            script->data_org -= length;
            ATPROG_SET(i, ATPROG(i) - length);
            ALIS_DEBUG(EDebugInfo, "shrinked to: %.6x", script->data_org);
        }
        else
        {
            ALIS_DEBUG(EDebugInfo, "OK");
        }
    }

    ALIS_DEBUG(EDebugInfo, "\n");

    if (id != 0)
    {
        s32 contextsize = get_context_size();

        u32 entidx = 0;
        u32 prevent = 0;
        
        while ((entidx = xread16(alis.atent + 4 + entidx)))
        {
            sAlisScriptLive *script = ENTSCR(entidx);
            ALIS_DEBUG(EDebugInfo, "\nChecking script: %s ", script->name);

            if (alis.platform.version >= 60)
            {
                if (!(xread8(V6_FARSLEEP(script->vram_org)) & V6_DORMANT) && start <= get_0x14_script_org_offset(script->vram_org))
                {
                    script->pc -= length;
                    script->pc_org -= length;
                    set_0x14_script_org_offset(script->vram_org, get_0x14_script_org_offset(script->vram_org) - length);
                    set_0x08_script_ret_offset(script->vram_org, get_0x08_script_ret_offset(script->vram_org) - length);
                }
            }
            else if (id == get_0x10_script_id(script->vram_org))
            {
                killent(entidx);
                entidx = prevent; //alis.varD5;
                ALIS_DEBUG(EDebugInfo, "removed");
            }
            else if (start <= get_0x14_script_org_offset(script->vram_org))
            {
                script->pc -= length;
                script->pc_org -= length;
                set_0x14_script_org_offset(script->vram_org, get_0x14_script_org_offset(script->vram_org) - length);
                set_0x08_script_ret_offset(script->vram_org, get_0x08_script_ret_offset(script->vram_org) - length);
                
                u32 org_offset = get_0x14_script_org_offset(script->vram_org);
                script->vacc_off = -contextsize - xread16(org_offset + 0x16);
                ALIS_DEBUG(EDebugInfo, " [va %.4x]", (s16)alis.script->vacc_off);
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, "OK");
            }
            
            prevent = entidx;
        }
    }
    
    ALIS_DEBUG(EDebugInfo, "\n");

    // move addresses to sprite data to match actual new locations (pre-v6 screens)
    for (s16 scidx = alis.platform.version >= 60 ? 0 : screen.ptscreen; scidx != 0; scidx = get_scr_to_next(scidx))
    {
        u16 spridx = get_scr_screen_id(scidx);
        if (spridx != 0)
        {
            while ((spridx = SPRITE_VAR(spridx)->link) != 0)
            {
                sSprite *sprite = SPRITE_VAR(spridx);
                // pending removal or pointing into the freed script: drop instead of shifting into its neighbour
                if (sprite->state == 1 || (sprite->newad >= start && sprite->newad < start + length))
                {
                    sprite->newad = 0;
                }
                else if (start < sprite->newad)
                {
                    sprite->newad -= length;
                }

                if (sprite->data >= start && sprite->data < start + length)
                {
                    sprite->data = 0;
                }
                else if (start < sprite->data)
                {
                    sprite->data -= length;
                }
            }
        }
    }
}

// Mirror of shrinkprog (id==0 path): open `length` bytes at `start` by shifting [start, finprog) up.
// Returns 0 if it would overflow VM RAM.

int growprog(s32 start, s32 length)
{
    if ((u32)(alis.finprog + length) > alis.finmem)
        return 0;

    // memmove handles the overlap (dst > src).
    memmove(alis.mem + start + length, alis.mem + start, (size_t)(alis.finprog - start));

    alis.finprog += length;
    audio_relocate(start, start, length);

    for (int i = 0; i < alis.nbprog; i++)
    {
        if (start <= (s32)alis.loaded_scripts[i]->data_org)
        {
            alis.loaded_scripts[i]->data_org += length;
            ATPROG_SET(i, ATPROG(i) + length);
        }
    }

    for (s16 scidx = alis.platform.version >= 60 ? 0 : screen.ptscreen; scidx != 0; scidx = get_scr_to_next(scidx))
    {
        u16 spridx = get_scr_screen_id(scidx);
        if (spridx != 0)
        {
            while ((spridx = SPRITE_VAR(spridx)->link) != 0)
            {
                sSprite *sprite = SPRITE_VAR(spridx);
                if (start < sprite->newad)
                    sprite->newad += length;
                
                if (start < sprite->data)
                    sprite->data  += length;
            }
        }
    }

    return 1;
}

// ----------------------------------------------------------------------------
// Load-time, IN PLACE per-format converters to 0x14 (transparent) / 0x16 (opaque), named after
// destofen's draw_*() handlers. Each writes pixels at +8 (8-bit chunky, or planar under
// ALIS_PLANAR_CONV) and returns the bytes added (0 = skip); the caller applies the table fixups.
// ----------------------------------------------------------------------------

// DOS scripts with no 3D-sampled resources; whitelist so unknown scripts stay chunky.
static const char *const k_2d_only_scripts[] = {
    "authors", "baratin", "baratind", "baratine", "baratinu", "body", "caporal", "flags",
    "lamort", "logo", "main", "map", "newpref", "newprefd", "newprefs", "newprefu", "objet",
    "ordi", "prefs", "prefsd", "prefsu", "present", "pupuce", "setup", "souris", "srain",
    "time", "trouble", "yoda",
};

// Per-script gate for EVERY format (conv_finish retypes converted sprites to 0x14/0x16, so test the
// plane stamp, never the format byte). Read by convert_one and sprite_measure, which must agree.
static int s_conv_planar_ok = 0;

static int conv_planar_allowed_for(const char *name)
{
    // v6 composes chunky, see v6_present
    if (alis.platform.version >= 60)
        return 0;

    if (!alis.platform.is_little_endian)
        return 1;                       // BE data: 3D resources carry their own formats, convert all

    if (!name)
        return 0;

    // Compare the basename without extension, case-insensitively ("MAIN.IO", "objet.io", ...).
    for (unsigned i = 0; i < sizeof(k_2d_only_scripts) / sizeof(k_2d_only_scripts[0]); i++)
    {
        const char *a = name, *b = k_2d_only_scripts[i];
        while (*b && *a && *a != '.')
        {
            int ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
            if (ca != *b)
                break;
            
            a++; b++;
        }

        if (*b == 0 && (*a == 0 || *a == '.'))
            return 1;
    }

    return 0;
}

// Callers are all inside ALIS_PLANAR_CONV blocks.
static inline int __attribute__((unused)) conv_planar_enabled(void) { return s_conv_planar_ok; }

// Planar iff byte 6 (padding for 0x14/0x16, never read by the original blitters) holds the plane
// count stamped by conv_finish; deplanar_8bit_inplace clears it.
int sprite_is_planar(const u8 *bitmap)
{
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    if (!bitmap)
        return 0;
    
    if (bitmap[0] != 0x14 && bitmap[0] != 0x16)
        return 0;
    
    return bitmap[6] == 4 || bitmap[6] == 8;
#else
    (void)bitmap;
    return 0;
#endif
}

#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
// Kalms 16x8 chunky->planar transpose (SDL Atari C2pConvert8): 16 pixel bytes -> 8 BE plane words,
// out[0..7] = plane0..7, MSB = pixel 0.
static inline void kalms16_8(const u8 *in, u16 *out)
{
#if defined(__m68k__)
    // The 16 pixel bytes are four BE longs; may_alias deref forces move.l (memcpy does not fold here).
    typedef u32 __attribute__((__may_alias__)) u32a;
    u32 d0 = *(const u32a *)(in);
    u32 d1 = *(const u32a *)(in + 4);
    u32 d2 = *(const u32a *)(in + 8);
    u32 d3 = *(const u32a *)(in + 12);
#else
    u32 d0=((u32)in[0]<<24)|((u32)in[1]<<16)|((u32)in[2]<<8)|in[3];
    u32 d1=((u32)in[4]<<24)|((u32)in[5]<<16)|((u32)in[6]<<8)|in[7];
    u32 d2=((u32)in[8]<<24)|((u32)in[9]<<16)|((u32)in[10]<<8)|in[11];
    u32 d3=((u32)in[12]<<24)|((u32)in[13]<<16)|((u32)in[14]<<8)|in[15];
#endif
    u32 d7;
    d7=((d1>>4)^d0)&0x0f0f0f0fu; d0^=d7; d1^=(d7<<4);
    d7=((d3>>4)^d2)&0x0f0f0f0fu; d2^=d7; d3^=(d7<<4);
    d7=((d2>>8)^d0)&0x00ff00ffu; d0^=d7; d2^=(d7<<8);
    d7=((d3>>8)^d1)&0x00ff00ffu; d1^=d7; d3^=(d7<<8);
    d7=((d2>>1)^d0)&0x55555555u; d0^=d7; d2^=(d7<<1);
    d7=((d3>>1)^d1)&0x55555555u; d1^=d7; d3^=(d7<<1);
    { u32 s2=(d2<<16)|(d2>>16), s0=(d0<<16)|(d0>>16);   // exchange low words d0<->d2 (swap idiom)
      u32 n0=(d0&0xffff0000u)|(s2&0xffffu), n2=(s0&0xffff0000u)|(d2&0xffffu); d0=n0; d2=n2; }
    { u32 s3=(d3<<16)|(d3>>16), s1=(d1<<16)|(d1>>16);   // exchange low words d1<->d3
      u32 n1=(d1&0xffff0000u)|(s3&0xffffu), n3=(s1&0xffff0000u)|(d3&0xffffu); d1=n1; d3=n3; }
    d7=((d2>>2)^d0)&0x33333333u; d0^=d7; d2^=(d7<<2);
    d7=((d3>>2)^d1)&0x33333333u; d1^=d7; d3^=(d7<<2);
    // Here d0=plane7:plane6, d1=plane3:plane2, d2=plane5:plane4, d3=plane1:plane0 (high:low). The
    // asm's final 4 swaps just reorder to plane6:plane7 etc.; fold that straight into extraction.
    out[0]=(u16)d3; out[1]=(u16)(d3>>16); out[2]=(u16)d1; out[3]=(u16)(d1>>16);
    out[4]=(u16)d2; out[5]=(u16)(d2>>16); out[6]=(u16)d0; out[7]=(u16)(d0>>16);
}

// 8-bit chunky indices -> 8-plane interleaved (one 16px chunk = 8 BE plane words = 16 bytes) via the
// Kalms transpose. Aligned full chunks feed kalms16_8 the 16 source bytes directly; partial/flipped
// chunks gather into a 16-byte scratch (0-padded on the right so short chunks left-align correctly).
void chunky8_to_planar(const u8 *idx8, s32 w, s32 h, int flip, u8 *dst)
{
    s32 cpr = (w + 15) >> 4;
    for (s32 y = 0; y < h; y++)
    {
        const u8 *row = idx8 + (s32)y * w;
        u16 *pw = (u16 *)(dst + (s32)y * cpr * 16);
        for (s32 cc = 0; cc < cpr; cc++, pw += 8)
        {
            s32 base = cc * 16;
            s32 n = w - base; if (n > 16) n = 16;     // valid columns in this chunk
            if (n == 16 && !flip)
            {
                kalms16_8(row + base, pw);            // fast path: 16 source bytes straight in
            }
            else
            {
                u8 px[16];
                if (!flip)
                {
                    s32 i = 0;
                    for (; i < n; i++)
                        px[i] = row[base + i];

                    for (; i < 16; i++)
                        px[i] = 0;
                }
                else
                {
                    s32 i = 0;
                    const u8 *r = row + (w - 1 - base);
                    for (; i < n; i++)
                        px[i] = r[-i];

                    for (; i < 16; i++)
                        px[i] = 0;
                }

                kalms16_8(px, pw);
            }
        }
    }
}

// Undo the load-time 8-plane conversion in place (the 3D renderer samples chunky). 4-plane can't:
// it is half the chunky size and compaction reclaimed the rest (see planar_tab_get_chunky).
// Clears the plane stamp, so repeat calls on shared resources are no-ops.
int deplanar_8bit_inplace(u32 bitmap_addr)
{
    u8 fmt = alis.mem[bitmap_addr];
    if (fmt != 0x14 && fmt != 0x16)
        return 0;                                  // not an 8-bit resource
    
    // Runtime call: check the stamp, not conv_planar_enabled() (valid only while converting).
    const int np = 8;
    if (alis.mem[bitmap_addr + 6] != np)
        return 0;

    s32 w = xread16(bitmap_addr + 2) + 1;
    s32 h = xread16(bitmap_addr + 4) + 1;
    if (w <= 0 || h <= 0)
        return 0;
    
    s32 cpr = (w + 15) >> 4;
    if (cpr * 16 > 1024)
        return 0;                                  // wider than the row scratch — leave it alone

    static u8 rowbuf[1024];
    for (s32 y = 0; y < h; y++)
    {
        // Host-native u16 reads mirror chunky8_to_planar's stores (not xread16).
        const u16 *pw = (const u16 *)(alis.mem + bitmap_addr + 8 + (s32)y * cpr * np * 2);
        u8 *out = rowbuf;
        for (s32 cc = 0; cc < cpr; cc++, pw += np)
        {
            for (int b = 15; b >= 0; b--)  // MSB = leftmost pixel
            {
                u8 v = 0;
                for (int k = 0; k < np; k++)
                    v |= (u8)(((pw[k] >> b) & 1) << k);
                
                *out++ = v;
            }
        }

        memcpy(alis.mem + bitmap_addr + 8 + (s32)y * w, rowbuf, (size_t)w);
    }

    alis.mem[bitmap_addr + 6] = 0;                 // chunky again
    return 1;
}

// 4-bit chunky indices -> 4-plane interleaved (one 16px chunk = 4 BE plane words = 8 bytes).
void chunky4_to_planar(const u8 *idx8, s32 w, s32 h, int flip, u8 *dst)
{
    s32 cpr = (w + 15) >> 4;
    for (s32 y = 0; y < h; y++)
    {
        const u8 *row = idx8 + (s32)y * w;
        u16 *pw = (u16 *)(dst + (s32)y * cpr * 8);
        for (s32 cc = 0; cc < cpr; cc++, pw += 4)
        {
            s32 base = cc * 16;
            s32 n = w - base; if (n > 16) n = 16;     // valid columns in this chunk
            u16 p0=0,p1=0,p2=0,p3=0;
            if (!flip)
            {
                const u8 *r = row + base;
                for (s32 i = 0; i < n; i++)
                {
                    u8 v = r[i];
                    p0=(u16)((p0<<1)|( v    &1)); p1=(u16)((p1<<1)|((v>>1)&1));
                    p2=(u16)((p2<<1)|((v>>2)&1)); p3=(u16)((p3<<1)|((v>>3)&1));
                }
            }
            else
            {
                const u8 *r = row + (w - 1 - base);
                for (s32 i = 0; i < n; i++)
                {
                    u8 v = r[-i];
                    p0=(u16)((p0<<1)|( v    &1)); p1=(u16)((p1<<1)|((v>>1)&1));
                    p2=(u16)((p2<<1)|((v>>2)&1)); p3=(u16)((p3<<1)|((v>>3)&1));
                }
            }

            if (n < 16)
            {
                int s = 16 - n;                       // left-align the (rare) short last chunk
                p0<<=s; p1<<=s; p2<<=s; p3<<=s;
            }
            
            pw[0]=p0; pw[1]=p1; pw[2]=p2; pw[3]=p3;
        }
    }
}

// Kalms/Kropacek 4-plane transpose (SDL Atari C2pConvert4): d0/d1 hold 16 scrambled pixels ->
// out[0..3] = plane0..3, MSB = pixel 0. Callers build d0/d1 inline.
static inline void kalms_transpose(u32 d0, u32 d1, u16 *out)
{
    u32 d7;
    d7 = ((d1>>8) ^ d0) & 0x00ff00ffu; d0 ^= d7; d1 ^= (d7<<8);
    d7 = ((d1>>1) ^ d0) & 0x55555555u; d0 ^= d7; d1 ^= (d7<<1);
    // Exchange inner words via rotate-by-16 (compiles to `swap`, not a slow shift by 16).
    u32 sd0 = (d0 << 16) | (d0 >> 16);             // = swap(d0)
    u32 sd1 = (d1 << 16) | (d1 >> 16);             // = swap(d1)
    u32 nd0 = (d0  & 0xFFFF0000u) | (sd1 & 0x0000FFFFu);
    u32 nd1 = (sd0 & 0xFFFF0000u) | (d1  & 0x0000FFFFu);
    d0 = nd0; d1 = nd1;
    d7 = ((d1>>2) ^ d0) & 0x33333333u; d0 ^= d7; d1 ^= (d7<<2);
    // Final per-plane swap folded into the extraction.
    out[0]=(u16)d1; out[1]=(u16)(d1>>16);           // plane0,1
    out[2]=(u16)d0; out[3]=(u16)(d0>>16);           // plane2,3
}

// Build d0/d1 from 16 loose pixel bytes (slow path: partial/misaligned/paloff chunks).
static inline void kalms16_4(const u8 *in, u16 *out)
{
    // <<8 accumulation: GCC m68k renders <<24/<<16 as slow register shifts.
    #define KP(i) ((u32)(in[i] & 0x0F))
    u32 d0 =          ((KP(0)<<4)|KP(4));
    d0 = (d0<<8)  |   ((KP(1)<<4)|KP(5));
    d0 = (d0<<8)  |   ((KP(2)<<4)|KP(6));
    d0 = (d0<<8)  |   ((KP(3)<<4)|KP(7));
    u32 d1 =          ((KP(8)<<4)|KP(12));
    d1 = (d1<<8)  |   ((KP(9)<<4)|KP(13));
    d1 = (d1<<8)  |   ((KP(10)<<4)|KP(14));
    d1 = (d1<<8)  |   ((KP(11)<<4)|KP(15));
    #undef KP
    kalms_transpose(d0, d1, out);
}

// Fused 4-bit nibble -> 4-plane converter in one pass (applies paloff + colour-0 transparency).
// `nib` is the raw packed source (2 px/byte, hi first).
static void nib_to_planar4(const u8 *nib, s32 w, s32 h, u8 paloff, int transp, u8 *dst)
{
    // paloff is 0 for sp==4, so the stored value is the raw nibble; paloff!=0 is a slow fallback.
    s32 cpr = (w + 15) >> 4;
    for (s32 y = 0; y < h; y++)
    {
        s32 rb = y * w;
        u16 *pw = (u16 *)(dst + (s32)y * cpr * 8);
        for (s32 cc = 0; cc < cpr; cc++, pw += 4)
        {
            s32 base = cc * 16;
            s32 n = w - base; if (n > 16) n = 16;
            s32 g0 = rb + base;
            u16 out[4];
            if (n == 16 && !(g0 & 1) && paloff == 0)
            {
                // Aligned full chunk: build d0/d1 straight from 8 source bytes (no px[] array).
                const u8 *r = nib + (g0 >> 1);
                u8 b0=r[0],b1=r[1],b2=r[2],b3=r[3],b4=r[4],b5=r[5],b6=r[6],b7=r[7];
                u32 d0 =         (u32)((b0&0xF0)|(b2>>4));        // <<8 accumulation (cheap shifts)
                d0 = (d0<<8) |   (u32)(((b0&0x0f)<<4)|(b2&0x0f));
                d0 = (d0<<8) |   (u32)((b1&0xF0)|(b3>>4));
                d0 = (d0<<8) |   (u32)(((b1&0x0f)<<4)|(b3&0x0f));
                u32 d1 =         (u32)((b4&0xF0)|(b6>>4));
                d1 = (d1<<8) |   (u32)(((b4&0x0f)<<4)|(b6&0x0f));
                d1 = (d1<<8) |   (u32)((b5&0xF0)|(b7>>4));
                d1 = (d1<<8) |   (u32)(((b5&0x0f)<<4)|(b7&0x0f));
                kalms_transpose(d0, d1, out);
            }
            else
            {
                u8 px[16];
                for (s32 i = 0; i < 16; i++)
                {
                    if (i < n)
                    {
                        s32 g = g0 + i;
                        u8  b = nib[g >> 1];
                        u8 nb = (g & 1) ? (b & 0x0f) : (b >> 4);
                        px[i] = (transp && nb == 0) ? 0 : (u8)(paloff + nb);
                    }
                    else
                    {
                        px[i] = 0;
                    }
                }

                kalms16_4(px, out);
            }

            pw[0]=out[0]; pw[1]=out[1]; pw[2]=out[2]; pw[3]=out[3];
        }
    }
}

// On the desktop ALIS_PLANAR_CONV build the native render (image_draw_planar.c) is NOT compiled,
// so own the plane-count globals it would otherwise define. (Atari defines them there.)
#if !defined(ALIS_NATIVE_PLANAR)
int g_planar_nplanes = 8;
int g_used_planes = 8;
#endif
#endif // ALIS_PLANAR_CONV

// Conversion scratch buffer, grown on demand and reused across loads (never freed).
static u8 *s_conv_tmp = NULL;
static s32 s_conv_tmp_sz = 0;
static u8 *conv_tmp(s32 n)
{
    if (n > s_conv_tmp_sz)
    {
        u8 *p = (u8 *)realloc(s_conv_tmp, (size_t)n);
        if (!p)
            return NULL;
        
        s_conv_tmp = p; s_conv_tmp_sz = n;
    }

    return s_conv_tmp;
}

// Shared tail. `tmp` holds the decoded 8-bit indices (w*h); writes header + pixels to `dst`.
// do_grow: open room in place with growprog (dst==src); else space is pre-allocated and the header
// is relocated. Returns bytes added.
static s32 conv_finish(s32 dst, s32 src, s32 src_off, s32 old_pix_bytes, s32 w, s32 h, u8 *tmp, int transp, int do_grow)
{
    s32 npix = w * h;
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    extern int  g_planar_nplanes;          // set by pass 1 of convert_sprites_inplace
    s32 cpr = (w + 15) >> 4;
    // Source storage planes: 4 for 16-colour games (half the RAM + read), 8 otherwise. The 4bpp_*
    // blitters + the p4 dispatch in destofen_planar consume 4-plane (8-byte) source chunks.
    int sp  = (g_planar_nplanes == 4) ? 4 : 8;
    s32 out_bytes = h * cpr * (sp * 2);    // planar interleaved (padded to 16px chunks)
#else
    s32 out_bytes = npix;                  // 8-bit chunky
#endif
    s32 grow = (8 + out_bytes) - (src_off + old_pix_bytes);
    if (do_grow)
    {
        if (grow > 0 && !growprog(src + src_off, grow))
            return 0;
    }
    else if (dst != src)
    {
        memcpy(alis.mem + dst, alis.mem + src, (size_t)src_off);   // relocate header bytes
    }
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    if (sp == 4)
    {
        chunky4_to_planar(tmp, w, h, 0, alis.mem + dst + 8);
    }
    else
    {
        chunky8_to_planar(tmp, w, h, 0, alis.mem + dst + 8);
    }

    alis.mem[dst + 6] = (u8)sp;            // source plane count (flip rebuild + blitter read it)
#else
    for (s32 p = 0; p < npix; p++)
        alis.mem[dst + 8 + p] = tmp[p];
    alis.mem[dst + 6] = 0;
#endif
    alis.mem[dst + 7] = 0;
    alis.mem[dst]     = transp ? 0x14 : 0x16;
    return grow;          // tmp is the reusable scratch buffer — not freed
}

#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
// Fused 4-plane tail: save the packed nibbles, make room as conv_finish, then nib_to_planar4.
static s32 conv_finish_nib4(s32 dst, s32 src, s32 src_off, s32 old_pix_bytes, s32 w, s32 h, u8 paloff, int transp, int do_grow)
{
    s32 cpr = (w + 15) >> 4;
    s32 raw = (w * h + 1) >> 1;             // packed nibble bytes to preserve across the move
    u8 *nib = conv_tmp(raw);
    if (!nib)
        return 0;
    
    memcpy(nib, alis.mem + src + src_off, (size_t)raw);
    s32 grow = (8 + h * cpr * 8) - (src_off + old_pix_bytes);
    if (do_grow)
    {
        if (grow > 0 && !growprog(src + src_off, grow))
            return 0;
    }
    else if (dst != src)
    {
        memcpy(alis.mem + dst, alis.mem + src, (size_t)src_off);   // relocate header bytes
    }

    nib_to_planar4(nib, w, h, paloff, transp, alis.mem + dst + 8);
    alis.mem[dst + 6] = 4;
    alis.mem[dst + 7] = 0;
    alis.mem[dst]     = transp ? 0x14 : 0x16;
    return grow;
}
#endif

// 0x00/0x02 — ST 4-bit chunky (pixels at +6, no palette offset).
static s32 conv_st_4bit(s32 src, s32 dst, int do_grow)
{
    s32 w = xread16(src + 2) + 1, h = xread16(src + 4) + 1, npix = w * h;
    if (npix <= 0)
        return 0;
    
    int transp = (alis.mem[src] == 0x00);
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    extern int g_planar_nplanes;
    if (g_planar_nplanes == 4)             // fused decode+transpose, no 8-bit scratch
        return conv_finish_nib4(dst, src, 6, npix >> 1, w, h, 0, transp, do_grow);
#endif
    u8 *tmp = conv_tmp(npix);
    if (!tmp)
        return 0;
    
    const u8 *s = alis.mem + src + 6;
    for (s32 p = 0; p < npix; p++)
    {
        u8 nib = (p & 1) ? (s[p >> 1] & 0x0f) : (s[p >> 1] >> 4);
        tmp[p] = (transp && nib == 0) ? 0 : nib;
    }

    return conv_finish(dst, src, 6, npix >> 1, w, h, tmp, transp, do_grow);
}

// 0x10/0x12 — 4-bit chunky with palette-bank offset (pixels at +8, paloff at +6).
static s32 conv_4to8bit(s32 src, s32 dst, int do_grow)
{
    s32 w = xread16(src + 2) + 1, h = xread16(src + 4) + 1, npix = w * h;
    if (npix <= 0)
        return 0;
    
    int transp = (alis.mem[src] == 0x10);
    u8 paloff  = alis.mem[src + 6];
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    extern int g_planar_nplanes;
    if (g_planar_nplanes == 4)             // paloff==0 here (idx fits 4 planes)
        return conv_finish_nib4(dst, src, 8, npix >> 1, w, h, paloff, transp, do_grow);
#endif
    u8 *tmp = conv_tmp(npix);
    if (!tmp)
        return 0;
    
    const u8 *s = alis.mem + src + 8;
    for (s32 p = 0; p < npix; p++)
    {
        u8 nib = (p & 1) ? (s[p >> 1] & 0x0f) : (s[p >> 1] >> 4);
        tmp[p] = (transp && nib == 0) ? 0 : (u8)(paloff + nib);
    }

    return conv_finish(dst, src, 8, npix >> 1, w, h, tmp, transp, do_grow);
}

// 0x10/0x12 on Amiga — 5-bit planar (pixels at +6, 5 separate bitplanes, linear bitstream).
static s32 conv_ami_5bit(s32 src, s32 dst, int do_grow)
{
    s32 w = xread16(src + 2) + 1, h = xread16(src + 4) + 1, npix = w * h;
    if (npix <= 0)
        return 0;
    
    int transp = (alis.mem[src] == 0x10);
    u32 planesize = (u32)npix >> 3;
    u8 *tmp = conv_tmp(npix);
    if (!tmp)
        return 0;
    
    const u8 *s = alis.mem + src + 6;
    for (s32 p = 0; p < npix; p++)
    {
        s32 byte = p >> 3; int bit = 7 - (p & 7);
        u8 idx = 0;
        for (int pl = 0; pl < 5; pl++)
            idx |= (u8)(((s[byte + pl * planesize] >> bit) & 1) << pl);
        
        tmp[p] = (transp && idx == 0) ? 0 : idx;
    }

    return conv_finish(dst, src, 6, (s32)(5 * planesize), w, h, tmp, transp, do_grow);
}

#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
// 0x14/0x16 — already 8-bit chunky; only needs re-encoding to planar (compiled whenever the
// planar conversion is active).
static s32 conv_8bit(s32 src, s32 dst, int do_grow)
{
    s32 w = xread16(src + 2) + 1, h = xread16(src + 4) + 1, npix = w * h;
    if (npix <= 0)
        return 0;
    
    int transp = (alis.mem[src] == 0x14);
    u8 *tmp = conv_tmp(npix);
    if (!tmp)
        return 0;
    
    const u8 *s = alis.mem + src + 8;
    for (s32 p = 0; p < npix; p++)
        tmp[p] = s[p];
    
    return conv_finish(dst, src, 8, npix, w, h, tmp, transp, do_grow);
}
#endif

// Dispatch + convert one sprite from `src` to `dst` (matches PASS-1/2 platform gating exactly).
// Returns bytes added, or 0 if this sprite is not converted on this platform.
static s32 convert_one(s32 src, s32 dst, int do_grow)
{
    u8 fmt = alis.mem[src];
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    if (!conv_planar_enabled())
        return 0;                           // see conv_planar_enabled(): nothing planar on DOS data
#endif
    if (fmt == 0x00)
    {
        if (alis.platform.kind != EPlatformMac && !PL_CGA())
            return conv_st_4bit(src, dst, do_grow);
    }
    else if (fmt == 0x02)
    {
        if (alis.platform.kind != EPlatformMac && !(PL_CGA() && alis.platform.uid == EGameMadShow))
            return conv_st_4bit(src, dst, do_grow);
    }
    else if (fmt == 0x10 || fmt == 0x12)
    {
        return (alis.platform.px_format == EPxFormatAmPlanar) ? conv_ami_5bit(src, dst, do_grow) : conv_4to8bit(src, dst, do_grow);
    }
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    else if (fmt == 0x14 || fmt == 0x16)
    {
        return conv_8bit(src, dst, do_grow);
    }
#endif
    return 0;
}

// Measure-only twin of convert_one: returns the grow (bytes added) and old on-disk total for the
// sprite at `addr`, WITHOUT converting, so the compactor can lay out final positions up front.
// Mirrors the dispatch gating + conv_finish size math exactly.
static s32 sprite_measure(s32 addr, s32 *p_old_total)
{
    u8 fmt = alis.mem[addr];
    int src_off, converts = 0;
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    if (!conv_planar_enabled())
        return -1;                           // lockstep with convert_one's blanket gate
#endif
    if (fmt == 0x00)
    {
        converts = (alis.platform.kind != EPlatformMac && !PL_CGA());
        src_off = 6;
    }
    else if (fmt == 0x02)
    {
        converts = (alis.platform.kind != EPlatformMac && !(PL_CGA() && alis.platform.uid == EGameMadShow));
        src_off = 6;
    }
    else if (fmt == 0x10 || fmt == 0x12)
    {
        converts = 1;
        src_off = (alis.platform.px_format == EPxFormatAmPlanar) ? 6 : 8;
    }
    else if (fmt == 0x14 || fmt == 0x16)
    {
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
        converts = 1;
#endif
        src_off = 8;
    }
    else
    {
        return -1;                           // not a convertible format (grow>=0 means "converts")
    }

    if (!converts)
        return -1;

    s32 w = xread16(addr + 2) + 1, h = xread16(addr + 4) + 1, npix = w * h;
    if (npix <= 0)
        return -1;

    s32 old_pix;
    if (fmt == 0x10 || fmt == 0x12)
    {
        old_pix = (alis.platform.px_format == EPxFormatAmPlanar) ? (s32)(5 * ((u32)npix >> 3)) : (npix >> 1);
    }
    else if (fmt == 0x14 || fmt == 0x16)
    {
        old_pix = npix;
    }
    else
    {
        old_pix = npix >> 1;                 // 0x00 / 0x02
    }

#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    extern int g_planar_nplanes;
    s32 cpr = (w + 15) >> 4;
    int sp  = (g_planar_nplanes == 4) ? 4 : 8;
    s32 out_bytes = h * cpr * (sp * 2);
#else
    s32 out_bytes = npix;
#endif
    *p_old_total = src_off + old_pix;
    return (8 + out_bytes) - (src_off + old_pix);
}

// One converting sprite, captured up front (addresses are original, pre-move).
typedef struct { s32 addr; s32 grow; s32 old_total; } ConvItem;
static ConvItem *s_conv_items = NULL;
static s32       s_conv_items_cap = 0;
static ConvItem *conv_items(s32 n)
{
    if (n > s_conv_items_cap)
    {
        ConvItem *p = (ConvItem *)realloc(s_conv_items, (size_t)n * sizeof(ConvItem));
        if (!p)
            return NULL;
        
        s_conv_items = p; s_conv_items_cap = n;
    }

    return s_conv_items;
}

static int conv_item_cmp(const void *a, const void *b)
{
    s32 x = ((const ConvItem *)a)->addr, y = ((const ConvItem *)b)->addr;
    return (x > y) - (x < y);
}

// ---- The original per-sprite path: each conversion calls growprog, which shifts everything above
// it (O(sprites x size) total bytes moved). Kept for ALIS_CONV_COMPACT=0 / A-B verification. ----
static s32 convert_incremental(u32 org, s32 l, s32 e, s32 tab)
{
    s32 total = 0;
    for (s32 i = 0; i < e; i++)
    {
        s32 a    = tab + i * 4;
        s32 t    = xread32(org + a);
        s32 addr = org + a + t;            // re-read after grows
        s32 grow = convert_one(addr, addr, 1 /*do_grow*/);
        if (grow <= 0)
            continue;

        for (s32 j = 0; j < e; j++)  // every sprite above this one shifted up by `grow`
        {
            s32 aj = tab + j * 4;
            s32 tj = xread32(org + aj);
            if (addr < org + aj + tj)
                xwrite32(org + aj, tj + grow);
        }

        xwrite32(org + l + 0x6, xread32(org + l + 0x6) + grow);
        xwrite32(org + l + 0xc, xread32(org + l + 0xc) + grow);
        total += grow;
    }

    return total;
}

// ---- Single-pass compaction: measure every sprite's grow up front, lay out final positions, then
// relocate each region exactly once (top-down) while converting each sprite straight from its old
// slot into its final slot. Each byte moves once instead of once per sprite-below-it. ----
static s32 convert_compact(u32 org, s32 l, s32 e, s32 tab)
{
    ConvItem *it = conv_items(e > 0 ? e : 1);
    if (!it)
        return convert_incremental(org, l, e, tab);        // OOM → safe fallback

    // PASS 2a: capture converting sprites + their grows (original, pre-move addresses).
    s32 m = 0, total = 0;
    for (s32 i = 0; i < e; i++)
    {
        s32 addr = org + tab + i * 4 + xread32(org + tab + i * 4);
        s32 old_total = 0;
        s32 grow = sprite_measure(addr, &old_total);
        // grow >= 0 means the sprite converts; grow == 0 (e.g. 8-bit chunky -> 8-plane) must too.
        if (grow >= 0)
        {
            it[m].addr = addr;
            it[m].grow = grow;
            it[m].old_total = old_total;
            m++;
            total += grow;
        }
    }

    if (m == 0)
        return 0;
    
    qsort(it, (size_t)m, sizeof(ConvItem), conv_item_cmp);   // ascending by address

    // Offset table: entry j gains the sum of grows of all converting sprites BELOW its target.
    for (s32 j = 0; j < e; j++)
    {
        s32 aj = tab + j * 4;
        s32 tj = xread32(org + aj);
        s32 addrj = org + aj + tj;
        s32 delta = 0;
        for (s32 k = 0; k < m && it[k].addr < addrj; k++)
            delta += it[k].grow;
        
        if (delta)
            xwrite32(org + aj, tj + delta);
    }

    xwrite32(org + l + 0x6, xread32(org + l + 0x6) + total);
    xwrite32(org + l + 0xc, xread32(org + l + 0xc) + total);

    // Reserve the total space at the top (the per-call growprog script/sprite fixups are no-ops
    // during load — the converting script is the last loaded — so a single finprog bump suffices).
    s32 old_finprog = alis.finprog;
    if ((u32)(old_finprog + total) > alis.finmem)
        return 0;
    
    alis.finprog = old_finprog + total;

    // Relocate + convert, top-down. `shift` = bytes the region just above sprite k moves up; it
    // equals G_k + grow_k where G_k (cumulative grow below k) is the offset of sprite k's new slot.
    s32 read_top = old_finprog;
    s32 shift = total;
    for (s32 k = m - 1; k >= 0; k--)
    {
        s32 ak = it[k].addr;
        s32 reg_lo = ak + it[k].old_total;          // first byte above sprite k's old data
        s32 reg_len = read_top - reg_lo;
        if (reg_len > 0 && shift > 0)
        {
            memmove(alis.mem + reg_lo + shift, alis.mem + reg_lo, (size_t)reg_len);
        }

        s32 Gk = shift - it[k].grow;                // sprite k's new slot = ak + Gk
        convert_one(ak, ak + Gk, 0 /*space pre-allocated*/);
        read_top = ak;
        shift = Gk;
    }

    return total;
}

#ifndef ALIS_CONV_COMPACT
#define ALIS_CONV_COMPACT 1
#endif

// Load-time, IN PLACE: convert every sprite to 0x14/0x16 (8-bit chunky, or planar under
// ALIS_PLANAR_CONV) via per-format converters dispatched like destofen. Applies cshrink's
// fixups with the sign flipped. Returns total bytes added (caller bumps script->sz).
s32 convert_sprites_inplace(u32 org, const char *name)
{
    // Decide ONCE per script, before pass 1: every convert_one / sprite_measure call below must see
    // the same answer or the compactor's layout desynchronizes from what actually gets written.
    s_conv_planar_ok = conv_planar_allowed_for(name);

    s32 l   = xread32(org + 0xe);
    s16 e   = xread16(org + l + 4);
    s32 tab = xread32(org + l) + l;
    s32 total = 0;
#if defined(ALIS_PLANAR_CONV) && ALIS_PLANAR_CONV
    extern int g_planar_nplanes;
    static int s_conv_maxidx = 0;          // deepest colour index seen → plane count (4/6/8)
    // PASS 1: plane count from the deepest colour index (format + paloff), before any growprog
    // moves the table offsets.
    for (s32 i = 0; i < e; i++)
    {
        s32 addr = org + tab + i * 4 + xread32(org + tab + i * 4);
        u8  fmt  = alis.mem[addr];
        int mb = (fmt == 0x00 || fmt == 0x02) ? 15
               : (fmt == 0x10 || fmt == 0x12) ? (alis.platform.px_format == EPxFormatAmPlanar ? 31 : alis.mem[addr + 6] + 15)
               : (fmt == 0x14 || fmt == 0x16) ? 255 : 0;
        if (mb > s_conv_maxidx)
            s_conv_maxidx = mb;
    }

    g_planar_nplanes = (s_conv_maxidx < 16) ? 4 : (s_conv_maxidx < 64) ? 6 : 8;
    // Screen copy follows the colour depth (no clinepal active at conversion time).
    extern int g_used_planes;
    g_used_planes = g_planar_nplanes;

#endif

    // PASS 2: convert every sprite (single-pass compaction; g_planar_nplanes was set in pass 1).
#if ALIS_CONV_COMPACT
    total = convert_compact(org, l, e, tab);
#else
    total = convert_incremental(org, l, e, tab);
#endif
    return total;
}

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines pointer table (256 values)
// ============================================================================
const sAlisOpcode opcodes[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
    DECL_OPCODE(0x01, cesc1,        "TODO: add desc"),
    DECL_OPCODE(0x02, cesc2,        "[N/I]"),
    DECL_OPCODE(0x03, cesc3,        "[N/I]"),
    DECL_OPCODE(0x04, cbreakpt,     "[N/I]"),
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
    DECL_OPCODE(0x27, clocate,      "set cursor position x, y"),
    DECL_OPCODE(0x28, ctab,         "tabulation, set cursor position x"),
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
    DECL_OPCODE(0x37, canim,        "v6 tween/anim slot driver (cnul pre-v6)"),
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
    DECL_OPCODE(0x45, cload,        "load and unpack a script, set into vm"),
    DECL_OPCODE(0x46, cdefsc,       "define scene ??"),
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
    DECL_OPCODE(0x8b, cnul,         "[N/I] null"),
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
    DECL_OPCODE(0xc9, cscdump,      "credits scroll (v6)"),
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
    DECL_OPCODE(0xd4, cautobase,    "[N/I]"),
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
    DECL_OPCODE(0xe2, cshrink,      "delete bitmap and shift following data"),
    DECL_OPCODE(0xe3, cdefmap,      "TODO: add desc"),
    DECL_OPCODE(0xe4, csetmap,      "TODO: add desc"),
    DECL_OPCODE(0xe5, cputmap,      "TODO: add desc"),
    DECL_OPCODE(0xe6, csavepal,     "TODO: add desc"),
    DECL_OPCODE(0xe7, csczoom,      "TODO: add desc"),
    DECL_OPCODE(0xe8, ctexmap,      "TODO: add desc"),
    DECL_OPCODE(0xe9, calloctab,    "TODO: add desc"),
    DECL_OPCODE(0xea, cfreetab,     "TODO: add desc"),
    DECL_OPCODE(0xeb, cscantab,     "[N/I]"),
    DECL_OPCODE(0xec, cneartab,     "[N/I]"),
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

// ============================================================================
#pragma mark - v6.0+ opcode table
// ============================================================================
// See storenames.c for the rationale: the byte->handler MAPPING is the
// version-specific part, so v6 gets its own table built once from the base plus
// the entries that actually differ, instead of a `version >= 60` test at the top
// of every handler. Handlers whose v6 difference is a genuine small tweak in the
// middle of shared logic keep their inline branch — forking those would only
// duplicate code.
static sAlisOpcode opcodes_v6[256];
static const sAlisOpcode opcodes_v6_ov[] = {
    DECL_OPCODE(0x0a, cjmp24_v6,          "v6: cjmp24"),
    DECL_OPCODE(0x11, cret_v6,            "v6: cret"),
    DECL_OPCODE(0x14, cbz24_v6,           "v6: cbz24"),
    DECL_OPCODE(0x17, cbnz24_v6,          "v6: cbnz24"),
    DECL_OPCODE(0x33, cleave_v6,          "v6: cleave"),
    DECL_OPCODE(0x38, cscmov_v6,          "v6: cscmov"),
    DECL_OPCODE(0x39, cscset_v6,          "v6: cscset"),
    DECL_OPCODE(0x43, cstopret_v6,        "v6: cstopret"),
    DECL_OPCODE(0x44, cexit_v6,           "v6: cexit"),
    DECL_OPCODE(0x4b, cerasen_v6,         "v6: cerasen"),
    DECL_OPCODE(0x51, cforme_v6,          "v6: cforme"),
    DECL_OPCODE(0x52, cdelforme_v6,       "v6: cdelforme"),
    DECL_OPCODE(0x53, ctstmov_v6,         "v6: ctstmov"),
    DECL_OPCODE(0x54, ctstset_v6,         "v6: ctstset"),
    DECL_OPCODE(0x55, cftstmov_v6,        "v6: cftstmov"),
    DECL_OPCODE(0x56, cftstset_v6,        "v6: cftstset"),
    DECL_OPCODE(0x61, csend_v6,           "v6: csend"),
    DECL_OPCODE(0x66, cscanclr_v6,        "v6: cscanclr"),
    DECL_OPCODE(0x74, cfreadv_v6,         "v6: cfreadv"),
    DECL_OPCODE(0x81, cpicture_v6,        "v6: cpicture"),
    DECL_OPCODE(0x89, cdefvect_v6,        "v6: cdefvect"),
    DECL_OPCODE(0x8a, csetvect_v6,        "v6: csetvect"),
    DECL_OPCODE(0x8e, cvtstmov_v6,        "v6: cvtstmov"),
    DECL_OPCODE(0x8f, cvftstmov_v6,       "v6: cvftstmov"),
    DECL_OPCODE(0x93, cfindmat_v6,        "v6: cfindmat"),
    DECL_OPCODE(0x98, csetvolum_v6,       "v6: csetvolum"),
    DECL_OPCODE(0xa1, cdelsound_v6,       "v6: cdelsound"),
    DECL_OPCODE(0xa3, cwtstmov_v6,        "v6: cwtstmov"),
    DECL_OPCODE(0xa4, cwftstmov_v6,       "v6: cwftstmov"),
    DECL_OPCODE(0xa5, ctstform_v6,        "v6: ctstform"),
    DECL_OPCODE(0xa6, cxput_v6,           "v6: cxput"),
    DECL_OPCODE(0xa7, cxputat_v6,         "v6: cxputat"),
    DECL_OPCODE(0xa8, cmput_v6,           "v6: cmput"),
    DECL_OPCODE(0xa9, cmputat_v6,         "v6: cmputat"),
    DECL_OPCODE(0xaa, cmxput_v6,          "v6: cmxput"),
    DECL_OPCODE(0xab, cmxputat_v6,        "v6: cmxputat"),
    DECL_OPCODE(0xba, cfont_v6,           "v6: cfont"),
    DECL_OPCODE(0xc3, cschoriz_v6,        "v6: cschoriz"),
    DECL_OPCODE(0xc7, creducing_v6,       "v6: creducing"),
    DECL_OPCODE(0xc8, cscmap_v6,          "v6: cscmap"),
    DECL_OPCODE(0xc9, cscdump_v6,         "v6: cscdump"),
    DECL_OPCODE(0xd0, calign_v6,          "v6: calign"),
    DECL_OPCODE(0xdb, ccancel_v6,         "v6: ccancel"),
    DECL_OPCODE(0xdc, ccancall_v6,        "v6: ccancall"),
    DECL_OPCODE(0xdd, ccancen_v6,         "v6: ccancen"),
    DECL_OPCODE(0xe8, ctexmap_v6,         "v6: ctexmap"),
    DECL_OPCODE(0xef, cscdark_v6,         "v6: cscdark"),
    DECL_OPCODE(0xf0, caset_v6,           "v6: caset"),
    DECL_OPCODE(0xf1, camov_v6,           "v6: camov"),
    DECL_OPCODE(0xf2, cscaset_v6,         "v6: cscaset"),
    DECL_OPCODE(0xf3, cscamov_v6,         "v6: cscamov"),
    DECL_OPCODE(0xf4, cscfollow_v6,       "v6: cscfollow"),
    DECL_OPCODE(0xf7, cwalkmap_v6,        "v6: cwalkmap"),
    DECL_OPCODE(0xf9, cavtstmov_v6,       "v6: cavtstmov"),
    DECL_OPCODE(0xfb, caim_v6,            "v6: caim"),
    DECL_OPCODE(0xfc, cpointpix_v6,       "v6: cpointpix"),
    DECL_OPCODE(0xfe, cscsky_v6,          "v6: cscsky"),
    DECL_OPCODE(0xff, czoom_v6,           "v6: czoom"),
};

const sAlisOpcode *alis_tab_opcodes(void)
{
    static const sAlisOpcode *live = NULL;
    if (live == NULL)
    {
        if (alis.platform.version < 60)
        {
            live = opcodes;
        }
        else
        {
            memcpy(opcodes_v6, opcodes, sizeof(opcodes));
            for (u32 i = 0; i < sizeof(opcodes_v6_ov) / sizeof(opcodes_v6_ov[0]); i++)
                opcodes_v6[(u8)opcodes_v6_ov[i].code] = opcodes_v6_ov[i];
            live = opcodes_v6;
        }
    }

    return live;
}
