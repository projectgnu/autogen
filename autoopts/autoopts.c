
/*
 *  $Id: autoopts.c,v 1.2 1998/06/17 20:21:07 bkorb Exp $
 *
 *  This file contains all of the routines that must be linked into
 *  an executable to use the generated option processing.  The optional
 *  routines are in separately compiled modules so that they will not
 *  necessarily be linked in.
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

#ident "$Id: autoopts.c,v 1.2 1998/06/17 20:21:07 bkorb Exp $"

tSCC zMisArg[]      = "%s: option `%s' requires an argument\n";
tSCC zNoArg[]       = "%s: option `%s' doesn't allow an argument\n";
tSCC zIllOptChr[]   = "%s: illegal option -- %c\n";
tSCC zIllOptStr[]   = "%s: %s option -- %s\n";
tSCC zIllegal[]     = "illegal";
tSCC zAmbiguous[]   = "ambiguous";


STATIC ag_bool loadValue( tOptions* pOpts, tOptDesc* pOD );
STATIC void loadPresetValue( tOptions*  pOpts, tOptDesc*  pOD );
STATIC tOptDesc* longOptionFind( tOptions*  pOpts, char* pzOptName,
                                 u_long* pFlags );
STATIC tOptDesc* shortOptionFind( tOptions*  pOpts, tUC optValue );
STATIC void filePreset( tOptions*  pOpts, const char* pzFileName );
STATIC void doPresets( tOptions*  pOpts );
STATIC tOptDesc* optionGet( tOptions* pOpts, int argCt, char** argVect );

#define NO_OPT_DESC (tOptDesc*)(~0)

/*
 *  loadValue
 *
 *  This routine handles equivalencing and invoking the handler procedure,
 *  if any.
 */
    STATIC ag_bool
loadValue( tOptions* pOpts, tOptDesc* pOD )
{
    /*
     *  Save a copy of the option procedure pointer.
     *  If this is an equivalence class option, we still want this proc.
     */
    tOptProc*  pOP    = pOD->pOptProc;

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
         *  Record the non-persistent options from the original option
         *  and add in the OPTST_EQUIVALENCE flag.
         */
        u_short f = (pOD->fOptState & ~OPTST_PERSISTENT) | OPTST_EQUIVALENCE;

        /*
         *  clear out non-persistent options and
         *  insert the options saved in 'f'.
         */
        p->fOptState &= OPTST_PERSISTENT;
        p->fOptState |= f;
        p->pzLastArg  = pOD->pzLastArg;

        p->optActualValue = pOD->optValue;
        p->optActualIndex = pOD->optIndex;
        pOD = p;

    } else {
        pOD->optActualValue = pOD->optValue;
        pOD->optActualIndex = pOD->optIndex;
    }

    /*
     *  Keep track of count only for DEFINED (command line) options.
     *  IF we have too many, build up an error message and bail.
     */
    if (  (pOD->fOptState & OPTST_DEFINED)
       && (++pOD->optOccCt > pOD->optMaxCt)  )  {
        tSCC zOnlyOne[]   = "one %s%s option allowed\n";
        tSCC zAtMost[]    = "%4$d %1$s%s options allowed\n";
        tSCC zEquiv[]     = "-equivalence";
        const char* pzEqv = (pOD->optEquivIndex != NO_EQUIVALENT)
                          ? zEquiv : zEquiv + sizeof( zEquiv )-1;
        tSCC zErrOnly[]   = "ERROR:  only ";

        if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
            const char* pzFmt = (pOD->optMaxCt > 1) ? zAtMost : zOnlyOne;
            fputs( zErrOnly, stderr );
            fprintf( stderr, pzFmt, pOD->pz_Name, pzEqv,
                     pOD->optMaxCt );
        }

        return AG_FALSE;
    }

    /*
     *  If provided a procedure to call, call it
     */
    if (pOP != (tpOptProc)NULL)
        (*pOP)( pOpts, pOD );
    return AG_TRUE;
}


