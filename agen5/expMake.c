
/*  -*- Mode: C -*-
 *
 *  expMake.c
 *  $Id: expMake.c,v 4.1 2005/01/01 00:20:57 bkorb Exp $
 *  This module implements Makefile construction functions.
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

/*=gfunc makefile_script
 *
 * what:  create makefile script
 * general_use:
 *
 * exparg: text, the text of the script
 *
 * doc:
 *  This function will take ordinary shell script text and reformat it
 *  so that it will work properly inside of a makefile shell script.
 *  Not every shell construct can be supported; the intent is to have
 *  most ordinary scripts work without much, if any, alteration.
 *
 *  The following transformations are performed on the source text:
 *
 *  @enumerate
 *  @item
 *  Trailing whitespace on each line is stripped.
 *
 *  @item
 *  Except for the last line, the string, " ; \\" is appended to the end of
 *  every line that does not end with a backslash, semi-colon,
 *  conjunction operator or pipe.  Note that this will mutilate multi-line
 *  quoted strings, but @command{make} renders it impossible to use multi-line
 *  constructs anyway.
 *
 *  @item
 *  If the line ends with a backslash, it is left alone.
 *
 *  @item
 *  If the line ends with one of the excepted operators, then a space and
 *  backslash is added.
 *
 *  @item
 *  The dollar sign character is doubled, unless it immediately precedes an
 *  opening parenthesis or the single character make macros '*', '<', '@@',
 *  '?' or '%'.  Other single character make macros that do not have enclosing
 *  parentheses will fail.  For shell usage of the "$@@", "$?" and "$*"
 *  macros, you must enclose them with curly braces, e.g., "$@{?@}".
 *  The ksh construct @code{$(<command>)} will not work.  Though some
 *  @command{make}s accept @code{$@{var@}} constructs, this function will
 *  assume it is for shell interpretation and double the dollar character.
 *  You must use @code{$(var)} for all @command{make} substitutions.
 *
 *  @item
 *  Double dollar signs are replaced by four before the next character
 *  is examined.
 *
 *  @item
 *  Every line is prefixed with a tab, unless the first line
 *  already starts with a tab.
 *
 *  @item
 *  The newline character on the last line, if present, is suppressed.
 *
 *  @item
 *  Blank lines are stripped.
 *  @end enumerate
 *
 *  @noindent
 *  This function is intended to be used approximately as follows:
 *
 *  @example
 *  $(TARGET) : $(DEPENDENCIES)
 *  <+ (out-push-new) +>
 *  ....mostly arbitrary shell script text....
 *  <+ (makefile-script (out-pop #t)) +>
 *  @end example
=*/
SCM
ag_scm_makefile_script( SCM text )
{
    tSCC   zNl[]  = " ; \\\n";

    SCM    res;
    char*  pzText = ag_scm2zchars( text, "GPL line prefix" );
    char   tabch;
    size_t sz     = strlen( pzText ) + 2;

    /*
     *  skip all blank lines and other initial white space
     *  in the source string.
     */
    if (! isspace( *pzText ))
        tabch = '\t';
    else {
        while (isspace( *++pzText ))  ;
        tabch  = (pzText[-1] == '\t') ? NUL : '\t';
    }

    if (*pzText == NUL)
        return gh_str02scm( "" );

    {
        char*  pz  = strchr( pzText, '\n' );
        size_t inc = ((*pzText == '\t') ? 0 : 1) + sizeof( zNl ) - 1;

        while (pz != NULL) {
            sz += inc;
            pz = strchr( pz+1, '\n' );
        }

        pz = strchr( pzText, '$' );
        while (pz != NULL) {
            sz++;
            pz = strchr( pz + 1, '$' );
        }
    }

    {
        char* pzRes = AGALOC( sz, "makefile text" );
        char* pzOut = pzRes;
        char* pzScn = pzText;

        /*
         *  Force the initial line to start with a tab.
         */
        *(pzOut++) = '\t';

        for (;;) {
            char ch = *(pzScn++);
            switch (ch) {
            case '\n':
                /*
                 *  Backup past trailing white space (other than newline).
                 */
                while (isspace( pzOut[ -1 ]) && (pzOut[ -1 ] != '\n'))
                    pzOut--;

                /*
                 *  Skip over empty lines, but leave leading white space
                 *  on the next non-empty line.
                 */
                {
                    char* pz = pzScn;
                    while (isspace( *pz )) {
                        if (*(pz++) == '\n')
                            pzScn = pz;
                    }
                }

                /*
                 *  The final newline is dropped.
                 */
                if (*pzScn == NUL)
                    goto copy_done;

                switch (pzOut[-1]) {
                case '\\':
                    /*
                     *  The newline is already escaped, so don't
                     *  insert our extra command termination.
                     */
                    *(pzOut++) = '\n';
                    break;

                case '&':
                    /*
                     *  A single ampersand is a backgrounded command.
                     *  We must terminate those statements, but not
                     *  statements conjoined with '&&'.
                     */
                    if ('&' != pzOut[-2])
                        goto append_statement_end;
                    /* FALLTHROUGH */

                case '|':
                case ';':
                    /*
                     *  Whatever the reason for a final '|' or ';' we will not
                     *  add a semi-colon after it.
                     */
                    strcpy( pzOut, zNl + 2 );
                    pzOut += sizeof( zNl ) - 3;
                    break;

                default:
                append_statement_end:
                    /*
                     *  Terminate the current command and escape the newline.
                     */
                    strcpy( pzOut, zNl );
                    pzOut += sizeof( zNl ) - 1;
                }

                /*
                 *  We have now started our next output line and there are
                 *  still data.  Indent with a tab, if called for.  If we do
                 *  insert a tab, then skip leading tabs on the line.
                 */
                if (tabch) {
                    *(pzOut++) = tabch;
                    while (*pzScn == tabch)  pzScn++;
                }
                break;

            case NUL:
                goto copy_done;

            case '$':
                /*
                 *  Double the dollar -- IFF it is not a makefile macro
                 */
                switch (*pzScn) {
                case '(': case '*': case '@': case '<': case '%': case '?':
                    break;

                case '$':
                    /*
                     *  Another special case:  $$ in the shell means process id
                     *  Avoid having to do a backward scan on the second '$'
                     *  by handling the next '$' now.  We get FOUR '$' chars.
                     */
                    pzScn++;
                    *(pzOut++) = '$';
                    *(pzOut++) = '$';  /* two down, two to go */
                    /* FALLTHROUGH */

                default:
                    *(pzOut++) = '$';
                }
                /* FALLTHROUGH */

            default:
                *(pzOut++) = ch;
            }
        } copy_done:;

        sz    = (pzOut - pzRes);
        res   = scm_makstr( sz, 0 );
        pzOut = SCM_CHARS( res );

        memcpy( pzOut, pzRes, sz );

        AGFREE( pzRes );
    }

    return res;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expMake.c */
