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
#include "video.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "deus3d.h"
#include "screen.h"
#include "unpack.h"
#include "utils.h"
#include <sys/stat.h>

// fileno is POSIX but may not be declared with strict C99
#if !defined(_WIN32)
extern int fileno(FILE *);
#endif

// ============================================================================
#pragma mark - Codesc1 routines
// ============================================================================

// Codesc1name no. 01 opcode 0x00 cnul
// Calls stub cnul

// Codesc1name no. 02 opcode 0x01 csoundon
// Known as consound in c-file
void csoundon(void)
{
    audio.fsound = 1;
}

// Codesc1name no. 03 opcode 0x02 csoundoff
// Known as coffsound in c-file
void csoundoff(void)
{
    audio.fsound = 0;
}

// Codesc1name no. 04 opcode 0x03 cmusicon
// Known as conmusic in c-file
void cmusicon(void)
{
    audio.fmusic = 1;
}

// Codesc1name no. 05 opcode 0x04 cmusicoff
// Known as coffmusic in c-file
void cmusicoff(void)
{
    audio.fmusic = 0;
}

// Codesc1name no. 06 opcode 0x05 cdelfilm
void cdelfilm(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        return;                     // a stub on 3DO (crunfilm already closed the stream)

    endfilm();

    fli_stream_close();   // close the streaming file handle (no-op in whole-file mode)

    if (bfilm.delptr != NULL)
        free(bfilm.delptr);

    memset(&bfilm, 0, sizeof(bfilm));
    if (alis.platform.version >= 60)
    {
        node_free(alis.filmnode);
        alis.filmnode = 0;
    }
}

// Codesc1name no. 07 opcode 0x06 copenfilm
void copenfilm(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        // 3DO only names the stream here; crunfilm plays it. Stores 0.
        char path[kPathMaxLen] = {0};
        readexec_opername();
        readexec_opername_swap();
        if (alis.sd7[0] != '/')
            strcpy(path, alis.platform.path);
        
        strcat(path, alis.sd7);
        film3do_open(path);
        readexec_opername();
        readexec_opername();
        alis.varD7 = 0;
        cstore_continue();
        return;
    }

    fli_stream_close();   // stale streaming film (script skipped cdelfilm) — don't leak the handle

    memset(&bfilm, 0, sizeof(bfilm));
    if (alis.platform.version >= 60)
    {
        node_free(alis.filmnode);                   // close the open film first
        alis.filmnode = 0;
    }

    readexec_opername();
    bfilm.id = alis.varD7;

    readexec_opername_swap();
    
    alis.fp = NULL;

    // The one CD carries both "150." (smaller, Falcon-sized) and "300." (fullscreen) video
    // variants; this remaps 150 -> 300 to force the fullscreen version. Native Atari (Falcon)
    // should use its own 150 videos: they play smoothly on the 16-bit fused film path and are
    // positioned correctly (fli_decomp16 honours fenx1/feny1). Keep the remap only off-target.
#if !defined(ALIS_USE_NATIVE_ATARI)
    char *at = strstr(alis.sd7, "150.");
    if (at != NULL)
    {
        memcpy(at, "300.", 4);
    }
#endif

    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    strcat(path, alis.sd7);

    afopen((char *)path, 1);

#if defined(ALIS_USE_NATIVE_ATARI)
    // Native Falcon prefers its own smaller "150." speech videos (they play on the
    // 16-bit fused film path and are positioned correctly), so the remap above is
    // skipped on-target. But not every data set ships the 150 variants — when the
    // 150 file is absent, afopen fails, addr stays NULL and the whole speech film
    // (and its audio) is silently skipped. Fall back to the "300." fullscreen
    // variant, the one the desktop build always opens successfully.
    if (alis.fp == NULL)
    {
        char *at = strstr(alis.sd7, "150.");
        if (at != NULL)
        {
            memcpy(at, "300.", 4);
            strcpy(path, alis.platform.path);
            strcat(path, alis.sd7);
            afopen((char *)path, 1);
        }
    }
#endif

    u8 *addr = NULL;

    if (alis.fp)
    {
        struct stat st;
        fstat(fileno(alis.fp), &st);
        off_t size = st.st_size;

        addr = malloc(size);
        if (addr)
        {
            fread(addr, size, 1, alis.fp);
        }
        else
        {
            // Not enough free RAM for the whole film — on a real Falcon the
            // game data leaves ~2.5 MB of heap while the large speech videos
            // run 3-7.5 MB, and this malloc failing silently skipped them.
            // Fall back to the original CD engine's scheme: stream the film
            // through a bounded sliding window (fli_stream_* in video.c).
            // fli_stream_open takes ownership of the file handle.
            if (fli_stream_open(alis.fp, (u32)size))
            {
                addr = bfilm.sbuf;
                alis.fp = NULL;
            }
            else
            {
                extern void dbglog(const char*,...);
                dbglog("[flistream] film '%s' (%ld bytes) SKIPPED — no RAM for it at all\n",
                       alis.sd7, (long)size);
            }
        }
    }

    if (alis.platform.version >= 60 && addr != NULL)
        alis.filmnode = node_take(0xb);             // film buffer
    
    bfilm.addr1 = addr;
    bfilm.addr2 = addr;
    bfilm.delptr = addr;
    // TODO: not realy sure what next value is used for
    readexec_opername();
    readexec_opername();
    bfilm.waitclock = alis.varD7;
    bfilm.basemain = alis.basemain;
    inifilm();
    alis.varD7 = bfilm.frames;
    cstore_continue();
}

// ============================================================================
#pragma mark - Codesc1 routines pointer table
// ============================================================================
const sAlisOpcode codesc1names[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
    DECL_OPCODE(0x01, csoundon,     "sound on"),
    DECL_OPCODE(0x02, csoundoff,    "sound off"),
    DECL_OPCODE(0x03, cmusicon,     "music on"),
    DECL_OPCODE(0x04, cmusicoff,    "music off"),
    DECL_OPCODE(0x05, cdelfilm,     "close and delete video from memory"),
    DECL_OPCODE(0x06, copenfilm,    "open video")
};

// ============================================================================
#pragma mark - Codesc2 routines pointer table
// ============================================================================

const sAlisOpcode codesc2names[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
};

// ============================================================================
#pragma mark - Codesc3 routines pointer table
// ============================================================================

const sAlisOpcode codesc3names[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
};


// ============================================================================
#pragma mark - v6.0+ escape-1 handlers (merged from the former newalis.c)
// ============================================================================
// v6 extends escape-1 from 7 entries to 82, so unlike the other tables this one
// is genuinely its own table rather than a base plus overrides. The handlers and
// the table now live beside the base escape handlers, which is where the file
// split says they belong.


