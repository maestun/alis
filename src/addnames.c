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
#include "image.h"
#include "mem.h"

// ============================================================================
#pragma mark - Add routines
// ============================================================================

// Addname no. 01 opcode 0x00 pnul
// Addname no. 02 opcode 0x02 pnul
// Addname no. 03 opcode 0x04 pnul
// Calls stub pnul

// Addname no. 04 opcode 0x06 alocb
static void alocb(void)
{
    s16 offset = script_read16();
    xadd8(alis.script->vram_org + offset, (u8)alis.varD7);
    alis.sr.zero = (xread8(alis.script->vram_org + offset) == 0);
}

// Addname no. 05 opcode 0x08 alocw
static void alocw(void)
{
    s16 offset = script_read16();
    xadd16(alis.script->vram_org + offset, alis.varD7);
    alis.sr.zero = (xread16(alis.script->vram_org + offset) == 0);
}

// Addname no. 06 opcode 0x0a alocp
static void alocp(void)
{
    s16 offset = script_read16();
    strcat((char *)get_vram(offset), (char *)alis.oldsd7);
}

// Addname no. 07 opcode 0x0c aloctp
static void aloctp(void)
{
    s32 addr = tabstring(alis.script->vram_org + script_read16());
    strcat((char *)(alis.mem + addr), (char *)alis.oldsd7);
}

// Addname no. 08 opcode 0x0e aloctc
static void aloctc(void)
{
    s32 addr = tabchar(alis.script->vram_org + script_read16());
    alis.varD7 = *alis.acc++;
    xadd8(addr, (char)alis.varD7);
}

// Addname no. 09 opcode 0x10 alocti
static void alocti(void)
{
    s32 addr = tabint(alis.script->vram_org + script_read16());
    alis.varD7 = *alis.acc++;
    xadd16(addr, alis.varD7);
}

// Addname no. 10 opcode 0x12 adirb
static void adirb(void)
{
    u8 offset = script_read8();
    xadd8(alis.script->vram_org + offset, (u8)alis.varD7);
    alis.sr.zero = (xread8(alis.script->vram_org + offset) == 0);
}

static void adirb_v6(void)
{
    u16 offset = (u16)script_read16();
    xadd32(alis.script->vram_org + offset, (s32)alis.varD7);
    alis.sr.zero = (xread32(alis.script->vram_org + offset) == 0);
}

// Addname no. 11 opcode 0x14 adirw
static void adirw(void)
{
    u8 offset = script_read8();
    xadd16(alis.script->vram_org + offset, alis.varD7);
    alis.sr.zero = (xread16(alis.script->vram_org + offset) == 0);
}

static void adirw_v6(void)
{
    u16 offset = (u16)script_read16();
    s32 addr = v6_tabcell(alis.script->vram_org + offset, 2);
    alis.varD7 = *alis.acc++;
    xadd32(addr, (s32)alis.varD7);
}

// adirl was between adirw and adirp. Robinson's Requiem/Ishar 3 have a stub

// Addname no. 12 opcode 0x16 adirp
static void adirp(void)
{
    u8 offset = script_read8();
    strcat((char *)get_vram(offset), (char *)alis.oldsd7);
}

static void adirp_v6(void)
{
    u16 offset = (u16)script_read16();
    xadd32(alis.basemain + offset, (s32)alis.varD7);
}

// Addname no. 13 opcode 0x18 adirtp
static void adirtp(void)
{
    s32 addr = tabstring(alis.script->vram_org + script_read8());
    strcat((char *)(alis.mem + addr), (char *)alis.oldsd7);
}

static void adirtp_v6(void)
{
    u16 offset = (u16)script_read16();
    s32 addr = v6_tabcell(alis.basemain + offset, 1);
    alis.varD7 = *alis.acc++;
    xadd32(addr, (s32)alis.varD7);
}

// Addname no. 14 opcode 0x1a adirtc
static void adirtc(void)
{
    s32 addr = tabchar(alis.script->vram_org + script_read8());
    alis.varD7 = *alis.acc++;
    xadd8(addr, (char)alis.varD7);
}

static void adirtc_v6(void)
{
    u16 offset = (u16)script_read16();
    u16 ent = (u16)xread16(alis.script->vram_org + offset);
    s32 vram = xread32(alis.atent + ent);
    if (vram <= 0)
        return;

    u16 offset2 = (u16)script_read16();
    xadd32(vram + offset2, (s32)alis.varD7);
}

