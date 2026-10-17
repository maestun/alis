
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "alis.h"
#include "alis_private.h"

extern u8 *disasm_buf;
extern int disasm_size;
extern int disasm_pc;
extern FILE *disasm_fout;

// Provide dummy alis wrapper since our get8/etc depend on it to be simple
extern sAlisVM alis;

void get8() { if (disasm_pc < disasm_size) { fprintf(disasm_fout, " %02X", disasm_buf[disasm_pc++]); } }
void get16() { get8(); get8(); }
void get24() { get8(); get8(); get8(); }
void get32() { get16(); get16(); }

void readexec_opcodes();
void readexec_opernames();
void readexec_storenames();
void readexec_addnames();
void readexec_esc1names();
void readexec_esc2names();
void readexec_esc3names();
void disasm_cnul() {
}

void disasm_cesc1() {
    readexec_esc1names();
}

void disasm_cesc2() {
    readexec_esc2names();
}

void disasm_cesc3() {
    readexec_esc3names();
}

void disasm_cbreakpt() {
}

void disasm_cjsr8() {
    get8();
}

void disasm_cjsr16() {
    get16();
}

void disasm_cjsr24() {
    get24();
}

void disasm_cjmp8() {
    get8();
    // WARNING: dynamic offset offset
}

void disasm_cjmp16() {
    get16();
    // WARNING: dynamic offset offset
}

void disasm_cjmp24() {
    get24();
    // WARNING: dynamic offset offset
}

void disasm_cjsrabs() {
}

void disasm_cjmpabs() {
}

void disasm_cjsrind16() {
}

void disasm_cjsrind24() {
}

void disasm_cjmpind16() {
}

void disasm_cjmpind24() {
}

void disasm_cret() {
}

void disasm_cbz8() {
    get8();
    // WARNING: dynamic offset offset
}

void disasm_cbz16() {
    get16();
    // WARNING: dynamic offset offset
}

void disasm_cbz24() {
    get24();
    // WARNING: dynamic offset offset
}

void disasm_cbnz8() {
    get8();
    // WARNING: dynamic offset offset
}

void disasm_cbnz16() {
    get16();
    // WARNING: dynamic offset offset
}

void disasm_cbnz24() {
    get24();
    // WARNING: dynamic offset offset
}

void disasm_cbeq8() {
    get8();
    // WARNING: dynamic offset offset
}

void disasm_cbeq16() {
    get16();
    // WARNING: dynamic offset offset
}

void disasm_cbeq24() {
    get24();
    // WARNING: dynamic offset offset
}

void disasm_cbne8() {
    get8();
    // WARNING: dynamic offset offset
}

void disasm_cbne16() {
    get16();
    // WARNING: dynamic offset offset
}

void disasm_cbne24() {
    get24();
    // WARNING: dynamic offset offset
}

void disasm_cstore() {
    readexec_opernames();
}

void disasm_ceval() {
    readexec_opernames();
}

void disasm_cadd() {
    readexec_opernames();
    readexec_addnames();
}

void disasm_csub() {
    readexec_opernames();
    readexec_addnames();
}

void disasm_cmul() {
}

void disasm_cdiv() {
}

void disasm_cvprint() {
    readexec_opernames();
}

void disasm_csprinti() {
    get8();
}

void disasm_csprinta() {
    readexec_opernames();
}

void disasm_clocate() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_ctab() {
    readexec_opernames();
}

void disasm_cdim() {
    get16();
    get8();
    get8();
    get16();
    get32();
    get32();
    get32();
    get16();
}

void disasm_crandom() {
    readexec_opernames();
}

void disasm_cloop8() {
    get8();
}

void disasm_cloop16() {
    get16();
}

void disasm_cloop24() {
    get24();
}

void disasm_cswitch1() {
    readexec_opernames();
    get8();
    get16();
    get16();
    for(int i=0; i<2; i++) get8();
}

void disasm_cswitch2() {
    readexec_opernames();
    get8();
    get16();
    // WARNING: dynamic offset alis
    get16();
}

void disasm_cstart8() {
    get8();
}

void disasm_cstart16() {
    get16();
}

void disasm_cstart24() {
    get24();
}

void disasm_cleave() {
    get32();
}

void disasm_cprotect() {
    readexec_opernames();
}

void disasm_casleep() {
    readexec_opernames();
    get32();
}

void disasm_cclock() {
    readexec_opernames();
}

void disasm_cnul() {
}

void disasm_cscmov() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscset() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cclipping() {
}

void disasm_cswitching() {
}

void disasm_cwlive() {
}

void disasm_cunload() {
    get16();
}

void disasm_cwakeup() {
    readexec_opernames();
    get32();
}

void disasm_csleep() {
}

void disasm_clive() {
}

void disasm_ckill() {
    readexec_opernames();
}

void disasm_cstop() {
}

void disasm_cstopret() {
}

void disasm_cexit() {
}

void disasm_cload() {
    get16();
    readexec_opernames();
}

void disasm_cdefsc() {
    get16();
    get8();
    get8();
}

void disasm_cscreen() {
    get16();
}

void disasm_cput() {
    readexec_opernames();
}

void disasm_cputnat() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cerase() {
}

void disasm_cerasen() {
    readexec_opernames();
}

void disasm_cset() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cmov() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_copensc() {
    get16();
    get8();
}

void disasm_cclosesc() {
    get16();
}

void disasm_cerasall() {
}

void disasm_cforme() {
    readexec_opernames();
}

void disasm_cdelforme() {
}

void disasm_ctstmov() {
    readexec_opernames();
    get16();
    readexec_opernames();
    get16();
    readexec_opernames();
    get16();
    readexec_opernames();
}

