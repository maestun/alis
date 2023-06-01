//
//  addnames.c
//  aodec
//
//  Created by zlot on 05/02/2020.
//  Copyright © 2020 zlot. All rights reserved.
//

#include "alis.h"
#include "alis_private.h"

#include "experimental.h"

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
    debug(EDebugInfo, " /* MISSING */");
}
static void aloctc(void) {
    debug(EDebugInfo, " /* MISSING */");
}

static void alocti(void) {
    debug(EDebugInfo, " /* STUBBED */");

    s16 offset = script_read16();

    // TODO: finish
    // short result = tabint(offset, alis.mem + );
//    xadd16(alis.script->vram_org + result, *a4);
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
    debug(EDebugInfo, " /* MISSING */");
}
static void adirtc(void) {
    debug(EDebugInfo, " /* CHECK */");
    s16 offset = tabchar(script_read8(), alis.mem + alis.script->vram_org);
    xwrite8(alis.script->vram_org + offset, (char)*(alis.acc));
    alis.acc++;
}
static void adirti(void) {
    s16 offset = tabint(script_read8(), alis.mem + alis.script->vram_org);
    xadd16(alis.script->vram_org + offset, xpop32(alis.script->vram_org));
}
static void amainb(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%.2x => %.6x]", (u8)alis.varD7, alis.basemain + offset);
    *(u8 *)(BASEMNMEM_PTR + offset) += (u8)alis.varD7;
}
static void amainw(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%.4x => %.6x]", (s16)alis.varD7, alis.basemain + offset);
    *(s16 *)(BASEMNMEM_PTR + offset) += (s16)alis.varD7;
}
static void amainp(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%s => %.6x]", (char *)alis.oldsd7, alis.basemain + offset);
    strcat((char *)(BASEMNMEM_PTR + offset), (char *)alis.oldsd7);
}
static void amaintp(void) {
    debug(EDebugInfo, " /* MISSING */");
}
static void amaintc(void) {
    s16 offset = tabchar(script_read16(), BASEMNMEM_PTR);
    debug(EDebugWarning, " [%.2x => %.6x]", *(u8 *)(alis.acc), alis.basemain + offset);
    *(u8 *)(BASEMNMEM_PTR + offset) += *(alis.acc);
    alis.acc++;
}
static void amainti(void) {
    s16 offset = tabint(script_read16(), BASEMNMEM_PTR);
    debug(EDebugWarning, " [%.4x => %.6x]", *(s16 *)(alis.acc), alis.basemain + offset);
    *(s16 *)(BASEMNMEM_PTR + offset) += *(alis.acc);
    alis.acc++;
}
static void ahimb(void) {
    debug(EDebugInfo, " /* MISSING */");
}
static void ahimw(void) {
    debug(EDebugInfo, " /* MISSING */");
}
static void ahimp(void) {
    debug(EDebugInfo, " /* MISSING */");
}
static void ahimtp(void) {
    debug(EDebugInfo, " /* MISSING */");
}
static void ahimtc(void) {
    debug(EDebugInfo, " /* MISSING */");
}
static void ahimti(void) {
    debug(EDebugInfo, " /* MISSING */");
}
static void spile(void) {
    debug(EDebugInfo, " /* MISSING */");
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
