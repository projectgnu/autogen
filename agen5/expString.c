
/*
 *  expString.c
 *  $Id: expString.c,v 4.16 2006/12/10 19:45:00 bkorb Exp $
 *
 *  Time-stamp:        "2006-12-10 11:05:08 bkorb"
 *  Last Committed:    $Date: 2006/12/10 19:45:00 $
 *
 *  This module implements expression functions that
 *  manipulate string values.
 */

/*
 *  AutoGen copyright 1992-2006 Bruce Korb
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
 *             51 Franklin Street, Fifth Floor,
 *             Boston, MA  02110-1301, USA.
 */

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static SCM
makeString( tCC*    pzText,
            tCC*    pzNewLine,
            size_t  newLineSize );

static SCM
shell_stringify( SCM obj, u_int qt );

static void
do_substitution(
    tCC*        pzStr,
    scm_sizet   strLen,
    SCM         match,
    SCM         repl,
    char**      ppzRes,
    scm_sizet*  pResLen );

static void
do_multi_subs(
    char**      ppzStr,
    scm_sizet*  pStrLen,
    SCM         match,
    SCM         repl );
/* = = = END-STATIC-FORWARD = = = */

static SCM
makeString( tCC*    pzText,
            tCC*    pzNewLine,
            size_t  newLineSize )
{
    char     z[ 256 ];
    char*    pzDta;
    char*    pzFre;
    tCC*     pzScn   = pzText;

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
    if (dtaSize >= sizeof(z))
         pzFre = pzDta = AGALOC( dtaSize, "quoting string" );
    else pzFre = pzDta = z;

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

        case '\t':
            *(pzDta++) = '\\';
            *(pzDta++) = 't';
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

    {
        SCM res = scm_makfrom0str( pzFre );
        if (pzFre != z)
            AGFREE( pzFre );
        return res;
    }
}


static SCM
shell_stringify( SCM obj, u_int qt )
{
    char*  pzDta;
    char*  pzNew;
    size_t dtaSize = 3;

    pzDta = ag_scm2zchars( obj, "AG Object" );

    {
        char*  pz = pzDta;

        for (;;) {
            char c = *(pz++);

            switch (c) {
            case NUL:
                goto loopDone1;

            case '"': case '`': case '\\':
                dtaSize += 2;
                break;

            default:
                dtaSize++;
            }
        } loopDone1:;
    }

    pzNew = AGALOC( dtaSize, "shell string" );

    {
        char* pz = pzNew;
        *(pz++) = qt;

        for (;;) {
            char c = (*(pz++) = *(pzDta++));
            switch (c) {
            case NUL:
                goto loopDone2;

            case '\\':
                /*
                 *  If someone went to the trouble to escape a backquote or a
                 *  dollar sign, then we should not neutralize it.  Note that
                 *  we handle a following backslash as a normal character.
                 *
                 *  i.e.  \\ --> \\\\ *BUT* \\$ --> \\\$
                 */
                c = *pzDta;
                switch (*pzDta) {
                case '$':
                    break;

                case '"':
                case '`':
                    /*
                     *  IF the ensuing quote character does *NOT* match the
                     *  quote character for the string, then we will preserve
                     *  the single copy of the backslash.  If it does match,
                     *  then we will double the backslash and a third backslash
                     *  will be inserted when we emit the quote character.
                     */
                    if (c != qt)
                        break;
                    /* FALLTHROUGH */

                default:
                    *(pz++) = '\\';   /* \   -->  \\    */
                }
                break;

            case '"': case '`':
                if (c == qt) {
                    /*
                     *  This routine does both `xx` and "xx" strings, we have
                     *  to worry about this stuff differently.  I.e., in ""
                     *  strings, add a single \ in front of ", and in ``
                     *  preserve a add \ in front of `.
                     */
                    pz[-1]  = '\\';       /* "   -->   \"   */
                    *(pz++) = c;
                }
            }
        } loopDone2:;

        pz[-1]  = qt;
        *pz     = NUL;
        dtaSize = (pz - pzNew);
    }

    {
        SCM res = AG_SCM_STR2SCM( pzNew, dtaSize );
        AGFREE( pzNew );
        return res;
    }
}


/* * * * *
 *
 *  Substitution routines.  These routines implement a sed-like
 *  experience, except that we don't use regex-es.  It is straight
 *  text substitution.
 */
