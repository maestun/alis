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

// VM-core translation unit: opt into hot-state register pinning (see alis.h).
#define ALIS_VM_CORE
#include "alis.h"
#include "alis_private.h"
#include "debug.h"
#include "image.h"
#include "mem.h"
#include "platform.h"
#include "script.h"
#include "unpack.h"
#include "utils.h"

// =============================================================================
// MARK: - Unpacker
// =============================================================================

// byte 0 -> magic
// byte 1..3 -> unpacked size (24 bits)
// byte 4..5 -> main script if zero
// if main: byte 6..21 -> main header
// if main: byte 22..29 -> dic
// if not main: byte 6..13 -> dic

u8 is_packedx(u32 magic) {
    return ((magic >> 24) & 0xf0) == 0xa0;
}

u32 get_unpacked_size(u32 magic) {
    return (magic & 0x00ffffff);
}

int is_main(u16 check) {
    return kMainScriptID == check;
}

// =============================================================================
// MARK: - Script API
// =============================================================================

int search_insert(u32 *nums, u32 size, int target_id) {
    
    int start = 0;
    int end = size - 1;
        
    while(start <= end) {
        int mid = start + (end - start) / 2;
        int current_id = read16(alis.mem + nums[mid]);
        if (current_id == target_id)
            return mid;
        
        if (current_id < target_id)
            start = mid + 1;
        else
            end = mid - 1;
    }
    
    return start;
}

void protect(void)
{
    if (alis.vprotect != 0)
    {
        u8 *workbuffptr = alis.buffer;
        alis.basemain = alis.atprog;
    }
    
    alis.vprotect = 0;
}

void debprot(void)
{
    protect();
}

void invdigit(u8 *sample)
{
    u8 type = sample[0];
    if (type == 2)
    {
        if (read32(sample + 0xc) != 0)
        {
            s16 length = (read32(sample + 2) - read32(sample + 0xc)) - 0x21;
            if (-1 < length)
            {
                u8 *dst = sample + 0x10 + read32(sample + 0xc);
                u8 *src = dst + 8;
                memmove(dst, src, length);
            }
            
            write32(sample + 0xc, read32(sample + 0xc) - 0x10);
        }
        
        write32(sample + 2, read32(sample + 2) - 0x10);
    }
    
    u32 length = read32(sample + 2) - 0x10;
    u8 *smpdata = sample + 0x10;
    
    if (alis.platform.kind == EPlatformMac)
    {
        if (sample[0] == 2)
        {
            for (int i = 0; i < length; i++)
            {
                smpdata[i] += 0x80;
                if (smpdata[i] == 0)
                    smpdata[i] = 1;
            }
        }
    }
    else if (alis.platform.kind == EPlatformAtari)
    {
        for (int i = 0; i < length; i++)
        {
            if (sample[0] == 2)
                smpdata[i] *= 2;

            smpdata[i] += 0x80;
            if (smpdata[i] == 0)
                smpdata[i] = 1;
        }
    }
    else if (alis.platform.kind == EPlatformAmiga)
    {
        for (int i = 0; i < length; i++)
        {
            if (smpdata[i] == 0)
                smpdata[i] = 1;
        }
    }

    if (alis.platform.version >= 20)
    {
        *(u8 *)(sample + 0x10 + length - 1) = 0;
    }
}

s8 tfibo[16] = {
    0xDE, 0xEB, 0xF3, 0xF8,
    0xFB, 0xFD, 0xFE, 0xFF,
    0x01, 0x02, 0x03, 0x05,
    0x08, 0x0D, 0x15, 0x22 };

