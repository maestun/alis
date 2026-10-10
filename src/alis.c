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

// This is a VM-core translation unit: opt into hot-state register pinning
// (see ALIS_VM_REGVARS in alis.h). Must precede any include.
#define ALIS_VM_CORE
#include "alis.h"
#include "alis_private.h"
#include "audio.h"
#include "image.h"
#include "mem.h"
#include "screen.h"
#include "sys/sys.h"
#include "render3d.h"
#include "utils.h"
#include "video.h"
#ifdef ALIS_DSP_MIXER
#include "audio/dsp_mixer.h"
#if defined(__atarist__) || defined(__TOS__)
#include "sys/sys_atari_dma_sound.h"
#endif
#endif

sAlisVM alis;
u32 alis_arena_size = 0;
const char *alis_fatal = NULL;
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


// Virtual accumulator origin (grows down), below every game's basemem.
static const u32 kAccOrg = 0x19c00;

const u32 kVirtualRAMSize       = 0xffff * sizeof(u8);

// SPRITE_VAR touches at most basesprite (0x8000) + finsprit (0xFFFF) + 0x30 bytes; 128 KB covers it.
const u32 kSpriteMemSize        = 0x20000;


extern s32 testIndex;
extern u32 testData[];

// =============================================================================
// MARK: - Private
// =============================================================================

void readexec(const sAlisOpcode * table, char * name, u8 identation) {
#ifndef NDEBUG
    if (alis.script->pc < alis.script->pc_org || alis.script->pc >= alis.script->pc_org + alis.script->data->sz || alis.script->pc - alis.script->pc_org == kVirtualRAMSize)
    {
        // pc overflow !
        printf("\n");
        ALIS_DEBUG(EDebugFatal, disalis ? "ERROR: %s" : "%s", "PC OVERFLOW !\n");
        ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
        alis.state = eAlisStateStopped;
    }
    else
    {
        // fetch code
        u8 code = *(VMEM + VSCRIPT->pc++);
        sAlisOpcode opcode = table[code];
        
        if (!disalis)
        {
            ALIS_DEBUG(EDebugInfo, " %s", opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
        }
        else
        {
            u32 prg_offset = alis.script->pc;
            u32 file_offset = prg_offset + alis.script->data->header.code_loc_offset + 2 + 6; //  2 -> size of code_loc_offset, 6 -> kPackedHeaderSize = 6
            if (alis.script->name == alis.main->name)
            {
                file_offset += 16; // 16 -> kVMSpecsSize = 16
            }

            if (!strcmp(name, "opcode"))
            {
                ALIS_DEBUG(EDebugInfo, "\n%s [%.6x]%.6x: %.2x: %s ### ", alis.script->name, file_offset, prg_offset, opcode.name[0] == 0 ? code : opcode.code, opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, "\n      --> [%.6x]%.6x: %.2x: %s ### ", file_offset, prg_offset, opcode.name[0] == 0 ? code : opcode.code, opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
            }
        }
        
        if (opcode.fptr == 0)
        {
            // The opcode (new?) is missing in the name tables, VM behaviour will be inadequate.
            // It is necessary to add it to the appropriate name table
            // (codop, codesc1, codesc2, codesc3, oper, store, or add)
            printf("\n");
            ALIS_DEBUG(EDebugFatal, disalis ? "ERROR: Opcode 0x%.2x is missing in %ss table.\n" : "Opcode 0x%.2x is missing in %ss table.\n", code, name);
            if (!VM_IGNORE_ERRORS)
            {
                ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
                alis.state = eAlisStateStopped;
            }
        }
        else
        {
            opcode.fptr();
        }
    }
#else
    sAlisOpcode opcode = table[*(VMEM + VSCRIPT->pc++)];
    opcode.fptr();
#endif
}

#ifndef NDEBUG
// When NDEBUG is defined, these are replaced by inline macros in alis.h
void readexec_opcode(void) {
    if (!disalis) {
       ALIS_DEBUG(EDebugInfo, "\n%s [%.6x:%.4x]: 0x%06x:", alis.script->name, alis.script->vram_org, (u16)(alis.script->vacc_off), alis.script->pc);
    }
    readexec(opcodes, "opcode", 0);
}

void readexec_codesc1name(void) {
    readexec(codesc1names, "codesc1name", 1);
}

void readexec_codesc2name(void) {
    readexec(codesc2names, "codesc2name", 1);
}

void readexec_codesc3name(void) {
    readexec(codesc3names, "codesc3name", 1);
}

void readexec_opername(void) {
    readexec(opernames, "opername", 1);
}

void readexec_storename(void) {
    readexec(storenames, "storename", 2);
}

void readexec_addname(void) {
    readexec(addnames, "addname", 2);
}

void readexec_addname_swap(void) {
    char *tmp = alis.sd7;
    alis.sd7 = alis.oldsd7;
    alis.oldsd7 = tmp;

    readexec_addname();
}

// gparam
void readexec_opername_saveD7(void) {
    alis.varD6 = alis.varD7;
    readexec_opername();
}

// gparam1
void readexec_opername_saveD6(void) {
    s16 tmp = alis.varD7;
    readexec_opername_saveD7();
    alis.varD6 = alis.varD7;
    alis.varD7 = tmp;
}

void readexec_opername_swap(void) {
    char *tmp = alis.sd7;
    alis.sd7 = alis.sd6;
    alis.sd6 = tmp;

    readexec_opername();
}
#endif


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
        alis.specs.mem_cap = fread32(fp);
        alis.specs.max_allocatable_vram = fread32(fp);
        alis.specs.vram_to_data_offset = fread32(fp);
        alis.specs.vram_to_data_offset += 3;
        alis.specs.vram_to_data_offset *= 0x30; // 0x28; // multiply by sprite data size
        
        alis.vprotect = 0;
        
        // set the location of scripts' vrams table
        alis.atprog = alis.basemem;
        alis.atprog_ptr = (u32 *)(alis.mem + alis.atprog);
        alis.atent = alis.atprog + alis.specs.script_data_tab_len * 4; // 0xf0;
        alis.atent_ptr = (sScriptLoc *)(alis.vram_org + alis.specs.script_data_tab_len * 4);
        alis.maxent = alis.specs.script_vram_tab_len;
        alis.debent = alis.atent + alis.maxent * 6;
        alis.finent = alis.debent;

        image.debsprit = ((alis.debent + alis.specs.max_allocatable_vram) | 0xf) + 1;
        image.finsprit += image.debsprit + alis.specs.vram_to_data_offset;
        
        alis.debprog = image.finsprit;
        alis.finprog = alis.debprog;
        alis.dernprog = alis.atprog;
        alis.maxprog = alis.specs.script_data_tab_len;

        alis.finmem = alis.platform.ram_sz - 0x9168;
        if (alis.specs.mem_cap && (u32)alis.debprog + alis.specs.mem_cap < alis.finmem)
            alis.finmem = alis.debprog + alis.specs.mem_cap;   // as the original

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

        ALIS_DEBUG(EDebugInfo, "\
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
              alis.specs.mem_cap);
        fclose(fp);

        // load main scripts as an usual script...
        sAlisScriptData *script = script_load(alis.platform.main);
        alis.main = script_live(script);
        alis.basemain = alis.main->vram_org;

        if (alis.platform.version == 10) {
            // NOTE: v1.0 default direction table: last index, then 26 unit vectors
            static const s8 dirtab[] = {
                0x19,
                1,0,0, 1,1,0, 0,1,0, -1,1,0, -1,0,0, -1,-1,0, 0,-1,0, 1,-1,0, 1,0,1,
                1,1,1, 0,1,1, -1,1,1, -1,0,1, -1,-1,1, 0,-1,1, 1,-1,1, 0,0,1, 1,0,-1,
                1,1,-1, 0,1,-1, -1,1,-1, -1,0,-1, -1,-1,-1, 0,-1,-1, 1,-1,-1, 0,0,-1 };
            memcpy(alis.mem + alis.vstandard, dirtab, sizeof(dirtab));
        }

        alis.dernent = xswap16(alis.atent_ptr[0].offset);
        alis.atent_ptr[0].offset = 0;
    }
}

