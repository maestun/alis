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

void shrinkprog(s32 start, s32 length, u16 script_id);
void killent(u16 d0w, u16 d5w);
void runson(void);

// ============================================================================
#pragma mark - Opcodes
// ============================================================================
static void cstore_continue(void);
static void cstore(void);
static void ceval(void);
static void cadd(void);
static void csub(void);
static void cvprint(void);
static void csprinti(void);
static void csprinta(void);
static void clocate(void);
static void ctab(void);
static void cdim(void);
static void crandom(void);
static void cloop(s32 offset);
static void cloop8(void);
static void cloop16(void);
static void cloop24(void);
static void cswitch1(void);
static void cswitch2(void);
static void cleave(void);
static void cprotect(void);
static void casleep(void);
static void cscmov(void);
static void cscset(void);
static void cclipping(void);
static void cswitching(void);
static void cwlive(void);
static void cunload(void);
static void cwakeup(void);
static void cstop(void);
static void csleep(void);
static void clive(void);
static void ckill(void);
static void cstop(void);
static void cstopret(void);
static void cexit(void);
static void cload(void);
static void cdefsc(void);
static void cscreen(void);
static void cput(void);
static void cputnat(void);
static void cerase(void);
static void cerasen(void);
static void cset(void);
static void cmov(void);
static void copensc(void);
static void cclosesc(void);
static void cerasall(void);
static void cforme(void);
static void cdelforme(void);
void clipform(void);
static void ctstmov(void);
static void ctstset(void);
static void cftstmov(void);
static void cftstset(void);
static void csuccent(void);
static void cpredent(void);
static void cnearent(void);
static void cneartyp(void);
static void cnearmat(void);
static void cviewent(void);
static void cviewtyp(void);
static void cviewmat(void);
static void corient(void);
static void crstent(void);
static void csend(void);
static void cscanclr(void);
static void cscanon(void);
static void cscanoff(void);
static void cinteron(void);
static void cinteroff(void);
static void callentity(void);
static void cpalette(void);
static void cdefcolor(void);
static void ctiming(void);
static void czap(void);
static void cexplode(void);
static void cding(void);
static void cnoise(void);
static void cinitab(void);
static void cfopen(void);
static void cfclose(void);
static void cfcreat(void);
static void cfdel(void);
static void cfreadv(void);
static void cfwritev(void);
static void cfwritei(void);
static void cfreadb(void);
static void cfwriteb(void);
static void cplot(void);
static void cdraw(void);
static void cbox(void);
static void cboxf(void);
static void cink(void);
static void cpset(void);
static void cpmove(void);
static void cpmode(void);
static void cpicture(void);
static void cxyscroll(void);
static void clinking(void);
static void cmouson(void);
static void cmousoff(void);
static void cmouse(void);
static void cdefmouse(void);
static void csetmouse(void);
static void cdefvect(void);
static void csetvect(void);
static void capproach(void);
static void cescape(void);
static void cvtstmov(void);
static void cvftstmov(void);
static void cvmov(void);
static void cdefworld(void);
static void cworld(void);
static void cfindmat(void);
static void cfindtyp(void);
static void cmusic(void);
static void cdelmusic(void);
static void ccadence(void);
static void csetvolum(void);
static void cxinv(void);
static void cxinvon(void);
static void cxinvoff(void);
static void clistent(void);
static void csound(void);
static void cmsound(void);
static void credon(void);
static void credoff(void);
static void cdelsound(void);
static void cwmov(void);
static void cwtstmov(void);
static void cwftstmov(void);
static void ctstform(void);
static void cxput(void);
static void cxputat(void);
static void cmput(void);
static void cmputat(void);
static void cmxput(void);
static void cmxputat(void);
static void cmmusic(void);
static void cmforme(void);
static void csettime(void);
static void cgettime(void);
static void cvinput(void);
static void csinput(void);
static void crunfilm(void);
static void cvpicprint(void);
static void cspicprint(void);
static void cvputprint(void);
static void csputprint(void);
static void cfont(void);
static void cpaper(void);
static void ctoblack(void);
static void cmovcolor(void);
static void ctopalet(void);
static void cnumput(void);
static void cscheart(void);
static void cscpos(void);
static void cscsize(void);
static void cschoriz(void);
static void cscvertic(void);
static void cscreduce(void);
static void cscscale(void);
static void creducing(void);
static void cscmap(void);
static void cscdump(void);
static void cfindcla(void);
static void cnearcla(void);
static void cviewcla(void);
static void cinstru(void);
static void cminstru(void);
static void cordspr(void);
static void calign(void);
static void cbackstar(void);
static void cstarring(void);
static void cengine(void);
static void cautobase(void);
static void cquality(void);
static void chsprite(void);
static void cselpalet(void);
static void clinepalet(void);
static void cautomode(void);
static void cautofile(void);
static void ccancel(void);
static void ccancall(void);
static void ccancen(void);
static void cblast(void);
static void cscback(void);
static void cscrolpage(void);
static void cmatent(void);
static void cshrink(void);
static void cdefmap(void);
static void csetmap(void);
static void cputmap(void);
static void csavepal(void);
static void csczoom(void);
static void ctexmap(void);
static void calloctab(void);
static void cfreetab(void);
static void cscantab(void);
static void cneartab(void);
static void cscsun(void);
static void cdarkpal(void);
static void cscdark(void);
static void caset(void);
static void camov(void);
static void cscaset(void);
static void cscamov(void);
static void cscfollow(void);
static void cscview(void);
static void cfilm(void);
static void cwalkmap(void);
static void catstmap(void);
static void cavtstmov(void);
static void cavmov(void);
static void caim(void);
static void cpointpix(void);
static void cchartmap(void);
static void cscsky(void);
static void czoom(void);

static void cnul(void);
static void cesc1(void);
static void cesc2(void);
static void cesc3(void);
static void cclock(void);
static void cbreakpt(void);
static void cmul(void);
static void cdiv(void);
static void cjsrabs(void);
static void cjmpabs(void);
static void cjsrind16(void);
static void cjsrind24(void);
static void cjmpind16(void);
static void cjmpind24(void);

static void cret(void);
static void cjsr(s32 offset);
static void cjsr8(void);
static void cjsr16(void);
static void cjsr24(void);

static void cjmp8(void);
static void cjmp16(void);
static void cjmp24(void);

static void cbz8(void);
static void cbz16(void);
static void cbz24(void);

static void cbnz8(void);
static void cbnz16(void);
static void cbnz24(void);

static void cbeq8(void);
static void cbeq16(void);
static void cbeq24(void);

static void cbne8(void);
static void cbne16(void);
static void cbne24(void);

static void cstart(s32 offset);
static void cstart8(void);
static void cstart16(void);
static void cstart24(void);


static void cstore_continue(void) {
    u8 *tmp = alis.sd7;
    alis.sd7 = alis.oldsd7;
    alis.oldsd7 = tmp;
    
//    s32 t0 = (s32)(alis.sd7 - alis.mem);
//    s32 t1 = (s32)(alis.oldsd7 - alis.mem);
//
//    printf("cstore_continue(void): Write to address $0195e4, new value is %d ($%x)\n", t0 & 0xffff, t0 & 0xffff);
//    printf("cstore_continue(void): Write to address $0195ec, new value is %d ($%x)\n", t1 & 0xffff, t1 & 0xffff);

    readexec_storename();
}

static void cstore(void) {
    readexec_opername_saveD7();
    cstore_continue();
}

static void ceval(void) {
    readexec_opername_saveD7();
}

static void cadd(void) {
    readexec_opername_saveD7();
    readexec_addname_swap();
}

static void csub(void) {
    readexec_opername_saveD7();
    alis.varD7 *= -1;
    readexec_addname_swap();
}

static void cvprint(void) {
    debug(EDebugWarning, " /* STUBBED */");
    alis.charmode = 0;
    readexec_opername_saveD7();

    // TODO: ...
}

static void csprinti(void) {
    alis.charmode = 0;
    for (u8 c = script_read8(); c != 0; c = script_read8())
    {
        alis_putchar(c);
    }
}

static void csprinta(void) {
    alis.charmode = 0;
    readexec_opername_saveD7();
    alis_putstring();
}

static void clocate(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    s16 xlocate = alis.varD7;
    s16 ylocate = alis.varD6;
//    io_locate();
}

static void ctab(void) {
    debug(EDebugWarning, " /* MISSING */");
}

// copy values to vram
// cdim(u16 offset, u8 counter, u8 value, optional u16[] values)
static void cdim(void) {

    // read word param
    u16 offset = script_read16();
    u8 counter = script_read8();
    u8 byte2 = script_read8();
    
    vwrite8(alis.script->vram_org + --offset, counter);
    vwrite8(alis.script->vram_org + --offset, byte2);
    
    // loop w/ counter, read words, store backwards
    while(counter--) {
        u16 w = script_read16();
        offset -= 2;
        vwrite16(alis.script->vram_org + offset, w);
    }
}

static void crandom(void) {
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
    
    // NOTE: looks like the only code called is adir* and aloc*
    // we set zero flag using last called instructions there
    
    save_loop_pc = alis.script->pc;
    alis.varD7 = -1;
    readexec_addname_swap();
    if(!alis.sr.zero)
    {
        alis.script->pc = save_loop_pc;
        script_jump(offset);
    }
}

static void cloop8(void) {
    cloop((s8)script_read8());
}

static void cloop16(void) {
    cloop(script_read16());
}

static void cloop24(void) {
    cloop(script_read24());
}

static void cswitch1(void) {
    readexec_opername();

    s16 addition = script_read8();
    if ((alis.script->pc & 1) != 0)
    {
        alis.script->pc ++;
    }

    s16 test;

    do
    {
        test = script_read16();
        if (alis.varD7 == test)
        {
            addition = script_read16();
            alis.script->pc += addition;
            return;
        }

        alis.script->pc += 2;
    }
    while (test <= alis.varD7 && (--addition) != -1);

    if (addition < 0)
    {
        return;
    }

    alis.script->pc += (addition * 4);
}

static void cswitch2(void) {
    readexec_opername();
    
    s16 addition = script_read8();
    s32 new_pc = alis.script->pc;
    if ((new_pc & 1) != 0)
    {
        alis.script->pc ++;
    }

    s16 test = script_read16();
    s16 val = alis.varD7 + test;
    if (val >= 0 && val <= addition)
    {
        alis.script->pc += (val * 2);
        s16 test2 = script_read16();
        alis.script->pc += test2;
    }
    else
    {
        alis.script->pc += (addition * 2 + 2);
    }
}

