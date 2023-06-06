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

#include "experimental.h"

sAlisVM alis;
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

void vram_debug(void);

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
        u8 code = *(alis.mem + alis.script->pc++);
        sAlisOpcode opcode = table[code];
        debug(EDebugInfo, " %s", opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
        return opcode.fptr();
    }
}

alisRet readexec_opcode(void) {
    debug(EDebugInfo, "\n%s [%.6x]: 0x%06x:", alis.script->name, alis.script->vram_org, alis.script->pc/* - alis.script->pc_org*/);
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
    u8 * tmp = alis.sd7;
    alis.sd7 = alis.oldsd7;
    alis.oldsd7 = tmp;
    
//    s32 t0 = (s32)(alis.sd7 - alis.mem);
//    s32 t1 = (s32)(alis.oldsd7 - alis.mem);
//
//    printf("ReadExecAddName(void): Write to address $0195e4, new value is %d ($%x)\n", t0 & 0xffff, t0 & 0xffff);
//    printf("ReadExecAddName(void): Write to address $0195ec, new value is %d ($%x)\n", t1 & 0xffff, t1 & 0xffff);

    return readexec_addname();
}

alisRet readexec_opername_saveD7(void) {
    alis.varD6 = alis.varD7;
    return readexec_opername();
}

alisRet readexec_opername_saveD6(void) {
    
    s16 tmp = alis.varD7;
    readexec_opername_saveD7();
    alis.varD6 = alis.varD7;
    alis.varD7 = tmp;
}

alisRet readexec_opername_swap(void) {
    u8 * tmp = alis.sd7;
    alis.sd7 = alis.sd6;
    alis.sd6 = tmp;
    
//    s32 t0 = (s32)(alis.sd7 - alis.mem);
//    s32 t1 = (s32)(alis.sd6 - alis.mem);
//
//    printf("ReadExecAddName(void): Write to address $0195e4, new value is %d ($%x)\n", t0 & 0xffff, t0 & 0xffff);
//    printf("ReadExecAddName(void): Write to address $0195e8, new value is %d ($%x)\n", t1 & 0xffff, t1 & 0xffff);

    return readexec_opername();
}


