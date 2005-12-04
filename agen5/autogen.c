
/*
 *  autogen.c
 *  $Id: autogen.c,v 4.11 2005/12/04 22:18:40 bkorb Exp $
 *  This is the main routine for autogen.
 */

/*
 *  AutoGen copyright 1992-2005 Bruce Korb
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
 *             51 Franklin Street, Fifth Floor,
 *             Boston, MA  02110-1301, USA.
 */

tSCC zClientInput[] = "client-input";

#define _State_(n)  #n,
tSCC* apzStateName[] = { STATE_TABLE };
#undef _State_

static sigjmp_buf  abendJumpEnv;
typedef void (sighandler_proc_t)( int sig );
static sighandler_proc_t ignoreSignal, abendSignal;

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static void
inner_main( int argc, char** argv );

static void
signalExit( int sig );

static void
abendSignal( int sig );

static void
ignoreSignal( int sig );

static void
doneCheck( void );

static void
signalSetup( sighandler_proc_t* chldHandler,
             sighandler_proc_t* dfltHandler );
/* = = = END-STATIC-FORWARD = = = */

static void
inner_main( int argc, char** argv )
{
    initialize( argc, argv );

    procState = PROC_STATE_LOAD_DEFS;
    ag_scmStrings_init();
    readDefines();

    /*
     *  Activate the AutoGen specific Scheme functions.
     *  Then load, process and unload the main template
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
    ag_scmStrings_deinit();

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

    signalSetup( ignoreSignal, abendSignal );

    gh_enter( argc, argv, inner_main );
    /* NOT REACHED */
    return 0;
}

static void
signalExit( int sig )
{
    tSCC zErr[] = "AutoGen aborting on signal %d (%s) in state %s\n";

    if (*pzOopsPrefix != NUL) {
        fputs( pzOopsPrefix, stderr );
        pzOopsPrefix = zNil;
    }

    fprintf( stderr, zErr, sig, strsignal( sig ),
             ((unsigned)procState <= PROC_STATE_DONE)
             ? apzStateName[ procState ] : "** BOGUS **" );

    fflush( stderr );
    fflush( stdout );
    if (pfTrace != stderr )
        fflush( pfTrace );

    if (procState == PROC_STATE_ABORTING)
        _exit( sig + 128 );

    procState = PROC_STATE_ABORTING;

    /*
     *  IF there is a current template, then we should report where we are
     *  so that the template writer knows where to look for their problem.
     */
    if (pCurTemplate != NULL) {
        tSCC zAt[]  = "processing template %s\n"
                      "            on line %d\n"
                      "       for function %s (%d)\n";
        int line, fnCd;
        tCC* pzFn;
        tCC* pzFl;

        if ( pCurMacro == NULL ) {
            pzFn = "pseudo-macro";
            line = 0;
            fnCd = -1;

        } else {
            int f = (pCurMacro->funcCode > FUNC_CT)
                    ? FTYP_SELECT : pCurMacro->funcCode;
            pzFn = apzFuncNames[ f ];
            line = pCurMacro->lineNo;
            fnCd = pCurMacro->funcCode;
        }
        pzFl = pCurTemplate->pzFileName;
        if (pzFl == NULL)
            pzFl = "NULL file name";
        fprintf( stderr, zAt, pzFl, line, pzFn, fnCd );
    }

    signalSetup( SIG_DFL, SIG_DFL );
    abort();
}


/*
 *  abendSignal catches signals we abend on.  The "siglongjmp" goes back
 *  to the real "main()" procedure and it will call "signalExit()", above.
 */
