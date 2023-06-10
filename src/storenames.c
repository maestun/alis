//
//  storenames.c
//  aodec
//
//  Created by zlot on 05/02/2020.
//  Copyright © 2020 zlot. All rights reserved.
//

#include "alis.h"
#include "alis_private.h"
#include "utils.h"

#include "experimental.h"

// ============================================================================
#pragma mark - Store Routines
// ============================================================================

static void cnul(void) {
}

/**
 * @brief reads a word (offset) from script, then stores d7 byte at (vram + offset)
 * 
 */
static void slocb(void) {
    s16 offset = script_read16();
    vwrite8(alis.script->vram_org + offset, alis.varD7);
}

/**
 * @brief reads a word (offset) from script, then stores d7 word at (vram + offset)
 * 
 */
static void slocw(void) {
    s16 offset = script_read16();
    vwrite16(alis.script->vram_org + offset, alis.varD7);
}

/**
 * @brief reads a word (offset) from script, then stores null-terminated string in ARRAY_C at (vram + offset)
 * 
 */
static void slocp(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)get_vram(offset), (char *)alis.oldsd7);
}

// Store at LOCation with offseT: Pointer
static void sloctp(void) {
    s16 offset = tabstring(script_read16(), alis.mem + alis.script->vram_org);
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)alis.mem + alis.script->vram_org + offset, (char *)alis.oldsd7);
}

// Store at LOCation with offseT: Char
static void sloctc(void) {
    s16 offset = tabchar(script_read16(), alis.mem + alis.script->vram_org);
    alis.varD7 = *(alis.acc);
    alis.acc++;
    vwrite8(alis.script->vram_org + offset, alis.varD7);
}

// Store at LOCation with offseT: Int
static void slocti(void) {
    s16 offset = tabint(script_read16(), alis.mem + alis.script->vram_org);
    alis.varD7 = *(alis.acc);
    alis.acc++;
    vwrite16(alis.script->vram_org + offset, alis.varD7);
}


/**
 * @brief reads a byte (offset) from script, then stores d7 byte at (vram + offset)
 * 
 */
static void sdirb(void) {
    vwrite8(alis.script->vram_org + script_read8(), (u8)alis.varD7);
}

/**
 * @brief reads a byte (offset) from script, then stores d7 word at (vram + offset)
 * 
 */
static void sdirw(void) {
    
    u8 offset = script_read8();
    vwrite16(alis.script->vram_org + offset, alis.varD7);
}

/**
 * @brief reads a byte (offset) from script, then stores null-terminated string in ARRAY_C at (vram + offset)
 * 
 */
static void sdirp(void) {
    u8 offset = script_read8();
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)get_vram(offset), (char *)alis.oldsd7);
}

static void sdirtp(void) {
    s16 offset = tabstring(script_read8(), alis.mem + alis.script->vram_org);
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)alis.oldsd7, (u8 *)alis.oldsd7 - alis.mem);
    strcpy((char *)get_vram(offset), (char *)alis.oldsd7);
}

static void sdirtc(void) {
    s16 offset = tabchar(script_read8(), alis.mem + alis.script->vram_org);
    alis.varD7 = *(alis.acc);
    alis.acc++;
    vwrite8(alis.script->vram_org + offset, alis.varD7);
}

static void sdirti(void) {
    s16 offset = tabint(script_read8(), alis.mem + alis.script->vram_org);
    alis.varD7 = *(alis.acc);
    alis.acc++;
    vwrite16(alis.script->vram_org + offset, alis.varD7);
}

static void smainb(void) {
    s16 offset = script_read16();
//    debug(EDebugWarning, " [%.2x => %.6x]", (u8)alis.varD7, alis.basemain + offset);
    vwrite8(alis.basemain + offset, (u8)alis.varD7);
}

static void smainw(void) {
    s16 offset = script_read16();
//    debug(EDebugWarning, " [%.4x => %.6x]", (s16)alis.varD7, alis.basemain + offset);
    vwrite16(alis.basemain + offset, (s16)alis.varD7);
}

static void smainp(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%s => %.6x]", (char *)alis.oldsd7, alis.basemain + offset);
    strcpy((char *)(alis.mem + alis.basemain + offset), (char *)alis.oldsd7);
}

