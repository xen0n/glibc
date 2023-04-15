/* LoongArch old-world compatibility shims.
   Copyright (C) 2023 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <fcntl.h>
#include <kernel_stat.h>
#include <sysdep.h>
#include <shlib-compat.h>

#if OTHER_SHLIB_COMPAT (ld, GLIBC_2_0, GLIBC_2_37)

int
attribute_compat_text_section
__loongarch_ow___xstat (int vers, const char *name, struct stat *buf)
{
  if (vers == _STAT_VER_KERNEL)
    return __fstatat (AT_FDCWD, name, buf, 0);
  return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);
}

compat_symbol (ld, __loongarch_ow___xstat, __xstat, GLIBC_2_27);

#endif
