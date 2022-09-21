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


sAlisVM vm;
sHost host;


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
    
    if(vm.script->pc - vm.script->pc_org == kVirtualRAMSize) {
        // pc overflow !
        debug(EDebugFatal, "PC OVERFLOW !");
    }
    else {
        // fetch code
        u8 code = *(vm.mem + vm.script->pc++);
        sAlisOpcode opcode = table[code];
        debug(EDebugVerbose, " %s", opcode.name);
        return opcode.fptr();
    }
}

alisRet readexec_opcode(void) {
    debug(EDebugVerbose, "\n%s: 0x%06x:", vm.script->name, vm.script->pc);
    return readexec(opcodes, "opcode", 0);
}

alisRet readexec_opername(void) {
    return readexec(opernames, "opername", 1);
}

alisRet readexec_storename(void) {
    return readexec(storenames, "storename", 2);
}

alisRet readexec_addname(void) {
    return readexec(addnames, "addname", 2);
}

alisRet readexec_addname_swap(void) {
    u8 * tmp = vm.sd7;
    vm.sd7 = vm.oldsd7;
    vm.oldsd7 = tmp;
    return readexec_addname();
}

alisRet readexec_opername_saveD7(void) {
    vm.varD6 = vm.varD7;
    return readexec_opername();
}

alisRet readexec_opername_swap(void) {
    u8 * tmp = vm.sd7;
    vm.sd7 = vm.oldsd7;
    vm.oldsd7 = tmp;
    return readexec_opername();
}

void alis_load_main(void) {
    // load main scripts as an usual script...
    vm.main = script_load(vm.platform.main);
    
}

void init_entities(void) {
    u32 offset = 0;
    u16 counter = 0;
    do {
        counter += 6;
        offset = vm.vram.atent + counter;
        vram_write16(offset - 2, counter);
        vram_write32(offset - 6, 0);
    } while(offset < vm.vram.debent);
}

// $1867c
void init_sprites(void) {
    debug(EDebugVerbose, "init_sprites\n");

    u16 d0 = 0x8000;
    vm.vram.basesprit = vm.vram.debsprit + d0;
    vm.vram.tvsprit = d0;
    
    // clr.l   (0xc,A0,D0w*0x1)
    vram_write32(vm.vram.basesprit + 0xc, 0);

    vram_write16(vm.vram.basesprit + 0x16, 319);
    vram_write16(vm.vram.basesprit + 0x18, 199);

    d0 += 0x28;
    vm.vram.backsprit = d0;
    vram_write32(vm.vram.basesprit + d0 + 1, 0);
    
    d0 += 0x28;
    vm.vram.texsprit = d0;
    vram_write32(vm.vram.basesprit + d0 + 0xc, 0);
    vram_write8(vm.vram.basesprit + d0, 0xfe);

    vm.vars.b_mousflag = 0;
    
    d0 += 0x28;
    vm.vars.libsprit = d0;

    u32 a1 = vm.vram.basesprit + d0;
 
    // TODO: continue at $1cff4
    
}



