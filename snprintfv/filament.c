/*  -*- Mode: C -*-  */

/* filament.c --- a bit like a string, but different =)O|
 * Copyright (C) 1999 Gary V. Vaughan
 * Originally by Gary V. Vaughan, 1999
 * This file is part of Snprintfv
 *
 * Snprintfv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Snprintfv program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * As a special exception to the GNU General Public License, if you
 * distribute this file as part of a program that also links with and
 * uses the libopts library from AutoGen, you may include it under
 * the same distribution terms used by the libopts library.
 */

/* Commentary:
 *
 * Try to exploit usage patterns to optimise string handling, and
 * as a happy consequence handle NUL's embedded in strings properly.
 *
 * o Since finding the length of a (long) string is time consuming and
 *   requires careful coding to cache the result in local scope: We
 *   keep count of the length of a Filament all the time, so finding the
 *   length is O(1) at the expense of a little bookkeeping while
 *   manipulating the Filament contents.
 *
 * o Constantly resizing a block of memory to hold a string is memory
 *   efficient, but slow:  Filaments are only ever expanded in size,
 *   doubling at each step to minimise the number of times the block
 *   needs to be reallocated and the contents copied (this problem is
 *   especially poignant for very large strings).
 *
 * o Most strings tend to be either relatively small and short-lived,
 *   or else long-lived but growing in asymptotically in size: To
 *   care for the former case, Filaments start off with a modest static
 *   buffer for the string contents to avoid any mallocations (except
 *   the initial one to get the structure!); the latter case is handled
 *   gracefully by the resizing algorithm in the previous point.
 *
 * o Extracting a C-style NUL terminated string from the Filament is
 *   an extremely common operation:  We ensure there is always a
 *   terminating NUL character after the last character in the string
 *   so that the conversion can be performed quickly.
 *
 * In summary, Filaments are good where you need to do a lot of length
 * calculations with your strings and/or gradually append more text
 * onto existing strings.  Filaments are also an easy way to get 8-bit
 * clean strings is a more lightweight approach isn't required.
 *
 * They probably don't buy much if you need to do insertions and partial
 * deletions, but optimising for that is a whole other problem!
 */

/* Code: */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WITH_DMALLOC
#  include <dmalloc.h>
#endif

#include "mem.h"
#include "filament.h"

#ifndef FILAMENT_BUFSIZ
#  define FILAMENT_BUFSIZ	512
#endif

struct filament
{
  char *value;			/* pointer to the start of the string */
  size_t length;		/* length of the string */
  size_t size;			/* total memory allocated */
  char buffer[FILAMENT_BUFSIZ];	/* usually string == &buffer[0] */
};


/* Save the overhead of a function call in the great majority of cases. */
#define FIL_MAYBE_EXTEND(fil, len, copy)  \
  do { \
    if ((len)>=(fil)->size) { fil_maybe_extend((fil), (len), (copy)); } \
  } while (0)

static inline void fil_maybe_extend PARAMS ((Filament * fil, size_t len, boolean copy));


/**
 * filnew: constructor
 * @init: address of the first byte to copy into the new object.
 * @len:  the number of bytes to copy into the new object.
 *
 * Create a new Filament object, initialised to hold a copy of the
 * first @len bytes starting at address @init.  If @init is NULL, or
 * @len is 0 (or less), then the initialised Filament will return the
 * empty string, "", if its value is queried.
 *
 * Return value:
 * A newly created Filament object is returned.
 **/
Filament *
filnew (init, len)
     const char *const init;
     size_t len;
{
  Filament *new;

  new = snv_new (Filament, 1);

  new->value = new->buffer;
  new->length = 0;
  new->size = FILAMENT_BUFSIZ;

  return (init || len) ? filinit (new, init, len) : new;
}

/**
 * filinit:
 * @fil: The Filament object to initialise.
 * @init: address of the first byte to copy into the new object.
 * @len:  the number of bytes to copy into the new object.
 *
 * Initialise a Filament object to hold a copy of the first @len bytes
 * starting at address @init.  If @init is NULL, or @len is 0 (or less),
 * then the Filament will be reset to hold the empty string, "".
 *
 * Return value:
 * The initialised Filament object is returned.
 **/
Filament *
filinit (fil, init, len)
     Filament *fil;
     const char *const init;
     size_t len;
{
  if (init == NULL || len < 1)
    {
      /* Recycle any dynamic memory assigned to the previous
         contents of @fil, and point back into the static buffer. */
      if (fil->value != fil->buffer)
	snv_delete (fil->value);

      fil->value = fil->buffer;
      fil->length = 0;
      fil->size = FILAMENT_BUFSIZ;
    }
  else
    {
      if (len < FILAMENT_BUFSIZ)
	{
	  /* We have initialisation data which will easily fit into
	     the static buffer: recycle any memory already assigned
	     and initialise in the static buffer. */
	  if (fil->value != fil->buffer)
	    {
	      snv_delete (fil->value);
	      fil->value = fil->buffer;
	      fil->size = FILAMENT_BUFSIZ;
	    }
	}
      else
	{
	  /* If we get to here then we never try to shrink the already
	     allocated dynamic buffer (if any), we just leave it in
	     place all ready to expand into later... */
	  FIL_MAYBE_EXTEND (fil, len, FALSE);
	}

      snv_assert (len < fil->size);

      fil->length = len;
      memcpy (fil->value, init, len);
    }

  return fil;
}