void script_guess_game(const char * script_path) {
    
    alis.platform.uid = 0;
    FILE * fp = fopen(script_path, "rb");
    if (fp) {
        ALIS_DEBUG(EDebugInfo, "\nLoading script file: %s\n", script_path);
        
        // get packed file size
        fseek(fp, 0L, SEEK_END);
        u32 input_sz = (u32)ftell(fp);
        rewind(fp);
        
        if (input_sz > 24)
        {
            // read header
            u32 magic = fread32(fp);
            u16 check = fread16(fp);
            
            if (is_main(check)) {
                
                alis.header.val0 = fread16(fp);
                alis.header.val1 = fread16(fp);
                alis.header.val2 = fread32(fp);
                alis.header.val3 = fread32(fp);
                alis.header.val4 = fread32(fp);
                
                alis.platform.uid = alis.header.val3 * alis.header.val4;
                
                EPxFormat px_format = EPxFormatChunky;
                switch (alis.platform.kind)
                {
                    case EPlatformMac: px_format = EPxFormatSTPlanar; break;
                    case EPlatformAtari: px_format = EPxFormatSTPlanar; break;
                    case EPlatformAmiga: px_format = EPxFormatAmPlanar; break;
                    default: px_format = EPxFormatChunky;
                }

                switch (alis.platform.uid)
                {
                    //#######################################################################################
                    //   In order of release
                    //#######################################################################################
                    case EGameManhattanDealers0:
                    case EGameManhattanDealers1:     strcpy(alis.platform.name, "Manhattan Dealers");
                                                     alis.platform.version = 10;  // 0 [0x00]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.dbl_buf = 0;
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameMadShow:               strcpy(alis.platform.name, "Mad Show");
                                                     alis.platform.version = 11;  // 2.1 [0x15], 2.3 [0x17], 0 [0x00]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.px_format = px_format;
                                                     alis.basemem = 0x23e00;
                                                     break;
                    //#######################################################################################
                    case EGameTarghan0:
                    case EGameTarghan1:              strcpy(alis.platform.name, "Targhan");
                                                     alis.platform.version = 12;  // 2.1 [0x15], 2.2 [0x16], 0 [0x00]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.px_format = px_format;
                                                     alis.platform.dbl_buf = 0;
                                                     alis.basemem = 0x26000;
                                                     break;
                    //#######################################################################################
                    case EGameWindsurfWilly:         strcpy(alis.platform.name, "Windsurf Willy");
                                                     alis.platform.version = 12;  // 2.1 [0x15], 0 [0x00]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.px_format = px_format;
                                                     alis.basemem = 0x25400;
                                                     break;
                    //#######################################################################################
                    case EGameLeFeticheMaya:         strcpy(alis.platform.name, "Le Fetiche Maya");
                                                     alis.platform.version = 13;  // 2.1 [0x15], 0 [0x00]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.px_format = px_format;
                                                     alis.platform.dbl_buf = 0;
                                                     alis.basemem = 0x25e00;
                                                     break;
                    //#######################################################################################
                    case EGameColorado:              strcpy(alis.platform.name, "Colorado");
                                                     alis.platform.version = 13;  // 2.1 [0x15]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.px_format = px_format;
                                                     alis.basemem = 0x26000;
                                                     break;
                    //#######################################################################################
                    case EGameStarblade:             strcpy(alis.platform.name, "Starblade");
                                                     alis.platform.version = 20;  // 2.1 [0x15]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.px_format = px_format;
                                                     alis.platform.dbl_buf = 0;
                                                     alis.basemem = 0x21400;
                                                     break;
                    //#######################################################################################
                    case EGameCrystalsOfArborea0:
                    case EGameCrystalsOfArborea1:
                    case EGameCrystalsOfArborea2:    strcpy(alis.platform.name, "Crystals Of Arborea");
                                                     alis.platform.version = 20;  // 2.2 [0x16]
                                                     alis.platform.bpp = 4;
                                                     alis.basemem = 0x1f300;
                                                     break;
                    //#######################################################################################
                    case EGameMetalMutant:           strcpy(alis.platform.name, "Metal Mutant");
                                                     alis.platform.version = 20;  // 2.2 [0x16]
                                                     alis.platform.bpp = 4;
                                                     alis.basemem = 0x20800;
                                                     alis.platform.dbl_buf = 0;
                                                     break;
                    //#######################################################################################
                    case EGameBostonBombClub:        strcpy(alis.platform.name, "Boston Bomb Club");
                                                     alis.platform.version = 21;  // 2.2 [0x16]
                                                     alis.platform.bpp = 4;
                                                     alis.platform.px_format = px_format;
                                                     alis.basemem = 0x27d00;
                                                     break;
                    //#######################################################################################
                    case EGameXyphoesFantasy:        strcpy(alis.platform.name, "Xyphoes Fantasy");
                                                     alis.platform.version = 0;
                                                     alis.platform.bpp = 4;
                                                     alis.platform.dbl_buf = 0;
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameKillingFist:           strcpy(alis.platform.name, "Killing Fist");
                                                     alis.platform.version = 0;
                                                     alis.platform.bpp = 4;
                                                     alis.platform.dbl_buf = 0;
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameStormMaster:           strcpy(alis.platform.name, "Storm Master");
                                                     alis.platform.version = 20;  // 2.3 [0x17]
                                                     alis.platform.bpp = 4;
                                                     alis.basemem = 0x1ff00;
                                                     break;
                    //#######################################################################################
                    case EGameIshar_1:               strcpy(alis.platform.name, "Ishar 1");
                                                     alis.platform.version = 20;  // 2.3 [0x17]
                                                     alis.basemem = 0x20000;
                                                     break;
                    //#######################################################################################
                    case EGameBunnyBricks:           strcpy(alis.platform.name, "Bunny Bricks");
                                                     alis.platform.version = 21;  // 2.3 [0x17]
                                                     alis.basemem = 0x22200;
                                                     break;
                    //#######################################################################################
                    case EGameTransarctica:          strcpy(alis.platform.name, "Transarctica");
                                                     alis.platform.version = 22;  // 2.3 [0x17], 2.8 [0x1c]
                                                     alis.basemem = 0x21d00;
                                                     break;
                    //#######################################################################################
                    case EGameIshar_2:               strcpy(alis.platform.name, "Ishar 2");
                                                     alis.platform.version = 21;  // 2.8 [0x1c]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameRobinsonsRequiem0:
                    case EGameRobinsonsRequiem1:     strcpy(alis.platform.name, "Robinson's Requiem");
                                                     alis.platform.version = 31;  // 3.0 [0x1e]
                                                     alis.basemem = 0x2b400;
                                                     break;
                    //#######################################################################################
                    case EGameIshar_3:               strcpy(alis.platform.name, "Ishar 3");
                                                     alis.platform.version = 30;  // 3.0 [0x1e]
                                                     alis.basemem = 0x25a00;
                                                     break;
                    //#######################################################################################
                    case EGameManualRRQ:             strcpy(alis.platform.name, "Robinson's Requiem CD Manual");
                                                     alis.platform.version = 60;  // 6.0 [0x3c]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameDeus:                  strcpy(alis.platform.name, "Deus");
                                                     alis.platform.version = 60;  // 6.0 [0x3c]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameTimeWarriors:          strcpy(alis.platform.name, "Time Warriors");
                                                     alis.platform.version = 72;  // 7.2 [0x48]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameAsghan:                strcpy(alis.platform.name, "Asghan");
                                                     alis.platform.version = 95;  // 9.5 [0x5f]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameTournamentOfWarriors:  strcpy(alis.platform.name, "Tournament Of Warriors");
                                                     alis.platform.version = 97;  // 9.7 [0x61]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameArabianNights:         strcpy(alis.platform.name, "Arabian Nights");
                                                     alis.platform.version = 113; // 11.3 [0x71]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameLesVisiteurs:          strcpy(alis.platform.name, "Les Visiteurs");
                                                     alis.platform.version = 113; // 11.3 [0x71]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    case EGameInspectorGadget:       strcpy(alis.platform.name, "Inspector Gadget");
                                                     alis.platform.version = 224; // 22.4 [0xe0]
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                    default:                         strcpy(alis.platform.name, "UNKNOWN");
                                                     alis.platform.version = 20;
                                                     alis.basemem = 0x22400;
                                                     break;
                    //#######################################################################################
                }

                // Announce once (native Atari calls this twice: memory pre-check, then alis_init).
                static int announced = 0;
                if (!announced) {
                    announced = 1;
                    printf("Starting %s %s (ALIS ver. %.1f)\n", alis.platform.name, alis.platform.desc, alis.platform.version / 10.0);
                    printf("         Host:  %s,  Platform:  %s,  Artificial version: %d\n", is_host_le() ? "LE" : "BE", alis.platform.is_little_endian ? "LE" : "BE", alis.platform.version);
                    printf("         Platform UID: Specs+0x08 x Specs+0x0c = Vmaxvram x Vmaxsprite\n");
                    printf("                   =>  0x%08x x 0x%08x = 0x%08x\n", alis.header.val3, alis.header.val4, alis.platform.uid);
                    fflush(stdout);
                }
            }
            else {
                ALIS_DEBUG(EDebugError, "The version of the %s file is not supported, or it is not a valid main file of the game.\n", script_path);
            }
        }
        else {
            ALIS_DEBUG(EDebugError, "The size of the main file %s is %d bytes, and it is smaller than required.\n", script_path, input_sz);
        }
        // cleanup
        fclose(fp);
    }
    else {
        ALIS_DEBUG(EDebugError, "Could not open main file %s.\n", script_path);
    }
}

