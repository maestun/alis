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


// =============================================================================
#pragma mark - TODO: opernames
// =============================================================================

void oimmb(void) {
    // reads a byte, extends into r7
    alis.varD7 = (s8)script_read8();
}

void oimmw(void) {
    // reads a word into r7
    alis.varD7 = script_read16();
}

void oimmp(void) {
    // reads null-terminated data into bssChunk3
    script_read_until_zero(alis.sd7);
    debug(EDebugWarning, " [\"%s\" <= sd7]", (char *)alis.sd7);
}

void olocb(void) {
    // read word offset, copy extended byte from ram[offset] into r7
    s16 offset = script_read16();
    alis.varD7 = (s8)xread8(alis.script->vram_org + offset);
}

void olocw(void) {
    // read word offset, copy word from ram[offset] into r7
    s16 offset = script_read16();
    alis.varD7 = xread16(alis.script->vram_org + offset);
}

void olocp(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)get_vram(offset), alis.script->vram_org + offset);
    strcpy((char *)alis.sd7, (char *)get_vram(offset));
}

void oloctp(void) {
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
}

void oloctc(void) {
    s16 offset = tabchar(script_read16(), alis.mem + alis.script->vram_org);
    alis.varD7 = (s8)xread8(alis.script->vram_org + offset);
}

void olocti(void) {
    s16 offset = tabint(script_read16(), alis.mem + alis.script->vram_org);
    alis.varD7 = xread16(alis.script->vram_org + offset);
}

// reads a byte offset from script,
// then reads an extended byte from vram[offset] into r7
void odirb(void) {
    u8 offset = script_read8();
    alis.varD7 = (s8)xread8(alis.script->vram_org + offset);
}

// reads a byte offset from script,
// then reads a word from vram[offset] into r7
void odirw(void) {
    u8 offset = script_read8();
    alis.varD7 = xread16(alis.script->vram_org + offset);
}

// reads a byte offset from script,
// then reads a null-terminated data stream from vram[offset] into bssChunk3
void odirp(void) {
    u8 offset = script_read8();
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)get_vram(offset), alis.script->vram_org + offset);
    strcpy((char *)alis.sd7, (char *)get_vram(offset));
}

void odirtp(void) {
    s16 offset = tabstring(script_read8(), alis.mem + alis.script->vram_org);
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)get_vram(offset), alis.script->vram_org + offset);
    strcpy((char *)alis.sd7, (char *)get_vram(offset));
}

void odirtc(void) {
    s16 offset = tabchar(script_read8(), alis.mem + alis.script->vram_org);
    alis.varD7 = (s8)xread8(alis.script->vram_org + offset);
}

void odirti(void) {
    s16 offset = tabint(script_read8(), alis.mem + alis.script->vram_org);
    alis.varD7 = xread16(alis.script->vram_org + offset);
}

void omainb(void) {
    s16 offset = script_read16();
    alis.varD7 = (s8)xread8(alis.basemain + offset);
}

void omainw(void) {
    s16 offset = script_read16();
    alis.varD7 = xread16(alis.basemain + offset);
}

void omainp(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%s <= %.6x]", (char *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    strcpy((char *)alis.sd7, (char *)(alis.mem + alis.basemain + offset));
}

void omaintp(void) {
    s16 offset = tabstring(script_read16(), alis.mem + alis.basemain);
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    strcpy((char *)alis.sd7, (char *)(alis.mem + alis.basemain + offset));
}

void omaintc(void) {
    s16 offset = tabchar(script_read16(), alis.mem + alis.basemain);
    alis.varD7 = (s8)xread8(alis.basemain + offset);
}

void omainti(void) {
    s16 offset = tabint(script_read16(), alis.mem + alis.basemain);
    alis.varD7 = xread16(alis.basemain + offset);
}

void ohimb(void) {
    u16 entry = xread16(alis.script->vram_org + (s16)script_read16());
    u32 vram_addr = xread32(alis.atent + entry);

    s16 offset = script_read16();
    alis.varD7 = (s8)xread8(vram_addr + offset);
}

void ohimw(void) {
    u16 entry = xread16(alis.script->vram_org + (s16)script_read16());
    u32 vram_addr = xread32(alis.atent + entry);

    s16 offset = script_read16();
    alis.varD7 = xread16(vram_addr + offset);
}

void ohimp(void) {
    u16 entry = xread16(alis.script->vram_org + (s16)script_read16());
    u32 vram_addr = xread32(alis.atent + entry);

    s16 offset = script_read16();
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)(alis.mem + vram_addr + offset), vram_addr + offset);
    strcpy((char *)alis.sd7, (char *)(alis.mem + vram_addr + offset));
}

