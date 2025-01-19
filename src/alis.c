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
#include "video.h"

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


const u32 kHostRAMSize          = 1024 * 1024 * 2;
const u32 kVirtualRAMSize       = 0xffff * sizeof(u8);


// =============================================================================
// MARK: - Private
// =============================================================================

//u8 used_opcodes[512];

void readexec(sAlisOpcode * table, char * name, u8 identation) {
#ifndef NDEBUG
    if (alis.script->pc < alis.script->pc_org || alis.script->pc >= alis.script->pc_org + alis.script->data->sz || alis.script->pc - alis.script->pc_org == kVirtualRAMSize) {
        // pc overflow !
        printf("\n");
        ALIS_DEBUG(EDebugFatal, disalis ? "ERROR: %s" : "%s", "PC OVERFLOW !\n");
        ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
        alis.state = eAlisStateStopped;
    }
    else {
        // fetch code
        u8 code = *(alis.mem + alis.script->pc++);
        sAlisOpcode opcode = table[code];
        
//        if (table == opcodes)
//        {
//            if (used_opcodes[code] == 0)
//            {
//                printf("Use of: %s\n", opcode.name);
//                used_opcodes[code] ++;
//            }
//        }

        if (!disalis) {
            ALIS_DEBUG(EDebugInfo, " %s", opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
        }
        else {
            u32 prg_offset = (alis.script->pc - alis.script->pc_org)-1;                        // -1 -> start with 0 not with 1
            u32 file_offset = prg_offset + alis.script->data->header.code_loc_offset + 2 + 6;  //  2 -> size of code_loc_offset 
                                                                                               //  6 -> kPackedHeaderSize = 6
            if (alis.script->name == alis.main->name) { file_offset += 16; }                   // 16 -> kVMSpecsSize = 16

            if (!strcmp(name, "opcode")) {
                ALIS_DEBUG(EDebugInfo, "\n%s [%.6x]%.6x: %.2x: %s ### ", alis.script->name, file_offset, prg_offset, opcode.name[0] == 0 ? code : opcode.code, opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
            }
            else {
                ALIS_DEBUG(EDebugInfo, "\n      --> [%.6x]%.6x: %.2x: %s ### ", file_offset, prg_offset, opcode.name[0] == 0 ? code : opcode.code, opcode.name[0] == 0 ? "UNKNOWN" : opcode.name);
            }
        }
        if (opcode.name[0] == 0) {  // The opcode (new?) is missing in the name tables, VM behaviour will be inadequate.
                                    // It is necessary to add it to the appropriate name table
                                    // (codop, codesc1, codesc2, codesc3, oper, store, or add)
              printf("\n");
              ALIS_DEBUG(EDebugFatal, disalis ? "ERROR: Opcode 0x%.2x is missing in %ss table.\n" : "Opcode 0x%.2x is missing in %ss table.\n", code, name);
              if (!VM_IGNORE_ERRORS) {
                  ALIS_DEBUG(EDebugSystem, "A STOP signal has been sent to the VM queue...\n");
                  alis.state = eAlisStateStopped;
              }
            }
            else {
               opcode.fptr();
            }
    }
#else
    sAlisOpcode opcode = table[*(alis.mem + alis.script->pc++)];
    opcode.fptr();
#endif
}

void readexec_opcode(void) {
#ifndef NDEBUG
    if (!disalis) {
       ALIS_DEBUG(EDebugInfo, "\n%s [%.6x:%.4x]: 0x%06x:", alis.script->name, alis.script->vram_org, (u16)(alis.script->vacc_off), alis.script->pc);
    }
#endif
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
        
//        // read raw specs header
//        alis.specs.script_data_tab_len = fread16(fp);
//        alis.specs.script_vram_tab_len = fread16(fp);
//        alis.specs.unused = fread32(fp);
//        alis.specs.max_allocatable_vram = fread32(fp);
//        alis.specs.vram_to_data_offset = fread32(fp);
//        alis.specs.vram_to_data_offset += 3;
//        alis.specs.vram_to_data_offset *= 0x28;
//        
//        alis.vprotect = 0;
//        
//        // set the location of scripts' vrams table
//        alis.atprog = ALIS_VM_RAM_ORG;
//        alis.atprog_ptr = (u32 *)(alis.mem + alis.atprog);
//        alis.atent = alis.atprog + 0xf0;
//        alis.atent_ptr = (sScriptLoc *)(alis.vram_org + 0xf0); // (alis.specs.script_data_tab_len * sizeof(u32)));
//        alis.maxent = alis.specs.script_vram_tab_len;
//        alis.debent = alis.atent + alis.maxent * 6;
//        alis.finent = alis.debent;
//
//        image.debsprit = ((alis.debent + alis.specs.max_allocatable_vram) | 0xf) + 1;
//        image.finsprit = image.debsprit + alis.specs.vram_to_data_offset;
//        alis.debprog = image.finsprit;
//        alis.finprog = alis.debprog;
//        alis.dernprog = alis.atprog;
//        alis.maxprog = 0x3c; // TODO: read from script / alis2
        
        // read raw specs header
        alis.specs.script_data_tab_len = fread16(fp);
        alis.specs.script_vram_tab_len = fread16(fp);
        alis.specs.unused = fread32(fp);
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

        // TODO: ...
        alis.finmem = kHostRAMSize; // 0xf6e98;

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
u8 alis_init(sPlatform platform) {
    ALIS_DEBUG(EDebugVerbose, "ALIS: Init.\n");

    alis.platform = platform;

    alis.cdspeed = 0x493e0;
    alis.pays = 0;         // pays: keymode=Q=>0; keymode=A=>1 (France); keymode=Z=>2 (Germany)
                           // cf. Z=QWERTZU=2, Q=QWERTY=1, A=AZERTY=0 -> keyboard_current
    vram_init();

    script_guess_game(platform.main);
    if (alis.platform.uid <= 0) return 1;

    alis.timeclock = 0;
    
    audio.fsound = 1;
    audio.fmusic = 1;
    audio.musicId = 0xffff;

//    alis.nmode = 0; // 0 = atari 16 colors
//                    // 3 = mono
//                    // 8 = falcon 256 colors
    
    memset(audio.tabinst, 0, sizeof(audio.tabinst));

    alis.restart_loop = 0;
    alis.automode = 0;
    
    memset(&image, 0, sizeof(image));
    
    switch (alis.platform.kind) {
        case EPlatformAtari:
        case EPlatformFalcon:
            memset(image.tpalet, 0xff, 768 * 4);
            memset(image.mpalet, 0xff, 768 * 4);
            break;
            
        case EPlatformPC:
            memcpy(image.tpalet, cga_palette, sizeof(cga_palette));
            memcpy(image.mpalet, cga_palette, sizeof(cga_palette));
            break;
            
        default:
            memset(image.tpalet, 0, 768 * 4);
            memset(image.mpalet, 0, 768 * 4);
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

    // init virtual ram
    alis.mem = malloc(sizeof(u8) * kHostRAMSize);
    memset(alis.mem, 0, sizeof(u8) * kHostRAMSize);

    image.spritemem = (u8 *)malloc(1024 * 1024);
    memset(image.spritemem, 0x0, 1024 * 1024);
    
    image.physic = (u8 *)malloc(alis.platform.width * alis.platform.height);
    memset(image.physic, 0, alis.platform.width * alis.platform.height);
    image.logic  = (u8 *)malloc(alis.platform.width * alis.platform.height);
    memset(image.logic, 0, alis.platform.width * alis.platform.height);

    image.logx1 = 0;
    image.logx2 = alis.platform.width - 1;
    image.logy1 = 0;
    image.logy2 = alis.platform.height - 1;

    // NOTE: cswitching is never called for older games
    // TODO: check other PC games. Robinson's Requiem, Storm Master (IBM PC): fswitch = 0
    // TODO: fswitch is not set to 1 on the PC, but in the current alis implementation
    // it must be set, otherwise the game won't show anything
    // if (alis.platform.kind == EPlatformPC)
    // {
    //   alis.fswitch = 0;
    // } else
    alis.fswitch = 1;
    alis.flagmain = 0;
    
    alis.fallent = 0;
    alis.fseq = 0;
    alis.fmuldes = 0;
    alis.fadddes = 0;
    alis.ferase = 0;

    alis.saversp = 0;
//    alis.basemem = 0x22400 + 0x3600;    // I3 0x25a00
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
    alis.vstandard = 0x153C6;
    memset(alis.mem + alis.vstandard, 0, 256);

    alis.tabptr = 0x1b82e; // tab containing 2 * 16 pointers
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
    
    // TODO: init virtual accumulator
//    alis.acc_org = alis.script->vram_org;
//    alis.acc = alis.script->vram_org + kVirtualRAMSize;
    alis.acc = alis.acc_org = (s16 *)(alis.mem + 0x1cdc0); // 0x198e2);
    
    alis.fmouse = 0xff;

    // init host system stuff
    host.pixelbuf.w = alis.platform.width;
    host.pixelbuf.h = alis.platform.height;
    host.pixelbuf.data = image.physic;
    host.pixelbuf.palette = image.ampalet;

    alis.load_delay = 0;
    alis.unload_delay = 0;
    
    // load main script
    alis_load_main();
    alis.script = alis.main;
    alis.basemain = alis.main->vram_org;
    
    alis.desmouse = NULL;

    return 0;
}

void alis_deinit(void) {
    // free scripts
    // TODO: use real script table / cunload
    // for(int i = 0; i < MAX_SCRIPTS; i++) {
    //     script_unload(alis.script);
    // }
    
    //vram_deinit(alis.vram);
    free(alis.mem);
}

extern sMV1Audio mv1a;
extern sMV2Audio mv2a;

extern u16 fls_drawing;
extern u16 fls_pallines;
extern s8  fls_state;
extern u8 fls_ham6;
extern u8 fls_s512;
extern u8 pvgalogic[1024 * 1024];
extern u8 *vgalogic;
extern u8 *vgalogic_df;
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
    size_t vram_size = sizeof(u8) * kHostRAMSize;
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
            fwrite(&i, 2, 1, fp);
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
            fwrite(&i, 2, 1, fp);
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

    fwrite(&(image), sizeof(image), 1, fp);
    fwrite(image.spritemem, 1024 * 1024, 1, fp);
    fwrite(image.physic, alis.platform.width * alis.platform.height, 1, fp);
    fwrite(image.logic, alis.platform.width * alis.platform.height, 1, fp);
    
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

    fwrite(&(audio), sizeof(audio), 1, fp);
    for (int i = 0; i < 4; i++)
    {
        value = (u32)((s64)audio.channels[i].address - (s64)alis.mem);
        fwrite(&value, 4, 1, fp);
    }
    
    fwrite(&audio_type, sizeof(audio_type), 1, fp);
    
    fwrite(&(mv1a), sizeof(mv1a), 1, fp);
    fwrite(&(mv2a), sizeof(mv2a), 1, fp);
    
    // FLI/FLC video
    
    fwrite(&bfilm, sizeof(bfilm), 1, fp);
    value = (u32)((s64)bfilm.addr1 - (s64)alis.mem);
    fwrite(&value, 4, 1, fp);
    value = (u32)((s64)bfilm.addr2 - (s64)alis.mem);
    fwrite(&value, 4, 1, fp);
    value = (u32)((s64)bfilm.endptr - (s64)alis.mem);
    fwrite(&value, 4, 1, fp);
    value = bfilm.delptr ? (u32)((s64)bfilm.delptr - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);
    
    fwrite(&fls_drawing, sizeof(fls_drawing), 1, fp);
    fwrite(&fls_pallines, sizeof(fls_pallines), 1, fp);
    fwrite(&fls_state, sizeof(fls_state), 1, fp);
    fwrite(&fls_ham6, sizeof(fls_ham6), 1, fp);
    fwrite(&fls_s512, sizeof(fls_s512), 1, fp);
    fwrite(&pvgalogic, sizeof(pvgalogic), 1, fp);
    
    value = (u32)((s64)vgalogic - (s64)pvgalogic);
    fwrite(&value, 4, 1, fp);

    value = (u32)((s64)vgalogic_df - (s64)pvgalogic);
    fwrite(&value, 4, 1, fp);

    value = endframe ? (u32)((s64)endframe - (s64)alis.mem) : 0;
    fwrite(&value, 4, 1, fp);

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

    memset(&alis, 0, alis_size);
    fread(&(alis), alis_size, 1, fp);

    strcpy(alis.platform.path, savepath);
    strcpy(alis.platform.main, savemain);
    
    size_t vram_size = sizeof(u8) * kHostRAMSize;
    fread(&vram_size, sizeof(size_t), 1, fp);
    alis.mem = malloc(vram_size);
    fread(alis.mem, vram_size, 1, fp);
    
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
    alis.acc_org = (s16 *)(alis.mem + value);
    
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
    alis.script = alis.live_scripts[value];
    
    // mouse
    
    fread(&value, 4, 1, fp);
    alis.desmouse = value ? alis.mem + value : NULL;
    
    u8 enable_mouse = 0;
    fread(&enable_mouse, 1, 1, fp);

    // image
    
    u8 *spritemem   = image.spritemem;
    u8 *physic      = image.physic;
    u8 *logic       = image.logic;
    u8 *backmap     = image.backmap;
    u8 *atpalet     = image.atpalet;
    u8 *ampalet     = image.ampalet;
    s16 *ptabfen    = image.ptabfen;
    u8 *bufpack     = image.bufpack;
    u8 *wlogic      = image.wlogic;
    
    memset(&image, 0, sizeof(image));
    fread(&(image), sizeof(image), 1, fp);
    
    image.spritemem = spritemem;
    image.physic    = physic;
    image.logic     = logic;
    image.backmap   = backmap;
    image.atpalet   = atpalet;
    image.ampalet   = ampalet;
    image.ptabfen   = ptabfen;
    image.bufpack   = bufpack;
    image.wlogic    = wlogic;
    
    fread(image.spritemem, 1024 * 1024, 1, fp);
    fread(image.physic, alis.platform.width * alis.platform.height, 1, fp);
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
        case 1: audio.soundrout = mv1_soundrout; break;
        case 2: audio.soundrout = mv2_soundrout; break;
        case 3: audio.soundrout = mv2_chiprout; break;
    }
    
    fread(&(mv1a), sizeof(mv1a), 1, fp);
    fread(&(mv2a), sizeof(mv2a), 1, fp);
    
    // FLI/FLC video
    
    fread(&bfilm, sizeof(bfilm), 1, fp);
    fread(&value, 4, 1, fp);
    bfilm.addr1 = alis.mem + value;
    fread(&value, 4, 1, fp);
    bfilm.addr2 = alis.mem + value;
    fread(&value, 4, 1, fp);
    bfilm.endptr = alis.mem + value;
    fread(&value, 4, 1, fp);
    bfilm.delptr = value ? alis.mem + value : 0;

    fread(&fls_drawing, sizeof(fls_drawing), 1, fp);
    fread(&fls_pallines, sizeof(fls_pallines), 1, fp);
    fread(&fls_state, sizeof(fls_state), 1, fp);
    fread(&fls_ham6, sizeof(fls_ham6), 1, fp);
    fread(&fls_s512, sizeof(fls_s512), 1, fp);
    fread(&pvgalogic, sizeof(pvgalogic), 1, fp);
    fread(&value, 4, 1, fp);
    vgalogic = pvgalogic + value;
    fread(&value, 4, 1, fp);
    vgalogic_df = pvgalogic + value;
    fread(&value, 4, 1, fp);
    endframe = value ? alis.mem + value : NULL;
    
    fclose(fp);

    host.pixelbuf.palette = image.ampalet;

    sys_init_timers();

    sys_enable_mouse(enable_mouse);
    set_update_cursor();

    printf("\n");
    ALIS_DEBUG(EDebugSystem, "Savestate loaded from: %s.\n", path);
}

void alis_loop(void) {
    
    alis.script->running = 1;
    while (alis.state && alis.script->running) {
        
        readexec_opcode();
    }
    
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
                sprite->sprite_0x28 = wcaz;
            }
        }
    }
}

