//
// Copyright 2024 Olivier Huguenot, Vadim Kindl
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
#include "audio.h"
#include "alis_private.h"
#include "channel.h"
#include "mem.h"
#include "image.h"
#include "screen.h"
#include "unpack.h"
#include "utils.h"
#include "video.h"

// ============================================================================
// Routines for new versions of ALIS interpreter
// (after Robinson's Requiem and Ishar 3)
//
// Preliminary version for further work
// Initial version: 2024-10-10
// ============================================================================
// For games:
//    Deus +
//    Time Warriors +
//    Asghan +
//    Tournament Of Warriors +
//    Arabian Nights ?
//    Les Visiteurs ?	
//    Inspector Gadget ?
// ============================================================================
// Changes and additions:
// ============================================================================
// 1. Opcode / Codop (Code-op) routines
// ============================================================================
// tcodop_0D: cjsrind16 => cjsrproc
// tcodop_0E: cjsrind24 => cstartproc
// tcodop_0F: cjmpind16 => cjmpproc
// tcodop_2A: crandom => cnul
// tcodop_33: cleave => cstartbase
// tcodop_34: cprotect => cnul
// tcodop_37: cnul => canim
// tcodop_3A: cclipping => cvecacopy
// tcodop_3B: cswitching => cvecrot
// tcodop_3C: cwlive => cveclive
// tcodop_43: cstopret => cvecdistabs
// tcodop_46: cdefsc => csctype
// tcodop_49: cputnat => cputat
// tcodop_4B: cerasen => cobjmov
// tcodop_50: cerasall => cnul
// tcodop_51: cforme => caddput
// tcodop_52: cdelforme => caddputat
// tcodop_5F: corient => ctstanim
// tcodop_6E: cnoise => cmodanim
// tcodop_79: cplot => cvecset
// tcodop_7A: cdraw => cvecmov
// tcodop_7B: cbox => cveccopy
// tcodop_7C: cboxf => cvecadd
// tcodop_7E: cpset => cvecsub
// tcodop_7F: cpmove => cvecsize
// tcodop_80: cpmode => cvecnorm
// tcodop_81: cpicture => cscworld
// tcodop_82: cxyscroll => cvecdist
// tcodop_89: cdefvect => cobjaset
// tcodop_8A: csetvect => cobjamov
// tcodop_8C: capproach => ctsthit
// tcodop_8D: cescape => cftsthit
// tcodop_93: cfindmat => cfindworld
// tcodop_98: csetvolum => cvolume
// tcodop_9E: cmsound => centsound
// tcodop_A2: cwmov => cmatterpix
// tcodop_A3: cwtstmov => ctsthere
// tcodop_A4: cwftstmov => cftsthere
// tcodop_A5: ctstform => cavftstmov
// tcodop_AA: cmxput => ceput
// tcodop_AB: cmxputat => ceputat
// tcodop_AC: cmmusic => cavtsthit
// tcodop_AD: cmforme => cputstring
// tcodop_B2: casleepfar => cfreezefar
// tcodop_B3: casleepon => cfaron
// tcodop_B4: casleepoff => cfaroff
// tcodop_B6: cvpicprint => cfreeze
// tcodop_B7: cspicprint => cunfreeze
// tcodop_C3: cschoriz => cscmode
// tcodop_C4: cscvertic => cscclr
// tcodop_C5: cscreduce => cunloadfar
// tcodop_C7: creducing => cobjmode
// tcodop_C9: cscdump => cframe
// tcodop_CE: cminstru => cavftsthit
// tcodop_D0: calign => clight
// tcodop_D5: cquality => cwalkanim
// tcodop_D6: chsprite => cfwalkanim
// tcodop_DB: ccancel => cdel
// tcodop_DC: ccancall => cobjlink
// tcodop_DD: ccancen => cobjset
// tcodop_E7: csczoom => crecalc
// tcodop_E8: ctexmap => cfwalk
// tcodop_ED: cscsun => cnul ; cobjcycle
// tcodop_EF: cscdark => cscshade
// tcodop_F7: cwalkmap => cwalk
// tcodop_FF: czoom => cobjscale
// ============================================================================
// 2. Codesc1 routines
// ============================================================================
// tcodesc1_07: + cdeffont
// tcodesc1_08: + cloadtext
// tcodesc1_09: + cafteron
// tcodesc1_0A: + cafteroff
// tcodesc1_0B: + cbeforon
// tcodesc1_0C: + cbeforoff
// tcodesc1_0D: + cscfocale
// tcodesc1_0E: + csccamera
// tcodesc1_0F: + ctexmap
// tcodesc1_10: + cscrender
// tcodesc1_11: + cvideo
// tcodesc1_12: + ctexture
// tcodesc1_13: + cplaycd
// tcodesc1_14: + cnul
// tcodesc1_15: + cobjtrans
// tcodesc1_16: + conscreen
// tcodesc1_17: + cgetglobal
// tcodesc1_18: + cgetlocal
// tcodesc1_19: + cmatitem
// tcodesc1_1A: + cmaskitem
// tcodesc1_1B: + cdefitem
// tcodesc1_1C: + cnul
// tcodesc1_1D: + cnul
// tcodesc1_1E: + cgetmap
// tcodesc1_1F: + cconvmap
// tcodesc1_20: + csetfilm
// tcodesc1_21: + cscmem
// tcodesc1_22: + cobjorient ; =amtstmap
// tcodesc1_23: + cobjaim
// tcodesc1_24: + cloadpic
// tcodesc1_25: + cputpic
// tcodesc1_26: + cfwriteg
// tcodesc1_27: + cfreadg
// tcodesc1_28: + cobjmat
// tcodesc1_29: + cobjmask
// tcodesc1_2A: + cscclut
// tcodesc1_2B: + cobjclut
// tcodesc1_2C: + copennet
// tcodesc1_2D: + cclosenet
// tcodesc1_2E: + csendnet
// tcodesc1_2F: + cmailnet
// tcodesc1_30: + cplayernet
// tcodesc1_31: + cgamenet
// tcodesc1_32: + csyncnet
// tcodesc1_33: + ctexitem
// tcodesc1_34: + cgetanim
// tcodesc1_35: + canimsize
// tcodesc1_36: + cscmath
// tcodesc1_37: + cintermode
// tcodesc1_38: + csysnet
// tcodesc1_39: + cobjshad
// tcodesc1_3A: + csphere
// tcodesc1_3B: + cscdark
// tcodesc1_3C: + cagetlocal
// tcodesc1_3D: + cfollow
// tcodesc1_3E: + cplayfilm
// tcodesc1_3F: + csoundtime
// tcodesc1_40: + cinfoitem
// tcodesc1_41: + cmorph
// tcodesc1_42: + cscmicro
// tcodesc1_43: + cptrset
// tcodesc1_44: + cdefproc
// tcodesc1_45: + cfreadm
// tcodesc1_46: + cfwritem
// tcodesc1_47: + cmaskent
// tcodesc1_48: + cvram
// tcodesc1_49: + cadresnet
// tcodesc1_4A: + cfdir
// tcodesc1_4B: + cnul ; entitem
// tcodesc1_4C: + cloadas
// tcodesc1_4D: + cfindobjitem
// tcodesc1_4E: + cobjid
// tcodesc1_4F: + cgetobjid
// tcodesc1_50: + cprelight
// tcodesc1_51: + croomdoor
// ============================================================================
// 3. Oper routines
// ============================================================================
// toper_12: odirb => olocl
// toper_14: odirw => oloctl
// toper_16: odirp => omainl
// toper_18: odirtp => omaintl
// toper_1A: odirtc => ohiml
// toper_1C: odirti => ohimtl
// toper_3C: pnul => oimml
// toper_3E: pnul => olocto
// toper_A0: ochange => ofpath
// toper_AA: + ogettext
// toper_AC: + oips
// toper_AE: + ogetent
// toper_B0: + osqrt
// toper_B2: + ocos
// toper_B4: + osin
// toper_B6: + otan
// toper_B8: + oatan
// toper_BA: + obufkey ; bidon...
// toper_BC: + omaxitem
// toper_BE: + otick
// toper_C0: + orandom
// toper_C2: + oflen
// toper_C4: + olocalkey
// toper_C6: + odivi
// toper_C8: + oprotect
// toper_CA: + otypitem
// toper_CC: + oloaded
// ============================================================================
// 4. Add routines
// ============================================================================
// tstore_12: sdirb => slocl
// tstore_14: sdirw => sloctl
// tstore_16: sdirp => smainl
// tstore_18: sdirtp => smaintl
// tstore_1A: sdirtc => shiml
// tstore_1C: sdirti => shimtl
// ============================================================================
// 5. Store routines
// tadd_12: adirb => alocl
// tadd_14: adirw => aloctl
// tadd_16: adirp => amainl
// tadd_18: adirtp => amaintl
// tadd_1A: adirtc => ahiml
// tadd_1C: adirti => ahimtl
// ============================================================================

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines
// ============================================================================

// Codopname no. 001 opcode 0x00 cnul
// Calls stub cnul (from old version)