static void cleave(void) {
    debug(EDebugWarning, " /* CHECK */");
    if (alis.fseq == 0)
    {
        s16 vacc_offset = get_0x0c_vacc_offset(alis.script->vram_org);
        if (vacc_offset != 0)
        {
            set_0x0c_vacc_offset(alis.script->vram_org, 0);
            set_0x08_script_ret_offset(alis.script->vram_org, vread32(alis.script->vram_org + vacc_offset));
            set_0x0a_vacc_offset(alis.script->vram_org, vacc_offset + 4);
            return;
        }
    }
    else
    {
        alis.script->vacc_off = get_0x0c_vacc_offset(alis.script->vram_org);
        debug(EDebugInfo, " [va %.4x]", (s16)alis.script->vacc_off);
        if (alis.script->vacc_off != 0)
        {
            set_0x0c_vacc_offset(alis.script->vram_org, 0);
            alis.script->pc = vread32(alis.script->vram_org + alis.script->vacc_off);
            debug(EDebugInfo, " (7NAME: %s, VACC: 0x%x, PC: 0x%x) ", alis.script->name, alis.script->vacc_off, alis.script->pc);
            return;
        }
    }
    
    if (alis.fseq == 0 && alis.saversp <= alis.script->vacc_off)
    {
        return;
    }
    
    s32 new_pc = vread32(alis.script->vram_org + alis.script->vacc_off);
    if (alis.script->vacc_off == get_0x0c_vacc_offset(alis.script->vram_org))
    {
        set_0x0c_vacc_offset(alis.script->vram_org, 0);
    }
    
    alis.script->pc = new_pc;
    debug(EDebugInfo, " (8NAME: %s, VRAM: 0x%x - 0x%x, VACC: 0x%x, PC: 0x%x) ", alis.script->name, alis.script->vram_org, alis.finent, alis.script->vacc_off, alis.script->pc_org);
}

static void cprotect(void) {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername();
    // alis.vprotect = alis.varD7;
}

static void casleep(void) {
    readexec_opername();
    if (-1 < alis.varD7)
    {
        sAlisScript *script = ENTSCR(alis.varD7);
        set_0x04_cstart_csleep(script->vram_org, 0);
    }
}

static void cscmov(void) {
    readexec_opername_saveD7();
    s16 x = alis.varD7;
    readexec_opername_saveD7();
    s16 y = alis.varD7;
    readexec_opername_saveD7();
    s16 z = alis.varD7;
    
    u16 scene = get_0x16_screen_id(alis.script->vram_org);
    set_scene_unknown0x2e(scene, z);
    set_scene_unknown0x2c(scene, y);
    set_scene_unknown0x2a(scene, x);
    set_scene_state(scene, get_scene_state(scene) | 0x80);
}

static void cscset(void) {
    readexec_opername_saveD7();
    s16 x = alis.varD7;
    readexec_opername_saveD7();
    s16 y = alis.varD7;
    readexec_opername_saveD7();
    s16 z = alis.varD7;
    
    u16 scene = get_0x16_screen_id(alis.script->vram_org);
    set_scene_unknown0x2e(scene, z - get_scene_depz(scene));
    set_scene_unknown0x2c(scene, y - get_scene_depy(scene));
    set_scene_unknown0x2a(scene, x - get_scene_depx(scene));
    set_scene_state(scene, get_scene_state(scene) | 0x80);

    // Ishar 3
//    if (z != get_scene_depz(scene))
//    {
//        set_scene_depz(scene, z);
//        set_scene_state(scene, get_scene_state(scene) | 0x80);
//    }
//
//    if (y != get_scene_depy(scene))
//    {
//        set_scene_depy(scene, y);
//        set_scene_state(scene, get_scene_state(scene) | 0x80);
//    }
//
//    if (x != get_scene_depx(scene))
//    {
//        set_scene_depx(scene, x);
//        set_scene_state(scene, get_scene_state(scene) | 0x80);
//    }
}

static void cclipping(void) {
    alis.fswitch = 0;
}

static void cswitching(void) {
    alis.fswitch = 1;
}

