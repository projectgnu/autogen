
/*
 *  $Id: autoopts.c,v 3.9 2002/05/11 20:23:52 bkorb Exp $
 *
 *  This file contains all of the routines that must be linked into
 *  an executable to use the generated option processing.  The optional
 *  routines are in separately compiled modules so that they will not
 *  necessarily be linked in.
 */

/*
 *  Automated Options copyright 1992-2002 Bruce Korb
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

#define __EXTENSIONS__
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <config.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_LIBGEN
#  include <libgen.h>
#endif

#include <streqv.h>
#include "autoopts.h"
#include "compat/compat.h"


#define ISNAMECHAR( c )    (isalnum(c) || ((c) == '_') || ((c) == '-'))

tSCC zBadVer[] = "Automated Options Processing Error!\n\
\t%s called optionProcess with structure version %d.%d.%d.\n\
\tThis library was compiled with version %d.%d.%d\n\
\tand requires a minimum structure version of %d.%d.%d\n";


tSCC zMisArg[]      = "%s: option `%s' requires an argument\n";
tSCC zNoArg[]       = "%s: option `%s' cannot have an argument\n";
tSCC zIllOptChr[]   = "%s: illegal option -- %c\n";
tSCC zIllegal[]     = "illegal";
tSCC zIllOptStr[]   = "%s: %s option -- %s\n";
tSCC zAmbiguous[]   = "ambiguous";

tSCC zOnlyOne[]     = "one %s%s option allowed\n";
tSCC zAtMost[]      = "%4$d %1$s%s options allowed\n";
tSCC zEquiv[]       = "-equivalence";
tSCC zErrOnly[]     = "ERROR:  only ";

typedef int tDirection;
#define DIRECTION_PRESET  -1
#define DIRECTION_PROCESS  1
#define PROCESSING(d)     ((d)>0)
#define PRESETTING(d)     ((d)<0)

/*
 *  loadValue
 *
 *  This routine handles equivalencing and invoking the handler procedure,
 *  if any.
 */
STATIC tSuccess
loadValue( pOpts, pOptState )
    tOptions*  pOpts;
    tOptState* pOptState;
{
    /*
     *  Save a copy of the option procedure pointer.
     *  If this is an equivalence class option, we still want this proc.
     */
    tOptDesc* pOD = pOptState->pOD;
    tOptProc* pOP = pOD->pOptProc;

    pOD->pzLastArg =  pOptState->pzOptArg;

    /*
     *  IF this is an equivalence class option,
     *  THEN
     *      Save the option value that got us to this option
     *      entry.  (It may not be pOD->optChar[0], if this is an
     *      equivalence entry.)
     *      set the pointer to the equivalence class base
     */
    if (pOD->optEquivIndex != NO_EQUIVALENT) {
        tOptDesc*  p = pOpts->pOptDesc + pOD->optEquivIndex;

        /*
         *  Add in the equivalence flag
         */
        pOptState->flags |= OPTST_EQUIVALENCE;
        p->pzLastArg      = pOD->pzLastArg;
        p->optActualValue = pOD->optValue;
        p->optActualIndex = pOD->optIndex;
        pOD = p;

    } else {
        pOD->optActualValue = pOD->optValue;
        pOD->optActualIndex = pOD->optIndex;
    }

    pOD->fOptState &= OPTST_PERSISTENT;
    pOD->fOptState |= (pOptState->flags & ~OPTST_PERSISTENT);

    /*
     *  Keep track of count only for DEFINED (command line) options.
     *  IF we have too many, build up an error message and bail.
     */
    if (  (pOD->fOptState & OPTST_DEFINED)
       && (++pOD->optOccCt > pOD->optMaxCt)  )  {
        const char* pzEqv = (pOD->optEquivIndex != NO_EQUIVALENT)
                          ? zEquiv : zEquiv + sizeof( zEquiv )-1;

        if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
            const char* pzFmt = (pOD->optMaxCt > 1) ? zAtMost : zOnlyOne;
            fputs( zErrOnly, stderr );
            fprintf( stderr, pzFmt, pOD->pz_Name, pzEqv,
                     pOD->optMaxCt );
        }

        return FAILURE;
    }

    /*
     *  If provided a procedure to call, call it
     */
    if (pOP != (tpOptProc)NULL)
        (*pOP)( pOpts, pOD );

    return SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  HUNT FOR OPTIONS IN THE ARGUMENT LIST
 *
 *  The next four procedures are "private" to nextOption().
 *  nextOption() uses findOptDesc() to find the next descriptor and it, in
 *  turn, uses longOptionFind() and shortOptionFind() to actually do the hunt.
 *
 *  longOptionFind
 *
 *  Find the long option descriptor for the current option
 */