// ============================================================================
// Routines for new versions of ALIS interpreter
// ============================================================================
// For games:
//    Deus +
//    Time Warriors +
//    Asghan +
//    Tournament Of Warriors +
//    Arabian Nights ?
//    Les Visiteurs ?	
//    Inspector Gadget ?
// ============================================================================
// Changes and additions:
// ============================================================================
// 1. Opcode / Codop (Code-op) routines
// ============================================================================
// tcodop_0D: cjsrind16 => cjsrproc
// tcodop_0E: cjsrind24 => cstartproc
// tcodop_0F: cjmpind16 => cjmpproc
// tcodop_2A: crandom => cnul
// tcodop_33: cleave => cstartbase
// tcodop_34: cprotect => cnul
// tcodop_37: cnul => canim
// tcodop_3A: cclipping => cvecacopy
// tcodop_3B: cswitching => cvecrot
// tcodop_3C: cwlive => cveclive
// tcodop_43: cstopret => cvecdistabs
// tcodop_46: cdefsc => csctype
// tcodop_49: cputnat => cputat
// tcodop_4B: cerasen => cobjmov
// tcodop_50: cerasall => cnul
// tcodop_51: cforme => caddput
// tcodop_52: cdelforme => caddputat
// tcodop_5F: corient => ctstanim
// tcodop_6E: cnoise => cmodanim
// tcodop_79: cplot => cvecset
// tcodop_7A: cdraw => cvecmov
// tcodop_7B: cbox => cveccopy
// tcodop_7C: cboxf => cvecadd
// tcodop_7E: cpset => cvecsub
// tcodop_7F: cpmove => cvecsize
// tcodop_80: cpmode => cvecnorm
// tcodop_81: cpicture => cscworld
// tcodop_82: cxyscroll => cvecdist
// tcodop_89: cdefvect => cobjaset
// tcodop_8A: csetvect => cobjamov
// tcodop_8C: capproach => ctsthit
// tcodop_8D: cescape => cftsthit
// tcodop_93: cfindmat => cfindworld
// tcodop_98: csetvolum => cvolume
// tcodop_9E: cmsound => centsound
// tcodop_A2: cwmov => cmatterpix
// tcodop_A3: cwtstmov => ctsthere
// tcodop_A4: cwftstmov => cftsthere
// tcodop_A5: ctstform => cavftstmov
// tcodop_AA: cmxput => ceput
// tcodop_AB: cmxputat => ceputat
// tcodop_AC: cmmusic => cavtsthit
// tcodop_AD: cmforme => cputstring
// tcodop_B2: casleepfar => cfreezefar
// tcodop_B3: casleepon => cfaron
// tcodop_B4: casleepoff => cfaroff
// tcodop_B6: cvpicprint => cfreeze
// tcodop_B7: cspicprint => cunfreeze
// tcodop_C3: cschoriz => cscmode
// tcodop_C4: cscvertic => cscclr
// tcodop_C5: cscreduce => cunloadfar
// tcodop_C7: creducing => cobjmode
// tcodop_C9: cscdump => cframe
// tcodop_CE: cminstru => cavftsthit
// tcodop_D0: calign => clight
// tcodop_D5: cquality => cwalkanim
// tcodop_D6: chsprite => cfwalkanim
// tcodop_DB: ccancel => cdel
// tcodop_DC: ccancall => cobjlink
// tcodop_DD: ccancen => cobjset
// tcodop_E7: csczoom => crecalc
// tcodop_E8: ctexmap => cfwalk
// tcodop_ED: cscsun => cnul ; cobjcycle
// tcodop_EF: cscdark => cscshade
// tcodop_F7: cwalkmap => cwalk
// tcodop_FF: czoom => cobjscale
// ============================================================================
// 2. Codesc1 routines
// ============================================================================
// tcodesc1_07: + cdeffont
// tcodesc1_08: + cloadtext
// tcodesc1_09: + cafteron
// tcodesc1_0A: + cafteroff
// tcodesc1_0B: + cbeforon
// tcodesc1_0C: + cbeforoff
// tcodesc1_0D: + cscfocale
// tcodesc1_0E: + csccamera
// tcodesc1_0F: + ctexmap
// tcodesc1_10: + cscrender
// tcodesc1_11: + cvideo
// tcodesc1_12: + ctexture
// tcodesc1_13: + cplaycd
// tcodesc1_14: + cnul
// tcodesc1_15: + cobjtrans
// tcodesc1_16: + conscreen
// tcodesc1_17: + cgetglobal
// tcodesc1_18: + cgetlocal
// tcodesc1_19: + cmatitem
// tcodesc1_1A: + cmaskitem
// tcodesc1_1B: + cdefitem
// tcodesc1_1C: + cnul
// tcodesc1_1D: + cnul
// tcodesc1_1E: + cgetmap
// tcodesc1_1F: + cconvmap
// tcodesc1_20: + csetfilm
// tcodesc1_21: + cscmem
// tcodesc1_22: + cobjorient ; =amtstmap
// tcodesc1_23: + cobjaim
// tcodesc1_24: + cloadpic
// tcodesc1_25: + cputpic
// tcodesc1_26: + cfwriteg
// tcodesc1_27: + cfreadg
// tcodesc1_28: + cobjmat
// tcodesc1_29: + cobjmask
// tcodesc1_2A: + cscclut
// tcodesc1_2B: + cobjclut
// tcodesc1_2C: + copennet
// tcodesc1_2D: + cclosenet
// tcodesc1_2E: + csendnet
// tcodesc1_2F: + cmailnet
// tcodesc1_30: + cplayernet
// tcodesc1_31: + cgamenet
// tcodesc1_32: + csyncnet
// tcodesc1_33: + ctexitem
// tcodesc1_34: + cgetanim
// tcodesc1_35: + canimsize
// tcodesc1_36: + cscmath
// tcodesc1_37: + cintermode
// tcodesc1_38: + csysnet
// tcodesc1_39: + cobjshad
// tcodesc1_3A: + csphere
// tcodesc1_3B: + cscdark
// tcodesc1_3C: + cagetlocal
// tcodesc1_3D: + cfollow
// tcodesc1_3E: + cplayfilm
// tcodesc1_3F: + csoundtime
// tcodesc1_40: + cinfoitem
// tcodesc1_41: + cmorph
// tcodesc1_42: + cscmicro
// tcodesc1_43: + cptrset
// tcodesc1_44: + cdefproc
// tcodesc1_45: + cfreadm
// tcodesc1_46: + cfwritem
// tcodesc1_47: + cmaskent
// tcodesc1_48: + cvram
// tcodesc1_49: + cadresnet
// tcodesc1_4A: + cfdir
// tcodesc1_4B: + cnul ; entitem
// tcodesc1_4C: + cloadas
// tcodesc1_4D: + cfindobjitem
// tcodesc1_4E: + cobjid
// tcodesc1_4F: + cgetobjid
// tcodesc1_50: + cprelight
// tcodesc1_51: + croomdoor
// ============================================================================
// 3. Oper routines
// ============================================================================
// toper_12: odirb => olocl
// toper_14: odirw => oloctl
// toper_16: odirp => omainl
// toper_18: odirtp => omaintl
// toper_1A: odirtc => ohiml
// toper_1C: odirti => ohimtl
// toper_3C: pnul => oimml
// toper_3E: pnul => olocto
// toper_A0: ochange => ofpath
// toper_AA: + ogettext
// toper_AC: + oips
// toper_AE: + ogetent
// toper_B0: + osqrt
// toper_B2: + ocos
// toper_B4: + osin
// toper_B6: + otan
// toper_B8: + oatan
// toper_BA: + obufkey ; bidon...
// toper_BC: + omaxitem
// toper_BE: + otick
// toper_C0: + orandom
// toper_C2: + oflen
// toper_C4: + olocalkey
// toper_C6: + odivi
// toper_C8: + oprotect
// toper_CA: + otypitem
// toper_CC: + oloaded
// ============================================================================
// 4. Add routines
// ============================================================================
// tstore_12: sdirb => slocl
// tstore_14: sdirw => sloctl
// tstore_16: sdirp => smainl
// tstore_18: sdirtp => smaintl
// tstore_1A: sdirtc => shiml
// tstore_1C: sdirti => shimtl
// ============================================================================
// 5. Store routines
// tadd_12: adirb => alocl
// tadd_14: adirw => aloctl
// tadd_16: adirp => amainl
// tadd_18: adirtp => amaintl
// tadd_1A: adirtc => ahiml
// tadd_1C: adirti => ahimtl
// ============================================================================

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines
// ============================================================================

// Codopname no. 001 opcode 0x00 cnul
// Calls stub cnul (from old version)


// Codopname no. 043 opcode 0x2a cnul
// Change v40+: crandom => cnul
// Calls stub cnul (from old version)


// Codopname no. 053 opcode 0x34 cnul
// Change v40+: cprotect => cnul
// Calls stub cnul (from old version)
// Codopname no. 081 opcode 0x50 cnul
// Change v40+: cerasall => cnul
// Calls stub cnul (from old version)// Codopname no. 140 opcode 0x8b cnul
// Calls stub cnul (from old version)


// Codopname no. 238 opcode 0xed cnul
// Change v40+: cscsun => cnul ; cobjcycle
// Calls stub cnul (from old version)

// ============================================================================
#pragma mark - Codesc1 routines
// ============================================================================

// Codesc1name no. 001 opcode 0x00 cnul
// Calls stub cnul (from old version)

// Codesc1name no. 002-005 opcodes 0x01-0x04 csoundon/csoundoff/cmusicon/cmusicoff
void csoundon(void);
void csoundoff(void);
void cmusicon(void);
void cmusicoff(void);

// v6 escapes 0x14/0x15/0x1c/0x1d
void camovnat(void);
void csetvectnat(void);
void casetnat(void);
void cdefvectnat(void);
void cgetglobal(void);
void cgetlocal(void);
void cobjaim(void);

// Codesc1name no. 006 opcode 0x05 cdelfilm
// Codesc1name no. 007 opcode 0x06 copenfilm
void copenfilm(void);
void cdelfilm(void);

// Codesc1name no. 008 opcode 0x07 cdeffont
static void cdeffont(void)
{
    readexec_opername(); s16 index  = alis.varD7;
    readexec_opername(); s16 mode   = alis.varD7;
    readexec_opername(); u8 op2 = (u8)alis.varD7;
    readexec_opername(); s16 foasc  = alis.varD7;
    readexec_opername(); s16 fonum  = alis.varD7;
    readexec_opername(); s16 fomax  = alis.varD7;
    readexec_opername(); s16 folarg = alis.varD7;
    readexec_opername(); s16 fohaut = alis.varD7;

    if (index >= 0 && index < (s16)(sizeof(alis.v6fonts) / sizeof(alis.v6fonts[0])))
    {
        alis.v6fonts[index].foasc   = foasc;
        alis.v6fonts[index].fonum   = fonum;
        alis.v6fonts[index].fomax   = fomax;
        alis.v6fonts[index].folarg  = (u8)folarg;
        alis.v6fonts[index].fohaut  = (u8)fohaut;
        alis.v6fonts[index].mode    = (u8)mode;
        alis.v6fonts[index].usemain = alis.flagmain;
        alis.v6fonts[index].valid   = 1;

        alis.v6xfonts[index].id        = mode;
        alis.v6xfonts[index].foasc     = foasc;
        alis.v6xfonts[index].count     = fomax;
        alis.v6xfonts[index].spacing   = folarg;
        alis.v6xfonts[index].height    = fohaut;
        alis.v6xfonts[index].glyphslot = (fonum >= 0) ? (u32)adresdes(fonum) : 0;
        alis.v6xfonts[index].valid     = 1;
        alis.v6xfonts[index].op2       = op2;
        alis.v6xfonts[index].form      = fonum;
        alis.v6xfonts[index].owner     = xread16(alis.script->vram_org + 0x3a);
        memcpy(alis.v6xfonts[index].ownername, alis.mem + alis.script->vram_org + 0x3c, 8);
    }
}

// Codesc1name no. 009 opcode 0x08 cloadtext
static u8 *text_read(const char *name, u32 *sz)
{
    char path[kPathMaxLen] = {0};
    snprintf(path, sizeof(path), "%s%s.to", alis.platform.path, name);
    strlower(path + strlen(alis.platform.path));
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        ALIS_DEBUG(EDebugError, "cloadtext: cannot open text file '%s'\n", path);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    long len = ftell(fp);
    rewind(fp);
    u8 *data = (u8 *)malloc((size_t)len + 1);
    if (data)
    {
        size_t rd = fread(data, 1, (size_t)len, fp);
        data[rd] = 0;
        *sz = (u32)rd;
        ALIS_DEBUG(EDebugInfo, "cloadtext: loaded '%s' (%u bytes)\n", path, *sz);
    }

    fclose(fp);
    return data;
}

// v6 resident text of block n (NULL if none)
u8 *v6text_get(u16 n, u32 *sz)
{
    for (int i = 0; n && i < 8; i++)
        if (alis.v6texts[i].node == n)
        {
            *sz = alis.v6texts[i].sz;
            return alis.v6texts[i].data;
        }

    return NULL;
}

