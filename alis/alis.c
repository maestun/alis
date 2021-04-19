//
//  alis_vm.c
//  lc3vm
//
//  Created by developer on 09/09/2020.
//  Copyright © 2020 Zlot. All rights reserved.
//

#include "alis.h"
#include "alis_private.h"
#include "sys/sys.h"
#include "utils.h"

sAlisVM alis;


sAlisError errors[] = {
    { ALIS_ERR_FOPEN,   "fopen", "Failed to open file %s\n" },
    { ALIS_ERR_FWRITE,  "fwrite", "Failed to write to file %s\n" },
    { ALIS_ERR_FCREATE, "fcreate", "" },
    { ALIS_ERR_FDELETE, "fdelete", "" },
    { ALIS_ERR_CDEFSC,  "cdefsc", "" },
    { ALIS_ERR_FREAD,   "fread", "" },
    { ALIS_ERR_FCLOSE,  "fclose", "" },
    { ALIS_ERR_FSEEK,   "fseek", "" }
};


// =============================================================================
// MARK: - Private
// =============================================================================
alisRet readexec(sAlisOpcode * table, char * name, u8 identation) {
    
    if(alis.script->pc - alis.script->pc_org == kVirtualRAMSize) {
        // pc overflow !
        debug(EDebugFatal, "PC OVERFLOW !");
    }
    else {
        // fetch code
        u8 code = *(alis.script->pc++);
        sAlisOpcode opcode = table[code];
        debug(EDebugVerbose, " %s", opcode.name);
        return opcode.fptr();
    }
}

alisRet readexec_opcode() {
    debug(EDebugVerbose, "\n%s: 0x%06x:", alis.script->name, script_pc(alis.script));
    return readexec(opcodes, "opcode", 0);
}

alisRet readexec_opername() {
    return readexec(opernames, "opername", 1);
}

alisRet readexec_storename() {
    return readexec(storenames, "storename", 2);
}

alisRet readexec_addname() {
    return readexec(addnames, "addname", 2);
}

alisRet readexec_addname_swap() {
    u8 * tmp = alis.bssChunk1;
    alis.bssChunk1 = alis.bssChunk3;
    alis.bssChunk3 = tmp;
    return readexec_addname();
}

alisRet readexec_opername_saveD7() {
    alis.varD6 = alis.varD7;
    return readexec_opername();
}

alisRet readexec_opername_swap() {
    u8 * tmp = alis.bssChunk1;
    alis.bssChunk1 = alis.bssChunk3;
    alis.bssChunk3 = tmp;
    return readexec_opername();
}


// =============================================================================
// MARK: - VM API
// =============================================================================
void alis_init(sPlatform platform) {
    
    // init virtual ram
    alis.mem = malloc(sizeof(u8) * kHostRAMSize);
    memset(alis.mem, 0, sizeof(u8) * kHostRAMSize);
    
    // init virtual registers
    alis.varD6 = alis.varD7 = 0;
    
    // init temp chunks
    // TODO: might as well (static) alloc some ram.
    alis.bssChunk1 = alis.mem + 0x1a1e6;
    alis.bssChunk2 = alis.mem + 0x1a2e6;
    alis.bssChunk3 = alis.mem + 0x1a3e6;
    
    // init helpers
    if(alis.fp) {
        fclose(alis.fp);
        alis.fp = NULL;
    }
    
    alis.platform = platform;
    
    // TODO: seems to be vram for alis vm start
    alis.vram_alloc = alis.mem + 0x22400;
    alis.vram_org = alis.mem + 0x224f0;
    
    // TODO: init virtual accumulator
//    alis.acc_org = alis.script->vram_org;
//    alis.acc = alis.script->vram_org + kVirtualRAMSize;
    alis.acc = alis.acc_org = (alis.mem + 0x198e2);
    alis.script_count = 0;
    
    // init host system stuff
    alis.pixelbuf.w = alis.platform.width;
    alis.pixelbuf.h = alis.platform.height;
    alis.pixelbuf.data = (u8 *)malloc(alis.pixelbuf.w * alis.pixelbuf.h);
}

