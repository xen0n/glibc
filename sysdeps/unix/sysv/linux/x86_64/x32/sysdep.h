/* Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

#ifndef _LINUX_X32_SYSDEP_H
#define _LINUX_X32_SYSDEP_H 1

#ifndef __ASSEMBLER__
#include <libc-internal.h>

typedef long long int __syscall_arg_t;

/* Syscall arguments for x32 follows x86_64 size, however pointers are 32
   bits in size.  This suppress the GCC warning "cast from pointer to 
   integer of different size" when calling __syscall_cancel with
   pointer as arguments.  */
# define __SSC(__x)						\
  ({								\
    __syscall_arg_t __ret;					\
    DIAG_PUSH_NEEDS_COMMENT;					\
    DIAG_IGNORE_NEEDS_COMMENT (4.7, "-Wpointer-to-int-cast");	\
    __ret = (sizeof(1 ? (__x) : 0ULL) < 8 ?			\
      (unsigned long int) (__x) : (long long int) (__x));	\
    DIAG_POP_NEEDS_COMMENT;					\
    __ret;							\
  })
#endif

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/x86_64/sysdep.h>
#include <sysdeps/x86_64/x32/sysdep.h>

#endif /* linux/x86_64/x32/sysdep.h */
