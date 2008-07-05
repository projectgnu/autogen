
/*
 *  $Id: reset.c,v 4.1 2008/07/05 23:13:39 bkorb Exp $
 *  Time-stamp:      "2008-06-24 16:09:07 bkorb"
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


/*=export_func  optionNumericVal
 * private:
 *
 * what:  Decipher a boolean value
 * arg:   + tOptions* + pOpts    + program options descriptor +
 * arg:   + tOptDesc* + pOptDesc + the descriptor for this arg +
 *
 * doc:
 *  Decipher a numeric value.
=*/
void
optionResetOpt( tOptions* pOpts, tOptDesc* pOD )
{
    tOptState opt_state = OPTSTATE_INITIALIZER(DEFINED);
    char const * pzArg = pOD->optArg.argString;
    tSuccess     succ;

    if ((pzArg == NULL) || (*pzArg == NUL)) {
        fputs(zNoResetArg, stderr);
        pOpts->pUsageProc(pOpts, EXIT_FAILURE);
    }

    switch (pzArg[1]) {
    case NUL:
        opt_state.pzOptArg = NULL;
        goto find_short;

    case '=':
    case ' ':
    case '\t':
        opt_state.pzOptArg = pzArg + 2;
        while (IS_HORIZ_WHITE_CHAR(*(opt_state.pzOptArg)))
            (opt_state.pzOptArg)++;

    find_short:
        succ = shortOptionFind(pOpts, (tAoUC)*pzArg, &opt_state);
        if (! SUCCESSFUL(succ)) {
            fprintf(stderr, zIllOptChr, pOpts->pzProgPath, *pzArg);
            pOpts->pUsageProc(pOpts, EXIT_FAILURE);
        }
        break;

    default:
        succ = longOptionFind(pOpts, pzArg, &opt_state);
        if (! SUCCESSFUL(succ)) {
            fprintf(stderr, zIllOptStr, pOpts->pzProgPath, *pzArg);
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
    pOD->fOptState &= OPTST_PERSISTENT_MASK;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/reset.c */