// Addname no. 15 opcode 0x1c adirti
static void adirti(void)
{
    s32 addr = tabint(alis.script->vram_org + script_read8());
    alis.varD7 = *alis.acc++;
    xadd16(addr, alis.varD7);
}

static void adirti_v6(void)
{
    u16 celloff = (u16)script_read16();
    u16 entoff = (u16)script_read16();
    s16 ent = xread16(alis.script->vram_org + entoff);
    s32 vram = xread32(alis.atent + ent);
    s32 addr = v6_tabcell(vram + celloff, 1);
    alis.varD7 = *alis.acc++;
    xadd32(addr, (s32)alis.varD7);
}

// Addname no. 16 opcode 0x1e amainb
static void amainb(void)
{
    s16 offset = script_read16();
    xadd8(alis.basemain + offset, (u8)alis.varD7);
}

// Addname no. 17 opcode 0x20 amainw
static void amainw(void)
{
    s16 offset = script_read16();
    xadd16(alis.basemain + offset, (s16)alis.varD7);
}

// Addname no. 18 opcode 0x22 amainp
static void amainp(void)
{
    s16 offset = script_read16();
    strcat((char *)(alis.mem + alis.basemain + offset), (char *)alis.oldsd7);
}

// Addname no. 19 opcode 0x24 amaintp
static void amaintp(void)
{
    s32 addr = tabstring(alis.basemain + script_read16());
    strcat((char *)(alis.mem + addr), (char *)alis.oldsd7);
}

// Addname no. 20 opcode 0x26 amaintc
static void amaintc(void)
{
    s32 addr = tabchar(alis.basemain + script_read16());
    alis.varD7 = *alis.acc++;
    xadd8(addr, (u8)alis.varD7);
}

// Addname no. 21 opcode 0x28 amainti
static void amainti(void)
{
    s32 addr = tabint(alis.basemain + script_read16());
    alis.varD7 = *alis.acc++;
    xadd16(addr, alis.varD7);
}

// Addname no. 22 opcode 0x2a ahimb
static void ahimb(void)
{
    if (alis.platform.version == 10)
    {
        s16 offset = xread16(alis.script->vram_org + script_read16());
        s32 addr = xread32(alis.atent + offset) + script_read16();
        xwrite8(addr, xread8(addr) + (u8)alis.varD7);
        return;
    }
    
    ALIS_DEBUG(EDebugInfo, "MISSING: ", __FUNCTION__);
}

static void ahimb_v6(void)
{
    u16 offset = (u16)script_read16();
    u16 ent = (u16)xread16(alis.script->vram_org + offset);
    s32 vram = xread32(alis.atent + ent);
    if (vram <= 0)
        return;

    u16 offset2 = (u16)script_read16();
    xadd8(vram + offset2, (u8)alis.varD7);
}

// Addname no. 23 opcode 0x2c ahimw
static void ahimw(void)
{
    s16 offset = xread16(alis.script->vram_org + script_read16());
    s32 offset2 = xread32(alis.atent + offset) + script_read16();
    xadd16(offset2, alis.varD7);
}

// Addname no. 24 opcode 0x2e ahimp
static void ahimp(void)
{
    ALIS_DEBUG(EDebugInfo, "MISSING: ", __FUNCTION__);
}

static void ahimp_v6(void)
{
    u16 offset = (u16)script_read16();
    u16 ent = (u16)xread16(alis.script->vram_org + offset);
    s32 vram = xread32(alis.atent + ent);
    u16 offset2 = (u16)script_read16();
    strcat((char *)(alis.mem + vram + offset2), (char *)alis.oldsd7);
}

// Addname no. 25 opcode 0x30 ahimtp
static void ahimtp(void)
{
    s16 offset1 = script_read16();
    s16 offset2 = script_read16();
    s32 addr = xread32(alis.atent + xread16(alis.script->vram_org + offset2));
    addr = tabstring(addr + offset1);
    strcat((char *)(alis.mem + addr), (char *)alis.oldsd7);
}

// Addname no. 26 opcode 0x32 ahimtc
static void ahimtc(void)
{
    ALIS_DEBUG(EDebugInfo, "MISSING: ", __FUNCTION__);
}

static void ahimtc_v6(void)
{
    s16 celloff = script_read16();
    u16 entoff = (u16)script_read16();
    s16 ent = xread16(alis.script->vram_org + entoff);
    s32 vram = xread32(alis.atent + ent);
    s32 addr = v6_tabcell(vram + celloff, 0);
    alis.varD7 = *alis.acc++;
    xadd8(addr, (u8)alis.varD7);
}

