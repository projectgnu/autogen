
/*
 *  $Id: enumeration.c,v 3.20 2003/11/23 19:15:28 bkorb Exp $
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

/* === STATIC PROCS === */
STATIC void
enumError(
    tOptions* pOpts,
    tOptDesc* pOD,
    tCC**     paz_names,
    int       name_ct );

STATIC uintptr_t
findName(
    tCC*          pzName,
    tOptions*     pOpts,
    tOptDesc*     pOD,
    tCC**         paz_names,
    unsigned int  name_ct );

/* === END STATIC PROCS === */

tSCC*  pz_enum_err_fmt;

STATIC void
enumError(
    tOptions* pOpts,
    tOptDesc* pOD,
    tCC**     paz_names,
    int       name_ct )
{
    if (pOpts != NULL)
        fprintf( option_usage_fp, pz_enum_err_fmt,
                 pOpts->pzProgName, pOD->pzLastArg );

    fprintf( option_usage_fp, "The valid \"%s\" option keywords are:\n",
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


STATIC uintptr_t
findName(
    tCC*          pzName,
    tOptions*     pOpts,
    tOptDesc*     pOD,
    tCC**         paz_names,
    unsigned int  name_ct )
{
    uintptr_t     res = name_ct;
    size_t        len = strlen( (char*)pzName );
    uintptr_t     idx;
    /*
     *  Look for an exact match, but remember any partial matches.
     *  Multiple partial matches means we have an ambiguous match.
     */
    for (idx = 0; idx < name_ct; idx++) {
        if (strncmp( (char*)paz_names[idx], (char*)pzName, len ) == 0) {
            if (paz_names[idx][len] == NUL)
                return idx;  /* full match */

            if (res != name_ct) {
                pz_enum_err_fmt = "%s error:  the keyword `%s' is ambiguous\n";
                option_usage_fp = stderr;
                enumError( pOpts, pOD, paz_names, name_ct );
            }
            res = idx; /* save partial match */
        }
    }

    /*
     *  no partial match -> error
     */
    if (res == name_ct) {
        pz_enum_err_fmt = "%s error:  `%s' does not match any keywords\n";
        option_usage_fp = stderr;
        enumError( pOpts, pOD, paz_names, name_ct );
    }

    /*
     *  Return the matching index as a char* pointer.
     *  The result gets stashed in a char* pointer, so it will have to fit.
     */
    return res;
}


/*=export_func  optionEnumerationVal
 * what:  Convert between enumeration values and strings
 * private:
 *
 * arg:   tOptions*,     pOpts,     the program options descriptor
 * arg:   tOptDesc*,     pOD,       enumeration option description
 * arg:   tCC**,         paz_names, list of enumeration names
 * arg:   unsigned int,  name_ct,   number of names in list
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
optionEnumerationVal(
    tOptions*     pOpts,
    tOptDesc*     pOD,
    tCC**         paz_names,
    unsigned int  name_ct )
{
    size_t        len;
    uintptr_t     idx;
    uintptr_t     res = name_ct;

    /*
     *  IF the program option descriptor pointer is invalid,
     *  then it is some sort of special request.
     */
    switch ((uintptr_t)pOpts) {
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

    return (char*)findName( pOD->pzLastArg, pOpts, pOD, paz_names, name_ct );
}


/*=export_func  optionSetMembers
 * what:  Convert between bit flag values and strings
 * private:
 *
 * arg:   tOptions*,     pOpts,     the program options descriptor
 * arg:   tOptDesc*,     pOD,       enumeration option description
 * arg:   tCC**,         paz_names, list of enumeration names
 * arg:   unsigned int,  name_ct,   number of names in list
 *
 * doc:   This converts the pzLastArg string from the option description
 *        into the index corresponding to an entry in the name list.
 *        This will match the generated enumeration value.
 *        Full matches are always accepted.  Partial matches are accepted
 *        if there is only one partial match.
=*/
void
optionSetMembers(
    tOptions*     pOpts,
    tOptDesc*     pOD,
    tCC**         paz_names,
    unsigned int  name_ct )
{
    /*
     *  IF the program option descriptor pointer is invalid,
     *  then it is some sort of special request.
     */
    switch ((uintptr_t)pOpts) {
    case 0UL:
        /*
         *  print the list of enumeration names.
         */
        enumError( pOpts, pOD, paz_names, name_ct );
        return;

    case 1UL:
    {
        /*
         *  print the name string.
         */
        uintptr_t bits = (uintptr_t)pOD->optCookie;
        uintptr_t res  = 0;
        size_t    len  = 0;

        while (bits != 0) {
            if (bits & 1) {
                if (len++ > 0) fputs( " | ", stdout );
                fputs( paz_names[ res ], stdout );
            }
            if (++res >= name_ct) break;
            bits >>= 1;
        }
        return;
    }

    case 2UL:
    {
        char*     pz;
        uintptr_t bits = (uintptr_t)pOD->optCookie;
        uintptr_t res  = 0;
        size_t    len  = 0;

        /*
         *  Replace the enumeration value with the name string.
         *  First, determine the needed length, then allocate and fill in.
         */
        while (bits != 0) {
            if (bits & 1)
                len += strlen( paz_names[ res ]) + 3;
            if (++res >= name_ct) break;
            bits >>= 1;
        }
        pOD->pzLastArg = pz = malloc( len );
        bits = (uintptr_t)pOD->optCookie;
        res = 0;
        while (bits != 0) {
            if (bits & 1) {
                if (pz != pOD->pzLastArg) {
                    strcpy( pz, " + " );
                    pz += 3;
                }
                strcpy( pz, paz_names[ res ]);
                pz += strlen( paz_names[ res ]);
            }
            if (++res >= name_ct) break;
            bits >>= 1;
        }
        return;
    }

    default:
        break;
    }

    {
        char*     pzArg = pOD->pzLastArg;
        uintptr_t res;
        if ((pzArg == NULL) || (*pzArg == NUL)) {
            pOD->optCookie = NULL;
            return;
        }

        res = (uintptr_t)pOD->optCookie;
        for (;;) {
            tSCC zSpn[] = " ,|+\t\r\f\n";
            char ch;
            int  iv, len;

            pzArg += strspn( pzArg, zSpn );
            iv = (*pzArg == '!');
            if (iv)
                pzArg += strspn( pzArg+1, zSpn ) + 1;

            len = strcspn( pzArg, zSpn );
            if (len == 0)
                break;
            ch = pzArg[len];
            pzArg[len] = NUL;

            if ((len == 3) && (strcmp( pzArg, "all" ) == 0)) {
                if (iv)
                     res = 0;
                else res = ~0;
            }
            else if ((len == 4) && (strcmp( pzArg, "none" ) == 0)) {
                if (! iv)
                    res = 0;
            }
            else {
                char* pz;
                uintptr_t bit = strtoul( pzArg, &pz, 0 );

                if (*pz != NUL)
                   bit = 1UL << findName(pzArg, pOpts, pOD, paz_names, name_ct);

                if (iv)
                     res &= ~bit;
                else res |= bit;
            }

            if (ch == NUL)
                break;
            pzArg += len;
            *(pzArg++) = ch;
        }
        if (name_ct < (8 * sizeof( uintptr_t ))) {
            res &= (1UL << name_ct) - 1UL;
        }

        pOD->optCookie = (void*)res;
    }
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/enumeration.c */
