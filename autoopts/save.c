
/*
 *  save.c  $Id: save.c,v 3.9 2003/03/02 23:39:10 bkorb Exp $
 *
 *  This module's routines will take the currently set options and
 *  store them into an ".rc" file for re-interpretation the next
 *  time the invoking program is run.
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
#include <time.h>

tSCC  zWarn[] = "%s WARNING:  cannot save options - ";

STATIC char*
findDirName( pOpts )
    tOptions*  pOpts;
{
    char*  pzDir;

    if (pOpts->specOptIdx.save_opts == 0)
        return (char*)NULL;

    pzDir = pOpts->pOptDesc[ pOpts->specOptIdx.save_opts ].pzLastArg;
    if ((pzDir != (char*)NULL) && (*pzDir != NUL))
        return pzDir;

    /*
     *  This function only works if there is a directory where
     *  we can stash the RC (INI) file.
     */
    {
        const char** papz = pOpts->papzHomeList;
        if (papz == (const char**)NULL)
            return (char*)NULL;

        while (papz[1] != (char*)NULL) papz++;
        pzDir = (char*)(*papz);
    }

    /*
     *  IF it does not require deciphering an env value, then just copy it
     */
    if (*pzDir != '$')
        return pzDir;

    {
        char* pzEndDir = strchr( ++pzDir, '/' );
        char* pzFileName;
        char* pzEnv;

        if (pzEndDir != (char*)NULL)
            *(pzEndDir++) = NUL;

        /*
         *  Make sure we can get the env value (after stripping off
         *  any trailing directory or file names)
         */
        pzEnv = getenv( pzDir );
        if (pzEnv == (char*)NULL) {
            fprintf( stderr, zWarn, pOpts->pzProgName );
            fprintf( stderr, "'%s' not defined\n", pzDir );
            return (char*)NULL;
        }

        if (pzEndDir == (char*)NULL)
            return pzEnv;

        {
            size_t sz = strlen( pzEnv ) + 2;
            if (pzEndDir != NULL)
                sz += strlen( pzEndDir );
            pzFileName = (char*)AGALOC( sz, "dir name" );
        }

        /*
         *  IF we stripped something off, now tack it back on.
         */
        if (pzFileName == (char*)NULL)
            return (char*)NULL;

        strcpy( pzFileName, pzEnv );
        pzDir = pzFileName + strlen( pzFileName );

        while ((pzDir > pzFileName) && (pzDir[-1] == '/')) pzDir--;
        *(pzDir++) = '/';
        strcpy( pzDir, pzEndDir );
        return pzFileName;
    }
}