// unloadprog freed text block n
void v6text_free(u16 n)
{
    for (int i = 0; n && i < 8; i++)
        if (alis.v6texts[i].node == n)
        {
            free(alis.v6texts[i].data);
            memset(&alis.v6texts[i], 0, sizeof(alis.v6texts[i]));
        }
}

// savestate restore: host buffers are gone, reread them by block name
void v6text_reload(void)
{
    for (int i = 0; i < 8; i++)
    {
        alis.v6texts[i].data = NULL;
        if (alis.v6texts[i].node == 0)
            continue;
        char name[9] = {0};
        memcpy(name, alis.mem + alis.atent + alis.v6texts[i].node + 6, 8);
        alis.v6texts[i].data = text_read(name, &alis.v6texts[i].sz);
    }
}

// load name.to as a new text bloc
u16 v6text_load(const char *name)
{
    int slot = 0;
    while (slot < 8 && alis.v6texts[slot].node != 0)
        slot++;
    u32 sz = 0;
    u8 *data = slot < 8 ? text_read(name, &sz) : NULL;
    if (data == NULL)
        return 0;
    u16 node = node_take(0xc);
    if (node == 0)
    {
        free(data);
        return 0;
    }

    node_setname(node, name);
    alis.v6texts[slot].node = node;
    alis.v6texts[slot].sz = sz;
    alis.v6texts[slot].data = data;
    return node;
}

static void cloadtext(void)
{
    readexec_opername_swap();

    char name[24] = {0};
    strncpy(name, alis.sd7, sizeof(name) - 5);

    if (alis.platform.version >= 60)
    {
        // a resident block of that name is not reloaded
        if (node_find(name) == 0)
            v6text_load(name);
        return;
    }

    u32 sz = 0;
    u8 *data = text_read(name, &sz);
    if (data == NULL)
        return;
    snprintf(alis.text_name, sizeof(alis.text_name), "%s.to", name);
    free(alis.text_data);
    alis.text_data = data;
    alis.text_sz = sz;
}

// v6 escape 0x09/0x0a/0x0b/0x0c
#define V6_VECFLAGS(vram) ((vram) - 0x67)

static void cafteron(void)
{
    u32 vram = alis.script->vram_org;
    u32 data = get_0x14_script_org_offset(vram);
    if (xread32(data + 0x20) != 0)
        xwrite8(V6_VECFLAGS(vram), xread8(V6_VECFLAGS(vram)) | 0x2);
}

// Codesc1name no. 011 opcode 0x0a cafteroff
static void cafteroff(void)
{
    u32 vram = alis.script->vram_org;
    xwrite8(V6_VECFLAGS(vram), xread8(V6_VECFLAGS(vram)) & 0xfd);
}

// Codesc1name no. 012 opcode 0x0b cbeforon
static void cbeforon(void)
{
    u32 vram = alis.script->vram_org;
    u32 data = get_0x14_script_org_offset(vram);
    if (xread32(data + 0x1c) != 0)
        xwrite8(V6_VECFLAGS(vram), xread8(V6_VECFLAGS(vram)) | 0x1);
}

// Codesc1name no. 013 opcode 0x0c cbeforoff
// Addition v40+: cbeforoff
static void cbeforoff(void)
{
    u32 vram = alis.script->vram_org;
    xwrite8(V6_VECFLAGS(vram), xread8(V6_VECFLAGS(vram)) & 0xfe);
}

// Codesc1name no. 014 opcode 0x0d cscfocale
static void readexec_opername_v6(void)
{
    if (alis.platform.version >= 60)
    {
        readexec_opername();
    }
    else
    {
        readexec_opername_saveD7();
    }
}

static void cscfocale(void)
{
    u32 scr = alis.basemain + (s16)xread16(alis.script->vram_org - 0x64);

    readexec_opername_v6();
    xwrite16(scr + 0x48, alis.varD7);
    readexec_opername_v6();
    xwrite16(scr + 0x4a, alis.varD7);
    readexec_opername_v6();
    xwrite16(scr + 0x4c, alis.varD7);
    xwrite16(scr + 0x4e, 0);
    xwrite16(scr + 0x50, 0);
    xwrite16(scr + 0x52, 0);
    readexec_opername_v6();
    xwrite32(scr + 0x142, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x146, (s32)alis.varD7);
    xwrite16(scr + 0x44, (s16)(xread32(scr + 0x146) >> 1));
    xwrite16(scr + 0x42, 0);
    xwrite16(scr + 0x46, 0);

    v6x_focangle(scr);

    u32 eax = ((u32)xread32(scr + 0x146) >> 1) + 1 + 0xc8;
    xwrite16(scr + 0x40, (s16)eax);
    alis.v6focstep = (s32)(eax >> 3);
    v6_scdark_precalc(scr);
}

