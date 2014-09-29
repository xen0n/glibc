/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    SINGLE_THREAD_P;							      \
    jne L(pseudo_cancel);						      \
    DO_CALL (syscall_name, args);					      \
    cmpq $-4095, %rax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
    ret;								      \
  L(pseudo_cancel):							      \
    subq  $24, %rsp;							      \
    cfi_def_cfa_offset (32);						      \
    movq  %r9, (%rsp);							      \
    movq  %r8, %r9;							      \
    movq  %rcx, %r8;							      \
    movq  %rdx, %rcx;							      \
    movq  %rsi, %rdx;							      \
    movq  %rdi, %rsi;							      \
    lea   SYS_ify (syscall_name), %edi;					      \
    call  JMP_SYSCALL_CANCEL;						      \
    cfi_def_cfa_offset (8);						      \
    addq  $24, %rsp;							      \
    cmpq  $-4095, %rax;							      \
    jae SYSCALL_ERROR_LABEL;

# if IS_IN (libpthread)
#  define __local_multiple_threads __pthread_multiple_threads
# elif IS_IN (libc)
#  define __local_multiple_threads __libc_multiple_threads
# elif !IS_IN (librt)
#  error Unsupported library
# endif

# if IS_IN (libpthread) || IS_IN (libc)
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P \
	  __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P cmpl $0, __local_multiple_threads(%rip)
#  endif

# else

#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P \
	  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
					   header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P cmpl $0, %fs:MULTIPLE_THREADS_OFFSET
#  endif /* __ASSEMBLER  */

# endif /* IS_IN (libpthread) || IS_IN (libc)  */

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)

static inline
uintptr_t __pthread_get_pc (const ucontext_t *uc)
{
  return (long int)uc->uc_mcontext.gregs[REG_RIP];
}
#endif
