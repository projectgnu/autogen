
/*
 *  usage.c  $Id: usage.c,v 2.13 2000/10/20 02:46:37 bkorb Exp $
 *
 *  This module implements the default usage procedure for
 *  Automated Options.  It may be overridden, of course.
 */

/*
 *  Automated Options copyright 1992-1999 Bruce Korb
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

#include <compat/compat.h>

#include "autoopts.h"

#ifndef HAVE_PATHFIND
/*
 *  The pathfind() routine either does not work or is not SVR4
 */
#  include <compat/pathfind.c>
#endif

tSCC zOptsOnly[]   = "All arguments are named options.\n";
tSCC zInverted[]   = "Flag options are disabled with a '+' marker.\n";
tSCC zNumberOpt[]  = "The '-#<number>' option may omit the hash char\n";
tSCC zHomePath[]   = " - reading file /... %s's exe directory .../%s \n";
tSCC zExamineFmt[] = " - examining environment variables named %s_*\n";
tSCC zMust[]       = "\t\t\t\t- must appear between %d and %d times\n";
tSCC zNoLim[]      = "\t\t\t\t- may appear multiple times\n";
tSCC zPreset[]     = "\t\t\t\t- may NOT appear - preset only\n";
tSCC zUpTo[]       = "\t\t\t\t- may appear up to %d times\n";
tSCC zNoPreset[]   = "\t\t\t\t- may not be preset\n";
tSCC zAlt[]        = "\t\t\t\t- an alternate for %s\n";
tSCC zEnab[]       = "\t\t\t\t- enabled by default\n";
tSCC zDis[]        = "\t\t\t\t- disabled as --%s\n";
tSCC zTabout[]     = "\t\t\t\t%s\n";
tSCC zTabHyp[]     = "\t\t\t\t- ";
tSCC zTabHypAnd[]  = "\t\t\t\t-- and ";
tSCC zDefaultOpt[] = "\t\t\t\t- default option for unnamed options\n";
tSCC zReqThese[]   = "requires these options:\n";
tSCC zProhib[]     = "prohibits these options:\n";

tSCC zNoReq_Short_Title[]   = "  Flg Arg Option-Name    Description\n";
tSCC zNoReq_NoShort_Title[] = "   Arg Option-Name    Description\n";
tSCC zNrmOptFmt[]           = " %1$3s %2$-14s %4$s\n";

tSCC zReq_Short_Title[]     = "  Flg Arg Option-Name   Req?  Description\n";
tSCC zReq_NoShort_Title[]   = "   Arg Option-Name   Req?  Description\n";
tSCC zReqOptFmt[]           = " %3s %-14s %-5s%s\n";

tSCC zIntro[]      = "\n\
The following option preset mechanisms are supported:\n";

tSCC zFlagOkay[]   =
"Options may be specified by doubled %1$ss and their name\n\
or by a single %1$s and the flag character/option value.\n";

tSCC zNoFlags[]    =
"Options are specified by their name and either single\n\
or doubled %ss.  Flag characters are not interpreted.\n";


