
/**
 * @file agInit.c
 *
 *  Do all the initialization stuff.  For daemon mode, only
 *  children will return.
 *
 * @addtogroup autogen
 * @{
 */
/*
 *  This file is part of AutoGen.
 *  Copyright (C) 1992-2014 Bruce Korb - all rights reserved
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

static char const * ld_lib_path = NULL;

/* = = = START-STATIC-FORWARD = = = */
static void
init_scm(void);

static char const *
make_quote_str(char const * str);

static void
dep_usage(char const * fmt, ...);

static void
add_sys_env(char * env_name);
/* = = = END-STATIC-FORWARD = = = */

#ifdef DAEMON_ENABLED
 static bool evalProto(char const ** ppzS, uint16_t* pProto);
 static void spawnPipe(char const* pzFile);
 static void spawnListens(char const * pzPort, sa_family_t af);
 static void daemonize(char const *, char const *, char const *,
                       char const *);
#endif

#include "expr.ini"

/**
 * Various initializations.
 *
 * @param arg_ct  the count of program arguments, plus 1.
 * @param arg_vec the program name plus its arguments
 */
LOCAL void
initialize(int arg_ct, char ** arg_vec)
{
    putenv(C(char *, ld_lib_path));

    scribble_init();

    /*
     *  Initialize all the Scheme functions.
     */
    ag_init();
    init_scm();

    last_scm_cmd = NULL;
    processing_state = PROC_STATE_OPTIONS;

    process_ag_opts(arg_ct, arg_vec);
    ag_exit_code = AUTOGEN_EXIT_LOAD_ERROR;

    if (OPT_VALUE_TRACE > TRACE_NOTHING)
        SCM_EVAL_CONST(INIT_SCM_DEBUG_FMT);

#ifdef DAEMON_ENABLED

    if (! HAVE_OPT(DAEMON))
        return;

#ifdef DEBUG_ENABLED
    {
        static char const logf[] = "/tmp/ag-debug.txt";
        daemonize("/", logf, logf, logf);
    }
#else
    daemonize("/", DEV_NULL, DEV_NULL, DEV_NULL);
#endif /* DEBUG_ENABLED */

    {
        sa_family_t  af  = AF_INET;
        char const * pzS = OPT_ARG(DAEMON);

        if (evalProto(&pzS, &af))
            spawnListens(pzS, af);
        else
            spawnPipe(pzS);
    }
#endif /* DAEMON_ENABLED */
}

static void
init_scm(void)
{
    last_scm_cmd = SCHEME_INIT_TEXT;

    {
        SCM ini_res = ag_scm_c_eval_string_from_file_line(
            SCHEME_INIT_TEXT, AG_TEXT_STRTABLE_FILE, SCHEME_INIT_TEXT_LINENO);
        AGDUPSTR(libguile_ver, scm2display(ini_res), "ini res");
    }

    {
        unsigned int maj, min, mic;
        switch (sscanf(libguile_ver, "%u.%u.%u", &maj, &min, &mic)) {
        case 2:
        case 3: break;
        default:
            AG_ABEND(aprf(GUILE_VERSION_BAD, libguile_ver));
            /* NOT_REACHED */
        }
        maj = min + (100 * maj);
        if ((GUILE_VERSION / 1000) != maj)
            AG_ABEND(aprf(GUILE_VERSION_WRONG, libguile_ver,
                          MK_STR(GUILE_VERSION)));
    }

    {
#       if GUILE_VERSION >= 200000
#         define SCHEME_INIT_DEBUG SCHEME_INIT_DEBUG_2_0
#       else
#         define SCHEME_INIT_DEBUG SCHEME_INIT_DEBUG_1_6
#       endif
        char * p = aprf(INIT_SCM_ERRS_FMT, SCHEME_INIT_DEBUG);
#       undef  SCHEME_INIT_DEBUG

        last_scm_cmd = p;
        ag_scm_c_eval_string_from_file_line(p, __FILE__, __LINE__);
        AGFREE(p);
    }
}

/**
 * make a name resilient to machinations made by 'make'.
 * Basically, dollar sign characters are doubled.
 *
 * @param str the input string
 * @returns a newly allocated string with the '$' characters doubled
 */