// =============================================================================
// MARK: - VM API
// =============================================================================
u8 alis_init(sPlatform platform) {
    
    ALIS_DEBUG(EDebugVerbose, "ALIS: Init.\n");

    alis.platform = platform;

    alis.cdspeed = 0x493e0;
    alis.pays = 0;         // pays: keymode=Q=>0; keymode=A=>1 (France); keymode=Z=>2 (Germany)
                           // cf. Z=QWERTZU=2, Q=QWERTY=1, A=AZERTY=0 -> keyboard_current
    vram_init();
    render3d_init();

    script_guess_game(platform.main);
    if (alis.platform.uid <= 0) return 1;

    // VM heap: the native pre-flight sizes it to the machine, else the game's preferred tier.
    u32 floor, pref;
    pl_arena_range(&alis.platform, &floor, &pref);
    alis.platform.ram_sz = alis_arena_size ? alis_arena_size : pref;
    alis.memclass = alis.platform.ram_sz < pref ? pl_model_memclass(alis.platform.ram_sz) : 0;
    printf("  VM heap: %u KB (game uid 0x%x, platform %d, v%u, bpp %u)\n",
           alis.platform.ram_sz / 1024,
           alis.platform.uid, alis.platform.kind,
           alis.platform.version, alis.platform.bpp);

    alis.timeclock = 0;
    
    audio.fsound = 1;
    audio.fmusic = 1;
    audio.musicId = 0xffff;

    memset(audio.tabinst, 0, sizeof(audio.tabinst));

    alis.restart_loop = 0;
    alis.automode = 0;

    // DOS platform config: bits encode detected hardware capabilities.
    // Bit 0: sound module 1, Bit 1: sound module 2, Bit 2: video,
    // Bit 6: 200KB+ RAM, Bit 7: 400KB+ RAM (replaces bit 6).
    // Scripts use oconfig / 0x40 to detect film replay capability.
    if (alis.platform.kind == EPlatformPC)
    {
        alis.theconfig = 0x87; // sound + video + high memory
    }
    
    dos_pal_reset();
    switch (alis.platform.kind) {
        case EPlatformPC:
            // CGA-art games; the VGA ones start black like the original DAC (first fade from black)
            if (alis.platform.version <= 11)
            {
                for (int i = 0; i < 4; i++)
                {
                    { sColorARGB *c = (sColorARGB *)&image.tpalet[i]; c->r = cga_palette[i * 4 + 0]; c->g = cga_palette[i * 4 + 1]; c->b = cga_palette[i * 4 + 2]; c->a = 0; }
                    { sColorARGB *c = (sColorARGB *)&image.mpalet[i]; c->r = cga_palette[i * 4 + 0]; c->g = cga_palette[i * 4 + 1]; c->b = cga_palette[i * 4 + 2]; c->a = 0; }
                }
                break;
            }
            // fall through

        default:
            memset(image.tpalet, 0, sizeof(image.tpalet));
            memset(image.mpalet, 0, sizeof(image.mpalet));
            break;
    }
    
    for (int i = 0; i < 768; i++)
        image.dkpalet[i] = 0x100;
    
    image.atpalet = image.tpalet;
    image.ampalet = image.mpalet;

    image.flinepal = 0;
    image.tlinepal[0] = 0;
    image.tlinepal[1] = 0;
    image.tlinepal[2] = 0xff;
    image.tlinepal[3] = 0;

    image.ptabfen = image.tabfen;
    image.fmouse = 0xff;
    image.fonum = 0xffff;
    image.loglarg = 0xa0; // 0x50 for st

    // VM heap, sized by pl_compute_ram_size. Can fail on a real Atari (main.c pre-flights it).
    alis.mem = malloc(sizeof(u8) * alis.platform.ram_sz);
    if (alis.mem == NULL) {
        printf("\nOut of memory: could not allocate the %u KB game heap.\n",
               alis.platform.ram_sz / 1024);
        return 1;
    }
    VM_SYNC_MEM();
    memset(alis.mem, 0, sizeof(u8) * alis.platform.ram_sz);
    _xmem = alis.mem;
    _xle  = alis.platform.is_little_endian;
    _xwc  = (alis.platform.version >= 30 && alis.platform.is_little_endian) ? 1 : 0;

    image.spritemem = (u8 *)malloc(kSpriteMemSize);
    if (image.spritemem == NULL) {
        printf("\nOut of memory: could not allocate the %u KB sprite pool.\n",
               (u32)kSpriteMemSize / 1024);
        return 1;
    }
    memset(image.spritemem, 0x0, kSpriteMemSize);

    image.buffer_alloc_size = alis.platform.width * (alis.platform.height + 2 * host.pixelbuf.surface_h);
    // Prefer platform framebuffers (native Atari: ST-RAM, VIDEL can't scan TT-RAM); NULL -> malloc.
    u8 *fb = sys_get_framebuffer(0);
    image.physic_alloc = fb ? fb : (u8 *)malloc(image.buffer_alloc_size);
    image.logic_alloc = image.physic_alloc;
    memset(image.physic_alloc, 0, image.buffer_alloc_size);

    if (alis.platform.dbl_buf) {
        u8 *fb = sys_get_framebuffer(1);
        image.logic_alloc = fb ? fb : (u8 *)malloc(image.buffer_alloc_size);
        memset(image.logic_alloc, 0, image.buffer_alloc_size);
    }

    image.physic = image.physic_alloc + host.pixelbuf.surface_h * alis.platform.width;
    image.logic  = image.logic_alloc  + host.pixelbuf.surface_h * alis.platform.width;

    image.logx1 = 0;
    image.logx2 = alis.platform.width - 1;
    image.logy1 = 0;
    image.logy2 = alis.platform.height - 1;

    // cswitching is never called by older games (see cswitching for the PC caveat).
    alis.fswitch = alis.platform.dbl_buf;
    alis.flagmain = 0;
    
    alis.fallent = 0;
    alis.fseq = 0;
    alis.fmuldes = 0;
    alis.fadddes = 0;
    alis.ferase = 0;

    alis.saversp = 0;
    alis.basevar = 0;
    alis.finmem = alis.basemem - 0x400; // 0x22000 + 0x3600;

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
    
    // NOTE: random address that should be empty
    alis.vstandard = 0x19d16; // 0x153C6;
    memset(alis.mem + alis.vstandard, 0, 256);

    // Work areas sit below every game's basemem (lowest 0x1f300): at 0x212ee/0x22880 they
    // overlapped the entity contexts of low-basemem games (Arborea: the expression stack
    // overwrote another entity's wait reload).
    alis.tabptr = 0x19c00; // tab containing 2 * 16 pointers
    memset(alis.mem + alis.tabptr, 0, 2 * 16 * 4);
    
    alis.sd7 = alis.bsd7;
    alis.sd6 = alis.bsd6;
    alis.oldsd7 = alis.bsd7bis;
    
    // init helpers
    alis.fp = NULL;

    // set the vram origin at some abitrary location (same as atari, to ease debug)
    alis.vram_org = alis.mem + alis.basemem;
    
    // the script data address table is located at vram start
    alis.script_data_orgs = (u32 *)alis.vram_org;
    
    alis.acc = alis.acc_org = (s16 *)(alis.mem + kAccOrg);
    
    alis.pretrlinetra = (s16 *)(alis.mem + 0x1ef7c);
    alis.pretglinetra = (s16 *)(alis.mem + 0x1f58e);

    alis.tlinetra  = (s16 *)(alis.mem + 0x1e602);
    alis.itlinetra = (s16 *)(alis.mem + 0x1e802);
    alis.trlinetra = (s16 *)(alis.mem + 0x1f07e);
    alis.tglinetra = (s16 *)(alis.mem + 0x1f690);
    
    alis.fmouse = 0xff;

    // init host system stuff
    host.pixelbuf.w = alis.platform.width;
    host.pixelbuf.h = alis.platform.height;
    host.pixelbuf.data = image.physic;
    host.pixelbuf.palette = image.mpalet;

    alis.load_delay = 0;
    alis.unload_delay = 0;
    
    // load main script
    alis_load_main();
    ALIS_SET_SCRIPT(alis.main);
    alis.basemain = alis.main->vram_org;
    
    alis.desmouse = NULL;

    // Robinsons Requiem
    
    image.fdoland = 0;
    image.scdirect = 0;
    image.tlpix = 0;
    image.atalti = 0;
    image.atlpix = 0;
    image.atalias = 0;
    image.tlland = 0;
    image.atlland = 0;
    image.landone = 0;
    image.purey = 0;
    image.purey2 = 0;
    image.fhorizon = 0;
    
    image.zoombid = 0x20de2;
    image.landdata = 0x1ccc2;

    return 0;
}

