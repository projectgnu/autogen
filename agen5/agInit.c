
/*
 *  agInit.c  $Id: agInit.c,v 4.15 2007/11/13 05:49:26 bkorb Exp $
 *
 *  Time-stamp:      "2007-11-12 20:43:38 bkorb"
 *
 *  Do all the initialization stuff.  For daemon mode, only
 *  children will return.
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2007 by Bruce Korb - all rights reserved
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

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by mk-fwd */
static void
addSysEnv( char* pzEnvName );
/* = = = END-STATIC-FORWARD = = = */

#ifdef DAEMON_ENABLED
 static ag_bool evalProto( tCC** ppzS, uint16_t* pProto );
 static void spawnPipe( char const* pzFile );
 static void spawnListens( tCC* pzPort, sa_family_t af );
 static void becomeDaemon( tCC*, tCC*, tCC*, tCC* );
#endif

#include "expr.ini"

LOCAL void
initialize( int arg_ct, char** arg_vec )
{
    /*
     *  Initialize all the Scheme functions.
     */
    ag_init();
    pzLastScheme = zSchemeInit;
    ag_scm_c_eval_string_from_file_line( zSchemeInit, SCHEME_INIT_FILE,
                                         schemeLine );
#ifndef scm_t_port
    {
        tSCC zInitRest[] =
            "(add-hook! before-error-hook error-source-line)\n"
            "(use-modules (ice-9 stack-catch))";
        pzLastScheme = zInitRest;
        ag_scm_c_eval_string_from_file_line( zInitRest, __FILE__, __LINE__-3 );
    }
#endif

    pzLastScheme = NULL;
    procState = PROC_STATE_OPTIONS;
    /*
     *  Set the last resort search directories first (lowest priority)
     *  The lowest of the low is the config time install data dir.
     *  Next is the *current* directory of this executable.
     */
    SET_OPT_TEMPL_DIRS( "$@" );
    SET_OPT_TEMPL_DIRS( "$$/../share/autogen" );

    {
        char z[ 128 ] = "__autogen__";
#if defined( HAVE_SOLARIS_SYSINFO )
        static const int nm[] = {
            SI_SYSNAME, SI_HOSTNAME, SI_ARCHITECTURE, SI_HW_PROVIDER,
#ifdef      SI_PLATFORM
            SI_PLATFORM,
#endif
            SI_MACHINE };
        int  ix;
        long sz;

        addSysEnv( z );
        for (ix = 0; ix < sizeof(nm)/sizeof(nm[0]); ix++) {
            sz = sysinfo( nm[ix], z+2, sizeof( z ) - 2);
            if (sz > 0) {
                sz += 2;
                while (z[sz-1] == NUL)  sz--;
                strcpy( z + sz, "__" );
                addSysEnv( z );
            }
        }

#elif defined( HAVE_UNAME_SYSCALL )
        struct utsname unm;

        addSysEnv( z );
        if (uname( &unm ) != 0) {
            fprintf( stderr, "Error %d (%s) making uname(2) call\n",
                     errno, strerror( errno ));
            exit( EXIT_FAILURE );
        }

        sprintf( z+2, "%s__", unm.sysname );
        addSysEnv( z );

        sprintf( z+2, "%s__", unm.machine );
        addSysEnv( z );

        sprintf( z+2, "%s__", unm.nodename );
        addSysEnv( z );
#else

        addSysEnv( z );
#endif
    }

    doOptions(arg_ct, arg_vec);

    if (OPT_VALUE_TRACE > TRACE_NOTHING) {
        tSCC zBT[] = "(debug-enable 'backtrace)";
        pzLastScheme = zBT;
        ag_scm_c_eval_string_from_file_line( zBT, __FILE__, __LINE__ - 2 );
    }

#ifdef DAEMON_ENABLED

    if (! HAVE_OPT( DAEMON ))
        return;

    if (0) {
        tSCC zDevNull[] =
#ifndef DEBUG_ENABLED
            "/dev/null";
#else
            "/tmp/AutoGenDebug.txt";
#endif /* DEBUG_ENABLED */
        becomeDaemon( "/", zDevNull, zDevNull, zDevNull );
    }

    {
        sa_family_t af = AF_INET;
        tCC*  pzS = OPT_ARG( DAEMON );

        if (evalProto( &pzS, &af ))
            spawnListens( pzS, af );
        else
            spawnPipe( pzS );
    }
#endif /* DAEMON_ENABLED */
}


