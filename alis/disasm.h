//
//  disasm.h
//  alis
//
//  Created by developer on 05/05/2021.
//  Copyright © 2021 Zlot. All rights reserved.
//

#ifndef disasm_h
#define disasm_h

#include "alis.h"
#include "utils.h"


typedef struct {
    u32     offset;
    u32     address;
    char    label[kNameMaxLen];
    u8      bytes[4096];
    char    code[4096];
    char    comment[4096];
} sDisasmLine;

void disasm(char * path);
void disasm_op(sAlisOpcode * table);

#endif /* disasm_h */
