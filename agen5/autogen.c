
/*
 *  autogen.c
 *  $Id: autogen.c,v 1.1.1.1 1999/10/14 00:33:53 bruce Exp $
 *  This is the main routine for autogen.
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

#define DEFINING
#include "autogen.h"
#include <signal.h>

STATIC sigjmp_buf  abendJumpEnv;
STATIC void signalSetup( void );


    STATIC void
inner_main( int argc, char** argv )
{
    signalSetup();
    {
        void ag_init( void );
        ag_init();
    }

    procState = PROC_STATE_OPTIONS;
    doOptions( argc, argv );
    readDefines();

    /*
     *  Load, process and unload the main template
     */
    procState = PROC_STATE_LOAD_TPL;
    {
        tTemplate* pTF = loadTemplate( pzTemplFileName );

        procState = PROC_STATE_EMITTING;
        processTemplate( pTF );

        procState = PROC_STATE_CLEANUP;
        unloadTemplate( pTF );
    }

    unloadDefs();
    procState = PROC_STATE_DONE;
    exit( EXIT_SUCCESS );
}

    int
main( int    argc,
      char** argv )
{
    if (sigsetjmp( abendJumpEnv, 0 ) != 0)
        exit( EXIT_FAILURE );

    gh_enter( argc, argv, inner_main );
    return 0; /* never reached */
}


    STATIC void
abendSignal( int sig )
{
    tSCC zErr[] = "AutoGen aborting on signal %d (%s)\n";
    tSCC zAt[]  = "processing template %s\n"
                  "            on line %d\n"
                  "       for function %s (%d)\n";
    int f = (pCurMacro->funcCode > FUNC_CT)
        ? FTYP_SELECT
        : pCurMacro->funcCode;

    fprintf( stderr, zErr, sig, strsignal( sig ));
    fprintf( stderr, zAt, pCurTemplate->pzFileName, pCurMacro->lineNo,
             apzFuncNames[ f ], pCurMacro->funcCode );
    siglongjmp( abendJumpEnv, sig );
}


    STATIC void
doneCheck( void )
{
    /*
     *  This hook is here  primarily to catch the situation
     *  where the Guile interpreter decides to call exit.
     */
    switch (procState) {
    case PROC_STATE_EMITTING:
        fprintf( stderr, "AutoGen ABENDED in template %s line %d\n",
                 pCurTemplate->pzFileName, pCurMacro->lineNo );

        /*
         *  We got here because someone called exit early.
         */
        do  {
#ifndef DEBUG
            closeOutput( AG_FALSE );
#else
            closeOutput( AG_TRUE );
#endif
        } while (pCurFp->pPrev != (tFpStack*)NULL);

    case PROC_STATE_LOAD_TPL:
    case PROC_STATE_ABORTING:
        break;

    default:
        return;
    }
    fflush( stdout );
    fflush( stderr );
    _exit( EXIT_FAILURE );
}


    STATIC void
signalSetup( void )
{
    struct sigaction  sa;
    int    sigNo = 0;

    atexit( doneCheck );

#ifdef MEMDEBUG
    atexit( &finalMemCheck );
    /* Set the snprintfv library's memory management function pointers
       to use AutoGen's debugging malloc. */
    snv_malloc  = (snv_pointer(*)SNV_PARAMS((size_t)))ag_alloc;
    snv_realloc = (snv_pointer(*)SNV_PARAMS((snv_pointer, size_t)))ag_realloc;
    snv_free    = (void(*)SNV_PARAMS((snv_pointer)))ag_free;
#endif

    sa.sa_handler = abendSignal;
    sa.sa_flags   = 0;
    sigemptyset( &sa.sa_mask );

    do  {
        sigaction( ++sigNo,  &sa, (struct sigaction*)NULL );
    } while (sigNo < NSIG);

    sa.sa_handler = SIG_IGN;
    sigaction( SIGCHLD,  &sa, (struct sigaction*)NULL );
}

/* end of autogen.c */
