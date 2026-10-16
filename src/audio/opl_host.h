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

// opl_host.h — host-side OPL register decoder + per-frame FM block emitter.
// Models the 68k side of the DSP OPL music player: consume the same register
// writes the ALIS sequencer emits, then hand the DSP resolved opl_block_t's.

#ifndef OPL_HOST_H
#define OPL_HOST_H

#include <stdint.h>
#include "opl_block.h"

typedef struct opl_host opl_host_t;

opl_host_t *opl_host_new(void);
void        opl_host_reset(opl_host_t *h);
void        opl_host_write_reg(opl_host_t *h, uint32_t reg, uint8_t data);
// Resolve channel ch (0..8) into *out (tll/rks computed here).
void        opl_host_get_block(opl_host_t *h, int ch, opl_block_t *out);
void        opl_host_delete(opl_host_t *h);

#endif
