/**
 * @file agShell.c
 *
 *  Time-stamp:        "2011-12-21 10:58:06 bkorb"
 *
 *  Manage a server shell process
 *
 *  This file is part of AutoGen.
 *  AutoGen Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
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
static char * cur_dir = NULL;
static char const cmd_fail_fmt[] =
    "CLOSING SHELL SERVER - command failure:\n\t%s\n";

/*=gfunc chdir
 *
 * what:   Change current directory
 *
 * exparg: dir, new directory name
 *
 * doc:  Sets the current directory for AutoGen.  Shell commands will run
 *       from this directory as well.  This is a wrapper around the Guile
 *       native function.  It returns its directory name argument and
 *       fails the program on failure.
=*/
SCM
ag_scm_chdir(SCM dir)
{
    static char const zChdirDir[] = "chdir directory";

    scm_chdir(dir);

    /*
     *  We're still here, so we have a valid argument.
     */
    if (cur_dir != NULL)
        free(cur_dir);
    {
        char const * pz = ag_scm2zchars(dir, zChdirDir);
        cur_dir = malloc(AG_SCM_STRLEN(dir) + 1);
        strcpy((char*)cur_dir, pz);
    }
    return dir;
}

/*=gfunc shell
 *
 * what:  invoke a shell script
 * general_use:
 *
 * exparg: command, shell command - the result value is the stdout output.
 *
 * doc:
 *  Generate a string by writing the value to a server shell and reading the
 *  output back in.  The template programmer is responsible for ensuring that
 *  it completes within 10 seconds.  If it does not, the server will be
 *  killed, the output tossed and a new server started.
 *
 *  Please note: This is the same server process used by the '#shell'
 *  definitions directive and backquoted @code{`} definitions.  There may be
 *  left over state from previous shell expressions and the @code{`}
 *  processing in the declarations.  However, a @code{cd} to the original
 *  directory is always issued before the new command is issued.
 *
 *  Also note:  When initializing, autogen will set the environment
 *  variable "AGexe" to the full path of the autogen executable.
=*/
SCM
ag_scm_shell(SCM cmd)
{
#ifndef SHELL_ENABLED
    return cmd;
#else
    if (! AG_SCM_STRING_P(cmd))
        return SCM_UNDEFINED;
    {
        char* pz = shell_cmd(ag_scm2zchars(cmd, "command"));
        cmd   = AG_SCM_STR02SCM(pz);
        AGFREE((void*)pz);
        return cmd;
    }
#endif
}

/*=gfunc shellf
 *
 * what:  format a string, run shell
 * general_use:
 *
 * exparg: format, formatting string
 * exparg: format-arg, list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist,
 *       then send the result to the shell for interpretation.
=*/
SCM
ag_scm_shellf(SCM fmt, SCM alist)
{
    int   len = scm_ilength(alist);
    char* pz;

#ifdef DEBUG_ENABLED
    if (len < 0)
        AG_ABEND("invalid alist to shellf");
#endif

    pz  = ag_scm2zchars(fmt, "format");
    fmt = run_printf(pz, len, alist);

#ifdef SHELL_ENABLED
    pz  = shell_cmd(ag_scm2zchars(fmt, "shell script"));
    fmt = AG_SCM_STR02SCM(pz);
    AGFREE((void*)pz);
#endif
    return fmt;
}

#ifndef SHELL_ENABLED
HIDE_FN(void closeServer(void) {;})

HIDE_FN(char * shell_cmd(char const* pzCmd)) {
     char* pz;
     AGDUPSTR(pz, pzCmd, "dummy shell command");
     return pz;
}
#else

/*
 *  Dual pipe opening of a child process
 */
static tpfPair      serv_pair     = { NULL, NULL };
static pid_t        serv_id       = NULLPROCESS;
static ag_bool      was_close_err = AG_FALSE;
static int          log_ct        = 0;
static char const   log_sep_fmt[] = "\n\n* * * * LOG ENTRY %d * * * *\n";
static char const   cmd_fmt[]     = "cd %s\n%s\n\necho\necho %s - %d\n";
static char const * last_cmd      = NULL;

