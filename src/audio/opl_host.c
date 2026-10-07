// opl_host.c — host-side OPL decode (the 68k half). No synthesis: it only
// tracks register/patch state and resolves it into opl_block_t per channel.
// Decode logic mirrors emu8950 OPL_writeReg, minus
// the synth; tll/rks resolution mirrors makeTllTable/makeRksTable + commit.

#include "opl_host.h"
#include <stdlib.h>
#include <string.h>

#define TL2EG(tl) ((tl) << 2)
#define EG_STEP   0.1875
#define dB2(x)    ((x) * 2)

/* tll/rks tables — the host owns these (DSP never sees them) */
static double  kl_dbl[16] = {
    dB2(0.000),dB2(9.000),dB2(12.000),dB2(13.875),dB2(15.000),dB2(16.125),
    dB2(16.875),dB2(17.625),dB2(18.000),dB2(18.750),dB2(19.125),dB2(19.500),
    dB2(19.875),dB2(20.250),dB2(20.625),dB2(21.000) };
// tll_table is 128 KB and only OPL titles use it, so it's allocated on first
// use (never freed) instead of sitting in BSS.
typedef uint32_t tll_row_t[64][4];
static tll_row_t *tll_table = NULL;      // [8*16][64][4] once allocated
static int32_t  rks_table[2][32][2];
static int host_tables_ready = 0;

static void host_build_tables(void) {
    int fnum, block, TL, KL, kx; int32_t tmp;

    if (tll_table == NULL) {
        tll_table = (tll_row_t *)malloc(sizeof(tll_row_t) * (8 * 16));
        if (tll_table == NULL)
            return;                      // host_tables_ready stays 0 -> callers skip tll
    }

    for (fnum=0; fnum<16; fnum++) for (block=0; block<8; block++)
      for (TL=0; TL<64; TL++) for (KL=0; KL<4; KL++) {
        kx = ((KL&1)<<1) | ((KL>>1)&1);
        if (KL==0) tll_table[(block<<4)|fnum][TL][KL] = TL2EG(TL);
        else {
            tmp = (int32_t)(kl_dbl[fnum] - dB2(3.000)*(7-block));
            if (tmp<=0) tll_table[(block<<4)|fnum][TL][KL] = TL2EG(TL);
            else tll_table[(block<<4)|fnum][TL][KL] =
                    (uint32_t)((tmp >> (3-kx)) / EG_STEP) + TL2EG(TL);
        }
      }
    { int f8,f9,blk,bf;
      for (f8=0; f8<2; f8++) for (f9=0; f9<2; f9++) for (blk=0; blk<8; blk++) {
        bf = (blk<<2)|(f9<<1)|f8;
        rks_table[0][bf][1]=(blk<<1)+f9;       rks_table[0][bf][0]=blk>>1;
        rks_table[1][bf][1]=(blk<<1)+(f9&f8);  rks_table[1][bf][0]=blk>>1;
      } }
    host_tables_ready = 1;
}

/* per-slot decoded patch (register fields) */
typedef struct {
    uint8_t AM,PM,EG,KR,ML,KL,TL,AR,DR,SL,RR,FB;
    uint16_t blk_fnum, fnum;
    uint8_t  blk;
} hpatch_t;

struct opl_host {
    hpatch_t slot[18];
    uint8_t  ch_alg[9];
    uint8_t  notesel;
    uint8_t  reg[256];
    uint8_t  key_cur[9];     // current key bit (per 0xB0 write)
    uint8_t  key_event[9];   // last key edge this frame (OPL_KEY_*), cleared on get_block
};

