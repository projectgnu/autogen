/*  -*- Mode: C -*-
 * --------------------------------------------------------------------
 * compat.h.in --- verbose but portable cpp defines for snprintfv
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
 *
 * Code:  */

#ifndef SNPRINTFV_COMPAT_H
#define SNPRINTFV_COMPAT_H 1

#ifdef __cplusplus
extern "C" {
#define SNV_END_EXTERN_C }
#else
#define SNV_END_EXTERN_C
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#ifdef HAVE_ERRNO_H
#  include <errno.h>
#  ifndef errno
     /* Some sytems #define this! */
     extern int errno;
#  endif
#endif

#if defined( HAVE_LIMITS_H )
#  include <limits.h>

#elif defined( HAVE_SYS_LIMITS_H )
#  include <sys/limits.h>

#elif defined( HAVE_VALUES_H )
#  ifndef MAXINT
#    include <values.h>
#  endif /* MAXINT */
#endif

#if defined( HAVE_STRING_H )
#  include <string.h>

#elif defined( HAVE_STRINGS_H )
#  include <strings.h>
#endif

#if defined( HAVE_MEMORY_H )
#  include <memory.h>
#endif

#if defined( HAVE_INTTYPES_H )
#  include <inttypes.h>

#elif defined( HAVE_STDINT_H )
#  include <stdint.h>
#endif

#ifndef HAVE_UINTMAX_T
#  if defined( HAVE_LONG_LONG )
     typedef long long intmax_t;
     typedef unsigned long long uintmax_t;
#  else
     typedef long intmax_t;
     typedef unsigned long uintmax_t;
#  endif
#endif

#if defined( HAVE_STDARG_H )
#  include <stdarg.h>
#  ifndef   VA_START
#    define VA_START(a, f)  va_start(a, f)
#    define VA_END(a)	    va_end(a)
#  endif /* VA_START */
#  define SNV_USING_STDARG_H
#elif defined( HAVE_VARARGS_H )
#  include <varargs.h>
#  ifndef   VA_START
#    define VA_START(a, f) va_start(a)
#    define VA_END(a)	 va_end(a)
#  endif /* VA_START */
#  undef  SNV_USING_STDARG_H
#else
#  include "must-have-stdarg-or-varargs"
#endif

#define MAX_INT MAXINT
#define MIN_INT MININT

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define textdomain(Domain)
# define _(Text) Text
#endif

/* inline and const keywords are (mostly) handled by config.h */
#ifdef __GNUC__
#  ifndef const
#    define const	__const
#  endif
#  ifndef signed
#    define signed	__signed
#  endif
#  ifndef volatile
#    define volatile	__volatile
#  endif
#else
#  ifndef __STDC__
#    undef  signed
#    define signed
#    undef  volatile
#    define volatile
#  endif
#endif

#ifdef __STDC__
#  define _STR(x)		#x
#  define _CONC(x, y)	x##y
   typedef void *snv_pointer;
   typedef const void *snv_constpointer;
#else
#  define _STR(x)		"x"
#  define _CONC(x, y)	x/**/y
   typedef char *snv_pointer;
   typedef char *snv_constpointer;
#endif

/* Define macros for storing integers inside pointers.
 * Be aware that it is only safe to use these macros to store `int'
 * values in `char*' (or `void*') words, and then extract them later.
 * Although it will work the other way round on many common
 * architectures, it is not portable to assume a `char*' can be
 * stored in an `int' and extracted later without loss of the msb's
 */

#define SNV_POINTER_TO_INT(p)	((long)(p))
#define SNV_POINTER_TO_UINT(p)	((unsigned long)(p))
#define SNV_INT_TO_POINTER(i)	((snv_pointer)(long)(i))
#define SNV_UINT_TO_POINTER(u)	((snv_pointer)(unsigned long)(u))

/* If FALSE is defined, we presume TRUE is defined too.  In this case,
   merely typedef boolean as being int.  Or else, define these all.  */
#ifndef FALSE
/* Do not use `enum boolean': this tag is used in SVR4 <sys/types.h>.  */
typedef enum
{ FALSE = 0, TRUE = 1 }
compatboolean;
#else
typedef int compatboolean;
#endif
#ifndef boolean
#  define boolean compatboolean
#endif

#ifdef __CYGWIN32__
#  ifndef __CYGWIN__
#    define __CYGWIN__
#  endif
#endif
#ifdef __CYGWIN__
#  ifndef _WIN32
#    define _WIN32
#  endif
#endif

#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS  0
#  define EXIT_FAILURE  1
#endif

#ifndef PARAMS
#  if __STDC__
#    ifndef NOPROTOS
#      define PARAMS(args)      args
#    endif
#  endif
#  ifndef PARAMS
#    define PARAMS(args)        ()
#  endif
#endif

#undef SNV_STMT_START
#undef SNV_STMT_END
#if defined (__GNUC__) && !defined (__STRICT_ANSI__) && !defined (__cplusplus)
#  define SNV_STMT_START	(void)(
#  define SNV_STMT_END		)
#else
#  if (defined (sun) || defined (__sun__))
#    define SNV_STMT_START	if (1)
#    define SNV_STMT_END	else (void)0
#  else
#    define SNV_STMT_START	do
#    define SNV_STMT_END	while (0)
#  endif
#endif

#undef SNV_GNUC_PRINTF
#undef SNV_GNUC_NORETURN
#if	__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#  ifdef SNV_USING_STDARG_H
#    define SNV_GNUC_PRINTF( args, format_idx, arg_idx )		\
  	args __attribute__((format (printf, format_idx, arg_idx)))
#  else
#    define SNV_GNUC_PRINTF( args, format_idx, arg_idx ) ()
#  endif
#  define SNV_GNUC_NORETURN						\
	__attribute__((noreturn))
#else /* !__GNUC__ */
#  ifdef SNV_USING_STDARG_H
#    define SNV_GNUC_PRINTF( args, format_idx, arg_idx ) args
#  else
#    define SNV_GNUC_PRINTF( args, format_idx, arg_idx ) ()
#  endif
#  define SNV_GNUC_NORETURN
#endif /* !__GNUC__ */

#define SNV_ASSERT_FMT  "file %s: line %d%s%s%s: assertion \"%s\" failed.\n"
#ifdef __GNUC__
#define SNV_ASSERT_FCN          " (", __PRETTY_FUNCTION__, ")"
#else
#define SNV_ASSERT_FCN          "", "", ""
#endif

#ifndef STR
#  define STR(s) _STR(s)
#endif

#define snv_assert(expr)		snv_fassert(stderr, expr)
#define snv_fassert(stream, expr)   SNV_STMT_START {        \
    if (!(expr))   {                                        \
    fprintf (stream, SNV_ASSERT_FMT, __FILE__, __LINE__,    \
             SNV_ASSERT_FCN, _STR(expr));                   \
    exit(EXIT_FAILURE);                                     \
    }; } SNV_STMT_END

#define return_if_fail(expr)		freturn_if_fail(stderr, expr)
#define freturn_if_fail(expr)       SNV_STMT_START {        \
    if (!(expr))   {                                        \
    fprintf (stream, SNV_ASSERT_FMT, __FILE__, __LINE__,    \
             SNV_ASSERT_FCN, _STR(expr));                   \
    return;                                                 \
    }; } SNV_STMT_END

#define return_val_if_fail(expr, val)	freturn_val_if_fail(stderr, expr, val)
#define freturn_val_if_fail(stream, expr, val)  SNV_STMT_START {    \
    if (!(expr))  {                                                 \
    fprintf (stream, SNV_ASSERT_FMT, __FILE__, __LINE__,            \
             SNV_ASSERT_FCN, _STR(expr));                           \
    return val;                                                     \
    }; } SNV_STMT_END

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)		((a) < 0 ? -(a) : (a))
#endif

typedef SNV_LONG_DOUBLE snv_long_double;

SNV_END_EXTERN_C
#endif /* SNPRINTFV_COMPAT_H
          compat.h ends here  */
