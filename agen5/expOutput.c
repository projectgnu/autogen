
/*
 *  $Id: expOutput.c,v 1.16 2001/08/25 00:18:17 bkorb Exp $
 *
 *  This module implements the output file manipulation function
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

#include <time.h>
#include <utime.h>

#include "autogen.h"
#include "expr.h"

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

STATIC int            suspendCt   = 0;
STATIC int            suspAllocCt = 0;
STATIC tSuspendName*  pSuspended  = NULL;
STATIC int            outputDepth = 1;

STATIC void addWriteAccess( char* pzFileName );



    EXPORT void
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

    STATIC void
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
 *  output files.  Use the @code{[#_ERROR 0#]} function instead.
 *  @xref{output controls}.
=*/
    SCM
ag_scm_out_delete( void )
{
    tSCC zSkipMsg[] = "NOTE:  skipping file '%s'\n";

    /*
     *  Delete the current output file
     */
    fprintf( stderr, zSkipMsg, pCurFp->pzOutName );
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
=*/
    SCM
ag_scm_out_move( SCM new_file )
{
    char* pz;
    int   len;

    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;

    len = SCM_LENGTH( new_file );
    pz  = AGALOC( len + 1, "move-to file name" );
    memcpy( (void*)pz, (void*)SCM_CHARS( new_file ), len );
    pz[len] = NUL;

    rename( pCurFp->pzOutName, pz );

    pCurFp->pzOutName = pz;  /* memory leak */
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

    if (pCurFp->pPrev == (tFpStack*)NULL) {
        fputs( "ERROR:  Cannot pop output with no output pushed\n",
               stderr );
        LOAD_ABORT( pCurTemplate, pCurMacro, "expr failed" );
    }


    if (gh_boolean_p( ret_contents ) && SCM_NFALSEP( ret_contents )) {
        long pos;
        pos = ftell( pCurFp->pFile );
        res = scm_makstr( pos, 0 );
        rewind( pCurFp->pFile );
        if (fread( SCM_CHARS( res ), pos, 1, pCurFp->pFile ) != 1) {
            fprintf( stderr, "Error %d (%s) rereading output\n", errno,
                     strerror( errno ));
            res = SCM_UNDEFINED;
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
    SCM res = SCM_UNDEFINED;
    tCC* pzSuspName = ag_scm2zchars( suspName, "suspend name" );

    if (pCurFp->pPrev == (tFpStack*)NULL) {
        fputs( "ERROR:  Cannot pop output with no output pushed\n",
               stderr );
        LOAD_ABORT( pCurTemplate, pCurMacro, "expr failed" );
    }

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

    return res;
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
            free( (void*)pSuspended[ ix ].pzSuspendName );
            if (ix < --suspendCt)
                pSuspended[ ix ] = pSuspended[ suspendCt ];
            ++outputDepth;
            return SCM_UNDEFINED;
        }
    }

    fprintf( stderr, "ERROR: no output file was suspended as ``%s''\n",
             pzName );
    LOAD_ABORT( pCurTemplate, pCurMacro, "resume failed" );
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

    p = (tFpStack*)AGALOC( sizeof( tFpStack ), "append - out file stack" );
    p->pPrev     = pCurFp;
    p->flags     = FPF_FREE;

    pzNewFile    = gh_scm2newstr( new_file, NULL );
    addWriteAccess( pzNewFile );
    p->pFile     = fopen( pzNewFile, "a" FOPEN_BINARY_FLAG "+" );

    p->pzOutName = pzNewFile;
    outputDepth++;
    pCurFp    = p;
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
 *  or @code{switch} closes it.  @xref{output controls}.
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
        pzNewFile = gh_scm2newstr( new_file, NULL );
        unlink( pzNewFile );
        addWriteAccess( pzNewFile );
        p->pFile  = fopen( pzNewFile, "a" FOPEN_BINARY_FLAG "+" );
    } else {
        AGDUPSTR( pzNewFile, ".agtmp.XXXXXX", "temp file name" );
        p->flags |= FPF_UNLINK;
        p->pFile  = fdopen( mkstemp( pzNewFile ), "a" FOPEN_BINARY_FLAG "+" );
    }

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
 *  you may not pop the base level output file.  @xref{output controls}.
=*/
    SCM
ag_scm_out_switch( SCM new_file )
{
    struct utimbuf tbuf;
    char*  pzNewFile;

    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;

    pzNewFile = gh_scm2newstr( new_file, NULL );

    /*
     *  IF no change, THEN ignore this
     */
    if (strcmp( pCurFp->pzOutName, pzNewFile ) == 0) {
        free( (void*)pzNewFile );
        return SCM_UNDEFINED;
    }

    removeWriteAccess( fileno( pCurFp->pFile ));

    /*
     *  Make sure we get a new file pointer!!
     *  and try to ensure nothing is in the way.
     */
    unlink( pzNewFile );
    if (   freopen( pzNewFile, "w" FOPEN_BINARY_FLAG, pCurFp->pFile )
        != pCurFp->pFile) {
        tSCC zOpen[] = "%s ERROR %d (%s): cannot open %s\n";
        fprintf( stderr, zOpen, pzProg,
                 errno, strerror( errno ), pzNewFile );
        LOAD_ABORT( pCurTemplate, pCurMacro, "expr failed" );
    }

    /*
     *  Set the mod time on the old file.
     */
    tbuf.actime  = time( (time_t*)NULL );
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
    return gh_str02scm( p->pzOutName );
}

/*
 * Local Variables:
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of expOutput.c */
