
/*
 *  expString.c
 *  $Id: expString.c,v 1.25 2001/05/09 05:25:59 bkorb Exp $
 *  This module implements expression functions that
 *  manipulate string values.
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

#ifndef HAVE_STRFTIME
#  include "compat/strftime.c"
#endif


    STATIC SCM
makeString( tCC*    pzText,
            tCC*    pzNewLine,
            size_t  newLineSize )
{
    char*    pzDta;
    tCC*     pzScn   = pzText;
    SCM      res;

    /*
     *  Start by counting the start and end quotes, plus the NUL.
     */
    size_t   dtaSize = sizeof( "\"\"" );

    for (;;) {
        char ch = *(pzScn++);
        if ((ch >= ' ') && (ch <= '~')) {

            /*
             *  One for each character, plus a backquote when needed
             */
            dtaSize++;
            if ((ch == '"') || (ch == '\\'))
                dtaSize++;

        }

        /*
         *  When not a normal character, then count the characters
         *  required to represent whatever it is.
         */
        else switch (ch) {
        case NUL:
            goto loopBreak;

        case '\n':
            dtaSize += newLineSize;
            break;

        case '\t':
        case '\a':
        case '\b':
        case '\f':
        case '\r':
        case '\v':
            dtaSize += 2;
            break;

        default:
            dtaSize += 4;
        }
    } loopBreak:;

    /*
     *  We now know how big the string is going to be.
     *  Allocate what we need.
     */
    res   = scm_makstr( (scm_sizet)dtaSize, 0 );
    pzDta = SCM_CHARS( res );
    pzScn = pzText;

    *(pzDta++) = '"';

    for (;;) {
        unsigned char ch = (unsigned char)*pzScn;
        if ((ch >= ' ') && (ch <= '~')) {
            if ((ch == '"') || (ch == '\\'))
                /*
                 *  We must escape these characters in the output string
                 */
                *(pzDta++) = '\\';
            *(pzDta++) = ch;

        } else switch (ch) {
        case NUL:
            goto copyDone;

        case '\n':
            /*
             *  place contiguous new-lines on a single line
             */
            while (pzScn[1] == '\n') {
                *(pzDta++) = '\\';
                *(pzDta++) = 'n';
                pzScn++;
            }

            /*
             *  Replace the new-line with its escaped representation.
             *  Also, break and restart the output string, indented
             *  7 spaces (so that after the '"' char is printed,
             *  any contained tabbing will look correct).
             *  Do *not* start a new line if there are no more data.
             */
            if (pzScn[1] == NUL) {
                *(pzDta++) = '\\';
                *(pzDta++) = 'n';
                goto copyDone;
            }

            strcpy( pzDta, pzNewLine );
            pzDta += newLineSize;
            break;

        case '\t':
            *(pzDta++) = '\\';
            *(pzDta++) = 't';
            break;

        case '\a':
            *(pzDta++) = '\\';
            *(pzDta++) = 'a';
            break;

        case '\b':
            *(pzDta++) = '\\';
            *(pzDta++) = 'b';
            break;

        case '\f':
            *(pzDta++) = '\\';
            *(pzDta++) = 'f';
            break;

        case '\r':
            *(pzDta++) = '\\';
            *(pzDta++) = 'r';
            break;

        case '\v':
            *(pzDta++) = '\\';
            *(pzDta++) = 'v';
            break;

        default:
            /*
             *  sprintf is safe here, because we already computed
             *  the amount of space we will be using.
             */
            pzDta += sprintf( pzDta, "\\%03o", ch );
        }

        pzScn++;
    } copyDone:

    /*
     *  End of string.  Terminate the quoted output.
     *  If necessary, deallocate the text string.
     *  Return the scan resumption point.
     */
    *(pzDta++) = '"';
    *pzDta = NUL;
    return res;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  EXPRESSION EVALUATION ROUTINES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc in_p
 *
 * what:   test for string in list
 * general_use:
 * exparg: test-string, string to look for
 * exparg: string-list, list of strings to check,, list
 *
 * doc:  Return SCM_BOOL_T if the first argument is duplicated
 *      in one of the entries in the second (list) argument.
