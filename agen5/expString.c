
/*
 *  expString.c
 *  $Id: expString.c,v 1.37 2001/12/01 20:26:19 bkorb Exp $
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
        {
            tSCC zFmt[] = "\\%03o";
            /*
             *  sprintf is safe here, because we already computed
             *  the amount of space we will be using.
             */
            sprintf( pzDta, zFmt, ch );
            pzDta += 4;
        }
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


    STATIC SCM
shell_stringify( SCM obj, char qt )
{
    size_t     dtaSize;
    char*      pzDta;
    char*      pz;
    char*      pzNew;
    SCM        res;

    pzDta = ag_scm2zchars( obj, "AG Object" );
    dtaSize = 3;
    pz = pzDta;
    for (;;) {
        char c = *(pz++);

        switch (c) {
        case NUL:
            goto loopDone1;

        case '"':
        case '`':
        case '\\':
            dtaSize += 2;
            break;

        default:
            dtaSize++;            
        }
    } loopDone1:;

    pz  = pzNew = AGALOC( dtaSize, "shell string" );
    *(pz++) = qt;

    for (;;) {
        char c = (*(pz++) = *(pzDta++));
        switch (c) {
        case NUL:
            goto loopDone2;

        case '\\':
            /*
             *  If someone went to the trouble to escape a backquote or a
             *  dollar sign, then we should not neutralize it.  Note that we
             *  handle a following backslash as a normal character.
             *  ALSO, now that this routine does both `xx` and "xx" strings,
             *  we have to worry about this stuff differently.  I.e., in ""
             *  strings, preserve a single \ in front of `, and in ``
             *  preserve a single \ in front of ".  Icky.
             *
             * i.e.  \\ --> \\\\ *BUT* \\$ --> \\\$
             */
            switch (*pzDta) {
            case '$':             /* \$  -->  \$    */
                break;
            case '`':             /* \`  -->  \`    */
            case '"':             /* \"  -->  \"    */
                if (c != qt)
                    break;
            default:              /* otherwise:     */
                *(pz++) = '\\';   /* \   -->  \\    */
            }
            break;

        case '"':
        case '`':
            if (c == qt) {
                pz[-1]  = '\\';       /* "   -->   \"   */
                *(pz++) = c;
            }
        }
    } loopDone2:;

    pz[-1]  = qt;
    dtaSize = (pz - pzNew);
    res     = scm_makstr( (scm_sizet)dtaSize, 0 );
    memcpy( SCM_CHARS( res ), pzNew, dtaSize );
    AGFREE( pzNew );

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
 * doc:  Return SCM_BOOL_T if the first argument string is found
 *      in one of the entries in the second (list-of-strings) argument.
=*/
    SCM
ag_scm_in_p( SCM obj, SCM list )
{
    int   len;
    int   lenz;
    SCM   car;
    char* pz1;

    if (! gh_string_p( obj ))
        return SCM_UNDEFINED;

    pz1  = SCM_CHARS(  obj );
    lenz = SCM_LENGTH( obj );

    /*
     *  If the second argument is a string somehow, then treat
     *  this as a straight out string comparison
     */
    if (gh_string_p( list )) {
        if (  (SCM_LENGTH( list ) == lenz)
           && (strncmp( pz1, SCM_CHARS( list ), lenz ) == 0) )
            return SCM_BOOL_T;
        return SCM_BOOL_F;
    }

    len = scm_ilength( list );
    if (len == 0)
        return SCM_BOOL_F;

    /*
     *  Search all the lists and sub-lists passed in
     */
    while (len-- > 0) {
        car  = SCM_CAR( list );
        list = SCM_CDR( list );

        /*
         *  This routine is listed as getting a list as the second
         *  argument.  That means that if someone builds a list and
         *  hands it to us, it magically becomes a nested list.
         *  This unravels that.
         */
        if (! gh_string_p( car )) {
            if (ag_scm_in_p( obj, car ) == SCM_BOOL_T)
                return SCM_BOOL_T;
            continue;
        }

        if (  (SCM_LENGTH( car ) == lenz)
           && (strncmp( pz1, SCM_CHARS( car ), lenz ) == 0) )
            return SCM_BOOL_T;
    }

    return SCM_BOOL_F;
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

    pzPfx   = ag_scm2zchars( prefix, "prefix" );
    pzDta   = \
    pzText  = ag_scm2zchars( text, "text" );
    pfx_size = strlen( pzPfx );
    out_size = pfx_size;

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
    pzText = pzDta;
    pzDta  = SCM_CHARS( res );
    strcpy( pzDta, pzPfx );
    pzDta    += pfx_size;
    out_size -= pfx_size;
    pfx_size++;

    for (;;) {
        char ch = *(pzText++);
        switch (ch) {
        case NUL:
            if (out_size != 0)
                LOAD_ABORT( pCurTemplate, pCurMacro, "(prefix ...) failed" );

            return res;

        case '\n':
            *pzDta    = ch;
            strcpy( pzDta+1, pzPfx );
            pzDta    += pfx_size;
            out_size -= pfx_size;
            break;

        default:
            out_size--;
            *(pzDta++) = ch;
            break;
        }
    }
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
        char* pz = runShell( ag_scm2zchars( cmd, "command" ));
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
    char* pzFmt = ag_scm2zchars( fmt, "format" );

    if (len <= 0)
        return fmt;

    fmt = run_printf( pzFmt, len, alist );
    pz  = runShell( ag_scm2zchars( fmt, "shell script" ));
    fmt = gh_str02scm( pz );
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
 *
 *  @strong{NOTE}:  some shells will not correctly handle unusual
 *  non-printing characters.  This routine works for most reasonably
 *  conventional ASCII strings.
=*/
    SCM
ag_scm_raw_shell_str( SCM obj )
{
    static const char zQ[] = "'\\''";

    size_t     dtaSize;
    char*      pzDta;
    char*      pz;
    SCM        res;

    pzDta   = ag_scm2zchars( obj, "AG Object" );
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
 *  shell will process into the original string, almost.  It will add the
 *  escape character @code{\\} before two special characters to
 *  accomplish this: the backslash @code{\\} and double quote @code{"}.
 *
 *  @strong{NOTE}: some shells will not correctly handle unusual
 *  non-printing characters.  This routine works for most reasonably
 *  conventional ASCII strings.
 *
 *  @strong{WARNING}:
 *@*
 *  This function omits the extra backslash in front of a backslash, however,
 *  if it is followed by either a backquote or a dollar sign.  It must do this
 *  because otherwise it would be impossible to protect the dollar sign or
 *  backquote from shell evaluation.  Consequently, it is not possible to
 *  render the strings "\\$" or "\\`".  The lesser of two evils.
 *
 *  All others characters are copied directly into the output.
 *
 *  The @code{sub-shell-str} variation of this routine behaves identically,
 *  except that the extra backslash is omitted in front of @code{"} instead
 *  of @code{`}.  You have to think about it.  I'm open to suggestions.
 *
 *  Meanwhile, the best way to document is with a detailed output example.
 *  If the backslashes make it through the texinfo processing correctly,
 *  below you will see what happens with three example strings.  The first
 *  example string contains a list of quoted @code{foo}s, the second is
 *  the same with a single backslash before the quote characters and the
 *  last is with two backslash escapes.  Below each is the result of the
 *  @code{raw-shell-str}, @code{shell-str} and @code{sub-shell-str} functions.
 *
 *  @example
 *  foo[0]       = 'foo' "foo" `foo` $foo
 *  raw-shell-str: ''\''foo'\'' "foo" `foo` $foo'
 *  shell-str:     "'foo' \"foo\" `foo` $foo"
 *  sub-shell-str: `'foo' "foo" \`foo\` $foo`
 *   
 *  foo[1]       = \'bar\' \"bar\" \`bar\` \$bar
 *  raw-shell-str: '\'\''bar\'\'' \"bar\" \`bar\` \$bar'
 *  shell-str:     "\\'bar\\' \\"bar\\" \`bar\` \$bar"
 *  sub-shell-str: `\\'bar\\' \"bar\" \\`bar\\` \$bar`
 *   
 *  foo[2]       = \\'BAZ\\' \\"BAZ\\" \\`BAZ\\` \\$BAZ
 *  raw-shell-str: '\\'\''BAZ\\'\'' \\"BAZ\\" \\`BAZ\\` \\$BAZ'
 *  shell-str:     "\\\\'BAZ\\\\' \\\\"BAZ\\\\" \\\`BAZ\\\` \\\$BAZ"
 *  sub-shell-str: `\\\\'BAZ\\\\' \\\"BAZ\\\" \\\\`BAZ\\\\` \\\$BAZ`
 *  @end example
 *
 *  There should be triple and quadruple backslashes in the last two lines of
 *  the example.  If this was not accurately reproduced, take a look at the
 *  agen5/test/shell.test test.  Notice the backslashes in front of the dollar
 *  signs.  It goes from zero to one to three for the "cooked" string examples.
=*/
    SCM
ag_scm_shell_str( SCM obj )
{
    return shell_stringify( obj, '"' );
}

/*=gfunc sub_shell_str
 *
 * what:  back quoted (sub-)shell string
 * general_use:
 *
 * exparg: string, string to transform
 *
 * doc:
 *   This function is substantially identical to @code{shell-str}, except
 *   that the quoting character is @code{`} and the "leave the escape alone"
 *   character is @code{"}.
=*/
    SCM
ag_scm_sub_shell_str( SCM obj )
{
    return shell_stringify( obj, '`' );
}


/*=gfunc stack
 *
 * what:  make list of AutoGen values
 *
 * exparg: ag-name, AutoGen value name
 *
 * doc:  Create a scheme list of all the strings that are associated
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

    res = SCM_EOL;

    ppDE = findEntryList( ag_scm2zchars( obj, "AG Object" ));
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
    tSCC zNewLine[] = "\\n\\\n";

    return makeString( ag_scm2zchars( str, "string" ),
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
    tSCC zNewLine[] = "\\n\"\n       \"";

    return makeString( ag_scm2zchars( str, "string" ),
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
    {
        char  map[ 256 ];
        int   i = 255;
        char* pzFrom = ag_scm2zchars( from_xform, "string" );
        char* pzTo   = ag_scm2zchars(   to_xform, "string" );

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

        pzTo = ag_scm2zchars( str, "string" );
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
 * indent-tabs-mode: nil
 * End:
 * end of expString.c */