#if defined(ALIS_PROFILE_DRAW)
// Sprite-conversion ticks for the current load (reset in script_load, reported in its LOAD line).
static u32 g_lp_conv_ticks = 0;
#endif

static void tochunky(u8 *bitmap)
{
    u16 width = read16(bitmap + 2) + 1;
    u16 height = read16(bitmap + 4) + 1;
    u8 pixels[16];
    u32 at = 6;
    
    for (int b = 0; b < width * height; b+=16)
    {
        memset(pixels, 0, 16);
        for (int c = 0; c < 8; c++)
        {
            uint32_t rot = (7 - c);
            uint32_t mask = 1 << rot;
            pixels[8 + c] = (((bitmap[at + 1] & mask) >> rot) << 0) | (((bitmap[at + 3] & mask) >> rot) << 1) | (((bitmap[at + 5] & mask) >> rot) << 2) | (((bitmap[at + 7] & mask) >> rot) << 3);
            pixels[0 + c] = (((bitmap[at + 0] & mask) >> rot) << 0) | (((bitmap[at + 2] & mask) >> rot) << 1) | (((bitmap[at + 4] & mask) >> rot) << 2) | (((bitmap[at + 6] & mask) >> rot) << 3);
        }
        
        for (int d = 0; d < 8; d++)
        {
            bitmap[at++] = (pixels[d * 2 + 0] << 4) | (pixels[d * 2 + 1]);
        }
    }
}

static void all_tosigned(u8 *data)
{
    u8 *hdr = data + read32(data + 0xe);
    u8 *tab = hdr + read32(hdr + 0xc);
    s16 count = read16(hdr + 0x10);
    if (count <= 0)
        return;
    
    u8 *done[count];
    s16 ndone = 0;

    for (s16 i = 0; i < count; i++)
    {
        u8 *smp = tab + (u16)read16(tab + i * 2);
        if (smp[0] != 1)
            continue;

        s16 j = 0;
        while (j < ndone && done[j] != smp)
            j++;
        
        if (j < ndone)
            continue;
        
        done[ndone++] = smp;

        u16 len = (smp[1] << 8) | smp[2];
        for (u16 k = 0; k < len; k++)
            smp[3 + k] ^= 0x80;
    }
}

static void all_tochunky(u8 *data)
{
    u8 *hdr = data + read32(data + 0xe);
    u8 *tab = hdr + read32(hdr);
    s16 count = read16(hdr + 4);
    if (count <= 0)
        return;
    
    u8 *done[count];
    s16 ndone = 0;

    for (s16 i = 0; i < count; i++)
    {
        u8 *bmp = tab + (s16)read16(tab + i * 2);
        if (bmp[0] != 0)
            continue;

        s16 j = 0;
        while (j < ndone && done[j] != bmp)
            j++;
        
        if (j < ndone)
            continue;
        
        done[ndone++] = bmp;

        tochunky(bmp);
    }
}

