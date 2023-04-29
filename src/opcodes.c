//
//  opcodes.c
//  alis
//
//

#include "alis.h"
#include "alis_private.h"
#include "utils.h"

#include "experimental.h"

#define BIT_SCAN        (0)
#define BIT_INTER       (1)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// ============================================================================
#pragma mark - Experimental
// ============================================================================

int putdataidx = 0;

s16 putdata[][4] = {
    { 0x0000, 0x0000, 0xfffb, 0x0a },
    { 0x00a0, 0x0003, 0x002b, 0x10 },
    { 0x0000, 0x0000, 0x0000, 0x05 },
    { 0x0000, 0x0000, 0xfffb, 0x09 },
    { 0x0050, 0x0002, 0x0034, 0x0b },
    { 0x0052, 0x0002, 0x0028, 0x0b },
    { 0x0054, 0x0002, 0x0020, 0x0b },
    { 0x0000, 0x0000, 0xfffb, 0x0a },
    { 0x00be, 0x0003, 0x002b, 0x10 },
    { 0x0046, 0x0002, 0x0028, 0x13 },
    { 0x00c3, 0x0003, 0x002b, 0x10 },
    { 0x004b, 0x0002, 0x0028, 0x12 },
    { 0x00c8, 0x0003, 0x002b, 0x10 },
    { 0x0050, 0x0002, 0x0028, 0x13 },
    { 0x00cd, 0x0003, 0x002b, 0x10 },
    { 0x0055, 0x0002, 0x0028, 0x11 },
    { 0x00d2, 0x0003, 0x002b, 0x10 },
    { 0x005a, 0x0002, 0x0028, 0x12 },
    { 0x00d7, 0x0003, 0x002b, 0x10 },
    { 0x005f, 0x0002, 0x0028, 0x13 },
    { 0x00dc, 0x0003, 0x002b, 0x10 },
    { 0x0064, 0x0002, 0x0028, 0x14 },
    { 0x00e1, 0x0003, 0x002b, 0x10 },
    { 0x0069, 0x0002, 0x0028, 0x11 },
    { 0x00e6, 0x0003, 0x002b, 0x10 },
    { 0x006e, 0x0002, 0x0028, 0x13 },
    { 0x00eb, 0x0003, 0x002b, 0x10 },
    { 0x0073, 0x0002, 0x0028, 0x14 },
    { 0x00f0, 0x0003, 0x002b, 0x10 },
    { 0x0078, 0x0002, 0x0028, 0x13 },
    { 0x00f5, 0x0003, 0x002b, 0x10 },
    { 0x007d, 0x0002, 0x0028, 0x14 },
    { 0x00fa, 0x0003, 0x002b, 0x10 },
    { 0x0082, 0x0002, 0x0028, 0x13 },
    { 0x00ff, 0x0003, 0x002b, 0x10 },
    { 0x0087, 0x0002, 0x0028, 0x11 },
    { 0x0104, 0x0003, 0x002b, 0x10 },
    { 0x008c, 0x0002, 0x0028, 0x13 },
    { 0x0109, 0x0003, 0x002b, 0x10 },
    { 0x0091, 0x0002, 0x0028, 0x12 },
    { 0x010e, 0x0003, 0x002b, 0x10 },
    { 0x0096, 0x0002, 0x0028, 0x11 },
    { 0x0113, 0x0003, 0x002b, 0x10 },
    { 0x009b, 0x0002, 0x0028, 0x14 },
    { 0x0118, 0x0003, 0x002b, 0x10 },
    { 0x00a0, 0x0002, 0x0028, 0x11 },
    { 0x011d, 0x0003, 0x002b, 0x10 },
    { 0x00a5, 0x0002, 0x0028, 0x12 },
    { 0x0122, 0x0003, 0x002b, 0x10 },
    { 0x00aa, 0x0002, 0x0028, 0x13 },
    { 0x0127, 0x0003, 0x002b, 0x10 },
    { 0x00af, 0x0002, 0x0028, 0x11 },
    { 0x012c, 0x0003, 0x002b, 0x10 },
    { 0x00b4, 0x0002, 0x0028, 0x12 },
    { 0x0131, 0x0003, 0x002b, 0x10 },
    { 0x00b9, 0x0002, 0x0028, 0x14 },
    { 0x0136, 0x0003, 0x002b, 0x10 },
    { 0x00be, 0x0002, 0x0028, 0x13 },
    { 0x013b, 0x0003, 0x002b, 0x10 },
    { 0x00c3, 0x0002, 0x0028, 0x12 },
    { 0x0140, 0x0003, 0x002b, 0x10 },
    { 0x00c8, 0x0002, 0x0028, 0x11 },
    { 0x0145, 0x0003, 0x002b, 0x10 },
    { 0x00cd, 0x0002, 0x0028, 0x13 },
    { 0x014a, 0x0003, 0x002b, 0x10 },
    { 0x00d2, 0x0002, 0x0028, 0x12 },
    { 0x014f, 0x0003, 0x002b, 0x10 },
    { 0x00d7, 0x0002, 0x0028, 0x11 },
    { 0x0154, 0x0003, 0x002b, 0x10 },
    { 0x00dc, 0x0002, 0x0028, 0x13 },
    { 0x0159, 0x0003, 0x002b, 0x10 },
    { 0x00e1, 0x0002, 0x0028, 0x12 },
    { 0x015e, 0x0003, 0x002b, 0x10 },
    { 0x00e6, 0x0002, 0x0028, 0x13 },
    { 0x0163, 0x0003, 0x002b, 0x10 },
    { 0x00eb, 0x0002, 0x0028, 0x12 },
    { 0x0168, 0x0003, 0x002b, 0x10 },
    { 0x00f0, 0x0002, 0x0028, 0x13 },
    { 0x016d, 0x0003, 0x002b, 0x10 },
    { 0x00f5, 0x0002, 0x0028, 0x11 },
    { 0x0172, 0x0003, 0x002b, 0x10 },
    { 0x00fa, 0x0002, 0x0028, 0x13 },
    { 0x0177, 0x0003, 0x002b, 0x10 },
    { 0x00ff, 0x0002, 0x0028, 0x14 },
    { 0x017c, 0x0003, 0x002b, 0x10 },
    { 0x0104, 0x0002, 0x0028, 0x11 },
    { 0x0181, 0x0003, 0x002b, 0x10 },
    { 0x0109, 0x0002, 0x0028, 0x12 },
    { 0x0186, 0x0003, 0x002b, 0x10 },
    { 0x010e, 0x0002, 0x0028, 0x14 },
    { 0x018b, 0x0003, 0x002b, 0x10 },
    { 0x0113, 0x0002, 0x0028, 0x13 },
    { 0x0190, 0x0003, 0x002b, 0x10 },
    { 0x0118, 0x0002, 0x0028, 0x11 },
    { 0x0195, 0x0003, 0x002b, 0x10 },
    { 0x011d, 0x0002, 0x0028, 0x14 },
    { 0x019a, 0x0003, 0x002b, 0x10 },
    { 0x0122, 0x0002, 0x0028, 0x11 },
    { 0x019f, 0x0003, 0x002b, 0x10 },
    { 0x0127, 0x0002, 0x0028, 0x13 },
    { 0x01a4, 0x0003, 0x002b, 0x10 },
    { 0x012c, 0x0002, 0x0028, 0x11 },
    { 0x01a9, 0x0003, 0x002b, 0x10 },
    { 0x0131, 0x0002, 0x0028, 0x14 },
    { 0x01ae, 0x0003, 0x002b, 0x10 },
    { 0x0136, 0x0002, 0x0028, 0x13 },
    { 0x01b3, 0x0003, 0x002b, 0x10 },
    { 0x013b, 0x0002, 0x0028, 0x11 },
    { 0x01b8, 0x0003, 0x002b, 0x10 },
    { 0x0140, 0x0002, 0x0028, 0x14 },
    { 0x01bd, 0x0003, 0x002b, 0x10 },
    { 0x0145, 0x0002, 0x0028, 0x12 },
    { 0x01c2, 0x0003, 0x002b, 0x10 },
    { 0x014a, 0x0002, 0x0028, 0x11 },
    { 0x01c7, 0x0003, 0x002b, 0x10 },
    { 0x014f, 0x0002, 0x0028, 0x14 },
    { 0x01cc, 0x0003, 0x002b, 0x10 },
    { 0x0154, 0x0002, 0x0028, 0x11 },
    { 0x01d1, 0x0003, 0x002b, 0x10 },
    { 0x0159, 0x0002, 0x0028, 0x14 },
    { 0x01d6, 0x0003, 0x002b, 0x10 },
    { 0x015e, 0x0002, 0x0028, 0x12 },
    { 0x01db, 0x0003, 0x002b, 0x10 },
    { 0x0163, 0x0002, 0x0028, 0x14 },
    { 0x0106, 0x0000, 0x003e, 0x02 },
    { 0x0106, 0x0000, 0x003e, 0x03 },
    { 0x0106, 0x0000, 0x003e, 0x04 },
    { 0x0106, 0x0000, 0x003e, 0x03 },
    { 0x0106, 0x0000, 0x003e, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x04 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x04 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x02 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x04 },
    { 0x0074, 0x0000, 0x0079, 0x03 },
    { 0x0074, 0x0000, 0x0079, 0x02 }};

#pragma mark - additions

void killent(u16 d0w, u16 d3, u16 d5w);
void runson(void);

// ============================================================================
#pragma mark - Opcodes
// ============================================================================
static void crstent(void);

static void cstore_continue() {
    // swap chunk 1 / 3
    u8 * tmp = alis.bsd7;
    alis.bsd7 = alis.bsd7bis;
    alis.bsd7bis = tmp;
    
    readexec_storename();
}

static void cstore() {
    readexec_opername_saveD7();
    cstore_continue();
}

static void ceval() {
    readexec_opername_saveD7();
}

static void cadd() {
    readexec_opername_saveD7();
    readexec_addname_swap();
}

static void csub() {
    readexec_opername_saveD7();
    alis.varD7 *= -1;
    readexec_addname_swap();
}

static void cvprint() {
    debug(EDebugWarning, " /* STUBBED */");
    alis.charmode = 0;
    readexec_opername_saveD7();

    // TODO: ...
}

static void csprinti() {
    alis.charmode = 0;
    for (u8 c = script_read8(); c != 0; c = script_read8())
    {
        alis_putchar(c);
    }
}

static void csprinta() {
    alis.charmode = 0;
    readexec_opername_saveD7();
    alis_putstring();
}

static void clocate() {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    s16 xlocate = alis.varD7;
    s16 ylocate = alis.varD6;
//    io_locate();
}

static void ctab() {
    debug(EDebugWarning, " /* MISSING */");
}

// copy values to vram
// cdim(u16 offset, u8 counter, u8 value, optional u16[] values)
static void cdim() {

    // read word param
    u16 offset = script_read16();
    u8 counter = script_read8();
    u8 byte2 = script_read8();
    
    vram_write8(--offset, counter);
    vram_write8(--offset, byte2);
    
    // loop w/ counter, read words, store backwards
    while(counter--) {
        u16 w = script_read16();
        offset -= 2;
        vram_write16(offset, w);
    }
}