static void
addSysEnv( char* pzEnvName )
{
    tSCC zFmt[] = "%s=1";
    int i = 2;

    for (;;) {
        if (IS_UPPER_CASE_CHAR(pzEnvName[i]))
            pzEnvName[i] = tolower(pzEnvName[i]);
        else if (! IS_ALPHANUMERIC_CHAR(pzEnvName[i]))
            pzEnvName[i] = '_';

        if (pzEnvName[ ++i ] == NUL)
            break;
    }

    /*
     *  If the user already has something in the environment, do not
     *  override it.
     */
    if (getenv( pzEnvName ) == NULL) {
        char* pz;

        if (OPT_VALUE_TRACE > TRACE_NOTHING)
            fprintf( pfTrace, "Adding ``%s'' to environment\n", pzEnvName );
        pz = aprf( zFmt, pzEnvName );
        TAGMEM( pz, "Added environment var" );
        putenv( pz );
    }
}


#ifdef DAEMON_ENABLED

  static ag_bool
evalProto( tCC** ppzS, uint16_t* pProto )
{
    tCC* pzS = *ppzS;

    if (IS_ALPHABETIC_CHAR(*pzS)) {
        inet_family_map_t* pMap = inet_family_map;
        do  {
            if (strncmp( pzS, pMap->pz_name, pMap->nm_len ) == 0) {
                *pProto = pMap->family;
                *ppzS += pMap->nm_len;
                return AG_TRUE;
            }
        } while ( (++pMap)->pz_name != NULL );
    }

    return IS_DEC_DIGIT_CHAR(*pzS);
}


  LOCAL void
handleSighup( int sig )
{
    redoOptions = AG_TRUE;
}


  static void
spawnPipe( tCC* pzFile )
{
#   define S_IRW_ALL \
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

#ifdef HAVE_FATTACH

    tFdPair fdpair;
    ag_bool removeDaemonFile = AG_FALSE;

    if (pipe( (int*)&fdpair ) != 0)
        AG_ABEND( aprf( zCannot, errno, "create", "a pipe", strerror(errno)));

    {
        struct stat sb;

        if (stat( pzFile, &sb ) != 0) {
            int fd;
            if (errno != ENOENT)
                AG_ABEND( aprf( zCannot, errno, "stat",
                                pzFile, strerror( errno )));
            fd = creat( pzFile, S_IRW_ALL );
            if (fd < 0)
                AG_ABEND( aprf( zCannot, errno, "creat(e)",
                                pzFile, strerror( errno )));
            close( fd );
            removeDaemonFile = AG_TRUE;
        }
    }

#ifdef HAVE_CONNLD
    (void)ioctl( fdpair.writeFd, I_PUSH, "connld" );
#endif /* HAVE_CONNLD */

    if (fattach( fdpair.writeFd, pzFile ) != 0)
        AG_ABEND( aprf( zCannot, errno, "fattach", pzFile, strerror(errno)));

    close( fdpair.writeFd );

    {
        struct pollfd polls[1];
        polls[0].fd     = fdpair.readFd;
        polls[0].events = POLLIN | POLLPRI;

        for (;;) {
            int ct = poll( polls, 1, -1 );
            struct strrecvfd recvfd;
            pid_t child;

            switch (ct) {
            case -1:
                if ((errno != EINTR) || (! redoOptions))
                    goto finish_with_fattach;

                optionRestore( &autogenOptions );
                doOptions( autogenOptions.origArgCt,
                           autogenOptions.origArgVect );
                SET_OPT_DEFINITIONS("-");
                break;

            case 1:
                if ((polls[0].revents & POLLIN) == 0)
                    continue;

                child = fork();
                switch (child) {
                default:
                    waitpid( child, &ct, 0 );
                    continue;

                case -1:
                    AG_ABEND(aprf(zCannot, errno, "fork", zNil, strerror(errno)));

                case 0:
                }
#ifdef HAVE_CONNLD
                if (ioctl( fdpair.readFd, I_RECVFD, &recvfd ) != 0)
                    AG_ABEND( aprf(zCannot, errno, "I_RECVFD", "on a pipe",
                                   strerror(errno)));
#else
                recvfd.fd = fdpair.readFd;
#endif /* ! HAVE_FATTACH */

                if (dup2( recvfd.fd, STDIN_FILENO ) != STDIN_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", "stdin",
                                   strerror(errno)));

                if (dup2( fdpair.writeFd, STDOUT_FILENO ) != STDOUT_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", "stdout",
                                   strerror(errno)));
                return;
            }
        }
    }

 finish_with_fattach:
    if (removeDaemonFile)
        unlink( pzFile );
