
/*
 *  $Id: makeshell.c,v 2.12 1998/10/30 16:49:12 bkorb Exp $
 *
 *  This module will interpret the options set in the tOptions
 *  structure and create a Bourne shell script capable of parsing them.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>

#include "autoopts.h"
#include "genshell.c"

tOptions*  pShellParseOptions = (tOptions*)NULL;

/* * * * * * * * * * * * * * * * * * * * *
 *
 *  Setup Format Strings
 */

static const char zPreamble[] =
"#!%s\n#\n"
"#  DO NOT EDIT THIS SECTION OF %s\n#\n"
"#  From here to the `-- do not delete this marker --',\n"
"#  the text has been generated %s\n";

static const char zLongUsageText[] =
"#  From the %s option definitions\n"
"#\n"
"#  Instead, copy it into the shell script you are maintaining\n"
"#\n"
"%s_LONGUSAGE_TEXT=`cat <<'__LONGUSAGE__EOF__'\n";

static const char zShortUsageText[] =
"__LONGUSAGE__EOF__\n`\n\n"
"%s_USAGE_TEXT=`cat <<'__USAGE__EOF__'\n";

static const char zEndUsageText[] =
"__USAGE__EOF__\n`\n\n";

static const char zVersText[] =
"%s_VERSION_TEXT=`cat <<'__VERSION_EOF__'\n";

static const char zEndVersionText[] =
"__VERSION_EOF__\n`\n\n";

static const char zMultiDef[] = "\n"
"%1$s_%2$s_CT=0\n"
"export %1$s_%2$s_CT\n";

static const char zSingleDef[] = "\n"
"unset %1$s_%2$s\n"
"%1$s_%2$s=''\n"
"export %1$s_%2$s\n";

/* * * * * * * * * * * * * * * * * * * * *
 *
 *  LOOP START
 *
 *  The loop may run in either of two modes:
 *  all options are named options (loop only)
 *  regular, marked option processing.
 */
static const char zLoopCase[] = "\n"
"OPT_PROCESS=true\n"
"OPT_ARG=\"$1\"\n\n"
"while ${OPT_PROCESS} && [ $# -gt 0 ]\ndo\n"
"    OPT_ELEMENT=''\n"
"    OPT_ARG_VAL=''\n\n"
     /*
      *  'OPT_ARG' may or may not match the current $1
      */
"    case \"${OPT_ARG}\" in\n"
"    -- )\n"
"        OPT_PROCESS=false\n"
"        shift\n"
"        ;;\n\n";

static const char zLoopOnly[] = "\n"
"OPT_ARG=\"$1\"\n\n"
"while [ $# -gt 0 ]\ndo\n"
"    OPT_ELEMENT=''\n"
"    OPT_ARG_VAL=''\n\n"
"    OPT_ARG=\"${1}\"\n";

/* * * * * * * * * * * * * * * *
 *
 *  CASE SELECTORS
 *
 *  If the loop runs as a regular option loop,
 *  then we must have selectors for each acceptable option
 *  type (long option, flag character and non-option)
 */
static const char zLongSelection[] =
"    --* )\n";

static const char zFlagSelection[] =
"    -* )\n";

static const char zEndSelection[] =
"        ;;\n\n";

static const char zNoSelection[] =
"    * )\n"
"         OPT_PROCESS=false\n"
"         ;;\n"
"    esac\n\n";

/* * * * * * * * * * * * * * * *
 *
 *  LOOP END
 */
static const char zLoopEnd[] =
"    if [ -n \"${OPT_ARG_VAL}\" ]\n"
"    then\n"
"        eval %1$s_${OPT_NAME}${OPT_ELEMENT}=\"'${OPT_ARG_VAL}'\"\n"
"        export %1$s_${OPT_NAME}${OPT_ELEMENT}\n"
"    fi\n"
"done\n\n"
"unset OPT_PROCESS\n"
"unset OPT_ELEMENT\n"
"unset OPT_ARG\n"
"unset OPT_NAME\n"
"unset OPT_CODE\n"
"unset OPT_ARG_VAL\n%s";

static const char zTrailerMarker[] = "\n"
"# # # # # # # # # #\n#\n"
"#  END OF AUTOMATED OPTION PROCESSING\n"
"#\n# # # # # # # # # # -- do not delete this marker --\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  OPTION SELECTION
 */
static const char zOptionCase[] =
"        case \"${OPT_CODE}\" in\n";