/**
 * fildelete: destructor
 * @fil: The Filament object for recycling.
 *
 * The memory being used by @fil is recycled.
 *
 * Return value:
 * The original contents of @fil are converted to a null terminated
 * string which is returned, either to be freed itself or else used
 * as a normal C string.  The entire Filament contents are copied into
 * this string including any embedded nulls.
 **/
char *
fildelete (fil)
     Filament *fil;
{
  char *value;

  if (fil->value == fil->buffer)
    {
      value = memcpy (snv_new (char, 1 + fil->length),
		      fil->buffer, 1 + fil->length);
      value[fil->length] = '\0';
    }
  else
    value = filval (fil);

  snv_delete (fil);

  return value;
}

/**
 * filval:
 * @fil: The Filament object being queried.
 *
 * Return value:
 * A pointer to the internal null terminated string held by the Filament
 * object is returned.  Since the @fil may contain embedded nulls, it
 * is not entirely safe to use the strfoo() API to examine the contents
 * of the return value.
 **/
char *
filval (fil)
     Filament *fil;
{
  /* Because we have been careful to ensure there is always at least
     one spare byte of allocated memory, it is safe to set it here. */
  fil->value[fil->length] = '\0';
  return fil->value;
}

/**
 * fillen:
 * @fil: The Filament object being queried.
 * 
 * Return value:
 * The length of @fil, including any embedded nulls, but excluding the
 * terminating null, is returned.
 **/
size_t
fillen (fil)
     Filament *fil;
{
  return fil->length;
}

/**
 * filelt:
 * @fil: The Filament being queried.
 * @n: A zero based index into @fil.
 * 
 * This function looks for the @n'th element of @fil.
 * 
 * Return value:
 * If @n is an index inside the Filament @fil, then the character stored
 * at that index cast to an int is returned, otherwise @n is outside
 * this range and -1 is returned.
 **/
int
filelt (fil, n)
     Filament *fil;
     size_t n;
{
  return ((n >= 0) && (n < fil->length)) ? (int) fil->value[n] : -1;
}

/**
 * filcat:
 * @fil: The destination Filament of the concatenation.
 * @str: The address of the source bytes for concatenation.
 * 
 * The bytes starting at address @str upto and including the first null
 * byte encountered are destructively concatenated to @fil.  If
 * necessary @fil is dynamically reallocated to make room for this
 * operation.
 * 
 * Return value: 
 * A pointer to the null terminated string (along with any embedded nulls)
 * which is the result of this concatenation is returned.
 **/
char *
filcat (fil, str)
     Filament *fil;
     const char *str;
{
  return filncat (fil, str, strlen (str));
}

/**
 * filncat:
 * @fil: The destination Filament of the concatenation.
 * @str: The address of the source bytes for concatenation.
 * @n: The number of bytes to be copied from @str.
 * 
 * @n bytes starting with the byte at address @str are destructively
 * concatenated to @fil.  If necessary, @fil is dynamically reallocated
 * to make room for this operation.
 * 
 * Return value:
 * A pointer to the null terminated string (along with any embedded nulls)
 * which is the result of this concatenation is returned.
 **/
char *
filncat (fil, str, n)
     Filament *fil;
     const char *str;
     size_t n;
{
  FIL_MAYBE_EXTEND (fil, n + fil->length, TRUE);

  memcpy (fil->value + fil->length, str, n);
  fil->length += n;

  return fil->value;
}

/**
 * filccat:
 * @fil: The destination Filament of the concatenation.
 * @c: The character to append to @fil.
 * 
 * @c is destructively concatenated to @fil.  If necessary, @fil is
 * dynamically reallocated to make room for this operation.  When used
 * repeatedly this function is considerably less efficient than %filncat(),
 * since it must maintain the %'\0' suffix for C string compatibility
 * after every appended character, and then overwrite that %'\0' on the
 * next call.
 * 
 * Return value:
 * A pointer to the null terminated string (along with any embedded nulls)
 * which is the result of this concatenation is returned.
 **/
char *
filccat (fil, c)
     Filament *fil;
     int c;
{
  FIL_MAYBE_EXTEND (fil, 1 + fil->length, TRUE);

  fil->value[fil->length++] = (char) c;

  return fil->value;
}

/* 
 * fil_maybe_extend
 * @fil: The Filament object which may need more string space.
 * @len: The length of the data to be stored in @fil.
 * @copy: whether to copy data from the static buffer on reallocation.
 *
 * This function will examine the difference between the amount of
 * space in @fil, and @len, the length required for the prospective
 * contents, and if necessary will assign a bigger block of memory
 * to @fil.
 */
static void
fil_maybe_extend (fil, len, copy)
     Filament *fil;
     size_t len;
     boolean copy;
{
  /* Only extend the buffer if there really isn't enough room for
     a string of length @len at all. */

  /* if (len >= fil->size) -- this is in the FIL_MAYBE_EXTEND macro now */
  {
    /* Usually we will simply double the amount of space previously
       allocated, but if the extra data is larger than the current
       size it *still* won't fit, so in that case we allocate enough
       room plus some conservative extra space to expand into. */
    fil->size += MAX (len, fil->size);
    if (fil->value == fil->buffer)
      {
	fil->value = snv_new (char, fil->size);
	if (copy)
	  memcpy (fil->value, fil->buffer, fil->length);
      }
    else
      fil->value = snv_renew (char, fil->value, fil->size);
  }
}

/* Filament.c ends here */
