//
//  vram.h
//  alis
//
//  Created by developer on 23/11/2020.
//  Copyright © 2020 Zlot. All rights reserved.
//

#ifndef vram_h
#define vram_h

#include "config.h"
#include "script.h"

// #define kHostRAMSize            (0x100000 * sizeof(u8))
// #define kHostVRAMSize           (0x8000 * sizeof(u8))
#define kVirtualRAMSize         (0x10000 * sizeof(u8))

// =============================================================================
// MARK: - VRAM OFFSETS
// Some data is stored *before* vram address
// =============================================================================
//#define VRAM_OFFSET_CHSPRITE                 (-0x2f)
//#define VRAM_OFFSET_UNKNOWN_1                (-0x2e)
//#define VRAM_OFFSET_CALIGN_1                 (-0x2d)
//#define VRAM_OFFSET_CALIGN_2                 (-0x2c)
//#define VRAM_OFFSET_CORDSPR                  (-0x2b)
//#define VRAM_OFFSET_CLINKING                 (-0x2a)
//#define VRAM_OFFSET_UNKNOWN_2                (-0x28)
//#define VRAM_OFFSET_CREDUCING_1              (-0x27)
//#define VRAM_OFFSET_CREDUCING_2              (-0x26)
//#define VRAM_OFFSET_CRED_ON_OFF              (-0x25)
//#define VRAM_OFFSET_CSCAN_CINTER             (-0x24)
//#define VRAM_OFFSET_UNKNOWN_3                (-0x23)
//#define VRAM_OFFSET_CWORLD_1                 (-0x22)
//#define VRAM_OFFSET_CWORLD_2                 (-0x21)
//#define VRAM_OFFSET_CSETVECT                 (-0x20)
//#define VRAM_OFFSET_OSCAN_OSCANCLR_1         (-0x1e)
//#define VRAM_OFFSET_OSCAN_OSCANCLR_2         (-0x1c)
//#define VRAM_OFFSET_CFORME_CDELFORME         (-0x1a)
//#define VRAM_OFFSET_UNKNOWN_4                (-0x18)
//#define VRAM_OFFSET_AO_SC_POSITION           (-0x16)
//#define VRAM_OFFSET_SCRIPT_ORG_ADDR          (-0x14)
//#define VRAM_OFFSET_UNKNOWN_5                (-0x10)
//#define VRAM_OFFSET_CZAP_CEXPLODE            (-0x0e)
//#define VRAM_OFFSET_VACC_OFFSET_D4           (-0x0c)
//#define VRAM_OFFSET_VACC_OFFSET              (-0x0a)
//#define VRAM_OFFSET_SCRIPT_PTR_ADDR          (-0x08)
//#define VRAM_OFFSET_CSTART_CSLEEP            (-0x04)
//#define VRAM_OFFSET_CXINV                    (-0x03)
//#define VRAM_OFFSET_UNKNOWN_6                (-0x02)
//#define VRAM_OFFSET_CSTART                   (-0x01)


typedef struct {
    // virtual ram
    u8 *    vram;               // host: a6 register
    
    // virtual accumulator
    u8 *    vacc;               // host: a4 register
    u16     vacc_offset;        // host: d4 register
} sVRAM;


u8 *    vram_ptr(u16 offset);

u8      vram_read8(s32 offset);
s16     vram_read8ext16(u16 offset);
s32     vram_read8ext32(u16 offset);
u16     vram_read16(s32 offset);
s32     vram_read16ext32(u16 offset);
u32     vram_read32(u16 offset);
void    vram_readp(u16 offset, u8 * dst_ptr);

void    vram_write8(s32 offset, u8 value);
void    vram_write16(s32 offset, u16 value);
void    vram_write32(s32 offset, u32 value);
void    vram_writep(u16 offset, u8 * src_ptr);

void    vram_setbit(u16 offset, u8 bit);
void    vram_clrbit(u16 offset, u8 bit);

void    vram_add8(u16 offset, u8 value);
void    vram_add16(u16 offset, u16 value);

void    vram_push32(u32 value);
u32     vram_peek32(void);
u32     vram_pop32(void);

void    vram_save_script_state(sAlisScript * script);

void    vram_debug(void);

#endif /* vram_h */