static void crandom() {
    readexec_opername();
    alis._random_number = alis.varD7;
    if(alis._random_number == 0) {
        alis._random_number = sys_random();
        // test key/joy input, if zero, random is set to $64 ???
    }
}

u32 save_loop_pc;

// cloopX(jmp_offset, addname(ram_offset))
// decrement value in RAM[offset2]
// if obtained value is zero, then jump with jmp_offset
static void cloop(s32 offset) {
    save_loop_pc = alis.script->pc;
    alis.varD7 = -1;
    readexec_addname_swap();
    if(!alis.sr.zero)
    {
        alis.script->pc = save_loop_pc;
        script_jump(offset);
    }
}

static void cloop8() {
    cloop(script_read8ext16());
}

static void cloop16() {
    cloop(script_read16());
}

static void cloop24() {
    cloop(script_read24());
}

static void cswitch1() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cswitch2() {
    readexec_opername();
    
    s16 addition = script_read8();
    s32 new_pc = alis.script->pc + 0;
    if ((new_pc & 1) != 0)
    {
        new_pc = alis.script->pc + 1;
    }

    s16 val = alis.varD7 + read16(alis.mem + new_pc, alis.platform.is_little_endian);
    if (val >= 0 && val <= addition)
    {
        new_pc += (val * 2) + 2;
        new_pc += read16(alis.mem + new_pc, alis.platform.is_little_endian) + 2;
    }
    else
    {
        new_pc = alis.script->pc + (addition * 2 + 2) + 2;
    }

    alis.script->pc = new_pc;
}

static void cleave() {
    debug(EDebugWarning, " /* MISSING */");
//    **************************************************************
//    *                          FUNCTION                          *
//    **************************************************************
//    undefined OPCODE_CLEAVE_0x33()
//undefined         D0b:1          <RETURN>
//    OPCODE_CLEAVE_0x33
//00013206 4a 39 00        tst.b      (B_CSTOPRET).l
//01 95 a3
//0001320c 66 00 00 1c     bne.w      LAB_0001322a
//00013210 32 2e ff f4     move.w     (-0xc,A6),D1w
//00013214 67 00 00 34     beq.w      OPCODE_CRET_0x11
//00013218 42 6e ff f4     clr.w      (-0xc,A6)
//0001321c 2d 76 10        move.l     (0x0,A6,D1w*0x1),(-0x8,A6)
//00 ff f8
//00013222 58 41           addq.w     #0x4,D1w
//00013224 3d 41 ff f6     move.w     D1w,(-0xa,A6)
//00013228 4e 75           rts
//    LAB_0001322a                                    XREF[1]:     0001320c(j)
//0001322a 38 2e ff f4     move.w     (-0xc,A6),D4w
//0001322e 67 00 00 1a     beq.w      OPCODE_CRET_0x11
//00013232 42 6e ff f4     clr.w      (-0xc,A6)
//00013236 26 76 40 00     movea.l    (0x0,A6,D4w*0x1),A3
//0001323a 58 44           addq.w     #0x4,D4w
//0001323c 4e 75           rts

}

static void cprotect() {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername();
    // alis.vprotect = alis.varD7;
}

static void casleep() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscset() {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername_saveD7();
    s16 x = alis.varD7;
    readexec_opername_saveD7();
    s16 y = alis.varD7;
    readexec_opername_saveD7();
    s16 d = alis.varD7;
    
//    u8 *sprite = alis.spritemem + alis.script->context->_0x18_unknown;
//    if (d != *(s16 *)(sprite + 0x1a))
//    {
//        *(s16 *)(sprite + 0x1a) = d;
//        *sprite = *sprite | 0x80;
//    }
//
//    if (y != *(s16 *)(sprite + 0x18))
//    {
//        *(s16 *)(sprite + 0x18) = y;
//        *sprite = *sprite | 0x80;
//    }
//
//    if (x != *(s16 *)(sprite + 0x16))
//    {
//        *(s16 *)(sprite + 0x16) = x;
//        *sprite = *sprite | 0x80;
//    }
}

static void cclipping() {
    alis._cclipping = 0;
}

static void cswitching() {
    alis._cclipping = 1;
    debug(EDebugWarning, " /* SIMULATED */");
}

static void cwlive() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cunload() {
    debug(EDebugWarning, " /* STUBBED */");
    s16 id = script_read16();
    if (id != -1 && id != alis.script->context->_0x10_script_id)
    {
        s16 index = debprotf(id);
        if (index != -1)
        {
            sAlisScript *script = alis.scripts[index];
            shrinkprog(script, 1);

            // s16 *a2 = (s16 *)0xfffffff;
            // s16 *a0 = *(s16 **)(alis.mem + index + atprog);
            //
            // for (s16 **curprog = atprog; curprog != dernprog; curprog++)
            // {
            //     if (((s32)a0 < (s32)*curprog) && ((s32)*curprog < (s32)a2))
            //     {
            //         a2 = *curprog;
            //     }
            // }
            //
            // if (a2 == (s16 *)0xfffffff)
            // {
            //     a2 = finprog;
            // }
            //
            // shrinkprog(a0, (s32)(s16 **)(index + (s32)atprog), a2, (s32)a2 - (s32)a0, index, 1) ;
        }

        // a0 = 34ba8
        // a1 = 22404
        // a2 = 39df0
        // d1 = 5248
        // d2 = 1
        // d7 = 1
        // shrinkprog(script, script->sz, id, 1) ;
    }
}

static void cwakeup() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cstop(void);

static void csleep() {
    alis.script->context->_0x4_cstart_csleep = 0;
    if (alis.fseq)
    {
        cstop();
    }
}

//void livescript(u16 *prog, u16 curent)
//{
//    sScriptLoc *curloc = (sScriptLoc *)(alis.mem + alis.atent + curent);
//    u16 nextent = curloc->offset;
//    curloc->offset = alis.dernent;
//
//    sScriptLoc *endloc = (sScriptLoc *)(alis.mem + alis.atent + alis.dernent);
//    alis.dernent = endloc->offset;
//    endloc->offset = nextent;
//
//    alis.nbent ++;
//
//    if (alis.maxent < alis.nbent)
//    {
//        // TODO: error
//        return;
//    }
//
//    s32 iVar6 = (short)prog[9] + alis.finent;
//    u16 *puVar7 = (u16 *)((short)prog[0xb] + iVar6);
//    u16 *puVar8 = puVar7 + 0x1a;
//    *(u16 **)(alis.mem + alis.atent + alis.dernent) = puVar8;
//    u16 uVar4 = (uint)(ushort)prog[10];
//    s32 iVar1 = uVar4 + (int)puVar8;
//    if (alis.debsprit <= (int)(uVar4 + (int)puVar8))
//    {
//        // TODO: error
//        return;
//    }
//
//    do
//    {
//        alis.finent = iVar1;
//        *(u16 *)((int)puVar7 + uVar4 + 0x32) = 0;
//        uVar4 = uVar4 - 2;
//        iVar1 = alis.finent;
//    }
//    while (uVar4 != 0);
//
//    s16 sVar5 = (short)iVar6 - (short)puVar8;
//    puVar7[0x15] = sVar5;
//    puVar7[0xc] = sVar5;
//    puVar7[0xb] = sVar5;
//    *(u32 *)(puVar7 + 0x16) = (u16)prog[2] + 2 + (int)prog;
//    puVar7[0x12] = *prog;
//    *(u16 **)(puVar7 + 0x10) = prog;
//    *(u8 *)(puVar7 + 3) = *(u8 *)(prog + 1);
//    *(u8 *)(puVar7 + 0x19) = 1;
//    *(u8 *)((int)puVar7 + 0x33) = 1;
//    *(u8 *)(puVar7 + 0x18) = 0xff;
//    puVar7[0xd] = 0xffff;
//    puVar7[0x13] = alis.dernent;
//    *(u8 *)(puVar7 + 8) = 2;
//    puVar7[0xe] = 0;
//    puVar7[0x14] = 0;
//    puVar7[9] = 0;
//    puVar7[10] = 0;
//    *(u8 *)((int)puVar7 + 0x31) = 0;
//    *(u8 *)((int)puVar7 + 0xf) = 0;
//    *(u8 *)(puVar7 + 7) = 0xff;
//    *(u8 *)(puVar7 + 5) = 0;
//    *(u8 *)(puVar7 + 4) = 0;
//    *(u8 *)((int)puVar7 + 7) = 0;
//    puVar7[6] = 0;
//    *(u8 *)((int)puVar7 + 5) = 0;
//    puVar7[1] = 0;
//    *puVar7 = 0;
//    *(u8 *)(puVar7 + 2) = 0;
//}

static void clive() {
    debug(EDebugWarning, " /* STUBBED */");
    alis.wcx = 0;
    alis.wcy = 0;
    alis.wcz = 0;
    
    s16 id = script_read16();
    if (id != -1)
    {
        s16 index = debprotf(id);
        if (index != -1)
        {
            sAlisScript *prog = alis.progs[index];

            sAlisScript *script = (sAlisScript *)malloc(sizeof(sAlisScript));
            memcpy(script, prog, sizeof(sAlisScript));
            script->vram_org = 0;
            
            index = alis.nbent;
            script_live(script);
            
            s16 curent = alis.varD5;
            alis.varD7 = alis.dernent;
            
            u8 *prev_vram = alis.mem + alis.script->vram_org;
            u8 *live_vram = alis.mem + script->vram_org;
            memcpy(live_vram, prev_vram, 8);
            
            *(s16 *)(live_vram + 0) += alis.wcx;
            *(s16 *)(live_vram + 2) += alis.wcy;
            *(s16 *)(live_vram + 1) += alis.wcz;
            
            *(u8 *)(live_vram + 0x9) = *(u8 *)(prev_vram + 0x9);
            *(u8 *)(live_vram + 0xa) = *(u8 *)(prev_vram + 0xa);
            *(u8 *)(live_vram + 0xb) = *(u8 *)(prev_vram + 0xb);
            
            script->context->_0x10_script_id = alis.script->context->_0x10_script_id;
            script->context->_0x16_screen_id = alis.script->context->_0x16_screen_id;
            script->context->_0x22_cworld    = alis.script->context->_0x22_cworld;
            script->context->_0x2a_clinking  = alis.script->context->_0xe_czap_cexplode_cnoise;
            
            sScriptLoc *curloc = (sScriptLoc *)(alis.mem + alis.atent + curent);
            u16 nextent = curloc->offset;
            curloc->offset = alis.dernent;
        
            sScriptLoc *endloc = (sScriptLoc *)(alis.mem + alis.atent + alis.dernent);
            alis.dernent = endloc->offset;
            endloc->offset = nextent;
            endloc->vram_offset = index;
        
            if (alis.maxent < alis.nbent)
            {
                // TODO: error
                return;
            }
        }
    }
    
    cstore_continue();
}

static void ckill() {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername_saveD7();
    if (alis.varD7 < 0)
        return;

    killent(alis.varD7, alis.varD5, 0);
}

static void cstop() {
    // in real program, adds 4 to real stack pointer
    alis.script->running = 0;
    printf("\n-- CSTOP --");
}