void alis_deinit(void) {
    // free scripts
    // TODO: use real script table / cunload
    free(alis.mem);
    alis.mem = NULL;
    VM_SYNC_MEM();

    free(image.spritemem);
    image.spritemem = NULL;

    // Free the surface-sized backing stores via the alloc base
    // pointers, not via image.physic/logic which are offset into the
    // alloc by surface_y_off. Single-buffered games (fswitch/dbl_buf 0)
    // share ONE allocation between logic and physic — free it only once.
    free(image.physic_alloc);
    if (image.logic_alloc != image.physic_alloc)
        free(image.logic_alloc);
    image.physic_alloc = NULL;
    image.logic_alloc = NULL;
    image.physic = NULL;
    image.logic = NULL;

    image.buffer_alloc_size = 0;
}

extern sMV1Audio mv1a;
extern sMV2Audio mv2a;

extern u16 fls_drawing;
extern u16 fls_pallines;
extern s8  fls_state;
extern u8 *endframe;

void alis_save_state(void)
{
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    strcat(path, "alis.state");

    FILE *fp = fopen(path, "wb");
    if (fp == NULL)
    {
        ALIS_DEBUG(EDebugError, "Failed to save state to: %s.\n", path);
        return;
    }
    
    u32 value;
    
    fwrite("ALIS", 5, 1, fp);
    fwrite(kSaveStateVersion, 5, 1, fp);

    // The size of alis differs on different versions of alis:
    // 1) if a new variable is added inside the structure
    // 2) if compiled with a different compiler (and the size of some variables is different)
    u32 alis_size = sizeof(alis);
    char alis_size_s[9] = {0};
    sprintf(alis_size_s, "%08x", alis_size);
    fwrite(alis_size_s, 8, 1, fp);

    fwrite(&(alis), sizeof(alis), 1, fp);
    size_t vram_size = sizeof(u8) * alis.platform.ram_sz;
    fwrite(&vram_size, sizeof(size_t), 1, fp);
    fwrite(alis.mem, vram_size, 1, fp);
    
    value = (u32)((s64)alis.script_data_orgs - (s64)alis.mem);
    fwrite(&value, 4, 1, fp);
    
    value = (u32)((s64)alis.ptrent - (s64)alis.tablent);
    fwrite(&value, 4, 1, fp);
    
    value = (u32)((s64)alis.acc - (s64)alis.mem);
    fwrite(&value, 4, 1, fp);

    value = (u32)((s64)alis.acc_org - (s64)alis.mem);
    fwrite(&value, 4, 1, fp);
    
    u32 loadedScrippts = 0;
    for (int i = 0; i < MAX_SCRIPTS; i++)
    {
        if (alis.loaded_scripts[i])
            loadedScrippts++;
    }

    fwrite(&loadedScrippts, 4, 1, fp);
    
    for (int i = 0; i < MAX_SCRIPTS; i++)
    {
        if (alis.loaded_scripts[i])
        {
            u16 idx = (u16)i;   // 2 bytes of an int are its HIGH half on big-endian hosts
            fwrite(&idx, 2, 1, fp);
            fwrite(alis.loaded_scripts[i], sizeof(sAlisScriptData), 1, fp);
        }
    }

    u32 mainIdx = 0;
    u32 scriptIdx = 0;
    u32 liveScrippts = 0;
    for (int i = 0; i < MAX_SCRIPTS; i++)
    {
        if (alis.live_scripts[i])
            liveScrippts++;
        
        if (alis.live_scripts[i] == alis.main)
            mainIdx = i;
        
        if (alis.live_scripts[i] == alis.script)
            scriptIdx = i;
    }

    fwrite(&liveScrippts, 4, 1, fp);
    
    for (int i = 0; i < MAX_SCRIPTS; i++)
    {
        if (alis.live_scripts[i])
        {
            u16 idx = (u16)i;   // 2 bytes of an int are its HIGH half on big-endian hosts
            fwrite(&idx, 2, 1, fp);
            fwrite(&(alis.live_scripts[i]->data->header.id), 2, 1, fp);
            fwrite(alis.live_scripts[i], sizeof(sAlisScriptLive), 1, fp);
        }
    }
    
    fwrite(&mainIdx, 4, 1, fp);
    fwrite(&scriptIdx, 4, 1, fp);
    
    // mouse
    
    value = alis.desmouse ? (u32)((s64)alis.desmouse - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);
    
    mouse_t mouse = sys_get_mouse();
    fwrite(&mouse.enabled, 1, 1, fp);

    // image

    sys_cursor_hold(1);
    fwrite(&(image), sizeof(image), 1, fp);
    fwrite(image.spritemem, kSpriteMemSize, 1, fp);
    fwrite(image.physic, alis.platform.width * alis.platform.height, 1, fp);
    // Single-buffered: logic == physic, so only one screen buffer exists. Load mirrors this.
    if (image.logic != image.physic)
        fwrite(image.logic, alis.platform.width * alis.platform.height, 1, fp);
    sys_cursor_hold(0);
    
    // audio
    
    u8 audio_type = 0;
    if (audio.soundrout == mv1_soundrout) {
        audio_type = 1;
    }
    else if (audio.soundrout == mv2_soundrout) {
        audio_type = 2;
    }
    else if (audio.soundrout == mv2_chiprout) {
        audio_type = 3;
    }
    else if (audio.soundrout == mv2_opl2rout) {
        audio_type = 4;
    }
    else if (audio.soundrout == mv0_soundrout) {
        audio_type = 5;
    }

    fwrite(&(audio), sizeof(audio), 1, fp);
    for (int i = 0; i < 4; i++)
    {
        value = (u32)((s64)audio.channels[i].address - (s64)alis.mem);
        fwrite(&value, 4, 1, fp);
    }

    fwrite(&audio_type, sizeof(audio_type), 1, fp);

    fwrite(&(mv1a), sizeof(mv1a), 1, fp);
    fwrite(&(mv2a), sizeof(mv2a), 1, fp);

    // OPL2 SFX channel state (non-Atari builds)
#if !defined(__TOS__) && !defined(__atarist__)
    extern u8 opl_sfx_initialized[2];
    extern u8 opl_sfx_type[2];
    fwrite(opl_sfx_initialized, sizeof(opl_sfx_initialized), 1, fp);
    fwrite(opl_sfx_type, sizeof(opl_sfx_type), 1, fp);
#else
    {
        u8 zeros[4] = {0};
        fwrite(zeros, 4, 1, fp);
    }
#endif
    
    // FLI/FLC video
    
    fwrite(&bfilm, sizeof(bfilm), 1, fp);
    value = bfilm.addr1  ? (u32)((s64)bfilm.addr1  - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);
    value = bfilm.addr2  ? (u32)((s64)bfilm.addr2  - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);
    value = bfilm.endptr ? (u32)((s64)bfilm.endptr - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);
    value = bfilm.delptr ? (u32)((s64)bfilm.delptr - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);
    
    fwrite(&fls_drawing, sizeof(fls_drawing), 1, fp);
    fwrite(&fls_pallines, sizeof(fls_pallines), 1, fp);
    fwrite(&fls_state, sizeof(fls_state), 1, fp);
    value = pvgalogic != NULL;   // film buffer only exists while a film is up
    fwrite(&value, 4, 1, fp);
    if (pvgalogic) {
        fwrite(pvgalogic, kVgaLogicSize, 1, fp);
        value = (u32)(vgalogic - pvgalogic);
        fwrite(&value, 4, 1, fp);
        value = (u32)(vgalogic_df - pvgalogic);
        fwrite(&value, 4, 1, fp);
    }

    value = endframe ? (u32)((s64)endframe - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);

    // screen list head (appended; older files end before it)
    fwrite(&screen.ptscreen, sizeof(screen.ptscreen), 1, fp);

    fclose(fp);

    printf("\n");
    ALIS_DEBUG(EDebugSystem, "Savestate saved to: %s.\n", path);
}