void disasm_ctstset() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cftstmov() {
    readexec_opernames();
    get16();
    readexec_opernames();
    get16();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cftstset() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_csuccent() {
}

void disasm_cpredent() {
}

void disasm_cnearent() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cneartyp() {
    readexec_opernames();
    get16();
    readexec_opernames();
}

void disasm_cnearmat() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cviewent() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cviewtyp() {
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
}

void disasm_cviewmat() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_corient() {
    readexec_opernames();
    get8();
    get8();
    get8();
}

void disasm_crstent() {
}

void disasm_csend() {
    get8();
    readexec_opernames();
    get32();
    get16();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscanon() {
}

void disasm_cscanoff() {
}

void disasm_cinteron() {
}

void disasm_cinteroff() {
}

void disasm_cscanclr() {
}

void disasm_callentity() {
}

void disasm_cpalette() {
    readexec_opernames();
    get32();
    get32();
}

void disasm_cdefcolor() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_ctiming() {
    readexec_opernames();
}

void disasm_czap() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cexplode() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cding() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cnoise() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cinitab() {
    get16();
    get16();
    get8();
}

void disasm_cfopen() {
    for(int i=0; i<1; i++) get8();
    readexec_opernames();
    readexec_opernames();
    get16();
}

void disasm_cfclose() {
}

void disasm_cfcreat() {
    get16();
}

void disasm_cfdel() {
}

void disasm_cfreadv() {
}

void disasm_cfwritev() {
    readexec_opernames();
}

void disasm_cfwritei() {
}

void disasm_cfreadb() {
    get16();
    get16();
    get8();
    get32();
    get32();
    get32();
    get16();
}

void disasm_cfwriteb() {
    get16();
    get16();
    get32();
    get8();
    get32();
    get32();
    get16();
}

void disasm_cplot() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cdraw() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cbox() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cboxf() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cink() {
    readexec_opernames();
}

void disasm_cpset() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cpmove() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cpmode() {
    readexec_opernames();
}

void disasm_cpicture() {
    readexec_opernames();
}

void disasm_cxyscroll() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_clinking() {
    readexec_opernames();
}

void disasm_cmouson() {
}

void disasm_cmousoff() {
}

void disasm_cmouse() {
}

void disasm_cdefmouse() {
    readexec_opernames();
    get32();
}

void disasm_csetmouse() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cdefvect() {
    get8();
    // WARNING: dynamic offset offset
}

void disasm_csetvect() {
    get16();
}

void disasm_cnul() {
}

void disasm_capproach() {
    readexec_opernames();
    get8();
}

void disasm_cescape() {
    readexec_opernames();
    get8();
}

void disasm_cvtstmov() {
    get8();
    get8();
    get8();
    readexec_opernames();
}

void disasm_cvftstmov() {
    get8();
    get8();
    get8();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cvmov() {
    get8();
    get8();
    get8();
}

void disasm_cdefworld() {
    get16();
    get8();
}

void disasm_cworld() {
    get8();
    get8();
}

void disasm_cfindmat() {
    readexec_opernames();
}

void disasm_cfindtyp() {
    get16();
    readexec_opernames();
    get32();
    get16();
}

void disasm_cmusic() {
}

void disasm_cdelmusic() {
    readexec_opernames();
}

void disasm_ccadence() {
    readexec_opernames();
}

void disasm_csetvolum() {
    readexec_opernames();
}

void disasm_cxinv() {
}

void disasm_cxinvon() {
}

void disasm_cxinvoff() {
}

void disasm_clistent() {
    get16();
    get16();
}

void disasm_csound() {
}

void disasm_cmsound() {
}

void disasm_credon() {
    readexec_opernames();
}

void disasm_credoff() {
    readexec_opernames();
}

void disasm_cdelsound() {
}

void disasm_cwmov() {
}

void disasm_cwtstmov() {
}

void disasm_cwftstmov() {
}

void disasm_ctstform() {
    get16();
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cxput() {
    readexec_opernames();
}

void disasm_cxputat() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cmput() {
    readexec_opernames();
}

void disasm_cmputat() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cmxput() {
    readexec_opernames();
}

void disasm_cmxputat() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cmmusic() {
}

void disasm_cmforme() {
    readexec_opernames();
}

void disasm_csettime() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cgettime() {
}

void disasm_cvinput() {
}

void disasm_csinput() {
}

void disasm_casleepfar() {
    readexec_opernames();
    get32();
    get16();
    get16();
    get16();
    get16();
}

void disasm_casleepon() {
}

void disasm_casleepoff() {
}

void disasm_crunfilm() {
    readexec_opernames();
}

void disasm_cvpicprint() {
    readexec_opernames();
}

void disasm_cspicprint() {
    readexec_opernames();
}

void disasm_cvputprint() {
    readexec_opernames();
}

void disasm_csputprint() {
    readexec_opernames();
}

void disasm_cfont() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cpaper() {
    readexec_opernames();
}

void disasm_ctoblack() {
    readexec_opernames();
}

void disasm_cmovcolor() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_ctopalet() {
    readexec_opernames();
    readexec_opernames();
    get32();
    get32();
}

void disasm_cnumput() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscheart() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscpos() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscsize() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cschoriz() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscvertic() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscreduce() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscscale() {
    readexec_opernames();
}

void disasm_creducing() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscmap() {
    readexec_opernames();
    get16();
    get16();
}

void disasm_cscdump() {
}

void disasm_cfindcla() {
    readexec_opernames();
    get32();
    get8();
    get16();
}

void disasm_cnearcla() {
}

void disasm_cviewcla() {
}

void disasm_cinstru() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
}

void disasm_cminstru() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
}

void disasm_cordspr() {
    readexec_opernames();
}

void disasm_calign() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cbackstar() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
    get8();
    get16();
}

void disasm_cstarring() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cengine() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cautobase() {
}

void disasm_cquality() {
    readexec_opernames();
}

void disasm_chsprite() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cselpalet() {
    readexec_opernames();
}

void disasm_clinepalet() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cautomode() {
}

void disasm_cautofile() {
    readexec_opernames();
}

void disasm_ccancel() {
}

void disasm_ccancall() {
}

void disasm_ccancen() {
}

void disasm_cblast() {
}

void disasm_cscback() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
    get16();
    get16();
    get8();
}

