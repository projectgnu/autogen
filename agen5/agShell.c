/*
 *  agShell
 *  $Id: agShell.c,v 1.1 1999/10/14 00:33:53 bruce Exp $
 *  Manage a server shell process
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

#include <compat/compat.h>

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#else
#  error NEED  <fcntl.h>
#endif

#include <signal.h>

#if HAVE_SYS_PROCSET_H
#  include <sys/procset.h>
#endif

#include "autogen.h"

#ifndef HAVE_SIGSEND
#  include <compat/sigsend.c>
#endif

/*
 *  Dual pipe opening of a child process
 */
STATIC tpChar  defArgs[]  = { (char*)NULL, (char*)NULL };
STATIC tpfPair serverPair = { (FILE*)NULL, (FILE*)NULL };
STATIC pid_t   serverId   = NULLPROCESS;
STATIC tpChar  pCurDir    = (char*)NULL;
STATIC ag_bool errClose   = AG_FALSE;

tSCC   zTxtBlock[] = "Text Block";
tSCC   zCmdFmt[]   = "\\cd %s\n%s\n\necho\necho %s\n";

const char* pzLastCmd = (const char*)NULL;


    STATIC void
closeServer( void )
{
    sigsend( P_PID, (id_t) serverId, SIGKILL );
    serverId = NULLPROCESS;
    fclose( serverPair.pfRead );
    fclose( serverPair.pfWrite );
    serverPair.pfRead = serverPair.pfWrite = (FILE*)NULL;
}


    STATIC void
sigHandler( int signo )
{
    fprintf( stderr, "Closing server:  %s signal (%d) received\n",
             strsignal( signo ), signo );
    errClose = AG_TRUE;
    if (pzLastCmd == (const char*)NULL)
        pzLastCmd = "?? unknown ??\n";

    fputs( "\nLast command issued:\n", stderr );
    fprintf( stderr, zCmdFmt, pCurDir, pzLastCmd, zShDone );
    pzLastCmd = (const char*)NULL;
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
            atexit( &closeServer );
            pCurDir = getcwd( (char*)NULL, MAXPATHLEN+1 );
#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_SHELL )
            if (HAVE_OPT( SHOW_SHELL ))
                fputs( "\nServer First Start\n", stderr );
        }
        else {
            fputs( "\nServer Restart\n", stderr );
#endif
        }
    }

    saNew.sa_handler = sigHandler;
    saNew.sa_flags   = 0;
    sigemptyset( &saNew.sa_mask );
    sigaction( SIGPIPE, &saNew, &saSave1 );
    sigaction( SIGALRM, &saNew, &saSave2 );

    errClose = AG_FALSE;

    {
        tSCC zTrap[] = "for f in 1 2 5 6 7 13 14\n"
                       "do trap \"echo trapped on $f >&2\" $f ; done";
        char* pz;
        pzLastCmd = zTrap;
        fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzLastCmd, zShDone );
        fflush( serverPair.pfWrite );
        pz = loadData( serverPair.pfRead );
#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_SHELL )
        if (HAVE_OPT( SHOW_SHELL ) && (*pz != NUL))
            fprintf( stderr, "Trap set result:  `%s'\n", pz );
        AGFREE( (void*)pz );
    }

    if (HAVE_OPT( SHOW_SHELL )) {
        tSCC zSetup[] = "set -x\n"
                        "exec 2> /dev/tty\n"
                        "trap >&2\n"
                        "echo server setup done >&2\n";
        char* pz;

        fputs( "Server traps set\n", stderr );
        pzLastCmd = zSetup;
        fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzLastCmd, zShDone );
        fflush( serverPair.pfWrite );
        pz = loadData( serverPair.pfRead );
        if (HAVE_OPT( SHOW_SHELL ))
            fprintf( stderr, "Trap state:\n%s\n", pz );
#endif
        AGFREE( (void*)pz );
    }
    pzLastCmd = (const char*)NULL;
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
    int
