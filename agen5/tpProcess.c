
/*
 *  agTempl.c
 *  $Id: tpProcess.c,v 1.12 2001/08/23 03:22:05 bkorb Exp $
 *  Parse and process the template data descriptions
 */

/*
 *  AutoGen copyright 1992-1999 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

#define HANDLE_FUNCTIONS
#include "autogen.h"

STATIC void openOutFile( tOutSpec* pOutSpec, tFpStack* pStk );


/*
 *  Generate all the text within a block.  The caller must
 *  know the exact bounds of the block.  "pEnd" actually
 *  must point to the first entry that is *not* to be emitted.
 */
    EXPORT void
generateBlock( tTemplate*   pT,
               tMacro*      pMac,
               tMacro*      pEnd )
{
    tSCC zFmt[] = "%-10s (%2X) in %s at line %d\n";
    int  fc;
#   define HANDLE_FUNCTIONS
#   include "functions.h"

    /*
     *  Set up the processing context for this block of macros.
     *  It is used by the Guile callback routines and the exception
     *  handling code.  It is all for user friendly diagnostics.
     */
    pCurTemplate = pT;

    while ((pMac != (tMacro*)NULL) && (pMac < pEnd)) {
        fc = pMac->funcCode;
        if (fc >= FUNC_CT)
            fc = FTYP_BOGUS;

        if (OPT_VALUE_TRACE >= TRACE_EVERYTHING) {

            fprintf( pfTrace, zFmt, apzFuncNames[ fc ], pMac->funcCode,
                     pT->pzFileName, pMac->lineNo );
            if (pMac->ozText > 0) {
                int   ct;
                char* pz = pT->pzTemplText + pMac->ozText;
                fputs( "  ", pfTrace );
                for (ct=0; ct < 32; ct++) {
                    char ch = *(pz++);
                    if (ch == '\0')
                        break;
                    if (ch == '\n')
                        break;
                    putc( ch, pfTrace );
                }
                putc( '\n', pfTrace );
            }
        }

        pCurMacro = pMac;
        pMac = (*(apHdlrProc[ fc ]))( pT, pMac );
    }
}


    EXPORT void
processTemplate( tTemplate* pTF )
{
    tFpStack fpRoot = { 0, (tFpStack*)NULL, (FILE*)NULL, (char*)NULL };

    forInfo.fi_depth = 0;

    /*
     *  IF the template file does not specify any output suffixes,
     *  THEN we will generate to standard out with the suffix set to zNone.
     *  With output going to stdout, we don't try to remove output on errors.
     */
    if (pOutSpecList == (tOutSpec*)NULL) {

        switch (setjmp( fileAbort )) {
        case SUCCESS:
        {
            tSCC zNone[]  = "* NONE *";
            pzCurSfx      = zNone;
            pCurFp        = &fpRoot;
            fpRoot.pFile  = stdout;
            fpRoot.pzOutName = "stdout";
            fpRoot.flags  = FPF_NOUNLINK | FPF_STATIC_NM;
            generateBlock( pTF, pTF->aMacros, pTF->aMacros + pTF->macroCt );
            break;
        }

        case PROBLEM:
            break;

        default:
            fputs( "Bogus return from setjmp\n", stderr );
        case FAILURE:
            exit( EXIT_FAILURE );
        }

        fclose( stdout );
        return;
    }

    for (;;) {
        tOutSpec*  pOS    = pOutSpecList;
        int        jumpCd = setjmp( fileAbort );

        /*
         *  HOW was that we got here?
         */
        if (jumpCd == SUCCESS) {
            /*
             *  Set the output file name buffer.
             *  It may get switched inside openOutFile.
             */
            openOutFile( pOS, &fpRoot );

            pzCurSfx = pOS->zSuffix;
            currDefCtx = rootDefCtx;
            generateBlock( pTF, pTF->aMacros, pTF->aMacros + pTF->macroCt );

            do  {
                closeOutput( AG_FALSE );  /* keep output */
            } while (pCurFp->pPrev != (tFpStack*)NULL);
        }

        else {
            /*
             *  We got here by a long jump.  Close/purge the open files.
             */
            do  {
                closeOutput( AG_TRUE );  /* discard output */
            } while (pCurFp->pPrev != (tFpStack*)NULL);

            /*
             *  On failure (or unknown jump type), we quit the program, too.
             */
            if (jumpCd != PROBLEM) {
                procState = PROC_STATE_ABORTING;
                exit( EXIT_FAILURE );
            }
        }

        /*
         *  Advance to the next output specification
         *  and free the old output spec.
         */
        pOutSpecList = pOS->pNext;
        AGFREE( (void*)pOS );

        if (pOutSpecList == (tOutSpec*)NULL)
            break;
    }
}


    EXPORT void
