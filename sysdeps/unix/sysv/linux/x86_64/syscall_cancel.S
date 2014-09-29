/* Cancellable syscall wrapper - x86_64 version.
   Copyright (C) 2014 Free Software Foundation, Inc.
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

/* long int [rax] __syscall_cancel_arch (int *cancelhandling [%rdi],
					 long int nr   [%rsi],
					 long int arg1 [%rdx],
					 long int arg2 [%rcx],
					 long int arg3 [%r8],
					 long int arg4 [%r9],
					 long int arg5 [SP+8],
					 long int arg6 [SP+16])  */

ENTRY (__syscall_cancel_arch)

	.globl __syscall_cancel_arch_start
	.type  __syscall_cancel_arch_start,@function
__syscall_cancel_arch_start:

	/* if (*cancelhandling & CANCELED_BITMASK)
	     __syscall_do_cancel()  */
        mov    (%rdi),%eax
	testb  $4, (%rdi)
        jne    __syscall_do_cancel

	/* Issue a 6 argument syscall, the nr [%rax] being the syscall
	   number.  */
        mov    %rdi,%r11
        mov    %rsi,%rax
        mov    %rdx,%rdi
        mov    %rcx,%rsi
        mov    %r8,%rdx
        mov    %r9,%r10
        mov    8(%rsp),%r8
        mov    16(%rsp),%r9
        mov    %r11,8(%rsp)
        syscall

	.globl __syscall_cancel_arch_end
	.type  __syscall_cancel_arch_end,@function
__syscall_cancel_arch_end:

	ret

END (__syscall_cancel_arch)
libc_hidden_def (__syscall_cancel_arch)
