
/*
 *  $Id: expOutput.c,v 1.3 1999/10/31 19:03:51 bruce Exp $
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

#ifndef S_IAMB
/*
 *  Access Mask Bits (3 special plus RWX for User Group & Others (9))
 */
#  define S_IAMB      (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
#endif

STATIC int outputDepth = 1;


    void
removeWriteAccess( int fd )
{
    struct stat    sbuf;
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
 * req: 0
 * doc:  
 *  Remove the current output file.  Cease processing the template for
 *  the current suffix.  It is an error if there are @code{push}-ed
 *  output files.  Use the @code{[#_ERROR 0#]} function instead.
=*/
    SCM
ag_scm_out_delete( void )
{
    tSCC zSkipMsg[] = "NOTE:  skipping file '%s'\n";

    /*
     *  Delete the current output file
     */
    fprintf( stderr, zSkipMsg, pCurFp->pzName );
    outputDepth = 1;
    longjmp( fileAbort, PROBLEM );
    /* NOTREACHED */
    return SCM_UNDEFINED;
}


/*=gfunc out_move
 *
 * exparg: new-name, new name for the current output file
 *
 * doc:  
 *  Rename current output file.
=*/
    SCM
ag_scm_out_move( SCM new_file )
{
    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;

    rename( pCurFp->pzName, SCM_CHARS( new_file ));
    pCurFp->pzName = strdup( SCM_CHARS( new_file ));
    return SCM_UNDEFINED;
}


/*=gfunc out_pop
 *
 * req: 0
 * doc:  
 *  If there has been a @code{push} on the output, then close that
 *  file and go back to the previously open file.  It is an error
 *  if there has not been a @code{push}.  The file name argument may be
 *  omitted and is ignored.
=*/
    SCM
ag_scm_out_pop( void )
{
    if (pCurFp->pPrev == (tFpStack*)NULL) {
        fputs( "ERROR:  Cannot pop output with no output pushed\n",
               stderr );
        longjmp( fileAbort, PROBLEM );
    }

    outputDepth--;
    closeOutput( AG_FALSE );
    return SCM_UNDEFINED;
}


/*=gfunc out_push_add
 *
 * exparg: file-name, name of the file to append text to
 *
 * doc: 
 *  Identical to @code{push-new}, except the contents are @strong{not}
 *  purged, but appended to. 
=*/
    SCM
ag_scm_out_push_add( SCM new_file )
{
    tFpStack* p;
    char*  pzNewFile;

    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;

    pzNewFile = SCM_CHARS( new_file );

    p = (tFpStack*)AGALOC( sizeof( tFpStack ));
    if (p == (tFpStack*)NULL) {
        fprintf( stderr, zAllocErr, pzProg,
                 sizeof( tFpStack ), "file ptr stack entry" );
        longjmp( fileAbort, FAILURE );
    }

    p->pPrev  = pCurFp;
    p->pzName = strdup( pzNewFile );
    addWriteAccess( pzNewFile );
    outputDepth++;
    p->pFile  = fopen( pzNewFile, "a" FOPEN_BINARY_FLAG );
    pCurFp    = p;
    return SCM_UNDEFINED;
}


/*=gfunc out_push_new
 *
 * exparg: file-name, name of the file to create
 *
 * doc:
 *  Leave the current output file open, but purge and create
 *  a new file that will remain open until a @code{pop} @code{delete}
 *  or @code{switch} closes it.
=*/
    SCM
ag_scm_out_push_new( SCM new_file )
{
    tFpStack* p;
    char*  pzNewFile;

    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;

    pzNewFile = SCM_CHARS( new_file );

    p = (tFpStack*)AGALOC( sizeof( tFpStack ));
    if (p == (tFpStack*)NULL) {
        fprintf( stderr, zAllocErr, pzProg,
                 sizeof( tFpStack ), "file ptr stack entry" );
        longjmp( fileAbort, FAILURE );
    }

    p->pPrev  = pCurFp;
    p->pzName = strdup( pzNewFile );
    unlink( pzNewFile );
    outputDepth++;
    p->pFile  = fopen( pzNewFile, "w" FOPEN_BINARY_FLAG );
    pCurFp    = p;
    return SCM_UNDEFINED;
}


/*=gfunc out_switch
 *
 * exparg: file-name, name of the file to create
 *
 * doc:
 *  Switch output files - close current file and make the current
 *  file pointer refer to the new file.  This is equivalent to
 *  @code{out-pop} followed by @code{out-push-new}, except that
 *  you may not pop the base level output file.
=*/
    SCM
ag_scm_out_switch( SCM new_file )
{
    struct utimbuf tbuf;
    char*  pzNewFile;

    if (! gh_string_p( new_file ))
        return SCM_UNDEFINED;

    pzNewFile = SCM_CHARS( new_file );

    /*
     *  IF no change, THEN ignore this
     */
    if (strcmp( pCurFp->pzName, pzNewFile ) == 0)
        return SCM_UNDEFINED;

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
        longjmp( fileAbort, FAILURE );
    }

    /*
     *  Set the mod time on the old file.
     */
    tbuf.actime  = time( (time_t*)NULL );
    tbuf.modtime = outTime;
    utime( pCurFp->pzName, &tbuf );
    pCurFp->pzName = strdup( pzNewFile );

    return SCM_UNDEFINED;
}


/*=gfunc out_depth
 *
 * req:  0
 *
 * doc:  Returns the depth of the output file stack.
=*/
    SCM
ag_scm_out_depth( void )
{
    return gh_int2scm( outputDepth );
}


/*=gfunc out_name
 *
 * req:  0
 *
 * doc:  Returns the name of the current output file.
=*/
    SCM
ag_scm_out_name( void )
{
    return gh_str02scm( pCurFp->pzName );
}
/* end of expOutput.c */