static char const *
make_quote_str(char const * str)
{
    size_t sz = strlen(str) + 1;
    char const * scan = str;
    char * res;

    for (;;) {
        char * p = strchr(scan, '$');
        if (p == NULL)
            break;
        sz++;
        scan = scan + 1;
    }

    res  = AGALOC(sz, "q name");
    scan = res;

    for (;;) {
        char * p = strchr(str, '$');

        if (p == NULL)
            break;
        sz = (size_t)(p - str) + 1;
        memcpy(res, str, sz);
        res += sz;
        str += sz;
        *(res++) = '$';
    }

    strcpy(res, str);
    return scan;
}

/**
 * Error in dependency specification
 *
 * @param fmt the error message format
 */
static void
dep_usage(char const * fmt, ...)
{
    char * msg;

    {
        va_list ap;
        va_start(ap, fmt);
        (void)vasprintf(&msg, fmt, ap);
        va_end(ap);
    }

    usage_message(USAGE_INVAL_DEP_OPT_FMT, msg);
    /* NOTREACHED */
}

/**
 * Configure a dependency option.
 * Handles any of these letters:  MFQTPGD as the first part of the option
 * argument.
 *
 * @param opts the autogen options data structure
 * @param pOptDesc the option descriptor for this option.
 */
LOCAL void
config_dep(tOptions * opts, tOptDesc * od)
{
    char const * opt_arg = od->optArg.argString;
    (void)opts;

    /*
     *  The option argument is optional.  Make sure we have one.
     */
    if (opt_arg == NULL)
        return;

    while (*opt_arg == 'M')  opt_arg++;
    opt_arg = SPN_WHITESPACE_CHARS(opt_arg);

    switch (*opt_arg) {
    case 'F':
        if (dep_file != NULL)
            dep_usage(CFGDEP_DUP_TARGET_MSG);

        opt_arg = SPN_WHITESPACE_CHARS(opt_arg + 1);
        AGDUPSTR(dep_file, opt_arg, "f name");
        break;

    case 'Q':
    case 'T':
    {
        bool quote_it = (*opt_arg == 'Q');

        if (dep_target != NULL)
            dep_usage(CFGDEP_DUP_TARGET_MSG);

        opt_arg = SPN_WHITESPACE_CHARS(opt_arg + 1);
        if (quote_it)
            dep_target = make_quote_str(opt_arg);
        else
            AGDUPSTR(dep_target, opt_arg, "t name");
        break;
    }

    case 'P':
        dep_phonies = true;
        break;

    case 'D':
    case 'G':
    case NUL:
        /*
         *  'D' and 'G' make sense to GCC, not us.  Ignore 'em.  If we
         *  found a NUL byte, then act like we found -MM on the command line.
         */
        break;

    default:
        dep_usage(CFGDEP_UNKNOWN_DEP_FMT, opt_arg);
    }
}

/**
 * Add a system name to the environment.  The input name is up-cased and
 * made to conform to environment variable names.  If not already in the
 * environment, it is added with the string value "1".
 *
 * @param env_name in/out: system name to export
 */
static void
add_sys_env(char * env_name)
{
    int i = 2;

    for (;;) {
        if (IS_UPPER_CASE_CHAR(env_name[i]))
            env_name[i] = (char)tolower(env_name[i]);
        else if (! IS_ALPHANUMERIC_CHAR(env_name[i]))
            env_name[i] = '_';

        if (env_name[ ++i ] == NUL)
            break;
    }

    /*
     *  If the user already has something in the environment, do not
     *  override it.
     */
    if (getenv(env_name) == NULL) {
        char * pz;

        if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE)
            fprintf(trace_fp, TRACE_ADD_TO_ENV_FMT, env_name);

        pz = aprf(ADD_SYS_ENV_VAL_FMT, env_name);
        putenv(pz);
    }
}

/**
 * Prepare the raft of environment variables.
 * This runs before Guile starts and grabs the value for LD_LIBRARY_PATH.
 * Guile likes to fiddle that.  When we run initialize(), we will force it
 * to match what we currently have.  Additionally, force our environment
 * to be "C" and export all the variables that describe our system.
 */
