/*
 *  $Id: expGperf.c,v 4.13 2006/09/24 02:19:09 bkorb Exp $
 *  This module implements the expression functions that should
 *  be part of Guile.
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
#ifndef SHELL_ENABLED
SCM
ag_scm_make_gperf( SCM name, SCM hlist )
{
    return SCM_UNDEFINED;
}

SCM
ag_scm_gperf( SCM name, SCM str )
{
    return SCM_UNDEFINED;
}
#else

/*=gfunc make_gperf
 *
 * what:   build a perfect hash function program
 * general_use:
 *
 * exparg: name , name of hash list
 * exparg: strings , list of strings to hash ,, list
 *
 * doc:  Build a program to perform perfect hashes of a known list of input
 *       strings.  This function produces no output, but prepares a program
 *       named, @file{gperf_<name>} for use by the gperf function
 *       @xref{SCM gperf}.
 *
 *       This program will be obliterated as AutoGen exits.
 *       However, you may incorporate the generated hashing function
 *       into your C program with commands something like the following:
 *
 *       @example
 *       [+ (shellf "sed '/^int main(/,$d;/^#line/d' $@{gpdir@}/%s.c"
 *                  name ) +]
 *       @end example
 *
 *       where @code{name} matches the name provided to this @code{make-perf}
 *       function.  @cose{gpdir} is the variable used to store the name of the
 *       temporary directory used to stash all the files.
=*/
SCM
ag_scm_make_gperf( SCM name, SCM hlist )
{
    SCM     newline  = AG_SCM_STR2SCM( "\n", 1 );
    char*   pzName   = ag_scm2zchars( name, "gperf name" );
    char*   pzList;

    if (! AG_SCM_STRING_P( name ))
        return SCM_UNDEFINED;

    /*
     *  Construct the newline separated list of values
     */
    hlist  = ag_scm_join( newline, hlist );
    pzList = ag_scm2zchars( hlist, "hash list" );

    /*
     *  Stash the concatenated list somewhere, hopefully without an alloc.
     */
    {
        static int  const  makeGperfLine = __LINE__ + 2;
        static char const* pzCleanup =
            "(add-cleanup \"rm -rf ${gpdir}\")";
        char* pzCmd = aprf( zMakeGperf, pzList, pzName );

        /*
         *  Run the command and ignore the results.
         *  In theory, the program should be ready.
         */
        pzList = runShell( pzCmd );
        AGFREE( pzCmd );
        if (pzList != NULL)
            free( pzList );

        if (pzCleanup != NULL) {
            (void)ag_scm_c_eval_string_from_file_line(
                pzCleanup, __FILE__, makeGperfLine );
            pzCleanup = NULL;
        }
    }
    return SCM_BOOL_T;
}


/*=gfunc gperf
 *
 * what:   perform a perfect hash function
 * general_use:
 *
 * exparg: name , name of hash list
 * exparg: str  , string to hash
 *
 * doc:  Perform the perfect hash on the input string.  This is only useful if
 *       you have previously created a gperf program with the @code{make-gperf}
 *       function @xref{SCM make-gperf}.  The @code{name} you supply here must
 *       match the name used to create the program and the string to hash must
 *       be one of the strings supplied in the @code{make-gperf} string list.
 *       The result will be a perfect hash index.
 *
 *       See the documentation for @command{gperf(1GNU)} for more details.
=*/
SCM
ag_scm_gperf( SCM name, SCM str )
{
    char*   pzCmd;
    char*   pzStr  = ag_scm2zchars( str,  "key-to-hash" );
    char*   pzName = ag_scm2zchars( name, "gperf name" );

    /*
     *  Format the gperf command and check the result.  If it fits in
     *  scribble space, use that.
     *  (If it does fit, then the test string fits already).
     */
    pzCmd = aprf( zRunGperf, pzName, pzStr );
    pzStr = runShell( pzCmd );
    if (*pzStr == NUL)
        str = SCM_UNDEFINED;
    else
        str = AG_SCM_STR02SCM( pzStr );

    AGFREE( pzCmd );
    AGFREE( pzStr );
    return str;
}
#endif
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expGperf.c */
