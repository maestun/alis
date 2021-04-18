//
//  addnames.c
//  aodec
//
//  Created by zlot on 05/02/2020.
//  Copyright © 2020 zlot. All rights reserved.
//

#include "alis.h"
#include "alis_private.h"

// ============================================================================
#pragma mark - Addnames routines
// ============================================================================
static void cnul() {
}
static void alocb() {
    u16 offset = script_read16();
    vram_add8(offset, (u8)alis.varD7);
    alis.sr.zero = (vram_read8(offset) == 0);
}
static void alocw() {
    u16 offset = script_read16();
    vram_add16(offset, alis.varD7);
    alis.sr.zero = (vram_read8(offset) == 0);
}
static void alocp() {
//    ADDNAME_ALOCP_0x5
//000181ac 10 1b           move.b     (A3)+,D0b
//000181ae e1 40           asl.w      #0x8,D0w
//000181b0 10 1b           move.b     (A3)+,D0b
    u16 offset = script_read16();
//000181b2 43 f6 00 00     lea        (0x0,A6,D0w*0x1),A1
    u8 * a1 = vram_ptr(offset);
//000181b6 20 79 00        movea.l    (ADDR_BSS_256_CHUNK_3).l,A0
//01 95 ea
    u8 * a0 = alis.bssChunk3;
//    LAB_000181bc                                    XREF[1]:     000181be(j)
//000181bc 4a 19           tst.b      (A1)+
//000181be 66 00 ff fc     bne.w      LAB_000181bc
    while (*a1 == 0) {
        a1++;
    }
//000181c2 53 89           subq.l     #0x1,A1
    a1--;
//    LAB_000181c4                                    XREF[1]:     000181c6(j)
//000181c4 12 d8           move.b     (A0)+,(A1)+
//000181c6 66 00 ff fc     bne.w      LAB_000181c4
    while (*a0) {
        *a1++ = *a0++;
    }
//000181ca 4e 75           rts
}
static void aloctp() {
    debug(EDebugInfo, "aloctp STUBBED\n");
}
static void aloctc() {
    debug(EDebugInfo, "aloctc STUBBED\n");
}
static void alocti() {
    debug(EDebugInfo, "alocti STUBBED\n");
}
static void adirb() {
    u8 offset = script_read8();
    vram_add8(offset, (u8)alis.varD7);
    alis.sr.zero = (vram_read8(offset) == 0);
}
static void adirw() {
    u8 offset = script_read8();
    vram_add16(offset, alis.varD7);
    alis.sr.zero = (vram_read8(offset) == 0);
}
static void adirp() {

}
static void adirtp() {
    debug(EDebugInfo, "adirtp STUBBED\n");
}
static void adirtc() {
    debug(EDebugInfo, "adirtc STUBBED\n");
}
static void adirti() {
    debug(EDebugInfo, "adirti STUBBED\n");
}
static void amainb() {
//    ADDNAME_AMAINB_0xf
//00018288 10 1b           move.b     (A3)+,D0b
//0001828a e1 40           asl.w      #0x8,D0w
//0001828c 10 1b           move.b     (A3)+,D0b
//0001828e 22 79 00        movea.l    (ADDR_VSTACK).l,A1
//01 95 7c
//00018294 df 31 00 00     add.b      D7b,(0x0,A1,D0w*0x1)
//00018298 4e 75           rts
}
static void amainw() {
//    ADDNAME_AMAINW_0x10
//0001829a 10 1b           move.b     (A3)+,D0b
//0001829c e1 40           asl.w      #0x8,D0w
//0001829e 10 1b           move.b     (A3)+,D0b
//000182a0 22 79 00        movea.l    (ADDR_VSTACK).l,A1
//01 95 7c
//000182a6 df 71 00 00     add.w      D7w,(0x0,A1,D0w*0x1)
//000182aa 4e 75           rts
}
static void amainp() {
    // log_debug("STUBBED");
}
static void amaintp() {
    // log_debug("STUBBED");
}
static void amaintc() {
    // log_debug("STUBBED");
}
static void amainti() {
    // log_debug("STUBBED");
}
static void ahimb() {
    // log_debug("STUBBED");
}
static void ahimw() {
    // log_debug("STUBBED");
}
static void ahimp() {
    // log_debug("STUBBED");
}
static void ahimtp() {
    // log_debug("STUBBED");
}
static void ahimtc() {
    // log_debug("STUBBED");
}
static void ahimti() {
    // log_debug("STUBBED");
}
static void spile() {
    // log_debug("STUBBED");
}
static void aeval() {
//    ADDNAME_AEVAL_0x1c
//0001843c 39 07           move.w     D7w,-(A4)
//0001843e 61 00 fa c6     bsr.w      OPERNAME_OEVAL_0x1c                              undefined OPERNAME_OEVAL_0x1c()
//00018442 41 f9 00        lea        (JTAB_ADDNAMES).l,A0                             =
//01 2f 6a
//00018448 10 1b           move.b     (A3)+,D0b
//0001844a 48 80           ext.w      D0w
//0001844c 30 30 00 00     move.w     (0x0,A0,D0w*offset JTAB_ADDNAMES),D0w            =
//00018450 4e f0 00 00     jmp        (0x0,A0,D0w*0x1)
    
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
    DECL_OPCODE(0x06, alocb, "TODO add desc"),
    {},
    DECL_OPCODE(0x08, alocw, "TODO add desc"),
    {},
    DECL_OPCODE(0x0a, alocp, "TODO add desc"),
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