void alis_load_state(void)
{
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    strcat(path, "alis.state");

    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    {
        printf("\n");
        ALIS_DEBUG(EDebugError, "Savestate file %s is not found.\n", path);
        return;
    }
    
    char buffer[1024];
    fread(buffer, 5, 1, fp);
    if (strcmp("ALIS", buffer))
    {
        printf("\n");
        ALIS_DEBUG(EDebugError, "Unknown savestate format in %s.\n", path);
        fclose(fp);
        return;
    }

    fread(buffer, 5, 1, fp);
    int ver = atoi(buffer);
    int ver_current = atoi(kSaveStateVersion);
    if (ver != ver_current)
    {
        printf("\n");
        ALIS_DEBUG(EDebugError, "The savestate version %d in %s is not supported.\n", ver, path);
        fclose(fp);
        return;
    }

    fread(buffer, 8, 1, fp);
    buffer[8] = 0;
    u32 alis_size = (u32)strtol(buffer, NULL, 16);
    u32 current_alis_size = sizeof(alis);

    if (current_alis_size != alis_size) {
        printf("\n");
        ALIS_DEBUG(EDebugError, "The savestate is made by a different version of alis (platform, version or compiler).\n");
        fclose(fp);
        return;
       }

    char savepath[kPathMaxLen] = {0};
    strcpy(savepath, alis.platform.path);
    char savemain[kPathMaxLen] = {0};
    strcpy(savemain, alis.platform.main);

    sys_isr_pause(1);
    sys_cursor_hold(1);
    u8 *oldmem = alis.mem;
    size_t oldmem_size = alis.platform.ram_sz;
    memset(&alis, 0, alis_size);
    fread(&(alis), alis_size, 1, fp);

    strcpy(alis.platform.path, savepath);
    strcpy(alis.platform.main, savemain);
    
    size_t vram_size = 0;
    fread(&vram_size, sizeof(size_t), 1, fp);
    // Reuse the arena: a second copy may not fit (4 MB machines) and the old one leaked.
    if (oldmem && vram_size == oldmem_size)
        alis.mem = oldmem;
    else {
        free(oldmem);
        alis.mem = malloc(vram_size);
    }
    VM_SYNC_MEM();
    _xmem = alis.mem;
    _xle  = alis.platform.is_little_endian;
    _xwc  = (alis.platform.version >= 30 && alis.platform.is_little_endian) ? 1 : 0;
    fread(alis.mem, vram_size, 1, fp);
#if defined(ALIS_NATIVE_PLANAR)
    planar_tab_flush();   // new memory image: caches are keyed by old addresses
#endif
#if defined(ALIS_NATIVE_16BPP)
    sprite_cache_flush();
#endif
#if defined(ALIS_RRQ_ASM_ZOOM) && ALIS_RRQ_ASM_ZOOM && !defined(ALIS_NO_ZOOM_TRIM)
    zoom_rows_flush();
#endif
    
    alis.vram_org = alis.mem + alis.basemem;
    
    // sd7
    
    if (alis.bsd7 == alis.sd7)
    {
        alis.sd7 = alis.bsd7 = (char *)(alis.mem + 0x1a1e6);
    }
    else if (alis.bsd7 == alis.sd6)
    {
        alis.sd6 = alis.bsd7 = (char *)(alis.mem + 0x1a1e6);
    }
    else if (alis.bsd7 == alis.oldsd7)
    {
        alis.oldsd7 = alis.bsd7 = (char *)(alis.mem + 0x1a1e6);
    }

    // sd6
    
    if (alis.bsd6 == alis.sd7)
    {
        alis.sd7 = alis.bsd6 = (char *)(alis.mem + 0x1a2e6);
    }
    else if (alis.bsd6 == alis.sd6)
    {
        alis.sd6 = alis.bsd6 = (char *)(alis.mem + 0x1a2e6);
    }
    else if (alis.bsd6 == alis.oldsd7)
    {
        alis.oldsd7 = alis.bsd6 = (char *)(alis.mem + 0x1a2e6);
    }

    // oldsd7
    
    if (alis.bsd7bis == alis.sd7)
    {
        alis.sd7 = alis.bsd7bis = (char *)(alis.mem + 0x1a3e6);
    }
    else if (alis.bsd7bis == alis.sd6)
    {
        alis.sd6 = alis.bsd6 = (char *)(alis.mem + 0x1a3e6);
    }
    else if (alis.bsd7bis == alis.oldsd7)
    {
        alis.oldsd7 = alis.bsd7bis = (char *)(alis.mem + 0x1a3e6);
    }
    
    alis.atprog_ptr = (u32 *)(alis.mem + alis.atprog);
    alis.atent_ptr = (sScriptLoc *)(alis.vram_org + alis.specs.script_data_tab_len * 4);

    u32 value;

    fread(&value, 4, 1, fp);
    alis.script_data_orgs = (u32 *)(alis.mem + value);

    fread(&value, 4, 1, fp);
    alis.ptrent = (s16 *)((s64)alis.tablent + value);

    fread(&value, 4, 1, fp);
    alis.acc = (s16 *)(alis.mem + value);

    fread(&value, 4, 1, fp);
    alis.acc = alis.acc_org = (s16 *)(alis.mem + kAccOrg);   // older saves stored 0x22880
    
    u16 idx = 0;
    u16 id = 0;

    memset(alis.loaded_scripts, 0, sizeof(alis.loaded_scripts));

    fread(&value, 4, 1, fp);
    for (int i = 0; i < value; i++)
    {
        fread(&idx, 2, 1, fp);
        alis.loaded_scripts[idx] = malloc(sizeof(sAlisScriptData));
        fread(alis.loaded_scripts[idx], sizeof(sAlisScriptData), 1, fp);
    }
    
    memset(alis.live_scripts, 0, sizeof(alis.live_scripts));

    fread(&value, 4, 1, fp);
    for (int i = 0; i < value; i++)
    {
        fread(&idx, 2, 1, fp);
        fread(&id, 2, 1, fp);
        alis.live_scripts[idx] = malloc(sizeof(sAlisScriptLive));
        fread(alis.live_scripts[idx], sizeof(sAlisScriptLive), 1, fp);
        
        for (int d = 0; d < MAX_SCRIPTS; d++)
        {
            if (alis.loaded_scripts[d] && alis.loaded_scripts[d]->header.id == id)
            {
                alis.live_scripts[idx]->name = alis.loaded_scripts[d]->name;
                alis.live_scripts[idx]->data = alis.loaded_scripts[d];
                break;
            }
        }
    }

    fread(&value, 4, 1, fp);
    alis.main = alis.live_scripts[value];

    fread(&value, 4, 1, fp);
    ALIS_SET_SCRIPT(alis.live_scripts[value]);
    
    // mouse
    
    fread(&value, 4, 1, fp);
    alis.desmouse = value ? alis.mem + value : NULL;
    
    u8 enable_mouse = 0;
    fread(&enable_mouse, 1, 1, fp);

    // image: runtime-allocated pointers (not into alis.mem) must survive the fread.

    u8 *spritemem    = image.spritemem;
    u8 *physic       = image.physic;
    u8 *logic        = image.logic;
    u8 *physic_alloc = image.physic_alloc;
    u8 *logic_alloc  = image.logic_alloc;
    u8 *backmap      = image.backmap;
    u32 *atpalet     = image.atpalet;
    u32 *ampalet     = image.ampalet;
    s16 *ptabfen     = image.ptabfen;
    u8 *bufpack      = image.bufpack;
    u8 *wlogic       = image.wlogic;
    u8 *bgcache_alloc = image.bgcache_alloc;   // host malloc (background cache) — not in alis.mem
    u8 *wdraw         = image.wdraw;            // current draw target (== logic or backmap)
#if ALIS_SDL_VER > 1
    u8  *depthbuf     = image.depthbuf;
    u8 **depthrows    = image.depthrows;
    u8  *omask        = image.omask;
    u32 *terrgba      = image.terrgba;
    u32 **terrgbarows = image.terrgbarows;
#endif

    memset(&image, 0, sizeof(image));
    fread(&(image), sizeof(image), 1, fp);

    image.spritemem    = spritemem;
    image.physic       = physic;
    image.logic        = logic;
    image.physic_alloc = physic_alloc;
    image.logic_alloc  = logic_alloc;
    image.backmap      = backmap;
    image.atpalet      = atpalet;
    image.ampalet      = ampalet;
    image.ptabfen      = ptabfen;
    image.bufpack      = bufpack;
    image.wlogic       = wlogic;
    image.bgcache_alloc = bgcache_alloc;
    image.wdraw         = wdraw;
#if ALIS_SDL_VER > 1
    image.depthbuf    = depthbuf;
    image.depthrows   = depthrows;
    image.omask       = omask;
    image.terrgba     = terrgba;
    image.terrgbarows = terrgbarows;
#endif
    
    fread(image.spritemem, kSpriteMemSize, 1, fp);
    fread(image.physic, alis.platform.width * alis.platform.height, 1, fp);
    // Single-buffered: logic == physic and only one buffer was saved (mirror alis_save_state).
    if (image.logic != image.physic)
        fread(image.logic, alis.platform.width * alis.platform.height, 1, fp);

    // audio
    
    u8 audio_type = 0;
    fread(&(audio), sizeof(audio), 1, fp);
    for (int i = 0; i < 4; i++)
    {
        fread(&value, 4, 1, fp);
        audio.channels[i].address = (s8 *)alis.mem + value;
    }

    fread(&audio_type, sizeof(audio_type), 1, fp);
    switch (audio_type)
    {
        case 1:  audio.soundrout = mv1_soundrout;  break;
        case 2:  audio.soundrout = mv2_soundrout;  break;
        case 3:  audio.soundrout = mv2_chiprout;   break;
        case 4:  audio.soundrout = mv2_opl2rout;   break;
        case 5:  audio.soundrout = mv0_soundrout; mv0_reset(); break;
        default: audio.soundrout = NULL;           break;
    }

    fread(&(mv1a), sizeof(mv1a), 1, fp);
    fread(&(mv2a), sizeof(mv2a), 1, fp);

    // OPL2 SFX channel state (non-Atari builds)
#if !defined(__TOS__) && !defined(__atarist__)
    extern u8 opl_sfx_initialized[2];
    extern u8 opl_sfx_type[2];
    fread(opl_sfx_initialized, sizeof(opl_sfx_initialized), 1, fp);
    fread(opl_sfx_type, sizeof(opl_sfx_type), 1, fp);
#else
    {
        u8 zeros[4];
        fread(zeros, 4, 1, fp);
    }
#endif
    
    // FLI/FLC video
    
    fread(&bfilm, sizeof(bfilm), 1, fp);
    fread(&value, 4, 1, fp);
    bfilm.addr1  = value ? alis.mem + value : NULL;
    fread(&value, 4, 1, fp);
    bfilm.addr2  = value ? alis.mem + value : NULL;
    fread(&value, 4, 1, fp);
    bfilm.endptr = value ? alis.mem + value : NULL;
    fread(&value, 4, 1, fp);
    bfilm.delptr = value ? alis.mem + value : NULL;

    // Streaming-film state (file handle, window buffer) is not restorable: clear the saved pointers.
    bfilm.sfp = NULL;
    bfilm.sbuf = NULL;
    bfilm.sfill = NULL;
    bfilm.sbuf_size = 0;
    bfilm.sremain = 0;
    bfilm.sfile_size = 0;

    fread(&fls_drawing, sizeof(fls_drawing), 1, fp);
    fread(&fls_pallines, sizeof(fls_pallines), 1, fp);
    fread(&fls_state, sizeof(fls_state), 1, fp);
    fread(&value, 4, 1, fp);
    if (value && vgalogic_alloc()) {
        fread(pvgalogic, kVgaLogicSize, 1, fp);
        fread(&value, 4, 1, fp);
        vgalogic = pvgalogic + value;
        fread(&value, 4, 1, fp);
        vgalogic_df = pvgalogic + value;
    } else if (!value) {
        vgalogic_free();
    }
    fread(&value, 4, 1, fp);
    endframe = value ? alis.mem + value : NULL;
    u16 ptscreen;
    if (fread(&ptscreen, sizeof(ptscreen), 1, fp) == 1)
        screen.ptscreen = ptscreen;
    
    fclose(fp);

    host.pixelbuf.palette = image.mpalet;
    dos_pal_sync_from_image();
#if ALIS_SDL_VER < 2
    {
        extern volatile u8 dirty_pal;
        dirty_pal = 1;   // push the restored palette (native/SDL1 only reload it when dirty)
    }
#endif

    sys_init_timers();

    sys_enable_mouse(enable_mouse);
    set_update_cursor();
    
    // requiem

    alis.pretrlinetra = (s16 *)(alis.mem + 0x1ef7c);
    alis.pretglinetra = (s16 *)(alis.mem + 0x1f58e);

    alis.tlinetra  = (s16 *)(alis.mem + 0x1e602);
    alis.itlinetra = (s16 *)(alis.mem + 0x1e802);
    alis.trlinetra = (s16 *)(alis.mem + 0x1f07e);
    alis.tglinetra = (s16 *)(alis.mem + 0x1f690);

    sys_cursor_hold(0);
    sys_isr_pause(0);

    printf("\n");
    ALIS_DEBUG(EDebugSystem, "Savestate loaded from: %s.\n", path);
}

