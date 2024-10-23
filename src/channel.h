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
#include "time.h"

typedef enum {
    eChannelTypeNone        = 0,
    eChannelTypeDingZap     = 1,
    eChannelTypeNoise       = 2,
    eChannelTypeExplode     = 3,
    eChannelTypeSample      = 0x80,
} eChannelType;

typedef struct {
    
    s8 state;
    s8 curson;
    s16 pere;
    eChannelType type;
    u8 *address;
    u32 played;
    s16 volume;
    u8 unk0x7;
    u32 length;
    s16 freq;
    u8 loop;

    s16 delta_volume;
    s16 delta_freq;

} sChannel;
