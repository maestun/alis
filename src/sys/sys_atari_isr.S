//
// Timer C calldown and MFP Timer A handlers for Atari (TOS / MiNT).
//
// Hooks etv_timer ($400, vector 0x100), called from TOS's Timer C ISR
// (documented 200 Hz, 50 Hz observed; the divider is calibrated at install).
// We dispatch into a tiny C helper that runs the divider, a recursion guard,
// and itroutine() itself — the same function the original Atari interpreter
// ran from VBL.
//
// Chain to the previous handler at the end via the "push old vector +
// RTS" tail-jmp idiom so GEMDOS bookkeeping (_hz_200, time-of-day,
// mouse keyclick) keeps running.
//
// Calling convention: etv_timer is invoked by TOS as
//     move.w  timerc_count, -(sp)
//     jsr     (etv_timer)
// so we must NOT touch the stacked word.
//
// dpalet is s32 16.16 fixed-point, so this handler does no FPU work and
// needs no fsave/frestore. Caller-saved m68k regs (d0/d1/a0/a1) get
// preserved across the JSR.
//
// etv_term ($408) cleanup is handled in C (atari_etv_term_handler in
// sys_atari.c / sys_sdl1.c) — the install must be a user-mode Setexc, not
// inside Supexec, otherwise MiNT's per-process p_term entry isn't
// updated and our handler is invisible to do_exit.
//

// Gate body on __m68k__ so non-Atari builds (Mac/Linux SDL1 desktop)
// can include this file in their target without the assembler choking
// on m68k mnemonics.
#if defined(__m68k__)

#include "m68k_asm.h"

	.text
	.globl	SYM(atari_timerc_trampoline)
	.globl	SYM(atari_timera_trampoline)
	.globl	SYM(atari_old_etv_timer)
	.globl	SYM(atari_old_etv_term)
	.extern	SYM(atari_timerc_c_callback)
	.extern	SYM(atari_timera_c_callback)

// MFP Timer A interrupt handler (vector $134). Unlike Timer C (which
// chains TOS's etv_timer), Timer A is a free MFP channel we own outright,
// so this is a real interrupt: save scratch regs, call the C feed, RTE.
// The C callback issues the MFP software-end-of-interrupt (writes ISRA).
SYM(atari_timera_trampoline):
	lea	-16(sp), sp
	movem.l	d0-d1/a0-a1, (sp)
	jsr	SYM(atari_timera_c_callback)
	movem.l	(sp), d0-d1/a0-a1
	lea	16(sp), sp
	rte

// ColdFire V4e movem.l only supports (An) — not -(An)/(An)+. Allocate
// the frame manually so the same code works on classic 68k AND ColdFire.
//
// ColdFire also forbids memory-to-memory MOVE in most addressing modes,
// including `move.l absolute, -(sp)` which would normally tail-chain
// the old vector. We emulate the push via a register scratch routed
// through 4(sp), keeping every operand a register-or-(An,disp) move.
SYM(atari_timerc_trampoline):
	lea	-16(sp), sp
	movem.l	d0-d1/a0-a1, (sp)
	jsr	SYM(atari_timerc_c_callback)
	movem.l	(sp), d0-d1/a0-a1
	lea	16(sp), sp

	// Push old etv_timer onto the stack and rts to chain.
	// d0 round-trip avoids the mem-to-(sp) form ColdFire rejects.
	lea	-8(sp), sp
	move.l	d0, (sp)
	move.l	SYM(atari_old_etv_timer), d0
	move.l	d0, 4(sp)
	move.l	(sp), d0
	addq.l	#4, sp
	rts

	.bss
	.even
SYM(atari_old_etv_timer):
	.long	0
SYM(atari_old_etv_term):
	.long	0

#endif /* __m68k__ */