static const char zOptionPartName[] =
"        %s | \\\n";

static const char zOptionFullName[] =
"        %s )\n";

static const char zOptionFlag[] =
"        '%c' )\n";

static const char zOptionEndSelect[] =
"            ;;\n\n";

static const char zOptionUnknown[] =
"        * )\n"
"            echo Unknown %s: \"${OPT_CODE}\" >&2\n"
"            echo \"$%s_USAGE_TEXT\"\n"
"            exit 1\n"
"            ;;\n"
"        esac\n\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  OPTION PROCESSING
 *
 *  Formats for emitting the text for handling particular options
 */
static const char zTextExit[] =
"            echo \"$%s_%s_TEXT\"\n"
"            exit 0\n";

static const char zCmdFmt[] =
"            %s\n";

static const char zCountTest[] =
"            if [ $%1$s_%2$s_CT -ge %3$d ] ; then\n"
"                echo Error:  more than %3$d %2$s options >&2\n"
"                echo \"$%s_USAGE_TEXT\"\n"
"                exit 1 ; fi\n";

static const char zMultiArg[] =
"            %1$s_%2$s_CT=`expr ${%1$s_%2$s_CT} + 1`\n"
"            OPT_ELEMENT=\"_${%1$s_%2$s_CT}\"\n"
"            OPT_NAME='%2$s'\n";

static const char zSingleArg[] =
"            if [ -n \"${%1$s_%2$s}\" ] ; then\n"
"                echo Error:  duplicate %2$s option >&2\n"
"                echo \"$%s_USAGE_TEXT\"\n"
"                exit 1 ; fi\n"
"            OPT_NAME='%2$s'\n";

static const char zNoMultiArg[] =
"            %1$s_%2$s_CT=0\n"
"            OPT_ELEMENT=''\n"
"            %1$s_%2$s='%3$s'\n"
"            export %1$s_%2$s\n"
"            OPT_NAME='%2$s'\n";

static const char zNoSingleArg[] =
"            if [ -n \"${%1$s_%2$s}\" ] ; then\n"
"                echo Error:  duplicate %2$s option >&2\n"
"                echo \"$%s_USAGE_TEXT\"\n"
"                exit 1 ; fi\n"
"            %1$s_%2$s='%3$s'\n"
"            export %1$s_%2$s\n"
"            OPT_NAME='%2$s'\n";

static const char zMayArg[]  =
"            eval %1$s_%2$s${OPT_ELEMENT}=true\n"
"            export %1$s_%2$s${OPT_ELEMENT}\n"
"            OPT_ARG=OK\n";

static const char zMustArg[] =
"            OPT_ARG=YES\n";

static const char zCantArg[] =
"            eval %1$s_%2$s${OPT_ELEMENT}=true\n"
"            export %1$s_%2$s${OPT_ELEMENT}\n"
"            OPT_ARG=NO\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  LONG OPTION PROCESSING
 *
 *  Formats for emitting the text for handling long option types
 */
static const char zLongOptInit[] =
"        OPT_CODE=`echo \"${OPT_ARG}\"|sed 's/^--//'`\n"
"        shift\n"
"        OPT_ARG=\"$1\"\n\n"
"        case \"${OPT_CODE}\" in *=* )\n"
"            OPT_ARG_VAL=`echo \"${OPT_CODE}\"|sed 's/^[^=]*=//'`\n"
"            OPT_CODE=`echo \"${OPT_CODE}\"|sed 's/=.*$//'` ;; esac\n\n";

static const char zLongOptArg[] =
"        case \"${OPT_ARG}\" in\n"
"        NO )\n"
"            OPT_ARG_VAL=''\n"
"            ;;\n\n"
"        YES )\n"
"            if [ -z \"${OPT_ARG_VAL}\" ]\n"
"            then\n"
"                if [ $# -eq 0 ]\n"
"                then\n"
"                    echo No argument provided for ${OPT_NAME} option >&2\n"
"                    echo \"$%s_USAGE\"\n"
"                    exit 1\n"
"                fi\n\n"
"                OPT_ARG_VAL=\"${OPT_ARG}\"\n"
"                shift\n"
"                OPT_ARG=\"$1\"\n"
"            fi\n"
"            ;;\n\n"
"        OK )\n"
"            if [ -z \"${OPT_ARG_VAL}\" ] && [ $# -gt 0 ]\n"
"            then\n"
"                case \"${OPT_ARG}\" in -* ) ;; * )\n"
"                    OPT_ARG_VAL=\"${OPT_ARG}\"\n"
"                    shift\n"
"                    OPT_ARG=\"$1\" ;; esac\n"
"            fi\n"
"            ;;\n"
"        esac\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  FLAG OPTION PROCESSING
 *
 *  Formats for emitting the text for handling flag option types
 */
