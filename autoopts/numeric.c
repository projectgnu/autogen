
/*
 *  $Id: numeric.c,v 4.14 2008/01/23 00:35:27 bkorb Exp $
 *  Time-stamp:      "2007-11-17 10:40:58 bkorb"
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
optionNumericVal( tOptions* pOpts, tOptDesc* pOD )
{
    char* pz;
    long  val;

    /*
     *  Numeric options may have a range associated with it.
     *  If it does, the usage procedure requests that it be
     *  emitted by passing a NULL pOD pointer.
     */
    if ((pOD == NULL) || (pOD->optArg.argString == NULL))
        return;

    val = strtol(pOD->optArg.argString, &pz, 0);
    if (pz == pOD->optArg.argString)
        goto bad_number;

    if ((pOD->fOptState & OPTST_SCALED_NUM) != 0)
        switch (*(pz++)) {
        case '\0': pz--; break;
        case 't':  val *= 1000;
        case 'g':  val *= 1000;
        case 'm':  val *= 1000;
        case 'k':  val *= 1000; break;

        case 'T':  val *= 1024;
        case 'G':  val *= 1024;
        case 'M':  val *= 1024;
        case 'K':  val *= 1024; break;

        default:   goto bad_number;
        }

    if (*pz != NUL)
        goto bad_number;

    if (pOD->fOptState & OPTST_ALLOC_ARG) {
        AGFREE(pOD->optArg.argString);
        pOD->fOptState &= ~OPTST_ALLOC_ARG;
    }

    pOD->optArg.argInt = val;
    return;

bad_number:
    fprintf( stderr, zNotNumber, pOpts->pzProgName, pOD->optArg.argString );
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0)
        (*(pOpts->pUsageProc))(pOpts, EXIT_FAILURE);

    pOD->optArg.argInt = ~0;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/numeric.c */