/* = = = START-STATIC-FORWARD = = = */
static void
handle_signal(int signo);

static void
set_orig_dir(void);

static ag_bool
send_cmd_ok(char const * cmd);

static void
start_server_cmd_trace(void);

static void
send_server_init_cmds(void);

static void
server_setup(void);

static int
chain_open(int stdinFd, char const ** ppArgs, pid_t * pChild);

static pid_t
server_open(tFdPair* pPair, char const ** ppArgs);

static pid_t
server_fp_open(tpfPair* pfPair, char const ** ppArgs);

static void
realloc_text(char ** p_txt, size_t * p_sz, size_t need_len);

static char*
load_data(void);
/* = = = END-STATIC-FORWARD = = = */

LOCAL void
close_server_shell(void)
{
    if (serv_id == NULLPROCESS)
        return;

    (void)kill(serv_id, SIGTERM);
#ifdef HAVE_USLEEP
    usleep(100000); /* 1/10 of a second */
#else
    sleep(1);
#endif
    (void)kill(serv_id, SIGKILL);
    serv_id = NULLPROCESS;

    /*
     *  This guard should not be necessary.  However, sometimes someone
     *  holds an allocation pthread lock when a seg fault occurrs.  fclose
     *  needs that lock, so we hang waiting for it.  Oops.  So, when we
     *  are aborting, we just let the OS close these file descriptors.
     */
    if (procState != PROC_STATE_ABORTING) {
        (void)fclose(serv_pair.pfRead);
        /*
         *  This is _completely_ wrong, but sometimes there are data left
         *  hanging about that gets sucked up by the _next_ server shell
         *  process.  That should never, ever be in any way possible, but
         *  it is the only explanation for a second server shell picking up
         *  the initialization string twice.  It must be a broken timing
         *  issue in the Linux stdio code.  I have no other explanation.
         */
        fflush(serv_pair.pfWrite);
        (void)fclose(serv_pair.pfWrite);
    }

    serv_pair.pfRead = serv_pair.pfWrite = NULL;
}

/**
 * handle SIGALRM and SIGPIPE signals while waiting for server shell
 * responses.
 */
static void
handle_signal(int signo)
{
    static int timeout_limit = 5;
    if ((signo == SIGALRM) && (--timeout_limit <= 0))
        AG_ABEND("Server shell timed out 5 times");

    fprintf(pfTrace, "Closing server:  %s signal (%d) received\n",
            strsignal(signo), signo);
    was_close_err = AG_TRUE;

    (void)fputs("\nLast command issued:\n", pfTrace);
    {
        char const* pz = (last_cmd == NULL)
            ? "?? unknown ??\n" : last_cmd;
        fprintf(pfTrace, cmd_fmt, cur_dir, pz, zShDone, log_ct);
    }
    last_cmd = NULL;
    close_server_shell();
}

/**
 * first time starting a server shell, we get our current directory.
 * That value is kept, but may be changed via a (chdir "...") scheme call.
 */
static void
set_orig_dir(void)
{
    char * p = malloc(AG_PATH_MAX);
    if (p == NULL)
        AG_ABEND("cannot allocate path name");

    cur_dir = getcwd(p, AG_PATH_MAX);

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
        fputs("\nServer First Start\n", pfTrace);
}

/**
 * Send a command string down to the server shell
 */
static ag_bool
send_cmd_ok(char const * cmd)
{
    last_cmd = cmd;
    fprintf(serv_pair.pfWrite, cmd_fmt, cur_dir, last_cmd,
            zShDone, ++log_ct);

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf(pfTrace, log_sep_fmt, log_ct);
        fprintf(pfTrace, cmd_fmt, cur_dir, last_cmd,
                zShDone, log_ct);
    }

    (void)fflush(serv_pair.pfWrite);
    if (was_close_err)
        fprintf(pfTrace, cmd_fail_fmt, cmd);
    return ! was_close_err;
}

