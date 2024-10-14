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
#include "image.h"
#include "mem.h"
#include "utils.h"

// ============================================================================
#pragma mark - Store routines
// ============================================================================

// Storename no. 01 opcode 0x00 pnul
// Storename no. 02 opcode 0x02 pnul
// Storename no. 03 opcode 0x04 pnul
// Calls stub pnul

// Storename no. 04 opcode 0x06 slocb
/**
 * @brief reads a word (offset) from script, then stores d7 byte at (vram + offset)
 *
 */
static void slocb(void) {
    s16 offset = script_read16();
    xwrite8(alis.script->vram_org + offset, alis.varD7);
}

// Storename no. 05 opcode 0x08 slocw
/**
 * @brief reads a word (offset) from script, then stores d7 word at (vram + offset)
 *
 */
static void slocw(void) {
    s16 offset = script_read16();
    xwrite16(alis.script->vram_org + offset, alis.varD7);
}

// Storename no. 06 opcode 0x0a slocp
/**
 * @brief reads a word (offset) from script, then stores null-terminated string in ARRAY_C at (vram + offset)
 *
 */
static void slocp(void) {
    s16 offset = script_read16();
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)get_vram(offset), (char *)alis.oldsd7);
}

// Storename no. 07 opcode 0x0c sloctp
// Store at LOCation with offseT: Pointer
static void sloctp(void) {
    s32 addr = tabstring(alis.script->vram_org + script_read16());
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)(alis.mem + addr), (char *)alis.oldsd7);
}

// Storename no. 08 opcode 0x0e sloctc
// Store at LOCation with offseT: Char
static void sloctc(void) {
    s32 addr = tabchar(alis.script->vram_org + script_read16());
    alis.varD7 = *alis.acc++;
    xwrite8(addr, alis.varD7);
}

// Storename no. 09 opcode 0x10 slocti
// Store at LOCation with offseT: Int
static void slocti(void) {

    u32 addr = tabint(alis.script->vram_org + script_read16());
    alis.varD7 = *alis.acc++;
    xwrite16(addr, alis.varD7);
}


// Storename no. 10 opcode 0x12 sdirb
/**
 * @brief reads a byte (offset) from script, then stores d7 byte at (vram + offset)
 *
 */
static void sdirb(void) {
    xwrite8(alis.script->vram_org + script_read8(), (u8)alis.varD7);
}

// Storename no. 11 opcode 0x14 sdirw
/**
 * @brief reads a byte (offset) from script, then stores d7 word at (vram + offset)
 *
 */
static void sdirw(void) {
    
    u8 offset = script_read8();
    xwrite16(alis.script->vram_org + offset, alis.varD7);
}

// Storename no. 12 opcode 0x16 sdirp
/**
 * @brief reads a byte (offset) from script, then stores null-terminated string in ARRAY_C at (vram + offset)
 *
 */
static void sdirp(void) {
    u8 offset = script_read8();
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)get_vram(offset), (char *)alis.oldsd7);
}

// Storename no. 13 opcode 0x18 sdirtp
static void sdirtp(void) {
    s32 addr = tabstring(alis.script->vram_org + script_read8());
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)(alis.mem + addr), (char *)alis.oldsd7);
}

// Storename no. 14 opcode 0x1a sdirtc
static void sdirtc(void) {
    s32 addr = tabchar(alis.script->vram_org + script_read8());
    alis.varD7 = *alis.acc++;
    xwrite8(addr, alis.varD7);
}

// Storename no. 15 opcode 0x1c sdirti
static void sdirti(void) {
    u32 addr = tabint(alis.script->vram_org + script_read8());
    alis.varD7 = *alis.acc++;
    xwrite16(addr, alis.varD7);
}

// Storename no. 16 opcode 0x1e smainb
static void smainb(void) {
    s16 offset = script_read16();
//    debug(EDebugWarning, " [%.2x => %.6x]", (u8)alis.varD7, alis.basemain + offset);
    xwrite8(alis.basemain + offset, (u8)alis.varD7);
}

// Storename no. 17 opcode 0x20 smainw
static void smainw(void) {
    s16 offset = script_read16();
//    debug(EDebugWarning, " [%.4x => %.6x]", (s16)alis.varD7, alis.basemain + offset);
    xwrite16(alis.basemain + offset, (s16)alis.varD7);
}

// Storename no. 18 opcode 0x22 smainp
static void smainp(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%s => %.6x]", (char *)alis.oldsd7, alis.basemain + offset);
    strcpy((char *)(alis.mem + alis.basemain + offset), (char *)alis.oldsd7);
}

