
/*
 *  autogen.c
 *  $Id: autogen.c,v 3.15 2002/06/01 17:56:14 bkorb Exp $
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
"(define header-file \"\")  (define header-guard \"\")\n"
"(define (make-header-guard hdr-pfx)\n"
"   (begin\n"
"      (set! header-file  (out-name))\n"
"      (set! header-guard (string-upcase! (string->c-name! (string-append\n"
"             (if (string? hdr-pfx) hdr-pfx \"HEADER\")\n"
"             \"_\" header-file \"_GUARD\" ))))\n"
"      (sprintf \"#ifndef %1$s\\n#define %1$s\" header-guard)\n"
")  )\n"
"(define autogen-version \"" AUTOGEN_VERSION "\")\n"
"(define-macro (defined-as predicate symbol)\n"
"  `(and (defined? ',symbol) (,predicate ,symbol)))";

STATIC sigjmp_buf  abendJumpEnv;

/*
 *  Local procedures
 */
STATIC void signalSetup( void );
STATIC void abendSignal( int sig );
STATIC void doneCheck( void );
STATIC void inner_main( int argc, char** argv );
STATIC void signalExit( int sig );

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
            signalExit( signo );
    }

    signalSetup();

    gh_enter( argc, argv, inner_main );
    /* NOT REACHED */
    return 0;
}


STATIC void
signalExit( int sig )
{
    if (*pzOopsPrefix != NUL) {
        fputs( pzOopsPrefix, stderr );
        pzOopsPrefix = "";
    }

    {
        tSCC zErr[] = "AutoGen aborting on signal %d (%s) in state %s\n";
#       define _State_(n)  #n,
        tSCC* apzStateName[] = { STATE_TABLE };
#       undef _State_

        fprintf( stderr, zErr, sig, strsignal( sig ),
                 ((unsigned)procState <= PROC_STATE_DONE)
                 ? apzStateName[ procState ] : "** BOGUS **" );
    }

    fflush( stderr );
    fflush( stdout );
    if (pfTrace != stderr )
        fflush( pfTrace );

    /*
     *  IF there is a current template, then we should report where we are
     *  so that the template writer knows where to look for their problem.
     */
    if (pCurTemplate != NULL) {
        tSCC zAt[]  = "processing template %s\n"
                      "            on line %d\n"
                      "       for function %s (%d)\n";
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
    exit( sig + 128 );
}


/*
 *  abendSignal catches signals we abend on.  The "siglongjmp" goes back
 *  to the real "main()" procedure and it will call "signalExit()", above.
 */
STATIC void
abendSignal( int sig )
{
    siglongjmp( abendJumpEnv, sig );
}


/*
 *  ignoreSignal is the handler for SIGCHLD.  If we set it to default,
 *  it will kill us.  If we set it to ignore, it will be inherited.
 *  Therefore, always in all programs set it to call a procedure.
 *  The "wait(3)" call will do magical things, but will not override SIGIGN.
 */
STATIC void
ignoreSignal( int sig )
{
#ifdef DEBUG_ENABLED
    fprintf( pfTrace, "Ignored signal %d (%s)\n", sig, strsignal( sig ));
#endif
    return;
}


/*
 *  This is called during normal exit processing.
 */
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
        if (*pzOopsPrefix != NUL) {
            fputs( pzOopsPrefix, stderr );
            pzOopsPrefix = "";
        }

        fprintf( stderr, "AutoGen ABENDED in template %s line %d\n",
                 pCurTemplate->pzFileName, pCurMacro->lineNo );

        /*
         *  We got here because someone called exit early.
         */
        do  {
#ifndef DEBUG_ENABLED
            closeOutput( AG_FALSE );
#else
            closeOutput( AG_TRUE );
#endif
        } while (pCurFp->pPrev != NULL);
        /* FALLTHROUGH */

    default:
        _exit( EXIT_FAILURE ); /* exit immediately -- no more processing */

    case PROC_STATE_INIT:
    case PROC_STATE_OPTIONS:
    case PROC_STATE_DONE:
        break; /* continue normal exit */
    }
}


#ifdef DEBUG_ENABLED
EXPORT void
ag_abend_at( tCC* pzMsg, tCC* pzFile, int line )
#else
EXPORT void
ag_abend( tCC* pzMsg )
#endif
{
    if (*pzOopsPrefix != NUL) {
        fputs( pzOopsPrefix, stderr );
        pzOopsPrefix = "";
    }

#ifdef DEBUG_ENABLED
    fprintf( stderr, "Giving up in %s line %d\n", pzFile, line );
#endif

    if ((procState >= PROC_STATE_LIB_LOAD) && (pCurTemplate != NULL)) {
        int line = (pCurMacro == NULL) ? -1 : pCurMacro->lineNo;
        fprintf( stderr, "Error in template %s, line %d\n\t",
                 pCurTemplate->pzFileName, line );
    }
    fputs( pzMsg, stderr );
    pzMsg += strlen( pzMsg );
    if (pzMsg[-1] != '\n')
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
             *  POSIX oversight.  Allowed to be fixed for next POSIX rev, tho
             *  it is "optional" to reset SIGCHLD on exec(2).
             */
#ifndef SIGCHLD
#  define SIGCHLD SIGCLD
#endif
        case SIGCHLD:
            sa.sa_handler = ignoreSignal;
            break;

            /*
             *  Signals we must leave alone.
             */
        case SIGKILL:
        case SIGSTOP:
            continue;

            /*
             *  Signals to ignore with SIG_IGN.
             */
        case 0: /* cannot happen, but the following might not be defined */
#ifdef SIGWINCH
        case SIGWINCH:
#endif
#ifdef SIGTSTP
        case SIGTSTP:  /* suspended  */
#endif
#ifdef SIGTTIN
        case SIGTTIN:  /* tty input  */
#endif
#ifdef SIGTTOU
        case SIGTTOU:  /* tty output */
#endif
            sa.sa_handler = SIG_IGN;
            break;

        default:
            sa.sa_handler = abendSignal;
        }
        sigaction( sigNo,  &sa, NULL );
    } while (++sigNo < NSIG);
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * tab-width: 4
 * End:
 * end of autogen.c */