static const char zFlagOptInit[] =
"        OPT_CODE=`echo \"${OPT_ARG}\" | sed 's/-\\(.\\).*/\\1/'`\n"
"        OPT_ARG=` echo \"${OPT_ARG}\" | sed 's/-.//'`\n\n";

static const char zFlagOptArg[] =
"        case \"${OPT_ARG}\" in\n"
"        NO )\n"
"            if [ -n \"${OPT_ARG}\" ]\n"
"            then\n"
"                OPT_ARG=-\"${OPT_ARG}\"\n"
"            else\n"
"                shift\n"
"                OPT_ARG=\"$1\"\n"
"            fi\n"
"            ;;\n\n"
"        YES )\n"
"            if [ -n \"${OPT_ARG}\" ]\n"
"            then\n"
"                OPT_ARG_VAL=\"${OPT_ARG}\"\n\n"
"            else\n"
"                if [ $# -eq 0 ]\n"
"                then\n"
"                    echo No argument provided for ${OPT_NAME} option >&2\n"
"                    echo \"$%s_USAGE\"\n"
"                    exit 1\n"
"                fi\n"
"                shift\n"
"                OPT_ARG_VAL=\"$1\"\n"
"            fi\n\n"
"            shift\n"
"            OPT_ARG=\"$1\"\n"
"            ;;\n\n"
"        OK )\n"
"            if [ -n \"${OPT_ARG}\" ]\n"
"            then\n"
"                OPT_ARG_VAL=\"${OPT_ARG}\"\n"
"                shift\n"
"                OPT_ARG=\"$1\"\n\n"
"            else\n"
"                shift\n"
"                if [ $# -gt 0 ]\n"
"                then\n"
"                    case \"$1\" in -* ) ;; * )\n"
"                        OPT_ARG_VAL=\"$1\"\n"
"                        shift ;; esac\n"
"                    OPT_ARG=\"$1\"\n"
"                fi\n"
"            fi\n"
"            ;;\n"
"        esac\n";

extern tOptProc doVersion;
extern tOptProc doPagedUsage;
extern tOptProc doLoadOpt;
char*  pzShell;
char*  pzTrailer = (char*)NULL;

STATIC void  emitFlag(  tOptions* pOpts );
STATIC void  emitLong(  tOptions* pOpts );
STATIC void  emitUsage( tOptions* pOpts );
STATIC void  emitSetup( tOptions* pOpts );
STATIC char* loadTrailer( const char* );


    void
putShellParse( tOptions* pOpts )
{
    tOptDesc* pOptDesc = pOpts->pOptDesc;
    int       optionCt = pOpts->optCt;

    /*
     *  Check for our SHELL option now.
     *  IF the output file contains the "#!" magic marker,
     *  it will override anything we do here.
     */
    if (HAVE_OPT( SHELL ))
        pzShell = OPT_ARG( SHELL );

    else if ((pzShell = getenv( "SHELL" )),
             pzShell == (char*)NULL)

        pzShell = "/bin/sh";

    /*
     *  Check for a specified output file
     */
    if (HAVE_OPT( SCRIPT ))
        pzTrailer = loadTrailer( OPT_ARG( SCRIPT ));

    emitUsage( pOpts );
    emitSetup( pOpts );

    switch (pOpts->fOptSet & (OPTPROC_LONGOPT|OPTPROC_SHORTOPT)) {
    case OPTPROC_LONGOPT:
        fputs( zLoopCase,        stdout );

        fputs( zLongSelection,   stdout );
        fputs( zLongOptInit,     stdout );
        emitLong( pOpts );
        printf( zLongOptArg,     pOpts->pzPROGNAME );
        fputs( zEndSelection,    stdout );

        fputs( zNoSelection,     stdout );
        break;

    case 0:
        fputs( zLoopOnly,        stdout );
        fputs( zLongOptInit,     stdout );
        emitLong( pOpts );
        printf( zLongOptArg,     pOpts->pzPROGNAME );
        break;

    case OPTPROC_SHORTOPT:
        fputs( zLoopCase,        stdout );

        fputs( zFlagSelection,   stdout );
        fputs( zFlagOptInit,     stdout );
        emitFlag( pOpts );
        printf( zFlagOptArg,     pOpts->pzPROGNAME );
        fputs( zEndSelection,    stdout );

        fputs( zNoSelection,     stdout );
        break;

    case OPTPROC_LONGOPT|OPTPROC_SHORTOPT:
        fputs( zLoopCase,        stdout );

        fputs( zLongSelection,   stdout );
        fputs( zLongOptInit,     stdout );
        emitLong( pOpts );
        printf( zLongOptArg,     pOpts->pzPROGNAME );
        fputs( zEndSelection,    stdout );

        fputs( zFlagSelection,   stdout );
        fputs( zFlagOptInit,     stdout );
        emitFlag( pOpts );
        printf( zFlagOptArg,     pOpts->pzPROGNAME );
        fputs( zEndSelection,    stdout );

        fputs( zNoSelection,     stdout );
        break;
    }

    printf( zLoopEnd, pOpts->pzPROGNAME, zTrailerMarker );
    if (pzTrailer != (char*)NULL)
        fputs( pzTrailer, stdout );

    fflush( stdout );
    fchmod( STDOUT_FILENO, 0755 );
    fclose( stdout );
}


    STATIC void
