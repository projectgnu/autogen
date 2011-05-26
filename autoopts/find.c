/**
 * @file check.c
 *
 * @brief Hunt for options in the option descriptor list
 *
 *  Time-stamp:      "2011-05-24 21:29:56 bkorb"
 *
 *  This file contains the routines that deal with processing quoted strings
 *  into an internal format.
 *
 *  This file is part of AutoOpts, a companion to AutoGen.
 *  AutoOpts is free software.
 *  AutoOpts is Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
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
 *  43b91e8ca915626ed3818ffb1b71248b pkg/libopts/COPYING.gplv3
 *  06a1a2e4760c90ea5e1dad8dfaac4d39 pkg/libopts/COPYING.lgplv3
 *  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd
 */

/**
 * find the name and name length we are looking for
 */
static int
parse_opt(char ** nm_pp, char ** arg_pp, char * buf, size_t bufsz)
{
    int  res = 0;
    char * p = *nm_pp;
    *arg_pp  = NULL;

    for (;;) {
        switch (*(p++)) {
        case NUL: return res;

        case '=':
            if (res >= bufsz)
                return -1;

            memcpy(buf, *nm_pp, res);

            buf[res] = NUL;
            *nm_pp   = buf;
            *arg_pp  = p;
            return res;

        default:
            res++;
        }
    }
}

/**
 *  Find the long option descriptor for the current option
 */
LOCAL tSuccess
opt_find_long(tOptions * pOpts, char * opt_name, tOptState * pOptState)
{
    char       name_buf[128];
    char *     opt_arg;
    int        nm_len =
        parse_opt(&opt_name, &opt_arg, name_buf, sizeof(name_buf));

    ag_bool    disable  = AG_FALSE;
    tOptDesc * pOD      = pOpts->pOptDesc;
    int        idx      = 0;
    int        idxLim   = pOpts->optCt;
    int        matchCt  = 0;
    int        matchIdx = 0;

    do  {
        /*
         *  If option disabled or a doc option, skip to next
         */
        if (pOD->pz_Name == NULL)
            continue;

        if (  SKIP_OPT(pOD)
           && (pOD->fOptState != (OPTST_OMITTED | OPTST_NO_INIT)))
            continue;

        if (strneqvcmp(opt_name, pOD->pz_Name, nm_len) == 0) {
            /*
             *  IF we have a complete match
             *  THEN it takes priority over any already located partial
             */
            if (pOD->pz_Name[ nm_len ] == NUL) {
                matchCt  = 1;
                matchIdx = idx;
                break;
            }
        }

        /*
         *  IF       there is a disable name
         *     *AND* no argument value has been supplied
         *              (disabled options may have no argument)
         *     *AND* the option name matches the disable name
         *  THEN ...
         */
        else if (  (pOD->pz_DisableName != NULL)
                && (strneqvcmp(opt_name, pOD->pz_DisableName, nm_len) == 0)
                )  {
            disable  = AG_TRUE;

            /*
             *  IF we have a complete match
             *  THEN it takes priority over any already located partial
             */
            if (pOD->pz_DisableName[ nm_len ] == NUL) {
                matchCt  = 1;
                matchIdx = idx;
                break;
            }
        }

        else
            continue;

        /*
         *  We found a partial match, either regular or disabling.
         *  Remember the index for later.
         */
        matchIdx = idx;

        if (++matchCt > 1)
            break;

    } while (pOD++, (++idx < idxLim));

    /*
     *  Make sure we either found an exact match or found only one partial
     */
    if (matchCt == 1) {
        pOD = pOpts->pOptDesc + matchIdx;

        if (SKIP_OPT(pOD)) {
            fprintf(stderr, zDisabledErr, pOpts->pzProgName, pOD->pz_Name);
            if (pOD->pzText != NULL)
                fprintf(stderr, " -- %s", pOD->pzText);
            fputc('\n', stderr);
            (*pOpts->pUsageProc)(pOpts, EXIT_FAILURE);
            /* NOTREACHED */
        }

        /*
         *  IF we found a disablement name,
         *  THEN set the bit in the callers' flag word
         */
        if (disable)
            pOptState->flags |= OPTST_DISABLED;

        pOptState->pOD      = pOD;
        pOptState->pzOptArg = opt_arg;
        pOptState->optType  = TOPT_LONG;
        return SUCCESS;
    }

    /*
     *  IF there is no equal sign
     *     *AND* we are using named arguments
     *     *AND* there is a default named option,
     *  THEN return that option.
     */
    if (  (opt_arg == NULL)
       && NAMED_OPTS(pOpts)
       && (pOpts->specOptIdx.default_opt != NO_EQUIVALENT)) {
        pOptState->pOD = pOpts->pOptDesc + pOpts->specOptIdx.default_opt;

        pOptState->pzOptArg = opt_name;
        pOptState->optType  = TOPT_DEFAULT;
        return SUCCESS;
    }

    /*
     *  IF we are to stop on errors (the default, actually)
     *  THEN call the usage procedure.
     */
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
        fprintf(stderr, (matchCt == 0) ? zIllOptStr : zAmbigOptStr,
                pOpts->pzProgPath, opt_name);
        (*pOpts->pUsageProc)(pOpts, EXIT_FAILURE);
    }

    return FAILURE;
}


/*
 *  opt_find_short
 *
 *  Find the short option descriptor for the current option
 */
