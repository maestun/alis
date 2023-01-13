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



// 'basemem'
// Absolute address of vm's virtual ram.
// On Steem emulator, the operating system gives us $22400.
#define ALIS_VM_RAM_ORG (0x22400)




// all these are offsets !
// for ishar 1 data w/ ishar2 interpreter:
typedef struct {

    // malloc'd
    u8* ram;
    
    u32 base;       // $22400, set once by host system's 'malloc'
//    u32 basevar;       // $0
    
    // address of a dword table that holds the adresses of loaded script data
    // (max size is read in packed main script's header, at offset 0x6)
    u32 atprog;        // $22400, set once, same as basemem
    
    // address of the next available atprog slot
    u32 dernprog;      // $22400, will increment by 4 each time a script is loaded
    
    u32 atent;          // $224f0, set once: atent = basemem + (vm.specs.maxprog * sizeof(ptr_t))
                        // atent ent contains 6*MAXENT (word read in MAIN.AO header at byte 0x8)
    u32 debent;         // $2261c, set once: debent = atent + (vm.specs.maxent * 6)

    u32 basemain;       // $22690, set once: basemain = debent + vmspecs[0x12] + vmspecs[0x16] + sizeof(struct sVMStat)

    u32 finent;        // $26902

    u32 debsprit;       // $283e0, set once: debsprit = ((atent + vm.specs.debsprit_offset) & 0x0f) + 1
    u32 finsprit;       // $2d278, set once: finsprit = debsprit + ((vm.specs.finsprit_offset + 3) * 40)
    u32 basesprit;      // $31f40, set at init_sprites()
    u16 tvsprit;        // 0x8000, set at init_sprites()
    u16 backsprit;      // 0x8028, set at init_sprites()
    u16 texsprit;       // 0x8050, set at init_sprites()

    u32 debprog;       // $2d278, set once: debprog = finsprit
    u32 finprog;       // $

    u32 finmem;         // end of host memory

} sAlisMemory;


void    vram_init(void);
void    vram_free(void);

u8 *    vram_ptr(u32 offset);

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
