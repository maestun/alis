//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the “Software”), 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "alis.h"
#include "alis_private.h"
#include "audio.h"
#include "image.h"
#include "mem.h"
#include "sys/sys.h"
#include "utils.h"


extern u8 *ampalet;
extern u8 *atpalet;

extern u8 mpalet[768 * 4];
extern u8 tpalet[768 * 4];

sAlisVM alis;
sHost host;


sAlisError errors[] = {
    { ALIS_ERR_FOPEN,   "fopen", "Failed to open file %s\n" },
    { ALIS_ERR_FWRITE,  "fwrite", "Failed to write to file %s\n" },
    { ALIS_ERR_FCREATE, "fcreate", "Failed to create file %s\n" },
    { ALIS_ERR_FDELETE, "fdelete", "Failed to delete file %s\n" },
    { ALIS_ERR_CDEFSC,  "cdefsc", "" },
    { ALIS_ERR_FREAD,   "fread", "Failed to read file %s\n" },
    { ALIS_ERR_FCLOSE,  "fclose", "Failed to close file" },
    { ALIS_ERR_FSEEK,   "fseek", "Failed to seek in file %s\n" }
};

// void vram_debug(void);

// =============================================================================
// MARK: - Private
// =============================================================================
alisRet readexec(sAlisOpcode * table, char * name, u8 identation) {
    
    if (alis.script->pc < alis.script->pc_org || alis.script->pc >= alis.script->pc_org + alis.script->data->sz || alis.script->pc - alis.script->pc_org == kVirtualRAMSize) {
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
    debug(EDebugInfo, "\n%s [%.6x:%.4x]: 0x%06x:", alis.script->name, alis.script->vram_org, (u16)(alis.script->vacc_off), alis.script->pc);
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
    char *tmp = alis.sd7;
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
    char *tmp = alis.sd7;
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
        alis.specs.script_data_tab_len = fread16(fp);
        alis.specs.script_vram_tab_len = fread16(fp);
        alis.specs.unused = fread32(fp);
        alis.specs.max_allocatable_vram = fread32(fp);
        alis.specs.vram_to_data_offset = fread32(fp);
        alis.specs.vram_to_data_offset += 3;
        alis.specs.vram_to_data_offset *= 0x28;
        
        alis.vprotect = 0;
        
        // set the location of scripts' vrams table
        alis.atprog = ALIS_VM_RAM_ORG;
        alis.atprog_ptr = (u32 *)(alis.mem + alis.atprog);
        alis.atent = alis.atprog + 0xf0;
        alis.atent_ptr = (sScriptLoc *)(alis.vram_org + 0xf0); // (alis.specs.script_data_tab_len * sizeof(u32)));
        alis.maxent = alis.specs.script_vram_tab_len;
        alis.debent = alis.atent + alis.maxent * 6;
        alis.finent = alis.debent;

        image.debsprit = ((alis.debent + alis.specs.max_allocatable_vram) | 0xf) + 1;
        image.finsprit = image.debsprit + alis.specs.vram_to_data_offset;
        alis.debprog = image.finsprit;
        alis.finprog = alis.debprog;
        alis.dernprog = alis.atprog;
        alis.maxprog = 0x3c; // TODO: read from script / alis2
        
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

        debug(EDebugInfo, "\
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
        sAlisScriptData *script = script_load(alis.platform.main);
        alis.main = script_live(script);
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

    alis.platform = platform;
    vram_init();

//    alis.nmode = 0; // 0 = atari 16 colors
//                    // 3 = mono
//                    // 8 = falcon 256 colors
    
    memset(audio.tabinst, 0, sizeof(audio.tabinst));

    alis.restart_loop = 0;
    alis.automode = 0;
    
    memset(tpalet, 0, 768 * 4);
    memset(mpalet, 0, 768 * 4);

    // init virtual ram
    alis.mem = malloc(sizeof(u8) * kHostRAMSize);
    memset(alis.mem, 0, sizeof(u8) * kHostRAMSize);

    image.spritemem = (u8 *)malloc(1024 * 1024);
    memset(image.spritemem, 0x0, 1024 * 1024);

    // NOTE: cswitching is never called for older games
    alis.fswitch = 1;
    alis.flagmain = 0;
    
    alis.fallent = 0;
    alis.fseq = 0;
    alis.fmuldes = 0;
    alis.fadddes = 0;
    alis.ferase = 0;

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

    alis.ctiming = 0;
    alis.prevkey = 0;

    // init virtual registers
    alis.varD6 = alis.varD7 = 0;
    
    // init temp chunks
    alis.bsd7 = (char *)(alis.mem + 0x1a1e6);
    alis.bsd6 = (char *)(alis.mem + 0x1a2e6);
    alis.bsd7bis = (char *)(alis.mem + 0x1a3e6);
    
    alis.sd7 = alis.bsd7;
    alis.sd6 = alis.bsd6;
    alis.oldsd7 = alis.bsd7bis;
    
    // init helpers
    if(alis.fp) {
        fclose(alis.fp);
        alis.fp = NULL;
    }
       
    // set the vram origin at some abitrary location (same as atari, to ease debug)
    alis.vram_org = alis.mem + ALIS_VM_RAM_ORG;
    
    // the script data address table is located at vram start
    alis.script_data_orgs = (u32 *)alis.vram_org;
    
    // TODO: init virtual accumulator
//    alis.acc_org = alis.script->vram_org;
//    alis.acc = alis.script->vram_org + kVirtualRAMSize;
    alis.acc = alis.acc_org = (s16 *)(alis.mem + 0x198e2);
    
    // init host system stuff
    host.pixelbuf.w = alis.platform.width;
    host.pixelbuf.h = alis.platform.height;
    host.pixelbuf.data = (u8 *)malloc(host.pixelbuf.w * host.pixelbuf.h);
    memset(host.pixelbuf.data, 0x0, host.pixelbuf.w * host.pixelbuf.h);
    host.pixelbuf.palette = ampalet;
    memset(host.pixelbuf.palette, 0xff, 768);

    // load main script
    alis_load_main();
    alis.script = alis.main;
    alis.basemain = alis.main->vram_org;
    
    gettimeofday(&alis.time, NULL); // TODO: remove system dependencies
}


void alis_deinit(void) {
    // free scripts
    // TODO: use real script table / cunload
    // for(int i = 0; i < kMaxScripts; i++) {
    //     script_unload(alis.script);
    // }
    
    //vram_deinit(alis.vram);
    // free(host.pixelbuf.data);
    free(alis.mem);
}

void alis_loop(void) {
    alis.script->running = 1;
    while (alis.running && alis.script->running) {
        
        readexec_opcode();
    }
    
    // alis loop was stopped by 'cexit', 'cstop', or user event
}

void savecoord(u8 *a6)
{
    // ishar 1&2
    image.oldcx = read16(a6 + 0);
    image.oldcy = read16(a6 + 2);
    image.oldcz = read16(a6 + 4);

//    //ishar 3
//    oldcx = *(u16 *)(a6 + 0x00);
//    oldcy = *(u16 *)(a6 + 0x08);
//    oldcz = *(u16 *)(a6 + 0x10);
//    oldacx = *(u16 *)(a6 + 0x18);
//    oldacy = *(u16 *)(a6 + 0x20);
//    oldacz = *(u16 *)(a6 + 0x28);
}

void updtcoord(u8 *a6)
{
    // ishar 1&2
    s16 addx = read16(a6 + 0) - image.oldcx;
    s16 addy = read16(a6 + 2) - image.oldcy;
    s16 addz = read16(a6 + 4) - image.oldcz;
    if (addz != 0 || addx != 0 || addy != 0)
    {
        for (sSprite *sprite = SPRITE_VAR(get_0x18_unknown(alis.script->vram_org)); sprite != NULL; sprite = SPRITE_VAR(sprite->to_next))
        {
            if (sprite->state == 0)
                sprite->state = 2;
            
            sprite->depx += addx;
            sprite->depy += addy;
            sprite->depz += addz;
        }
    }
    
    // ishar 3
//    s16 angle = *(u16 *)(a6 + 0x18);
//    if (angle != oldacx)
//    {
//        if ((0x168 < angle) && 0x168 < (angle = angle - 0x168))
//            angle = angle % 0x168;
//
//        if (angle < -0x168 && (angle += 0x168) < -0x168)
//            angle = angle % 0x168;
//
//        *(s16 *)(a6 + 0x18) = angle;
//    }
//
//    angle = *(u16 *)(a6 + 0x20);
//    if (angle != oldacy)
//    {
//        if ((0x168 < angle) && 0x168 < (angle = angle - 0x168))
//            angle = angle % 0x168;
//
//        if (angle < -0x168 && (angle += 0x168) < -0x168)
//            angle = angle % 0x168;
//
//        *(s16 *)(a6 + 0x20) = angle;
//    }
//
//    angle = *(u16 *)(a6 + 0x28);
//    if (angle != oldacy)
//    {
//        if ((0x168 < angle) && 0x168 < (angle = angle - 0x168))
//            angle = angle % 0x168;
//
//        if (angle < -0x168 && (angle += 0x168) < -0x168)
//            angle = angle % 0x168;
//
//        *(s16 *)(a6 + 0x28) = angle;
//    }
//
//    s16 addx = *(s16 *)(a6 + 0x00) - oldcx;
//    s16 addy = *(s16 *)(a6 + 0x08) - oldcy;
//    s16 addz = *(s16 *)(a6 + 0x10) - oldcz;
//    u16 unknown28 = *(u16 *)(a6 + 0x28);
//
//    if (angle != oldacz || addz != 0 || addx != 0 || addy != 0)
//    {
//        for (SpriteVariables *sprite = SPRITE_VAR(get_0x18_unknown(alis.script->vram_org)); sprite != NULL; sprite = SPRITE_VAR(sprite->to_next))
//        {
//            if (sprite->state == 0)
//                sprite->state = 2;
//
//            sprite->depx += addx;
//            sprite->depy += addy;
//            sprite->depz += addz;
//            sprite->sprite_0x28 = unknown28;
//        }
//    }
}

void alis_main(void) {
    do {
        alis.running = sys_poll_event();
        alis.restart_loop = 0;
        
        itroutine();
        
        alis.script = ENTSCR(alis.varD5);
        u8 *vram_addr = alis.mem + alis.script->vram_org;
        
        alis.fallent = 0;
        alis.fseq = 0;
        
        if (get_0x24_scan_inter(alis.script->vram_org) < 0 && (get_0x24_scan_inter(alis.script->vram_org) & 2) == 0) 
        {
            s32 script_offset = swap32((alis.mem + get_0x14_script_org_offset(alis.script->vram_org) + 10));
            if (script_offset != 0)
            {
                alis.script->vacc_off = alis.saversp = get_0x0a_vacc_offset(alis.script->vram_org);
                savecoord(vram_addr);
                alis.script->pc = get_0x14_script_org_offset(alis.script->vram_org) + 10 + script_offset;
                alis_loop();
                updtcoord(vram_addr);
            }
        }
        
        if (get_0x04_cstart_csleep(alis.script->vram_org) == 0)
        {
            debug(EDebugInfo, "\n SLEEPING %s", alis.script->name);
        }

        if (get_0x04_cstart_csleep(alis.script->vram_org) != 0)
        {
            if ((s8)get_0x04_cstart_csleep(alis.script->vram_org) < 0)
            {
                set_0x04_cstart_csleep(alis.script->vram_org, 1);
            }
            
            set_0x01_wait_count(alis.script->vram_org, get_0x01_wait_count(alis.script->vram_org) - 1);
            debug(EDebugInfo, "\n %s %s [%.2x, %.2x] ", get_0x01_wait_count(alis.script->vram_org) == 0 ? "RUNNING" : "WAITING", alis.script->name, get_0x01_wait_count(alis.script->vram_org), get_0x02_wait_cycles(alis.script->vram_org));
            if (get_0x01_wait_count(alis.script->vram_org) == 0)
            {
                savecoord(vram_addr);
                
                alis.script->pc = get_0x08_script_ret_offset(alis.script->vram_org);
                alis.script->vacc_off = get_0x0a_vacc_offset(alis.script->vram_org);
                alis.fseq++;
                alis_loop();
                
                if (alis.restart_loop != 0)
                {
                    alis.varD5 = xread16(alis.atent + 4 + alis.varD5);
                    if (alis.varD5 == 0)
                    {
                        alis.script = ENTSCR(alis.varD5);
                        draw();
                    }

                    continue;
                }

                set_0x0a_vacc_offset(alis.script->vram_org, alis.script->vacc_off);
                set_0x08_script_ret_offset(alis.script->vram_org, alis.script->pc);
                
                s32 script_offset = swap32(alis.mem + get_0x14_script_org_offset(alis.script->vram_org) + 6);
                if (script_offset != 0)
                {
                    alis.fseq = 0;
                    alis.script->vacc_off = alis.saversp = get_0x0a_vacc_offset(alis.script->vram_org);
                    alis.script->pc = get_0x14_script_org_offset(alis.script->vram_org) + 6 + script_offset;
                    alis_loop();
                }
                
                updtcoord(vram_addr);
                
                set_0x01_wait_count(alis.script->vram_org, get_0x02_wait_cycles(alis.script->vram_org));
                alis.acc = alis.acc_org;
            }
        }
        
        alis.varD5 = xread16(alis.atent + 4 + alis.varD5);
        if (alis.varD5 == 0)
        {
            alis.script = ENTSCR(alis.varD5);
            draw();
        }
    }
    while (alis.running);
}


u8 alis_start(void) {
    u8 ret = 0;
    
    // run !
    alis.running = 1;
    alis.cstopret = 0;
    alis.varD5 = 0;
    alis_main();
    
    // alis was stopped by 'cexit' opcode
    return ret;
}

void alis_error(int errnum, ...) {
    va_list args;
    va_start(args, errnum);
    
    sAlisError err = { 0,0,0 };

    int len = sizeof(errors) / sizeof(sAlisError);
    for (int e = 0; e < len; e++)
    {
        if (errors[e].errnum == errnum)
        {
            err = errors[e];
            break;
        }
    }
    
    vdebug(EDebugError, err.descfmt, args);
    va_end(args);
    exit(-1);
}


// =============================================================================
// MARK: - MEMORY ACCESS
// =============================================================================

typedef void (*fWrite16)(u32, s16);
typedef void (*fWrite32)(u32, s32);

static fWrite16 _write16_fptr;
static fWrite32 _write24_fptr;
static fWrite32 _write32_fptr;

typedef u16 (*fConvert16)(u16);
typedef u32 (*fConvert32)(u32);

static fConvert16 _convert16;
static fConvert32 _convert24;
static fConvert32 _convert32;

static fConvert16 _pcconvert16;
static fConvert32 _pcconvert24;
static fConvert32 _pcconvert32;

static u16 _swap16(u16 value) {
    return (value <<  8) | (value >>  8);
}

static u16 _linear16(u16 value) {
    return value;
}

static u32 _swap24(u32 value) {
    return (((value >> 24) & 0xff) | 
            ((value <<  8) & 0xff0000) | 
            ((value >>  8) & 0xff00)) >> 8;
}

static u32 _linear24(u32 value) {
    return value;
}

static u32 _swap32(u32 value) {
    return ((value >> 24) & 0xff) |
           ((value <<  8) & 0xff0000) |
           ((value >>  8) & 0xff00) |
           ((value << 24) & 0xff000000);
}

static u32 _linear32(u32 value) {
    return value;
}

void vram_init(void) {
    _convert16 = (alis.platform.is_little_endian == is_host_le()) ? _linear16 : _swap16;
    _convert24 = (alis.platform.is_little_endian == is_host_le()) ? _linear24 : _swap24;
    _convert32 = (alis.platform.is_little_endian == is_host_le()) ? _linear32 : _swap32;

    _pcconvert16 = (alis.platform.kind == EPlatformPC) ? ((alis.platform.is_little_endian == is_host_le()) ? _swap16 : _linear16) : _convert16;
    _pcconvert24 = (alis.platform.kind == EPlatformPC) ? ((alis.platform.is_little_endian == is_host_le()) ? _swap24 : _linear24) : _convert24;
    _pcconvert32 = (alis.platform.kind == EPlatformPC) ? ((alis.platform.is_little_endian == is_host_le()) ? _swap32 : _linear32) : _convert32;
}

u16 read16(const u8 *ptr) {
    return _convert16(*((u16*)ptr));
}

u32 read24(const u8 *ptr) {
    u32 result = 0;
    result = (u32)ptr[0] | ((u32)ptr[1] << 8) | ((u32)ptr[2] << 16);
    result = _convert24(result);
    return (result > 0x7FFFFF) ? (result << 8) & 0xff : result;
}

u32 read32(const u8 *ptr) {
    return _convert32(*((u32*)ptr));
}

void write32(const u8 *ptr, u32 value) {
    *((u32*)ptr) = _convert32(value);
}

u16 swap16(const u8 *ptr) {
    return _convert16(*((u16*)ptr));
}

u32 swap32(const u8 *ptr) {
    return _convert32(*((u32*)ptr));
}

u8 xread8(u32 offset) {
    debug(EDebugVerbose, " [%.2x <= %.6x]", *(u8 *)(alis.mem + offset), offset);
    return *(alis.mem + offset);
}

s16 xread16(u32 offset) {
    u16* ptr = (u16*)(alis.mem + offset);
    u16 result = _convert16(*ptr);
    debug(EDebugVerbose, " [%.4x <= %.6x]", result, offset);
    return (s16)result;
}

s32 xread32(u32 offset) {
    u32* ptr = (u32*)(alis.mem + offset);
    s32 result = _convert32(*ptr);
    debug(EDebugVerbose, " [%.4x <= %.6x]", result, offset);
    return (s32)result;
}


s16 xpcread16(u32 offset) {
    u16* ptr = (u16*)(alis.mem + offset);
    u16 result = _pcconvert16(*ptr);
    debug(EDebugVerbose, " [%.4x <= %.6x]", result, offset);
    return (s16)result;
}

s32 xpcread32(u32 offset) {
    u32* ptr = (u32*)(alis.mem + offset);
    s32 result = _pcconvert32(*ptr);
    debug(EDebugVerbose, " [%.4x <= %.6x]", result, offset);
    return (s32)result;
}

void xpcwrite16(u32 offset, s16 value) {
    debug(EDebugVerbose, " [%.4x => %.6x]", value, offset);
    *(s16 *)(alis.mem + offset) = _pcconvert16(value);
}

void xpcwrite32(u32 offset, s32 value) {
    debug(EDebugVerbose, " [%.8x => %.6x]", value, offset);
    *(s32 *)(alis.mem + offset) = _pcconvert32(value);
}

u8 * get_vram(s16 offset) {
    debug(EDebugVerbose, " [%s <= %.6x]", (char *)(alis.mem + alis.script->vram_org + offset), alis.script->vram_org + offset);
    return (u8 *)(alis.mem + alis.script->vram_org + offset);
}

u16 xswap16(u16 value) {
    return _convert16(value);
}

u32 xswap32(u32 value) {
    return _convert32(value);
}

u8 * xreadptr(u32 offset) {
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + offset), offset);
    return (u8 *)(alis.mem + offset);
}