void alis_load_main(void) {
    
    // 22400    = atprog > 2edd8 (22400 > 34ba8) ALIS_VM_RAM_ORG
    // 224f0    = atent
    // 2261c    = finent
    // 22690    = header
    
    alis.nbprog = 0;
    
    // packed main script contains vm specs in the header
    FILE * fp = fopen(alis.platform.main, "rb");
    if (fp) {
        // skip 6 bytes
        fseek(fp, 6, SEEK_CUR);
        
        // read raw specs header
        alis.specs.script_data_tab_len = fread16(fp, alis.platform.is_little_endian);
        alis.specs.script_vram_tab_len = fread16(fp, alis.platform.is_little_endian);
        alis.specs.unused = fread32(fp, alis.platform.is_little_endian);
        alis.specs.max_allocatable_vram = fread32(fp, alis.platform.is_little_endian);
        alis.specs.vram_to_data_offset = fread32(fp, alis.platform.is_little_endian);
        alis.specs.vram_to_data_offset += 3;
        alis.specs.vram_to_data_offset *= 0x28;
        
        // set the location of scripts' vrams table
        alis.atprog = ALIS_VM_RAM_ORG;
        alis.atprog_ptr = (u32 *)(alis.mem + alis.atprog);
        alis.atent = alis.atprog + 0xf0;
        alis.atent_ptr = (sScriptLoc *)(alis.vram_org + 0xf0); // (alis.specs.script_data_tab_len * sizeof(u32)));
        // alis.atent = (s32)((u8 *)alis.atent_ptr - alis.mem);
        alis.maxent = alis.specs.script_vram_tab_len;
        alis.debent = alis.atent + alis.maxent * 6;
        alis.finent = alis.debent;

        alis.debsprit = ((alis.debent + alis.specs.max_allocatable_vram) | 0xf) + 1;
        alis.finsprit = alis.debsprit + alis.specs.vram_to_data_offset;
        alis.debprog = alis.finsprit;
        alis.finprog = alis.debprog;
        alis.dernprog = alis.atprog;
        alis.maxprog = 0x3c;
        
        // TODO: ...
        alis.finmem = 0xf6e98;

        inisprit();

        // compute the end address of the scripts' vrams table
        u32 script_vram_tab_end = (u32)((u8 *)alis.atent_ptr - alis.mem + (alis.specs.script_vram_tab_len * sizeof(sScriptLoc)));

        // populate the script vrams table with the offsets (routine at $18cd8)
        for(int idx = 0; idx < alis.specs.script_vram_tab_len; idx++) {
            u16 offset = xswap16((1 + idx) * sizeof(sScriptLoc));
            alis.atent_ptr[idx] = (sScriptLoc){0, offset};
        }

        alis.specs.script_vram_max_addr = ((alis.debent + alis.specs.max_allocatable_vram) | 0xf) + 1; // ((script_vram_tab_end + alis.specs.max_allocatable_vram) | 0b111) + 1;

        u32 main_script_data_addr = alis.specs.script_vram_max_addr + alis.specs.vram_to_data_offset;

        debug(EDebugVerbose, "\
- script data table count: %d (0x%x), located at 0x%x\n\
- script vram table count: %d (0x%x), located at 0x%x, ends at 0x%x\n\
- script data located at 0x%x\n\
- vram allocatable up to 0x%x \n\
- unused (?) dword from header: 0x%x\n",
              alis.specs.script_data_tab_len, alis.specs.script_data_tab_len, (u8 *)alis.script_data_orgs - alis.mem,
              alis.specs.script_vram_tab_len, alis.specs.script_vram_tab_len, (u8 *)alis.atent_ptr - alis.mem,
              script_vram_tab_end,
              main_script_data_addr,
              alis.specs.script_vram_max_addr,
              alis.specs.unused);
        fclose(fp);

        // load main scripts as an usual script...
        alis.main = script_load(alis.platform.main);
        script_live(alis.main);
        alis.basemain = alis.main->vram_org;

        alis.dernent = xswap16(alis.atent_ptr[0].offset);
        alis.atent_ptr[0].offset = 0;
    }
}


// =============================================================================
// MARK: - VM API
// =============================================================================