sAlisScriptData * script_init(const char * name, u8 * data, u32 data_sz) {
    s16 id = swap16((data + 0));
    s16 insert = debprotf(id);
    if (insert > 0 && insert < alis.nbprog)
    {
        sAlisScriptData *script = alis.loaded_scripts[insert];
        if (script->header.id == id)
        {
            for (int i = 0; i < alis.nbprog; i++)
            {
                ALIS_DEBUG(EDebugVerbose, "\n%c%s ID %.2x AT %.6x", i == insert ? '*' : ' ', alis.loaded_scripts[i]->name, read16(alis.mem + alis.atprog_ptr[i]), alis.atprog_ptr[i]);
            }
            
            ALIS_DEBUG(EDebugVerbose, "\n");

            ALIS_DEBUG(EDebugVerbose, " (NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", alis.script->name, alis.script->vram_org, alis.finent, alis.script->vacc_off, alis.script->pc_org);

            ALIS_DEBUG(EDebugVerbose, "Initialized script '%s' (ID = 0x%02x)\nDATA at address 0x%x - 0x%x\n", script->name, script->header.id, script->data_org, alis.finprog);
            return script;
        }
    }
    
    // init script
    sAlisScriptData * script = (sAlisScriptData *)malloc(sizeof(sAlisScriptData));
    memset(script, 0, sizeof(sAlisScriptData));
    strcpy(script->name, name);
    script->sz = data_sz;
    script->type = alis.typepack;
    
    // script data
    script->header.id = swap16((data + 0));
    
    if (alis.platform.kind == EPlatformPC)
    {
        script->header.unknown01 = *(data + 4);
        script->header.unknown02 = *(data + 5);
        script->header.code_loc_offset = swap16((data + 2));
    }
    else
    {
        script->header.unknown01 = *(data + 2);
        script->header.unknown02 = *(data + 3);
        script->header.code_loc_offset = swap16((data + 4));
    }
    
    script->header.ret_offset = swap32((data + 6));
    script->header.dw_unknown3 = swap32((data + 10));
    script->header.dw_unknown4 = swap32((data + 14));
    script->header.w_unknown5 = swap16((data + 18));
    script->header.vram_alloc_sz = swap16((data + 20));
    script->header.w_unknown7 = swap16((data + 22));
    script->data_org = alis.finprog;
    // Scripts must stay below finmem (io_malloc blocks sit above it), as the original.
    // Overflow-safe: data_sz may be a wrapped negative s32.
    if ((u32)script->data_org > alis.finmem ||
        data_sz > alis.finmem - (u32)script->data_org)
    {
        ALIS_DEBUG(EDebugError, "Out of VM memory loading '%s': need 0x%x at 0x%x, have 0x%x\n",
                   name, data_sz, script->data_org, alis.finmem);
        alis_fatal = "Out of game memory: this machine has too little RAM for this game.";
        free(script);
        return NULL;
    }
    memcpy(alis.mem + script->data_org, data, data_sz);

    // get insert point
    insert = search_insert(alis.atprog_ptr, alis.nbprog, script->header.id);

    // v1.0 keeps scripts word aligned
    alis.finprog += alis.platform.version == 10 ? (data_sz + 1) & ~1 : data_sz;
    alis.dernprog += 4;
    alis.nbprog ++;

    for (int i = alis.nbprog - 2; i >= insert; i--)
    {
        alis.atprog_ptr[i + 1] = alis.atprog_ptr[i];
        alis.loaded_scripts[i + 1] = alis.loaded_scripts[i];
    }

    alis.atprog_ptr[insert] = script->data_org;
    alis.loaded_scripts[insert] = script;

    for (int i = 0; i < alis.nbprog; i++)
    {
        ALIS_DEBUG(EDebugInfo, "\n%c%s ID %.2x AT %.6x", i == insert ? '*' : ' ', alis.loaded_scripts[i]->name, read16(alis.mem + alis.atprog_ptr[i]), alis.atprog_ptr[i]);
    }
    
    ALIS_DEBUG(EDebugInfo, "\n");
    ALIS_DEBUG(EDebugInfo, "Initialized script '%s' (ID = 0x%02x)\nDATA at address 0x%x - 0x%x\n", script->name, script->header.id, script->data_org, alis.finprog);
    
    if (alis.platform.version == 10)
    {
        if (alis.platform.kind != EPlatformPC)
            all_tochunky(alis.mem + script->data_org);
        
        if (alis.platform.kind == EPlatformOldAtari)
            all_tosigned(alis.mem + script->data_org);
        
        return script;
    }
    
    if (alis.platform.kind == EPlatformMac)
    {
        data = alis.mem + script->data_org;
        s32 l = read32(data + 0xe);

        u8 pixels[16];
        
        s32 sprites = read16(data + l + 4);
        
        for (s32 i = 0; i < sprites; i++)
        {
            s32 a = read32(data + l) + l + i * 4;
            u32 at = read32(data + a) + a;
            u8 *bitmap = data + at;
            
            if (bitmap[0] == 0 || bitmap[0] == 2)
            {
                u16 width = read16(bitmap + 2) + 1;
                u16 height = read16(bitmap + 4) + 1;
                
                at = 6;
                
                for (int b = 0; b < width * height; b+=16)
                {
                    memset(pixels, 0, 16);
                    for (int c = 0; c < 8; c++)
                    {
                        uint32_t rot = (7 - c);
                        uint32_t mask = 1 << rot;
                        pixels[8 + c] = (((bitmap[at + 1] & mask) >> rot) << 0) | (((bitmap[at + 3] & mask) >> rot) << 1);
                        pixels[0 + c] = (((bitmap[at + 0] & mask) >> rot) << 0) | (((bitmap[at + 2] & mask) >> rot) << 1);
                    }
                    
                    for (int d = 0; d < 4; d++)
                    {
                        bitmap[at++] = (pixels[d * 4 + 0] << 6) | (pixels[d * 4 + 1] << 4 | pixels[d * 4 + 2] << 2) | (pixels[d * 4 + 3]);
                    }
                }
            }
        }
    }
    
//    if ((script->type & 1) == 0)
    {
        data = alis.mem + script->data_org;
        s32 l = read32(data + 0xe);

        // convert images if needed
        
        if (alis.platform.kind == EPlatformAtari)
        {
            s32 sprites = read16(data + l + 4);
            
            for (s32 i = 0; i < sprites; i++)
            {
                s32 a = read32(data + l) + l + i * 4;
                u32 at = read32(data + a) + a;
                u8 *bitmap = data + at;
                
                if (bitmap[0] == 0 || bitmap[0] == 2)
                {
                    if ((script->type & 1) == 0)
                        tochunky(bitmap);
                }
                else if (bitmap[0] == 0x18 || bitmap[0] == 0x1a)
                {
                    u16 width = read16(bitmap + 2) + 1;
                    u16 height = read16(bitmap + 4) + 1;
                    s32 nbytes = (width * height) >> 1;

                    u16 *ptr = (u16 *)(bitmap + 6);
                    for (int h = 0; h < nbytes; h++, ptr++)
                    {
                        *ptr = (*ptr | *ptr << 4);
                    }
                }
            }
        }
        else if (alis.platform.kind == EPlatformAmiga)
        {
            s32 sprites = read16(data + l + 4);
            
            for (s32 i = 0; i < sprites; i++)
            {
                s32 a = read32(data + l) + l + i * 4;
                u32 at = read32(data + a) + a;
                u8 *bitmap = data + at;
                
                if (bitmap[0] == 0 || bitmap[0] == 2)
                {
                    if ((script->type & 1) == 0)
                    {
                        if (alis.platform.uid == EGameMadShow || alis.platform.uid == EGameLeFeticheMaya || alis.platform.uid == EGameTarghan0 || alis.platform.uid == EGameTarghan1)
                        {
                            u16 width = read16(bitmap + 2) + 1;
                            u16 height = read16(bitmap + 4) + 1;
                            
                            u8 *temp = malloc(width * height);
                            
                            at = 6;
                            
                            s32 px = 0;
                            u32 planesize = (width * height) / 8;
                            u8 c0, c1, c2, c3;
                            
                            for (s32 h = 0; h < height; h++)
                            {
                                u8 *tgt = temp + (h * width);
                                for (s32 w = 0; w < width; w++, tgt++, px++)
                                {
                                    s32 idx = at + (w + h * width) / 8;
                                    c0 = *(bitmap + idx);
                                    c1 = *(bitmap + (idx += planesize));
                                    c2 = *(bitmap + (idx += planesize));
                                    c3 = *(bitmap + (idx += planesize));
                                    
                                    int bit = 7 - (w % 8);
                                    *tgt = ((c0 >> bit) & 1) | ((c1 >> bit) & 1) << 1 | ((c2 >> bit) & 1) << 2 | ((c3 >> bit) & 1) << 3;
                                }
                            }
                            
                            for (int b = 0; b < width * height / 2; b++)
                            {
                                bitmap[at++] = (temp[b * 2 + 0] << 4) | (temp[b * 2 + 1]);
                            }
                            
                            free(temp);
                        }
                    }
                }
                else if (bitmap[0] == 0x18 || bitmap[0] == 0x1a)
                {
                    u16 width = read16(bitmap + 2) + 1;
                    u16 height = read16(bitmap + 4) + 1;
                    s32 nbytes = (width * height) >> 1;

                    u16 *ptr = (u16 *)(bitmap + 6);
                    for (int h = 0; h < nbytes; h++, ptr++)
                    {
                        *ptr = (*ptr | *ptr << 4);
                    }
                }
            }
        }
        else if (alis.platform.kind == EPlatformAmigaAGA)
        {
            s32 sprites = read16(data + l + 4);

            for (s32 i = 0; i < sprites; i++)
            {
                s32 a = read32(data + l) + l + i * 4;
                u32 at = read32(data + a) + a;
                u8 *bitmap = data + at;

                if (bitmap[0] == 0x10 || bitmap[0] == 0x12)
                {
                    // Bake palette offset low nibble into pixel data
                    // (matches Amiga io_pixel behavior)
                    u8 pal_offset = bitmap[6] & 0x0f;
                    bitmap[6] -= pal_offset;

                    if (pal_offset != 0)
                    {
                        u16 width = read16(bitmap + 2) + 1;
                        u16 height = read16(bitmap + 4) + 1;
                        u8 *pixels = bitmap + 8;
                        s32 nbytes = (width * height) >> 1;

                        for (s32 p = 0; p < nbytes; p++)
                        {
                            u8 byte = pixels[p];
                            u8 hi = (byte >> 4) & 0x0f;
                            u8 lo = byte & 0x0f;

                            if (bitmap[0] == 0x10)
                            {
                                if (hi != 0) hi += pal_offset;
                                if (lo != 0) lo += pal_offset;
                            }
                            else
                            {
                                hi += pal_offset;
                                lo += pal_offset;
                            }

                            pixels[p] = (hi << 4) | (lo & 0x0f);
                        }
                    }
                }
            }
        }

#if (defined(ALIS_CONV_INPLACE) && ALIS_CONV_INPLACE) || (defined(ALIS_NATIVE_PLANAR) && ALIS_NATIVE_PLANAR)
        // Convert every sprite IN PLACE at load (as the original Falcon io_pixel): to 8-bit chunky,
        // or planar on native Atari. destofen reads the result straight from bitmap+8.
        {
            // `name` selects the per-script 2D/3D policy on DOS data (opcodes.c
            // conv_planar_allowed_for): the 3D renderer samples its resources as chunky.
            extern s32 convert_sprites_inplace(u32 org, const char *name);
#if defined(ALIS_PROFILE_DRAW)
            extern u32 sys_profile_ticks_safe(void);
            u32 _lp_c0 = sys_profile_ticks_safe();
#endif
            script->sz += convert_sprites_inplace(script->data_org, script->name);
#if defined(ALIS_PROFILE_DRAW)
            g_lp_conv_ticks += sys_profile_ticks_safe() - _lp_c0;
#endif
        }
#endif
    }

    {
        data = alis.mem + script->data_org;
        s32 l = read32(data + 0xe);

        // convert samples

        u32 maxlen = 0;

        s32 samples = read16(data + l + 0x10);
        for (s32 s = 0; s < samples; s++)
        {
            s32 a = read32(data + 0xc + l) + l + s * 4;
            s32 at = read32(data + a) + a;
            
            u8 *sample = data + at;
            if (sample[0] == 1 || sample[0] == 2)
            {
                if (alis.platform.kind == EPlatformPC
                    ? (sample[6] == 1 && (sample[0] == 1 || sample[0] == 2) && script->type != 0)
                    : sample[6] == 1 && script->type & 1)
                {
                    u32 fulllen = read32(sample + 2);
                    s32 length = ((fulllen - 0x10) >> 1);

                    if (maxlen < length)
                        maxlen = length;
                }
            }
        }
        
        s8 *temp = maxlen ? malloc(maxlen + 1) : NULL;
            
        for (s32 s = 0; s < samples; s++)
        {
            s32 a = read32(data + 0xc + l) + l + s * 4;
            s32 at = read32(data + a) + a;
            
            u8 *sample = data + at;
            if (sample[0] == 1 || sample[0] == 2)
            {
                if (alis.platform.kind == EPlatformPC
                    ? (sample[6] == 1 && (sample[0] == 1 || sample[0] == 2) && script->type != 0)
                    : sample[6] == 1 && script->type & 1)
                {
                    u32 fulllen = read32(sample + 2);
                    s8 *smpdata = (s8 *)(sample + 0x10);

                    s32 length = ((fulllen - 0x10) >> 1);
                    memcpy(temp, smpdata, length);

                    s8 newval = (u8)temp[0];
                    s8 *smpptr0 = (s8 *)sample + 0x11;
                    s8 *smpptr2 = smpptr0;
                    smpptr2[-1] = newval;

                    for (int i = 1; i < length; i++)
                    {
                        smpptr2 = smpptr0;
                        smpptr2[0] = (newval += tfibo[(u8)temp[i] >> 4]);
                        smpptr2[1] = (newval += tfibo[(u8)temp[i] & 0xf]);
                        smpptr0 = smpptr2 + 2;
                    }

                    smpptr2[0] = 0;
                    smpptr2[1] = 0;
                    
                    fulllen -= 2;
                    write32(sample + 2, fulllen);
                    
                    if (sample[0] == 2)
                    {
                        if (read32(sample + 0xc) != 0)
                        {
                            smpptr0 = (s8 *)sample + read32(sample + 0xc) + 0x10;
                            for (int i = 0; i < 8; i++)
                            {
                                smpptr0[i] = -1;
                            }
                        }
                        
                        s32 smplen = fulllen - 0x10;
                        if (read32(sample + 0xc) != 0)
                        {
                            smplen = fulllen - 8;
                        }
                        
                        smpptr0 = (s8 *)sample + smplen;
                        for (int i = 0; i < 8; i++)
                        {
                            smpptr0[i] = -1;
                        }
                    }
                }

                invdigit(sample);
            }
        }
            
        if (temp)
            free(temp);
    }

    return script;
}

