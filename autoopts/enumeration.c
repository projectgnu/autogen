
/*
 *  $Id: enumeration.c,v 3.11 2003/04/21 03:35:35 bkorb Exp $
 *
 *   Automated Options Paged Usage module.
 *
 *  This routine will run run-on options through a pager so the
 *  user may examine, print or edit them at their leisure.
 */

/*
 *  Automated Options copyright 1992-2003 Bruce Korb
 *
 *  Automated Options is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  Automated Options is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Automated Options.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 *
 * As a special exception, Bruce Korb gives permission for additional
 * uses of the text contained in his release of AutoOpts.
 *
 * The exception is that, if you link the AutoOpts library with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the AutoOpts library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by Bruce Korb under
 * the name AutoOpts.  If you copy code from other sources under the
 * General Public License into a copy of AutoOpts, as the General Public
 * License permits, the exception does not apply to the code that you add
 * in this way.  To avoid misleading anyone as to the status of such
 * modified files, you must delete this exception notice from them.
 *
 * If you write modifications of your own for AutoOpts, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 */

tSCC*  pz_fmt;

static void
enumError( pOpts, pOD, paz_names, name_ct )
    tOptions* pOpts;
    tOptDesc* pOD;
    tCC**     paz_names;
    int       name_ct;
{
    if (pOpts != NULL)
        fprintf( option_usage_fp, pz_fmt, pOpts->pzProgName, pOD->pzLastArg );

    fprintf( option_usage_fp, "The valid %s option keywords are:\n",
             pOD->pz_Name );

    if (**paz_names == 0x7F) {
        paz_names++;
        name_ct--;
    }

    do  {
        fprintf( option_usage_fp, "\t%s\n", *(paz_names++) );
    } while (--name_ct > 0);

    if (pOpts != NULL)
        (*(pOpts->pUsageProc))( pOpts, EXIT_FAILURE );
}


/*=export_func  optionEnumerationVal
 * what:  put the option state into Guile symbols
 * private:
 *
 * arg:   tOptions*, pOpts,     the program options descriptor
 * arg:   tOptDesc*, pOD,       enumeration option description
 * arg:   tCC**,     paz_names, list of enumeration names
 * arg:   int,       name_ct,   number of names in list
 *
 * ret_type:  char*
 * ret_desc:  the enumeration value cast as a char*
 *
 * doc:   This converts the pzLastArg string from the option description
 *        into the index corresponding to an entry in the name list.
 *        This will match the generated enumeration value.
 *        Full matches are always accepted.  Partial matches are accepted
 *        if there is only one partial match.
=*/
char*
optionEnumerationVal( pOpts, pOD, paz_names, name_ct )
    tOptions* pOpts;
    tOptDesc* pOD;
    tCC**     paz_names;
    int       name_ct;
{
    size_t      len;
    int         idx;
    uintptr_t   res = -1;

    /*
     *  IF the program option descriptor pointer is invalid,
     *  then it is some sort of special request.
     */
    switch ((u_long)pOpts) {
    case 0UL:
        /*
         *  print the list of enumeration names.
         */
        enumError( pOpts, pOD, paz_names, name_ct );
        return (char*)0UL;

    case 1UL:
        /*
         *  print the name string.
         */
        fputs( paz_names[ (int)(pOD->pzLastArg) ], stdout );
        return (char*)0UL;

    case 2UL:
        /*
         *  Replace the enumeration value with the name string.
         */
        return (char*)paz_names[ (uintptr_t)(pOD->pzLastArg) ];

    default:
        break;
    }

    len = strlen( pOD->pzLastArg );

    /*
     *  Look for an exact match, but remember any partial matches.
     *  Multiple partial matches means we have an ambiguous match.
     */
    for (idx = 0; idx < name_ct; idx++) {
        if (strncmp( paz_names[idx], pOD->pzLastArg, len ) == 0) {
            if (paz_names[idx][len] == NUL)
                return (char*)idx;
            if (res != -1) {
                pz_fmt = "%s error:  the keyword `%s' is ambiguous\n";
                option_usage_fp = stderr;
                enumError( pOpts, pOD, paz_names, name_ct );
            }
            res = idx;
        }
    }

    if (res < 0) {
        pz_fmt = "%s error:  `%s' does not match any keywords\n";
        option_usage_fp = stderr;
        enumError( pOpts, pOD, paz_names, name_ct );
    }

    /*
     *  Return the matching index as a char* pointer.
     *  The result gets stashed in a char* pointer, so it will have to fit.
     */
    return (char*)res;
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/enumeration.c */
