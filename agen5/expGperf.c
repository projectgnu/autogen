/*
 *  $Id: expGperf.c,v 4.7 2005/12/04 00:57:30 bkorb Exp $
 *  This module implements the expression functions that should
 *  be part of Guile.
 */

/*
 *  AutoGen copyright 1992-2005 Bruce Korb
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

tSCC zMakeGperf[] =
"gperf_%2$s=.gperf.$$/%2$s\n"
"[ -d .gperf.$$ ] || mkdir .gperf.$$\n"

"cd .gperf.$$ || {\n"
"  echo 'cannot mkdir and enter .gperf.$$' >&2\n"
"  exit 1\n}\n"

"dir=`pwd` \n( "

/*
 *  From here to the matching closing paren, we are constructing the
 *  gperf input file with every character written to stdout
 */
"cat <<'_EOF_'\n"
"%%{\n"
"#" "include <stdio.h>\n"
"typedef struct index t_index;\n"
"%%}\n"
"struct index { char* name; int idx; };\n"
"%%%%\n"
"_EOF_\n"

"idx=1\n"
"while read f\n"
"do echo \"${f}, ${idx}\"\n"
"   idx=`expr ${idx} + 1`\n"
"done <<_EOLIST_\n"
"%1$s\n"
"_EOLIST_\n"

"cat <<'_EOF_'\n"
"%%%%\n"
"int main( int argc, char** argv ) {\n"
"    char*    pz = argv[1];\n"
"    t_index* pI = in_word_set( pz, strlen( pz ));\n"
"    if (pI == NULL)\n"
"        return 1;\n"
"    printf( \"0x%%02X\\n\", pI->idx );\n"
"    return 0;\n"
"}\n"
"_EOF_\n"

/*
 *  Now save the text into a xx.gperf file and also pass it
 *  through a pipe to gperf for it to do its thing.
 */
") | tee %2$s.gperf | gperf -t -D -k'*' > %2$s.c\n"

/*
 *  actually build the program using "make"
 */
"res=`make %2$s 2>&1`\n"
"test $? -eq 0 || die \"${res}\"\n"
"echo rm -rf ${dir}";


tSCC zRunGperf[] = "${gperf_%s} %s";


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
 *       This program will be obliterated within a few seconds after
 *       AutoGen exits.
=*/
SCM
ag_scm_make_gperf( SCM name, SCM hlist )
{
    SCM     newline  = gh_str2scm( "\n", 1 );
    char*   pzName   = ag_scm2zchars( name, "gperf name" );
    char*   pzList;

    if (! AG_SCM_STRING_P( name ))
        return SCM_UNDEFINED;

    /*
     *  Construct the newline separated list of values
     */
    hlist = ag_scm_join( newline, hlist );
    pzList = ag_scm2zchars( hlist, "hash list" );

    /*
     *  Stash the concatenated list somewhere, hopefully without an alloc.
     */
    {
        static const int makeGperfLine = __LINE__ + 2;
        static const char zCleanup[] =
            "(set! shell-cleanup (string-append shell-cleanup \"%s\n\"))\n";
        char* pzCmd;
        pzCmd = aprf( zMakeGperf, pzList, pzName, getpid() );

        /*
         *  Run the command and ignore the results.
         *  In theory, the program should be ready.
         */
        pzList = runShell( pzCmd );
        AGFREE( pzCmd );

        pzCmd = aprf( zCleanup, pzList );
        AGFREE( pzList );

        (void)ag_scm_c_eval_string_from_file_line(
            pzCmd, __FILE__, makeGperfLine );
        AGFREE( pzCmd );
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
        str = gh_str02scm( pzStr );

    AGFREE( pzCmd );
    AGFREE( pzStr );
    return str;
}
#endif
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expGperf.c */
