/*
 *  agShell
 *  $Id: agShell.c,v 3.15 2003/02/16 00:04:39 bkorb Exp $
 *  Manage a server shell process
 */

/*
 *  AutoGen copyright 1992-2003 Bruce Korb
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

#include "autogen.h"

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#else
#  error NEED  <fcntl.h>
#endif

#include <signal.h>

#if HAVE_SYS_PROCSET_H
#  include <sys/procset.h>
#endif

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
STATIC tpfPair serverPair = { NULL, NULL };
STATIC pid_t   serverId   = NULLPROCESS;
STATIC tpChar  pCurDir    = NULL;
STATIC ag_bool errClose   = AG_FALSE;

tSCC   zCmdFmt[]   = "cd %s\n%s\n\necho\necho %s\n";

const char* pzLastCmd = NULL;

STATIC void sigHandler( int signo );
STATIC void serverSetup( void );


EXPORT void
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
        (void)fclose( serverPair.pfWrite );
    }

    serverPair.pfRead = serverPair.pfWrite = NULL;
}


STATIC void
sigHandler( int signo )
{
#ifndef HAVE_STRSIGNAL
    extern char* strsignal PROTO(( int ));
#endif
    fprintf( pfTrace, "Closing server:  %s signal (%d) received\n",
             strsignal( signo ), signo );
    errClose = AG_TRUE;

    (void)fputs( "\nLast command issued:\n", pfTrace );
    {
        const char* pz = (pzLastCmd == NULL)
            ? "?? unknown ??\n" : pzLastCmd;
        fprintf( pfTrace, zCmdFmt, pCurDir, pz, zShDone );
    }
    pzLastCmd = NULL;
    closeServer();
}


STATIC void
serverSetup( void )
{
    struct sigaction  saNew;
    struct sigaction  saSave1;
    struct sigaction  saSave2;

    {
        static int doneOnce = 0;
        if (doneOnce++ == 0) {
            (void)atexit( &closeServer );
            pCurDir = getcwd( NULL, MAXPATHLEN+1 );
#if defined( DEBUG_ENABLED )
            if (HAVE_OPT( SHOW_SHELL ))
                fputs( "\nServer First Start\n", pfTrace );
        }
        else {
            fputs( "\nServer Restart\n", pfTrace );
#endif
        }
    }

    saNew.sa_handler = sigHandler;
    saNew.sa_flags   = 0;
    (void)sigemptyset( &saNew.sa_mask );
    (void)sigaction( SIGPIPE, &saNew, &saSave1 );
    (void)sigaction( SIGALRM, &saNew, &saSave2 );

    errClose = AG_FALSE;

    {
        static char zTrap[] =
            /*
             *  IF it is possible to put the base shell into POSIX/Bourne mode,
             *  THEN do so...
             */
            "if test -n \"${ZSH_VERSION+set}\" "
                "&& (emulate sh) >/dev/null 2>&1\n"
            "then\n"
            "  emulate sh\n"
            "  NULLCMD=:\n"
            "elif test -n \"${BASH_VERSION+set}\" "
                  "&& (set -o posix) >/dev/null 2>&1\n"
            "then\n"
            "  set -o posix\n"
            "fi\n"

            "for f in 1 2 5 6 7 13 14\n"
            "do trap \"echo trapped on $f >&2\" $f 2>/dev/null\n"
            "done\n" "unalias cd 2>/dev/null >&2\n" "AG_pid=\000000000000\n";
        static char* pzPid = NULL;
        char* pz;
        pzLastCmd = zTrap;
        if (pzPid == NULL)
            pzPid = zTrap + strlen( zTrap );
        sprintf( pzPid, "%d\n", getpid() );
        fprintf( serverPair.pfWrite, zCmdFmt, pCurDir,
                 pzLastCmd, zShDone );
        (void)fflush( serverPair.pfWrite );
        pz = loadData( serverPair.pfRead );
#if defined( DEBUG_ENABLED )
        if (HAVE_OPT( SHOW_SHELL ) && (*pz != NUL))
            fprintf( pfTrace, "Trap set result:  `%s'\n", pz );
        AGFREE( (void*)pz );
    }

    if (HAVE_OPT( SHOW_SHELL )) {
        tSCC zSetup[] = "set -x\n"
                        "exec 2> /dev/tty\n"
                        "trap\n"
                        "echo server setup done\n";
        char* pz;

        fputs( "Server traps set\n", pfTrace );
        pzLastCmd = zSetup;
        fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzLastCmd, zShDone );
        (void)fflush( serverPair.pfWrite );
        pz = loadData( serverPair.pfRead );
        fprintf( pfTrace, "Trap state:\n%s\n", pz );