static void cwlive(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cunload(void) {
    s16 id = script_read16();
    if (id != -1 && id != get_0x10_script_id(alis.script->vram_org))
    {
        s16 index = debprotf(id);
        if (index != -1)
        // while (index != -1)
        {
            u32 data_org = alis.atprog_ptr[index];
            u32 data_end = 0xfffffff;
            
            for (int i = 0; i < alis.nbprog; i++)
            {
                if (data_org < alis.atprog_ptr[i] && alis.atprog_ptr[i] < data_end)
                {
                    data_end = alis.atprog_ptr[i];
                }
            }
            
            if (data_end == 0xfffffff)
            {
                data_end = alis.finprog;
            }
            
//            shrinkprog(data_org, data_end - data_org, id);
            sAlisScript *script = alis.progs[index];
            if (script->data_org != data_org)
            {
                sleep(0);
            }
            
            s32 sz = data_end - data_org;
            if (script->sz != sz)
            {
                sleep(0);
            }

            debug(EDebugInfo, " (NAME: %s, ID: 0x%x) ", script->name, script->header.id);
            shrinkprog(script->data_org, script->sz, script->header.id);
        }
    }
}

static void cwakeup(void) {
    readexec_opername();
    if (-1 < alis.varD7)
    {
        sAlisScript *script = ENTSCR(alis.varD7);
        set_0x04_cstart_csleep(script->vram_org, 1);
    }
}

static void csleep(void) {
    set_0x04_cstart_csleep(alis.script->vram_org, 0);
    if (alis.fseq)
    {
        cstop();
    }
}

static void clive(void) {
    alis.wcx = 0;
    alis.wcy = 0;
    alis.wcz = 0;
    
    alis.varD7 = -1;
    
    s16 id = script_read16();
    if (id != -1)
    {
        s16 index = debprotf(id);
        if (index != -1)
        {
            sAlisScript *prog = alis.progs[index];

            sAlisScript *script = (sAlisScript *)malloc(sizeof(sAlisScript));
            memcpy(script, prog, sizeof(sAlisScript));
            
            script_live(script);
            
            u8 *prev_vram = alis.mem + alis.script->vram_org;
            u8 *next = alis.mem + script->vram_org;
            memcpy(next, prev_vram, 8);
            
            xadd16(script->vram_org + 0, alis.wcx);
            xadd16(script->vram_org + 2, alis.wcz);
            xadd16(script->vram_org + 4, alis.wcy);
            
            *(u8 *)(next + 0x9) = *(u8 *)(prev_vram + 0x9);
            *(u8 *)(next + 0xa) = *(u8 *)(prev_vram + 0xa);
            *(u8 *)(next + 0xb) = *(u8 *)(prev_vram + 0xb);

            *(s16 *)(alis.mem + script->vram_org - 0x28) = *(s16 *)(alis.mem + alis.script->vram_org - 0xe);
            *(s16 *)(alis.mem + script->vram_org - 0x16) = *(s16 *)(alis.mem + alis.script->vram_org - 0x16);
            *(s16 *)(alis.mem + script->vram_org - 0x22) = *(s16 *)(alis.mem + alis.script->vram_org - 0x22);
            *(s16 *)(alis.mem + script->vram_org - 0x2a) = *(s16 *)(alis.mem + script->vram_org - 0xe);

            // NOTE: just to show already running scripts
            
            debug(EDebugInfo, "\n");

            u16 tent = 0;
            u32 loop = 8 > alis.nbent ? 8 : alis.nbent;
            for (int i = 0; i < loop; i++)
            {
                tent = alis.atent_ptr[i].offset;

                sAlisScript *s = alis.scripts[i];
                if (s && s->vram_org)
                {
                    u32 datasize = sizeof(sScriptContext) + s->header.w_unknown5 + s->header.w_unknown7;
                    s32 vramsize = s->header.vram_alloc_sz;
                    s32 shrinkby = datasize + vramsize;
                    
                    debug(EDebugInfo, "%c[%s ID: %.2x(%.2x), %.2x, %.6x, %.6x] \n", script->vram_org == s->vram_org ? '*' : ' ', s->name, s->header.id, get_0x10_script_id(s->vram_org), xswap16(tent), s->vram_org, shrinkby);
                }
                else
                {
                    debug(EDebugInfo, " [ empty  ID: 00(00), %.2x, 000000, 000000] \n", xswap16(tent));
                }
            }

            debug(EDebugInfo, " [finent %.6x] \n", alis.finent);

            if (alis.maxent < alis.nbent)
            {
                debug(EDebugFatal, "ERROR: Exceeded number of scripts slots!\n");
                return;
            }
        }
    }

    cstore_continue();
}

static void ckill(void) {
    readexec_opername_saveD7();
    if (alis.varD7 < 0)
        return;

     killent(alis.varD7, alis.varD5);
}

static void cstop(void) {
    // in real program, adds 4 to real stack pointer
    alis.script->running = 0;
    debug(EDebugInfo, "\n-- CSTOP --");
}

static void cstopret(void) {
    // never seen in ishar execution (boot2game)
    debug(EDebugWarning, " /* MISSING */");                                     
}

static void cexit(void) {
    if (alis.varD5 == 0)
    {
        exit(-1);
        return;
    }

    killent(alis.varD5, alis.varD5);
    alis.script->running = 0;
}

static void cload(void) {
    
    // get script ID
    u16 id = script_read16();
    if(id != 0) {
        // not main script, depack and load into vm
        char path[kPathMaxLen] = {0};
        strcpy(path, alis.platform.path);
        script_read_until_zero((u8 *)(path + strlen(alis.platform.path)));
        strcpy(strrchr(path, '.') + 1, alis.platform.ext);
        script_load(strlower((char *)path));
    }
    else
    {
        // NOTE: load main script and start game loop
        // we are dooing it elsewhere, shouldnt ever be reached;
        debug(EDebugWarning, " /* STUBBED */");

        readexec_opername_swap();
    }
}

// =============================================================================
void FUN_00013d82(void) {
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
void FUN_00013dda(void) {
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
static void cdefsc(void) {
    if (alis.libsprit == 0)
        return;

    u16 scene = script_read16();
    set_scene_state(scene, 0x40);
    set_scene_numelem(scene, script_read8());
    set_scene_screen_id(scene, alis.libsprit);

    u8 *ptr = alis.mem + alis.basemain + scene + 6;
    for (int i = 0; i < 32; i++, ptr++)
        *ptr = script_read8();
    
    set_scene_to_next(scene, 0);
//    set_scene_newx(scene, swap16(get_scene_newx(scene), alis.platform.is_little_endian));
//    set_scene_newy(scene, swap16(get_scene_newy(scene), alis.platform.is_little_endian));
//    set_scene_width(scene, swap16((u8 *)&(scene->width), alis.platform.is_little_endian));
//    set_scene_height(scene, swap16((u8 *)&(scene->height), alis.platform.is_little_endian));
//    set_scene_unknown0x0a(scene, swap16((u8 *)&(scene->unknown0x0a), alis.platform.is_little_endian));
//    set_scene_unknown0x0c(scene, swap16((u8 *)&(scene->unknown0x0c), alis.platform.is_little_endian));
    set_scene_unknown0x2a(scene, 0);
    set_scene_unknown0x2c(scene, 0);
    set_scene_unknown0x2e(scene, 0);

    SpriteVariables *sprite = SPRITE_VAR(alis.libsprit);
    sprite->link = 0;
    sprite->numelem = get_scene_numelem(scene);
    sprite->newx = get_scene_newx(scene);// & 0xfff0;
    sprite->newy = get_scene_newy(scene);
    sprite->newd = 0x7fff;
    sprite->depx = get_scene_newx(scene) + get_scene_width(scene);// | 0xf;
    sprite->depy = get_scene_newy(scene) + get_scene_height(scene);

    alis.libsprit = sprite->to_next;

    scadd(scene);
    vectoriel(scene);
}

static void cscreen(void) {
    u16 screen_id = script_read16();
    if (screen_id != get_0x16_screen_id(alis.script->vram_org)) {
        set_0x16_screen_id(alis.script->vram_org, screen_id);
    }
}

static void cput(void) {
    alis.flagmain = 0;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org);

    readexec_opername();
    
    alis.depx = 0;
    alis.depy = 0;
    alis.depz = 0;
    alis.numelem = 0;
    
    u8 idx = alis.varD7;

    put(idx);
}

static void cputnat(void) {
    alis.flagmain = 0;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org);
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

static void cerase(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cerasen(void) {
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

static void cset(void) {
    readexec_opername();
    vwrite16(alis.script->vram_org + 0, alis.varD7);
    readexec_opername();
    vwrite16(alis.script->vram_org + 2, alis.varD7);
    readexec_opername();
    vwrite16(alis.script->vram_org + 4, alis.varD7);
}

static void cmov(void) {
    readexec_opername();
    xadd16(alis.script->vram_org + 0, alis.varD7);
    readexec_opername();
    xadd16(alis.script->vram_org + 2, alis.varD7);
    readexec_opername();
    xadd16(alis.script->vram_org + 4, alis.varD7);
}

static void copensc(void) {
    u16 id = script_read16();
    *(alis.mem + alis.basemain + id) &= 0xbf;
    *(alis.mem + alis.basemain + id) |= 0x80;

    scbreak(id);
    scadd(id);
}

static void cclosesc(void) {
    u16 id = script_read16();
    *(alis.mem + alis.basemain + id) |= 0x40;
}

static void cerasall(void) {
    u16 tmpidx = 0;
    u16 curidx = get_0x18_unknown(alis.script->vram_org);
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }
    
    alis.ferase = 0;
}

static void cforme(void) {
    readexec_opername();
    set_0x1a_cforme(alis.script->vram_org, alis.varD7);
}

static void cdelforme(void) {
    set_0x1a_cforme(alis.script->vram_org, -1);
}

s16 px1;
s16 py1;
s16 pz1;

s16 px2;
s16 py2;
s16 pz2;

s32 multiform(s32 vram, s32 a2, s32 a3, s32 a4, s32 d0);
s32 monoform(s32 vram, s32 a2, s32 a3, s32 a4, s32 d0, u8 zf);

// TODO: cleanup
s32 traitfirm(s32 vram, s32 a3, s32 a4, s32 d0)
{
    debug(EDebugWarning, " /* CHECK */");
    s16 uVar1;
    u8 bVar2;
    u16 uVar3;
    s16 sVar4;
    s16 sVar5;
    s16 sVar6;
    s16 sVar7;
    s16 sVar8;
    s16 sVar9;
    s16 sVar10;

    s32 a2 = a4 + xread16(a4 + d0 * 2);
    s32 puVar11 = a2 + 2;
    if (-1 < xread16(a2))
    {
        if (xread16(a2) == 0)
        {
            if ((xread16(puVar11) & alis.matmask) == 0)
                return 1;

            alis.goodmat = xread16(a2 + 2);
            sVar4 = (s8)xread8(a2 + 2);
            if (get_0x03_xinv(vram) != 0)
                sVar4 = -sVar4;

            sVar4 += xread16(vram + 0);
            sVar5 = (s8)xread8(a2 + 5) + xread16(vram + 2);
            sVar7 = (s8)xread8(a2 + 3) + xread16(vram + 4);
            sVar8 = (s8)xread8(a2 + 7);
            if (get_0x03_xinv(vram) != 0)
                sVar8 = -sVar8;

            sVar8 += sVar4;
            sVar9 = (s8)xread8(a2 + 4) + sVar5;
            sVar10 = (s8)xread8(a2 + 9) + sVar7;
        }
        else
        {
            if ((s8)xread8(a2) != 1)
                return 1;

            if ((xread16(puVar11) & alis.matmask) == 0)
                return 1;

            alis.goodmat = xread16(a2 + 2);
            sVar4 = xread16(a2 + 4);
            if (get_0x03_xinv(vram) != 0)
                sVar4 = -sVar4;

            sVar4 += xread16(vram + 0);
            sVar5 = xread16(a2 + 6) + xread16(vram + 2);
            sVar7 = xread16(a2 + 8) + xread16(vram + 4);
            sVar8 = xread16(a2 + 10);
            if (get_0x03_xinv(vram) != 0)
                sVar8 = -sVar8;

            sVar8 += sVar4;
            sVar9 = xread16(a2 + 12) + sVar5;
            sVar10 = xread16(a2 + 14) + sVar7;
        }

        sVar6 = sVar5;
        if (sVar9 <= sVar5)
        {
            sVar6 = sVar9;
            sVar9 = sVar5;
        }

        if ((py1 <= sVar9) && (sVar6 <= py2))
        {
            sVar5 = sVar7;
            if (sVar10 <= sVar7)
            {
                sVar5 = sVar10;
                sVar10 = sVar7;
            }

            if ((pz1 <= sVar10) && (sVar5 <= pz2))
            {
                sVar5 = sVar4;
                if (sVar8 <= sVar4)
                {
                    sVar5 = sVar8;
                    sVar8 = sVar4;
                }

                if ((px1 <= sVar8) && (sVar5 <= px2))
                    return 0;
            }
        }

        return 1;
    }

    bVar2 = xread8(a2 + 1);
    uVar3 = (u16)bVar2;
    if (bVar2 != 0)
    {
        uVar3 = bVar2 - 1;
        do
        {
            uVar1 = xread16(puVar11);
            if ((-1 < uVar1) && (uVar3 = traitfirm(vram, a3, a4, uVar1)))
                return uVar3;

            uVar3 --;
            puVar11 = puVar11 + 1;
        }
        while (uVar3 != 0xffff);

        uVar3 = 1;
    }

    return uVar3;
}

// TODO: cleanup
s32 traitform(s32 vram, s32 a2, s32 a4, s32 d0)
{
    debug(EDebugWarning, " /* CHECK */");

    d0 = d0 * 2;
    s32 a3 = xread16(alis.baseform + d0) + alis.baseform;
    if (xread16(a3) < 0)
    {
        return multiform(vram, a2, a3, a4, d0);
    }
    
    return monoform(vram, a2, a3, a4, d0, xread16(a3) == 0);
}

// TODO: cleanup
s32 multiform(s32 vram, s32 a2, s32 a3, s32 a4, s32 d0)
{
    debug(EDebugWarning, " /* CHECK */");
    
    s8 test = xread8(a3 - 1);
    u8 result = test == 0;
    if (!result)
    {
        s16 length = xread8(a3 - 1) - 1;
        
        do
        {
            d0 = xread16(a3);
            if (-1 < d0)
            {
                result = traitform(vram, a2, a4, d0);
                if (result)
                {
                    return result;
                }
            }
            
            length --;
            a3 += 2;
        }
        while (length != -1);
        
        result = 1;
    }
    
    return result;
}

// TODO: cleanup
s32 monoform(s32 vram, s32 a2, s32 a3, s32 a4, s32 d0, u8 zf)
{
    s16 frmx;
    s16 frmy;
    s16 frmz;

    s32 a3nxt;
    
    if (zf)
    {
        frmx = (s8)xread8(a3 + 2);
        frmy = (s8)xread8(a3 + 3);
        frmz = (s8)xread8(a3 + 4);
        if (get_0x03_xinv(alis.script->vram_org) != 0)
            frmx = -frmx;

        frmx += alis.wcx;
        frmy += alis.wcy;
        frmz += alis.wcz;
        px2 = (s8)xread8(a3 + 5);
        py2 = (s8)xread8(a3 + 6);
        pz2 = (s8)xread8(a3 + 7);
        a3nxt = a3 + 8;
        if (get_0x03_xinv(alis.script->vram_org) != 0)
            px2 = -px2;

        px2 += frmx;
        py2 += frmy;
        pz2 += frmz;
    }
    else
    {
        if (xread8(a3 - 2) != 1)
            return 1;

        frmx = xread16(a3 + 2);
        frmy = xread16(a3 + 4);
        frmz = xread16(a3 + 6);
        if (get_0x03_xinv(alis.script->vram_org) != 0)
            frmx = -frmx;

        frmx += alis.wcx;
        frmy += alis.wcy;
        frmz += alis.wcz;
        
        px2 = xread16(a3 + 8);
        py2 = xread16(a3 + 0xa);
        pz2 = xread16(a3 + 0xc);
        a3nxt = a3 + 0xe;
        if (get_0x03_xinv(alis.script->vram_org) != 0)
            px2 = -px2;

        px2 += frmx;
        py2 += frmy;
        pz2 += frmz;
    }
    
    py1 = frmy;
    if (py2 <= frmy)
    {
        py1 = py2;
        py2 = frmy;
    }
    
    pz1 = frmz;
    if (pz2 <= frmz)
    {
        pz1 = pz2;
        pz2 = frmz;
    }
    
    px1 = frmx;
    if (px2 <= frmx)
    {
        px1 = px2;
        px2 = frmx;
    }
    
    s32 a2b = a2 + 2;
    if (xread16(a2) < 0)
    {
        u8 result = xread8(a2 + 1) != 0 ? 1 : 0;
        if (result)
        {
            s16 length = xread8(a2 + 1) - 1;
            
            do
            {
                d0 = xread16(a2b);
                a2b += 2;
                if (-1 < d0)
                {
                    result = traitfirm(vram, a3nxt, a4, d0);
                    if (!result)
                        return result;
                }
                
                length --;
            }
            while (length != -1);
        }
        
        return result;
    }

    u16 result;

    s16 tmpx1;
    s16 tmpy1;
    s16 tmpz1;
    s16 tmpx2;
    s16 tmpy2;
    s16 tmpz2;

    if (xread16(a2) == 0)
    {
        result = xread16(a2b) & alis.matmask;
        if (result == 0)
            return 1;
        
        alis.goodmat = xread16(a2 + 2);
        
        tmpx1 = (s8)xread8(a2 + 4);
        tmpy1 = (s8)xread8(a2 + 5);
        tmpz1 = (s8)xread8(a2 + 6);
        if (get_0x03_xinv(vram) != 0)
            tmpx1 = -tmpx1;
        
        tmpx1 += xread16(vram + 0);
        tmpy1 += xread16(vram + 2);
        tmpz1 += xread16(vram + 4);
        
        tmpx2 = (s8)xread8(a2 + 7);
        tmpy2 = (s8)xread8(a2 + 8);
        tmpz2 = (s8)xread8(a2 + 9);
        if (get_0x03_xinv(vram) != 0)
            tmpx2 = -tmpx2;
        
        tmpx2 += tmpx1;
        tmpy2 += tmpy1;
        tmpz2 += tmpz1;
    }
    else
    {
        if (xread8(a2) != 1)
            return 1;
        
        result = xread16(a2b) & alis.matmask;
        if (result == 0)
            return 1;
        
        alis.goodmat = xread16(a2 + 2);
        
        tmpx1 = xread16(a2 + 4);
        tmpy1 = xread16(a2 + 6);
        tmpz1 = xread16(a2 + 8);
        if (get_0x03_xinv(vram) != 0)
            tmpx1 = -tmpx1;
        
        tmpx1 += xread16(vram + 0);
        tmpy1 += xread16(vram + 2);
        tmpz1 += xread16(vram + 4);

        tmpx2 = xread16(a2 + 0xa);
        tmpy2 = xread16(a2 + 0xc);
        tmpz2 = xread16(a2 + 0xe);
        if (get_0x03_xinv(vram) != 0)
            tmpx2 = -tmpx2;
        
        tmpx2 += tmpx1;
        tmpy2 += tmpy1;
        tmpz2 += tmpz1;
    }
    
    s16 tmp = tmpy1;
    if (tmpy2 <= tmpy1)
    {
        tmp = tmpy2;
        tmpy2 = tmpy1;
    }
    
    if ((py1 <= tmpy2) && (tmp <= py2))
    {
        tmp = tmpz1;
        if (tmpz2 <= tmpz1)
        {
            tmp = tmpz2;
            tmpz2 = tmpz1;
        }
        
        if ((pz1 <= tmpz2) && (tmp <= pz2))
        {
            tmp = tmpx1;
            if (tmpx2 <= tmpx1)
            {
                tmp = tmpx2;
                tmpx2 = tmpx1;
            }
            
            if ((px1 <= tmpx2) && (tmp <= px2))
                return 0;
        }
    }
    
    return 1;
}

void clipform(void) {
    alis.ptrent = alis.tablent;
    if (-1 < alis.wforme)
    {
        s32 val = get_0x14_script_org_offset(alis.script->vram_org);
        s32 addr = xread32(val + 0xe) + val;
        alis.baseform = xread32(addr + 6) + addr;
        s32 atforme = xread16(alis.baseform + alis.wforme * 2) + alis.baseform;
        s16 entidx = 0;

        do
        {
            s32 ent_vram = xread32(alis.atent + entidx);
            if (xread16(ent_vram + 6) == xread16(alis.script->vram_org + 6) && -1 < xread16(ent_vram - 0x1a) && alis.script->vram_org != ent_vram)
            {
                s32 val2 = get_0x14_script_org_offset(ent_vram);
                s32 addr2 = xread32(val2 + 0xe) + val2;
                s32 a4 = xread32(addr2 + 6) + addr2;
                s32 iVar3 = get_0x1a_cforme(ent_vram) * 2;
                s8 test = xread8(atforme);
                s32 a2 = xread16(a4 + iVar3) + a4;
                
                s32 result = 0;

                if (test < 0)
                {
                    result = multiform(ent_vram, a2, atforme + 2, a4, iVar3);
                }
                else
                {
                    result = monoform(ent_vram, a2, atforme + 2, a4, iVar3, test == 0);
                }
                
                if (!result)
                {
                    if (alis.witmov != '\0')
                    {
                        alis.fmitmov = 1;
                        return;
                    }

                    int index = (int)((alis.ptrent - alis.tablent) / 2);
                    alis.matent[index] = alis.goodmat;
                    alis.tablent[index] = entidx;
                    alis.ptrent += 2;
                    atforme += 2;

                    if (alis.fallent == 0)
                    {
                        break;
                    }
                }
            }

            entidx = xread16(alis.atent + 4 + entidx);
        }
        while (entidx != 0);
    }

    if (alis.witmov == 0)
    {
        // NOTE: matent!!!
        int index = (int)((alis.ptrent - alis.tablent) / 2);
        alis.matent[index] = 0;
        alis.tablent[index] = -1;
        alis.ptrent++;
    }
}

static void ctstmov(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername();
    alis.wcx = vread16(alis.script->vram_org + 0) + alis.varD7;
    readexec_opername();
    alis.wcy = vread16(alis.script->vram_org + 2) + alis.varD7;
    readexec_opername();
    alis.wcz = vread16(alis.script->vram_org + 4) + alis.varD7;
    readexec_opername();
    alis.wforme = get_0x1a_cforme(alis.script->vram_org);
    alis.matmask = alis.varD7;
    clipform();
    crstent();
}

static void ctstset(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cftstmov(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername();
    alis.wcx = vread16(alis.script->vram_org + 0) + alis.varD7;
    readexec_opername();
    alis.wcy = vread16(alis.script->vram_org + 2) + alis.varD7;
    readexec_opername();
    alis.wcz = vread16(alis.script->vram_org + 4) + alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    readexec_opername();
    alis.wforme = alis.varD7;
    clipform();
    crstent();
}

static void cftstset(void) {
    readexec_opername();
    alis.wcx = alis.varD7;
    readexec_opername();
    alis.wcy = alis.varD7;
    readexec_opername();
    alis.wcz = alis.varD7;
    readexec_opername();
    alis.matmask = alis.varD7;
    readexec_opername();
    alis.wforme = alis.varD7;
    clipform();
    crstent();
}

static void csuccent(void) {
    
    alis.varD7 = *alis.ptrent;
    
    if (-1 < *alis.ptrent)
        alis.ptrent ++;

    cstore_continue();
}

static void cpredent(void) {
    if (alis.ptrent != alis.tablent)
    {
        alis.ptrent -= 2;
        alis.varD7 = *alis.ptrent;
    }
    else
    {
        alis.varD7 = -1;
    }

    cstore_continue();
}

u8 calcnear(u32 source, u32 target)
{
    s16 wcx = xread16(target + 0) - xread16(source + 0);
    s16 wcz = xread16(target + 2) - xread16(source + 2);
    s16 wcy = xread16(target + 4) - xread16(source + 4);
    s32 val = (s32)wcx * (s32)wcx + (s32)wcz * (s32)wcz + (s32)wcy * (s32)wcy;
    if (val <= alis.valnorme && (alis.fview == 0 || val <= (s32)(s16)(xread8(source + 0x9) * wcx + xread8(source + 0x5) * wcz + xread8(source + 0xb) * wcy) * (s32)alis.valchamp))
    {
        return 1;
    }
    
    return 0;
}

static void cnearent(void) {
    debug(EDebugWarning, " /* CHECK */");
    
    alis.fview = 0;
    
    readexec_opername();
    u16 val = alis.varD7;
    alis.valnorme = (u32)val * (u32)val;
    
    readexec_opername();
    s16 entidx = alis.varD7;
    s16 tabidx = 0;
    
    u32 src_vram = alis.script->vram_org;
    u32 tgt_vram = alis.atent + entidx;
    
    if (xread16(tgt_vram + 6) == xread16(src_vram + 6))
    {
        u8 result = calcnear(src_vram, tgt_vram);
        if (!result)
        {
            alis.tablent[tabidx++] = entidx;
        }
    }
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    crstent();
}

static void cneartyp(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cnearmat(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewent(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewtyp(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewmat(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void corient(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void crstent(void) {
    alis.ptrent = alis.tablent;
    csuccent();
}

static void csend(void) {
    u16 length = script_read8();
    readexec_opername();
    s16 entry = alis.varD7;
    if (entry == -1)
        goto CSENDEXIT;
    
    sAlisScript *script = ENTSCR(entry);
    if (script->vram_org == 0)
        goto CSENDEXIT;
    
    if ((get_0x24_scan_inter(script->vram_org) & 1) != 0)
        goto CSENDEXIT;
    
    s16 offset = swap16((alis.mem + get_0x14_script_org_offset(script->vram_org) + 0x16), alis.platform.is_little_endian);
    s16 old_vacc = get_0x1c_scan_clr(script->vram_org);
    s16 new_vacc;

    while (1)
    {
        readexec_opername();

        new_vacc = old_vacc + 2;
        if (-0x35 < new_vacc)
        {
            new_vacc -= offset;
        }
        
        if (new_vacc == get_0x1e_scan_clr(script->vram_org))
            break;
        
        vwrite16(script->vram_org + old_vacc, alis.varD7);
        
        length --;
        old_vacc = new_vacc;
        if (length == 0xffff)
        {
            set_0x1c_scan_clr(script->vram_org, new_vacc);
            set_0x24_scan_inter(script->vram_org, get_0x24_scan_inter(script->vram_org) | 0x80);
            return;
        }
    }
    
    set_0x1c_scan_clr(script->vram_org, new_vacc);
    
    while ((--length) != 0xffff)
    {
CSENDEXIT:
        readexec_opername();
    }
}

static void cscanclr(void) {
    set_0x1e_scan_clr(alis.script->vram_org, get_0x1c_scan_clr(alis.script->vram_org));
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0x7f);
}

static void cscanon(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfe);
}

static void cscanoff(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 1);
    cscanclr();
}

static void cinteron(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) & 0xfd);
}

static void cinteroff(void) {
    set_0x24_scan_inter(alis.script->vram_org, get_0x24_scan_inter(alis.script->vram_org) | 2);
}

static void callentity(void) {
    alis.fallent = 1;
}

static void cpalette(void) {
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

static void cdefcolor(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void ctiming(void) {
    readexec_opername();
    alis._ctiming = (u8)(alis.varD7 & 0xff);
}

static void czap(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    alis.pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    alis.priorson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    readexec_opername();
    alis.freqson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    if (alis.volson == 0)
    {
        return;
    }
    
    readexec_opername();
    alis.dvolson = 0;
    alis.typeson = 1;
    alis.dfreqson = alis.varD7;
    runson();
}

static void cexplode(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    alis.pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    alis.priorson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    readexec_opername();
    alis.freqson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    if (alis.volson == 0)
    {
        return;
    }
    
    alis.dfreqson = 0;
    u32 vol32 = ((u32)alis.volson << 8) / (u32)alis.volson;
    s16 vol16 = (s16)vol32;
    if (vol32 == 0)
    {
        vol16 = 1;
    }
    
    alis.dvolson = -vol16;
    alis.typeson = 3;
    runson();
}

static void cding(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    alis.pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    alis.priorson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    readexec_opername();
    alis.freqson = alis.varD7;
    readexec_opername();
    alis.longson = alis.varD7;
    if (alis.longson == 0)
    {
        return;
    }
    
    alis.dfreqson = 0;
    u32 tmpa = ((u32)alis.volson << 8) / (u32)alis.longson;
    u32 tmpb = ((u32)alis.volson << 8) % (u32)alis.longson;
    u32 newdvolson = tmpa == 0 ? 1 : tmpb << 0x10 | tmpa;

    alis.dvolson = -(s16)newdvolson;
    alis.typeson = 1;
    runson();
}

static void cnoise(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    alis.pereson = get_0x0e_script_ent(alis.script->vram_org);
    readexec_opername();
    alis.priorson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    readexec_opername();
    alis.freqson = alis.varD7;
    readexec_opername();
    alis.volson = alis.varD7;
    if (alis.volson == 0)
    {
        return;
    }
    
    alis.dfreqson = 0;
    u32 vol32 = ((u32)alis.volson << 8) / (u32)alis.volson;
    s16 vol16 = (s16)vol32;
    if (vol32 == 0)
    {
        vol16 = 1;
    }
    
    alis.dvolson = -vol16;
    alis.typeson = 2;
    runson();
}

static void cinitab(void) {
    debug(EDebugWarning, " /* MISSING */");
}

int FUN_FileExists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    
    return 0;
}

uint FUN_OpenFile(char *path, uint mode, uint d1)
{
    u32 uVar1;
    
    u16 w_file_open_mode = (u16)mode;
    char *_addr_file_name = path;
    if ((mode & 0x100) == 0 || FUN_FileExists(path))
    {
        u8 bVar2 = (mode & 0x200) == 0;
        if (bVar2)
        {
            alis.fp = sys_fopen(path, d1);
            if (alis.fp != NULL)
            {
                uVar1 = mode & 0xffff0000 | (uint)w_file_open_mode;
                if ((w_file_open_mode & 0x400) == 0)
                {
                    return uVar1;
                }
                
                bVar2 = (int)mode < 0;
                // SYS_SeekFile();
                if (!bVar2)
                {
                    return uVar1;
                }
            }
            //uVar1 = SYS_PrintError();
            return uVar1;
        }
        
        if (FUN_FileExists(path))
        {
            // uVar1 = FUN_FileTruncate();
            return uVar1;
        }
    }
    
    // uVar1 = FUN_CreateFile();
    return uVar1;
}

static void cfopen(void) {
    char path[kPathMaxLen] = {0};
    strcpy(path, alis.platform.path);
    
    u16 mode;

    if(*(alis.mem + alis.script->pc) == 0xff)
    {
        script_jump(1);
        readexec_opername_swap();
        readexec_opername();

        strcat(path, (char *)alis.sd7);
        mode = alis.varD7;
    }
    else
    {
        script_read_until_zero((u8 *)(path + strlen(alis.platform.path)));
        mode = script_read16();
    }

    alis.fp = sys_fopen((char *)path, mode);
    if(alis.fp == NULL) {
        alis_error(ALIS_ERR_FOPEN, path);
    }
}

static void cfclose(void) {
    if(sys_fclose(alis.fp) < 0) {
        alis_error(ALIS_ERR_FCLOSE);
    }
    
    alis.fp = NULL;
}

static void cfcreat(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfdel(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfreadv(void) {
    fread(alis.buffer, 2, 1, alis.fp);
    alis.varD7 = xswap16(*(s16 *)alis.buffer);
    cstore_continue();
}

static void cfwritev(void) {
    readexec_opername();
    *(s16 *)alis.buffer = xswap16(alis.varD7);
    fwrite(alis.buffer, 2, 1, alis.fp);
}

static void cfwritei(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfreadb(void) {
    s32 addr = (s16)script_read16();
    if (addr == 0)
    {
        addr = (s16)script_read16();
        addr += alis.basemain;
    }
    else
    {
        addr += alis.script->vram_org;
    }

    u16 length = script_read16();

    // NOTE: *.fic files in all platforms are identical, some byteswaping should be needed
    fread(alis.mem + addr, length, 1, alis.fp);
}

static void cfwriteb(void) {
    s32 addr = (s16)script_read16();
    if (addr == 0)
    {
        addr = (s16)script_read16();
        addr += alis.basemain;
    }
    else
    {
        addr += alis.script->vram_org;
    }

    u16 length = script_read16();

    fwrite(alis.mem + addr, length, 1, alis.fp);
}

static void cplot(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdraw(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 prevx = alis.poldx;
    s16 prevy = alis.poldy;
    
    alis.poldx += alis.varD7;
    alis.poldy += alis.varD6;
    
    // io_line(prevx, prevy);
}

static void cbox(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cboxf(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cink(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpset(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpmove(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpmode(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpicture(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cxyscroll(void) {
    readexec_opername();
    readexec_opername();
}

static void clinking(void) {
    readexec_opername();
    set_0x2a_clinking(alis.script->vram_org, alis.varD7);
}

static void cmouson(void) {
    sys_enable_mouse(1);
}

static void cmousoff(void) {
    sys_enable_mouse(0);
}

// 0x86 - 14d62
static void cmouse(void) {
    mouse_t mouse = sys_get_mouse();
   
    alis.varD7 = mouse.x;
    cstore_continue();
    
    alis.varD7 = mouse.y;
    cstore_continue();
    
    alis.varD7 = mouse.lb ? 1 : (mouse.rb ? 2 : 0);
    cstore_continue();
    
    mouse.lb = 0;
    
    alis.butmouse = 0;
    alis.cbutmouse = 0;
}

static void cdefmouse(void) {
    alis.flagmain = 0;
    readexec_opername();
    u8 *ptr = alis.mem + alis.script->data_org + adresdes(alis.varD7);
    if (*ptr == 0 || *ptr == 0x10 || *ptr == 0x14)
    {
        alis.desmouse = ptr;
    }
}

static void csetmouse(void) {
    readexec_opername();
    u16 x = alis.varD7;
    readexec_opername();
    u16 y = alis.varD6;
    sys_set_mouse(x, y);
}

static void cdefvect(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void csetvect(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void capproach(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cescape(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername();
    s16 sVar2 = alis.varD7;
    if (sVar2 != -1)
    {
//        deb_approach();
//
//        s32 iVar1 = 0x7fffffff;
//        int unaff_D4;
//        char unaff_D6b = alis.varD6;
//        u8 *in_A0;
//        u8 *in_A1;
//        int unaff_A6;
//
//        do
//        {
//            calscal();
//
//            if (unaff_D4 < iVar1)
//            {
//                unaff_D6b = (char)sVar2;
//                iVar1 = unaff_D4;
//                in_A1 = in_A0;
//            }
//
//            in_A0 += 3;
//            sVar2 --1;
//        }
//        while (sVar2 != -1);
//
//        *(char *)(unaff_A6 + 8) = *(char *)(unaff_A6 + 8) - unaff_D6b;
//        *(u8 *)(unaff_A6 + 9) = *in_A1;
//        *(u8 *)(unaff_A6 + 10) = in_A1[1];
//        *(u8 *)(unaff_A6 + 0xb) = in_A1[2];
    }
}

static void cvtstmov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cvftstmov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cvmov(void) {
    debug(EDebugWarning, " /* CHCECK */");
    xadd16(alis.script->vram_org + 0, xread8(alis.script->vram_org + 0x9));
    xadd16(alis.script->vram_org + 2, xread8(alis.script->vram_org + 0x5));
    xadd16(alis.script->vram_org + 4, xread8(alis.script->vram_org + 0xb));
}

static void cdefworld(void) {
    debug(EDebugWarning, " /* CHCECK */");
    s16 offset = script_read16();
    u8 counter = 5;
    while(counter--) {
        vwrite8(alis.script->vram_org + offset, script_read8());
    }
}

static void cworld(void) {
    vwrite8(alis.script->vram_org + 0xffde, script_read8());
    vwrite8(alis.script->vram_org + 0xffdf, script_read8());
}

static void cfindmat(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfindtyp(void) {
    readexec_opername();
    
    if (alis.varD7 < 0)
    {
        alis.varD7 &= 0xff;
    }

    s16 script_id = alis.varD7;

    s16 offset = 0;
    
    sAlisScript *script;
    
    s32 tabidx = 0;
    
    do
    {
        script = ENTSCR(offset);
        if (script_id == get_0x10_script_id(script->vram_org) && alis.script->vram_org != script->vram_org)
        {
            alis.matent[tabidx] = 0;
            alis.tablent[tabidx] = offset;
            tabidx ++;
            
            if (alis.fallent == 0)
            {
                break;
            }
        }
        
        offset = xread16(alis.atent + 4 + offset);
    }
    while (offset != 0);
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    crstent();
}

static void cmusic(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    alis.flagmain = 0;
    
    readexec_opername();
    s16 idx = alis.varD7;
    s16 addr = adresmus(idx);

    u8 type = xread8(alis.script->data_org + addr);
    if (type != 4)
    {
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
        readexec_opername();
    }
    else
    {
//        alis.mupnote = addr + 6;
        readexec_opername();
//        alis.muvolume = 0;
//        alis.maxvolume = alis.maxvolume & 0xff | alis.varD7 << 8;
        readexec_opername();
//        alis.mutempo = (u8)alis.varD7;
        readexec_opername();
//        alis.muattac = alis.varD7 + 1;
        readexec_opername();
//        alis.muduree = (u16)alis.varD7;
        readexec_opername();
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
    }
}

static void cdelmusic(void) {
    debug(EDebugWarning, " /* STUBBED */");
    readexec_opername();
//    alis.muchute = alis.varD7;
//    offmusic();
}

static void ccadence(void) {
    debug(EDebugWarning, " /* STUBBED */");
    
    readexec_opername();
    
    s16 tempo = alis.varD7;
    if (tempo == 0)
        tempo = 1;

//    alis.mutempo = (u8)tempo;
//    tempmusic();
}

static void csetvolum(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cxinv(void) {
    u8 xinv = get_0x03_xinv(alis.script->vram_org);
    BIT_CHG(xinv, 0);
    set_0x03_xinv(alis.script->vram_org, xinv);
}

static void cxinvon(void) {
    set_0x03_xinv(alis.script->vram_org, 1);
}

static void cxinvoff(void) {
    set_0x03_xinv(alis.script->vram_org, 0);
}

static void clistent(void) {
    s16 entidx = 0;
    s16 tabidx = 0;
    while ((entidx = vread16(alis.atent + 4 + entidx)) != 0)
    {
        alis.matent[tabidx] = 0;
        alis.tablent[tabidx] = entidx;
        tabidx ++;
    }
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    crstent();
}

static void csound(void) {
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
    s8 type = xread8(alis.script->data_org + addr);
    if (type == 1 || type == 2)
    {
        if (alis.speedsam == 0)
            alis.speedsam = xread8(alis.script->data_org + addr + 1);

        alis.longsam = xread32(alis.script->data_org + addr + 2) - 0x10;
        alis.startsam = alis.script->data_org + addr + 0x10;
        alis.typeson = 0x80;
        runson();
    }
}

static void cmsound(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void credon(void) {
    set_0x25_credon_credoff(alis.script->vram_org, 0x0);
}

static void credoff(void) {
    set_0x25_credon_credoff(alis.script->vram_org, 0xff);
}

static void cdelsound(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwmov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwtstmov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwftstmov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void ctstform(void) {
    alis.wcx = vread16(alis.script->vram_org + 0);
    alis.wcy = vread16(alis.script->vram_org + 2);
    alis.wcz = vread16(alis.script->vram_org + 4);
    readexec_opername();
    alis.matmask = alis.varD7;
    readexec_opername();
    alis.wforme = alis.varD7;
    clipform();
    crstent();
}

static void cxput(void) {
    alis.flagmain = 0;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org) ^ 1;
    readexec_opername();
    alis.depx = 0;
    alis.depy = 0;
    alis.depz = 0;
    alis.numelem = 0;
    
    u8 idx = alis.varD7;
    put(idx);
}

static void cxputat(void) {
    alis.flagmain = 0;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org) ^ 1;

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

static void cmput(void) {
    alis.flagmain = 1;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org);
    readexec_opername();
    u8 idx = alis.varD7;
    alis.depx = 0;
    alis.depy = 0;
    alis.depz = 0;
    alis.numelem = 0;
    put(idx);
}

static void cmputat(void) {
    alis.flagmain = 1;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org);
    readexec_opername_saveD7();
    alis.depx = alis.varD7;
    readexec_opername_saveD7();
    alis.depy = alis.varD7;
    readexec_opername_saveD7();
    alis.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    alis.numelem = alis.varD6;
    put(alis.varD7);
}

static void cmxput(void) {
    alis.flagmain = 1;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org) ^ 1;
    readexec_opername();
    alis.depx = 0;
    alis.depy = 0;
    alis.depz = 0;
    alis.numelem = 0;
    put(alis.varD7);
}

static void cmxputat(void) {
    alis.flagmain = 1;
    alis.flaginvx = get_0x03_xinv(alis.script->vram_org) ^ 1;
    readexec_opername_saveD7();
    alis.depx = alis.varD7;
    readexec_opername_saveD7();
    alis.depy = alis.varD7;
    readexec_opername_saveD7();
    alis.depz = alis.varD7;
    readexec_opername_saveD7();
    readexec_opername_saveD6();
    alis.numelem = alis.varD6;
    put(alis.varD7);
}

static void cmmusic(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmforme(void) {
    readexec_opername();
    set_0x1a_cforme(alis.script->vram_org, alis.varD7);
}

static void csettime(void) {
    readexec_opername();
    u16 h = alis.varD7;
    readexec_opername();
    u16 m = alis.varD7;
    readexec_opername();
    u16 s = alis.varD7;
    sys_set_time(h, m, s);
}

static void cgettime(void) {
    time_t t = sys_get_time();
    alis.varD7 = t << 16 & 0xff;
    cstore_continue();
    alis.varD7 = t << 8 & 0xff;
    cstore_continue();
    alis.varD7 = t << 0 & 0xff;
    cstore_continue();
}

void oval(void);
void getval(void)
{
    u8 c;
    u8 *ptr = alis.sd7;
    while (1)
    {
        c = getchar();
        if (c == 0xd)
        {
            break;
        }
        
        if (c == 8)
        {
            if (ptr != alis.sd7)
            {
                ptr --;
                *ptr = 0;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else if ((c == 0x2d) || ((-1 < (char)(c - 0x30) && (c < 0x3a))))
        {
            *ptr++ = c;
            putchar(c);
        }
    }
    
    *ptr = 0;
    
    oval();
}

static void cvinput(void) {
    debug(EDebugWarning, " /* CHECK */");
    alis.charmode = 0;
    getval();
    cstore_continue();
}

void getstring(void)
{
    u8 c;
    u8 *ptr = alis.sd7;
    while (1)
    {
        c = getchar();
        if (c == '\r')
        {
            break;
        }
        
        if (c == '\b')
        {
            if (ptr != alis.sd7)
            {
                ptr --;
                *ptr = 0;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        }
        else
        {
            *ptr++ = c;
            putchar(c);
        }
    }
    
    *ptr = 0;
}

static void csinput(void) {
    debug(EDebugWarning, " /* CHECK */");
    alis.charmode = 0;
    getstring();
    cstore_continue();
}

static void crunfilm(void) {
    debug(EDebugWarning, " /* MISSING */");
}

void printd0(s16 d0w)
{
    u8 *ptr = alis.sd7;
    vald0(alis.sd7, d0w);
    
    while (*ptr != 0)
    {
        alis_putchar(*ptr++);
    }
}

void putval(s16 d7w)
{
    printd0(d7w);
}

static void cvpicprint(void) {
    debug(EDebugWarning, " /* CHECK */");
    alis.charmode = 1;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

static void cspicprint(void) {
    alis.charmode = 1;
    readexec_opername_swap();
    alis_putstring();
}

static void cvputprint(void) {
    alis.charmode = 2;
    readexec_opername_saveD7();
    putval(alis.varD7);
}

static void csputprint(void) {
    alis.charmode = 2;
    readexec_opername_swap();
    alis_putstring();
}

static void cfont(void) {
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

static void cpaper(void) {
    debug(EDebugWarning, " /* MISSING */");
}

// fade-out to black
static void ctoblack(void) {
    readexec_opername_saveD6();
    
    s16 duration = alis.varD6;
    ctoblackpal(duration);
}

static void cmovcolor(void) {
    debug(EDebugWarning, " /* MISSING */");
}

// fade-in to palette
static void ctopalet(void) {
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

static void cnumput(void) {
    readexec_opername();
    alis.depx = alis.varD7;
    readexec_opername();
    alis.depy = alis.varD7;
    readexec_opername();
    alis.depz = alis.varD7;
    readexec_opername();
    alis.numelem = alis.varD7;
}

static void cscheart(void) {
    readexec_opername();
    readexec_opername_saveD6();

    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scene_unknown0x0a(screen_id, alis.varD7);
        set_scene_unknown0x0c(screen_id, alis.varD6);
        set_scene_state(screen_id, get_scene_state(screen_id) | 0x80);
    }
}

static void cscpos(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scene_newx(screen_id, alis.varD7);
        set_scene_newy(screen_id, alis.varD6);
        set_scene_state(screen_id, get_scene_state(screen_id) | 0x80);
    }
}

static void cscsize(void) {
    readexec_opername();
    readexec_opername_saveD6();
    
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scene_width(screen_id, alis.varD7);
        set_scene_height(screen_id, alis.varD6);
        set_scene_state(screen_id, get_scene_state(screen_id) | 0x80);
    }
}

static void cschoriz(void) {
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != '\0')
    {
        readexec_opername();
        set_scene_unknown0x20(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scene_unknown0x21(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scene_unknown0x22(screen_id, (u8)alis.varD7);
        set_scene_state(screen_id, get_scene_state(screen_id) | 0x80);
    }
}

static void cscvertic(void) {
    s16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != '\0')
    {
        readexec_opername();
        set_scene_unknown0x23(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scene_unknown0x24(screen_id, (u8)alis.varD7);
        readexec_opername();
        set_scene_unknown0x25(screen_id, (u8)alis.varD7);
        set_scene_state(screen_id, get_scene_state(screen_id) | 0x80);
    }
}

static void cscreduce(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername();
    s16 creducing = alis.varD7;
    readexec_opername_saveD6();
    s8 clinkingA = alis.varD6;
    u16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        readexec_opername();
        u8 clinkingB = alis.varD7;

        set_scene_creducing(screen_id, creducing);
        set_scene_clinking(screen_id, (s16)(clinkingA - 1) << 8 | clinkingB);
        set_scene_state(screen_id, get_scene_state(screen_id) | 0x80);
    }
}

static void cscscale(void) {
    readexec_opername();
    s8 scale = alis.varD7;
    u16 screen_id = get_0x16_screen_id(alis.script->vram_org);
    if (screen_id != 0)
    {
        set_scene_credon_off(screen_id, scale);
        set_scene_state(screen_id, get_scene_state(screen_id) | 0x80);
    }
}

static void creducing(void) {
    readexec_opername();
    set_0x26_creducing(alis.script->vram_org, alis.varD7);
    readexec_opername();
    set_0x27_creducing(alis.script->vram_org, alis.varD7);
}

static void cscmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscdump(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cfindcla(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername();
    s16 vacc_offset = alis.varD7;
    s16 offset = 0;
    
    sAlisScript *script;
    
    s32 tabidx = 0;

    do
    {
        script = ENTSCR(offset);
        if ((char)vacc_offset == get_0x0c_vacc_offset(script->vram_org) && alis.script->vram_org != script->vram_org)
        {
            alis.matent[tabidx] = 0;
            alis.tablent[tabidx] = offset;
            tabidx ++;
            
            if (alis.fallent == 0)
            {
                break;
            }
        }
        
        offset = vread16(alis.atent + 4 + offset);
    }
    while (offset != 0);
    
    alis.tablent[tabidx] = -1;
    alis.fallent = 0;
    
    crstent();
}

static void cnearcla(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cviewcla(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cinstru(void) {
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
        u8 type = vread8(alis.script->data_org + addr);
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

static void cminstru(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cordspr(void) {
    readexec_opername();
    set_0x2b_cordspr(alis.script->vram_org, (u8)alis.varD7);
}

static void calign(void) {
    debug(EDebugWarning, " /* CHECK */");
    
    u8 bit;

    readexec_opername();

    if (((u8)alis.varD7 & 0x10) == 0)
    {
        bit = 4;
    }
    else if (((u8)alis.varD7 & 0x8) == 0)
    {
        bit = 3;
    }
    else if (((u8)alis.varD7 & 0x4) == 0)
    {
        bit = 2;
    }
    else if (((u8)alis.varD7 & 0x2) == 0)
    {
        bit = 1;
    }
    else
    {
        bit = 0;
    }

    set_0x2c_calign(alis.script->vram_org, bit);
    
    readexec_opername();
    set_0x2d_calign(alis.script->vram_org, alis.varD7);
}

static void cbackstar(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cstarring(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cengine(void) {
    readexec_opername();
    readexec_opername();
    readexec_opername();
    readexec_opername();
}

static void cautobase(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cquality(void) {
    readexec_opername();
    alis.vquality = alis.varD7;
}

static void chsprite(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cselpalet(void) {
    readexec_opername();
    alis.varD7 &= 0x3; // 4 palettes: 0...3
    thepalet = alis.varD7;
    defpalet = 1;
}

static void clinepalet(void) {
    readexec_opername();
    readexec_opername_saveD6();
}

static void cautomode(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cautofile(void) {
    readexec_opername_swap();
}

static void ccancel(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void ccancall(void) {
    alis.ferase = 1;
    
    u16 tmpidx = 0;
    u16 curidx = get_0x18_unknown(alis.script->vram_org);
    while (curidx)
    {
        killelem(&curidx, &tmpidx);
    }
    
    alis.ferase = 0;
}

static void ccancen(void) {
    alis.ferase = 1;
    cerasen();
}

static void cblast(void) {
    alis.ferase = 1;
    ckill();
}

static void cscback(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscrolpage(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cmatent(void) {
    int index = (int)((alis.ptrent - alis.tablent) / 2);
    alis.varD7 = alis.matent[index];
    cstore_continue();
}

static void cshrink(void) {
    
    // Delete bitmap and shift following data
    
    readexec_opername();
    readexec_opername_saveD6();
    
    alis.flagmain = 0;
    
    s16 width;

    u32 addr = adresdes(alis.varD7);
    u8 *data = alis.mem + alis.script->data_org + addr;
    if (*data == 0 || *data == 2)
    {
        width = (swap16(data + 2, alis.platform.is_little_endian) + 1) >> 1;
    }
    else if (*data == 0x1a || *data == 0x18)
    {
        width = swap16(data + 2, alis.platform.is_little_endian) + 1;
    }
    else if (*data == 0x16 || *data == 0x14 || *data == 0x1e || *data == 0x1c)
    {
        width = swap16(data + 2, alis.platform.is_little_endian) + 1;
    }
    else if (*data == 0x12 && *data == 0x10)
    {
        width = (swap16(data + 2, alis.platform.is_little_endian) + 1) >> 1;
    }
    else
    {
        return;
    }
    
    if (*(u16 *)(data + 4) != 0)
    {
        s16 height = swap16(data + 4, alis.platform.is_little_endian) + 1;
        s32 bits = width * height;
        
        u32 offset = get_0x14_script_org_offset(alis.script->vram_org);
        s32 l = vread32(offset + 0xe);
        s16 e = vread16(offset + l + 4);
        s32 a = vread32(offset + l) + l;

        for (s32 i = 0; i < e; i++, a += 4)
        {
            s32 t = vread32(offset + a);
            if (addr < a + t)
            {
                t -= bits;
                vwrite32(offset + a, t);
            }
        }
        
        vwrite32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0x6, vread32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0x6) - bits);
        vwrite32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0xc, vread32(get_0x14_script_org_offset(alis.script->vram_org) + l + 0xc) - bits);
        vwrite16(alis.script->data_org + addr + 2, 0xf);
        vwrite16(alis.script->data_org + addr + 4, 0);
        
        debug(EDebugInfo, " (NAME: %s, ID: 0x%x) ", alis.script->name, alis.script->header.id);
        shrinkprog(alis.script->data_org + addr + 6, bits, 0);
    }
}

static void cdefmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void csetmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cputmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

u8 svpalet[1024];
u8 svpalet2[1024];

extern u8 tpalet[1024];
extern u8 mpalet[1024];
extern float dpalet[1024];

static void csavepal(void) {
    debug(EDebugWarning, " /* CHECK */");
    readexec_opername();
    s16 mode = alis.varD7;
    if (mode < 0 && -3 < mode)
    {
        u8 *tgt = (mode != -1) ? svpalet2 : svpalet;
        memcpy(tgt, tpalet, 1024);
    }
}

static void csczoom(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void ctexmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void calloctab(void) {
    debug(EDebugWarning, " /* STUBBED */");
    s16 offset = (s8)script_read8();
    if (offset == 0)
    {
        offset = (s8)script_read8();
    }
    
//    u32 *tabptr = (u32 *)(alis.mem + alis.basemain + offset);
//    if (*(s8 *)(tabptr - 1) < 0)
//    {
//        *tabptr = io_malloc();
//    }
}

static void cfreetab(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscantab(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cneartab(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscsun(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cdarkpal(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscdark(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void caset(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void camov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscaset(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscamov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscfollow(void) {
    debug(EDebugWarning, " /* CHECK */");
    u32 screen = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
    readexec_opername();
    vwrite16(screen + 0x60, alis.varD7);
    readexec_opername();
    vwrite8(screen + 0x84, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x86, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x88, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x8a, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x8c, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x8e, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x90, alis.varD7);
}

static void cscview(void) {
    debug(EDebugWarning, " /* CHECK */");
    u32 screen = alis.basemain + get_0x16_screen_id(alis.script->vram_org);
    readexec_opername();
    vwrite16(screen + 0x64, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x66, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x68, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x6a, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x6c, alis.varD7);
    readexec_opername();
    vwrite16(screen + 0x6e, alis.varD7);
    vwrite8(screen, vread8(screen) | 0x80);
}

static void cfilm(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cwalkmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void catstmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cavtstmov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cavmov(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void caim(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cpointpix(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cchartmap(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void cscsky(void) {
    debug(EDebugWarning, " /* MISSING */");
}

static void czoom(void) {
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
static void cnul(void)      {
    debug(EDebugVerbose, " N/I ");
}
static void cesc1(void)     {
    debug(EDebugVerbose, " N/I ");
}
static void cesc2(void)     {
    debug(EDebugVerbose, " N/I ");
}
static void cesc3(void)     {
    debug(EDebugVerbose, " N/I ");
}
static void cclock(void)    {
    debug(EDebugVerbose, " N/I ");
}
static void cbreakpt(void)  {
    debug(EDebugVerbose, " N/I ");
}
static void cmul(void)      {
    debug(EDebugVerbose, " N/I ");
}
static void cdiv(void)      {
    debug(EDebugVerbose, " N/I ");
}
static void cjsrabs(void)   {
    debug(EDebugVerbose, " N/I ");
}
static void cjmpabs(void)   {
    debug(EDebugVerbose, " N/I ");
}
static void cjsrind16(void) {
    debug(EDebugVerbose, " N/I ");
}
static void cjsrind24(void) {
    debug(EDebugVerbose, " N/I ");
}
static void cjmpind16(void) {
    debug(EDebugVerbose, " N/I ");
}
static void cjmpind24(void) {
    debug(EDebugVerbose, " N/I ");
}


// ============================================================================
#pragma mark - Flow control - Subroutines
// ============================================================================
static void cret(void) {
    
    if (alis.fseq == 0 && alis.saversp <= alis.script->vacc_off)
    {
        cstop();
        return;
    }
    
    s32 offset = xpop32();
    if (alis.script->vacc_off == get_0x0c_vacc_offset(alis.script->vram_org))
    {
        set_0x0c_vacc_offset(alis.script->vram_org, 0);
    }
    
    if (alis.script->vacc_off >= -0x34)
    {
        debug(EDebugError, " VACC out of bounds!!! ");
    }
    
    alis.script->pc = alis.script->pc_org + offset;
}

static void cjsr(s32 offset) {

    // save return **OFFSET**, not ADDRESS
    // TODO: dans la vm originale on empile la 'vraie' adresse du PC en 32 bits
    // Là on est en 64 bits, donc j'empile l'offset
    // TODO: peut-on stocker une adresse de retour *virtuelle* ?
    // Sinon ça oblige à créer une pile virtuelle d'adresses
    //   dont la taille est platform-dependent
    xpush32((u32)(alis.script->pc - alis.script->pc_org));
    script_jump(offset);
}

static void cjsr8(void) {
    // read byte, extend sign
    s16 offset = (s8)script_read8();
    cjsr(offset);
}

static void cjsr16(void) {
    s16 offset = script_read16();
    cjsr(offset);
}

static void cjsr24(void) {
    s32 offset = script_read24();
    cjsr(offset);
}


// ============================================================================
#pragma mark - Flow control - Jump
// ============================================================================

static void cjmp8(void) {
    s16 offset = (s8)script_read8();
    script_jump(offset);
}

static void cjmp16(void) {
    s16 offset = script_read16();
    script_jump(offset);
}

static void cjmp24(void) {
    s32 offset = script_read24();
    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if zero
// ============================================================================

static void cbz8(void) {
    s16 offset = alis.varD7 ? 1 : (s8)script_read8();
    script_jump(offset);
}

static void cbz16(void) {
    s16 offset = alis.varD7 ? 2 : script_read16();
    script_jump(offset);
}

static void cbz24(void) {
    s32 offset = alis.varD7 ? 3 : script_read24();
    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if non-zero
// ============================================================================

static void cbnz8(void) {
    s16 offset = alis.varD7 == 0 ? 1 : (s8)script_read8();
    script_jump(offset);
}

static void cbnz16(void) {
    s16 offset = alis.varD7 == 0 ? 2 : script_read16();
    script_jump(offset);
}

static void cbnz24(void) {
    s32 offset = alis.varD7 == 0 ? 3 : script_read24();
    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if equal
// ============================================================================

static void cbeq8(void) {
    s16 offset = alis.varD7 == alis.varD6 ? 1 : (s8)script_read8();
    script_jump(offset);
}
static void cbeq16(void) {
    s16 offset = alis.varD7 == alis.varD6 ? 2 : script_read16();
    script_jump(offset);
}
static void cbeq24(void) {
    s32 offset = alis.varD7 == alis.varD6 ? 3 : script_read24();
    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Branch if not equal
// ============================================================================

static void cbne8(void) {
    s16 offset = alis.varD7 != alis.varD6 ? 1 : (s8)script_read8();
    script_jump(offset);
}
static void cbne16(void) {
    s16 offset = alis.varD7 != alis.varD6 ? 2 : script_read16();
    script_jump(offset);
}
static void cbne24(void) {
    s32 offset = alis.varD7 != alis.varD6 ? 3 : script_read24();
    script_jump(offset);
}


// ============================================================================
#pragma mark - Flow control - Start
// ============================================================================
static void cstart(s32 offset) {
    if (alis.fseq == 0)
    {
        if (get_0x0c_vacc_offset(alis.script->vram_org) == 0)
        {
            s16 vacc_offset = get_0x0a_vacc_offset(alis.script->vram_org) - 4;
            set_0x0c_vacc_offset(alis.script->vram_org, vacc_offset);
            set_0x0a_vacc_offset(alis.script->vram_org, vacc_offset);
            vwrite32(alis.script->vram_org + vacc_offset, (u32)(get_0x08_script_ret_offset(alis.script->vram_org) - alis.script->pc_org));
            set_0x08_script_ret_offset(alis.script->vram_org, offset + alis.script->pc);
        }
        else
        {
            set_0x0a_vacc_offset(alis.script->vram_org, get_0x0c_vacc_offset(alis.script->vram_org));
            set_0x08_script_ret_offset(alis.script->vram_org, offset + alis.script->pc);
        }
        
        set_0x04_cstart_csleep(alis.script->vram_org, 1);
        set_0x01_cstart(alis.script->vram_org, 1);
    }
    else
    {
        if (get_0x0c_vacc_offset(alis.script->vram_org) == 0)
        {
            xpush32((u32)(alis.script->pc - alis.script->pc_org));
            set_0x0c_vacc_offset(alis.script->vram_org, alis.script->vacc_off);
        }
        else
        {
            alis.script->vacc_off = get_0x0c_vacc_offset(alis.script->vram_org);
        }

        alis.script->pc += offset;
    }
}

static void cstart8(void) {
    // read byte, extend sign to word, then to long
    cstart((s8)script_read8());
}

static void cstart16(void) {
    cstart((s16)script_read16());
}

static void cstart24(void) {
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
    DECL_OPCODE(0xe2, cshrink, "Delete bitmap and shift following data"),
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

void killent(u16 killent, u16 testent)
{
    // script
    
    sAlisScript *killscript = ENTSCR(killent);
    sAlisScript *prevscript = alis.script;
    
    // NOTE: to accomodate cerasall()
    alis.script = killscript;
    
    u32 script_vram = killscript->vram_org;

    debug(EDebugInfo, "\n killent: [%.2x, %.6x][%.6x] \n", killent, script_vram, prevscript->vram_org);
    debug(EDebugInfo, " killent: [%.2x, %.6x] \n", xread16(alis.atent + 4 + killent), xread32(alis.atent + killent));

    u16 tent = 0;
    u16 pent = 0;

    do
    {
        sAlisScript *s = ENTSCR(tent);
        tent = xread16(alis.atent + 4 + tent);

        u32 datasize = sizeof(sScriptContext) + s->header.w_unknown5 + s->header.w_unknown7;
        s32 vramsize = s->header.vram_alloc_sz;
        s32 shrinkby = datasize + vramsize;

        debug(EDebugInfo, "%c[%s (%.2x), %.2x, %.6x, %.6x] \n", pent == killent ? '*' : ' ', s->name, get_0x10_script_id(s->vram_org), pent, s->vram_org, shrinkby);
        pent = tent;
    }
    while (tent);

    if (script_vram == 0)
    {
        alis.ferase = 0;
        return;
    }
    
    cerasall();

    u32 datasize = sizeof(sScriptContext) + killscript->header.w_unknown5 + killscript->header.w_unknown7;
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
    
    u16 curent = 0;
    u16 prevent = 0;
    u16 nextent;
    
    s32 vram_org = killscript->vram_org;

    do
    {
        nextent = xread16(alis.atent + 4 + prevent);
        if (killent == nextent)
        {
            curent = prevent;
        }

        prevent = nextent;
        
        sAlisScript *curscript = ENTSCR(nextent);
        if (vram_org <= curscript->vram_org)
        {
            curscript->vram_org -= shrinkby;
            xsub32(alis.atent + nextent, shrinkby);
        }
    }
    while (nextent);

    xwrite16(alis.atent + curent + 4, xread16(alis.atent + 4 + killent));
    xwrite16(alis.atent + killent + 4, alis.dernent);
    xwrite32(alis.atent + killent, 0);

    killscript->vram_org = 0;

    alis.dernent = killent;
    alis.nbent --;

    // if we deleted currently running script, set previous script and restart loop
    if (testent == killent)
    {
        alis.varD5 = curent;
        alis.restart_loop = 1;
        cstop();
    }
    else
    {
        alis.script = prevscript;
    }
}

void shrinkprog(s32 start, s32 length, u16 script_id)
{
    u8 *target = alis.mem;
    u8 *source = alis.mem + length;
    
    // copy scripts to freed space
    for (s32 i = start; i < alis.finprog - length; i++)
    {
        target[i] = source[i];
    }
    
    debug(EDebugInfo, "\nFreeing range %.6x - %.6x", start, start + length);

    alis.finprog -= length;
    
    if (script_id != 0)
    {
        s8 idx = -1;
        for (s32 i = 0; i < alis.nbprog; i++)
        {
            sAlisScript *scr = alis.progs[i];
            if (scr->header.id == script_id)
            {
                idx = i;
                debug(EDebugInfo, "\nRemoved prog: %s at: %.6x ", scr->name, scr->data_org);
            }
            else
            {
                debug(EDebugInfo, "\nSkipped prog: %s at: %.6x ", scr->name, scr->data_org);
            }

            if (idx > 0)
            {
                alis.progs[i] = i < alis.maxprog ? alis.progs[i + 1] : 0;
            }
        }

        u8 found = 0;
        for (s32 i = 0; i < alis.nbprog; i++)
        {
            s32 location = alis.atprog_ptr[i];
            if (start == location)
            {
                found = 1;
            }
            
            if (found)
            {
                alis.atprog_ptr[i] = i < alis.maxprog ? alis.atprog_ptr[i + 1] : 0;
            }
        }

        alis.dernprog -= 4;
        alis.nbprog --;
    }

    debug(EDebugInfo, "\nShifting range %.6x - %.6x", alis.atprog, alis.dernprog);

    for (int i = 0; i < alis.nbprog; i++)
    {
        sAlisScript *script = alis.progs[i];
        debug(EDebugInfo, "\nChecking prog: %s at: %.6x ", script->name, script->data_org);
            
        if (start <= script->data_org)
        {
            script->data_org -= length;
            alis.atprog_ptr[i] -= length;
            debug(EDebugInfo, "shrinked to: %.6x", script->data_org);
        }
        else
        {
            debug(EDebugInfo, "OK");
        }
    }

    debug(EDebugInfo, "\n");

    if (script_id != 0)
    {
        u32 entidx = 0;
        u32 prevent = 0;
        
        while ((entidx = xread16(alis.atent + 4 + entidx)))
        {
            sAlisScript *script = ENTSCR(entidx);
            debug(EDebugInfo, "\nChecking script: %s at: %.6x ", script->name, script->data_org);

            if (script_id == get_0x10_script_id(script->vram_org))
            {
                killent(entidx, alis.varD5);
                entidx = prevent; //alis.varD5;
                debug(EDebugInfo, "removed");
            }
            else if (start <= get_0x14_script_org_offset(script->vram_org))
            {
                script->pc -= length;
                script->pc_org -= length;
                script->data_org -= length;
                set_0x14_script_org_offset(script->vram_org, get_0x14_script_org_offset(script->vram_org) - length);
                set_0x08_script_ret_offset(script->vram_org, get_0x08_script_ret_offset(script->vram_org) - length);
                
                u32 org_offset = get_0x14_script_org_offset(script->vram_org);
                script->vacc_off = -0x34 - xread16(org_offset + 0x16);
                debug(EDebugInfo, " [va %.4x]", (s16)alis.script->vacc_off);

                while (get_0x0a_vacc_offset(script->vram_org) < script->vacc_off)
                {
                    script->vacc_off -= 4;
                    xsub32(script->vram_org + script->vacc_off, length);
                    debug(EDebugInfo, " [%.8x => va %.4x + %.6x (%.6x)]", xread32(script->vram_org + script->vacc_off), (s16)script->vacc_off, script->vram_org, script->vacc_off + script->vram_org);
                }

                debug(EDebugInfo, "shrinked to: %.6x", script->data_org);
            }
            else
            {
                debug(EDebugInfo, "OK");
            }
            
            prevent = entidx;
        }
    }
    
    debug(EDebugInfo, "\n");

    // move addresses to sprite data to match actual new locations
    
    for (s16 scidx = ptscreen; scidx != 0; scidx = get_scene_to_next(scidx))
    {
        s16 spridx = get_scene_screen_id(scidx);
        if (spridx != 0)
        {
            while ((spridx = SPRITE_VAR(spridx)->link) != 0)
            {
                SpriteVariables *sprite = SPRITE_VAR(spridx);
                if (start < sprite->newad)
                    sprite->newad -= length;

                if (start < sprite->data)
                    sprite->data -= length;
            }
        }
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
//
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
