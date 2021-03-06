//
//  opcodes.c
//  alis
//
//

#include "alis.h"
#include "alis_private.h"
#include "utils.h"


#define BIT_SCAN        (0)
#define BIT_INTER       (1)


// ============================================================================
#pragma mark - Opcodes
// ============================================================================
static void cstore_continue() {
    // swap chunk 1 / 3
    u8 * tmp = alis.bssChunk1;
    alis.bssChunk1 = alis.bssChunk3;
    alis.bssChunk3 = tmp;
    
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
    debug(EDebugWarning, " /* MISSING */");
}

static void csprinti() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csprinta() {
    debug(EDebugWarning, " /* MISSING */");
}

static void clocate() {
    debug(EDebugWarning, " /* MISSING */");
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


// cloopX(jmp_offset, addname(ram_offset))
// decrement value in RAM[offset2]
// if obtained value is zero, then jump with jmp_offset
static void cloop(u32 offset) {
    alis.varD7 = -1;
    readexec_addname_swap();
    if(alis.sr.zero) {
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
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void casleep() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscmov() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscset() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void cwakeup() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csleep() {
    debug(EDebugWarning, " /* MISSING */");
}

static void clive() {
    debug(EDebugWarning, " /* STUBBED */");
    alis._DAT_000195fa = 0;
    alis._DAT_000195fc = 0;
    alis._DAT_000195fe = 0;
    
    u16 d2 = script_read16();
    
    cstore_continue();
}

static void ckill() {
    debug(EDebugWarning, " /* MISSING */");
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
    /*
     **************************************************************
     *                          FUNCTION                          *
     **************************************************************
     undefined OPCODE_CEXIT_0x44()
undefined         D0b:1          <RETURN>
     OPCODE_CEXIT_0x44
00013988 4a 45           tst.w      D5w
0001398a 66 00 00 06     bne.w      __d5_not_zero
0001398e 60 00 f6 1c     bra.w      FUN_SHOW_FATAL_ERROR::LAB_TERMINATE
     __d5_not_zero                                   XREF[1]:     0001398a(j)
00013992 30 05           move.w     D5w,D0w
00013994 60 00 55 82     bra.w      FUN_00018f18                                     undefined FUN_00018f18()
     -- Flow Override: CALL_RETURN (CALL_TERMINATOR)

     */
    alis.running = 0;
    debug(EDebugWarning, " /* STUBBED */");
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
        
        sAlisScript * script = script_load((char *)path);        
        alis_register_script(script);
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
    debug(EDebugWarning, " /* STUBBED */");
    /*
     ; code address: $139ca
     opcode_cdefsc:
         move.b         (A3)+,D0
         asl.w          #$8,D0
         move.b         (A3)+,D0
    */
    u16 offset = script_read16();
    /*
         movea.l        (ADDR_VSTACK).l,A0 ; correspond ?? a6 !!! / A0 vaut $224f0, contient $22690 soit vstack
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
}

static void cscreen() {
    u16 pos = script_read16();
    if (pos != alis.script->context._0x16_screen_position) {
        alis.script->context._0x16_screen_position = pos;
    }
}

static void cput() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cputnat() {
    
    // CALLED AT EACH FRAME WHEN SCREEN UPDATE NEEDED ?
    
    debug(EDebugWarning, " /* MISSING */");
//    **************************************************************
//    *                          FUNCTION                          *
//    **************************************************************
//    undefined OPCODE_CPUTNAT_0x49()
//undefined         D0b:1          <RETURN>
//    OPCODE_CPUTNAT_0x49
//00013e82 42 39 00        clr.b      (DAT_0001959f).l
//01 95 9f
//00013e88 13 ee ff        move.b     (-0x3,A6),(DAT_0001959c).l
//fd 00 01
//95 9c
//    LAB_00013e90                                    XREF[3]:     00013ef0(j), 00013f18(j),
//                                                                 00013f32(j)
//00013e90 61 00 36 da     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00013e94 33 c7 00        move.w     D7w,(DAT_00019588).l
//01 95 88
//00013e9a 61 00 36 d0     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00013e9e 33 c7 00        move.w     D7w,(DAT_0001958a).l
//01 95 8a
//00013ea4 61 00 36 c6     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00013ea8 33 c7 00        move.w     D7w,(DAT_0001958c).l
//01 95 8c
//00013eae 61 00 36 bc     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//00013eb2 61 00 36 ac     bsr.w      FUN_READEXEC_OPERNAME_PUSH_D7                    undefined FUN_READEXEC_OPERNAME_
//00013eb6 13 c6 00        move.b     D6b,(DAT_00019594).l
//01 95 94
//00013ebc 4e f9 00        jmp        FUN_00013f50.l                                   undefined FUN_00013f50()
//01 3f 50
//    -- Flow Override: CALL_RETURN (CALL_TERMINATOR)
    readexec_opername_saveD7();
    readexec_opername_saveD7();
    readexec_opername_saveD7();
    readexec_opername_saveD7();
    readexec_opername_saveD7();
}

static void cerase() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cerasen() {
    debug(EDebugWarning, " /* MISSING */");
    readexec_opername_saveD7();
//    **************************************************************
//    *                          FUNCTION                          *
//    **************************************************************
//    undefined OPCODE_CERASEN_0x4b()
//undefined         D0b:1          <RETURN>
//    OPCODE_CERASEN_0x4b
//0001452a 61 00 30 40     bsr.w      FUN_READEXEC_OPERNAME_SAVE_D7                    undefined FUN_READEXEC_OPERNAME_
//0001452e 13 c7 00        move.b     D7b,(DAT_00019594).l
//01 95 94
//00014534 22 79 00        movea.l    (DAT_1955e_usedby_cdefsc).l,A1
//01 95 5e
//    LAB_0001453a                                    XREF[1]:     0001454e(j)
//0001453a 61 00 fc 02     bsr.w      FUN_0001413e                                     undefined FUN_0001413e()
//0001453e 67 00 00 0a     beq.w      LAB_0001454a
//00014542 42 39 00        clr.b      (DAT_0001951d).l
//01 95 1d
//00014548 4e 75           rts
//    LAB_0001454a                                    XREF[1]:     0001453e(j)
//0001454a 61 00 00 64     bsr.w      FUN_000145b0                                     undefined FUN_000145b0()
//0001454e 60 00 ff ea     bra.w      LAB_0001453a

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
}

static void cclosesc() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cerasall() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cforme() {
    readexec_opername();
    alis.script->context._0x1a_cforme = alis.varD7;
}

static void cdelforme() {
    alis.script->context._0x1a_cforme = -1;
}

static void ctstmov() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void cpredent() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void csend() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscanclr() {
//    u16 w = vram_read16(VRAM_OFFSET_OSCAN_OSCANCLR_2);
//    vram_write16(VRAM_OFFSET_OSCAN_OSCANCLR_1, w);
//    vram_clrbit(VRAM_OFFSET_CSCAN_CINTER, 7);

    alis.script->context._0x1e_scan_clr = alis.script->context._0x1c_scan_clr;
    alis.script->context._0x24_scan_inter.scan_clr_bit_7 = 0;
}

static void cscanon() {
    alis.script->context._0x24_scan_inter.scan_off_bit_0 = 0;
//    vram_clrbit(VRAM_OFFSET_CSCAN_CINTER, 0);
}

static void cscanoff() {
    alis.script->context._0x24_scan_inter.scan_off_bit_0 = 1;
//    vram_setbit(VRAM_OFFSET_CSCAN_CINTER, 0);
    cscanclr();
}

static void cinteron() {
    alis.script->context._0x24_scan_inter.inter_off_bit_1 = 0;
//    vram_clrbit(VRAM_OFFSET_CSCAN_CINTER, 1);
}

static void cinteroff() {
    alis.script->context._0x24_scan_inter.inter_off_bit_1 = 1;
//    vram_setbit(VRAM_OFFSET_CSCAN_CINTER, 1);
}

static void callentity() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpalette() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void cfwritev() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfwritei() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfreadb() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfwriteb() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cplot() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdraw() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void cmusic() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdelmusic() {
    debug(EDebugWarning, " /* MISSING */");
}

static void ccadence() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csetvolum() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cxinv() {
    BIT_CHG(alis.script->context._0x3_xinv, 0);
}

static void cxinvon() {
    alis.script->context._0x3_xinv = 1;
}

static void cxinvoff() {
    alis.script->context._0x3_xinv = 0;
}

static void clistent() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csound() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void cxputat() {
    debug(EDebugWarning, " /* MISSING */");
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
    alis.script->context._0x1a_cforme = alis.varD7;
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
}

static void cspicprint() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cvputprint() {
    debug(EDebugWarning, " /* MISSING */");
}

static void csputprint() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfont() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpaper() {
    debug(EDebugWarning, " /* MISSING */");
}

// fade-out to black
static void ctoblack() {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername_saveD7();
}

static void cmovcolor() {
    debug(EDebugWarning, " /* MISSING */");
}

// fade-in to palette
static void ctopalet() {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername();
    
    u16 save = alis.varD7;
    readexec_opername_saveD7();
    alis.varD6 = alis.varD7;
    alis.varD7 = save;
}

static void cnumput() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscheart() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}

static void cminstru() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cordspr() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
//    **************************************************************
//    *                          FUNCTION                          *
//    **************************************************************
//    undefined OPCODE_CSELPALET_0xd7()
//undefined         D0b:1          <RETURN>
//    OPCODE_CSELPALET_0xd7
//00016288 61 00 12 e4     bsr.w      FUN_READEXEC_OPERNAME                            undefined FUN_READEXEC_OPERNAME()
//0001628c 02 47 00 03     andi.w     #0x3,D7w
//00016290 13 c7 00        move.b     D7b,(DAT_0001968c).l
//01 96 8c
//00016296 13 fc 00        move.b     #0x1,(DAT_0001968d).l
//01 00 01
//96 8d
//0001629e 4e 75           rts
    readexec_opername();
    alis.varD7 &= 0x3; // 4 palettes: 0...3
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
    debug(EDebugWarning, " /* MISSING */");
}

static void ccancen() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cblast() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscback() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscrolpage() {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmatent() {
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
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
    debug(EDebugWarning, " /* MISSING */");
}


// ============================================================================
#pragma mark - Unimplemented opcodes
// ============================================================================
static void cnul()      {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cesc1()     {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cesc2()     {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cesc3()     {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cclock()    {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cbreakpt()  {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cmul()      {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cdiv()      {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cjsrabs()   {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cjmpabs()   {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cjsrind16() {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cjsrind24() {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cjmpind16() {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
}
static void cjmpind24() {
    debug(EDebugVerbose, "%s: N/I", __FUNCTION__);
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
    // L?? on est en 64 bits, donc j'empile l'offset
    // TODO: peut-on stocker une adresse de retour *virtuelle* ?
    // Sinon ??a oblige ?? cr??er une pile virtuelle d'adresses
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
static void cbz(u32 offset) {
    if(alis.varD7 == 0) {
//        alis.pc += offset;
        script_jump(offset);
    }
}

static void cbz8() {
    cbz(script_read8ext16());
}

static void cbz16() {
    cbz(script_read16());
}
static void cbz24() {
    cbz(script_read24());
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