void disasm_cscrolpage() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cmatent() {
}

void disasm_cshrink() {
    readexec_opernames();
    readexec_opernames();
    get32();
    get16();
    get16();
    get16();
    get16();
    get16();
    get32();
    get16();
    get32();
    get32();
    get32();
    get32();
}

void disasm_cdefmap() {
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
    get32();
    get32();
    get16();
    get16();
    get16();
    get8();
}

void disasm_csetmap() {
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
    get16();
}

void disasm_cputmap() {
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    get16();
    get16();
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    get16();
    get16();
    get16();
    get16();
}

void disasm_csavepal() {
    readexec_opernames();
}

void disasm_csczoom() {
}

void disasm_ctexmap() {
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
    get32();
    get32();
    get8();
    get16();
    get16();
    get8();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
    get32();
    get32();
    get8();
    get16();
    get16();
    get8();
}

void disasm_calloctab() {
    get16();
    get16();
    get8();
    get32();
    get32();
}

void disasm_cfreetab() {
}

void disasm_cscantab() {
}

void disasm_cneartab() {
}

void disasm_cscsun() {
}

void disasm_cdarkpal() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscdark() {
    readexec_opernames();
    readexec_opernames();
    get32();
    readexec_opernames();
    get8();
    get16();
    get16();
    get16();
    get32();
}

void disasm_caset() {
}

void disasm_camov() {
}

void disasm_cscaset() {
}

void disasm_cscamov() {
}

void disasm_cscfollow() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cscview() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get8();
}

void disasm_cfilm() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cwalkmap() {
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_catstmap() {
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    get16();
    get32();
}

void disasm_cavtstmov() {
    get8();
    get8();
    get8();
    get16();
    get16();
    get32();
    get32();
    get32();
    get32();
    get32();
    get32();
    readexec_opernames();
}

void disasm_cavmov() {
    get8();
    get8();
    get8();
    get16();
    get16();
}

void disasm_caim() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get16();
    get16();
    get16();
    get16();
    readexec_opernames();
    get16();
}

void disasm_cpointpix() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_cchartmap() {
    get16();
    get16();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get32();
    get16();
    get16();
    get16();
    get16();
    get16();
    get16();
    get8();
    get32();
    get16();
    get16();
    get16();
    readexec_opernames();
    get16();
    get8();
    get32();
    get32();
    get16();
    get16();
}

void disasm_cscsky() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    get32();
    get32();
    get8();
    get16();
    get32();
    get16();
    get8();
}

void disasm_czoom() {
    readexec_opernames();
    readexec_opernames();
}

void disasm_oimmb() {
    get8();
}

void disasm_oimmw() {
    get16();
}

void disasm_oimmp() {
    get16();
    get16();
}

void disasm_olocb() {
    get16();
    get8();
}

void disasm_olocw() {
    get16();
    get16();
}

void disasm_olocp() {
    get16();
    get16();
}

void disasm_oloctp() {
    get16();
}

void disasm_oloctc() {
    get16();
    get8();
}

void disasm_olocti() {
    get16();
    get16();
}

void disasm_odirb() {
    get8();
    get8();
}

void disasm_odirw() {
    get8();
    get16();
}

void disasm_odirp() {
    get8();
    get16();
}

void disasm_odirtp() {
    get8();
}

void disasm_odirtc() {
    get8();
    get8();
}

void disasm_odirti() {
    get8();
    get16();
}

void disasm_omainb() {
    get16();
    get8();
}

void disasm_omainw() {
    get16();
    get16();
}

void disasm_omainp() {
    get16();
    get16();
}

void disasm_omaintp() {
    get16();
}

void disasm_omaintc() {
    get16();
    get8();
}

void disasm_omainti() {
    get16();
    get16();
}

void disasm_ohimb() {
    get16();
    get32();
    get16();
    get8();
}

void disasm_ohimw() {
    get16();
    get32();
    get16();
    get16();
}

void disasm_ohimp() {
    get16();
    get32();
    get16();
    get16();
}

void disasm_ohimtp() {
}

void disasm_ohimtc() {
    get16();
    get16();
    get8();
}

void disasm_ohimti() {
    get16();
    get16();
    get16();
}

void disasm_opile() {
}

void disasm_oeval() {
    readexec_opernames();
}

void disasm_ofin() {
}

void disasm_pnul() {
}

void disasm_pnul() {
}

void disasm_opushacc() {
}

void disasm_oand() {
    readexec_opernames();
}

void disasm_oor() {
    readexec_opernames();
}

void disasm_oxor() {
    readexec_opernames();
}

void disasm_oeqv() {
    readexec_opernames();
}

void disasm_oegal() {
    readexec_opernames();
}

void disasm_odiff() {
    readexec_opernames();
}

void disasm_oinfeg() {
    readexec_opernames();
}

void disasm_osupeg() {
    readexec_opernames();
}

void disasm_oinf() {
    readexec_opernames();
}

