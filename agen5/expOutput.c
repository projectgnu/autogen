
/**
 * \file expOutput.c
 *
 *  Time-stamp:        "2010-07-08 22:34:17 bkorb"
 *
 *  This module implements the output file manipulation function
 *
 *  This file is part of AutoGen.
 *  AutoGen Copyright (c) 1992-2010 by Bruce Korb - all rights reserved
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

#ifndef S_IAMB
/*
 *  Access Mask Bits (3 special plus RWX for User Group & Others (9))
 */
#  define S_IAMB      (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
#endif

typedef struct {
    tCC*        pzSuspendName;
    tFpStack*   pOutDesc;
} tSuspendName;

static int            suspendCt   = 0;
static int            suspAllocCt = 0;
static tSuspendName*  pSuspended  = NULL;
static int            outputDepth = 1;

/* = = = START-STATIC-FORWARD = = = */
static void
addWriteAccess( char* pzFileName );
/* = = = END-STATIC-FORWARD = = = */

LOCAL void
removeWriteAccess( int fd )
{
    struct stat    sbuf;

    /*
     *  If the output is supposed to be writable, then also see if
     *  it is a temporary condition (set vs. a command line option).
     */
    if (ENABLED_OPT( WRITABLE )) {
        if (! HAVE_OPT( WRITABLE ))
            CLEAR_OPT( WRITABLE );
        return;
    }

    /*
     *  Set our usage mask to all all the access
     *  bits that do not provide for write access
     */
#   define USE_MASK ((unsigned)(~(S_IWUSR|S_IWGRP|S_IWOTH)))
    fstat( fd, &sbuf );

    /*
     *  Mask off the write permission bits, but ensure that
     *  the user read bit is set.
     */
    sbuf.st_mode = ((unsigned)sbuf.st_mode & USE_MASK) | S_IRUSR;
    fchmod( fd, sbuf.st_mode & S_IAMB );
}

static void
addWriteAccess( char* pzFileName )
{
    struct stat sbuf;

#ifdef DEBUG_ENABLED
    /*
     *  "stat(2)" does not initialize the entire structure.
     */
    memset( &sbuf, NUL, sizeof(sbuf) );
#endif

    stat( pzFileName, &sbuf );

    /*
     *  Or in the user write bit
     */
    sbuf.st_mode |= S_IWUSR;
    chmod( pzFileName, sbuf.st_mode & S_IAMB );
}


/*=gfunc out_delete
 *
 * what: delete current output file
 * doc:
 *  Remove the current output file.  Cease processing the template for
 *  the current suffix.  It is an error if there are @code{push}-ed
 *  output files.  Use the @code{(error "0")} scheme function instead.
 *  @xref{output controls}.
=*/
SCM
ag_scm_out_delete( void )
{
    tSCC zSkipMsg[] = "NOTE:  skipping file '%s'\n";

    /*
     *  Delete the current output file
     */
    if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE)
        fprintf( pfTrace, zSkipMsg, pCurFp->pzOutName );
    outputDepth = 1;
    longjmp( fileAbort, PROBLEM );
    /* NOTREACHED */
    return SCM_UNDEFINED;
}


/*=gfunc out_move
 *
 * what:   change name of output file
 * exparg: new-name, new name for the current output file
 *
 * doc:    Rename current output file.  @xref{output controls}.
 *         Please note: changing the name will not save a temporary
 *         file from being deleted.  It @i{may}, however, be used on the
 *         root output file.
=*/
SCM
ag_scm_out_move( SCM new_file )
{
    size_t sz = AG_SCM_STRLEN( new_file );
    char*  pz = (char*)AGALOC( sz + 1, "new file name string" );
    memcpy( pz, AG_SCM_CHARS( new_file ), sz );
    pz[ sz ] = NUL;

    if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE)
        fprintf( pfTrace, "renaming %s to %s\n",  pCurFp->pzOutName, pz);
    rename(pCurFp->pzOutName, pz);
    if ((pCurFp->flags & FPF_STATIC_NM) == 0)
        AGFREE( (void*)pCurFp->pzOutName );
    AGDUPSTR( pCurFp->pzOutName, pz, "file name" );
    pCurFp->flags &= ~FPF_STATIC_NM;
    return SCM_UNDEFINED;
}