/* OPL channel -> {mod,car} slot index (emu8950 stbl, melody) */
static const int stbl[32] = {0,2,4,1,3,5,-1,-1,6,8,10,7,9,11,-1,-1,
                             12,14,16,13,15,17,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

opl_host_t *opl_host_new(void) {
    opl_host_t *h;
    if (!host_tables_ready) host_build_tables();
    h = calloc(1, sizeof(*h));
    return h;
}
void opl_host_reset(opl_host_t *h) { memset(h, 0, sizeof(*h)); }
void opl_host_delete(opl_host_t *h) { free(h); }

static void set_fnumber(opl_host_t *h, int ch, int fnum) {
    hpatch_t *car = &h->slot[ch*2+1], *mod = &h->slot[ch*2];
    car->fnum = fnum; car->blk_fnum = (car->blk_fnum & 0x1c00) | (fnum & 0x3ff);
    mod->fnum = fnum; mod->blk_fnum = (mod->blk_fnum & 0x1c00) | (fnum & 0x3ff);
}
static void set_block(opl_host_t *h, int ch, int blk) {
    hpatch_t *car = &h->slot[ch*2+1], *mod = &h->slot[ch*2];
    car->blk = blk; car->blk_fnum = ((blk&7)<<10) | (car->blk_fnum & 0x3ff);
    mod->blk = blk; mod->blk_fnum = ((blk&7)<<10) | (mod->blk_fnum & 0x3ff);
}

void opl_host_write_reg(opl_host_t *h, uint32_t reg, uint8_t data) {
    int s, c; reg &= 0xff; h->reg[reg] = data;
    if (reg == 0x08) h->notesel = (data >> 6) & 1;
    else if (0x20 <= reg && reg < 0x40) { s = stbl[reg-0x20];
        if (s>=0){ h->slot[s].AM=(data>>7)&1; h->slot[s].PM=(data>>6)&1;
                   h->slot[s].EG=(data>>5)&1; h->slot[s].KR=(data>>4)&1; h->slot[s].ML=data&15; } }
    else if (0x40 <= reg && reg < 0x60) { s = stbl[reg-0x40];
        if (s>=0){ h->slot[s].KL=(data>>6)&3; h->slot[s].TL=data&63; } }
    else if (0x60 <= reg && reg < 0x80) { s = stbl[reg-0x60];
        if (s>=0){ h->slot[s].AR=(data>>4)&15; h->slot[s].DR=data&15; } }
    else if (0x80 <= reg && reg < 0xa0) { s = stbl[reg-0x80];
        if (s>=0){ h->slot[s].SL=(data>>4)&15; h->slot[s].RR=data&15; } }
    else if (0xa0 <= reg && reg < 0xa9) { c = reg-0xa0;
        set_fnumber(h, c, data + ((h->reg[reg+0x10]&3)<<8)); }
    else if (0xb0 <= reg && reg < 0xb9) { c = reg-0xb0;
        set_fnumber(h, c, ((data&3)<<8) + h->reg[reg-0x10]); set_block(h, c, (data>>2)&7);
        { uint8_t k = (data & 0x20) ? 1 : 0;
          if (k != h->key_cur[c]) { h->key_event[c] = k ? OPL_KEY_ON : OPL_KEY_OFF; h->key_cur[c] = k; } } }
    else if (0xc0 <= reg && reg < 0xc9) { c = reg-0xc0;
        h->slot[c*2].FB=(data>>1)&7; h->ch_alg[c]=data&1; }
    else if (reg == 0xbd) { /* am/pm depth: never used by RRQ, ignored */ }
    /* reg 0x01/0xe0 waveform-select: dropped (WSE never enabled, pure sine) */
}

void opl_host_get_block(opl_host_t *h, int ch, opl_block_t *b) {
    int op;
    memset(b, 0, sizeof(*b));
    b->key        = h->key_event[ch];   /* last key edge this frame */
    h->key_event[ch] = OPL_KEY_NONE;    /* consumed */
    b->connection = h->ch_alg[ch];
    b->fnum       = h->slot[ch*2].fnum & 0x3ff;
    b->blk        = h->slot[ch*2].blk & 7;
    for (op = 0; op < 2; op++) {
        hpatch_t *s = &h->slot[ch*2 + op];
        b->op[op].ML = s->ML; b->op[op].AM = s->AM; b->op[op].PM = s->PM; b->op[op].EG = s->EG;
        b->op[op].AR = s->AR; b->op[op].DR = s->DR; b->op[op].SL = s->SL; b->op[op].RR = s->RR;
        b->op[op].FB = s->FB;
        // tll_table is allocated lazily and can be NULL if that malloc failed;
        // fall back to the no-key-scaling value (the KL==0 row) rather than
        // dereferencing it — OPL keeps playing, just without key-scale damping.
        b->op[op].tll = tll_table ? tll_table[s->blk_fnum >> 6][s->TL][s->KL]
                                  : (uint32_t)TL2EG(s->TL);
        b->op[op].rks = rks_table[h->notesel][s->blk_fnum >> 8][s->KR];
    }
}