static void
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
static void
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
static void
doneCheck( void )
{
    tSCC zErr[] =
        "Scheme evaluation error.  AutoGen ABEND-ing in template\n"
        "\t%s on line %d\n";

#ifdef SHELL_ENABLED
    ag_scm_c_eval_string_from_file_line(
        "(if (> (string-length shell-cleanup) 0)"
        " (shell shell-cleanup) )", __FILE__, __LINE__ );
#endif

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

    switch (procState) {
    case PROC_STATE_EMITTING:
    case PROC_STATE_INCLUDING:
        /*
         *  A library (viz., Guile) procedure has called exit(3C).
         *  The AutoGen abort paths all set procState to PROC_STATE_ABORTING.
         */
        if (*pzOopsPrefix != NUL) {
            /*
             *  Emit the CGI page header for an error message.  We will rewind
             *  stderr and write the contents to stdout momentarily.
             */
            fputs( pzOopsPrefix, stdout );
            pzOopsPrefix = zNil;
        }

        scm_backtrace();
        fprintf( stderr, zErr, pCurTemplate->pzFileName, pCurMacro->lineNo );

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
        break; /* continue failure exit */

    default:
        fprintf( stderr, "ABEND-ing in %s state\n", apzStateName[procState] );
        /* FALLTHROUGH */

    case PROC_STATE_ABORTING:
        if (*pzOopsPrefix != NUL) {
            /*
             *  Emit the CGI page header for an error message.  We will rewind
             *  stderr and write the contents to stdout momentarily.
             */
            fputs( pzOopsPrefix, stdout );
            pzOopsPrefix = zNil;
        }
        break; /* continue failure exit */

    case PROC_STATE_OPTIONS:
        /* Exiting in option processing state is verbose enough */
    case PROC_STATE_DONE:
        break; /* continue normal exit */
    }

    if (pzLastScheme != NULL) {
        tSCC zGuileFail[] =
            "Failing Guile command:  = = = = =\n\n%s\n\n"
            "=================================\n";
        fprintf( stderr, zGuileFail, pzLastScheme );
    }

    /*
     *  IF we diverted stderr, then now is the time to copy the text to stdout.
     */
    if (pzTmpStderr == NULL)
        return;

    do {
        long pos = ftell( stderr );
        char* pz;

        /*
         *  Don't bother with the overhead if there is no work to do.
         */
        if (pos <= 0)
            break;
        pz = AGALOC( pos, "stderr redirected text" );
        rewind( stderr );
        fread( pz, 1, pos, stderr );
        fwrite( pz, 1, pos, stdout );
        AGFREE( pz );
    } while (0);

    fclose( stderr );
    unlink( pzTmpStderr );
    AGFREE( pzTmpStderr );
    pzTmpStderr = NULL;
}


LOCAL void
ag_abend_at( tCC* pzMsg
#ifdef DEBUG_ENABLED
    , tCC* pzFile, int line
#endif
    )
{
    if (*pzOopsPrefix != NUL) {
        fputs( pzOopsPrefix, stderr );
        pzOopsPrefix = zNil;
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
            /* NOTREACHED */
        }
    }
}


static void
signalSetup( sighandler_proc_t* chldHandler,
             sighandler_proc_t* dfltHandler )
{
    struct sigaction  sa;
    int    sigNo  = 1;
#ifdef SIGRTMIN
    const int maxSig = SIGRTMIN-1;
#else
    const int maxSig = NSIG;
#endif
    atexit( doneCheck );

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
            sa.sa_handler = chldHandler;
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

#ifdef DAEMON_ENABLED
# error DAEMON-ization of AutoGen is not ready for prime time
        case SIGHUP:
            if (HAVE_OPT( DAEMON )) {
                sa.sa_handler = handleSighup;
                break;
            }
            /* FALLTHROUGH */
#endif

        default:
            sa.sa_handler = dfltHandler;
        }
        sigaction( sigNo,  &sa, NULL );
    } while (++sigNo < maxSig);
}

#ifndef HAVE_STRFTIME
#  include <compat/strftime.c>
#endif

#ifndef HAVE_STRSIGNAL
#  include <compat/strsignal.c>
#endif

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/autogen.c */
