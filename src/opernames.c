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
#include "image.h"
#include "mem.h"
#include "utils.h"


void pop_sd6(void)
{
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)alis.acc, (u8 *)alis.acc - alis.mem);
    
    char *src = (char *)alis.acc;
    strcpy(alis.sd6, src);

    s32 len = (s32)strlen(src) + 1;
    if (len % 2 == 1)
    {
        src++;
    }
    
    src += len;
    alis.acc = (s16 *)src;
}

void pop_sd7(void)
{
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)alis.acc, (u8 *)alis.acc - alis.mem);
    
    char *src = (char *)alis.acc;
    strcpy(alis.sd7, src);

    s32 len = (s32)strlen(src) + 1;
    if (len % 2 == 1)
        src++;
    
    src += len;
    alis.acc = (s16 *)src;
}

void push_sd6(void)
{
    char *tgt = (char *)alis.acc;

    s32 len = (s32)strlen(alis.sd6) + 1;
    if (len % 2 == 1)
        *(--tgt) = 0;

    tgt -= len;
    strcpy(tgt, alis.sd6);
    alis.acc = (s16 *)tgt;
}

void push_sd7(void)
{
    char *tgt = (char *)alis.acc;

    s32 len = (s32)strlen(alis.sd7) + 1;
    if (len % 2 == 1)
        *(--tgt) = 0;

    tgt -= len;
    strcpy(tgt, alis.sd7);
    alis.acc = (s16 *)tgt;
}

// =============================================================================
#pragma mark - TODO: opernames
// =============================================================================

// Opername no. 01 opcode 0x00 oimmb
void oimmb(void) {
    // reads a byte, extends into r7
    alis.varD7 = (s8)script_read8();
}

// Opername no. 02 opcode 0x02 oimmw
void oimmw(void) {
    // reads a word into r7
    alis.varD7 = script_read16();
}

// Opername no. 03 opcode 0x04 oimmp
void oimmp(void) {
    // reads null-terminated data into bssChunk3
    script_read_until_zero(alis.sd7);
    debug(EDebugVerbose, " [\"%s\" <= sd7]", alis.sd7);
}

// Opername no. 04 opcode 0x06 olocb
void olocb(void) {
    // read word offset, copy extended byte from ram[offset] into r7
    s16 offset = script_read16();
    alis.varD7 = (s8)xread8(alis.script->vram_org + offset);
}

// Opername no. 05 opcode 0x08 olocw
void olocw(void) {
    // read word offset, copy word from ram[offset] into r7
    s16 offset = script_read16();
    alis.varD7 = xread16(alis.script->vram_org + offset);
}

// Opername no. 06 opcode 0x0a olocp
void olocp(void) {
    s16 offset = script_read16();
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)get_vram(offset), alis.script->vram_org + offset);
    strcpy(alis.sd7, (char *)get_vram(offset));
}

// Opername no. 07 opcode 0x0c oloctp
void oloctp(void) {
    s32 addr = tabstring(alis.script->vram_org + script_read16());
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + addr), addr);
    strcpy(alis.sd7, (char *)(alis.mem + addr));
}

// Opername no. 08 opcode 0x0e oloctc
void oloctc(void) {
    s32 addr = tabchar(alis.script->vram_org + script_read16());
    alis.varD7 = (s8)xread8(addr);
}

// Opername no. 09 opcode 0x10 olocti
void olocti(void) {
    s32 addr = tabint(alis.script->vram_org + script_read16());
    alis.varD7 = xread16(addr);
}

// Opername no. 10 opcode 0x12 odirb
// reads a byte offset from script,
// then reads an extended byte from vram[offset] into r7
void odirb(void) {
    u8 offset = script_read8();
    alis.varD7 = (s8)xread8(alis.script->vram_org + offset);
}

