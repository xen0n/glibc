/* Copyright (C) 2015 Free Software Foundation, Inc.
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

/* This testcase checks if there is resource leakage if the syscall has
   returned from kernelspace, but before userspace saves the return
   value.  The 'leaker' thread should be able to close the file
   descriptor if the resource is already allocated, meaning that
   if the cancellation signal arrives *after* the open syscal
   return from kernel, the side-effect should be visible to
   application.  */

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void *
writeopener (void *arg)
{
  int fd;
  for (;;)
    {
      fd = open (arg, O_WRONLY);
      close (fd);
    }
}

void *
leaker (void *arg)
{
  int fd = open (arg, O_RDONLY);
  pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, 0);
  close (fd);
  return 0;
}


#define ITER_COUNT 1000
#define MAX_FILENO 1024

static int
do_test (void)
{
  pthread_t td, bg;
  struct stat st;
  char tmp[] = "/tmp/cancel_race_XXXXXX";
  struct timespec ts;
  int i;
  int ret = 0;

  mktemp (tmp);
  mkfifo (tmp, 0600);
  srand (1);

  pthread_create (&bg, 0, writeopener, tmp);
  for (i = 0; i < ITER_COUNT; i++)
    {
      pthread_create (&td, NULL, leaker, tmp);
      ts.tv_nsec = rand () % 100000;
      ts.tv_sec = 0;
      nanosleep (&ts, NULL);
      pthread_cancel (td);
      pthread_join (td, NULL);
    }

  unlink (tmp);

  for (i = STDERR_FILENO+1; i < MAX_FILENO; i++)
    {
      if (!fstat (i, &st))
	{
	  printf ("leaked fd %d\n", i);
	  ret = 1;
	}
    }
  return ret;
}

#define TEST_FUNCTION do_test ()
#define TIMEOUT 10
#include "../test-skeleton.c"
