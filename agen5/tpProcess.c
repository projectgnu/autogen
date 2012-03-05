
/**
 * @file tpProcess.c
 *
 *  Parse and process the template data descriptions
 *
 * Time-stamp:        "2012-03-04 19:37:00 bkorb"
 *
 * This file is part of AutoGen.
 * AutoGen Copyright (c) 1992-2012 by Bruce Korb - all rights reserved
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

static out_stack_t fpRoot = { 0, NULL, NULL, NULL };

/* = = = START-STATIC-FORWARD = = = */
static void
trace_macro(templ_t * pT, macro_t * pMac);

static void
do_stdout_tpl(templ_t * pTF);

static void
open_output(out_spec_t * spec);
/* = = = END-STATIC-FORWARD = = = */

/**
 *  Generate all the text within a block.  The caller must
 *  know the exact bounds of the block.  "pEnd" actually
 *  must point to the first entry that is *not* to be emitted.
 */
LOCAL void
generateBlock(templ_t * pT, macro_t * pMac, macro_t * pEnd)
{
    /*
     *  Set up the processing context for this block of macros.
     *  It is used by the Guile callback routines and the exception
     *  handling code.  It is all for user friendly diagnostics.
     */
    current_tpl = pT;

    while ((pMac != NULL) && (pMac < pEnd)) {
        teFuncType fc = pMac->md_code;
        if (fc >= FUNC_CT)
            fc = FTYP_BOGUS;

        if (OPT_VALUE_TRACE >= TRACE_EVERYTHING)
            trace_macro(pT, pMac);

        cur_macro = pMac;
        pMac = (*(apHdlrProc[ fc ]))(pT, pMac);
        ag_scribble_free();
    }
}

/**
 *  Print out information about the invocation of a macro
 */
static void
trace_macro(templ_t * pT, macro_t * pMac)
{
    teFuncType fc = pMac->md_code;
    if (fc >= FUNC_CT)
        fc = FTYP_BOGUS;

    fprintf(trace_fp, TRACE_MACRO_FMT, apzFuncNames[fc], pMac->md_code,
            pT->td_file, pMac->md_line);

    if (pMac->md_txt_off > 0) {
        int   ct;
        char* pz = pT->td_text + pMac->md_txt_off;
        fputs("  ", trace_fp);
        for (ct=0; ct < 32; ct++) {
            char ch = *(pz++);
            if (ch == NUL)
                break;
            if (ch == NL)
                break;
            putc(ch, trace_fp);
        }
        putc(NL, trace_fp);
    }
}

/**
 *  The template output goes to stdout.  Perhaps because output
 *  is for a CGI script.  In any case, this case must be handled
 *  specially.
 */
static void
do_stdout_tpl(templ_t * pTF)
{
    char   const *    pzRes;
    SCM    res;

    last_scm_cmd = NULL; /* We cannot be in Scheme processing */

    switch (setjmp (abort_jmp_buf)) {
    case SUCCESS:
        break;

    case PROBLEM:
        if (*oops_pfx != NUL) {
            fprintf(stdout, DO_STDOUT_TPL_ABANDONED, oops_pfx);
            oops_pfx = zNil;
        }
        fclose(stdout);
        return;

    default:
        fprintf(stdout, DO_STDOUT_TPL_BADR, oops_pfx);

    case FAILURE:
        exit(EXIT_FAILURE);
    }

    curr_sfx         = DO_STDOUT_TPL_NOSFX;
    curr_def_ctx     = root_def_ctx;
    cur_fpstack      = &fpRoot;
    fpRoot.stk_fp    = stdout;
    fpRoot.stk_fname = DO_STDOUT_TPL_STDOUT;
    fpRoot.stk_flags = FPF_NOUNLINK | FPF_STATIC_NM;
    if (OPT_VALUE_TRACE >= TRACE_EVERYTHING)
        fputs(DO_STDOUT_TPL_START_STD, trace_fp);

    /*
     *  IF there is a CGI prefix for error messages,
     *  THEN divert all output to a temporary file so that
     *  the output will be clean for any error messages we have to emit.
     */
    if (*oops_pfx == NUL)
        generateBlock(pTF, pTF->td_macros, pTF->td_macros + pTF->td_mac_ct);

    else {
        (void)ag_scm_out_push_new(SCM_UNDEFINED);

        generateBlock(pTF, pTF->td_macros, pTF->td_macros + pTF->td_mac_ct);

        /*
         *  Read back in the spooled output.  Make sure it starts with
         *  a content-type: prefix.  If not, we supply our own HTML prefix.
         */
        res   = ag_scm_out_pop(SCM_BOOL_T);
        pzRes = AG_SCM_CHARS(res);

        /* 13:  "content-type:" */
        if (strneqvcmp(pzRes, DO_STDOUT_TPL_CONTENT, 13) != 0)
            fputs(DO_STDOUT_TPL_CONTENT, stdout);

        fwrite(pzRes, AG_SCM_STRLEN(res), (size_t)1, stdout);
    }

    fclose(stdout);
}