// =============================================================================
// MARK: - VM API
// =============================================================================
void alis_init(sPlatform platform) {
    
    debug(EDebugVerbose, "ALIS: Init.\n");
    
    vm.platform = platform;    
    
    // init virtual ram
    vm.mem = malloc(sizeof(u8) * platform.ram_sz);
    memset(vm.mem, 0, sizeof(u8) * platform.ram_sz);

    // open packed main script, it contains vm specs in the header
    FILE * fp = fopen(platform.main, "rb");
    if (fp) {
        // skip 6 bytes
        fseek(fp, sizeof(sPackedScriptHeader), SEEK_CUR);
        
        // read raw specs header
        vm.specs.max_prog = fread16(fp, platform);
        vm.specs.max_ent = fread16(fp, platform);
        vm.specs.max_host_ram = fread32(fp, platform);
        vm.specs.debsprit_offset = fread32(fp, platform);
        vm.specs.finsprit_offset = fread32(fp, platform);

        // vm.vram.org = 0x22400;
        vm.vram.basemem = 0x22400;
        vm.vram.basevar = 0;

        vm.vram.atprog = vm.vram.basemem;
        vm.vram.dernprog = vm.vram.basemem;

        vm.vram.atent = vm.vram.basemem + (vm.specs.max_prog * sizeof(uint32_t));
        vm.vram.debent = vm.vram.atent + (vm.specs.max_ent * 6);
        vm.vram.finent = vm.vram.debent;

        vm.vram.debsprit = ((vm.vram.finent + vm.specs.debsprit_offset) | 0xf) + 1;
        vm.vram.finsprit = vm.vram.debsprit + ((vm.specs.finsprit_offset + 3) * 40);

        vm.vram.debprog = vm.vram.finsprit;
        vm.vram.finprog = vm.vram.debprog;

        u32 phys_max_ram = platform.ram_sz - platform.vram_sz - 0x400;
        u32 script_max_ram = vm.vram.finprog + vm.specs.max_host_ram;
        vm.vram.finmem = phys_max_ram > script_max_ram ? script_max_ram : phys_max_ram;

        printf("\
basemem:    0x%x\n\
basevar:    0x%x\n\
atprog:     0x%x\n\
dernprog:   0x%x\n\
atent:      0x%x\n\
debent:     0x%x\n\
finent:     0x%x\n\
debsprit:   0x%x\n\
finsprit:   0x%x\n\
debprog:    0x%x\n\
finprog:    0x%x\n\
finmem:     0x%x\n",
            vm.vram.basemem,
            vm.vram.basevar, 
            vm.vram.atprog, 
            vm.vram.dernprog, 
            vm.vram.atent, 
            vm.vram.debent, 
            vm.vram.finent, 
            vm.vram.debsprit, 
            vm.vram.finsprit, 
            vm.vram.debprog, 
            vm.vram.finprog, 
            vm.vram.finmem);

        init_entities();

        init_sprites();

        // init virtual registers
        vm.varD6 = vm.varD7 = 0;
        
        // init temp chunks
        // TODO: might as well (static) alloc some ram.
        vm.sd7 = vm.mem + 0x1a1e6;
        vm.sd6 = vm.mem + 0x1a2e6;
        vm.oldsd7 = vm.mem + 0x1a3e6;
        
        // init helpers
        if(vm.fp) {
            fclose(vm.fp);
            vm.fp = NULL;
        }
        
        // set the vram origin at some arbitrary location (same as atari, to ease debug)
        vm.vram_org = vm.mem + ALIS_VM_RAM_ORG;
        
        // the script data address table is located at vram start
        vm.script_data_orgs = (u32 *)vm.vram_org;
        
        // TODO: init virtual accumulator
    //    vm.acc_org = vm.script->vram_org;
    //    vm.acc = vm.script->vram_org + kVirtualRAMSize;
        vm.acc = vm.acc_org = (vm.mem + 0x198e2);
        //vm.script_count = 0;
        
        // init host system stuff
        host.pixelbuf.w = vm.platform.width;
        host.pixelbuf.h = vm.platform.height;
        host.pixelbuf.data = (u8 *)malloc(host.pixelbuf.w * host.pixelbuf.h);
        
        // load main script
        alis_load_main();
        vm.script = vm.main;

        // // set the location of scripts' vrams table
        // vm.script_vram_orgs = (sScriptLoc *)(vm.vram_org + (vm.specs.script_data_tab_len * sizeof(u8 *)));
        
        // // compute the end address of the scripts' vrams table
        // u32 script_vram_tab_end = (u32)((u8 *)vm.script_vram_orgs - vm.mem + (vm.specs.script_vram_tab_len * sizeof(sScriptLoc)));

        // // populate the script vrams table with the offsets (routine at $18cd8)
        // for(int idx = 0; idx < vm.specs.script_vram_tab_len; idx++) {
        //     u16 offset = (1 + idx) * sizeof(sScriptLoc);
        //     vm.script_vram_orgs[idx] = (sScriptLoc){0, offset};
        // }
        
        // vm.specs.script_vram_max_addr = ((script_vram_tab_end + vm.specs.max_allocatable_vram) | 0b111) + 1;

        // u32 main_script_data_addr = vm.specs.script_vram_max_addr + vm.specs.vram_to_data_offset;

//         debug(EDebugVerbose, "\
// - script data table count: %d (0x%x), located at 0x%x\n\
// - script vram table count: %d (0x%x), located at 0x%x, ends at 0x%x\n\
// - script data located at 0x%x\n\
// - vram allocatable up to 0x%x \n\
// - unused (?) dword from header: 0x%x\n",
//               vm.specs.script_data_tab_len, vm.specs.script_data_tab_len, (u8 *)vm.script_data_orgs - vm.mem,
//               vm.specs.script_vram_tab_len, vm.specs.script_vram_tab_len, (u8 *)vm.script_vram_orgs - vm.mem,
//               script_vram_tab_end,
//               main_script_data_addr,
//               vm.specs.script_vram_max_addr,
//               vm.specs.unused);
        fclose(fp);
    }
    else {
        debug(EDebugFatal, "main script not found: %s\n", platform.main);
    }
}