STATIC char*
findFileName( pOpts )
    tOptions*  pOpts;
{
    char*  pzDir;
    tSCC   zNoStat[] = "error %d (%s) stat-ing %s\n";
    struct stat stBuf;

    pzDir = findDirName( pOpts );
    if (pzDir == (char*)NULL)
        return (char*)NULL;

    /*
     *  See if we can find the specified directory.
     *  We use a once-only loop structure so we can bail
     *  out if we can recover from the unusual condition.
     */
    if (stat( pzDir, &stBuf ) != 0) do {

        /*
         *  IF we could not, check to see if we got a full
         *  path to a file name that has not been created yet.
         */
        if (errno == ENOENT) {
            /*
             *  Strip off the last component,
             *  stat the remaining string
             *  and that string must name a directory
             */
            char*  pzDirCh = strrchr( pzDir, '/' );
            if (pzDirCh == (char*)NULL) {
                stBuf.st_mode = S_IFREG;
                break;  /* bail out of error condition */
            }

            *pzDirCh = NUL;
            if (  (stat( pzDir, &stBuf ) == 0)
               && S_ISDIR( stBuf.st_mode )) {

                /*
                 *  We found the directory.
                 *  Restore the file name and mark the
                 *  full name as a regular file
                 */
                *pzDirCh = '/';
                stBuf.st_mode = S_IFREG;

                continue;  /* bail out of error condition */
            }

            *pzDirCh = '/';
        }

        /*
         *  We got a bogus name.
         */
        fprintf( stderr, zWarn, pOpts->pzProgName );
        fprintf( stderr, zNoStat, errno, strerror( errno ), pzDir );
        return (char*)NULL;
    } while (0);

    /*
     *  IF what we found was a directory,
     *  THEN tack on the RC file name
     */
    if (S_ISDIR( stBuf.st_mode )) {
        size_t sz = strlen( pzDir ) + strlen( pOpts->pzRcName ) + 2;

        {
            char*  pzPath = (char*)AGALOC( sz, "file name" );
#ifdef HAVE_SNPRINTF
            snprintf( pzPath, sz, "%s/%s", pzDir, pOpts->pzRcName );
#else
            sprintf( pzPath, "%s/%s", pzDir, pOpts->pzRcName );
#endif
            pzDir = pzPath;
        }

        /*
         *  IF we cannot stat the object for any reason other than
         *     it does not exist, then we bail out
         */
        if (stat( pzDir, &stBuf ) != 0) {
            if (errno != ENOENT) {
                fprintf( stderr, zWarn, pOpts->pzProgName );
                fprintf( stderr, zNoStat, errno, strerror( errno ),
                         pzDir );
                return (char*)NULL;
            }

            /*
             *  It does not exist yet, but it will be a regular file
             */
            stBuf.st_mode = S_IFREG;
        }
    }

    /*
     *  Make sure that whatever we ultimately found, that it either is
     *  or will soon be a file.
     */
    if (! S_ISREG( stBuf.st_mode )) {
        fprintf( stderr, zWarn, pOpts->pzProgName );
        fprintf( stderr, "'%s' is not a regular file.\n", pzDir );
        return (char*)NULL;
    }

    /*
     *  Get rid of the old file
     */
    unlink( pzDir );
    return pzDir;
}


STATIC void
printEntry( fp, p, pzLA )
    FILE*      fp;
    tOptDesc*  p;
    char*      pzLA;
{
    /*
     *  There is an argument.  Pad the name so values line up
     */
    fprintf( fp, "%-18s",
             (DISABLED_OPT( p )) ? p->pz_DisableName : p->pz_Name );

    /*
     *  IF the option is numeric only,
     *  THEN the char pointer is really the number
     */
    if ((p->fOptState & OPTST_NUMERIC) != 0)
        fprintf( fp, "  %ld\n", (t_word)pzLA );

    /*
     *  OTHERWISE, FOR each line of the value text, ...
     */
    else if (pzLA == NULL)
        fputc( '\n', fp );

    else {
        fputc( ' ', fp ); fputc( ' ', fp );
        for (;;) {
            char* pzNl = strchr( pzLA, '\n' );

            /*
             *  IF this is the last line
             *  THEN print it and bail
             */
            if (pzNl == (char*)NULL)
                break;

            /*
             *  Print the continuation and the text from the current line
             */
            fputs( "\\\n", fp );
            *pzNl = NUL;
            fputs( pzLA, fp );
            *pzNl = '\n';   /* Restore the newline */

            pzLA  = pzNl+1; /* advance the Last Arg pointer */
        }

        /*
         *  Terminate the entry
         */
        fputs( pzLA, fp );
        fputc( '\n', fp );
    }
}


/*=export_func  optionSaveFile
 *
 * what:  saves the option state to a file
 *
 * arg:   tOptions*,   pOpts,  program options descriptor
 *
 * doc:
 *
 * This routine will save the state of option processing to a file.  The name
 * of that file can be specified with the argument to the @code{--save-opts}
 * option, or by appending the @code{rcfile} attribute to the last
 * @code{homerc} attribute.  If no @code{rcfile} attribute was specified, it
 * will default to @code{.@i{programname}rc}.  If you wish to specify another
 * file, you should invoke the @code{SET_OPT_SAVE_OPTS( @i{filename} )} macro.
 *
 * err:
 *
 * If no @code{homerc} file was specified, this routine will silently return
 * and do nothing.  If the output file cannot be created or updated, a message
 * will be printed to @code{stderr} and the routine will return.
=*/
#define ARGTYPE \
  (OPTST_NUMERIC | OPTST_STACKED | OPTST_ENUMERATION | OPTST_BOOLEAN )