void ohimtp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void ohimtc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void ohimti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// pop from accumulator into r6
void opile(void) {
    // save r6 into r7
    alis.varD7 = alis.varD6;
    alis.varD6 = *(alis.acc);
    alis.acc++;
}

// start eval loop, will stop after ofin() is called
void oeval(void) {
    alis.oeval_loop = 1;
    while(alis.oeval_loop) {
        readexec_opername();
    }
    
    debug(EDebugInfo, " [ = %d]", (s8)alis.varD7);
}

// stop eval loop
void ofin(void) {
    alis.oeval_loop = 0;
}

// push value from r7 register to accumulator
void opushacc(void) {
    *(--alis.acc) = alis.varD7;
}

// r7 = variable AND r7
void oand(void) {
    alis.varD6 = alis.varD7;
    readexec_opername();
    alis.varD7 &= alis.varD6;
}

// r7 = variable OR r7
void oor(void) {
    alis.varD6 = alis.varD7;
    readexec_opername();
    alis.varD7 |= alis.varD6;
}

// r7 = variable XOR r7
void oxor(void) {
    readexec_opername_saveD7();
    alis.varD7 ^= alis.varD6;
}

// r7 = variable EQV r7
void oeqv(void) {
    readexec_opername_saveD7();
    alis.varD7 ^= alis.varD6;
    alis.varD7 = ~alis.varD7;
}

// r6 == r7
void oegal(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 == alis.varD7) ? -1 : 0;
}

// r6 != r7
void odiff(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 != alis.varD7) ? -1 : 0;
}

// r6 <= r7
void oinfeg(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 <= alis.varD7) ? -1 : 0;
}

// r6 >= r7
void osupeg(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 >= alis.varD7) ? -1 : 0;
}

// r6 < r7
void oinf(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 < alis.varD7) ? -1 : 0;
}

// r6 > r7
void osup(void) {
    readexec_opername_saveD7();
    alis.varD7 = (alis.varD6 > alis.varD7) ? -1 : 0;
}

// r7 += variable
void oadd(void) {
    readexec_opername_saveD7();
    alis.varD7 += alis.varD6;
}

// r7 -= variable
void osub(void) {
    readexec_opername_saveD7();
    alis.varD6 -= alis.varD7;
    alis.varD7 = alis.varD6;
}

// r7 %= variable
void omod(void) {
    readexec_opername_saveD7();
    alis.varD6 %= alis.varD7;
    alis.varD7 = alis.varD6;
}

// r7 /= variable
void odiv(void) {
    readexec_opername_saveD7();
    alis.varD6 /= alis.varD7;
    alis.varD7 = alis.varD6;
}

// r7 *= variable
void omul(void) {
    readexec_opername_saveD7();
    alis.varD7 *= alis.varD6;
}


void oneg(void) {
    alis.varD7 = -alis.varD7;
}


void oabs(void) {
    if(alis.varD7 < 0) {
        alis.varD7 = -alis.varD7;
    }
}

void ornd(void) {
//    alis.varD7 = sys_random() % alis.varD7;
    u32 result = alis.random_number;
    result = alis.random_number = (u16)(result * 0x7ab7 + -0x77f);
    result = (alis.varD7 & 0xffff) * result;
    alis.varD7 = result * 0x10000 | result >> 0x10;
}

void osgn(void) {
    if(alis.varD7 > 0) {
        alis.varD7 = 1;
    }
    else if(alis.varD7 < 0) {
        alis.varD7 = -1;
    }
}

void onot(void) {
    alis.varD7 = ~alis.varD7;
}

void oinkey(void) {
    alis.varD7 = alis.automode ? alis.prevkey : (alis.prevkey = io_inkey());
    debug(EDebugInfo, " [%d] ", alis.varD7);
}

void okeyon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void ojoy(void) {
    if (alis.automode == 0)
    {
        alis.varD7 = io_joy(alis.varD7);
    }
}

void oprnd(void) {
    u32 result = *(alis.acc);
    result = (u16)(result << 10 | result >> 6);
    result = (u16)(result * 0x7ab7 + -0x77f);
    result = (alis.varD7 & 0xffff) * result;
    alis.varD7 = result * 0x10000 | result >> 0x10;
    alis.acc++;
}