void alis_deinit(void) {
    // free scripts
    // TODO: use real script table / cunload
    for(int i = 0; i < kMaxScripts; i++) {
        script_unload(vm.script);
    }
//    free(vm.sd7);
//    free(vm.sd6);
//    free(vm.oldsd7);
    
    //vram_deinit(vm.vram);
    free(host.pixelbuf.data);
    free(vm.mem);
}


void alis_loop(void) {
    vm.script->running = 1;
    while (vm.running && vm.script->running) {
        vm.running = sys_poll_event();
        readexec_opcode();
        sys_render(host.pixelbuf);
    }
    // alis loop was stopped by 'cexit', 'cstop', or user event
}


void alis_register_script(sAlisScript * script) {
    u8 id = script->header.id;
    vm.scripts[id] = script;
    // vm.script_id_stack[vm.script_count++] = id;
}


//u8 alis_main(void) {
//    undefined4 uVar1;
//    undefined2 uVar2;
//    char *pcVar3;
//    short sVar4;
//    int iVar5;
//
//
//    sVar4 = 0;
//    iVar5 = addr_atent;
//    do {
//      iVar5 = *(int *)(iVar5 + sVar4);
//      fallent = 0;
//      fseq = '\0';
//      if (((*(char *)(iVar5 + -0x24) < '\0') && ((*(byte *)(iVar5 + -0x24) & 2) == 0)) &&
//         (*(int *)(*(int *)(iVar5 + -0x14) + 10) != 0)) {
//        save_rsp = *(undefined2 *)(iVar5 + -10);
//        savecoord();
//        FUN_READEXEC_OPCODE();
//        updtcoo0();
//      }
//      if (*(char *)(iVar5 + -4) != '\0') {
//        if (*(char *)(iVar5 + -4) < '\0') {
//          *(undefined *)(iVar5 + -4) = 1;
//        }
//        pcVar3 = (char *)(iVar5 + -1);
//        *pcVar3 = *pcVar3 + -1;
//        if (*pcVar3 == '\0') {
//          savecoord();
//          uVar1 = *(undefined4 *)(iVar5 + -8);
//          uVar2 = *(undefined2 *)(iVar5 + -10);
//          fseq = fseq + '\x01';
//          FUN_READEXEC_OPCODE();
//          *(undefined2 *)(iVar5 + -10) = uVar2;
//          *(undefined4 *)(iVar5 + -8) = uVar1;
//          if (*(int *)(*(int *)(iVar5 + -0x14) + 6) != 0) {
//            fseq = '\0';
//            save_rsp = *(undefined2 *)(iVar5 + -10);
//            FUN_READEXEC_OPCODE();
//          }
//          updtcoo0();
//          *(undefined *)(iVar5 + -1) = *(undefined *)(iVar5 + -2);
//        }
//      }
//      sVar4 = *(short *)(addr_atent + 4 + (int)sVar4);
//      iVar5 = addr_atent;
//      if (sVar4 == 0) {
//        image();
//      }
//    } while( true );
//}

u8 alis_main(void) {
    u8 ret = 0;
    
    // run !
    vm.running = 1;
    while (vm.running) {
        
        vm._cstopret = 0;
        vm._callentity = 0;
        
        // fetch script to run
        // u8 id = vm.script_id_stack[vm.script_index];
        // vm.script = vm.scripts[id];
        // vm.sid = vm.script->header.id;
        vm.script->pc = vm.script->context._0x8_script_ret_offset;
        
        alis_loop();
        
        // TODO: handle virtual interrupts
        
        // save virtual accumulator offset
        vm.script->context._0xc_vacc_offset = vm.script->vacc_off;
        
        // push return address for the current script
        vm.script->context._0x8_script_ret_offset = vm.script->pc;
        
        // compute offset in current script to continue loop in same script
        u32 offset = vm.script->header.ret_offset;
        if(offset) {
            // the current script has an 'offset' in its header
            // so we must perform a change of script... within the same script
            // the address to jump to is:
            vm.script->pc = vm.script->data_org + offset + 6; /* skip ID, word 1, word 2: 6 bytes */
            alis_loop();
            vm.script_index++;
        }
        else {
            // return to previous script ?
            vm.script_index--;
        }
    }
    
    // alis was stopped by 'cexit' opcode
    return ret;
}