// Addname no. 27 opcode 0x34 ahimti
static void ahimti(void)
{
    ALIS_DEBUG(EDebugInfo, "MISSING: ", __FUNCTION__);
}

static void ahimti_v6(void)
{
    u16 celloff = (u16)script_read16();
    u16 entoff = (u16)script_read16();
    s16 ent = xread16(alis.script->vram_org + entoff);
    s32 vram = xread32(alis.atent + ent);
    s32 addr = v6_tabcell(vram + celloff, 1);
    alis.varD7 = *alis.acc++;
    xadd16(addr, alis.varD7);
}

// Addname no. 28 opcode 0x36 spile
// Calls store->spile

// Addname no. 29 opcode 0x38 aeval
static void aeval(void)
{
    if (alis.platform.version >= 60)
        alis.acc = alis.acc_org;

    *(--alis.acc) = alis.varD7;
    oeval();
    readexec_addname();
}

// Addname no. 30 opcode 0x3a ofin
// Storename no. 30 opcode 0x3a ofin
// Calls oper->ofin

// ============================================================================
#pragma mark - Add routines pointer table
// ============================================================================
const sAlisOpcode addnames[] = {
    DECL_OPCODE(0x00, pnul,         "[N/I] null pointer"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x02, pnul,         "[N/I] null pointer"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x04, pnul,         "[N/I] null pointer"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x06, alocb,        "reads word (offset) from script, adds byte from d7 to byte at (vram+offset)"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x08, alocw,        "reads word (offset) from script, adds word from d7 to word at (vram+offset)"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x0a, alocp,        "read word (offset) from script, concatenate null-terminated string at ARRAY_A to null-terminated string located at (vram+offset)"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x0c, aloctp,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x0e, aloctc,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x10, alocti,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x12, adirb,        "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x14, adirw,        "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x16, adirp,        "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x18, adirtp,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x1a, adirtc,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x1c, adirti,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x1e, amainb,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x20, amainw,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x22, amainp,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x24, amaintp,      "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x26, amaintc,      "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x28, amainti,      "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x2a, ahimb,        "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x2c, ahimw,        "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x2e, ahimp,        "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x30, ahimtp,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x32, ahimtc,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x34, ahimti,       "TODO add desc"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x36, spile,        "pops word from virtual accumulator into r7 (calls store->spile)"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x38, aeval,        "starts aeval loop"),
    DECL_OPCODE(0, NULL, ""),
    DECL_OPCODE(0x3a, ofin,         "ends aeval loop (calls oper->ofin)"),
    DECL_OPCODE(0, NULL, "")
};

// ============================================================================
#pragma mark - v6 add table
// ============================================================================

static sAlisOpcode addnames_v6[256];
static const sAlisOpcode addnames_v6_ov[] = {
    DECL_OPCODE(0x12, adirb_v6,     "v6: add-store local long"),
    DECL_OPCODE(0x14, adirw_v6,     "v6: add-store loctab long"),
    DECL_OPCODE(0x16, adirp_v6,     "v6: add-store main long"),
    DECL_OPCODE(0x18, adirtp_v6,    "v6: add-store maintab long"),
    DECL_OPCODE(0x1a, adirtc_v6,    "v6: add-store maintab char"),
    DECL_OPCODE(0x1c, adirti_v6,    "v6: add-store maintab int"),
    DECL_OPCODE(0x2a, ahimb_v6,     "v6: add-store him byte"),
    DECL_OPCODE(0x2e, ahimp_v6,     "v6: add-store him long"),
    DECL_OPCODE(0x32, ahimtc_v6,    "v6: add-store himtab char"),
    DECL_OPCODE(0x34, ahimti_v6,    "v6: add-store himtab int"),
};

const sAlisOpcode *alis_tab_addnames(void)
{
    static const sAlisOpcode *live = NULL;
    if (live == NULL)
    {
        if (alis.platform.version < 60)
        {
            live = addnames;
        }
        else
        {
            memcpy(addnames_v6, addnames, sizeof(addnames));
            for (u32 i = 0; i < sizeof(addnames_v6_ov) / sizeof(addnames_v6_ov[0]); i++)
                addnames_v6[(u8)addnames_v6_ov[i].code] = addnames_v6_ov[i];
            live = addnames_v6;
        }
    }

    return live;
}
