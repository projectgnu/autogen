
/*
 *  $Id: makeshell.c,v 3.0 2001/12/09 19:43:58 bkorb Exp $
 *
 *  This module will interpret the options set in the tOptions
 *  structure and create a Bourne shell script capable of parsing them.
 */

/*
 *  Automated Options copyright 1992-2001 Bruce Korb
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
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>

#ifdef __STDC__

#include "autoopts.h"
#include "genshell.c"

tOptions*  pShellParseOptions = (tOptions*)NULL;

/* * * * * * * * * * * * * * * * * * * * *
 *
 *  Setup Format Strings
 */
static const char zStartMarker[] =
"# # # # # # # # # # -- do not modify this marker --\n#\n"
"#  DO NOT EDIT THIS SECTION";

static const char zPreamble[] =
"%s OF %s\n#\n"
"#  From here to the next `-- do not modify this marker --',\n"
"#  the text has been generated %s\n";

static const char zEndPreamble[] =
"#  From the %s option definitions\n#\n";

static const char zMultiDef[] = "\n"
"%1$s_%2$s_CT=0\n"
"export %1$s_%2$s_CT\n";

static const char zSingleDef[] = "\n"
"%1$s_%2$s='%3$s'\n"
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
"unset OPT_PROCESS || :\n"
"unset OPT_ELEMENT || :\n"
"unset OPT_ARG || :\n"
"unset OPT_ARG_NEEDED || :\n"
"unset OPT_NAME || :\n"
"unset OPT_CODE || :\n"
"unset OPT_ARG_VAL || :\n%2$s";

static const char zTrailerMarker[] = "\n"
"# # # # # # # # # #\n#\n"
"#  END OF AUTOMATED OPTION PROCESSING\n"
"#\n# # # # # # # # # # -- do not modify this marker --\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  OPTION SELECTION
 */
static const char zOptionCase[] =
"        case \"${OPT_CODE}\" in\n";

static const char zOptionPartName[] =
"        '%s' | \\\n";

static const char zOptionFullName[] =
"        '%s' )\n";

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

static const char zPagedUsageExit[] =
"            echo \"$%s_LONGUSAGE_TEXT\" | ${PAGER-more}\n"
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
"            OPT_ARG_NEEDED=OK\n";

static const char zMustArg[] =
"            OPT_ARG_NEEDED=YES\n";

static const char zCantArg[] =
"            eval %1$s_%2$s${OPT_ELEMENT}=true\n"
"            export %1$s_%2$s${OPT_ELEMENT}\n"
"            OPT_ARG_NEEDED=NO\n";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  LONG OPTION PROCESSING
 *
 *  Formats for emitting the text for handling long option types
 */
static const char zLongOptInit[] =
"        OPT_CODE=`echo \"X${OPT_ARG}\"|sed 's/^X-*//'`\n"
"        shift\n"
"        OPT_ARG=\"$1\"\n\n"
"        case \"${OPT_CODE}\" in *=* )\n"
"            OPT_ARG_VAL=`echo \"${OPT_CODE}\"|sed 's/^[^=]*=//'`\n"
"            OPT_CODE=`echo \"${OPT_CODE}\"|sed 's/=.*$//'` ;; esac\n\n";

static const char zLongOptArg[] =
"        case \"${OPT_ARG_NEEDED}\" in\n"
"        NO )\n"
"            OPT_ARG_VAL=''\n"
"            ;;\n\n"
"        YES )\n"
"            if [ -z \"${OPT_ARG_VAL}\" ]\n"
"            then\n"
"                if [ $# -eq 0 ]\n"
"                then\n"
"                    echo No argument provided for ${OPT_NAME} option >&2\n"
"                    echo \"$%s_USAGE_TEXT\"\n"
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
"        OPT_CODE=`echo \"X${OPT_ARG}\" | sed 's/X-\\(.\\).*/\\1/'`\n"
"        OPT_ARG=` echo \"X${OPT_ARG}\" | sed 's/X-.//'`\n\n";

static const char zFlagOptArg[] =
"        case \"${OPT_ARG_NEEDED}\" in\n"
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
"                    echo \"$%s_USAGE_TEXT\"\n"
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


#define TEXTTO_TABLE \
        _TT_( LONGUSAGE ) \
        _TT_( USAGE ) \
        _TT_( VERSION )
#define _TT_(n) \
        TT_ ## n ,

typedef enum { TEXTTO_TABLE COUNT_TT } teTextTo;

#undef _TT_

extern tOptProc doVersion;
extern tOptProc doPagedUsage;
extern tOptProc doLoadOpt;
STATIC char*  pzShell   = (char*)NULL;
STATIC char*  pzTrailer = (char*)NULL;

STATIC void  emitFlag(  tOptions* pOpts );
STATIC void  emitLong(  tOptions* pOpts );
STATIC void  emitUsage( tOptions* pOpts );
STATIC void  emitSetup( tOptions* pOpts );
STATIC char* loadTrailer( const char* );