void alis_error(u8 errnum, ...) {
    sAlisError err = errors[errnum];
    debug(EDebugError, err.descfmt, errnum);
    exit(-1);
}


void alis_debug(void) {
    printf("\n-- ALIS --\nCurrent script: '%s' (0x%02x)\n", vm.script->name, vm.script->header.id);
    printf("R6  0x%04x\n", vm.varD6);
    printf("R7  0x%04x\n", vm.varD7);

    
    script_debug(vm.script);
//    printf("ACC OFFSET=0x%04x (byte=0x%02x) (word=0x%04x)\n",
//           (u16)(vm.acc - vm.acc_org),
//           (u8)(*vm.acc),
//           (u16)(*vm.acc));
//
//    printf("BSS1=%s\n", vm.sd7);
//    printf("BSS2=%s\n", vm.sd6);
//    printf("BSS3=%s\n", vm.oldsd7);
    
    vram_debug();
}


// =============================================================================
// MARK: - MEMORY ACCESS
// =============================================================================

/**
 * @brief Returns a byte pointer on (vram+offset)
 * 
 * @param offset 
 * @return u8* 
 */
u8 * vram_ptr(u16 offset) {
    return (u8 *)(vm.mem + /*vm.script->vram_org*/ + offset);
}

u8 vram_read8(s32 offset) {
    return *(u8 *)(vm.mem + /*vm.script->vram_org*/ + offset);
}

s16 vram_read8ext16(u16 offset) {
    s16 ret = *(u8 *)(vm.mem + /*vm.script->vram_org +*/ offset);
    if(BIT_CHK(ret, 7)) {
        ret |= 0xff00;
    }
    return ret;
}

s32 vram_read8ext32(u16 offset) {
    s32 ret = *(u8 *)(vm.mem + /*vm.script->vram_org +*/ offset);
    if(BIT_CHK(ret, 7)) {
        ret |= 0xffffff00;
    }
    return ret;
}

u16 vram_read16(s32 offset) {
    return *(u16 *)(vm.mem + /*vm.script->vram_org +*/ offset);
}

s32 vram_read16ext32(u16 offset) {
    s32 ret = *(u16 *)(vm.mem + /*vm.script->vram_org +*/ offset);
    if(BIT_CHK(ret, 15)) {
        ret |= 0xffffff00;
    }
    return ret;
}

void vram_readp(u16 offset, u8 * dst_ptr) {
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



// =============================================================================
#pragma mark - Common routines
// =============================================================================
u16 string_array_common(u16 offset) {
    debug(EDebugWarning, "\n%s STUBBED\n", __FUNCTION__);
    return offset;
}

u16 int_array_common(u16 offset) {
    debug(EDebugWarning, "\n%s STUBBED\n", __FUNCTION__);
    return offset;
}

u16 char_array_common(u16 offset) {

    /*
      0001781e 41 f6 00      lea       (__DAT_0000fffe,A6,D0w*0x1),A0
               fe
      00017822 d0 47         add.w     D7w,D0w
      00017824 51 c9 00      dbf       D1w,__loop
               04
      00017828 4e 75         rts
                         __loop                                    XREF[2]:   00017824(j), 00017830(j)  
      0001782a 34 1c         move.w    (A4)+,D2w
      0001782c c5 e0         muls.w    -(A0),D2
      0001782e d0 42         add.w     D2w,D0w
      00017830 51 c9 ff      dbf       D1w,__loop
               f8
      00017834 4e 75         rts
      00017836 e2 48         lsr.w     #0x1,D0w
      00017838 32 00         move.w    D0w,D1w
      0001783a 20 3c 00      move.l    #0x17,D0
               00 00 17
      00017840 4e f9 00      jmp       perror.l
               01 94 94
      00017846 32 00         move.w    D0w,D1w
      00017848 20 3c 00      move.l    #0x18,D0
               00 00 18
      0001784e 4e f9 00      jmp       perror.l
               01 94 94
*/
    
    // move.b    (-0x1,A6,D0w*0x1),D1b
    // ext.w     D1w
    s16 d1w = vram_read8ext16(offset - 1);

    // lea  (__DAT_0000fffe,A6,D0w*0x1),A0

    // add.w     D7w,D0w
    offset += vm.varD7;
    while(--d1w > 0) {
        u16 d2 = *(vm.acc++);
        u16 mul = vram_read16(offset - 2);
        d2 *= mul;
        offset += d2;
    }
    
    return offset;
}
