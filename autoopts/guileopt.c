
/*
 *  $Id: guileopt.c,v 4.6 2005/02/15 01:34:13 bkorb Exp $
 * Time-stamp:      "2005-02-14 14:30:24 bkorb"
 *
 *  This module will export the option values to the Guile environment.
 */

/*
 *  Automated Options copyright 1992-2005 Bruce Korb
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
#include <guile/gh.h>

#include "autoopts/options.h"
#include "autoopts.h"

/*=export_func  export_options_to_guile
 * what:  put the option state into Guile symbols
 * private:
 *
 * arg:   tOptions*, pOpts, the program options descriptor
 * doc:   This routine will emit Guile/Scheme code that puts the option
 *        processing state into Scheme symbols known to the Guile
 *        interpreter.
=*/
void
export_options_to_guile( pOpts )
    tOptions* pOpts;
{
    tOptDesc*  pOD = pOpts->pOptDesc;
    int        ct  = pOpts->presetOptCt;
    char z[ 4096 ];

    for (;--ct >= 0;pOD++) {
        if (SKIP_OPT(pOD))
            continue;

        /*
         *  We always indicate the presence/absence enabled/disabled state.
         */
        sprintf( z, "(define opt-enabled-%s #%c)\n", pOD->pz_Name,
                 (DISABLED_OPT(pOD) ? 'f' : 't'));
#ifdef DEBUG
        fputs( z, stderr );
#endif
        gh_eval_str( z );

        sprintf( z, "(define have-opt-%s #%c)\n", pOD->pz_Name,
                 UNUSED_OPT( pOD ) ? 'f' : 't' );
#ifdef DEBUG
        fputs( z, stderr );
#endif
        gh_eval_str( z );

        /*
         *  IF the option has not been set, it *still* may have
         *  a default value set.  Check for that.  (Options that
         *  take numeric arguments will always have a value set.)
         */
        if (UNUSED_OPT( pOD )) {
            if (OPTST_GET_ARGTYPE(pOD->fOptState) == OPARG_TYPE_NUMERIC) {
                sprintf( z, "(define opt-arg-%s %d)\n", pOD->pz_Name,
                         (uintptr_t)pOD->pzLastArg );
#ifdef DEBUG
                fputs( z, stderr );
#endif
                gh_eval_str( z );
            }
            else if (pOD->pzLastArg != 0) {
                sprintf( z, "(define opt-arg-%s \"%s\")\n", pOD->pz_Name,
                         pOD->pzLastArg );
#ifdef DEBUG
                fputs( z, stderr );
#endif
                gh_eval_str( z );
            }
            continue;
        }

        /*
         *  IF the option can occur several times, then emit the count.
         */
        if (pOD->optMaxCt > 1) {
            sprintf( z, "(define opt-ct-%s %ld)\n",
                     pOD->pz_Name, pOD->optOccCt );
#ifdef DEBUG
            fputs( z, stderr );
#endif
            gh_eval_str( z );
        }

        /*
         *  IF there is a stack of option args, emit them as a list.
         */
        if (pOD->optCookie != NULL) {
            tArgList* pAL = (tArgList*)pOD->optCookie;
            int       act = pAL->useCt;
            tCC**     ppa = pAL->apzArgs;
            char*     pz  = z;

            pz += sprintf( pz, "(define opt-args-%s `(", pOD->pz_Name );
            while (--act >= 0)
                pz += sprintf( pz, " \"%s\"", *(ppa++) );
            strcpy( pz, " ))\n" );
#ifdef DEBUG
            fputs( z, stderr );
#endif
            gh_eval_str( z );

        }

        /*
         *  IF the option takes a numeric value, set the value
         */
        else if (OPTST_GET_ARGTYPE(pOD->fOptState) == OPARG_TYPE_NUMERIC) {
            sprintf( z, "(define opt-arg-%s %d)\n", pOD->pz_Name,
                     (uintptr_t)pOD->pzLastArg );
#ifdef DEBUG
            fputs( z, stderr );
#endif
            gh_eval_str( z );
        }

        /*
         *  IF the option has a string value, emit that.
         */
        else if (pOD->pzLastArg != 0) {
            sprintf( z, "(define opt-arg-%s \"%s\")\n", pOD->pz_Name,
                     pOD->pzLastArg );
#ifdef DEBUG
            fputs( z, stderr );
#endif
            gh_eval_str( z );
        }
    }
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/guileopt.c */