void disasm_osup() {
    readexec_opernames();
}

void disasm_oadd() {
    readexec_opernames();
}

void disasm_osub() {
    readexec_opernames();
}

void disasm_omod() {
    readexec_opernames();
}

void disasm_odiv() {
    readexec_opernames();
}

void disasm_omul() {
    readexec_opernames();
}

void disasm_oneg() {
}

void disasm_oabs() {
}

void disasm_ornd() {
}

void disasm_osgn() {
}

void disasm_onot() {
}

void disasm_oinkey() {
}

void disasm_okeyon() {
}

void disasm_ojoy() {
}

void disasm_oprnd() {
}

void disasm_oscan() {
    get16();
    get16();
}

void disasm_oshiftkey() {
}

void disasm_ofree() {
}

void disasm_omodel() {
}

void disasm_ogetkey() {
}

void disasm_oleft() {
}

void disasm_oright() {
}

void disasm_omid() {
}

void disasm_olen() {
}

void disasm_oasc() {
}

void disasm_ostr() {
}

void disasm_osadd() {
    readexec_opernames();
}

void disasm_osegal() {
    readexec_opernames();
}

void disasm_osdiff() {
    readexec_opernames();
}

void disasm_osinfeg() {
    readexec_opernames();
}

void disasm_ossupeg() {
    readexec_opernames();
}

void disasm_osinf() {
    readexec_opernames();
}

void disasm_ossup() {
    readexec_opernames();
}

void disasm_ospushacc() {
}

void disasm_ospile() {
}

void disasm_oval() {
}

void disasm_oexistf() {
}

void disasm_ochr() {
}

void disasm_ochange() {
}

void disasm_ocountry() {
}

void disasm_omip() {
}

void disasm_ojoykey() {
}

void disasm_oconfig() {
}

void disasm_pnul() {
}

void disasm_pnul() {
}

void disasm_pnul() {
}

void disasm_slocb() {
    get16();
}

void disasm_slocw() {
    get16();
}

void disasm_slocp() {
    get16();
}

void disasm_sloctp() {
    get16();
}

void disasm_sloctc() {
    get16();
}

void disasm_slocti() {
    get16();
}

void disasm_sdirb() {
    get8();
}

void disasm_sdirw() {
    get8();
}

void disasm_sdirp() {
    get8();
}

void disasm_sdirtp() {
    get8();
}

void disasm_sdirtc() {
    get8();
}

void disasm_sdirti() {
    get8();
}

void disasm_smainb() {
    get16();
}

void disasm_smainw() {
    get16();
}

void disasm_smainp() {
    get16();
}

void disasm_smaintp() {
    get16();
}

void disasm_smaintc() {
    get16();
}

void disasm_smainti() {
    get16();
}

void disasm_shimb() {
    get16();
    get16();
    get32();
    get16();
}

void disasm_shimw() {
    get16();
    get16();
    get32();
    get16();
}

void disasm_shimp() {
}

void disasm_shimtp() {
}

void disasm_shimtc() {
}

void disasm_shimti() {
}

void disasm_spile() {
}

void disasm_seval() {
    readexec_storenames();
}

void disasm_ofin() {
}

void disasm_pnul() {
}

void disasm_pnul() {
}

void disasm_pnul() {
}

void disasm_alocb() {
    get16();
    get8();
}

void disasm_alocw() {
    get16();
    get16();
}

void disasm_alocp() {
    get16();
}

void disasm_aloctp() {
    get16();
}

void disasm_aloctc() {
    get16();
}

void disasm_alocti() {
    get16();
}

void disasm_adirb() {
    get8();
    get8();
}

void disasm_adirw() {
    get8();
    get16();
}

void disasm_adirp() {
    get8();
}

void disasm_adirtp() {
    get8();
}

void disasm_adirtc() {
    get8();
}

void disasm_adirti() {
    get8();
}

void disasm_amainb() {
    get16();
}

void disasm_amainw() {
    get16();
}

void disasm_amainp() {
    get16();
}

void disasm_amaintp() {
    get16();
}

void disasm_amaintc() {
    get16();
}

void disasm_amainti() {
    get16();
}

void disasm_ahimb() {
    get16();
    get16();
    get8();
}

void disasm_ahimw() {
    get16();
    get16();
}

void disasm_ahimp() {
}

void disasm_ahimtp() {
    get16();
    get16();
    get16();
}

void disasm_ahimtc() {
}

void disasm_ahimti() {
}

void disasm_spile() {
}

void disasm_aeval() {
    readexec_addnames();
}

void disasm_ofin() {
}

void disasm_cnul() {
}

void disasm_csoundon() {
}

void disasm_csoundoff() {
}

void disasm_cmusicon() {
}

void disasm_cmusicoff() {
}

void disasm_cdelfilm() {
}

void disasm_copenfilm() {
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
    readexec_opernames();
}

void disasm_cnul() {
}

void disasm_cnul() {
}

