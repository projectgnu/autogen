/**
 * \file agShell
 *
 *  Time-stamp:        "2010-12-09 11:55:08 bkorb"
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
#ifndef SHELL_ENABLED
 void  closeServer(void) { }

 int   chainOpen(int stdinFd, tCC** ppArgs, pid_t* pChild) { return -1; }

 pid_t openServer(tFdPair* pPair, tCC** ppArgs) { return NOPROCESS; }

 pid_t openServerFP(tpfPair* pfPair, tCC** ppArgs) { return NOPROCESS; }

 char* runShell(char const* pzCmd) {
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
server_setup(void);

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
        fprintf(pfTrace, cmd_fmt, pCurDir, pz, zShDone, log_ct);
    }
    last_cmd = NULL;
    close_server_shell();
}


static void
server_setup(void)
{
    {
        static int do_once = 0;
        if (do_once == 0) {
            char* p = malloc(AG_PATH_MAX);
            if (p == NULL)
                AG_ABEND("cannot allocate path name");

            pCurDir = (tpChar)getcwd(p, AG_PATH_MAX);

            if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
                fputs("\nServer First Start\n", pfTrace);

            do_once = 1;
        }
        else if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
            fputs("\nServer Restart\n", pfTrace);
    }

    {
        struct sigaction new_sa;
        new_sa.sa_handler = handle_signal;
        new_sa.sa_flags   = 0;
        (void)sigemptyset(&new_sa.sa_mask);
        (void)sigaction(SIGPIPE, &new_sa, NULL);
        (void)sigaction(SIGALRM, &new_sa, NULL);
    }

    was_close_err = AG_FALSE;

    {
        char* pz;
        last_cmd = shell_init_str;
        sprintf(shell_init_str + shell_init_len, "%u\n",
                (unsigned int)getpid());
        fprintf(serv_pair.pfWrite, cmd_fmt, pCurDir, last_cmd,
                zShDone, ++log_ct);

        if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
            fprintf(pfTrace, log_sep_fmt, log_ct);
            fprintf(pfTrace, cmd_fmt, pCurDir, last_cmd, zShDone, log_ct);
        }

        (void)fflush(serv_pair.pfWrite);
        pz = load_data();
        if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL)
            fputs("(result discarded)\n", pfTrace);
        AGFREE((void*)pz);
    }

    if (OPT_VALUE_TRACE >= TRACE_EVERYTHING) {
        tSCC zSetup[] = "set -x\n"
                        "trap\n"
                        "echo server setup done\n";
        char* pz;

        fputs("Server traps set\n", pfTrace);
        last_cmd = zSetup;
        fprintf(serv_pair.pfWrite, cmd_fmt, pCurDir, last_cmd,
                zShDone, ++log_ct);
        if (pfTrace != stderr) {
            fprintf(pfTrace, log_sep_fmt, log_ct);
            fprintf(pfTrace, cmd_fmt, pCurDir, last_cmd, zShDone, log_ct);
        }

        (void)fflush(serv_pair.pfWrite);
        pz = load_data();
        if (pfTrace != stderr)
            fputs("(result discarded)\n", pfTrace);
        fprintf(pfTrace, "Trap state:\n%s\n", pz);
        AGFREE((void*)pz);
    }
    last_cmd = NULL;
}


/**
 *  Given an FD for an inferior process to use as stdin,
 *  start that process and return a NEW FD that that process
 *  will use for its stdout.  Requires the argument vector
 *  for the new process and, optionally, a pointer to a place
 *  to store the child's process id.
 */
LOCAL int
chainOpen(int stdinFd, char const ** ppArgs, pid_t * pChild)
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
LOCAL pid_t
openServer(tFdPair* pPair, tCC** ppArgs)
{
    pid_t chId = NOPROCESS;

    /*
     *  Create a bi-directional pipe.  Writes on 0 arrive on 1
     *  and vice versa, so the parent and child processes will
     *  read and write to opposite FD's.
     */
    if (pipe((int*)pPair) < 0)
        return NOPROCESS;

    pPair->readFd = chainOpen(pPair->readFd, ppArgs, &chId);
    if (chId == NOPROCESS)
        close(pPair->writeFd);

    return chId;
}


/**
 *  Identical to "openServer()", except that the "fd"'s are "fdopen(3)"-ed
 *  into file pointers instead.
 */
LOCAL pid_t
openServerFP(tpfPair* pfPair, tCC** ppArgs)
{
    tFdPair   fdPair;
    pid_t     chId = openServer(&fdPair, ppArgs);

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

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf(pfTrace, "\n= = = RESULT %d bytes:\n%s%s\n"
                "= = = = = = = = = = = = = = =\n",
                (int)textSize, pzText, zLine);
    }

    *pzScan  = NUL;
    return AGREALOC((void*)pzText, textSize, "resizing text output");
}


/**
 *  Run a semi-permanent server shell.  The program will be the
 *  one named by the environment variable $SHELL, or default to "sh".
 *  If one of the commands we send to it takes too long or it dies,
 *  we will shoot it and restart one later.
 */
LOCAL char*
runShell(char const*  pzCmd)
{
    tSCC zCmdFail[] = "CLOSING SHELL SERVER - command failure:\n\t%s\n";

    /*
     *  IF the shell server process is not running yet,
     *  THEN try to start it.
     */
    if (serv_id == NULLPROCESS) {
        static char pz4_z[] = "PS4=>ag> ";
        putenv(pz4_z);
        serv_id = openServerFP(&serv_pair, serverArgs);
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
    last_cmd = pzCmd;
    fprintf(serv_pair.pfWrite, cmd_fmt, pCurDir, pzCmd, zShDone, ++log_ct);

    if (OPT_VALUE_TRACE >= TRACE_SERVER_SHELL) {
        fprintf(pfTrace, log_sep_fmt, log_ct);
        fprintf(pfTrace, cmd_fmt, pCurDir, pzCmd, zShDone, log_ct);
    }

    if (serv_pair.pfWrite != NULL)
        fflush(serv_pair.pfWrite);

    if (was_close_err) {
        fprintf(pfTrace, zCmdFail, pzCmd);
        return NULL;
    }

    /*
     *  Now try to read back all the data.  If we fail due to either
     *  a sigpipe or sigalrm (timeout), we will return the nil string.
     */
    {
        char* pz = load_data();
        if (pz == NULL) {
            fprintf(pfTrace, zCmdFail, pzCmd);
            close_server_shell();
            pz = (char*)AGALOC(1, "Text Block");

            *pz = NUL;

        } else if (was_close_err)
            fprintf(pfTrace, zCmdFail, pzCmd);

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