/**
 * pop the current output spec structure.  Deallocate it and the
 * file name, too, if necessary.
 */
LOCAL out_spec_t *
nextOutSpec(out_spec_t * pOS)
{
    out_spec_t * res = pOS->os_next;

    if (pOS->os_dealloc_fmt)
        AGFREE(pOS->os_file_fmt);

    AGFREE(pOS);
    return res;
}


LOCAL void
processTemplate(templ_t* tpl)
{
    forInfo.fi_depth = 0;

    /*
     *  IF the template file does not specify any output suffixes,
     *  THEN we will generate to standard out with the suffix set to zNoSfx.
     *  With output going to stdout, we don't try to remove output on errors.
     */
    if (output_specs == NULL) {
        do_stdout_tpl(tpl);
        return;
    }

    do  {
        out_spec_t*  pOS    = output_specs;

        /*
         * We cannot be in Scheme processing.  We've either just started
         * or we've made a long jump from our own code.  If we've made a
         * long jump, we've printed a message that is sufficient and we
         * don't need to print any scheme expressions.
         */
        last_scm_cmd = NULL;

        /*
         *  HOW was that we got here?
         */
        switch (setjmp(abort_jmp_buf)) {
        case SUCCESS:
            if (OPT_VALUE_TRACE >= TRACE_EVERYTHING) {
                fprintf(trace_fp, PROC_TPL_START, pOS->os_sfx);
                fflush(trace_fp);
            }
            /*
             *  Set the output file name buffer.
             *  It may get switched inside open_output.
             */
            open_output(pOS);
            memcpy(&fpRoot, cur_fpstack, sizeof(fpRoot));
            AGFREE(cur_fpstack);
            cur_fpstack         = &fpRoot;
            curr_sfx       = pOS->os_sfx;
            curr_def_ctx     = root_def_ctx;
            cur_fpstack->stk_flags &= ~FPF_FREE;
            cur_fpstack->stk_prev  = NULL;
            generateBlock(tpl, tpl->td_macros, tpl->td_macros+tpl->td_mac_ct);

            do  {
                out_close(false);  /* keep output */
            } while (cur_fpstack->stk_prev != NULL);
            break;

        case PROBLEM:
            /*
             *  We got here by a long jump.  Close/purge the open files.
             */
            do  {
                out_close(true);  /* discard output */
            } while (cur_fpstack->stk_prev != NULL);
            last_scm_cmd = NULL; /* "problem" means "drop current output". */
            break;

        default:
            fprintf(trace_fp, PROC_TPL_BOGUS_RET, oops_pfx);
            oops_pfx = zNil;
            /* FALLTHROUGH */

        case FAILURE:
            /*
             *  We got here by a long jump.  Close/purge the open files.
             */
            do  {
                out_close(true);  /* discard output */
            } while (cur_fpstack->stk_prev != NULL);

            /*
             *  On failure (or unknown jump type), we quit the program, too.
             */
            processing_state = PROC_STATE_ABORTING;
            do pOS = nextOutSpec(pOS);
            while (pOS != NULL);
            exit(EXIT_FAILURE);
        }

        output_specs = nextOutSpec(pOS);
    } while (output_specs != NULL);
}


