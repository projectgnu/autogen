
/*
 *  usage.c  $Id: usage.c,v 3.10 2003/02/16 00:04:40 bkorb Exp $
 *
 *  This module implements the default usage procedure for
 *  Automated Options.  It may be overridden, of course.
 */

/*
 *  Automated Options copyright 1992-2003 Bruce Korb
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

#include "autoopts.h"

tSCC zAlt[]        = "\t\t\t\t- an alternate for %s\n";
tSCC zDefaultOpt[] = "\t\t\t\t- default option for unnamed options\n";
tSCC zDis[]        = "\t\t\t\t- disabled as --%s\n";
tSCC zEnab[]       = "\t\t\t\t- enabled by default\n";
tSCC zExamineFmt[] = " - examining environment variables named %s_*\n";
tSCC zHomePath[]   = " - reading file /... %s's exe directory .../%s \n";
tSCC zMust[]       = "\t\t\t\t- must appear between %d and %d times\n";
tSCC zNoLim[]      = "\t\t\t\t- may appear multiple times\n";
tSCC zNoPreset[]   = "\t\t\t\t- may not be preset\n";
tSCC zNumberOpt[]  = "The '-#<number>' option may omit the hash char\n";
tSCC zOptsOnly[]   = "All arguments are named options.\n";
tSCC zPreset[]     = "\t\t\t\t- may NOT appear - preset only\n";
tSCC zProhib[]     = "prohibits these options:\n";
tSCC zReqThese[]   = "requires these options:\n";
tSCC zTabHypAnd[]  = "\t\t\t\t-- and ";
tSCC zTabHyp[]     = "\t\t\t\t- ";
tSCC zTabout[]     = "\t\t\t\t%s\n";
tSCC zUpTo[]       = "\t\t\t\t- may appear up to %d times\n";

tSCC zNoReq_Short_Title[]   = "  Flg Arg Option-Name    Description\n";
tSCC zNoReq_NoShort_Title[] = "   Arg Option-Name    Description\n";
tSCC zNrmOptFmt[]           = " %1$3s %2$-14s %4$s\n";

tSCC zReq_Short_Title[]     = "  Flg Arg Option-Name   Req?  Description\n";
tSCC zReq_NoShort_Title[]   = "   Arg Option-Name   Req?  Description\n";
tSCC zReqOptFmt[]           = " %3s %-14s %-5s%s\n";

tSCC zIntro[]      = "\n\
The following option preset mechanisms are supported:\n";

tSCC zFlagOkay[]   =
"Options may be specified by doubled hyphens and their name\n\
or by a single hyphen and the flag character (option value).\n";

tSCC zNoFlags[]    =
"Options are specified by their name and either single\n\
or doubled %ss.  Flag characters are not interpreted.\n";

FILE* option_usage_fp = NULL;

static void printInitList PROTO(( tCC** papz, ag_bool*, tCC*, tCC* ));

void
optionUsage( pOptions, exitCode )
    tOptions* pOptions;
    int       exitCode;
{
    tCC*    pOptFmt;
    tCC*    pOptTitle;
    ag_bool displayEnum = AG_FALSE;

    /*
     *  Paged usage will preset option_usage_fp to an output file.
     *  If it hasn't already been set, then set it to standard output
     *  on successful exit (help was requested), otherwise error out.
     */
    if (option_usage_fp == NULL)
        option_usage_fp = (exitCode != EXIT_SUCCESS) ? stderr : stdout;

    fprintf( option_usage_fp, pOptions->pzUsageTitle, pOptions->pzProgName );

    /*
     *  Determine which header and which option formatting string to use
     */
    switch (pOptions->fOptSet & (OPTPROC_NO_REQ_OPT | OPTPROC_SHORTOPT)) {
    case (OPTPROC_NO_REQ_OPT | OPTPROC_SHORTOPT):
        pOptTitle = zNoReq_Short_Title;
        pOptFmt   = zNrmOptFmt;
        break;

    case OPTPROC_NO_REQ_OPT:
        pOptTitle = zNoReq_NoShort_Title;
        pOptFmt   = zNrmOptFmt;
        break;

    case OPTPROC_SHORTOPT:
        pOptTitle = zReq_Short_Title;
        pOptFmt   = zReqOptFmt;
        break;

    default:
    case 0:
        pOptTitle = zReq_NoShort_Title;
        pOptFmt   = zReqOptFmt;
    }

    /*
     *  When we exit with EXIT_SUCCESS and the first option is a doc option,
     *  we do *NOT* want to emit the column headers.  Otherwise, we do.
     */
    if (  (exitCode != EXIT_SUCCESS)
       || ((pOptions->pOptDesc->fOptState & OPTST_DOCUMENT) == 0) )

        fputs( pOptTitle, option_usage_fp );

    {
        int        ct     = pOptions->optCt;
        int        optNo  = 0;
        tOptDesc*  pOD    = pOptions->pOptDesc;
        int        docCt  = 0;

        do  {
            tSCC  zReqArg[]  = "YES";
            tSCC  zNumArg[]  = "Num";
            tSCC  zKeyArg[]  = "KWd";
            tSCC  zBoolArg[] = "T/F";
            tSCC  zOptArg[]  = "opt";
            tSCC  zNoArg[]   = "no ";
            tSCC  zBreak[]   = "\n%s\n\n%s";
            tSCC  zAuto[]    = "Auto-supported Options:";

            tCC*  pzArgType;

            if ((pOD->fOptState & OPTST_OMITTED) != 0)
                continue;

            if ((pOD->fOptState & OPTST_DOCUMENT) != 0) {
                if (exitCode == EXIT_SUCCESS) {
                    fprintf( option_usage_fp, zBreak, pOD->pzText, pOptTitle );
                    docCt++;
                }

                continue;
            }

            /*
             *  IF       this is the first auto-opt maintained option
             *    *AND*  we are doing a full help
             *    *AND*  there are documentation options
             *    *AND*  the last one was not a doc option,
             *  THEN document that the remaining options are not user opts
             */
            if (  (pOptions->presetOptCt == optNo)
               && (exitCode == EXIT_SUCCESS)
               && (docCt > 0)
               && ((pOD[-1].fOptState & OPTST_DOCUMENT) == 0) )
                fprintf( option_usage_fp, zBreak, zAuto, pOptTitle );

            /*
             *  Flag prefix:  IF no flags at all, then omit it.
             *  If not printable (not allowed for this option),
             *  then blank, else print it.
             */
            if ((pOptions->fOptSet & OPTPROC_SHORTOPT) == 0)
                 fputs( "  ",   option_usage_fp );
            else if (! isgraph( pOD->optValue))
                 fputs( "     ", option_usage_fp );
            else fprintf( option_usage_fp, "   -%c", pOD->optValue );

            /*
             *  convert arg type code into a string
             */
            if ((pOD->fOptState & OPTST_NUMERIC) != 0)
                pzArgType = zNumArg;

            else if ((pOD->fOptState & OPTST_BOOLEAN) != 0)
                pzArgType = zBoolArg;

            else if ((pOD->fOptState & OPTST_ENUMERATION) != 0) {
                displayEnum |= (pOD->pOptProc != NULL) ? AG_TRUE : AG_FALSE;
                pzArgType = zKeyArg;
            }

            else switch (pOD->optArgType) {
            case ':': pzArgType = zReqArg; break;
            case '?': pzArgType = zOptArg; break;
            default:
            case ' ': pzArgType = zNoArg;  break;
            }

            /*
             *  Print the option description
             */
            fprintf( option_usage_fp, pOptFmt, pzArgType, pOD->pz_Name,
                     (pOD->optMinCt != 0) ? zReqArg : zOptArg, pOD->pzText );

            /*
             *  IF we were not invoked because of the --help option,
             *  THEN keep this output short...
             */
            if (exitCode != EXIT_SUCCESS)
                continue;

            /*
             *  IF there are option conflicts or dependencies,
             *  THEN print them here.
             */
            if (  (pOD->pOptMust != NULL)
               || (pOD->pOptCant != NULL) ) {

                fputs( zTabHyp, option_usage_fp );

                /*
                 *  DEPENDENCIES:
                 */
                if (pOD->pOptMust != NULL) {
                    const int* pOptNo = pOD->pOptMust;

                    fputs( zReqThese, option_usage_fp );
                    for (;;) {
                        fprintf( option_usage_fp, zTabout, pOptions->pOptDesc[
                                 *pOptNo ].pz_Name );
                        if (*++pOptNo == NO_EQUIVALENT)
                            break;
                    }

                    if (pOD->pOptCant != NULL)
                        fputs( zTabHypAnd, option_usage_fp );
                }

                /*
                 *  CONFLICTS:
                 */
                if (pOD->pOptCant != NULL) {
                    const int* pOptNo = pOD->pOptCant;

                    fputs( zProhib, option_usage_fp );
                    for (;;) {
                        fprintf( option_usage_fp, zTabout, pOptions->pOptDesc[
                                 *pOptNo ].pz_Name );
                        if (*++pOptNo == NO_EQUIVALENT)
                            break;
                    }
                }
            }

            /*
             *  IF there is a disablement string
             *  THEN print the disablement info
             */
            if (pOD->pz_DisableName != NULL )
                fprintf( option_usage_fp, zDis, pOD->pz_DisableName );

            /*
             *  IF the numeric option has a special callback,
             *  THEN call it, requesting the range or other special info
             */
            if (  (pOD->fOptState & OPTST_NUMERIC)
                  && (pOD->pOptProc != NULL)
                  && (pOD->pOptProc != optionNumericVal) ) {
                (*(pOD->pOptProc))( pOptions, NULL );
            }

            /*
             *  IF the option defaults to being enabled,
             *  THEN print that out
             */
            if (pOD->fOptState & OPTST_INITENABLED)
                fputs( zEnab, option_usage_fp );

            /*
             *  IF  the option is in an equivalence class
             *        AND not the designated lead
             *  THEN print equivalence and continue
             */
            if (  (pOD->optEquivIndex != NO_EQUIVALENT)
               && (pOD->optEquivIndex != optNo )  )  {
                fprintf( option_usage_fp, zAlt,
                         pOptions->pOptDesc[ pOD->optEquivIndex ].pz_Name );
                continue;
            }

            /*
             *  IF this particular option can NOT be preset
             *    AND some form of presetting IS allowed,
             *  THEN advise that this option may not be preset.
             */
            if (  ((pOD->fOptState & OPTST_NO_INIT) != 0)
               && (  (pOptions->papzHomeList != NULL)
                  || (pOptions->pzPROGNAME != NULL)
               )  )

                fputs( zNoPreset, option_usage_fp );

            /*
             *  Print the appearance requirements.
             */
            switch (pOD->optMinCt) {
            case 1:
            case 0:
                /*
                 *  IF the max is more than one, print an "UP TO" message
                 */
                switch (pOD->optMaxCt) {
                case 0:
                     fputs( zPreset, option_usage_fp );
                     break;

                case 1:
                     break;

                case NOLIMIT:
                     fputs( zNoLim, option_usage_fp );
                     break;

                default:
                     fprintf( option_usage_fp, zUpTo, pOD->optMaxCt );
                     break;
                }
                break;

            default:
                /*
                 *  More than one is required.  Print the range.
                 */
                fprintf( option_usage_fp, zMust, pOD->optMinCt, pOD->optMaxCt );
            }

            if (  NAMED_OPTS( pOptions )
               && (pOptions->specOptIdx.default_opt == pOD->optIndex))
                fputs( zDefaultOpt, option_usage_fp );
        }  while (pOD++, optNo++, (--ct > 0));
    }

    fputc( '\n', option_usage_fp );

    /*
     *  Describe the mechanics of denoting the options
     */
    {
        tCC*   pzFmt   =  (pOptions->fOptSet & OPTPROC_SHORTOPT)
                          ? zFlagOkay : zNoFlags;

        if ((pOptions->fOptSet & OPTPROC_LONGOPT) != 0)
            fputs( pzFmt, option_usage_fp );

        else if ((pOptions->fOptSet & OPTPROC_SHORTOPT) == 0)
            fputs( zOptsOnly, option_usage_fp );

        if ((pOptions->fOptSet & OPTPROC_NUM_OPT) != 0)
            fputs( zNumberOpt, option_usage_fp );
    }

    if (pOptions->pzExplain != NULL)
        fputs( pOptions->pzExplain, option_usage_fp );

    /*
     *  IF the user is asking for help (thus exiting with SUCCESS),
     *  THEN see what additional information we can provide.
     */
    if (exitCode == EXIT_SUCCESS) {
        ag_bool  initIntro = AG_TRUE;

        /*
         *  Display all the places we look for RC files
         */
        printInitList( pOptions->papzHomeList, &initIntro,
                       pOptions->pzRcName, pOptions->pzProgPath );

        /*
         *  Let the user know about environment variable settings
         */
        if ((pOptions->fOptSet & OPTPROC_ENVIRON) != 0) {
            if (initIntro)
                fputs( zIntro, option_usage_fp );

            fprintf( option_usage_fp, zExamineFmt, pOptions->pzPROGNAME );
        }

        /*
         *  IF we found an enumeration,
         *  THEN hunt for it again.  Call the handler proc with a NULL
         *       option struct pointer.  That tells it to display the keywords.
         */
        if (displayEnum) {
            int        ct     = pOptions->optCt;
            int        optNo  = 0;
            tOptDesc*  pOD    = pOptions->pOptDesc;

            fputc( '\n', option_usage_fp );
            fflush( option_usage_fp );
            do  {
                if ((pOD->fOptState & OPTST_ENUMERATION) != 0)
                    (*(pOD->pOptProc))( NULL, pOD );
            }  while (pOD++, optNo++, (--ct > 0));
        }

        /*
         *  If there is a detail string, now is the time for that.
         */
        if (pOptions->pzDetail != NULL)
            fputs( pOptions->pzDetail, option_usage_fp );
    }

    if (pOptions->pzBugAddr != NULL)
        fprintf( option_usage_fp, "\nplease send bug reports to:  %s\n",
                 pOptions->pzBugAddr );

    exit( exitCode );
}