void alis_init(sPlatform platform) {
    debug(EDebugVerbose, "ALIS: Init.\n");

    alis.nmode = 0; // 0 = atari 16 colors
                    // 3 = mono
                    // 8 = falcon 256 colors

    alis.restart_loop = 0;
    alis.automode = 0;

    // init virtual ram
    alis.mem = malloc(sizeof(u8) * kHostRAMSize);
    memset(alis.mem, 0, sizeof(u8) * kHostRAMSize);

    alis.spritemem = (u8 *)malloc(1024 * 1024);
    memset(alis.spritemem, 0x0, 1024 * 1024);

    alis.flagmain = 0;
    alis.numelem = 0;
    
    alis.fallent = 0;
    alis.fseq = 0;
    alis.flaginvx = 0;
    alis.fmuldes = 0;
    alis.fadddes = 0;
    alis.ferase = 0;

    alis.depx = 0;
    alis.depy = 0;
    alis.depz = 0;

    alis.saversp = 0;
    alis.basemem = 0x22400;
    alis.basevar = 0;
    alis.finmem = 0x22000;

    alis.nbprog = 0;
    alis.maxprog = 0;
    alis.atprog = alis.basemem;
    
    alis.nbent = 0;
    alis.ptrent = alis.tablent;

    memset(alis.tablent, 0, sizeof(alis.tablent));
    memset(alis.matent, 0, sizeof(alis.matent));
    memset(alis.buffer, 0, sizeof(alis.buffer));

    alis._ctiming = 0;
    
    alis.prevkey = 0;

//    alis.atent = alis.atprog + 0xf0;
//    alis.maxent = 0x32;
//    alis.debent = alis.atent + alis.maxent * 6;
//    alis.debsprit = alis.debent;
//    alis.finsprit = alis.debent + 0xc7bc;
//    alis.debprog = alis.finsprit;
//    alis.finprog = alis.debprog;
//    alis.dernprog = alis.atprog;
//    alis.finent = alis.debent;
//    // alis.basemain
//
////    alis.debsprit = 0;
////    alis.finsprit = 0x8000;
//
//    inisprit();
    
    // init virtual registers
    alis.varD6 = alis.varD7 = 0;
    
    // init temp chunks
    // TODO: might as well (static) alloc some ram.
    alis.bsd7 = alis.mem + 0x1a1e6;
    alis.bsd6 = alis.mem + 0x1a2e6;
    alis.bsd7bis = alis.mem + 0x1a3e6;
    
    alis.sd7 = alis.bsd7;
    alis.sd6 = alis.bsd6;
    alis.oldsd7 = alis.bsd7bis;
    
//    s32 t0 = (s32)(alis.sd7 - alis.mem);
//    s32 t1 = (s32)(alis.sd6 - alis.mem);
//    s32 t2 = (s32)(alis.oldsd7 - alis.mem);
//
//    printf("FUN_STARTUP: Write to address $0195e4, new value is %d ($%x)\n", t0 & 0xffff, t0 & 0xffff);
//    printf("FUN_STARTUP: Write to address $0195e8, new value is %d ($%x)\n", t1 & 0xffff, t1 & 0xffff);
//    printf("FUN_STARTUP: Write to address $0195ec, new value is %d ($%x)\n", t2 & 0xffff, t2 & 0xffff);
    
    // init helpers
    if(alis.fp) {
        fclose(alis.fp);
        alis.fp = NULL;
    }
    
    alis.platform = platform;
    
    // set the vram origin at some abitrary location (same as atari, to ease debug)
    alis.vram_org = alis.mem + ALIS_VM_RAM_ORG;
    
    // the script data address table is located at vram start
    alis.script_data_orgs = (u32 *)alis.vram_org;
    
    // TODO: init virtual accumulator
//    alis.acc_org = alis.script->vram_org;
//    alis.acc = alis.script->vram_org + kVirtualRAMSize;
    alis.acc = alis.acc_org = (s16 *)(alis.mem + 0x198e2);
    //alis.script_count = 0;
    
    // init host system stuff
    host.pixelbuf.w = alis.platform.width;
    host.pixelbuf.h = alis.platform.height;
    host.pixelbuf.data = (u8 *)malloc(host.pixelbuf.w * host.pixelbuf.h);
    memset(host.pixelbuf.data, 0x0, host.pixelbuf.w * host.pixelbuf.h);
    host.pixelbuf.palette = ampalet;
    memset(host.pixelbuf.palette, 0xff, 256 * 3);
    
//    // NOTE: grayscale pal for debugging
//    for (int ii = 0; ii < 16; ii++)
//    {
//        for (int i = 0; i < 16; i++)
//        {
//            host.pixelbuf.palette[(ii * 16 * 3) + (i * 3) + 0] = 80 + i * 10;
//            host.pixelbuf.palette[(ii * 16 * 3) + (i * 3) + 1] = 80 + i * 10;
//            host.pixelbuf.palette[(ii * 16 * 3) + (i * 3) + 2] = 80 + i * 10;
//        }
//    }

    // load main script
    alis_load_main();
    alis.script = alis.main;
    alis.basemain = alis.main->vram_org;
    
    gettimeofday(&alis.time, NULL);
}


void alis_deinit(void) {
    // free scripts
    // TODO: use real script table / cunload
    for(int i = 0; i < kMaxScripts; i++) {
        script_unload(alis.script);
    }
//    free(alis.bssChunk1);
//    free(alis.bssChunk2);
//    free(alis.bssChunk3);
    
    //vram_deinit(alis.vram);
    free(host.pixelbuf.data);
    free(alis.mem);
}

