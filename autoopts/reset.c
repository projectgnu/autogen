
/*
 *  $Id: reset.c,v 4.3 2008/07/28 02:18:55 bkorb Exp $
 *  Time-stamp:      "2008-07-27 15:50:40 bkorb"
 *
 *  This file is part of AutoOpts, a companion to AutoGen.
 *  AutoOpts is free software.
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


/*=export_func  optionResetOpt
 * private:
 *
 * what:  Reset the value of an option
 * arg:   + tOptions* + pOpts    + program options descriptor  +
 * arg:   + tOptDesc* + pOptDesc + the descriptor for this arg +
 *
 * doc:
 *  This code will cause another option to be reset to its initial state.
 *  For example, --reset=foo will cause the --foo option to be reset.
=*/
void
optionResetOpt( tOptions* pOpts, tOptDesc* pOD )
{
    tOptState opt_state = OPTSTATE_INITIALIZER(DEFINED);
    char const * pzArg = pOD->optArg.argString;
    tSuccess     succ;

    if (  (! HAS_originalOptArgArray(pOpts))
       || (pOpts->originalOptArgCookie == NULL)) {
        fputs(zResetNotConfig, stderr);
        _exit(EX_SOFTWARE);
    }

    if ((pzArg == NULL) || (*pzArg == NUL)) {
        fputs(zNoResetArg, stderr);
        pOpts->pUsageProc(pOpts, EXIT_FAILURE);
    }

    if (pzArg[1] == NUL) {
        succ = shortOptionFind(pOpts, (tAoUC)*pzArg, &opt_state);
        if (! SUCCESSFUL(succ)) {
            fprintf(stderr, zIllOptChr, pOpts->pzProgPath, *pzArg);
            pOpts->pUsageProc(pOpts, EXIT_FAILURE);
        }
    } else {
        succ = longOptionFind(pOpts, (char *)pzArg, &opt_state);
        if (! SUCCESSFUL(succ)) {
            fprintf(stderr, zIllOptStr, pOpts->pzProgPath, pzArg);
            pOpts->pUsageProc(pOpts, EXIT_FAILURE);
        }
    }

    /*
     *  We've found the indicated option.  Turn off all non-persistent
     *  flags because we're forcing the option back to its initialized state.
     *  Call any callout procedure to handle whatever it needs to.
     *  Finally, clear the reset flag, too.
     */
    pOD = opt_state.pOD;
    pOD->fOptState &= OPTST_PERSISTENT_MASK;
    pOD->fOptState |= OPTST_RESET;
    if (pOD->pOptProc != NULL)
        pOD->pOptProc(pOpts, pOD);
    pOD->optArg.argString =
        pOpts->originalOptArgArray[ pOD->optIndex ].argString;
    pOD->optCookie = pOpts->originalOptArgCookie[ pOD->optIndex ];
    pOD->fOptState &= OPTST_PERSISTENT_MASK;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/reset.c */