/*
 *  For preset values, we will allways have an argument pointer.
 *  We do a special hack:  when arguments are prohibited, we use
 *  the argument for an indicator to say if it should be regarded
 *  as a disabled option ('+' marked), regular option ('-' marked),
 *  or purged from the set of preset options.  Thus, if you run the
 *  following in your Bourne shell:
 *
 *     MYPROG_FOO_OPT=NO ; export MYPROG_FOO_OPT
 *
 *  then after processing all the presets, the "myprog" program
 *  will have the "Foo-Opt" option removed from the set of
 *  selected options.  NB: Foo-Opt must not be equivalenced to
 *  another option.  On the other hand:
 *
 *     MYPROG_FOO_OPT=FALSE ; export MYPROG_FOO_OPT
 *
 *  This will cause Foo-Opt to be marked as a disabled option,
 *  whether or not '+' options are normally processed.
 *  ALSO:  options that take arguments cannot be deleted or marked
 *  as 'disabled'.  These options should use empty/non-existent
 *  arguments to indicate such things.
 */
    STATIC void
loadPresetValue( tOptions*  pOpts, tOptDesc*  pOD )
{
    /*
     *  IF the option is numeric,
     *  THEN make sure it is not an omit (no/NO) value,
     *       and then set the pointer to the numeric value
     */
    if ((pOD->fOptState & OPTST_NUMERIC) != 0) {
        char ch = *pOD->pzLastArg;
        if ((ch == 'n') || (ch == 'N')) {
             pOD->fOptState &= OPTST_PERSISTENT;
             pOD->pzLastArg  = (char*)NULL;
        }

        else
            pOD->pzLastArg = (char*)strtol( pOD->pzLastArg, (char**)NULL, 0 );

    /*
     *  OTHERWISE, the interpretation of the option value depends
     *  on the type of value argument the option takes
     */
    } else switch (pOD->optArgType ) {
    case ARG_MAY:
        /*
         *  OPTIONAL ARGUMENT
         */
        if (*pOD->pzLastArg == NUL)
             pOD->pzLastArg = (char*)NULL;
        else pOD->pzLastArg = strdup( pOD->pzLastArg );
        break;

    case ARG_MUST:
        /*
         *  REQUIRED ARGUMENT
         */
        if (*pOD->pzLastArg == NUL)
             pOD->pzLastArg = "";
        else pOD->pzLastArg = strdup( pOD->pzLastArg );
        break;

    default: /* no argument allowed */
        /*
         *  NO ARGUMENT ALLOWED
         */
        switch (*pOD->pzLastArg) {
        case 'n': case 'N':
            /*
             *  'N'o, we do not want this option set
             *  We clear the option and do not process it
             */
            pOD->fOptState &= OPTST_PERSISTENT;
            break;

        case '+':
        case 'f': case 'F':
            /*
             *  Set this option to 'F'alse (opposite of '-' option)
             */
            pOD->fOptState |= OPTST_DISABLED;
            break;

        case '-': case NUL:
        case 'y': case 'Y': case 't': case 'T':
            /*
             *  'Y'es, it is 'T'rue we want this regular option.
             */
        default:
            break;
        }
        pOD->pzLastArg = (char*)NULL;
        break;
    }

    /*
     *  IF the preset value does not indicate dropping the value,
     *  THEN load the value.
     */
    if ((pOD->fOptState & OPTST_SET_MASK) != OPTST_INIT)
        loadValue( pOpts, pOD );
}



    STATIC tOptDesc*
longOptionFind( tOptions*  pOpts,
                char*      pzOptName,
                u_long*    pFlags )
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

    for (;;) {
        if (strneqvcmp( pzOptName, pOD->pz_Name, nameLen ) == 0) {

            /*
             *  Remember the index for later.
             */
            matchIdx = idx;
    
            /*
             *  IF we have a complete match
             *  THEN it takes priority over any already located partial
             */
            if (pOD->pz_Name[ nameLen ] == NUL) {
                matchCt = 1;
                break;
            }
    
            matchCt++;
        }
        if (++idx >= idxLim)
            break;
        pOD++;
    }

    if (pzEq != (char*)NULL)
        *pzEq = '=';

    /*
     *  Make sure we either found an exact match or found only one partial
     */
    if (matchCt == 1) {
        /*
         *  IF we found a disablement name,
         *  THEN set the bit in the callers' flag word
         */
        if (disable)
            *pFlags |= OPTST_DISABLED;
        pOD = pOpts->pOptDesc + matchIdx;

        return pOD;
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

    return (tOptDesc*)NULL;
}


    STATIC tOptDesc*