void readexec_opcodes() {
    if (disasm_pc >= disasm_size) return;
    u8 op = disasm_buf[disasm_pc++];
    fprintf(disasm_fout, " %02X ["opcodes":%02X]", op, op);
    switch(op) {
        case 0x00: disasm_cnul(); break;
        case 0x01: disasm_cesc1(); break;
        case 0x02: disasm_cesc2(); break;
        case 0x03: disasm_cesc3(); break;
        case 0x04: disasm_cbreakpt(); break;
        case 0x05: disasm_cjsr8(); break;
        case 0x06: disasm_cjsr16(); break;
        case 0x07: disasm_cjsr24(); break;
        case 0x08: disasm_cjmp8(); break;
        case 0x09: disasm_cjmp16(); break;
        case 0x0a: disasm_cjmp24(); break;
        case 0x0b: disasm_cjsrabs(); break;
        case 0x0c: disasm_cjmpabs(); break;
        case 0x0d: disasm_cjsrind16(); break;
        case 0x0e: disasm_cjsrind24(); break;
        case 0x0f: disasm_cjmpind16(); break;
        case 0x10: disasm_cjmpind24(); break;
        case 0x11: disasm_cret(); break;
        case 0x12: disasm_cbz8(); break;
        case 0x13: disasm_cbz16(); break;
        case 0x14: disasm_cbz24(); break;
        case 0x15: disasm_cbnz8(); break;
        case 0x16: disasm_cbnz16(); break;
        case 0x17: disasm_cbnz24(); break;
        case 0x18: disasm_cbeq8(); break;
        case 0x19: disasm_cbeq16(); break;
        case 0x1a: disasm_cbeq24(); break;
        case 0x1b: disasm_cbne8(); break;
        case 0x1c: disasm_cbne16(); break;
        case 0x1d: disasm_cbne24(); break;
        case 0x1e: disasm_cstore(); break;
        case 0x1f: disasm_ceval(); break;
        case 0x20: disasm_cadd(); break;
        case 0x21: disasm_csub(); break;
        case 0x22: disasm_cmul(); break;
        case 0x23: disasm_cdiv(); break;
        case 0x24: disasm_cvprint(); break;
        case 0x25: disasm_csprinti(); break;
        case 0x26: disasm_csprinta(); break;
        case 0x27: disasm_clocate(); break;
        case 0x28: disasm_ctab(); break;
        case 0x29: disasm_cdim(); break;
        case 0x2a: disasm_crandom(); break;
        case 0x2b: disasm_cloop8(); break;
        case 0x2c: disasm_cloop16(); break;
        case 0x2d: disasm_cloop24(); break;
        case 0x2e: disasm_cswitch1(); break;
        case 0x2f: disasm_cswitch2(); break;
        case 0x30: disasm_cstart8(); break;
        case 0x31: disasm_cstart16(); break;
        case 0x32: disasm_cstart24(); break;
        case 0x33: disasm_cleave(); break;
        case 0x34: disasm_cprotect(); break;
        case 0x35: disasm_casleep(); break;
        case 0x36: disasm_cclock(); break;
        case 0x37: disasm_cnul(); break;
        case 0x38: disasm_cscmov(); break;
        case 0x39: disasm_cscset(); break;
        case 0x3a: disasm_cclipping(); break;
        case 0x3b: disasm_cswitching(); break;
        case 0x3c: disasm_cwlive(); break;
        case 0x3d: disasm_cunload(); break;
        case 0x3e: disasm_cwakeup(); break;
        case 0x3f: disasm_csleep(); break;
        case 0x40: disasm_clive(); break;
        case 0x41: disasm_ckill(); break;
        case 0x42: disasm_cstop(); break;
        case 0x43: disasm_cstopret(); break;
        case 0x44: disasm_cexit(); break;
        case 0x45: disasm_cload(); break;
        case 0x46: disasm_cdefsc(); break;
        case 0x47: disasm_cscreen(); break;
        case 0x48: disasm_cput(); break;
        case 0x49: disasm_cputnat(); break;
        case 0x4a: disasm_cerase(); break;
        case 0x4b: disasm_cerasen(); break;
        case 0x4c: disasm_cset(); break;
        case 0x4d: disasm_cmov(); break;
        case 0x4e: disasm_copensc(); break;
        case 0x4f: disasm_cclosesc(); break;
        case 0x50: disasm_cerasall(); break;
        case 0x51: disasm_cforme(); break;
        case 0x52: disasm_cdelforme(); break;
        case 0x53: disasm_ctstmov(); break;
        case 0x54: disasm_ctstset(); break;
        case 0x55: disasm_cftstmov(); break;
        case 0x56: disasm_cftstset(); break;
        case 0x57: disasm_csuccent(); break;
        case 0x58: disasm_cpredent(); break;
        case 0x59: disasm_cnearent(); break;
        case 0x5a: disasm_cneartyp(); break;
        case 0x5b: disasm_cnearmat(); break;
        case 0x5c: disasm_cviewent(); break;
        case 0x5d: disasm_cviewtyp(); break;
        case 0x5e: disasm_cviewmat(); break;
        case 0x5f: disasm_corient(); break;
        case 0x60: disasm_crstent(); break;
        case 0x61: disasm_csend(); break;
        case 0x62: disasm_cscanon(); break;
        case 0x63: disasm_cscanoff(); break;
        case 0x64: disasm_cinteron(); break;
        case 0x65: disasm_cinteroff(); break;
        case 0x66: disasm_cscanclr(); break;
        case 0x67: disasm_callentity(); break;
        case 0x68: disasm_cpalette(); break;
        case 0x69: disasm_cdefcolor(); break;
        case 0x6a: disasm_ctiming(); break;
        case 0x6b: disasm_czap(); break;
        case 0x6c: disasm_cexplode(); break;
        case 0x6d: disasm_cding(); break;
        case 0x6e: disasm_cnoise(); break;
        case 0x6f: disasm_cinitab(); break;
        case 0x70: disasm_cfopen(); break;
        case 0x71: disasm_cfclose(); break;
        case 0x72: disasm_cfcreat(); break;
        case 0x73: disasm_cfdel(); break;
        case 0x74: disasm_cfreadv(); break;
        case 0x75: disasm_cfwritev(); break;
        case 0x76: disasm_cfwritei(); break;
        case 0x77: disasm_cfreadb(); break;
        case 0x78: disasm_cfwriteb(); break;
        case 0x79: disasm_cplot(); break;
        case 0x7a: disasm_cdraw(); break;
        case 0x7b: disasm_cbox(); break;
        case 0x7c: disasm_cboxf(); break;
        case 0x7d: disasm_cink(); break;
        case 0x7e: disasm_cpset(); break;
        case 0x7f: disasm_cpmove(); break;
        case 0x80: disasm_cpmode(); break;
        case 0x81: disasm_cpicture(); break;
        case 0x82: disasm_cxyscroll(); break;
        case 0x83: disasm_clinking(); break;
        case 0x84: disasm_cmouson(); break;
        case 0x85: disasm_cmousoff(); break;
        case 0x86: disasm_cmouse(); break;
        case 0x87: disasm_cdefmouse(); break;
        case 0x88: disasm_csetmouse(); break;
        case 0x89: disasm_cdefvect(); break;
        case 0x8a: disasm_csetvect(); break;
        case 0x8b: disasm_cnul(); break;
        case 0x8c: disasm_capproach(); break;
        case 0x8d: disasm_cescape(); break;
        case 0x8e: disasm_cvtstmov(); break;
        case 0x8f: disasm_cvftstmov(); break;
        case 0x90: disasm_cvmov(); break;
        case 0x91: disasm_cdefworld(); break;
        case 0x92: disasm_cworld(); break;
        case 0x93: disasm_cfindmat(); break;
        case 0x94: disasm_cfindtyp(); break;
        case 0x95: disasm_cmusic(); break;
        case 0x96: disasm_cdelmusic(); break;
        case 0x97: disasm_ccadence(); break;
        case 0x98: disasm_csetvolum(); break;
        case 0x99: disasm_cxinv(); break;
        case 0x9a: disasm_cxinvon(); break;
        case 0x9b: disasm_cxinvoff(); break;
        case 0x9c: disasm_clistent(); break;
        case 0x9d: disasm_csound(); break;
        case 0x9e: disasm_cmsound(); break;
        case 0x9f: disasm_credon(); break;
        case 0xa0: disasm_credoff(); break;
        case 0xa1: disasm_cdelsound(); break;
        case 0xa2: disasm_cwmov(); break;
        case 0xa3: disasm_cwtstmov(); break;
        case 0xa4: disasm_cwftstmov(); break;
        case 0xa5: disasm_ctstform(); break;
        case 0xa6: disasm_cxput(); break;
        case 0xa7: disasm_cxputat(); break;
        case 0xa8: disasm_cmput(); break;
        case 0xa9: disasm_cmputat(); break;
        case 0xaa: disasm_cmxput(); break;
        case 0xab: disasm_cmxputat(); break;
        case 0xac: disasm_cmmusic(); break;
        case 0xad: disasm_cmforme(); break;
        case 0xae: disasm_csettime(); break;
        case 0xaf: disasm_cgettime(); break;
        case 0xb0: disasm_cvinput(); break;
        case 0xb1: disasm_csinput(); break;
        case 0xb2: disasm_casleepfar(); break;
        case 0xb3: disasm_casleepon(); break;
        case 0xb4: disasm_casleepoff(); break;
        case 0xb5: disasm_crunfilm(); break;
        case 0xb6: disasm_cvpicprint(); break;
        case 0xb7: disasm_cspicprint(); break;
        case 0xb8: disasm_cvputprint(); break;
        case 0xb9: disasm_csputprint(); break;
        case 0xba: disasm_cfont(); break;
        case 0xbb: disasm_cpaper(); break;
        case 0xbc: disasm_ctoblack(); break;
        case 0xbd: disasm_cmovcolor(); break;
        case 0xbe: disasm_ctopalet(); break;
        case 0xbf: disasm_cnumput(); break;
        case 0xc0: disasm_cscheart(); break;
        case 0xc1: disasm_cscpos(); break;
        case 0xc2: disasm_cscsize(); break;
        case 0xc3: disasm_cschoriz(); break;
        case 0xc4: disasm_cscvertic(); break;
        case 0xc5: disasm_cscreduce(); break;
        case 0xc6: disasm_cscscale(); break;
        case 0xc7: disasm_creducing(); break;
        case 0xc8: disasm_cscmap(); break;
        case 0xc9: disasm_cscdump(); break;
        case 0xca: disasm_cfindcla(); break;
        case 0xcb: disasm_cnearcla(); break;
        case 0xcc: disasm_cviewcla(); break;
        case 0xcd: disasm_cinstru(); break;
        case 0xce: disasm_cminstru(); break;
        case 0xcf: disasm_cordspr(); break;
        case 0xd0: disasm_calign(); break;
        case 0xd1: disasm_cbackstar(); break;
        case 0xd2: disasm_cstarring(); break;
        case 0xd3: disasm_cengine(); break;
        case 0xd4: disasm_cautobase(); break;
        case 0xd5: disasm_cquality(); break;
        case 0xd6: disasm_chsprite(); break;
        case 0xd7: disasm_cselpalet(); break;
        case 0xd8: disasm_clinepalet(); break;
        case 0xd9: disasm_cautomode(); break;
        case 0xda: disasm_cautofile(); break;
        case 0xdb: disasm_ccancel(); break;
        case 0xdc: disasm_ccancall(); break;
        case 0xdd: disasm_ccancen(); break;
        case 0xde: disasm_cblast(); break;
        case 0xdf: disasm_cscback(); break;
        case 0xe0: disasm_cscrolpage(); break;
        case 0xe1: disasm_cmatent(); break;
        case 0xe2: disasm_cshrink(); break;
        case 0xe3: disasm_cdefmap(); break;
        case 0xe4: disasm_csetmap(); break;
        case 0xe5: disasm_cputmap(); break;
        case 0xe6: disasm_csavepal(); break;
        case 0xe7: disasm_csczoom(); break;
        case 0xe8: disasm_ctexmap(); break;
        case 0xe9: disasm_calloctab(); break;
        case 0xea: disasm_cfreetab(); break;
        case 0xeb: disasm_cscantab(); break;
        case 0xec: disasm_cneartab(); break;
        case 0xed: disasm_cscsun(); break;
        case 0xee: disasm_cdarkpal(); break;
        case 0xef: disasm_cscdark(); break;
        case 0xf0: disasm_caset(); break;
        case 0xf1: disasm_camov(); break;
        case 0xf2: disasm_cscaset(); break;
        case 0xf3: disasm_cscamov(); break;
        case 0xf4: disasm_cscfollow(); break;
        case 0xf5: disasm_cscview(); break;
        case 0xf6: disasm_cfilm(); break;
        case 0xf7: disasm_cwalkmap(); break;
        case 0xf8: disasm_catstmap(); break;
        case 0xf9: disasm_cavtstmov(); break;
        case 0xfa: disasm_cavmov(); break;
        case 0xfb: disasm_caim(); break;
        case 0xfc: disasm_cpointpix(); break;
        case 0xfd: disasm_cchartmap(); break;
        case 0xfe: disasm_cscsky(); break;
        case 0xff: disasm_czoom(); break;
        default: fprintf(disasm_fout, " [UNKNOWN]"); break;
    }
}

