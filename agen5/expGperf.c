
/*
 *  $Id: expGperf.c,v 1.2 2000/09/30 02:39:34 bkorb Exp $
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
"gperf_%3$s=.ZZPURGE.$$/%3$s\n"
"[ -d .ZZPURGE.$$ ] || mkdir .ZZPURGE.$$\n"
"( ( while ps -p %4$d ; do sleep 3 ; done\n"
"    rm -rf .ZZPURGE.$$ ) > /dev/null 2>&1 & )\n"
"cd .ZZPURGE.$$ || echo 'cannot mkdir and enter .ZZPURGE.$$' >&2\n"
"( echo '#include <stdio.h>'\n"
"  gperf -D -k'*' 2> /dev/null\n"
"  cat <<'_EOF_'\n"
"int\n"
"main( int argc, char** argv )\n"
"{\n"
"    char* pz  = argv[1];\n"
"    long  adr = (long)in_word_set( pz, strlen( pz ));\n"
"    if (adr == 0)\n"
"        return 1;\n"
"    printf( \"%%04X\\n\", adr & 0x0%1$X );\n"
"    return 0;\n"
"}\n"
"_EOF_\n"
") > %3$s.c <<_EOLIST_\n"
"%2$s\n"
"_EOLIST_\n"
"make %3$s";


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
        long    mask     = 0x00FFUL;
        char*   pzCmd;

        if ((listLen + nameLen + 2) < sizeof( zScribble ))
            pzList = zScribble + nameLen + 1;
        else {
            pzList = (char*)AGALOC( listLen + 1, "gperf string list" );
            mustFree = AG_TRUE;
        }

        memcpy( pzList, SCM_CHARS( hlist ), listLen );
        pzList[ listLen ] = NUL;

        /*
         *  The algorithm requires a mask big enough to get all the
         *  distinguishing bits of the address of some strings.
         */
        while (mask < listLen)
            mask = (mask << 1) | 1UL;

        pzCmd = asprintf( zMakeGperf, mask, pzList, zScribble, getpid() );
        if (pzCmd == (char*)NULL) {
            fprintf( stderr, "Error using mask %0X, list %s for ``%s''\n\n",
                     mask, pzList, zScribble );
            fprintf( stderr, zMakeGperf, mask, pzList, zScribble, getpid() );

            LOAD_ABORT( pCurTemplate, pCurMacro, "formatting shell command" );
        }
        if (mustFree)
            AGFREE( pzList );

        /*
         *  Run the command and ignore the results.
         *  In theory, the program should be ready.
         */
        fputs( pzCmd, stderr );;
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