shortOptionFind( tOptions*  pOpts,
                 tUC        optValue )
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
        if (optValue == pRes->optValue)
            return pRes;

        /*
         *  Advance to next option description
         */
        pRes++;

        /*
         *  IF we have searched everything, ...
         */
        if (--ct <= 0) {
            /*
             *  IF    the character value is a digit
             *    AND there is a special number option ("-nn")
             *  THEN the result is the "option" itself and the
             *       option is the specially marked "number" option.
             */
            if (   isdigit( optValue )
               && (pOpts->specOptIdx.number_option != NO_EQUIVALENT) ) {
                pRes = pOpts->pOptDesc + pOpts->specOptIdx.number_option;
                (pOpts->pzCurOpt)--;
                return pRes;
            }

            /*
             *  IF we are to stop on errors (the default, actually)
             *  THEN call the usage procedure.
             */
            if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
                fprintf( stderr, zIllOptChr, pOpts->pzProgPath, optValue );
                (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );
            }
            return (tOptDesc*)NULL;
        }
    }
}



    STATIC void
loadOptionLine( tOptions*  pOpts, u_long optFlag, char* pzLine )
{
    char*  pzOptionArg;

    if (*pzLine == '+') {
        optFlag |= OPTST_DISABLED;
        while (isspace( *++pzLine )) /*NULL*/;
    }
    else if (*pzLine == '-')
        while (isspace( *++pzLine )) /*NULL*/;

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

        pzOptionArg = pz;
    }

    /*
     *  IF    we can find the option
     *    AND the option has not been set from the command line
     * ...
     */
    {
        tOptDesc*  pOD = longOptionFind( pOpts, pzLine, &optFlag );
        if ((pOD != (tOptDesc*)NULL) && ((pOD->fOptState & OPTST_SET) == 0)) {
            /*
             *  Clear out non-persistent flags and set our new collection
             */
            pOD->fOptState &= OPTST_PERSISTENT;
            pOD->fOptState |= optFlag;
            pOD->pzLastArg  = pzOptionArg;

            loadPresetValue( pOpts, pOD );
        }
    }
}

    void
optionLoadLine( tOptions*  pOpts, char*  pzLine )
{
    loadOptionLine( pOpts, 0, pzLine );
}


    STATIC void
filePreset( tOptions*  pOpts, const char* pzFileName )
{
    FILE*  fp  = fopen( pzFileName, (const char*)"r" );
    u_int saveOpt = pOpts->fOptSet;

    char  zLine[ 0x1000 ];

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
        u_long optFlag = OPTST_PRESET;

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

        /*
         *  Ignore blank and comment lines
         */
        if ((*pzLine == NUL) || (*pzLine == '#'))
            continue;

        loadOptionLine( pOpts, optFlag, pzLine );
    }

    pOpts->fOptSet = saveOpt;
    fclose( fp );
}


/*
 *  doPresets - check for preset values from an rc file or the envrionment
 */
    STATIC void