static void
do_substitution(
    tCC*        pzStr,
    scm_sizet   strLen,
    SCM         match,
    SCM         repl,
    char**      ppzRes,
    scm_sizet*  pResLen )
{
    char* pzMatch = ag_scm2zchars( match, "match text" );
    char* pzRepl  = ag_scm2zchars( repl,  "repl text" );
    int   matchL  = AG_SCM_STRLEN( match );
    int   replL   = AG_SCM_STRLEN( repl );
    int   endL    = strLen;

    int   repCt   = 0;
    tCC*  pz      = pzStr;
    char* pzRes;
    char* pzScan;

    for (;;) {
        tCC* pzNxt = strstr( pz, pzMatch );
        if (pzNxt == NULL)
            break;
        repCt++;
        pz = pzNxt + matchL;
    }

    /*
     *  No substitutions -- no work.  The caller will have to track
     *  whether or not to deallocate the result.
     */
    if (repCt == 0)
        return;

    endL   = endL + (replL * repCt) - (matchL * repCt);
    pzScan = pzRes = AGALOC( endL + 1, "substitution" );
    pz     = pzStr;

    for (;;) {
        tCC* pzNxt = strstr( pz, pzMatch );
        if (pzNxt == NULL)
            break;
        if (pz != pzNxt) {
            memcpy( pzScan, pz, (size_t)(pzNxt - pz) );
            pzScan += (pzNxt - pz);
        }
        memcpy( pzScan, pzRepl, (size_t)replL );
        pzScan += replL;
        pz = pzNxt + matchL;
    }
    strcpy( pzScan, pz );
    *ppzRes = pzRes;
    *pResLen = (pzScan - pzRes) + strlen( pz );
}


/*
 *  Recursive routine.  It calls itself for list values and calls
 *  "do_substitution" for string values.  Each substitution will
 *  be done in the order found in the tree walk of list values.
 *  The "match" and "repl" trees *must* be identical in structure.
 */
