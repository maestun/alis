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


// ============================================================================
#pragma mark - Addnames routines
// ============================================================================
static void cnul(void) {
}

/**
 * @brief reads word (offset) from script,
 *          adds byte from d7 to byte at (vram+offset)
 * 
 */
static void alocb(void) {
    s16 offset = script_read16();
    xadd8(alis.script->vram_org + offset, (u8)alis.varD7);
    alis.sr.zero = (xread8(alis.script->vram_org + offset) == 0);
}

/**
 * @brief reads word (offset) from script,
 *          adds word from d7 to word at (vram+offset)
 * 
 */
static void alocw(void) {
    s16 offset = script_read16();
    xadd16(alis.script->vram_org + offset, alis.varD7);
    alis.sr.zero = (xread16(alis.script->vram_org + offset) == 0);
}

/**
 * @brief read word (offset) from script,
 *          concatenate null-terminated string at ARRAY_A
 *          to null-terminated string located at (vram+offset)
 */
static void alocp(void) {
    s16 offset = script_read16();
    strcat((char *)get_vram(offset), (char *)alis.oldsd7);
}

static void aloctp(void) {
    debug(EDebugInfo, "CHECK: ", __FUNCTION__);
    s16 offset = tabstring(script_read16(), alis.script->vram_org);
    strcat((char *)(alis.mem + alis.script->vram_org + offset), (char *)alis.oldsd7);
}
static void aloctc(void) {
    s16 offset = tabchar(script_read16(), alis.script->vram_org);
    xadd8(alis.script->vram_org + offset, (char)*alis.acc++);
}

static void alocti(void) {
    s16 offset = tabint(script_read16(), alis.script->vram_org);
    xadd16(alis.script->vram_org + offset, *alis.acc++);
}
static void adirb(void) {
    u8 offset = script_read8();
    xadd8(alis.script->vram_org + offset, (u8)alis.varD7);
    alis.sr.zero = (xread8(alis.script->vram_org + offset) == 0);
}
static void adirw(void) {
    u8 offset = script_read8();
    xadd16(alis.script->vram_org + offset, alis.varD7);
    alis.sr.zero = (xread16(alis.script->vram_org + offset) == 0);
}
static void adirp(void) {
    u8 offset = script_read8();
    strcat((char *)get_vram(offset), (char *)alis.oldsd7);
}

static void adirtp(void) {
    debug(EDebugInfo, "CHECK: ", __FUNCTION__);
    s16 offset = tabstring(script_read8(), alis.script->vram_org);
    strcat((char *)(alis.mem + alis.script->vram_org + offset), (char *)alis.oldsd7);
}
static void adirtc(void) {
    s16 offset = tabchar(script_read8(), alis.script->vram_org);
    xadd8(alis.script->vram_org + offset, (char)*alis.acc++);
}
static void adirti(void) {
    s16 offset = tabint(script_read8(), alis.script->vram_org);
    xadd16(alis.script->vram_org + offset, *alis.acc++);
}
static void amainb(void) {
    s16 offset = script_read16();
    xadd8(alis.basemain + offset, (u8)alis.varD7);
}
static void amainw(void) {
    s16 offset = script_read16();
    xadd16(alis.basemain + offset, (s16)alis.varD7);
}
static void amainp(void) {
    s16 offset = script_read16();
    strcat((char *)(alis.mem + alis.basemain + offset), (char *)alis.oldsd7);
}
static void amaintp(void) {
    debug(EDebugInfo, "CHECK: ", __FUNCTION__);
    s16 offset = tabstring(script_read16(), alis.basemain);
    strcat((char *)(alis.mem + alis.basemain + offset), (char *)alis.oldsd7);
}
static void amaintc(void) {
    s16 offset = tabchar(script_read16(), alis.basemain);
    xadd8(alis.basemain + offset, (u8)*alis.acc++);
}
static void amainti(void) {
    s16 offset = tabint(script_read16(), alis.basemain);
    xadd16(alis.basemain + offset, *alis.acc++);
}
static void ahimb(void) {
    debug(EDebugInfo, "MISSING: ", __FUNCTION__);
}
static void ahimw(void) {
    s16 offset = xread16(alis.script->vram_org + script_read16());
    s32 offset2 = xread32(alis.atent + offset) + script_read16();
    xadd16(offset2, alis.varD7);
}
static void ahimp(void) {
    debug(EDebugInfo, "MISSING: ", __FUNCTION__);
}
static void ahimtp(void) {
    debug(EDebugInfo, "CHECK: ", __FUNCTION__);
    s32 addr = xread32(alis.atent + xread16(alis.script->vram_org + script_read16()));
    s16 offset = tabstring(script_read16(), alis.script->vram_org);
    strcat((char *)(alis.mem + addr + offset), (char *)alis.oldsd7);
}
static void ahimtc(void) {
    debug(EDebugInfo, "MISSING: ", __FUNCTION__);
}
static void ahimti(void) {
    debug(EDebugInfo, "MISSING: ", __FUNCTION__);
}
static void spile(void) {
    debug(EDebugInfo, "MISSING: ", __FUNCTION__);
}
static void aeval(void) {
    *(--alis.acc) = alis.varD7;
    oeval();
    readexec_addname();
}