// -----------------------------------------------------------------------------
// VM-vs-render frame profiler (-DALIS_VM_PROFILE=1): ticks in alis_loop() vs draw(),
// accumulated over a window of frames and dumped as a ratio.
// -----------------------------------------------------------------------------
#ifndef ALIS_VM_PROFILE
# define ALIS_VM_PROFILE 0
#endif
#if ALIS_VM_PROFILE
#include <stdio.h>
// Accumulators are extern-visible: draw() (image.c) times only the render work,
// AFTER sys_delay_frame()'s frame-cap sleep, so the sleep is NOT counted.
u32 g_prof_vm = 0;               // accumulated ticks in alis_loop (VM dispatch)
u32 g_prof_draw = 0;             // accumulated ticks in draw() render work
u32 g_prof_frames = 0;
static FILE *g_prof_file = NULL; // own log file, independent of dbglog/backend
static u32 g_op_count[256];      // per-opcode execution count (this window)
static u32 g_op_total = 0;       // total opcodes executed (this window)
static u32 g_op_time[256];       // per-opcode accumulated TICKS (finds heavy opcodes)
u32 g_oper_count[256];           // per-OPERAND (opername) execution count
u32 g_oper_total = 0;            // total operands evaluated (this window)
u32 g_cload_sleep = 0;           // cload: ticks in artificial sys_sleep_interactive
u32 g_cload_work = 0;            // cload: ticks in real file load + unpack
u32 g_cload_calls = 0;           // cload: number of calls (this window)
u32 g_prof_c2p = 0;              // ticks in trsfen (chunky->physical transfer / c2p)
// Called once per frame from draw() after rendering; dumps every ALIS_VM_PROFILE_WINDOW frames.
void prof_frame_end(void) {
    // Window size in frames: -DALIS_VM_PROFILE_WINDOW=n.
#ifndef ALIS_VM_PROFILE_WINDOW
#define ALIS_VM_PROFILE_WINDOW 512
#endif
    if (++g_prof_frames < ALIS_VM_PROFILE_WINDOW)
        return;
    if (!g_prof_file) {
        // fallbacks: hard drive, then current dir, then floppy
        g_prof_file = fopen("C:\\vmprof.log", "w");
        if (!g_prof_file) g_prof_file = fopen("vmprof.log", "w");
        if (!g_prof_file) g_prof_file = fopen("A:\\vmprof.log", "w");
    }
    if (g_prof_file) {
        u32 tot = g_prof_vm + g_prof_draw; if (!tot) tot = 1;
        fprintf(g_prof_file, "VMPROF %u frames: vm=%u draw=%u  -> VM=%u%% DRAW=%u%%  ops=%u\n",
                g_prof_frames, g_prof_vm, g_prof_draw,
                (u32)((g_prof_vm * 100ULL) / tot), (u32)((g_prof_draw * 100ULL) / tot),
                g_op_total);
        // top 12 opcodes by execution count this window (hex code -> map via opcodes.c)
        fprintf(g_prof_file, "  OPHIST:");
        u32 ot = g_op_total ? g_op_total : 1;
        for (int n = 0; n < 12; n++) {
            int best = -1; u32 bestc = 0;
            for (int i = 0; i < 256; i++)
                if (g_op_count[i] > bestc) { bestc = g_op_count[i]; best = i; }
            if (best < 0) break;
            fprintf(g_prof_file, " %02x=%u(%u%%)", best, bestc, (u32)((bestc * 100ULL) / ot));
            g_op_count[best] = 0; // remove so next pass finds the next-highest
        }
        fprintf(g_prof_file, "\n");
        // top 12 operands (opernames) by execution count this window
        fprintf(g_prof_file, "  OPERHIST:");
        u32 et = g_oper_total ? g_oper_total : 1;
        for (int n = 0; n < 12; n++) {
            int best = -1; u32 bestc = 0;
            for (int i = 0; i < 256; i++)
                if (g_oper_count[i] > bestc) { bestc = g_oper_count[i]; best = i; }
            if (best < 0) break;
            fprintf(g_prof_file, " %02x=%u(%u%%)", best, bestc, (u32)((bestc * 100ULL) / et));
            g_oper_count[best] = 0;
        }
        fprintf(g_prof_file, " (opers=%u)\n", g_oper_total);
        // top 12 opcodes by accumulated TIME (the actual cost — finds heavy opcodes)
        fprintf(g_prof_file, "  OPTIME:");
        u32 vt = g_prof_vm ? g_prof_vm : 1;
        for (int n = 0; n < 12; n++) {
            int best = -1; u32 bestt = 0;
            for (int i = 0; i < 256; i++)
                if (g_op_time[i] > bestt) { bestt = g_op_time[i]; best = i; }
            if (best < 0) break;
            fprintf(g_prof_file, " %02x=%u(%u%%)", best, bestt, (u32)((bestt * 100ULL) / vt));
            g_op_time[best] = 0;
        }
        fprintf(g_prof_file, "\n");
        fprintf(g_prof_file, "  CLOAD: calls=%u sleep=%u work=%u  |  c2p=%u (%u%% of draw)\n",
                g_cload_calls, g_cload_sleep, g_cload_work,
                g_prof_c2p, (u32)((g_prof_c2p * 100ULL) / (g_prof_draw ? g_prof_draw : 1)));
        fflush(g_prof_file);
    }
    g_cload_sleep = g_cload_work = g_cload_calls = 0;
    g_prof_c2p = 0;
    for (int i = 0; i < 256; i++) { g_op_count[i] = 0; g_oper_count[i] = 0; g_op_time[i] = 0; }
    g_op_total = 0;
    g_oper_total = 0;
    g_prof_vm = g_prof_draw = g_prof_frames = 0;
}
# define PROF_VM_T0()   u32 _pvm0 = sys_profile_ticks()
# define PROF_VM_ADD()  (g_prof_vm += sys_profile_ticks() - _pvm0)
# define PROF_OP(op)    (g_op_count[(op)]++, g_op_total++)
#else
# define PROF_VM_T0()   ((void)0)
# define PROF_VM_ADD()  ((void)0)
# define PROF_OP(op)    ((void)0)
#endif

