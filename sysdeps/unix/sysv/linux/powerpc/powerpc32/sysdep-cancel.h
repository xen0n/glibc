/* Cancellable system call stubs.  Linux/PowerPC version.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Franz Sirl <Franz.Sirl-kernel@lauterbach.com>, 2003.

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

# if !IS_IN (libc)
#  define SETUP_PIC							\
    bcl     20,31,got_label;						\
got_label:

#  define CANCEL_JUMPTARGET						\
    stw   r30,8(r1);							\
    mflr  r30;								\
    addis r30,r30,_GLOBAL_OFFSET_TABLE_-got_label@ha;			\
    addi  r30,r30,_GLOBAL_OFFSET_TABLE_-got_label@l;			\
    bl    __syscall_cancel@plt;						\
    lwz   r30,8(r1)
# else
#  define SETUP_PIC
#  if defined SHARED && defined PIC
#   define CANCEL_JUMPTARGET						\
    bl    __GI___syscall_cancel@locaL
#  else
#   define CANCEL_JUMPTARGET						\
    bl    __syscall_cancel
#  endif
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
  ENTRY (name)								\
    SINGLE_THREAD_P;							\
    bne- L(pseudo_cancel);						\
    DO_CALL (SYS_ify (syscall_name));					\
    bnslr+;								\
    b __syscall_error@local;						\
  L(pseudo_cancel):							\
    stwu r1,-16(r1);							\
    cfi_adjust_cfa_offset (16);						\
    mflr r0;								\
    SETUP_PIC;								\
    stw  r0,20(r1);							\
    cfi_offset (lr, 4);							\
    mr   r9,r8;								\
    mr   r8,r7;								\
    mr   r7,r6;								\
    mr   r6,r5;								\
    mr   r5,r4;								\
    mr   r4,r3;								\
    li   r3,SYS_ify (syscall_name);					\
    CANCEL_JUMPTARGET;							\
    lwz  r0,20(r1);							\
    addi r1,r1,16;							\
    cfi_adjust_cfa_offset (-16);					\
    mtlr r0;								\
    cfi_restore (lr);							\
    b    __syscall_cancel_error@local;

# undef PSEUDO_RET
# define PSEUDO_RET

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P                                               \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,                         \
                                   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P                                               \
  lwz 10,MULTIPLE_THREADS_OFFSET(2);                                    \
  cmpwi 10,0
# endif

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)

static inline
uintptr_t __pthread_get_pc (const ucontext_t *uc)
{
  return uc->uc_mcontext.uc_regs->gregs[PT_NIP];
}
#endif
