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
#  define JMP_STARTFUNC							      \
    subl $16, %esp
#  define JMP_SYSCALL_CANCEL						      \
    HIDDEN_JUMPTARGET(__syscall_cancel)
#  define JMP_ENDFUNC							      \
    addl $44, %esp;							      \
    cfi_def_cfa_offset (4)
# else
#  define JMP_STARTFUNC							      \
    pushl %ebx;								      \
    cfi_def_cfa_offset (8);						      \
    cfi_offset (ebx, -8);						      \
    SETUP_PIC_REG (bx);							      \
    addl $_GLOBAL_OFFSET_TABLE_, %ebx;					      \
    subl $12, %esp
#  define JMP_SYSCALL_CANCEL \
    __syscall_cancel@plt
#  define JMP_ENDFUNC							      \
    addl $40, %esp;							      \
    cfi_def_cfa_offset (8);						      \
    popl %ebx;								      \
    cfi_restore (ebx);							      \
    cfi_def_cfa_offset (4)
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    cmpl $0, %gs:MULTIPLE_THREADS_OFFSET;				      \
    jne L(pseudo_cancel);						      \
    DO_CALL (syscall_name, args);					      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
    ret;								      \
  L(pseudo_cancel):							      \
    JMP_STARTFUNC;							      \
    cfi_def_cfa_offset (20);						      \
    pushl 40(%esp);							      \
    cfi_def_cfa_offset (24);						      \
    pushl 40(%esp);							      \
    cfi_def_cfa_offset (28);						      \
    pushl 40(%esp);							      \
    cfi_def_cfa_offset (32);						      \
    pushl 40(%esp);							      \
    cfi_def_cfa_offset (36);						      \
    pushl 40(%esp);							      \
    cfi_def_cfa_offset (40);						      \
    pushl 40(%esp);							      \
    cfi_def_cfa_offset (44);						      \
    pushl $SYS_ify (syscall_name);					      \
    cfi_def_cfa_offset (48);						      \
    call JMP_SYSCALL_CANCEL;						      \
    JMP_ENDFUNC;							      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL

# undef PSEUDO_RET
# define PSEUDO_RET

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P cmpl $0, %gs:MULTIPLE_THREADS_OFFSET
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
uintptr_t __pthread_get_pc (const ucontext_t *uc)
{
  return (long int)uc->uc_mcontext.gregs[REG_EIP];
}
#endif