void
putShellParse( tOptions* pOpts )
{
    /*
     *  Check for our SHELL option now.
     *  IF the output file contains the "#!" magic marker,
     *  it will override anything we do here.
     */
    if (HAVE_OPT( SHELL ))
        pzShell = OPT_ARG( SHELL );

    else if (! ENABLED_OPT( SHELL ))
        pzShell = (char*)NULL;

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
    if ((pzTrailer != (char*)NULL) && (*pzTrailer != '\0'))
        fputs( pzTrailer, stdout );
    else if (ENABLED_OPT( SHELL ))
        printf( "\nenv | egrep %s_\n", pOpts->pzPROGNAME );

    fflush( stdout );
    fchmod( STDOUT_FILENO, 0755 );
    fclose( stdout );
}


STATIC void
textToVariable( tOptions* pOpts, teTextTo whichVar, tOptDesc* pOD )
{
    int  nlHoldCt = 0;
    int  pipeFd[2];
    FILE* fp;

#   define _TT_(n) static const char z ## n [] = #n;
    TEXTTO_TABLE
#   undef _TT_
#   define _TT_(n) z ## n ,
      static const char*  apzTTNames[] = { TEXTTO_TABLE };
#   undef _TT_

    printf( "%s_%s_TEXT='", pOpts->pzPROGNAME, apzTTNames[ whichVar ]);
    fflush( stdout );

    if (pipe( pipeFd ) != 0) {
        fprintf( stderr, "Error %d (%s) from the pipe(2) syscall\n",
                 errno, strerror( errno ));
        exit( EXIT_FAILURE );
    }

    switch (fork()) {
    case -1:
        printf( "Cannot obtain %s usage\n", pOpts->pzProgName );
        exit( EXIT_FAILURE );
        break;

    case 0:
        dup2( pipeFd[1], STDERR_FILENO );
        dup2( pipeFd[1], STDOUT_FILENO );
        close( pipeFd[0] );

        switch (whichVar) {
        case TT_LONGUSAGE:
            (*(pOpts->pUsageProc))( pOpts, EXIT_SUCCESS );
            /* NOTREACHED */
            exit( EXIT_FAILURE );

        case TT_USAGE:
            (*(pOpts->pUsageProc))( pOpts, EXIT_FAILURE );
            /* NOTREACHED */
            exit( EXIT_FAILURE );

        case TT_VERSION:
            pOD->pzLastArg = "c";
            doVersion( pOpts, pOD );
            /* NOTREACHED */

        default:
            exit( EXIT_FAILURE );
        }

    default:
        close( pipeFd[1] );
        fp = fdopen( pipeFd[0], "r" FOPEN_BINARY_FLAG );
    }

    for (;;) {
        int  ch = fgetc( fp );
        switch (ch) {

        case '\n':
            nlHoldCt++;
            break;

        case '\'':
            while (nlHoldCt > 0) {
                fputc( '\n', stdout );
                nlHoldCt--;
            }
            fputs( "'\\''", stdout );
            break;

        case EOF:
            goto endCharLoop;

        default:
            while (nlHoldCt > 0) {
                fputc( '\n', stdout );
                nlHoldCt--;
            }
            fputc( ch, stdout );
            break;
        }
    } endCharLoop:;

    fputs( "'\n\n", stdout );
    close( pipeFd[0] );
}