static void cstopret() {
    // never seen in ishar execution (boot2game)
    debug(EDebugWarning, " /* MISSING */");                                     
}

static void cexit() {
    if (alis.varD5 == 0)
    {
        exit(-1);
        return;
    }

    killent(alis.varD5, alis.script->context->_0x16_screen_id, alis.varD5);
    alis.script->running = 0;
}

static void cload() {
    // get script ID
    u16 id = script_read16();
    if(id == 0) {
        // main script !
        readexec_opername_swap();
        
        // TODO: init some stuff ?
        
        // reset vm ?
        // in ishar this is never reached
    }
    else {
        // not main script, depack and load into vm
        char path[kPathMaxLen] = {0};
        strcpy(path, alis.platform.path);
        script_read_until_zero((u8 *)(path + strlen(alis.platform.path)));
        strcpy(strrchr(path, '.') + 1, alis.platform.ext);
        
        sAlisScript * script = script_load(strlower((char *)path));
    }
}

// =============================================================================
void FUN_00013d82() {
//    clr.w      ($4,A0,D0)
//    move.w     (W_000195a0).l,D1
//
//    bne.w      __not_zero
//    move.w     D0,(W_000195a0).l
//    rts
//__not_zero:
//    move.w     D1,D2
//    move.w     ($4,A0,D2),D1
//    bne.w      __not_zero
//    move.w     D0,($4,A0,D2)
//    rts
}
// =============================================================================
void FUN_00013dda() {
//    move.b         ($21,A0),D0
//    ext.w          D0
//    move.b         ($25,A0),D1
//    ext.w          D1
//    muls.w         D1,D0
//    move.b         ($24,A0),D2
//    ext.w          D2
//    move.b         ($22,A0),D3
//    ext.w          D3
//    muls.w         D3,D2
//    sub.w          D2,D0
//    move.b         D0,($26,A0)
//    move.b         ($22,A0),D0
//    ext.w          D0
//    move.b         ($23,A0),D1
//    ext.w          D1
//    muls.w         D1,D0
//    move.b         ($25,A0),D2
//    ext.w          D2
//    move.b         ($20,A0),D3
//    ext.w          D3
//    muls.w         D3,D2
//    sub.w          D2,D0
//    move.b         D0,($27,A0)
//    move.b         ($20,A0),D0
//    ext.w          D0
//    move.b         ($24,A0),D1
//    ext.w          D1
//    muls.w         D1,D0
//    move.b         ($23,A0),D2
//    ext.w          D2
//    move.b         ($21,A0),D3
//    ext.w          D3
//    muls.w         D3,D2
//    sub.w          D2,D0
//    move.b         D0,($28,A0)
//    rts
//    move.b         ($21,A0),D0
//    ext.w          D0
//    move.b         ($25,A0),D1
//    ext.w          D1
//    muls.w         D1,D0
//    move.b         ($24,A0),D2
//    ext.w          D2
//    move.b         ($22,A0),D3
//    ext.w          D3
//    muls.w         D3,D2
}


// reads 35 bytes
static void cdefsc() {
    /*
     ; code address: $139ca
     opcode_cdefsc:
         move.b         (A3)+,D0
         asl.w          #$8,D0
         move.b         (A3)+,D0
    */
    u16 offset = script_read16();
    /*
         movea.l        (ADDR_VSTACK).l,A0 ; correspond à a6 !!! / A0 vaut $224f0, contient $22690 soit vstack
     */
    u8 * ram = vram_ptr(0);
    /*
         bset.b         #$6,(A0,D0)
     */
    *(ram + offset) &= ALIS_BIT_6;
    /*
         move.b         (A3)+,($1,A0,D0)
     */
    *(ram + offset + 1) = script_read8();
    // *(u8 *)(alis.spritemem - basemain + offset + 1)
    
    /*
         moveq          #$1f,D1
         lea            ($6,A0,D0),A1

     __loop32:
         move.b         (A3)+,(A1)+
         dbf            D1,__loop32
    */
    
    u8 counter = 32;
    u8 * ptr = ram + offset + 6;
    while(counter--) {
        *ptr++ = script_read8();
    }

    // basemain     0x00022690
    // basesprite   0x00031f40
    // libsprit     0x00008078 R
    // libsprit     0x000080a0 W
    
    // sprites 39daa, 373da, 38db0, 3892a, 

    
    /*
         movea.l        (ADDR_SCRIPT_0001955e).l,A1 ; qque part dans le script
         
         move.w         (W_00019562).l,D1
         
         beq.w          __error10
         move.w         ($4,A1,D1),(W_00019562).l
         
         
         move.w         D1,($2,A0,D0)
         clr.w          ($6,A1,D1)
         clr.w          ($4,A0,D0)
         move.b         ($1,A0,D0),($1,A1,D1)
         
         move.w         ($e,A0,D0),D2
         andi.w         #-$10,D2
         move.w         D2,($c,A1,D1)
         move.w         ($10,A0,D0),($e,A1,D1)
         
         move.w         #$7fff,($10,A1,D1)
         
         move.w         ($e,A0,D0),D2
         add.w          ($12,A0,D0),D2
         ori.w          #$f,D2
         move.w         D2,($16,A1,D1)
         move.w         ($10,A0,D0),D2
         add.w          ($14,A0,D0),D2
         move.w         D2,($18,A1,D1)
         clr.w          ($2a,A0,D0)
         clr.w          ($2c,A0,D0)
         clr.w          ($2e,A0,D0)
         bsr.w          FUN_00013d82
         lea            ($0,A0,D0),A0
         bsr.w          FUN_00013dda
         rts
         
     __error10:
         move.l     #$a,D0

     forever:
         bra forever
     */
    
    OPCODE_CDEFSC_0x46(ram, offset);
}

static void cscreen() {
    u16 screen_id = script_read16();
    if (screen_id != alis.script->context->_0x16_screen_id) {
        alis.script->context->_0x16_screen_id = screen_id;
    }
}

static void cput() {
    alis.flagmain = 0;
    *(u8 *)(&alis.flaginvx) = *(u8 *)(alis.mem + alis.script->context->_0x3_xinv);
    
    readexec_opername();
    
    alis.depx = 0;
    alis.depz = 0;
    alis.numelem = 0;
    
    u8 idx = alis.varD7;

    put(idx);
}

static void cputnat() {
    alis.flagmain = 0;
    *(u8 *)(&alis.flaginvx) = *(u8 *)(alis.mem + alis.script->context->_0x3_xinv);
    readexec_opername_saveD7();
    alis.depx = alis.varD7;
    readexec_opername_saveD7();
    alis.depy = alis.varD7;
    readexec_opername_saveD7();
    alis.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    
    alis.numelem = alis.varD6;
    u8 idx = alis.varD7;
    
    put(idx);
}

static void cerase() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cerasen() {
    readexec_opername_saveD7();
    alis.numelem = alis.varD7;
    
    u16 curidx = 0;
    u16 previdx = 0;

    while (1)
    {
        u8 ret = searchelem(&curidx, &previdx);
        if (ret == 0)
            break;
        
        killelem(&curidx, &previdx);
    }
    
    alis.ferase = 0;
}

static void cset() {
    readexec_opername();
    vram_write16(0, alis.varD7);
    readexec_opername();
    vram_write16(2, alis.varD7);
    readexec_opername();
    vram_write16(4, alis.varD7);
}

static void cmov() {
    readexec_opername();
    vram_add16(0, alis.varD7);
    readexec_opername();
    vram_add16(2, alis.varD7);
    readexec_opername();
    vram_add16(4, alis.varD7);
}

static void copensc() {
//    **************************************************************
//    *                          FUNCTION                          *
//    **************************************************************
//    undefined OPCODE_COPENSC_0x4e()
//undefined         D0b:1          <RETURN>
//    OPCODE_COPENSC_0x4e
//00013d4c 10 1b           move.b     (A3)+,D0b
//00013d4e e1 40           asl.w      #0x8,D0w
//00013d50 10 1b           move.b     (A3)+,D0b
//00013d52 20 79 00        movea.l    (ADDR_VSTACK).l,A0
//01 95 7c
//00013d58 08 b0 00        bclr.b     0x6,(0x0,A0,D0w*0x1)
//06 00 00
//00013d5e 08 f0 00        bset.b     0x7,(0x0,A0,D0w*0x1)
//07 00 00
//00013d64 61 00 00 42     bsr.w      FUN_00013da8                                     undefined FUN_00013da8()
//00013d68 61 00 00 18     bsr.w      FUN_00013d82                                     undefined FUN_00013d82()
//00013d6c 4e 75           rts
    u16 id = script_read16();
    vram_clrbit(id, ALIS_BIT_6);
    vram_setbit(id, ALIS_BIT_7);
    
    debug(EDebugWarning, " /* STUBBED */");
    
//    *(s8 *)(alis.mem + alis.script->vram_org + basemain + id) = *(s8 *)(alis.mem + alis.script->vram_org + basemain + id) & 0xbf;
//    *(s8 *)(alis.mem + alis.script->vram_org + basemain + id) = *(s8 *)(alis.mem + alis.script->vram_org + basemain + id) | 0x80;
//    scbreak(id);
//    scadd(id);
}

static void cclosesc() {
    debug(EDebugWarning, " /* STUBBED */");
    
    u16 id = script_read16();
    *(u8 *)(alis.mem + alis.basemain + id) |= 0x40;
}

static void cerasall() {
    u16 tmpidx = 0;
    u16 curidx = alis.script->context->_0x18_unknown;
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }
    
    alis.ferase = 0;
}

static void cforme() {
    readexec_opername();
    alis.script->context->_0x1a_cforme = alis.varD7;
}

static void cdelforme() {
    alis.script->context->_0x1a_cforme = -1;
}

void clipform(void)
{
//  int iVar1;
//  short sVar2;
//  short *psVar3;
//  int iVar4;
//  short *psVar5;
//  int unaff_A6;
//  u8 uVar6;
//
//  alis.ptrent = arr_tablent_256;
//  if (-1 < alis.wforme)
//  {
//    iVar4 = *(int *)(*(int *)(unaff_A6 + -0x14) + 0xe) + *(int *)(unaff_A6 + -0x14);
//    baseform = *(int *)(iVar4 + 6) + iVar4;
//    psVar5 = (short *)(*(short *)(alis.baseform + alis.wforme * 2) + alis.baseform);
//    sVar2 = 0;
//    iVar4 = addr_atent;
//    do
//    {
//      iVar1 = *(int *)(iVar4 + sVar2);
//      if (((*(short *)(iVar1 + 6) == *(short *)(unaff_A6 + 6)) && (-1 < *(short *)(iVar1 + -0x1a))) && (unaff_A6 != iVar1))
//      {
//        uVar6 = *psVar5 == 0;
//        if (*psVar5 < 0) {
//          multiform();
//        }
//        else
//        {
//          monoform();
//        }
//        if (!(u8)uVar6)
//        {
//          if (alis.witmov != '\0')
//          {
//              alis.fmitmov = 1;
//            return;
//          }
//          *(short *)((int)alis.ptrent + 0x100) = alis.goodmat;
//          psVar3 = (short *)((int)ptrent + 2);
//          *(short *)alis.ptrent = sVar2;
//          ptrent = (u8 *)psVar3;
//          if (fallent == '\0')
//          {
//              break;
//          }
//        }
//      }
//      sVar2 = *(short *)(iVar4 + 4 + (int)sVar2);
//    }
//      while (sVar2 != 0);
//  }
//  if (witmov == '\0')
//  {
//    *(short *)((int)alis.ptrent + 0x100) = 0;
//    *(short *)alis.ptrent = -1;
//      alis.ptrent = (u8 *)((int)alis.ptrent + 2);
//  }
}