void
optionSaveFile( pOpts )
    tOptions* pOpts;
{
    char*  pzFName;

    pzFName = findFileName( pOpts );
    if (pzFName == (char*)NULL)
        return;

    {
        tOptDesc* pOD = pOpts->pOptDesc;
        int       ct  = pOpts->presetOptCt;
        FILE*     fp  = fopen( pzFName, "w" FOPEN_BINARY_FLAG );

        if (fp == (FILE*)NULL) {
            fprintf( stderr, zWarn, pOpts->pzProgName );
            fprintf( stderr, "error %d (%s) creating %s\n", errno,
                     strerror( errno ), pzFName );
            return;
        }

        {
            const char*  pz = pOpts->pzUsageTitle;
            fputs( "#  ", fp );
            do { fputc( *pz, fp ); } while (*(pz++) != '\n');
        }

        {
            time_t  timeVal = time( (time_t*)NULL );
            char*   pzTime  = ctime( &timeVal );

            fprintf( fp, "#  preset/initialization file\n#  %s#\n",
                     pzTime );
#ifdef HAVE_ALLOCATED_CTIME
            /*
             *  The return values for ctime(), localtime(), and gmtime()
             *  normally point to static data that is overwritten by each call.
             */
            free( (void*)pzTime );
#endif
        }

        /*
         *  FOR each of the defined options, ...
         */
        do  {
            tOptDesc*  p;

            /*
             *  IF    the option has not been defined
             *     OR it does not take an initialization value
             *     OR it is equivalenced to another option
             *  THEN continue (ignore it)
             */
            if (UNUSED_OPT( pOD ))
                continue;

            if ((pOD->fOptState & (OPTST_NO_INIT|OPTST_DOCUMENT|OPTST_OMITTED))
                 != 0)
                continue;

            if (  (pOD->optEquivIndex != NO_EQUIVALENT)
               && (pOD->optEquivIndex != pOD->optIndex))
                continue;

            /*
             *  Set a temporary pointer to the real option description
             *  (i.e. account for equivalencing)
             */
            p = ((pOD->fOptState & OPTST_EQUIVALENCE) != 0)
                ? (pOpts->pOptDesc + pOD->optActualIndex) : pOD;

            /*
             *  IF    no arguments are allowed
             *  THEN just print the name and continue
             */
            if (p->optArgType == ARG_NONE) {
                fprintf( fp, "%s\n", (DISABLED_OPT( p ))
                                     ? p->pz_DisableName
                                     : p->pz_Name );
                continue;
            }

            switch (p->fOptState & ARGTYPE) {
            case 0:
            case OPTST_NUMERIC:
                printEntry( fp, p, p->pzLastArg );
                break;

            case OPTST_STACKED:
            {
                tArgList*  pAL = (tArgList*)p->optCookie;
                int        uct = pAL->useCt;
                char**     ppz = pAL->apzArgs;

                /*
                 *  Disallow multiple copies of disabled options.
                 */
                if (uct > 1)
                    p->fOptState &= ~OPTST_DISABLED;

                while (uct-- > 0)
                    printEntry( fp, p, *(ppz++) );
                break;
            }

            case OPTST_ENUMERATION:
            {
                intptr_t enumVal = (int)(p->pzLastArg);
                /*
                 *  This is a magic incantation that will convert the
                 *  enumeration value back into a string suitable for printing.
                 *  We restore the numeric value after printing.
                 */
                (*(p->pOptProc))( (tOptions*)2UL, p );
                printEntry( fp, p, p->pzLastArg );
                p->pzLastArg = (char*)enumVal;
                break;
            }

            case OPTST_BOOLEAN:
                printEntry( fp, p, (p->pzLastArg != 0) ? "true" : "false" );
                break;

            default:
                break; /* bogus - skip it */
            }
        } while ( (pOD++), (--ct > 0));

        fclose( fp );
    }
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * save.c ends here */
