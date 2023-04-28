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

// ============================================================================
#pragma mark - Store Routines
// ============================================================================

static void cnul() {
}

/**
 * @brief reads a word (offset) from script, then stores d7 byte at (vram + offset)
 * 
 */
static void slocb() {
    vram_write8(script_read16(), (u8)alis.varD7);
}

/**
 * @brief reads a word (offset) from script, then stores d7 word at (vram + offset)
 * 
 */
static void slocw() {
    vram_write16(script_read16(), alis.varD7);
}

/**
 * @brief reads a word (offset) from script, then stores null-terminated string in ARRAY_C at (vram + offset)
 * 
 */
static void slocp() {
    vram_writep(script_read16(), alis.bsd7bis);
}

// Store at LOCation with offseT: Pointer
static void sloctp() {
    u16 offset = loctp_common(script_read16());
    u8 * ptr = alis.bsd7bis;
    while (*ptr) {
        vram_write8(offset++, *ptr++);
    }
}

// Store at LOCation with offseT: Char
static void sloctc() {
    u16 offset = loctc_common(script_read16());
    alis.varD7 = *(alis.acc++);
    vram_write8(offset, alis.varD7);
}

// Store at LOCation with offseT: Int
static void slocti() {
    u16 offset = locti_common(script_read16());
    alis.varD7 = *(alis.acc++);
    vram_write16(offset, alis.varD7);
}


/**
 * @brief reads a byte (offset) from script, then stores d7 byte at (vram + offset)
 * 
 */
static void sdirb() {
    vram_write8(script_read8(), (u8)alis.varD7);
}

/**
 * @brief reads a byte (offset) from script, then stores d7 word at (vram + offset)
 * 
 */
static void sdirw() {
    
    u8 offset = script_read8();
//    printf("\nXXX sdirw: 0x%.6x > 0x%.2x\n", (u16)alis.varD7, offset);
    vram_write16(offset, (u16)alis.varD7);
}

/**
 * @brief reads a byte (offset) from script, then stores null-terminated string in ARRAY_C at (vram + offset)
 * 
 */
static void sdirp() {
    vram_writep(script_read8(), alis.bsd7bis);
}

static void sdirtp() {
    debug(EDebugInfo, " /* MISSING */");
}

static void sdirtc() {
    debug(EDebugInfo, " /* MISSING */");
}

static void sdirti() {
    debug(EDebugInfo, " /* MISSING */");
}

static void smainb() {
    debug(EDebugInfo, " /* CHECK */");

    s16 offset = script_read16();
    *(u8 *)(alis.mem + alis.basemain + offset) = (u8)alis.varD7;
}

static void smainw() {
    debug(EDebugInfo, " /* CHECK */");

    s16 offset = script_read16();
    *(u16 *)(alis.mem + alis.basemain + offset) = (u16)alis.varD7;
}

static void smainp() {
    debug(EDebugInfo, " /* MISSING */");
}

static void smaintp() {
    debug(EDebugInfo, " /* MISSING */");
}

static void smaintc() {
    debug(EDebugInfo, " /* MISSING */");
}

static void smainti() {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimb() {
    debug(EDebugInfo, " /* CHECK */");
    
    s16 offset = script_read16();
    s16 ent = vram_read16(offset);

    u32 index = *(u32 *)(alis.mem + alis.atent + ent);

    s16 offset2 = script_read16();
    alis.varD7 = *(u8 *)(alis.mem + alis.scripts[index]->vram_org + offset2);
}

static void shimw() {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimp() {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimtp() {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimtc() {
    debug(EDebugInfo, " /* MISSING */");
}

static void shimti() {
    debug(EDebugInfo, " /* MISSING */");
}

static void spile() {
    *(--alis.acc) = alis.varD7;
}

static void seval() {
//    00015d2c 39 07           move.w     D7w,-(A4)
//    00015d2e 61 00 fd 86     bsr.w      FUN_READEXEC_OPNAME                              undefined FUN_READEXEC_OPNAME()
//                         -- Flow Override: CALL_RETURN (CALL_TERMINATOR)
//    00015d32 41 f9 00        lea        (JTAB_STORENAME).l,A0
//             01 0f 92
//    00015d38 10 1b           move.b     (A3)+,D0b
//    00015d3a 48 80           ext.w      D0w
//    00015d3c 30 30 00 00     move.w     (0x0,A0,D0w*offset JTAB_STORENAME),D0w
//    00015d40 4e f0 00 00     jmp        (0x0,A0,D0w*0x1)

    
    // save r7 to virtual accumulator
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