emitUsage( tOptions* pOpts )
{
    /*
     *  First, switch stdout to the output file name.
     *  Then, change the program name to the one defined
     *  by the definitions (rather than the current
     *  executable name).  Down case the upper cased name.
     */
    {
        char*    pzOutName;
        char     zTimeBuf[ 128 ];
        char*    pz;

        {
            time_t    curTime = time( (time_t*)NULL );
            struct tm*  pTime = localtime( &curTime );
            strftime( zTimeBuf, 128, "%A %B %e, %Y at %r %Z", pTime );
        }

        if (HAVE_OPT( SCRIPT ))
             pzOutName = OPT_ARG( SCRIPT );
        else pzOutName = "stdout";

        printf( zPreamble, pzShell, pzOutName, zTimeBuf );

        /*
         *  Get a copy of the original program name in lower case
         */
        pzOutName = zTimeBuf;
        pz        = (char*)pOpts->pzPROGNAME;
        for (;;) {
            if ((*pzOutName++ = tolower( *pz++ )) == '\0')
                break;
        }
        printf( zLongUsageText, zTimeBuf, pOpts->pzPROGNAME );
    }

    fflush( stdout );

    switch (fork()) {
    case -1:
        printf( "Cannot obtain %s usage\n", pOpts->pzProgName );
        break;

    case 0:
        dup2( STDOUT_FILENO, STDERR_FILENO );
        (*(pOpts->pUsageProc))( pOpts, EXIT_SUCCESS );

    default:
    {
        int  stat;
        wait( &stat );
    }
    }

    printf( zShortUsageText, pOpts->pzPROGNAME );
    fflush( stdout );

    switch (fork()) {
    case -1:
        printf( "Cannot obtain %s usage\n", pOpts->pzProgName );
        break;

    case 0:
        dup2( STDOUT_FILENO, STDERR_FILENO );
        (*(pOpts->pUsageProc))( pOpts, EXIT_FAILURE );

    default:
    {
        int  stat;
        wait( &stat );
    }
    }

    fputs( zEndUsageText, stdout );

    {
        tOptDesc* pOptDesc = pOpts->pOptDesc;
        int       optionCt = pOpts->optCt;

        for (;;) {
            if (pOptDesc->pOptProc == doVersion) {
                printf( zVersText, pOpts->pzPROGNAME );
                fflush( stdout );

                switch (fork()) {
                case -1:
                    printf( "Cannot obtain %s version\n", pOpts->pzProgName );
                    break;

                case 0:
                    dup2( STDOUT_FILENO, STDERR_FILENO );
                    pOptDesc->pzLastArg = "copyright";
                    doVersion( pOpts, pOptDesc );

                default:
                {
                    int  stat;
                    wait( &stat );
                }
                }
                fputs( zEndVersionText, stdout );
                break;
            }

            if (--optionCt <= 0)
                break;
            pOptDesc++;
        }
    }
}


    STATIC void