chainOpen( int       stdinFd,
           tpChar*   ppArgs,
           pid_t*    pChild  )
{
    tFdPair   stdoutPair = { -1, -1 };
    pid_t     chId;
    char*     pzShell;

    /*
     *  If we did not get an arg list, use the default
     */
    if (ppArgs == (tpChar*)NULL)
        ppArgs = defArgs;

    /*
     *  If the arg list does not have a program,
     *  assume the "SHELL" from the environment, or, failing
     *  that, then sh.  Set argv[0] to whatever we decided on.
     */
    if (pzShell = *ppArgs,
       (pzShell == (char*)NULL) || (*pzShell == NUL)){

        pzShell = getenv( "SHELL" );
        if (pzShell == (char*)NULL)
            pzShell = "sh";

        *ppArgs = pzShell;
    }

    /*
     *  Create a pipe it will be the child process' stdout,
     *  and the parent will read from it.
     */
    if (pipe( (int*)&stdoutPair ) < 0) {
        if (pChild != (pid_t*)NULL)
            *pChild = NOPROCESS;
        return -1;
    }

#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_SHELL )
    if (HAVE_OPT( SHOW_SHELL )) {
        tSCC   zPath[] = "PATH";
        char*  pzPath  = getenv( zPath );

        if (pzPath == (char*)NULL)
            pzPath = pzShell;

        else {
            pzPath = pathfind( pzPath, pzShell, "rxs" );
            if (pzPath == (char*)NULL)
                pzPath = pzShell;
        }

        fprintf( stderr, "Starting shell `%s'\n", pzPath );
    }
#endif

    /*
     *  Call fork() and see which process we become
     */
    chId = fork();
    switch (chId) {
    case NOPROCESS:    /* parent - error in call */
        close( stdinFd );
        close( stdoutPair.readFd );
        close( stdoutPair.writeFd );
        if (pChild != (pid_t*)NULL)
            *pChild = NOPROCESS;
        return -1;

    default:           /* parent - return opposite FD's */
        if (pChild != (pid_t*)NULL)
            *pChild = chId;

        close( stdinFd );
        close( stdoutPair.writeFd );
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
    setvbuf( stdout, (char*)NULL, _IONBF, 0 );

    execvp( pzShell, ppArgs );
    fprintf( stderr, "Error %d:  Could not execvp( '%s', ... ):  %s\n",
             errno, pzShell, strerror( errno ));
    AG_ABEND;
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
    pid_t
openServer( tFdPair* pPair, tpChar* ppArgs )
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
    pid_t
openServerFP( tpfPair* pfPair, tpChar* ppArgs )
{
    tFdPair   fdPair;
    pid_t     chId = openServer( &fdPair, ppArgs );

    if (chId == NOPROCESS)
        return chId;

    pfPair->pfRead  = fdopen( fdPair.readFd,  "r" FOPEN_TEXT_FLAG );
    pfPair->pfWrite = fdopen( fdPair.writeFd, "w" FOPEN_TEXT_FLAG );
    return chId;
}


/*
 *  loadData
 *
 *  Read data from a file pointer (a pipe to a process in this context)
 *  until we either get EOF or we get a marker line back.
 *  The read data are stored in a malloc-ed string that is truncated
 *  to size at the end.  Input is assumed to be an ASCII string.
 */
    char*
loadData( FILE* fp )
{
    char*   pzText;
    size_t  textSize;
    char*   pzScan;
    char    zLine[ 1024 ];

    textSize = 4096;
    pzScan   =
    pzText   = AGALOC( textSize );
    if (pzText == (char*)NULL) {
        fprintf( stderr, zAllocErr, pzProg,
                 textSize, zTxtBlock );
        AG_ABEND;
    }

     *pzText  = NUL;

     for (;;) {
         size_t  usedCt;

         /*
          *  Set a timeout so we do not wait forever.
          */
         alarm( OPT_VALUE_TIMEOUT );

         if (fgets( zLine, sizeof( zLine ), fp ) == (char*)NULL)
             break;

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
             p = AGREALOC( (void*)pzText, textSize );
             if (p == (void*)NULL) {
                 fprintf( stderr, zAllocErr, pzProg,
                          textSize, "Realloc Text Block" );
                 AGFREE( (void*)pzText );
                 return (char*)NULL;
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
     return AGREALOC( (void*)pzText, (int)(pzScan - pzText) + 1 );
}


/*
 *  Run a semi-permanent server shell.  The program will be the
 *  one named by the environment variable $SHELL, or default to "sh".
 *  If one of the commands we send to it takes too long or it dies,
 *  we will shoot it and restart one later.
 */
    char*
runShell( const char*  pzCmd )
{
    tSCC zCmdFail[] = "CLOSING SHELL SERVER - command failure:\n\t%s\n";

    /*
     *  IF the shell server process is not running yet,
     *  THEN try to start it.
     */
    if (serverId == NULLPROCESS) {
        serverId = openServerFP( &serverPair, defArgs );
        if (serverId > 0)
            serverSetup();
    }

    /*
     *  IF it is still not running,
     *  THEN return the nil string.
     */
    if (serverId <= 0) {
        char* pz = (char*)AGALOC( 1 );
        if (pz == (char*)NULL) {
            fprintf( stderr, zAllocErr, pzProg,
                     1, zTxtBlock );
            AG_ABEND;
        }

        *pz = NUL;
        return pz;
    }

    /*
     *  Make sure the process will pay attention to us,
     *  send the supplied command, and then
     *  have it output a special marker that we can find.
     */
    pzLastCmd = pzCmd;
#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_SHELL )
    if (HAVE_OPT( SHOW_SHELL )) {
        fputc( '\n', stderr );
        fprintf( stderr, zCmdFmt, pCurDir, pzCmd, zShDone );
    }
#endif
    fprintf( serverPair.pfWrite, zCmdFmt, pCurDir, pzCmd, zShDone );
    fflush( serverPair.pfWrite );

    if (errClose) {
        fprintf( stderr, zCmdFail, pzCmd );
        return (char*)NULL;
    }

    /*
     *  Now try to read back all the data.  If we fail due to either
     *  a sigpipe or sigalrm (timeout), we will return the nil string.
     */
    {
        char* pz = loadData( serverPair.pfRead );
        if (pz == (char*)NULL) {
            fprintf( stderr, zCmdFail, pzCmd );
            closeServer();
            pz = (char*)AGALOC( 1 );
            if (pz == (char*)NULL) {
                fprintf( stderr, zAllocErr, pzProg,
                         1, zTxtBlock );
                AG_ABEND;
            }

            *pz = NUL;

        } else if (errClose)
            fprintf( stderr, zCmdFail, pzCmd );

        pzLastCmd = (char*)NULL;
        return pz;
    }
}
/* end of agShell.c */