#   undef S_IRW_ALL

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#else  /* ! HAVE_FATTACH */
    tFdPair fdpair;
    char* pzIn;
    char* pzOut;

    {
        size_t len = 2 * (strlen(pzFile) + 5);
        pzIn = AGALOC( len + 5, "fifo file name" );
        pzOut = pzIn + sprintf( pzIn, "%s-in", pzFile ) + 1;
    }

    unlink( pzIn );
    if ((mkfifo( pzIn, S_IRW_ALL ) != 0) && (errno != EEXIST))
        AG_ABEND( aprf( zCannot, errno, "mkfifo",    pzIn, strerror(errno)));

    (void)sprintf( pzOut, "%s-out", pzFile );
    unlink( pzOut );
    if ((mkfifo( pzOut, S_IRW_ALL ) != 0) && (errno != EEXIST))
        AG_ABEND( aprf( zCannot, errno, "mkfifo",    pzOut, strerror(errno)));

    fdpair.readFd = open( pzIn, O_RDONLY );
    if (fdpair.readFd < 0)
        AG_ABEND( aprf( zCannot, errno, "open fifo", pzIn, strerror(errno)));


    {
        struct pollfd polls[1];
        polls[0].fd     = fdpair.readFd;
        polls[0].events = POLLIN | POLLPRI;

        for (;;) {
            int ct = poll( polls, 1, -1 );
            struct strrecvfd recvfd;
            pid_t child;

            switch (ct) {
            case -1:
                if ((errno != EINTR) || (! redoOptions))
                    goto no_fattach_finish;

                optionRestore( &autogenOptions );
                doOptions( autogenOptions.origArgCt,
                           autogenOptions.origArgVect );
                SET_OPT_DEFINITIONS("-");
                break;

            case 1:
                if ((polls[0].revents & POLLIN) == 0)
                    continue;

                child = fork();
                switch (child) {
                default:
                    waitpid( child, &ct, 0 );
                    continue;

                case -1:
                    AG_ABEND(aprf(zCannot, errno, "fork", zNil, strerror(errno)));

                case 0:
                }

                if (dup2( fdpair.readFd, STDIN_FILENO ) != STDIN_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", "stdin",
                                   strerror(errno)));

                fdpair.writeFd = open( pzOut, O_WRONLY );
                if (fdpair.writeFd < 0)
                    AG_ABEND( aprf( zCannot, errno, "open fifo", pzOut,
                                    strerror(errno)));

                polls[0].fd = fdpair.writeFd;
                polls[0].events = POLLOUT;
                if (poll( polls, 1, -1 ) != 1)
                    AG_ABEND( aprf(zCannot, errno, "poll", "write pipe",
                                   strerror( errno )));

                if (dup2( fdpair.writeFd, STDOUT_FILENO ) != STDOUT_FILENO)
                    AG_ABEND( aprf(zCannot, errno, "dup2", pzOut,
                                   strerror(errno)));
                return;
            }
        }
    }

 no_fattach_finish:
    unlink( pzIn );
    unlink( pzOut );
    AGFREE( pzIn );
#endif /* ! HAVE_FATTACH */
#   undef S_IRW_ALL

    exit( EXIT_SUCCESS );
}


  static void