void alis_loop(void) {

    PROF_VM_T0();
    alis.script->running = 1;
    while (alis.state && alis.script->running) {

#ifndef NDEBUG
        u32 pc_before = alis.script->pc;
#endif
#if ALIS_VM_PROFILE
        {
            u8 _op = VMEM[VSCRIPT->pc];   // peek opcode before readexec consumes it
            g_op_count[_op]++; g_op_total++;
            u32 _t0 = sys_profile_ticks();
            readexec_opcode();
            g_op_time[_op] += sys_profile_ticks() - _t0;  // full opcode time -> this opcode
        }
#else
        readexec_opcode();
#endif
#ifndef NDEBUG
        if (alis.script->running && alis.state && (alis.script->pc < alis.script->pc_org || alis.script->pc >= alis.script->pc_org + alis.script->data->sz))
        {
            printf("\n*** PC CORRUPTION DETECTED ***\n");
            printf("  Script: %s\n", alis.script->name);
            printf("  PC before opcode: 0x%06x\n", pc_before);
            printf("  PC after opcode:  0x%06x\n", alis.script->pc);
            printf("  PC range: 0x%06x - 0x%06x\n", alis.script->pc_org, alis.script->pc_org + alis.script->data->sz);
            printf("  Opcode byte was: 0x%02x\n", *(alis.mem + pc_before));
            printf("  vram_org: 0x%06x, vacc_off: 0x%04x\n", alis.script->vram_org, (u16)alis.script->vacc_off);
            printf("  Bytes at pc_before: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                   alis.mem[pc_before], alis.mem[pc_before+1], alis.mem[pc_before+2], alis.mem[pc_before+3],
                   alis.mem[pc_before+4], alis.mem[pc_before+5], alis.mem[pc_before+6], alis.mem[pc_before+7]);
        }
#endif
    }

    PROF_VM_ADD();
    // alis loop was stopped by 'cexit', 'cstop', or user event
}

void savecoord(u32 addr)
{
    image.oldcx = xread16(addr + ALIS_SCR_WCX);
    image.oldcy = xread16(addr + ALIS_SCR_WCY);
    image.oldcz = xread16(addr + ALIS_SCR_WCZ);

    // ishar 3

    if (alis.platform.version >= 30) {

        image.oldacx = xread16(addr + ALIS_SCR_WCAX);
        image.oldacy = xread16(addr + ALIS_SCR_WCAY);
        image.oldacz = xread16(addr + ALIS_SCR_WCAZ);
    }
}

void updtcoord(u32 addr)
{
    if (alis.platform.version < 30) {
        // ishar 1&2
        s16 addx = xread16(addr + 0) - image.oldcx;
        s16 addy = xread16(addr + 2) - image.oldcy;
        s16 addz = xread16(addr + 4) - image.oldcz;
        if (addz != 0 || addx != 0 || addy != 0)
        {
            if (alis.platform.version == 10)
                scalaire_v10(get_0x16_screen_id(alis.script->vram_org), &addx, &addy, &addz);

            for (sSprite *sprite = SPRITE_VAR(get_0x18_unknown(alis.script->vram_org)); sprite != NULL; sprite = SPRITE_VAR(sprite->to_next))
            {
                if (sprite->state == 0)
                    sprite->state = 2;
                
                sprite->depx += addx;
                sprite->depy += addy;
                sprite->depz += addz;
            }
        }
    }
    else {
        // ishar 3
        s16 angle = xread16(addr + ALIS_SCR_WCAX);
        if (angle != image.oldacx)
        {
            if ((0x168 < angle) && 0x168 < (angle -= 0x168))
                angle %= 0x168;

            if (angle < -0x168 && (angle += 0x168) < -0x168)
                angle %= 0x168;

            xwrite16(addr + ALIS_SCR_WCAX, angle);
        }

        angle = xread16(addr + ALIS_SCR_WCAY);
        if (angle != image.oldacy)
        {
            if ((0x168 < angle) && 0x168 < (angle -= 0x168))
                angle %= 0x168;

            if (angle < -0x168 && (angle += 0x168) < -0x168)
                angle %= 0x168;

            xwrite16(addr + ALIS_SCR_WCAY, angle);
        }

        angle = xread16(addr + ALIS_SCR_WCAZ);
        if (angle != image.oldacz)
        {
            if ((0x168 < angle) && 0x168 < (angle -= 0x168))
                angle %= 0x168;

            if (angle < -0x168 && (angle += 0x168) < -0x168)
                angle %= 0x168;

            xwrite16(addr + ALIS_SCR_WCAZ, angle);
        }

        s16 addx = xread16(addr + ALIS_SCR_WCX) - image.oldcx;
        s16 addy = xread16(addr + ALIS_SCR_WCY) - image.oldcy;
        s16 addz = xread16(addr + ALIS_SCR_WCZ) - image.oldcz;
        u16 wcaz = xread16(addr + ALIS_SCR_WCAZ);

        if (angle != image.oldacz || addz != 0 || addx != 0 || addy != 0)
        {
            for (sSprite *sprite = SPRITE_VAR(get_0x18_unknown(alis.script->vram_org)); sprite != NULL; sprite = SPRITE_VAR(sprite->to_next))
            {
                if (sprite->state == 0)
                    sprite->state = 2;

                sprite->depx += addx;
                sprite->depy += addy;
                sprite->depz += addz;
                // Angle stored at memory offset 0x26 within u32 at 0x24:
                // on LE that's the HIGH word, on BE it's the LOW word
                if (alis.platform.is_little_endian)
                    sprite->sprite_0x28 = (sprite->sprite_0x28 & 0xFFFF) | ((u32)wcaz << 16);
                else
                    sprite->sprite_0x28 = (sprite->sprite_0x28 & 0xFFFF0000) | (wcaz & 0xFFFF);
            }
        }
    }
}

