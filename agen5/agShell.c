/*
 *  agShell
 *  $Id: agShell.c,v 4.8 2005/10/22 21:57:43 bkorb Exp $
 *  Manage a server shell process
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

#ifndef SHELL_ENABLED
 void closeServer( void ) { }

 int chainOpen( int stdinFd, tCC** ppArgs, pid_t* pChild ) { return -1; }

 pid_t openServer( tFdPair* pPair, tCC** ppArgs ) { return NOPROCESS; }

 pid_t openServerFP( tpfPair* pfPair, tCC** ppArgs ) { return NOPROCESS; }

 char* runShell( const char* pzCmd ) {
     char* pz;
     AGDUPSTR( pz, pzCmd, "dummy shell command" );
     return pz;
 }
#else

/*
 *  Dual pipe opening of a child process
 */
static tpfPair serverPair = { NULL, NULL };
static pid_t   serverId   = NULLPROCESS;
static tpChar  pCurDir    = NULL;
static ag_bool errClose   = AG_FALSE;
static int     logCount   = 0;
static tCC     logIntroFmt[] = "\n\n* * * * LOG ENTRY %d * * * *\n";

tSCC   zCmdFmt[]   = "cd %s\n%s\n\necho\necho %s - %d\n";

const char* pzLastCmd = NULL;

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static void
sigHandler( int signo );

static void
serverSetup( void );

static char*
loadData( void );
/* = = = END-STATIC-FORWARD = = = */


LOCAL void
closeServer( void )
{
    if (serverId == NULLPROCESS)
        return;

    (void)kill( serverId, SIGKILL );
    serverId = NULLPROCESS;

    /*
     *  This guard should not be necessary.  However, sometimes someone
     *  holds an allocation pthread lock when a seg fault occurrs.  fclose
     *  needs that lock, so we hang waiting for it.  Oops.  So, when we
     *  are aborting, we just let the OS close these file descriptors.
     */
    if (procState != PROC_STATE_ABORTING) {
        (void)fclose( serverPair.pfRead );
        /*
         *  This is _completely_ wrong, but sometimes there are data left
         *  hanging about that gets sucked up by the _next_ server shell
         *  process.  That should never, ever be in any way possible, but
         *  it is the only explanation for a second server shell picking up
         *  the initialization string twice.  It must be a broken timing
         *  issue in the Linux stdio code.  I have no other explanation.
         */
        fflush( serverPair.pfWrite );
        (void)fclose( serverPair.pfWrite );
    }

    serverPair.pfRead = serverPair.pfWrite = NULL;
}


static void
sigHandler( int signo )
{
    fprintf( pfTrace, "Closing server:  %s signal (%d) received\n",
             strsignal( signo ), signo );
    errClose = AG_TRUE;

    (void)fputs( "\nLast command issued:\n", pfTrace );
    {
        const char* pz = (pzLastCmd == NULL)
            ? "?? unknown ??\n" : pzLastCmd;
        fprintf( pfTrace, zCmdFmt, pCurDir, pz, zShDone, logCount );
    }
    pzLastCmd = NULL;
    closeServer();
}


static void
serverSetup( void )
{
    struct sigaction  saNew;
    struct sigaction  saSave1;
    struct sigaction  saSave2;

    {
        static int doneOnce = 0;
        if (doneOnce++ == 0) {
            char* p = malloc( MAXPATHLEN );
            if (p == NULL)
                AG_ABEND( "cannot allocate path name" );

            (void)atexit( &closeServer );
            pCurDir = (tpChar)getcwd( p, MAXPATHLEN );

            if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
                fputs( "\nServer First Start\n", pfTrace );
        }
        else {
            if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
                fputs( "\nServer Restart\n", pfTrace );
        }
    }

    saNew.sa_handler = sigHandler;
    saNew.sa_flags   = 0;
    (void)sigemptyset( &saNew.sa_mask );
    (void)sigaction( SIGPIPE, &saNew, &saSave1 );
    (void)sigaction( SIGALRM, &saNew, &saSave2 );

    errClose = AG_FALSE;

    {
        static char* pzPid = NULL;
        char* pz;
        pzLastCmd = zShellInit;
        if (pzPid == NULL)
            pzPid = zShellInit + strlen(zShellInit);
        sprintf( pzPid, "%d\n", getpid() );
        fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzLastCmd,
                 zShDone, ++logCount );

        if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
            fprintf( pfTrace, logIntroFmt, logCount );
            fprintf( pfTrace, zCmdFmt, pCurDir, pzLastCmd, zShDone, logCount );
        }

        (void)fflush( serverPair.pfWrite );
        pz = loadData();
        if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
            fputs( "(result discarded)\n", pfTrace );
        AGFREE( (void*)pz );
    }

