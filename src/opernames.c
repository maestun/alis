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

// =============================================================================
#pragma mark - TODO: opernames
// =============================================================================

void oimmb(void) {
    // reads a byte, extends into r7
    vm.varD7 = script_read8ext16();
}

void oimmw(void) {
    // reads a word into r7
    vm.varD7 = script_read16();
}

void oimmp(void) {
    // reads null-terminated data into sd7
    script_read_until_zero(vm.sd7);
}

void olocb(void) {
    // read word offset, copy extended byte from ram[offset] into r7
    u16 offset = script_read16();
    vm.varD7 = vram_read8ext16(offset);
}

void olocw(void) {
    // read word offset, copy word from ram[offset] into r7
    u16 offset = script_read16();
    vm.varD7 = vram_read16(offset);
}

void olocp(void) {
    u16 offset = script_read16();
    vram_writep(offset, vm.oldsd7);
}

void oloctp(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oloctc(void) {
    u16 offset = script_read16();
    u16 ret = char_array_common(offset);
    vm.varD7 = vram_read8(ret);
}

void olocti(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

// reads a byte offset from script,
// then reads an extended byte from vram[offset] into r7
void odirb(void/* u8 offset */) {
    u8 offset = script_read8();
    vm.varD7 = vram_read8ext16(offset);
}

// reads a byte offset from script,
// then reads a word from vram[offset] into r7
void odirw(void/* u8 offset */) {
    u8 offset = script_read8();
    vm.varD7 = vram_read16(offset);;
}

// reads a byte offset from script,
// then reads a null-terminated data stream from vram[offset] into oldsd7
void odirp(void/* u8 offset */) {
    u8 offset = script_read8();
    vram_readp(offset, vm.oldsd7);
}

void odirtp(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void odirtc(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void odirti(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omainb(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omainw(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omainp(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omaintp(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omaintc(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omainti(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ohimb(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ohimw(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ohimp(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ohimtp(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ohimtc(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ohimti(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

// pop from accumulator into r6
void opile(void) {
    // save r6 into r7
    vm.varD7 = vm.varD6;
    vm.varD6 = *(vm.acc++);
}

// start eval loop, will stop after ofin() is called
void oeval(void) {
    vm.oeval_loop = 1;
    while(vm.oeval_loop) {
        readexec_opername();
    }
}

// stop eval loop
void ofin(void) {
    vm.oeval_loop = 0;
}

// push value from r7 register to accumulator
void opushacc(void) {
    *(--vm.acc) = vm.varD7;
}

// r7 = variable AND r7
void oand(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 &= vm.varD6;
}

// r7 = variable OR r7
void oor(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 |= vm.varD6;
}

// r7 = variable XOR r7
void oxor(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 ^= vm.varD6;
}

// r7 = variable EQV r7
void oeqv(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 ^= vm.varD6;
    vm.varD7 = ~vm.varD7;
}

// r6 == r7
void oegal(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 = (vm.varD6 == vm.varD7) ? 0xff : 0x0;
}

// r6 != r7
void odiff(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 = (vm.varD6 != vm.varD7) ? 0xff : 0x0;
}

// r6 <= r7
void oinfeg(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 = (vm.varD6 <= vm.varD7) ? 0xff : 0x0;
}

// r6 >= r7
void osupeg(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 = (vm.varD6 >= vm.varD7) ? 0xff : 0x0;
}

// r6 < r7
void oinf(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 = (vm.varD6 < vm.varD7) ? 0xff : 0x0;
}

// r6 > r7
void osup(void) {
    vm.varD6 = vm.varD7;
    readexec_opername();
    vm.varD7 = (vm.varD6 > vm.varD7) ? 0xff : 0x0;
}

// r7 += variable
void oadd(void) {
//00017932 61 00 fc 38     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00017936 de 46           add.w      D6w,D7w
//00017938 4e 75           rts
    readexec_opername_saveD7();
    vm.varD7 += vm.varD6;
}

// r7 -= variable
void osub(void) {
//0001793a 61 00 fc 30     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//0001793e 9c 47           sub.w      D7w,D6w
//00017940 3e 06           move.w     D6w,D7w
//00017942 4e 75           rts
    readexec_opername_saveD7();
    vm.varD6 -= vm.varD7;
    vm.varD7 = vm.varD6;
}

// r7 %= variable
void omod(void) {
//00017944 61 00 fc 26     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00017948 48 c6           ext.l      D6
//0001794a 8d c7           divs.w     D7w,D6
//0001794c 2e 06           move.l     D6,D7
//0001794e 48 47           swap       D7
//00017950 4e 75           rts
    readexec_opername_saveD7();
    vm.varD6 %= vm.varD7;
    vm.varD7 = vm.varD6;
}

// r7 /= variable
void odiv(void) {
//00017952 61 00 fc 18     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00017956 48 c6           ext.l      D6
//00017958 8d c7           divs.w     D7w,D6
//0001795a 3e 06           move.w     D6w,D7w
//0001795c 4e 75           rts
    readexec_opername_saveD7();
    vm.varD6 /= vm.varD7;
    vm.varD7 = vm.varD6;
}

// r7 *= variable
void omul(void) {
//0001795e 61 00 fc 0c     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00017962 cf c6           muls.w     D6w,D7
//00017964 4e 75           rts
    readexec_opername_saveD7();
    vm.varD7 *= vm.varD6;
}


void oneg(void) {
    vm.varD7 = -vm.varD7;
}


void oabs(void) {
    if(vm.varD7 < 0) {
        vm.varD7 = -vm.varD7;
    }
}

void ornd(void) {
    vm.varD7 = sys_random();
}

void osgn(void) {
    if(vm.varD7 > 0) {
        vm.varD7 = 1;
    }
    else if(vm.varD7 < 0) {
        vm.varD7 = 0xffff;
    }
}

void onot(void) {
    vm.varD7 = ~vm.varD7;
}

void oinkey(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void okeyon(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ojoy(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oprnd(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oscan(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oshiftkey(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ofree(void) {
    debug(EDebugWarning, "\n%s SIMULATED\n", __FUNCTION__);
    vm.varD7 = 0x321;
}

void omodel(void) {
    vm.varD7 = sys_get_model();
}

void ogetkey(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oleft(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oright(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omid(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void olen(void) {
    vm.varD7 = strlen((const char *)vm.oldsd7);
}

void oasc(void) {
    vm.varD7 = 0;
    vm.varD7 = vm.oldsd7[0];
}

void ostr(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void osadd(void) {
    // TODO: strcat ??
    debug(EDebugWarning, " /* STUBBED */");
}

void osegal(void) {
    // TODO: strcmp ??
    debug(EDebugWarning, " /* STUBBED */");
}

void osdiff(void) {
    // TODO: !strcmp ??
    debug(EDebugWarning, " /* STUBBED */");
}

void osinfeg(void) {
    // TODO: string equ or < ??
    debug(EDebugWarning, " /* STUBBED */");
}

void ossupeg(void) {
    // TODO: string equ or > ??
    debug(EDebugWarning, " /* STUBBED */");
}

void osinf(void) {
    // TODO: string < ??
    debug(EDebugWarning, " /* STUBBED */");
}

void ossup(void) {
    // TODO: string > ??
    debug(EDebugWarning, " /* STUBBED */");
}

void ospushacc(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void ospile(void) {
    u8 * tmp = vm.oldsd7;
    vm.oldsd7 = vm.sd6;
    vm.sd6 = tmp;
    
    debug(EDebugWarning, " /* STUBBED */");
}

void oval(void) {
    // TODO: compute int value of oldsd7 string -> d7 ??
    debug(EDebugWarning, " /* STUBBED */");
}

void oexistf(void) {
    char path[kPathMaxLen] = {0};
    strcpy(path, vm.platform.path);
    strcat(path, (char *)vm.oldsd7);
    vm.varD7 = sys_fexists(path) ? 0xffff : 0x0;
}

void ochr(void) {
    vm.oldsd7[0] = (u8)vm.varD7;
}

void ochange(void) {
    // TODO: change le drive courant ??
    debug(EDebugWarning, " /* STUBBED */");
}

void ocountry(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void omip(void) {
    vm.varD7 = 0x64;
}

void ojoykey(void) {
    debug(EDebugWarning, " /* STUBBED */");
}

void oconfig(void) {
    vm.varD7 = 0;
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
                "if file name in oldsd7 exists, returns 0xffff in varD7, else 0"),
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