void readexec_opernames() {
    if (disasm_pc >= disasm_size) return;
    u8 op = disasm_buf[disasm_pc++];
    fprintf(disasm_fout, " %02X ["opernames":%02X]", op, op);
    switch(op) {
        case 0x00: disasm_oimmb(); break;
        case 0x02: disasm_oimmw(); break;
        case 0x04: disasm_oimmp(); break;
        case 0x06: disasm_olocb(); break;
        case 0x08: disasm_olocw(); break;
        case 0x0a: disasm_olocp(); break;
        case 0x0c: disasm_oloctp(); break;
        case 0x0e: disasm_oloctc(); break;
        case 0x10: disasm_olocti(); break;
        case 0x12: disasm_odirb(); break;
        case 0x14: disasm_odirw(); break;
        case 0x16: disasm_odirp(); break;
        case 0x18: disasm_odirtp(); break;
        case 0x1a: disasm_odirtc(); break;
        case 0x1c: disasm_odirti(); break;
        case 0x1e: disasm_omainb(); break;
        case 0x20: disasm_omainw(); break;
        case 0x22: disasm_omainp(); break;
        case 0x24: disasm_omaintp(); break;
        case 0x26: disasm_omaintc(); break;
        case 0x28: disasm_omainti(); break;
        case 0x2a: disasm_ohimb(); break;
        case 0x2c: disasm_ohimw(); break;
        case 0x2e: disasm_ohimp(); break;
        case 0x30: disasm_ohimtp(); break;
        case 0x32: disasm_ohimtc(); break;
        case 0x34: disasm_ohimti(); break;
        case 0x36: disasm_opile(); break;
        case 0x38: disasm_oeval(); break;
        case 0x3a: disasm_ofin(); break;
        case 0x3c: disasm_pnul(); break;
        case 0x3e: disasm_pnul(); break;
        case 0x40: disasm_opushacc(); break;
        case 0x42: disasm_oand(); break;
        case 0x44: disasm_oor(); break;
        case 0x46: disasm_oxor(); break;
        case 0x48: disasm_oeqv(); break;
        case 0x4a: disasm_oegal(); break;
        case 0x4c: disasm_odiff(); break;
        case 0x4e: disasm_oinfeg(); break;
        case 0x50: disasm_osupeg(); break;
        case 0x52: disasm_oinf(); break;
        case 0x54: disasm_osup(); break;
        case 0x56: disasm_oadd(); break;
        case 0x58: disasm_osub(); break;
        case 0x5a: disasm_omod(); break;
        case 0x5c: disasm_odiv(); break;
        case 0x5e: disasm_omul(); break;
        case 0x60: disasm_oneg(); break;
        case 0x62: disasm_oabs(); break;
        case 0x64: disasm_ornd(); break;
        case 0x66: disasm_osgn(); break;
        case 0x68: disasm_onot(); break;
        case 0x6a: disasm_oinkey(); break;
        case 0x6c: disasm_okeyon(); break;
        case 0x6e: disasm_ojoy(); break;
        case 0x70: disasm_oprnd(); break;
        case 0x72: disasm_oscan(); break;
        case 0x74: disasm_oshiftkey(); break;
        case 0x76: disasm_ofree(); break;
        case 0x78: disasm_omodel(); break;
        case 0x7a: disasm_ogetkey(); break;
        case 0x7c: disasm_oleft(); break;
        case 0x7e: disasm_oright(); break;
        case 0x80: disasm_omid(); break;
        case 0x82: disasm_olen(); break;
        case 0x84: disasm_oasc(); break;
        case 0x86: disasm_ostr(); break;
        case 0x88: disasm_osadd(); break;
        case 0x8a: disasm_osegal(); break;
        case 0x8c: disasm_osdiff(); break;
        case 0x8e: disasm_osinfeg(); break;
        case 0x90: disasm_ossupeg(); break;
        case 0x92: disasm_osinf(); break;
        case 0x94: disasm_ossup(); break;
        case 0x96: disasm_ospushacc(); break;
        case 0x98: disasm_ospile(); break;
        case 0x9a: disasm_oval(); break;
        case 0x9c: disasm_oexistf(); break;
        case 0x9e: disasm_ochr(); break;
        case 0xa0: disasm_ochange(); break;
        case 0xa2: disasm_ocountry(); break;
        case 0xa4: disasm_omip(); break;
        case 0xa6: disasm_ojoykey(); break;
        case 0xa8: disasm_oconfig(); break;
        default: fprintf(disasm_fout, " [UNKNOWN]"); break;
    }
}

