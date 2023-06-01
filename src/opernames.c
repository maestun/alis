//
//  opernames.c
//  aodec
//
//  Created by zlot on 05/02/2020.
//  Copyright © 2020 zlot. All rights reserved.
//

#include "alis.h"
#include "alis_private.h"
#include "utils.h"

#include "experimental.h"


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
}

void olocb(void) {
    // read word offset, copy extended byte from ram[offset] into r7
    s16 offset = script_read16();
    alis.varD7 = (s8)vram_read8(offset);
}

void olocw(void) {
    // read word offset, copy word from ram[offset] into r7
    s16 offset = script_read16();
    alis.varD7 = vram_read16(offset);
}

void olocp(void) {
    s16 offset = script_read16();
    vram_readp(offset, alis.sd7);
}

void oloctp(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oloctc(void) {
    s16 offset = tabchar(script_read16(), alis.mem + alis.script->vram_org);
    alis.varD7 = vram_read8(offset);
}

void olocti(void) {
    s16 offset = tabint(script_read16(), alis.mem + alis.script->vram_org);
    alis.varD7 = read16(vram_ptr(offset), alis.platform.is_little_endian);
}

// reads a byte offset from script,
// then reads an extended byte from vram[offset] into r7
void odirb(void) {
    u8 offset = script_read8();
    alis.varD7 = (s8)vram_read8(offset);
}

// reads a byte offset from script,
// then reads a word from vram[offset] into r7
void odirw(void) {
    u8 offset = script_read8();
    alis.varD7 = vram_read16(offset);
    // printf("\nXXodirw: 0x%.6x > 0x%.2x\n", (u16)alis.varD7, offset);
}

// reads a byte offset from script,
// then reads a null-terminated data stream from vram[offset] into bssChunk3
void odirp(void) {
    u8 offset = script_read8();
    vram_readp(offset, alis.sd7);
}

void odirtp(void) {
    debug(EDebugWarning, " /* CHECK */");
    s16 offset = tabstring(script_read8(), alis.mem + alis.script->vram_org);
    strcpy((char *)alis.sd7, (char *)(alis.mem + alis.script->vram_org + offset));
}

void odirtc(void) {
    debug(EDebugWarning, " /* CHECK */");
    s16 offset = tabchar(script_read8(), alis.mem + alis.script->vram_org);
    alis.varD7 = *(char *)(alis.mem + alis.script->vram_org + offset);
}

void odirti(void) {
    s16 offset = tabint(script_read8(), alis.mem + alis.script->vram_org);
    alis.varD7 = read16(vram_ptr(offset), alis.platform.is_little_endian);
}

void omainb(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%.2x <= %.6x]", *(s8 *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    alis.varD7 = *(s8 *)(alis.mem + alis.basemain + offset);
}

void omainw(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%.4x <= %.6x]", *(s16 *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    alis.varD7 = *(s16 *)(alis.mem + alis.basemain + offset);
}

