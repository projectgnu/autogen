
/*
 *  autogen.c
 *  $Id$
 *
 *  Time-stamp:        "2010-02-23 12:16:02 bkorb"
 *
 *  This is the main routine for autogen.
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
 *
 * AutoGen is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AutoGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

tSCC zClientInput[] = "client-input";

#define _State_(n)  #n,
tSCC* apzStateName[] = { STATE_TABLE };
#undef _State_

static sigjmp_buf  abendJumpEnv;
static int         abendJumpSignal = 0;

typedef void (sighandler_proc_t)( int sig );
static sighandler_proc_t ignoreSignal, abendSignal;

/* = = = START-STATIC-FORWARD = = = */
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
    atexit(doneCheck);
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
        tTemplate* pTF = loadTemplate(pzTemplFileName, NULL);

        procState = PROC_STATE_EMITTING;
        processTemplate( pTF );

        procState = PROC_STATE_CLEANUP;
        cleanup( pTF );
    }

    procState = PROC_STATE_DONE;
    signalSetup( SIG_DFL, SIG_DFL );
    exit( EXIT_SUCCESS );
}

int
main(int argc, char** argv)
{
    optionSaveState(&autogenOptions);
    pfTrace = stderr;

    /*
     *  IF sigsetjmp returns with a signal number,
     *  THEN you cannot capture the value portably.  So, the jumper has
     *  stashed it for use now.
     */
    if (sigsetjmp( abendJumpEnv, 0 ) != 0)
        signalExit( abendJumpSignal );

    signalSetup(ignoreSignal, abendSignal);

#if GUILE_VERSION >= 107000
    if (getenv( "GUILE_WARN_DEPRECATED" ) == NULL)
        putenv( (char*)(void*)"GUILE_WARN_DEPRECATED=no" );
#endif

    gh_enter( argc, argv, inner_main );
    /* NOT REACHED */
    return EXIT_FAILURE;
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
        int line;
        teFuncType fnCd;
        tCC* pzFn;
        tCC* pzFl;

        if ( pCurMacro == NULL ) {
            pzFn = "pseudo-macro";
            line = 0;
            fnCd = -1;

        } else {
            teFuncType f =
                (pCurMacro->funcCode > FUNC_CT)
                    ? FTYP_SELECT : pCurMacro->funcCode;
            pzFn = apzFuncNames[ f ];
            line = pCurMacro->lineNo;
            fnCd = pCurMacro->funcCode;
        }
        pzFl = pCurTemplate->pzTplFile;
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
    switch (procState) {
    case PROC_STATE_DONE:
    case PROC_STATE_ABORTING:
        break;

    default:
        abendJumpSignal = sig;
        siglongjmp( abendJumpEnv, sig );
    }
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
#if GUILE_VERSION >= 107000
    int exit_code = EXIT_SUCCESS;
#endif

    /*
     *  There are contexts wherein this function can get registered twice.
     */
    {
        static int doneCheckDone = 0;
        if (doneCheckDone)
            return;
        doneCheckDone = 1;
    }

#ifdef SHELL_ENABLED
    ag_scm_c_eval_string_from_file_line(
        "(if (> (string-length shell-cleanup) 0)"
        " (shell shell-cleanup) )", __FILE__, __LINE__ - 1 );
    closeServer();
#endif

    fflush( stdout );
    fflush( stderr );

    if (pfTrace != stderr ) {
        if (trace_is_to_pipe) {
            int status;

            pclose( pfTrace );
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

        if (OPT_VALUE_TRACE > TRACE_NOTHING)
            scm_backtrace();

        fprintf( stderr, zErr, pCurTemplate->pzTplFile, pCurMacro->lineNo );

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

#if GUILE_VERSION >= 107000
        exit_code = EXIT_FAILURE;
#endif
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
     *  This is done for CGI mode wherein we produce an error page in case of
     *  an error, but otherwise discard stderr.
     */
    if (pzTmpStderr != NULL) {
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
            fread(  pz, (size_t)1, (size_t)pos, stderr );
            fwrite( pz, (size_t)1, (size_t)pos, stdout );
            AGFREE( pz );
        } while (0);

        fclose( stderr );
        unlink( pzTmpStderr );
        AGFREE( pzTmpStderr );
        pzTmpStderr = NULL;
    }

#if GUILE_VERSION >= 107000
    if (exit_code != EXIT_SUCCESS) {
        fflush(stderr);
        fflush(stdout);
        _exit(exit_code);
    }
#endif
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
        int l_no = (pCurMacro == NULL) ? -1 : pCurMacro->lineNo;
        fprintf( stderr, "Error in template %s, line %d\n\t",
                 pCurTemplate->pzTplFile, l_no );
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

            /*
             *  Signals we choose to leave alone.
             */
#ifdef SIGTSTP
        case SIGTSTP:
#endif
            continue;

#if defined(DEBUG_ENABLED)
        case SIGBUS:
        case SIGSEGV:
            /*
             *  While debugging AutoGen, we want seg faults to happen and
             *  trigger core dumps.  Make sure this happens.
             */
            sa.sa_handler = SIG_DFL;
            break;
#endif

            /*
             *  Signals to ignore with SIG_IGN.
             */
        case 0: /* cannot happen, but the following might not be defined */
#ifdef SIGWINCH
        case SIGWINCH:
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
  Choke Me.
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

LOCAL void *
ao_malloc (size_t sz)
{
    void * res = malloc(sz);
    if (res == NULL) {
        fprintf(stderr, "malloc of %zd bytes failed\n", sz);
        exit( EXIT_FAILURE );
    }
    return res;
}


LOCAL void *
ao_realloc (void *p, size_t sz)
{
    void * res = realloc(p, sz);
    if (res == NULL) {
        fprintf(stderr, "realloc of %zd bytes at 0x%p failed\n", sz, p);
        exit( EXIT_FAILURE );
    }
    return res;
}


LOCAL void
ao_free (void *p)
{
    if (p != NULL)
        free(p);
}


LOCAL char *
ao_strdup (char const * str)
{
    char * res = strdup(str);
    if (res == NULL) {
        fprintf(stderr, "strdup of %d byte string failed\n", (int)strlen(str));
        exit( EXIT_FAILURE );
    }
    return res;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/autogen.c */
