//
//  disasm.c
//  alis
//
//  Created by developer on 05/05/2021.
//  Copyright © 2021 Zlot. All rights reserved.
//

#include "disasm.h"
#include "alis_private.h"


void print_disasm_line(sDisasmLine * line) {
    
}


void disasm_op(sAlisOpcode * table) {
    // fetch code
    u8 code = *(alis.mem + alis.script->pc++);
    sAlisOpcode opcode = table[code];
    debug(EDebugVerbose, " %s", opcode.name);
    return opcode.fptr();
}


void disasm(char * path) {
    alis.disasm = 1;
    alis.script = script_load(path);
    debug(EDebugVerbose, "\n; begin disasm of %s", alis.script->name);
    
    while(alis.script->pc < alis.script->data_org + alis.script->sz) {
        debug(EDebugVerbose, "\n0x%06x / 0x%06x",
              alis.script->pc - alis.script->data_org,
              alis.script->pc);
        disasm_op(opcodes);
    }
}