static void
do_multi_subs(
    char**      ppzStr,
    scm_sizet*  pStrLen,
    SCM         match,
    SCM         repl )
{
    char* pzStr = *ppzStr;
    char* pzOri = pzStr;
    char* pzNxt = pzStr;

    /*
     *  Loop for as long as our list has more entries
     */
    while (! SCM_NULLP( match )) {
        /*
         *  "CAR" is the current value, "CDR" is rest of list
         */
        SCM  matchCar  = SCM_CAR( match );
        SCM  replCar   = SCM_CAR( repl );

        match = SCM_CDR( match );
        repl  = SCM_CDR( repl );

        if (AG_SCM_STRING_P( matchCar )) {
            do_substitution( pzStr, *pStrLen, matchCar, replCar,
                             &pzNxt, pStrLen );

            /*
             *  Be sure to free intermediate results.
             *  passed-in values are freed by the caller.
             */
            if ((pzStr != pzOri) && (pzStr != pzNxt))
                AGFREE( pzStr );
            pzStr = pzNxt;
        }

        else if (gh_list_p( matchCar ))
            do_multi_subs( &pzStr, pStrLen, matchCar, replCar );

        else
            /*
             *  Whatever it is it is not part of what we would expect.  Bail.
             */
            break;
    }

    *ppzStr = pzStr;
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
    int     len;
    size_t  lenz;
    SCM     car;
    tCC *   pz1;

    if (! AG_SCM_STRING_P( obj ))
        return SCM_UNDEFINED;

    pz1  = AG_SCM_CHARS(  obj );
    lenz = AG_SCM_STRLEN( obj );

    /*
     *  If the second argument is a string somehow, then treat
     *  this as a straight out string comparison
     */
    if (AG_SCM_STRING_P( list )) {
        if (  (AG_SCM_STRLEN( list ) == lenz)
           && (strncmp( pz1, AG_SCM_CHARS( list ), lenz) == 0))
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
        if (! AG_SCM_STRING_P( car )) {
            if (ag_scm_in_p( obj, car ) == SCM_BOOL_T)
                return SCM_BOOL_T;
            continue;
        }

        if (  (AG_SCM_STRLEN( car ) == lenz)
           && (strncmp( pz1, AG_SCM_CHARS( car ), lenz ) == 0) )
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
    size_t     sep_len;
    scm_sizet  str_len;
    char*      pzRes;
    tCC *      pzSep;
    char*      pzScan;

    if (! AG_SCM_STRING_P( sep ))
        return SCM_UNDEFINED;

    sv_l_len = l_len = scm_ilength( list );
    if (l_len == 0)
        return AG_SCM_STR02SCM(zNil);

    pzSep   = AG_SCM_CHARS( sep );
    sep_len = AG_SCM_STRLEN( sep );
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
        if (! AG_SCM_STRING_P( car )) {
            if (car != SCM_UNDEFINED)
                car = ag_scm_join( sep, car );
            if (! AG_SCM_STRING_P( car ))
                return SCM_UNDEFINED;
        }

        str_len += AG_SCM_STRLEN( car );

        if (--l_len <= 0)
            break;

        str_len += sep_len;
    }

    l_len = sv_l_len;
    pzRes = pzScan = ag_scribble( str_len );

    /*
     *  Now, copy each one into the output
     */
    for (;;) {
        size_t cpy_len;

        car   = SCM_CAR( alist );
        alist = SCM_CDR( alist );

        /*
         *  This unravels nested lists.
         */
        if (! AG_SCM_STRING_P( car ))
            car = ag_scm_join( sep, car );

        cpy_len = AG_SCM_STRLEN( car );
        memcpy( (void*)pzScan, AG_SCM_CHARS( car ), cpy_len );
        pzScan += cpy_len;

        /*
         *  IF we reach zero, then do not insert a separation and bail out
         */
        if (--l_len <= 0)
            break;
        memcpy( (void*)pzScan, (void*)pzSep, sep_len );
        pzScan += sep_len;
    }

    return AG_SCM_STR2SCM( pzRes, str_len );
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
    size_t   out_size, rem_size;
    size_t   pfx_size;
    char*    pzRes;

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

    pzText = pzDta;
    pzRes  = pzDta = ag_scribble( rem_size = out_size );
    strcpy( pzDta, pzPfx );
    pzDta    += pfx_size;
    rem_size -= pfx_size;
    pfx_size++;

    for (;;) {
        char ch = *(pzText++);
        switch (ch) {
        case NUL:
            if (rem_size != 0)
                AG_ABEND( "(prefix ...) failed" );

            return AG_SCM_STR2SCM( pzRes, out_size );

        case '\n':
            *pzDta    = ch;
            strcpy( pzDta+1, pzPfx );
            pzDta    += pfx_size;
            rem_size -= pfx_size;
            break;

        default:
            rem_size--;
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
    if (! AG_SCM_STRING_P( cmd ))
        return SCM_UNDEFINED;
    {
        char* pz = runShell( ag_scm2zchars( cmd, "command" ));
        cmd   = AG_SCM_STR02SCM( pz );
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

#ifdef DEBUG_ENABLED
    if (len < 0)
        AG_ABEND( "invalid alist to shellf" );
#endif

    pz  = ag_scm2zchars( fmt, "format" );
    fmt = run_printf( pz, len, alist );

    pz  = runShell( ag_scm2zchars( fmt, "shell script" ));
    fmt = AG_SCM_STR02SCM( pz );
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
 *  @strong{Notice}:  some shells will not correctly handle unusual
 *  non-printing characters.  This routine works for most reasonably
 *  conventional ASCII strings.
=*/
SCM
ag_scm_raw_shell_str( SCM obj )
{
    char*      pzDta;
    char*      pz;
    char*      pzFree;

    pzDta = ag_scm2zchars( obj, "AG Object" );

    {
        size_t dtaSize = AG_SCM_STRLEN( obj ) + 3; /* NUL + 2 quotes */
        pz = pzDta-1;
        for (;;) {
            pz = strchr( pz+1, '\'' );
            if (pz == NULL)
                break;
            dtaSize += 3; /* '\'' -> 3 additional chars */
        }

        pzFree = pz = AGALOC( dtaSize + 2, "raw string" );
    }

    /*
     *  Handle leading single quotes before starting the first quote.
     */
    while (*pzDta == '\'') {
        *(pz++) = '\\';
        *(pz++) = '\'';

        /*
         *  IF pure single quotes, then we're done.
         */
        if (*++pzDta == NUL) {
            *pz = NUL;
            goto returnString;
        }
    }

    /*
     *  Start quoting.  If the string is empty, we wind up with two quotes.
     */
    *(pz++) = '\'';

    for (;;) {
        switch (*(pz++) = *(pzDta++)) {
        case NUL:
            goto loopDone;

        case '\'':
            /*
             *  We've inserted a single quote, which ends the quoting session.
             *  Now, insert escaped quotes for every quote char we find, then
             *  restart the quoting.
             */
            pzDta--;
            do {
                *(pz++) = '\\';
                *(pz++) = '\'';
            } while (*++pzDta == '\'');
            if (*pzDta == NUL) {
                *pz = NUL;
                goto returnString;
            }
            *(pz++) = '\'';
        }
    } loopDone:;
    pz[-1] = '\'';
    *pz    = NUL;

 returnString:
    {
        SCM res = scm_makfrom0str( pzFree );
        AGFREE( pzFree );
        return res;
    }
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
 *  If the backslashes make it through the text processing correctly,
 *  below you will see what happens with three example strings.  The first
 *  example string contains a list of quoted @code{foo}s, the second is
 *  the same with a single backslash before the quote characters and the
 *  last is with two backslash escapes.  Below each is the result of the
 *  @code{raw-shell-str}, @code{shell-str} and @code{sub-shell-str} functions.
 *
 *  @example
 *  foo[0]           ''foo'' 'foo' "foo" `foo` $foo
 *  raw-shell-str -> \'\''foo'\'\'' '\''foo'\'' "foo" `foo` $foo'
 *  shell-str     -> "''foo'' 'foo' \"foo\" `foo` $foo"
 *  sub-shell-str -> `''foo'' 'foo' "foo" \`foo\` $foo`
 *
 *  foo[1]           \'bar\' \"bar\" \`bar\` \$bar
 *  raw-shell-str -> '\'\''bar\'\'' \"bar\" \`bar\` \$bar'
 *  shell-str     -> "\\'bar\\' \\\"bar\\\" \`bar\` \$bar"
 *  sub-shell-str -> `\\'bar\\' \"bar\" \\\`bar\\\` \$bar`
 *
 *  foo[2]           \\'BAZ\\' \\"BAZ\\" \\`BAZ\\` \\$BAZ
 *  raw-shell-str -> '\\'\''BAZ\\'\'' \\"BAZ\\" \\`BAZ\\` \\$BAZ'
 *  shell-str     -> "\\\\'BAZ\\\\' \\\\\"BAZ\\\\\" \\\`BAZ\\\` \\\$BAZ"
 *  sub-shell-str -> `\\\\'BAZ\\\\' \\\"BAZ\\\" \\\\\`BAZ\\\\\` \\\$BAZ`
 *  @end example
 *
 *  There should be four, three, five and three backslashes for the four
 *  examples on the last line, respectively.  The next to last line should
 *  have four, five, three and three backslashes.  If this was not accurately
 *  reproduced, take a look at the agen5/test/shell.test test.  Notice the
 *  backslashes in front of the dollar signs.  It goes from zero to one to
 *  three for the "cooked" string examples.
=*/
SCM
ag_scm_shell_str( SCM obj )
{
    return shell_stringify( obj, (unsigned)'"' );
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
    return shell_stringify( obj, (unsigned)'`' );
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

        str  = AG_SCM_STR02SCM( pDE->val.pzText );
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

        pzTo = (char*)(void*)AG_SCM_CHARS( str );
        i    = AG_SCM_STRLEN( str );
        while (i-- > 0) {
            *pzTo = map[ (int)*pzTo ];
            pzTo++;
        }
    }
    return str;
}


/*=gfunc string_tr
 *
 * what:  convert characters with new result
 * general_use:
 *
 *  exparg:  source, string to transform
 *  exparg:  match,  characters to be converted
 *  exparg:  translation, conversion list
 *
 * doc: This is identical to @code{string-tr!}, except that it does not
 *      over-write the previous value.
=*/
SCM
ag_scm_string_tr( SCM Str, SCM From, SCM To )
{
    scm_sizet lenz  = AG_SCM_STRLEN( Str );
    SCM       res   = AG_SCM_STR2SCM( AG_SCM_CHARS(Str), lenz );
    return ag_scm_string_tr_x( res, From, To );
}


/*=gfunc string_substitute
 *
 * what:  multiple global replacements
 * general_use:
 *
 *  exparg:  source, string to transform
 *  exparg:  match,  substring or substring list to be replaced
 *  exparg:  repl,   replacement strings or substrings
 *
 * doc: @code{match} and  @code{repl} may be either a single string or
 *      a list of strings.  Either way, they must have the same structure
 *      and number of elements.  For example, to replace all less than
 *      and all greater than characters, do something like this:
 *
 * @example
 *      (string-substitute source
 *      ("&"     "<"    ">")
 *      ("&amp;" "&lt;" "&gt;"))
 * @end example
=*/
SCM
ag_scm_string_substitute( SCM Str, SCM Match, SCM Repl )
{
    tCC*       pzStr;
    scm_sizet  len;
    SCM        res;

    if (! AG_SCM_STRING_P( Str ))
        return SCM_UNDEFINED;

    pzStr = AG_SCM_CHARS( Str );
    len   = AG_SCM_STRLEN( Str );

    if (AG_SCM_STRING_P( Match ))
        do_substitution( pzStr, len, Match, Repl,
                         (char**)&pzStr, &len );
    else
        do_multi_subs( (char**)&pzStr, &len, Match, Repl );

    res = AG_SCM_STR2SCM( pzStr, len );

    /*
     *  IF we have an allocated intermediate result,
     *  THEN we must free it.  We might have our original string.
     */
    if (pzStr != AG_SCM_CHARS( Str ))
        AGFREE( (char*)pzStr );
    return res;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expString.c */