static void ctstmov() {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername();
    s16 tmp = alis.varD7;
    alis.wcx = ((s16 *)(alis.mem + alis.script->vram_org))[0] + tmp;
    readexec_opername();
    tmp = alis.varD7;
    alis.wcy = ((s16 *)(alis.mem + alis.script->vram_org))[1] + tmp;
    readexec_opername();
    tmp = alis.varD7;
    alis.wcz = ((s16 *)(alis.mem + alis.script->vram_org))[2] + tmp;
    readexec_opername();
    alis.wforme = ((s16 *)(alis.mem + alis.script->vram_org))[-0xd];
    alis.matmask = alis.varD7;
    clipform();
    crstent();
}

static void ctstset() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cftstmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cftstset() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csuccent() {
    debug(EDebugWarning, " /* CHECK */");
    if (-1 < *alis.ptrent)
        alis.ptrent ++;

    cstore_continue();
}

static void cpredent() {
    debug(EDebugWarning, " /* CHECK */");
    if (alis.ptrent != alis.tablent)
        alis.ptrent --;

    cstore_continue();
}

static void cnearent() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cneartyp() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cnearmat() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewent() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewtyp() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewmat() {
    debug(EDebugWarning, " /* MISSING */");
}

static void corient() {
    debug(EDebugWarning, " /* MISSING */");
}

static void crstent() {
    debug(EDebugWarning, " /* CHECK */");
    alis.ptrent = alis.tablent;
    csuccent();
}

static void csend() {
    debug(EDebugWarning, " /* STUBBED */");
    
    u16 length = script_read8();
    readexec_opername();
    s16 id = alis.varD7;
    if (id == -1)
        goto LAB_0001613e;
    
    sScriptLoc *entry = &(alis.script_vram_orgs[id / sizeof(sScriptLoc)]);
    u32 index = entry->vram_offset; // prevent / sizeof(sScriptLoc);
    sAlisScript *script = alis.scripts[index];
    if (script->vram_org == 0)
        goto LAB_0001613e;
    
    if ((script->context->_0x24_scan_inter.data & 1) != 0)
        goto LAB_0001613e;
    
    u8 *script_addr = alis.mem + script->vram_org;
    s16 sVar2 = swap16((alis.mem + script->context->_0x14_script_org_offset + 0x16), alis.platform.is_little_endian);
    s16 sVar4 = script->context->_0x1c_scan_clr; // expected value 0xffac
    s16 sVar5;

    while (1)
    {
        readexec_opername();

        sVar5 = sVar4 + 2;
        if (-0x35 < sVar5)
        {
            sVar5 -= sVar2;
        }
        
        if (sVar5 == script->context->_0x1e_scan_clr)
            break;
        
        *(s16 *)(script_addr + sVar4) = alis.varD7;
        
        length --;
        sVar4 = sVar5;
        if (length == 0xffff)
        {
            script->context->_0x1c_scan_clr = sVar5;
            script->context->_0x24_scan_inter.data |= 0x80;
            return;
        }
    }
    
    script->context->_0x1c_scan_clr = sVar5;
    
    while ((--length) != 0xffff)
    {
LAB_0001613e:
        readexec_opername();
    }
}

static void cscanclr() {
//    u16 w = vram_read16(VRAM_OFFSET_OSCAN_OSCANCLR_2);
//    vram_write16(VRAM_OFFSET_OSCAN_OSCANCLR_1, w);
//    vram_clrbit(VRAM_OFFSET_CSCAN_CINTER, 7);

    alis.script->context->_0x1e_scan_clr = alis.script->context->_0x1c_scan_clr;
    alis.script->context->_0x24_scan_inter.scan_clr_bit_7 = 0;
}

static void cscanon() {
    alis.script->context->_0x24_scan_inter.scan_off_bit_0 = 0;
//    vram_clrbit(VRAM_OFFSET_CSCAN_CINTER, 0);
}

static void cscanoff() {
    alis.script->context->_0x24_scan_inter.scan_off_bit_0 = 1;
//    vram_setbit(VRAM_OFFSET_CSCAN_CINTER, 0);
    cscanclr();
}

static void cinteron() {
    alis.script->context->_0x24_scan_inter.inter_off_bit_1 = 0;
//    vram_clrbit(VRAM_OFFSET_CSCAN_CINTER, 1);
}

static void cinteroff() {
    alis.script->context->_0x24_scan_inter.inter_off_bit_1 = 1;
//    vram_setbit(VRAM_OFFSET_CSCAN_CINTER, 1);
}

static void callentity() {
    alis.fallent = 1;
}

static void cpalette() {
    alis.flagmain = 0;

    readexec_opername();
    u16 palidx = alis.varD7;
    u16 duration = 0;
    
    if (palidx < 0)
    {
        // TODO: ...
        debug(EDebugWarning, " /* STUBBED */");
    }
    else
    {
        if (alis.nmode != 1 && alis.nmode != 5)
        {
        }
        
        s32 addr = adresdes(palidx);
        u8 *paldata = alis.mem + alis.script->data_org + addr;
        ctopalette(paldata, duration);
    }
}

static void cdefcolor() {
    debug(EDebugWarning, " /* MISSING */");
}

static void ctiming() {
    readexec_opername();
    alis._ctiming = (u8)(alis.varD7 & 0xff);
}

static void czap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cexplode() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cding() {
    debug(EDebugWarning, " /* STUBBED */");
    
    alis.pereson = alis.script->context->_0xe_czap_cexplode_cnoise;
    readexec_opername();
    alis.priorson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    readexec_opername();
    alis.freqson = alis.varD7;
    readexec_opername();
    alis.longson = alis.varD7;
    if (alis.longson == 0)
      return;

    alis.dfreqson = 0;
    u32 tmpa = ((u32)alis.volson << 8) / (u32)alis.longson;
    u32 tmpb = ((u32)alis.volson << 8) % (u32)alis.longson;
    u32 newdvolson = tmpa == 0 ? 1 : tmpb << 0x10 | tmpa;

    alis.dvolson = -(s16)newdvolson;
    alis.typeson = 1;
    runson();
}

static void cnoise() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cinitab() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfopen() {
    u16 id = 0;
    if(*(alis.mem + alis.script->pc) == 0xff) {
//        ++(alis.script->pc);
        script_jump(1);
        readexec_opername_swap();
        readexec_opername();
    }
    else {
        char path[kPathMaxLen] = {0};
        strcpy(path, alis.platform.path);
        script_read_until_zero((u8 *)(path + strlen(alis.platform.path)));
        id = script_read16();
        alis.fp = sys_fopen((char *)path);
        if(alis.fp == NULL) {
            alis_error(ALIS_ERR_FOPEN, path);
        }
    }
}

static void cfclose() {
    if(sys_fclose(alis.fp) < 0) {
        alis_error(ALIS_ERR_FCLOSE);
    }
}

static void cfcreat() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfdel() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfreadv() {
    debug(EDebugWarning, " /* CHECK */");
    fread(alis.buffer, 2, 1, alis.fp);
    cstore_continue();
}

static void cfwritev() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfwritei() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfreadb() {
    u32 addr = script_read16();
    u16 length = script_read16();

    if (addr == 0)
    {
        addr = script_read16();
        addr += alis.basemain;
    }
    else
    {
        addr += alis.script->vram_org;
    }
    
    fread(alis.mem + addr, length, 1, alis.fp);
}

static void cfwriteb() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cplot() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdraw() {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 prevx = alis.poldx;
    s16 prevy = alis.poldy;
    
    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;
    
    // io_line(prevx, prevy);
}

static void cbox() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cboxf() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cink() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpset() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpmove() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpmode() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpicture() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cxyscroll() {
    readexec_opername();
    readexec_opername();
}

static void clinking() {
    readexec_opername();
    vram_write16(0xffd6, alis.varD7);
}

static void cmouson() {
    sys_enable_mouse(1);
}

static void cmousoff() {
    sys_enable_mouse(0);
}

// 0x86 - 14d62
static void cmouse() {
    mouse_t mouse = sys_get_mouse();
    
    alis.varD7 = mouse.x;
    readexec_storename();
    
    alis.varD7 = mouse.y;
    readexec_storename();
    
    alis.varD7 = mouse.lb;
    readexec_storename();
}

static void cdefmouse() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csetmouse() {
    readexec_opername();
    u16 x = alis.varD7;
    readexec_opername();
    u16 y = alis.varD6;
    sys_set_mouse(x, y);
}

static void cdefvect() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csetvect() {
    debug(EDebugWarning, " /* MISSING */");
}

static void capproach() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cescape() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cvtstmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cvftstmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cvmov() {
//    OPCODE_CVMOV_0x90
//00014686 10 2e 00 09     move.b     (0x9,A6),D0b
//0001468a 48 80           ext.w      D0w
//0001468c d1 6e 00 00     add.w      D0w,(0x0,A6)
//00014690 10 2e 00 0a     move.b     (0xa,A6),D0b
//00014694 48 80           ext.w      D0w
//00014696 d1 6e 00 02     add.w      D0w,(0x2,A6)
//0001469a 10 2e 00 0b     move.b     (0xb,A6),D0b
//0001469e 48 80           ext.w      D0w
//000146a0 d1 6e 00 04     add.w      D0w,(0x4,A6)
//000146a4 4e 75           rts
    debug(EDebugWarning, " /* MISSING */");
}

static void cdefworld() {
//    OPCODE_CDEFWORLD_0x91
//000173f0 10 1b           move.b     (A3)+,D0b
//000173f2 e1 40           asl.w      #0x8,D0w
//000173f4 10 1b           move.b     (A3)+,D0b
//000173f6 32 3c 00 05     move.w     #0x5,D1w
//    LAB_000173fa                                    XREF[1]:     000173fe(j)
//000173fa 1d 9b 00 00     move.b     (A3)+,(0x0,A6,D0w*0x1)
//000173fe 51 c9 ff fa     dbf        D1w,LAB_000173fa
//00017402 4e 75           rts
    u16 offset = script_read16();
    u8 counter = 5;
    while(counter--) {
        vram_write8(offset, script_read8());
    }
    debug(EDebugWarning, " /* MISSING */");
}

static void cworld() {
//    OPCODE_CWORLD_0x92
//00017404 1d 5b ff de     move.b     (A3)+,(0xFFDE,A6)
//00017408 1d 5b ff df     move.b     (A3)+,(0xFFDF,A6)
//0001740c 4e 75           rts
    vram_write8(0xffde, script_read8());
    vram_write8(0xffdf, script_read8());
}

