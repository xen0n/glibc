/* Cancellable syscall wrapper - s390x version.
   Copyright (C) 2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* long int [r2] __syscall_cancel_arch (int *cancelhandling [r2],
					long int nr   [r3],
					long int arg1 [r4],
					long int arg2 [r5],
					long int arg3 [r6],
					long int arg4 [SP+160],
					long int arg5 [SP+168],
					long int arg6 [SP+176])  */

ENTRY (__syscall_cancel_arch)

	/* Save registers and setup stack.  */
	stmg	%r6,%r15,48(%r15)  /* Save registers.  */
	cfi_offset (%r15,-40)
	cfi_offset (%r14,-48)
	cfi_offset (%r13,-56)
	cfi_offset (%r12,-64)
	cfi_offset (%r11,-72)
	cfi_offset (%r10,-80)
	cfi_offset (%r9,-88)
	cfi_offset (%r8,-96)
	cfi_offset (%r7,-104)
	cfi_offset (%r6,-112)
	lgr	%r1,%r15
	lg	%r0,8(%r15)	   /* Load eos.	 */
	aghi	%r15,-160	   /* Buy stack space.	*/
	cfi_adjust_cfa_offset (160)
	stg	%r1,0(%r15)	   /* Store back chain.	 */
	stg	%r0,8(%r15)	   /* Store eos.  */

	.globl __syscall_cancel_arch_start
	.type  __syscall_cancel_arch_start,@function
__syscall_cancel_arch_start:

	/* if (*cancelhandling & CANCELED_BITMASK)
	     __syscall_do_cancel()  */
	tm	3(%r2),4
	jne	1f

	/* Issue a 6 argument syscall  */
	lgr	%r1,%r3		/* Move syscall number.  */
	lgr	%r2,%r4		/* First parameter.  */
	lgr	%r3,%r5		/* Second parameter.  */
	lgr	%r4,%r6		/* Third parameter.  */
	lg	%r5,320(%r15)	/* Fourth parameter.  */
	lg	%r6,328(%r15)	/* Fifth parameter.  */
	lg	%r7,336(%r15)	/* Sixth parameter.  */

	svc	0		/* svc number is always in r1.  */

	.globl __syscall_cancel_arch_end
	.type  __syscall_cancel_arch_end,@function
__syscall_cancel_arch_end:
	lg	%r15,0(%r15)	/* load back chain */
	cfi_adjust_cfa_offset (-160)
	lmg	%r6,%r15,48(%r15) /* Load registers.  */

	br	%r14

	/* Branch to __syscall_do_cancel  */
1:
	lg	%r15,0(%r15)	/* load back chain */
	cfi_adjust_cfa_offset (-160)
	lmg	%r6,%r15,48(%r15) /* Load registers.  */
	jg	__syscall_do_cancel
END (__syscall_cancel_arch)
libc_hidden_def (__syscall_cancel_arch)
