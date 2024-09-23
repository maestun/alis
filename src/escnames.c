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
#include "audio.h"
#include "video.h"

// ============================================================================
#pragma mark - Escnames routines
// ============================================================================

// Escname no. 01 opcode 0x00 cnul
static void cnul(void) {
}

// Escname no. 02 opcode 0x01 consound
void consound(void) {
    audio.fsound = 1;
}

// Escname no. 03 opcode 0x02 coffsound
void coffsound(void) {
    audio.fsound = 0;
}

// Escname no. 04 opcode 0x03 conmusic
void conmusic(void) {
    audio.fmusic = 1;
}

// Escname no. 05 opcode 0x04 coffmusic
void coffmusic(void) {
    audio.fmusic = 0;
}

// Escname no. 06 opcode 0x05 cdelfilm
void cdelfilm(void) {
    endfilm();
    
    if (bfilm.delptr != NULL)
        free(bfilm.delptr);

    memset(&bfilm, 0, sizeof(bfilm));
}

// Escname no. 07 opcode 0x06 copenfilm
void copenfilm(void) {
    memset(&bfilm, 0, sizeof(bfilm));

    readexec_opername();
    bfilm.id = alis.varD7;

    readexec_opername_swap();
    
    alis.fp = NULL;

    // TODO: hack!!!

    char *at = strstr(alis.sd7, "150.");
    if (at != NULL)
    {
        memcpy(at, "300.", 4);
    }
    
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    strcat(path, alis.sd7);

    afopen((char *)path, 1);
    
    u8 *addr = NULL;
    
    if (alis.fp)
    {
        struct stat st;
        fstat(fileno(alis.fp), &st);
        off_t size = st.st_size;
        
        addr = malloc(size);
        if (addr)
        {
            fread(addr, size, 1, alis.fp);
        }
    }
    
    bfilm.addr1 = addr;
    bfilm.addr2 = addr;
    bfilm.delptr = addr;
    // TODO: not realy sure what next value is used for
    readexec_opername();
    readexec_opername();
    bfilm.waitclock = alis.varD7;
    bfilm.basemain = alis.basemain;
    inifilm();
    alis.varD7 = bfilm.frames;
    cstore_continue();
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