STATIC tSuccess
longOptionFind( pOpts, pzOptName, pOptState )
    tOptions*   pOpts;
    char*       pzOptName;
    tOptState*  pOptState;
{
    ag_bool    disable  = AG_FALSE;
    char*      pzEq     = strchr( pzOptName, '=' );
    tOptDesc*  pOD      = pOpts->pOptDesc;
    int        idx      = 0;
    int        idxLim   = pOpts->optCt;
    int        matchCt  = 0;
    int        matchIdx = 0;
    int        nameLen;

    /*
     *  IF the value is attached to the name,
     *  THEN clip it off.
     *  Either way, figure out how long our name is
     */
    if (pzEq != (char*)NULL) {
        nameLen = (int)(pzEq - pzOptName);
        *pzEq = NUL;
    } else nameLen = strlen( pzOptName );

    do  {
        if (SKIP_OPT(pOD))
            continue;

        if (strneqvcmp( pzOptName, pOD->pz_Name, nameLen ) == 0) {
            /*
             *  IF we have a complete match
             *  THEN it takes priority over any already located partial
             */
            if (pOD->pz_Name[ nameLen ] == NUL) {
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
        else if (  (pOD->pz_DisableName != (char*)NULL)
                && (strneqvcmp( pzOptName, pOD->pz_DisableName, nameLen ) == 0)
                )  {
            disable  = AG_TRUE;

            /*
             *  IF we have a complete match
             *  THEN it takes priority over any already located partial
             */
            if (pOD->pz_DisableName[ nameLen ] == NUL) {
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

    if (pzEq != (char*)NULL)
        *(pzEq++) = '=';

    /*
     *  Make sure we either found an exact match or found only one partial
     */
    if (matchCt == 1) {
        /*
         *  IF we found a disablement name,
         *  THEN set the bit in the callers' flag word
         */
        if (disable)
            pOptState->flags |= OPTST_DISABLED;

        pOptState->pOD      = pOpts->pOptDesc + matchIdx;
        pOptState->pzOptArg = pzEq;
        pOptState->optType  = TOPT_LONG;
        return SUCCESS;
    }

    /*
     *  IF there is no equal sign
     *     *AND* we are using named arguments
     *     *AND* there is a default named option,
     *  THEN return that option.
     */
    if (  (pzEq == (char*)NULL)
       && NAMED_OPTS(pOpts)
       && (pOpts->specOptIdx.default_opt != NO_EQUIVALENT)) {
        pOptState->pOD = pOpts->pOptDesc + pOpts->specOptIdx.default_opt;

        pOptState->pzOptArg = pzOptName;
        pOptState->optType  = TOPT_DEFAULT;
        return SUCCESS;
    }

    /*
     *  IF we are to stop on errors (the default, actually)
     *  THEN call the usage procedure.
     */
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
        fprintf( stderr, zIllOptStr, pOpts->pzProgPath,
                 (matchCt == 0) ? zIllegal : zAmbiguous, pzOptName );
        (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
    }

    return FAILURE;
}


/*
 *  shortOptionFind
 *
 *  Find the short option descriptor for the current option
 */
STATIC tSuccess
shortOptionFind( pOpts, optValue, pOptState )
    tOptions*   pOpts;
    tUC         optValue;
    tOptState*  pOptState;
{
    tOptDesc*  pRes = pOpts->pOptDesc;
    int        ct   = pOpts->optCt;

    /*
     *  Search the option list
     */
    for (;;) {
        /*
         *  IF the values match,
         *  THEN we stop here
         */
        if ((! SKIP_OPT(pRes)) && (optValue == pRes->optValue)) {
            pOptState->pOD     = pRes;
            pOptState->optType = TOPT_SHORT;
            return SUCCESS;
        }

        /*
         *  Advance to next option description
         */
        pRes++;

        /*
         *  IF we have searched everything, ...
         */
        if (--ct <= 0)
            break;
    }

    /*
     *  IF    the character value is a digit
     *    AND there is a special number option ("-n")
     *  THEN the result is the "option" itself and the
     *       option is the specially marked "number" option.
     */
    if (  isdigit( optValue )
       && (pOpts->specOptIdx.number_option != NO_EQUIVALENT) ) {
        pOptState->pOD = \
        pRes           = pOpts->pOptDesc + pOpts->specOptIdx.number_option;
        (pOpts->pzCurOpt)--;
        pOptState->optType = TOPT_SHORT;
        return SUCCESS;
    }

    /*
     *  IF we are to stop on errors (the default, actually)
     *  THEN call the usage procedure.
     */
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
        fprintf( stderr, zIllOptChr, pOpts->pzProgPath, optValue );
        (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
    }

    return FAILURE;
}


/*
 *  findOptDesc
 *
 *  Find the option descriptor for the current option
 */
STATIC tSuccess
findOptDesc( pOpts, pOptState )
    tOptions*  pOpts;
    tOptState* pOptState;
{
    /*
     *  IF we are continuing a short option list (e.g. -xyz...)
     *  THEN continue a single flag option.
     *  OTHERWISE see if there is room to advance and then do so.
     */
    if ((pOpts->pzCurOpt != (char*)NULL) && (*pOpts->pzCurOpt != NUL))
        return shortOptionFind( pOpts, *pOpts->pzCurOpt, pOptState );

    if (pOpts->curOptIdx >= pOpts->origArgCt)
        return PROBLEM; /* NORMAL COMPLETION */

    pOpts->pzCurOpt = pOpts->origArgVect[ pOpts->curOptIdx ];

    /*
     *  IF all arguments must be named options, ...
     */
    if (NAMED_OPTS(pOpts)) {
        char* pz = pOpts->pzCurOpt;
        pOpts->curOptIdx++;

        /*
         *  Skip over any flag/option markers.
         *  In this mode, they are not required.
         */
        while (*pz == '-') pz++;

        return longOptionFind( pOpts, pz, pOptState );
    }

    /*
     *  Note the kind of flag/option marker
     */
    if (*((pOpts->pzCurOpt)++) != '-')
        return PROBLEM; /* NORMAL COMPLETION - rest are args */

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
            return PROBLEM; /* NORMAL COMPLETION */

        if ((pOpts->fOptSet & OPTPROC_LONGOPT) == 0) {
            fprintf( stderr, zIllOptStr, pOpts->pzProgPath,
                     zIllegal, pOpts->pzCurOpt-2 );
            return FAILURE;
        }

        return longOptionFind( pOpts, pOpts->pzCurOpt, pOptState );
    }

    /*
     *  If short options are not allowed, then do long
     *  option processing.  Otherwise the character must be a
     *  short (i.e. single character) option.
     */
    if ((pOpts->fOptSet & OPTPROC_SHORTOPT) != 0)
        return shortOptionFind( pOpts, *pOpts->pzCurOpt, pOptState );

    return longOptionFind( pOpts, pOpts->pzCurOpt, pOptState );
}


/*
 *  nextOption
 *
 *  Find the option descriptor and option argument (if any) for the
 *  next command line argument.  DO NOT modify the descriptor.  Put
 *  all the state in the state argument so that the option can be skipped
 *  without consequence (side effect).
 */
STATIC tSuccess
nextOption( pOpts, pOptState )
    tOptions*   pOpts;
    tOptState*  pOptState;
{
    tSuccess res;

    res = findOptDesc( pOpts, pOptState );
    if (! SUCCESSFUL( res ))
        return res;
    pOptState->flags |= (pOptState->pOD->fOptState & OPTST_PERSISTENT);

    /*
     *  Figure out what to do about option arguments.  An argument may be
     *  required, not associated with the option, or be optional.  We detect the
     *  latter by examining for an option marker on the next possible argument.
     *  Disabled mode option selection also disables option arguments.
     */
    if ((pOptState->flags & OPTST_DISABLED) != 0)
         pOptState->argType = ARG_NONE;
    else pOptState->argType = pOptState->pOD->optArgType;

    switch (pOptState->argType) {
    case ARG_MUST:
        /*
         *  An option argument is required.  Long options can either have
         *  a separate command line argument, or an argument attached by
         *  the '=' character.  Figure out which.
         */
        switch (pOptState->optType) {
        case TOPT_SHORT:
            /*
             *  See if an arg string follows the flag character
             */
            if (*++(pOpts->pzCurOpt) == NUL)
                pOpts->pzCurOpt = pOpts->origArgVect[ pOpts->curOptIdx++ ];
            pOptState->pzOptArg = pOpts->pzCurOpt;
            break;

        case TOPT_LONG:
            /*
             *  See if an arg string has already been assigned (glued on
             *  with an `=' character)
             */
            if (pOptState->pzOptArg == (char*)NULL)
                pOptState->pzOptArg = pOpts->origArgVect[ pOpts->curOptIdx++ ];
            break;

        default:
#ifdef DEBUG
            fputs( "AutoOpts lib error: option type not selected\n",
                   stderr );
            exit( EXIT_FAILURE );
#endif

        case TOPT_DEFAULT:
            /*
             *  The option was selected by default.  The current token is
             *  the option argument.
             */
            break;
        }

        /*
         *  Make sure we did not overflow the argument list.
         */
        if (pOpts->curOptIdx > pOpts->origArgCt) {
            fprintf( stderr, zMisArg, pOpts->pzProgPath,
                     pOptState->pOD->pz_Name );
            return FAILURE;
        }

        pOpts->pzCurOpt = (char*)NULL;  /* next time advance to next arg */
        break;

    case ARG_MAY:
        /*
         *  An option argument is optional.
         */
        switch (pOptState->optType) {
        case TOPT_SHORT:
            if (*++pOpts->pzCurOpt != NUL)
                pOptState->pzOptArg = pOpts->pzCurOpt;
            else {
                char* pzLA = pOpts->origArgVect[ pOpts->curOptIdx ];

                /*
                 *  BECAUSE it is optional, we must make sure
                 *  we did not find another flag and that there
                 *  is such an argument.
                 */
                if ((pzLA == (char*)NULL) || (*pzLA == '-'))
                    pOptState->pzOptArg = (char*)NULL;
                else {
                    pOpts->curOptIdx++; /* argument found */
                    pOptState->pzOptArg = pzLA;
                }
            }
            break;

        case TOPT_LONG:
            /*
             *  Look for an argument if we don't already have one (glued on
             *  with a `=' character) *AND* we are not in named argument mode
             */
            if (  (pOptState->pzOptArg == (char*)NULL)
               && (! NAMED_OPTS(pOpts))) {
                char* pzLA = pOpts->origArgVect[ pOpts->curOptIdx ];

                /*
                 *  BECAUSE it is optional, we must make sure
                 *  we did not find another flag and that there
                 *  is such an argument.
                 */
                if ((pzLA == (char*)NULL) || (*pzLA == '-'))
                    pOptState->pzOptArg = (char*)NULL;
                else {
                    pOpts->curOptIdx++; /* argument found */
                    pOptState->pzOptArg = pzLA;
                }
            }
            break;

        default:
        case TOPT_DEFAULT:
            fputs( "AutoOpts lib error: defaulted to option with optional arg\n",
                   stderr );
            exit( EXIT_FAILURE );
        }

        /*
         *  After an option with an optional argument, we will
         *  *always* start with the next option because if there
         *  were any characters following the option name/flag,
         *  they would be interpreted as the argument.
         */
        pOpts->pzCurOpt = (char*)NULL;
        break;

    default: /* CANNOT */
        /*
         *  No option argument.  Make sure next time around we find
         *  the correct option flag character for short options
         */
        if (pOptState->optType == TOPT_SHORT)
            (pOpts->pzCurOpt)++;

        /*
         *  It is a long option.  Make sure there was no ``=xxx'' argument
         */
        else if (pOptState->pzOptArg != (char*)NULL) {
            fprintf( stderr, zNoArg, pOpts->pzProgPath,
                     pOptState->pOD->pz_Name );
            return FAILURE;
        }

        /*
         *  It is a long option.  Advance to next command line argument.
         */
        else
            pOpts->pzCurOpt = (char*)NULL;
    }

    return SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  DO PRESETS
 *
 *  The next several routines do the immediate action pass on the command
 *  line options, then the environment variables then the RC files in
 *  reverse order.  Once done with that, the order is reversed and all
 *  the RC files and environment variables are processed again, this time
 *  only processing the non-immediate action options.  doPresets() will
 *  then return for optionProcess() to do the final pass on the command
 *  line arguments.
 *
 *  doPresets()     is only called by optionProcess().
 *  loadOptionLine  is used by optionLoadLine() and doPresets()
 *  filePreset      is used by doLoadOpt() and doPresets()
 *
 *
 *  optionMakePath  --  translate and construct a path
 *
 *  This routine does environment variable expansion if the first character
 *  is a ``$''.  If it starts with two dollar characters, then the path
 *  is relative to the location of the executable.
 */
ag_bool
optionMakePath( pzBuf, bufSize, pzName, pzProgPath )
    char*    pzBuf;
    size_t   bufSize;
    tCC*     pzName;
    tCC*     pzProgPath;
{
    if (bufSize <= strlen( pzName ))
        return AG_FALSE;

    /*
     *  IF not an environment variable, just copy the data
     */
    if (*pzName != '$') {
        strcpy( pzBuf, pzName );
        return AG_TRUE;
    }

    /*
     *  IF the name starts with "$$", then it must start be "$$" or
     *  it must start with "$$/".  In either event, replace the "$$"
     *  with the path to the executable and append a "/" character.
     */
    if (pzName[1] == '$') {
        tCC*  pzPath;
        tCC*  pz;

        if (strchr( pzProgPath, '/' ) != (char*)NULL)
            pzPath = pzProgPath;
        else {
            pzPath = pathfind( getenv( "PATH" ), (char*)pzProgPath, "rx" );

            if (pzPath == (char*)NULL)
                return AG_FALSE;
        }

        pz = strrchr( pzPath, '/' );

        /*
         *  IF we cannot find a directory name separator,
         *  THEN we do not have a path name to our executable file.
         */
        if (pz == (char*)NULL)
            return AG_FALSE;

        /*
         *  Skip past the "$$" and, maybe, the "/".  Anything else is invalid.
         */
        pzName += 2;
        switch (*pzName) {
        case '/':
            pzName++;
        case NUL:
            break;
        default:
            return AG_FALSE;
        }

        /*
         *  Concatenate the file name to the end of the executable path.
         *  The result may be either a file or a directory.
         */
        if ((pz - pzPath)+1 + strlen(pzName) >= bufSize)
            return AG_FALSE;

        memcpy( pzBuf, pzPath, (pz - pzPath)+1 );
        strcpy( pzBuf + (pz - pzPath) + 1, pzName );
    }

    /*
     *  See if the env variable is followed by specified directories
     *  (We will not accept any more env variables.)
     */
    else {
        char* pzDir = pzBuf;

        for (;;) {
            char ch = *++pzName;
            if (! ISNAMECHAR( ch ))
                break;
            *(pzDir++) = ch;
        }

        if (pzDir == pzBuf)
            return AG_FALSE;

        *pzDir = NUL;

        pzDir = getenv( pzBuf );

        /*
         *  Environment value not found -- skip the home list entry
         */
        if (pzDir == (char*)NULL)
            return AG_FALSE;

        if (strlen( pzDir ) + 1 + strlen( pzName ) >= bufSize)
            return AG_FALSE;

        sprintf( pzBuf, "%s%s", pzDir, pzName );
    }

    return AG_TRUE;
}


/*
 *  doImmediateOpts - scan the command line for immediate action options
 */
STATIC tSuccess
doImmediateOpts( pOpts )
    tOptions*  pOpts;
{
    /*
     *  IF the struct version is not the current, and also
     *     either too large (?!) or too small,
     *  THEN emit error message and fail-exit
     */
    if (  ( pOpts->structVersion  != OPTIONS_STRUCT_VERSION )
       && (  (pOpts->structVersion > OPTIONS_STRUCT_VERSION )
          || (pOpts->structVersion < MIN_OPTION_VERSION     )
       )  )  {
        fprintf( stderr, zBadVer, pOpts->origArgVect[0],
                 NUM_TO_VER( pOpts->structVersion ),
                 NUM_TO_VER( OPTIONS_STRUCT_VERSION ),
                 NUM_TO_VER( MIN_OPTION_VERSION ));
        exit( EXIT_FAILURE );
    }

    {
        const char* pz = strrchr( *pOpts->origArgVect, '/' );

        if (pz == (char*)NULL)
             pOpts->pzProgName = *pOpts->origArgVect;
        else pOpts->pzProgName = pz+1;

        pOpts->pzProgPath = *pOpts->origArgVect;
    }

    pOpts->curOptIdx = 1;     /* start by skipping program name */
    pOpts->pzCurOpt  = NULL;

    /*
     *  when comparing long names, these are equivalent
     */
    strequate( (const char*)"-_^" );

    /*
     *  Examine all the options from the start.  We process any options that
     *  are marked for immediate processing.
     */
    for (;;) {
        tOptState optState = { NULL, OPTST_DEFINED, TOPT_UNDEFINED, 0, NULL };

        tSuccess res = nextOption( pOpts, &optState );
        switch (res) {
        case FAILURE:
            goto optionsDone;

        case PROBLEM:
            /*
             *  FIXME:  Here is where we have to worry about how to reorder
             *          arguments.  Not today.
             */
            return SUCCESS; /* no more args */

        case SUCCESS:
            break;
        }

        /*
         *  IF this *is* an immediate-attribute option, then do it.
         */
        switch (optState.flags & (OPTST_DISABLE_IMM|OPTST_IMM)) {
        case 0:                   /* never */
            continue;

        case OPTST_DISABLE_IMM:   /* do enabled options later */
            if ((optState.flags & OPTST_DISABLED) == 0)
                continue;
            break;

        case OPTST_IMM:           /* do disabled options later */
            if ((optState.flags & OPTST_DISABLED) != 0)
                continue;
            break;

        case OPTST_DISABLE_IMM|OPTST_IMM: /* always */
            break;
        }

        if (! SUCCESSFUL( loadValue( pOpts, &optState )))
            break;
    } optionsDone:;

    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0)
        (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
    return FAILURE;
}


STATIC void
loadOptionLine( pOpts, pOS, pzLine, direction )
    tOptions*  pOpts;
    tOptState* pOS;
    char*      pzLine;
    tDirection direction;
{
    /*
     *  Strip off the first token on the line.
     *  No quoting, space separation only.
     */
    {
        char* pz = pzLine;
        while (  (! isspace( *pz ))
              && (*pz != NUL)
              && (*pz != '=' )  ) pz++;

        /*
         *  IF we exited because we found either a space char or an '=',
         *  THEN terminate the name (clobbering either a space or '=')
         *       and scan over any more white space that follows.
         */
        if (*pz != NUL) {
            *pz++ = NUL;
            while (isspace( *pz )) pz++;
        }

        /*
         *  Make sure we can find the option in our tables and initing it is OK
         */
        if (! SUCCESSFUL( longOptionFind( pOpts, pzLine, pOS )))
            return;
        if (pOS->flags & OPTST_NO_INIT)
            return;

        pOS->pzOptArg = pz;
    }

    switch (pOS->flags & (OPTST_IMM|OPTST_DISABLE_IMM)) {
    case 0:
        /*
         *  The selected option has no immediate action.
         *  THEREFORE, if the direction is PRESETTING
         *  THEN we skip this option.
         */
        if (PRESETTING(direction))
            return;
        break;

    case OPTST_IMM:
        if (PRESETTING(direction)) {
            /*
             *  We are in the presetting direction with an option we handle
             *  immediately for enablement, but normally for disablement.
             *  Therefore, skip if disabled.
             */
            if ((pOS->flags & OPTST_DISABLED) == 0)
                return;
        } else {
            /*
             *  We are in the processing direction with an option we handle
             *  immediately for enablement, but normally for disablement.
             *  Therefore, skip if NOT disabled.
             */
            if ((pOS->flags & OPTST_DISABLED) != 0)
                return;
        }
        break;

    case OPTST_DISABLE_IMM:
        if (PRESETTING(direction)) {
            /*
             *  We are in the presetting direction with an option we handle
             *  immediately for disablement, but normally for disablement.
             *  Therefore, skip if NOT disabled.
             */
            if ((pOS->flags & OPTST_DISABLED) != 0)
                return;
        } else {
            /*
             *  We are in the processing direction with an option we handle
             *  immediately for disablement, but normally for disablement.
             *  Therefore, skip if disabled.
             */
            if ((pOS->flags & OPTST_DISABLED) == 0)
                return;
        }
        break;

    case OPTST_IMM|OPTST_DISABLE_IMM:
        /*
         *  The selected option is always for immediate action.
         *  THEREFORE, if the direction is PROCESSING
         *  THEN we skip this option.
         */
        if (PROCESSING(direction))
            return;
        break;
    }

    /*
     *  Fix up the args.
     */
    switch (pOS->pOD->optArgType) {
    case ARG_NONE:
        if (*pOS->pzOptArg != NUL)
            return;
        pOS->pzOptArg = NULL;
        break;

    case ARG_MAY:
        if (*pOS->pzOptArg == NUL)
            pOS->pzOptArg = NULL;

    case ARG_MUST:
        if (*pOS->pzOptArg == NUL)
             pOS->pzOptArg = "";
        else pOS->pzOptArg = strdup( pOS->pzOptArg );
        break;
    }

    loadValue( pOpts, pOS );
}


/*
 *  filePreset
 *
 *  Load a file containing presetting information (an RC file).
 */
STATIC void
filePreset( pOpts, pzFileName, direction )
    tOptions*     pOpts;
    const char*   pzFileName;
    int           direction;
{
    typedef enum { SEC_NONE, SEC_LOOKING, SEC_PROCESS } teSec;
    teSec   sec     = SEC_NONE;
    FILE*   fp      = fopen( pzFileName, (const char*)"r" FOPEN_BINARY_FLAG );
    u_int   saveOpt = pOpts->fOptSet;
    size_t  secNameLen;
    char    zLine[ 0x1000 ];

    if (fp == (FILE*)NULL)
        return;

    /*
     *  DO NOT STOP ON ERRORS.  During preset, they are ignored.
     */
    pOpts->fOptSet &= ~OPTPROC_ERRSTOP;

    /*
     *  FOR each line in the file...
     */
    while (fgets( zLine, sizeof( zLine ), fp ) != (char*)NULL) {
        char*  pzLine = zLine;

        for (;;) {
            pzLine += strlen( pzLine );

            /*
             *  IF the line is full, we stop...
             */
            if (pzLine >= zLine + (sizeof( zLine )-2))
                break;
            /*
             *  Trim of trailing white space.
             */
            while ((pzLine > zLine) && isspace(pzLine[-1])) pzLine--;
            *pzLine = NUL;
            /*
             *  IF the line is not continued, then exit the loop
             */
            if (pzLine[-1] != '\\')
                break;
            /*
             *  insert a newline and get the continuation
             */
            pzLine[-1] = '\n';
            fgets( pzLine, sizeof( zLine ) - (int)(pzLine - zLine), fp );
        }

        pzLine = zLine;
        while (isspace( *pzLine )) pzLine++;

        switch (*pzLine) {
        case NUL:
        case '#':
            /*
             *  Ignore blank and comment lines
             */
            continue;

        case '[':
            /*
             *  Enter a section IFF sections are requested and the section
             *  name matches.  If the file is not sectioned,
             *  then all will be handled.
             */
            if (pOpts->pzPROGNAME == (char*)NULL)
                goto fileDone;

            switch (sec) {
            case SEC_NONE:
                sec = SEC_LOOKING;
                secNameLen = strlen( pOpts->pzPROGNAME );
                /* FALLTHROUGH */

            case SEC_LOOKING:
                if (  (strncmp( pzLine+1, pOpts->pzPROGNAME, secNameLen ) != 0)
                   || (pzLine[secNameLen+1] != ']')  )
                    continue;
                sec = SEC_PROCESS;
                break;

            case SEC_PROCESS:
                goto fileDone;
            }
            break;

        default:
            /*
             *  Load the line only if we are not in looking-for-section state
             */
            if (sec == SEC_LOOKING)
                continue;
        }

        {
            tOptState st = { NULL, OPTST_PRESET, TOPT_UNDEFINED, 0, NULL };
            loadOptionLine( pOpts, &st, pzLine, direction );
        }
    } fileDone:;

    pOpts->fOptSet = saveOpt;
    fclose( fp );
}


/*
 *  doEnvPresets - check for preset values from the envrionment
 *  This routine should process in all, immediate or normal modes....
 */
STATIC void
doEnvPresets( pOpts, type )
    tOptions*       pOpts;
    teEnvPresetType type;
{
    int        ct;
    tOptState  st;
    char*      pzFlagName;
    size_t     spaceLeft;
    char       zEnvName[ 128 ];

    /*
     *  Finally, see if we are to look at the environment
     *  variables for initial values.
     */
    if ((pOpts->fOptSet & OPTPROC_ENVIRON) == 0)
        return;

    ct  = pOpts->presetOptCt;
    st.pOD = pOpts->pOptDesc;

    pzFlagName = zEnvName
        + snprintf( zEnvName, sizeof( zEnvName ), "%s_", pOpts->pzPROGNAME );
    spaceLeft = sizeof( zEnvName ) - (pzFlagName - zEnvName) - 1;

    for (;ct-- > 0; st.pOD++) {
        /*
         *  If presetting is disallowed, then skip this entry
         */
        if ((st.pOD->fOptState & OPTST_NO_INIT) != 0)
            continue;

        /*
         *  IF there is no such environment variable,
         *  THEN skip this entry, too.
         */
        if (strlen( st.pOD->pz_NAME ) >= spaceLeft)
            continue;

        /*
         *  Set up the option state
         */
        strcpy( pzFlagName, st.pOD->pz_NAME );
        st.pzOptArg = getenv( zEnvName );
        if (st.pzOptArg == (char*)NULL)
            continue;
        st.flags    = OPTST_PRESET | st.pOD->fOptState;
        st.optType  = TOPT_UNDEFINED;
        st.argType  = 0;

        if (  (st.pOD->pz_DisablePfx != (char*)NULL)
           && (streqvcmp( st.pzOptArg, st.pOD->pz_DisablePfx ) == 0)) {
            st.flags |= OPTST_DISABLED;
            st.pzOptArg = (char*)NULL;
        }

        switch (type) {
        case ENV_IMM:
            /*
             *  Process only immediate actions
             */
            if (st.flags & OPTST_DISABLED) {
                if ((st.flags & OPTST_DISABLE_IMM) == 0)
                    continue;
            } else {
                if ((st.flags & OPTST_IMM) == 0)
                    continue;
            }
            break;

        case ENV_NON_IMM:
            /*
             *  Process only NON immediate actions
             */
            if (st.flags & OPTST_DISABLED) {
                if ((st.flags & OPTST_DISABLE_IMM) != 0)
                    continue;
            } else {
                if ((st.flags & OPTST_IMM) != 0)
                    continue;
            }
            break;

        default: /* process everything */
            break;
        }

        /*
         *  Make sure the option value string is persistent and consistent.
         *  This may be a memory leak, but we cannot do anything about it.
         *
         *  The interpretation of the option value depends
         *  on the type of value argument the option takes
         */
        if (st.pzOptArg != (char*)NULL)
            switch (st.pOD->optArgType) {
            case ARG_MAY:
                if (*st.pzOptArg == NUL) {
                    st.pzOptArg = (char*)NULL;
                    break;
                }
                /* FALLTHROUGH */

            case ARG_MUST:
                if (*st.pzOptArg == NUL)
                     st.pzOptArg = "";
                else st.pzOptArg = strdup( st.pzOptArg );
                break;

            default: /* no argument allowed */
                st.pzOptArg = (char*)NULL;
                break;
            }

        loadValue( pOpts, &st );
    }
}


/*
 *  doPresets - check for preset values from an rc file or the envrionment
 */
STATIC tSuccess
doPresets( pOpts )
    tOptions*  pOpts;
{
#   define SKIP_RC_FILES \
    DISABLED_OPT(&(pOpts->pOptDesc[ pOpts->specOptIdx.save_opts+1]))

    {
        tSuccess  res = doImmediateOpts( pOpts );
        if (! SUCCESSFUL( res ))
            return res;
    }

    /*
     *  IF there are no RC files,
     *  THEN do any environment presets and leave.
     */
    if (  (pOpts->papzHomeList == (const char**)NULL)
       || SKIP_RC_FILES )  {
        doEnvPresets( pOpts, ENV_ALL );
        return SUCCESS;
    }

    doEnvPresets( pOpts, ENV_IMM );

    {
        int   idx;
        int   inc = DIRECTION_PRESET;
        tCC*  pzPath;
        char  zFileName[ 4096 ];

        /*
         *  Find the last RC entry (highest priority entry)
         */
        for (idx = 0; pOpts->papzHomeList[ idx+1 ] != NULL; ++idx)  ;

        /*
         *  For every path in the home list, ...  *TWICE* We start at the last
         *  (highest priority) entry, work our way down to the lowest priority,
         *  handling the immediate options.
         *  Then we go back up, doing the normal options.
         */
        for (;;) {
            struct stat StatBuf;

            /*
             *  IF we've reached the bottom end, change direction
             */
            if (idx < 0) {
                inc = DIRECTION_PROCESS;
                idx = 0;
            }

            pzPath = pOpts->papzHomeList[ idx ];

            /*
             *  IF we've reached the top end, bail out
             */
            if (pzPath == (char*)NULL)
                break;

            idx += inc;

            if (! optionMakePath( zFileName, sizeof( zFileName ),
                                  pzPath, pOpts->pzProgPath ))
                continue;

            /*
             *  IF the file name we constructed is a directory,
             *  THEN append the Resource Configuration file name
             *  ELSE we must have the complete file name
             */
            if (stat( zFileName, &StatBuf ) != 0)
                continue; /* bogus name - skip the home list entry */

            if (S_ISDIR( StatBuf.st_mode )) {
                size_t len = strlen( zFileName );
                char* pz;

                if (len + 1 + strlen( pOpts->pzRcName ) >= sizeof( zFileName ))
                    continue;

                pz = zFileName + len;
                if (pz[-1] != '/')
                    *(pz++) = '/';
                strcpy( pz, pOpts->pzRcName );
            }

            filePreset( pOpts, zFileName, inc );

            /*
             *  IF we are now to skip RC files AND we are presetting,
             *  THEN change direction.  We must go the other way.
             */
            if ((SKIP_RC_FILES) && PRESETTING(inc)) {
                idx -= inc;  /* go back and reprocess current file */
                inc =  DIRECTION_PROCESS;
            }
        } /* For every path in the home list, ... */
    }

    doEnvPresets( pOpts, ENV_NON_IMM );
    return SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  VERIFY OPTION CONSISTENCY
 *
 *  Make sure that the argument list passes our consistency tests.
 */
STATIC int
checkConsistency( pOpts )
    tOptions*  pOpts;
{
    int       errCt = 0;

    tSCC zCantFmt[]   = "ERROR:  %s option conflicts with the %s option\n";
    tSCC zReqFmt[]    = "ERROR:  %s option requires the %s option\n";

    tOptDesc*  pOD = pOpts->pOptDesc;
    int        oCt = pOpts->presetOptCt;

    /*
     *  FOR each of "oCt" options, ...
     */
    for (;;) {
        const int*  pMust = pOD->pOptMust;
        const int*  pCant = pOD->pOptCant;

        /*
         *  IF the current option was provided on the command line
         *  THEN ensure that any "MUST" requirements are not
         *       "DEFAULT" (unspecified) *AND* ensure that any
         *       "CANT" options have not been SET or DEFINED.
         */
        if (SELECTED_OPT(pOD)) {
            if (pMust != (const int*)NULL) for (;;) {
                tOptDesc*  p = pOpts->pOptDesc + *(pMust++);
                if (UNUSED_OPT(p)) {
                    const tOptDesc* pN = pOpts->pOptDesc + pMust[-1];
                    errCt++;
                    fprintf( stderr, zReqFmt, pOD->pz_Name, pN->pz_Name );
                }

                if (*pMust == NO_EQUIVALENT)
                    break;
            }

            if (pCant != (const int*)NULL) for (;;) {
                tOptDesc*  p = pOpts->pOptDesc + *(pCant++);
                if (SELECTED_OPT(p)) {
                    const tOptDesc* pN = pOpts->pOptDesc + pCant[-1];
                    errCt++;
                    fprintf( stderr, zCantFmt, pOD->pz_Name, pN->pz_Name );
                }

                if (*pCant == NO_EQUIVALENT)
                    break;
            }
        }

        /*
         *  IF       this option is not equivalenced to another,
         *        OR it is equivalenced to itself (is the equiv. root)
         *  THEN we need to make sure it occurrs often enough.
         */
        if (  (pOD->optEquivIndex == NO_EQUIVALENT)
           || (pOD->optEquivIndex == pOD->optIndex) )   do {
            tSCC zNotEnough[] = "ERROR:  The %s option must appear %d times\n";
            tSCC zNeedOne[]   = "ERROR:  The %s option is required\n";

            /*
             *  IF the occurrance counts have been satisfied,
             *  THEN there is no problem.
             */
            if (pOD->optOccCt >= pOD->optMinCt)
                break;

            /*
             *  IF presetting is okay and it has been preset,
             *  THEN min occurrance count doesn't count
             */
#           define PRESET_OK  (OPTST_PRESET | OPTST_MUST_SET)
            if ((pOD->fOptState & PRESET_OK) == PRESET_OK)
                break;

            errCt++;
            if (pOD->optMinCt > 1)
                fprintf( stderr, zNotEnough, pOD->pz_Name, pOD->optMinCt );
            else fprintf( stderr, zNeedOne, pOD->pz_Name );
           } while (0);

        if (--oCt <= 0)
            break;
        pOD++;
    }

    /*
     *  IF we are stopping on errors, check to see if any remaining
     *  arguments are required to be there or prohibited from being there.
     */
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {

        /*
         *  Check for prohibition
         */
        if ((pOpts->fOptSet & OPTPROC_NO_ARGS) != 0) {
            if (pOpts->origArgCt > pOpts->curOptIdx) {
                fprintf( stderr, "%s: Command line arguments not allowed\n",
                         pOpts->pzProgName );
                ++errCt;
            }
        }

        /*
         *  ELSE not prohibited, check for being required
         */
        else if ((pOpts->fOptSet & OPTPROC_ARGS_REQ) != 0) {
            if (pOpts->origArgCt <= pOpts->curOptIdx) {
                fprintf( stderr, "%s: Command line arguments required\n",
                         pOpts->pzProgName );
                ++errCt;
            }
        }
    }

    return errCt;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  THESE ROUTINES ARE CALLABLE FROM THE GENERATED OPTION PROCESSING CODE
 */
/*=--subblock=arg=arg_type,arg_name,arg_desc =*/
/*=*
 * library:  opts
 * header:   your-opts.h
 *
 * lib_description:
 *
 *  These are the routines that libopts users may call directly from their
 *  code.  There are several other routines that can be called by code
 *  generated by the libopts option templates, but they are not to be
 *  called from any other user code.  The @file{options.h} is fairly clear
 *  about this, too.
=*/
/*=export_func  optionLoadLine
 *
 * what:  process a string for an option name and value
 *
 * arg:   tOptions*,   pOpts,  program options descriptor
 * arg:   const char*, pzLine, NUL-terminated text
 *
 * doc:
 *
 * This is a user callable routine for setting options from, for
 * example, the contents of a file that they read in.
 * Only one option may appear in the text.  It will be treated
 * as a normal (non-preset) option.
 *
 * When passed a pointer to the option struct and a string, it will
 * find the option named by the first token on the string and set
 * the option argument to the remainder of the string.  The caller must
 * NUL terminate the string.  Any embedded new lines will be included
 * in the option argument.
 *
 * err:   Invalid options are silently ignored.  Invalid option arguments
 *        will cause a warning to print, but the function should return.
=*/
void optionLoadLine( pOpts, pzLine )
    tOptions*  pOpts;
    tCC*       pzLine;
{
    tOptState st = { NULL, OPTST_SET, TOPT_UNDEFINED, 0, NULL };
    char* pz = strdup( pzLine );
    loadOptionLine( pOpts, &st, pz, DIRECTION_PROCESS );
}


/*
 *  doLoadOpt
 *
 *  This is callable from the option descriptor.
 *  It is referenced when homerc files are enabled.
 */
void doLoadOpt( pOpts, pOptDesc )
    tOptions*  pOpts;
    tOptDesc*  pOptDesc;
{
    /*
     *  IF the option is not being disabled,
     *  THEN load the file.  There must be a file.
     *  (If it is being disabled, then the disablement processing
     *  already took place.  It must be done to suppress preloading
     *  of ini/rc files.)
     */
    if (! DISABLED_OPT( pOptDesc )) {
        struct stat sb;
        if (stat( pOptDesc->pzLastArg, &sb ) != 0) {
            tSCC zMsg[] =
                "File error %d (%s) opening %s for loading options\n";

            if ((pOpts->fOptSet & OPTPROC_ERRSTOP) == 0)
                return;

            fprintf( stderr, zMsg, errno, strerror( errno ),
                     pOptDesc->pzLastArg );
            (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
            /* NOT REACHED */
        }

        if (! S_ISREG( sb.st_mode )) {
            tSCC zMsg[] =
                "error:  cannot load options from non-regular file %s\n";

            if ((pOpts->fOptSet & OPTPROC_ERRSTOP) == 0)
                return;

            fprintf( stderr, zMsg, pOptDesc->pzLastArg );
            (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
            /* NOT REACHED */
        }

        filePreset( pOpts, pOptDesc->pzLastArg, DIRECTION_PROCESS );
    }
}


/*=export_func  optionVersion
 *
 * what:     return the compiled AutoOpts version number
 * ret_type: const char*
 * ret_desc: the version string in constant memory
 * doc:
 *  Returns the full version string compiled into the library.
 *  The returned string cannot be modified.
=*/
const char*
optionVersion()
{
    static const char zVersion[] =
        STR( AO_CURRENT.AO_REVISION );

    return zVersion;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*=export_func optionProcess
 *
 * what: this is the main option processing routine
 *
 * arg:  + tOptions* + pOpts + program options descriptor +
 * arg:  + int       + argc  + program arg count  +
 * arg:  + char**    + argv  + program arg vector +
 * ret_type:  int
 * ret_desc:  the count of the arguments processed
 *
 * doc:
 *
 * This is the main entry point for processing options.  It is intended
 * that this procedure be called once at the beginning of the execution of
 * a program.  Depending on options selected earlier, it is sometimes
 * necessary to stop and restart option processing, or to select completely
 * different sets of options.  This can be done easily, but you generally
 * do not want to do this.
 *
 * The number of arguments processed always includes the program name.
 * If one of the arguments is "--", then it is counted and the
 * processing stops.  If an error was encountered and errors are
 * to be tolerated, then the returned value is the index of the
 * argument causing the error.
 *
 * err:  Errors will cause diagnostics to be printed.  @code{exit(3)} may
 *       or may not be called.  It depends upon whether or not the options
 *       were generated with the "allow-errors" attribute, or if the
 *       ERRSKIP_OPTERR or ERRSTOP_OPTERR macros were invoked.
=*/
int
optionProcess( pOpts, argCt, argVect )
    tOptions*  pOpts;
    int        argCt;
    char**     argVect;
{
    /*
     *  Establish the real program name, the program full path,
     *  and do all the presetting the first time thru only.
     */
    if ((pOpts->fOptSet & OPTPROC_INITDONE) == 0) {
        pOpts->origArgCt   = argCt;
        pOpts->origArgVect = argVect;
        pOpts->fOptSet    |= OPTPROC_INITDONE;

        if (FAILED( doPresets( pOpts )))
            return 0;

        pOpts->curOptIdx = 1;
        pOpts->pzCurOpt = (char*)NULL;
    }

    /*
     *  IF we are (re)starting,
     *  THEN reset option location
     */
    else if (pOpts->curOptIdx <= 0) {
        pOpts->curOptIdx = 1;
        pOpts->pzCurOpt = (char*)NULL;
    }

    /*
     *  Now, process all the options from our current position onward.
     *  (This allows interspersed options and arguments for the few
     *  non-standard programs that require it.)
     */
    for (;;) {
        tOptState optState = { NULL, OPTST_DEFINED, TOPT_UNDEFINED, 0, NULL };

        switch (nextOption( pOpts, &optState )) {
        case FAILURE:
            if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0)
                (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
            goto optionsBad;

        case PROBLEM:
            goto optionsDone;

        case SUCCESS:
            break;
        }

        /*
         *  IF this is not an immediate-attribute option, then do it.
         */
        switch (optState.flags & (OPTST_DISABLE_IMM|OPTST_IMM)) {
        case 0:                   /* always */
            break;

        case OPTST_DISABLE_IMM:   /* disabled options already done */
            if ((optState.flags & OPTST_DISABLED) != 0)
                continue;
            break;

        case OPTST_IMM:           /* enabled options already done */
            if ((optState.flags & OPTST_DISABLED) == 0)
                continue;
            break;

        case OPTST_DISABLE_IMM|OPTST_IMM: /* opt already done */
            continue;
        }

        if (! SUCCESSFUL( loadValue( pOpts, &optState ))) {
            if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0)
                (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
            break;
        }
    }

 optionsBad:
    return pOpts->origArgCt;

 optionsDone:

    /*
     *  IF    there were no errors
     *    AND we have RC/INI files
     *    AND there is a request to save the files
     *  THEN do that now before testing for conflicts.
     *       (conflicts are ignored in preset options)
     */
    if (pOpts->specOptIdx.save_opts != 0) {
        tOptDesc*  pOD = pOpts->pOptDesc + pOpts->specOptIdx.save_opts;

        if (SELECTED_OPT( pOD )) {
            optionSaveFile( pOpts );
            exit( EXIT_SUCCESS );
        }
    }

    /*
     *  IF we are checking for errors,
     *  THEN look for too few occurrences of required options
     */
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
        if (checkConsistency( pOpts ) != 0)
            (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
    }

    return pOpts->curOptIdx;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * autoopts.c ends here */
