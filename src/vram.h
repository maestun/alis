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

#define kVirtualACCSize         (64 * sizeof(u8))

// data berfore bamsemain size
#define kVMStatusSize           (sizeof(sVMStatus) * sizeof(u8))


typedef struct {
    // offset 0
    u16 cx; // copy into oldcx in savecoord()
    // offset 2
    u16 cy; // copy into oldcy in savecoord()
    // offset 4
    u16 cz; // copy into oldcz in savecoord()
    
} sMemoryMapAfterBasemain;

// =============================================================================
// MARK: - VRAM OFFSETS
// Some data is stored *before* vram address
// =============================================================================

// all the vars stored in reverse ordef BEFORE basemain ($22690)
// should be kVMStatusSize long
typedef struct {
    
    u16 kOffsetMinus52;
    u16 kOffsetMinus50;
    u8 kOffsetMinus48;

    //#define VRAM_OFFSET_CHSPRITE                 (-0x2f)
    u8 chsprite_kOffsetMinus47;
    //#define VRAM_OFFSET_UNKNOWN_1                (-0x2e)
    u8 unknown1_kOffsetMinus46;
    //#define VRAM_OFFSET_CALIGN_1                 (-0x2d)
    u8 calign_1_kOffsetMinus45;
    //#define VRAM_OFFSET_CALIGN_2                 (-0x2c)
    u8 calign_2_kOffsetMinus44;
    //#define VRAM_OFFSET_CORDSPR                  (-0x2b)
    u8 cordspr_kOffsetMinus43;
    //#define VRAM_OFFSET_CLINKING                 (-0x2a)
    u16 clinking_kOffsetMinus42;
    //#define VRAM_OFFSET_UNKNOWN_2                (-0x28)
    u8 unknown2_kOffsetMinus40;
    //#define VRAM_OFFSET_CREDUCING_1              (-0x27)
    u8 creducing_1_kOffsetMinus39;
    //#define VRAM_OFFSET_CREDUCING_2              (-0x26)
    u8 creducing_2_kOffsetMinus38;
    //#define VRAM_OFFSET_CRED_ON_OFF              (-0x25)
    u8 cred_on_off_kOffsetMinus37;
    //#define VRAM_OFFSET_CSCAN_CINTER             (-0x24)
    u8 cscan_cinter_kOffsetMinus36;
    //#define VRAM_OFFSET_UNKNOWN_3                (-0x23)
    u8 unknown3_kOffsetMinus35;
    //#define VRAM_OFFSET_CWORLD_1                 (-0x22)
    u8 cworld1_kOffsetMinus34;
    //#define VRAM_OFFSET_CWORLD_2                 (-0x21)
    u8 cworld2_kOffsetMinus33;
    //#define VRAM_OFFSET_CSETVECT                 (-0x20)
    u16 csetvect_kOffsetMinus32;
    //#define VRAM_OFFSET_OSCAN_OSCANCLR_1         (-0x1e)
    u16 oscan_oscanclr_1_kOffsetMinus30;
    //#define VRAM_OFFSET_OSCAN_OSCANCLR_2         (-0x1c)
    u16 oscan_oscanclr_2_kOffsetMinus28;
    //#define VRAM_OFFSET_CFORME_CDELFORME         (-0x1a)
    u16 cforme_delforme_kOffsetMinus26;
    //#define VRAM_OFFSET_UNKNOWN_4                (-0x18)
    u16 unknown4_kOffsetMinus24;
    //#define VRAM_OFFSET_AO_SC_POSITION           (-0x16)
    u16 ao_sc_position_kOffsetMinus22;
    //#define VRAM_OFFSET_SCRIPT_ORG_ADDR          (-0x14)
    u32 script_data_addr_kOffsetMinus20;
    //#define VRAM_OFFSET_UNKNOWN_5                (-0x10)
    u16 unknown5_kOffsetMinus16;
    //#define VRAM_OFFSET_CZAP_CEXPLODE            (-0x0e)
    u16 czap_cexplode_kOffsetMinus14;
    //#define VRAM_OFFSET_VACC_OFFSET_D4           (-0x0c)
    u16 vacc_offset_d4_kOffsetMinus12;
    //#define VRAM_OFFSET_VACC_OFFSET              (-0x0a)
    u16 vacc_offset_kOffsetMinus10;
    //#define VRAM_OFFSET_SCRIPT_PTR_ADDR          (-0x08)
    u32 script_code_addr_kOffsetMinus8;
    //#define VRAM_OFFSET_CSTART_CSLEEP            (-0x04)
    u8 cstart_csleep_kOffsetMinus4;
    //#define VRAM_OFFSET_CXINV                    (-0x03)
    u8 cxyinv_kOffsetMinus3;
    //#define VRAM_OFFSET_UNKNOWN_6                (-0x02)
    u8 unknown6_kOffsetMinus2;
    //#define VRAM_OFFSET_CSTART                   (-0x01)
    u8 cstart_kOffsetMinus1;
} sVMStatus;


//typedef struct {
//    // virtual ram
//    u8 *    vram;                   // host: a6 register
//    
//    // virtual accumulator
//    u8      vacc[kVirtualACCSize];  // host: a4 register
//    u16     vacc_offset;            // host: d4 register
//} sVRAM;


u8 *    vram_ptr(u16 offset);

u8      vram_read8(s32 offset);
s16     vram_read8ext16(s32 offset);
s32     vram_read8ext32(s32 offset);
u16     vram_read16(s32 offset);
s32     vram_read16ext32(s32 offset);
u32     vram_read32(s32 offset);
void    vram_readp(s32 offset, u8 * dst_ptr);

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