#if defined(ALIS_DEBUG_AUTOLOAD)
// Test runs: load alis.state once, ALIS_DEBUG_AUTOLOAD ms after the main loop starts (1 = 2 s).
// Too early (first VM steps) leaves the engine half set up; slow machines need a longer delay.
static void autoload_state(void)
{
    static u32 t0;
    static u8 done;
    u32 delay = ALIS_DEBUG_AUTOLOAD > 1 ? ALIS_DEBUG_AUTOLOAD : 2000;
    if (!t0) t0 = sys_ticks() | 1;
    if (!done) {
        if (sys_ticks() - t0 > delay) { done = 1; t0 = sys_ticks() | 1; alis.state = eAlisStateLoad; }
        return;
    }
#if defined(ALIS_DEBUG_AUTOSAVE)
    // ...and save it back ALIS_DEBUG_AUTOSAVE ms later (refreshes old savestates).
    if (done == 1 && sys_ticks() - t0 > ALIS_DEBUG_AUTOSAVE) { done = 2; alis.state = eAlisStateSave; }
#endif
}
#endif

// Native-only VM startup tracing via dbglog (sys_atari.c), capped; no-op elsewhere.
#if defined(ALIS_USE_NATIVE_ATARI)
extern void dbglog(const char *fmt, ...);
static int g_natlog_n = 0;
# define NATLOG(...)  do { if (g_natlog_n < 60) { g_natlog_n++; dbglog(__VA_ARGS__); } } while(0)
#else
# define NATLOG(...)  ((void)0)
#endif

void alis_main_V2(void) {
    NATLOG("V2 enter state=%d varD5=%d\n", alis.state, alis.varD5);
    while (alis.state) {
        NATLOG("V2 loop top varD5=%d\n", alis.varD5);
        
        if (alis.state == eAlisStateSave)
        {
            alis_save_state();
            alis.state = eAlisStateRunning;
        }
        else if (alis.state == eAlisStateLoad)
        {
            alis_load_state();
            alis.state = eAlisStateRunning;
        }
        
        alis.restart_loop = 0;
        
        ALIS_SET_SCRIPT(ENTSCR(alis.varD5));
        
        alis.fallent = 0;
        alis.fseq = 0;
        alis.acc = alis.acc_org;
        u8 killed = 0;

        if (get_0x24_scan_inter(alis.script->vram_org) < 0 && (get_0x24_scan_inter(alis.script->vram_org) & 2) == 0)
        {
            s32 script_offset = swap32((alis.mem + get_0x14_script_org_offset(alis.script->vram_org) + 10));
            if (script_offset != 0)
            {
                alis.script->vacc_off = alis.saversp = get_0x0a_vacc_offset(alis.script->vram_org);
                savecoord(alis.script->vram_org);
                alis.script->pc = get_0x14_script_org_offset(alis.script->vram_org) + 10 + script_offset;
                alis_loop();
                killed = alis.restart_loop && alis.platform.version == 10;
                if (!killed)
                    updtcoord(alis.script->vram_org);
            }
        }

        if (!killed && get_0x04_cstart_csleep(alis.script->vram_org) == 0)
        {
            ALIS_DEBUG(EDebugInfo, "\n SLEEPING %s", alis.script->name);
        }

        if (!killed && get_0x04_cstart_csleep(alis.script->vram_org) != 0)
        {
            if ((s8)get_0x04_cstart_csleep(alis.script->vram_org) < 0)
            {
                set_0x04_cstart_csleep(alis.script->vram_org, 1);
            }

            set_0x01_wait_count(alis.script->vram_org, get_0x01_wait_count(alis.script->vram_org) - 1);
            ALIS_DEBUG(EDebugInfo, "\n %s %s [%.2x, %.2x] ", get_0x01_wait_count(alis.script->vram_org) == 0 ? "RUNNING" : "WAITING", alis.script->name, get_0x01_wait_count(alis.script->vram_org), get_0x02_wait_cycles(alis.script->vram_org));
            if (get_0x01_wait_count(alis.script->vram_org) == 0)
            {
                savecoord(alis.script->vram_org);
                
                alis.script->pc = get_0x08_script_ret_offset(alis.script->vram_org);
                alis.script->vacc_off = get_0x0a_vacc_offset(alis.script->vram_org);
                alis.fseq++;
                NATLOG("pre alis_loop pc=%x\n", alis.script->pc);
                alis_loop();
                NATLOG("post alis_loop pc=%x\n", alis.script->pc);

                sys_delay_loop();
                
                if (alis.restart_loop == 0)
                {
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
                    
                    if (alis.restart_loop == 0)
                    {
                        updtcoord(alis.script->vram_org);
                        
                        set_0x01_wait_count(alis.script->vram_org, get_0x02_wait_cycles(alis.script->vram_org));
                    }
                }
            }
        }
        
        alis.varD5 = xread16(alis.atent + 4 + alis.varD5);
        if (alis.varD5 == 0)
        {
            ALIS_SET_SCRIPT(ENTSCR(alis.varD5));
            NATLOG("frame boundary: pre draw()\n");
            draw();
            NATLOG("post draw()\n");
#ifdef ALIS_DSP_MIXER
            dsp_mixer_poll();
#if defined(__atarist__) || defined(__TOS__)
            atari_dma_sound_poll();   // STE/TT: top up the DMA ring buffer
#endif
#endif
#if ALIS_USE_THREADS <= 0
            sys_poll_event();
#endif
        }
    }
}

void alis_main_V3(void) {
    NATLOG("V3 enter state=%d varD5=%d\n", alis.state, alis.varD5);
    while (alis.state) {
        NATLOG("V3 loop top varD5=%d\n", alis.varD5);
#if defined(ALIS_DEBUG_AUTOLOAD)
        autoload_state();
#endif

        if (alis.state == eAlisStateSave)
        {
            alis_save_state();
            alis.state = eAlisStateRunning;
        }
        else if (alis.state == eAlisStateLoad)
        {
            alis_load_state();
            alis.state = eAlisStateRunning;
        }

        alis.restart_loop = 0;

        ALIS_SET_SCRIPT(ENTSCR(alis.varD5));
        NATLOG("V3 script=%s vram=%x pc=%x\n", alis.script->name, alis.script->vram_org, alis.script->pc);

        alis.fallent = 0;
        alis.fseq = 0;
        alis.acc = alis.acc_org;

        if (get_0x24_scan_inter(alis.script->vram_org) < 0 && (get_0x24_scan_inter(alis.script->vram_org) & 2) == 0)
        {
            s32 script_offset = xread32(get_0x14_script_org_offset(alis.script->vram_org) + 10);
            if (script_offset != 0)
            {
                alis.script->vacc_off = alis.saversp = get_0x0a_vacc_offset(alis.script->vram_org);
                savecoord(alis.script->vram_org);
                alis.script->pc = get_0x14_script_org_offset(alis.script->vram_org) + 10 + script_offset;
                alis_loop();
                updtcoord(alis.script->vram_org);
            }
        }
        
        if (get_0x04_cstart_csleep(alis.script->vram_org) == 0)
        {
            ALIS_DEBUG(EDebugInfo, "\n SLEEPING %s", alis.script->name);
        }

        if (get_0x04_cstart_csleep(alis.script->vram_org) != 0)
        {
            if ((s8)get_0x04_cstart_csleep(alis.script->vram_org) < 0)
            {
                set_0x04_cstart_csleep(alis.script->vram_org, 1);
            }
            
            if (get_0x3e_wait_time(alis.script->vram_org) <= alis.timeclock) {
                set_0x3e_wait_time(alis.script->vram_org, get_0x3a_wait_cycles(alis.script->vram_org) + alis.timeclock);
                savecoord(alis.script->vram_org);
                
                alis.script->pc = get_0x08_script_ret_offset(alis.script->vram_org);
                alis.script->vacc_off = get_0x0a_vacc_offset(alis.script->vram_org);
                alis.fseq++;
                NATLOG("pre alis_loop pc=%x\n", alis.script->pc);
                alis_loop();
                NATLOG("post alis_loop pc=%x\n", alis.script->pc);

                if (alis.restart_loop == 0)
                {
                    set_0x0a_vacc_offset(alis.script->vram_org, alis.script->vacc_off);
                    set_0x08_script_ret_offset(alis.script->vram_org, alis.script->pc);
                    
                    s32 script_offset = xread32(get_0x14_script_org_offset(alis.script->vram_org) + 6);
                    if (script_offset != 0)
                    {
                        alis.fseq = 0;
                        alis.script->vacc_off = alis.saversp = get_0x0a_vacc_offset(alis.script->vram_org);
                        alis.script->pc = get_0x14_script_org_offset(alis.script->vram_org) + 6 + script_offset;
                        alis_loop();
                    }
                    
                    if (alis.restart_loop == 0)
                    {
                        updtcoord(alis.script->vram_org);
                    }
                }
            }
        }
        
        alis.varD5 = xread16(alis.atent + 4 + alis.varD5);
        if (alis.varD5 == 0)
        {
            ALIS_SET_SCRIPT(ENTSCR(alis.varD5));
            NATLOG("frame boundary: pre draw()\n");
            draw();
            NATLOG("post draw()\n");
#ifdef ALIS_DSP_MIXER
            dsp_mixer_poll();
#if defined(__atarist__) || defined(__TOS__)
            atari_dma_sound_poll();   // STE/TT: top up the DMA ring buffer
#endif
#endif
#if ALIS_USE_THREADS <= 0
            sys_poll_event();
#endif
        }
    }
}