static void cfindmat() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfindtyp() {
    debug(EDebugWarning, " /* CHECK */");
    
    readexec_opername();
    u16 script_id = alis.varD7;
    if ((s16)script_id < 0)
    {
        script_id = script_id & 0xff;
    }
    
    s16 offset = 0;
    s16 *tabintptr = (s16 *)alis.tablent;
    
    u32 index;
    sScriptLoc *location;
    sAlisScript *script;
    
    do
    {
        location = (sScriptLoc *)(alis.mem + alis.atent + offset);
        index = location->vram_offset;
        script = alis.scripts[index];
        
        if (script_id == script->context->_0x10_script_id && alis.script->vram_org != script->vram_org)
        {
            tabintptr[0x80] = 0;
            *tabintptr = offset;
            if (alis.fallent == 0)
            {
                break;
            }

            tabintptr ++;
        }
        
        offset = location->offset;
    }
    while (offset != 0);
    *tabintptr = -1;
    
    alis.fallent = 0;
    
    crstent();
}

static void cmusic() {
    debug(EDebugWarning, " /* STUBBED */");
    char *in_A0;
    
    alis.flagmain = 0;
    
    readexec_opername();
    s16 idx = alis.varD7;
    s16 addr = adresmus(idx);

    u8 type = *(u8 *)(alis.mem + alis.script->data_org + addr);
//    if (type != 4)
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
    }
//    else
//    {
//        alis.mupnote = addr + 6;
//        readexec_opername();
//        alis.muvolume = 0;
//        alis.maxvolume = alis.maxvolume & 0xff | alis.varD7 << 8;
//        readexec_opername();
//        alis.mutempo = (u8)alis.varD7;
//        readexec_opername();
//        alis.muattac = alis.varD7 + 1;
//        readexec_opername();
//        alis.muduree = (u16)alis.varD7;
//        readexec_opername();
//        alis.muchute = alis.varD7 + 1;
//
//        if (muattac != 0)
//        {
//            alis.dattac = alis.maxvolume / alis.muattac;
//        }
//        if (alis.muchute != 0)
//        {
//            alis.dchute = alis.maxvolume / alis.muchute;
//        }
//
//        alis.muvolume = *(u8 *)(&alis.maxvolume);
//        gomusic();
//        alis.mustate = 1;
//    }
}

static void cdelmusic() {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername();
//    alis.muchute = alis.varD7;
//    offmusic();
}

static void ccadence() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csetvolum() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cxinv() {
    BIT_CHG(alis.script->context->_0x3_xinv, 0);
}

static void cxinvon() {
    alis.script->context->_0x3_xinv = 1;
}

static void cxinvoff() {
    alis.script->context->_0x3_xinv = 0;
}

static void clistent() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csound() {
    debug(EDebugWarning, " /* STUBBED */");
    alis.flagmain = 0;
    readexec_opername();
    u8 index = alis.varD7;
    readexec_opername();
    alis.priorson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    readexec_opername();
    alis.loopsam = alis.varD7;
    readexec_opername();
    alis.speedsam = alis.varD7;
    
    s32 addr = adresmus(index);
//    s8 *sample = (s8 *)(alis.mem + alis.script->data_org + addr);
//    if ((*sample == 1) || (*sample == 2))
//    {
//        if (alis.speedsam == '\0')
//            alis.speedsam = sample[1];
//
//        alis.longsam = *(s32 *)(sample + 2) + -0x10;
//        alis.startsam = alis.script->data_org + addr + 0x10;
//        alis.typeson = 0x80;
//        runson();
//    }
}

static void cmsound() {
    debug(EDebugWarning, " /* MISSING */");
}

static void credon() {
    debug(EDebugWarning, " /* MISSING */");
}

static void credoff() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdelsound() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwtstmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwftstmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void ctstform() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cxput() {
    alis.flagmain = 0;
    *(u8 *)(&alis.flaginvx) = *(u8 *)(alis.mem + alis.script->context->_0x3_xinv) ^ 1;
    readexec_opername();
    alis.depx = 0;
    alis.depz = 0;
    alis.numelem = 0;
    
    u8 idx = alis.varD7;
    put(idx);
}

static void cxputat() {
    alis.flagmain = 0;
    *(u8 *)(&alis.flaginvx) = *(u8 *)(alis.mem + alis.script->context->_0x3_xinv);

    readexec_opername_saveD7();
    alis.depx = alis.varD7;
    readexec_opername_saveD7();
    alis.depy = alis.varD7;
    readexec_opername_saveD7();
    alis.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    u8 idx = alis.varD7;
    alis.numelem = alis.varD6;
    
    put(idx);
}

static void cmput() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmputat() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmxput() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmxputat() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmmusic() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmforme() {
    readexec_opername();
    alis.script->context->_0x1a_cforme = alis.varD7;
}

static void csettime() {
    readexec_opername();
    u16 h = alis.varD7;
    readexec_opername();
    u16 m = alis.varD7;
    readexec_opername();
    u16 s = alis.varD7;
    sys_set_time(h, m, s);
}

static void cgettime() {
    time_t t = sys_get_time();
    alis.varD7 = t << 16 & 0xff;
    cstore_continue();
    alis.varD7 = t << 8 & 0xff;
    cstore_continue();
    alis.varD7 = t << 0 & 0xff;
    cstore_continue();
}

static void cvinput() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csinput() {
    debug(EDebugWarning, " /* MISSING */");
}

static void crunfilm() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cvpicprint() {
    debug(EDebugWarning, " /* MISSING */");
    alis.charmode = 1;
    readexec_opername_saveD7();
    // putval();
}

static void cspicprint() {
    alis.charmode = 1;
    sparam();
    alis_putstring();
}

static void cvputprint() {
    debug(EDebugWarning, " /* STUBBED */");
    alis.charmode = 2;
    readexec_opername_saveD7();
    // putval();
}

static void csputprint() {
    alis.charmode = 2;
    sparam();
    alis_putstring();
}

static void cfont() {
    readexec_opername();
    alis.foasc = alis.varD7;
    readexec_opername();
    alis.fonum = alis.varD7;
    readexec_opername();
    alis.folarg = (u8)alis.varD7;
    readexec_opername();
    alis.fohaut = (u8)alis.varD7;
    readexec_opername();
    alis.fomax = alis.varD7;
}

static void cpaper() {
    debug(EDebugWarning, " /* MISSING */");
}

// fade-out to black
static void ctoblack() {
    readexec_opername_saveD6();
    
    s16 duration = alis.varD6;
    ctoblackpal(duration);
}

static void cmovcolor() {
    debug(EDebugWarning, " /* MISSING */");
}

// fade-in to palette
static void ctopalet() {
    readexec_opername();
    readexec_opername_saveD6();
    
    alis.flagmain = 0;
   
    u16 palidx = alis.varD7;
    u16 duration = alis.varD6;
    
    if (palidx < 0)
    {
        // TODO: ...
        debug(EDebugWarning, " /* STUBBED */");
    }
    else
    {
        s32 addr = adresdes(palidx);
        u8 *paldata = alis.mem + alis.script->data_org + addr;
        ctopalette(paldata, duration);
    }
}

static void cnumput() {
    readexec_opername();
    alis.depx = alis.varD7;
    readexec_opername();
    alis.depy = alis.varD7;
    readexec_opername();
    alis.depz = alis.varD7;
    readexec_opername();
    alis.numelem = alis.varD7;
}

static void cscheart() {
    debug(EDebugWarning, " /* CHECK */");
    
    readexec_opername();
    readexec_opername_saveD6();

    u8 *baseptr = (u8 *)(alis.mem + alis.basemain);
    s16 screen_id = alis.script->context->_0x16_screen_id;
    if (screen_id != 0)
    {
        *(u16 *)(alis.mem + alis.basemain + 0xa + screen_id) = alis.varD7;
        *(u16 *)(alis.mem + alis.basemain + 0xc + screen_id) = alis.varD6;
        baseptr[screen_id] |= 0x80;
    }
}

static void cscpos() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscsize() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cschoriz() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscvertic() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscreduce() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscscale() {
    debug(EDebugWarning, " /* MISSING */");
}

static void creducing() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscdump() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfindcla() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cnearcla() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewcla() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cinstru() {
    debug(EDebugWarning, " /* STUBBED */");
    
    alis.flagmain = 0;
    readexec_opername();
    s16 instid = alis.varD7;
    readexec_opername();
    s16 idx = alis.varD7;
    readexec_opername();
    s16 instidx = alis.varD7;

    s32 addr;

    if (idx < 0)
    {
        instidx = 0;
        addr = 0;
    }
    else
    {
        addr = adresmus(idx);
        u8 type = *(u8 *)(alis.mem + alis.script->data_org + addr);
        if (((type != 1) && (type != 2)) && (type != 5))
        {
            return;
        }
        
        addr += 0x10;
    }
    
    // TODO: implement
//    s16 at = ((s16)instid & 0x1f) << 3;
//    *(s32 *)(tabinst + at) = addr;
//    *(u16 *)(tabinst + at + 4) = instidx;
}

static void cminstru() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cordspr() {
    readexec_opername();
    alis.script->context->_0x2b_cordspr = (u8)alis.varD7;
}

static void calign() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cbackstar() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cstarring() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cengine() {
    readexec_opername();
    readexec_opername();
    readexec_opername();
    readexec_opername();
}

static void cautobase() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cquality() {
    debug(EDebugWarning, " /* MISSING */");
}

static void chsprite() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cselpalet() {
    readexec_opername();
    alis.varD7 &= 0x3; // 4 palettes: 0...3
    thepalet = alis.varD7;
    defpalet = 1;
}

static void clinepalet() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cautomode() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cautofile() {
    debug(EDebugWarning, " /* MISSING */");
}

static void ccancel() {
    debug(EDebugWarning, " /* MISSING */");
}

static void ccancall() {
    alis.ferase = 1;
    
    u16 tmpidx = 0;
    u16 curidx = alis.script->context->_0x18_unknown;
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }
    
    alis.ferase = 0;
}

static void ccancen() {
    debug(EDebugWarning, " /* CHECK */");
    
    alis.ferase = 1;

    readexec_opername_saveD7();
    alis.numelem = alis.varD7;
    
    u16 curidx = 0;
    u16 previdx = 0;

    while (1)
    {
        u8 ret = searchelem(&curidx, &previdx);
        if (ret == 0)
            break;
        
        killelem(&curidx, &previdx);
    }
    
    alis.ferase = 0;
}

static void cblast() {
    debug(EDebugWarning, " /* CHECK */");
    
    alis.ferase = 1;
    readexec_opername_saveD7();
    if (alis.varD7 < 0)
        return;

    killent(alis.varD7, alis.varD5, 0);
}

static void cscback() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscrolpage() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmatent() {
    debug(EDebugWarning, " /* CHECK */");
    cstore_continue();
}

static void cshrink() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdefmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csetmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cputmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csavepal() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csczoom() {
    debug(EDebugWarning, " /* MISSING */");
}

static void ctexmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void calloctab() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfreetab() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscantab() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cneartab() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscsun() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdarkpal() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscdark() {
    debug(EDebugWarning, " /* MISSING */");
}

static void caset() {
    debug(EDebugWarning, " /* MISSING */");
}