LOCAL void
out_close(bool purge)
{
    if ((cur_fpstack->stk_flags & FPF_NOCHMOD) == 0)
        make_readonly(fileno(cur_fpstack->stk_fp));

    if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE)
        fprintf(trace_fp, OUT_CLOSE_TRACE_WRAP, __func__,
                cur_fpstack->stk_fname);

    fclose(cur_fpstack->stk_fp);

    /*
     *  Only stdout and /dev/null are marked, "NOUNLINK"
     */
    if ((cur_fpstack->stk_flags & FPF_NOUNLINK) == 0) {
        /*
         *  IF we are told to purge the file OR the file is an AutoGen temp
         *  file, then get rid of the output.
         */
        if (purge || ((cur_fpstack->stk_flags & FPF_UNLINK) != 0))
            unlink(cur_fpstack->stk_fname);

        else {
            struct utimbuf tbuf;

            tbuf.actime  = time(NULL);
            tbuf.modtime = outfile_time;

            /*
             *  The putative start time is one second earlier than the
             *  earliest output file time, regardless of when that is.
             */
            if (outfile_time <= start_time)
                start_time = outfile_time - 1;

            utime(cur_fpstack->stk_fname, &tbuf);
        }
    }

    /*
     *  Do not deallocate statically allocated names
     */
    if ((cur_fpstack->stk_flags & FPF_STATIC_NM) == 0)
        AGFREE((void*)cur_fpstack->stk_fname);

    /*
     *  Do not deallocate the root entry.  It is not allocated!!
     */
    if ((cur_fpstack->stk_flags & FPF_FREE) != 0) {
        out_stack_t* p = cur_fpstack;
        cur_fpstack = p->stk_prev;
        AGFREE((void*)p);
    }
}


/**
 *  Figure out what to use as the base name of the output file.
 *  If an argument is not provided, we use the base name of
 *  the definitions file.
 */
static void
open_output(out_spec_t * spec)
{
    static char const write_mode[] = "w" FOPEN_BINARY_FLAG "+";

    char const * out_file = NULL;

    if (strcmp(spec->os_sfx, OPEN_OUTPUT_NULL) == 0) {
        static int const flags = FPF_NOUNLINK | FPF_NOCHMOD | FPF_TEMPFILE;
    null_open:
        open_output_file(DEV_NULL, DEV_NULL_LEN, write_mode, flags);
        return;
    }

    /*
     *  IF we are to skip the current suffix,
     *  we will redirect the output to /dev/null and
     *  perform all the work.  There may be side effects.
     */
    if (HAVE_OPT(SKIP_SUFFIX)) {
        int     ct  = STACKCT_OPT(SKIP_SUFFIX);
        const char ** ppz = STACKLST_OPT(SKIP_SUFFIX);

        while (--ct >= 0) {
            if (strcmp(spec->os_sfx, *ppz++) == 0)
                goto null_open;
        }
    }

    /*
     *  Remove any suffixes in the last file name
     */
    {
        char const * def_file = OPT_ARG(BASE_NAME);
        char   z[AG_PATH_MAX];
        const char * pst = strrchr(def_file, '/');
        char * end;

        pst = (pst == NULL) ? def_file : (pst + 1);

        /*
         *  We allow users to specify a suffix with '-' and '_', but when
         *  stripping a suffix from the "base name", we do not recognize 'em.
         */
        end = strchr(pst, '.');
        if (end != NULL) {
            size_t len = (unsigned)(end - pst);
            if (len >= sizeof(z))
                AG_ABEND("--base-name name is too long");

            memcpy(z, pst, len);
            z[ end - pst ] = NUL;
            pst = z;
        }

        /*
         *  Now formulate the output file name in the buffer
         *  provided as the input argument.
         */
        out_file = aprf(spec->os_file_fmt, pst, spec->os_sfx);
        if (out_file == NULL)
            AG_ABEND(aprf(OPEN_OUTPUT_BAD_FMT, spec->os_file_fmt, pst,
                          spec->os_sfx));
    }

    open_output_file(out_file, strlen(out_file), write_mode, 0);
    free((void *)out_file);
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/tpProcess.c */
