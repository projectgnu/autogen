
/*
 *  autoopts.h  $Id: autoopts.h,v 2.9 1999/07/07 19:41:00 bkorb Exp $
 *
 *  This file defines all the global structures and special values
 *  used in the automated option processing library.
 */

/*
 *  Automated Options copyright 1992-1999 Bruce Korb
 *
 *  Automated Options is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  Automated Options is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Automated Options.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 *
 * As a special exception, Bruce Korb gives permission for additional
 * uses of the text contained in his release of AutoOpts.
 *
 * The exception is that, if you link the AutoOpts library with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the AutoOpts library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by Bruce Korb under
 * the name AutoOpts.  If you copy code from other sources under the
 * General Public License into a copy of AutoOpts, as the General Public
 * License permits, the exception does not apply to the code that you add
 * in this way.  To avoid misleading anyone as to the status of such
 * modified files, you must delete this exception notice from them.
 *
 * If you write modifications of your own for AutoOpts, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 */

#ifndef AUTOGEN_AUTOOPTS_H
#define AUTOGEN_AUTOOPTS_H

#include "options.h"

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

/*
 *  Some systems have broken printf's!!
 *  So, always link with our own stdio library :-(
 */
#include <snprintfv/snprintfv.h>

#ifndef NUL
#  define NUL '\0'
#endif

#ifndef NULL
#  define NULL 0x0
#endif

#ifndef EXIT_SUCCESS
#  define  EXIT_SUCCESS 0
#  define  EXIT_FAILURE 1
#endif

#ifndef FOPEN_BINARY_FLAG
#  ifdef USE_FOPEN_BINARY
#    define FOPEN_BINARY_FLAG	"b"
#  else
#    define FOPEN_BINARY_FLAG
#  endif
#endif

#ifndef FOPEN_TEXT_FLAG
#  ifdef USE_TEXT_BINARY
#    define FOPEN_TEXT_FLAG	"t"
#  else
#    define FOPEN_TEXT_FLAG
#  endif
#endif

#undef STATIC

#ifdef DEBUG
#  define STATIC
#else
#  define STATIC static
#endif

#ifndef MSDOS
#  define DIR_SEP_CHAR   '/'
#  define OPT_CHAR       '-'
#else
#  define DIR_SEP_CHAR   '\\'
#  define OPT_CHAR       '/'
#endif

#ifndef HAVE_PATHFIND
/*
 * pathfind looks for a a file with name FILENAME and MODE access
 * along colon delimited PATH, and returns the full pathname as a
 * string, or NULL if not found.
 */
#  ifdef __STDC__
     extern char* pathfind(const char *, const char *, const char *);
#  else
     extern char* pathfind();
#  endif
#endif

#ifndef STR
#  define _STR(s) #s
#  define STR(s)  _STR(s)
#endif

#define NAMED_OPTS(po) \
        (((po)->fOptSet & (OPTPROC_SHORTOPT | OPTPROC_LONGOPT)) == 0)

/*
 *  The pager state is used by doPagedUsage() procedure.
 *  When it runs, it sets itself up to be called again on exit.
 *  If, however, a routine needs a child process to do some work
 *  before it is done, then 'pagerState' must be set to
 *  'PAGER_STATE_CHILD' so that doPagedUsage() will not try
 *  to run the pager program before its time.
 */
typedef enum {
    PAGER_STATE_INITIAL,
    PAGER_STATE_READY,
    PAGER_STATE_CHILD
} tePagerState;

extern tePagerState pagerState;

#ifdef MEMDEBUG
   extern void* ag_alloc( size_t, const char* );
   extern void* ag_realloc( void*, size_t, const char* );
   extern void  ag_free( void* );

#  define AGALOC( c )      ag_alloc( c, __FILE__ " at " STR( __LINE__ ))
#  define AGREALOC( p, c)  ag_realloc( p, c, __FILE__ " at " STR( __LINE__ ))
#  define AGFREE( p )      ag_free( p )
#else
#  define AGALOC( c )      malloc( c )
#  define AGREALOC( p, c)  realloc( p, c )
#  define AGFREE( p )      free( p )
#endif

#endif /* AUTOGEN_AUTOOPTS_H */
/* autoopts.h ends here */