void oscan(void) {
    s16 scan_clr = get_0x1e_scan_clr(alis.script->vram_org);
    if (scan_clr == get_0x1c_scan_clr(alis.script->vram_org))
    {
        alis.varD7 = -1;
        return;
    }
    
    s16 result = xread16(alis.script->vram_org + scan_clr);
    scan_clr += 2;
    if (-0x35 < scan_clr)
    {
        scan_clr -= xread16(get_0x14_script_org_offset(alis.script->vram_org) + 0x16);
    }

    set_0x1e_scan_clr(alis.script->vram_org, scan_clr);
    if (scan_clr == get_0x1c_scan_clr(alis.script->vram_org))
    {
        set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0x7f);
    }
    
    alis.varD7 = result;
}

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

void ofree(void) {
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
    else if (alis.varD7 != 3)
    {
        if (alis.varD7 == 4)
        {
            alis.varD7 = 0;
            
            for (s16 spritidx = image.libsprit; spritidx != 0; spritidx = SPRITE_VAR(spritidx)->to_next)
            {
                alis.varD7 ++;
            }
            
            return;
        }
        else if (0x40 < alis.varD7)
        {
            if (alis.varD7 < 0x49)
            {
                alis.varD7 = io_dfree();
                return;
            }
            
            if ((0x60 < alis.varD7) && (alis.varD7 < 0x69))
            {
                alis.varD7 = io_dfree();
                return;
            }
        }
        
        alis.varD7 =  -1;
    }
    else
    {
        alis.varD7 = alis.maxent - alis.nbent;
    }
}

void omodel(void) {
    alis.varD7 = sys_get_model();
}

void ogetkey(void) {
    alis.varD7 = sys_get_key();
    debug(EDebugInfo, " [%d] ", alis.varD7);
}

void oleft(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void oright(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void omid(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void olen(void) {
    alis.varD7 = strlen((const char *)alis.sd7);
}

void oasc(void) {
    alis.varD7 = 0;
    alis.varD7 = alis.sd7[0];
}

void ostr(void) {
    valtostr(alis.sd7, alis.varD7);
}

void osadd(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    
    readexec_opername_swap();
    
    strcat((char *)alis.sd6, (char *)alis.sd7);

    u8 *tmp = alis.sd6;
    alis.sd6 = alis.sd7;
    alis.sd7 = tmp;
}

void osegal(void) {
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) ? 0 : -1;
}

void osdiff(void) {
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) ? -1 : 0;
}

void osinfeg(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) <= 0 ? -1 : 0;
}

void ossupeg(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) >= 0 ? -1 : 0;
}

void osinf(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) < 0 ? -1 : 0;
}

void ossup(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) > 0 ? -1 : 0;
}

void ospushacc(void) {
    *(--alis.acc) = alis.varD7;
}

void pop_sd6(void)
{
    debug(EDebugWarning, " [\"%s\" <= %.6x]", (char *)alis.acc, (u8 *)alis.acc - alis.mem);
    strcpy((char *)alis.sd6, (char *)alis.acc);
    alis.acc += (s32)strlen((char *)alis.sd6) / 2;
}

void ospile(void) {
    debug(EDebugWarning, "CHECK: %s", __FUNCTION__);

    u8 * tmp = alis.sd6;
    alis.sd6 = alis.sd7;
    alis.sd7 = tmp;
    
//    s32 t0 = (s32)(alis.sd7 - alis.mem);
//    s32 t1 = (s32)(alis.sd6 - alis.mem);
//
//    printf("osadd(void): Write to address $0195e8, new value is %d ($%x)\n", t1 & 0xffff, t1 & 0xffff);
//    printf("osadd(void): Write to address $0195e4, new value is %d ($%x)\n", t0 & 0xffff, t0 & 0xffff);

    pop_sd6();
}

void oval(void) {
    // TODO: compute int value of bssChunk3 string -> d7 ??
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    
    s8 cVar1;
    s8 bVar2;
    
    s16 sVar3 = 0;
    u8 neg = 0;
    if (alis.sd7[0] != 0)
    {
        u8 *string = alis.sd7;

        neg = alis.sd7[0] == 0x2d;
        if (neg)
        {
            string ++;
        }
        
        if (*string == 0x24)
        {
            while (1)
            {
                string ++;
                bVar2 = *string;
                if (bVar2 == 0 || (bVar2 - 0x30) < 0)
                    break;
                
                if (bVar2 < 0x3a)
                {
                    cVar1 = bVar2 - 0x30;
                }
                else
                {
                    bVar2 = bVar2 & 0xdf;
                    if (bVar2 + 0xbf < 0 || 0x46 < bVar2)
                        break;
                    
                    cVar1 = bVar2 - 0x37;
                }
                
                sVar3 = sVar3 * 0x10 + cVar1;
            }
        }
        else
        {
            while (1)
            {
                bVar2 = *string;
                if ((bVar2 == 0 || (bVar2 - 0x30) < 0) || 0x39 < bVar2)
                    break;
                
                sVar3 = sVar3 * 10 + (bVar2 - 0x30);
                string ++;
            }
        }
    }

    alis.varD7 = neg ? -sVar3 : sVar3;
}