// ============================================================================
#pragma mark - Addnames pointer table
// ============================================================================
sAlisOpcode addnames[] = {
    DECL_OPCODE(0x00, cnul,
                "[N/I]"),
    {},
    DECL_OPCODE(0x02, cnul,
                "[N/I]"),
    {},
    DECL_OPCODE(0x04, cnul,
                "[N/I]"),
    {},
    DECL_OPCODE(0x06, alocb, 
                "reads word (offset) from script, adds byte from d7 to byte at (vram+offset)"),
    {},
    DECL_OPCODE(0x08, alocw, 
                "reads word (offset) from script, adds word from d7 to word at (vram+offset)"),
    {},
    DECL_OPCODE(0x0a, alocp, 
                "read word (offset) from script, concatenate null-terminated string at ARRAY_A to null-terminated string located at (vram+offset)"),
    {},
    DECL_OPCODE(0x0c, aloctp, "TODO add desc"),
    {},
    DECL_OPCODE(0x0e, aloctc, "TODO add desc"),
    {},
    DECL_OPCODE(0x10, alocti, "TODO add desc"),
    {},
    DECL_OPCODE(0x12, adirb, "TODO add desc"),
    {},
    DECL_OPCODE(0x14, adirw, "TODO add desc"),
    {},
    DECL_OPCODE(0x16, adirp, "TODO add desc"),
    {},
    DECL_OPCODE(0x18, adirtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x1a, adirtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x1c, adirti, "TODO add desc"),
    {},
    DECL_OPCODE(0x1e, amainb, "TODO add desc"),
    {},
    DECL_OPCODE(0x20, amainw, "TODO add desc"),
    {},
    DECL_OPCODE(0x22, amainp, "TODO add desc"),
    {},
    DECL_OPCODE(0x24, amaintp, "TODO add desc"),
    {},
    DECL_OPCODE(0x26, amaintc, "TODO add desc"),
    {},
    DECL_OPCODE(0x28, amainti, "TODO add desc"),
    {},
    DECL_OPCODE(0x2a, ahimb, "TODO add desc"),
    {},
    DECL_OPCODE(0x2c, ahimw, "TODO add desc"),
    {},
    DECL_OPCODE(0x2e, ahimp, "TODO add desc"),
    {},
    DECL_OPCODE(0x30, ahimtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x32, ahimtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x34, ahimti, "TODO add desc"),
    {},
    DECL_OPCODE(0x36, spile, "TODO add desc"),
    {},
    DECL_OPCODE(0x38, aeval, "TODO add desc"),
    {},
    DECL_OPCODE(0x3a, ofin, "TODO add desc")
};
