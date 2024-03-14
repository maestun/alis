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

#pragma once

#include "config.h"

#define kMainScriptID           (0)
#define kDepackFolder           "depack"
#define kDepackExtension        "bin"


// CONTEXT: each script has some context data, stored before vram origin
typedef struct {
    // vram - 0x33: ???
    u16 _0x34_unknown;
    
    // vram - 0x30: ???
    u8 _0x32_unknown;
    
    // vram - 0x30: ???
    u8 _0x31_unknown;
    
    // vram - 0x30: ???
    u8 _0x30_unknown;
    
    // vram - 0x2f: chsprite
    u8 _0x2f_chsprite;
    
    // vram - 0x2e: script header word 2
    u8 _0x2e_script_header_word_2;
    
    // vram - 0x2d: calign
    u8 _0x2d_calign;
    
    // vram - 0x2c: calign
    u8 _0x2c_calign;
    
    // vram - 0x2b: cordspr
    u8 _0x2b_cordspr;
    
    // vram - 0x2a: clinking OK
    u16 _0x2a_clinking;
    
    // vram - 0x28: ???
    u8 _0x28_unknown;
    
    // vram - 0x27: creducing
    u8 _0x27_creducing;
    
    // vram - 0x26: creducing
    u8 _0x26_creducing;
    
    // vram - 0x25: credon / credoff
    u8 _0x25_credon_credoff;
    
    // vram - 0x24: scan/inter bitfield
    union {
        struct {
            u8 scan_off_bit_0: 1;
            u8 inter_off_bit_1: 1;
            u8 _dummy2: 1;
            u8 _dummy3: 1;
            u8 _dummy4: 1;
            u8 _dummy5: 1;
            u8 _dummy6: 1;
            u8 scan_clr_bit_7: 1;
        };
        u8 data;
    } _0x24_scan_inter;
    
    // vram - 0x23: ???
    u8 _0x23_unknown;
    
    // vram - 0x22: cworld SHORT!?
    u8 _0x22_cworld;
    
    // vram - 0x21: cworld
    u8 _0x21_cworld;
    
    // vram - 0x20: csetvect
    u16 _0x20_set_vect;
    
    // vram - 0x1e: oscan / cscanclr
    u16 _0x1e_scan_clr;
    
    // vram - 0x1c: oscan / cscanclr
    u16 _0x1c_scan_clr;
    
    // vram - 0x1a: cforme / cdelform / cmforme
    s16 _0x1a_cforme;
    
    // vram - 0x18: ??? OK
    u16 _0x18_unknown;
    
    // vram - 0x16: screen position OK
    u16 _0x16_screen_id;
    
    // vram - 0x14: script header origin OK
    u32 _0x14_script_org_offset;
    
    // vram - 0x10: script id OK
    u16 _0x10_script_id;
    
    // vram - 0xe: czap / cexplode / cnoise OK
    u16 _0x0e_script_ent;
    
    // vram - 0xc: vacc offset OK
    u16 _0x0c_vacc_offset;
    
    // vram - 0xa: vacc offset OK
    u16 _0x0a_vacc_offset;
    
    // vram - 0x8: script return offset OK
    u32 _0x08_script_ret_offset;
    
    // vram - 0x4: cstart / csleep OK
    u8 _0x04_cstart_csleep;
    
    // vram - 0x3: xinv OK
    u8 _0x03_xinv;
    
    // vram - 0x2: ?? OK
    u8 _0x02_unknown;
    
    // vram - 0x1: cstart OK
    u8 _0x01_cstart;
} sScriptContext;

// HEADER: read from depacked script 24 BYTES
typedef struct {
    u16     id;
    u8      unknown01;       // seems to be always 0x1700 on atari, copied at (vram - $2e)
    u8      unknown02;
    u16     code_loc_offset;// where does code start from (script header + id) ? on atari, always $16
    u32     ret_offset;     // some scripts have a "sub-script" into them: this is the offset to their code location
    u32     dw_unknown3;
    u32     dw_unknown4;
    u16     w_unknown5;     // almost always 0x20 (is 0xa for "message*" and 0x4 for "objet")
    u16     vram_alloc_sz;  // number of bytes to alloc fo this script
    u16     w_unknown7;     // almost always 0x20 (is 0x4 for "message*" and "objet")
} sScriptHeader;


// =============================================================================
// MARK: - SCRIPT
// =============================================================================
typedef struct {
    char            name[kNameMaxLen];
    u32             sz;

    // read from depacked file
    sScriptHeader    header;
    
    u8              type;

    // offset to script data
    u32             data_org;
} sAlisScriptData;

typedef struct {
    
    char *          name;

    sAlisScriptData *   data;
    
    // each script has its own virtual context and memory
    u32             vram_org;
    s16             vacc_off;

    u8              running;
    u32             pc;     // offset in memory
    u32             pc_org; // offset in memory
} sAlisScriptLive;


