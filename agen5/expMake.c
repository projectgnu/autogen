
/**
 * @file expMake.c
 *
 *  Time-stamp:        "2011-12-18 12:12:59 bkorb"
 *
 *  This module implements Makefile construction functions.
 *
 *  This file is part of AutoGen.
 *  AutoGen Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
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
static char const zNl[]  = " ; \\\n";

static ag_bool
handle_eol(char ** ppzi, char ** ppzo, char tabch, char * bol)
{
    char * pzScn = *ppzi;
    char * pzOut = *ppzo;
    int    l_len = pzOut - bol;

    /*
     *  Backup past trailing white space (other than newline).
     */
    while (IS_WHITESPACE_CHAR(pzOut[ -1 ]) && (pzOut[ -1 ] != NL))
        pzOut--;

    /*
     *  Skip over empty lines, but leave leading white space
     *  on the next non-empty line.
     */
    {
        char* pz = pzScn;
        while (IS_WHITESPACE_CHAR(*pz)) {
            if (*(pz++) == NL)
                pzScn = pz;
        }
    }

    /*
     *  The final newline is dropped.
     */
    if (*pzScn == NUL)
        return AG_FALSE;

    switch (pzOut[-1]) {
    case '\\':
        /*
         *  The newline is already escaped, so don't
         *  insert our extra command termination.
         */
        *(pzOut++) = NL;
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
    skip_semi_colon:
        /*
         *  Whatever the reason for a final '|' or ';' we will not
         *  add a semi-colon after it.
         */
        strcpy(pzOut, zNl + 2);
        pzOut += sizeof(zNl) - 3;
        break;

    case 'n': // "then" or "in"
        if (l_len < 3)
            goto append_statement_end;

        if (pzOut[-2] == 'i') {
            if ((l_len == 3) || IS_WHITESPACE_CHAR(pzOut[-3]))
                goto skip_semi_colon;
            goto append_statement_end;
        }

        if (  (l_len < 5)
           || (  (l_len > 5)
              && ! IS_WHITESPACE_CHAR(pzOut[-5]) ))
            goto append_statement_end;
        if (strncmp(pzOut-4, "the", 3) == 0)
            goto skip_semi_colon;
        goto append_statement_end;

    case 'e': // "else"
        if (  (l_len < 5)
           || (  (l_len > 5)
              && ! IS_WHITESPACE_CHAR(pzOut[-5]) ))
            goto append_statement_end;
        if (strncmp(pzOut-4, "els", 3) == 0)
            goto skip_semi_colon;
        goto append_statement_end;

    default:
    append_statement_end:
        /*
         *  Terminate the current command and escape the newline.
         */
        strcpy(pzOut, zNl);
        pzOut += sizeof(zNl) - 1;
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

    *ppzi = pzScn;
    *ppzo = pzOut;
    return AG_TRUE;
}

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
 *  every line that does not end with certain special characters or keywords.
 *  Note that this will mutilate multi-line quoted strings, but @command{make}
 *  renders it impossible to use multi-line constructs anyway.
 *
 *  @item
 *  If the line ends with a backslash, it is left alone.
 *
 *  @item
 *  If the line ends with a semi-colon, conjunction operator,
 *  pipe (vertical bar) or one of the keywords "then", "else" or "in", then
 *  a space and a backslash is added, but no semi-colon.
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
ag_scm_makefile_script(SCM text)
{
    SCM    res;
    char*  pzText = ag_scm2zchars(text, "GPL line prefix");
    char   tabch;
    size_t sz     = strlen(pzText) + 2;

    /*
     *  skip all blank lines and other initial white space
     *  in the source string.
     */
    if (! IS_WHITESPACE_CHAR(*pzText))
        tabch = TAB;
    else {
        while (IS_WHITESPACE_CHAR(*++pzText))  ;
        tabch  = (pzText[-1] == TAB) ? NUL : TAB;
    }

    if (*pzText == NUL)
        return AG_SCM_STR02SCM(zNil);

    {
        char*  pz  = strchr(pzText, NL);
        size_t inc = ((*pzText == TAB) ? 0 : 1) + sizeof(zNl) - 1;

        while (pz != NULL) {
            sz += inc;
            pz = strchr(pz+1, NL);
        }

        pz = strchr(pzText, '$');
        while (pz != NULL) {
            sz++;
            pz = strchr(pz + 1, '$');
        }
    }

    {
        char * res_str    = AGALOC(sz, "makefile text");
        char * out_scan   = res_str;
        char * src_scan   = pzText;
        char * start_line = out_scan;
        /*
         *  Force the initial line to start with a tab.
         */
        *(out_scan++) = TAB;

        for (;;) {
            char ch = *(src_scan++);
            switch (ch) {
            case NL:
                if (! handle_eol(&src_scan, &out_scan, tabch, start_line))
                    goto copy_done;
                start_line = out_scan;
                break;

            case NUL:
                goto copy_done;

            case '$':
                /*
                 *  Double the dollar -- IFF it is not a makefile macro
                 */
                switch (*src_scan) {
                case '(': case '*': case '@': case '<': case '%': case '?':
                    break;

                case '$':
                    /*
                     *  Another special case:  $$ in the shell means process id
                     *  Avoid having to do a backward scan on the second '$'
                     *  by handling the next '$' now.  We get FOUR '$' chars.
                     */
                    src_scan++;
                    *(out_scan++) = '$';
                    *(out_scan++) = '$';  /* two down, two to go */
                    /* FALLTHROUGH */

                default:
                    *(out_scan++) = '$';
                }
                /* FALLTHROUGH */

            default:
                *(out_scan++) = ch;
            }
        } copy_done:;

        sz    = (out_scan - res_str);
        res   = AG_SCM_STR2SCM(res_str, sz);
        AGFREE(res_str);
    }

    return res;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expMake.c */