// Opername no. 11 opcode 0x14 odirw
// reads a byte offset from script,
// then reads a word from vram[offset] into r7
void odirw(void) {
    u8 offset = script_read8();
    alis.varD7 = xread16(alis.script->vram_org + offset);
}

// Opername no. 12 opcode 0x16 odirp
// reads a byte offset from script,
// then reads a null-terminated data stream from vram[offset] into bssChunk3
void odirp(void) {
    u8 offset = script_read8();
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)get_vram(offset), alis.script->vram_org + offset);
    strcpy(alis.sd7, (char *)get_vram(offset));
}

// Opername no. 13 opcode 0x18 odirtp
void odirtp(void) {
    s32 addr = tabstring(alis.script->vram_org + script_read8());
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + addr), addr);
    strcpy(alis.sd7, (char *)(alis.mem + addr));
}

// Opername no. 14 opcode 0x1a odirtc
void odirtc(void) {
    s32 addr = tabchar(alis.script->vram_org + script_read8());
    alis.varD7 = (s8)xread8(addr);
}

// Opername no. 15 opcode 0x1c odirti
void odirti(void) {
    s32 addr = tabint(alis.script->vram_org + script_read8());
    alis.varD7 = xread16(addr);
}

// Opername no. 16 opcode 0x1e omainb
void omainb(void) {
    s16 offset = script_read16();
    alis.varD7 = (s8)xread8(alis.basemain + offset);
}

// Opername no. 17 opcode 0x20 omainw
void omainw(void) {
    s16 offset = script_read16();
    alis.varD7 = xread16(alis.basemain + offset);
}

// Opername no. 18 opcode 0x22 omainp
void omainp(void) {
    s16 offset = script_read16();
    debug(EDebugVerbose, " [%s <= %.6x]", (char *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    strcpy(alis.sd7, (char *)(alis.mem + alis.basemain + offset));
}

// Opername no. 19 opcode 0x24 omaintp
void omaintp(void) {
    s32 addr = tabstring(alis.basemain + script_read16());
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + addr), addr);
    strcpy(alis.sd7, (char *)(alis.mem + addr));
}

// Opername no. 20 opcode 0x26 omaintc
void omaintc(void) {
    s32 addr = tabchar(alis.basemain + script_read16());
    alis.varD7 = (s8)xread8(addr);
}

// Opername no. 21 opcode 0x28 omainti
void omainti(void) {
    s32 addr = tabint(alis.basemain + script_read16());
    alis.varD7 = xread16(addr);
}

// Opername no. 22 opcode 0x2a ohimb
void ohimb(void) {
    u16 entry = xread16(alis.script->vram_org + (s16)script_read16());
    u32 vram_addr = xread32(alis.atent + entry);

    s16 offset = script_read16();
    alis.varD7 = (s8)xread8(vram_addr + offset);
}

// Opername no. 23 opcode 0x2c ohimw
void ohimw(void) {
    u16 entry = xread16(alis.script->vram_org + (s16)script_read16());
    u32 vram_addr = xread32(alis.atent + entry);

    s16 offset = script_read16();
    alis.varD7 = xread16(vram_addr + offset);
}

// Opername no. 24 opcode 0x2e ohimp
void ohimp(void) {
    u16 entry = xread16(alis.script->vram_org + (s16)script_read16());
    u32 vram_addr = xread32(alis.atent + entry);

    s16 offset = script_read16();
    debug(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + vram_addr + offset), vram_addr + offset);
    strcpy(alis.sd7, (char *)(alis.mem + vram_addr + offset));
}

// Opername no. 25 opcode 0x30 ohimtp
void ohimtp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 26 opcode 0x32 ohimtc
void ohimtc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 27 opcode 0x34 ohimti
void ohimti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 28 opcode 0x36 opile
// pop from accumulator into r6
void opile(void) {
    // save r6 into r7
    alis.varD7 = alis.varD6;
    alis.varD6 = *alis.acc++;
}

