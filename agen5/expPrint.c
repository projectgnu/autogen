
/*
 *  $Id: expPrint.c,v 1.7 1999/11/24 23:30:12 bruce Exp $
 *
 *  The following code is necessary because the user can give us
 *  a printf format requiring a string pointer yet fail to provide
 *  a valid pointer, thus it will fault.  This code protects
 *  against the fault so an error message can be emitted instead of
 *  a core dump :-)
 */

/*
 *  AutoGen copyright 1992-1999 Bruce Korb
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

#include "autogen.h"
#include "expGuile.h"


STATIC sigjmp_buf printJumpEnv;

    STATIC void
printFault( int sig )
{
    siglongjmp( printJumpEnv, sig );
}


    STATIC size_t
safePrintf( char* pzBuf, size_t bufSize, char* pzFmt, void** argV )
{
    tSCC zOvfloFmt[] = "%s ERROR:  formating error with:\n\t`%s'\n";
    tSCC zBadArgs[]  = "Bad args to sprintf";

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
        extern char* strsignal();

        /*
         *  IF the fprintf command in the then clause has not failed yet,
         *  THEN perform that fprintf
         */
        if (sigsetjmp( printJumpEnv, 0 ) == 0)
            fprintf( stderr, "%s ERROR:  %s processing printf format:\n"
                     "\t%s\n", pzProg,
                     strsignal( faultType ), pzFmt );

        /*
         *  The "sprintfv" command below faulted, so we exit
         */
        LOAD_ABORT( pCurTemplate, pCurMacro, zBadArgs );
    }

    printSize = snprintfv( pzBuf, bufSize, pzFmt, (void*)argV );

    sigaction( SIGBUS,  &saSave1, (struct sigaction*)NULL );
    sigaction( SIGSEGV, &saSave2, (struct sigaction*)NULL );
    return printSize;
}


    STATIC SCM
run_printf( char* pzFmt, int len, SCM alist )
{
    SCM     res;
    void**  arglist;
    void**  argp;
    char*   pzBuf;
    size_t  bfSize = (len * 64) + strlen( pzFmt );

    arglist = argp = (void**)AGALOC( len * sizeof( void* ));
    pzBuf = (char*)AGALOC( bfSize );

    do  {
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
            *(argp++) = (void*)gh_scm2long( car );
            break;

        case GH_TYPE_SYMBOL:
        case GH_TYPE_STRING:
            *(argp++) = (void*)SCM_CHARS( car );
            break;

        case GH_TYPE_PROCEDURE:
            *(argp++) = (void*)"(*)()";
            break;

        case GH_TYPE_VECTOR:
        case GH_TYPE_LIST:
            *(argp++) = (void*)"...";
            break;
        }
    } while (--len > 0);
    bfSize = safePrintf( pzBuf, bfSize, pzFmt, arglist );
    res = gh_str2scm( pzBuf, bfSize );
    AGFREE( (void*)pzBuf );
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
    int len = scm_ilength( alist );

    if (! gh_string_p( fmt ))
        return SCM_UNDEFINED;

    if (len <= 0)
        return fmt;

    return run_printf( SCM_CHARS( fmt ), len, alist );
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
 *       Write to the default output port.
=*/
    SCM
ag_scm_printf( SCM fmt, SCM alist )
{
    SCM res;
    int len = scm_ilength( alist );

    if (! gh_string_p( fmt ))
        return SCM_UNDEFINED;

    if (len <= 0)
        return fmt;

    res = run_printf( SCM_CHARS( fmt ), len, alist );

    gh_display( res );
    return SCM_UNDEFINED;
}


/*=gfunc fprintf
 *
 * what:  format to a file
 * general_use:
 *
 * exparg: port, output port
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
 *       Write to a specified port.
=*/
    SCM
ag_scm_fprintf( SCM port, SCM fmt, SCM alist )
{
    SCM res;
    int len = scm_ilength( alist );

    if (! gh_string_p( fmt ))
        return SCM_UNDEFINED;

    if (len <= 0)
        return fmt;

    res = run_printf( SCM_CHARS( fmt ), len, alist );

    return  scm_display( res, port );
}
/* end of expPrint.c */
