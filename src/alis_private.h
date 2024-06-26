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

#include "alis.h"
#include "debug.h"
#include "sys/sys.h"

void readexec_opcode(void);
void readexec_opername(void);
void readexec_storename(void);
void readexec_addname(void);
void readexec_opername_saveD7(void);
void readexec_opername_saveD6(void);
void readexec_addname_swap(void);
void readexec_opername_swap(void);

void cstore_continue(void);

extern sAlisOpcode  opcodes[];
extern sAlisOpcode  opernames[];
extern sAlisOpcode  storenames[];
extern sAlisOpcode  addnames[];
extern sAlisError   errors[];

// common opcode helpers
extern u16  loctc_common(u16 offset);
extern u16  locti_common(u16 offset);
extern u16  loctp_common(u16 offset);
extern void oeval(void);
extern void ofin(void);