/*=gfunc out_pop
 *
 * what:   close current output file
 * exparg: disp, return contents of the file, optional
 * doc:
 *  If there has been a @code{push} on the output, then close that
 *  file and go back to the previously open file.  It is an error
 *  if there has not been a @code{push}.  @xref{output controls}.
 *
 *  If there is no argument, no further action is taken.  Otherwise,
 *  the argument should be @code{#t} and the contents of the file
 *  are returned by the function.
=*/
SCM
ag_scm_out_pop(SCM ret_contents)
{
    SCM res = SCM_UNDEFINED;

    if (pCurFp->pPrev == NULL)
        AG_ABEND("ERROR:  Cannot pop output with no output pushed\n");

    if (AG_SCM_BOOL_P(ret_contents) && SCM_NFALSEP(ret_contents)) {
        unsigned long pos = ftell(pCurFp->pFile);

        if (pos != 0) {
            char* pz = ag_scribble((size_t)pos);
            rewind(pCurFp->pFile);
            if (fread(pz, (size_t)pos, (size_t)1, pCurFp->pFile) != 1)
                AG_ABEND(aprf("Error %d (%s) rereading output\n",
                              errno, strerror(errno)));
            res = AG_SCM_STR2SCM(pz, pos);
        }
    }

    if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
        fputs("out-pop\n", pfTrace);

    outputDepth--;
    closeOutput(AG_FALSE);
    return res;
}


/*=gfunc out_suspend
 *
 * what:   suspend current output file
 * exparg: suspName, A name tag for reactivating
 * doc:
 *  If there has been a @code{push} on the output, then set aside the
 *  output descriptor for later reactiviation with @code{(out-resume "xxx")}.
 *  The tag name need not reflect the name of the output file.  In fact,
 *  the output file may be an anonymous temporary file.  You may also
 *  change the tag every time you suspend output to a file, because the
 *  tag names are forgotten as soon as the file has been "resumed".
=*/
SCM
ag_scm_out_suspend( SCM suspName )
{
    if (pCurFp->pPrev == NULL)
        AG_ABEND( "ERROR:  Cannot pop output with no output pushed" );

    if (++suspendCt > suspAllocCt) {
        suspAllocCt += 8;
        if (pSuspended == NULL)
            pSuspended = (tSuspendName*)
                AGALOC( suspAllocCt * sizeof( tSuspendName ),
                        "suspended file list" );
        else
            pSuspended = (tSuspendName*)
                AGREALOC( (void*)pSuspended,
                          suspAllocCt * sizeof( tSuspendName ),
                          "augmenting suspended file list" );
    }

    pSuspended[ suspendCt-1 ].pzSuspendName = gh_scm2newstr( suspName, NULL );
    pSuspended[ suspendCt-1 ].pOutDesc      = pCurFp;
    if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
        fprintf(pfTrace, "suspended output to '%s'\n",
                pSuspended[ suspendCt-1 ].pzSuspendName);

    pCurFp = pCurFp->pPrev;
    outputDepth--;

    return SCM_UNDEFINED;
}


/*=gfunc out_resume
 *
 * what:   resume suspended output file
 * exparg: suspName, A name tag for reactivating
 * doc:
 *  If there has been a suspended output, then make that output descriptor
 *  current again.  That output must have been suspended with the same tag
 *  name given to this routine as its argument.
=*/
SCM
ag_scm_out_resume( SCM suspName )
{
    int  ix  = 0;
    tCC* pzName = ag_scm2zchars( suspName, "resume name" );

    for (; ix < suspendCt; ix++) {
        if (strcmp( pSuspended[ ix ].pzSuspendName, pzName ) == 0) {
            pSuspended[ ix ].pOutDesc->pPrev = pCurFp;
            pCurFp = pSuspended[ ix ].pOutDesc;
            free( (void*)pSuspended[ ix ].pzSuspendName ); /* Guile alloc */
            if (ix < --suspendCt)
                pSuspended[ ix ] = pSuspended[ suspendCt ];
            ++outputDepth;
            if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
                fprintf(pfTrace, "resuming output to '%s'\n", pzName);
            return SCM_UNDEFINED;
        }
    }

    AG_ABEND( aprf( "ERROR: no output file was suspended as ``%s''\n",
                    pzName ));
    /* NOTREACHED */
    return SCM_UNDEFINED;
}