// TOOD: use engine version instead of specific game
// move to platform
s32 get_context_size(void)
{
    switch (alis.platform.uid)
    {
        case EGameManhattanDealers0:
        case EGameManhattanDealers1:
        case EGameLeFeticheMaya:
        case EGameColorado:
        case EGameStarblade:
        case EGameMadShow:
        case EGameWindsurfWilly:
            return 0x34;
//            return 0x2e;
            break;
        case EGameTarghan0:
        case EGameTarghan1:
            return 0x34;
//            return 0x30;
            break;
        case EGameCrystalsOfArborea0:
        case EGameCrystalsOfArborea1:
        case EGameCrystalsOfArborea2:
        case EGameStormMaster:
        case EGameMetalMutant:
        case EGameTransarctica:
        case EGameBostonBombClub:
        case EGameBunnyBricks:
        case EGameIshar_1:
        case EGameIshar_2:
            return 0x34;
            break;
        case EGameIshar_3:
        case EGameRobinsonsRequiem0:
        case EGameRobinsonsRequiem1:
        case EGameDeus:
            return 0x3e;
            break;
        default:
            return -1;
    }
}

sAlisScriptLive *script_live(sAlisScriptData * prog) {
    
    sAlisScriptLive *script = (sAlisScriptLive *)malloc(sizeof(sAlisScriptLive));
    memset(script, 0, sizeof(sAlisScriptLive));
    
    script->name        = prog->name;
    script->data        = prog;

    u8 *data = alis.mem + script->data->data_org;
    
    s32 prevfin = swap16((data + 0x12)) + alis.finent;
    
    s32 contextsize = get_context_size();

    alis.finent += swap16(data + 0x16) + swap16(data + 0x12) + contextsize;

    script->vacc_off = (prevfin - alis.finent) & 0xffff;
    script->vram_org = alis.finent;
    
    u16 vram_length = swap16((data + 0x14));
    memset(alis.mem + script->vram_org, 0, vram_length);

    s16 curent = alis.varD5;
    alis.varD7 = alis.dernent;
    
    int caller_idx = curent / sizeof(sScriptLoc);
    int script_idx = alis.dernent / sizeof(sScriptLoc);

    ALIS_DEBUG(EDebugVerbose, " add at idx: %d hooked to idx: %d. ", script_idx, caller_idx);

    set_0x0a_vacc_offset(script->vram_org, script->vacc_off);
    set_0x1c_scan_clr(script->vram_org, script->vacc_off);
    set_0x1e_scan_clr(script->vram_org, script->vacc_off);
    set_0x08_script_ret_offset(script->vram_org, script->data->data_org + script->data->header.code_loc_offset + 2);
    set_0x10_script_id(script->vram_org, script->data->header.id);
    set_0x14_script_org_offset(script->vram_org, script->data->data_org);
    set_0x2e_script_header_word_2(script->vram_org, script->data->header.unknown01);
    set_0x02_wait_cycles(script->vram_org, 1);
    set_0x01_wait_count(script->vram_org, 1);
    set_0x04_cstart_csleep(script->vram_org, 0xff);
    set_0x1a_cforme(script->vram_org, -1);
    set_0x0e_script_ent(script->vram_org, alis.dernent);
    set_0x24_scan_inter(script->vram_org, alis.platform.version >= 31 ? 6 : 2);
    set_0x18_unknown(script->vram_org, 0);
    set_0x0c_vacc_offset(script->vram_org, 0);
    set_0x22_cworld(script->vram_org, 0);
    set_0x20_set_vect(script->vram_org, 0);
    set_0x03_xinv(script->vram_org, 0);
    set_0x25_credon_credoff(script->vram_org, 0);
    set_0x26_creducing(script->vram_org, 0xff);
    set_0x28_unknown(script->vram_org, 0); // wide
    set_0x2a_clinking(script->vram_org, 0); // byte
    set_0x2c_calign(script->vram_org, 0);
    set_0x2d_calign(script->vram_org, 0);

    // TODO: RRQ changes
//    0001bde6 42 68 ff ca     clr.w      (-0x36,A0)
//    0001bdea 42 68 ff c8     clr.w      (-0x38,A0)
    if (!(alis.platform.kind == EPlatformPC && alis.platform.uid == EGameColorado))   // Colorado DOS keeps 0xff
        set_0x26_creducing(script->vram_org, 0);
    // -----------------
    
    if (contextsize > 0x2e)
    {
        set_0x2f_chsprite(script->vram_org, 0);
        set_0x30_unknown(script->vram_org, 0);
        set_0x32_unknown(script->vram_org, 0); // wide
        set_0x34_unknown(script->vram_org, 0);
    }
    
    if (contextsize > 0x34)
    {
        set_0x36_unknown(script->vram_org, 0);
        set_0x38_unknown(script->vram_org, 0);
        set_0x3a_wait_cycles(script->vram_org, 0);
        set_0x3e_wait_time(script->vram_org, 0);
    }
    
    //set_0x2d_calign(script->vram_org, script->header.unknown02);
    
    script->pc = script->pc_org = get_0x08_script_ret_offset(script->vram_org);
    
    s16 nextent = xswap16(alis.atent_ptr[caller_idx].offset);
    alis.atent_ptr[caller_idx].offset = xswap16(alis.dernent);
    alis.dernent = xswap16(alis.atent_ptr[script_idx].offset);
    alis.atent_ptr[script_idx].offset = xswap16(nextent);
    alis.atent_ptr[script_idx].vram_offset = xswap32(script->vram_org);
    alis.live_scripts[script_idx] = script;
    alis.finent += vram_length;
    alis.nbent ++;

    ALIS_DEBUG(EDebugVerbose, " (NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", script->name, script->vram_org, alis.finent, script->vacc_off, script->pc_org);
    return script;
}