// Opername no. 29 opcode 0x38 oeval
// start eval loop, will stop after ofin() is called
void oeval(void) {
    alis.oeval_loop = 1;
    while(alis.oeval_loop) {
        readexec_opername();
    }
    
    debug(EDebugInfo, " [ = %d]", (s8)alis.varD7);
}

// Opername no. 30 opcode 0x3a ofin
// stop eval loop
void ofin(void) {
    alis.oeval_loop = 0;
}

// Opername no. 33 opcode 0x40 opushacc
// push value from r7 register to accumulator
void opushacc(void) {
    *(--alis.acc) = alis.varD7;
}

// Opername no. 34 opcode 0x42 oand
// r7 = variable AND r7
void oand(void) {
    alis.varD6 = alis.varD7;
    readexec_opername();
    alis.varD7 &= alis.varD6;
}

// Opername no. 35 opcode 0x44 oor
// r7 = variable OR r7
void oor(void) {
    alis.varD6 = alis.varD7;
    readexec_opername();
    alis.varD7 |= alis.varD6;
}

// Opername no. 36 opcode 0x46 oxor
// r7 = variable XOR r7
void oxor(void) {
    readexec_opername_saveD7();
    alis.varD7 ^= alis.varD6;
}

// Opername no. 37 opcode 0x48 oeqv
// r7 = variable EQV r7
void oeqv(void) {
    readexec_opername_saveD7();
    alis.varD7 ^= alis.varD6;
    alis.varD7 = ~alis.varD7;
}

// Opername no. 38 opcode 0x4a oegal
// r6 == r7
void oegal(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 == alis.varD7) ? -1 : 0;
}

// Opername no. 39 opcode 0x4c odiff
// r6 != r7
void odiff(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 != alis.varD7) ? -1 : 0;
}

// Opername no. 40 opcode 0x4e oinfeg
// r6 <= r7
void oinfeg(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 <= alis.varD7) ? -1 : 0;
}

// Opername no. 41 opcode 0x50 osupeg
// r6 >= r7
void osupeg(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 >= alis.varD7) ? -1 : 0;
}

// Opername no. 42 opcode 0x52 oinf
// r6 < r7
void oinf(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 < alis.varD7) ? -1 : 0;
}

// Opername no. 43 opcode 0x54 osup
// r6 > r7
void osup(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 > alis.varD7) ? -1 : 0;
}

// Opername no. 44 opcode 0x56 oadd
// r7 += variable
void oadd(void) {
    readexec_opername_saveD7();
    alis.varD7 += alis.varD6;
}

// Opername no. 45 opcode 0x58 osub
// r7 -= variable
void osub(void) {
    readexec_opername_saveD7();
    alis.varD6 -= alis.varD7;
    alis.varD7 = alis.varD6;
}

// Opername no. 46 opcode 0x5a omod
// r7 %= variable
void omod(void) {
    readexec_opername_saveD7();
    alis.varD6 %= alis.varD7;
    alis.varD7 = alis.varD6;
}

// Opername no. 47 opcode 0x5c odiv
// r7 /= variable
void odiv(void) {
    readexec_opername_saveD7();
    alis.varD6 /= alis.varD7;
    alis.varD7 = alis.varD6;
}

// Opername no. 48 opcode 0x5e omul
// r7 *= variable
void omul(void) {
    readexec_opername_saveD7();
    alis.varD7 *= alis.varD6;
}

// Opername no. 49 opcode 0x60 oneg
void oneg(void) {
    alis.varD7 = -alis.varD7;
}

// Opername no. 50 opcode 0x62 oabs
void oabs(void) {
    if(alis.varD7 < 0) {
        alis.varD7 = -alis.varD7;
    }
}

// Opername no. 51 opcode 0x64 ornd
void ornd(void) {
//    alis.varD7 = sys_random() % alis.varD7;
    u32 result = alis.random_number;
    result = alis.random_number = (u16)(result * 0x7ab7 + -0x77f);
    result = (alis.varD7 & 0xffff) * result;
    alis.varD7 = result * 0x10000 | result >> 0x10;
}