closeOutput( ag_bool purge )
{
    removeWriteAccess( fileno( pCurFp->pFile ));

    fclose( pCurFp->pFile );

    if ((pCurFp->flags & FPF_NOUNLINK) == 0) {
        /*
         *  IF we are told to purge the file OR the file is an AutoGen temp
         *  file, then get rid of the output.
         */
        if (purge || ((pCurFp->flags & FPF_UNLINK) != 0))
            unlink( pCurFp->pzOutName );

        else {
            struct utimbuf tbuf;

            tbuf.actime  = time( (time_t*)NULL );
            tbuf.modtime = outTime;

            utime( pCurFp->pzOutName, &tbuf );
        }
    }

    /*
     *  Do not deallocate statically allocated names
     */
    if ((pCurFp->flags & FPF_STATIC_NM) == 0)
        AGFREE( (void*)pCurFp->pzOutName );

    /*
     *  Do not deallocate the root entry.  It is not allocated!!
     */
    if ((pCurFp->flags & FPF_FREE) != 0) {
        tFpStack* p = pCurFp;
        pCurFp = p->pPrev;
        AGFREE( (void*)p );
    }
}


    STATIC void
openOutFile( tOutSpec* pOutSpec, tFpStack* pStk )
{
    char*  pzDefFile;

    /*
     *  Figure out what to use as the base name of the output file.
     *  If an argument is not provided, we use the base name of
     *  the definitions file.
     */
    pzDefFile = OPT_ARG( BASE_NAME );

    /*
     *  Remove any suffixes in the last file name
     */
    {
        char*  p = strrchr( pzDefFile, '/' );
        if (p == (char*)NULL)
            p = pzDefFile;

        p = strchr( p, '.' );
        if (p != (char*)NULL)
            *p = NUL;
        /*
         *  Now formulate the output file name in the buffer
         *  provided as the input argument.
         */
        pStk->pzOutName = asprintf( pOutSpec->pzFileFmt,
                                    pzDefFile, pOutSpec->zSuffix );
        if (p != (char*)NULL)
            *p = '.';
    }

    pCurFp = pStk;

    if (strcmp( pOutSpec->zSuffix, "null" ) == 0)
        goto openNull;

    /*
     *  IF we are to skip the current suffix,
     *  we will redirect the output to /dev/null and
     *  perform all the work.  There may be side effects.
     */
    if (HAVE_OPT( SKIP_SUFFIX )) {
        int     ct  = STACKCT_OPT(  SKIP_SUFFIX );
        char**  ppz = STACKLST_OPT( SKIP_SUFFIX );

        while (--ct >= 0) {
            if (strcmp( pOutSpec->zSuffix, *ppz++ ) == 0) {
                tSCC zDevNull[] = "/dev/null";

            openNull:
                /*
                 *  Make the output a no-op, but perform the operations.
                 */
                pStk->pzOutName = (char*)zDevNull;
                pStk->flags    |= FPF_STATIC_NM | FPF_NOUNLINK;
                pStk->pFile     = fopen( zDevNull, "w" FOPEN_BINARY_FLAG );
                if (pStk->pFile != (FILE*)NULL)
                    return;

                goto openError;
            }
        }
    }

    unlink( pStk->pzOutName );
    pStk->pFile = fopen( pStk->pzOutName, "w" FOPEN_BINARY_FLAG );

    if (pStk->pFile == (FILE*)NULL) {
    openError:
        fprintf( stderr, zCannot, pzProg, errno,
                 "create", pStk->pzOutName, strerror( errno ));
        LOAD_ABORT( pCurTemplate, pCurMacro, "file creation error" );
    }
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * end of tpProcess.c */
