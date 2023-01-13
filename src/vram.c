//
//  vram.c
//  alis
//


#include "vram.h"




// =============================================================================
// MARK: - MEMORY ACCESS
// =============================================================================

u8 * vram_ptr(u16 offset) {
    return (u8 *)(vm.mem + /*vm.script->vram_org*/ + offset);
}

u8 vram_read8(s32 offset) {
    return *(u8 *)(vm.mem + /*vm.script->vram_org*/ + offset);
}

s16 vram_read8ext16(s32 offset) {
    s16 ret = *(u8 *)(vm.mem + /*vm.script->vram_org +*/ offset);
    if(BIT_CHK(ret, 7)) {
        ret |= 0xff00;
    }
    return ret;
}

s32 vram_read8ext32(s32 offset) {
    s32 ret = *(u8 *)(vm.mem + /*vm.script->vram_org +*/ offset);
    if(BIT_CHK(ret, 7)) {
        ret |= 0xffffff00;
    }
    return ret;
}

u16 vram_read16(s32 offset) {
    return *(u16 *)(vm.mem + /*vm.script->vram_org +*/ offset);
}

u32 vram_read32(s32 offset) {
    return *(u32 *)(vm.mem + offset);
}

s32 vram_read16ext32(s32 offset) {
    s32 ret = *(u16 *)(vm.mem + /*vm.script->vram_org +*/ offset);
    if(BIT_CHK(ret, 15)) {
        ret |= 0xffffff00;
    }
    return ret;
}

void vram_readp(s32 offset, u8 * dst_ptr) {
    u8 * src_ptr = vm.mem + /*vm.script->vram_org +*/ offset;
    do {
        *dst_ptr++ = *src_ptr++;
    } while (*src_ptr);
}

void vram_write8(s32 offset, u8 value) {
    *(u8 *)(vm.mem /*+ vm.script->vram_org*/ + offset) = value;
    debug(EDebugVerbose, "\nwrote char 0x%02x at vram offset 0x%06x", value, offset);
}

void vram_write16(s32 offset, u16 value) {
    *(u16 *)(vm.mem /*+ vm.script->vram_org*/ + offset) = value;
    debug(EDebugVerbose, "\nwrote word 0x%04x at vram offset 0x%06x", value, offset);
}

void vram_write32(s32 offset, u32 value) {
    *(u32 *)(vm.mem /*+ vm.script->vram_org*/ + offset) = value;
    debug(EDebugVerbose, "\nwrote dword 0x%08x at vram offset 0x%06x", value, offset);
}

void vram_writep(u16 offset, u8 * src_ptr) {
    u8 * dst_ptr = vm.mem + /*vm.script->vram_org +*/ offset;
    do {
        *dst_ptr++ = *src_ptr++;
    } while (*src_ptr);
}

void vram_setbit(u16 offset, u8 bit) {
    BIT_SET(*(u8 *)(vm.mem + /*vm.script->vram_org +*/ offset), bit);
}

void vram_clrbit(u16 offset, u8 bit) {
    BIT_CLR(*(u8 *)(vm.mem + /*vm.script->vram_org +*/ offset), bit);
}

void vram_add8(u16 offset, u8 value) {
    *(u8 *)(vm.mem + /*vm.script->vram_org +*/ offset) += value;
}

void vram_add16(u16 offset, u16 value) {
    *(u16 *)(vm.mem + /*vm.script->vram_org +*/ offset) += value;
}


// =============================================================================
// MARK: - Virtual Stack access
// =============================================================================
void vram_push32(u32 value) {
    vm.script->vacc_off -= sizeof(u32);
    *(u32 *)(vm.mem + /*vm.script->vram_org +*/ vm.script->vacc_off) = value;
}

u32 vram_peek32(void) {
    return *(u32 *)(vm.mem + /*vm.script->vram_org +*/ vm.script->vacc_off);
}

u32 vram_pop32(void) {
    u32 ret = vram_peek32();
    vm.script->vacc_off += sizeof(u32);
    return ret;
}

void vram_save_script_state(sAlisScript * script) {
    
}


// =============================================================================
// MARK: - Virtual RAM debug
// =============================================================================
void vram_debug(void) {
    u8 width = 16;
    
    printf("Stack Offset=0x%04x (word=0x%04x) (dword=0x%08x)\n",
           vm.script->vacc_off,
           (u16)(*(vm.mem + /*vm.script->vram_org +*/ vm.script->vacc_off)),
           (u32)(*(vm.mem + /*vm.script->vram_org +*/ vm.script->vacc_off)));

    printf("Virtual RAM:\n");
    
    printf("       ");
    for(u8 j = 0; j < width; j++) {
        printf("  %x", j);
    }
    printf("\n");
    
    for(u32 i = 0; i < kVirtualRAMSize; i += width) {
        printf("0x%04x: ", i);
        for(u8 j = 0; j < width; j++) {
            printf("%02x ", (vm.mem /*+ vm.script->vram_org*/)[i + j]);
        }
        printf("\n");
    }
}

void vram_debug_addr(u16 addr) {
    u8 width = 16;
    printf("Virtual RAM:\n");
    
    printf("       ");
    for(u8 j = 0; j < width; j++) {
        printf("  %x", j);
    }
    printf("\n");
    printf("0x%04x: ", addr);
    for(u32 j = addr; j < addr + width; j++) {
        printf("%02x ", (vm.mem /*+ vm.script->vram_org*/)[j]);
    }
}
