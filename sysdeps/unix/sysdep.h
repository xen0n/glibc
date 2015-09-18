/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

#include <sysdeps/generic/sysdep.h>

#include <sys/syscall.h>
#define	HAVE_SYSCALLS

#ifndef __ASSEMBLER__
# include <errno.h>

/* Note that using a `PASTE' macro loses.  */
#define	SYSCALL__(name, args)	PSEUDO (__##name, name, args)
#define	SYSCALL(name, args)	PSEUDO (name, name, args)

/* Cancellation macros.  */
#ifndef __SSC
typedef long int __syscall_arg_t;
# define __SSC(__x) ((__syscall_arg_t) (__x))
#endif

long int __syscall_cancel (__syscall_arg_t nr, __syscall_arg_t arg1,
			   __syscall_arg_t arg2, __syscall_arg_t arg3,
			   __syscall_arg_t arg4, __syscall_arg_t arg5,
			   __syscall_arg_t arg6);
libc_hidden_proto (__syscall_cancel);

#define __SYSCALL0(name) \
  (__syscall_cancel)(__NR_##name, 0, 0, 0, 0, 0, 0)
#define __SYSCALL1(name, a1) \
  (__syscall_cancel)(__NR_##name, __SSC(a1), 0, 0, 0, 0, 0)
#define __SYSCALL2(name, a1, a2) \
  (__syscall_cancel)(__NR_##name, __SSC(a1), __SSC(a2), 0, 0, 0, 0)
#define __SYSCALL3(name, a1, a2, a3) \
  (__syscall_cancel)(__NR_##name, __SSC(a1), __SSC(a2), __SSC(a3), 0, 0, 0)
#define __SYSCALL4(name, a1, a2, a3, a4) \
  (__syscall_cancel)(__NR_##name, __SSC(a1), __SSC(a2), __SSC(a3), \
		     __SSC(a4), 0, 0)
#define __SYSCALL5(name, a1, a2, a3, a4, a5) \
  (__syscall_cancel)(__NR_##name, __SSC(a1), __SSC(a2), __SSC(a3), \
		     __SSC(a4), __SSC(a5), 0)
#define __SYSCALL6(name, a1, a2, a3, a4, a5, a6) \
  (__syscall_cancel)(__NR_##name, __SSC(a1), __SSC(a2), __SSC(a3), \
		     __SSC(a4), __SSC(a5), __SSC(a6))
/* FIXME: syscall with 7-arguments.  */
#define __SYSCALL7(name, a1, a2, a3, a4, a5, a6, a7) \
  (__syscall_cancel)(__NR_##name, __SSC(a1), __SSC(a2), __SSC(a3), \
		     __SSC(a4), __SSC(a5), __SSC(a6))

#define __SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SYSCALL_NARGS(...) \
  __SYSCALL_NARGS_X (__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b)     a##b
#define __SYSCALL_CONCAT(a,b)       __SYSCALL_CONCAT_X (a, b)
#define __SYSCALL_DISP(b,...) \
  __SYSCALL_CONCAT (b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __SYSCALL_CALL(...) __SYSCALL_DISP (__SYSCALL, __VA_ARGS__)

#define SYSCALL_CANCEL_NCS(name, nr, args...) \
  __SYSCALL_CALL (name, nr, args)

#define SYSCALL_CANCEL(...) \
  ({									\
    long int sc_ret = __SYSCALL_CALL (__VA_ARGS__);			\
    if (SYSCALL_CANCEL_ERROR (sc_ret))					\
      {									\
        __set_errno (SYSCALL_CANCEL_ERRNO (sc_ret));			\
        sc_ret = -1L;							\
      }									\
    sc_ret;								\
  })

#endif

/* Machine-dependent sysdep.h files are expected to define the macro
   PSEUDO (function_name, syscall_name) to emit assembly code to define the
   C-callable function FUNCTION_NAME to do system call SYSCALL_NAME.
   r0 and r1 are the system call outputs.  MOVE(x, y) should be defined as
   an instruction such that "MOVE(r1, r0)" works.  ret should be defined
   as the return instruction.  */

#ifndef SYS_ify
#define SYS_ify(syscall_name) SYS_##syscall_name
#endif

/* Terminate a system call named SYM.  This is used on some platforms
   to generate correct debugging information.  */
#ifndef PSEUDO_END
#define PSEUDO_END(sym)
#endif
#ifndef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(sym)	PSEUDO_END(sym)
#endif
#ifndef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(sym)	PSEUDO_END(sym)
#endif

/* Wrappers around system calls should normally inline the system call code.
   But sometimes it is not possible or implemented and we use this code.  */
#ifndef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) __syscall_##name (args)
#endif