/*
 PACKED SCRIPT FILE FORMAT
 byte(s)    len     role
 -------------------------------------------------------------------------------
 0          1       if high nibble is A, file is packed (b[0] & 0xf0 == 0xa0)
 1..3       3       unpacked size
 4...5      2       if zero, this is the main script
 
 (main only)
 6...21     16      main header bytes
 22...29    8       unpack dictionary
 30...xx    ?       packed data
 
 (other)
 6...13     8       unpack dictionary
 14...xx    ?       packed data

 
 UNPACKED SCRIPT FILE FORMAT
 byte(s)    len     role
 -------------------------------------------------------------------------------
 0...23     24      header bytes
 24...xx    ?       unpacked data (1st word is ID, must be zero)
 */
sAlisScriptData * script_load(const char * script_path) {

    sAlisScriptData * script = NULL;

    s32 unpack_sz = -1;
#if defined(ALIS_PROFILE_DRAW)
    // Per-script load profiling: unpack_script vs script_init (sprite conversion) + growprog volume.
    extern u32 sys_profile_ticks_safe(void);   // 200 Hz, 5 ms/tick
    extern u32 g_lp_grow_bytes;
    extern u32 g_lp_c2p_ticks, g_lp_grow_ticks, g_lp_fixup_ticks;
    extern void dbglog(const char *fmt, ...);
    const char *_lp_name = strrchr(script_path, kPathSeparator);
    _lp_name = _lp_name ? _lp_name + 1 : script_path;
    u32 _lp_unpack = 0, _lp_init = 0, _lp_t = 0;
    g_lp_grow_bytes = 0;
    g_lp_conv_ticks = 0;
    g_lp_c2p_ticks = 0; g_lp_grow_ticks = 0; g_lp_fixup_ticks = 0;
#endif

    FILE * fp = fopen(script_path, "rb");
    if (fp) {
        ALIS_DEBUG(EDebugInfo, "\nLoading script file: %s\n", script_path);
        
        // get packed file size
        fseek(fp, 0L, SEEK_END);
        u32 input_sz = (u32)ftell(fp);
        rewind(fp);

        // read header
        u32 magic = fread32(fp);
        u16 check = fread16(fp);
        
        // alloc and unpack
        unpack_sz = get_unpacked_size(magic);
        u8 * unpack_buf = (u8 *)malloc(1024 + unpack_sz * sizeof(u8));
        memset(unpack_buf, 0, unpack_sz);

        // TODO: check if this was already loaded, if so use cache
        
        // decrunch if needed
        alis.typepack = magic >> 24;
        if (alis.platform.kind == EPlatformPC) {
            alis.typepack &= 0xfe;
        }
        
        if (alis.typepack < 0)
        {
            ALIS_DEBUG(EDebugInfo, "Unpacking...\n");

            u32 pak_sz = input_sz - kPackedHeaderSize - kPackedDictionarySize;
            if(is_main(check)) {
                ALIS_DEBUG(EDebugInfo, "Main script detected.\n");
                
                // skip vm specs
                fseek(fp, kVMSpecsSize, SEEK_CUR);
                pak_sz -= kVMSpecsSize;
            }

#if defined(ALIS_PROFILE_DRAW)
            _lp_t = sys_profile_ticks_safe();
#endif
            unpack_sz = unpack_script(script_path, unpack_buf);
#if defined(ALIS_PROFILE_DRAW)
            _lp_unpack = sys_profile_ticks_safe() - _lp_t;
#endif
            if (unpack_sz > 0) {
#if defined(ALIS_PROFILE_DRAW)
                _lp_t = sys_profile_ticks_safe();
#endif
                script = script_init(strrchr(script_path, kPathSeparator) + 1, unpack_buf, unpack_sz);
#if defined(ALIS_PROFILE_DRAW)
                _lp_init = sys_profile_ticks_safe() - _lp_t;
#endif
            }
        }
        else {

            ALIS_DEBUG(EDebugInfo, "Loading...\n");

            // not packed, still might be script

            s32 seekto = kPackedHeaderSize;
            if(is_main(check))
                seekto += kVMSpecsSize;

            unpack_sz -= seekto;

            if (unpack_sz <= 0) {
                ALIS_DEBUG(EDebugError, "Skipping '%s': file too small (only %d bytes after header)\n",
                           script_path, (s32)input_sz - seekto);
            }
            else {
                fseek(fp, seekto, SEEK_SET);
                fread(unpack_buf, sizeof(u8), unpack_sz, fp);

#if defined(ALIS_PROFILE_DRAW)
                _lp_t = sys_profile_ticks_safe();
#endif
                script = script_init(strrchr(script_path, kPathSeparator) + 1, unpack_buf, unpack_sz);
#if defined(ALIS_PROFILE_DRAW)
                _lp_init = sys_profile_ticks_safe() - _lp_t;
#endif
            }
        }

        // cleanup
        free(unpack_buf);
        fclose(fp);
#if defined(ALIS_PROFILE_DRAW)
        // conv split: c2p (chunky->planar) | grow (growprog memmove) | fix (O(n^2) offset table)
        // | prep (per-sprite malloc + nibble-unpack pass + free = conv - the other three).
        u32 _lp_prep = g_lp_conv_ticks;
        if (_lp_prep > g_lp_c2p_ticks + g_lp_grow_ticks + g_lp_fixup_ticks)
            _lp_prep -= g_lp_c2p_ticks + g_lp_grow_ticks + g_lp_fixup_ticks;
        else _lp_prep = 0;
        dbglog("LOAD %-13s unpack=%lums init=%lums conv=%lums [c2p=%lu grow=%lu fix=%lu prep=%lu] shift=%luKB\n",
               _lp_name,
               (unsigned long)(_lp_unpack * 5), (unsigned long)(_lp_init * 5),
               (unsigned long)(g_lp_conv_ticks * 5),
               (unsigned long)(g_lp_c2p_ticks * 5), (unsigned long)(g_lp_grow_ticks * 5),
               (unsigned long)(g_lp_fixup_ticks * 5), (unsigned long)(_lp_prep * 5),
               (unsigned long)(g_lp_grow_bytes >> 10));
#endif
        
        if (unpack_sz < 0) {
            ALIS_DEBUG(EDebugFatal, "Failed to unpack script at path '%s'\n", script_path);
            exit(-1);
        }
    }
    else {
        ALIS_DEBUG(EDebugFatal, "Failed to open script at path '%s'\n", script_path);
    }
    
    if (alis.platform.uid == EGameMadShow && alis.platform.kind == EPlatformAtari)
    {
        if (script->header.id == 0)
        {
            // HACK: patch main script of Mad Show to work from hdd
            // NOTE: game was distributed on two single-sided 3.5" disks and check disk by loading disk.fic containig number of diskette

            char *locations[2] = { NULL, NULL };
            if ((locations[0] = strarr((char *)alis.mem + script->data_org, "disk.fic", unpack_sz)))
            {
                if ((locations[1] = strarr(locations[0] + 8, "disk.fic", unpack_sz)))
                {
                    for (u8 i = 1; i < 3; i++)
                    {
                        char name[16] = { 0 };
                        sprintf(name, "dsk%d.fic", i);
                        
                        strcpy(locations[i - 1], name);

                        char path[kPathMaxLen] = {0};
                        strcpy(path, alis.platform.path);
                        strcat(path, name);

                        if (sys_fexists(path) == false)
                        {
                            FILE *fp = fopen(path, "wb");
                            if (fp)
                            {
                                fputc(0, fp);
                                fputc(i, fp);
                                fclose(fp);
                            }
                            else
                            {
                                ALIS_DEBUG(EDebugFatal, "Failed to create '%s'\n", path);
                            }
                        }
                    }
                }
            }
        }
    }
        
    return script;
}