void alis_deinit() {
    // free scripts
    for(int i = 0; i < kMaxScripts; i++) {
        script_unload(alis.script);
    }
//    free(alis.bssChunk1);
//    free(alis.bssChunk2);
//    free(alis.bssChunk3);
    
    //vram_deinit(alis.vram);
    free(alis.pixelbuf.data);
    free(alis.mem);
}


void alis_loop() {
    alis.script->running = 1;
    while (alis.running && alis.script->running) {
        alis.running = sys_poll_event();
        readexec_opcode();
        sys_render(alis.pixelbuf);
    }
    // alis loop was stopped by 'cexit', 'cstop', or user event
}


void alis_register_script(sAlisScript * script) {
    u8 id = script->header.id;
    alis.scripts[id] = script;
    alis.script_id_stack[alis.script_count++] = id;
}


u8 alis_main() {
    u8 ret = 0;
    
    // load main script
    sAlisScript * main_script = script_load(alis.platform.main);
    alis_register_script(main_script);
    alis.script_index = 0;
    
    // run !
    alis.running = 1;
    while (alis.running) {

        // fetch script to run
        u8 id = alis.script_id_stack[alis.script_index];
        alis.script = alis.scripts[id];
        // alis.sid = alis.script->header.id;
        alis.script->pc = alis.mem + alis.script->context._0x8_script_ret_offset;
        
        alis_loop();
        
        // TODO: handle virtual interrupts
        
        // save virtual accumulator offset
        alis.script->context._0xc_vacc_offset = alis.script->vacc_off;
        
        // push return address for the current script
        alis.script->context._0x8_script_ret_offset = (u32)(alis.script->pc - alis.mem);
        
        // compute offset in current script to continue loop in same script
        u32 offset = alis.script->header.ret_offset;
        if(offset) {
            // the current script has an 'offset' in its header
            // so we must perform a change of script... within the same script
            // the address to jump to is:
            alis.script->pc = alis.script->data_org + offset + 6; /* skip ID, word 1, word 2: 6 bytes */
            alis_loop();
            alis.script_index++;
        }
        else {
            // return to previous script ?
            alis.script_index--;
        }
    }
    
    // alis was stopped by 'cexit' opcode
    return ret;
}





//
//u8 _alis_main() {
//    u8 ret = 0;
//    alis.running = 1;
//    while(alis.running) {
//        alis_loop();
//        
//        // save virtual accumulator offset
//        vram_write16(VRAM_OFFSET_VACC_OFFSET_D4, alis.script->vacc_off);
//        
//        // save script pointer offset
//        vram_write16(VRAM_OFFSET_SCRIPT_PTR_ADDR, alis.script->pc - alis.script->pc_org);
//        
//        // save script origin offset
//        u32 offset = alis.script->header.ret_offset;
//        if (offset != 0) {
//            // the current script has an 'offset' in its header
//            // so we must perform a change of script... within the same script O_o
//            // the address to jump to is:
//            alis.script->pc = alis.script->data_org + offset + 6;
//            alis_loop();
//            
//            // TODO: FUN_RETURN_FROM_CSTOP
//            
//            
//            // TODO: what is this ?
//            u8 b = vram_read8(VRAM_OFFSET_UNKNOWN_2);
//            vram_write8(VRAM_OFFSET_CSTART, b);
//            
//            // TODO: compute next script and set pc
//            alis_loop();
//        }
//        //        u8 next_id = alis.scriptID;
//        //        u8 current_id = alis.script->ID;
//        //        alis.script = alis.scripts[next_id];
//        //        alis.scriptID = current_id;
//        // TODO: save current script program counter
//        // TODO: change script
//        
//        // alis_next_script();
//    }
//    return ret;
//}


void alis_error(u8 errnum, ...) {
    sAlisError err = errors[errnum];
    debug(EDebugError, err.descfmt, errnum);
    exit(-1);
}


void alis_debug() {
    printf("\n-- ALIS --\nCurrent script: '%s' (0x%02x)\n", alis.script->name, alis.script->header.id);
    printf("R6  0x%04x\n", alis.varD6);
    printf("R7  0x%04x\n", alis.varD7);

    
    script_debug(alis.script);
//    printf("ACC OFFSET=0x%04x (byte=0x%02x) (word=0x%04x)\n",
//           (u16)(alis.acc - alis.acc_org),
//           (u8)(*alis.acc),
//           (u16)(*alis.acc));
//
//    printf("BSS1=%s\n", alis.bssChunk1);
//    printf("BSS2=%s\n", alis.bssChunk2);
//    printf("BSS3=%s\n", alis.bssChunk3);
    
    vram_debug();
}