/*=gfunc out_emit_suspended
 *
 * what:   emit the text of suspended output
 * exparg: suspName, A name tag of suspended output
 * doc:
 *  This function is equivalent to
 *  @code{(begin (out-resume <name>) (out-pop #t))}
=*/
SCM
ag_scm_out_emit_suspended( SCM suspName )
{
    (void)ag_scm_out_resume(suspName);
    return ag_scm_out_pop(SCM_BOOL_T);
}


/*=gfunc ag_fprintf
 *
 * what:  format to autogen stream
 * general_use:
 *
 * exparg: ag-diversion, AutoGen diversion name or number
 * exparg: format,       formatting string
 * exparg: format-arg,   list of arguments to formatting string, opt, list
 *
 * doc:  Format a string using arguments from the alist.
 *       Write to a specified AutoGen diversion.
 *       That may be either a specified suspended output stream
 *       (@pxref{SCM out-suspend}) or an index into the output stack
 *       (@pxref{SCM out-push-new}).  @code{(ag-fprintf 0 ...)} is
 *       equivalent to @code{(emit (sprintf ...))}, and
 *       @code{(ag-fprintf 1 ...)} sends output to the most recently
 *       suspended output stream.
=*/
SCM
ag_scm_ag_fprintf( SCM port, SCM fmt, SCM alist )
{
    static char const invalid_z[] = "ag-fprintf: 'port' is invalid";
    int   list_len = scm_ilength( alist );
    char* pzFmt    = ag_scm2zchars( fmt, zFormat );
    SCM   res      = run_printf( pzFmt, list_len, alist );

    /*
     *  If "port" is a string, it must match one of the suspended outputs.
     *  Otherwise, we'll fall through to the abend.
     */
    if (AG_SCM_STRING_P(port)) {
        int  ix  = 0;
        tCC* pzName = ag_scm2zchars( port, "resume name" );

        for (; ix < suspendCt; ix++) {
            if (strcmp( pSuspended[ ix ].pzSuspendName, pzName ) == 0) {
                tFpStack* pSaveFp = pCurFp;
                pCurFp = pSuspended[ ix ].pOutDesc;
                (void) ag_scm_emit(res);
                pCurFp = pSaveFp;
                return SCM_UNDEFINED;
            }
        }
    }

    /*
     *  If "port" is a number, it is an index into the output stack with "0"
     *  (zero) representing the current output and "1" the last suspended
     *  output.  If the number is out of range, we'll fall through to the
     *  abend.
     */
    else if (AG_SCM_NUM_P(port)) {
        tFpStack* pSaveFp = pCurFp;
        unsigned long val = gh_scm2ulong(port);

        for (; val > 0; val--) {
            pCurFp = pCurFp->pPrev;
            if (pCurFp == NULL) {
                pCurFp = pSaveFp;
                goto fprintf_woops;
            }
        }

        (void) ag_scm_emit(res);
        pCurFp  = pSaveFp;
        return SCM_UNDEFINED;
    }

    /*
     *  Still here?  We have a bad "port" specification.
     */
    fprintf_woops:

    AG_ABEND(invalid_z);
    /* NOTREACHED */
    return SCM_UNDEFINED;
}


/*
 * Some common code for creating a new file
 */
LOCAL void
open_output_file(char const * fname, size_t nmsz, char const * mode, int flags)
{
    char *    pz;
    tFpStack* p  = AGALOC(sizeof(*p), "out file stack");

    pz = (char*)AGALOC(nmsz + 1, "file name string");
    memcpy( pz, fname, nmsz );
    pz[ nmsz ] = NUL;
    memset(p, NUL, sizeof(*p));
    p->pzOutName = pz;

    /*
     *  IF we are creating the file and we are allowed to unlink the output,
     *  then start by unlinking the thing.
     */
    if ((*mode == 'w') && ((flags & FPF_NOUNLINK) == 0)) {
        if ((unlink(pz) != 0) && (errno != ENOENT))
            AG_CANT("unlink", pz);
    }

    p->pFile = fopen(pz, mode);
    if (p->pFile == NULL)
        AG_CANT("open for output", pz);

    p->pPrev = pCurFp;
    pCurFp   = p;
    p->flags = FPF_FREE | flags;
    outputDepth++;

    addWriteAccess(pz);

    if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE)
        fprintf(pfTrace, "open_output_file '%s' mode %s\n", fname, mode);

    if ((pfDepends != NULL) && ((flags & FPF_TEMPFILE) == 0))
        fprintf(pfDepends, " \\\n\t%s", fname);
}

