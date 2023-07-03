//
//  vram->c
//  alis
//
//  Created by developer on 23/11/2020.
//  Copyright © 2020 Zlot. All rights reserved.
//

#include "vram.h"

//sVRAM * vram_init() {
//    
//
//    
//    sVRAM * vram = (sVRAM *)malloc(sizeof(sVRAM));
//    memset(vram->ram, 0, kVirtualRAMSize * sizeof(u8));
//    vram->stack_offset = 0xffac;//kVirtualRAMSize; // TODO: ????
//    return vram;
//}

//void vram_deinit(sVRAM * vram) {
//    free(vram);
//}


// =============================================================================
// MARK: - Virtual RAM access
// =============================================================================
u8 * vram_ptr(sVRAM * vram, u16 offset) {
    return (u8 *)(vram->ram + offset);
}

u8 vram_read8(sVRAM * vram, u16 offset) {
    return *(u8 *)(vram->ram + offset);
}

u16 vram_read16(sVRAM * vram, u16 offset) {
    return *(u16 *)(vram->ram + offset);
}

void vram_readp(sVRAM * vram, u16 offset, u8 * dst_ptr) {
    u8 * src_ptr = vram->ram + offset;
    do {
        *dst_ptr++ = *src_ptr++;
    } while (*src_ptr);
}

void vram_write8(sVRAM * vram, u16 offset, u8 value) {
    *(u8 *)(vram->ram + offset) = value;
}

void vram_write16(sVRAM * vram, u16 offset, u16 value) {
    *(u16 *)(vram->ram + offset) = value;
}

void vram_writep(sVRAM * vram, u16 offset, u8 * src_ptr) {
    u8 * dst_ptr = vram->ram + offset;
    do {
        *dst_ptr++ = *src_ptr++;
    } while (*src_ptr);
}

void vram_setbit(sVRAM * vram, u16 offset, u8 bit) {
    BIT_SET(*(u8 *)(vram->ram + offset), bit);
}

void vram_clrbit(sVRAM * vram, u16 offset, u8 bit) {
    BIT_CLR(*(u8 *)(vram->ram + offset), bit);
}

void vram_add8(sVRAM * vram, u16 offset, u8 value) {
    *(u8 *)(vram->ram + offset) += value;
}

void vram_add16(sVRAM * vram, u16 offset, u16 value) {
    *(u16 *)(vram->ram + offset) += value;
}


// =============================================================================
// MARK: - Virtual Stack access
// =============================================================================
void vram_push32(sVRAM * vram, u32 value) {
    vram->stack_offset -= sizeof(u32);
    *(u32 *)(vram->ram + vram->stack_offset) = value;
}

u32 vram_peek32(sVRAM * vram) {
    return *(u32 *)(vram->ram + vram->stack_offset);
}

u32 vram_pop32(sVRAM * vram) {
    u32 ret = vram_peek32(vram);
    vram->stack_offset += sizeof(u32);
    return ret;
}

void vram_save_script_state(sVRAM * vram, sAlisScript * script) {
    
}


// =============================================================================
// MARK: - Virtual RAM debug
// =============================================================================
void vram_debug(sVRAM * vram) {
    u8 width = 16;
    
    printf("Stack Offset=0x%04x (word=0x%04x) (dword=0x%08x)\n",
           vram->stack_offset,
           (u16)(*(vram->ram + vram->stack_offset)),
           (u32)(*(vram->ram + vram->stack_offset)));

    printf("Virtual RAM:\n");
    
    printf("       ");
    for(u8 j = 0; j < width; j++) {
        printf("  %x", j);
    }
    printf("\n");
    
    for(u32 i = 0; i < kVirtualRAMSize; i += width) {
        printf("0x%04x: ", i);
        for(u8 j = 0; j < width; j++) {
            printf("%02x ", vram->ram[i + j]);
        }
        printf("\n");
    }
}

void vram_debug_addr(sVRAM * vram, u16 addr) {
    u8 width = 16;
    printf("Virtual RAM:\n");
    
    printf("       ");
    for(u8 j = 0; j < width; j++) {
        printf("  %x", j);
    }
    printf("\n");
    printf("0x%04x: ", addr);
    for(u32 j = addr; j < addr + width; j++) {
        printf("%02x ", vram->ram[j]);
    }
}