doPresets( tOptions*  pOpts )
{
    u_int fOptSet = pOpts->fOptSet;

    /*
     *  when comparing long names, these are equivalent
     */
    strequate( (const char*)"_-^" );

    /*
     *  FIRST, see if we are to look for an rc file where the program
     *  was found.  These values will have the lowest priority.
     */
    if ((fOptSet & OPTPROC_EXERC) != 0) do {
        char   zFileName[ 1024 ];
        char*  pzPath = pathfind( getenv( (const char*)"PATH" ),
                                  pOpts->pzProgPath, (const char*)"x" );
        char* pz;

        if (pzPath == (char*)NULL)
            break;

        pz = strrchr( pzPath, DIR_SEP_CHAR );
        /*
         *  IF we cannot find a directory name separator,
         *  THEN we do not have a path name to our executable file.
         */
        if (pz == (char*)NULL)
            break;

        *pz = NUL;

        /*
         *  Concatenate the rc file name to the end of the executable path and
         *  see if we can find that file and process it.
         */
        strcpy( zFileName, pzPath );
        pz = zFileName + strlen( zFileName );
        if (pz[-1] != DIR_SEP_CHAR)
            *(pz++) = DIR_SEP_CHAR;
        strcpy( pz, pOpts->pzRcName );

        filePreset( pOpts, zFileName );
    } while (AG_FALSE);

    /*
     *  Next, search the list of "home" directories.
     *  Each entry will either be an environment variable name
     *  with a '$' prefixed (e.g. "$HOME"), or a path name
     *  (either relative or absolute, the file system interprets).
     */
    if (pOpts->papzHomeList != (const char**)NULL) {
        const char** papzHL = pOpts->papzHomeList;
        for (;;) {
            const char* pzPath = *(papzHL++);
            char        zFileName[ 1024 ];

            /*
             *  Break when done
             */
            if (pzPath == (char*)NULL)
                break;

            /*
             *  IF not an environment variable, just copy the data
             */
            if (*pzPath != '$') {
                strcpy( zFileName, pzPath );
            } else {
                /*
                 *  See if the env variable is followed by specified directories
                 *  (We will not accept any more env variables.)
                 */
                char* pzDir = strchr( pzPath+1, DIR_SEP_CHAR );
                char* pzEnv;

                if (pzDir != (char*)NULL)
                    *pzDir = NUL;

                pzEnv = getenv( pzPath+1 );

                /*
                 *  Environment value not found -- skip the home list entry
                 */
                if (pzEnv == (char*)NULL)
                    continue;

                strcpy( zFileName, pzEnv );

                /*
                 *  IF we found a directory that followed the env variable,
                 *  THEN tack it onto the value we found
                 */
                if (pzDir != (char*)NULL) {
                    pzEnv = zFileName + strlen( zFileName );
                    if (pzEnv[-1] != DIR_SEP_CHAR)
                        *(pzEnv++) = DIR_SEP_CHAR;
                    strcpy( pzEnv, pzDir+1 );
                    *pzDir = DIR_SEP_CHAR;
                }
            }

            /*
             *  IF the file name we constructed is a directory,
             *  THEN append the Resource Configuration file name
             *  ELSE we must have the complete file name
             */
            {
                struct stat StatBuf;
                if (stat( zFileName, &StatBuf ) != 0)
                    continue; /* bogus name - skip the home list entry */

                if (S_ISDIR( StatBuf.st_mode )) {
                    char* pz = zFileName + strlen( zFileName );
                    if (pz[-1] != DIR_SEP_CHAR)
                        *(pz++) = DIR_SEP_CHAR;

                    strcpy( pz, pOpts->pzRcName );
                }
            }

            filePreset( pOpts, zFileName );
        }
    }

    /*
     *  Finally, see if we are to look at the environment
     *  variables for initial values.
     */
    if ((pOpts->fOptSet & OPTPROC_ENVIRON) != 0) {
        int           ct  = pOpts->presetOptCt;
        tOptDesc*     pOD = pOpts->pOptDesc;
        char          zEnvName[ 64 ];
        char*         pzFlagName;
        strcpy( zEnvName, pOpts->pzPROGNAME );
        pzFlagName = zEnvName + strlen( zEnvName );
        *(pzFlagName++) = '_';

        for (;ct-- > 0; pOD++) {
            char*  pz;

            /*
             *  If presetting is disallowed, then skip this entry
             */
            if ((pOD->fOptState & OPTST_NO_INIT) != 0)
                continue;

            strcpy( pzFlagName, pOD->pz_NAME );
            pz = getenv( zEnvName );

            if (pz != (char*)NULL) {
                /*
                 *  Environment options *CANNOT* be '+'-marked
                 */
                pOD->fOptState &= OPTST_PERSISTENT;
                pOD->fOptState |= OPTST_PRESET;
                pOD->pzLastArg  = pz;

                loadPresetValue( pOpts, pOD );
            }
        }
    }
}


    STATIC tOptDesc*
