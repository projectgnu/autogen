
/*
 *  $Id: putshell.c,v 1.1 1998/04/29 23:14:31 bkorb Exp $
 *
 *  This module will interpret the options set in the tOptions
 *  structure and print them to standard out in a fashion that
 *  will allow them to be interpreted by the Bourne or Korn shells.
 */

/*
 *  Automated Options copyright 1992-1998 Bruce Korb
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
#include <string.h>
#include "autoopts.h"

/*
 *  Make sure embedded single quotes come out okay
 */
    static void
putQuotedStr( char* pzStr )
{
    if (pzStr != (char*)NULL) for (;;) {
        char* pz = strchr( pzStr, '\'' );
        if (pz != (char*)NULL)
            *pz = NUL;
        fputs( pzStr, stdout );
        if (pz == (char*)NULL)
            break;
        fputs( "'\"'\"'", stdout );
        pzStr = pz+1;
    }
}

    void
putBourneShell( tOptions* pOpts )
{
    int       optCt = pOpts->presetOptCt;
    tOptDesc* pOD   = pOpts->pOptDesc;
    tSCC zOptCtFmt[]  = "OPTION_CT=%d\nexport OPTION_CT\n";
    tSCC zOptNumFmt[] = "%s_%s=%d\nexport %1$s_%s\n";

    printf( zOptCtFmt, pOpts->curOptIdx-1 );

    for (;optCt-- > 0; pOD++) {
        if (UNUSED_OPT( pOD ))
            continue;

        /*
         *  We assume stacked arguments if the cookie is non-NULL
         */
        if (pOD->optCookie != (void*)NULL) {
            tSCC zOptCookieCt[] = "%s_%s_CT=%d\nexport %1$s_%s_CT\n";

            tArgList*  pAL = (tArgList*)pOD->optCookie;
            char**     ppz = pAL->apzArgs;
            int        ct  = pAL->useCt;

            printf( zOptCookieCt, pOpts->pzPROGNAME, pOD->pz_NAME, ct );

            while (--ct >= 0) {
                tSCC zOptNumArg[] = "%s_%s_%d='";
                tSCC zOptEnd[]    = "'\nexport %s_%s_%d\n";

                printf( zOptNumArg, pOpts->pzPROGNAME, pOD->pz_NAME,
                        pAL->useCt-ct );
		putQuotedStr( *(ppz++) );
                printf( zOptEnd, pOpts->pzPROGNAME, pOD->pz_NAME,
                        pAL->useCt-ct );
            }
        }

        /*
         *  If the argument type is numeric, the last arg pointer
         *  is really the VALUE of the string that was pointed to.
         */
        else if ((pOD->fOptState & OPTST_NUMERIC) != 0)
            printf( zOptNumFmt, pOpts->pzPROGNAME, pOD->pz_NAME, pOD->pzLastArg );

        /*
         *  IF the option has an empty value,
         *  THEN we set the argument to the occurrance count.
         */
        else if (  (pOD->pzLastArg == (char*)NULL)
                || (pOD->pzLastArg[0] == NUL) )

            printf( zOptNumFmt, pOpts->pzPROGNAME, pOD->pz_NAME, pOD->optOccCt );

        /*
         *  This option has a text value
         */
        else {
            tSCC zOptValFmt[] = "%s_%s='";
            tSCC zOptEnd[]    = "'\nexport %1$s_%s\n";

            printf( zOptValFmt, pOpts->pzPROGNAME, pOD->pz_NAME );
	    putQuotedStr( pOD->pzLastArg );
            printf( zOptEnd, pOpts->pzPROGNAME, pOD->pz_NAME );
        }
    }
}