void readexec_storenames() {
    if (disasm_pc >= disasm_size) return;
    u8 op = disasm_buf[disasm_pc++];
    fprintf(disasm_fout, " %02X ["storenames":%02X]", op, op);
    switch(op) {
        case 0x00: disasm_pnul(); break;
        case 0x02: disasm_pnul(); break;
        case 0x04: disasm_pnul(); break;
        case 0x06: disasm_slocb(); break;
        case 0x08: disasm_slocw(); break;
        case 0x0a: disasm_slocp(); break;
        case 0x0c: disasm_sloctp(); break;
        case 0x0e: disasm_sloctc(); break;
        case 0x10: disasm_slocti(); break;
        case 0x12: disasm_sdirb(); break;
        case 0x14: disasm_sdirw(); break;
        case 0x16: disasm_sdirp(); break;
        case 0x18: disasm_sdirtp(); break;
        case 0x1a: disasm_sdirtc(); break;
        case 0x1c: disasm_sdirti(); break;
        case 0x1e: disasm_smainb(); break;
        case 0x20: disasm_smainw(); break;
        case 0x22: disasm_smainp(); break;
        case 0x24: disasm_smaintp(); break;
        case 0x26: disasm_smaintc(); break;
        case 0x28: disasm_smainti(); break;
        case 0x2a: disasm_shimb(); break;
        case 0x2c: disasm_shimw(); break;
        case 0x2e: disasm_shimp(); break;
        case 0x30: disasm_shimtp(); break;
        case 0x32: disasm_shimtc(); break;
        case 0x34: disasm_shimti(); break;
        case 0x36: disasm_spile(); break;
        case 0x38: disasm_seval(); break;
        case 0x3a: disasm_ofin(); break;
        default: fprintf(disasm_fout, " [UNKNOWN]"); break;
    }
}