/*=gfunc out_push_add
 *
 * what:   append output to file
 * exparg: file-name, name of the file to append text to
 *
 * doc:
 *  Identical to @code{push-new}, except the contents are @strong{not}
 *  purged, but appended to.  @xref{output controls}.
=*/
SCM
ag_scm_out_push_add(SCM new_file)
{
    static char const append_mode[] = "a" FOPEN_BINARY_FLAG "+";

    if (! AG_SCM_STRING_P( new_file ))
        AG_ABEND("No output file specified to add to");

    open_output_file(AG_SCM_CHARS(new_file), AG_SCM_STRLEN(new_file),
                     append_mode, 0);

    return SCM_UNDEFINED;
}


/*=gfunc out_push_new
 *
 * what:   purge and create output file
 * exparg: file-name, name of the file to create, optional
 *
 * doc:
 *  Leave the current output file open, but purge and create
 *  a new file that will remain open until a @code{pop} @code{delete}
 *  or @code{switch} closes it.  The file name is optional and, if omitted,
 *  the output will be sent to a temporary file that will be deleted when
 *  it is closed.
 *  @xref{output controls}.
=*/
SCM
ag_scm_out_push_new( SCM new_file )
{
    static char const write_mode[] = "w" FOPEN_BINARY_FLAG "+";

    if (AG_SCM_STRING_P(new_file)) {
        open_output_file( AG_SCM_CHARS(new_file), AG_SCM_STRLEN(new_file),
                          write_mode, 0);
        return SCM_UNDEFINED;
    }

#if defined(ENABLE_FMEMOPEN)
    if (! HAVE_OPT( NO_FMEMOPEN ))
#endif
    {
        static char const mk_tmp_dir[] = "(make-tmp-dir)";
        static int  const line_no      = __LINE__ - 1;

        /*
         *  IF we do not have fopencookie(3GNU) or funopen(3BSD), then this
         *  block is a pure "else" clause.  If we do have one of those, then
         *  the block is executed when a file name is not specified *and*
         *  --no-fmemopen was *not* selected.
         */
        static char * pzTemp = NULL;
        static size_t tmplen = 0;

        char *  tmp_fnm;
        int     tmpfd;

        if (pzTemp == NULL) {
            ag_scm_c_eval_string_from_file_line(mk_tmp_dir, __FILE__, line_no);
            pzTemp = runShell("echo ${tmp_dir}/ag-XXXXXX");
            tmplen = strlen(pzTemp) + 1;
        }

        tmp_fnm = ag_scribble(tmplen);
        memcpy(tmp_fnm, pzTemp, tmplen);
        tmpfd = mkstemp(tmp_fnm);
        if (tmpfd < 0)
            AG_CANT("mkstemp", pzTemp);

        open_output_file(tmp_fnm, tmplen - 1, write_mode, FPF_TEMPFILE);
        close(tmpfd);
        return SCM_UNDEFINED;
    }
#if defined(ENABLE_FMEMOPEN)

    {
        char *     pzNewFile;
        tFpStack * p;

        /*
         *  This block is used IFF ENABLE_FMEMOPEN is defined and if
         *  --no-fmemopen is selected on the command line.
         */
        p = (tFpStack*)AGALOC(sizeof(tFpStack), "out file stack");
        p->pPrev  = pCurFp;
        p->flags  = FPF_FREE;
        p->pFile  = ag_fmemopen(NULL, (size_t)0, "wb+");
        pzNewFile = (char*)zMemFile;
        p->flags |= FPF_STATIC_NM | FPF_NOUNLINK | FPF_NOCHMOD;

        if (p->pFile == NULL)
            AG_CANT("open 'wb+'", pzNewFile);

        p->pzOutName = pzNewFile;
        outputDepth++;
        pCurFp    = p;

        if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE)
            fprintf(pfTrace, "out-push-new on temp file\n");
    }
#endif /* ENABLE_FMEMOPEN */

    return SCM_UNDEFINED;
}