/**
 * Tracing level is TRACE_EVERYTHING, so send the server shell
 * various commands to start "set -x" tracing and display the
 * trap actions.
 */
static void
start_server_cmd_trace(void)
{
    static char const set_xtrace[] =
        "set -x\n"
        "trap\n"
        "echo server setup done\n";

    fputs("Server traps set\n", pfTrace);
    if (send_cmd_ok(set_xtrace)) {
        char * pz = load_data();
        fputs("(result discarded)\n", pfTrace);
        fprintf(pfTrace, "Trap state:\n%s\n", pz);
        AGFREE((void*)pz);
    }
}

/**
 * Send down the initialization string with our PID in it, as well
 * as the full path name of the autogen executable.
 */
static void
send_server_init_cmds(void)
{
    was_close_err = AG_FALSE;

    {
        char * pzc = AGALOC(sizeof(shell_init_str)
                            + 11 // log10(1 << 32) + 1
                            + strlen(autogenOptions.pzProgPath),
                            "server init");
        sprintf(pzc, shell_init_str, (unsigned int)getpid(),
                autogenOptions.pzProgPath,
                (pfDepends == NULL) ? "" : pzDepFile);

        if (send_cmd_ok(pzc))
            AGFREE((void*)load_data());
        AGFREE(pzc);
    }

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
        fputs("(result discarded)\n", pfTrace);

    if (OPT_VALUE_TRACE >= TRACE_EVERYTHING)
        start_server_cmd_trace();
}

/**
 * Perform various initializations required when starting
 * a new server shell process.
 */
static void
server_setup(void)
{
    if (cur_dir == NULL)
        set_orig_dir();
    else if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
        fputs("\nServer Restart\n", pfTrace);

    {
        struct sigaction new_sa;
        new_sa.sa_handler = handle_signal;
        new_sa.sa_flags   = 0;
        (void)sigemptyset(&new_sa.sa_mask);
        (void)sigaction(SIGPIPE, &new_sa, NULL);
        (void)sigaction(SIGALRM, &new_sa, NULL);
    }

    send_server_init_cmds();

    last_cmd = NULL;
}

/**
 *  Given an FD for an inferior process to use as stdin,
 *  start that process and return a NEW FD that that process
 *  will use for its stdout.  Requires the argument vector
 *  for the new process and, optionally, a pointer to a place
 *  to store the child's process id.
 *
 * @param stdinFd the file descriptor for the process' stdin
 * @param ppArgs  The program and argument vector
 * @param pChild  where to stash the child process PID
 *
 * @returns the read end of a pipe the child process uses for stdout
 */