LOCAL tSuccess
opt_find_short(tOptions* pOpts, uint_t optValue, tOptState* pOptState)
{
    tOptDesc*  pRes = pOpts->pOptDesc;
    int        ct   = pOpts->optCt;

    /*
     *  Search the option list
     */
    do  {
        if (optValue != pRes->optValue)
            continue;

        if (SKIP_OPT(pRes)) {
            if (  (pRes->fOptState == (OPTST_OMITTED | OPTST_NO_INIT))
               && (pRes->pz_Name != NULL)) {
                fprintf(stderr, zDisabledErr, pOpts->pzProgPath, pRes->pz_Name);
                if (pRes->pzText != NULL)
                    fprintf(stderr, " -- %s", pRes->pzText);
                fputc('\n', stderr);
                (*pOpts->pUsageProc)(pOpts, EXIT_FAILURE);
                /* NOTREACHED */
            }
            goto short_opt_error;
        }

        pOptState->pOD     = pRes;
        pOptState->optType = TOPT_SHORT;
        return SUCCESS;

    } while (pRes++, --ct > 0);

    /*
     *  IF    the character value is a digit
     *    AND there is a special number option ("-n")
     *  THEN the result is the "option" itself and the
     *       option is the specially marked "number" option.
     */
    if (  IS_DEC_DIGIT_CHAR(optValue)
       && (pOpts->specOptIdx.number_option != NO_EQUIVALENT) ) {
        pOptState->pOD = \
        pRes           = pOpts->pOptDesc + pOpts->specOptIdx.number_option;
        (pOpts->pzCurOpt)--;
        pOptState->optType = TOPT_SHORT;
        return SUCCESS;
    }

short_opt_error:

    /*
     *  IF we are to stop on errors (the default, actually)
     *  THEN call the usage procedure.
     */
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
        fprintf(stderr, zIllOptChr, pOpts->pzProgPath, optValue);
        (*pOpts->pUsageProc)(pOpts, EXIT_FAILURE);
    }

    return FAILURE;
}


/*
 *  findOptDesc
 *
 *  Find the option descriptor for the current option
 */
LOCAL tSuccess
find_opt(tOptions* pOpts, tOptState* pOptState)
{
    /*
     *  IF we are continuing a short option list (e.g. -xyz...)
     *  THEN continue a single flag option.
     *  OTHERWISE see if there is room to advance and then do so.
     */
    if ((pOpts->pzCurOpt != NULL) && (*pOpts->pzCurOpt != NUL))
        return opt_find_short(pOpts, (tAoUC)*(pOpts->pzCurOpt), pOptState);

    if (pOpts->curOptIdx >= pOpts->origArgCt)
        return PROBLEM; /* NORMAL COMPLETION */

    pOpts->pzCurOpt = pOpts->origArgVect[ pOpts->curOptIdx ];

    /*
     *  IF all arguments must be named options, ...
     */
    if (NAMED_OPTS(pOpts)) {
        char *   pz  = pOpts->pzCurOpt;
        int      def;
        tSuccess res; 
        tAoUS *  def_opt;

        pOpts->curOptIdx++;

        if (*pz != '-')
            return opt_find_long(pOpts, pz, pOptState);

        /*
         *  The name is prefixed with one or more hyphens.  Strip them off
         *  and disable the "default_opt" setting.  Use heavy recasting to
         *  strip off the "const" quality of the "default_opt" field.
         */
        while (*(++pz) == '-')   ;
        def_opt = (void *)&(pOpts->specOptIdx.default_opt);
        def = *def_opt;
        *def_opt = NO_EQUIVALENT;
        res = opt_find_long(pOpts, pz, pOptState);
        *def_opt = def;
        return res;
    }

    /*
     *  Note the kind of flag/option marker
     */
    if (*((pOpts->pzCurOpt)++) != '-')
        return PROBLEM; /* NORMAL COMPLETION - this + rest are operands */

    /*
     *  Special hack for a hyphen by itself
     */
    if (*(pOpts->pzCurOpt) == NUL)
        return PROBLEM; /* NORMAL COMPLETION - this + rest are operands */

    /*
     *  The current argument is to be processed as an option argument
     */
    pOpts->curOptIdx++;

    /*
     *  We have an option marker.
     *  Test the next character for long option indication
     */
    if (pOpts->pzCurOpt[0] == '-') {
        if (*++(pOpts->pzCurOpt) == NUL)
            /*
             *  NORMAL COMPLETION - NOT this arg, but rest are operands
             */
            return PROBLEM;

        /*
         *  We do not allow the hyphen to be used as a flag value.
         *  Therefore, if long options are not to be accepted, we punt.
         */
        if ((pOpts->fOptSet & OPTPROC_LONGOPT) == 0) {
            fprintf(stderr, zIllOptStr, pOpts->pzProgPath,
                    zIllegal, pOpts->pzCurOpt-2);
            return FAILURE;
        }

        return opt_find_long(pOpts, pOpts->pzCurOpt, pOptState);
    }

    /*
     *  If short options are not allowed, then do long
     *  option processing.  Otherwise the character must be a
     *  short (i.e. single character) option.
     */
    if ((pOpts->fOptSet & OPTPROC_SHORTOPT) != 0)
        return opt_find_short(pOpts, (tAoUC)*(pOpts->pzCurOpt), pOptState);

    return opt_find_long(pOpts, pOpts->pzCurOpt, pOptState);
}