/*=gfunc out_switch
 *
 * what:   close and create new output
 * exparg: file-name, name of the file to create
 *
 * doc:
 *  Switch output files - close current file and make the current
 *  file pointer refer to the new file.  This is equivalent to
 *  @code{out-pop} followed by @code{out-push-new}, except that
 *  you may not pop the base level output file, but you may
 *  @code{switch} it.  @xref{output controls}.
=*/
SCM
ag_scm_out_switch( SCM new_file )
{
    struct utimbuf tbuf;
    char*  pzNewFile;

    if (! AG_SCM_STRING_P( new_file ))
        return SCM_UNDEFINED;
    {
        size_t sz = AG_SCM_STRLEN( new_file );
        pzNewFile = (char*)AGALOC( sz + 1, "new file name string" );
        memcpy( pzNewFile, AG_SCM_CHARS( new_file ), sz );
        pzNewFile[ sz ] = NUL;
    }

    /*
     *  IF no change, THEN ignore this
     */
    if (strcmp( pCurFp->pzOutName, pzNewFile ) == 0) {
        AGFREE( (void*)pzNewFile );
        return SCM_UNDEFINED;
    }

    removeWriteAccess( fileno( pCurFp->pFile ));

    /*
     *  Make sure we get a new file pointer!!
     *  and try to ensure nothing is in the way.
     */
    unlink( pzNewFile );
    if (  freopen( pzNewFile, "w" FOPEN_BINARY_FLAG "+", pCurFp->pFile )
       != pCurFp->pFile)

        AG_CANT("freopen", pzNewFile);

    /*
     *  Set the mod time on the old file.
     */
    tbuf.actime  = time( NULL );
    tbuf.modtime = outTime;
    utime( pCurFp->pzOutName, &tbuf );
    if (OPT_VALUE_TRACE > TRACE_DEBUG_MESSAGE)
        fprintf(pfTrace, "switching output from %s to '%s'\n",
                pCurFp->pzOutName, pzNewFile);
    pCurFp->pzOutName = pzNewFile;  /* memory leak */

    return SCM_UNDEFINED;
}


/*=gfunc out_depth
 *
 * what: output file stack depth
 * doc:  Returns the depth of the output file stack.
 *       @xref{output controls}.
=*/
SCM
ag_scm_out_depth( void )
{
    return AG_SCM_INT2SCM( outputDepth );
}


/*=gfunc out_name
 *
 * what: current output file name
 * doc:  Returns the name of the current output file.  If the current file
 *       is a temporary, unnamed file, then it will scan up the chain until
 *       a real output file name is found.
 *       @xref{output controls}.
=*/
SCM
ag_scm_out_name( void )
{
    tFpStack* p = pCurFp;
    while (p->flags & FPF_UNLINK)  p = p->pPrev;
    return AG_SCM_STR02SCM( (void*)p->pzOutName );
}


/*=gfunc out_line
 *
 * what: output file line number
 * doc:  Returns the current line number of the output file.
 *       It rewinds and reads the file to count newlines.
=*/
SCM
ag_scm_out_line( void )
{
    int lineNum = 1;

    do {
        long svpos = ftell( pCurFp->pFile );
        long pos   = svpos;

        if (pos == 0)
            break;

        rewind(pCurFp->pFile);
        do {
            int ich = fgetc( pCurFp->pFile );
            unsigned char ch = ich;
            if (ich < 0)
                break;
            if (ch == (unsigned char)'\n')
                lineNum++;
        } while (--pos > 0);
        fseek(pCurFp->pFile, svpos, SEEK_SET);
    } while(0);

    return AG_SCM_INT2SCM( lineNum );
}

