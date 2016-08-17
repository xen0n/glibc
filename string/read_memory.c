/* Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <string.h>

/* This function is an optimization fence.  It doesn't do anything
   itself, but calls to it prevent calls to explicit_bzero from being
   optimized away.  In order to achieve this effect, this function
   must never, under any circumstances, be inlined or subjected to
   inter-procedural optimization.  string.h declares this function
   with attributes that, in conjunction with the no-op asm insert, are
   sufficient to prevent problems in the current (2016) generation of
   compilers, but *only if* this file is *not* compiled with -flto.
   At present, this is not an issue since glibc is never compiled with
   -flto, but should that ever change, this file must be excepted.

   The 'volatile' below is technically not necessary but is included
   for explicitness.  */

void
internal_function
__internal_glibc_read_memory(const void *s, size_t len)
{
  asm volatile ("");
}
libc_hidden_def (__internal_glibc_read_memory)
strong_alias (__internal_glibc_read_memory, __glibc_read_memory)
