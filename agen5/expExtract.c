/*
 *  expExtract.c
 *  $Id: expExtract.c,v 4.19 2009/01/01 16:49:26 bkorb Exp $
 *
 *  Time-stamp:        "2007-07-04 11:18:16 bkorb"
 *
 *  This module implements a file extraction function.
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
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
static char const*
loadExtractData( char const* pzNewFile );

static SCM
buildEmptyText( char const* pzStart, char const* pzEnd,
                SCM def );

static SCM
extractText( char const* pzText, char const* pzStart, char const* pzEnd,
             SCM def );
/* = = = END-STATIC-FORWARD = = = */

/*
 *  loadExtractData
 *
 *  Load a file into memory.  Keep it in memory and try to reuse it
 *  if we get called again.  Likely, there will be several extractions
 *  from a single file.
 */
static char const*
loadExtractData( char const* pzNewFile )
{
    static char const* pzFile = NULL;
    static char const* pzText = NULL;
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
       && (strcmp( pzFile, pzNewFile ) == 0))
        return pzText;

    if (  (stat( pzNewFile, &sbuf ) != 0)
       || (! S_ISREG(sbuf.st_mode))
       || (sbuf.st_size < 10) )
        return NULL;

    if (pzFile != NULL) {
        AGFREE( (void*)pzFile );
        AGFREE( (void*)pzText );
        pzFile = pzText = NULL;
    }

    AGDUPSTR( pzFile, pzNewFile, "extract file name" );
    pzIn = (char*)AGALOC( sbuf.st_size + 1, "Extraction File Text" );

    if (! HAVE_OPT( WRITABLE ))
        SET_OPT_WRITABLE;

    pzText = (char const*)pzIn;

    /*
     *  Suck up the file.  We must read it all.
     */
    {
        FILE* fp = fopen( pzNewFile, "r" );
        if (fp == NULL)
            goto bad_return;

        do  {
            size_t sz = fread(pzIn, (size_t)1, (size_t)sbuf.st_size, fp);
            if (sz == 0) {
                fprintf( stderr, "Error %d (%s) reading %d bytes of %s\n",
                         errno, strerror( errno ), (int)sbuf.st_size, pzFile );
                AG_ABEND( "read failure" );
            }

            pzIn += sz;
            sbuf.st_size -= sz;
        } while (sbuf.st_size > 0);

        *pzIn = NUL;
        fclose( fp );
    }

    return pzText;

 bad_return:

    AGFREE( (void*)pzFile );
    pzFile = NULL;
    AGFREE( (void*)pzText );
    pzText = NULL;

    return pzText;
}


/*
 *  Could not find the file or could not find the markers.
 *  Either way, emit an empty enclosure.
 */
static SCM
buildEmptyText( char const* pzStart, char const* pzEnd,
                SCM def )
{
    size_t mlen = strlen( pzStart ) + strlen( pzEnd ) + 3;
    char* pzOut;

    if (! AG_SCM_STRING_P( def )) {
        pzOut = ag_scribble( mlen );
        sprintf( pzOut, "%s\n%s", pzStart, pzEnd );

    } else {
        char* pzDef = ag_scm2zchars( def, "default extract string" );
        mlen += AG_SCM_STRLEN( def ) + 1;
        pzOut = ag_scribble( mlen );
        sprintf( pzOut, "%s\n%s\n%s", pzStart, pzDef, pzEnd );
    }

    return AG_SCM_STR02SCM( pzOut );
}


/*
 *  If we got it, emit it.
 */