// Codopname no. 002 opcode 0x01 cesc1
static void cesc1(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 003 opcode 0x02 cesc2
static void cesc2(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 004 opcode 0x03 cesc3
static void cesc3(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 005 opcode 0x04 cbreakpt
static void cbreakpt(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 006 opcode 0x05 cjsr8
static void cjsr8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 007 opcode 0x06 cjsr16
static void cjsr16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 008 opcode 0x07 cjsr24
static void cjsr24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 009 opcode 0x08 cjmp8
static void cjmp8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 010 opcode 0x09 cjmp16
static void cjmp16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 011 opcode 0x0a cjmp24
static void cjmp24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 012 opcode 0x0b cjsrabs
static void cjsrabs(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 013 opcode 0x0c cjmpabs
static void cjmpabs(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 014 opcode 0x0d cjsrproc
// Change v40+: cjsrind16 => cjsrproc
static void cjsrproc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 015 opcode 0x0e cstartproc
// Change v40+: cjsrind24 => cstartproc
static void cstartproc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 016 opcode 0x0f cjmpproc
// Change v40+: cjmpind16 => cjmpproc
static void cjmpproc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 017 opcode 0x10 cjmpind24
static void cjmpind24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 018 opcode 0x11 cret
static void cret(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 019 opcode 0x12 cbz8
static void cbz8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 020 opcode 0x13 cbz16
static void cbz16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 021 opcode 0x14 cbz24
static void cbz24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 022 opcode 0x15 cbnz8
static void cbnz8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 023 opcode 0x16 cbnz16
static void cbnz16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 024 opcode 0x17 cbnz24
static void cbnz24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 025 opcode 0x18 cbeq8
static void cbeq8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 026 opcode 0x19 cbeq16
static void cbeq16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 027 opcode 0x1a cbeq24
static void cbeq24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 028 opcode 0x1b cbne8
static void cbne8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 029 opcode 0x1c cbne16
static void cbne16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 030 opcode 0x1d cbne24
static void cbne24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 031 opcode 0x1e cstore
static void cstore(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 032 opcode 0x1f ceval
static void ceval(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 033 opcode 0x20 cadd
static void cadd(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 034 opcode 0x21 csub
static void csub(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 035 opcode 0x22 cmul
static void cmul(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 036 opcode 0x23 cdiv
static void cdiv(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 037 opcode 0x24 cvprint
static void cvprint(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 038 opcode 0x25 csprinti
static void csprinti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 039 opcode 0x26 csprinta
static void csprinta(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 040 opcode 0x27 clocate
static void clocate(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 041 opcode 0x28 ctab
static void ctab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 042 opcode 0x29 cdim
static void cdim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 043 opcode 0x2a cnul
// Change v40+: crandom => cnul
// Calls stub cnul (from old version)

// Codopname no. 044 opcode 0x2b cloop8
static void cloop8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 045 opcode 0x2c cloop16
static void cloop16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 046 opcode 0x2d cloop24
static void cloop24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 047 opcode 0x2e cswitch1
static void cswitch1(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 048 opcode 0x2f cswitch2
static void cswitch2(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 049 opcode 0x30 cstart8
static void cstart8(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 050 opcode 0x31 cstart16
static void cstart16(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 051 opcode 0x32 cstart24
static void cstart24(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 052 opcode 0x33 cstartbase
// Change v40+: cleave => cstartbase
static void cstartbase(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 053 opcode 0x34 cnul
// Change v40+: cprotect => cnul
// Calls stub cnul (from old version)

// Codopname no. 054 opcode 0x35 casleep
static void casleep(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 055 opcode 0x36 cclock
static void cclock(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 056 opcode 0x37 canim
// Change v40+: cnul => canim
static void canim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 057 opcode 0x38 cscmov
static void cscmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 058 opcode 0x39 cscset
static void cscset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 059 opcode 0x3a cvecacopy
// Change v40+: cclipping => cvecacopy
static void cvecacopy(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 060 opcode 0x3b cvecrot
// Change v40+: cswitching => cvecrot
static void cvecrot(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 061 opcode 0x3c cveclive
// Change v40+: cwlive => cveclive
static void cveclive(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 062 opcode 0x3d cunload
static void cunload(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 063 opcode 0x3e cwakeup
static void cwakeup(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 064 opcode 0x3f csleep
static void csleep(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 065 opcode 0x40 clive
static void clive(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 066 opcode 0x41 ckill
static void ckill(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 067 opcode 0x42 cstop
static void cstop(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 068 opcode 0x43 cvecdistabs
// Change v40+: cstopret => cvecdistabs
static void cvecdistabs(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 069 opcode 0x44 cexit
static void cexit(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 070 opcode 0x45 cload
static void cload(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 071 opcode 0x46 csctype
// Change v40+: cdefsc => csctype
static void csctype(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 072 opcode 0x47 cscreen
static void cscreen(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 073 opcode 0x48 cput
static void cput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 074 opcode 0x49 cputat
// Change v40+: cputnat => cputat
static void cputat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 075 opcode 0x4a cerase
static void cerase(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 076 opcode 0x4b cobjmov
// Change v40+: cerasen => cobjmov
static void cobjmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 077 opcode 0x4c cset
static void cset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 078 opcode 0x4d cmov
static void cmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 079 opcode 0x4e copensc
static void copensc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 080 opcode 0x4f cclosesc
static void cclosesc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 081 opcode 0x50 cnul
// Change v40+: cerasall => cnul
// Calls stub cnul (from old version)

// Codopname no. 082 opcode 0x51 caddput
// Change v40+: cforme => caddput
static void caddput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 083 opcode 0x52 caddputat
// Change v40+: cdelforme => caddputat
static void caddputat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 084 opcode 0x53 ctstmov
static void ctstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 085 opcode 0x54 ctstset
static void ctstset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 086 opcode 0x55 cftstmov
static void cftstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 087 opcode 0x56 cftstset
static void cftstset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 088 opcode 0x57 csuccent
static void csuccent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 080 opcode 0x58 cpredent
static void cpredent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 090 opcode 0x59 cnearent
static void cnearent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 091 opcode 0x5a cneartyp
static void cneartyp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 092 opcode 0x5b cnearmat
static void cnearmat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 093 opcode 0x5c cviewent
static void cviewent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 094 opcode 0x5d cviewtyp
static void cviewtyp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 095 opcode 0x5e cviewmat
static void cviewmat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 096 opcode 0x5f ctstanim
// Change v40+: corient => ctstanim
static void ctstanim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 097 opcode 0x60 crstent
static void crstent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 098 opcode 0x61 csend
static void csend(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 099 opcode 0x62 cscanon
static void cscanon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 100 opcode 0x63 cscanoff
static void cscanoff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 101 opcode 0x64 cinteron
static void cinteron(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 102 opcode 0x65 cinteroff
static void cinteroff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 103 opcode 0x66 cscanclr
static void cscanclr(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 104 opcode 0x67 callentity
static void callentity(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 105 opcode 0x68 cpalette
static void cpalette(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 106 opcode 0x69 cdefcolor
static void cdefcolor(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 107 opcode 0x6a ctiming
static void ctiming(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 108 opcode 0x6b czap
static void czap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 109 opcode 0x6c cexplode
static void cexplode(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 110 opcode 0x6d cding
static void cding(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 111 opcode 0x6e cmodanim
// Change v40+: cnoise => cmodanim
static void cmodanim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 112 opcode 0x6f cinitab
static void cinitab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 113 opcode 0x70 cfopen
static void cfopen(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 114 opcode 0x71 cfclose
static void cfclose(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 115 opcode 0x72 cfcreat
static void cfcreat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 116 opcode 0x73 cfdel
static void cfdel(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 117 opcode 0x74 cfreadv
static void cfreadv(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 118 opcode 0x75 cfwritev
static void cfwritev(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 119 opcode 0x76 cfwritei
static void cfwritei(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 120 opcode 0x77 cfreadb
static void cfreadb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 121 opcode 0x78 cfwriteb
static void cfwriteb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 122 opcode 0x79 cvecset
// Change v40+: cplot => cvecset
static void cvecset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 123 opcode 0x7a cvecmov
// Change v40+: cdraw => cvecmov
static void cvecmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 124 opcode 0x7b cveccopy
// Change v40+: cbox => cveccopy
static void cveccopy(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 125 opcode 0x7c cvecadd
// Change v40+: cboxf => cvecadd
static void cvecadd(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 126 opcode 0x7d cink
static void cink(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 127 opcode 0x7e cvecsub
// Change v40+: cpset => cvecsub
static void cvecsub(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 128 opcode 0x7f cvecsize
// Change v40+: cpmove => cvecsize
static void cvecsize(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 129 opcode 0x80 cvecnorm
// Change v40+: cpmode => cvecnorm
static void cvecnorm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 130 opcode 0x81 cscworld
// Change v40+: cpicture => cscworld
static void cscworld(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 131 opcode 0x82 cvecdist
// Change v40+: cxyscroll => cvecdist
static void cvecdist(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 132 opcode 0x83 clinking
static void clinking(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 133 opcode 0x84 cmouson
static void cmouson(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 134 opcode 0x85 cmousoff
static void cmousoff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 135 opcode 0x86 cmouse
static void cmouse(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 136 opcode 0x87 cdefmouse
static void cdefmouse(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 137 opcode 0x88 csetmouse
static void csetmouse(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 138 opcode 0x89 cobjaset
// Change v40+: cdefvect => cobjaset
static void cobjaset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 139 opcode 0x8a cobjamov
// Change v40+: csetvect => cobjamov
static void cobjamov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 140 opcode 0x8b cnul
// Calls stub cnul (from old version)

// Codopname no. 141 opcode 0x8c ctsthit
// Change v40+: capproach => ctsthit
static void ctsthit(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 142 opcode 0x8d cftsthit
// Change v40+: cescape => cftsthit
static void cftsthit(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 143 opcode 0x8e cvtstmov
static void cvtstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 144 opcode 0x8f cvftstmov
static void cvftstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 145 opcode 0x90 cvmov
static void cvmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 146 opcode 0x91 cdefworld
static void cdefworld(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 147 opcode 0x92 cworld
static void cworld(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 148 opcode 0x93 cfindworld
// Change v40+: cfindmat => cfindworld
static void cfindworld(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 149 opcode 0x94 cfindtyp
static void cfindtyp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 150 opcode 0x95 cmusic
static void cmusic(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 151 opcode 0x96 cdelmusic
static void cdelmusic(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 152 opcode 0x97 ccadence
static void ccadence(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 153 opcode 0x98 cvolume
// Change v40+: csetvolum => cvolume
static void cvolume(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 154 opcode 0x99 cxinv
static void cxinv(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 155 opcode 0x9a cxinvon
static void cxinvon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 156 opcode 0x9b cxinvoff
static void cxinvoff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 157 opcode 0x9c clistent
static void clistent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 158 opcode 0x9d csound
static void csound(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 159 opcode 0x9e centsound
// Change v40+: cmsound => centsound
static void centsound(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 160 opcode 0x9f credon
static void credon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 161 opcode 0xa0 credoff
static void credoff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 162 opcode 0xa1 cdelsound
static void cdelsound(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 163 opcode 0xa2 cmatterpix
// Change v40+: cwmov => cmatterpix
static void cmatterpix(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 164 opcode 0xa3 ctsthere
// Change v40+: cwtstmov => ctsthere
static void ctsthere(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 165 opcode 0xa4 cftsthere
// Change v40+: cwftstmov => cftsthere
static void cftsthere(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 166 opcode 0xa5 cavftstmov
// Change v40+: ctstform => cavftstmov
static void cavftstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 167 opcode 0xa6 cxput
static void cxput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 168 opcode 0xa7 cxputat
static void cxputat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 169 opcode 0xa8 cmput
static void cmput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 170 opcode 0xa9 cmputat
static void cmputat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 171 opcode 0xaa ceput
// Change v40+: cmxput => ceput
static void ceput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 172 opcode 0xab ceputat
// Change v40+: cmxputat => ceputat
static void ceputat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 173 opcode 0xac cavtsthit
// Change v40+: cmmusic => cavtsthit
static void cavtsthit(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 174 opcode 0xad cputstring
// Change v40+: cmforme => cputstring
static void cputstring(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 175 opcode 0xae csettime
static void csettime(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 176 opcode 0xaf cgettime
static void cgettime(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 177 opcode 0xb0 cvinput
static void cvinput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 178 opcode 0xb1 csinput
static void csinput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 179 opcode 0xb2 cfreezefar
// Change v40+: casleepfar => cfreezefar
static void cfreezefar(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 180 opcode 0xb3 cfaron
// Change v40+: casleepon => cfaron
static void cfaron(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 181 opcode 0xb4 cfaroff
// Change v40+: casleepoff => cfaroff
static void cfaroff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 182 opcode 0xb5 crunfilm
static void crunfilm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 183 opcode 0xb6 cfreeze
// Change v40+: cvpicprint => cfreeze
static void cfreeze(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 184 opcode 0xb7 cunfreeze
// Change v40+: cspicprint => cunfreeze
static void cunfreeze(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 185 opcode 0xb8 cvputprint
static void cvputprint(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 186 opcode 0xb9 csputprint
static void csputprint(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 187 opcode 0xba cfont
static void cfont(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 188 opcode 0xbb cpaper
static void cpaper(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 189 opcode 0xbc ctoblack
static void ctoblack(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 190 opcode 0xbd cmovcolor
static void cmovcolor(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 191 opcode 0xbe ctopalet
static void ctopalet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 192 opcode 0xbf cnumput
static void cnumput(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 193 opcode 0xc0 cscheart
static void cscheart(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 194 opcode 0xc1 cscpos
static void cscpos(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 195 opcode 0xc2 cscsize
static void cscsize(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 196 opcode 0xc3 cscmode
// Change v40+: cschoriz => cscmode
static void cscmode(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 197 opcode 0xc4 cscclr
// Change v40+: cscvertic => cscclr
static void cscclr(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 198 opcode 0xc5 cunloadfar
// Change v40+: cscreduce => cunloadfar
static void cunloadfar(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 199 opcode 0xc6 cscscale
static void cscscale(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 200 opcode 0xc7 cobjmode
// Change v40+: creducing => cobjmode
static void cobjmode(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 201 opcode 0xc8 cscmap
static void cscmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 202 opcode 0xc9 cframe
// Change v40+: cscdump => cframe
static void cframe(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 203 opcode 0xca cfindcla
static void cfindcla(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 204 opcode 0xcb cnearcla
static void cnearcla(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 205 opcode 0xcc cviewcla
static void cviewcla(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 206 opcode 0xcd cinstru
static void cinstru(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 207 opcode 0xce cavftsthit
// Change v40+: cminstru => cavftsthit
static void cavftsthit(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 208 opcode 0xcf cordspr
static void cordspr(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 209 opcode 0xd0 clight
// Change v40+: calign => clight
static void clight(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 210 opcode 0xd1 cbackstar
static void cbackstar(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 211 opcode 0xd2 cstarring
static void cstarring(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 212 opcode 0xd3 cengine
static void cengine(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 213 opcode 0xd4 cautobase
static void cautobase(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 214 opcode 0xd5 cwalkanim
// Change v40+: cquality => cwalkanim
static void cwalkanim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 215 opcode 0xd6 cfwalkanim
// Change v40+: chsprite => cfwalkanim
static void cfwalkanim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 216 opcode 0xd7 cselpalet
static void cselpalet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 217 opcode 0xd8 clinepalet
static void clinepalet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 218 opcode 0xd9 cautomode
static void cautomode(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 219 opcode 0xda cautofile
static void cautofile(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 220 opcode 0xdb cdel
// Change v40+: ccancel => cdel
static void cdel(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 221 opcode 0xdc cobjlink
// Change v40+: ccancall => cobjlink
static void cobjlink(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 222 opcode 0xdd cobjset
// Change v40+: ccancen => cobjset
static void cobjset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 223 opcode 0xde cblast
static void cblast(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 224 opcode 0xdf cscback
static void cscback(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 225 opcode 0xe0 cscrolpage
static void cscrolpage(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 226 opcode 0xe1 cmatent
static void cmatent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 227 opcode 0xe2 cshrink
static void cshrink(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 228 opcode 0xe3 cdefmap
static void cdefmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 229 opcode 0xe4 csetmap
static void csetmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 230 opcode 0xe5 cputmap
static void cputmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 231 opcode 0xe6 csavepal
static void csavepal(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 232 opcode 0xe7 crecalc
// Change v40+: csczoom => crecalc
static void crecalc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 233 opcode 0xe8 cfwalk
// Change v40+: ctexmap => cfwalk
static void cfwalk(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 234 opcode 0xe9 calloctab
static void calloctab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 235 opcode 0xea cfreetab
static void cfreetab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 236 opcode 0xeb cscantab
static void cscantab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 237 opcode 0xec cneartab
static void cneartab(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 238 opcode 0xed cnul
// Change v40+: cscsun => cnul ; cobjcycle
// Calls stub cnul (from old version)

// Codopname no. 239 opcode 0xee cdarkpal
static void cdarkpal(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 240 opcode 0xef cscshade
// Change v40+: cscdark => cscshade
static void cscshade(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 241 opcode 0xf0 caset
static void caset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 242 opcode 0xf1 camov
static void camov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 243 opcode 0xf2 cscaset
static void cscaset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 244 opcode 0xf3 cscamov
static void cscamov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 245 opcode 0xf4 cscfollow
static void cscfollow(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 246 opcode 0xf5 cscview
static void cscview(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 247 opcode 0xf6 cfilm
static void cfilm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 248 opcode 0xf7 cwalk
// Change v40+: cwalkmap => cwalk
static void cwalk(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 249 opcode 0xf8 catstmap
static void catstmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 250 opcode 0xf9 cavtstmov
static void cavtstmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 251 opcode 0xfa cavmov
static void cavmov(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 252 opcode 0xfb caim
static void caim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 253 opcode 0xfc cpointpix
static void cpointpix(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 254 opcode 0xfd cchartmap
static void cchartmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 255 opcode 0xfe cscsky
static void cscsky(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codopname no. 256 opcode 0xff cobjscale
// Change v40+: czoom => cobjscale
static void cobjscale(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// ============================================================================
#pragma mark - Codesc1 routines
// ============================================================================

// Codesc1name no. 001 opcode 0x00 cnul
// Calls stub cnul (from old version)

// Codesc1name no. 002 opcode 0x01 csoundon
static void csoundon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 003 opcode 0x02 csoundoff
static void csoundoff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 004 opcode 0x03 cmusicon
static void cmusicon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 005 opcode 0x04 cmusicoff
static void cmusicoff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 006 opcode 0x05 cdelfilm
static void cdelfilm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 007 opcode 0x06 copenfilm
static void copenfilm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 008 opcode 0x07 cdeffont
// Addition v40+: cdeffont
static void cdeffont(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 009 opcode 0x08 cloadtext
// Addition v40+: cloadtext
static void cloadtext(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 010 opcode 0x09 cafteron
// Addition v40+: cafteron
static void cafteron(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 011 opcode 0x0a cafteroff
// Addition v40+: cafteroff
static void cafteroff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 012 opcode 0x0b cbeforon
// Addition v40+: cbeforon
static void cbeforon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 013 opcode 0x0c cbeforoff
// Addition v40+: cbeforoff
static void cbeforoff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 014 opcode 0x0d cscfocale
// Addition v40+: cscfocale
static void cscfocale(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 015 opcode 0x0e csccamera
// Addition v40+: csccamera
static void csccamera(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 016 opcode 0x0f ctexmap
// Addition v40+: ctexmap
static void ctexmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 017 opcode 0x10 cscrender
// Addition v40+: cscrender
static void cscrender(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 018 opcode 0x11 cvideo
// Addition v40+: cvideo
static void cvideo(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 019 opcode 0x12 ctexture
// Addition v40+: ctexture
static void ctexture(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 020 opcode 0x13 cplaycd
// Addition v40+: cplaycd
static void cplaycd(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 021 opcode 0x14 cnul
// Addition v40+: cnul
// Calls stub cnul (from old version)

// Codesc1name no. 022 opcode 0x15 cobjtrans
// Addition v40+: cobjtrans
static void cobjtrans(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 023 opcode 0x16 conscreen
// Addition v40+: conscreen
static void conscreen(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 024 opcode 0x17 cgetglobal
// Addition v40+: cgetglobal
static void cgetglobal(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 025 opcode 0x18 cgetlocal
// Addition v40+: cgetlocal
static void cgetlocal(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 026 opcode 0x19 cmatitem
// Addition v40+: cmatitem
static void cmatitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 027 opcode 0x1a cmaskitem
// Addition v40+: cmaskitem
static void cmaskitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 028 opcode 0x1b cdefitem
// Addition v40+: cdefitem
static void cdefitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 029 opcode 0x1c cnul
// Addition v40+: cnul
// Calls stub cnul (from old version)

// Codesc1name no. 030 opcode 0x1d cnul
// Addition v40+: cnul
// Calls stub cnul (from old version)

// Codesc1name no. 031 opcode 0x1e cgetmap
// Addition v40+: cgetmap
static void cgetmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 032 opcode 0x1f cconvmap
// Addition v40+: cconvmap
static void cconvmap(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 033 opcode 0x20 csetfilm
// Addition v40+: csetfilm
static void csetfilm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 034 opcode 0x21 cscmem
// Addition v40+: cscmem
static void cscmem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 035 opcode 0x22 cobjorient
// Addition v40+: cobjorient ; =amtstmap
static void cobjorient(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 036 opcode 0x23 cobjaim
// Addition v40+: cobjaim
static void cobjaim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 037 opcode 0x24 cloadpic
// Addition v40+: cloadpic
static void cloadpic(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 038 opcode 0x25 cputpic
// Addition v40+: cputpic
static void cputpic(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 039 opcode 0x26 cfwriteg
// Addition v40+: cfwriteg
static void cfwriteg(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 040 opcode 0x27 cfreadg
// Addition v40+: cfreadg
static void cfreadg(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 041 opcode 0x28 cobjmat
// Addition v40+: cobjmat
static void cobjmat(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 042 opcode 0x29 cobjmask
// Addition v40+: cobjmask
static void cobjmask(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 043 opcode 0x2a cscclut
// Addition v40+: cscclut
static void cscclut(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 044 opcode 0x2b cobjclut
// Addition v40+: cobjclut
static void cobjclut(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 045 opcode 0x2c copennet
// Addition v40+: copennet
static void copennet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 046 opcode 0x2d cclosenet
// Addition v40+: cclosenet
static void cclosenet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 047 opcode 0x2e csendnet
// Addition v40+: csendnet
static void csendnet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 048 opcode 0x2f cmailnet
// Addition v40+: cmailnet
static void cmailnet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 049 opcode 0x30 cplayernet
// Addition v40+: cplayernet
static void cplayernet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 050 opcode 0x31 cgamenet
// Addition v40+: cgamenet
static void cgamenet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 051 opcode 0x32 csyncnet
// Addition v40+: csyncnet
static void csyncnet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 052 opcode 0x33 ctexitem
// Addition v40+: ctexitem
static void ctexitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 053 opcode 0x34 cgetanim
// Addition v40+: cgetanim
static void cgetanim(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 054 opcode 0x35 canimsize
// Addition v40+: canimsize
static void canimsize(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 055 opcode 0x36 cscmath
// Addition v40+: cscmath
static void cscmath(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 056 opcode 0x37 cintermode
// Addition v40+: cintermode
static void cintermode(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 057 opcode 0x38 csysnet
// Addition v40+: csysnet
static void csysnet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 058 opcode 0x39 cobjshad
// Addition v40+: cobjshad
static void cobjshad(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 059 opcode 0x3a csphere
// Addition v40+: csphere
static void csphere(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 060 opcode 0x3b cscdark
// Addition v40+: cscdark
static void cscdark(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 061 opcode 0x3c cagetlocal
// Addition v40+: cagetlocal
static void cagetlocal(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 062 opcode 0x3d cfollow
// Addition v40+: cfollow
static void cfollow(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 063 opcode 0x3e cplayfilm
// Addition v40+: cplayfilm
static void cplayfilm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 064 opcode 0x3f csoundtime
// Addition v40+: csoundtime
static void csoundtime(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 065 opcode 0x40 cinfoitem
// Addition v40+: cinfoitem
static void cinfoitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 066 opcode 0x41 cmorph
// Addition v40+: cmorph
static void cmorph(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 067 opcode 0x42 cscmicro
// Addition v40+: cscmicro
static void cscmicro(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 068 opcode 0x43 cptrset
// Addition v40+: cptrset
static void cptrset(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 069 opcode 0x44 cdefproc
// Addition v40+: cdefproc
static void cdefproc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 070 opcode 0x45 cfreadm
// Addition v40+: cfreadm
static void cfreadm(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 071 opcode 0x46 cfwritem
// Addition v40+: cfwritem
static void cfwritem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 072 opcode 0x47 cmaskent
// Addition v40+: cmaskent
static void cmaskent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 073 opcode 0x48 cvram
// Addition v40+: cvram
static void cvram(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 074 opcode 0x49 cadresnet
// Addition v40+: cadresnet
static void cadresnet(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 075 opcode 0x4a cfdir
// Addition v40+: cfdir
static void cfdir(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 076 opcode 0x4b cnul
// Addition v40+: cnul ; entitem
// Calls stub cnul (from old version)

// Codesc1name no. 077 opcode 0x4c cloadas
// Addition v40+: cloadas
static void cloadas(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 078 opcode 0x4d cfindobjitem
// Addition v40+: cfindobjitem
static void cfindobjitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 079 opcode 0x4e cobjid
// Addition v40+: cobjid
static void cobjid(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 080 opcode 0x4f cgetobjid
// Addition v40+: cgetobjid
static void cgetobjid(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 081 opcode 0x50 cprelight
// Addition v40+: cprelight
static void cprelight(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Codesc1name no. 082 opcode 0x51 croomdoor
// Addition v40+: croomdoor
static void croomdoor(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// =============================================================================
#pragma mark - Oper (Operation) routines
// =============================================================================

// Opername no. 001 opcode 0x00 oimmb
static void oimmb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 002 opcode 0x02 oimmw
static void oimmw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 003 opcode 0x04 oimmp
static void oimmp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 004 opcode 0x06 olocb
static void olocb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 005 opcode 0x08 olocw
static void olocw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 006 opcode 0x0a olocp
static void olocp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 007 opcode 0x0c oloctp
static void oloctp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 008 opcode 0x0e oloctc
static void oloctc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 009 opcode 0x10 olocti
static void olocti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 010 opcode 0x12 odirb
static void odirb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 011 opcode 0x14 odirw
static void odirw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 012 opcode 0x16 odirp
static void odirp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 013 opcode 0x18 odirtp
static void odirtp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 014 opcode 0x1a odirtc
static void odirtc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 015 opcode 0x1c odirti
static void odirti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 016 opcode 0x1e omainb
static void omainb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 017 opcode 0x20 omainw
static void omainw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 018 opcode 0x22 omainp
static void omainp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 019 opcode 0x24 omaintp
static void omaintp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 020 opcode 0x26 omaintc
static void omaintc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 021 opcode 0x28 omainti
static void omainti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 022 opcode 0x2a ohimb
static void ohimb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 023 opcode 0x2c ohimw
static void ohimw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 024 opcode 0x2e ohimp
static void ohimp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 025 opcode 0x30 ohimtp
static void ohimtp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 026 opcode 0x32 ohimtc
static void ohimtc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 027 opcode 0x34 ohimti
static void ohimti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 028 opcode 0x36 opile
static void opile(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 029 opcode 0x38 oeval
// Calls oper->oeval (from old version)

// Opername no. 030 opcode 0x3a ofin
// Calls oper->ofin (from old version)

// Opername no. 031 opcode 0x3c oimml
// Change v40+: pnul => oimml
static void oimml(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 032 opcode 0x3e olocto
// Change v40+: pnul => olocto
static void olocto(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 033 opcode 0x40 opushacc
static void opushacc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 034 opcode 0x42 oand
static void oand(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 035 opcode 0x44 oor
static void oor(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 036 opcode 0x46 oxor
static void oxor(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 037 opcode 0x48 oeqv
static void oeqv(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 038 opcode 0x4a oegal
static void oegal(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 039 opcode 0x4c odiff
static void odiff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 040 opcode 0x4e oinfeg
static void oinfeg(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 041 opcode 0x50 osupeg
static void osupeg(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 042 opcode 0x52 oinf
static void oinf(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 043 opcode 0x54 osup
static void osup(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 044 opcode 0x56 oadd
static void oadd(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 045 opcode 0x58 osub
static void osub(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 046 opcode 0x5a omod
static void omod(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 047 opcode 0x5c odiv
static void odiv(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 048 opcode 0x5e omul
static void omul(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 049 opcode 0x60 oneg
static void oneg(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 050 opcode 0x62 oabs
static void oabs(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 051 opcode 0x64 ornd
static void ornd(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 052 opcode 0x66 osgn
static void osgn(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 053 opcode 0x68 onot
static void onot(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 054 opcode 0x6a oinkey
static void oinkey(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 055 opcode 0x6c okeyon
static void okeyon(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 056 opcode 0x6e ojoy
static void ojoy(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 057 opcode 0x70 oprnd
static void oprnd(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 058 opcode 0x72 oscan
static void oscan(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 059 opcode 0x74 oshiftkey
static void oshiftkey(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 060 opcode 0x76 ofree
static void ofree(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 061 opcode 0x78 omodel
static void omodel(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 062 opcode 0x7a ogetkey
static void ogetkey(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 063 opcode 0x7c oleft
static void oleft(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 064 opcode 0x7e oright
static void oright(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 065 opcode 0x80 omid
static void omid(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 066 opcode 0x82 olen
static void olen(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 067 opcode 0x84 oasc
static void oasc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 068 opcode 0x86 ostr
static void ostr(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 069 opcode 0x88 osadd
static void osadd(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 070 opcode 0x8a osegal
static void osegal(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 071 opcode 0x8c osdiff
static void osdiff(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 072 opcode 0x8e osinfeg
static void osinfeg(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 073 opcode 0x90 ossupeg
static void ossupeg(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 074 opcode 0x92 osinf
static void osinf(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 075 opcode 0x94 ossup
static void ossup(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 076 opcode 0x96 ospushacc
static void ospushacc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 077 opcode 0x98 ospile
static void ospile(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 078 opcode 0x9a oval
static void oval(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 079 opcode 0x9c oexistf
static void oexistf(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 080 opcode 0x9e ochr
static void ochr(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 081 opcode 0xa0 ofpath
// Change v40+: ochange => ofpath
static void ofpath(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 082 opcode 0xa2 ocountry
static void ocountry(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 083 opcode 0xa4 omip
static void omip(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 084 opcode 0xa6 ojoykey
static void ojoykey(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 085 opcode 0xa8 oconfig
static void oconfig(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 086 opcode 0xaa ogettext
// Addition v40+: ogettext
static void ogettext(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 087 opcode 0xac oips
// Addition v40+: oips
static void oips(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 088 opcode 0xae ogetent
// Addition v40+: ogetent
static void ogetent(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 089 opcode 0xb0 osqrt
// Addition v40+: osqrt
static void osqrt(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 090 opcode 0xb2 ocos
// Addition v40+: ocos
static void ocos(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 091 opcode 0xb4 osin
// Addition v40+: osin
static void osin(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 092 opcode 0xb6 otan
// Addition v40+: otan
static void otan(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 093 opcode 0xb8 oatan
// Addition v40+: oatan
static void oatan(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 094 opcode 0xba obufkey
// Addition v40+: obufkey ; bidon...
static void obufkey(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 095 opcode 0xbc omaxitem
// Addition v40+: omaxitem
static void omaxitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 096 opcode 0xbe otick
// Addition v40+: otick
static void otick(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 097 opcode 0xc0 orandom
// Addition v40+: orandom
static void orandom(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 098 opcode 0xc2 oflen
// Addition v40+: oflen
static void oflen(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 099 opcode 0xc4 olocalkey
// Addition v40+: olocalkey
static void olocalkey(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 100 opcode 0xc6 odivi
// Addition v40+: odivi
static void odivi(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 101 opcode 0xc8 oprotect
// Addition v40+: oprotect
static void oprotect(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 102 opcode 0xca otypitem
// Addition v40+: otypitem
static void otypitem(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Opername no. 103 opcode 0xcc oloaded
// Addition v40+: oloaded
static void oloaded(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// ============================================================================
#pragma mark - Store routines
// ============================================================================

// Storename no. 001 opcode 0x00 pnul
// Calls stub pnul (from old version)

// Storename no. 002 opcode 0x02 pnul
// Calls stub pnul (from old version)

// Storename no. 003 opcode 0x04 pnul
// Calls stub pnul (from old version)

// Storename no. 004 opcode 0x06 slocb
static void slocb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 005 opcode 0x08 slocw
static void slocw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 006 opcode 0x0a slocp
static void slocp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 007 opcode 0x0c sloctp
static void sloctp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 008 opcode 0x0e sloctc
static void sloctc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 009 opcode 0x10 slocti
static void slocti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 010 opcode 0x12 slocl
// Change v40+: sdirb => slocl
static void slocl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 011 opcode 0x14 sloctl
// Change v40+: sdirw => sloctl
static void sloctl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 012 opcode 0x16 smainl
// Change v40+: sdirp => smainl
static void smainl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 013 opcode 0x18 smaintl
// Change v40+: sdirtp => smaintl
static void smaintl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 014 opcode 0x1a shiml
// Change v40+: sdirtc => shiml
static void shiml(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 015 opcode 0x1c shimtl
// Change v40+: sdirti => shimtl
static void shimtl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 016 opcode 0x1e smainb
static void smainb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 017 opcode 0x20 smainw
static void smainw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 018 opcode 0x22 smainp
static void smainp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 019 opcode 0x24 smaintp
static void smaintp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 020 opcode 0x26 smaintc
static void smaintc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 021 opcode 0x28 smainti
static void smainti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 022 opcode 0x2a shimb
static void shimb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 023 opcode 0x2c shimw
static void shimw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 024 opcode 0x2e shimp
static void shimp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 025 opcode 0x30 shimtp
static void shimtp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 026 opcode 0x32 shimtc
static void shimtc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 027 opcode 0x34 shimti
static void shimti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 028 opcode 0x36 spile
// Calls store->spile (from old version)

// Storename no. 029 opcode 0x38 seval
static void seval(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Storename no. 030 opcode 0x3a ofin
// Calls oper->ofin (from old version)

// ============================================================================
#pragma mark - Add routines
// ============================================================================

// Addname no. 001 opcode 0x00 pnul
// Calls stub pnul (from old version)

// Addname no. 002 opcode 0x02 pnul
// Calls stub pnul (from old version)

// Addname no. 003 opcode 0x04 pnul
// Calls stub pnul (from old version)

// Addname no. 004 opcode 0x06 alocb
static void alocb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 005 opcode 0x08 alocw
static void alocw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 006 opcode 0x0a alocp
static void alocp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 007 opcode 0x0c aloctp
static void aloctp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 008 opcode 0x0e aloctc
static void aloctc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 009 opcode 0x10 alocti
static void alocti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 010 opcode 0x12 alocl
// Change v40+: adirb => alocl
static void alocl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 011 opcode 0x14 aloctl
// Change v40+: adirw => aloctl
static void aloctl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 012 opcode 0x16 amainl
// Change v40+: adirp => amainl
static void amainl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 013 opcode 0x18 amaintl
// Change v40+: adirtp => amaintl
static void amaintl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 014 opcode 0x1a ahiml
// Change v40+: adirtc => ahiml
static void ahiml(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 015 opcode 0x1c ahimtl
// Change v40+: adirti => ahimtl
static void ahimtl(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 016 opcode 0x1e amainb
static void amainb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 017 opcode 0x20 amainw
static void amainw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 018 opcode 0x22 amainp
static void amainp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 019 opcode 0x24 amaintp
static void amaintp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 020 opcode 0x26 amaintc
static void amaintc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 021 opcode 0x28 amainti
static void amainti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 022 opcode 0x2a ahimb
static void ahimb(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 023 opcode 0x2c ahimw
static void ahimw(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 024 opcode 0x2e ahimp
static void ahimp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 025 opcode 0x30 ahimtp
static void ahimtp(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 026 opcode 0x32 ahimtc
static void ahimtc(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 027 opcode 0x34 ahimti
static void ahimti(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 028 opcode 0x36 spile
// Calls store->spile (from old version)

// Addname no. 029 opcode 0x38 aeval
static void aeval(void) {
    debug(EDebugWarning, "MISSING: %s", __FUNCTION__);
}

// Addname no. 030 opcode 0x3a ofin
// Calls oper->ofin (from old version)

// ============================================================================
#pragma mark - Opcode / Codop (Code-op) routines pointer table
// ============================================================================
sAlisOpcode opcodes_v40[] = {
    DECL_OPCODE(0x00, cnul,         "null"),
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
    DECL_OPCODE(0x0d, cjsrproc,     "TODO: add desc"),   // tcodop_0D: cjsrind16 => cjsrproc
    DECL_OPCODE(0x0e, cstartproc,   "TODO: add desc"),   // tcodop_0E: cjsrind24 => cstartproc
    DECL_OPCODE(0x0f, cjmpproc,     "TODO: add desc"),   // tcodop_0F: cjmpind16 => cjmpproc
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
    DECL_OPCODE(0x20, cadd,         "TODO: add desc"),
    DECL_OPCODE(0x21, csub,         "TODO: add desc"),
    DECL_OPCODE(0x22, cmul,         "[N/I]"),
    DECL_OPCODE(0x23, cdiv,         "[N/I]"),
    DECL_OPCODE(0x24, cvprint,      "TODO: add desc"),
    DECL_OPCODE(0x25, csprinti,     "TODO: add desc"),
    DECL_OPCODE(0x26, csprinta,     "TODO: add desc"),
    DECL_OPCODE(0x27, clocate,      "TODO: add desc"),
    DECL_OPCODE(0x28, ctab,         "TODO: add desc"),
    DECL_OPCODE(0x29, cdim,         "TODO: add desc"),
    DECL_OPCODE(0x2a, cnul,         "TODO: add desc"),   // tcodop_2A: crandom => cnul
    DECL_OPCODE(0x2b, cloop8,       "TODO: add desc"),
    DECL_OPCODE(0x2c, cloop16,      "TODO: add desc"),
    DECL_OPCODE(0x2d, cloop24,      "TODO: add desc"),
    DECL_OPCODE(0x2e, cswitch1,     "TODO: add desc"),
    DECL_OPCODE(0x2f, cswitch2,     "TODO: add desc"),
    DECL_OPCODE(0x30, cstart8,      "TODO: add desc"),
    DECL_OPCODE(0x31, cstart16,     "TODO: add desc"),
    DECL_OPCODE(0x32, cstart24,     "TODO: add desc"),
    DECL_OPCODE(0x33, cstartbase,   "TODO: add desc"),   // tcodop_33: cleave => cstartbase
    DECL_OPCODE(0x34, cnul,         "TODO: add desc"),   // tcodop_34: cprotect => cnul
    DECL_OPCODE(0x35, casleep,      "TODO: add desc"),
    DECL_OPCODE(0x36, cclock,       "TODO: add desc"),
    DECL_OPCODE(0x37, canim,        "TODO: add desc"),   // tcodop_37: cnul => canim
    DECL_OPCODE(0x38, cscmov,       "TODO: add desc"),
    DECL_OPCODE(0x39, cscset,       "TODO: add desc"),
    DECL_OPCODE(0x3a, cvecacopy,    "TODO: add desc"),   // tcodop_3A: cclipping => cvecacopy
    DECL_OPCODE(0x3b, cvecrot,      "TODO: add desc"),   // tcodop_3B: cswitching => cvecrot
    DECL_OPCODE(0x3c, cveclive,     "TODO: add desc"),   // tcodop_3C: cwlive => cveclive
    DECL_OPCODE(0x3d, cunload,      "TODO: add desc"),
    DECL_OPCODE(0x3e, cwakeup,      "TODO: add desc"),
    DECL_OPCODE(0x3f, csleep,       "TODO: add desc"),
    DECL_OPCODE(0x40, clive,        "TODO: add desc"),
    DECL_OPCODE(0x41, ckill,        "TODO: add desc"),
    DECL_OPCODE(0x42, cstop,        "TODO: add desc"),
    DECL_OPCODE(0x43, cvecdistabs,  "TODO: add desc"),   // tcodop_43: cstopret => cvecdistabs
    DECL_OPCODE(0x44, cexit,        "TODO: add desc"),
    DECL_OPCODE(0x45, cload,        "Load and unpack a script, set into vm"),
    DECL_OPCODE(0x46, csctype,      "TODO: add desc"),   // tcodop_46: cdefsc => csctype
    DECL_OPCODE(0x47, cscreen,      "TODO: add desc"),
    DECL_OPCODE(0x48, cput,         "TODO: add desc"),
    DECL_OPCODE(0x49, cputat,       "TODO: add desc"),   // tcodop_49: cputnat => cputat
    DECL_OPCODE(0x4a, cerase,       "TODO: add desc"),
    DECL_OPCODE(0x4b, cobjmov,      "TODO: add desc"),   // tcodop_4B: cerasen => cobjmov
    DECL_OPCODE(0x4c, cset,         "TODO: add desc"),
    DECL_OPCODE(0x4d, cmov,         "TODO: add desc"),
    DECL_OPCODE(0x4e, copensc,      "TODO: add desc"),
    DECL_OPCODE(0x4f, cclosesc,     "TODO: add desc"),
    DECL_OPCODE(0x50, cnul,         "TODO: add desc"),   // tcodop_50: cerasall => cnul
    DECL_OPCODE(0x51, caddput,      "TODO: add desc"),   // tcodop_51: cforme => caddput
    DECL_OPCODE(0x52, caddputat,    "TODO: add desc"),   // tcodop_52: cdelforme => caddputat
    DECL_OPCODE(0x53, ctstmov,      "TODO: add desc"),
    DECL_OPCODE(0x54, ctstset,      "TODO: add desc"),
    DECL_OPCODE(0x55, cftstmov,     "TODO: add desc"),
    DECL_OPCODE(0x56, cftstset,     "TODO: add desc"),
    DECL_OPCODE(0x57, csuccent,     "TODO: add desc"),
    DECL_OPCODE(0x58, cpredent,     "TODO: add desc"),
    DECL_OPCODE(0x59, cnearent,     "TODO: add desc"),
    DECL_OPCODE(0x5a, cneartyp,     "TODO: add desc"),
    DECL_OPCODE(0x5b, cnearmat,     "TODO: add desc"),
    DECL_OPCODE(0x5c, cviewent,     "TODO: add desc"),
    DECL_OPCODE(0x5d, cviewtyp,     "TODO: add desc"),
    DECL_OPCODE(0x5e, cviewmat,     "TODO: add desc"),
    DECL_OPCODE(0x5f, ctstanim,     "TODO: add desc"),   // tcodop_5F: corient => ctstanim
    DECL_OPCODE(0x60, crstent,      "TODO: add desc"),
    DECL_OPCODE(0x61, csend,        "TODO: add desc"),
    DECL_OPCODE(0x62, cscanon,      "TODO: add desc"),
    DECL_OPCODE(0x63, cscanoff,     "TODO: add desc"),
    DECL_OPCODE(0x64, cinteron,     "TODO: add desc"),
    DECL_OPCODE(0x65, cinteroff,    "TODO: add desc"),
    DECL_OPCODE(0x66, cscanclr,     "TODO: add desc"),
    DECL_OPCODE(0x67, callentity,   "TODO: add desc"),
    DECL_OPCODE(0x68, cpalette,     "TODO: add desc"),
    DECL_OPCODE(0x69, cdefcolor,    "TODO: add desc"),
    DECL_OPCODE(0x6a, ctiming,      "TODO: add desc"),
    DECL_OPCODE(0x6b, czap,         "TODO: add desc"),
    DECL_OPCODE(0x6c, cexplode,     "TODO: add desc"),
    DECL_OPCODE(0x6d, cding,        "TODO: add desc"),
    DECL_OPCODE(0x6e, cmodanim,     "TODO: add desc"),   // tcodop_6E: cnoise => cmodanim
    DECL_OPCODE(0x6f, cinitab,      "TODO: add desc"),
    DECL_OPCODE(0x70, cfopen,       "TODO: add desc"),
    DECL_OPCODE(0x71, cfclose,      "TODO: add desc"),
    DECL_OPCODE(0x72, cfcreat,      "TODO: add desc"),
    DECL_OPCODE(0x73, cfdel,        "TODO: add desc"),
    DECL_OPCODE(0x74, cfreadv,      "TODO: add desc"),
    DECL_OPCODE(0x75, cfwritev,     "TODO: add desc"),
    DECL_OPCODE(0x76, cfwritei,     "TODO: add desc"),
    DECL_OPCODE(0x77, cfreadb,      "TODO: add desc"),
    DECL_OPCODE(0x78, cfwriteb,     "TODO: add desc"),
    DECL_OPCODE(0x79, cvecset,      "TODO: add desc"),   // tcodop_79: cplot => cvecset
    DECL_OPCODE(0x7a, cvecmov,      "TODO: add desc"),   // tcodop_7A: cdraw => cvecmov
    DECL_OPCODE(0x7b, cveccopy,     "TODO: add desc"),   // tcodop_7B: cbox => cveccopy
    DECL_OPCODE(0x7c, cvecadd,      "TODO: add desc"),   // tcodop_7C: cboxf => cvecadd
    DECL_OPCODE(0x7d, cink,         "TODO: add desc"),
    DECL_OPCODE(0x7e, cvecsub,      "TODO: add desc"),   // tcodop_7E: cpset => cvecsub
    DECL_OPCODE(0x7f, cvecsize,     "TODO: add desc"),   // tcodop_7F: cpmove => cvecsize
    DECL_OPCODE(0x80, cvecnorm,     "TODO: add desc"),   // tcodop_80: cpmode => cvecnorm
    DECL_OPCODE(0x81, cscworld,     "TODO: add desc"),   // tcodop_81: cpicture => cscworld
    DECL_OPCODE(0x82, cvecdist,     "TODO: add desc"),   // tcodop_82: cxyscroll => cvecdist
    DECL_OPCODE(0x83, clinking,     "TODO: add desc"),
    DECL_OPCODE(0x84, cmouson,      "display mouse cursor"),
    DECL_OPCODE(0x85, cmousoff,     "hide mouse cursor"),
    DECL_OPCODE(0x86, cmouse,       "get mouse status (x, y, buttons) and store"),
    DECL_OPCODE(0x87, cdefmouse,    "TODO: define mouse sprite ???"),
    DECL_OPCODE(0x88, csetmouse,    "set mouse position"),
    DECL_OPCODE(0x89, cobjaset,     "TODO: add desc"),   // tcodop_89: cdefvect => cobjaset
    DECL_OPCODE(0x8a, cobjamov,     "TODO: add desc"),   // tcodop_8A: csetvect => cobjamov
    DECL_OPCODE(0x8b, cnul,         "null"),
    DECL_OPCODE(0x8c, ctsthit,      "TODO: add desc"),   // tcodop_8C: capproach => ctsthit
    DECL_OPCODE(0x8d, cftsthit,     "TODO: add desc"),   // tcodop_8D: cescape => cftsthit
    DECL_OPCODE(0x8e, cvtstmov,     "TODO: add desc"),
    DECL_OPCODE(0x8f, cvftstmov,    "TODO: add desc"),
    DECL_OPCODE(0x90, cvmov,        "TODO: add desc"),
    DECL_OPCODE(0x91, cdefworld,    "TODO: add desc"),
    DECL_OPCODE(0x92, cworld,       "TODO: add desc"),
    DECL_OPCODE(0x93, cfindworld,   "TODO: add desc"),   // tcodop_93: cfindmat => cfindworld
    DECL_OPCODE(0x94, cfindtyp,     "TODO: add desc"),
    DECL_OPCODE(0x95, cmusic,       "TODO: add desc"),
    DECL_OPCODE(0x96, cdelmusic,    "TODO: add desc"),
    DECL_OPCODE(0x97, ccadence,     "TODO: add desc"),
    DECL_OPCODE(0x98, cvolume,      "TODO: add desc"),   // tcodop_98: csetvolum => cvolume
    DECL_OPCODE(0x99, cxinv,        "TODO: add desc"),
    DECL_OPCODE(0x9a, cxinvon,      "TODO: add desc"),
    DECL_OPCODE(0x9b, cxinvoff,     "TODO: add desc"),
    DECL_OPCODE(0x9c, clistent,     "TODO: add desc"),
    DECL_OPCODE(0x9d, csound,       "TODO: add desc"),
    DECL_OPCODE(0x9e, centsound,    "TODO: add desc"),   // tcodop_9E: cmsound => centsound
    DECL_OPCODE(0x9f, credon,       "TODO: add desc"),
    DECL_OPCODE(0xa0, credoff,      "TODO: add desc"),
    DECL_OPCODE(0xa1, cdelsound,    "TODO: add desc"),
    DECL_OPCODE(0xa2, cmatterpix,   "TODO: add desc"),   // tcodop_A2: cwmov => cmatterpix
    DECL_OPCODE(0xa3, ctsthere,     "TODO: add desc"),   // tcodop_A3: cwtstmov => ctsthere
    DECL_OPCODE(0xa4, cftsthere,    "TODO: add desc"),   // tcodop_A4: cwftstmov => cftsthere
    DECL_OPCODE(0xa5, cavftstmov,   "TODO: add desc"),   // tcodop_A5: ctstform => cavftstmov
    DECL_OPCODE(0xa6, cxput,        "TODO: add desc"),
    DECL_OPCODE(0xa7, cxputat,      "TODO: add desc"),
    DECL_OPCODE(0xa8, cmput,        "TODO: add desc"),
    DECL_OPCODE(0xa9, cmputat,      "TODO: add desc"),
    DECL_OPCODE(0xaa, ceput,        "TODO: add desc"),   // tcodop_AA: cmxput => ceput
    DECL_OPCODE(0xab, ceputat,      "TODO: add desc"),   // tcodop_AB: cmxputat => ceputat
    DECL_OPCODE(0xac, cavtsthit,    "TODO: add desc"),   // tcodop_AC: cmmusic => cavtsthit
    DECL_OPCODE(0xad, cputstring,   "TODO: add desc"),   // tcodop_AD: cmforme => cputstring
    DECL_OPCODE(0xae, csettime,     "set current time"),
    DECL_OPCODE(0xaf, cgettime,     "get current time"),
    DECL_OPCODE(0xb0, cvinput,      "TODO: add desc"),
    DECL_OPCODE(0xb1, csinput,      "TODO: add desc"),
    DECL_OPCODE(0xb2, cfreezefar,   "TODO: add desc"),   // tcodop_B2: casleepfar => cfreezefar
    DECL_OPCODE(0xb3, cfaron,       "TODO: add desc"),   // tcodop_B3: casleepon => cfaron
    DECL_OPCODE(0xb4, cfaroff,      "TODO: add desc"),   // tcodop_B4: casleepoff => cfaroff
    DECL_OPCODE(0xb5, crunfilm,     "TODO: add desc"),
    DECL_OPCODE(0xb6, cfreeze,      "TODO: add desc"),   // tcodop_B6: cvpicprint => cfreeze
    DECL_OPCODE(0xb7, cunfreeze,    "TODO: add desc"),   // tcodop_B7: cspicprint => cunfreeze
    DECL_OPCODE(0xb8, cvputprint,   "TODO: add desc"),
    DECL_OPCODE(0xb9, csputprint,   "TODO: add desc"),
    DECL_OPCODE(0xba, cfont,        "TODO: add desc"),
    DECL_OPCODE(0xbb, cpaper,       "TODO: add desc"),
    DECL_OPCODE(0xbc, ctoblack,     "fade-out screen to black"),
    DECL_OPCODE(0xbd, cmovcolor,    "TODO: add desc"),
    DECL_OPCODE(0xbe, ctopalet,     "fade-in screen to palette"),
    DECL_OPCODE(0xbf, cnumput,      "TODO: add desc"),
    DECL_OPCODE(0xc0, cscheart,     "TODO: add desc"),
    DECL_OPCODE(0xc1, cscpos,       "TODO: add desc"),
    DECL_OPCODE(0xc2, cscsize,      "TODO: add desc"),
    DECL_OPCODE(0xc3, cscmode,      "TODO: add desc"),   // tcodop_C3: cschoriz => cscmode
    DECL_OPCODE(0xc4, cscclr,       "TODO: add desc"),   // tcodop_C4: cscvertic => cscclr
    DECL_OPCODE(0xc5, cunloadfar,   "TODO: add desc"),   // tcodop_C5: cscreduce => cunloadfar
    DECL_OPCODE(0xc6, cscscale,     "TODO: add desc"),
    DECL_OPCODE(0xc7, cobjmode,     "TODO: add desc"),   // tcodop_C7: creducing => cobjmode
    DECL_OPCODE(0xc8, cscmap,       "TODO: add desc"),
    DECL_OPCODE(0xc9, cframe,       "TODO: add desc"),   // tcodop_C9: cscdump => cframe
    DECL_OPCODE(0xca, cfindcla,     "TODO: add desc"),
    DECL_OPCODE(0xcb, cnearcla,     "TODO: add desc"),
    DECL_OPCODE(0xcc, cviewcla,     "TODO: add desc"),
    DECL_OPCODE(0xcd, cinstru,      "TODO: add desc"),
    DECL_OPCODE(0xce, cavftsthit,   "TODO: add desc"),   // tcodop_CE: cminstru => cavftsthit
    DECL_OPCODE(0xcf, cordspr,      "TODO: add desc"),
    DECL_OPCODE(0xd0, clight,       "TODO: add desc"),   // tcodop_D0: calign => clight
    DECL_OPCODE(0xd1, cbackstar,    "TODO: add desc"),
    DECL_OPCODE(0xd2, cstarring,    "TODO: add desc"),
    DECL_OPCODE(0xd3, cengine,      "TODO: add desc"),
    DECL_OPCODE(0xd4, cautobase,    "TODO: add desc"),
    DECL_OPCODE(0xd5, cwalkanim,    "TODO: add desc"),   // tcodop_D5: cquality => cwalkanim
    DECL_OPCODE(0xd6, cfwalkanim,   "TODO: add desc"),   // tcodop_D6: chsprite => cfwalkanim
    DECL_OPCODE(0xd7, cselpalet,    "TODO: add desc"),
    DECL_OPCODE(0xd8, clinepalet,   "TODO: add desc"),
    DECL_OPCODE(0xd9, cautomode,    "TODO: add desc"),
    DECL_OPCODE(0xda, cautofile,    "TODO: add desc"),
    DECL_OPCODE(0xdb, cdel,         "TODO: add desc"),   // tcodop_DB: ccancel => cdel
    DECL_OPCODE(0xdc, cobjlink,     "TODO: add desc"),   // tcodop_DC: ccancall => cobjlink
    DECL_OPCODE(0xdd, cobjset,      "TODO: add desc"),   // tcodop_DD: ccancen => cobjset
    DECL_OPCODE(0xde, cblast,       "TODO: add desc"),
    DECL_OPCODE(0xdf, cscback,      "TODO: add desc"),
    DECL_OPCODE(0xe0, cscrolpage,   "TODO: add desc"),
    DECL_OPCODE(0xe1, cmatent,      "TODO: add desc"),
    DECL_OPCODE(0xe2, cshrink,      "Delete bitmap and shift following data"),
    DECL_OPCODE(0xe3, cdefmap,      "TODO: add desc"),
    DECL_OPCODE(0xe4, csetmap,      "TODO: add desc"),
    DECL_OPCODE(0xe5, cputmap,      "TODO: add desc"),
    DECL_OPCODE(0xe6, csavepal,     "TODO: add desc"),
    DECL_OPCODE(0xe7, crecalc,      "TODO: add desc"),   // tcodop_E7: csczoom => crecalc
    DECL_OPCODE(0xe8, cfwalk,       "TODO: add desc"),   // tcodop_E8: ctexmap => cfwalk
    DECL_OPCODE(0xe9, calloctab,    "TODO: add desc"),
    DECL_OPCODE(0xea, cfreetab,     "TODO: add desc"),
    DECL_OPCODE(0xeb, cscantab,     "TODO: add desc"),
    DECL_OPCODE(0xec, cneartab,     "TODO: add desc"),
    DECL_OPCODE(0xed, cnul,         "TODO: add desc"),   // tcodop_ED: cscsun => cnul ; cobjcycle
    DECL_OPCODE(0xee, cdarkpal,     "TODO: add desc"),
    DECL_OPCODE(0xef, cscshade,     "TODO: add desc"),   // tcodop_EF: cscdark => cscshade
    DECL_OPCODE(0xf0, caset,        "TODO: add desc"),
    DECL_OPCODE(0xf1, camov,        "TODO: add desc"),
    DECL_OPCODE(0xf2, cscaset,      "TODO: add desc"),
    DECL_OPCODE(0xf3, cscamov,      "TODO: add desc"),
    DECL_OPCODE(0xf4, cscfollow,    "TODO: add desc"),
    DECL_OPCODE(0xf5, cscview,      "TODO: add desc"),
    DECL_OPCODE(0xf6, cfilm,        "TODO: add desc"),
    DECL_OPCODE(0xf7, cwalk,        "TODO: add desc"),   // tcodop_F7: cwalkmap => cwalk
    DECL_OPCODE(0xf8, catstmap,     "TODO: add desc"),
    DECL_OPCODE(0xf9, cavtstmov,    "TODO: add desc"),
    DECL_OPCODE(0xfa, cavmov,       "TODO: add desc"),
    DECL_OPCODE(0xfb, caim,         "TODO: add desc"),
    DECL_OPCODE(0xfc, cpointpix,    "TODO: add desc"),
    DECL_OPCODE(0xfd, cchartmap,    "TODO: add desc"),
    DECL_OPCODE(0xfe, cscsky,       "TODO: add desc"),
    DECL_OPCODE(0xff, cobjscale,    "TODO: add desc")    // tcodop_FF: czoom => cobjscale
};

// ============================================================================
#pragma mark - Codesc1 routines pointer table
// ============================================================================
sAlisOpcode codesc1names_v40[] = {
    DECL_OPCODE(0x00, cnul,         "null"),
    DECL_OPCODE(0x01, csoundon,     "TODO: add desc"),
    DECL_OPCODE(0x02, csoundoff,    "TODO: add desc"),
    DECL_OPCODE(0x03, cmusicon,     "TODO: add desc"),
    DECL_OPCODE(0x04, cmusicoff,    "TODO: add desc"),
    DECL_OPCODE(0x05, cdelfilm,     "TODO: add desc"),
    DECL_OPCODE(0x06, copenfilm,    "TODO: add desc"),
    DECL_OPCODE(0x07, cdeffont,     "TODO: add desc"),   // tcodesc1_07: + cdeffont
    DECL_OPCODE(0x08, cloadtext,    "TODO: add desc"),   // tcodesc1_08: + cloadtext
    DECL_OPCODE(0x09, cafteron,     "TODO: add desc"),   // tcodesc1_09: + cafteron
    DECL_OPCODE(0x0a, cafteroff,    "TODO: add desc"),   // tcodesc1_0A: + cafteroff
    DECL_OPCODE(0x0b, cbeforon,     "TODO: add desc"),   // tcodesc1_0B: + cbeforon
    DECL_OPCODE(0x0c, cbeforoff,    "TODO: add desc"),   // tcodesc1_0C: + cbeforoff
    DECL_OPCODE(0x0d, cscfocale,    "TODO: add desc"),   // tcodesc1_0D: + cscfocale
    DECL_OPCODE(0x0e, csccamera,    "TODO: add desc"),   // tcodesc1_0E: + csccamera
    DECL_OPCODE(0x0f, ctexmap,      "TODO: add desc"),   // tcodesc1_0F: + ctexmap
    DECL_OPCODE(0x10, cscrender,    "TODO: add desc"),   // tcodesc1_10: + cscrender
    DECL_OPCODE(0x11, cvideo,       "TODO: add desc"),   // tcodesc1_11: + cvideo
    DECL_OPCODE(0x12, ctexture,     "TODO: add desc"),   // tcodesc1_12: + ctexture
    DECL_OPCODE(0x13, cplaycd,      "TODO: add desc"),   // tcodesc1_13: + cplaycd
    DECL_OPCODE(0x14, cnul,         "TODO: add desc"),   // tcodesc1_14: + cnul
    DECL_OPCODE(0x15, cobjtrans,    "TODO: add desc"),   // tcodesc1_15: + cobjtrans
    DECL_OPCODE(0x16, conscreen,    "TODO: add desc"),   // tcodesc1_16: + conscreen
    DECL_OPCODE(0x17, cgetglobal,   "TODO: add desc"),   // tcodesc1_17: + cgetglobal
    DECL_OPCODE(0x18, cgetlocal,    "TODO: add desc"),   // tcodesc1_18: + cgetlocal
    DECL_OPCODE(0x19, cmatitem,     "TODO: add desc"),   // tcodesc1_19: + cmatitem
    DECL_OPCODE(0x1a, cmaskitem,    "TODO: add desc"),   // tcodesc1_1A: + cmaskitem
    DECL_OPCODE(0x1b, cdefitem,     "TODO: add desc"),   // tcodesc1_1B: + cdefitem
    DECL_OPCODE(0x1c, cnul,         "TODO: add desc"),   // tcodesc1_1C: + cnul
    DECL_OPCODE(0x1d, cnul,         "TODO: add desc"),   // tcodesc1_1D: + cnul
    DECL_OPCODE(0x1e, cgetmap,      "TODO: add desc"),   // tcodesc1_1E: + cgetmap
    DECL_OPCODE(0x1f, cconvmap,     "TODO: add desc"),   // tcodesc1_1F: + cconvmap
    DECL_OPCODE(0x20, csetfilm,     "TODO: add desc"),   // tcodesc1_20: + csetfilm
    DECL_OPCODE(0x21, cscmem,       "TODO: add desc"),   // tcodesc1_21: + cscmem
    DECL_OPCODE(0x22, cobjorient,   "TODO: add desc"),   // tcodesc1_22: + cobjorient ; =amtstmap
    DECL_OPCODE(0x23, cobjaim,      "TODO: add desc"),   // tcodesc1_23: + cobjaim
    DECL_OPCODE(0x24, cloadpic,     "TODO: add desc"),   // tcodesc1_24: + cloadpic
    DECL_OPCODE(0x25, cputpic,      "TODO: add desc"),   // tcodesc1_25: + cputpic
    DECL_OPCODE(0x26, cfwriteg,     "TODO: add desc"),   // tcodesc1_26: + cfwriteg
    DECL_OPCODE(0x27, cfreadg,      "TODO: add desc"),   // tcodesc1_27: + cfreadg
    DECL_OPCODE(0x28, cobjmat,      "TODO: add desc"),   // tcodesc1_28: + cobjmat
    DECL_OPCODE(0x29, cobjmask,     "TODO: add desc"),   // tcodesc1_29: + cobjmask
    DECL_OPCODE(0x2a, cscclut,      "TODO: add desc"),   // tcodesc1_2A: + cscclut
    DECL_OPCODE(0x2b, cobjclut,     "TODO: add desc"),   // tcodesc1_2B: + cobjclut
    DECL_OPCODE(0x2c, copennet,     "TODO: add desc"),   // tcodesc1_2C: + copennet
    DECL_OPCODE(0x2d, cclosenet,    "TODO: add desc"),   // tcodesc1_2D: + cclosenet
    DECL_OPCODE(0x2e, csendnet,     "TODO: add desc"),   // tcodesc1_2E: + csendnet
    DECL_OPCODE(0x2f, cmailnet,     "TODO: add desc"),   // tcodesc1_2F: + cmailnet
    DECL_OPCODE(0x30, cplayernet,   "TODO: add desc"),   // tcodesc1_30: + cplayernet
    DECL_OPCODE(0x31, cgamenet,     "TODO: add desc"),   // tcodesc1_31: + cgamenet
    DECL_OPCODE(0x32, csyncnet,     "TODO: add desc"),   // tcodesc1_32: + csyncnet
    DECL_OPCODE(0x33, ctexitem,     "TODO: add desc"),   // tcodesc1_33: + ctexitem
    DECL_OPCODE(0x34, cgetanim,     "TODO: add desc"),   // tcodesc1_34: + cgetanim
    DECL_OPCODE(0x35, canimsize,    "TODO: add desc"),   // tcodesc1_35: + canimsize
    DECL_OPCODE(0x36, cscmath,      "TODO: add desc"),   // tcodesc1_36: + cscmath
    DECL_OPCODE(0x37, cintermode,   "TODO: add desc"),   // tcodesc1_37: + cintermode
    DECL_OPCODE(0x38, csysnet,      "TODO: add desc"),   // tcodesc1_38: + csysnet
    DECL_OPCODE(0x39, cobjshad,     "TODO: add desc"),   // tcodesc1_39: + cobjshad
    DECL_OPCODE(0x3a, csphere,      "TODO: add desc"),   // tcodesc1_3A: + csphere
    DECL_OPCODE(0x3b, cscdark,      "TODO: add desc"),   // tcodesc1_3B: + cscdark
    DECL_OPCODE(0x3c, cagetlocal,   "TODO: add desc"),   // tcodesc1_3C: + cagetlocal
    DECL_OPCODE(0x3d, cfollow,      "TODO: add desc"),   // tcodesc1_3D: + cfollow
    DECL_OPCODE(0x3e, cplayfilm,    "TODO: add desc"),   // tcodesc1_3E: + cplayfilm
    DECL_OPCODE(0x3f, csoundtime,   "TODO: add desc"),   // tcodesc1_3F: + csoundtime
    DECL_OPCODE(0x40, cinfoitem,    "TODO: add desc"),   // tcodesc1_40: + cinfoitem
    DECL_OPCODE(0x41, cmorph,       "TODO: add desc"),   // tcodesc1_41: + cmorph
    DECL_OPCODE(0x42, cscmicro,     "TODO: add desc"),   // tcodesc1_42: + cscmicro
    DECL_OPCODE(0x43, cptrset,      "TODO: add desc"),   // tcodesc1_43: + cptrset
    DECL_OPCODE(0x44, cdefproc,     "TODO: add desc"),   // tcodesc1_44: + cdefproc
    DECL_OPCODE(0x45, cfreadm,      "TODO: add desc"),   // tcodesc1_45: + cfreadm
    DECL_OPCODE(0x46, cfwritem,     "TODO: add desc"),   // tcodesc1_46: + cfwritem
    DECL_OPCODE(0x47, cmaskent,     "TODO: add desc"),   // tcodesc1_47: + cmaskent
    DECL_OPCODE(0x48, cvram,        "TODO: add desc"),   // tcodesc1_48: + cvram
    DECL_OPCODE(0x49, cadresnet,    "TODO: add desc"),   // tcodesc1_49: + cadresnet
    DECL_OPCODE(0x4a, cfdir,        "TODO: add desc"),   // tcodesc1_4A: + cfdir
    DECL_OPCODE(0x4b, cnul,         "TODO: add desc"),   // tcodesc1_4B: + cnul ; entitem
    DECL_OPCODE(0x4c, cloadas,      "TODO: add desc"),   // tcodesc1_4C: + cloadas
    DECL_OPCODE(0x4d, cfindobjitem, "TODO: add desc"),   // tcodesc1_4D: + cfindobjitem
    DECL_OPCODE(0x4e, cobjid,       "TODO: add desc"),   // tcodesc1_4E: + cobjid
    DECL_OPCODE(0x4f, cgetobjid,    "TODO: add desc"),   // tcodesc1_4F: + cgetobjid
    DECL_OPCODE(0x50, cprelight,    "TODO: add desc"),   // tcodesc1_50: + cprelight
    DECL_OPCODE(0x51, croomdoor,    "TODO: add desc")    // tcodesc1_51: + croomdoor
};

// =============================================================================
#pragma mark - Oper (Operation) routines pointer table
// =============================================================================
sAlisOpcode opernames_v40[] = {
    DECL_OPCODE(0x00, oimmb,        "read immediate byte from script, extend to word, copy into r7"),
    {},
    DECL_OPCODE(0x02, oimmw,        "read immediate word from script, copy into r7"),
    {},
    DECL_OPCODE(0x04, oimmp,        "read immediate bytes from script until zero, copy into str1"),
    {},
    DECL_OPCODE(0x06, olocb,        "read offset word from script, extend read byte at vram[offset] to word, copy into r7"),
    {},
    DECL_OPCODE(0x08, olocw,        "read offset word from script, read word at vram[offset], copy into r7"),
    {},
    DECL_OPCODE(0x0a, olocp,        "read offset word from script, read bytes at vram[offset] until zero, copy into str1"),
    {},
    DECL_OPCODE(0x0c, oloctp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x0e, oloctc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x10, olocti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x12, odirb,        "TODO add desc"),
    {},
    DECL_OPCODE(0x14, odirw,        "TODO add desc"),
    {},
    DECL_OPCODE(0x16, odirp,        "TODO add desc"),
    {},
    DECL_OPCODE(0x18, odirtp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x1a, odirtc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x1c, odirti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x1e, omainb,       "TODO add desc"),
    {},
    DECL_OPCODE(0x20, omainw,       "TODO add desc"),
    {},
    DECL_OPCODE(0x22, omainp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x24, omaintp,      "TODO add desc"),
    {},
    DECL_OPCODE(0x26, omaintc,      "TODO add desc"),
    {},
    DECL_OPCODE(0x28, omainti,      "TODO add desc"),
    {},
    DECL_OPCODE(0x2a, ohimb,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2c, ohimw,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2e, ohimp,        "TODO add desc"),
    {},
    DECL_OPCODE(0x30, ohimtp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x32, ohimtc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x34, ohimti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x36, opile,        "copy r6 to r7, pop word form virtual accumulator into r6"),
    {},
    DECL_OPCODE(0x38, oeval,        "starts an expression evaluation loop"),
    {},
    DECL_OPCODE(0x3a, ofin,         "ends an expression evaluation loop"),
    {},
    DECL_OPCODE(0x3c, oimml,        "TODO add desc"),    // toper_3C: pnul => oimml
    {},
    DECL_OPCODE(0x3e, olocto,       "TODO add desc"),    // toper_3E: pnul => olocto
    {},
    DECL_OPCODE(0x40, opushacc,     "push word from r7 into virtual accumulator"),
    {},
    DECL_OPCODE(0x42, oand,         "AND binary operator"),
    {},
    DECL_OPCODE(0x44, oor,          "OR binary operator"),
    {},
    DECL_OPCODE(0x46, oxor,         "XOR binary operator"),
    {},
    DECL_OPCODE(0x48, oeqv,         "EQV binary operator"),
    {},
    DECL_OPCODE(0x4a, oegal,        "is equal test"),
    {},
    DECL_OPCODE(0x4c, odiff,        "is non-equal test"),
    {},
    DECL_OPCODE(0x4e, oinfeg,       "is inferior or equal test"),
    {},
    DECL_OPCODE(0x50, osupeg,       "is superior or equal test"),
    {},
    DECL_OPCODE(0x52, oinf,         "is inferior test"),
    {},
    DECL_OPCODE(0x54, osup,         "is superior test"),
    {},
    DECL_OPCODE(0x56, oadd,         "addition operator"),
    {},
    DECL_OPCODE(0x58, osub,         "substraction operator"),
    {},
    DECL_OPCODE(0x5a, omod,         "modulo operator"),
    {},
    DECL_OPCODE(0x5c, odiv,         "division operator"),
    {},
    DECL_OPCODE(0x5e, omul,         "multiplication operator"),
    {},
    DECL_OPCODE(0x60, oneg,         "negation operator"),
    {},
    DECL_OPCODE(0x62, oabs,         "absolute value operator"),
    {},
    DECL_OPCODE(0x64, ornd,         "get random number"),
    {},
    DECL_OPCODE(0x66, osgn,         "sign test operator"),
    {},
    DECL_OPCODE(0x68, onot,         "binary NOT operator"),
    {},
    DECL_OPCODE(0x6a, oinkey,       "TODO add desc"),
    {},
    DECL_OPCODE(0x6c, okeyon,       "TODO add desc"),
    {},
    DECL_OPCODE(0x6e, ojoy,         "TODO add desc"),
    {},
    DECL_OPCODE(0x70, oprnd,        "TODO add desc"),
    {},
    DECL_OPCODE(0x72, oscan,        "TODO add desc"),
    {},
    DECL_OPCODE(0x74, oshiftkey,    "TODO add desc"),
    {},
    DECL_OPCODE(0x76, ofree,        "TODO add desc"),
    {},
    DECL_OPCODE(0x78, omodel,       "get host device model"),
    {},
    DECL_OPCODE(0x7a, ogetkey,      "TODO add desc"),
    {},
    DECL_OPCODE(0x7c, oleft,        "TODO add desc"),
    {},
    DECL_OPCODE(0x7e, oright,       "TODO add desc"),
    {},
    DECL_OPCODE(0x80, omid,         "TODO add desc"),
    {},
    DECL_OPCODE(0x82, olen,         "TODO add desc"),
    {},
    DECL_OPCODE(0x84, oasc,         "TODO add desc"),
    {},
    DECL_OPCODE(0x86, ostr,         "TODO add desc"),
    {},
    DECL_OPCODE(0x88, osadd,        "TODO add desc"),
    {},
    DECL_OPCODE(0x8a, osegal,       "TODO add desc"),
    {},
    DECL_OPCODE(0x8c, osdiff,       "TODO add desc"),
    {},
    DECL_OPCODE(0x8e, osinfeg,      "TODO add desc"),
    {},
    DECL_OPCODE(0x90, ossupeg,      "TODO add desc"),
    {},
    DECL_OPCODE(0x92, osinf,        "TODO add desc"),
    {},
    DECL_OPCODE(0x94, ossup,        "TODO add desc"),
    {},
    DECL_OPCODE(0x96, ospushacc,    "TODO add desc"),
    {},
    DECL_OPCODE(0x98, ospile,       "TODO add desc"),
    {},
    DECL_OPCODE(0x9a, oval,         "TODO add desc"),
    {},
    DECL_OPCODE(0x9c, oexistf,      "if file name in bssChunk3 exists, returns 0xffff in varD7, else 0"),
    {},
    DECL_OPCODE(0x9e, ochr,         "TODO add desc"),
    {},
    DECL_OPCODE(0xa0, ofpath,       "TODO add desc"),    // toper_A0: ochange => ofpath
    {},
    DECL_OPCODE(0xa2, ocountry,     "TODO add desc"),
    {},
    DECL_OPCODE(0xa4, omip,         "TODO add desc"),
    {},
    DECL_OPCODE(0xa6, ojoykey,      "TODO add desc"),
    {},
    DECL_OPCODE(0xa8, oconfig,      "unknown"),
    {},
    DECL_OPCODE(0xaa, ogettext,     "TODO: add desc"),   // toper_AA: + ogettext
    {},
    DECL_OPCODE(0xac, oips,         "TODO: add desc"),   // toper_AC: + oips
    {},
    DECL_OPCODE(0xae, ogetent,      "TODO: add desc"),   // toper_AE: + ogetent
    {},
    DECL_OPCODE(0xb0, osqrt,        "TODO: add desc"),   // toper_B0: + osqrt
    {},
    DECL_OPCODE(0xb2, ocos,         "TODO: add desc"),   // toper_B2: + ocos
    {},
    DECL_OPCODE(0xb4, osin,         "TODO: add desc"),   // toper_B4: + osin
    {},
    DECL_OPCODE(0xb6, otan,         "TODO: add desc"),   // toper_B6: + otan
    {},
    DECL_OPCODE(0xb8, oatan,        "TODO: add desc"),   // toper_B8: + oatan
    {},
    DECL_OPCODE(0xba, obufkey,      "TODO: add desc"),   // toper_BA: + obufkey ; bidon...
    {},
    DECL_OPCODE(0xbc, omaxitem,     "TODO: add desc"),   // toper_BC: + omaxitem
    {},
    DECL_OPCODE(0xbe, otick,        "TODO: add desc"),   // toper_BE: + otick
    {},
    DECL_OPCODE(0xc0, orandom,      "TODO: add desc"),   // toper_C0: + orandom
    {},
    DECL_OPCODE(0xc2, oflen,        "TODO: add desc"),   // toper_C2: + oflen
    {},
    DECL_OPCODE(0xc4, olocalkey,    "TODO: add desc"),   // toper_C4: + olocalkey
    {},
    DECL_OPCODE(0xc6, odivi,        "TODO: add desc"),   // toper_C6: + odivi
    {},
    DECL_OPCODE(0xc8, oprotect,     "TODO: add desc"),   // toper_C8: + oprotect
    {},
    DECL_OPCODE(0xca, otypitem,     "TODO: add desc"),   // toper_CA: + otypitem
    {},
    DECL_OPCODE(0xcc, oloaded,      "TODO: add desc"),   // toper_CC: + oloaded
    {}
};

// ============================================================================
#pragma mark - Store routines pointer table
// ============================================================================
sAlisOpcode storenames_v40[] = {
    DECL_OPCODE(0x00, pnul,         "null pointer"),
    {},
    DECL_OPCODE(0x02, pnul,         "null pointer"),
    {},
    DECL_OPCODE(0x04, pnul,         "null pointer"),
    {},
    DECL_OPCODE(0x06, slocb,        "store byte from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x08, slocw,        "store word from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x0a, slocp,        "store null-terminated string from r7 at virtual ram location"),
    {},
    DECL_OPCODE(0x0c, sloctp,       "copy bss3 at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x0e, sloctc,       "pop byte from accumulator, store at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x10, slocti,       "pop word from accumulator, store at virtual ram location with offset"),
    {},
    DECL_OPCODE(0x12, slocl,        "TODO add desc"),    // tstore_12: sdirb => slocl
    {},
    DECL_OPCODE(0x14, sloctl,       "TODO add desc"),    // tstore_14: sdirw => sloctl
    {},
    DECL_OPCODE(0x16, smainl,       "TODO add desc"),    // tstore_16: sdirp => smainl
    {},
    DECL_OPCODE(0x18, smaintl,      "TODO add desc"),    // tstore_18: sdirtp => smaintl
    {},
    DECL_OPCODE(0x1a, shiml,        "TODO add desc"),    // tstore_1A: sdirtc => shiml
    {},
    DECL_OPCODE(0x1c, shimtl,       "TODO add desc"),    // tstore_1C: sdirti => shimtl
    {},
    DECL_OPCODE(0x1e, smainb,       "TODO add desc"),
    {},
    DECL_OPCODE(0x20, smainw,       "TODO add desc"),
    {},
    DECL_OPCODE(0x22, smainp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x24, smaintp,      "TODO add desc"),
    {},
    DECL_OPCODE(0x26, smaintc,      "TODO add desc"),
    {},
    DECL_OPCODE(0x28, smainti,      "TODO add desc"),
    {},
    DECL_OPCODE(0x2a, shimb,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2c, shimw,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2e, shimp,        "TODO add desc"),
    {},
    DECL_OPCODE(0x30, shimtp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x32, shimtc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x34, shimti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x36, spile,        "pops word from virtual accumulator into r7"),
    {},
    DECL_OPCODE(0x38, seval,        "starts seval loop"),
    {},
    DECL_OPCODE(0x3a, ofin,         "ends seval loop (calls oper->ofin)"),
    {}
};

// ============================================================================
#pragma mark - Add routines pointer table
// ============================================================================
sAlisOpcode addnames_v40[] = {
    DECL_OPCODE(0x00, pnul,         "null pointer"),
    {},
    DECL_OPCODE(0x02, pnul,         "null pointer"),
    {},
    DECL_OPCODE(0x04, pnul,         "null pointer"),
    {},
    DECL_OPCODE(0x06, alocb,        "reads word (offset) from script, adds byte from d7 to byte at (vram+offset)"),
    {},
    DECL_OPCODE(0x08, alocw,        "reads word (offset) from script, adds word from d7 to word at (vram+offset)"),
    {},
    DECL_OPCODE(0x0a, alocp,        "read word (offset) from script, concatenate null-terminated string at ARRAY_A to null-terminated string located at (vram+offset)"),
    {},
    DECL_OPCODE(0x0c, aloctp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x0e, aloctc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x10, alocti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x12, alocl,        "TODO add desc"),    // tadd_12: adirb => alocl
    {},
    DECL_OPCODE(0x14, aloctl,       "TODO add desc"),    // tadd_14: adirw => aloctl
    {},
    DECL_OPCODE(0x16, amainl,       "TODO add desc"),    // tadd_16: adirp => amainl
    {},
    DECL_OPCODE(0x18, amaintl,      "TODO add desc"),    // tadd_18: adirtp => amaintl
    {},
    DECL_OPCODE(0x1a, ahiml,        "TODO add desc"),    // tadd_1A: adirtc => ahiml
    {},
    DECL_OPCODE(0x1c, ahimtl,       "TODO add desc"),    // tadd_1C: adirti => ahimtl
    {},
    DECL_OPCODE(0x1e, amainb,       "TODO add desc"),
    {},
    DECL_OPCODE(0x20, amainw,       "TODO add desc"),
    {},
    DECL_OPCODE(0x22, amainp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x24, amaintp,      "TODO add desc"),
    {},
    DECL_OPCODE(0x26, amaintc,      "TODO add desc"),
    {},
    DECL_OPCODE(0x28, amainti,      "TODO add desc"),
    {},
    DECL_OPCODE(0x2a, ahimb,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2c, ahimw,        "TODO add desc"),
    {},
    DECL_OPCODE(0x2e, ahimp,        "TODO add desc"),
    {},
    DECL_OPCODE(0x30, ahimtp,       "TODO add desc"),
    {},
    DECL_OPCODE(0x32, ahimtc,       "TODO add desc"),
    {},
    DECL_OPCODE(0x34, ahimti,       "TODO add desc"),
    {},
    DECL_OPCODE(0x36, spile,        "pops word from virtual accumulator into r7 (calls store->spile)"),
    {},
    DECL_OPCODE(0x38, aeval,        "starts aeval loop"),
    {},
    DECL_OPCODE(0x3a, ofin,         "ends aeval loop (calls oper->ofin)"),
    {}
};
