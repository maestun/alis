// sys_atari_mix.S — hot part of the 8-bit sample mixer (sys.c sys_audio_callback_S8).
//
// int mix_run_s8(s8 *buf, int bi, int len, u8 adv, const s16 *vt, const s8 *addr, u32 end,
//                u32 ratio, u32 *accp, int *smpidxp, u32 *playedp, s32 *s0p)
//
// Same contract as sys.c MIX_RUN: mixes from buf[bi] until the buffer is full or the next source
// step would reach `end`; returns the index it stopped at, state as at the top of that iteration.
// gcc spills this loop's state to the stack; here it all stays in registers.
#if defined(__m68k__) && !defined(__mcoldfire__)

#include "m68k_asm.h"

	.text
	.globl	SYM(mix_run_s8)
	.globl	SYM(mix_run_s8_store)

| args after the movem (8 regs) + return address
	.equ	A_BUF, 36
	.equ	A_BI, 40
	.equ	A_LEN, 44
	.equ	A_ADV, 48
	.equ	A_VT, 52
	.equ	A_ADDR, 56
	.equ	A_END, 60
	.equ	A_RATIO, 64
	.equ	A_ACC, 68
	.equ	A_SMP, 72
	.equ	A_PLAYED, 76
	.equ	A_S0, 80

| One output sample. d0 acc, d2 played, d3 smpidx, d4 ratio, d5 s0, d6 end, a0 out, a2 vt, a3 addr.
| store = the buffer is still zero: write s0 >> 8 (kept in d7) without reading the buffer back.
.macro MIX_SAMPLE stereo, store
	move.l	d0,d1
	clr.w	d1
	swap	d1			| add = acc >> 16
	beq.s	1f
	add.l	d3,d1
	cmp.l	d6,d1
	jbcc	Lmix_done		| loop / splice / end: the caller's loop body takes it
	move.l	d1,d3
	moveq	#0,d1
	move.b	0(a3,d3.l),d1
	add.w	d1,d1
	move.w	0(a2,d1.w),d5		| s0 = vt[sample]
.if \store
	move.w	d5,d7
	asr.w	#8,d7
.endif
1:
.if \store
	move.b	d7,(a0)+
.if \stereo
	move.b	d7,(a0)+
.endif
.else
	move.b	(a0),d1
	lsl.w	#8,d1
	add.w	d5,d1
	asr.w	#8,d1
	move.b	d1,(a0)+
.if \stereo
	move.b	d1,(a0)+
.endif
.endif
	addq.l	#1,d2
	and.l	#0xffff,d0
	add.l	d4,d0
.endm

.macro MIX_LOOP label, stereo, store
\label:
	MIX_SAMPLE \stereo, \store
	cmpa.l	a1,a0
	bcs.s	\label
	jbra	Lmix_done
.endm

SYM(mix_run_s8_store):
	movem.l	d2-d7/a2-a3,-(sp)
	moveq	#1,d7
	bra.s	Lmix_start
SYM(mix_run_s8):
	movem.l	d2-d7/a2-a3,-(sp)
	moveq	#0,d7
Lmix_start:
	movea.l	A_BUF(sp),a0
	adda.l	A_BI(sp),a0
	movea.l	A_VT(sp),a2
	movea.l	A_ADDR(sp),a3
	move.l	A_END(sp),d6
	move.l	A_RATIO(sp),d4
	movea.l	A_ACC(sp),a1
	move.l	(a1),d0
	movea.l	A_SMP(sp),a1
	move.l	(a1),d3
	movea.l	A_PLAYED(sp),a1
	move.l	(a1),d2
	movea.l	A_S0(sp),a1
	move.l	(a1),d5
	movea.l	A_BUF(sp),a1
	adda.l	A_LEN(sp),a1		| end of buffer
	cmpa.l	a1,a0
	jbcc	Lmix_done
	cmpi.l	#1,A_ADV(sp)
	sne	d1			| d1 = stereo
	tst.w	d7
	beq.s	1f
	move.w	d5,d7
	asr.w	#8,d7
	tst.b	d1
	jbne	Lmix_stereo_st
	jbra	Lmix_mono_st
1:	tst.b	d1
	jbne	Lmix_stereo
	MIX_LOOP Lmix_mono, 0, 0
	MIX_LOOP Lmix_stereo, 1, 0
	MIX_LOOP Lmix_mono_st, 0, 1
	MIX_LOOP Lmix_stereo_st, 1, 1
Lmix_done:
	movea.l	A_ACC(sp),a1
	move.l	d0,(a1)
	movea.l	A_SMP(sp),a1
	move.l	d3,(a1)
	movea.l	A_PLAYED(sp),a1
	move.l	d2,(a1)
	movea.l	A_S0(sp),a1
	ext.l	d5
	move.l	d5,(a1)
	move.l	a0,d0
	sub.l	A_BUF(sp),d0		| buffer index
	movem.l	(sp)+,d2-d7/a2-a3
	rts

#endif