// =============================================================================
// MARK: - MEMORY ACCESS
// =============================================================================
u8 * vram_ptr(u16 offset) {
    return (u8 *)(alis.script->vram_org + offset);
}

u8 vram_read8(s32 offset) {
    return *(u8 *)(alis.script->vram_org + offset);
}

s16 vram_read8ext16(u16 offset) {
    s16 ret = *(u8 *)(alis.script->vram_org + offset);
    if(BIT_CHK(ret, 7)) {
        ret |= 0xff00;
    }
    return ret;
}

s32 vram_read8ext32(u16 offset) {
    s32 ret = *(u8 *)(alis.script->vram_org + offset);
    if(BIT_CHK(ret, 7)) {
        ret |= 0xffffff00;
    }
    return ret;
}

u16 vram_read16(s32 offset) {
    return *(u16 *)(alis.script->vram_org + offset);
}

s32 vram_read16ext32(u16 offset) {
    s32 ret = *(u16 *)(alis.script->vram_org + offset);
    if(BIT_CHK(ret, 15)) {
        ret |= 0xffffff00;
    }
    return ret;
}

void vram_readp(u16 offset, u8 * dst_ptr) {
    u8 * src_ptr = alis.script->vram_org + offset;
    do {
        *dst_ptr++ = *src_ptr++;
    } while (*src_ptr);
}

void vram_write8(s32 offset, u8 value) {
    *(u8 *)(alis.script->vram_org + offset) = value;
}

void vram_write16(s32 offset, u16 value) {
    *(u16 *)(alis.script->vram_org + offset) = value;
}

void vram_writep(u16 offset, u8 * src_ptr) {
    u8 * dst_ptr = alis.script->vram_org + offset;
    do {
        *dst_ptr++ = *src_ptr++;
    } while (*src_ptr);
}

void vram_setbit(u16 offset, u8 bit) {
    BIT_SET(*(u8 *)(alis.script->vram_org + offset), bit);
}

void vram_clrbit(u16 offset, u8 bit) {
    BIT_CLR(*(u8 *)(alis.script->vram_org + offset), bit);
}

void vram_add8(u16 offset, u8 value) {
    *(u8 *)(alis.script->vram_org + offset) += value;
}

void vram_add16(u16 offset, u16 value) {
    *(u16 *)(alis.script->vram_org + offset) += value;
}


// =============================================================================
// MARK: - Virtual Stack access
// =============================================================================
void vram_push32(u32 value) {
    alis.script->vacc_off -= sizeof(u32);
    *(u32 *)(alis.script->vram_org + alis.script->vacc_off) = value;
}

u32 vram_peek32() {
    return *(u32 *)(alis.script->vram_org + alis.script->vacc_off);
}

u32 vram_pop32() {
    u32 ret = vram_peek32();
    alis.script->vacc_off += sizeof(u32);
    return ret;
}

void vram_save_script_state(sAlisScript * script) {
    
}


// =============================================================================
// MARK: - Virtual RAM debug
// =============================================================================
void vram_debug() {
    u8 width = 16;
    
    printf("Stack Offset=0x%04x (word=0x%04x) (dword=0x%08x)\n",
           alis.script->vacc_off,
           (u16)(*(alis.script->vram_org + alis.script->vacc_off)),
           (u32)(*(alis.script->vram_org + alis.script->vacc_off)));

    printf("Virtual RAM:\n");
    
    printf("       ");
    for(u8 j = 0; j < width; j++) {
        printf("  %x", j);
    }
    printf("\n");
    
    for(u32 i = 0; i < kVirtualRAMSize; i += width) {
        printf("0x%04x: ", i);
        for(u8 j = 0; j < width; j++) {
            printf("%02x ", alis.script->vram_org[i + j]);
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
        printf("%02x ", alis.script->vram_org[j]);
    }
}