static int
chain_open(int stdinFd, char const ** ppArgs, pid_t * pChild)
{
    tFdPair   stdoutPair = { -1, -1 };
    pid_t     chId;
    char const *      pzShell;

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

        pzShell = pzShellProgram;
        *ppArgs = pzShell;
    }

    /*
     *  Create a pipe it will be the child process' stdout,
     *  and the parent will read from it.
     */
    if (pipe((int*)&stdoutPair) < 0) {
        if (pChild != NULL)
            *pChild = NOPROCESS;
        return -1;
    }

    fflush(stdout);
    fflush(stderr);
    fflush(pfTrace);

    /*
     *  Call fork() and see which process we become
     */
    chId = fork();
    switch (chId) {
    case NOPROCESS:    /* parent - error in call */
        close(stdinFd);
        close(stdoutPair.readFd);
        close(stdoutPair.writeFd);
        if (pChild != NULL)
            *pChild = NOPROCESS;
        return -1;

    default:           /* parent - return opposite FD's */
        if (pChild != NULL)
            *pChild = chId;

        close(stdinFd);
        close(stdoutPair.writeFd);
        if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
            fprintf(pfTrace, "Server shell is pid %u\n", (unsigned int)chId);

        fflush(pfTrace);
        return stdoutPair.readFd;

    case NULLPROCESS:  /* child - continue processing */
        break;
    }

    /*
     *  Close the pipe end handed back to the parent process,
     *  plus stdin and stdout.
     */
    close(stdoutPair.readFd);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    /*
     *  Set stdin/out to the fd passed in and the write end of our new pipe.
     */
    fcntl(stdoutPair.writeFd, F_DUPFD, STDOUT_FILENO);
    fcntl(stdinFd,            F_DUPFD, STDIN_FILENO);

    /*
     *  set stderr to our trace file (if not stderr).
     */
    if (pfTrace != stderr) {
        close(STDERR_FILENO);
        fcntl(fileno(pfTrace), F_DUPFD, STDERR_FILENO);
    }

    /*
     *  Make the output file unbuffered only.
     *  We do not want to wait for shell output buffering.
     */
    setvbuf(stdout, NULL, _IONBF, (size_t)0);

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf(pfTrace, "Server shell %s starts\n", pzShell);

        fflush(pfTrace);
    }

    execvp((char*)pzShell, (char**)ppArgs);
    AG_CANT("execvp", pzShell);
    /* NOTREACHED */
    return -1;
}


/**
 *  Given a pointer to an argument vector, start a process and
 *  place its stdin and stdout file descriptors into an fd pair
 *  structure.  The "writeFd" connects to the inferior process
 *  stdin, and the "readFd" connects to its stdout.  The calling
 *  process should write to "writeFd" and read from "readFd".
 *  The return value is the process id of the created process.
 */
static pid_t
server_open(tFdPair* pPair, char const ** ppArgs)
{
    pid_t chId = NOPROCESS;

    /*
     *  Create a bi-directional pipe.  Writes on 0 arrive on 1
     *  and vice versa, so the parent and child processes will
     *  read and write to opposite FD's.
     */
    if (pipe((int*)pPair) < 0)
        return NOPROCESS;

    pPair->readFd = chain_open(pPair->readFd, ppArgs, &chId);
    if (chId == NOPROCESS)
        close(pPair->writeFd);

    return chId;
}


/**
 *  Identical to "server_open()", except that the "fd"'s are "fdopen(3)"-ed
 *  into file pointers instead.
 */
static pid_t
server_fp_open(tpfPair* pfPair, char const ** ppArgs)
{
    tFdPair   fdPair;
    pid_t     chId = server_open(&fdPair, ppArgs);

    if (chId == NOPROCESS)
        return chId;

    pfPair->pfRead  = fdopen(fdPair.readFd,  "r" FOPEN_BINARY_FLAG);
    pfPair->pfWrite = fdopen(fdPair.writeFd, "w" FOPEN_BINARY_FLAG);
    return chId;
}

static void
realloc_text(char ** p_txt, size_t * p_sz, size_t need_len)
{
    size_t sz = (*p_sz + need_len + 0xFFF) & ~0xFFF;
    void * p = AGREALOC((void*)*p_txt, sz, "expanding text");
    if (p == NULL)
        AG_ABEND(aprf(zAllocWhat, sz, "Realloc Text Block"));

    *p_txt = p;
    *p_sz  = sz;
}

/**
 *  Read data from a file pointer (a pipe to a process in this context)
 *  until we either get EOF or we get a marker line back.
 *  The read data are stored in a malloc-ed string that is truncated
 *  to size at the end.  Input is assumed to be an ASCII string.
 */