static SCM
extractText( char const* pzText, char const* pzStart, char const* pzEnd,
             SCM def )
{
    char const* pzS = strstr( pzText, pzStart );
    char const* pzE;

    if (pzS == NULL)
        return buildEmptyText( pzStart, pzEnd, def );

    pzE = strstr( pzS, pzEnd );
    if (pzE == NULL)
        return buildEmptyText( pzStart, pzEnd, def );

    pzE += strlen( pzEnd );

    return AG_SCM_STR2SCM( pzS, (size_t)(pzE - pzS) );
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
 * doc:
 *
 * This function is used to help construct output files that may contain
 * text that is carried from one version of the output to the next.
 *
 * The first two arguments are required, the second are optional:
 *
 * @itemize @bullet
 * @item
 *      The @code{file-name} argument is used to name the file that
 *      contains the demarcated text.
 * @item
 *      The @code{marker-fmt} is a formatting string that is used to construct
 *      the starting and ending demarcation strings.  The sprintf function is
 *      given the @code{marker-fmt} with two arguments.  The first is either
 *      "START" or "END".  The second is either "DO NOT CHANGE THIS COMMENT"
 *      or the optional @code{caveat} argument.
 * @item
 *      @code{caveat} is presumed to be absent if it is the empty string
 *      (@code{""}).  If absent, ``DO NOT CHANGE THIS COMMENT'' is used
 *      as the second string argument to the @code{marker-fmt}.
 * @item
 *      When a @code{default} argument is supplied and no pre-existing text
 *      is found, then this text will be inserted between the START and END
 *      markers.
 * @end itemize
 *
 * @noindent
 * The resulting strings are presumed to be unique within
 * the subject file.  As a simplified example:
 *
 * @example
 * [+ (extract "fname" "// %s - SOMETHING - %s" ""
 *             "example default") +]
 * @end example
 * @noindent
 * will result in the following text being inserted into the output:
 *
 * @example
 * // START - SOMETHING - DO NOT CHANGE THIS COMMENT
 * example default
 * // END   - SOMETHING - DO NOT CHANGE THIS COMMENT
 * @end example
 *
 * @noindent
 * The ``@code{example default}'' string can then be carried forward to
 * the next generation of the output, @strong{@i{provided}} the output
 * is not named "@code{fname}" @i{and} the old output is renamed to
 * "@code{fname}" before AutoGen-eration begins.
 *
 * @table @strong
 * @item NB:
 * You can set aside previously generated source files inside the pseudo
 * macro with a Guile/scheme function, extract the text you want to keep
 * with this extract function.  Just remember you should delete it at the
 * end, too.  Here is an example from my Finite State Machine generator:
 * @end table
 *
 * @example
 * [+ AutoGen5 Template  -*- Mode: text -*-
 *    h=%s-fsm.h   c=%s-fsm.c
 *    (shellf
 *    "[ -f %1$s-fsm.h ] && mv -f %1$s-fsm.h .fsm.head
 *     [ -f %1$s-fsm.c ] && mv -f %1$s-fsm.c .fsm.code" (base-name)) +]
 * @end example
 *
 * This code will move the two previously produced output files to files
 * named ".fsm.head" and ".fsm.code".  At the end of the 'c' output
 * processing, I delete them.
=*/
SCM
ag_scm_extract( SCM file, SCM marker, SCM caveat, SCM def )
{
    char const* pzStart;
    char const* pzEnd;
    char const* pzText;

    if (! AG_SCM_STRING_P( file ) || ! AG_SCM_STRING_P( marker ))
        return SCM_UNDEFINED;

    pzText = loadExtractData( ag_scm2zchars( file, "extract file" ));

    {
        char const* pzMarker = ag_scm2zchars( marker, "extract marker" );
        char const* pzCaveat = "DO NOT CHANGE THIS COMMENT";

        if (AG_SCM_STRING_P( caveat ) && (AG_SCM_STRLEN( caveat ) > 0))
            pzCaveat = ag_scm2zchars( caveat, "extract caveat" );

        pzStart = aprf( pzMarker, "START", pzCaveat );
        pzEnd   = aprf( pzMarker, "END  ", pzCaveat );
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


/*=gfunc find_file
 *
 * what:   locate a file in the search path
 * exparg: file-name,  name of file with text
 * exparg: @suffix  @  file suffix to try, too   @ opt @
 *
 * doc:
 *
 * AutoGen has a search path that it uses to locate template and definition
 * files.  This function will search the same list for @file{file-name}, both
 * with and without the @file{.suffix}, if provided.
=*/
SCM
ag_scm_find_file( SCM file, SCM suffix )
{
    tSCC  zFun[] = "ag_scm_find_file";
    SCM res = SCM_UNDEFINED;

    if (! AG_SCM_STRING_P( file ))
        scm_wrong_type_arg( zFun, 1, file );

    {
        char z[ AG_PATH_MAX+1 ];
        char* pz = ag_scm2zchars( file, "file-name" );

        /*
         *  The suffix is optional.  If provided, it will be a string.
         */
        if (AG_SCM_STRING_P( suffix )) {
            char* apz[2];
            apz[0] = ag_scm2zchars( suffix, "file suffix" );
            apz[1] = NULL;
            if (SUCCESSFUL( findFile( pz, z, (tCC**)apz, NULL )))
                res = AG_SCM_STR02SCM( z );

        } else if (SUCCESSFUL( findFile( pz, z, NULL, NULL )))
            res = AG_SCM_STR02SCM( z );
    }

    return res;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expExtract.c */