emitSetup( tOptions* pOpts )
{
    tOptDesc* pOptDesc = pOpts->pOptDesc;
    int       optionCt = pOpts->presetOptCt;

    for (;optionCt > 0; pOptDesc++, --optionCt) {

        if (  ((pOptDesc->fOptState & OPTST_DOCUMENT) != 0)
           || (pOptDesc->pz_NAME == (char*)NULL) )
            continue;

        if (pOptDesc->optMaxCt > 1)
             printf( zMultiDef,  pOpts->pzPROGNAME, pOptDesc->pz_NAME );
        else printf( zSingleDef, pOpts->pzPROGNAME, pOptDesc->pz_NAME );
    }
}


    STATIC void
printOptionAction( tOptions* pOpts, tOptDesc* pOptDesc )
{
    if (pOptDesc->pOptProc == doVersion)
        printf( zTextExit, pOpts->pzPROGNAME, "VERSION" );

    else if (pOptDesc->pOptProc == doPagedUsage)
        printf( zTextExit, pOpts->pzPROGNAME, "LONGUSAGE" );

    else if (pOptDesc->pOptProc == doLoadOpt) {
        printf( zCmdFmt, "echo 'Warning:  Cannot load options files' >&2" );
        printf( zCmdFmt, "OPT_ARG=YES" );

    } else if (pOptDesc->pz_NAME == (char*)NULL) {

        if (pOptDesc->pOptProc == (tOptProc*)NULL) {
            printf( zCmdFmt, "echo 'Warning:  Cannot save options files' "
                    ">&2" );
            printf( zCmdFmt, "OPT_ARG=OK" );
        } else
            printf( zTextExit, pOpts->pzPROGNAME, "LONGUSAGE" );

    } else {
        if (pOptDesc->optMaxCt == 1)
            printf( zSingleArg, pOpts->pzPROGNAME, pOptDesc->pz_NAME );
        else {
            if ((unsigned)pOptDesc->optMaxCt < NOLIMIT)
                printf( zCountTest, pOpts->pzPROGNAME,
                        pOptDesc->pz_NAME, pOptDesc->optMaxCt );

            printf( zMultiArg, pOpts->pzPROGNAME, pOptDesc->pz_NAME );
        }

        switch (pOptDesc->optArgType) {
        case ARG_MAY:
            printf( zMayArg,  pOpts->pzPROGNAME, pOptDesc->pz_NAME );
            break;

        case ARG_MUST:
            fputs( zMustArg, stdout );
            break;

        default:
            printf( zCantArg, pOpts->pzPROGNAME, pOptDesc->pz_NAME );
            break;
        }
    }
    fputs( zOptionEndSelect, stdout );
}


    STATIC void
printOptionInaction( tOptions* pOpts, tOptDesc* pOptDesc )
{
    if (pOptDesc->pOptProc == doLoadOpt) {
        printf( zCmdFmt, "echo 'Warning:  Cannot suppress the loading of "
                "options files' >&2" );

    } else if (pOptDesc->optMaxCt == 1)
        printf( zNoSingleArg, pOpts->pzPROGNAME,
                pOptDesc->pz_NAME, pOptDesc->pz_DisablePfx );
    else
        printf( zNoMultiArg, pOpts->pzPROGNAME,
                pOptDesc->pz_NAME, pOptDesc->pz_DisablePfx );

    printf( zCmdFmt, "OPT_ARG=NO" );
    fputs( zOptionEndSelect, stdout );
}


    STATIC void
emitFlag( tOptions* pOpts )
{
    tOptDesc* pOptDesc = pOpts->pOptDesc;
    int       optionCt = pOpts->optCt;

    fputs( zOptionCase, stdout );

    for (;optionCt > 0; pOptDesc++, --optionCt) {

        if ((pOptDesc->fOptState & OPTST_DOCUMENT) != 0)
            continue;

        if (isprint( pOptDesc->optValue )) {
            printf( zOptionFlag, pOptDesc->optValue );
            printOptionAction( pOpts, pOptDesc );
        }
    }
    printf( zOptionUnknown, "flag", pOpts->pzPROGNAME );
}



    STATIC void