=*/
    SCM
ag_scm_in_p( SCM obj, SCM list )
{
    int   len;
    SCM   car, res;
    char* pz1;

    if (! gh_string_p( obj ))
        return SCM_UNDEFINED;

    len = scm_ilength( list );
    pz1 = gh_scm2newstr( obj, NULL );
    res = SCM_BOOL_F;

    while (--len >= 0) {
        car  = SCM_CAR( list );
        list = SCM_CDR( list );
        if (gh_string_p( car )) {
            char* pz2 = gh_scm2newstr( car, NULL );
            if (strcmp( pz1, pz2 ) == 0) {
                res = SCM_BOOL_T;
                free( (void*)pz2 );
                break;
            }
            free( (void*)pz2 );
        }
    }

    free( (void*)pz1 );
    return res;
}


/*=gfunc join
 *
 * what:   join string list with separator
 * general_use:
 * exparg: separator, string to insert between entries
 * exparg: list, list of strings to join,, list
 *
 * doc:  With the first argument as the separator string,
 *       joins together an a-list of strings into one long string.
 *       The list may contain nested lists, partly because you
 *       cannot always control that.
=*/
    SCM
ag_scm_join( SCM sep, SCM list )
{
    int        l_len, sv_l_len;
    SCM        car;
    SCM        alist = list;
    char*      pzSep;
    int        sep_len;
    scm_sizet  str_len;
    SCM        res;
    char*      pzRes;

    if (! gh_string_p( sep ))
        return SCM_UNDEFINED;

    sv_l_len = l_len = scm_ilength( list );
    if (l_len == 0)
	return gh_str02scm( "" );

    pzSep   = SCM_ROCHARS( sep );
    sep_len = SCM_LENGTH( sep );
    str_len = 0;

    /*
     *  Count up the lengths of all the strings to be joined.
     */
    for (;;) {
        car  = SCM_CAR( list );
        list = SCM_CDR( list );

        /*
         *  This routine is listed as getting a list as the second
         *  argument.  That means that if someone builds a list and
         *  hands it to us, it magically becomes a nested list.
         *  This unravels that.
         */
        if (! gh_string_p( car )) {
            if (car != SCM_UNDEFINED)
                car = ag_scm_join( sep, car );
            if (! gh_string_p( car ))
                return SCM_UNDEFINED;
        }

        str_len += SCM_LENGTH( car );

        if (--l_len <= 0)
            break;

        str_len += sep_len;
    }

    l_len = sv_l_len;
    res   = scm_makstr( str_len, 0 );
    pzRes = SCM_CHARS( res );

    /*
     *  Now, copy each one into the output
     */
    for (;;) {
        int cpy_len;

        car   = SCM_CAR( alist );
        alist = SCM_CDR( alist );

        /*
         *  This unravels nested lists.
         */
        if (! gh_string_p( car ))
            car = ag_scm_join( sep, car );

        cpy_len = SCM_LENGTH( car );
        memcpy( (void*)pzRes, SCM_ROCHARS( car ), cpy_len );
        pzRes += cpy_len;

        /*
         *  IF we reach zero, then do not insert a separation and bail out
         */
        if (--l_len <= 0)
            break;
        memcpy( (void*)pzRes, (void*)pzSep, sep_len );
        pzRes += sep_len;
    }

    return res;
}


/*=gfunc prefix
 *
 * what:  prefix lines with a string
 * general_use:
 *
 * exparg: prefix, string to insert at start of each line
 * exparg: text, multi-line block of text
 *
 * doc:
 *  Prefix every line in the second string with the first string.
 *
 *  For example, if the first string is "# " and the second contains:
 *  @example
 *  two
 *  lines
 *  @end example
 *  @noindent
 *  The result string will contain:
 *  @example
 *  # two
 *  # lines
 *  @end example
=*/
    SCM
