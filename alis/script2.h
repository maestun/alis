//
//  script2.h
//  alis
//
//  Created by developer on 15/04/2021.
//  Copyright © 2021 Zlot. All rights reserved.
//

#ifndef script2_h
#define script2_h

#include "config.h"



typedef struct {
    // identifier
    u16     id;
    char    name[kNameMaxLen];
    
    // memory
    u8      vram[0xffff];
    u16     vacc_offset;
    
    // status
    u8      status[0x2f];
    
    // data
    u8 *    data;
    u8 *    code;
    
} sScript;
u8              script_is_main(sScript * aScript);
u8              script_read_vram(sScript * aScript, s32 offset);
sScript *       script_load(u16 aID, char * aDataPath);

#endif /* script2_h */
