//
//  script.h
//  alis
//

#ifndef script_h
#define script_h

#include "config.h"

#define kDepackFolder           "depack"
#define kDepackExtension        "bin"


typedef struct __attribute__((packed)) {
    u32 magic_len;
    u16 script_id;
} sPackedScriptHeader;


// CONTEXT: each script has some context data, stored before vram origin
// for main: all the vars are stored in reverse ordef BEFORE basemain ($22690)
// should be 52 bytes (0x34) long
typedef struct {
    // vram - 0x34: ???
    u16 _0x34_unknown;
    
    // vram - 0x32: ???
    u16 _0x32_unknown;
    
    // vram - 0x30: ???
    u8 _0x30_unknown;
    
    // vram - 0x2f: chsprite
    u8 _0x2f_chsprite;
    
    // vram - 0x2e: script header byte 2
    u8 _0x2e_script_header_byte_2;
    
    // vram - 0x2d: calign
    u8 _0x2d_calign;
    
    // vram - 0x2c: calign
    u8 _0x2c_calign;
    
    // vram - 0x2b: cordspr
    u8 _0x2b_cordspr;
    
    // vram - 0x2a: clinking
    u8 _0x2a_clinking;
    
    // vram - 0x28: ???
    u8 _0x28_unknown;
    
    // vram - 0x27: creducing
    u8 _0x27_creducing;
    
    // vram - 0x26: creducing
    u8 _0x26_creducing;
    
    // vram - 0x25: credon / credoff
    u8 _0x25_credon_credoff;
    
    // vram - 0x24: scan/inter bitfield
    u8 _0x24_scan_inter;
    
    // vram - 0x23: ???
    u8 _0x23_unknown;
    
    // vram - 0x22: cworld
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
    
    // vram - 0x18: ???
    u16 _0x18_unknown;
    
    // vram - 0x16: screen position
    u16 _0x16_screen_position;
    
    // vram - 0x14: script data origin (pointer to header)
    u32 _0x14_script_org_offset;
    
    // vram - 0x10: script id
    u16 _0x10_script_id;
    
    // vram - 0xe: czap / cexplode / cnoise
    u16 _0xe_czap_cexplode_cnoise;
    
    // vram - 0xc: vacc offset
    u16 _0xc_vacc_offset;
    
    // vram - 0xa: vacc offset
    u16 _0xa_vacc_offset;
    
    // vram - 0x8: script return offset
    u32 _0x8_script_ret_offset;
    
    // vram - 0x4: cstart / csleep
    u8 _0x4_cstart_csleep;
    
    // vram - 0x3: xinv
    u8 _0x3_xinv;
    
    // vram - 0x2: ??
    u8 _0x2_unknown;
    
    // vram - 0x1: cstart
    u8 _0x1_cstart;
} sScriptContext;

// HEADER: read from depacked script 24 BYTES
typedef struct {
    u16     id;
    u8      b_0x17;                   // seems to be always 0x17 on atari, copied at (vram - $2e)
    u8      b_0x00;                   // seems to be always 0x00 on atari
    u16     code_start_offset_minus_2;  // where does code start from (script header + id) ? on atari, always $16
    u32     offset_to_subscript_routine;     // some scripts have a "sub-script" into them: this is the offset to their code location
    u32     offset_to_interrupt_routine;
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
    sScriptHeader   header;

    // offset to script data
    u32             data_org;
    
    // offset to script code
    u32             code_org;
    
    // TODO: each script has its own virtual context and memory ??
    //u32             vram_org;
    u16             vacc_off;
    
    sScriptContext  context;
    
    u8              running;
    
    u32             pc;     // offset in memory
} sAlisScript;


sAlisScript *   script_load(const char * script_path);
void            script_unload(sAlisScript * script);

// read data from script, these will increase the virtual program counter
u8              script_read8(void);
s16             script_read8ext16(void);
s32             script_read8ext32(void);
u16             script_read16(void);
s32             script_read16ext32(void);
u32             script_read24(void);
void            script_read_bytes(u32 len, u8 * dest);
void            script_read_until_zero(u8 * dest);
void            script_jump(s32 offset);

void            script_debug(sAlisScript * script);
#endif /* script_h */