void omainp(void) {
    s16 offset = script_read16();
    debug(EDebugWarning, " [%s <= %.6x]", (char *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    strcpy((char *)alis.sd7, (char *)(BASEMNMEM_PTR + offset));
}

void omaintp(void) {
    s16 offset = tabstring(script_read16(), alis.mem + alis.basemain);
    debug(EDebugWarning, " [%s <= %.6x]", (char *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    strcpy((char *)alis.sd7, (char *)(alis.mem + alis.basemain + offset));
}

void omaintc(void) {
    s16 offset = tabchar(script_read16(), alis.mem + alis.basemain);
    debug(EDebugWarning, " [%.2x <= %.6x]", *(s8 *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    alis.varD7 = *(s8 *)(alis.mem + alis.basemain + offset);
}

void omainti(void) {
    s16 offset = tabint(script_read16(), alis.mem + alis.basemain);
    debug(EDebugWarning, " [%.4x <= %.6x]", *(s16 *)(alis.mem + alis.basemain + offset), alis.basemain + offset);
    alis.varD7 = *(s16 *)(alis.mem + alis.basemain + offset);
}

void ohimb(void) {
    u16 entry = vram_read16(script_read16());
    u32 vram_addr = ENTVRAM(entry);

    s16 offset = script_read16();
    debug(EDebugWarning, " [%.2x <= %.6x]", *(s8 *)(alis.mem + vram_addr + offset), vram_addr + offset);
    alis.varD7 = *(s8 *)(alis.mem + vram_addr + offset);
}

void ohimw(void) {
    u16 entry = vram_read16(script_read16());
    u32 vram_addr = ENTVRAM(entry);

    s16 offset = script_read16();
    debug(EDebugWarning, " [%.4x <= %.6x]", *(s16 *)(alis.mem + vram_addr + offset), vram_addr + offset);
    alis.varD7 = *(s16 *)(alis.mem + vram_addr + offset);
}

void ohimp(void) {
    u16 entry = vram_read16(script_read16());
    u32 vram_addr = ENTVRAM(entry);

    s16 offset = script_read16();
    strcpy((char *)alis.sd7, (char *)(alis.mem + vram_addr + offset));
}

void ohimtp(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void ohimtc(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void ohimti(void) {
    debug(EDebugWarning, " /* MISSING */");
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
    alis.varD7 = sys_random() % alis.varD7;
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
    
    if (alis.varD7)
    {
        printf(" [ON] ");
    }
}

void okeyon(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void ojoy(void) {
    if (alis.automode == 0)
    {
        alis.varD7 = io_joy(alis.varD7);
    }
}

void oprnd(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void oscan(void) {
    if (alis.script->context->_0x1e_scan_clr == alis.script->context->_0x1c_scan_clr)
        return;

    s16 scan_clr = alis.script->context->_0x1e_scan_clr + 2;
    if (-0x35 < scan_clr)
        scan_clr -= swap16((alis.mem + alis.script->context->_0x14_script_org_offset + 0x16), alis.platform.is_little_endian);

    alis.script->context->_0x1e_scan_clr = scan_clr;
    if (scan_clr == alis.script->context->_0x1c_scan_clr)
        alis.script->context->_0x24_scan_inter.scan_clr_bit_7 &= 0x7f;
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
        alis.varD7 = (alis.debsprit - alis.finent) / 1000;
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
            
            for (s16 spritidx = alis.libsprit; spritidx != 0; spritidx = SPRITE_VAR(spritidx)->to_next)
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
    debug(EDebugWarning, " /* MISSING */");
}

void oleft(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void oright(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void omid(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void olen(void) {
    alis.varD7 = strlen((const char *)alis.sd7);
}

void oasc(void) {
    alis.varD7 = 0;
    alis.varD7 = alis.sd7[0];
}

void ostr(void) {
    debug(EDebugWarning, " /* CHECK */");
    vald0(alis.sd7, alis.varD7);
}

void osadd(void) {
    debug(EDebugWarning, " /* CHECK */");
    
    readexec_opername_swap();
    
    strcat((char *)alis.sd6, (char *)alis.sd7);

    u8 *tmp = alis.sd6;
    alis.sd6 = alis.sd7;
    alis.sd7 = tmp;
}

void osegal(void) {
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) == 0 ? 0 : -1;
}

void osdiff(void) {
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) == 0 ? -1 : 0;
}

void osinfeg(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) <= 0 ? -1 : 0;
}

void ossupeg(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) >= 0 ? -1 : 0;
}

void osinf(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) < 0 ? -1 : 0;
}

void ossup(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername_swap();
    alis.varD7 = strcmp((char *)alis.sd6, (char *)alis.sd7) > 0 ? -1 : 0;
}

void ospushacc(void) {
    *(--alis.acc) = alis.varD7;
}

void pop_sd6(void)
{
    strcpy((char *)alis.sd6, (char *)alis.acc);
    alis.acc += (s32)strlen((char *)alis.sd6) / 2;
}

void ospile(void) {
    debug(EDebugWarning, " /* CHECK */");

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
    debug(EDebugWarning, " /* STUBBED */");
    
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
    debug(EDebugWarning, " /* STUBBED */");
    alis.varD7 = -1;
}

void ocountry(void) {
    debug(EDebugWarning, " /* MISSING */");
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
