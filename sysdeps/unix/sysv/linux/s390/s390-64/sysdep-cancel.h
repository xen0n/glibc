/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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
#endif

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

# if IS_IN (libc)
#  define JMP_SYSCALL_CANCEL HIDDEN_JUMPTARGET(__syscall_cancel)
# else
#  define JMP_SYSCALL_CANCEL __syscall_cancel@plt
# endif

# define STORE_0 /* Nothing */
# define STORE_1 /* Nothing */
# define STORE_2 /* Nothing */
# define STORE_3 /* Nothing */
# define STORE_4 stg %r6,48(%r15);		\
 cfi_offset (%r6,-112);
# define STORE_5 STORE_4
# define STORE_6 STORE_4

# define LOAD_0 /* Nothing */
# define LOAD_1 /* Nothing */
# define LOAD_2 /* Nothing */
# define LOAD_3 /* Nothing */
# define LOAD_4 lg %r6,48(%r15);
# define LOAD_5 LOAD_4
# define LOAD_6 LOAD_4

# define MOVE_ARGS_0
# define MOVE_ARGS_1 lgr %r3,%r2;		\
	 MOVE_ARGS_0
# define MOVE_ARGS_2 lgr %r4,%r3;		\
	 MOVE_ARGS_1
# define MOVE_ARGS_3 lgr %r5,%r4;		\
	 MOVE_ARGS_2
# define MOVE_ARGS_4 lgr %r6,%r5;		\
	 MOVE_ARGS_3
# define MOVE_ARGS_5 stg %r6,160(%r15);		\
	 MOVE_ARGS_4
# define MOVE_ARGS_6 lg %r14,160(%r14);		\
	 stg %r14,168(%r15);			\
	 MOVE_ARGS_5


# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
	.text;								      \
L(pseudo_cancel):							      \
	cfi_startproc;							      \
	stmg	%r14,%r15,112(%r15);					      \
	cfi_offset (%r15,-40);						      \
	cfi_offset (%r14,-48);						      \
	STORE_##args							      \
	lgr	%r14,%r15;						      \
	aghi	%r15,-176;						      \
	cfi_adjust_cfa_offset (176);					      \
	stg	%r14,0(%r15);						      \
	MOVE_ARGS_##args						      \
	lghi	%r2,SYS_ify (syscall_name);				      \
	brasl	%r14,JMP_SYSCALL_CANCEL;				      \
	lmg	%r14,%r15,112+176(%r15);			      	      \
	cfi_restore (%r14);						      \
	cfi_restore (%r15);						      \
	LOAD_##args							      \
	cfi_endproc;							      \
	j	L(pseudo_check);					      \
ENTRY(name)								      \
	SINGLE_THREAD_P							      \
	jne	L(pseudo_cancel);					      \
	DO_CALL(syscall_name, args);					      \
L(pseudo_check):							      \
	lghi	%r4,-4095;						      \
	clgr	%r2,%r4;						      \
	jgnl	SYSCALL_ERROR_LABEL;					      \
L(pseudo_end):

# if IS_IN (libpthread)
#  define __local_multiple_threads	__pthread_multiple_threads
# elif IS_IN (libc)
#  define __local_multiple_threads	__libc_multiple_threads
# elif !IS_IN (librt)
#  error Unsupported library
# endif

# if IS_IN (libpthread) || IS_IN (libc)
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P \
  __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P \
	larl	%r1,__local_multiple_threads;				      \
	icm	%r0,15,0(%r1);
#  endif

# else

#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P \
	ear	%r1,%a0;						      \
	sllg	%r1,%r1,32;						      \
	ear	%r1,%a1;						      \
	icm	%r1,15,MULTIPLE_THREADS_OFFSET(%r1);
#  endif

# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)

static inline
uintptr_t __pthread_get_pc (const struct ucontext *uc)
{
  return uc->uc_mcontext.psw.addr;
}
#endif