static void smaintp(void) {
    s16 offset = tabstring(script_read16(), alis.mem + alis.basemain);
    debug(EDebugWarning, " [%s => %.6x]", (char *)alis.oldsd7, alis.basemain + offset);
    strcpy((char *)(alis.mem + alis.basemain + offset), (char *)alis.oldsd7);
}

static void smaintc(void) {
    s16 offset = tabchar(script_read16(), alis.mem + alis.basemain);
    alis.varD7 = *(alis.acc);
    alis.acc++;
    vwrite8(alis.basemain + offset, (u8)alis.varD7);
}

static void smainti(void) {
    s16 offset = tabint(script_read16(), alis.mem + alis.basemain);
    alis.varD7 = *(alis.acc);
    alis.acc++;
    vwrite16(alis.basemain + offset, (s16)alis.varD7);
}

static void shimb(void) {
    s16 offset = script_read16();
    s16 ent = vread16(alis.script->vram_org + offset);

    s32 vram = xread32(alis.atent + ent);

    s16 offset2 = script_read16();
    vwrite8(vram + offset2, (u8)alis.varD7);
}

static void shimw(void) {
    s16 offset = script_read16();
    s16 ent = vread16(alis.script->vram_org + offset);

    s32 vram = xread32(alis.atent + ent);

    s16 offset2 = script_read16();
    vwrite16(vram + offset2, (s16)alis.varD7);
}

static void shimp(void) {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimtp(void) {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimtc(void) {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimti(void) {
    debug(EDebugInfo, " /* MISSING */");
}

static void spile(void) {
    *(--alis.acc) = alis.varD7;
}

static void seval(void) {
    *(--alis.acc) = alis.varD7;
    oeval();
    readexec_storename();
}


// ============================================================================
#pragma mark - Store pointer table
// In the disasm, the table JTAB_STOREADD_NAMES is at address $10f92 -> $10fce.
// So it contains 0x3c / 60 bytes, or 30 words, it matches the storeadd count.
// So the routine FUN_READEXEC_OPERNAME() gets a BYTE code from script pc,
//  then retrieves a WORD offset at (JTAB_OPERNAMES[code]), then
//  jumps at the address (JTAB_OPERAMES + offset).
// ============================================================================
sAlisOpcode storenames[] = {
    DECL_OPCODE(0x00, cnul, "-"),
    {},
    DECL_OPCODE(0x02, cnul, "-"),
    {},
    DECL_OPCODE(0x04, cnul, "-"),
    {},
    DECL_OPCODE(0x06, slocb, "store byte from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x08, slocw, "store word from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x0a, slocp, "store null-terminated string from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x0c, sloctp, "copy bss3 at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x0e, sloctc, "pop byte from accumulator, store at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x10, slocti, "pop word from accumulator, store at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x12, sdirb, "store byte from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x14, sdirw, "store word from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x16, sdirp, "store null-terminated string from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x18, sdirtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x1a, sdirtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x1c, sdirti, "TODO add desc"),
    {},
    DECL_OPCODE(0x1e, smainb, "TODO add desc"),
    {},
    DECL_OPCODE(0x20, smainw, "TODO add desc"),
    {},
    DECL_OPCODE(0x22, smainp, "TODO add desc"),
    {},
    DECL_OPCODE(0x24, smaintp, "TODO add desc"),
    {},
    DECL_OPCODE(0x26, smaintc, "TODO add desc"),
    {},
    DECL_OPCODE(0x28, smainti, "TODO add desc"),
    {},
    DECL_OPCODE(0x2a, shimb, "TODO add desc"),
    {},
    DECL_OPCODE(0x2c, shimw, "TODO add desc"),
    {},
    DECL_OPCODE(0x2e, shimp, "TODO add desc"),
    {},
    DECL_OPCODE(0x30, shimtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x32, shimtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x34, shimti, "TODO add desc"),
    {},
    DECL_OPCODE(0x36, spile, "pops word from virtual accumulator into r7"),
    {},
    DECL_OPCODE(0x38, seval, "starts eval loop"),
    {},
    DECL_OPCODE(0x3a, ofin, "ends eval loop (calls opername/ofin)"),
};
