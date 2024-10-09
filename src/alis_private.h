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
void readexec_codesc1name(void);
void readexec_codesc2name(void);
void readexec_codesc3name(void);
void readexec_opername(void);
void readexec_storename(void);
void readexec_addname(void);
void readexec_opername_saveD7(void);
void readexec_opername_saveD6(void);
void readexec_addname_swap(void);
void readexec_opername_swap(void);

void cstore_continue(void);

extern sAlisOpcode  opcodes[];
extern sAlisOpcode  codesc1names[];
extern sAlisOpcode  codesc2names[];
extern sAlisOpcode  codesc3names[];
extern sAlisOpcode  opernames[];
extern sAlisOpcode  storenames[];
extern sAlisOpcode  addnames[];
extern sAlisError   errors[];

// newer versions of ALIS
extern sAlisOpcode  opcodes_v40[];
extern sAlisOpcode  codesc1names_v40[];
extern sAlisOpcode  opernames_v40[];
extern sAlisOpcode  storenames_v40[];
extern sAlisOpcode  addnames_v40[];

// common opcode helpers
extern u16  loctc_common(u16 offset);
extern u16  locti_common(u16 offset);
extern u16  loctp_common(u16 offset);

extern void cnul(void);   // Stub for codop, codesc1, codesc2, codesc3
extern void pnul(void);   // Stub for oper, add, store

extern void oeval(void);  // Opername opcode 0x38 oeval
extern void ofin(void);   // Opername opcode 0x3a ofin. Used in Add and Store tables as well
extern void spile(void);  // Storename opcode 0x36 spile. Used in Add table as well