// Codesc1name no. 015 opcode 0x0e csccamera
static void csccamera(void)
{
    if (alis.platform.version >= 60)
    {
        v6_skipprot();
        return;
    }

    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// resource record for a form id; *slot = its self-relative table entry
static u32 texres(s16 id, u32 *slot)
{
    *slot = adresdes(id);
    return *slot + xread32(*slot);
}

// a linked resource remembers the current texture id at +8
static void texmark(u32 res)
{
    if (res)
        xwrite32(res + 8, alis.texcur | (u32)alis.texcur << 16);
}

// Codesc1name no. 016 opcode 0x0f ctexmap
static void ctexmap(void)
{
    if (alis.platform.uid != EGameRobinsonsRequiem3DO)
        return;
    alis.texmap = read_vaddr();
    readexec_opername_v6();
    alis.texcur = alis.varD7;
    u32 e = alis.texmap + (s16)alis.varD7 * 0x14 - 0xc00;
    readexec_opername_v6();
    xwrite8(e + 8, alis.varD7);
    readexec_opername_v6();
    u32 res = 0, slot;
    s8 kind = xread8(e + 8);
    if (kind > 0)
    {
        res = texres(alis.varD7, &slot);
        xwrite32(e, res - e);
    }
    else if (kind == 0)
    {
        xwrite32(e, 0);
    }

    readexec_opername_v6();
    readexec_opername_v6();
    xwrite8(e + 4, alis.varD7);
    readexec_opername_v6();
    texmark(res);

    readexec_opername_v6();
    s8 mode = alis.varD7;
    xwrite8(e + 9, mode);
    if (mode < 0)
    {
        for (int i = 0; i < 4; i++)
            readexec_opername_v6();
        
        xwrite32(e + 0xc, 0);
        return;
    }

    readexec_opername_v6();
    if (mode == 2)
    {
        xwrite16(e + 0x14, alis.varD7);
        for (int i = 0; i < 3; i++)
            readexec_opername_v6();
        
        return;
    }

    alis.texcur = alis.varD7;
    res = 0;
    if (mode == 0)
    {
        xwrite32(e + 0xc, 0);
    }
    else
    {
        res = texres(alis.varD7, &slot);
        u8 type = xread8(res);
        if (type == 6 || type == 2)
        {
            xwrite16(e + 0xa, xread32(res + 0x14));
            s16 sub = xread32(res + 0xc);
            if (type == 6)
            {
                alis.texcur += sub;
                slot += sub * 4;
                res = slot + xread32(slot);
            }
            else
            {
                alis.texcur = sub;
                res = texres(sub, &slot);
            }
        }
        else
        {
            xwrite16(e + 0xa, 0);
        }

        xwrite32(e + 0xc, res - (e + 0xc));
        xwrite32(e + 0x10, (u16)xread16(alis.script->vram_org + 0x3a));
    }

    for (int i = 0; i < 3; i++)
        readexec_opername_v6();
    texmark(res);
}

// Codesc1name no. 017 opcode 0x10 cscrender
static void cscrender(void)
{
    u32 scr = alis.basemain + (s16)xread16(alis.script->vram_org - 0x64);   // MOVSX
    readexec_opername_v6();
    xwrite16(scr + 0x13c, alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x14a, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x14e, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x152, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x15a, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x156, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x15e, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x166, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x16a, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x162, (s32)alis.varD7);
    readexec_opername_v6();
    xwrite32(scr + 0x172, (s32)alis.varD7);
}

// Codesc1name no. 018 opcode 0x11 cvideo
static void cvideo(void)
{
    readexec_opername();
    s16 mode = alis.varD7;
    readexec_opername_saveD7();
    s16 w = alis.varD7;
    readexec_opername_saveD7();
    s16 h = alis.varD7;

    ALIS_DEBUG(EDebugWarning, "cvideo: script=%s mode=%d w=%d h=%d\n", alis.script->name, mode, w, h);

    if (alis.platform.version >= 60)
    {
        if (mode == 0 || mode == 2)
        {
            alis.videomode = 0;
        }
        else if (mode == 1)
        {
            alis.videomode = 1;
        }

        resize_screen(w, h);
    }
}

// Codesc1name no. 019 opcode 0x12 ctexture
static void ctexture(void)
{
    readexec_opername_v6();
    s32 entry = adresdes(alis.varD7);
    s32 desc  = (entry == 0xf) ? 0 : entry + xread32(entry);
    readexec_opername_v6();
    s16 idx = alis.varD7;

    if (desc == 0 || xread8(desc) != 8 || (u16)idx >= (u16)xread16(desc + 0x32))
    {
        readexec_opername_v6();
        readexec_opername_v6();
        readexec_opername_v6();
        readexec_opername_v6();
        return;
    }

    s32 item = desc + ((u16)idx << 5) + xread32(desc + 0x44);
    readexec_opername_v6();
    u8 sub = (u8)alis.varD7;

    switch (sub)
    {
        case 1:
        {
            readexec_opername_v6();
            s32 e2 = adresdes(alis.varD7);
            s32 d2 = (e2 == 0xf) ? 0 : e2 + xread32(e2);
            xwrite32(item + 0xc, e2 - item);
            xwrite32(item, d2 - item);
            readexec_opername_v6();
            xwrite8(item + 0x10, (u8)alis.varD7);
            readexec_opername_v6();
            xwrite8(item + 0x12, (u8)alis.varD7);
            xwrite8(item + 0x13, 0);
            xwrite8(item + 0x11, 0);
            break;
        }
        case 5:
        {
            readexec_opername_v6();
            xwrite16(item + 0x14, alis.varD7);
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
        case 6:
        {
            readexec_opername_v6();
            xwrite16(item + 0x16, alis.varD7);
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
        case 3:
        {
            readexec_opername_v6();
            s16 p1 = alis.varD7;
            readexec_opername_v6();
            s16 p2 = alis.varD7;
            readexec_opername_v6();
            s16 p3 = alis.varD7;
            xwrite8(item + 0xb, (u8)(((p3 & 3) << 6) + ((p1 & 7) << 3) + (p2 & 7)));
            break;
        }
        case 7:
        {
            readexec_opername_v6();
            xwrite8(item + 0x8, xread8(item + 0x8) & 0xf7);
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
        case 8:
        {
            readexec_opername_v6();
            xwrite8(item + 0x8, xread8(item + 0x8) | 0x8);
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
        case 9:
        {
            readexec_opername_v6();
            u8 dl = (u8)alis.varD7;
            if (dl <= 3)
                dl = 3;
            
            xwrite8(item + 0x9, (xread8(item + 0x9) & 0xfc) | dl);
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
        case 0xa:
        {
            readexec_opername_v6();
            xwrite8(item + 0xa, (u8)alis.varD7);
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
        case 0xb:
        {
            readexec_opername_v6();
            if (alis.varD7 != 0)
            {
                xwrite8(item + 0x8, xread8(item + 0x8) & 0xef);
            }
            else
            {
                xwrite8(item + 0x8, xread8(item + 0x8) | 0x10);
            }
            
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
        default:
        {
            readexec_opername_v6();
            readexec_opername_v6();
            readexec_opername_v6();
            break;
        }
    }
}

// Codesc1name no. 020 opcode 0x13 cplaycd
static void cplaycd(void)
{
    readexec_opername();
    s16 cmd = alis.varD7;
    readexec_opername();
    s16 n = alis.varD7;
    readexec_opername();
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
    {
        static char trk = '0';
        if (cmd == 1 && n >= 0 && n <= 9)
        {
            trk = '0' + n;
        }
        else if (cmd == 2)
        {
            char path[kPathMaxLen];
            snprintf(path, sizeof(path), "%strack%c", alis.platform.path, trk);
            cdtrack_play(path, (s8)alis.varD7);
        }
        else if (cmd == 3)
        {
            cdtrack_stop();
        }

        alis.varD7 = 0;
    }

    // TODO: CD audio playback / sound routine - Deus DOS sound (OPL2/CD) not implemented.
    cstore_continue();
}

// Codesc1name no. 021 opcode 0x14 cnul
// Codesc1name no. 022 opcode 0x15 cobjtrans
// Codesc1name no. 023 opcode 0x16 conscreen
static void conscreen(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 024/025 opcodes 0x17 cgetglobal / 0x18 cgetlocal: opcodes.c
// Codesc1name no. 026 opcode 0x19 cmatitem
static void item_word(u32 off)
{
    readexec_opername();
    s32 desc = adresdes((s16)alis.varD7);
    u32 rec = (desc == 0xf) ? 0 : (u32)desc + xread32((u32)desc);
    readexec_opername();
    if (rec == 0)
        return;
    
    u8 type = xread8(rec);
    if (type != 3 && type != 9 && type != 2)
        xwrite16(rec + off, alis.varD7);
}

static void cmatitem(void)
{
    item_word(0x8);
}

// Codesc1name no. 027 opcode 0x1a cmaskitem
static void cmaskitem(void)
{
    item_word(0xa);
}

// Codesc1name no. 028 opcode 0x1b cdefitem
static void cdefitem(void)
{
    if (alis.platform.version < 60)
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        return;
    }

    readexec_opername();
    s32 form = (s16)alis.varD7;
    s32 desc = adresdes(form);
    u32 rec = (desc == 0xf) ? 0 : (u32)desc + xread32((u32)desc);
    u8 type = rec ? xread8(rec) : 0xff;

    if (type == 7)
    {
        readexec_opername();
        s32 sel = alis.varD7;
        if (sel == 3)
        {
            readexec_opername();
            if (rec)
                xwrite8(rec + 0x1b, (u8)alis.varD7);
            
            readexec_opername();
            if (rec)
                xwrite16(rec + 0x12, (u16)alis.varD7);
            
            readexec_opername();
            if (rec)
                xwrite16(rec + 0x38, 0xff);
            
            return;
        }
        
        if (sel == 8)
        {
            readexec_opername();
            if (rec)
                xwrite8(rec + 0x37, (u8)alis.varD7);
            
            readexec_opername();
            readexec_opername();
            return;
        }
        
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        return;
    }

    if (type == 8)
    {
        readexec_opername();
        s32 sel = alis.varD7;
        if (sel == 7)
        {
            readexec_opername();
            if (rec)
                xwrite16(rec + 0x26, (u16)alis.varD7);
            
            readexec_opername();
            readexec_opername();
            return;
        }
        
        if (rec)
        {
            if (sel == 4)
            {
                xwrite32(rec + 0x28, xread32(rec + 0x28) | 0x10);
            }
            else if (sel == 5)
            {
                xwrite32(rec + 0x28, xread32(rec + 0x28) | 0x20);
            }
            else if (sel == 6)
            {
                xwrite32(rec + 0x28, xread32(rec + 0x28) | 0x08);
            }
            else if (sel == 9)
            {
                xwrite32(rec + 0x28, xread32(rec + 0x28) | 0x80);
            }
        }

        readexec_opername();
        readexec_opername();
        readexec_opername();
        return;
    }

    readexec_opername();
    readexec_opername();
    readexec_opername();
    readexec_opername();
}

// Codesc1name no. 029 opcode 0x1c cnul
// Codesc1name no. 030 opcode 0x1d cnul
// Codesc1name no. 031 opcode 0x1e cgetmap
static void cgetmap(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 032 opcode 0x1f cconvmap
static void cconvmap(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 033 opcode 0x20 csetfilm
static void csetfilm(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 034 opcode 0x21 cscmem
static void cscmem(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

static void cmapcell(void)
{
    if (alis.platform.uid == EGameRobinsonsRequiem3DO)
        v6_mapcell(-1);
}

// Codesc1name no. 035 opcode 0x22
// Codesc1name no. 036 opcode 0x23 cobjaim: opcodes.c (shares the caim tail)
// Codesc1name no. 037 opcode 0x24 cloadpic
static void cloadpic(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 038 opcode 0x25 cputpic
static void cputpic(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// ---------------------------------------------------------------------------
// v6 save game, byte-exact DOS chunk format, little endian.
// ---------------------------------------------------------------------------

static u8 sg_hdr[16];

static void sg_le32(u8 *p, u32 v) { p[0] = (u8)v; p[1] = (u8)(v >> 8); p[2] = (u8)(v >> 16); p[3] = (u8)(v >> 24); }
static void sg_le16(u8 *p, u16 v) { p[0] = (u8)v; p[1] = (u8)(v >> 8); }
static void sg_write(const void *p, u32 n) { fwrite(p, 1, n, alis.fp); }

static void sg_chunk(const char *tag, u32 a, int n)
{
    memcpy(sg_hdr, tag, 4);
    sg_le32(sg_hdr + 4, a);
    sg_write(sg_hdr, n);
}

static u32 sg_link(u32 vram, u32 p)
{
    return p == v6_elemhead(vram) ? vram - 0x2c : p == v6_elemtail(vram) ? vram - 0x24 : p;
}

static void sg_ctx(sAlisScriptLive *s, u8 *c)
{
    u32 v = s->vram_org;
    u32 org = get_0x14_script_org_offset(v);
    u16 qsize = xread16(org + 0x36), stk = xread16(org + 0x34);
    u8 *d = c + 0x40;
    memset(c, 0, 0x40);
    sg_le16(d - 0x40, get_0x1e_scan_clr(v));
    sg_le16(d - 0x3e, get_0x1c_scan_clr(v));
    sg_le16(d - 0x3c, get_0x0a_vacc_offset(v) + 0x40);
    sg_le16(d - 0x3a, (u16)-(0x40 + qsize));
    sg_le16(d - 0x38, (u16)-(0x40 + qsize + stk));
    sg_le16(d - 0x36, (u16)(xread32(alis.atent + get_0x0e_script_ent(v) + 8) - xread16(alis.atent + get_0x0e_script_ent(v) + 6)));
    u16 mark = xread16(v - 0x7c);
    sg_le16(d - 0x32, mark ? mark + 0x40 : 0);
    u32 home = xread32(v - 0x80);
    sg_le32(d - 0x30, mark ? s->pc_org + home : 0);
    sg_le32(d - 0x2c, v - 0x2c);
    sg_le32(d - 0x28, sg_link(v, v6el_next(v6_elemhead(v))));
    sg_le32(d - 0x24, sg_link(v, v6el_prev(v6_elemtail(v))));
    sg_le32(d - 0x20, v - 0x24);
    d[-0x1c] = xread8(v - 0x66);
    sg_le16(d - 0x18, xread16(v - 0x64));
    sg_le16(d - 0x16, get_0x16_screen_id(v));
    sg_le16(d - 0x14, xread16(V6_CLINKING(v)));
    sg_le16(d - 0x12, get_0x3a_wait_cycles(v));
    sg_le32(d - 0x10, get_0x3e_wait_time(v));
    sg_le32(d - 0x0c, org);
    sg_le32(d - 0x08, get_0x08_script_ret_offset(v));
    d[-0x04] = V6_SLOT_STRIDE;
    d[-0x03] = get_0x2b_cordspr(v);
    u8 s24 = (u8)get_0x24_scan_inter(v);
    d[-0x02] = (xread8(v - 0x67) & 0x0b) | (s24 & 1 ? 0 : 4) | (s24 & 4 ? 0 : 0x10);
    u8 fl = xread8(V6_FARSLEEP(v));
    d[-0x01] = (get_0x04_cstart_csleep(v) == 0 ? 1 : 0) | (fl & (2 | V6_DORMANT)) | (s24 & 0x80);
}

// palette, 6-bit components
static void sg_pal(u32 *pal, u8 *out)
{
    for (int i = 0; i < 256; i++)
    {
        u8 r = 0, g = 0, b = 0;
        PAL_READ_RGB(pal, i, r, g, b);
        if (image.pal_format == EPal565)
        {
            out[i * 3 + 0] = (u8)(r << 1 | r >> 4);
            out[i * 3 + 1] = g;
            out[i * 3 + 2] = (u8)(b << 1 | b >> 4);
        }
        else
        {
            out[i * 3 + 0] = r >> 2;
            out[i * 3 + 1] = g >> 2;
            out[i * 3 + 2] = b >> 2;
        }
    }
}

static void sg_save(u32 flags)
{
    memcpy(sg_hdr, "ALIS", 4);
    sg_le32(sg_hdr + 4, 8);
    sg_le32(sg_hdr + 8, flags);
    sg_le32(sg_hdr + 0xc, 0x9e30);
    sg_write(sg_hdr, 16);

    if (flags & 0x40)
    {
        sg_chunk("SYST", 0x284, 8);
        u8 b[4];
        sg_le32(b, alis.basemain);
        sg_write(b, 4);
        for (int i = 0; i < 16; i++)
        {
            u8 f[0x28] = {0};
            if (alis.v6xfonts[i].valid)
            {
                sg_le32(f + 0x00, alis.v6xfonts[i].id);
                f[0x04] = alis.v6xfonts[i].op2;
                sg_le32(f + 0x08, alis.v6xfonts[i].foasc);
                sg_le32(f + 0x0c, alis.v6xfonts[i].form < 0 ? (u32)(s32)alis.v6xfonts[i].form : alis.v6xfonts[i].glyphslot);
                sg_le32(f + 0x10, alis.v6xfonts[i].count);
                sg_le32(f + 0x14, alis.v6xfonts[i].height);
                sg_le32(f + 0x18, alis.v6xfonts[i].spacing);
                sg_le16(f + 0x1c, alis.v6xfonts[i].owner);
                sg_le16(f + 0x1e, alis.v6xfonts[i].form);
                memcpy(f + 0x20, alis.v6xfonts[i].ownername, 8);
            }

            sg_write(f, 0x28);
        }
    }

    if (flags & 0x01)
    {
        sg_chunk("PROG", 0, 8);
        u16 list[256];
        int n = 0;
        for (u16 b = xread16(alis.atent + NODE_BLKHEAD + 0xe); b != NODE_BLKTAIL && n < 256; b = xread16(alis.atent + b + 0xe))
            list[n++] = b;
        for (int i = n - 2; i >= 0; i--)
        {
            u16 b = list[i];
            u8 t = xread8(alis.atent + b + 0x15);
            if (t != 2 && t != 0xc && t != 0xd)
                continue;
            if (memcmp(alis.mem + alis.atent + b + 6, "main\0\0\0\0", 8) == 0)
                continue;
            memcpy(sg_hdr, alis.mem + alis.atent + b + 6, 8);
            sg_le32(sg_hdr + 8, 0);
            sg_le32(sg_hdr + 0xc, 3 | (t << 8));
            sg_write(sg_hdr, 16);
        }

        sg_le32(sg_hdr, 0);
        sg_write(sg_hdr, 16);
    }

    if (flags & 0x02)
    {
        sg_chunk("ENTI", 0, 8);
        u16 e = xread16(alis.atent + 4);
        if (!(flags & 0x10) && e)
            e = xread16(alis.atent + e + 4);
        for (; e != 0; e = xread16(alis.atent + e + 4))
        {
            sAlisScriptLive *s = ENTSCR(e);
            u32 v = s->vram_org;
            u8 *buf = malloc(0x40 + 0x10000);
            if (!buf)
                break;
            sg_ctx(s, buf);
            u16 vsz = (u16)(buf[0x40 - 0x36] | buf[0x40 - 0x35] << 8);
            memcpy(buf + 0x40, alis.mem + v, vsz);
            
            // elem links into head/tail nodes
            for (u32 g = v6el_next(v6_elemhead(v)); g != v6_elemtail(v) && g >= v && g < v + vsz; g = v6el_next(g))
            {
                sg_le32(buf + 0x40 + (g - v), sg_link(v, v6el_prev(g)));
                sg_le32(buf + 0x40 + (g - v) + 4, sg_link(v, v6el_next(g)));
            }

            memcpy(sg_hdr, alis.mem + v + 0x3c, 8);
            sg_le32(sg_hdr + 8, vsz + 0x40);
            sg_le32(sg_hdr + 0xc, 0);
            sg_write(sg_hdr, 16);
            sg_write(buf, vsz + 0x40);
            free(buf);
        }

        sg_le32(sg_hdr, 0);
        sg_write(sg_hdr, 16);
    }

    if (flags & 0x04)
    {
        sg_chunk("PALE", 0, 8);
        u8 p[0x300];
        sg_pal(image.ampalet, p);  sg_write(p, 0x300);
        sg_pal(image.atpalet, p);  sg_write(p, 0x300);
        sg_pal(image.svpalet, p);  sg_write(p, 0x300);
        sg_pal(image.svpalet2, p); sg_write(p, 0x300);
        u8 dk[0x600];
        for (int i = 0; i < 0x300; i++)
            sg_le16(dk + i * 2, (u16)image.dkpalet[i]);
        
        sg_write(dk, 0x600);
    }

    u32 scr = alis.basemain + (u16)xread16(alis.script->vram_org - 0x64);
    if ((flags & 0x08) && xread32(scr + 0xb4) != 0)
    {
        u32 h = xread32(scr + 0xb4);
        if (scrnode_get(scr, 0xb4) == 0)
        {
            ALIS_DEBUG(EDebugWarning, "fwriteg: screen +0xb4 is not a shading block, SHAD skipped\n");
        }
        else
        {
            u32 data = xread32(h);
            u32 raw = xread32(data - 8) - 8;
            u32 n = (raw | 0xf) + 1;
            memcpy(sg_hdr, "SHAD", 4);
            sg_le32(sg_hdr + 4, n + 4);
            sg_le32(sg_hdr + 8, scr - alis.basemain);
            sg_write(sg_hdr, 12);
            sg_write(alis.mem + data, raw);
            u8 z[0x10] = {0};
            sg_write(z, n - raw);
            memcpy(sg_hdr, "SHAV", 4);
            sg_le32(sg_hdr + 4, 0x2c);
            sg_le32(sg_hdr + 8, scr - alis.basemain);
            sg_write(sg_hdr, 12);
            sg_write(alis.mem + scr + 0x94, 0x28);
        }
    }

    if (flags & 0x20)
    {
        sg_chunk("VIDE", 8, 8);
        u8 b[8];
        sg_le32(b, alis.videomode);
        sg_le16(b + 4, alis.platform.width);
        sg_le16(b + 6, alis.platform.height);
        sg_write(b, 8);
    }
}

// Codesc1name no. 039 opcode 0x26 cfwriteg
static void cfwriteg(void)
{
    readexec_opername();
    if (alis.platform.version >= 60 && alis.platform.kind != EPlatform3DO && alis.fp)
        sg_save((u32)alis.varD7);
}

// ---------------------------------------------------------------------------
// v6 load game. Reads DOS and our saves; unlike DOS it validates the chunks first,
// skips entities whose program is missing and only rebases pointers that point
// into the old program
// ---------------------------------------------------------------------------

void killent(u16 entidx);
void shrinkprog(s32 start, s32 length, u16 script_id);
u32 v6_worldhead(void);

static u32 sg_rd32(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }
static u16 sg_rd16(const u8 *p) { return (u16)(p[0] | p[1] << 8); }

typedef struct {
    const u8 *f;
    u32 len;
    u32 syst, prog, enti, pale, shad, shav, vide;
} sSgFile;

static int sg_tag(const sSgFile *g, u32 at, const char *tag)
{
    return at + 8 <= g->len && memcmp(g->f + at, tag, 4) == 0;
}

// chunk walk in DOS read order; 0 if the file is broken
static int sg_parse(sSgFile *g, u32 flags)
{
    if (g->len < 16 || memcmp(g->f, "ALIS", 4) != 0)
        return 0;
    
    u32 at = 16;
    if (flags & 0x40)
    {
        if (!sg_tag(g, at, "SYST") || at + 8 + 0x284 > g->len)
            return 0;
        
        g->syst = at + 8;
        at += 8 + 0x284;
    }

    if (sg_tag(g, at, "PROG"))
    {
        g->prog = at += 8;
        for (;; at += 16)
        {
            if (at + 16 > g->len)
                return 0;
            
            if (sg_rd32(g->f + at) == 0)
                break;
        }

        at += 16;
    }

    if (sg_tag(g, at, "ENTI"))
    {
        g->enti = at += 8;
        for (;;)
        {
            if (at + 16 > g->len)
                return 0;
            
            if (sg_rd32(g->f + at) == 0)
                break;
            
            u32 sz = sg_rd32(g->f + at + 8);
            if (sz < 0x40 || at + 16 + sz > g->len)
                return 0;
            
            at += 16 + sz;
        }

        at += 16;
    }

    if (sg_tag(g, at, "PALE"))
    {
        if (at + 8 + 0x1200 > g->len)
            return 0;
        
        g->pale = at + 8;
        at += 8 + 0x1200;
    }

    if ((flags & 0x08) && sg_tag(g, at, "SHAD"))     // DOS reads it blindly (Q2)
    {
        u32 n = sg_rd32(g->f + at + 4);
        if (n < 4 || at + 8 + n > g->len || !sg_tag(g, at + 8 + n, "SHAV") || at + 8 + n + 12 + 0x28 > g->len)
            return 0;
        
        g->shad = at + 8;
        at += 8 + n;
        g->shav = at + 8;
        at += 12 + 0x28;
    }

    if ((flags & 0x20) && sg_tag(g, at, "VIDE") && at + 16 <= g->len)
        g->vide = at + 8;
    return 1;
}

static void sg_reset(void)
{
    u16 main = xread16(alis.basemain + 0x3a);
    alis.ferase = 0;
    for (u16 e; (e = xread16(alis.atent + main + 4)) != 0; )
        killent(e);
    
    for (u16 b = xread16(alis.atent + NODE_BLKHEAD + 0xe); b != NODE_BLKTAIL; )
    {
        u16 next = xread16(alis.atent + b + 0xe);
        sAlisScriptData *prog = node_prog(b);
        if (prog && prog != alis.main->data)
        {
            u32 end = alis.finprog;
            for (int i = 0; i < alis.nbprog; i++)
            {
                if (ATPROG(i) > prog->data_org && ATPROG(i) < end)
                    end = ATPROG(i);
            }
            
            shrinkprog(prog->data_org, end - prog->data_org, prog->header.id);
        }

        v6text_free(b);
        b = next;
    }

    for (int i = 0; i < 16; i++)
    {
        if (alis.scrnode[i].node && scrnode_get(alis.basemain + alis.scrnode[i].scr, alis.scrnode[i].field))
            io_mfree(alis.scrnode[i].handle);
        
        memset(&alis.scrnode[i], 0, sizeof(alis.scrnode[i]));
    }

    alis.filmnode = 0;

    u16 mprog = alis.main->data->node;
    for (int i = main / 0x18 + 1; i < alis.maxent; i++)
    {
        if (i * 0x18 != mprog)
            memset(alis.mem + alis.atent + i * 0x18, 0, 0x18);
    }
    
    xwrite16(alis.atent + 4, main);
    xwrite16(alis.atent + main + 4, 0);
    xwrite16(alis.atent + NODE_BLKHEAD + 0xe, mprog);
    xwrite16(alis.atent + mprog + 0xe, NODE_BLKTAIL);
    u16 first = main + 0x18, last = (alis.maxent - 2) * 0x18;  // node n-1 orphaned
    for (u16 n = first; n < last; n += 0x18)
    {
        xwrite16(alis.atent + n + 0x10, n + 0x18);
    }
    
    xwrite16(alis.atent + last + 0x10, last);
    alis.nodefree = first;
    alis.nodetail = last;
}

static void sg_entity(u32 X, const u8 *r, u32 fbase, u32 mainold, s32 dmain, int ismain)
{
    const u8 *c = r + 16;
    const u8 *d = c + 0x40;
    u32 sz = sg_rd32(r + 8) - 0x40;
    u16 node = xread16(X + 0x3a);
    u16 vsz = (u16)(xread32(alis.atent + node + 8) - xread16(alis.atent + node + 6));
    sAlisScriptLive *s = ENTSCR(node);
    u32 org = get_0x14_script_org_offset(X);
    u32 oldprog = sg_rd32(d - 0x0c);
    s32 dP = (s32)(org - oldprog);
    u32 psz = alis.finprog - org;
    for (int i = 0; i < alis.nbprog; i++)
        if (ATPROG(i) > org && ATPROG(i) - org < psz)
            psz = ATPROG(i) - org;

    memcpy(alis.mem + X, d, sz < vsz ? sz : vsz);
    xwrite8(X - 0x66, d[-0x1c]);
    xwrite16(X - 0x64, sg_rd16(d - 0x18));
    set_0x16_screen_id(X, sg_rd16(d - 0x16));
    set_0x3a_wait_cycles(X, sg_rd16(d - 0x12));
    set_0x3e_wait_time(X, 0);
    set_0x2b_cordspr(X, d[-0x03]);
    u8 f2 = d[-0x02];
    xwrite8(X - 0x67, (f2 & 0x0b) | 4);
    u8 s24 = (u8)get_0x24_scan_inter(X) & ~(0x80 | 1 | 4);
    set_0x24_scan_inter(X, s24 | (f2 & 4 ? 0 : 1) | (f2 & 0x10 ? 0 : 4));
    xwrite8(V6_FARSLEEP(X), 0);                     // -1 = 0
    if (get_0x04_cstart_csleep(X) == 0)
        set_0x04_cstart_csleep(X, 1);
    
    if (s)
        s->vram_org = X;

    // the entity takes back its saved id
    u16 want = xread16(X + 0x3a);
    if (want != node)
    {
        if (want % 0x18 == 0 && want > NODE_BLKTAIL && want / 0x18 < alis.maxent)
        {
            node_swap(node, want);                  // any node, as DOS
        }
        else
        {
            xwrite16(X + 0x3a, node);
        }
    }

    node = xread16(X + 0x3a);
    set_0x0e_script_ent(X, node);
    xwrite16(V6_CLINKING(X), node);

    if (ismain)                                     // reset the world render lists
    {
        for (u32 W = v6_worldhead(); W; )
        {
            u32 T = W + 0x58;
            xwrite32(W + 0x38, T); xwrite32(W + 0x34, 0); xwrite8(W + 0x22, 0);
            xwrite32(T + 0x34, W); xwrite32(T + 0x38, T); xwrite8(T + 0x22, 0);
            u32 nx = xread32(W + 0xb4);
            if (!nx)
                break;
            xwrite32(W + 0xb4, nx - fbase + alis.basemain);   // DOS keeps the old base here
            W = nx - fbase + alis.basemain;
        }
    }

    // elem chain and slots: vram delta dV, program delta dP
    u32 vold = sg_rd32(d - 0x2c) + 0x2c;
    s32 dV = (s32)(X - vold);
    u32 head = v6_elemhead(X), tail = v6_elemtail(X);
#define SG_LINK(p) ((p) == vold - 0x2c ? head : (p) == vold - 0x24 ? tail : (p) + dV)
    v6el_set_prev(head, head);
    v6el_set_next(head, SG_LINK(sg_rd32(d - 0x28)));
    v6el_set_prev(tail, SG_LINK(sg_rd32(d - 0x24)));
    v6el_set_next(tail, tail);
    for (u32 E = v6el_next(head), guard = 0; E != tail && guard < 0x1000; E = v6el_next(E), guard++)
    {
        if (E < X || E + 0x50 > X + vsz)
        {
            ALIS_DEBUG(EDebugError, "freadg: broken elem chain at %x\n", E);
            v6el_set_next(head, tail);
            v6el_set_prev(tail, head);
            break;
        }

        v6el_set_prev(E, SG_LINK(v6el_prev(E)));
        v6el_set_next(E, SG_LINK(v6el_next(E)));
        u32 sp = E;
        for (u8 i = xread8(E + 0x33); i != 0; i--, sp += V6_SLOT_STRIDE)
        {
            u8 st = v6sp_state(sp);
            for (int f = 0; f < 2; f++)
            {
                u32 off = f ? 0x50 : 0x3c;
                if (f ? (st != 0x30 && st != 0x3a) : st == 0x38)
                    continue;
                
                u32 p = xread32(sp + off);
                if (p >= oldprog && p - oldprog < psz)
                {
                    xwrite32(sp + off, p + dP);
                }
                else if (!ismain && mainold && p >= mainold && p - mainold < alis.main->data->sz)
                {
                    xwrite32(sp + off, p + dmain);
                }
            }

            if (xread32(sp + 0x08))
                xwrite32(sp + 0x08, xread32(sp + 0x08) + dV);
            
            if (xread32(sp + 0x34) && xread16(sp + 0x2c))
            {
                u32 W = alis.basemain + xread16(sp + 0x2c);
                u32 n = xread32(W + 0x38);
                xwrite32(W + 0x38, sp);
                xwrite32(n + 0x34, sp);
                xwrite32(sp + 0x34, W);
                xwrite32(sp + 0x38, n);
                xwrite8(sp + 0x23, xread8(sp + 0x23) | 1);
            }
        }
    }
#undef SG_LINK
}

static void sg_pal_in(u32 *pal, const u8 *in)
{
    for (int i = 0; i < 256; i++)
    {
        u8 r = in[i * 3] & 0x3f, g = in[i * 3 + 1] & 0x3f, b = in[i * 3 + 2] & 0x3f;
        PAL_WRITE_RGB(pal, i, (u8)(r << 2 | r >> 4), (u8)(g << 2 | g >> 4), (u8)(b << 2 | b >> 4));
    }
}

static void sg_load(u32 flags)
{
    fseek(alis.fp, 0, SEEK_END);
    long len = ftell(alis.fp);
    rewind(alis.fp);
    sSgFile g = {0};
    u8 *f = len > 0 ? malloc((size_t)len) : NULL;
    if (!f || fread(f, 1, (size_t)len, alis.fp) != (size_t)len)
    {
        free(f);
        return;
    }

    g.f = f;
    g.len = (u32)len;
    if (alis.script != alis.main || !sg_parse(&g, flags))
    {
        ALIS_DEBUG(EDebugError, "freadg: not a valid save game\n");
        free(f);
        return;
    }

    u32 scr0 = alis.basemain + (u16)xread16(alis.script->vram_org - 0x64);
    v6snd_stopchan(-1);
    sg_reset();
    xwrite32(scr0 + 0xb4, 0);
    xwrite32(scr0 + 0xbc, 0);

    u32 fbase = alis.basemain;
    if (g.syst)
    {
        fbase = sg_rd32(f + g.syst);
        for (int i = 0; i < 16; i++)
        {
            const u8 *r = f + g.syst + 4 + i * 0x28;
            alis.v6xfonts[i].id = (s32)sg_rd32(r);
            alis.v6xfonts[i].op2 = r[4];
            alis.v6xfonts[i].foasc = (s32)sg_rd32(r + 8);
            alis.v6xfonts[i].glyphslot = 0;
            alis.v6xfonts[i].count = (s32)sg_rd32(r + 0x10);
            alis.v6xfonts[i].height = (s32)sg_rd32(r + 0x14);
            alis.v6xfonts[i].spacing = (s32)sg_rd32(r + 0x18);
            alis.v6xfonts[i].owner = sg_rd16(r + 0x1c);
            alis.v6xfonts[i].form = (s16)sg_rd16(r + 0x1e);
            memcpy(alis.v6xfonts[i].ownername, r + 0x20, 8);
            alis.v6xfonts[i].valid = r[0x20] != 0;
            alis.v6fonts[i].foasc = alis.v6xfonts[i].foasc;
            alis.v6fonts[i].fonum = alis.v6xfonts[i].form;
            alis.v6fonts[i].fomax = alis.v6xfonts[i].count;
            alis.v6fonts[i].folarg = (u8)alis.v6xfonts[i].spacing;
            alis.v6fonts[i].fohaut = (u8)alis.v6xfonts[i].height;
            alis.v6fonts[i].mode = (u8)alis.v6xfonts[i].id;
            alis.v6fonts[i].usemain = memcmp(r + 0x20, "main\0\0\0\0", 8) == 0;
            alis.v6fonts[i].valid = alis.v6xfonts[i].valid;
        }
    }

    for (u32 at = g.prog; at && sg_rd32(f + at) != 0; at += 16)
    {
        char name[9] = {0};
        memcpy(name, f + at, 8);
        u8 t = f[at + 0xd];
        if (node_find(name) != 0)
            continue;
        if (t == 0xc)
        {
            v6text_load(name);
        }
        else if (t == 2)
        {
            char path[kPathMaxLen] = {0};
            snprintf(path, sizeof(path), "%s%s.%s", alis.platform.path, name, alis.platform.ext);
            strlower(path + strlen(alis.platform.path));
            script_load(path);
        }
        else
        {
            ALIS_DEBUG(EDebugWarning, "freadg: block %s of type %x not reloaded\n", name, t);
        }
    }

    u32 prev = alis.basemain;
    u32 mainold = 0;
    s32 dmain = 0;
    for (u32 at = g.enti; at && sg_rd32(f + at) != 0; at += 16 + sg_rd32(f + at + 8))
    {
        const u8 *r = f + at;
        char name[9] = {0};
        memcpy(name, r, 8);
        int ismain = memcmp(r, "main\0\0\0\0", 8) == 0;
        u32 X = alis.basemain;
        if (ismain)
        {
            mainold = sg_rd32(r + 16 + 0x40 - 0x0c);
            dmain = (s32)(alis.main->data->data_org - mainold);
        }
        else
        {
            sAlisScriptData *prog = node_prog(node_find(name));
            if (prog == NULL)
            {
                ALIS_DEBUG(EDebugError, "freadg: entity of missing program %s skipped\n", name);
                continue;
            }

            u16 pid = xread16(prev + 0x3a);
            sAlisScriptLive *caller = alis.script;
            s16 d5 = alis.varD5;
            alis.script = ENTSCR(pid);
            alis.varD5 = pid;
            sAlisScriptLive *e = script_live(prog);
            alis.script = caller;
            alis.varD5 = d5;
            if (e == NULL)
                continue;
            
            X = e->vram_org;
        }

        sg_entity(X, r, fbase, mainold, dmain, ismain);
        prev = X;
    }

    if (g.pale)
    {
        sg_pal_in(image.ampalet, f + g.pale);
        sg_pal_in(image.atpalet, f + g.pale + 0x300);
        sg_pal_in(image.svpalet, f + g.pale + 0x600);
        sg_pal_in(image.svpalet2, f + g.pale + 0x900);
        for (int i = 0; i < 0x300; i++)
            image.dkpalet[i] = (s16)sg_rd16(f + g.pale + 0xc00 + i * 2);
    }

    if (g.shad)
    {
        u32 D = alis.basemain + sg_rd32(f + g.shad);
        for (u16 o = screen.ptscreen; o != 0; o = xread16(alis.basemain + o + 0x5c))
        {
            u32 scr = alis.basemain + o;
            if (xread32(scr + 0x5e))
                xwrite32(scr + 0x5e, xread32(scr + 0x5e) - fbase + alis.basemain);
            
            if (xread8(scr + 0x5a) == 6)
            {
                xwrite32(scr + 0xb4, 0);
                xwrite32(scr + 0xbc, 0);
                if (!(xread8(scr + 0x58) & 0x40))
                    v6x3d_open(scr);
            }
        }

        u32 n = sg_rd32(f + g.shad - 4) - 4;
        u16 node = node_take(0xb);
        s32 h = io_malloc(n);
        if (h)
        {
            memcpy(alis.mem + xread32(h), f + g.shad + 4, n);
            xwrite32(D + 0xb4, h);
            if (node)
                scrnode_set(D, 0xb4, node);
        }

        u32 keep = xread32(scr0 + 0xb4);
        memcpy(alis.mem + scr0 + 0x94, f + g.shav + 4, 0x28);
        xwrite32(scr0 + 0xb4, keep);
    }

    if (g.vide)
    {
        u8 mode = f[g.vide];
        if (mode != alis.videomode)
        {
            alis.videomode = mode == 1;
            resize_screen(mode == 1 ? 640 : 320, mode == 1 ? 480 : 200);
        }
    }

    for (int i = 0; i < 16; i++)
    {
        if (!alis.v6xfonts[i].valid)
            continue;
        
        char name[9] = {0};
        memcpy(name, alis.v6xfonts[i].ownername, 8);
        sAlisScriptData *prog = node_prog(node_find(name));
        s16 form = alis.v6xfonts[i].form;
        if (prog && form >= 0 && (u16)form < xread16(prog->data_org + 0x3e))
            alis.v6xfonts[i].glyphslot = prog->data_org + 0x40 + form * 4;
    }

    image.ftopal = 0xff;
    free(f);
}

// Codesc1name no. 040 opcode 0x27 cfreadg
static void cfreadg(void)
{
    readexec_opername();
    if (alis.platform.version >= 60 && alis.platform.kind != EPlatform3DO && alis.fp)
        sg_load((u32)alis.varD7);
}

// Codesc1name no. 041 opcode 0x28 cobjmat
static void cobjmat(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 042 opcode 0x29 cobjmask
static void cobjmask(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 043 opcode 0x2a cscclut
static void cscclut(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 044 opcode 0x2b cobjclut
static void cobjclut(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 045 opcode 0x2c copennet
static void copennet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 046 opcode 0x2d cclosenet
static void cclosenet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 047 opcode 0x2e csendnet
static void csendnet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 048 opcode 0x2f cmailnet
static void cmailnet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 049 opcode 0x30 cplayernet
static void cplayernet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 050 opcode 0x31 cgamenet
static void cgamenet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 051 opcode 0x32 csyncnet
static void csyncnet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 052 opcode 0x33 ctexitem
static void ctexitem(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 053 opcode 0x34 cgetanim
static void cgetanim(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 054 opcode 0x35 canimsize
static void canimsize(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 055 opcode 0x36 cscmath
static void cscmath(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 056 opcode 0x37 cintermode
static void cintermode(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 057 opcode 0x38 csysnet
static void csysnet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 058 opcode 0x39 cobjshad
static void cobjshad(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 059 opcode 0x3a csphere
static void csphere(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 060 opcode 0x3b cscdark
static void cscdark(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 061 opcode 0x3c cagetlocal
static void cagetlocal(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 062 opcode 0x3d cfollow
static void cfollow(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 063 opcode 0x3e cplayfilm
static void cplayfilm(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 064 opcode 0x3f csoundtime
static void csoundtime(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 065 opcode 0x40 cinfoitem
static void cinfoitem(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 066 opcode 0x41 cmorph
static void cmorph(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 067 opcode 0x42 cscmicro
static void cscmicro(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 068 opcode 0x43 cptrset
static void cptrset(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 069 opcode 0x44 cdefproc
static void cdefproc(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 070 opcode 0x45 cfreadm
static void cfreadm(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 071 opcode 0x46 cfwritem
static void cfwritem(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 072 opcode 0x47 cmaskent
static void cmaskent(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 073 opcode 0x48 cvram
static void cvram(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 074 opcode 0x49 cadresnet
static void cadresnet(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 075 opcode 0x4a cfdir
static void cfdir(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 076 opcode 0x4b cnul
// Codesc1name no. 077 opcode 0x4c cloadas
static void cloadas(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 078 opcode 0x4d cfindobjitem
static void cfindobjitem(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 079 opcode 0x4e cobjid
static void cobjid(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 080 opcode 0x4f cgetobjid
static void cgetobjid(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 081 opcode 0x50 cprelight
static void cprelight(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 082 opcode 0x51 croomdoor
static void croomdoor(void)
{
    ALIS_DEBUG(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// =============================================================================
#pragma mark - Oper (Operation) routines
// =============================================================================

// Opername no. 029 opcode 0x38 oeval
// Calls oper->oeval (from old version)

// Opername no. 030 opcode 0x3a ofin
// Calls oper->ofin (from old version)


// ============================================================================
#pragma mark - Store routines
// ============================================================================

// Storename no. 001 opcode 0x00 pnul
// Calls stub pnul (from old version)

// Storename no. 002 opcode 0x02 pnul
// Calls stub pnul (from old version)

// Storename no. 003 opcode 0x04 pnul
// Calls stub pnul (from old version)

// Storename no. 028 opcode 0x36 spile
// Calls store->spile (from old version)


// Storename no. 030 opcode 0x3a ofin
// Calls oper->ofin (from old version)

// ============================================================================
#pragma mark - Add routines
// ============================================================================

// Addname no. 001 opcode 0x00 pnul
// Calls stub pnul (from old version)

// Addname no. 002 opcode 0x02 pnul
// Calls stub pnul (from old version)

// Addname no. 003 opcode 0x04 pnul
// Calls stub pnul (from old version)

// Addname no. 028 opcode 0x36 spile
// Calls store->spile (from old version)


// Addname no. 030 opcode 0x3a ofin
// Calls oper->ofin (from old version)

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines pointer table
// ============================================================================
#pragma mark - Codesc1 routines pointer table
// ============================================================================
const sAlisOpcode codesc1names_v40[] = {
    DECL_OPCODE(0x00, cnul,         "[N/I] null"),
    DECL_OPCODE(0x01, csoundon,     "sound on"),
    DECL_OPCODE(0x02, csoundoff,    "sound off"),
    DECL_OPCODE(0x03, cmusicon,     "music on"),
    DECL_OPCODE(0x04, cmusicoff,    "music off"),
    DECL_OPCODE(0x05, cdelfilm,     "close and delete video from memory"),
    DECL_OPCODE(0x06, copenfilm,    "open video"),
    DECL_OPCODE(0x07, cdeffont,     "TODO: add desc"),   // tcodesc1_07: + cdeffont
    DECL_OPCODE(0x08, cloadtext,    "TODO: add desc"),   // tcodesc1_08: + cloadtext
    DECL_OPCODE(0x09, cafteron,     "TODO: add desc"),   // tcodesc1_09: + cafteron
    DECL_OPCODE(0x0a, cafteroff,    "TODO: add desc"),   // tcodesc1_0A: + cafteroff
    DECL_OPCODE(0x0b, cbeforon,     "TODO: add desc"),   // tcodesc1_0B: + cbeforon
    DECL_OPCODE(0x0c, cbeforoff,    "TODO: add desc"),   // tcodesc1_0C: + cbeforoff
    DECL_OPCODE(0x0d, cscfocale,    "TODO: add desc"),   // tcodesc1_0D: + cscfocale
    DECL_OPCODE(0x0e, csccamera,    "TODO: add desc"),   // tcodesc1_0E: + csccamera
    DECL_OPCODE(0x0f, ctexmap,      "TODO: add desc"),   // tcodesc1_0F: + ctexmap
    DECL_OPCODE(0x10, cscrender,    "TODO: add desc"),   // tcodesc1_10: + cscrender
    DECL_OPCODE(0x11, cvideo,       "TODO: add desc"),   // tcodesc1_11: + cvideo
    DECL_OPCODE(0x12, ctexture,     "TODO: add desc"),   // tcodesc1_12: + ctexture
    DECL_OPCODE(0x13, cplaycd,      "TODO: add desc"),   // tcodesc1_13: + cplaycd
    DECL_OPCODE(0x14, camovnat,     "add raw angles to entity"),    // DOS @00008732 (was cnul = drift)
    DECL_OPCODE(0x15, csetvectnat,  "add raw angles to elem"),      // DOS @000081c9 (v40 name was cobjtrans)
    DECL_OPCODE(0x16, conscreen,    "TODO: add desc"),   // tcodesc1_16: + conscreen
    DECL_OPCODE(0x17, cgetglobal,   "TODO: add desc"),   // tcodesc1_17: + cgetglobal
    DECL_OPCODE(0x18, cgetlocal,    "TODO: add desc"),   // tcodesc1_18: + cgetlocal
    DECL_OPCODE(0x19, cmatitem,     "TODO: add desc"),   // tcodesc1_19: + cmatitem
    DECL_OPCODE(0x1a, cmaskitem,    "TODO: add desc"),   // tcodesc1_1A: + cmaskitem
    DECL_OPCODE(0x1b, cdefitem,     "TODO: add desc"),   // tcodesc1_1B: + cdefitem
    DECL_OPCODE(0x1c, casetnat,     "set raw angles on entity"),    // DOS @00008752 (was cnul = drift)
    DECL_OPCODE(0x1d, cdefvectnat,  "set raw angles on elem"),      // DOS @00008206 (was cnul = drift)
    DECL_OPCODE(0x1e, cgetmap,      "TODO: add desc"),   // tcodesc1_1E: + cgetmap
    DECL_OPCODE(0x1f, cconvmap,     "TODO: add desc"),   // tcodesc1_1F: + cconvmap
    DECL_OPCODE(0x20, csetfilm,     "TODO: add desc"),   // tcodesc1_20: + csetfilm
    DECL_OPCODE(0x21, cscmem,       "TODO: add desc"),   // tcodesc1_21: + cscmem
    DECL_OPCODE(0x22, cmapcell,     "3DO: land cell"),   // DOS entry = bare RET @00005485 (v40 name was cobjorient)
    DECL_OPCODE(0x23, cobjaim,      "TODO: add desc"),   // tcodesc1_23: + cobjaim
    DECL_OPCODE(0x24, cloadpic,     "TODO: add desc"),   // tcodesc1_24: + cloadpic
    DECL_OPCODE(0x25, cputpic,      "TODO: add desc"),   // tcodesc1_25: + cputpic
    DECL_OPCODE(0x26, cfwriteg,     "TODO: add desc"),   // tcodesc1_26: + cfwriteg
    DECL_OPCODE(0x27, cfreadg,      "TODO: add desc"),   // tcodesc1_27: + cfreadg
    DECL_OPCODE(0x28, cobjmat,      "TODO: add desc"),   // tcodesc1_28: + cobjmat
    DECL_OPCODE(0x29, cobjmask,     "TODO: add desc"),   // tcodesc1_29: + cobjmask
    DECL_OPCODE(0x2a, cscclut,      "TODO: add desc"),   // tcodesc1_2A: + cscclut
    DECL_OPCODE(0x2b, cobjclut,     "TODO: add desc"),   // tcodesc1_2B: + cobjclut
    DECL_OPCODE(0x2c, copennet,     "TODO: add desc"),   // tcodesc1_2C: + copennet
    DECL_OPCODE(0x2d, cclosenet,    "TODO: add desc"),   // tcodesc1_2D: + cclosenet
    DECL_OPCODE(0x2e, csendnet,     "TODO: add desc"),   // tcodesc1_2E: + csendnet
    DECL_OPCODE(0x2f, cmailnet,     "TODO: add desc"),   // tcodesc1_2F: + cmailnet
    DECL_OPCODE(0x30, cplayernet,   "TODO: add desc"),   // tcodesc1_30: + cplayernet
    DECL_OPCODE(0x31, cgamenet,     "TODO: add desc"),   // tcodesc1_31: + cgamenet
    DECL_OPCODE(0x32, csyncnet,     "TODO: add desc"),   // tcodesc1_32: + csyncnet
    DECL_OPCODE(0x33, ctexitem,     "TODO: add desc"),   // tcodesc1_33: + ctexitem
    DECL_OPCODE(0x34, cgetanim,     "TODO: add desc"),   // tcodesc1_34: + cgetanim
    DECL_OPCODE(0x35, canimsize,    "TODO: add desc"),   // tcodesc1_35: + canimsize
    DECL_OPCODE(0x36, cscmath,      "TODO: add desc"),   // tcodesc1_36: + cscmath
    DECL_OPCODE(0x37, cintermode,   "TODO: add desc"),   // tcodesc1_37: + cintermode
    DECL_OPCODE(0x38, csysnet,      "TODO: add desc"),   // tcodesc1_38: + csysnet
    DECL_OPCODE(0x39, cobjshad,     "TODO: add desc"),   // tcodesc1_39: + cobjshad
    DECL_OPCODE(0x3a, csphere,      "TODO: add desc"),   // tcodesc1_3A: + csphere
    DECL_OPCODE(0x3b, cscdark,      "TODO: add desc"),   // tcodesc1_3B: + cscdark
    DECL_OPCODE(0x3c, cagetlocal,   "TODO: add desc"),   // tcodesc1_3C: + cagetlocal
    DECL_OPCODE(0x3d, cfollow,      "TODO: add desc"),   // tcodesc1_3D: + cfollow
    DECL_OPCODE(0x3e, cplayfilm,    "TODO: add desc"),   // tcodesc1_3E: + cplayfilm
    DECL_OPCODE(0x3f, csoundtime,   "TODO: add desc"),   // tcodesc1_3F: + csoundtime
    DECL_OPCODE(0x40, cinfoitem,    "TODO: add desc"),   // tcodesc1_40: + cinfoitem
    DECL_OPCODE(0x41, cmorph,       "TODO: add desc"),   // tcodesc1_41: + cmorph
    DECL_OPCODE(0x42, cscmicro,     "TODO: add desc"),   // tcodesc1_42: + cscmicro
    DECL_OPCODE(0x43, cptrset,      "TODO: add desc"),   // tcodesc1_43: + cptrset
    DECL_OPCODE(0x44, cdefproc,     "TODO: add desc"),   // tcodesc1_44: + cdefproc
    DECL_OPCODE(0x45, cfreadm,      "TODO: add desc"),   // tcodesc1_45: + cfreadm
    DECL_OPCODE(0x46, cfwritem,     "TODO: add desc"),   // tcodesc1_46: + cfwritem
    DECL_OPCODE(0x47, cmaskent,     "TODO: add desc"),   // tcodesc1_47: + cmaskent
    DECL_OPCODE(0x48, cvram,        "TODO: add desc"),   // tcodesc1_48: + cvram
    DECL_OPCODE(0x49, cadresnet,    "TODO: add desc"),   // tcodesc1_49: + cadresnet
    DECL_OPCODE(0x4a, cfdir,        "TODO: add desc"),   // tcodesc1_4A: + cfdir
    DECL_OPCODE(0x4b, cnul,         "[N/I] null"),       // tcodesc1_4B: + cnul ; entitem
    DECL_OPCODE(0x4c, cloadas,      "TODO: add desc"),   // tcodesc1_4C: + cloadas
    DECL_OPCODE(0x4d, cfindobjitem, "TODO: add desc"),   // tcodesc1_4D: + cfindobjitem
    DECL_OPCODE(0x4e, cobjid,       "TODO: add desc"),   // tcodesc1_4E: + cobjid
    DECL_OPCODE(0x4f, cgetobjid,    "TODO: add desc"),   // tcodesc1_4F: + cgetobjid
    DECL_OPCODE(0x50, cprelight,    "TODO: add desc"),   // tcodesc1_50: + cprelight
    DECL_OPCODE(0x51, croomdoor,    "TODO: add desc")    // tcodesc1_51: + croomdoor
};

// =============================================================================
#pragma mark - Oper (Operation) routines pointer table
// ============================================================================
#pragma mark - Store routines pointer table
// ============================================================================
#pragma mark - Add routines pointer table
