
/*
 *  Time-stamp:      "2003-05-26 08:04:30 bkorb"
 *
 *  autoopts.h  $Id: autoopts.h,v 3.17 2003/05/31 23:15:06 bkorb Exp $
 *
 *  This file defines all the global structures and special values
 *  used in the automated option processing library.
 */

/*
 *  Automated Options copyright 1992-2003 Bruce Korb
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

#define __EXTENSIONS__

#include "config.h"
#include "compat/compat.h"
#include <sys/param.h>
#include <sys/wait.h>
#include <time.h>
#include <utime.h>

#include "options.h"
#include "streqv.h"

/*
 *  Convert the number to a list usable in a printf call
 */
#define NUM_TO_VER(n)       ((n) >> 12), ((n) >> 7) & 0x001F, (n) & 0x007F

#define NAMED_OPTS(po) \
        (((po)->fOptSet & (OPTPROC_SHORTOPT | OPTPROC_LONGOPT)) == 0)

#define SKIP_OPT(p)  (((p)->fOptState & (OPTST_DOCUMENT|OPTST_OMITTED)) != 0)

/*
 *  Procedure success codes
 *
 *  USAGE:  define procedures to return "tSuccess".  Test their results
 *          with the SUCCEEDED, FAILED and HADGLITCH macros.
 */
#define SUCCESS  ((tSuccess) 0)
#define FAILURE  ((tSuccess)-1)
#define PROBLEM  ((tSuccess) 1)

typedef int tSuccess;

#define SUCCEEDED( p )     ((p) == SUCCESS)
#define SUCCESSFUL( p )    SUCCEEDED( p )
#define FAILED( p )        ((p) <  SUCCESS)
#define HADGLITCH( p )     ((p) >  SUCCESS)

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

typedef enum {
    ENV_ALL,
    ENV_IMM,
    ENV_NON_IMM
} teEnvPresetType;

typedef enum {
    TOPT_UNDEFINED = 0,
    TOPT_SHORT,
    TOPT_LONG,
    TOPT_DEFAULT
} teOptType;

typedef struct {
    tOptDesc*  pOD;
    tUL        flags;
    teOptType  optType;
    int        argType;
    char*      pzOptArg;
} tOptState;

#  define AGALOC( c, w )        malloc( c )
#  define AGREALOC( p, c, w )   realloc( p, c )
#  define AGFREE( p )           free( p )
#  define AGDUPSTR( p, s, w )   p = strdup( s )
#  define TAGMEM( m, t )

#ifdef AUTOGEN_BUILD
#  include <snprintfv/printf.h>
#else  /* NOT AUTOGEN_BUILD */
#endif /* AUTOGEN_BUILD */

/*
 *  File pointer for usage output
 */
extern FILE* option_usage_fp;

/*
 *  optionUsage print the usage text for the program described by the
 *  option descriptor.  Does not return.  Calls ``exit(3)'' with exitCode.
 */
extern void optionUsage( tOptions*, int exitCode );

/*
 *  optionSave saves the option state into an RC or INI file in
 *  the *LAST* defined directory in the papzHomeList.
 */
extern void optionSave( tOptions* pOpts );

/*
 *  optionMakePath  --  translate and construct a path
 *
 *  This routine does environment variable expansion if the first character
 *  is a ``$''.  If it starts with two dollar characters, then the path
 *  is relative to the location of the executable.
 */
extern ag_bool optionMakePath(
    char*    pzBuf,
    size_t   bufSize,
    tCC*     pzName,
    tCC*     pzProgPath );

extern tOptProc doVersion, doPagedUsage, doLoadOpt;

#endif /* AUTOGEN_AUTOOPTS_H */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/autoopts.h */
