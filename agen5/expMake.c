
/*  -*- Mode: C -*-
 *
 *  expMake.c
 *  $Id: expMake.c,v 3.2 2002/01/12 05:10:01 bkorb Exp $
 *  This module implements Makefile construction functions.
 */

/*
 *  AutoGen copyright 1992-2002 Bruce Korb
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

#include "expr.h"
#include "autogen.h"

/*=gfunc makefile_script
 *
 * what:  create makefile script
 * general_use:
 *
 * exparg: text, the text of the script
 *
 * doc:
 *  This function will take ordinary shell script text and reformat it
 *  so that it will work properly inside of a shell script.
 *  It does this by:
 *
 *  @enumerate
 *  @item
 *  Except for the last line, putting the string, " ; \" on the end of every
 *  line that does not end with a backslash.
 *
 *  @item
 *  Doubling the dollar sign character, unless it immediately precedes an
 *  opening parenthesis or the single character make macros '*', '<', '@@',
 *  '?' or '%'.  Other single character make macros without enclosing
 *  parentheses will fail.  For shell usage of the "$@@", "$?" and "$*"
 *  macros, you must enclose them with curly braces, i.e., "$@{?@}".
 *
 *  @item
 *  Prefixes every line with a tab, unless the first line
 *  already starts with a tab.
 *
 *  @item
 *  The newline character on the last line, if present, is suppressed.
 *  @end enumerate
 *
 *  @noindent
 *  This function is intended to be used approximately as follows:
 *
 *  @example
 *  $(TARGET) : $(DEPENDENCIES)
 *  <+ (out-push-new) +>
 *  ....arbitrary shell script text....
 *  <+ (makefile-script (out-pop #t)) +>
 *  @end example
=*/
SCM
ag_scm_makefile_script( SCM text )
{
    tSCC   zNl[]  = " ; \\\n";

    SCM    res;
    char*  pzText = ag_scm2zchars( text, "GPL line prefix" );
    char   tabch  = (*pzText == '\t') ? NUL : '\t';
    size_t sz     = strlen( pzText ) + 2;

    /*
     *  skip all blank lines and other initial white space
     *  in the source string.
     */
    while (isspace( *pzText ))  pzText++;

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
                 *  If the newline is already escaped, then we won't
                 *  insert our extra command termination
                 */
                if (pzOut[-1] == '\\') {
                    *(pzOut++) = ch;
                    if (tabch)
                        *(pzOut++) = tabch;
                    break;
                }

                /*
                 *  The final newline is dropped.
                 */
                if (*pzScn == NUL)
                    goto copy_done;

                /*
                 *  Terminate the current command and escape the newline
                 *  Indent, too, but only if the first line was *not*.
                 */
                strcpy( pzOut, zNl );
                pzOut += sizeof( zNl ) - 1;
                if (tabch)
                    *(pzOut++) = tabch;
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
                    *(pzOut++) = '$';
                    *(pzOut++) = '$';
                    *(pzOut++) = '$';
                    *(pzOut++) = '$';
                    pzScn++;
                    continue;

                default:
                    *(pzOut++) = '$';
                }
                /* FALLTHROUGH */

            default:
                *(pzOut++) = ch;
            }
        } copy_done:;

        /*
         *  Strip trailing white space
         */
        while (   (pzOut > pzRes)
               && isspace( pzOut[-1] ) )  pzOut--;

        /*
         *  IF the text does not start with a tab, we insert our own.
         *  However, so far we have not accommodated the first one.
         *  Add it to the allocation requirement.
         */
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
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of expMake.c */