// Opername no. 52 opcode 0x66 osgn
void osgn(void) {
    if(alis.varD7 > 0) {
        alis.varD7 = 1;
    }
    else if(alis.varD7 < 0) {
        alis.varD7 = -1;
    }
}

// Opername no. 53 opcode 0x68 onot
void onot(void) {
    alis.varD7 = ~alis.varD7;
}

// Opername no. 54 opcode 0x6a oinkey
void oinkey(void) {
    alis.varD7 = alis.automode ? alis.prevkey : (alis.prevkey = io_inkey());
    debug(EDebugInfo, " [%d] ", alis.varD7);
}

// Opername no. 55 opcode 0x6c okeyon
void okeyon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 56 opcode 0x6e ojoy
void ojoy(void) {
    if (alis.automode == 0)
    {
        alis.varD7 = io_joy(alis.varD7);
    }
}

// Opername no. 57 opcode 0x70 oprnd
void oprnd(void) {
    u32 result = *alis.acc++;
    result = (u16)(result << 10 | result >> 6);
    result = (u16)(result * 0x7ab7 - 0x77f);
    result = (alis.varD7 & 0xffff) * result;
    alis.varD7 = result * 0x10000 | result >> 0x10;
}

// Opername no. 58 opcode 0x72 oscan
void oscan(void) {
    s16 new_val = get_0x1e_scan_clr(alis.script->vram_org);
    if (new_val == get_0x1c_scan_clr(alis.script->vram_org))
    {
        alis.varD7 = -1;
        return;
    }
    
    s16 limit = alis.platform.version >= 30 ? -0x3f : -0x35;

    s16 result = xread16(alis.script->vram_org + new_val);
    new_val += 2;
    if (limit < new_val)
    {
        new_val -= xread16(get_0x14_script_org_offset(alis.script->vram_org) + 0x16);
    }

    set_0x1e_scan_clr(alis.script->vram_org, new_val);
    if (new_val == get_0x1c_scan_clr(alis.script->vram_org))
    {
        set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0x7f);
    }
    
    alis.varD7 = result;
}

// Opername no. 59 opcode 0x74 oshiftkey
void oshiftkey(void) {
    // TODO: if automode is active use value stored in D1
    alis.varD7 = alis.automode ? 0 : io_shiftkey();
}

s16 io_dfree(void)
{
    u32 result;
    
//    __m68k_trap(1);
    // I2 #fa00 = 0001965c
    
    if (*(u32 *)(alis.mem + 0xfa00) < ((u32 *)(alis.buffer))[0])
    {
        result = 32000000;
    }
    else
    {
        result = ((((u32 *)(alis.buffer))[2] & 0xffff) * (((u32 *)(alis.buffer))[3] & 0xffff) & 0xffff) * (((u32 *)(alis.buffer))[0] & 0xffff);
        if (32000000 < result)
            result = 32000000;
    }
    
    return result / 1000;

}

