
/*
 *  autogen.c
 *  $Id: autogen.c,v 3.4 2002/01/13 08:04:32 bkorb Exp $
 *  This is the main routine for autogen.
 */

/*
 *  AutoGen copyright 1992-2002 Bruce Korb
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
#include <assert.h>

tSCC zSchemeInit[] =
"(add-hook! before-error-hook error-source-line)\n"
"(define autogen-version \"" AUTOGEN_VERSION "\")";

STATIC sigjmp_buf  abendJumpEnv;

/*
 *  Local procedures
 */
STATIC void signalSetup( void );
STATIC void abendSignal( int sig );
STATIC void doneCheck( void );
STATIC void inner_main( int argc, char** argv );


STATIC void
inner_main( int argc, char** argv )
{
    void ag_init( void );
    tTemplate* pTF;

    /*
     *  Initialize all but the processing Scheme functions
     */
    ag_init();
    procState = PROC_STATE_OPTIONS;
    doOptions( argc, argv );

    procState = PROC_STATE_LOAD_DEFS;
    readDefines();

    /*
     *  Activate the AutoGen specific Scheme functions.
     *  Load, process and unload the main template
     */
    procState = PROC_STATE_LOAD_TPL;

    /*
     *  Initialize the processing Scheme functions
     */
    ag_init();
    gh_eval_str( (char*)zSchemeInit );
    pTF = loadTemplate( pzTemplFileName );

    procState = PROC_STATE_EMITTING;
    processTemplate( pTF );

    procState = PROC_STATE_CLEANUP;
    unloadTemplate( pTF );
    unloadDefs();

    procState = PROC_STATE_DONE;
    exit( EXIT_SUCCESS );
}

    int
main( int    argc,
      char** argv )
{
    pfTrace = stderr;
    {
        int signo = sigsetjmp( abendJumpEnv, 0 );
        if (signo != 0)
            exit( signo + 128 );
    }

    signalSetup();

    gh_enter( argc, argv, inner_main );
    /* NOT REACHED */
    return 0;
}


STATIC void
doneCheck( void )
{
    fflush( stdout );
    fflush( stderr );

    if (pfTrace != stderr ) {
        if (* OPT_ARG( TRACE_OUT ) == '|') {
            int status;

            pclose( pfTrace );
            closeServer();
            while (wait( &status ) > 0)  ;
        }
        else fclose( pfTrace );
    }

    /*
     *  This hook is here  primarily to catch the situation
     *  where the Guile interpreter decides to call exit.
     */
    switch (procState) {
    case PROC_STATE_EMITTING:
    case PROC_STATE_INCLUDING:
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
        break;

    case PROC_STATE_INIT:
    case PROC_STATE_OPTIONS:
    case PROC_STATE_DONE:
        return;
    }

    _exit( EXIT_FAILURE );
}


#ifdef DEBUG
EXTERN void
ag_abend_at( tCC* pzMsg, tCC* pzFile, int line )
#else
EXTERN void
ag_abend( tCC* pzMsg )
#endif
{
    if (*pzOopsPrefix != NUL)
        fputs( pzOopsPrefix, stderr );

#ifdef DEBUG
    fprintf( stderr, "Giving up in %s line %d\n", pzFile, line );
#endif

    if (procState >= PROC_STATE_LIB_LOAD)
        fprintf( stderr, "Error in template %s, line %d\n\t",
                 pCurTemplate->pzFileName, pCurMacro->lineNo );
    fputs( pzMsg, stderr );
    fputc( '\n', stderr );

    {
        teProcState oldState = procState;
        procState = PROC_STATE_ABORTING;

        switch (oldState) {
        case PROC_STATE_EMITTING:
        case PROC_STATE_INCLUDING:
        case PROC_STATE_CLEANUP:
            longjmp( fileAbort, FAILURE );
            /* NOTREACHED */
        default:
            exit(EXIT_FAILURE);
        }
    }
}


STATIC void
ignoreSignal( int sig )
{
#ifdef DEBUG
    fprintf( pfTrace, "Ignored signal %d (%s)\n", sig, strsignal( sig ));
#endif
    return;
}


STATIC void
abendSignal( int sig )
{
    tSCC zErr[] = "AutoGen aborting on signal %d (%s)\n";
    tSCC zAt[]  = "processing template %s\n"
                  "            on line %d\n"
                  "       for function %s (%d)\n";

    fprintf( stderr, zErr, sig, strsignal( sig ));
    fflush( stderr );
    fflush( stdout );
    if (pfTrace != stderr )
        fflush( pfTrace );

    /*
     *  IF there is a current template, then we should report where we are
     *  so that the template writer knows where to look for their problem.
     */
    if (pCurTemplate != (tTemplate*)NULL) {
        int f;

        if ( pCurMacro == NULL ) {
            fputs( "NULL pCurMacro in abendSignal()\n", stderr );
            _exit( 128 + SIGABRT );
        }

        f = (pCurMacro->funcCode > FUNC_CT)
                ? FTYP_SELECT : pCurMacro->funcCode;
        fprintf( stderr, zAt, pCurTemplate->pzFileName, pCurMacro->lineNo,
                 apzFuncNames[ f ], pCurMacro->funcCode );
    }

    procState = PROC_STATE_ABORTING;
    siglongjmp( abendJumpEnv, sig );
}


STATIC void
signalSetup( void )
{
    struct sigaction  sa;
    int    sigNo = 1;

    atexit( doneCheck );

#ifdef MEMDEBUG
    atexit( &finalMemCheck );
    /* Set the snprintfv library's memory management function pointers
       to use AutoGen's debugging malloc. */
    snv_malloc  = (snv_pointer(*)SNV_PARAMS((size_t)))ag_alloc;
    snv_realloc = (snv_pointer(*)SNV_PARAMS((snv_pointer, size_t)))ag_realloc;
    snv_free    = (void(*)SNV_PARAMS((snv_pointer)))ag_free;
#endif

    sa.sa_flags   = 0;
    sigemptyset( &sa.sa_mask );

    do  {
        switch (sigNo) {
            /*
             *  Signal handling for SIGCHLD needs to be ignored.  Do *NOT* use
             *  SIG_IGN to do this.  That gets inherited by programs that need
             *  to be able to use wait(2) calls.  At the same time, we don't
             *  want our children to become zombies.  We may run out of zombie
             *  space.  Therefore, set the handler to an empty procedure.
             *  POSIX oversight.  Fixed for next POSIX rev.
             */
        case SIGCHLD:
            sa.sa_handler = ignoreSignal;
            break;

#ifdef SIGWINCH
        case SIGWINCH:
#endif
        case SIGSTOP:  /* suspended */
#ifdef SIGTSTP
        case SIGTSTP:  /* suspended */
#endif
            continue;

        default:
            sa.sa_handler = abendSignal;
        }
        sigaction( sigNo,  &sa, (struct sigaction*)NULL );
    } while (++sigNo < NSIG);

    sa.sa_handler = ignoreSignal;
    sigaction( SIGCHLD,  &sa, (struct sigaction*)NULL );
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autogen.c */