STATIC void
emitUsage( tOptions* pOpts )
{
    char     zTimeBuf[ 128 ];

    /*
     *  First, switch stdout to the output file name.
     *  Then, change the program name to the one defined
     *  by the definitions (rather than the current
     *  executable name).  Down case the upper cased name.
     */
    {
        char*    pzOutName;
        char*    pz;

        {
            time_t    curTime = time( (time_t*)NULL );
            struct tm*  pTime = localtime( &curTime );
            strftime( zTimeBuf, 128, "%A %B %e, %Y at %r %Z", pTime );
        }

        if (HAVE_OPT( SCRIPT ))
             pzOutName = OPT_ARG( SCRIPT );
        else pzOutName = "stdout";

        if (pzShell != (char*)NULL)
            printf( "#! %s\n", pzShell );

        printf( zPreamble, zStartMarker, pzOutName, zTimeBuf );

        /*
         *  Get a copy of the original program name in lower case
         */
        pzOutName = zTimeBuf;
        pz        = (char*)pOpts->pzPROGNAME;
        for (;;) {
            if ((*pzOutName++ = tolower( *pz++ )) == '\0')
                break;
        }
    }

    printf( zEndPreamble, zTimeBuf, pOpts->pzPROGNAME );

    pOpts->pzProgPath = pOpts->pzProgName = zTimeBuf;
    textToVariable( pOpts, TT_LONGUSAGE,  (tOptDesc*)NULL );
    textToVariable( pOpts, TT_USAGE, (tOptDesc*)NULL );

    {
        tOptDesc* pOptDesc = pOpts->pOptDesc;
        int       optionCt = pOpts->optCt;

        for (;;) {
            if (pOptDesc->pOptProc == doVersion) {
                textToVariable( pOpts, TT_VERSION, pOptDesc );
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
    const char* pzFmt;
    const char* pzValue;

    for (;optionCt > 0; pOptDesc++, --optionCt) {
        char zVal[20];

        if (SKIP_OPT(pOptDesc) || (pOptDesc->pz_NAME == (char*)NULL))
            continue;

        if (pOptDesc->optMaxCt > 1)
             pzFmt = zMultiDef;
        else pzFmt = zSingleDef;

        if (pOptDesc->fOptState & OPTST_NUMERIC) {
            snprintf( zVal, sizeof( zVal ), "%ld", (tUL)pOptDesc->pzLastArg );
            pzValue = zVal;
        } else if (pOptDesc->pzLastArg == (char*)NULL)
            pzValue = "";
        else
            pzValue = pOptDesc->pzLastArg;

        printf( pzFmt, pOpts->pzPROGNAME, pOptDesc->pz_NAME, pzValue );
    }
}


STATIC void
printOptionAction( tOptions* pOpts, tOptDesc* pOptDesc )
{
    if (pOptDesc->pOptProc == doVersion)
        printf( zTextExit, pOpts->pzPROGNAME, "VERSION" );

    else if (pOptDesc->pOptProc == doPagedUsage)
        printf( zPagedUsageExit, pOpts->pzPROGNAME );

    else if (pOptDesc->pOptProc == doLoadOpt) {
        printf( zCmdFmt, "echo 'Warning:  Cannot load options files' >&2" );
        printf( zCmdFmt, "OPT_ARG_NEEDED=YES" );

    } else if (pOptDesc->pz_NAME == (char*)NULL) {

        if (pOptDesc->pOptProc == (tOptProc*)NULL) {
            printf( zCmdFmt, "echo 'Warning:  Cannot save options files' "
                    ">&2" );
            printf( zCmdFmt, "OPT_ARG_NEEDED=OK" );
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

    printf( zCmdFmt, "OPT_ARG_NEEDED=NO" );
    fputs( zOptionEndSelect, stdout );
}


STATIC void
emitFlag( tOptions* pOpts )
{
    tOptDesc* pOptDesc = pOpts->pOptDesc;
    int       optionCt = pOpts->optCt;

    fputs( zOptionCase, stdout );

    for (;optionCt > 0; pOptDesc++, --optionCt) {

        if (SKIP_OPT(pOptDesc))
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

        if (SKIP_OPT(pOptDesc))
            continue;

        for (;;) {
            matchCt = 0;

            if ((pOD == pOptDesc) || SKIP_OPT(pOD)){
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

            if ((pOD == pOptDesc) || SKIP_OPT(pOptDesc)){
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


char*
setShell( char** ppzData, char* pzEndMarker )
{
    char* pz;
    char* pzRes;
    char* pzData = *ppzData;

    /*
     *  IF the file begins with the magic marker (it ought to),
     *  THEN we will set the `pzShell' pointer to the first
     *       non-whitespace character on this line.
     *       (If it is all white, we will leave `pzShell' alone.)
     */
    if (strncmp( pzData, "#!", 2 ) != 0)
        return pzShell;

    pz = pzData + 2 + strspn( pzData+2, " \t\v\f" );
    if (*pz == '\n')
        return pzShell;

    pzRes = pz;
    /*
     *  We now have the start of the string that we return
     *  as the result.  Now, we must determine where to end
     *  the string.
     */
    pz = strstr( pz, zStartMarker );

    /*
     *  IF we can find our start marker (we ought to),
     *  THEN we will keep everything up to that.
     *       This allows users to keep initial comments.
     *       This should be the normal return point.
     */
    if ((pz != (char*)NULL) && (pz < pzEndMarker)) {
        *pz = NUL;
        return pzRes;
    }

    /*
     *  Go back to the result point and scan over entries on this line.
     */
    pz = pzRes;
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
        if (*pz == '\n')
            pz = pzShell;

        while (! isspace( *++pz ))  ;
    }

    /*
     *  IF we did not find our marker,
     *  THEN the 'rest' output starts on the next line.
     */
    if (pzEndMarker == (char*)NULL) {
        (*ppzData) = strchr( pz, '\n' );
        if ((*ppzData) != (char*)NULL)
            (*ppzData)++;
    }

    /*
     *  NUL terminate the shell name and argument.
     */
    *pz = '\0';
    return pzRes;
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
        fp = fopen( pzFile, "r" FOPEN_BINARY_FLAG );

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
        pzScan  = strstr( pzData, zTrailerMarker );
        pzShell = setShell( &pzData, pzScan );

        /*
         *  Check to see if the data contains
         *  our marker.  If it does, then we will skip over it
         */
        if (pzScan != (char*)NULL)
            pzData = pzScan + sizeof( zTrailerMarker ) - 1;

    } while (AG_FALSE);

    freopen( pzFile, "w" FOPEN_BINARY_FLAG, stdout );
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
            "This incarnation of genshell will produce\n"
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
#else
int putShellParse( pOpts )
    tOptions* pOpts;
{
    fputs( "putShellParse disabled for pre-ANSI C\n", stderr );
    exit( EXIT_FAILURE );
}
#endif

/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * makeshell.c ends here */