int alis_thread(void *data) {
    alis.cstopret = 0;
    alis.varD5 = 0;

    NATLOG("alis_thread: version=%d main=%c\n", alis.platform.version,
           alis.platform.version < 30 ? '2' : '3');

    if (alis.platform.version < 30)
    {
        alis_main_V2();
    }
    else
    {
        alis_main_V3();
    }

    NATLOG("alis_thread: returned\n");
    return 0;
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
    
    ALIS_VDEBUG(EDebugError, err.descfmt, args);
    va_end(args);
    exit(-1);
}

// =============================================================================
#pragma mark - Finding resources in the script
// =============================================================================

s32 adresdes(s32 idx)
{
    u32 addr = get_0x14_script_org_offset(alis.flagmain ? alis.main->vram_org : alis.script->vram_org);
    if (alis.platform.version == 10)
    {
        if (alis.platform.kind == EPlatformPC)
        {
            addr += (u16)xread16(addr + 0xe);
            addr += (u16)xread16(addr);
        }
        else
        {
            addr += (u32)xread32(addr + 0xe);
            addr += (u32)xread32(addr);
        }
        
        return addr + (s16)xread16(addr + idx * 2);
    }

    addr += xread32(addr + 0xe);
    
    s32 len = xread16(addr + 4);
    if (len > idx)
        return addr + xread32(addr) + idx * 4;

    ALIS_DEBUG(EDebugFatal, "ERROR: Failed to read graphic resource at index %d (0x%06x >= length 0x%06x) from script %s\n", idx, idx, len, alis.flagmain ? alis.main->name : alis.script->name);
    return 0xf;
}

s32 adresform(s16 idx)
{
    u32 addr = get_0x14_script_org_offset(alis.script->vram_org);
    if (alis.platform.version == 10)
    {
        if (alis.platform.kind == EPlatformPC)
        {
            addr += (u16)xread16(addr + 0xe);
            addr += (u16)xread16(addr + 6);
        }
        else
        {
            addr += (u32)xread32(addr + 0xe);
            addr += (u32)xread32(addr + 6);
        }
        
        return addr + (s16)xread16(addr + idx * 2);
    }
    
    addr += xread32(addr + 0xe);
    
    s32 len = xread16(addr + 0xa);
    if (len > idx)
    {
        addr += xread32(addr + 0x6);
        return addr + xread16(addr + (idx * 2));
    }
    
    ALIS_DEBUG(EDebugFatal, "ERROR: Failed to get form %s\n", idx, alis.flagmain ? alis.main->name : alis.script->name);
    return 0;
}

// Absolute address of sound resource idx in the main script (flagmain) or the current one.
s32 adresmus(s32 idx)
{
    u32 mem = get_0x14_script_org_offset(alis.flagmain ? alis.main->vram_org : alis.script->vram_org);

    // v1.0: table of unsigned 16-bit offsets from its base
    if (alis.platform.version == 10)
    {
        u32 addr = mem;
        if (alis.platform.kind == EPlatformPC)
        {
            addr += (u16)xread16(addr + 0xe);
            addr += (u16)xread16(addr + 0xc);
        }
        else
        {
            addr += (u32)xread32(addr + 0xe);
            addr += (u32)xread32(addr + 0xc);
        }

        return addr + (u16)xread16(addr + idx * 2);
    }

    s32 off = xread32(mem + 0xe);
    u32 addr = mem + off;

    s32 len = xread16(addr + 0x10);
    if (len > idx)
    {
        s32 at = xread32(addr + 0xc) + off + idx * 4;
        return mem + xread32(mem + at) + at;
    }

    ALIS_DEBUG(EDebugFatal, "ERROR: Failed to read sound resource at index %d from script %s\n", idx, alis.flagmain ? alis.main->name : alis.script->name);
    return mem + 0x11;
}

#pragma mark -
#pragma mark tab functions

s32 tabint(u32 addr)
{
    s32 result = addr + alis.varD7 * 2;
    s8 length = xread8(--addr);
    --addr;
    
    if (length < 0)
    {
        if (alis.platform.version >= 30)
        {
            u32 newaddr = xread32(addr + 2);
            newaddr = xread32(newaddr);
            result = newaddr + alis.varD7 * 2;

            length &= 0xf;
            for (s32 i = 0; i < length; i++)
            {
                result += xread32((addr -= 4)) * *alis.acc++;
            }
        }
    }
    else
    {
        if (alis.platform.version >= 30)
        {
            length &= 0xf;
        }

        for (int i = 0; i < length; i++)
        {
            result += (u16)xread16((addr -= 2)) * *alis.acc++;
        }
    }
    
    return result;
}

s32 tabchar(u32 addr)
{
    s32 result = addr + alis.varD7;
    s8 length = xread8(--addr);
    --addr;
    
    if (length < 0)
    {
        if (alis.platform.version >= 30)
        {
            u32 newaddr = xread32(addr + 2);
            newaddr = xread32(newaddr);
            result = newaddr + alis.varD7;

            length &= 0xf;
            for (s32 i = 0; i < length; i++)
            {
                result += xread32((addr -= 4)) * *alis.acc++;
            }
        }
    }
    else
    {
        if (alis.platform.version >= 30)
        {
            length &= 0xf;
        }

        for (int i = 0; i < length; i++)
        {
            result += (u16)xread16((addr -= 2)) * *alis.acc++;
        }
    }

    return result;
}

s32 tabstring(u32 addr)
{
    s32 result = addr;
    s8 length = xread8(--addr);
    result += alis.varD7 * xread8(--addr);

    if (length < 0)
    {
        if (alis.platform.version >= 30)
        {
            u32 newaddr = xread32(addr + 2);
            newaddr = xread32(newaddr);
            result = newaddr + alis.varD7 * xread8(addr + 1);

            length &= 0xf;
            for (s32 i = 0; i < length; i++)
            {
                result += xread32((addr -= 4)) * *alis.acc++;
            }
        }
    }
    else
    {
        if (alis.platform.version >= 30)
        {
            length &= 0xf;
        }

        for (int i = 0; i < length; i++)
        {
            result += (u16)xread16((addr -= 2)) * *alis.acc++;
        }
    }
    
    return result;
}

FILE *afopen(char *path, u16 openmode)
{
    bool exists = sys_fexists(path);

    alis.fp = sys_fopen((char *)path, openmode);
    alis.typepack = 0;
    alis.openmode = openmode;
    if ((openmode & 0x100) == 0 || exists)
    {
        if ((openmode & 0x200) == 0)
        {
            if (alis.fp != NULL)
            {
                if ((openmode & 0x800) != 0)
                {
                    fread(alis.buffer, 0xc, 1, alis.fp);

                    u32 type = xswap32be(*(u32 *)alis.buffer);
                    if (type == 0x50423630)
                    {
                        alis.typepack = 0xa0;
                        alis.wordpack = 0;
                        alis.longpack = xswap32be(*(u32 *)(alis.buffer + 4));
                    }
                    else if (type == 0x50573630)
                    {
                        alis.typepack = 0xa0;
                        alis.wordpack = 1;
                        alis.longpack = xswap32be(*(u32 *)(alis.buffer + 4));
                    }
                }
            }
        }
    }
    
    return alis.fp;
}
