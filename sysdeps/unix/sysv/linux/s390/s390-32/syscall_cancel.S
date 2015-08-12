/* Cancellable syscall wrapper - s390 version.
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
					long int arg4 [SP+96],
					long int arg5 [SP+100],
					long int arg6 [SP+104])  */

ENTRY (__syscall_cancel_arch)
	/* Save registers and setup stack.  */
	stm     %r6,%r15,24(%r15)	/* Save registers */
	cfi_offset (%r15, -36)
	cfi_offset (%r14, -40)
	cfi_offset (%r13, -44)
	cfi_offset (%r12, -48)
	cfi_offset (%r11, -52)
	cfi_offset (%r10, -56)
	cfi_offset (%r9, -60)
	cfi_offset (%r8, -64)
	cfi_offset (%r7, -68)
	cfi_offset (%r6, -72)
	lr      %r1,%r15
	l       %r0,4(0,%r15)		/* Load eos */
	ahi     %r15,-96		/* Buy stack space */
	cfi_adjust_cfa_offset (96)
	st      %r1,0(0,%r15)		/* Store back chain */
	st      %r0,4(0,%r15)		/* Store eos */

	.globl __syscall_cancel_arch_start
	.type  __syscall_cancel_arch_start,@function
__syscall_cancel_arch_start:

	/* if (*cancelhandling & CANCELED_BITMASK)
	     __syscall_do_cancel()  */
	tm	3(%r2),4
	jne	1f

	/* Issue a 6 argument syscall  */
	lr	%r1,%r3			/* Move syscall number.  */
	lr	%r2,%r4			/* First parameter.  */
	lr	%r3,%r5			/* Second parameter.  */
	lr	%r4,%r6			/* Third parameter.  */
	l	%r5,192(%r15)		/* Fourth parameter.  */
	l	%r6,196(%r15)		/* Fifth parameter.  */
	l	%r7,200(%r15)		/* Sixth parameter.  */

	svc	0			/* svc number is always in r1.  */
	.globl __syscall_cancel_arch_end
	.type  __syscall_cancel_arch_end,@function
__syscall_cancel_arch_end:
	l	%r15,0(%r15)		/* Load back chain.  */
	cfi_adjust_cfa_offset (-96)
	lm	%r6,15,24(%r15)		/* Load registers.  */

	br	%r14

	/* Branch to __syscall_do_cancel  */
1:
	l	%r15,0(%r15)		/* Load back chain.  */
	cfi_adjust_cfa_offset (-96)
	lm	%r6,%r15,24(%r15)	/* Load registers.  */
	jg	__syscall_do_cancel
END (__syscall_cancel_arch)
libc_hidden_def (__syscall_cancel_arch)