emitLong( tOptions* pOpts )
{
    tOptDesc* pOptDesc = pOpts->pOptDesc;
    int       optionCt = pOpts->optCt;

    fputs( zOptionCase, stdout );

    for (;optionCt > 0; pOptDesc++, --optionCt) {
        tOptDesc* pOD = pOpts->pOptDesc;
        int       oCt = pOpts->optCt;
        int       min = 1;
        int       matchCt = 0;
        char      zName[ 256 ];
        char*     pz = zName;

        if ((pOptDesc->fOptState & OPTST_DOCUMENT) != 0)
            continue;

        for (;;) {
            matchCt = 0;

            if (  (pOD == pOptDesc)
               || ((pOD->fOptState & OPTST_DOCUMENT) != 0)  ){
                if (--oCt <= 0)
                    break;
                pOD++;
                continue;
            }

            while (  toupper( pOD->pz_Name[matchCt] )
                  == toupper( pOptDesc->pz_Name[matchCt] ))
                matchCt++;
            if (matchCt > min)
                min = matchCt;

            if (pOD->pz_DisableName != (char*)NULL) {
                matchCt = 0;
                while (  toupper( pOD->pz_DisableName[matchCt] )
                      == toupper( pOptDesc->pz_Name[matchCt] ))
                    matchCt++;
                if (matchCt > min)
                    min = matchCt;
            }
            if (--oCt <= 0)
                break;
            pOD++;
        }

        /*
         *  IF the 'min' is all or one short of the name length,
         *  THEN the entire string must be matched.
         */
        if (  (pOptDesc->pz_Name[min  ] == NUL)
           || (pOptDesc->pz_Name[min+1] == NUL) )
            printf( zOptionFullName, pOptDesc->pz_Name );

        else {
            for (matchCt = 0; matchCt <= min; matchCt++)
                *pz++ = pOptDesc->pz_Name[matchCt];

            for (;;) {
                *pz = NUL;
                printf( zOptionPartName, zName );
                *pz++ = pOptDesc->pz_Name[matchCt++];
                if (pOptDesc->pz_Name[matchCt] == NUL) {
                    *pz = NUL;
                    printf( zOptionFullName, zName );
                    break;
                }
            }
        }

        printOptionAction( pOpts, pOptDesc );
        if (pOptDesc->pz_DisableName == (char*)NULL)
            continue;

        pOD = pOpts->pOptDesc;
        oCt = pOpts->optCt;
        min = 1;
        pz = zName;

        for (;;) {
            matchCt = 0;

            if (  (pOD == pOptDesc)
               || ((pOD->fOptState & OPTST_DOCUMENT) != 0)  ){
                if (--oCt <= 0)
                    break;
                pOD++;
                continue;
            }

            while (  toupper( pOD->pz_Name[matchCt] )
                  == toupper( pOptDesc->pz_DisableName[matchCt] ))
                matchCt++;
            if (matchCt > min)
                min = matchCt;

            if (pOD->pz_DisableName != (char*)NULL) {
                matchCt = 0;
                while (  toupper( pOD->pz_DisableName[matchCt] )
                      == toupper( pOptDesc->pz_DisableName[matchCt] ))
                    matchCt++;
                if (matchCt > min)
                    min = matchCt;
            }
            if (--oCt <= 0)
                break;
            pOD++;
        }

        /*
         *  IF the 'min' is all or one short of the name length,
         *  THEN the entire string must be matched.
         */
        if (  (pOptDesc->pz_DisableName[min  ] == NUL)
           || (pOptDesc->pz_DisableName[min+1] == NUL) )
            printf( zOptionFullName, pOptDesc->pz_DisableName );

        else {
            for (matchCt = 0; matchCt <= min; matchCt++)
                *pz++ = pOptDesc->pz_DisableName[matchCt];

            for (;;) {
                *pz = NUL;
                printf( zOptionPartName, zName );
                *pz++ = pOptDesc->pz_DisableName[matchCt++];
                if (pOptDesc->pz_DisableName[matchCt] == NUL) {
                    *pz = NUL;
                    printf( zOptionFullName, zName );
                    break;
                }
            }
        }

        printOptionInaction( pOpts, pOptDesc );
    }
    printf( zOptionUnknown, "option", pOpts->pzPROGNAME );
}


    STATIC char*