ag_scm_prefix( SCM prefix, SCM text )
{
    char*    pzPfx;
    char*    pzText;
    char*    pzDta;
    size_t   out_size;
    size_t   pfx_size;
    SCM      res;

    if ((! gh_string_p( prefix )) || (! gh_string_p( text )))
        return SCM_UNDEFINED;

    pzPfx   = SCM_CHARS( prefix );
    pzText  = SCM_CHARS( text );
    pfx_size = strlen( pzPfx );
    out_size = 1;

    for (;;) {
        switch (*(pzText++)) {
        case NUL:
            goto exit_count;
        case '\n':
            out_size += pfx_size;
            /* FALLTHROUGH */
        default:
            out_size++;
        }
    } exit_count:;

    res    = scm_makstr( (scm_sizet)out_size, 0 );
    pzDta  = SCM_CHARS( res );
    pzText = SCM_CHARS( text );
    strcpy( pzDta, pzPfx );
    pzDta += pfx_size;

    for (;;) {
        switch (*(pzDta++) = *(pzText++)) {
        case NUL:
            goto exit_copy;

        case '\n':
            strcpy( pzDta, pzPfx );
            pzDta += pfx_size;
            break;

        default:
            break;
        }
    } exit_copy:;

    return res;
}


/*=gfunc shell
 *
 * what:  invoke a shell script
 * general_use:
 *
 * exparg: command, shell command - the result value is stdout
 *
 * doc:
 *  Generate a string by writing the value to
 *  a server shell and reading the output back in.  The template
 *  programmer is responsible for ensuring that it completes
 *  within 10 seconds.  If it does not, the server will be killed,
 *  the output tossed and a new server started.
 *
 *  NB:  This is the same server process used by the '#shell'
 *  definitions directive and backquoted @code{`} definitions.
 *  There may be left over state from previous shell expressions
 *  and the @code{`} processing in the declarations.  However, a
 *  @code{cd} to the original directory is always issued before
 *  the new command is issued.
=*/
    SCM