// Opername no. 60 opcode 0x76 ofree
void ofree(void) {
    
    if (alis.platform.version <= 20)
    {
        if (alis.varD7 == 0)
        {
            alis.varD7 = (alis.finmem - alis.finprog) / 1000;
        }
        else if (alis.varD7 == 1)
        {
            alis.varD7 = (image.debsprit - alis.finent) / 1000;
        }
        else if (alis.varD7 == 2)
        {
            alis.varD7 = alis.maxprog - alis.nbprog;
        }
        else if (alis.varD7 == 3)
        {
            alis.varD7 = alis.maxent - alis.nbent;
        }
        else if (alis.varD7 == 4)
        {
            alis.varD7 = 0;
            for (s16 spritidx = image.libsprit; spritidx != 0; spritidx = SPRITE_VAR(spritidx)->to_next)
            {
                alis.varD7 ++;
            }
        }
        else if ((alis.varD7 == 0x61) || (alis.varD7 == 0x41))
        {
            alis.varD7 = io_dfree();
        }
        else if ((alis.varD7 == 0x62) || (alis.varD7 == 0x42))
        {
            alis.varD7 = io_dfree();
        }
        else if ((alis.varD7 != 99) && (alis.varD7 != 0x43))
        {
            alis.varD7 = -1;
        }
        else
        {
            alis.varD7 = io_dfree();
        }
    }
    else
    {
        if (alis.varD7 == 0)
        {
            alis.varD7 = (alis.finmem - alis.finprog) / 1000;
        }
        else if (alis.varD7 == 1)
        {
            alis.varD7 = (image.debsprit - alis.finent) / 1000;
        }
        else if (alis.varD7 == 2)
        {
            alis.varD7 = alis.maxprog - alis.nbprog;
        }
        else if (alis.varD7 == 3)
        {
            alis.varD7 = alis.maxent - alis.nbent;
        }
        else if (alis.varD7 == 4)
        {
            alis.varD7 = 0;
            for (s16 spritidx = image.libsprit; spritidx != 0; spritidx = SPRITE_VAR(spritidx)->to_next)
            {
                alis.varD7 ++;
            }
        }
        else if (0x40 < alis.varD7)
        {
            if (alis.varD7 < 0x49)
            {
                alis.varD7 = io_dfree();
            }
            
            if ((0x60 < alis.varD7) && (alis.varD7 < 0x69))
            {
                alis.varD7 = io_dfree();
            }
        }
        else
        {
            alis.varD7 =  -1;
        }
    }
}

// Opername no. 61 opcode 0x78 omodel
void omodel(void) {
    alis.varD7 = sys_get_model();
}

// Opername no. 62 opcode 0x7a ogetkey
void ogetkey(void) {
    alis.varD7 = sys_get_key();
    debug(EDebugInfo, " [%d] ", alis.varD7);
}

// Opername no. 63 opcode 0x7c oleft
void oleft(void) {
    pop_sd7();
    *(u8 *)(alis.sd7 + (alis.varD7 & 0xff)) = 0;
}