void script_unload(sAlisScriptData * script) {
}

bool is_delay_script(char *name) {
    
    if (alis.platform.uid == EGameIshar_1 && strstr(name, "auteur."))
    {
        alis.unload_delay = 5000;
        return true;
    }
    else if (alis.platform.uid == EGameLeFeticheMaya)
    {
        if (strstr(name, "abomaya."))
        {
            alis.load_delay = 1000;
        }
        else if (strstr(name, "presente."))
        {
            alis.load_delay = alis.platform.kind == EPlatformPC ? 2500 : 30000;
        }
        else if (strstr(name, "sdivers."))
        {
            alis.load_delay = 2500;
        }

        return true;
    }
    else if (alis.platform.uid == EGameTarghan0 || alis.platform.uid == EGameTarghan1)
    {
        if (strstr(name, "gen."))
        {
            alis.load_delay = alis.platform.kind == EPlatformPC || alis.platform.kind == EPlatformMac ? 2500 : 60000;
        }

        return true;
    }

    return false;
}

// =============================================================================
// MARK: - Script data access
// =============================================================================

// Release builds use the static-inline versions in alis.h; these logging
// variants are debug-only (see script.h).
#ifndef NDEBUG
u8 script_read8(void) {
    u8 ret = (VMEM[VSCRIPT->pc++]);
    ALIS_DEBUG(EDebugInfo, " 0x%02x", ret & 0xff);
    return ret;
}

/**
 * @brief Reads a word from current script
 *
 * @return u16
 */
u16 script_read16(void) {

    u32 val = read16(VMEM + VSCRIPT->pc);
    VSCRIPT->pc += 2;
    ALIS_DEBUG(EDebugInfo, " 0x%04x", val & 0xffff);
    return val;
}

u32 script_read24(void) {
    u32 val = read24(VMEM + VSCRIPT->pc);
    VSCRIPT->pc += 3;
    ALIS_DEBUG(EDebugInfo, " 0x%06x", val & 0xffffff);
    return val;
}

u32 script_read32(void) {

    u32 val = read32(VMEM + VSCRIPT->pc);
    VSCRIPT->pc += 4;
    ALIS_DEBUG(EDebugInfo, " 0x%x", val);
    return val;
}
#endif

void script_read_bytes(u32 len, u8 * dest) {
    while(len--) *dest++ = VMEM[VSCRIPT->pc++];
}

void script_read_until_zero(char * dest) {
    while((*dest++ = VMEM[VSCRIPT->pc++]));
}
