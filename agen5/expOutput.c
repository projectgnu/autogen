
/*
 *  $Id: expOutput.c,v 4.2 2005/01/08 22:56:19 bkorb Exp $
 *
 *  This module implements the output file manipulation function
 */

/*
 *  AutoGen copyright 1992-2004 Bruce Korb
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
/* static forward declarations maintained by :mkfwd */
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
#   define USE_MASK ~(S_IWUSR|S_IWGRP|S_IWOTH)
    fstat( fd, &sbuf );

    /*
     *  Mask off the write permission bits, but ensure that
     *  the user read bit is set.
     */
    sbuf.st_mode = (sbuf.st_mode & USE_MASK) | S_IRUSR;
    fchmod( fd, sbuf.st_mode & S_IAMB );
}

static void
addWriteAccess( char* pzFileName )
{
    struct stat    sbuf;
    stat( pzFileName, &sbuf );
    /*
     *  Or in the user write bit
     */
    sbuf.st_mode = sbuf.st_mode | S_IWUSR;
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
    size_t sz = SCM_LENGTH( new_file );
    char*  pz = (char*)AGALOC( sz + 1, "new file name string" );
    memcpy( pz, SCM_CHARS( new_file ), sz );
    pz[ sz ] = NUL;

    rename( pCurFp->pzOutName, pz );
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
ag_scm_out_pop( SCM ret_contents )
{
    SCM res = SCM_UNDEFINED;

    if (pCurFp->pPrev == NULL)
        AG_ABEND( "ERROR:  Cannot pop output with no output pushed\n" );

    if (gh_boolean_p( ret_contents ) && SCM_NFALSEP( ret_contents )) {
        long pos = ftell( pCurFp->pFile );
        res = scm_makstr( pos, 0 );
        if (pos != 0) {
            rewind( pCurFp->pFile );
            if (fread( SCM_CHARS( res ), pos, 1, pCurFp->pFile ) != 1)
                AG_ABEND( aprf( "Error %d (%s) rereading output\n",
                                errno, strerror( errno )));
        }
    }

    outputDepth--;
    closeOutput( AG_FALSE );
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
    pCurFp = pCurFp->pPrev;
    outputDepth--;

    return SCM_UNDEFINED;
}


/*=gfunc out_resume
 *
 * what:   resume current output file
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
            return SCM_UNDEFINED;
        }
    }

    AG_ABEND( aprf( "ERROR: no output file was suspended as ``%s''\n",
                    pzName ));
    /* NOTREACHED */
    return SCM_UNDEFINED;
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
ag_scm_out_push_add( SCM new_file )
{
    tFpStack* p;
    char*  pzNewFile;

    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;

    {
        size_t sz = SCM_LENGTH( new_file );
        pzNewFile = (char*)AGALOC( sz + 1, "append file name string" );
        memcpy( pzNewFile, SCM_CHARS( new_file ), sz );
        pzNewFile[ sz ] = NUL;
    }

    addWriteAccess( pzNewFile );

    {
        FILE* fp = fopen( pzNewFile, "a" FOPEN_BINARY_FLAG "+" );

        if (fp == NULL)
            AG_ABEND( aprf( zCannot, errno, "open for write", pzNewFile,
                            strerror( errno )));
        p = (tFpStack*)AGALOC( sizeof( tFpStack ), "append - out file stack" );
        p->pFile = fp;
    }

    p->pPrev     = pCurFp;
    p->flags     = FPF_FREE;
    p->pzOutName = pzNewFile;
    outputDepth++;
    pCurFp       = p;
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
    tFpStack* p;
    char*  pzNewFile;

    p = (tFpStack*)AGALOC( sizeof( tFpStack ), "new - out file stack" );
    p->pPrev = pCurFp;
    p->flags = FPF_FREE;

    if (gh_string_p( new_file )) {
        size_t sz = SCM_LENGTH( new_file );
        pzNewFile = (char*)AGALOC( sz + 1, "new file name string" );
        memcpy( pzNewFile, SCM_CHARS( new_file ), sz );
        pzNewFile[ sz ] = NUL;
        unlink( pzNewFile );
        addWriteAccess( pzNewFile );
        p->pFile = fopen( pzNewFile, "a" FOPEN_BINARY_FLAG "+" );
    }
#   if defined(ENABLE_FMEMOPEN)
    else if (! HAVE_OPT( NO_FMEMOPEN ))
#   else
    else
#   endif
    {
        /*
         *  IF we do not have fopencookie(3GNU) or funopen(3BSD), then this
         *  block is a pure "else" clause.  If we do have one of those, then
         *  the block is executed when a file name is not s specified *and*
         *  --no-fmemopen was *not* selected.
         */
        tSCC* pzTemp = NULL;
        int tmpfd;

        if (pzTemp == NULL) {
            char* pz = getenv( "TEMP" );
            if (pz == NULL) {
                pz = getenv( "TMP" );
                if (pz == NULL)
                    pz = "/tmp";
            }

            pzTemp = aprf( "%s/agtmp.XXXXXX", pz );
            TAGMEM( pzTemp, "Saved temp file template" );
        }

        AGDUPSTR( pzNewFile, pzTemp, "temp file name" );
        p->flags |= FPF_UNLINK;
        tmpfd = mkstemp( pzNewFile );
        if (tmpfd < 0)
            AG_ABEND( aprf( "failed to create temp file from `%s'", pzTemp ));

        p->pFile  = fopen( pzNewFile, "w" FOPEN_BINARY_FLAG "+" );
        close( tmpfd );
    }
#   ifdef ENABLE_FMEMOPEN
    else {
        /*
         *  This block is a pure "else" clause that is only compiled if neither
         *  "fopencookie" nor "funopen" is available in the local C library.
         *  Otherwise, anonymous output without --no-fmemopen being selected
         *  will get us here.
         */
        p->pFile  = fmemopen( NULL, 0, "wb+" );
        pzNewFile = "in-mem buffer";
        p->flags |= FPF_STATIC_NM | FPF_NOUNLINK | FPF_NOCHMOD;
    }
#   endif /* ENABLE_FMEMOPEN */

    if (p->pFile == NULL)
        AG_ABEND( aprf( zCannot, errno, "open for write", pzNewFile,
                        strerror( errno )));

    p->pzOutName = pzNewFile;
    outputDepth++;
    pCurFp    = p;
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

    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;
    {
        size_t sz = SCM_LENGTH( new_file );
        pzNewFile = (char*)AGALOC( sz + 1, "new file name string" );
        memcpy( pzNewFile, SCM_CHARS( new_file ), sz );
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
    if (   freopen( pzNewFile, "w" FOPEN_BINARY_FLAG "+", pCurFp->pFile )
        != pCurFp->pFile)

        AG_ABEND( aprf( zCannot, errno, "freopen", pzNewFile,
                        strerror( errno )));

    /*
     *  Set the mod time on the old file.
     */
    tbuf.actime  = time( NULL );
    tbuf.modtime = outTime;
    utime( pCurFp->pzOutName, &tbuf );
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
    return gh_int2scm( outputDepth );
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
    return gh_str02scm( (void*)p->pzOutName );
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

        rewind( pCurFp->pFile );
        do {
            int ich = fgetc( pCurFp->pFile );
            unsigned char ch = ich;
            if (ich < 0)
                break;
            if (ch == (unsigned char)'\n')
                lineNum++;
        } while (--pos > 0);
        fseek( pCurFp->pFile, svpos, SEEK_SET );
    } while(0);

    return gh_int2scm( lineNum );
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expOutput.c */
