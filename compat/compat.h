/*  -*- Mode: C -*-  */

/* --- fake the preprocessor into handlng portability */

/*
 *  Time-stamp:      "2002-09-21 13:38:29 bkorb"
 *
 * Author:           Gary V Vaughan <gvaughan@oranda.demon.co.uk>
 * Created:          Mon Jun 30 15:54:46 1997
 *
 * $Id: compat.h,v 3.5 2003/04/19 02:40:33 bkorb Exp $
 */
#ifndef COMPAT_H
#define COMPAT_H 1

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN extern
#endif

#undef STATIC

#ifdef DEBUG
#  define STATIC
#else
#  define STATIC static
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

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#else
#  error NEED <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#else
#  error NEED <sys/stat.h>
#endif

#ifdef STDC_HEADERS
#  ifndef HAVE_STRING_H
#     define HAVE_STRING_H  1
#  endif
#  ifndef HAVE_STDLIB_H
#     define HAVE_STDLIB_H  1
#  endif
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#else
#  error NEED <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#   include <errno.h>
#else
#  error NEED <errno.h>
#endif

#ifdef HAVE_MEMORY_H
#  include <memory.h>
#else
#  error NEED <memory.h>
#endif

/* some systems #def errno! and others do not declare it!! */
#ifndef errno
   extern       int     errno;
#endif

#include <stdio.h>
#include <ctype.h>

/* Some machines forget this! */

# ifndef EXIT_FAILURE
#   define EXIT_SUCCESS 0
#   define EXIT_FAILURE 1
# endif

#ifndef NUL
#  define NUL '\0'
#endif

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#elif HAVE_SYS_LIMITS_H
#  include <sys/limits.h>
#endif

#if !defined (MAXPATHLEN) && defined (HAVE_SYS_PARAM_H)
#  include <sys/param.h>
#endif /* !MAXPATHLEN && HAVE_SYS_PARAM_H */

#if !defined (MAXPATHLEN) && defined (PATH_MAX)
#  define MAXPATHLEN PATH_MAX
#endif /* !MAXPATHLEN && PATH_MAX */

#if !defined (MAXPATHLEN)
#  define MAXPATHLEN 1024
#endif /* MAXPATHLEN */

# ifndef LONG_MAX
#   define LONG_MAX     ~(1L << (8*sizeof(long) -1))
#   define INT_MAX      ~(1 << (8*sizeof(int) -1))
#   define SHORT_MAX    ~(1 << (8*sizeof(short) -1))
# endif

# ifndef ULONG_MAX
#   define ULONG_MAX    ~(OUL)
#   define UINT_MAX     ~(OU)
#   define USHORT_MAX   ~(OUS)
# endif

/* redefine these for BSD style string libraries */
#ifndef HAVE_STRCHR
#  define strchr        index
#  define strrchr       rindex
#endif

#if HAVE_STRING_H
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#else
   extern char *strchr(), *strrchr();
#endif

#if defined(HAVE_LIBGEN) && defined(HAVE_LIBGEN_H)
#  include <libgen.h>
#endif

#ifndef HAVE_PATHFIND
  EXTERN char *pathfind PARAMS((const char *, const char *, const char *));
#endif

#ifndef NULL
#  define NULL 0
#endif

#ifndef FOPEN_BINARY_FLAG
#  ifdef USE_FOPEN_BINARY
#    define FOPEN_BINARY_FLAG   "b"
#  else
#    define FOPEN_BINARY_FLAG
#  endif
#endif

#ifndef FOPEN_TEXT_FLAG
#  ifdef USE_TEXT_BINARY
#    define FOPEN_TEXT_FLAG     "t"
#  else
#    define FOPEN_TEXT_FLAG
#  endif
#endif

#ifndef STR
#  define _STR(s) #s
#  define STR(s)  _STR(s)
#endif

/* ##### Pointer sized word ##### */

/* FIXME:  the MAX stuff in here is broken! */
#if SIZEOF_CHARP > SIZEOF_INT
   typedef long t_word;
   #define WORD_MAX  LONG_MAX
   #define WORD_MIN  LONG_MIN
#else /* SIZEOF_CHARP <= SIZEOF_INT */
   typedef int t_word;
   #define WORD_MAX  INT_MAX
   #define WORD_MIN  INT_MIN
#endif

# if defined (_POSIX_SOURCE)
/* Posix does not require that the d_ino field be present, and some
   systems do not provide it. */
#    define REAL_DIR_ENTRY(dp) 1
# else /* !_POSIX_SOURCE */
#    define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
# endif /* !_POSIX_SOURCE */

# if defined (HAVE_DIRENT_H)
#   include <dirent.h>
#   define D_NAMLEN(dirent) strlen((dirent)->d_name)
# else /* !HAVE_DIRENT_H */
#   define dirent direct
#   define D_NAMLEN(dirent) (dirent)->d_namlen
#   if defined (HAVE_SYS_NDIR_H)
#     include <sys/ndir.h>
#   endif /* HAVE_SYS_NDIR_H */
#   if defined (HAVE_SYS_DIR_H)
#     include <sys/dir.h>
#   endif /* HAVE_SYS_DIR_H */
#   if defined (HAVE_NDIR_H)
#     include <ndir.h>
#   endif /* HAVE_NDIR_H */
#   if !defined (HAVE_SYS_NDIR_H) && \
       !defined (HAVE_SYS_DIR_H)  && \
       !defined (HAVE_NDIR_H)
#     include "ndir.h"
#   endif /* !HAVE_SYS_NDIR_H && !HAVE_SYS_DIR_H && !HAVE_NDIR_H */
# endif /* !HAVE_DIRENT_H */

#ifdef HAVE_SETJMP_H
#  include <setjmp.h>
#else
#  error NEED <setjmp.h>
#endif

#endif /* COMPAT_H */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * compat.h ends here */