void alis_main_V2(void) {
    while (alis.state) {
        
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
        
        alis.script = ENTSCR(alis.varD5);
        
        alis.fallent = 0;
        alis.fseq = 0;
        alis.acc = alis.acc_org;

        if (get_0x24_scan_inter(alis.script->vram_org) < 0 && (get_0x24_scan_inter(alis.script->vram_org) & 2) == 0)
        {
            s32 script_offset = swap32((alis.mem + get_0x14_script_org_offset(alis.script->vram_org) + 10));
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
            
            set_0x01_wait_count(alis.script->vram_org, get_0x01_wait_count(alis.script->vram_org) - 1);
            ALIS_DEBUG(EDebugInfo, "\n %s %s [%.2x, %.2x] ", get_0x01_wait_count(alis.script->vram_org) == 0 ? "RUNNING" : "WAITING", alis.script->name, get_0x01_wait_count(alis.script->vram_org), get_0x02_wait_cycles(alis.script->vram_org));
            if (get_0x01_wait_count(alis.script->vram_org) == 0)
            {
                savecoord(alis.script->vram_org);
                
                alis.script->pc = get_0x08_script_ret_offset(alis.script->vram_org);
                alis.script->vacc_off = get_0x0a_vacc_offset(alis.script->vram_org);
                alis.fseq++;
                alis_loop();
                
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
            alis.script = ENTSCR(alis.varD5);
            draw();
#if ALIS_USE_THREADS <= 0
            sys_poll_event();
#endif
        }
    }
}

void alis_main_V3(void) {
    while (alis.state) {

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
        
        alis.script = ENTSCR(alis.varD5);
        
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
            
            if (get_0x3e_wait_time(alis.script->vram_org) < alis.timeclock) {
                set_0x3e_wait_time(alis.script->vram_org, get_0x3a_wait_cycles(alis.script->vram_org) + alis.timeclock);
                savecoord(alis.script->vram_org);
                
                alis.script->pc = get_0x08_script_ret_offset(alis.script->vram_org);
                alis.script->vacc_off = get_0x0a_vacc_offset(alis.script->vram_org);
                alis.fseq++;
                alis_loop();
                
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
            alis.script = ENTSCR(alis.varD5);
            draw();
#if ALIS_USE_THREADS <= 0
            sys_poll_event();
#endif
        }
    }
}

int alis_thread(void *data) {
    alis.cstopret = 0;
    alis.varD5 = 0;
    
    if (alis.platform.version < 30)
    {
        alis_main_V2();
    }
    else
    {
        alis_main_V3();
    }

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
    addr += xread32(addr + 0xe);
    addr += xread32(addr + 0x6);
    return addr + xread16(addr + (idx * 2));
}

s32 adresmus(s32 idx)
{
    u32 mem = get_0x14_script_org_offset(alis.flagmain ? alis.main->vram_org : alis.script->vram_org);
    s32 off = xread32(mem + 0xe);
    u32 addr = mem + off;

    s32 len = xread16(addr + 0x10);
    if (len > idx)
    {
        s32 at = xread32(addr + 0xc) + off + idx * 4;
        return xread32(mem + at) + at;
    }

    ALIS_DEBUG(EDebugFatal, "ERROR: Failed to read sound resource at index %d from script %s\n", idx, alis.flagmain ? alis.main->name : alis.script->name);
    return 0x11;
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

                    u32 type = xpcswap32(*(u32 *)alis.buffer);
                    if (type == 0x50423630)
                    {
                        alis.typepack = 0xa0;
                        alis.wordpack = 0;
                        alis.longpack = xpcswap32(*(u32 *)(alis.buffer + 4));
                    }
                    else if (type == 0x50573630)
                    {
                        alis.typepack = 0xa0;
                        alis.wordpack = 1;
                        alis.longpack = xpcswap32(*(u32 *)(alis.buffer + 4));
                    }
                }
            }
        }
    }
    
    return alis.fp;
}