static void
printInitList( papz, pInitIntro, pzRc, pzPN )
    tCC**    papz;
    ag_bool* pInitIntro;
    tCC*     pzRc;
    tCC*     pzPN;
{
    tSCC zPathFmt[] = " - reading file %s";
    char zPath[ MAXPATHLEN+1 ];

    if (papz == NULL)
        return;

    fputs( zIntro, option_usage_fp );
    *pInitIntro = AG_FALSE;

    for (;;) {
        const char* pzPath = *(papz++);

        if (pzPath == NULL)
            break;

        if (optionMakePath( zPath, sizeof( zPath ), pzPath, pzPN ))
            pzPath = zPath;

        /*
         *  Print the name of the "homerc" file.  If the "rcfile" name is
         *  not empty, we may or may not print that, too...
         */
        fprintf( option_usage_fp, zPathFmt, pzPath );
        if (*pzRc != NUL) {
            struct stat sb;

            /*
             *  IF the "homerc" file is a directory,
             *  then append the "rcfile" name.
             */
            if (  (stat( pzPath, &sb ) == 0)
              &&  S_ISDIR( sb.st_mode ) ) {
                fputc( '/', option_usage_fp );
                fputs( pzRc, option_usage_fp );
            }
        }

        fputc( '\n', option_usage_fp );
    }
}

/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * tab-width: 4
 * End:
 * usage.c ends here */
