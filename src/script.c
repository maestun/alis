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
#include "patches.h"
#include "platform.h"
#include "script.h"
#include "unpack.h"
#include "utils.h"

#define kDeusBaseShift  (0xed210 - 0x24100)

// =============================================================================
// MARK: - Unpacker
// =============================================================================

// byte 0 -> magic
// byte 1..3 -> unpacked size (24 bits)
// byte 4..5 -> main script if zero
// if main: byte 6..21 -> main header
// if main: byte 22..29 -> dic
// if not main: byte 6..13 -> dic

u8 is_packedx(u32 magic)
{
    return ((magic >> 24) & 0xf0) == 0xa0;
}

u32 get_unpacked_size(u32 magic)
{
    return (magic & 0x00ffffff);
}

int is_main(u16 check)
{
    return kMainScriptID == check;
}

// =============================================================================
// MARK: - Script API
// =============================================================================

int search_insert(u32 size, int target_id)
{
    int start = 0;
    int end = size - 1;
        
    while(start <= end)
    {
        int mid = start + (end - start) / 2;
        int current_id = read16(alis.mem + ATPROG(mid));
        if (current_id == target_id)
            return mid;
        
        if (current_id < target_id)
        {
            start = mid + 1;
        }
        else
        {
            end = mid - 1;
        }
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
    
    // Ishar 1 ST plays MV1
    // samples stay as stored, 0xFF marks loop end / release end
    u8 mv1st = (type == 2 && alis.platform.kind == EPlatformAtari && alis.platform.uid == EGameIshar_1);
    if (type == 2 && !mv1st)
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
        // instruments are stored at 1/3 level
        if (sample[0] == 2)
        {
            static u8 mac3[256];
            if (mac3[0] == 0)
            {
                for (int i = 0; i < 256; i++)
                {
                    s32 v = (s8)i * 3;
                    v = v < -128 ? -128 : v > 127 ? 127 : v;
                    mac3[i] = v ? (u8)v : 1;
                }
            }

            for (int i = 0; i < length; i++)
                smpdata[i] = mac3[smpdata[i]];
        }
    }
    else if (alis.platform.kind == EPlatformAtari)
    {
        for (int i = 0; i < length; i++)
        {
            // MV1 marker -> end of sample
            if (mv1st && smpdata[i] == 0xff)
            {
                smpdata[i] = 0;
                continue;
            }

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

// main header fields are read before the platform readers are set up
static u32 hdr32le(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }

void script_guess_game(const char * script_path)
{
    alis.platform.uid = 0;
    alis.platform.ega = 0;
    FILE * fp = sys_fopen(script_path, "rb");
    if (fp)
    {
        ALIS_DEBUG(EDebugInfo, "\nLoading script file: %s\n", script_path);
        
        // get packed file size
        fseek(fp, 0L, SEEK_END);
        u32 input_sz = (u32)ftell(fp);
        rewind(fp);
        
        if (input_sz > 0x24)
        {
            // Try v6.0+ format detection first:
            // v6.0 scripts have an 8-byte name field at offset 0x08,
            // and the main script has "main" there.
            u8 header_buf[0x24];
            fread(header_buf, 1, 0x24, fp);

            u8 is_v60 = (memcmp(header_buf + 8, "main", 4) == 0 && header_buf[12] == 0);
            rewind(fp);

            // read header (old format)
            u32 magic = fread32(fp);
            u16 check = fread16(fp);

            if(is_v60)
            {
                // v6.0+: version byte at +0x06, the same in all of a game's scripts
                u32 specs = hdr32le(header_buf + 0x10);
                switch (header_buf[6])
                {
                    case 0x3c: alis.platform.uid = alis.platform.kind == EPlatform3DO ? EGameRobinsonsRequiem3DO
                                   : hdr32le(header_buf + 0x20) == 0 ? EGameManualRRQ : EGameDeus; break;
                    case 0x48:
                    case 0x4a: alis.platform.uid = EGameTimeWarriors; break;
                    case 0x5f: alis.platform.uid = EGameAsghan; break;
                    case 0x61: alis.platform.uid = EGameTournamentOfWarriors; break;
                    case 0x71: alis.platform.uid = specs == 700 ? EGameLesVisiteurs : EGameArabianNights; break;
                    case 0xe0: alis.platform.uid = EGameInspectorGadget; break;
                    default:   alis.platform.uid = EGameUnknown; break;
                }
            }
            else if(is_main(check))
            {
                alis.header.val0 = fread16(fp);
                alis.header.val1 = fread16(fp);
                alis.header.val2 = fread32(fp);
                alis.header.val3 = fread32(fp);
                alis.header.val4 = fread32(fp);

                alis.platform.uid = alis.header.val3 * alis.header.val4;
            }

            if(alis.platform.uid != 0)
            {
                
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
                                                     // EGA PC re-release (dictionary packer 0xa1)
                                                     alis.platform.ega = alis.platform.kind == EPlatformPC && (magic >> 24) == 0xa1;
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
                    case EGameRobinsonsRequiem3DO:   strcpy(alis.platform.name, "Robinson's Requiem");
                                                     alis.platform.version = 60;  // 6.0 [0x3c]
                                                     alis.basemem = 0x22400;
                                                     alis.platform.width = 320;
                                                     alis.platform.height = 200;
                                                     break;
                    //#######################################################################################
                    case EGameDeus:                  strcpy(alis.platform.name, "Deus");
                                                     alis.platform.version = 60;  // 6.0 [0x3c]
                                                     // TODO: fix it
                                                     // MAIN vram at 0xed210 as in DOS: DOS save
                                                     // loads keep MAIN-internal pointers absolute
                                                     alis.basemem = 0x22400 + kDeusBaseShift;
                                                     // Deus is a 16MB-era DOS game, default 4MB PC pool is far too small, using 16MB pool
                                                     alis.platform.ram_sz = 0x1000000 + kDeusBaseShift;
                                                     break;
                    //#######################################################################################
                    case EGameTimeWarriors:          strcpy(alis.platform.name, "Time Warriors");
                                                     alis.platform.version = header_buf[6];  // 7.2 [0x48], 7.4 [0x4a]
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
                if (!announced)
                {
                    announced = 1;
                    printf("Starting %s %s (ALIS ver. %.1f)\n", alis.platform.name, alis.platform.desc, alis.platform.version / 10.0);
                    printf("         Host:  %s,  Platform:  %s,  Artificial version: %d\n", is_host_le() ? "LE" : "BE", alis.platform.is_little_endian ? "LE" : "BE", alis.platform.version);
                    if (!is_v60)
                    {
                        printf("         Platform UID: Specs+0x08 x Specs+0x0c = Vmaxvram x Vmaxsprite\n");
                        printf("                   =>  0x%08x x 0x%08x = 0x%08x\n", alis.header.val3, alis.header.val4, alis.platform.uid);
                    }

                    fflush(stdout);
                }
            }
            else
            {
                ALIS_DEBUG(EDebugError, "The version of the %s file is not supported, or it is not a valid main file of the game.\n", script_path);
            }
        }
        else
        {
            ALIS_DEBUG(EDebugError, "The size of the main file %s is %d bytes, and it is smaller than required.\n", script_path, input_sz);
        }

        // cleanup
        fclose(fp);
    }
    else
    {
        ALIS_DEBUG(EDebugError, "Could not open main file %s.\n", script_path);
    }
}

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
    
    for (s16 i = 0; i < count; i++)
    {
        u16 off = read16(tab + i * 2);
        u8 *smp = tab + off;
        if (smp[0] != 1)
            continue;

        s16 j = 0;
        while (j < i && (u16)read16(tab + j * 2) != off)
            j++;
        
        if (j < i)
            continue;

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
    
    for (s16 i = 0; i < count; i++)
    {
        u16 off = read16(tab + i * 2);
        u8 *bmp = tab + (s16)off;
        if (bmp[0] != 0)
            continue;

        s16 j = 0;
        while (j < i && (u16)read16(tab + j * 2) != off)
            j++;
        
        if (j < i)
            continue;

        tochunky(bmp);
    }
}

// v6 memory nodes
// FIFO free list linked through node +0x10, type at +0x15. Returns 0 when exhausted.
// Non-entity nodes go to the front of the block list (head NODE_BLKHEAD, next at +0xe,
// name at +6, DOS block+8). Entity nodes keep the vram block layout at +6 / +8.
u16 node_take(u8 type)
{
    u16 n = alis.nodefree;
    u16 nx = xread16(alis.atent + n + 0x10);
    
    // NOTE: DOS refuses the last two free nodes; 3DO has no check
    if (n == 0 || n == alis.nodetail || (alis.platform.kind != EPlatform3DO && nx == alis.nodetail))
    {
        ALIS_DEBUG(EDebugError, "node_take: no free memory node\n");
        return 0;
    }

    alis.nodefree = nx;
    xwrite8(alis.atent + n + 0x15, type);
    if (type != 1)
    {
        for (int i = 0; i < 8; i++)
            xwrite8(alis.atent + n + 6 + i, 0);
        
        xwrite16(alis.atent + n + 0xe, xread16(alis.atent + NODE_BLKHEAD + 0xe));
        xwrite16(alis.atent + NODE_BLKHEAD + 0xe, n);
    }

    return n;
}

void node_free(u16 n)
{
    if (n == 0)
        return;
    if (xread8(alis.atent + n + 0x15) != 1)
    {
        // unlink from the block list
        u16 p = NODE_BLKHEAD;
        while (xread16(alis.atent + p + 0xe) != n && p != NODE_BLKTAIL)
            p = xread16(alis.atent + p + 0xe);
        
        if (p != NODE_BLKTAIL)
            xwrite16(alis.atent + p + 0xe, xread16(alis.atent + n + 0xe));
    }

    xwrite8(alis.atent + n + 0x15, 0);
    xwrite16(alis.atent + alis.nodetail + 0x10, n);
    alis.nodetail = n;
}

void node_setname(u16 n, const char *name)
{
    for (int i = 0, end = 0; i < 8; i++)
    {
        if (!name[i] || name[i] == '.')
            end = 1;
        
        xwrite8(alis.atent + n + 6 + i, end ? 0 : (u8)name[i]);
    }
}

u16 node_find(const char *name)
{
    u8 key[8] = {0};
    for (int i = 0; i < 8 && name[i]; i++)
        key[i] = (u8)name[i];
    
    for (u16 n = xread16(alis.atent + NODE_BLKHEAD + 0xe); n != NODE_BLKTAIL; n = xread16(alis.atent + n + 0xe))
    {
        u8 t = xread8(alis.atent + n + 0x15);
        if (t != 2 && t != 0xc && (t != 0xd || alis.platform.kind == EPlatform3DO))
            continue;
        
        if (memcmp(alis.mem + alis.atent + n + 6, key, 8) == 0)
            return n;
    }

    return 0;
}

void node_swap(u16 a, u16 b)
{
    if (a == b)
        return;
    
    u8 t[0x18];
    memcpy(t, alis.mem + alis.atent + a, 0x18);
    memcpy(alis.mem + alis.atent + a, alis.mem + alis.atent + b, 0x18);
    memcpy(alis.mem + alis.atent + b, t, 0x18);
#define NSW(x) ((x) == a ? b : (x) == b ? a : (x))
    for (int i = 0; i < alis.maxent; i++)
    {
        u32 n = alis.atent + i * 0x18;
        xwrite16(n + 4, NSW(xread16(n + 4)));          // run chain
        xwrite16(n + 0xe, NSW(xread16(n + 0xe)));      // block list
        xwrite16(n + 0x10, NSW(xread16(n + 0x10)));    // free list
    }

    alis.nodefree = NSW(alis.nodefree);
    alis.nodetail = NSW(alis.nodetail);
    alis.filmnode = NSW(alis.filmnode);
    for (int i = 0; i < 16; i++)
    {
        alis.scrnode[i].node = NSW(alis.scrnode[i].node);
    }
    
    for (int i = 0; i < 8; i++)
    {
        alis.v6texts[i].node = NSW(alis.v6texts[i].node);
    }
    
    for (int i = 0; i < alis.nbprog; i++)
    {
        if (alis.loaded_scripts[i])
            alis.loaded_scripts[i]->node = NSW(alis.loaded_scripts[i]->node);
    }
    
#undef NSW
    sAlisScriptLive *sa = alis.live_scripts[a / 0x18];
    alis.live_scripts[a / 0x18] = alis.live_scripts[b / 0x18];
    alis.live_scripts[b / 0x18] = sa;
    for (u16 i = 0; i < 2; i++)
    {
        u16 id = i ? b : a;
        sAlisScriptLive *e = alis.live_scripts[id / 0x18];
        if (e && xread8(alis.atent + id + 0x15) == 1)
        {
            if (xread16(V6_CLINKING(e->vram_org)) == xread16(e->vram_org + 0x3a))
                xwrite16(V6_CLINKING(e->vram_org), id);
            
            xwrite16(e->vram_org + 0x3a, id);
            set_0x0e_script_ent(e->vram_org, id);
        }
    }
}

// node of a screen buffer field (+0xb4 / +0xbc)
void scrnode_set(u32 scr, u16 field, u16 node)
{
    int slot = -1;
    for (int i = 0; i < 16; i++)
    {
        if (alis.scrnode[i].scr == (u16)(scr - alis.basemain) && alis.scrnode[i].field == field)
        {
            slot = i;
            break;
        }
        
        if (slot < 0 && alis.scrnode[i].node == 0)
        {
            slot = i;
        }
    }

    if (slot < 0)
        return;
    
    alis.scrnode[slot].scr = (u16)(scr - alis.basemain);
    alis.scrnode[slot].field = field;
    alis.scrnode[slot].node = node;
    alis.scrnode[slot].handle = xread32(scr + field);
}

// 0 if the field no longer holds the buffer the node was taken for
u16 scrnode_get(u32 scr, u16 field)
{
    for (int i = 0; i < 16; i++)
    {
        if (alis.scrnode[i].node && alis.scrnode[i].scr == (u16)(scr - alis.basemain) && alis.scrnode[i].field == field)
            return alis.scrnode[i].handle == xread32(scr + field) ? alis.scrnode[i].node : 0;
    }
    
    return 0;
}

sAlisScriptData *node_prog(u16 n)
{
    for (int i = 0; n && i < alis.nbprog; i++)
    {
        if (alis.loaded_scripts[i] && alis.loaded_scripts[i]->node == n)
            return alis.loaded_scripts[i];
    }
    
    return NULL;
}

sAlisScriptData * script_init(const char * name, u8 * data, u32 data_sz)
{
    // v6 low word of the u32 size (on BE the high word would be 0 for most scripts)
    s16 id = alis.platform.version >= 60 ? (s16)swap32(data + 0) : swap16(data + 0);
    s16 insert = debprotf(id);
    
    // v6 checks by name before loading (node_find); its ids may collide
    if (alis.platform.version < 60 && insert > 0 && insert < alis.nbprog)
    {
        sAlisScriptData *script = alis.loaded_scripts[insert];
        if (script->header.id == id)
        {
            for (int i = 0; i < alis.nbprog; i++)
            {
                ALIS_DEBUG(EDebugVerbose, "\n%c%s ID %.2x AT %.6x", i == insert ? '*' : ' ', alis.loaded_scripts[i]->name, read16(alis.mem + ATPROG(i)), ATPROG(i));
            }
            
            ALIS_DEBUG(EDebugVerbose, "\n");

            ALIS_DEBUG(EDebugVerbose, " (NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", alis.script->name, alis.script->vram_org, alis.finent, alis.script->vacc_off, alis.script->pc_org);

            ALIS_DEBUG(EDebugVerbose, "Initialized script '%s' (ID = 0x%02x)\nDATA at address 0x%x - 0x%x\n", script->name, script->header.id, script->data_org, alis.finprog);
            return script;
        }
    }
    
    if (alis.platform.version < 60)
        script_fix(name, data, data_sz);

    // init script
    sAlisScriptData * script = (sAlisScriptData *)malloc(sizeof(sAlisScriptData));
    memset(script, 0, sizeof(sAlisScriptData));
    strcpy(script->name, name);
    script->sz = data_sz;
    script->type = alis.typepack;
    
    // script data
    if (alis.platform.version >= 60)
    {
        // v6.0+ header layout (0x38 bytes):
        // 0x00: u32 total_size
        // 0x04: u32 unknown (contains version info)
        // 0x08: 8 bytes script name (null-terminated)
        // 0x10: u32 inisprit
        // 0x14: u32 unknown
        // 0x18: u32 code_loc/ret_offset
        // 0x1c: u32 flag1 (scan/inter bit 0)
        // 0x20: u32 flag2 (scan/inter bit 1)
        // 0x24: u32 flag3 / ret_offset for sub-script
        // 0x28: u32 unknown
        // 0x2c: u32 unknown
        // 0x30: u32 vram_alloc_sz (32-bit!)
        // 0x34: u16 w_unknown5
        // 0x36: u16 w_unknown7

        // Use first u16 as synthetic ID for script table compatibility
        script->header.id = id;
        script->header.unknown01 = *(data + 4);
        script->header.unknown02 = *(data + 5);
        script->header.code_loc_offset = (u16)swap32(data + 0x18);
        script->header.ret_offset = swap32((data + 0x18));
        script->header.dw_unknown3 = swap32((data + 0x1c));
        script->header.dw_unknown4 = swap32((data + 0x20));
        script->header.w_unknown5 = swap16((data + 0x34));
        script->header.vram_alloc_sz = (u16)swap32(data + 0x30);
        script->header.w_unknown7 = swap16((data + 0x36));
    }
    else
    {
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
    }

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

    if (alis.mem + script->data_org != data)
        memcpy(alis.mem + script->data_org, data, data_sz);

    // get insert point
    insert = search_insert(alis.nbprog, script->header.id);

    // v1.0 keeps scripts word aligned
    alis.finprog += alis.platform.version == 10 ? (data_sz + 1) & ~1 : data_sz;
    alis.dernprog += 4;
    alis.nbprog ++;

    for (int i = alis.nbprog - 2; i >= insert; i--)
    {
        ATPROG_SET(i + 1, ATPROG(i));
        alis.loaded_scripts[i + 1] = alis.loaded_scripts[i];
    }

    ATPROG_SET(insert, script->data_org);
    alis.loaded_scripts[insert] = script;
    if (alis.platform.version >= 60)
    {
        script->node = node_take(2);                        // DOS 0000dd37: program node
        node_setname(script->node, (const char *)alis.mem + script->data_org + 8);
    }

    for (int i = 0; i < alis.nbprog; i++)
    {
        ALIS_DEBUG(EDebugInfo, "\n%c%s ID %.2x AT %.6x", i == insert ? '*' : ' ', alis.loaded_scripts[i]->name, read16(alis.mem + ATPROG(i)), ATPROG(i));
    }
    
    ALIS_DEBUG(EDebugInfo, "\n");
    ALIS_DEBUG(EDebugInfo, "Initialized script '%s' (ID = 0x%02x)\nDATA at address 0x%x - 0x%x\n", script->name, script->header.id, script->data_org, alis.finprog);
    
    if (alis.platform.version >= 60)
    {
        // v6 samples (type 9) are signed 8-bit
        // the mixer takes unsigned
        data = alis.mem + script->data_org;
        u32 n = read16(data + 0x3e);
        for (u32 i = 0, slot = 0x40; i < n && slot + 4 <= data_sz; i++, slot += 4)
        {
            u32 off = read32(data + slot), at = slot + off;
            if (!off || at + 0x20 > data_sz || data[at] != 9)
                continue;
            
            u32 len = read32(data + at + 0x10);
            u8 *smp = data + at + 0x20;
            for (u32 k = 0; k < len && at + 0x20 + k < data_sz; k++)
                smp[k] ^= 0x80;
        }
    }

    if (alis.platform.version == 10)
    {
        if (alis.platform.kind != EPlatformPC)
            all_tochunky(alis.mem + script->data_org);
        
        if (alis.platform.kind == EPlatformOldAtari)
            all_tosigned(alis.mem + script->data_org);
        
        return script;
    }
    
    if (alis.platform.version >= 60)
        return script;

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

                    u8 *ptr = bitmap + 6;
                    for (s32 h = 0; h < width * height; h++)
                        ptr[h] |= (u8)(ptr[h] << 4);
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

                    u8 *ptr = bitmap + 6;
                    for (s32 h = 0; h < width * height; h++)
                        ptr[h] |= (u8)(ptr[h] << 4);
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
                                if (hi != 0)
                                    hi += pal_offset;
                                
                                if (lo != 0)
                                    lo += pal_offset;
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
            script->sz += convert_sprites_inplace(script->data_org, script->name);
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
            if (at < 0 || (u32)at >= script->sz)
                continue;
            
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
            if (at < 0 || (u32)at >= script->sz)
                continue;
            
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
            return 0x3e;
            break;
        case EGameRobinsonsRequiem3DO:
        case EGameDeus:
            return 0x80;
            break;
        default:
            return -1;
    }
}

sAlisScriptLive *script_live(sAlisScriptData * prog)
{
    // Deus entity room comes from the shared arena, 3DO is using fixed pool, perror 0x23 when full
    if (alis.platform.uid == EGameDeus || alis.platform.kind == EPlatform3DO)
    {
        u8 *pd = alis.mem + prog->data_org;
        u32 need = swap16(pd + 0x34) + swap16(pd + 0x36) + get_context_size() + (u16)swap32(pd + 0x30);
        if (alis.platform.kind == EPlatform3DO ? alis.finent + need > v6_enttop() : !v6_growent(need))
        {
            ALIS_DEBUG(EDebugError, "Out of VM memory creating '%s' entity\n", prog->name);
            alis_fatal = "Out of game memory: this machine has too little RAM for this game.";
            return NULL;
        }
    }

    u16 newent = alis.platform.version >= 60 ? node_take(1) : (u16)alis.dernent;
    if (newent == 0 && alis.platform.version >= 60)
        return NULL;

    sAlisScriptLive *script = (sAlisScriptLive *)malloc(sizeof(sAlisScriptLive));
    memset(script, 0, sizeof(sAlisScriptLive));
    
    script->name        = prog->name;
    script->data        = prog;

    u8 *data = alis.mem + script->data->data_org;

    u16 w_unknown5, w_unknown7, vram_length;
    if (alis.platform.version >= 60)
    {
        w_unknown5 = swap16((data + 0x34));
        w_unknown7 = swap16((data + 0x36));
        vram_length = (u16)swap32(data + 0x30);
    }
    else
    {
        w_unknown5 = swap16((data + 0x12));
        w_unknown7 = swap16((data + 0x16));
        vram_length = swap16((data + 0x14));
    }

    s32 prevfin = w_unknown5 + alis.finent;

    s32 contextsize = get_context_size();

    alis.finent += w_unknown7 + w_unknown5 + contextsize;

    script->vacc_off = (prevfin - alis.finent) & 0xffff;
    script->vram_org = alis.finent;

    memset(alis.mem + script->vram_org, 0, vram_length);

    s16 curent = alis.varD5;
    alis.varD7 = newent;
    
    int caller_idx = curent / ENT_STRIDE;
    int script_idx = newent / ENT_STRIDE;

    ALIS_DEBUG(EDebugVerbose, " add at idx: %d hooked to idx: %d. ", script_idx, caller_idx);

    set_0x0a_vacc_offset(script->vram_org, script->vacc_off);
    set_0x1c_scan_clr(script->vram_org, script->vacc_off);
    set_0x1e_scan_clr(script->vram_org, script->vacc_off);
    if (alis.platform.version >= 60)
    {
        set_0x08_script_ret_offset(script->vram_org, script->data->data_org + 0x18 + script->data->header.ret_offset);
    }
    else
    {
        set_0x08_script_ret_offset(script->vram_org, script->data->data_org + script->data->header.code_loc_offset + 2);
    }

    set_0x10_script_id(script->vram_org, script->data->header.id);
    set_0x14_script_org_offset(script->vram_org, script->data->data_org);
    set_0x2e_script_header_word_2(script->vram_org, script->data->header.unknown01);
    set_0x02_wait_cycles(script->vram_org, 1);
    set_0x01_wait_count(script->vram_org, 1);
    set_0x04_cstart_csleep(script->vram_org, 0xff);
    set_0x1a_cforme(script->vram_org, -1);
    set_0x0e_script_ent(script->vram_org, newent);
    set_0x24_scan_inter(script->vram_org, alis.platform.version >= 60 ? 4 : alis.platform.version >= 31 ? 6 : 2);
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

    if (contextsize > 0x3e)
    {
        xwrite16(script->vram_org - 0x40, 0);
    }

    if (alis.platform.version >= 60)
    {
        u32 head = v6_elemhead(script->vram_org);
        u32 tail = v6_elemtail(script->vram_org);
        v6el_set_next(head, tail);
        v6el_set_prev(head, head);
        v6el_set_prev(tail, head);
        v6el_set_next(tail, tail);
        xwrite16(script->vram_org - 0x64, 0);
        xwrite8(script->vram_org - 0x66, 0);
        xwrite8(V6_FARSLEEP(script->vram_org), 0);
        xwrite16(script->vram_org - 0x7c, 0);
        xwrite32(script->vram_org - 0x80, 0);

        {
            u16 qsize = (u16)xread16(script->data->data_org + 0x36);
            s16 qbase = -(s16)(0x40 + qsize);
            set_0x1c_scan_clr(script->vram_org, qbase);
            set_0x1e_scan_clr(script->vram_org, qbase);
            if (qsize > 0x20)
                ALIS_DEBUG(EDebugWarning, "v6 scan queue of %s is 0x%x (> 0x20): cells reach below -0x60 into the context extras!\n", script->name, qsize);
        }

        {
            u32 data = script->data->data_org;
            u8 vf = 0x4;
            if (xread32(data + 0x1c) != 0)
                vf |= 0x1;
            if (xread32(data + 0x20) != 0)
                vf |= 0x2;
            if (xread32(data + 0x24) != 0)
                vf |= 0x8;
            xwrite8(script->vram_org - 0x67, vf);
        }

        xwrite16(script->vram_org + 0x3a, newent);
        xwrite16(V6_CLINKING(script->vram_org), newent);
        xwrite32(script->vram_org + 0x3c, xread32(script->data->data_org + 0x8));
        xwrite32(script->vram_org + 0x40, xread32(script->data->data_org + 0xc));

        if (alis.script != NULL && alis.script->vram_org != 0 && alis.script->vram_org != script->vram_org)
        {
            u32 p = alis.script->vram_org;
            u32 c = script->vram_org;
            xwrite16(c + 0x38, xread16(p + 0x3a));
            xwrite32(c + 0x0c, xread32(p + 0x0c));
            xwrite32(c + 0x10, xread32(p + 0x10));
            xwrite32(c + 0x14, xread32(p + 0x14));
            xwrite16(c + 0x18, xread16(p + 0x18));
            xwrite16(c + 0x1a, xread16(p + 0x1a));
            xwrite16(c + 0x1c, xread16(p + 0x1c));
            xwrite16(c + 0x26, xread16(p + 0x26));
            xwrite16(c + 0x28, xread16(p + 0x28));
            xwrite16(c + 0x2a, xread16(p + 0x2a));
            xwrite8(c + 0x24, xread8(p + 0x24));
            xwrite16(c - 0x64, xread16(p - 0x64));
            set_0x16_screen_id(c, get_0x16_screen_id(p));
            xwrite8(c - 0x66, xread8(p - 0x66));
        }
    }
    
    //set_0x2d_calign(script->vram_org, script->header.unknown02);
    
    script->pc = script->pc_org = get_0x08_script_ret_offset(script->vram_org);

    // link right after the creator
    u16 nextent = xread16(alis.atent + curent + 4);
    xwrite16(alis.atent + curent + 4, newent);
    if (alis.platform.version < 60)
        alis.dernent = xread16(alis.atent + newent + 4);
    
    xwrite16(alis.atent + newent + 4, nextent);
    xwrite32(alis.atent + newent, script->vram_org);
    if (alis.platform.version >= 60)
    {
        // block layout for killent
        xwrite16(alis.atent + newent + 6, w_unknown5 + w_unknown7 + contextsize);
        xwrite32(alis.atent + newent + 8, w_unknown5 + w_unknown7 + contextsize + vram_length);
    }

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
sAlisScriptData * script_load(const char * script_path)
{
    sAlisScriptData * script = NULL;

    s32 unpack_sz = -1;

    FILE * fp = sys_fopen(script_path, "rb");
    if (fp)
    {
        ALIS_DEBUG(EDebugInfo, "\nLoading script file: %s\n", script_path);

        // get packed file size
        fseek(fp, 0L, SEEK_END);
        u32 input_sz = (u32)ftell(fp);
        rewind(fp);

        if (alis.platform.version >= 60)
        {
            unpack_sz = input_sz;
            // read straight into the arena where script_init puts it
            u8 *in_arena = alis.mem && alis.finprog <= alis.finmem && unpack_sz <= alis.finmem - alis.finprog ? alis.mem + alis.finprog : NULL;
            u8 * unpack_buf = in_arena ? in_arena : (u8 *)malloc(1024 + unpack_sz * sizeof(u8));
            if (!unpack_buf)
            {
                fclose(fp);
                alis_fatal = "Out of memory loading a script.";
                return NULL;
            }
            memset(unpack_buf, 0, unpack_sz);

            fread(unpack_buf, sizeof(u8), unpack_sz, fp);

            u8 is_main_script = (memcmp(unpack_buf + 8, "main", 4) == 0 && unpack_buf[12] == 0);
            if (is_main_script)
            {
                ALIS_DEBUG(EDebugInfo, "Main script detected (v6.0+).\n");
            }

            script = script_init(strrchr(script_path, kPathSeparator) + 1, unpack_buf, unpack_sz);

            if (!in_arena)
                free(unpack_buf);
            
            fclose(fp);
        }
        else
        {
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
            // v1.1 re-release (Mad Show) marks the dictionary packer with 0xa1
            if (alis.platform.kind == EPlatformPC && alis.platform.version != 11)
            {
                alis.typepack &= 0xfe;
            }

            if (alis.typepack < 0)
            {
                ALIS_DEBUG(EDebugInfo, "Unpacking...\n");

                u32 pak_sz = input_sz - kPackedHeaderSize - kPackedDictionarySize;
                if(is_main(check))
                {
                    ALIS_DEBUG(EDebugInfo, "Main script detected.\n");

                    // skip vm specs
                    fseek(fp, kVMSpecsSize, SEEK_CUR);
                    pak_sz -= kVMSpecsSize;
                }

                unpack_sz = unpack_script(script_path, unpack_buf);
                if (unpack_sz > 0)
                {
                    script = script_init(strrchr(script_path, kPathSeparator) + 1, unpack_buf, unpack_sz);
                }
            }
            else
            {
                ALIS_DEBUG(EDebugInfo, "Loading...\n");

                // not packed, still might be script

                s32 seekto = kPackedHeaderSize;
                if(is_main(check))
                    seekto += kVMSpecsSize;

                unpack_sz -= seekto;

                if (unpack_sz <= 0)
                {
                    ALIS_DEBUG(EDebugError, "Skipping '%s': file too small (only %d bytes after header)\n",
                               script_path, (s32)input_sz - seekto);
                }
                else
                {
                    fseek(fp, seekto, SEEK_SET);
                    fread(unpack_buf, sizeof(u8), unpack_sz, fp);

                    script = script_init(strrchr(script_path, kPathSeparator) + 1, unpack_buf, unpack_sz);
                }
            }

            // cleanup
            free(unpack_buf);
            fclose(fp);
            
            // damaged data already set alis_fatal: let the VM stop and show it
            if (unpack_sz < 0 && !alis_fatal)
            {
                ALIS_DEBUG(EDebugFatal, "Failed to unpack script at path '%s'\n", script_path);
                exit(-1);
            }
        }
    }
    else
    {
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
                            FILE *fp = sys_fopen(path, "wb");
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

void script_unload(sAlisScriptData * script)
{
}

bool is_delay_script(char *name)
{
    
    if (alis.platform.uid == EGameIshar_1 && stristr(name, "auteur."))
    {
        alis.unload_delay = 5000;
        return true;
    }
    else if (alis.platform.uid == EGameLeFeticheMaya)
    {
        if (stristr(name, "abomaya."))
        {
            alis.load_delay = 1000;
        }
        else if (stristr(name, "presente."))
        {
            alis.load_delay = alis.platform.kind == EPlatformPC ? 2500 : 30000;
        }
        else if (stristr(name, "sdivers."))
        {
            alis.load_delay = 2500;
        }
        else if (alis.platform.kind == EPlatformPC && stristr(name, "charge."))
        {
            alis.load_delay = 2500;
        }
        else if (alis.platform.kind == EPlatformAmiga && stristr(name, "debuvisi."))
        {
            alis.load_delay = 2500;
        }

        return true;
    }
    else if (alis.platform.uid == EGameTarghan0 || alis.platform.uid == EGameTarghan1)
    {
        if (stristr(name, "gen."))
        {
            alis.load_delay = alis.platform.kind == EPlatformPC || alis.platform.kind == EPlatformMac ? 2500 : 60000;
        }

        return true;
    }
    else
    {
        // EPlatformUnknown matches any platform
        static const struct { s32 uid; EPlatform kind; const char *name; } loadscr[] = {
            { EGameColorado, EPlatformPC, "neigepc." },
            { EGameStarblade, EPlatformPC, "loading." },
            { EGameStormMaster, EPlatformPC, "panneau." },
            { EGameMetalMutant, EPlatformUnknown, "apolo." },
            { EGameMetalMutant, EPlatformUnknown, "sono." },
            { EGameIshar_2, EPlatformUnknown, "present1." },
        };

        for (int i = 0; i < sizeof(loadscr) / sizeof(*loadscr); i++)
        {
            if (alis.platform.uid == loadscr[i].uid && (loadscr[i].kind == EPlatformUnknown || loadscr[i].kind == alis.platform.kind) && stristr(name, loadscr[i].name))
            {
                alis.load_delay = 2500;
                return true;
            }
        }
    }

    return false;
}

// =============================================================================
// MARK: - Script data access
// =============================================================================

// Release builds use the static-inline versions in alis.h; these logging
// variants are debug-only (see script.h).
#ifndef NDEBUG
u8 script_read8(void)
{
    u8 ret = (VMEM[VSCRIPT->pc++]);
    ALIS_DEBUG(EDebugInfo, " 0x%02x", ret & 0xff);
    return ret;
}

/**
 * @brief Reads a word from current script
 *
 * @return u16
 */
u16 script_read16(void)
{
    u32 val = read16(VMEM + VSCRIPT->pc);
    VSCRIPT->pc += 2;
    ALIS_DEBUG(EDebugInfo, " 0x%04x", val & 0xffff);
    return val;
}

u32 script_read24(void)
{
    u32 val = read24(VMEM + VSCRIPT->pc);
    VSCRIPT->pc += 3;
    ALIS_DEBUG(EDebugInfo, " 0x%06x", val & 0xffffff);
    return val;
}

u32 script_read32(void)
{

    u32 val = read32(VMEM + VSCRIPT->pc);
    VSCRIPT->pc += 4;
    ALIS_DEBUG(EDebugInfo, " 0x%x", val);
    return val;
}
#endif

void script_read_bytes(u32 len, u8 * dest)
{
    while(len--)
        *dest++ = VMEM[VSCRIPT->pc++];
}

void script_read_until_zero(char * dest)
{
    while((*dest++ = VMEM[VSCRIPT->pc++]));
}