#if defined( DEBUG_ENABLED )
    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        tSCC zSetup[] = "set -x\n"
                        "exec 2> /dev/tty\n"
                        "trap\n"
                        "echo server setup done\n";
        char* pz;

        fputs( "Server traps set\n", pfTrace );
        pzLastCmd = zSetup;
        fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzLastCmd,
                 zShDone, ++logCount );
        if (pfTrace != stderr) {
            fprintf( pfTrace, logIntroFmt, logCount );
            fprintf( pfTrace, zCmdFmt, pCurDir, pzLastCmd, zShDone, logCount );
        }

        (void)fflush( serverPair.pfWrite );
        pz = loadData();
        if (pfTrace != stderr)
            fputs( "(result discarded)\n", pfTrace );
        fprintf( pfTrace, "Trap state:\n%s\n", pz );
        AGFREE( (void*)pz );
    }
#endif
    pzLastCmd = NULL;
}


/*
 *  chainOpen
 *
 *  Given an FD for an inferior process to use as stdin,
 *  start that process and return a NEW FD that that process
 *  will use for its stdout.  Requires the argument vector
 *  for the new process and, optionally, a pointer to a place
 *  to store the child's process id.
 */
LOCAL int
chainOpen( int       stdinFd,
           tCC**     ppArgs,
           pid_t*    pChild  )
{
    tFdPair   stdoutPair = { -1, -1 };
    pid_t     chId;
    tCC*      pzShell;

    /*
     *  If we did not get an arg list, use the default
     */
    if (ppArgs == NULL)
        ppArgs = serverArgs;

    /*
     *  If the arg list does not have a program,
     *  assume the zShellProg from the environment, or, failing
     *  that, then sh.  Set argv[0] to whatever we decided on.
     */
    if (pzShell = *ppArgs,
       (pzShell == NULL) || (*pzShell == NUL)) {

        if (pzShellProgram != NULL)
            pzShell = pzShellProgram;

        else {
            pzShell = getenv( zShellEnv );
            if (pzShell == NULL)
                pzShell = "sh";
            pzShellProgram = pzShell;
        }

        /*
         *  Disallow 'csh' and 'tcsh', or any other *csh variation.
         *  They invariably lead to problems.
         */
        {
            size_t l = strlen( pzShell );
            if (  (l > 2)
               && (strcmp( pzShell + l - 3, "csh" ) == 0)  )  {
                tSCC zBadShell[] =
                    "Your shell is '%s'.  AutoGen will attempt to use 'sh'\n";

                fprintf( stdout, zBadShell, pzShell );
                pzShell = "sh";
            }
        }

        *ppArgs = pzShell;
    }

    /*
     *  Create a pipe it will be the child process' stdout,
     *  and the parent will read from it.
     */
    if (pipe( (int*)&stdoutPair ) < 0) {
        if (pChild != NULL)
            *pChild = NOPROCESS;
        return -1;
    }

#if defined( DEBUG_ENABLED )
    if (HAVE_OPT( SHOW_SHELL )) {
        tSCC   zPath[] = "PATH";
        tCC*   pzPath  = getenv( zPath );

        if (pzPath == NULL)
            pzPath = pzShell;

        else {
            pzPath = pathfind( pzPath, pzShell, "rxs" );
            if (pzPath == NULL)
                pzPath = pzShell;
        }

        fprintf( pfTrace, "Starting shell `%s'\n", pzPath );
    }
#endif
    fflush( stdout );
    fflush( stderr );
    fflush( pfTrace );

    /*
     *  Call fork() and see which process we become
     */
    chId = fork();
    switch (chId) {
    case NOPROCESS:    /* parent - error in call */
        close( stdinFd );
        close( stdoutPair.readFd );
        close( stdoutPair.writeFd );
        if (pChild != NULL)
            *pChild = NOPROCESS;
        return -1;

    default:           /* parent - return opposite FD's */
        if (pChild != NULL)
            *pChild = chId;

        close( stdinFd );
        close( stdoutPair.writeFd );
        if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
            fprintf( pfTrace, "Server shell is pid %d\n", chId );

        fflush( pfTrace );
        return stdoutPair.readFd;

    case NULLPROCESS:  /* child - continue processing */
        break;
    }

    /*
     *  Close the pipe end handed back to the parent process,
     *  plus stdin and stdout.
     */
    close( stdoutPair.readFd );
    close( STDIN_FILENO );
    close( STDOUT_FILENO );

    /*
     *  Set stdin/out to the fd passed in and the write end of our new pipe.
     */
    fcntl( stdoutPair.writeFd, F_DUPFD, STDOUT_FILENO );
    fcntl( stdinFd,            F_DUPFD, STDIN_FILENO );

    /*
     *  set stderr to our trace file (if not stderr).
     */
    if (pfTrace != stderr) {
        close( STDERR_FILENO );
        fcntl( fileno(pfTrace), F_DUPFD, STDERR_FILENO );
    }

    /*
     *  Make the output file unbuffered only.
     *  We do not want to wait for shell output buffering.
     */
    setvbuf( stdout, NULL, _IONBF, 0 );

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf( pfTrace, "Server shell %s starts\n", pzShell );

        fflush( pfTrace );
    }

    execvp( (char*)pzShell, (char**)ppArgs );
    AG_ABEND( aprf( "Could not execvp( '%s', ... ):  %d - %s\n",
                    pzShell, errno, strerror( errno )));
    /* NOTREACHED */
    return -1;
}