// Storename no. 19 opcode 0x24 smaintp
static void smaintp(void) {
    s32 addr = tabstring(alis.basemain + script_read16());
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + addr), addr);
    strcpy((char *)(alis.mem + addr), (char *)alis.oldsd7);
}

// Storename no. 20 opcode 0x26 smaintc
static void smaintc(void) {
    s32 addr = tabchar(alis.basemain + script_read16());
    alis.varD7 = *alis.acc++;
    xwrite8(addr, alis.varD7);
}

// Storename no. 21 opcode 0x28 smainti
static void smainti(void) {
    u32 addr = tabint(alis.basemain + script_read16());
    alis.varD7 = *alis.acc++;
    xwrite16(addr, alis.varD7);
}

// Storename no. 22 opcode 0x2a shimb
static void shimb(void) {
    s16 offset = script_read16();
    s16 ent = xread16(alis.script->vram_org + offset);

    s32 vram = xread32(alis.atent + ent);

    s16 offset2 = script_read16();
    xwrite8(vram + offset2, (u8)alis.varD7);
}

// Storename no. 23 opcode 0x2c shimw
static void shimw(void) {
    s16 offset = script_read16();
    s16 ent = xread16(alis.script->vram_org + offset);

    s32 vram = xread32(alis.atent + ent);

    s16 offset2 = script_read16();
    xwrite16(vram + offset2, (s16)alis.varD7);
}

// Storename no. 24 opcode 0x2e shimp
static void shimp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 25 opcode 0x30 shimtp
static void shimtp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 26 opcode 0x32 shimtc
static void shimtc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 27 opcode 0x34 shimti
static void shimti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 28 opcode 0x36 spile
void spile(void) {
    *(--alis.acc) = alis.varD7;
}

// Storename no. 29 opcode 0x38 seval
static void seval(void) {
    *(--alis.acc) = alis.varD7;
    oeval();
    readexec_storename();
}

// Storename no. 30 opcode 0x3a ofin
// Addname no. 30 opcode 0x3a ofin
// Calls oper->ofin

void cstore_continue(void) {
    char *tmp = alis.sd7;
    alis.sd7 = alis.oldsd7;
    alis.oldsd7 = tmp;

    readexec_storename();
}

// ============================================================================
#pragma mark - Store routines pointer table
// In the disasm, the table JTAB_STOREADD_NAMES is at address $10f92 -> $10fce.
// So it contains 0x3c / 60 bytes, or 30 words, it matches the storeadd count.
// So the routine FUN_READEXEC_OPERNAME() gets a BYTE code from script pc,
//  then retrieves a WORD offset at (JTAB_OPERNAMES[code]), then
//  jumps at the address (JTAB_OPERAMES + offset).
// ============================================================================
sAlisOpcode storenames[] = {
    DECL_OPCODE(0x00, pnul,         "[N/I] null pointer"),
    {},
    DECL_OPCODE(0x02, pnul,         "[N/I] null pointer"),
    {},
    DECL_OPCODE(0x04, pnul,         "[N/I] null pointer"),
    {},
    DECL_OPCODE(0x06, slocb,        "store byte from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x08, slocw,        "store word from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x0a, slocp,        "store null-terminated string from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x0c, sloctp,       "copy bss3 at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x0e, sloctc,       "pop byte from accumulator, store at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x10, slocti,       "pop word from accumulator, store at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x12, sdirb,        "store byte from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x14, sdirw,        "store word from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x16, sdirp,        "store null-terminated string from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x18, sdirtp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x1a, sdirtc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x1c, sdirti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x1e, smainb,       "TODO add desc"),
    {},
    DECL_OPCODE(0x20, smainw,       "TODO add desc"),
    {},
    DECL_OPCODE(0x22, smainp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x24, smaintp,      "TODO add desc"),
    {},
    DECL_OPCODE(0x26, smaintc,      "TODO add desc"),
    {},
    DECL_OPCODE(0x28, smainti,      "TODO add desc"),
    {},
    DECL_OPCODE(0x2a, shimb,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2c, shimw,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2e, shimp,        "TODO add desc"),
    {},
    DECL_OPCODE(0x30, shimtp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x32, shimtc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x34, shimti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x36, spile,        "pops word from virtual accumulator into r7"),
    {},
    DECL_OPCODE(0x38, seval,        "starts seval loop"),
    {},
    DECL_OPCODE(0x3a, ofin,         "ends seval loop (calls oper->ofin)"),
    {}
};
