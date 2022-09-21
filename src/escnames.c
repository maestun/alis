//
//  escnames.c
//  aodec
//
//  Created by zlot on 05/02/2020.
//  Copyright © 2020 zlot. All rights reserved.
//

#include "alis.h"
#include "alis_private.h"

// ============================================================================
#pragma mark - Escnames routines
// ============================================================================
static void cnul(void) {
}
void consound(void) {
    debug(EDebugWarning, " /* MISSING */");
}
void coffsound(void) {
    debug(EDebugWarning, " /* MISSING */");
}
void conmusic(void) {
    debug(EDebugWarning, " /* MISSING */");
}
void coffmusic(void) {
    debug(EDebugWarning, " /* MISSING */");
}
void cdelfilm(void) {
    debug(EDebugWarning, " /* MISSING */");
}
void copenfilm(void) {
    debug(EDebugWarning, " /* MISSING */");
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