LOCAL void
prep_env(void)
{
    /*
     *  as of 2.0.2, Guile will fiddle with strings all on its own accord.
     *  Coerce the environment into being POSIX ASCII strings so it keeps
     *  its bloody stinking nose out of our data.
     */
    putenv(C(char *, LC_ALL_IS_C));

    /*
     *  If GUILE_WARN_DEPRECATED has not been defined, then likely we are
     *  not in a development environment and likely we don't want to give
     *  our users any angst.
     */
    if (getenv(GUILE_WARN_DEP_STR) == NULL)
        putenv(C(char *, GUILE_WARN_NO_ENV));

    ld_lib_path = getenv(LD_LIB_PATH_STR);
    if (ld_lib_path == NULL) {
        ld_lib_path = LD_LIB_PATH_PFX;

    } else {
        size_t psz = strlen(ld_lib_path) + 1;
        char * p = AGALOC(LD_LIB_PATH_PFX_LEN + psz, "lp");
        memcpy(p, LD_LIB_PATH_PFX, LD_LIB_PATH_PFX_LEN);
        memcpy(p + LD_LIB_PATH_PFX_LEN, ld_lib_path, psz);
        ld_lib_path = p;
    }

    /*
     *  Set the last resort search directories first (lowest priority)
     *  The lowest of the low is the config time install data dir.
     *  Next is the *current* directory of this executable.
     *  Last (highest priority) of the low priority is the library data dir.
     */
    SET_OPT_TEMPL_DIRS(DFT_TPL_DIR_DATA);
    SET_OPT_TEMPL_DIRS(DFT_TPL_DIR_RELATIVE);
    SET_OPT_TEMPL_DIRS(LIBDATADIR);

    {
        char z[ SCRIBBLE_SIZE ] = "__autogen__";
#if defined(HAVE_SOLARIS_SYSINFO)
        static const int nm[] = {
            SI_SYSNAME, SI_HOSTNAME, SI_ARCHITECTURE, SI_HW_PROVIDER,
#ifdef      SI_PLATFORM
            SI_PLATFORM,
#endif
            SI_MACHINE };
        int  ix;
        long sz;

        add_sys_env(z);
        for (ix = 0; ix < sizeof(nm)/sizeof(nm[0]); ix++) {
            sz = sysinfo(nm[ix], z+2, sizeof(z) - 2);
            if (sz > 0) {
                sz += 2;
                while (z[sz-1] == NUL)  sz--;
                strcpy(z + sz, ADD_ENV_VARS_SUFFIX_FMT+2);
                add_sys_env(z);
            }
        }

#elif defined(HAVE_UNAME_SYSCALL)
        struct utsname unm;

        add_sys_env(z);
        if (uname(&unm) != 0)
            AG_CANT(UNAME_CALL_NAME, SYSCALL_NAME);

        sprintf(z+2, ADD_ENV_VARS_SUFFIX_FMT, unm.sysname);
        add_sys_env(z);

        sprintf(z+2, ADD_ENV_VARS_SUFFIX_FMT, unm.machine);
        add_sys_env(z);

        sprintf(z+2, ADD_ENV_VARS_SUFFIX_FMT, unm.nodename);
        add_sys_env(z);
#else

        add_sys_env(z);
#endif
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  T H E   F O L L O W I N G   I S   D E A D   C O D E
 *
 *  Someday, I want to enable daemon code, but need lotsa time.....
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef DAEMON_ENABLED

  static bool
evalProto(char const ** ppzS, uint16_t* pProto)
{
    char const * pzS = *ppzS;

    if (IS_ALPHABETIC_CHAR(*pzS)) {
        inet_family_map_t* pMap = inet_family_map;
        do  {
            if (strncmp(pzS, pMap->pz_name, pMap->nm_len) == 0) {
                *pProto = pMap->family;
                *ppzS += pMap->nm_len;
                return true;
            }
        } while ((++pMap)->pz_name != NULL);
    }

    return IS_DEC_DIGIT_CHAR(*pzS);
}

  LOCAL void
handleSighup(int sig)
{
    redoOptions = true;
}

  static void
spawnPipe(char const * pzFile)
{
#   define S_IRW_ALL \
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
    fd_pair_t fdpair;
    char* pzIn;
    char* pzOut;

    {
        size_t len = 2 * (strlen(pzFile) + 5);
        pzIn = AGALOC(len + 5, "fifo name");
        pzOut = pzIn + sprintf(pzIn, PIPE_FIFO_IN_NAME_FMT, pzFile) + 1;
    }

    unlink(pzIn);
    if ((mkfifo(pzIn, S_IRW_ALL) != 0) && (errno != EEXIST))
        AG_CANT(PIPE_MKFIFO_NAME,    pzIn);

    (void)sprintf(pzOut, PIPE_FIFO_OUT_NAME_FMT, pzFile);
    unlink(pzOut);
    if ((mkfifo(pzOut, S_IRW_ALL) != 0) && (errno != EEXIST))
        AG_CANT(PIPE_MKFIFO_NAME,    pzOut);

    fdpair.fd_read = open(pzIn, O_RDONLY);
    if (fdpair.fd_read < 0)
        AG_CANT(PIPE_FIFO_OPEN, pzIn);

    {
        struct pollfd polls[1];
        polls[0].fd     = fdpair.fd_read;
        polls[0].events = POLLIN | POLLPRI;

        for (;;) {
            int ct = poll(polls, 1, -1);
            struct strrecvfd recvfd;
            pid_t child;

            switch (ct) {
            case -1:
                if ((errno != EINTR) || (! redoOptions))
                    goto spawnpipe_finish;

                optionRestore(&autogenOptions);
                process_ag_opts(autogenOptions.origArgCt,
                          autogenOptions.origArgVect);
                SET_OPT_DEFINITIONS(PIPE_DEFS_STDIN_STR);
                break;

            case 1:
                if ((polls[0].revents & POLLIN) == 0)
                    continue;

                child = fork();
                switch (child) {
                default:
                    waitpid(child, &ct, 0);
                    continue;

                case -1:
                    AG_CANT(PIPE_FORK_NAME, zNil);

                case 0:
                }

                if (dup2(fdpair.fd_read, STDIN_FILENO) != STDIN_FILENO)
                    AG_CANT(PIPE_DUP2_NAME_STR, PIPE_DEFS_STDIN_NAME);

                fdpair.fd_write = open(pzOut, O_WRONLY);
                if (fdpair.fd_write < 0)
                    AG_CANT(PIPE_FIFO_OPEN, pzOut);

                polls[0].fd = fdpair.fd_write;
                polls[0].events = POLLOUT;
                if (poll(polls, 1, -1) != 1)
                    AG_CANT(PIPE_POLL_NAME_STR, PIPE_WRITE_NAME_STR);

                if (dup2(fdpair.fd_write, STDOUT_FILENO) != STDOUT_FILENO)
                    AG_CANT(PIPE_DUP2_NAME_STR, pzOut);

                return;
            }
        }
    }

 spawnpipe_finish:
    unlink(pzIn);
    unlink(pzOut);
    AGFREE(pzIn);

#   undef S_IRW_ALL

    exit(AUTOGEN_EXIT_SUCCESS);
}


  static void
spawnListens(char const * pzPort, sa_family_t addr_family)
{
    int socket_fd = socket(addr_family, SOCK_STREAM, 0);
    union {
        struct sockaddr     addr;
        struct sockaddr_in  in_addr;
        struct sockaddr_un  un_addr;
    } sa;

    uint32_t        addr_len;

    if (socket_fd < 0)
        AG_CANT("socket", "AF_INET/SOCK_STREAM");

    switch (addr_family) {

    case AF_UNIX:
    {
        uint32_t p_len = strlen(pzPort);

        if (p_len > sizeof(sa.un_addr.sun_path))
            AG_ABEND(aprf(PATH_TOO_BIG, p_len));
        sa.un_addr.sun_family  = AF_UNIX;
        strncpy(sa.un_addr.sun_path, pzPort, p_len);
        addr_len = sizeof(sa.un_addr) - sizeof(sa.un_addr.sun_path) + p_len;
    }
    break;

    case AF_INET:
    {
        uint16_t port;
        char* pz;

        sa.in_addr.sin_family      = AF_INET;
        sa.in_addr.sin_addr.s_addr = INADDR_ANY;

        errno = 0;
        if ((unlink(pzPort) != 0) && (errno != ENOENT))
            AG_CANT("unlink", pzPort);

        port = (uint16_t)strtol(pzPort, &pz, 0);
        if ((errno != 0) || (*pz != NUL))
            AG_ABEND(aprf(PORT_NUM_BAD, pzPort));

        sa.in_addr.sin_port = htons((short)port);
        addr_len = sizeof(sa.in_addr);
    }
    break;

    default:
        AG_ABEND(aprf(ADDR_FAMILY_BAD, addr_family));
    }

    if (bind(socket_fd, &sa.addr, addr_len) < 0) {
        char* pz = aprf(LISTEN_PORT_FMT, pzPort, addr_family);
        AG_CANT("bind", pz);
    }

    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0)
        AG_CANT("socket-fcntl", "FNDELAY");

    if (listen(socket_fd, 5) < 0)
        AG_CANT("listen", aprf(LISTEN_PORT_FMT, pzPort));

    for (;;) {
        fd_set fds;
        int    max_fd = socket_fd;
        int    new_conns;

        FD_ZERO(&fds);
        FD_SET(socket_fd, &fds);

        new_conns = select(max_fd, &fds, NULL, NULL, NULL);
        if (new_conns < 0) {
            if (errno == EINTR)
                continue;

            if (! redoOptions) {
                unlink(pzPort);
                exit(AUTOGEN_EXIT_SUCCESS);
            }

            optionRestore(&autogenOptions);
            process_ag_opts(autogenOptions.origArgCt,
                      autogenOptions.origArgVect);
            SET_OPT_DEFINITIONS("-");

            continue;
        }

        if (new_conns > 0) {
            switch (fork()) {
            default: continue;
            case -1:
                AG_CANT("fork", zNil);

            case 0:  break;
            }
            break;
        }
    }

    for (;;) {
        static int try_ct = 0;
        struct sockaddr addr;
        socklen_t addr_len;
        int fd = accept(socket_fd, &addr, &addr_len);
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

    if (dup2(socket_fd, STDOUT_FILENO) != STDOUT_FILENO)
        AG_CANT("dup2", "out on socket_fd");
    if (dup2(socket_fd, STDIN_FILENO) != STDIN_FILENO)
        AG_CANT("dup2", "in on socket_fd");
}


  static void
daemonize(char const * pzStdin, char const * pzStdout, char const * pzStderr,
          char const * pzDaemonDir)
{
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
            fserr(AUTOGEN_EXIT_FS_ERROR, "fork", "");
            /* NOTREACHED */

        default:
            exit(AUTOGEN_EXIT_SUCCESS);

        case 0:
            break;
        }
    }

    /*
     *  Now, become a process group and session group leader.
     */
    if (setsid() == -1)
        fserr(AUTOGEN_EXIT_FS_ERROR, "setsid", "");

    /*
     *  There is now no controlling terminal.  However, if we open anything
     *  that resembles a terminal, it will become our controlling terminal.
     *  So, we will fork again and the child will not be a group leader and
     *  thus cannot gain a controlling terminal.
     */
    switch (fork()) {
    case -1:
        fserr(AUTOGEN_EXIT_FS_ERROR, "fork", "");

    default:
        exit(AUTOGEN_EXIT_SUCCESS);  /* parent process - silently go away */

    case 0:
        break;
    }

    umask(0);
    if (pzDaemonDir == (char*)NULL)
        pzDaemonDir = "/";

    chdir(pzDaemonDir);

    /*
     *  Reopen the input, output and error files, unless we were told not to
     */
    if (pzStdin != (char*)NULL)
        freopen(pzStdin,  "r", stdin);

    if (pzStdout != (char*)NULL)
        freopen(pzStdout, "w", stdout);

    if (pzStderr != (char*)NULL)
        freopen(pzStderr, "w", stderr);

    /* We are a daemon now */
}
#endif /* DAEMON_ENABLED */
/**
 * @}
 *
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agInit.c */
