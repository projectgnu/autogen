
/*
 *  $Id: expGperf.c,v 1.4 2000/09/30 14:59:01 bkorb Exp $
 *  This module implements the expression functions that should
 *  be part of Guile.
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

#include "autogen.h"
#include <guile/gh.h>
#include "expGuile.h"
#include "expr.h"

tSCC zMakeGperf[] =
"gperf_%2$s=.ZZPURGE.$$/%2$s\n"
"[ -d .ZZPURGE.$$ ] || mkdir .ZZPURGE.$$\n"
"( ( while ps -p %3$d ; do sleep 3 ; done\n"
"    rm -rf .ZZPURGE.$$ ) > /dev/null 2>&1 & )\n"
"cd .ZZPURGE.$$ || {\n"
"  echo 'cannot mkdir and enter .ZZPURGE.$$' >&2\n"
"  exit 1\n}\n"

"( cat <<'_EOF_'\n"
"%%{\n"
"#include <stdio.h>\n"
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

") | tee %2$s.gperf | gperf -t -D -k'*' > %2$s.c || exit 1\n"

"make %2$s || exit 1";


tSCC zRunGperf[] = "${gperf_%s} %s || exit 1";


/*=gfunc make_gperf
 *
 * what:   build a perfect hash function
 * general_use:
 *
 * exparg: name , name of hash list
 * exparg: list , list of strings to hash ,, list
 *
 * doc:  build a program to perform perfect hashes of a known
 *       list of input strings.
=*/
    SCM
ag_scm_make_gperf( SCM name, SCM hlist )
{
    SCM     newline  = gh_str2scm( "\n", 1 );
    int     nameLen;
    ag_bool mustFree = AG_FALSE;
    char*   pzList;

    if (! gh_string_p( name ))
        return SCM_UNDEFINED;

    /*
     *  Construct the newline separated list of values
     */
    hlist = ag_scm_join( newline, hlist );
    if (! gh_string_p( hlist ))
        return SCM_UNDEFINED;

    /*
     *  Put the name of the hash program into scribble memory
     */
    nameLen = SCM_LENGTH( name );
    if (nameLen > 128)
        return SCM_UNDEFINED;

    memcpy( zScribble, SCM_CHARS( name ), nameLen );
    zScribble[ nameLen ] = NUL;

    /*
     *  Stash the concatenated list somewhere, hopefully without an alloc.
     */
    {
        int listLen = SCM_LENGTH( hlist );
        char*   pzCmd;

        if ((listLen + nameLen + 2) < sizeof( zScribble ))
            pzList = zScribble + nameLen + 1;
        else {
            pzList = (char*)AGALOC( listLen + 1, "gperf string list" );
            mustFree = AG_TRUE;
        }

        memcpy( pzList, SCM_CHARS( hlist ), listLen );
        pzList[ listLen ] = NUL;

        pzCmd = asprintf( zMakeGperf, pzList, zScribble, getpid() );
        if (pzCmd == (char*)NULL) {
            fprintf( stderr, "Error using %s list:  ``%s''\n\n",
                     zScribble, pzList );
            fprintf( stderr, zMakeGperf, pzList, zScribble, getpid() );

            LOAD_ABORT( pCurTemplate, pCurMacro, "formatting shell command" );
        }
        if (mustFree)
            AGFREE( pzList );

        /*
         *  Run the command and ignore the results.
         *  In theory, the program should be ready.
         */
        pzList = runShell( pzCmd );
        AGFREE( pzCmd );
        AGFREE( pzList );
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
 * doc:  Hash the input string
=*/
    SCM
ag_scm_gperf( SCM name, SCM str )
{
    ag_bool mustFree = AG_FALSE;
    int     nameLen, strLen, cmdLen;
    char*   pzCmd;
    char*   pzStr;
    char*   pzName = zScribble;

    if ((! gh_string_p( str )) || (! gh_string_p( name )))
        return SCM_UNDEFINED;

    /*
     *  Extract the name and sample string
     */
    nameLen = SCM_LENGTH( name );
    if (nameLen > 128)
        return SCM_UNDEFINED;
    memcpy( zScribble, SCM_CHARS( name ), nameLen );
    zScribble[ nameLen ] = NUL;

    strLen = SCM_LENGTH( str );
    if ((strLen + nameLen + 2) < sizeof( zScribble ))
        pzStr = zScribble + nameLen + 1;
    else
        pzStr = (char*)AGALOC( strLen + 1, "gperf hash string" );
    memcpy( pzStr, SCM_CHARS( str ), strLen );
    pzStr[ strLen ] = NUL;

    /*
     *  Format the gperf command and check the result.  If it fits in
     *  scribble space, use that.
     *  (If it does fit, then the test string fits already).
     */
    if (  ((strLen + nameLen + 2) * 2 + sizeof( zRunGperf ))
          < sizeof( zScribble ))  {
        pzCmd = pzStr + strLen + 1;
        sprintf( pzCmd, zRunGperf, zScribble, pzStr );
        mustFree = AG_FALSE;

    } else {
        pzCmd = asprintf( zRunGperf, zScribble, pzStr );
        if (mustFree)
            AGFREE( pzStr );
        mustFree = AG_TRUE;  /* now it is ``pzCmd'' that is allocated */
    }

    pzStr = runShell( pzCmd );
    if (*pzStr == NUL)
        str = SCM_UNDEFINED;
    else
        str = gh_str02scm( pzStr );

    if (mustFree)
        AGFREE( pzCmd );
    AGFREE( pzStr );
    return str;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * end of expGperf.c */
