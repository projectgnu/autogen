
/*
 *  $Id: enumeration.c,v 2.3 2000/10/17 17:09:19 bkorb Exp $
 *
 *   Automated Options Paged Usage module.
 *
 *  This routine will run run-on options through a pager so the
 *  user may examine, print or edit them at their leisure.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#include "autoopts.h"

tSCC*  pz_fmt;

DEF_PROC_4( static, void, enumError,
            tOptions*, pOpts,
            tOptDesc*, pOD,
            tCC**, paz_names,
            int, name_ct )
{
    if (pOpts != NULL)
        fprintf( stderr, pz_fmt, pOpts->pzProgName, pOD->pzLastArg );

    fprintf( stderr, "The valid %s option keywords are:\n", pOD->pz_Name );
    do  {
        fprintf( stderr, "\t%s\n", *(paz_names++) );
    } while (--name_ct > 0);

    if (pOpts != NULL)
        (*(pOpts->pUsageProc))( pOpts, EXIT_FAILURE );
}

/*
 *  Run the usage output through a pager.
 *  This is very handy if it is very long.
 */
DEF_PROC_4( , char*, optionEnumerationVal,
            tOptions*, pOpts,
            tOptDesc*, pOD,
            tCC**, paz_names,
            int, name_ct )
{
    size_t  len;
    int     idx;
    int     res = -1;

    /*
     *  IF there is no option struct pointer, then we are being
     *  called (indirectly) by the usage routine.  It wants the
     *  keyword list to be printed.
     */
    if (pOpts == NULL) {
        enumError( pOpts, pOD, paz_names, name_ct );
        return;
    }

    len = strlen( pOD->pzLastArg );

    /*
     *  Look for an exact match, but remember any partial matches.
     *  Multiple partial matches means we have an ambiguous match.
     */
    for (idx = 0; idx < name_ct; idx++) {
        if (strncmp( paz_names[idx], pOD->pzLastArg, len ) == 0) {
            if (paz_names[idx][len] == NUL)
                return (char*)idx;
            if (res != -1) {
                pz_fmt = "%s error:  the keyword `%s' is ambiguous\n";
                enumError( pOpts, pOD, paz_names, name_ct );
            }
            res = idx;
        }
    }

    if (res < 0) {
        pz_fmt = "%s error:  `%s' does not match any keywords\n";
        enumError( pOpts, pOD, paz_names, name_ct );
    }

    /*
     *  Return the matching index as a char* pointer.
     *  The result gets stashed in a char* pointer, so it will have to fit.
     */
    return (char*)res;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * pgusage.c ends here */