#endif
        AGFREE( (void*)pz );
    }
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
EXPORT int
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
#if defined( DEBUG_ENABLED )
        if (HAVE_OPT( SHOW_SHELL ))
            fprintf( pfTrace, "Server shell is pid %d\n", chId );
#endif
        return stdoutPair.readFd;

    case NULLPROCESS:  /* child - continue processing */
        break;
    }

    /*
     *  Close the pipe end handed back to the parent process
     */
    close( stdoutPair.readFd );

    /*
     *  Close our current stdin and stdout
     */
    close( STDIN_FILENO );
    close( STDOUT_FILENO );

    /*
     *  Make the fd passed in the stdin, and the write end of
     *  the new pipe become the stdout.
     */
    fcntl( stdoutPair.writeFd, F_DUPFD, STDOUT_FILENO );
    fcntl( stdinFd,            F_DUPFD, STDIN_FILENO );

    /*
     *  Make the output file unbuffered only.
     *  We do not want to wait for shell output buffering.
     */
    setvbuf( stdout, NULL, _IONBF, 0 );

#if defined( DEBUG_ENABLED )
    if (HAVE_OPT( SHOW_SHELL ))
        fprintf( pfTrace, "Server shell %s starts\n", pzShell );
#endif
    execvp( (char*)pzShell, (char**)ppArgs );
    AG_ABEND( aprf( "Could not execvp( '%s', ... ):  %d - %s\n",
                    pzShell, errno, strerror( errno )));
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
EXPORT pid_t
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
EXPORT pid_t
openServerFP( tpfPair* pfPair, tCC** ppArgs )
{
    tFdPair   fdPair;
    pid_t     chId = openServer( &fdPair, ppArgs );

    if (chId == NOPROCESS)
        return chId;

    pfPair->pfRead  = fdopen( fdPair.readFd,  "r" FOPEN_TEXT_FLAG );
    pfPair->pfWrite = fdopen( fdPair.writeFd, "w" FOPEN_TEXT_FLAG );
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
EXPORT char*
loadData( FILE* fp )
{
    char*   pzText;
    size_t  textSize;
    char*   pzScan;
    char    zLine[ 1024 ];

    textSize = 4096;
    pzScan   = \
    pzText   = AGALOC( textSize, "Text Block" );

    *pzText  = NUL;

    for (;;) {
        size_t  usedCt;
#ifdef OPT_VALUE_TIMEOUT
        /*
         *  Set a timeout so we do not wait forever.
         */
        alarm( OPT_VALUE_TIMEOUT );
        if (fgets( zLine, sizeof( zLine ), fp ) == NULL) {
            alarm( 0 );
            break;
        }
        alarm( 0 );

#else
        if (fgets( zLine, sizeof( zLine ), fp ) == NULL)
            break;
#endif
        /*
         *  Check for magic character sequence indicating 'DONE'
         */
        if (strncmp( zLine, zShDone, STRSIZE( zShDone )) == 0)
            break;

        strcpy( pzScan, zLine );

        /*
         *  Stop now if we are at EOF
         */
        if (feof( fp ))
            break;

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
    *pzScan = NUL;
    return AGREALOC( (void*)pzText, (int)(pzScan - pzText) + 1,
                     "resizing text output" );
}

#ifdef SHELL_ENABLED
/*
 *  Run a semi-permanent server shell.  The program will be the
 *  one named by the environment variable $SHELL, or default to "sh".
 *  If one of the commands we send to it takes too long or it dies,
 *  we will shoot it and restart one later.
 */
EXPORT char*
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
#if defined( DEBUG_ENABLED )
    if (HAVE_OPT( SHOW_SHELL )) {
        fputc( '\n', pfTrace );
        fprintf( pfTrace, zCmdFmt, pCurDir, pzCmd, zShDone );
    }
#endif
    fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzCmd, zShDone );
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
        char* pz = loadData( serverPair.pfRead );
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
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agShell.c */
