/*
 *  $Id: expGperf.c,v 3.14 2004/02/01 21:26:45 bkorb Exp $
 *  This module implements the expression functions that should
 *  be part of Guile.
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

tSCC zMakeGperf[] =
"gperf_%2$s=.ZZPURGE.$$/%2$s\n"
"[ -d .ZZPURGE.$$ ] || mkdir .ZZPURGE.$$\n"

/*
 *  The following little scriplet will ensure that the junk we leave
 *  around will be removed not more than three seconds after we finish.
 *  The third argument to this format string is the process id of AutoGen.
 */
"( ( while ps -p %3$d ; do sleep 3 ; done\n"
"    rm -rf .ZZPURGE.$$ ) > /dev/null 2>&1 & )\n"

"cd .ZZPURGE.$$ || {\n"
"  echo 'cannot mkdir and enter .ZZPURGE.$$' >&2\n"
"  exit 1\n}\n"

"( "

/*
 *  From here to the matching closing paren, we are constructing the
 *  pgerf input file with every character written to stdout
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
"make %2$s || exit 1";


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

    if (! gh_string_p( name ))
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
        char* pzCmd;
        pzCmd = aprf( zMakeGperf, pzList, pzName, getpid() );

        /*
         *  Run the command and ignore the results.
         *  In theory, the program should be ready.
         */
        AGFREE( runShell( pzCmd ));
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
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expGperf.c */
