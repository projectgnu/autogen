
/*
 *  $Id: guileopt.c,v 4.15 2008/01/23 00:35:27 bkorb Exp $
 * Time-stamp:      "2007-07-04 10:23:29 bkorb"
 *
 *  This module will export the option values to the Guile environment.
 *
 *  This file is part of AutoOpts, a companion to AutoGen.
 *  AutoOpts is free software.
 *  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *
 *  AutoOpts is available under any one of two licenses.  The license
 *  in use must be one of these two and the choice is under the control
 *  of the user of the license.
 *
 *   The GNU Lesser General Public License, version 3 or later
 *      See the files "COPYING.lgplv3" and "COPYING.gplv3"
 *
 *   The Modified Berkeley Software Distribution License
 *      See the file "COPYING.mbsd"
 *
 *  These files have the following md5sums:
 *
 *  239588c55c22c60ffe159946a760a33e pkg/libopts/COPYING.gplv3
 *  fa82ca978890795162346e661b47161a pkg/libopts/COPYING.lgplv3
 *  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd
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