static char*
load_data(void)
{
    char*   pzText;
    size_t  textSize = 4096;
    size_t  usedCt   = 0;
    char*   pzScan;
    char    zLine[ 1024 ];
    int     retryCt = 0;

    pzScan   = \
        pzText = AGALOC(textSize, "Text Block");

    *pzText  = NUL;

    for (;;) {
        char * line_p;

        /*
         *  Set a timeout so we do not wait forever.  Sometimes we don't wait
         *  at all and we should.  Retry in those cases (but not on EOF).
         */
        alarm((unsigned int)OPT_VALUE_TIMEOUT);
        line_p = fgets(zLine, (int)sizeof(zLine), serv_pair.pfRead);
        alarm(0);

        if (line_p == NULL) {
            /*
             *  Guard against a server timeout
             */
            if (serv_id == NULLPROCESS)
                break;

            if ((OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) || (retryCt++ > 0))
                fprintf(pfTrace, "fs err %d (%s) reading from server shell\n",
                         errno, strerror(errno));

            if (feof(serv_pair.pfRead) || (retryCt > 32))
                break;

            continue;  /* no data - retry */
        }

        /*
         *  Check for magic character sequence indicating 'DONE'
         */
        if (strncmp(zLine, zShDone, STRSIZE(zShDone)) == 0)
            break;

        {
            size_t llen = strlen(zLine);
            if (textSize <= usedCt + llen) {
                realloc_text(&pzText, &textSize, llen);
                pzScan = pzText + usedCt;
            }

            memcpy(pzScan, zLine, llen);
            usedCt += llen;
            pzScan += llen;
        }

        /*
         *  Stop now if server timed out or if we are at EOF
         */
        if ((serv_id == NULLPROCESS) || feof(serv_pair.pfRead)) {
            fputs("feof on data load\n", pfTrace);
            break;
        }
    }

    /*
     *  Trim off all trailing white space and shorten the buffer
     *  to the size actually used.
     */
    while ((pzScan > pzText) && IS_WHITESPACE_CHAR(pzScan[-1])) pzScan--;
    textSize = (pzScan - pzText) + 1;
    *pzScan  = NUL;

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf(pfTrace, "\n= = = RESULT %d bytes:\n%s%s\n"
                "= = = = = = = = = = = = = = =\n",
                (int)textSize, pzText, zLine);
    }

    return AGREALOC((void*)pzText, textSize, "resizing text output");
}


/**
 *  Run a semi-permanent server shell.  The program will be the
 *  one named by the environment variable $SHELL, or default to "sh".
 *  If one of the commands we send to it takes too long or it dies,
 *  we will shoot it and restart one later.
 */
LOCAL char*
shell_cmd(char const * pzCmd)
{
    /*
     *  IF the shell server process is not running yet,
     *  THEN try to start it.
     */
    if (serv_id == NULLPROCESS) {
        static char const ps4_z[] = "PS4=>${FUNCNAME:-ag}> ";
        putenv((char *)ps4_z);
        serv_id = server_fp_open(&serv_pair, serverArgs);
        if (serv_id > 0)
            server_setup();
    }

    /*
     *  IF it is still not running,
     *  THEN return the nil string.
     */
    if (serv_id <= 0) {
        char* pz = (char*)AGALOC(1, "Text Block");

        *pz = NUL;
        return pz;
    }

    /*
     *  Make sure the process will pay attention to us,
     *  send the supplied command, and then
     *  have it output a special marker that we can find.
     */
    if (! send_cmd_ok(pzCmd))
        return NULL;

    /*
     *  Now try to read back all the data.  If we fail due to either
     *  a sigpipe or sigalrm (timeout), we will return the nil string.
     */
    {
        char* pz = load_data();
        if (pz == NULL) {
            fprintf(pfTrace, cmd_fail_fmt, pzCmd);
            close_server_shell();
            pz = (char*)AGALOC(1, "Text Block");

            *pz = NUL;

        } else if (was_close_err)
            fprintf(pfTrace, cmd_fail_fmt, pzCmd);

        last_cmd = NULL;
        return pz;
    }
}

#endif /* ! SHELL_ENABLED */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/agShell.c */
