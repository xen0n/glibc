/* The tunable framework.  See the README to know how to use the tunable in
   a glibc module.

   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <libc-internal.h>

#define TUNABLES_INTERNAL 1
#include "dl-tunables.h"

#define GLIBC_TUNABLES "GLIBC_TUNABLES"

/* Compare environment names, bounded by the name hardcoded in glibc.  */
static bool
is_name (const char *orig, const char *envname)
{
  for (;*orig != '\0' && *envname != '\0'; envname++, orig++)
    if (*orig != *envname)
      break;

  /* The ENVNAME is immediately followed by a value.  */
  if (*orig == '\0' && *envname == '=')
    return true;
  else
    return false;
}

static char *tunables_strdup (const char *in)
{
  size_t i = 0;

  while (in[i++]);

  char *out = __mmap (NULL, ALIGN_UP (i, __getpagesize ()),
		      PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1,
		      0);

  if (out == MAP_FAILED)
    return NULL;

  i--;

  while (i-- > 0)
    out[i] = in[i];

  return out;
}

static char **
get_next_env (char **envp, char **name, size_t *namelen, char **val)
{
  while (envp != NULL && *envp != NULL)
    {
      char *envline = *envp;
      int len = 0;

      while (envline[len] != '\0' && envline[len] != '=')
	len++;

      /* Just the name and no value, go to the next one.  */
      if (envline[len] == '\0')
	continue;

      *name = envline;
      *namelen = len;
      *val = &envline[len + 1];

      return ++envp;
    }

  return NULL;
}

/* If the value does not validate, don't bother initializing the internal type
   and also take care to clear the recorded string value in STRVAL.  */
#define RETURN_IF_INVALID_RANGE(__cur, __val) \
({									      \
  __typeof ((__cur)->type.min) min = (__cur)->type.min;			      \
  __typeof ((__cur)->type.max) max = (__cur)->type.max;			      \
  if (min != max && ((__val) < min || (__val) > max))			      \
    return;								      \
})

/* Validate range of the input value and initialize the tunable CUR if it looks
   good.  */
static void
tunable_initialize (tunable_t *cur, const char *strval)
{
  switch (cur->type.type_code)
    {
    case TUNABLE_TYPE_INT_32:
	{
	  int32_t val = (int32_t) __strtoul_internal (strval, NULL, 0, 0);
	  RETURN_IF_INVALID_RANGE (cur, val);
	  cur->val.numval = (int64_t) val;
	  cur->strval = strval;
	  break;
	}
    case TUNABLE_TYPE_SIZE_T:
	{
	  size_t val = (size_t) __strtoul_internal (strval, NULL, 0, 0);
	  RETURN_IF_INVALID_RANGE (cur, val);
	  cur->val.numval = (int64_t) val;
	  cur->strval = strval;
	  break;
	}
    case TUNABLE_TYPE_STRING:
	{
	  cur->val.strval = cur->strval = strval;
	  break;
	}
    default:
      __builtin_unreachable ();
    }
}

static void
parse_tunables (char *tunestr)
{
  if (tunestr == NULL || *tunestr == '\0')
    return;

  char *p = tunestr;

  while (true)
    {
      char *name = p;
      size_t len = 0;

      /* First, find where the name ends.  */
      while (p[len] != '=' && p[len] != ':' && p[len] != '\0')
	len++;

      /* If we reach the end of the string before getting a valid name-value
	 pair, bail out.  */
      if (p[len] == '\0')
	return;

      /* We did not find a valid name-value pair before encountering the
	 colon.  */
      if (p[len]== ':')
	{
	  p += len + 1;
	  continue;
	}

      p += len + 1;

      char *value = p;
      len = 0;

      while (p[len] != ':' && p[len] != '\0')
	len++;

      char end = p[len];
      p[len] = '\0';

      /* Add the tunable if it exists.  */
      for (size_t i = 0; i < sizeof (tunable_list) / sizeof (tunable_t); i++)
	{
	  tunable_t *cur = &tunable_list[i];

	  /* If we are in a secure context (AT_SECURE) then ignore the tunable
	     unless it is explicitly marked as secure.  Tunable values take
	     precendence over their envvar aliases.  */
	  if (__libc_enable_secure && !cur->is_secure)
	    continue;

	  if (is_name (cur->name, name))
	    {
	      tunable_initialize (cur, value);
	      break;
	    }
	}

      if (end == ':')
	p += len + 1;
      else
	return;
    }
}

/* Initialize the tunables list from the environment.  For now we only use the
   ENV_ALIAS to find values.  Later we will also use the tunable names to find
   values.  */
void
__tunables_init (char **envp)
{
  char *envname = NULL;
  char *envval = NULL;
  size_t len = 0;

  while ((envp = get_next_env (envp, &envname, &len, &envval)) != NULL)
    {
      if (is_name (GLIBC_TUNABLES, envname))
	{
	  char *val = tunables_strdup (envval);
	  if (val != NULL)
	    parse_tunables (val);
	  continue;
	}

      for (int i = 0; i < sizeof (tunable_list) / sizeof (tunable_t); i++)
	{
	  tunable_t *cur = &tunable_list[i];

	  /* Skip over tunables that have either been set already or should be
	     skipped.  */
	  if (cur->strval != NULL || cur->env_alias == NULL
	      || (__libc_enable_secure && !cur->is_secure))
	    continue;

	  const char *name = cur->env_alias;

	  /* We have a match.  Initialize and move on to the next line.  */
	  if (is_name (name, envname))
	    {
	      tunable_initialize (cur, envval);
	      break;
	    }
	}
    }
}

/* Set the tunable value.  This is called by the module that the tunable exists
   in. */
void
__tunable_set_val (tunable_id_t id, void *valp, tunable_callback_t callback)
{
  tunable_t *cur = &tunable_list[id];

  /* Don't do anything if our tunable was not set during initialization or if
     it failed validation.  */
  if (cur->strval == NULL)
    return;

  if (valp == NULL)
    goto cb;

  switch (cur->type.type_code)
    {
    case TUNABLE_TYPE_INT_32:
	{
	  *((int32_t *) valp) = (int32_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_SIZE_T:
	{
	  *((size_t *) valp) = (size_t) cur->val.numval;
	  break;
	}
    case TUNABLE_TYPE_STRING:
	{
	  *((const char **)valp) = cur->val.strval;
	  break;
	}
    default:
      __builtin_unreachable ();
    }

cb:
  if (callback)
    callback (&cur->val);
}