void readexec_addnames() {
    if (disasm_pc >= disasm_size) return;
    u8 op = disasm_buf[disasm_pc++];
    fprintf(disasm_fout, " %02X ["addnames":%02X]", op, op);
    switch(op) {
        case 0x00: disasm_pnul(); break;
        case 0x02: disasm_pnul(); break;
        case 0x04: disasm_pnul(); break;
        case 0x06: disasm_alocb(); break;
        case 0x08: disasm_alocw(); break;
        case 0x0a: disasm_alocp(); break;
        case 0x0c: disasm_aloctp(); break;
        case 0x0e: disasm_aloctc(); break;
        case 0x10: disasm_alocti(); break;
        case 0x12: disasm_adirb(); break;
        case 0x14: disasm_adirw(); break;
        case 0x16: disasm_adirp(); break;
        case 0x18: disasm_adirtp(); break;
        case 0x1a: disasm_adirtc(); break;
        case 0x1c: disasm_adirti(); break;
        case 0x1e: disasm_amainb(); break;
        case 0x20: disasm_amainw(); break;
        case 0x22: disasm_amainp(); break;
        case 0x24: disasm_amaintp(); break;
        case 0x26: disasm_amaintc(); break;
        case 0x28: disasm_amainti(); break;
        case 0x2a: disasm_ahimb(); break;
        case 0x2c: disasm_ahimw(); break;
        case 0x2e: disasm_ahimp(); break;
        case 0x30: disasm_ahimtp(); break;
        case 0x32: disasm_ahimtc(); break;
        case 0x34: disasm_ahimti(); break;
        case 0x36: disasm_spile(); break;
        case 0x38: disasm_aeval(); break;
        case 0x3a: disasm_ofin(); break;
        default: fprintf(disasm_fout, " [UNKNOWN]"); break;
    }
}

void readexec_esc1names() {
    if (disasm_pc >= disasm_size) return;
    u8 op = disasm_buf[disasm_pc++];
    fprintf(disasm_fout, " %02X ["esc1names":%02X]", op, op);
    switch(op) {
        case 0x00: disasm_cnul(); break;
        case 0x01: disasm_csoundon(); break;
        case 0x02: disasm_csoundoff(); break;
        case 0x03: disasm_cmusicon(); break;
        case 0x04: disasm_cmusicoff(); break;
        case 0x05: disasm_cdelfilm(); break;
        case 0x06: disasm_copenfilm(); break;
        default: fprintf(disasm_fout, " [UNKNOWN]"); break;
    }
}

void readexec_esc2names() {
    if (disasm_pc >= disasm_size) return;
    u8 op = disasm_buf[disasm_pc++];
    fprintf(disasm_fout, " %02X ["esc2names":%02X]", op, op);
    switch(op) {
        case 0x00: disasm_cnul(); break;
        default: fprintf(disasm_fout, " [UNKNOWN]"); break;
    }
}

void readexec_esc3names() {
    if (disasm_pc >= disasm_size) return;
    u8 op = disasm_buf[disasm_pc++];
    fprintf(disasm_fout, " %02X ["esc3names":%02X]", op, op);
    switch(op) {
        case 0x00: disasm_cnul(); break;
        default: fprintf(disasm_fout, " [UNKNOWN]"); break;
    }
}
