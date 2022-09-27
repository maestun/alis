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

// TODO: complete this
sAlisError errors[] = {
    { ALIS_ERR_FOPEN,   "fopen", "Failed to open file %s\n" },
    { ALIS_ERR_FWRITE,  "fwrite", "Failed to write to file %s\n" },
    { ALIS_ERR_FCREATE, "fcreate", "" },
    { ALIS_ERR_FDELETE, "fdelete", "" },
    { ALIS_ERR_CDEFSC,  "cdefsc", "" },
    { ALIS_ERR_FREAD,   "fread", "" },
    { ALIS_ERR_FCLOSE,  "fclose", "" },
    { ALIS_ERR_MAXPROG, "maxprog", "Loaded the max number of scripts" },
    { ALIS_ERR_FSEEK,   "fseek", "" }
};


// =============================================================================
// MARK: - Private
// =============================================================================
alisRet readexec(sAlisOpcode * table, char * name, u8 identation) {
    
    if(vm.script->pc - vm.script->code_org == kVirtualRAMSize) {
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


void set_prog(sAlisScript* aScript) {
    
    
    // script size should be even
    if(aScript->sz & 1) {
        aScript->sz++;
    }
    
    // compute end of script data offset
    vm.vram.finprog += aScript->sz;
    
    if(vm.vram.finprog > vm.vram.finmem) {
        alis_error(ALIS_ERR_PROG_OVERFLOW, "Erreur débordement programme");
    }
    
    // check for a free program slot
    do {
        vm.vram.dernprog += sizeof(u32);
    } while(vram_read32(vm.vram.atprog));
    
    vram_write32(vm.vram.dernprog, aScript->header.id);
    if(++vm.vars.nb_prog > vm.specs.max_prog) {
        alis_error(ALIS_ERR_MAXPROG, "Trop de programmes");
    }
    
    // io_pixel()
    // io_digit()
//    if (nmode == 1 || nmode == 2 || nmode == 3 || nmode == 5) {
//        io_to_mono();
//    }
}


void live_main(void) {
    // should be $22690
    vm.vram.basemain = vm.vram.debent + vm.script->header.w_unknown5 + vm.script->header.w_unknown7 + kVMStatusSize;
    
    // should be $26902
    vm.vram.finent = vm.vram.basemain + vm.script->header.vram_alloc_sz; // ram to allocate
    
    // clear ent memory (from basemem => finent)
    memset(vm.mem + vm.vram.basemain, 0, vm.script->header.vram_alloc_sz);
    
    // should be $ffac
    vm.vacc_offset = vm.vram.debent - vm.vram.basemain + vm.script->header.w_unknown5;
    
    // now set vstat
    vm.vstat.vacc_offset_kOffsetMinus10 = vm.vacc_offset;
    vm.vstat.oscan_oscanclr_2_kOffsetMinus28 = vm.vacc_offset;
    vm.vstat.oscan_oscanclr_1_kOffsetMinus30 = vm.vacc_offset;
    
    // should be $2d290
    vm.vstat.script_code_addr_kOffsetMinus8 = vm.script->code_org;
    
    vm.vstat.unknown5_kOffsetMinus16 = vm.script->header.id;

    // should be $2d278
    vm.vstat.script_data_addr_kOffsetMinus20 = vm.script->data_org;
    
    vm.vstat.unknown1_kOffsetMinus46 = vm.script->header.b_0x17;
    vm.vstat.unknown6_kOffsetMinus2 = 1;
    vm.vstat.cstart_kOffsetMinus1 = 1;
    vm.vstat.cstart_csleep_kOffsetMinus4 = 0xff;
    vm.vstat.cforme_delforme_kOffsetMinus26 = 0xffff;
    vm.vstat.czap_cexplode_kOffsetMinus14 = 0;
    vm.vstat.cscan_cinter_kOffsetMinus36 = 2;
    
    vm.vstat.unknown4_kOffsetMinus24 = 0;
    vm.vstat.vacc_offset_d4_kOffsetMinus12 = 0;
    vm.vstat.cworld1_kOffsetMinus34 = 0;
    vm.vstat.csetvect_kOffsetMinus32 = 0;
    vm.vstat.cxyinv_kOffsetMinus3 = 0;
    vm.vstat.cred_on_off_kOffsetMinus37 = 0;
    
    vm.vstat.creducing_2_kOffsetMinus38 = 0xff;
    vm.vstat.clinking_kOffsetMinus42 = 0;
    vm.vstat.calign_2_kOffsetMinus44 = 0;
    vm.vstat.calign_1_kOffsetMinus45 = 0;
    vm.vstat.unknown2_kOffsetMinus40 = 0;
    vm.vstat.chsprite_kOffsetMinus47 = 0;
    vm.vstat.kOffsetMinus50 = 0;
    vm.vstat.kOffsetMinus52 = 0;
    vm.vstat.kOffsetMinus48 = 0;
}

void save_coord(void) {
    
}

void update_coord(void) {
    
}



void alis_load_main(void) {
    vm.script = script_load(vm.platform.main);
    set_prog(vm.script);
    live_main();
}

void init_entities(void) {
    
    // init atent array
    // contains (max_ent * 6) bytes
    u32 offset = 0;
    u16 counter = 0;
    do {
        counter += 6; // sizeof(sEnt)
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
    
    vram_write32(vm.vram.basesprit + d0 + 0xc, 0);
    vram_write16(vm.vram.basesprit + d0 + 0x16, vm.platform.width - 1);
    vram_write16(vm.vram.basesprit + d0 + 0x18, vm.platform.height - 1);

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
        
        // read raw vm specs from unpacked main script's header
        vm.specs.max_prog = fread16(fp, platform);
        vm.specs.max_ent = fread16(fp, platform);
        vm.specs.max_host_ram = fread32(fp, platform);
        vm.specs.debsprit_offset = fread32(fp, platform);
        vm.specs.finsprit_offset = fread32(fp, platform);

        // for debugging and comparing w/ emulator we set this to $22400
        vm.vram.basemem = ALIS_VM_RAM_ORG; // address computed by host system's 'malloc'
        // vm.vram.basevar = 0; // looks unused ?

        vm.vram.atprog = vm.vram.basemem; // set once, addr of script adresses
        vm.vram.dernprog = vm.vram.basemem; // addr of next available script slot in atprog

        vm.vram.atent = vm.vram.basemem + (vm.specs.max_prog * sizeof(uint32_t));
        vm.vram.debent = vm.vram.atent + (vm.specs.max_ent * 6);
        vm.vram.finent = vm.vram.debent;
        
        u32 phys_max_ram = platform.ram_sz - platform.video_ram_sz - 0x400;
        u32 script_max_ram = vm.vram.finprog + vm.specs.max_host_ram;
        vm.vram.finmem = phys_max_ram > script_max_ram ? script_max_ram : phys_max_ram;

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

        init_entities();
        
        vm.vram.debsprit = ((vm.vram.finent + vm.specs.debsprit_offset) | 0xf) + 1;

        init_sprites();

        vm.vram.finsprit = vm.vram.debsprit + ((vm.specs.finsprit_offset + 3) * 40);

        vm.vram.debprog = vm.vram.finsprit;
        vm.vram.finprog = vm.vram.debprog;
        
        // load main script
        alis_load_main();

        printf("\n\
basemem:    0x%x\n\
atprog:     0x%x\n\
dernprog:   0x%x\n\
atent:      0x%x\n\
debent:     0x%x\n\
basemain:   0x%x\n\
finent:     0x%x\n\
debsprit:   0x%x\n\
finsprit:   0x%x\n\
basesprit:  0x%x\n\
debprog:    0x%x\n\
finprog:    0x%x\n\
finmem:     0x%x\n",
            vm.vram.basemem,
            vm.vram.atprog,
            vm.vram.dernprog,
            vm.vram.atent,
            vm.vram.debent,
            vm.vram.basemain,
            vm.vram.finent,
            vm.vram.debsprit,
            vm.vram.finsprit,
            vm.vram.basesprit,
            vm.vram.debprog,
            vm.vram.finprog,
            vm.vram.finmem);

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
    vm.running = 1;
    vm.script->running = 1;
    while (vm.running && vm.script->running) {
        vm.running = sys_poll_event();
        readexec_opcode();
        sys_render(host.pixelbuf);
    }
    // alis loop was stopped by 'cexit', 'cstop', or user event
}


//void alis_register_script(sAlisScript * script) {
//    u8 id = script->header.id;
//    vm.scripts[id] = script;
//    // vm.script_id_stack[vm.script_count++] = id;
//}


void alis_engine(void) {
    
    // will contain a vram address
    u32 d5 = 0;
    u32 a6 = 0;
    u32 a0, a1 = 0;
    
    // $13010
__reprog:
    d5 = 0;
    a0 = vm.vram.atent;
    goto __moteur2;

    // $13026
__moteur1:
    d5 = vm.vram.atent + d5 + 4;
    if (d5 != 0) {
        goto __moteur2;
    }
    // eq to image()
    sys_render(host.pixelbuf);
    // goto __moteur2;
    
__moteur2:
    a6 = vram_read32(vm.vram.atent + d5);
    // a6 = (vm.vram.atent + d5); // TODO: adresse pointée par (a0 + d5) = $22690
    vm.vars.ent.fallent = 0;
    vm.vars.fseq = 0;
    
    // a1 <- script.header[offset_interrupt_routine]
    a1 = vm.script->header.offset_to_interrupt_routine;
    if ((vm.vstat.cscan_cinter_kOffsetMinus36 > 0) ||
        (vm.vstat.cscan_cinter_kOffsetMinus36 & 1) != 0 ||
        a1 == 0) {
        goto __moteur4;
    }
    
// __before_moteur4:
    vm.script->pc = vm.mem + vm.script->data_org + 10 + vm.script->header.offset_to_interrupt_routine;
    vm.varD4 = vm.vstat.vacc_offset_kOffsetMinus10;
    vm.vars.save_rsp = vm.varD4;
    save_coord();
    alis_loop();
    update_coord();
    
    
__moteur4:
    if(vm.vstat.cstart_csleep_kOffsetMinus4 == 0) {
        goto __moteur1;
    }
    else if(vm.vstat.cstart_csleep_kOffsetMinus4 > 0) {
        goto __moteur6;
    }
    else {
        vm.vstat.cstart_csleep_kOffsetMinus4 = 1;
    }
    
__moteur6:
    if(--vm.vstat.cstart_kOffsetMinus1 > 0) {
        goto __moteur1;
    }
    save_coord();
    
    // reset to start of current script
    vm.script->pc = vm.vstat.script_code_addr_kOffsetMinus8;
    vm.varD4 = vm.vstat.vacc_offset_kOffsetMinus10;
    vm.vars.fseq++;
    
    // start opcode execution loop
    alis_loop();
    
    // cstop brought us here
    vm.vstat.vacc_offset_kOffsetMinus10 = vm.varD4;
    vm.vstat.script_code_addr_kOffsetMinus8 = vm.script->pc;
    
    // check offset to script subroutine
    if(vm.script->header.offset_to_subscript_routine == 0) {
        // no subroutine
        goto __moteur5;
    }
    
    vm.vars.fseq = 0;
    vm.script->pc = vm.script->header.offset_to_subscript_routine;
    vm.varD4 = vm.vstat.vacc_offset_kOffsetMinus10;
    vm.vars.save_rsp = vm.varD4;
    
    // start opcode subroutine execution loop
    alis_loop();
    
__moteur5:
    update_coord();
    vm.vstat.cstart_kOffsetMinus1 = vm.vstat.unknown6_kOffsetMinus2;
    goto __moteur1;
}


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
        u32 offset = vm.script->header.offset_to_subscript_routine;
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