/*
 *  openServer
 *
 *  Given a pointer to an argument vector, start a process and
 *  place its stdin and stdout file descriptors into an fd pair
 *  structure.  The "writeFd" connects to the inferior process
 *  stdin, and the "readFd" connects to its stdout.  The calling
 *  process should write to "writeFd" and read from "readFd".
 *  The return value is the process id of the created process.
 */
LOCAL pid_t
openServer( tFdPair* pPair, tCC** ppArgs )
{
    pid_t     chId;

    /*
     *  Create a bi-directional pipe.  Writes on 0 arrive on 1
     *  and vice versa, so the parent and child processes will
     *  read and write to opposite FD's.
     */
    if (pipe( (int*)pPair ) < 0)
        return NOPROCESS;

    pPair->readFd = chainOpen( pPair->readFd, ppArgs, &chId );
    if (chId == NOPROCESS)
        close( pPair->writeFd );

    return chId;
}


/*
 *  openServerFP
 *
 *  Identical to "openServer()", except that the "fd"'s are "fdopen(3)"-ed
 *  into file pointers instead.
 */
LOCAL pid_t
openServerFP( tpfPair* pfPair, tCC** ppArgs )
{
    tFdPair   fdPair;
    pid_t     chId = openServer( &fdPair, ppArgs );

    if (chId == NOPROCESS)
        return chId;

    pfPair->pfRead  = fdopen( fdPair.readFd,  "r" FOPEN_BINARY_FLAG );
    pfPair->pfWrite = fdopen( fdPair.writeFd, "w" FOPEN_BINARY_FLAG );
    return chId;
}
#endif /* SHELL_ENABLED */


/*
 *  loadData
 *
 *  Read data from a file pointer (a pipe to a process in this context)
 *  until we either get EOF or we get a marker line back.
 *  The read data are stored in a malloc-ed string that is truncated
 *  to size at the end.  Input is assumed to be an ASCII string.
 */