static void camov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscaset() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscamov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscfollow() {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername();
    s16 sVar1 = alis.varD7;
    readexec_opername();
    s16 sVar2 = alis.varD7;
    readexec_opername();
    s16 sVar3 = alis.varD7;
    readexec_opername();
    s16 sVar4 = alis.varD7;
    readexec_opername();
    s16 sVar5 = alis.varD7;
    readexec_opername();
    s16 sVar6 = alis.varD7;
    readexec_opername();
    s16 sVar7 = alis.varD7;
    readexec_opername();
    s16 sVar8 = alis.varD7;
    
    s32 iVar9 = alis.script->context->_0x16_screen_id + alis.basemain;
//    *(s16 *)(iVar9 + 0x90) = sVar8;
//    *(s16 *)(iVar9 + 0x8e) = sVar7;
//    *(s16 *)(iVar9 + 0x8c) = sVar6;
//    *(s16 *)(iVar9 + 0x8a) = sVar5;
//    *(s16 *)(iVar9 + 0x88) = sVar4;
//    *(s16 *)(iVar9 + 0x86) = sVar3;
//    *(s8 *) (iVar9 + 0x84) = (s8)sVar2;
//    *(s16 *)(iVar9 + 0x60) = sVar1;
}

static void cscview() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfilm() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwalkmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void catstmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cavtstmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cavmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void caim() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpointpix() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cchartmap() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscsky() {
    debug(EDebugWarning, " /* MISSING */");
}

static void czoom() {
    debug(EDebugWarning, " /* I3 SPECIFIC? */");
    
//    readexec_opername();
//    s16 tmp0 = alis.varD7;
//    alis.script->context->_0x36 = tmp0;
    
//    readexec_opername();
//    s16 tmp1 = alis.varD7;
//    alis.script->context->_0x38 = tmp1;
}


// ============================================================================
#pragma mark - Unimplemented opcodes
// ============================================================================
static void cnul()      {
    debug(EDebugVerbose, " N/I ");
}
static void cesc1()     {
    debug(EDebugVerbose, " N/I ");
}
static void cesc2()     {
    debug(EDebugVerbose, " N/I ");
}
static void cesc3()     {
    debug(EDebugVerbose, " N/I ");
}
static void cclock()    {
    debug(EDebugVerbose, " N/I ");
}
static void cbreakpt()  {
    debug(EDebugVerbose, " N/I ");
}
static void cmul()      {
    debug(EDebugVerbose, " N/I ");
}
static void cdiv()      {
    debug(EDebugVerbose, " N/I ");
}
static void cjsrabs()   {
    debug(EDebugVerbose, " N/I ");
}
static void cjmpabs()   {
    debug(EDebugVerbose, " N/I ");
}
static void cjsrind16() {
    debug(EDebugVerbose, " N/I ");
}
static void cjsrind24() {
    debug(EDebugVerbose, " N/I ");
}
static void cjmpind16() {
    debug(EDebugVerbose, " N/I ");
}
static void cjmpind24() {
    debug(EDebugVerbose, " N/I ");
}


// ============================================================================
#pragma mark - Flow control - Subroutines
// ============================================================================
static void cret() {
    // return from subroutine (cjsr)
    // retrieve return address **OFFSET** from virtual stack
    u32 pc_offset = vram_pop32();
    alis.script->pc = alis.script->pc_org + pc_offset;
}

static void cjsr(u32 offset) {

    // save return **OFFSET**, not ADDRESS
    // TODO: dans la vm originale on empile la 'vraie' adresse du PC en 32 bits
    // Là on est en 64 bits, donc j'empile l'offset
    // TODO: peut-on stocker une adresse de retour *virtuelle* ?
    // Sinon ça oblige à créer une pile virtuelle d'adresses
    //   dont la taille est platform-dependent
    u32 pc_offset = (u32)(alis.script->pc - alis.script->pc_org);
    vram_push32(pc_offset);
    script_jump(offset);
}

static void cjsr8() {
    // read byte, extend sign
    u16 offset = script_read8ext16();
    cjsr(offset);
}

static void cjsr16() {
    u16 offset = script_read16();
    cjsr(offset);
}

static void cjsr24() {
    u32 offset = script_read24();
    cjsr(offset);
}


// ============================================================================
#pragma mark - Flow control - Jump
// ============================================================================
static void cjmp(u32 offset) {
//    alis.pc += offset;
    script_jump(offset);
}

static void cjmp8() {
    cjmp(script_read8ext16());
}

static void cjmp16() {
    cjmp(script_read16());
}

static void cjmp24() {
    cjmp(script_read24());
}


// ============================================================================
#pragma mark - Flow control - Branch if zero
// ============================================================================

static void cbz8() {
    s16 offset = alis.varD7 ? 1 : script_read8ext16();
    script_jump(offset);
}

static void cbz16() {
    s16 offset = alis.varD7 ? 2 : script_read16();
    script_jump(offset);
}

