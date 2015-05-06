/* Copyright (C) 2003-2016 Free Software Foundation, Inc.

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
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
# include <sys/ucontext.h>
#endif

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

# if IS_IN (libc)
#  define JMP_SYSCALL_CANCEL HIDDEN_JUMPTARGET(__syscall_cancel)
# else
#  define JMP_SYSCALL_CANCEL __syscall_cancel
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
ENTRY (name);								\
	SINGLE_THREAD_P(16);						\
	cbnz	w16, L(pseudo_cancel);					\
	DO_CALL (syscall_name, args);					\
	b	L(pseudo_finish);					\
L(pseudo_cancel):							\
	stp     x29, x30, [sp, -16]!;					\
	cfi_def_cfa_offset (16);					\
	cfi_offset (29, -16);						\
	cfi_offset (30, -8);						\
	add	x29, sp, 0;						\
	cfi_def_cfa_register (29);					\
	mov	x6, x5;							\
	mov	x5, x4;							\
	mov	x4, x3;							\
	mov	x3, x2;							\
	mov	x2, x1;							\
	mov	x1, x0;							\
	mov	x0, SYS_ify (syscall_name);				\
	bl	JMP_SYSCALL_CANCEL;					\
	ldp     x29, x30, [sp], 16;					\
	cfi_restore (30);                                               \
	cfi_restore (29);						\
	cfi_def_cfa (31, 0);						\
L(pseudo_finish):							\
        cmn     x0, 4095;                                               \
        b.cs    L(syscall_error);

# undef PSEUDO_END
# define PSEUDO_END(name)						\
	SYSCALL_ERROR_HANDLER;						\
	cfi_endproc;							\
	.size	name, .-name;

# if IS_IN (libpthread)
#  define __local_multiple_threads __pthread_multiple_threads
# elif IS_IN (libc)
#  define __local_multiple_threads __libc_multiple_threads
# endif

# if IS_IN (libpthread) || IS_IN (libc)
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P(R)						\
	adrp	x##R, __local_multiple_threads;				\
	ldr	w##R, [x##R, :lo12:__local_multiple_threads]
#  endif
# else
/*  There is no __local_multiple_threads for librt, so use the TCB.  */
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P(R)						\
	mrs     x##R, tpidr_el0;					\
	sub	x##R, x##R, PTHREAD_SIZEOF;				\
	ldr	w##R, [x##R, PTHREAD_MULTIPLE_THREADS_OFFSET]
#  endif
# endif

#elif !defined __ASSEMBLER__

/* For rtld, et cetera.  */
# define SINGLE_THREAD_P 1
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)

static inline
uintptr_t __pthread_get_pc (const struct ucontext *uc)
{
  return uc->uc_mcontext.pc;
}
#endif