DEF_PROC_2( , void, optionUsage,
            tOptions*,  pOptions,
            int,        exitCode )
{
    tCC*    pOptFmt;
    tCC*    pOptTitle;
    FILE*   fp = stderr;
    ag_bool displayEnum = AG_FALSE;

    fprintf( fp, pOptions->pzUsageTitle, pOptions->pzProgName );

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
     *  IF we are NOT going to start the option list with a documentation opt,
     *  THEN emit the title here.
     *  Doc options are emitted only if the exitCode is SUCCESS.
     */
    if (  (exitCode != EXIT_SUCCESS)
       || ((pOptions->pOptDesc->fOptState & OPTST_DOCUMENT) != 0) )

        fputs( pOptTitle, fp );

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

            if ((pOD->fOptState & OPTST_DOCUMENT) != 0) {
                if (exitCode == EXIT_SUCCESS) {
                    fprintf( fp, zBreak, pOD->pzText, pOptTitle );
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
                fprintf( fp, zBreak, zAuto, pOptTitle );

            /*
             *  Flag prefix:  IF no flags at all, then omit it.
             *  If not printable (not allowed for this option),
             *  then blank, else print it.
             */
            if ((pOptions->fOptSet & OPTPROC_SHORTOPT) == 0)
                 fputs( "  ",   fp );
            else if (! isgraph( pOD->optValue))
                 fputs( "     ", fp );
            else fprintf( fp, "   -%c", pOD->optValue );

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
            fprintf( fp, pOptFmt, pzArgType, pOD->pz_Name,
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
            if (  (pOD->pOptMust != (int*)NULL)
               || (pOD->pOptCant != (int*)NULL) ) {

                fputs( zTabHyp, fp );

                /*
                 *  DEPENDENCIES:
                 */
                if (pOD->pOptMust != (int*)NULL) {
                    const int* pOptNo = pOD->pOptMust;

                    fputs( zReqThese, fp );
                    for (;;) {
                        fprintf( fp, zTabout, pOptions->pOptDesc[
                                 *pOptNo ].pz_Name );
                        if (*++pOptNo == NO_EQUIVALENT)
                            break;
                    }

                    if (pOD->pOptCant != (int*)NULL)
                        fputs( zTabHypAnd, fp );
                }

                /*
                 *  CONFLICTS:
                 */
                if (pOD->pOptCant != (int*)NULL) {
                    const int* pOptNo = pOD->pOptCant;

                    fputs( zProhib, fp );
                    for (;;) {
                        fprintf( fp, zTabout, pOptions->pOptDesc[
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
            if (pOD->pz_DisableName != (char*)NULL )
                fprintf( fp, zDis, pOD->pz_DisableName );

            /*
             *  IF the option defaults to being enabled,
             *  THEN print that out
             */
            if (pOD->fOptState & OPTST_INITENABLED)
                fputs( zEnab, fp );

            /*
             *  IF  the option is in an equivalence class
             *        AND not the designated lead
             *  THEN print equivalence and continue
             */
            if (  (pOD->optEquivIndex != NO_EQUIVALENT)
               && (pOD->optEquivIndex != optNo )  )  {
                fprintf( fp, zAlt,
                         pOptions->pOptDesc[ pOD->optEquivIndex ].pz_Name );
                continue;
            }

            /*
             *  IF this particular option can NOT be preset
             *    AND some form of presetting IS allowed,
             *  THEN advise that this option may not be preset.
             */
            if (  ((pOD->fOptState & OPTST_NO_INIT) != 0)
               && (  (pOptions->papzHomeList != (const char**)NULL)
                  || (pOptions->pzPROGNAME != (const char*)NULL)
               )  )

                fputs( zNoPreset, fp );

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
                     fputs( zPreset, fp );
                     break;

                case 1:
                     break;

                case NOLIMIT:
                     fputs( zNoLim, fp );
                     break;

                default:
                     fprintf( fp, zUpTo, pOD->optMaxCt );
                     break;
                }
                break;

            default:
                /*
                 *  More than one is required.  Print the range.
                 */
                fprintf( fp, zMust, pOD->optMinCt, pOD->optMaxCt );
            }

            if (  NAMED_OPTS( pOptions )
               && (pOptions->specOptIdx.default_opt == pOD->optIndex))
                fputs( zDefaultOpt, fp );
        }  while (pOD++, optNo++, (--ct > 0));
    }

    fputc( '\n', fp );

    /*
     *  Describe the mechanics of denoting the options
     */
    {
        u_int  fOptSet = pOptions->fOptSet;
        tCC*   pzFmt   =  (fOptSet & OPTPROC_SHORTOPT) ? zFlagOkay : zNoFlags;

        if ((fOptSet & OPTPROC_PLUSMARKS) != 0)
            fputs( zInverted, fp );

        if ((fOptSet & OPTPROC_LONGOPT) != 0)
            fprintf( fp, pzFmt,
                     (fOptSet & OPTPROC_DISABLEOK) ? "marker" : "hyphen" );

        else if ((fOptSet & OPTPROC_SHORTOPT) == 0)
            fputs( zOptsOnly, fp );

        if ((fOptSet & OPTPROC_NUM_OPT) != 0)
            fputs( zNumberOpt, fp );
    }

    if (pOptions->pzExplain != (char*)NULL)
        fputs( pOptions->pzExplain, fp );

    /*
     *  IF the user is asking for help (thus exiting with SUCCESS),
     *  THEN see what additional information we can provide.
     */
    if (exitCode == EXIT_SUCCESS) {
        ag_bool  initIntro = AG_TRUE;
        u_int    fOptSet   = pOptions->fOptSet;
        tSCC     zPathFmt[] = " - reading file %s/%s\n";

        /*
         *  Display all the places we look for RC files
         */
        if (pOptions->papzHomeList != (const char**)NULL) {
            const char** papzHL = pOptions->papzHomeList;
            for (;;) {
                const char* pzPath = *(papzHL++);

                if (pzPath == (char*)NULL)
                    break;

                if (initIntro) {
                    fputs( zIntro, fp );
                    initIntro = AG_FALSE;
                }

                fprintf( fp, zPathFmt, pzPath, pOptions->pzRcName );
            }
        }

        /*
         *  Let the user know about environment variable settings
         */
        if ((pOptions->fOptSet & OPTPROC_ENVIRON) != 0) {
            if (initIntro)
                fputs( zIntro, fp );

            fprintf( fp, zExamineFmt, pOptions->pzPROGNAME );
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
            int        docCt  = 0;

            fputc( '\n', fp );
            fflush( fp );
            do  {
                if ((pOD->fOptState & OPTST_ENUMERATION) != 0)
                    (*(pOD->pOptProc))( NULL, pOD );
            }  while (pOD++, optNo++, (--ct > 0));
        }

        /*
         *  If there is a detail string, now is the time for that.
         */
        if (pOptions->pzDetail != (char*)NULL)
            fputs( pOptions->pzDetail, fp );

        /*
         *  IF      there is a detail file
         *     AND  we are using the pager
         *  THEN uncompress the detail file and dump to fp
         */
        if (  (pOptions->pzDetailFile != (char*)NULL)
           && (! UNUSED_OPT( pOptions->pOptDesc
                           + pOptions->specOptIdx.more_help ))) {
            char* pzPath = getenv( "PATH" );
            char* pzHelpPath;

            char  zCmd[ MAXPATHLEN + 64 ];
            char  zDetFile[ MAXPATHLEN ];

            do {
                char*  pz = zDetFile
                    + snprintf( zDetFile, sizeof(zDetFile), "%s.README",
                                pOptions->pzDetailFile );
                pzHelpPath = pathfind( pzPath, zDetFile, "r" );

                if (pzHelpPath != (char*)NULL) {
                    tSCC zCmdFmt[] = "cat %s >&2";
                    snprintf( zCmd, sizeof(zCmd), zCmdFmt, pzHelpPath );
                    break;
                }

                strcpy( pz, ".Z" );
                pzHelpPath = pathfind( pzPath, zDetFile, "r" );

                if (pzHelpPath != (char*)NULL) {
                    tSCC zCmdFmt[] = "uncompress -c < %s >&2";
                    snprintf( zCmd, sizeof(zCmd), zCmdFmt, pzHelpPath );
                    break;
                }

                strcpy( pz, ".gz" );
                pzHelpPath = pathfind( pzPath, zDetFile, "r" );

                if (pzHelpPath != (char*)NULL) {
                    tSCC zCmdFmt[] = "gunzip -c %s >&2";
                    snprintf( zCmd, sizeof(zCmd), zCmdFmt, pzHelpPath );
                    break;
                }

                strcpy( pz, ".bz2" );
                pzHelpPath = pathfind( pzPath, zDetFile, "r" );

                if (pzHelpPath != (char*)NULL) {
                    tSCC zCmdFmt[] = "bunzip2 -c %s >&2";
                    snprintf( zCmd, sizeof(zCmd), zCmdFmt, pzHelpPath );
                    break;
                }

            } while (0);

            /*
             *  IF we can actually find the detail file,
             *  THEN really do it.
             */
            if (pzHelpPath != (char*)NULL) {
                fflush( fp );
                system( zCmd );
            }
        }
    }

    exit( exitCode );
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * usage.c ends here */
