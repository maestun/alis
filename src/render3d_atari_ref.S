/*
 * render3d_atari_ref.S - the doland lift as it was before the 2026-10-04 walk optimisations
 * (tools/asm_lift/ref/), assembled under _ref names. ALIS_RRQ_ASM_DOLAND_VERIFY=2 compares
 * the current asm against it byte for byte (the C reference has drifted from the original).
 */
#include "m68k_asm.h"
#define doland_body_asm doland_body_asm_ref
#define initltra_asm    initltra_asm_ref

#if defined(ALIS_RRQ_ASM_DOLAND) && ALIS_RRQ_ASM_DOLAND && defined(ALIS_RRQ_ASM_DOLAND_VERIFY) && ALIS_RRQ_ASM_DOLAND_VERIFY == 2

/* ---- doland_mirror mirror block (offsets = render3d_atari_glue.h, asserted in glue.c) ---- */
	.equiv	DL_ATLLAND,     0
	.equiv	DL_ATLPIX,      4
	.equiv	DL_ATALTI,      8
	.equiv	DL_ATALIAS,    12
	.equiv	DL_PTRDARK,    16
	.equiv	DL_BASESPRITE, 20
	.equiv	DL_ATEXSPRITE, 24
	.equiv	DL_MAPSCREEN,  28
	.equiv	DL_MEMFIX,     32
	.equiv	DL_CLIPX1,     36
	.equiv	DL_CLIPY2,     38
	.equiv	DL_LANDCLIPY1, 40
	.equiv	DL_LANDCLIPX2, 42
	.equiv	DL_LANDCLIPH,  44
	.equiv	DL_VBARCLIPX2, 46
	.equiv	DL_WLOGY1,     48
	.equiv	DL_WLOGLARG,   50
	.equiv	DL_VDARKW,     52
	.equiv	DL_PRECX,      54
	.equiv	DL_SPRITPROF,  56
	.equiv	DL_SPRITNEXT,  58
	.equiv	DL_SOLH,       60
	.equiv	DL_SOLPIXY,    62
	.equiv	DL_TOPH,       64
	.equiv	DL_TOPPIXY,    66
	.equiv	DL_VBARLARG,   68
	.equiv	DL_VBARX,      70
	.equiv	DL_VBARMID,    72
	.equiv	DL_VBARBOT,    74
	.equiv	DL_FTSTPIX,    76
	.equiv	DL_XTSTPIX,    78
	.equiv	DL_YTSTPIX,    80
	.equiv	DL_CXTSTPIX,   82
	.equiv	DL_CYTSTPIX,   84
	.equiv	DL_CZTSTPIX,   86
	.equiv	DL_DTSTPIX,    88
	.equiv	DL_NTSTPIX,    90
	.equiv	DL_ETSTPIX,    92
	.equiv	DL_IS4BIT,     94
	.equiv	DL_TEX_SAVED_PERSP, 96
	.equiv	DL_TEX_PERSP,  100
	.equiv	DL_TEX_VSTEP,  104
	.equiv	DL_TEX_HSTEP,  108
	.equiv	DL_TEX_V,      112
	.equiv	DL_TEX_BASE,   116
	.equiv	DL_TEX_WMASK,  120
	.equiv	DL_TEX_HMASK,  122
	.equiv	DL_TEX_DARK,   124
	.equiv	DL_SIZE,       128


	.text
	.even

/* ---------------------------------------------------------------------------
 * C-ABI veneer: void doland_body_asm(u8 *scene_ptr, u8 *rc_ptr)
 * Loads A2/A3 (the original's calling convention) and runs the lifted doland.
 * ------------------------------------------------------------------------- */
	.globl	SYM(doland_body_asm)
SYM(doland_body_asm):
	movem.l	d2-d7/a2-a6,-(sp)	| C callee-saved (11 regs, 44 bytes)
	movea.l	48(sp),a2		| scene ptr   (44 + 4 return addr)
	movea.l	52(sp),a3		| rc ptr
	bsr	rrq_doland		| the lifted original (saves/restores all)
	movem.l	(sp)+,d2-d7/a2-a6
	rts

/* ---------------------------------------------------------------------------
 * void initltra_asm(void) — one-time table build (lifted initltra)
 * ------------------------------------------------------------------------- */
	.globl	SYM(initltra_asm)
SYM(initltra_asm):
	movem.l	d0-d7/a0-a6,-(sp)
	bsr	rrq_initltra
	movem.l	(sp)+,d0-d7/a0-a6
	rts

/* ===========================================================================
 * LIFTED CODE
 * Blob A: bars + initltra + line tables/bodies.
 * Blob C: Falcon CD textured (0x0B) fill.
 * Blob B: thunk veneers + doland + dotop (references Blob A's
 * barland/tbarland labels and the Lrrq_* data block).
 * ========================================================================= */

#include "../tools/asm_lift/ref/blob_a_bars.inc.S"
#include "../tools/asm_lift/blob_c_textured.inc.S"
#include "../tools/asm_lift/ref/blob_b_doland.inc.S"

#endif /* ALIS_RRQ_ASM_DOLAND */
