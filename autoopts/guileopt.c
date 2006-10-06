
/*
 *  $Id: guileopt.c,v 4.13 2006/10/06 05:27:22 bkorb Exp $
 * Time-stamp:      "2006-10-05 21:04:56 bkorb"
 *
 *  This module will export the option values to the Guile environment.
 */

/*
 *  Automated Options copyright 1992-2006 Bruce Korb
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
 *             51 Franklin Street, Fifth Floor,
 *             Boston, MA  02110-1301, USA.
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
#include "config.h"
#include <stdio.h>
#if defined(HAVE_STRING_H)
#  include <string.h>
#elif defined(HAVE_STRINGS_H)
#  include <strings.h>
#else
   choke me -- no strings header
#endif
#include <guile/gh.h>

#include "autoopts/options.h"

#define SKIP_OPT(p)  (((p)->fOptState & (OPTST_DOCUMENT|OPTST_OMITTED)) != 0)

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
export_options_to_guile( tOptions* pOpts )
{
    tOptDesc*  pOD = pOpts->pOptDesc;
    int        ct  = pOpts->presetOptCt;
    char z[ 4096 ];

    for (;--ct >= 0;pOD++) {
        static char const opt_state[] =
            "(define opt-enabled-%s #%c) "
            "(define have-opt-%s #%c)";
        static char const num_arg[]  = "(define opt-arg-%s %ld)\n";
        static char const str_arg[]  = "(define opt-arg-%s \"%s\")\n";
        static char const bool_arg[] = "(define opt-arg-%s #%c)\n";


        if (SKIP_OPT(pOD))
            continue;

        /*
         *  We always indicate the presence/absence enabled/disabled state.
         */
        sprintf( z, opt_state,
                 pOD->pz_Name, DISABLED_OPT(pOD) ? 'f' : 't',
                 pOD->pz_Name, UNUSED_OPT( pOD ) ? 'f' : 't' );
        gh_eval_str( z );

        /*
         *  IF the option has not been set, it *still* may have
         *  a default value set.  Check for that.  (Options that
         *  take numeric arguments will always have a value set.)
         */
        if (UNUSED_OPT( pOD )) {
            switch (OPTST_GET_ARGTYPE(pOD->fOptState)) {
            case OPARG_TYPE_NONE:
            case OPARG_TYPE_ENUMERATION:
            case OPARG_TYPE_HIERARCHY:
                /* forget it. */
                break;

            case OPARG_TYPE_STRING:
                if (pOD->optArg.argString != 0) {
                    sprintf(z, str_arg, pOD->pz_Name, pOD->optArg.argString);
                    gh_eval_str( z );
                }
                break;

            case OPARG_TYPE_BOOLEAN:
                sprintf(z, bool_arg, pOD->pz_Name,
                        pOD->optArg.argBool ? 't' : 'f');
                gh_eval_str( z );
                break;

            case OPARG_TYPE_MEMBERSHIP:
                if (pOD->optArg.argEnum == 0)
                    break;
                /* FALLTHROUGH */

            case OPARG_TYPE_NUMERIC:
                sprintf( z, num_arg, pOD->pz_Name, pOD->optArg.argInt );
                gh_eval_str( z );
            }
            continue;
        }

        /*
         *  IF the option can occur several times, then emit the count.
         */
        if (pOD->optMaxCt > 1) {
            sprintf( z, "(define opt-ct-%s %d)\n",
                     pOD->pz_Name, pOD->optOccCt );
            gh_eval_str( z );
        }

        /*
         *  IF there is a stack of option args, emit them as a list.
         */
        if ((pOD->optCookie != NULL) && (pOD->fOptState & OPTST_STACKED)) {
            tArgList* pAL = (tArgList*)pOD->optCookie;
            int       act = pAL->useCt;
            tCC**     ppa = pAL->apzArgs;
            char*     pz  = z;

            pz += sprintf( pz, "(define opt-args-%s `(", pOD->pz_Name );
            while (--act >= 0)
                pz += sprintf( pz, " \"%s\"", *(ppa++) );
            strcpy( pz, " ))\n" );
            gh_eval_str( z );

        }

        /*
         *  IF the option takes a numeric value, set the value
         */
        else if (OPTST_GET_ARGTYPE(pOD->fOptState) == OPARG_TYPE_NUMERIC) {
            sprintf( z, num_arg, pOD->pz_Name, pOD->optArg.argInt );
            gh_eval_str( z );
        }

        /*
         *  IF the option has a string value, emit that.
         */
        else if (pOD->optArg.argString != 0) {
            sprintf( z, str_arg, pOD->pz_Name, pOD->optArg.argString );
            gh_eval_str( z );
        }
    }
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/guileopt.c */