optionGet( tOptions*   pOpts, int argCt, char** argVect )
{
    ag_bool   isLongOpt      = AG_FALSE;
    tOptDesc* pRes           = (tOptDesc*)NULL;
    u_long    optFlags       = OPTST_DEFINED;

    /*
     *  IF we are starting,
     *  THEN reset values
     */
    if (pOpts->curOptIdx <= 0) {
        pOpts->curOptIdx = 1;
        pOpts->pzCurOpt = (char*)NULL;
    }

    /*
     *  IF we are continuing a short option list (i.e. -xyz...)
     *  THEN continue a single flag option.
     *  OTHERWISE see if there is room to advance and then do so.
     */
    if ((pOpts->pzCurOpt != (char*)NULL) && (*pOpts->pzCurOpt != NUL)) {
        if ((pOpts->fOptSet & OPTPROC_DISABLEDOPT) != 0)
            optFlags |= OPTST_DISABLED;

        pRes = shortOptionFind( pOpts, *pOpts->pzCurOpt );
    }

    else {
        char  firstChar, secondChar;

        if (pOpts->curOptIdx >= argCt)
            return pRes; /* NORMAL COMPLETION */

        pOpts->pzCurOpt = argVect[ pOpts->curOptIdx ];

        /*
         *  IF all arguments must be options, ...
         */
        if ((pOpts->fOptSet & (OPTPROC_SHORTOPT | OPTPROC_LONGOPT)) == 0) {
            char* pz = pOpts->pzCurOpt;
            pOpts->curOptIdx++;

            /*
             *  Skip over any flag/option markers.
             *  In this mode, they are not required.
             */
            if (pOpts->fOptSet & OPTPROC_DISABLEOK) {
                if (*pz == '+')
                    optFlags |= OPTST_DISABLED;

                while ((*pz == '+') || (*pz == '-')) pz++;
            }

            else while (*pz == '-') pz++;

            isLongOpt = AG_TRUE;
            pRes = longOptionFind( pOpts, pz, &optFlags );
        }

        else {
            /*
             *  Note the kind of flag/option marker
             */
            firstChar = *((pOpts->pzCurOpt)++);
            switch (firstChar) {
            case '-':
                break;

            case '+':
                if (pOpts->fOptSet & OPTPROC_DISABLEOK) {
                    optFlags |= OPTST_DISABLED;
                    break;
                }
                /* FALLTHROUGH */
            default:
                return pRes; /* NORMAL COMPLETION - rest are args */
            }

            /*
             *  The current argument is to be processed as an option argument
             */
            pOpts->curOptIdx++;

            /*
             *  We have an option marker.
             *  Test the next character for long option indication
             */
            secondChar = *pOpts->pzCurOpt;
            switch (secondChar) {
            case '+':
            case '-':
                if (firstChar == secondChar) {
                    if (*++(pOpts->pzCurOpt) == NUL)
                        return pRes; /* NORMAL COMPLETION */

                    if ((pOpts->fOptSet & OPTPROC_LONGOPT) == 0) {
                        fprintf( stderr, zIllOptStr, pOpts->pzProgPath,
                                 zIllegal, pOpts->pzCurOpt-2 );
                        break;
                    }

                    pRes = longOptionFind( pOpts, pOpts->pzCurOpt, &optFlags );
                    isLongOpt = AG_TRUE;
                    break;
                }
                /* FALLTHROUGH */ /* option marker was "-+" or "+-" */

            case ':':
            case NUL:
                fprintf( stderr, zIllOptChr, pOpts->pzProgPath,
                         zIllegal, secondChar ? secondChar : ' ' );
                break;  /* ERROR COMPLETION */

            default:
                /*
                 *  The character is a legal flag character.
                 *  If short options are not allowed, then do long
                 *  option processing.  Otherwise the character must be a
                 *  short (i.e. single character) option.
                 */
                if ((pOpts->fOptSet & OPTPROC_SHORTOPT) == 0) {
                    isLongOpt = AG_TRUE;
                    pRes = longOptionFind( pOpts, pOpts->pzCurOpt, &optFlags );

                } else {
                    pRes = shortOptionFind( pOpts, *pOpts->pzCurOpt );

                    /*
                     *  Whenever we want to save the "Disablement Opt" state,
                     *  we will pass through here.  It happens when:
                     *  1) a new option flag is detected
                     *  2) it is a short flag, and
                     *  3) we are not in long-option-only mode.
                     */
                    if ((optFlags & OPTST_DISABLED) != 0)
                         pOpts->fOptSet |=  OPTPROC_DISABLEDOPT;
                    else pOpts->fOptSet &= ~OPTPROC_DISABLEDOPT;
                }

                break;
            }
        }
    }

    /*
     *  IF we could not find a descriptor,
     *  THEN bail out.
     */
    if (pRes == (tOptDesc*)NULL) {
    errorBail:
        pOpts->curOptIdx = argCt;
        pOpts->pzCurOpt  = (char*)NULL;
        return NO_OPT_DESC;  /* ERROR COMPLETION */
    }

    /*
     *  Figure out what to do about option  arguments.
     *  An argument may be required, not associated with the option,
     *  or be optional.  We detect the latter by examining for an option
     *  marker ('+' or '-') on the next possible argument.
     */
    switch (pRes->optArgType) {
    case ARG_MUST:
        /*
         *  An option argument is required.
         */
        if (isLongOpt) {
            char* pz = strchr( pOpts->pzCurOpt, '=' );

            /*
             *  IF the argument is attached to the long name,
             *  THEN set the pointer
             *  ELSE get the next argumet because we consumed the
             *       entire argument with the option name
             */
            if (pz != (char*)NULL) {
                pRes->pzLastArg = pz+1;
            } else {
                pRes->pzLastArg = argVect[ pOpts->curOptIdx++ ];
            }
        } else {
            if (*++pOpts->pzCurOpt == NUL)
                pOpts->pzCurOpt = argVect[ pOpts->curOptIdx++ ];
            pRes->pzLastArg = pOpts->pzCurOpt;
        }

        if (pOpts->curOptIdx > argCt) {
            fprintf( stderr, zMisArg, pOpts->pzProgPath, pRes->pz_Name );
            pRes = NO_OPT_DESC;
        }

        pOpts->pzCurOpt = (char*)NULL;  /* next time advance to next arg */
        break;

    case ARG_MAY:
        /*
         *  An option argument is optional.
         */
        if (isLongOpt) {
            char* pz = strchr( pOpts->pzCurOpt, '=' );

            /*
             *  IF the argument is attached to the long name,
             *  THEN set the pointer
             *  ELSE get the next argumet because we consumed the
             *       entire argument with the option name
             */
            if (pz != (char*)NULL) {
                pRes->pzLastArg = pz+1;
            } else {
                char* pzLA = pRes->pzLastArg = argVect[ pOpts->curOptIdx ];

                /*
                 *  BECAUSE it is optional, we must make sure
                 *  we did not find another flag and that there
                 *  is such an argument.
                 */
                if ( pzLA != (char*)NULL) {
                    if ((*pzLA == '-') || (*pzLA == '+'))
                         pRes->pzLastArg = (char*)NULL;
                    else pOpts->curOptIdx++; /* argument found */
                }
            }

        } else {
            if (*++pOpts->pzCurOpt != NUL)
                pRes->pzLastArg = pOpts->pzCurOpt;
            else {
                char* pzLA = pRes->pzLastArg = argVect[ pOpts->curOptIdx ];

                /*
                 *  BECAUSE it is optional, we must make sure
                 *  we did not find another flag and that there
                 *  is such an argument.
                 */
                if ( pzLA != (char*)NULL) {
                    if ((*pzLA == '-') || (*pzLA == '+'))
                         pRes->pzLastArg = (char*)NULL;
                    else pOpts->curOptIdx++; /* argument found */
                }
            }
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
         *  the correct flag (next argument for long options,
         *  maybe the next character for short flags).
         */
        if (! isLongOpt) {
            (pOpts->pzCurOpt)++;
        } else {
            if (strchr( pOpts->pzCurOpt, '=' ) != (char*)NULL) {
                fprintf( stderr, zNoArg, pOpts->pzProgPath, pRes->pz_Name );
                goto errorBail;
            }
            pOpts->pzCurOpt = (char*)NULL;
        }
    }

    pRes->fOptState &= OPTST_PERSISTENT;
    pRes->fOptState |= optFlags;

    if ((pRes->fOptState & OPTST_NUMERIC) != 0) {
        if (pRes->pzLastArg == (char*)NULL) {
            pOpts->curOptIdx = argCt;
            pOpts->pzCurOpt  = (char*)NULL;
            return NO_OPT_DESC;  /* ERROR COMPLETION */
        }
        pRes->pzLastArg = (char*)strtol( pRes->pzLastArg, (char**)NULL, 0 );
    }

    return pRes;  /* SUCCESSFUL RETURN */
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Define the option processing routine
 */
    int
optionProcess( tOptions*  pOpts, int argCt, char** argVect )
{
    tOptDesc* pOD;
    int       errCt = 0;

    /*
     *  Establish the real program name, the program full path,
     *  and do all the presetting the first time thru only.
     */
    if ((pOpts->fOptSet & OPTPROC_INITDONE) == 0) {
        const char* pz = strrchr( *argVect, DIR_SEP_CHAR );

        if (pz == (char*)NULL)
             pOpts->pzProgName = *argVect;
        else pOpts->pzProgName = pz+1;

        pOpts->pzProgPath = *argVect;
        doPresets( pOpts );

        pOpts->fOptSet |= OPTPROC_INITDONE;
    }

    /*
     *  Now, process all the options from our current position onward.
     *  (This allows interspersed options and arguments for the few
     *  non-standard programs that require it.)
     */
    while (pOD = optionGet( pOpts, argCt, argVect ),
           pOD != (tOptDesc*)NULL )  {

        if (pOD == NO_OPT_DESC) {
            errCt++;
            break;
        }

        if (! loadValue( pOpts, pOD ))
            errCt++;
    }

    /*
     *  IF    there were no errors
     *    AND we have RC/INI files
     *    AND there is a request to save the files
     *  THEN do that now before testing for conflicts.
     *       (conflicts are ignored in preset options)
     */
    if (  (errCt == 0)
       && (pOpts->specOptIdx.save_opts != 0) )  {
        tOptDesc*  pOD = pOpts->pOptDesc + pOpts->specOptIdx.save_opts;

        if (SELECTED_OPT( pOD )) {
            optionSave( pOpts );

            /*
             *  Unless this option was marked with a '+', we bail now.
             */
            if (! DISABLED_OPT( pOD ))
                exit( EXIT_SUCCESS );
    }   }

    /*
     *  IF we are checking for errors,
     *  THEN look for too few occurrances of required options
     */
    if ((pOpts->fOptSet & OPTPROC_ERRSTOP) != 0) {
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
             *    AND it does not occur often enough
             *  THEN note the error
             */
            if (  (  (pOD->optEquivIndex == NO_EQUIVALENT)
                  || (pOD->optEquivIndex == pOD->optIndex) )
               && (pOD->optOccCt <  pOD->optMinCt)  )  {
                tSCC zNotEnough[] = "ERROR:  The %s option must appear %d times\n";
                tSCC zNeedOne[]   = "ERROR:  The %s option is required\n";

                errCt++;
                if (pOD->optMinCt > 1)
                     fprintf( stderr, zNotEnough, pOD->pz_Name, pOD->optMinCt );
                else fprintf( stderr, zNeedOne, pOD->pz_Name );
            }

            if (--oCt <= 0)
                break;
            pOD++;
        }
    }

    if (errCt != 0)
        (*pOpts->pUsageProc)( pOpts, EXIT_FAILURE );

    return pOpts->curOptIdx;
}