void oexistf(void) {
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    strcat(path, (char *)alis.oldsd7);
    char *dotptr = strrchr(path, '.');
    if (dotptr)
    {
        if (strcasecmp(dotptr + 1, "fic"))
        {
            strcpy(dotptr + 1, alis.platform.ext);
        }
    }
    
    alis.varD7 = sys_fexists(path) ? -1 : 0;
}

void ochr(void) {
    alis.sd7[0] = (u8)alis.varD7;
    alis.sd7[1] = 0;
}

void ochange(void) {
    // TODO: change le drive courant ??
    debug(EDebugWarning, "STUBBED: %s", __FUNCTION__);
    alis.varD7 = -1;
}

void ocountry(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

void omip(void) {
    alis.varD7 = 0x64;
}

void ojoykey(void) {
    if (alis.automode == 0)
    {
        alis.varD7 = io_joykey(alis.varD7);
    }
}

void oconfig(void) {
    alis.varD7 = 0;
}

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
    DECL_OPCODE(0x01, oimmw,
                "read immediate word from script, copy into r7"),
    {},
    DECL_OPCODE(0x02, oimmp,
                "read immediate bytes from script until zero, copy into str1"),
    {},
    DECL_OPCODE(0x03, olocb,
                "read offset word from script, extend read byte at vram[offset] to word, copy into r7"),
    {},
    DECL_OPCODE(0x04, olocw,
                "read offset word from script, read word at vram[offset], copy into r7"),
    {},
    DECL_OPCODE(0x05, olocp,
                "read offset word from script, read bytes at vram[offset] until zero, copy into str1"),
    {},
    DECL_OPCODE(0x06, oloctp, "TODO add desc"),
    {},
    DECL_OPCODE(0x07, oloctc, "TODO add desc"),
    {},
    DECL_OPCODE(0x08, olocti, "TODO add desc"),
    {},
    DECL_OPCODE(0x09, odirb, "TODO add desc"),
    {},
    DECL_OPCODE(0x0A, odirw, "TODO add desc"),
    {},
    DECL_OPCODE(0x0B, odirp, "TODO add desc"),
    {},
    DECL_OPCODE(0x0C, odirtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x0D, odirtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x0E, odirti, "TODO add desc"),
    {},
    DECL_OPCODE(0x0F, omainb, "TODO add desc"),
    {},
    DECL_OPCODE(0x10, omainw, "TODO add desc"),
    {},
    DECL_OPCODE(0x11, omainp, "TODO add desc"),
    {},
    DECL_OPCODE(0x12, omaintp, "TODO add desc"),
    {},
    DECL_OPCODE(0x13, omaintc, "TODO add desc"),
    {},
    DECL_OPCODE(0x14, omainti, "TODO add desc"),
    {},
    DECL_OPCODE(0x15, ohimb, "TODO add desc"),
    {},
    DECL_OPCODE(0x16, ohimw, "TODO add desc"),
    {},
    DECL_OPCODE(0x17, ohimp, "TODO add desc"),
    {},
    DECL_OPCODE(0x18, ohimtp, "TODO add desc"),
    {},
    DECL_OPCODE(0x19, ohimtc, "TODO add desc"),
    {},
    DECL_OPCODE(0x1A, ohimti, "TODO add desc"),
    {},
    DECL_OPCODE(0x1B, opile,
                "copy r6 to r7, pop word form virtual accumulator into r6"),
    {},
    DECL_OPCODE(0x1C, oeval,
                "starts an expression evaluation loop"),
    {},
    DECL_OPCODE(0x1D, ofin,
                "ends an expression evaluation loop"),
    {},
    DECL_OPCODE(0x1E, cnul,
                "[N/I]"),
    {},
    DECL_OPCODE(0x1F, cnul,
                "[N/I]"),
    {},
    DECL_OPCODE(0x20, opushacc,
                "push word from r7 into virtual accumulator"),
    {},
    DECL_OPCODE(0x21, oand,
                "AND binary operator"),
    {},
    DECL_OPCODE(0x22, oor,
                "OR binary operator"),
    {},
    DECL_OPCODE(0x23, oxor,
                "XOR binary operator"),
    {},
    DECL_OPCODE(0x24, oeqv,
                "EQV binary operator"),
    {},
    DECL_OPCODE(0x25, oegal,
                "is equal test"),
    {},
    DECL_OPCODE(0x26, odiff,
                "is non-equal test"),
    {},
    DECL_OPCODE(0x27, oinfeg,
                "is inferior or equal test"),
    {},
    DECL_OPCODE(0x28, osupeg,
                "is superior or equal test"),
    {},
    DECL_OPCODE(0x29, oinf,
                "is inferior test"),
    {},
    DECL_OPCODE(0x2A, osup,
                "is superior test"),
    {},
    DECL_OPCODE(0x2B, oadd,
                "addition operator"),
    {},
    DECL_OPCODE(0x2C, osub,
                "substraction operator"),
    {},
    DECL_OPCODE(0x2D, omod,
                "modulo operator"),
    {},
    DECL_OPCODE(0x2E, odiv,
                "division operator"),
    {},
    DECL_OPCODE(0x2F, omul,
                "multiplication operator"),
    {},
    DECL_OPCODE(0x30, oneg,
                "negation operator"),
    {},
    DECL_OPCODE(0x31, oabs,
                "absolute value operator"),
    {},
    DECL_OPCODE(0x32, ornd,
                "get random number"),
    {},
    DECL_OPCODE(0x33, osgn,
                "sign test operator"),
    {},
    DECL_OPCODE(0x34, onot,
                "binary NOT operator"),
    {},
    DECL_OPCODE(0x35, oinkey, "TODO add desc"),
    {},
    DECL_OPCODE(0x36, okeyon, "TODO add desc"),
    {},
    DECL_OPCODE(0x37, ojoy, "TODO add desc"),
    {},
    DECL_OPCODE(0x38, oprnd, "TODO add desc"),
    {},
    DECL_OPCODE(0x39, oscan, "TODO add desc"),
    {},
    DECL_OPCODE(0x3A, oshiftkey, "TODO add desc"),
    {},
    DECL_OPCODE(0x3B, ofree, "TODO add desc"),
    {},
    DECL_OPCODE(0x3C, omodel,
                "get host device model"),
    {},
    DECL_OPCODE(0x3D, ogetkey, "TODO add desc"),
    {},
    DECL_OPCODE(0x3E, oleft, "TODO add desc"),
    {},
    DECL_OPCODE(0x3F, oright, "TODO add desc"),
    {},
    DECL_OPCODE(0x40, omid, "TODO add desc"),
    {},
    DECL_OPCODE(0x41, olen, "TODO add desc"),
    {},
    DECL_OPCODE(0x42, oasc, "TODO add desc"),
    {},
    DECL_OPCODE(0x43, ostr, "TODO add desc"),
    {},
    DECL_OPCODE(0x44, osadd, "TODO add desc"),
    {},
    DECL_OPCODE(0x45, osegal, "TODO add desc"),
    {},
    DECL_OPCODE(0x46, osdiff, "TODO add desc"),
    {},
    DECL_OPCODE(0x47, osinfeg, "TODO add desc"),
    {},
    DECL_OPCODE(0x48, ossupeg, "TODO add desc"),
    {},
    DECL_OPCODE(0x49, osinf, "TODO add desc"),
    {},
    DECL_OPCODE(0x4A, ossup, "TODO add desc"),
    {},
    DECL_OPCODE(0x4B, ospushacc, "TODO add desc"),
    {},
    DECL_OPCODE(0x4C, ospile, "TODO add desc"),
    {},
    DECL_OPCODE(0x4D, oval, "TODO add desc"),
    {},
    DECL_OPCODE(0x4E, oexistf,
                "if file name in bssChunk3 exists, returns 0xffff in varD7, else 0"),
    {},
    DECL_OPCODE(0x4F, ochr, "TODO add desc"),
    {},
    DECL_OPCODE(0x50, ochange, "TODO add desc"),
    {},
    DECL_OPCODE(0x51, ocountry, "TODO add desc"),
    {},
    DECL_OPCODE(0x52, omip, "TODO add desc"),
    {},
    DECL_OPCODE(0x53, ojoykey, "TODO add desc"),
    {},
    DECL_OPCODE(0x54, oconfig,
                "unknown"),
    {}
};
