
/*
 *  expString.c
 *  $Id: expString.c,v 1.4 1999/10/31 19:08:16 bruce Exp $
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

#include "autogen.h"
#include <guile/gh.h>
#include "expGuile.h"

#ifndef HAVE_STRFTIME
#  include "compat/strftime.c"
#endif

    STATIC SCM
makeString( tCC*    pzText,
	    tCC*    pzNewLine,
            size_t  newLineSize )
{
    tCC*     pzDta   = pzText;
    char*    pzScn;
    SCM      res;
    size_t   dtaSize = sizeof( "\"\"" );

    for (;;) {
        char ch = *(pzDta++);
        if ((ch >= ' ') && (ch <= '~')) {
            dtaSize++;
            if ((ch == '"') || (ch == '\\'))
                dtaSize++;

        } else switch (ch) {
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
 * exparg: test-string, string to look for
 * exparg: string-list, list of strings to check,, list
 *
 * opt:  1
 * doc:  Return SCM_BOOL_T if the first argument is duplicated
 *      in the second (list) argument.
=*/
    SCM
ag_scm_in_p( SCM obj, SCM list )
{
    int   len;
    SCM   car;
    char* pz;

    if (! gh_string_p( obj ))
        return SCM_UNDEFINED;

    len = scm_ilength( list );
    pz  = SCM_CHARS( obj );

    while (--len >= 0) {
        car  = SCM_CAR( list );
        list = SCM_CDR( list );
        if (gh_string_p( car )) {
            if (strcmp( pz, SCM_CHARS( car )) == 0)
                return SCM_BOOL_T;
        }
    }

    return SCM_BOOL_F;
}


/*=gfunc join
 *
 * exparg: separator, string to insert between entries
 * exparg: list, list of strings to join,, list
 *
 * opt:  1
 * doc:  With the first argument as the separator string,
 *      joins together the second argument a-list of strings
 *      into one long string.
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
    pzSep   = SCM_CHARS( sep );
    sep_len = strlen( pzSep );
    str_len = 1;

    /*
     *  Count up the lengths of all the strings to be joined.
     */
    while (--l_len >= 0) {
        car  = SCM_CAR( list );
        list = SCM_CDR( list );
        if (! gh_string_p( car ))
            return SCM_UNDEFINED;

        str_len += strlen( SCM_CHARS( car ));
    }

    l_len = sv_l_len;
    res   = scm_makstr( str_len, 0 );
    pzRes = SCM_CHARS( res );

    /*
     *  Now, copy each one into the output
     */
    for (;;) {
        car  = SCM_CAR( alist );
        alist = SCM_CDR( alist );

        strcpy( pzRes, SCM_CHARS( car ));
        pzRes += strlen( pzRes );

        /*
         *  IF we reach zero, then do not insert a separation and bail out
         */
        if (--l_len <= 0)
            break;
        strcpy( pzRes, pzSep );
        pzRes += sep_len;
    }

    return res;
}


/*=gfunc prefix
 *
 * exparg: prefix, string to insert at start of each line
 * exparg: text, multi-line block of text
 *
 * req:  2
 * doc:
 *
 *  Prefix every line in the second string with the first string.
 *  E.g. if the first string is "# " and the second contains
 *  "two\nlines", then the result contain "# two\n# lines".
 *
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

    return text;
}


/*=gfunc shell
 *
 * exparg: command, shell command - the result value is stdout
 *
 * doc:
 *
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


/*=gfunc raw_shell_str
 *
 * exparg: string, string to transform
 *
 * doc:
 *
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

    return obj;
}


/*=gfunc shell_str
 *
 * exparg: string, string to transform
 *
 * doc:
 *  Convert the text of the string into a double quoted string
 *  that a normal shell will process into the original string.
 *  (Before doing macro expansion, that is.)
 *  The escaped characters are @code{\\} and @code{"}.
 *  All others are copied directly into the output.
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
    dtaSize = strlen( pzDta ) + 3;
    pz = pzDta;
    for (;;) {
        switch (*(pz++)) {
        case NUL:
            goto loopDone1;

        case '"':
        case '\\':
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

        case '"':
        case '\\':
            pz[-1]  = '\\';
            *(pz++) = pzDta[-1];
        }
    } loopDone2:;
    pz[-1] = '"';
    *pz    = NUL;

    return res;
}


/*=gfunc stack
 *
 * exparg: ag-name, AutoGen value name
 *
 * doc:  Create a list of all the strings that are associated
 *      with a name.
=*/
    SCM
ag_scm_stack( SCM obj )
{
    SCM   res = SCM_EOL;
    SCM * pos = &res;
    if (! gh_string_p( obj ))
        return SCM_UNDEFINED;
#ifdef LATER
    while (elt != SCM_UNDEFINED)
    {
        *pos = scm_cons (elt, SCM_EOL);
        pos = SCM_CDRLOC (*pos);
        elt = va_arg (foo, SCM);
    }
#endif
    return res;
}


/*=gfunc kr_string
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
evalExpr_kr_string( SCM str )
{
    tSCC    zNewLine[] = "\\n\\\n";

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    return makeString( SCM_CHARS( str ),
                       zNewLine, sizeof( zNewLine )-1 );
}


/*=gfunc c_string
 *
 * exparg: string, string to reformat
 *
 * doc:
 *  Reform a string so that, when printed, the C compiler will be able
 *  to compile the data and construct a string that contains exactly
 *  what the current string contains.  Many non-printing characters are
 *  replaced with escape sequences.  New-lines are replaced with a
 *  backslash-n sequence @code{\\n}, followed by a closing quote, a
 *  newline seven spaces and another re-opening quote.  The compiler
 *  will implicitly concatenate them.  The reader will see line breaks.
 *  A K&R compiler will choke.  Use @code{kr-string} for that.
=*/
    SCM
ag_scm_c_string( SCM str )
{
    tSCC       zNewline[] = "\\n\"\n       \"";

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    return makeString( SCM_CHARS( str ),
                       zNewLine, sizeof( zNewLine )-1 );
}


/*=gfunc string_tr_x
 *
 *  exparg:  source, string to transform
 *  exparg:  match,  characters to be converted
 *  exparg:  translation, result characters
 *
 *  req:  3
 *
 * doc:  This is the same as the @code{tr(1)} program, except the
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
            *pzTo = map[ *pzTo ];
            pzTo++;
        }
    }
    return str;
}
/* end of expString.c */