spawnListens( tCC* pzPort, sa_family_t addr_family )
{
    tSCC zPortFmt[] = "to port %s with %d type address";
    int socket_fd = socket( addr_family, SOCK_STREAM, 0 );
    union {
        struct sockaddr     addr;
        struct sockaddr_in  in_addr;
        struct sockaddr_un  un_addr;
    } sa;

    uint32_t        addr_len;

    if (socket_fd < 0)
        AG_ABEND( aprf( zCannot, errno, "socket", "AF_INET/SOCK_STREAM",
                        strerror( errno )));

    switch (addr_family) {

    case AF_UNIX:
    {
        uint32_t p_len = strlen( pzPort );

        if (p_len > sizeof(sa.un_addr.sun_path))
            AG_ABEND( aprf( "AF_UNIX path exceeds %d", p_len ));
        sa.un_addr.sun_family  = AF_UNIX;
        strncpy( sa.un_addr.sun_path, pzPort, p_len );
        addr_len = sizeof( sa.un_addr ) - sizeof( sa.un_addr.sun_path ) + p_len;
    }
    break;

    case AF_INET:
    {
        uint16_t port;
        char* pz;

        sa.in_addr.sin_family      = AF_INET;
        sa.in_addr.sin_addr.s_addr = INADDR_ANY;

        errno = 0;
        if ((unlink( pzPort ) != 0) && (errno != ENOENT))
            AG_ABEND(aprf( zCannot, errno, "unlink", pzPort, strerror(errno)));

        port = (uint16_t)strtol( pzPort, &pz, 0 );
        if ((errno != 0) || (*pz != NUL))
            AG_ABEND( aprf( "Invalid port number:  '%s'", pzPort ));

        sa.in_addr.sin_port = htons( (short)port );
        addr_len = sizeof( sa.in_addr );
    }
    break;

    default:
        AG_ABEND(aprf("The '%d' address family cannot be handled", addr_family));
    }

    if (bind( socket_fd, &sa.addr, addr_len ) < 0) {
        char* pz = aprf( zPortFmt, pzPort, addr_family );
        AG_ABEND( aprf( zCannot, errno, "bind", pz, strerror( errno )));
    }

    if (fcntl( socket_fd, F_SETFL, O_NONBLOCK ) < 0) {
        AG_ABEND( aprf( zCannot, errno, "socket-fcntl", "FNDELAY",
                        strerror( errno )));
    }

    if (listen( socket_fd, 5 ) < 0) {
        char* pz = aprf( zPortFmt, pzPort );
        AG_ABEND( aprf( zCannot, errno, "listen", pz, strerror( errno )));
    }

    for (;;) {
        fd_set fds;
        int    max_fd = socket_fd;
        int    new_conns;

        FD_ZERO( &fds );
        FD_SET( socket_fd, &fds );

        new_conns = select( max_fd, &fds, NULL, NULL, NULL );
        if (new_conns < 0) {
            if (errno == EINTR)
                continue;

            if (! redoOptions) {
                unlink( pzPort );
                exit( EXIT_SUCCESS );
            }

            optionRestore( &autogenOptions );
            doOptions( autogenOptions.origArgCt,
                       autogenOptions.origArgVect );
            SET_OPT_DEFINITIONS("-");

            continue;
        }

        if (new_conns > 0) {
            switch (fork()) {
            default: continue;
            case -1: AG_ABEND(aprf(zCannot, errno, "fork", zNil,strerror(errno)));
            case 0:  break;
            }
            break;
        }
    }

    for (;;) {
        static int try_ct = 0;
        struct sockaddr addr;
        socklen_t addr_len;
        int fd = accept( socket_fd, &addr, &addr_len );
        if (fd < 0)
            switch (errno) {
            case EINTR:
            case EAGAIN:
#if (EAGAIN != EWOULDBLOCK)
            case EWOULDBLOCK:
#endif
                if (try_ct++ < 10000) {
                    sleep(1);
                    continue;
                }
            }
        socket_fd = fd;
        break;
    }

    if (dup2( socket_fd, STDOUT_FILENO ) != STDOUT_FILENO)
        AG_ABEND( aprf(zCannot, errno, "dup2", "out on socket_fd",
                       strerror(errno)));
    if (dup2( socket_fd, STDIN_FILENO ) != STDIN_FILENO)
        AG_ABEND( aprf(zCannot, errno, "dup2", "in on socket_fd",
                       strerror(errno)));
}


  static void
becomeDaemon( tCC* pzStdin,
              tCC* pzStdout,
              tCC* pzStderr,
              tCC* pzDaemonDir )
{
    tSCC zNoFork[] = "Error %d while forking:  %s\n";
    /*
     *  Become a daemon process by exiting the current process
     *  and allowing the child to continue.  Also, change stdin,
     *  stdout and stderr to point to /dev/null and change to
     *  the root directory ('/').
     */
    {
        int ret = fork();

        switch (ret) {
        case -1:
            fprintf( stderr, zNoFork, errno, strerror( errno ));
        default:
            exit( ret );

        case 0:
            break;
        }
    }

    /*
     *  Now, become a process group and session group leader.
     */
    if (setsid() == -1) {
        fprintf( stderr, "Error %d setting session ID:  %s\n",
                 errno, strerror( errno ));
        exit( 99 );
    }

    /*
     *  There is now no controlling terminal.  However, if we open anything
     *  that resembles a terminal, it will become our controlling terminal.
     *  So, we will fork again and the child will not be a group leader and
     *  thus cannot gain a controlling terminal.
     */
    switch (fork()) {
    case -1:
        fprintf( stderr, zNoFork, errno, strerror( errno ));
        exit( 99 );

    case 0:
        break;

    default:
        exit( 0 );  /* parent process - silently go away */
    }

    umask( 0 );
    if (pzDaemonDir == (char*)NULL)
        pzDaemonDir = "/";

    chdir( pzDaemonDir );

    /*
     *  Reopen the input, output and error files, unless we were told not to
     */
    if (pzStdin != (char*)NULL)
        freopen( pzStdin,  "r", stdin );

    if (pzStdout != (char*)NULL)
        freopen( pzStdout, "w", stdout );

    if (pzStderr != (char*)NULL)
        freopen( pzStderr, "w", stderr );

    /* We are a daemon now */
}
#endif /* DAEMON_ENABLED */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agInit.c */
