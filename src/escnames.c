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
#include "alis_private.h"

// ============================================================================
#pragma mark - Escnames routines
// ============================================================================
static void cnul(void) {
}
void consound(void) {
    debug(EDebugWarning, "MISSING: ", __FUNCTION__);
}
void coffsound(void) {
    debug(EDebugWarning, "MISSING: ", __FUNCTION__);
}
void conmusic(void) {
    debug(EDebugWarning, "MISSING: ", __FUNCTION__);
}
void coffmusic(void) {
    debug(EDebugWarning, "MISSING: ", __FUNCTION__);
}
void cdelfilm(void) {
    debug(EDebugWarning, "MISSING: ", __FUNCTION__);
}
void copenfilm(void) {
    debug(EDebugWarning, "MISSING: ", __FUNCTION__);
}


// ============================================================================
#pragma mark - Escnames pointer table
// ============================================================================
sAlisOpcode escnames[] = {
    DECL_OPCODE(0x00, cnul,         "null"),
    DECL_OPCODE(0x01, consound,     "TODO: add desc"),
    DECL_OPCODE(0x02, coffsound,    "TODO: add desc"),
    DECL_OPCODE(0x03, conmusic,     "TODO: add desc"),
    DECL_OPCODE(0x04, coffmusic,    "TODO: add desc"),
    DECL_OPCODE(0x05, cdelfilm,     "TODO: add desc"),
    DECL_OPCODE(0x06, copenfilm,    "TODO: add desc")
};