void alis_loop(void) {
    alis.script->running = 1;
    while (alis.running && alis.script->running) {
        
        if (xread16(0x269de) == 0x600)
        {
            sleep(0);
        }
        
        if (alis.script->pc == 0x057138)
        {
            sleep(0);
        }
        
        readexec_opcode();
    }
    
    // alis loop was stopped by 'cexit', 'cstop', or user event
}

u8 alis_main(void) {
    u8 ret = 0;
    
    // run !
    alis.running = 1;
    while (alis.running) {
        
        alis._cstopret = 0;
        alis._callentity = 0;
        alis.varD5 = 0;
        moteur();
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

u8 * get_vram(s16 offset) {
    debug(EDebugVerbose, " [%s <= %.6x]", (char *)(alis.mem + alis.script->vram_org + offset), alis.script->vram_org + offset);
    return (u8 *)(alis.mem + alis.script->vram_org + offset);
}

u16 xswap16(u16 value) {
    u16 result = value;
    return alis.platform.is_little_endian == is_host_le() ? result : (result <<  8) | (result >>  8);
}

u32 xswap24(u32 value) {
    u32 result = value;
    return alis.platform.is_little_endian == is_host_le() ? result : (((result >> 24) & 0xff) | ((result <<  8) & 0xff0000) | ((result >>  8) & 0xff00)) >> 8;
}

u32 xswap32(u32 value) {
    u32 result = value;
    return alis.platform.is_little_endian == is_host_le() ? result : ((result >> 24) & 0xff) | ((result <<  8) & 0xff0000) | ((result >>  8) & 0xff00) | ((result << 24) & 0xff000000);
}

u8 * xreadptr(u32 offset) {
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + offset), offset);
    return (u8 *)(alis.mem + offset);
}

u8 xread8(u32 offset) {
    debug(EDebugVerbose, " [%.2x <= %.6x]", *(u8 *)(alis.mem + offset), offset);
    return *(alis.mem + offset);
}

s16 xread16(u32 offset) {
    s16 result = swap16((alis.mem + offset), alis.platform.is_little_endian);
//    s16 result = *(s16 *)(alis.mem + offset);
    debug(EDebugVerbose, " [%.4x <= %.6x]", result, offset);
    return result;
}

s32 xread32(u32 offset) {
    s32 result = swap32((alis.mem + offset), alis.platform.is_little_endian);
//    s32 result = *(s32 *)(alis.mem + offset);
    debug(EDebugVerbose, " [%.8x <= %.6x]", result, offset);
    return result;
}

void xwrite8(u32 offset, u8 value) {
    debug(EDebugVerbose, " [%.2x => %.6x]", value, offset);
    *(u8 *)(alis.mem + offset) = value;
    if (xread16(0x269de) == 0x600)
    {
        sleep(0);
    }
}

void xwrite16(u32 offset, s16 value) {
    debug(EDebugVerbose, " [%.4x => %.6x]", value, offset);
    *(s16 *)(alis.mem + offset) = swap16((u8 *)&value, alis.platform.is_little_endian);
//    *(s16 *)(alis.mem + offset) = value;
    if (xread16(0x269de) == 0x600)
    {
        sleep(0);
    }
}

void xwrite32(u32 offset, s32 value) {
    debug(EDebugVerbose, " [%.8x => %.6x]", value, offset);
    *(s32 *)(alis.mem + offset) = swap32((u8 *)&value, alis.platform.is_little_endian);
//    *(s32 *)(alis.mem + offset) = value;
}

void xadd8(s32 offset, s8 value) {
    debug(EDebugVerbose, " [%d + %d => %.6x]", *(u8 *)(alis.mem + offset), value, offset);
    *(s8 *)(alis.mem + offset) += value;
}

void xadd16(s32 offset, s16 addition) {
    s16 value = xread16(offset);
    debug(EDebugVerbose, " [%d + %d => %.6x]", value, addition, offset);
    xwrite16(offset, value + addition);
}