static void cbz24() {
    s16 offset = alis.varD7 ? 3 : script_read24();
    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if non-zero
// ============================================================================
static void cbnz(s32 offset) {
    if(alis.varD7 != 0) {
//        alis.pc += offset;
        script_jump(offset);
    }
}
static void cbnz8() {
    cbnz(script_read8ext16());
}

static void cbnz16() {
    cbnz(script_read16());
}

static void cbnz24() {
    cbnz(script_read24());
}


// ============================================================================
#pragma mark - Flow control - Branch if equal
// ============================================================================
static void cbeq(u32 offset) {
    if(alis.varD7 == alis.varD6) {
//        alis.pc += offset;
        script_jump(offset);
    }
}
static void cbeq8() {
    cbeq(script_read8ext16());
}
static void cbeq16() {
    cbeq(script_read16());
}
static void cbeq24() {
    cbeq(script_read24());
}


// ============================================================================
#pragma mark - Flow control - Branch if not equal
// ============================================================================
static void cbne(u32 offset) {
    if(alis.varD7 != alis.varD6) {
//        alis.pc += offset;
        script_jump(offset);
    }
}
static void cbne8() {
    cbne(script_read8ext16());
}
static void cbne16() {
    cbne(script_read16());
}
static void cbne24() {
    cbne(script_read24());
}


// ============================================================================
#pragma mark - Flow control - Start
// ============================================================================
static void cstart(u32 offset) {
//    OPCODE_CSTART_COMMON                            XREF[2]:     00013182(j), 00013202(j)
//0001318e 4a 39 00        tst.b      (B_CSTOPRET).l
//01 95 a3
//00013194 66 00 00 42     bne.w      LAB_000131d8
//00013198 4a 6e ff f4     tst.w      (-0xc,A6)
//0001319c 66 00 00 20     bne.w      LAB_000131be
//000131a0 32 2e ff f6     move.w     (-0xa,A6),D1w
//000131a4 59 41           subq.w     #0x4,D1w
//000131a6 3d 41 ff f4     move.w     D1w,(-0xc,A6)
//000131aa 3d 41 ff f6     move.w     D1w,(-0xa,A6)
//000131ae 2d ae ff        move.l     (-0x8,A6),(0x0,A6,D1w*0x1)
//f8 10 00
//000131b4 d0 8b           add.l      A3,D0
//000131b6 2d 40 ff f8     move.l     D0,(-0x8,A6)
//000131ba 60 00 00 0e     bra.w      LAB_000131ca
//    LAB_000131be                                    XREF[1]:     0001319c(j)
//000131be 3d 6e ff        move.w     (-0xc,A6),(-0xa,A6)
//f4 ff f6
//000131c4 d0 8b           add.l      A3,D0
//000131c6 2d 40 ff f8     move.l     D0,(-0x8,A6)
//    LAB_000131ca                                    XREF[1]:     000131ba(j)
//000131ca 1d 7c 00        move.b     #0x1,(-0x4,A6)
//01 ff fc
//000131d0 1d 7c 00        move.b     #0x1,(0xFFFF,A6)
//01 ff ff
//000131d6 4e 75           rts

    debug(EDebugWarning, "%s STUBBED", __FUNCTION__);
}
static void cstart8() {
    // read byte, extend sign to word, then to long
    cstart(script_read8ext32());
}

static void cstart16() {
    cstart(script_read16ext32());
}

static void cstart24() {
    cstart(script_read24());
}


// ============================================================================
#pragma mark - Opcode pointer table (256 values)
// ============================================================================
sAlisOpcode opcodes[] = {
    DECL_OPCODE(0x00, cnul,         "TODO: add desc"),
    DECL_OPCODE(0x01, cesc1,        "TODO: add desc"),
    DECL_OPCODE(0x02, cesc2,        "TODO: add desc"),
    DECL_OPCODE(0x03, cesc3,        "TODO: add desc"),
    DECL_OPCODE(0x04, cbreakpt,     "TODO: add desc"),
    DECL_OPCODE(0x05, cjsr8,        "jump to sub-routine with 8-bit offset"),
    DECL_OPCODE(0x06, cjsr16,       "jump to sub-routine with 16-bit offset"),
    DECL_OPCODE(0x07, cjsr24,       "jump to sub-routine with 24-bit offset"),
    DECL_OPCODE(0x08, cjmp8,        "jump (8-bit offset)"),
    DECL_OPCODE(0x09, cjmp16,       "jump (16-bit offset)"),
    DECL_OPCODE(0x0a, cjmp24,       "jump (24-bit offset)"),
    DECL_OPCODE(0x0b, cjsrabs,      "[N/I] jump to sub-routine w/ absolute addr"),
    DECL_OPCODE(0x0c, cjmpabs,      "[N/I] jump to absolute addr"),
    DECL_OPCODE(0x0d, cjsrind16,    "[N/I] jump to sub-routine w/ indirect (16-bit offset)"),
    DECL_OPCODE(0x0e, cjsrind24,    "[N/I] jump to sub-routine w/ indirect (24-bit offset)"),
    DECL_OPCODE(0x0f, cjmpind16,    "[N/I] jump w/ indirect (16-bit offset)"),
    DECL_OPCODE(0x10, cjmpind24,    "[N/I] jump w/ indirect (24-bit offset)"),
    DECL_OPCODE(0x11, cret,         "return from sub-routine"),
    DECL_OPCODE(0x12, cbz8,         "branch if zero with 8-bit offset"),
    DECL_OPCODE(0x13, cbz16,        "branch if zero with 16-bit offset"),
    DECL_OPCODE(0x14, cbz24,        "branch if zero with 24-bit offset"),
    DECL_OPCODE(0x15, cbnz8,        "branch if non-zero with 8-bit offset"),
    DECL_OPCODE(0x16, cbnz16,       "branch if non-zero with 16-bit offset"),
    DECL_OPCODE(0x17, cbnz24,       "branch if non-zero with 24-bit offset"),
    DECL_OPCODE(0x18, cbeq8,        "branch if equal with 8-bit offset"),
    DECL_OPCODE(0x19, cbeq16,       "branch if equal with 16-bit offset"),
    DECL_OPCODE(0x1a, cbeq24,       "branch if equal with 24-bit offset"),
    DECL_OPCODE(0x1b, cbne8,        "branch if non-equal with 8-bit offset"),
    DECL_OPCODE(0x1c, cbne16,       "branch if non-equal with 16-bit offset"),
    DECL_OPCODE(0x1d, cbne24,       "branch if non-equal with 24-bit offset"),
    DECL_OPCODE(0x1e, cstore,       "store expression"),
    DECL_OPCODE(0x1f, ceval,        "start expression evaluation"),
    DECL_OPCODE(0x20, cadd, "TODO: add desc"),
    DECL_OPCODE(0x21, csub, "TODO: add desc"),
    DECL_OPCODE(0x22, cmul,         "[N/I]"),
    DECL_OPCODE(0x23, cdiv,         "[N/I]"),
    DECL_OPCODE(0x24, cvprint, "TODO: add desc"),
    DECL_OPCODE(0x25, csprinti, "TODO: add desc"),
    DECL_OPCODE(0x26, csprinta, "TODO: add desc"),
    DECL_OPCODE(0x27, clocate, "TODO: add desc"),
    DECL_OPCODE(0x28, ctab, "TODO: add desc"),
    DECL_OPCODE(0x29, cdim,         "TODO: add desc"),
    DECL_OPCODE(0x2a, crandom,      "generate a random number"),
    DECL_OPCODE(0x2b, cloop8, "TODO: add desc"),
    DECL_OPCODE(0x2c, cloop16, "TODO: add desc"),
    DECL_OPCODE(0x2d, cloop24, "TODO: add desc"),
    DECL_OPCODE(0x2e, cswitch1, "TODO: add desc"),
    DECL_OPCODE(0x2f, cswitch2, "TODO: add desc"),
    DECL_OPCODE(0x30, cstart8, "TODO: add desc"),
    DECL_OPCODE(0x31, cstart16, "TODO: add desc"),
    DECL_OPCODE(0x32, cstart24, "TODO: add desc"),
    DECL_OPCODE(0x33, cleave, "TODO: add desc"),
    DECL_OPCODE(0x34, cprotect, "TODO: add desc"),
    DECL_OPCODE(0x35, casleep, "TODO: add desc"),
    DECL_OPCODE(0x36, cclock, "TODO: add desc"),
    DECL_OPCODE(0x37, cnul, "TODO: add desc"),
    DECL_OPCODE(0x38, cscmov, "TODO: add desc"),
    DECL_OPCODE(0x39, cscset, "TODO: add desc"),
    DECL_OPCODE(0x3a, cclipping, "TODO: add desc"),
    DECL_OPCODE(0x3b, cswitching, "TODO: add desc"),
    DECL_OPCODE(0x3c, cwlive, "TODO: add desc"),
    DECL_OPCODE(0x3d, cunload, "TODO: add desc"),
    DECL_OPCODE(0x3e, cwakeup, "TODO: add desc"),
    DECL_OPCODE(0x3f, csleep, "TODO: add desc"),
    DECL_OPCODE(0x40, clive, "TODO: add desc"),
    DECL_OPCODE(0x41, ckill, "TODO: add desc"),
    DECL_OPCODE(0x42, cstop, "TODO: add desc"),
    DECL_OPCODE(0x43, cstopret, "TODO: add desc"),
    DECL_OPCODE(0x44, cexit, "TODO: add desc"),
    DECL_OPCODE(0x45, cload,        "Load and depack a script, set into vm"),
    DECL_OPCODE(0x46, cdefsc,       "Define Scene ??"),
    DECL_OPCODE(0x47, cscreen, "TODO: add desc"),
    DECL_OPCODE(0x48, cput, "TODO: add desc"),
    DECL_OPCODE(0x49, cputnat, "TODO: add desc"),
    DECL_OPCODE(0x4a, cerase, "TODO: add desc"),
    DECL_OPCODE(0x4b, cerasen, "TODO: add desc"),
    DECL_OPCODE(0x4c, cset,         "TODO: add desc"),
    DECL_OPCODE(0x4d, cmov, "TODO: add desc"),
    DECL_OPCODE(0x4e, copensc, "TODO: add desc"),
    DECL_OPCODE(0x4f, cclosesc, "TODO: add desc"),
    DECL_OPCODE(0x50, cerasall, "TODO: add desc"),
    DECL_OPCODE(0x51, cforme, "TODO: add desc"),
    DECL_OPCODE(0x52, cdelforme, "TODO: add desc"),
    DECL_OPCODE(0x53, ctstmov, "TODO: add desc"),
    DECL_OPCODE(0x54, ctstset, "TODO: add desc"),
    DECL_OPCODE(0x55, cftstmov, "TODO: add desc"),
    DECL_OPCODE(0x56, cftstset, "TODO: add desc"),
    DECL_OPCODE(0x57, csuccent, "TODO: add desc"),
    DECL_OPCODE(0x58, cpredent, "TODO: add desc"),
    DECL_OPCODE(0x59, cnearent, "TODO: add desc"),
    DECL_OPCODE(0x5a, cneartyp, "TODO: add desc"),
    DECL_OPCODE(0x5b, cnearmat, "TODO: add desc"),
    DECL_OPCODE(0x5c, cviewent, "TODO: add desc"),
    DECL_OPCODE(0x5d, cviewtyp, "TODO: add desc"),
    DECL_OPCODE(0x5e, cviewmat, "TODO: add desc"),
    DECL_OPCODE(0x5f, corient, "TODO: add desc"),
    DECL_OPCODE(0x60, crstent, "TODO: add desc"),
    DECL_OPCODE(0x61, csend, "TODO: add desc"),
    DECL_OPCODE(0x62, cscanon, "TODO: add desc"),
    DECL_OPCODE(0x63, cscanoff, "TODO: add desc"),
    DECL_OPCODE(0x64, cinteron, "TODO: add desc"),
    DECL_OPCODE(0x65, cinteroff, "TODO: add desc"),
    DECL_OPCODE(0x66, cscanclr, "TODO: add desc"),
    DECL_OPCODE(0x67, callentity, "TODO: add desc"),
    DECL_OPCODE(0x68, cpalette, "TODO: add desc"),
    DECL_OPCODE(0x69, cdefcolor, "TODO: add desc"),
    DECL_OPCODE(0x6a, ctiming, "TODO: add desc"),
    DECL_OPCODE(0x6b, czap, "TODO: add desc"),
    DECL_OPCODE(0x6c, cexplode, "TODO: add desc"),
    DECL_OPCODE(0x6d, cding, "TODO: add desc"),
    DECL_OPCODE(0x6e, cnoise, "TODO: add desc"),
    DECL_OPCODE(0x6f, cinitab, "TODO: add desc"),
    DECL_OPCODE(0x70, cfopen, "TODO: add desc"),
    DECL_OPCODE(0x71, cfclose, "TODO: add desc"),
    DECL_OPCODE(0x72, cfcreat, "TODO: add desc"),
    DECL_OPCODE(0x73, cfdel, "TODO: add desc"),
    DECL_OPCODE(0x74, cfreadv, "TODO: add desc"),
    DECL_OPCODE(0x75, cfwritev, "TODO: add desc"),
    DECL_OPCODE(0x76, cfwritei, "TODO: add desc"),
    DECL_OPCODE(0x77, cfreadb, "TODO: add desc"),
    DECL_OPCODE(0x78, cfwriteb, "TODO: add desc"),
    DECL_OPCODE(0x79, cplot, "TODO: add desc"),
    DECL_OPCODE(0x7a, cdraw, "TODO: add desc"),
    DECL_OPCODE(0x7b, cbox, "TODO: add desc"),
    DECL_OPCODE(0x7c, cboxf, "TODO: add desc"),
    DECL_OPCODE(0x7d, cink, "TODO: add desc"),
    DECL_OPCODE(0x7e, cpset, "TODO: add desc"),
    DECL_OPCODE(0x7f, cpmove, "TODO: add desc"),
    DECL_OPCODE(0x80, cpmode, "TODO: add desc"),
    DECL_OPCODE(0x81, cpicture, "TODO: add desc"),
    DECL_OPCODE(0x82, cxyscroll, "TODO: add desc"),
    DECL_OPCODE(0x83, clinking, "TODO: add desc"),
    DECL_OPCODE(0x84, cmouson,      "display mouse cursor"),
    DECL_OPCODE(0x85, cmousoff,     "hide mouse cursor"),
    DECL_OPCODE(0x86, cmouse,       "get mouse status (x, y, buttons) and store"),
    DECL_OPCODE(0x87, cdefmouse,    "TODO: define mouse sprite ???"),
    DECL_OPCODE(0x88, csetmouse,    "set mouse position"),
    DECL_OPCODE(0x89, cdefvect, "TODO: add desc"),
    DECL_OPCODE(0x8a, csetvect, "TODO: add desc"),
    DECL_OPCODE(0x8b, cnul,         "[N/I]"),
    DECL_OPCODE(0x8c, capproach, "TODO: add desc"),
    DECL_OPCODE(0x8d, cescape, "TODO: add desc"),
    DECL_OPCODE(0x8e, cvtstmov, "TODO: add desc"),
    DECL_OPCODE(0x8f, cvftstmov, "TODO: add desc"),
    DECL_OPCODE(0x90, cvmov, "TODO: add desc"),
    DECL_OPCODE(0x91, cdefworld, "TODO: add desc"),
    DECL_OPCODE(0x92, cworld, "TODO: add desc"),
    DECL_OPCODE(0x93, cfindmat, "TODO: add desc"),
    DECL_OPCODE(0x94, cfindtyp, "TODO: add desc"),
    DECL_OPCODE(0x95, cmusic, "TODO: add desc"),
    DECL_OPCODE(0x96, cdelmusic, "TODO: add desc"),
    DECL_OPCODE(0x97, ccadence, "TODO: add desc"),
    DECL_OPCODE(0x98, csetvolum, "TODO: add desc"),
    DECL_OPCODE(0x99, cxinv, "TODO: add desc"),
    DECL_OPCODE(0x9a, cxinvon, "TODO: add desc"),
    DECL_OPCODE(0x9b, cxinvoff, "TODO: add desc"),
    DECL_OPCODE(0x9c, clistent, "TODO: add desc"),
    DECL_OPCODE(0x9d, csound, "TODO: add desc"),
    DECL_OPCODE(0x9e, cmsound, "TODO: add desc"),
    DECL_OPCODE(0x9f, credon, "TODO: add desc"),
    DECL_OPCODE(0xa0, credoff, "TODO: add desc"),
    DECL_OPCODE(0xa1, cdelsound, "TODO: add desc"),
    DECL_OPCODE(0xa2, cwmov, "TODO: add desc"),
    DECL_OPCODE(0xa3, cwtstmov, "TODO: add desc"),
    DECL_OPCODE(0xa4, cwftstmov, "TODO: add desc"),
    DECL_OPCODE(0xa5, ctstform, "TODO: add desc"),
    DECL_OPCODE(0xa6, cxput, "TODO: add desc"),
    DECL_OPCODE(0xa7, cxputat, "TODO: add desc"),
    DECL_OPCODE(0xa8, cmput, "TODO: add desc"),
    DECL_OPCODE(0xa9, cmputat, "TODO: add desc"),
    DECL_OPCODE(0xaa, cmxput, "TODO: add desc"),
    DECL_OPCODE(0xab, cmxputat, "TODO: add desc"),
    DECL_OPCODE(0xac, cmmusic, "TODO: add desc"),
    DECL_OPCODE(0xad, cmforme, "TODO: add desc"),
    DECL_OPCODE(0xae, csettime,     "set current time"),
    DECL_OPCODE(0xaf, cgettime,     "get current time"),
    DECL_OPCODE(0xb0, cvinput, "TODO: add desc"),
    DECL_OPCODE(0xb1, csinput, "TODO: add desc"),
    DECL_OPCODE(0xb2, cnul,         "[N/I]"),
    DECL_OPCODE(0xb3, cnul,         "[N/I]"),
    DECL_OPCODE(0xb4, cnul,         "[N/I]"),
    DECL_OPCODE(0xb5, crunfilm, "TODO: add desc"),
    DECL_OPCODE(0xb6, cvpicprint, "TODO: add desc"),
    DECL_OPCODE(0xb7, cspicprint, "TODO: add desc"),
    DECL_OPCODE(0xb8, cvputprint, "TODO: add desc"),
    DECL_OPCODE(0xb9, csputprint, "TODO: add desc"),
    DECL_OPCODE(0xba, cfont, "TODO: add desc"),
    DECL_OPCODE(0xbb, cpaper, "TODO: add desc"),
    DECL_OPCODE(0xbc, ctoblack,     "fade-out screen to black"),
    DECL_OPCODE(0xbd, cmovcolor, "TODO: add desc"),
    DECL_OPCODE(0xbe, ctopalet,     "fade-in screen to palette"),
    DECL_OPCODE(0xbf, cnumput, "TODO: add desc"),
    DECL_OPCODE(0xc0, cscheart, "TODO: add desc"),
    DECL_OPCODE(0xc1, cscpos, "TODO: add desc"),
    DECL_OPCODE(0xc2, cscsize, "TODO: add desc"),
    DECL_OPCODE(0xc3, cschoriz, "TODO: add desc"),
    DECL_OPCODE(0xc4, cscvertic, "TODO: add desc"),
    DECL_OPCODE(0xc5, cscreduce, "TODO: add desc"),
    DECL_OPCODE(0xc6, cscscale, "TODO: add desc"),
    DECL_OPCODE(0xc7, creducing, "TODO: add desc"),
    DECL_OPCODE(0xc8, cscmap, "TODO: add desc"),
    DECL_OPCODE(0xc9, cscdump, "TODO: add desc"),
    DECL_OPCODE(0xca, cfindcla, "TODO: add desc"),
    DECL_OPCODE(0xcb, cnearcla, "TODO: add desc"),
    DECL_OPCODE(0xcc, cviewcla, "TODO: add desc"),
    DECL_OPCODE(0xcd, cinstru, "TODO: add desc"),
    DECL_OPCODE(0xce, cminstru, "TODO: add desc"),
    DECL_OPCODE(0xcf, cordspr, "TODO: add desc"),
    DECL_OPCODE(0xd0, calign, "TODO: add desc"),
    DECL_OPCODE(0xd1, cbackstar, "TODO: add desc"),
    DECL_OPCODE(0xd2, cstarring, "TODO: add desc"),
    DECL_OPCODE(0xd3, cengine, "TODO: add desc"),
    DECL_OPCODE(0xd4, cautobase, "TODO: add desc"),
    DECL_OPCODE(0xd5, cquality, "TODO: add desc"),
    DECL_OPCODE(0xd6, chsprite, "TODO: add desc"),
    DECL_OPCODE(0xd7, cselpalet, "TODO: add desc"),
    DECL_OPCODE(0xd8, clinepalet, "TODO: add desc"),
    DECL_OPCODE(0xd9, cautomode, "TODO: add desc"),
    DECL_OPCODE(0xda, cautofile, "TODO: add desc"),
    DECL_OPCODE(0xdb, ccancel, "TODO: add desc"),
    DECL_OPCODE(0xdc, ccancall, "TODO: add desc"),
    DECL_OPCODE(0xdd, ccancen, "TODO: add desc"),
    DECL_OPCODE(0xde, cblast, "TODO: add desc"),
    DECL_OPCODE(0xdf, cscback, "TODO: add desc"),
    DECL_OPCODE(0xe0, cscrolpage, "TODO: add desc"),
    DECL_OPCODE(0xe1, cmatent, "TODO: add desc"),
    DECL_OPCODE(0xe2, cshrink, "TODO: add desc"),
    DECL_OPCODE(0xe3, cdefmap, "TODO: add desc"),
    DECL_OPCODE(0xe4, csetmap, "TODO: add desc"),
    DECL_OPCODE(0xe5, cputmap, "TODO: add desc"),
    DECL_OPCODE(0xe6, csavepal, "TODO: add desc"),
    DECL_OPCODE(0xe7, csczoom, "TODO: add desc"),
    DECL_OPCODE(0xe8, ctexmap, "TODO: add desc"),
    DECL_OPCODE(0xe9, calloctab, "TODO: add desc"),
    DECL_OPCODE(0xea, cfreetab, "TODO: add desc"),
    DECL_OPCODE(0xeb, cscantab, "TODO: add desc"),
    DECL_OPCODE(0xec, cneartab, "TODO: add desc"),
    DECL_OPCODE(0xed, cscsun, "TODO: add desc"),
    DECL_OPCODE(0xee, cdarkpal, "TODO: add desc"),
    DECL_OPCODE(0xef, cscdark, "TODO: add desc"),
    DECL_OPCODE(0xf0, caset, "TODO: add desc"),
    DECL_OPCODE(0xf1, camov, "TODO: add desc"),
    DECL_OPCODE(0xf2, cscaset, "TODO: add desc"),
    DECL_OPCODE(0xf3, cscamov, "TODO: add desc"),
    DECL_OPCODE(0xf4, cscfollow, "TODO: add desc"),
    DECL_OPCODE(0xf5, cscview, "TODO: add desc"),
    DECL_OPCODE(0xf6, cfilm, "TODO: add desc"),
    DECL_OPCODE(0xf7, cwalkmap, "TODO: add desc"),
    DECL_OPCODE(0xf8, catstmap, "TODO: add desc"),
    DECL_OPCODE(0xf9, cavtstmov, "TODO: add desc"),
    DECL_OPCODE(0xfa, cavmov, "TODO: add desc"),
    DECL_OPCODE(0xfb, caim, "TODO: add desc"),
    DECL_OPCODE(0xfc, cpointpix, "TODO: add desc"),
    DECL_OPCODE(0xfd, cchartmap, "TODO: add desc"),
    DECL_OPCODE(0xfe, cscsky, "TODO: add desc"),
    DECL_OPCODE(0xff, czoom, "TODO: add desc")
};

void killent(u16 killent, u16 curent, u16 testent)
{
    // script
    
    sScriptLoc *killloc = (sScriptLoc *)(alis.mem + alis.atent + killent);
    sAlisScript *killscript = alis.scripts[killloc->vram_offset];
    sAlisScript *prevscript = alis.script;
    
    // NOTE: to accomodate cerasall()
    alis.script = killscript;
    
    u32 script_vram = killscript->vram_org;
    if (script_vram == 0)
    {
        alis.ferase = 0;
        return;
    }
    
    cerasall();

    u32 datasize = sizeof(killscript->context) + killscript->header.w_unknown5 + killscript->header.w_unknown7;
    s32 vramsize = killscript->header.vram_alloc_sz;
    s32 shrinkby = datasize + vramsize;

    u32 target = script_vram - datasize;
    u32 source = target + shrinkby;

    // copy work mem to freed space
    
    do
    {
        *(u16 *)(alis.mem + target) = *(u16 *)(alis.mem + source);
        target += 2;
        source += 2;
    }
    while (source < alis.finent);

    alis.finent -= shrinkby;

    // change work addresses of next scripts to match new locations
    
    s32 prevent = 0;
    s32 nextent;

    while (1)
    {
        nextent = *(u16 *)(alis.mem + alis.atent + 4 + prevent);
        if (killent == nextent)
        {
            curent = prevent;
        }

        if (nextent == 0)
        {
            break;
        }

        prevent = nextent;
        
        sAlisScript *curscript = alis.scripts[*(u32 *)(alis.mem + alis.atent + nextent)];
        if (killscript->vram_org <= curscript->vram_org)
        {
            curscript->vram_org -= shrinkby;
        }
    }

    sScriptLoc *curloc = (sScriptLoc *)(alis.mem + alis.atent + curent);
    curloc->offset = killloc->offset;
    killloc->offset = alis.dernent;
    killloc->vram_offset = 0;

    alis.dernent = killent;
    alis.nbent --;
    
    alis.script = prevscript;
    
    if (testent == killent)
    {
        moteur1(curent);
    }
}

void runson(void)
{
//    s8 *a3;
//
//    s8 cVar1 = alis.priorson;
//    if (alis.priorson < 0)
//    {
//        a3 = &bcanal0;
//        if (((alis.priorson == DAT_000205c1) || (a3 = &bcanal1, alis.priorson == DAT_000205d1)) || (a3 = &bcanal2, alis.priorson == DAT_000205e1))
//            goto runson10;
//
//joined_r0x0001b270:
//
//        a3 = &bcanal3;
//        if (alis.priorson == DAT_000205f1)
//            goto runson10;
//    }
//    else
//    {
//        a3 = &bcanal0;
//        if (((((s8)alis.pereson == DAT_000205ce) && (alis.priorson == DAT_000205c1)) || ((a3 = &bcanal1, alis.pereson == DAT_000205de && (alis.priorson == DAT_000205d1)))) || ((a3 = &bcanal2, alis.pereson == DAT_000205ee && (alis.priorson == DAT_000205e1))))
//            goto runson10;
//        if (pereson == DAT_000205fe)
//            goto joined_r0x0001b270;
//    }
//
//    a3 = &bcanal0;
//    s8 cVar2 = DAT_000205c1;
//    if (DAT_000205d1 < DAT_000205c1)
//    {
//        a3 = &bcanal1;
//        cVar2 = DAT_000205d1;
//    }
//
//    if (DAT_000205e1 < cVar2)
//    {
//        a3 = &bcanal2;
//        cVar2 = DAT_000205e1;
//    }
//
//    if (DAT_000205f1 < cVar2)
//    {
//        a3 = &bcanal3;
//        cVar2 = DAT_000205f1;
//    }
//
//    if (((-1 < alis.priorson) && (cVar2 < 0)) && (cVar2 != -0x80))
//    {
//        return;
//    }
//
//    if (alis.priorson < cVar2)
//    {
//        return;
//    }
//
//runson10:
//
//    if (alis.typeson < 0)
//    {
//        gosound(a3);
//    }
//    else
//    {
//        a3[0] = -0x80;
//        a3[1] = cVar1;
//        a3[2] = alis.typeson;
//        *(u16 *)(a3 + 0xa) = alis.freqson;
//        *(u16 *)(a3 + 0xc) = alis.dfreqson;
//        a3[6] = alis.volson;
//        a3[7] = 0;
//        *(u16 *)(a3 + 0x8) = alis.dvolson;
//        *(u16 *)(a3 + 0x4) = alis.longson;
//        *(s16 *)(a3 + 0xe) = alis.pereson;
//        a3[0] = '\x02';
//    }
}