static char*
loadData( void )
{
    char*   pzText;
    size_t  textSize;
    char*   pzScan;
    char    zLine[ 1024 ];
    int     retryCt = 0;

    textSize = 4096;
    pzScan   = \
    pzText   = AGALOC( textSize, "Text Block" );

    *pzText  = NUL;

    for (;;) {
        size_t  usedCt;

        /*
         *  Set a timeout so we do not wait forever.  Sometimes we don't wait
         *  at all and we should.  Retry in those cases (but not on EOF).
         */
        alarm( OPT_VALUE_TIMEOUT );
        if (fgets( zLine, sizeof( zLine ), serverPair.pfRead ) == NULL) {
            alarm( 0 );

            if ((OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) || (retryCt++ > 0))
                fprintf( pfTrace, "fs err %d (%s) reading from server shell\n",
                         errno, strerror(errno) );

            if (feof( serverPair.pfRead ) || (retryCt > 32))
                break;

            continue;  /* no data - retry */
        }
        alarm( 0 );

        /*
         *  Check for magic character sequence indicating 'DONE'
         */
        if (strncmp( zLine, zShDone, STRSIZE( zShDone )) == 0)
            break;

        strcpy( pzScan, zLine );

        /*
         *  Stop now if we are at EOF
         */
        if (feof( serverPair.pfRead )) {
            fputs( "feof on data load\n", pfTrace );
            break;
        }

        pzScan += strlen( zLine );
        usedCt = (size_t)(pzScan - pzText);

        /*
         *  IF we are running low on space in our line buffer,
         *  THEN expand the line
         */
        if (textSize - usedCt < sizeof(zLine)) {

            size_t off = (size_t)(pzScan - pzText);
            void*  p;
            textSize += 4096;
            p = AGREALOC( (void*)pzText, textSize, "expanding text" );
            if (p == NULL) {
                tSCC zRTB[] = "Realloc Text Block";
                AG_ABEND( aprf( zAllocWhat, textSize, zRTB ));
            }

            pzText   = (char*)p;
            pzScan = pzText + off;
        }
    }

    /*
     *  Trim off all trailing white space and shorten the buffer
     *  to the size actually used.
     */
    while ((pzScan > pzText) && isspace( pzScan[-1] )) pzScan--;
    textSize = (pzScan - pzText) + 1;

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf( pfTrace, "\n= = = RESULT %d bytes:\n%s%s\n"
                 "= = = = = = = = = = = = = = =\n",
                 textSize, pzText, zLine );
    }

    *pzScan  = NUL;
    return AGREALOC( (void*)pzText, textSize, "resizing text output" );
}

#ifdef SHELL_ENABLED
/*
 *  Run a semi-permanent server shell.  The program will be the
 *  one named by the environment variable $SHELL, or default to "sh".
 *  If one of the commands we send to it takes too long or it dies,
 *  we will shoot it and restart one later.
 */
LOCAL char*
runShell( const char*  pzCmd )
{
    tSCC zCmdFail[] = "CLOSING SHELL SERVER - command failure:\n\t%s\n";

    /*
     *  IF the shell server process is not running yet,
     *  THEN try to start it.
     */
    if (serverId == NULLPROCESS) {
        serverId = openServerFP( &serverPair, serverArgs );
        if (serverId > 0)
            serverSetup();
    }

    /*
     *  IF it is still not running,
     *  THEN return the nil string.
     */
    if (serverId <= 0) {
        char* pz = (char*)AGALOC( 1, "Text Block" );

        *pz = NUL;
        return pz;
    }

    /*
     *  Make sure the process will pay attention to us,
     *  send the supplied command, and then
     *  have it output a special marker that we can find.
     */
    pzLastCmd = pzCmd;
    fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzCmd, zShDone, ++logCount );

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf( pfTrace, logIntroFmt, logCount );
        fprintf( pfTrace, zCmdFmt, pCurDir, pzCmd, zShDone, logCount );
    }

    fflush( serverPair.pfWrite );

    if (errClose) {
        fprintf( pfTrace, zCmdFail, pzCmd );
        return NULL;
    }

    /*
     *  Now try to read back all the data.  If we fail due to either
     *  a sigpipe or sigalrm (timeout), we will return the nil string.
     */
    {
        char* pz = loadData();
        if (pz == NULL) {
            fprintf( pfTrace, zCmdFail, pzCmd );
            closeServer();
            pz = (char*)AGALOC( 1, "Text Block" );

            *pz = NUL;

        } else if (errClose)
            fprintf( pfTrace, zCmdFail, pzCmd );

        pzLastCmd = NULL;
        return pz;
    }
}

#endif /* ! SHELL_ENABLED */
/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agShell.c */