void xwrite8(u32 offset, u8 value) {
    debug(EDebugVerbose, " [%.2x => %.6x]", value, offset);
    *(u8 *)(alis.mem + offset) = value;
}

void xwrite16(u32 offset, s16 value) {
    debug(EDebugVerbose, " [%.4x => %.6x]", value, offset);
    *(s16 *)(alis.mem + offset) = _convert16(value);
}

void xwrite32(u32 offset, s32 value) {
    debug(EDebugVerbose, " [%.8x => %.6x]", value, offset);
    *(s32 *)(alis.mem + offset) = _convert32(value);
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

void xadd32(s32 offset, s32 addition) {
    s32 value = xread32(offset);
    debug(EDebugVerbose, " [%d + %d => %.6x]", value, addition, offset);
    xwrite32(offset, value + addition);
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

void xsub32(s32 offset, s32 sub) {
    s32 value = xread32(offset);
    debug(EDebugVerbose, " [%d - %d => %.6x]", value, sub, offset);
    xwrite32(offset, value - sub);
}

void xpush32(s32 value) {
    alis.script->vacc_off -= sizeof(u32);
    xwrite32(alis.script->vram_org + alis.script->vacc_off, value);
    debug(EDebugInfo, " [%.8x => va %.4x + %.6x (%.6x)]", value, (s16)alis.script->vacc_off, alis.script->vram_org, alis.script->vacc_off + alis.script->vram_org);
}

s32 xpeek32(void) {
    return xread32(alis.script->vram_org + alis.script->vacc_off);
}

s32 xpop32(void) {
    s32 ret = xpeek32();
    debug(EDebugInfo, " [%.8x <= va %.4x + %.6x (%.6x)]", ret, (s16)alis.script->vacc_off, alis.script->vram_org, alis.script->vacc_off + alis.script->vram_org);
    alis.script->vacc_off += sizeof(s32);
    return ret;
}

u32 fread32(FILE* fp) {
    u32 v = 0;
    fread(&v, sizeof(u32), 1, fp);
    return swap32((u8 *)&v);
}

u16 fread16(FILE* fp) {
    u16 v = 0;
    fread(&v, sizeof(u16), 1, fp);
    return swap16((u8 *)&v);
}


// =============================================================================
#pragma mark - Finding resources in the script
// =============================================================================

s32 adresdei(s32 idx)
{
    u32 addr = get_0x14_script_org_offset(alis.main->vram_org);
    s32 l = xread32(addr + 0xe);
    s32 e = xread16(addr + l + 4);
    if (e > idx)
    {
        s32 a = xread32(addr + l) + l + idx * 4;
        return addr + a;
    }

    return 0xf;
}

s32 adresdes(s32 idx)
{
    if (alis.flagmain != 0)
        return adresdei(idx);
    
    u32 addr = get_0x14_script_org_offset(alis.script->vram_org);
    s32 l = xread32(addr + 0xe);
    s32 e = xread16(addr + l + 4);
    if (e > idx)
    {
        s32 a = xread32(addr + l) + l + idx * 4;
        return addr + a;
    }

    return 0xf;
}

s32 adresform(s16 idx)
{
    // TODO: fix this
    u8 *addr = alis.mem + get_0x14_script_org_offset(alis.script->vram_org);
    addr += read32(addr + 0xe);
    addr += read32(addr + 0x6);
    addr += read16(addr + (idx * 2));
    return (s32)(addr - alis.mem);
}

s32 adresmui(s32 idx)
{
    u8 *addr = alis.mem + get_0x14_script_org_offset(alis.main->vram_org);
    s32 l = read32(addr + 0xe);
    s32 e = read16(addr + 0x10 + l);
    if (e > idx)
    {
        s32 a = read32(addr + 0xc + l) + l + idx * 4;
        return read32(addr + a) + a;
    }

    return 0x11;
}

s32 adresmus(s32 idx)
{
    if (alis.flagmain != 0)
        return adresmui(idx);

    u8 *addr = alis.mem + get_0x14_script_org_offset(alis.script->vram_org);
    s32 l = read32(addr + 0xe);
    s32 e = read16(addr + 0x10 + l);
    if (e > idx)
    {
        s32 a = read32(addr + 0xc + l) + l + idx * 4;
        return read32(addr + a) + a;
    }

    return 0x11;
}

#pragma mark -
#pragma mark tab functions

s16 tabchar(s16 offset, u8 *address)
{
    s16 *ptr = (s16 *)(address + offset - 2);
    s16 result = offset + alis.varD7;
    s16 length = *(s8 *)(address + offset - 1);
    for (int i = 0; i < length; i++)
    {
        result += xswap16(*(--ptr)) * *alis.acc++;
        debug(EDebugInfo, "\n  [%.8x => %.6x]", result, xswap16(*(ptr)) * *alis.acc);
    }

    return result;
}

s16 tabstring(s16 offset, u8 *address)
{
    s16 *ptr = (s16 *)(address + offset - 2);
    s16 result = offset + alis.varD7 * (ushort)*(u8 *)ptr;
    s16 length = *(s8 *)(address + offset - 1);
    for (int i = 0; i < length; i++)
    {
        result += xswap16(*(--ptr)) * *alis.acc++;
        debug(EDebugInfo, "\n  [%.8x => %.6x]", result, xswap16(*(ptr)) * *alis.acc);
    }
    
    return result;
}

s16 tabint(s16 offset, u8 *address)
{
    s16 *ptr = (s16 *)(address + offset - 2);
    s16 result = offset + alis.varD7 * 2;
    s16 length = *(s8 *)(address + offset - 1);
    for (int i = 0; i < length; i++)
    {
        result += xswap16(*(--ptr)) * *alis.acc++;
        debug(EDebugInfo, "\n  [%.8x => %.6x]", result, xswap16(*(ptr)) * *alis.acc);
    }
    
    return result;
}
