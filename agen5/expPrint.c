
/*
 *  $Id: expPrint.c,v 3.8 2002/06/27 01:36:13 bkorb Exp $
 *
 *  The following code is necessary because the user can give us
 *  a printf format requiring a string pointer yet fail to provide
 *  a valid pointer, thus it will fault.  This code protects
 *  against the fault so an error message can be emitted instead of
 *  a core dump :-)
 */

/*
 *  AutoGen copyright 1992-2002 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

#include <signal.h>
#include <compat/compat.h>

#ifndef HAVE_STRSIGNAL
#  include <compat/strsignal.c>
#endif

#include "expr.h"
#include "autogen.h"

STATIC sigjmp_buf printJumpEnv;
STATIC void    printFault( int sig );
STATIC size_t safePrintf( char* pzBuf, size_t bufSize,
                          char* pzFmt, void** argV );
tSCC pzFormatName[] = "format";

STATIC void
printFault( int sig )
{
    siglongjmp( printJumpEnv, sig );
}


STATIC size_t
safePrintf( char* pzBuf, size_t bufSize, char* pzFmt, void** argV )
{
    tSCC zBadArgs[]  = "Bad args to sprintf";
    tSCC zBadFmt[]   = "%s ERROR:  %s processing printf format:\n\t%s\n";

    size_t printSize;
    int    faultType;

    struct sigaction  sa;
    struct sigaction  saSave1;
    struct sigaction  saSave2;

    sa.sa_handler = printFault;
    sa.sa_flags   = 0;
    sigemptyset( &sa.sa_mask );

    sigaction( SIGBUS,  &sa, &saSave1 );
    sigaction( SIGSEGV, &sa, &saSave2 );

    /*
     *  IF the sprintfv call below is going to address fault,
     *  THEN ...
     */
    if (faultType = sigsetjmp( printJumpEnv, 0 ),
        faultType != 0) {
#ifndef HAVE_STRSIGNAL
        extern char* strsignal PARAMS((int signo));
#endif
        /*
         *  IF the fprintf command in the then clause has not failed yet,
         *  THEN perform that fprintf
         */
        if (sigsetjmp( printJumpEnv, 0 ) == 0)
            fprintf( pfTrace, zBadFmt, pzProg, strsignal( faultType ), pzFmt );

        /*
         *  The "sprintfv" command below faulted, so we exit
         */
        AG_ABEND( zBadArgs );
    }

    printSize = snprintfv( pzBuf, bufSize, pzFmt, (void*)argV );
    if ((printSize & ~0xFFFFFF) != 0)
        AG_ABEND( asprintf( "snprintf returned 0x%08X\n", printSize ));

    sigaction( SIGBUS,  &saSave1, NULL );
    sigaction( SIGSEGV, &saSave2, NULL );
    return printSize;
}


EXPORT SCM
run_printf( char* pzFmt, int len, SCM alist )
{
    SCM     res;
    void**  arglist;
    void**  argp;
    char*   pzBuf;
    size_t  bfSize = (len * 128) + strlen( pzFmt );

    void**  freelist;
    void**  freep;

    /*
     *  Allocate everything we need at once.  Round up to next 4096 bytes.
     *  Most likely, we allocate and free a page every time.
     */
    bfSize += 2 * len * sizeof( void* ) + 0x100F;
    bfSize &= ~0x0FFF;

    arglist  = argp  = (void**)AGALOC( bfSize, "sprintf buffer" );
    freelist = freep = arglist + len;
    pzBuf    = (char*)(freelist + len);
    bfSize  -= 2 * len * sizeof( void* );

    while (len-- > 0) {
        SCM  car = SCM_CAR( alist );
        alist = SCM_CDR( alist );
        switch (gh_type_e( car )) {
        default:
        case GH_TYPE_UNDEFINED:
            *(argp++) = (void*)"???";
            break;

        case GH_TYPE_BOOLEAN:
            *(argp++) = (void*)((car == SCM_BOOL_F) ? "#f" : "#t");
            break;

        case GH_TYPE_CHAR:
            *(argp++) = (void*)(int)gh_scm2char( car );
            break;

        case GH_TYPE_PAIR:
            *(argp++) = (void*)"..";
            break;

        case GH_TYPE_NUMBER:
            *(argp++) = (void*)gh_scm2ulong( car );
            break;

        case GH_TYPE_SYMBOL:
        case GH_TYPE_STRING:
            *(freep++) = *(argp++) = (void*)gh_scm2newstr( car, NULL );
            break;

        case GH_TYPE_PROCEDURE:
            *(argp++) = (void*)"(*)()";
            break;

        case GH_TYPE_VECTOR:
        case GH_TYPE_LIST:
            *(argp++) = (void*)"...";
            break;
        }
    }

    /*
     *  Do the formatting and allocate a new SCM to hold the result.
     *  Free up any allocations made by ``gh_scm2newstr''
     */
    bfSize = safePrintf( pzBuf, bfSize, pzFmt, arglist );
    res = gh_str2scm( pzBuf, bfSize );

    while (freelist < freep)
        free( *(freelist++) );

    AGFREE( (void*)arglist );
    return res;
}


/*=gfunc sprintf
 *
 * what:  format a string
 * general_use:
 *
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
=*/
    SCM
ag_scm_sprintf( SCM fmt, SCM alist )
{
    int   list_len = scm_ilength( alist );
    char* pzFmt    = ag_scm2zchars( fmt, pzFormatName );

    if (list_len <= 0)
        return fmt;

    return run_printf( pzFmt, list_len, alist );
}


/*=gfunc printf
 *
 * what:  format to stdout
 * general_use:
 *
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
 *       Write to the standard out port.  The result will NOT appear in your
 *       output.  Use this to print information messages to a template user.
 *       Use ``(sprintf ...)'' to add text to your document.
=*/
    SCM
ag_scm_printf( SCM fmt, SCM alist )
{
    int   list_len = scm_ilength( alist );
    char* pzFmt    = ag_scm2zchars( fmt, pzFormatName );

    gh_display( run_printf( pzFmt, list_len, alist ));
    return SCM_UNDEFINED;
}


/*=gfunc fprintf
 *
 * what:  format to a file
 * general_use:
 *
 * exparg: port, Guile-scheme output port
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
 *       Write to a specified port.  The result will NOT appear in your
 *       output.  Use this to print information messages to a template user.
=*/
    SCM
ag_scm_fprintf( SCM port, SCM fmt, SCM alist )
{
    int   list_len = scm_ilength( alist );
    char* pzFmt    = ag_scm2zchars( fmt, pzFormatName );
    SCM   res      = run_printf( pzFmt, list_len, alist );

    return  scm_display( res, port );
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of expPrint.c */