// Opername no. 64 opcode 0x7e oright
void oright(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 65 opcode 0x80 omid
void omid(void) {
    char *src = alis.sd7 + *alis.acc++;

    pop_sd7();
    
    char *tgt = alis.sd7;

    alis.varD7 -= 1;
    for (; alis.varD7 > -1; alis.varD7--, src++, tgt++)
    {
        *tgt = *src;
        
        if (*tgt == 0)
            break;
    }
    
    if (alis.varD7 < 0)
    {
        *tgt = 0;
    }
}

// Opername no. 66 opcode 0x82 olen
void olen(void) {
    alis.varD7 = strlen((const char *)alis.sd7);
}

// Opername no. 67 opcode 0x84 oasc
void oasc(void) {
    alis.varD7 = alis.sd7[0];
}

// Opername no. 68 opcode 0x86 ostr
void ostr(void) {
    valtostr(alis.sd7, alis.varD7);
}

// Opername no. 69 opcode 0x88 osadd
void osadd(void) {
    readexec_opername_swap();
    
    strcat(alis.sd6, alis.sd7);

    char *tmp = alis.sd6;
    alis.sd6 = alis.sd7;
    alis.sd7 = tmp;
}

// Opername no. 70 opcode 0x8a osegal
void osegal(void) {
    readexec_opername_swap();
    alis.varD7 = strcmp(alis.sd6, alis.sd7) ? 0 : -1;
}

// Opername no. 71 opcode 0x8c osdiff
void osdiff(void) {
    readexec_opername_swap();
    alis.varD7 = strcmp(alis.sd6, alis.sd7) ? -1 : 0;
}

// Opername no. 72 opcode 0x8e osinfeg
void osinfeg(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp(alis.sd6, alis.sd7) <= 0 ? -1 : 0;
}

// Opername no. 73 opcode 0x90 ossupeg
void ossupeg(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp(alis.sd6, alis.sd7) >= 0 ? -1 : 0;
}

// Opername no. 74 opcode 0x92 osinf
void osinf(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp(alis.sd6, alis.sd7) < 0 ? -1 : 0;
}

// Opername no. 75 opcode 0x94 ossup
void ossup(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp(alis.sd6, alis.sd7) > 0 ? -1 : 0;
}

// Opername no. 76 opcode 0x96 ospushacc
void ospushacc(void) {
    push_sd7();
}

// Opername no. 77 opcode 0x98 ospile
void ospile(void) {
    char * tmp = alis.sd6;
    alis.sd6 = alis.sd7;
    alis.sd7 = tmp;

    pop_sd6();
}

// Opername no. 78 opcode 0x9a oval
void oval(void) {
    // TODO: compute int value of bssChunk3 string -> d7 ??
//    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    
    s16 result = 0;
    u8 flip = 0;
    
    if (alis.sd7[0] != 0)
    {
        char *string = alis.sd7;

        flip = alis.sd7[0] == 0x2d;
        if (flip)
        {
            string ++;
        }
        
        s8 c1;
        s8 c2;
        
        if (*string == 0x24)
        {
            while (1)
            {
                string ++;
                c2 = *string;
                if (c2 == 0 || (c2 - 0x30) < 0)
                    break;
                
                if (c2 < 0x3a)
                {
                    c1 = c2 - 0x30;
                }
                else
                {
                    c2 = c2 & 0xdf;
                    if (c2 + 0xbf < 0 || 0x46 < c2)
                        break;
                    
                    c1 = c2 - 0x37;
                }
                
                result = result * 0x10 + c1;
            }
        }
        else
        {
            while (1)
            {
                c2 = *string;
                if ((c2 == 0 || (c2 - 0x30) < 0) || 0x39 < c2)
                    break;
                
                result = result * 10 + (c2 - 0x30);
                string ++;
            }
        }
    }

    alis.varD7 = flip ? -result : result;
}

// Opername no. 79 opcode 0x9c oexistf
void oexistf(void) {
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
//    strcat(path, (char *)alis.oldsd7);
    strcat(path, (char *)alis.sd7);
    char *dotptr = strrchr(path, '.');
    if (dotptr)
    {
        if (strcasecmp(dotptr + 1, "fic") && strcasecmp(dotptr + 1, "cst") && strcasecmp(dotptr + 1, "dat") && !sys_fexists(path))
        {
            strcpy(dotptr + 1, alis.platform.ext);
        }
    }
    
    alis.varD7 = sys_fexists(path) ? -1 : 0;
}

// Opername no. 80 opcode 0x9e ochr
void ochr(void) {
    alis.sd7[0] = (u8)alis.varD7;
    alis.sd7[1] = 0;
}

// Opername no. 81 opcode 0xa0 ochange
void ochange(void) {
    // TODO: change le drive courant ??
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    alis.varD7 = 0;
}

// Opername no. 82 opcode 0xa2 ocountry
void ocountry(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 83 opcode 0xa4 omip
void omip(void) {
    alis.varD7 = 0x64;
}

// Opername no. 84 opcode 0xa6 ojoykey
void ojoykey(void) {
    if (alis.automode == 0)
    {
        alis.varD7 = io_joykey(alis.varD7);
    }
}

// Opername no. 85 opcode 0xa8 oconfig
void oconfig(void) {
    alis.varD7 = 0;
}

// Opername no. 31 opcode 0x3c cnul
// Opername no. 32 opcode 0x3e cnul
void cnul(void) {
}

// =============================================================================
#pragma mark - Opername Pointer table (85 values)
// In the disasm, the table JTAB_OPERNAMES is at address $10ee8 -> $10f92.
// So it contains 0xaa / 170 bytes, or 85 words, it matches the opernames count.
// So the routine FUN_READEXEC_OPERNAME() gets a BYTE code from script pc,
//  then retrieves a WORD offset at (JTAB_OPERNAMES[code]), then
//  jumps at the address (JTAB_OPERAMES + offset).
// =============================================================================
sAlisOpcode opernames[] = {
    DECL_OPCODE(0x00, oimmb,
                "read immediate byte from script, extend to word, copy into r7"),
    {},
    DECL_OPCODE(0x02, oimmw,
                "read immediate word from script, copy into r7"),
    {},
    DECL_OPCODE(0x04, oimmp,
                "read immediate bytes from script until zero, copy into str1"),
    {},
    DECL_OPCODE(0x06, olocb,
                "read offset word from script, extend read byte at vram[offset] to word, copy into r7"),
    {},
    DECL_OPCODE(0x08, olocw,
                "read offset word from script, read word at vram[offset], copy into r7"),
    {},
    DECL_OPCODE(0x0a, olocp,
                "read offset word from script, read bytes at vram[offset] until zero, copy into str1"),
    {},
    DECL_OPCODE(0x0c, oloctp, "TODO add desc"),
    {},
    DECL_OPCODE(0x0e, oloctc, "TODO add desc"),
    {},
    DECL_OPCODE(0x10, olocti, "TODO add desc"),
    {},
    DECL_OPCODE(0x12, odirb, "TODO add desc"),
    {},
    DECL_OPCODE(0x14, odirw, "TODO add desc"),
    {},
    DECL_OPCODE(0x16, odirp, "TODO add desc"),
    {},
    DECL_OPCODE(0x18, odirtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x1a, odirtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x1c, odirti, "TODO add desc"),
    {},
    DECL_OPCODE(0x1e, omainb, "TODO add desc"),
    {},
    DECL_OPCODE(0x20, omainw, "TODO add desc"),
    {},
    DECL_OPCODE(0x22, omainp, "TODO add desc"),
    {},
    DECL_OPCODE(0x24, omaintp, "TODO add desc"),
    {},
    DECL_OPCODE(0x26, omaintc, "TODO add desc"),
    {},
    DECL_OPCODE(0x28, omainti, "TODO add desc"),
    {},
    DECL_OPCODE(0x2a, ohimb, "TODO add desc"),
    {},
    DECL_OPCODE(0x2c, ohimw, "TODO add desc"),
    {},
    DECL_OPCODE(0x2e, ohimp, "TODO add desc"),
    {},
    DECL_OPCODE(0x30, ohimtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x32, ohimtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x34, ohimti, "TODO add desc"),
    {},
    DECL_OPCODE(0x36, opile,
                "copy r6 to r7, pop word form virtual accumulator into r6"),
    {},
    DECL_OPCODE(0x38, oeval,
                "starts an expression evaluation loop"),
    {},
    DECL_OPCODE(0x3a, ofin,
                "ends an expression evaluation loop"),
    {},
    DECL_OPCODE(0x3c, cnul,
                "[N/I]"),
    {},
    DECL_OPCODE(0x3e, cnul,
                "[N/I]"),
    {},
    DECL_OPCODE(0x40, opushacc,
                "push word from r7 into virtual accumulator"),
    {},
    DECL_OPCODE(0x42, oand,
                "AND binary operator"),
    {},
    DECL_OPCODE(0x44, oor,
                "OR binary operator"),
    {},
    DECL_OPCODE(0x46, oxor,
                "XOR binary operator"),
    {},
    DECL_OPCODE(0x48, oeqv,
                "EQV binary operator"),
    {},
    DECL_OPCODE(0x4a, oegal,
                "is equal test"),
    {},
    DECL_OPCODE(0x4c, odiff,
                "is non-equal test"),
    {},
    DECL_OPCODE(0x4e, oinfeg,
                "is inferior or equal test"),
    {},
    DECL_OPCODE(0x50, osupeg,
                "is superior or equal test"),
    {},
    DECL_OPCODE(0x52, oinf,
                "is inferior test"),
    {},
    DECL_OPCODE(0x54, osup,
                "is superior test"),
    {},
    DECL_OPCODE(0x56, oadd,
                "addition operator"),
    {},
    DECL_OPCODE(0x58, osub,
                "substraction operator"),
    {},
    DECL_OPCODE(0x5a, omod,
                "modulo operator"),
    {},
    DECL_OPCODE(0x5c, odiv,
                "division operator"),
    {},
    DECL_OPCODE(0x5e, omul,
                "multiplication operator"),
    {},
    DECL_OPCODE(0x60, oneg,
                "negation operator"),
    {},
    DECL_OPCODE(0x62, oabs,
                "absolute value operator"),
    {},
    DECL_OPCODE(0x64, ornd,
                "get random number"),
    {},
    DECL_OPCODE(0x66, osgn,
                "sign test operator"),
    {},
    DECL_OPCODE(0x68, onot,
                "binary NOT operator"),
    {},
    DECL_OPCODE(0x6a, oinkey, "TODO add desc"),
    {},
    DECL_OPCODE(0x6c, okeyon, "TODO add desc"),
    {},
    DECL_OPCODE(0x6e, ojoy, "TODO add desc"),
    {},
    DECL_OPCODE(0x70, oprnd, "TODO add desc"),
    {},
    DECL_OPCODE(0x72, oscan, "TODO add desc"),
    {},
    DECL_OPCODE(0x74, oshiftkey, "TODO add desc"),
    {},
    DECL_OPCODE(0x76, ofree, "TODO add desc"),
    {},
    DECL_OPCODE(0x78, omodel,
                "get host device model"),
    {},
    DECL_OPCODE(0x7a, ogetkey, "TODO add desc"),
    {},
    DECL_OPCODE(0x7c, oleft, "TODO add desc"),
    {},
    DECL_OPCODE(0x7e, oright, "TODO add desc"),
    {},
    DECL_OPCODE(0x80, omid, "TODO add desc"),
    {},
    DECL_OPCODE(0x82, olen, "TODO add desc"),
    {},
    DECL_OPCODE(0x84, oasc, "TODO add desc"),
    {},
    DECL_OPCODE(0x86, ostr, "TODO add desc"),
    {},
    DECL_OPCODE(0x88, osadd, "TODO add desc"),
    {},
    DECL_OPCODE(0x8a, osegal, "TODO add desc"),
    {},
    DECL_OPCODE(0x8c, osdiff, "TODO add desc"),
    {},
    DECL_OPCODE(0x8e, osinfeg, "TODO add desc"),
    {},
    DECL_OPCODE(0x90, ossupeg, "TODO add desc"),
    {},
    DECL_OPCODE(0x92, osinf, "TODO add desc"),
    {},
    DECL_OPCODE(0x94, ossup, "TODO add desc"),
    {},
    DECL_OPCODE(0x96, ospushacc, "TODO add desc"),
    {},
    DECL_OPCODE(0x98, ospile, "TODO add desc"),
    {},
    DECL_OPCODE(0x9a, oval, "TODO add desc"),
    {},
    DECL_OPCODE(0x9c, oexistf,
                "if file name in bssChunk3 exists, returns 0xffff in varD7, else 0"),
    {},
    DECL_OPCODE(0x9e, ochr, "TODO add desc"),
    {},
    DECL_OPCODE(0xa0, ochange, "TODO add desc"),
    {},
    DECL_OPCODE(0xa2, ocountry, "TODO add desc"),
    {},
    DECL_OPCODE(0xa4, omip, "TODO add desc"),
    {},
    DECL_OPCODE(0xa6, ojoykey, "TODO add desc"),
    {},
    DECL_OPCODE(0xa8, oconfig, "unknown"),
    {}
};