void            script_guess_game(const char * script_path);
sAlisScriptData *script_load(const char * script_path);
void            script_unload(sAlisScriptData * script);

bool            is_delay_script(char * name);

sAlisScriptLive *script_live(sAlisScriptData * script);

// read data from script, these will increase the virtual program counter
u8              script_read8(void);
u16             script_read16(void);
u32             script_read24(void);
u32             script_read32(void);
void            script_read_bytes(u32 len, u8 * dest);
void            script_read_until_zero(char * dest);
void            script_jump(s32 offset);

// void            script_debug(sAlisScriptData * script);
s32 get_context_size(void);

u32 get_0x3e_wait_time(u32 vram);
u16 get_0x3a_wait_cycles(u32 vram);
u16 get_0x38_unknown(u32 vram);
u16 get_0x36_unknown(u32 vram);
u16 get_0x34_unknown(u32 vram);
u8 get_0x32_unknown(u32 vram);
u8 get_0x31_unknown(u32 vram);
u8 get_0x30_unknown(u32 vram);
u8 get_0x2f_chsprite(u32 vram);
u8 get_0x2e_script_header_word_2(u32 vram);
u8 get_0x2d_calign(u32 vram);
u8 get_0x2c_calign(u32 vram);
u8 get_0x2b_cordspr(u32 vram);
u16 get_0x2a_clinking(u32 vram);
u8 get_0x28_unknown(u32 vram);
u8 get_0x27_creducing(u32 vram);
u8 get_0x26_creducing(u32 vram);
u8 get_0x25_credon_credoff(u32 vram);
s8 get_0x24_scan_inter(u32 vram);
u8 get_0x23_unknown(u32 vram);
u16 get_0x22_cworld(u32 vram);
//u8 get_0x21_cworld(u32 vram);
u16 get_0x20_set_vect(u32 vram);
s16 get_0x1e_scan_clr(u32 vram);
s16 get_0x1c_scan_clr(u32 vram);
s16 get_0x1a_cforme(u32 vram);
u16 get_0x18_unknown(u32 vram);
u16 get_0x16_screen_id(u32 vram);
u32 get_0x14_script_org_offset(u32 vram);
u16 get_0x10_script_id(u32 vram);
u16 get_0x0e_script_ent(u32 vram);
s16 get_0x0c_vacc_offset(u32 vram);
s16 get_0x0a_vacc_offset(u32 vram);
u32 get_0x08_script_ret_offset(u32 vram);
u8 get_0x04_cstart_csleep(u32 vram);
u8 get_0x03_xinv(u32 vram);
u8 get_0x02_wait_cycles(u32 vram);
u8 get_0x01_wait_count(u32 vram);

void set_0x3e_wait_time(u32 vram, u32 val);
void set_0x3a_wait_cycles(u32 vram, u16 val);
void set_0x38_unknown(u32 vram, u16 val);
void set_0x36_unknown(u32 vram, u16 val);
void set_0x34_unknown(u32 vram, u16 val);
void set_0x32_unknown(u32 vram, u8 val);
void set_0x31_unknown(u32 vram, u8 val);
void set_0x30_unknown(u32 vram, u8 val);
void set_0x2f_chsprite(u32 vram, u8 val);
void set_0x2e_script_header_word_2(u32 vram, u8 val);
void set_0x2d_calign(u32 vram, u8 val);
void set_0x2c_calign(u32 vram, u8 val);
void set_0x2b_cordspr(u32 vram, u8 val);
void set_0x2a_clinking(u32 vram, u16 val);
void set_0x28_unknown(u32 vram, u8 val);
void set_0x27_creducing(u32 vram, u8 val);
void set_0x26_creducing(u32 vram, u8 val);
void set_0x25_credon_credoff(u32 vram, u8 val);
void set_0x24_scan_inter(u32 vram, s8 val);
void set_0x23_unknown(u32 vram, u8 val);
void set_0x22_cworld(u32 vram, u16 val);
//void set_0x21_cworld(u32 vram, u8 val);
void set_0x20_set_vect(u32 vram, u16 val);
void set_0x1e_scan_clr(u32 vram, s16 val);
void set_0x1c_scan_clr(u32 vram, s16 val);
void set_0x1a_cforme(u32 vram, s16 val);
void set_0x18_unknown(u32 vram, u16 val);
void set_0x16_screen_id(u32 vram, u16 val);
void set_0x14_script_org_offset(u32 vram, u32 val);
void set_0x10_script_id(u32 vram, u16 val);
void set_0x0e_script_ent(u32 vram, u16 val);
void set_0x0c_vacc_offset(u32 vram, s16 val);
void set_0x0a_vacc_offset(u32 vram, s16 val);
void set_0x08_script_ret_offset(u32 vram, u32 val);
void set_0x04_cstart_csleep(u32 vram, u8 val);
void set_0x03_xinv(u32 vram, u8 val);
void set_0x02_wait_cycles(u32 vram, u8 val);
void set_0x01_wait_count(u32 vram, u8 val);