void xsub8(s32 offset, s8 value) {
    debug(EDebugVerbose, " [%d - %d => %.6x]", *(u8 *)(alis.mem + offset), value, offset);
    *(s8 *)(alis.mem + offset) -= value;
}

void xsub16(s32 offset, s16 sub) {
    s16 value = xread16(offset);
    debug(EDebugVerbose, " [%d - %d => %.6x]", value, sub, offset);
    xwrite16(offset, value - sub);
}

void xpush32(u32 offset, u32 value) {
    alis.script->vacc_off -= sizeof(u32);
    xwrite32(offset + alis.script->vacc_off, value);
}

s32 xpeek32(u32 offset) {
    return xread32(offset + alis.script->vacc_off);
}

s32 xpop32(s32 offset) {
    s32 ret = xpeek32(offset);
    alis.script->vacc_off += sizeof(s32);
    return ret;
}

u8 vread8(u32 offset) {
    debug(EDebugInfo, " [%.2x <= %.6x]", *(u8 *)(alis.mem + offset), offset);
    return *(alis.mem + offset);
}

s16 vread16(u32 offset) {
    s16 result = swap16((alis.mem + offset), alis.platform.is_little_endian);
//    s16 result = *(s16 *)(alis.mem + offset);
    debug(EDebugInfo, " [%.4x <= %.6x]", result, offset);
    return result;
}

s32 vread32(u32 offset) {
    s32 result = swap32((alis.mem + offset), alis.platform.is_little_endian);
//    s32 result = *(s32 *)(alis.mem + offset);
    debug(EDebugInfo, " [%.8x <= %.6x]", result, offset);
    return result;
}

void vwrite8(u32 offset, u8 value) {
    debug(EDebugInfo, " [%.2x => %.6x]", value, offset);
    *(u8 *)(alis.mem + offset) = value;
    if (xread16(0x269de) == 0x600)
    {
        sleep(0);
    }
}

void vwrite16(u32 offset, s16 value) {
    debug(EDebugInfo, " [%.4x => %.6x]", value, offset);
    *(s16 *)(alis.mem + offset) = swap16((u8 *)&value, alis.platform.is_little_endian);
//    *(s16 *)(alis.mem + offset) = value;
    if (xread16(0x269de) == 0x600)
    {
        sleep(0);
    }
}

void vwrite32(u32 offset, s32 value) {
    debug(EDebugInfo, " [%.8x => %.6x]", value, offset);
    *(s32 *)(alis.mem + offset) = swap32((u8 *)&value, alis.platform.is_little_endian);
//    *(s32 *)(alis.mem + offset) = value;
}

/**
 * @brief Returns a byte pointer on (vram+offset)
 *
 * @param offset position in vram
 * @return u8*
 */

//void vram_setbit(s16 offset, u8 bit) {
//    BIT_SET(*(u8 *)(alis.mem + alis.script->vram_org + offset), bit);
//}
//
//void vram_clrbit(s16 offset, u8 bit) {
//    BIT_CLR(*(u8 *)(alis.mem + alis.script->vram_org + offset), bit);
//}
//
// =============================================================================
// MARK: - Virtual RAM debug
// =============================================================================
void vram_debug(void) {
    u8 width = 16;
    
    printf("Stack Offset=0x%04x (word=0x%04x) (dword=0x%08x)\n",
           alis.script->vacc_off,
           (u16)(xread16(alis.script->vram_org + alis.script->vacc_off)),
           (u32)(xread32(alis.script->vram_org + alis.script->vacc_off)));

    printf("Virtual RAM:\n");
    
    printf("       ");
    for(u8 j = 0; j < width; j++) {
        printf("  %x", j);
    }
    printf("\n");
    
    for(u32 i = 0; i < kVirtualRAMSize; i += width) {
        printf("0x%04x: ", i);
        for(u8 j = 0; j < width; j++) {
            printf("%02x ", get_vram(0)[i + j]);
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
        printf("%02x ", (alis.mem + alis.script->vram_org)[j]);
    }
}