ag_scm_shell( SCM cmd )
{
    if (! gh_string_p( cmd ))
        return SCM_UNDEFINED;
    {
        char* pz = runShell( SCM_CHARS( cmd ));
        cmd   = gh_str02scm( pz );
        AGFREE( (void*)pz );
        return cmd;
    }
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
ag_scm_shellf( SCM fmt, SCM alist )
{
    int   len = scm_ilength( alist );
    char* pz;

    if (! gh_string_p( fmt ))
        return SCM_UNDEFINED;

    if (len <= 0)
        return fmt;

    fmt = run_printf( SCM_CHARS( fmt ), len, alist );
    pz = runShell( SCM_CHARS( fmt ));
    fmt   = gh_str02scm( pz );
    AGFREE( (void*)pz );
    return fmt;
}


/*=gfunc raw_shell_str
 *
 * what:  single quote shell string
 * general_use:
 *
 * exparg: string, string to transform
 *
 * doc:
 *  Convert the text of the string into a singly quoted string
 *  that a normal shell will process into the original string.
 *  (It will not do macro expansion later, either.)
 *  Contained single quotes become tripled, with the middle quote
 *  escaped with a backslash.  Normal shells will reconstitute the
 *  original string.
=*/
    SCM
ag_scm_raw_shell_str( SCM obj )
{
    static const char zQ[] = "'\\''";

    size_t     dtaSize;
    char*      pzDta;
    char*      pz;
    SCM        res;

    if (! gh_string_p( obj ))
        return SCM_UNDEFINED;

    pzDta = SCM_CHARS( obj );
    dtaSize = strlen( pzDta ) + 3;
    pz = pzDta-1;
    for (;;) {
        pz = strchr( pz+1, '\'' );
        if (pz == (char*)NULL)
            break;
        dtaSize += STRSIZE( zQ );
    }

    res = scm_makstr( (scm_sizet)dtaSize, 0 );
    pz  = SCM_CHARS( res );

    *(pz++) = '\'';

    for (;;) {
        switch (*(pz++) = *(pzDta++)) {
        case NUL:
            goto loopDone;

        case '\'':
            strcpy( pz-1, zQ );
            pz += STRSIZE( zQ ) - 1;
        }
    } loopDone:;
    pz[-1] = '\'';
    *pz    = NUL;

    return res;
}


/*=gfunc shell_str
 *
 * what:  double quote shell string
 * general_use:
 *
 * exparg: string, string to transform
 *
 * doc:
 *
 *  Convert the text of the string into a double quoted string that a normal
 *  shell will process into the original string, almost.  It will add the escape
 *  character before two special characters to accomplish this: the backslash
 *  and double quote @code{"}.  It omits the extra backslash, however, if a
 *  backslash is followed by either a backquote or a dollar sign.  This makes it
 *  impossible to render the sequence of a backslash followed by a dollar sign,
 *  but it does make it possible to protect that dollar sign from shell
 *  evaluation.  The lesser of two evils.
 *
 *  All others characters are copied directly into the output.
=*/
    SCM
ag_scm_shell_str( SCM obj )
{
    size_t     dtaSize;
    char*      pzDta;
    char*      pz;
    SCM        res;

    if (! gh_string_p( obj ))
        return SCM_UNDEFINED;

    pzDta = SCM_CHARS( obj );
    dtaSize = 3;
    pz = pzDta;
    for (;;) {
        switch (*(pz++)) {
        case NUL:
            goto loopDone1;

        case '\\':
        case '"':
            dtaSize += 2;
            break;

        default:
            dtaSize++;            
        }
    } loopDone1:;

    res = scm_makstr( (scm_sizet)dtaSize, 0 );
    pz  = SCM_CHARS( res );

    *(pz++) = '"';

    for (;;) {
        switch (*(pz++) = *(pzDta++)) {
        case NUL:
            goto loopDone2;

        case '\\':
            /*
             *  This deserves a bit of explanation.  Basically, if someone went
             *  to the trouble to escape a backquote or a dollar sign, then we
             *  should not neutralize it.  Note that we handle a following
             *  backslash as a normal character.  i.e.  \\ --> \\\\
             *  *BUT*   \\$ --> \\\$
             */
            switch (*pzDta) {
            case '`':             /* \`  -->  \`    */
            case '$':             /* \$  -->  \$    */
                break;
            default:              /* otherwise:     */
                *(pz++) = '\\';   /* \   -->  \\    */
            }
            break;

        case '"':
            pz[-1]  = '\\';       /* "   -->   \"   */
            *(pz++) = '"';
        }
    } loopDone2:;

    pz[-1] = '"';
    *pz    = NUL;

    return res;
}


/*=gfunc stack
 *
 * what:  make list of AutoGen values
 *
 * exparg: ag-name, AutoGen value name
 *
 * doc:  Create a list of all the strings that are associated
 *       with a name.  They must all be text values or we choke.
=*/
    SCM
ag_scm_stack( SCM obj )
{
    SCM   res;
    SCM * pos = &res;
    tDefEntry** ppDE;
        tDefEntry* pDE;
        SCM        str;

    if (! gh_string_p( obj ))
        return SCM_UNDEFINED;

    res = SCM_EOL;

    ppDE = findEntryList( SCM_CHARS( obj ) );
    if (ppDE == NULL)
        return SCM_EOL;

    for (;;) {
        pDE = *(ppDE++);

        if (pDE == NULL)
            break;

        if (pDE->valType != VALTYP_TEXT)
            return SCM_UNDEFINED;

        str  = gh_str02scm( pDE->pzValue );
        *pos = scm_cons( str, SCM_EOL );
        pos  = SCM_CDRLOC( *pos );
    }

    return res;
}


/*=gfunc kr_string
 *
 * what:  emit string for K&R C
 * general_use:
 *
 * exparg: string, string to reformat
 *
 *  doc:
 *  Reform a string so that, when printed, a K&R C compiler will be able
 *  to compile the data and construct a string that contains exactly
 *  what the current string contains.  Many non-printing characters are
 *  replaced with escape sequences.  New-lines are replaced with a
 *  backslash-n-backslash and newline sequence,
=*/
    SCM
ag_scm_kr_string( SCM str )
{
    tSCC    zNewLine[] = "\\n\\\n";

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    return makeString( SCM_CHARS( str ),
                       zNewLine, sizeof( zNewLine )-1 );
}


/*=gfunc c_string
 *
 * what:  emit string for ANSI C
 * general_use:
 *
 * exparg: string, string to reformat
 *
 * doc:
 *  Reform a string so that, when printed, the C compiler will be able to
 *  compile the data and construct a string that contains exactly what the
 *  current string contains.  Many non-printing characters are replaced with
 *  escape sequences.  Newlines are replaced with a backslash, an @code{n}, a
 *  closing quote, a newline, seven spaces and another re-opening quote.  The
 *  compiler will implicitly concatenate them.  The reader will see line
 *  breaks.
 *
 *  A K&R compiler will choke.  Use @code{kr-string} for that compiler.
 *
=*/
    SCM
ag_scm_c_string( SCM str )
{
    tSCC       zNewLine[] = "\\n\"\n       \"";

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    return makeString( SCM_CHARS( str ),
                       zNewLine, sizeof( zNewLine )-1 );
}


/*=gfunc string_tr_x
 *
 * what:  convert characters
 * general_use:
 *
 *  exparg:  source, string to transform
 *  exparg:  match,  characters to be converted
 *  exparg:  translation, conversion list
 *
 * doc: This is the same as the @code{tr(1)} program, except the
 *      string to transform is the first argument.  The second and
 *      third arguments are used to construct mapping arrays for the
 *      transformation of the first argument.
 *
 *      It is too bad this little program has so many different
 *      and incompatible implementations!
=*/
    SCM
ag_scm_string_tr_x( SCM str, SCM from_xform, SCM to_xform )
{
    if (   (! gh_string_p( str ))
        || (! gh_string_p( from_xform ))
        || (! gh_string_p( to_xform )))
        return SCM_UNDEFINED;
    {
        char  map[ 256 ];
        int   i = 255;
        char* pzFrom = SCM_CHARS( from_xform );
        char* pzTo   = SCM_CHARS(   to_xform );

        do  {
            map[i] = i;
        } while (--i > 0);

        for (;i <= 255;i++) {
            unsigned char fch = (unsigned char)*(pzFrom++);
            unsigned char tch = (unsigned char)*(pzTo++);

            if (tch == NUL) {
                pzTo--;
                tch = pzTo[-1];
            }

            switch (fch) {
            case NUL:
                goto mapDone;

            case '-':
                if ((i > 0) && (tch == '-')) {
                    unsigned char fs = (unsigned char)pzFrom[-2];
                    unsigned char fe = (unsigned char)pzFrom[0];
                    unsigned char ts = (unsigned char)pzTo[-2];
                    unsigned char te = (unsigned char)pzTo[0];
                    if (te != NUL) {
                        while (fs < fe) {
                            map[ fs++ ] = ts;
                            if (ts < te) ts++;
                        }
                        break;
                    }
                }

            default:
                map[ fch ] = tch;
            }
        } mapDone:;

        pzTo = SCM_CHARS( str );
        while (*pzTo != NUL) {
            *pzTo = map[ (int)*pzTo ];
            pzTo++;
        }
    }
    return str;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * end of expString.c */