#if 0 /* for compilers that do not like C++ comments... */
// /*=gfunc   make_header_guard
//  *
//  * what:   make self-inclusion guard
//  *
//  * exparg: name , header group name
//  *
//  * doc:
//  * This function will create a @code{#ifndef}/@code{#define}
//  * sequence for protecting a header from multiple evaluation.
//  * It will also set the Scheme variable @code{header-file}
//  * to the name of the file being protected and it will set
//  * @code{header-guard} to the name of the @code{#define} being
//  * used to protect it.  It is expected that this will be used
//  * as follows:
//  * @example
//  * [+ (make-header-guard "group_name") +]
//  * ...
//  * #endif /* [+ (. header-guard) +] */
//  *
//  * #include "[+ (. header-file)  +]"
//  * @end example
//  * @noindent
//  * The @code{#define} name is composed as follows:
//  *
//  * @enumerate
//  * @item
//  * The first element is the string argument and a separating underscore.
//  * @item
//  * That is followed by the name of the header file with illegal
//  * characters mapped to underscores.
//  * @item
//  * The end of the name is always, "@code{_GUARD}".
//  * @item
//  * Finally, the entire string is mapped to upper case.
//  * @end enumerate
//  *
//  * The final @code{#define} name is stored in an SCM symbol named
//  * @code{header-guard}.  Consequently, the concluding @code{#endif} for the
//  * file should read something like:
//  *
//  * @example
//  * #endif /* [+ (. header-guard) +] */
//  * @end example
//  *
//  * The name of the header file (the current output file) is also stored
//  * in an SCM symbol, @code{header-file}.  Therefore, if you are also
//  * generating a C file that uses the previously generated header file,
//  * you can put this into that generated file:
//  *
//  * @example
//  * #include "[+ (. header-file) +]"
//  * @end example
//  *
//  * Obviously, if you are going to produce more than one header file from
//  * a particular template, you will need to be careful how these SCM symbols
//  * get handled.
// =*/
#endif
SCM
ag_scm_make_header_guard(SCM name)
{

    char const * opz; // output file name string
    size_t       osz;

    char const * gpz; // guard name string
    size_t       gsz;

    {
        tFpStack* p = pCurFp;
        while (p->flags & FPF_UNLINK)  p = p->pPrev;
        opz = p->pzOutName;
        osz = strlen(opz);
    }

    /*
     *  Construct the gard name using the leader (passed in or "HEADER")
     *  and the trailer (always "_GUARD") and the output file name in between.
     */
    {
        static char const hdr[] = "HEADER";
        static char const grd[] = "_GUARD";

        /*
         *  Leader string and its length.  Usually passed, but defaults
         *  to "HEADER"
         */
        char const * lpz = AG_SCM_STRING_P(name) ? AG_SCM_CHARS(name) : hdr;
        size_t lsz = (lpz == hdr) ? (sizeof(hdr) - 1) : AG_SCM_STRLEN(name);

        /*
         *  Full, maximal length of output
         */
        size_t hsz = lsz + osz + sizeof(grd) + 1;
        char * scan_p;

        /*
         * Sanity:
         */
        if (*lpz == NUL) {
            lpz = hdr;
            lsz = sizeof(hdr) - 1;
            hsz += lsz;
        }
        scan_p = AGALOC(hsz, "header guard string");

        gpz = scan_p;
        do  {
            *(scan_p++) = toupper(*(lpz++));
        } while (--lsz > 0);

        /*
         *  This copy converts non-alphanumerics to underscores,
         *  but never inserts more than one at a time.  Thus, we may
         *  not use all of the space in "gpz".
         */
        lpz = opz;
        do  {
            *(scan_p++) = '_';
            while ((! IS_ALPHANUMERIC_CHAR(*lpz)) && (*lpz != NUL))  lpz++;
            while (IS_ALPHANUMERIC_CHAR(*lpz))
                *(scan_p++) = toupper(*(lpz++));
        } while (*lpz != NUL);

        memcpy(scan_p, grd, sizeof(grd));
        gsz = (scan_p - gpz) + sizeof(grd) - 1;
    }

    {
        static int  const line_no = __LINE__ + 2;
        static char const setvar[] =
            "(set! header-file \"%s\") "
            "(set! header-guard \"%s\")";
        char z[sizeof(setvar) + SCRIBBLE_SIZE];
        char * p = z;
        size_t cmdsz = sizeof(setvar) + gsz + osz;
        if (cmdsz > sizeof(z))
            p = AGALOC(cmdsz, "Guile set cmd");
        snprintf(p, cmdsz, setvar, opz, gpz);
        (void)ag_scm_c_eval_string_from_file_line(
            p, __FILE__, line_no);
        if (p != z)
            AGFREE(p);
    }

    {
        static char const ifndef[] = "#ifndef %1$s\n#define %1$s 1";
        char z[sizeof(ifndef) + SCRIBBLE_SIZE];
        char * p = z;
        size_t strsz = sizeof(ifndef) + (2 * gsz);
        if (strsz > sizeof(z))
            p = AGALOC(strsz, "ifndef guard");
        snprintf(p, strsz, ifndef, gpz);
        name = AG_SCM_STR02SCM( p );
        if (p != z)
            AGFREE(p);
    }

    AGFREE(gpz);
    return (name);
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expOutput.c */