loadTrailer( const char* pzFile )
{
    FILE* fp;
    char* pzData = (char*)NULL;
    struct stat stbf;

    do  {
        char*  pzScan;
        int    sizeLeft;

        /*
         *  IF we cannot stat the file,
         *  THEN assume we are creating a new file.
         *       Skip the loading of the old data.
         */
        if (stat( pzFile, &stbf ) != 0)
            break;

        /*
         *  The file must be a regular file
         */
        if (! S_ISREG( stbf.st_mode )) {
            fprintf( stderr, "Error `%s' is not a regular file\n",
                     pzFile );
            exit( EXIT_FAILURE );
        }

        pzData = (char*)malloc( stbf.st_size + 1 );
        fp = fopen( pzFile, "r" );

        sizeLeft = stbf.st_size;
        pzScan   = pzData;

        /*
         *  Read in all the data as fast as our OS will let us.
         */
        for (;;) {
            int inct = fread( (void*)pzScan, 1, sizeLeft, fp );
            if (inct == 0)
                break;

            pzScan   += inct;
            sizeLeft -= inct;

            if (sizeLeft == 0)
                break;
        }

        /*
         *  NUL-terminate the data and look for our end marker.
         */
        *pzScan = '\0';
        fclose( fp );
        pzScan = strstr( pzData, zTrailerMarker );

        /*
         *  IF the file begins with the magic marker (it ought to),
         *  THEN we will set the `pzShell' pointer to the first
         *       non-whitespace character on this line.
         *       (If it is all white, we will leave `pzShell' alone.)
         */
        if (strncmp( pzData, "#!", 2 ) == 0) {
            char* pz = pzData + 2 + strspn( pzData+2, " \t\v\f" );
            if (! isspace( *pz )) {
                pzShell = pz;
                while (! isspace( *++pz ))  ;

                /*
                 *  IF there is something else on the line,
                 *  THEN append it to the 'SHELL' value.
                 *       It is used as the first argument to the script.
                 */
                if ((*pz == ' ') || (*pz == '\t')) {
                    pz += strspn( pz, " \t\v\f" );

                    /*
                     *  IF all we have is white space to the newline,
                     *  THEN go back and take only the shell name.
                     */
                    if (isspace( *pz ))
                        pz = pzShell;

                    while (! isspace( *++pz ))  ;
                }

                /*
                 *  IF we did not find our marker,
                 *  THEN the 'rest' output starts on the next line.
                 */
                if (pzScan == (char*)NULL) {
                    pzData = strchr( pz, '\n' );
                    if (pzData != (char*)NULL)
                        pzData++;
                }

                /*
                 *  NUL terminate the shell name and argument.
                 */
                *pz = '\0';
            }
        }

        /*
         *  Check to see if the data contains
         *  our marker.  If it does, then we will skip over it
         */
        if (pzScan != (char*)NULL)
            pzData = pzScan + sizeof( zTrailerMarker ) - 1;

    } while (AG_FALSE);

    freopen( pzFile, "w", stdout );
    return pzData;
}



    void
genshelloptUsage( tOptions*  pOptions, int exitCode )
{
    /*
     *  IF not EXIT_SUCCESS,
     *  THEN emit the short form of usage.
     */
    if (exitCode != EXIT_SUCCESS)
        optionUsage( pOptions, exitCode );
    fflush( stderr );
    fflush( stdout );

    /*
     *  First, print our usage
     */
    switch (fork()) {
    case -1:
        optionUsage( pOptions, EXIT_FAILURE );
        /*NOTREACHED*/
        _exit( EXIT_FAILURE );

    case 0:
        pagerState = PAGER_STATE_CHILD;
        optionUsage( pOptions, EXIT_SUCCESS );
        /*NOTREACHED*/
        _exit( EXIT_FAILURE );

    default:
    {
        int  stat;
        wait( &stat );
    }
    }

    /*
     *  Generate the pzProgName, since optionProcess() normally
     *  gets it from the command line
     */
    {
        char* pz = strdup( pShellParseOptions->pzPROGNAME );
        pShellParseOptions->pzProgName = pz;
        while (*pz != NUL) {
            *pz = tolower( *pz );
            pz++;
        }
    }

    /*
     *  Separate the makeshell usage from the client usage
     */
    {
        static const char zMsg[] =
            "\n= = = = = = = =\n\n"
            "This incarnation of genshell is will produce\n"
             "a shell script to parse the options for %s:\n\n";
        fprintf( stderr, zMsg, pShellParseOptions->pzProgName );
    }
    fflush( stderr );
    fflush( stdout );

    /*
     *  Now, print the client usage.
     */
    switch (fork()) {
    case 0:
        pagerState = PAGER_STATE_CHILD;
        /*FALLTHROUGH*/
    case -1:
        optionUsage( pShellParseOptions, EXIT_FAILURE );

    default:
    {
        int  stat;
        wait( &stat );
    }
    }

    exit( EXIT_SUCCESS );
}
