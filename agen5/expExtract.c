/*  -*- Mode: C -*-
 *
 *  expExtract.c
 *  $Id: expExtract.c,v 1.3 2001/08/29 03:10:48 bkorb Exp $
 *  This module implements a file extraction function.
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

#include <string.h>

#include "expr.h"
#include "autogen.h"

/*
 *  loadExtractData
 *
 *  Load a file into memory.  Keep it in memory and try to reuse it
 *  if we get called again.  Likely, there will be several extractions
 *  from a single file.
 */
    STATIC const char*
loadExtractData( const char* pzNewFile )
{
    static const char* pzFile = NULL;
    static const char* pzText = NULL;
    struct stat  sbuf;
    char* pzIn;

    /*
     *  Make sure that we:
     *
     *  o got the file name from the SCM value
     *  o return the old text if we are searching the same file
     *  o have a regular file with some data
     *  o can allocate the space we need...
     *
     *  If we don't know about the current file, we leave the data
     *  from any previous file we may have loaded.
     */
    if (pzNewFile == NULL)
        return NULL;

    if (  (pzFile != NULL)
       && (strcmp( pzFile, pzNewFile ) == 0)) {
        free( (void*)pzNewFile );
        return pzText;
    }

    if (  (stat( pzNewFile, &sbuf ) != 0)
       || (! S_ISREG(sbuf.st_mode))
       || (sbuf.st_size < 10) ) {
        free( (void*)pzNewFile );
        return NULL;
    }

    if (pzFile != NULL) {
        free( (void*)pzFile );
        AGFREE( (void*)pzText );
        pzFile = pzText = NULL;
    }

    pzFile = pzNewFile;
    pzIn = (char*)AGALOC( sbuf.st_size + 1, "Extraction File Text" );
    if (pzIn == NULL)
        goto bad_return;

    if (! HAVE_OPT( WRITABLE ))
        SET_OPT_WRITABLE;

    pzText = (const char*)pzIn;

    /*
     *  Suck up the file.  We must read it all.
     */
    {
        FILE* fp = fopen( pzNewFile, "r" );
        if (fp == NULL)
            goto bad_return;

        do  {
            size_t sz = fread( pzIn, 1, sbuf.st_size, fp );
            if (sz == 0) {
                fprintf( stderr, "Error %d (%s) reading %d bytes of %s\n",
                         errno, strerror( errno ), sbuf.st_size, pzFile );
                LOAD_ABORT( pCurTemplate, pCurMacro, "read failure" );
            }

            pzIn += sz;
            sbuf.st_size -= sz;
        } while (sbuf.st_size > 0);

        *pzIn = NUL;
        fclose( fp );
    }

    return pzText;

 bad_return:

    if (pzFile != NULL) {
        free( (void*)pzFile );
        pzFile = NULL;
    }

    if (pzText != NULL) {
        AGFREE( (void*)pzText );
        pzText = NULL;
    }

    return NULL;
}


/*
 *  Could not find the file or could not find the markers.
 *  Either way, emit an empty enclosure.
 */
    STATIC SCM
buildEmptyText( const char* pzStart, const char* pzEnd,
                SCM def )
{
    size_t mlen = strlen( pzStart ) + strlen( pzEnd );
    SCM res;
    char* pzDef;
    size_t dlen;

    if (! gh_string_p( def ))
        dlen = 0;

    else {
        dlen = SCM_LENGTH( def ) + 1;
        pzDef = ag_scm2zchars( def, "default extract string" );
    }

    res = scm_makstr( mlen + dlen + 1, 0 );
    if (dlen > 0)
         sprintf( SCM_CHARS( res ), "%s\n%s\n%s", pzStart, pzDef, pzEnd );
    else sprintf( SCM_CHARS( res ), "%s\n%s", pzStart, pzEnd );

    return res;
}


/*
 *  If we got it, emit it.
 */
    STATIC SCM
extractText( const char* pzText, const char* pzStart, const char* pzEnd,
             SCM def )
{
    const char* pzS = strstr( pzText, pzStart );
    const char* pzE;
    SCM res;

    if (pzS == NULL)
        return buildEmptyText( pzStart, pzEnd, def );

    pzE = strstr( pzS, pzEnd );
    if (pzE == NULL)
        return buildEmptyText( pzStart, pzEnd, def );

    pzE += strlen( pzEnd );
    res = scm_makstr( pzE - pzS, 0 );
    memcpy( SCM_CHARS( res ), pzS, pzE - pzS );

    return res;
}


/*=gfunc extract
 *
 * what:   extract text from another file
 * general_use:
 * exparg: file-name,  name of file with text
 * exparg: marker-fmt, format for marker text
 * exparg: caveat,     warn about changing marker, opt
 * exparg: default,    default initial text,       opt
 *
 * doc: This function is used to help construct output files that may contain
 *      text that is carried from one version of the output to the next.
 *
 *      The @code{file} argument is used to name the file that contains the
 *      demarcated text.
 *@*
 *      The @code{marker-fmt} is a formatting string that is used to construct
 *      the starting and ending demarcation strings.  The sprintf function is
 *      given the @code{marker-fmt} with two arguments.  The first is either
 *      "START" or "END".  The second is either "DO NOT CHANGE THIS COMMENT"
 *      or the optional @code{caveat} argument.
 *@*
 *      @code{caveat} is presumed to be absent if it is the empty string
 *      (@code{""}).  The resulting strings are presumed to be unique within
 *      the subject file.
 *@*
 *      When a @code{default} argument is supplied and no pre-existing text
 *      is found, then this text will be inserted between the START and END
 *      markers.
=*/
    SCM
ag_scm_extract( SCM file, SCM marker, SCM caveat, SCM def )
{
    const char* pzStart;
    const char* pzEnd;
    const char* pzText;

    if (! gh_string_p( file ) || ! gh_string_p( marker ))
        return SCM_UNDEFINED;

    pzText = loadExtractData( gh_scm2newstr( file, NULL ));

    {
        const char* pzMarker = gh_scm2newstr( marker, NULL );
        const char* pzCaveat = "DO NOT CHANGE THIS COMMENT";
        int static_caveat = 1;
        if (gh_string_p( caveat ) && (SCM_LENGTH( caveat ) > 0)) {
            static_caveat = 0;
            pzCaveat = gh_scm2newstr( caveat, NULL );
        }
        pzStart = asprintf( pzMarker, "START", pzCaveat );
        pzEnd   = asprintf( pzMarker, "END  ", pzCaveat );
        if (! static_caveat)
            free( (void*)pzCaveat );
    }

    {
        SCM res;

        if (pzText == NULL)
             res = buildEmptyText( pzStart, pzEnd, def );
        else res = extractText( pzText, pzStart, pzEnd, def );

        AGFREE( (void*)pzStart );
        AGFREE( (void*)pzEnd );
        return res;
    }
}

/*
 * Local Variables:
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of expExtract.c */
