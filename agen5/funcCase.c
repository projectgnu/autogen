
/*
 *  $Id: funcCase.c,v 3.12 2003/04/13 21:42:13 bkorb Exp $
 *
 *  This module implements the CASE text function.
 */
/*=--subblock=exparg=arg_name,arg_desc,arg_optional,arg_list=*/
/*
 *  AutoGen copyright 1992-2003 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */
#ifndef DEFINE_LOAD_FUNCTIONS

#include "autogen.h"

#include REGEX_HEADER

#undef  IS_LOW
#define IS_LOW(c)  (((c) <= 'z') && ((c) >= 'a'))

#ifndef _toupper
#  ifdef __toupper
#    define _toupper(c)     __toupper(c)
#  else
#    define _toupper(c)     toupper(c)
#  endif
#endif

#define PTRUP(p) STMTS(if(IS_LOW(*(p)))*(p)=_toupper(*(p));(p)++)

tSCC zBadRe[]  = "Invalid regular expression:  error %d (%s):\n%s";

STATIC void     compile_re( regex_t* pRe, char* pzPat, int flags );
STATIC void     upString( char* pz );

typedef tSuccess (tSelectProc)( char* pzText, char* pzMatch );
STATIC tSelectProc
    Select_Compare,
    Select_Compare_End,
    Select_Compare_Start,
    Select_Compare_Full,
    Select_Equivalent,
    Select_Equivalent_End,
    Select_Equivalent_Start,
    Select_Equivalent_Full,
    Select_Match,
    Select_Match_End,
    Select_Match_Start,
    Select_Match_Full,
    Select_Match_Always;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

STATIC void
compile_re( regex_t* pRe, char* pzPat, int flags )
{
    int  rerr = regcomp( pRe, pzPat, flags );
    if (rerr != 0) {
        char zEr[ 128 ];
        regerror( rerr, pRe, zEr, sizeof( zEr ));
        fprintf( stderr, zBadRe, rerr, zEr, pzPat );
        AG_ABEND( "Bad regular expression" );
    }
}


STATIC void
upString( char* pz )
{
    while (*pz != NUL)
        PTRUP(pz);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_contains_p
 *
 * what:   substring match
 * general_use:
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*==*"
 *
 * doc:  Test to see if a string contains a substring.  "strstr(3)"
 *       will find an address.
=*/
STATIC tSuccess
Select_Compare( char* pzText, char* pzMatch )
{
    return (strstr( pzText, pzMatch )) ? SUCCESS : FAILURE;
}

    SCM
ag_scm_string_contains_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );

    return (strstr( pzText, pzSubstr) == NULL) ? SCM_BOOL_F : SCM_BOOL_T;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_ends_with_p
 *
 * what:   string ending
 * general_use:
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*=="
 *
 * doc:  Test to see if a string ends with a substring.
 *       strcmp(3) returns zero for comparing the string ends.
=*/
STATIC tSuccess
Select_Compare_End( char* pzText, char* pzMatch )
{
    size_t   vlen = strlen( pzMatch );
    size_t   tlen = strlen( pzText );
    tSuccess res;

    if (tlen < vlen)
        res = FAILURE;
    else if (strcmp( pzText + (tlen - vlen), pzMatch ) == 0)
         res = SUCCESS;
    else res = FAILURE;

    return res;
}

    SCM
ag_scm_string_ends_with_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    return (SUCCESSFUL( Select_Compare_End( pzText, pzSubstr )))
        ? SCM_BOOL_T : SCM_BOOL_F;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_starts_with_p
 *
 * what:   string starting
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "==*"
 *
 * doc:  Test to see if a string starts with a substring.
=*/
STATIC tSuccess
Select_Compare_Start( char* pzText, char* pzMatch )
{
    size_t  vlen = strlen( pzMatch );
    tSuccess res;

    if (strncmp( pzText, pzMatch, vlen ) == 0)
         res = SUCCESS;
    else res = FAILURE;

    return res;
}

    SCM
ag_scm_string_starts_with_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    return (SUCCESSFUL( Select_Compare_Start( pzText, pzSubstr )))
        ? SCM_BOOL_T : SCM_BOOL_F;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_equals_p
 *
 * what:   string matching
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "=="
 *
 * doc:  Test to see if two strings exactly match.
=*/
STATIC tSuccess
Select_Compare_Full( char* pzText, char* pzMatch )
{
    return (strcmp( pzText, pzMatch ) == 0) ? SUCCESS : FAILURE;
}

    SCM
ag_scm_string_equals_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );

    return (strcmp( pzText, pzSubstr) == 0) ? SCM_BOOL_T : SCM_BOOL_F;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_contains_eqv_p
 *
 * what:   caseless substring
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*=*"
 *
 * doc:  Test to see if a string contains an equivalent string.
 *       `equivalent' means the strings match, but without regard
 *       to character case and certain characters are considered `equivalent'.
 *       Viz., '-', '_' and '^' are equivalent.
=*/
STATIC tSuccess
Select_Equivalent( char* pzText, char* pzMatch )
{
    char*    pz;
    tSuccess res = SUCCESS;
    AGDUPSTR( pz, pzText, "equiv chars" );
    upString( pz );
    if (strstr( pz, pzMatch ) == NULL)
        res = FAILURE;
    AGFREE( (void*)pz );

    return res;
}

    SCM
ag_scm_string_contains_eqv_p( SCM text, SCM substr )
{
    tSCC zSrch[] = "search string";
    char* pzSubstr;
    SCM   res;

    AGDUPSTR( pzSubstr, ag_scm2zchars( substr, zSrch ), "substring" );

    upString( pzSubstr );
    if (SUCCESSFUL( Select_Equivalent( ag_scm2zchars( text, "sample text" ),
                                       pzSubstr )))
         res = SCM_BOOL_T;
    else res = SCM_BOOL_F;
    AGFREE( (void*)pzSubstr );
    return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_ends_eqv_p
 *
 * what:   caseless string ending
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*="
 *
 * doc:  Test to see if a string ends with an equivalent string.
=*/
STATIC tSuccess
Select_Equivalent_End( char* pzText, char* pzMatch )
{
    size_t   vlen = strlen( pzMatch );
    size_t   tlen = strlen( pzText );

    if (tlen < vlen)
        return FAILURE;

    return (streqvcmp( pzText + (tlen - vlen), pzMatch ) == 0)
           ? SUCCESS
           : FAILURE;
}

    SCM
ag_scm_string_ends_eqv_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    return (SUCCESSFUL( Select_Equivalent_End( pzText, pzSubstr )))
        ? SCM_BOOL_T : SCM_BOOL_F;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_starts_eqv_p
 *
 * what:   caseless string start
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "=*"
 *
 * doc:  Test to see if a string starts with an equivalent string.
=*/
STATIC tSuccess
Select_Equivalent_Start( char* pzText, char* pzMatch )
{
    size_t   vlen = strlen( pzMatch );

    return (strneqvcmp( pzText, pzMatch, vlen ) == 0)
           ? SUCCESS
           : FAILURE;
}

    SCM
ag_scm_string_starts_eqv_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    return (SUCCESSFUL( Select_Equivalent_Start( pzText, pzSubstr )))
        ? SCM_BOOL_T : SCM_BOOL_F;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_eqv_p
 *
 * what:   caseless string match
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "="
 *
 * doc:  Test to see if two strings are equivalent.  `equivalent' means the
 *       strings match, but without regard to character case and certain
 *       characters are considered `equivalent'.  Viz., '-', '_' and '^' are
 *       equivalent.  If the arguments are not strings, then the result of the
 *       numeric comparison is returned.
 *
 *       This is an overloaded operation.  If the arguments are not both
 *       strings, then the query is passed through to @code{scm_num_eq_p()}.
=*/
STATIC tSuccess
Select_Equivalent_Full( char* pzText, char* pzMatch )
{
    return (streqvcmp( pzText, pzMatch ) == 0) ? SUCCESS : FAILURE;
}

    SCM
ag_scm_string_eqv_p( SCM text, SCM substr )
{
    char* pzText;
    char* pzSubstr;

    /*
     *  We are overloading the "=" operator.  Our arguments may be
     *  numbers...
     */
    if (! gh_string_p( text ) || ! gh_string_p( substr ))
        return scm_num_eq_p( text, substr );

    pzText   = ag_scm2zchars( text, "text to match" );
    pzSubstr = ag_scm2zchars( substr, "match expr" );

    return (streqvcmp( pzText, pzSubstr) == 0) ? SCM_BOOL_T : SCM_BOOL_F;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_has_match_p
 *
 * what:   contained regex match
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*~~*"
 *
 * doc:  Test to see if a string contains a pattern.
 *       Case is significant.
=*/
/*=gfunc string_has_eqv_match_p
 *
 * what:   caseless regex contains
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*~*"
 *
 * doc:  Test to see if a string contains a pattern.
 *       Case is not significant.
=*/
STATIC tSuccess
Select_Match( char* pzText, char* pzMatch )
{
    /*
     *  On the first call for this macro, compile the expression
     */
    if (pCurMacro->funcPrivate == NULL) {
        regex_t*  pRe = AGALOC( sizeof( *pRe ), "select match re" );
        compile_re( pRe, pzMatch, pCurMacro->res );
        pCurMacro->funcPrivate = (void*)pRe;
    }

    if (regexec( (regex_t*)pCurMacro->funcPrivate, pzText, 0,
                 NULL, 0 ) != 0)
        return FAILURE;
    return SUCCESS;
}

    SCM
ag_scm_string_has_match_p( SCM text, SCM substr )
{
    SCM      res;
    regex_t  re;

    compile_re( &re, ag_scm2zchars( substr, "match expr" ), REG_EXTENDED );

    if (regexec( &re, ag_scm2zchars( text, "text to match" ), 0,
                 NULL, 0 ) == 0)
         res = SCM_BOOL_T;
    else res = SCM_BOOL_F;
    regfree( &re );

    return res;
}

    SCM
ag_scm_string_has_eqv_match_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    SCM      res;
    regex_t  re;

    compile_re( &re, pzSubstr, REG_EXTENDED | REG_ICASE );

    if (regexec( &re, pzText, 0, NULL, 0 ) == 0)
         res = SCM_BOOL_T;
    else res = SCM_BOOL_F;
    regfree( &re );

    return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_end_match_p
 *
 * what:   regex match end
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*~~"
 *
 * doc:  Test to see if a string ends with a pattern.
 *       Case is significant.
=*/
/*=gfunc string_end_eqv_match_p
 *
 * what:   caseless regex ending
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "*~"
 *
 * doc:  Test to see if a string ends with a pattern.
 *       Case is not significant.
=*/
STATIC tSuccess
Select_Match_End( char* pzText, char* pzMatch )
{
    regmatch_t m[2];
    /*
     *  On the first call for this macro, compile the expression
     */
    if (pCurMacro->funcPrivate == NULL) {
        regex_t*  pRe = AGALOC( sizeof( *pRe ), "select match end re" );
        compile_re( pRe, pzMatch, pCurMacro->res );
        pCurMacro->funcPrivate = (void*)pRe;
    }

    if (regexec( (regex_t*)pCurMacro->funcPrivate, pzText, 2, m, 0 ) != 0)
        return FAILURE;
    if (m[0].rm_eo != strlen( pzText ))
        return FAILURE;
    return SUCCESS;
}

    SCM
ag_scm_string_end_match_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    SCM        res;
    regex_t    re;
    regmatch_t m[2];

    compile_re( &re, pzSubstr, REG_EXTENDED );

    if (regexec( &re, pzText, 2, m, 0 ) != 0)
         res = SCM_BOOL_F;
    else if (m[0].rm_eo != strlen( pzText ))
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    regfree( &re );

    return res;
}

    SCM
ag_scm_string_end_eqv_match_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    SCM        res;
    regex_t    re;
    regmatch_t m[2];

    compile_re( &re, pzSubstr, REG_EXTENDED | REG_ICASE );

    if (regexec( &re, pzText, 2, m, 0 ) != 0)
         res = SCM_BOOL_F;
    else if (m[0].rm_eo != strlen( pzText ))
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    regfree( &re );

    return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_start_match_p
 *
 * what:   regex match start
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "~~*"
 *
 * doc:  Test to see if a string starts with a pattern.
 *       Case is significant.
=*/
/*=gfunc string_start_eqv_match_p
 *
 * what:   caseless regex start
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "~*"
 *
 * doc:  Test to see if a string starts with a pattern.
 *       Case is not significant.
=*/
STATIC tSuccess
Select_Match_Start( char* pzText, char* pzMatch )
{
    regmatch_t m[2];
    /*
     *  On the first call for this macro, compile the expression
     */
    if (pCurMacro->funcPrivate == NULL) {
        regex_t*  pRe = AGALOC( sizeof( *pRe ), "select match start re" );
        compile_re( pRe, pzMatch, pCurMacro->res );
        pCurMacro->funcPrivate = (void*)pRe;
    }

    if (regexec( (regex_t*)pCurMacro->funcPrivate, pzText, 2, m, 0 ) != 0)
        return FAILURE;
    if (m[0].rm_so != 0)
        return FAILURE;
    return SUCCESS;
}

    SCM
ag_scm_string_start_match_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    SCM        res;
    regex_t    re;
    regmatch_t m[2];

    compile_re( &re, pzSubstr, REG_EXTENDED );

    if (regexec( &re, pzText, 2, m, 0 ) != 0)
         res = SCM_BOOL_F;
    else if (m[0].rm_so != 0)
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    regfree( &re );

    return res;
}

    SCM
ag_scm_string_start_eqv_match_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    SCM        res;
    regex_t    re;
    regmatch_t m[2];

    compile_re( &re, pzSubstr, REG_EXTENDED | REG_ICASE );

    if (regexec( &re, pzText, 2, m, 0 ) != 0)
         res = SCM_BOOL_F;
    else if (m[0].rm_so != 0)
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    regfree( &re );

    return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc string_match_p
 *
 * what:   regex match
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "~~"
 *
 * doc:  Test to see if a string fully matches a pattern.
 *       Case is significant.
=*/
/*=gfunc string_eqv_match_p
 *
 * what:   caseless regex match
 * general_use:
 *
 * exparg: text, text to test for pattern
 * exparg: match, pattern/substring to search for
 *
 * string: "~"
 *
 * doc:  Test to see if a string fully matches a pattern.
 *       Case is not significant, but any character equivalences
 *       must be expressed in your regular expression.
=*/
STATIC tSuccess
Select_Match_Full( char* pzText, char* pzMatch )
{
    regmatch_t m[2];

    /*
     *  On the first call for this macro, compile the expression
     */
    if (pCurMacro->funcPrivate == NULL) {
        regex_t*  pRe = AGALOC( sizeof( *pRe ), "select match full re" );
        if (OPT_VALUE_TRACE > TRACE_EXPRESSIONS) {
            fprintf( pfTrace, "Compiling ``%s'' with bits 0x%X\n",
                     pzMatch, pCurMacro->res );
        }
        compile_re( pRe, pzMatch, pCurMacro->res );
        pCurMacro->funcPrivate = pRe;
    }

    if (regexec( (regex_t*)pCurMacro->funcPrivate, pzText, 2, m, 0 ) != 0)
        return FAILURE;

    if (  (m[0].rm_eo != strlen( pzText ))
       || (m[0].rm_so != 0) )
        return FAILURE;
    return SUCCESS;
}

    SCM
ag_scm_string_match_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    SCM        res;
    regex_t    re;
    regmatch_t m[2];

    compile_re( &re, pzSubstr, REG_EXTENDED );

    if (regexec( &re, pzText, 2, m, 0 ) != 0)
         res = SCM_BOOL_F;
    else if (  (m[0].rm_eo != strlen( pzText ))
            || (m[0].rm_so != 0) )
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    regfree( &re );

    return res;
}

    SCM
ag_scm_string_eqv_match_p( SCM text, SCM substr )
{
    char* pzText   = ag_scm2zchars( text, "text to match" );
    char* pzSubstr = ag_scm2zchars( substr, "match expr" );
    SCM        res;
    regex_t    re;
    regmatch_t m[2];

    compile_re( &re, pzSubstr, REG_EXTENDED | REG_ICASE );

    if (regexec( &re, pzText, 2, m, 0 ) != 0)
         res = SCM_BOOL_F;
    else if (  (m[0].rm_eo != strlen( pzText ))
            || (m[0].rm_so != 0) )
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    regfree( &re );

    return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 *  We don't bother making a Guile function for this one :)
 */
STATIC tSuccess
Select_Match_Always( char* pzText, char* pzMatch )
{
    return SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=macfunc CASE
 *
 *  what:   Select one of several template blocks
 *  handler_proc:
 *
 *  desc:
 *
 *  The arguments are evaluated and converted to a string, if necessary.
 *  (@pxref{EXPR})  The scope of the macro is up to the matching ESAC
 *  function.  Within the scope of a CASE, this string is matched against
 *  case selection macros.  There are sixteen match macros that are derived
 *  from four different ways the test may be performed, plus an "always
 *  true" match.  The code for each selection expression is formed as follows:
 *
 *  @enumerate
 *  @item
 *  Must the match start matching from the beginning of the string?
 *  If not, then the match macro code starts with an asterisk (@code{*}).
 *  @item
 *  Must the match finish matching at the end of the string?
 *  If not, then the match macro code ends with an asterisk (@code{*}).
 *  @item
 *  Is the match a pattern match or a string comparison?
 *  If a comparison, use an equal sign (@code{=}).
 *  If a pattern match, use a tilde (@code{~}).
 *  @item
 *  Is the match case sensitive?
 *  If alphabetic case is important, double the tilde or equal sign.
 *  @item
 *  Do you need a default match when none of the others match?
 *  Use a single asterisk (@code{*}).
 *  @end enumerate
 *
 *  @noindent
 *  For example:
 *
 *  @example
 *  [+ CASE <full-expression> +]
 *  [+ ~~*  "[Tt]est" +]reg exp must match at start, not at end
 *  [+ ==   "TeSt"    +]a full-string, case sensitive compare
 *  [+ =    "TEST"    +]a full-string, case insensitive compare
 *  [+ *              +]always match - no testing
 *  [+ ESAC +]
 *  @end example
 *
 *  @code{<full-expression>} (@pxref{expression syntax}) may be any expression,
 *  including the use of apply-codes and value-names.  If the expression yields
 *  a number, it is converted to a decimal string.
 *
 *  These case selection codes have also been implemented as
 *  Scheme expression functions using the same codes
 *  (@pxref{Common Functions}).
=*/
/*=macfunc ESAC
 *
 *  what:   Terminate the @code{CASE} Template Block
 *
 *  desc:
 *    This macro ends the @code{CASE} function template block.
 *    For a complete description, @xref{CASE}.
=*/
    tMacro*
mFunc_Case( tTemplate* pT, tMacro* pMac )
{
    typedef tSuccess (t_match_proc)( char*, char* );
    /*
     *  There are only 13 procedures because the case insenstive matching
     *  get mapped into the previous four.  The last is "match always".
     */
    static t_match_proc* match_procs[] = {
        &Select_Compare_Full,
        &Select_Compare_End,
        &Select_Compare_Start,
        &Select_Compare,

        &Select_Equivalent_Full,
        &Select_Equivalent_End,
        &Select_Equivalent_Start,
        &Select_Equivalent,

        &Select_Match_Full,
        &Select_Match_End,
        &Select_Match_Start,
        &Select_Match,

        &Select_Match_Always
    };

    tSCC* apzMatchName[] = {
        "COMPARE_FULL",
        "COMPARE_END",
        "COMPARE_START",
        "CONTAINS",

        "EQUIVALENT_FULL",
        "EQUIVALENT_END",
        "EQUIVALENT_START",
        "EQUIV_CONTAINS",

        "MATCH_FULL",
        "MATCH_END",
        "MATCH_START",
        "MATCH_WITHIN",

        "MATCH_ALWAYS"
    };

    tMacro*   pEnd = pT->aMacros + pMac->endIndex;
    ag_bool needFree;
    char* pzSampleText = evalExpression( &needFree );

    /*
     *  Search through the selection clauses until we either
     *  reach the end of the list for this CASE macro, or we match.
     */
    for (;;) {
        tSuccess mRes;
        pMac = pT->aMacros + pMac->sibIndex;
        if (pMac >= pEnd) {
            if (OPT_VALUE_TRACE >= TRACE_BLOCK_MACROS) {
                fprintf( pfTrace, "CASE string `%s' did not match\n",
                         pzSampleText );

                if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
                    fprintf( pfTrace, zFileLine, pCurTemplate->pzFileName,
                             pMac->lineNo );
            }

            break;
        }

        /*
         *  The current macro becomes the selected selection macro
         */
        pCurMacro = pMac;
        mRes = (*(match_procs[ pMac->funcCode & 0x0F ])
               )( pzSampleText, pT->pzTemplText + pMac->ozText );

        /*
         *  IF match, THEN generate and stop looking for a match.
         */
        if (SUCCEEDED( mRes )) {
            if (OPT_VALUE_TRACE >= TRACE_BLOCK_MACROS) {
                fprintf( pfTrace, "CASE string `%s' %s matched `%s'\n",
                         pzSampleText,
                         apzMatchName[ pMac->funcCode & 0x0F ],
                         pT->pzTemplText + pMac->ozText );

                if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
                    fprintf( pfTrace, zFileLine, pCurTemplate->pzFileName,
                             pMac->lineNo );
            }

            generateBlock( pT, pMac + 1, pT->aMacros + pMac->sibIndex );
            break;
        }
        else if (OPT_VALUE_TRACE == TRACE_EVERYTHING) {
            fprintf( pfTrace, "CASE no match: `%s' %s vs. `%s'\n",
                     pzSampleText,
                     apzMatchName[ pMac->funcCode & 0x0F ],
                     pT->pzTemplText + pMac->ozText );
        }
    }

    if (needFree)
        AGFREE( pzSampleText );

    return pEnd;
}

#endif /* DEFINE_LOAD_FUNCTIONS */

#include REGEX_HEADER

/*
 *  This is global data used to keep track of the current CASE
 *  statement being processed.  When CASE statements nest,
 *  these data are copied onto the stack and restored when
 *  the nested CASE statement's ESAC function is found.
 */
typedef struct case_stack tCaseStack;
struct case_stack {
    tMacro*  pCase;
    tMacro*  pSelect;
};

STATIC tCaseStack  current_case;
STATIC tLoadProc mLoad_Select;

static tpLoadProc apCaseLoad[ FUNC_CT ]   = { NULL };
static tpLoadProc apSelectOnly[ FUNC_CT ] = { NULL };

/*
 *  mLoad_CASE
 *
 *  This function is called to set up (load) the macro
 *  when the template is first read in (before processing).
 */
    tMacro*
mLoad_Case( tTemplate* pT, tMacro* pMac, tCC** ppzScan )
{
    size_t         srcLen     = (size_t)pMac->res;   /* macro len  */
    tCaseStack     save_stack = current_case;
    tMacro*        pEsacMac;

    /*
     *  Save the global macro loading mode
     */
    tpLoadProc*    papLP = papLoadProc;

    /*
     *  IF there is no associated text expression
     *  THEN woops!  what are we to case on?
     */
    if (srcLen == 0)
        AG_ABEND_IN( pT, pMac, "expressionless CASE" );

    /*
     *  Load the expression
     */
    (void)mLoad_Expr( pT, pMac, ppzScan );

    /*
     *  IF this is the first time here,
     *  THEN set up the "CASE" mode callout table.
     *  It is the standard table, except entries are inserted
     *  for SELECT and ESAC.
     */
    if (apCaseLoad[0] == NULL) {
        int i;

        /*
         *  Until there is a selection clause, _any_ other
         *  macro will be illegal (except comments :)!
         */
        for (i=0; i < FUNC_CT; i++)
            apSelectOnly[i] = &mLoad_Bogus;

        memcpy( (void*)apCaseLoad, apLoadProc, sizeof( apLoadProc ));
        apSelectOnly[ FTYP_COMMENT] = mLoad_Comment;
        apSelectOnly[ FTYP_SELECT ] = \
        apCaseLoad[   FTYP_SELECT ] = &mLoad_Select;
        apCaseLoad[   FTYP_ESAC   ] = &mLoad_Ending;
    }

    /*
     *  Set the "select macro only" loading mode
     */
    papLoadProc = apSelectOnly;

    /*
     *  Save global pointers to the current macro entry.
     *  We will need this to link the CASE, SELECT and ESAC
     *  functions together.
     */
    current_case.pCase = current_case.pSelect = pMac;

    /*
     *  Continue parsing the template from this nested level
     */
    pEsacMac = parseTemplate( pMac+1, ppzScan );
    if (*ppzScan == NULL)
        AG_ABEND_IN( pT, pMac, "ESAC not found" );

    /*
     *  Tell the last select macro where its end is.
     *  (It ends with the "next" sibling.  Since there
     *  is no "next" at the end, it is a tiny lie.)
     *
     *  Also, make sure the CASE macro knows where the end is.
     */
    pMac->endIndex = \
    current_case.pSelect->sibIndex = (pEsacMac - pT->aMacros);

    /*
     *  Restore any enclosing CASE function's context.
     */
    current_case = save_stack;

    /*
     *  Restore the global macro loading mode
     */
    papLoadProc  = papLP;

    /*
     *  Return the next available macro descriptor
     */
    return pEsacMac;
}


/*=macfunc SELECT
 *
 *  what:    Selection block for CASE function
 *  desc:
 *    This macro selects a block of text by matching an expression
 *    against the sample text expression evaluated in the @code{CASE}
 *    macro.  @xref{CASE}.
 *
 *    You do not specify a @code{SELECT} macro with the word ``select''.
 *    Instead, you must use one of the 17 match operators described in
 *    the @code{CASE} macro description.
=*/
STATIC tMacro*
mLoad_Select( tTemplate* pT, tMacro* pMac, tCC** ppzScan )
{
    const char*    pzScan = *ppzScan;  /* text after macro */
    char*          pzCopy = pT->pNext; /* next text dest   */
    const char*    pzSrc  = (char*)pMac->ozText; /* macro text */
    long           srcLen = pMac->res;           /* macro len  */

    tSCC zInvSel[] = "Invalid selection clause";
    teFuncType typ = FTYP_SELECT_COMPARE_FULL;

    /*
     *  Set the global macro loading mode
     */
    papLoadProc = apCaseLoad;
    pMac->res   = 0;
    if (srcLen == 0)
        AG_ABEND_IN( pT, pMac, "Empty macro text" );

    /*
     *  IF the first character is an asterisk,
     *  THEN the match can start anywhere in the string
     */
    if (*pzSrc == '*') {
        pzSrc++;
        if (isspace( *pzSrc ) || (*pzSrc == NUL)) {
            typ    = FTYP_SELECT_MATCH_ANYTHING;
            srcLen = 0;
            pMac->ozText = 0;
            goto selection_done;
        }

        typ |= FTYP_SELECT_COMPARE_SKP_START;
    }

    /*
     *  The next character must indicate whether we are
     *  pattern matching ('~') or doing string compares ('=')
     */
    switch (*pzSrc++) {
    case '~':
        /*
         *  Or in the pattern matching bit
         */
        typ |= FTYP_SELECT_MATCH_FULL;
        pMac->res = REG_EXTENDED;
        /* FALLTHROUGH */

    case '=':
        /*
         *  IF the '~' or '=' is doubled,
         *  THEN it is a case sensitive match.  Skip over the char.
         *  ELSE or in the case insensitive bit
         */
        if (pzSrc[0] == pzSrc[-1]) {
            pzSrc++;
        } else {
            typ |= FTYP_SELECT_EQUIVALENT_FULL;
        }
        break;

    default:
        AG_ABEND_IN( pT, pMac, zInvSel );
    }

    /*
     *  IF the last character is an asterisk,
     *  THEN the match may end before the test string ends.
     *       OR in the "may end early" bit.
     */
    if (*pzSrc == '*') {
        pzSrc++;
        typ |= FTYP_SELECT_COMPARE_SKP_END;
    }

    if (! isspace( *pzSrc ))
        AG_ABEND_IN( pT, pMac, zInvSel );

    while (isspace(*pzSrc)) pzSrc++;
    srcLen -= pzSrc - (const char*)pMac->ozText;
    if (srcLen <= 0)
        AG_ABEND_IN( pT, pMac, zInvSel );

    /*
     *  See if we are doing case insensitive regular expressions
     */
    if ((typ & FTYP_SELECT_EQV_MATCH_FULL) == FTYP_SELECT_EQV_MATCH_FULL) {
        int  bitSet;
        pMac->res = REG_EXTENDED | REG_ICASE;

        /*
         *  Turn off the case comparison mode for regular expressions.
         *  We don't have to worry about it.  It is done for us.
         */
        bitSet  = ~FTYP_SELECT_EQUIVALENT_FULL;
        bitSet |= FTYP_SELECT_COMPARE_FULL; /* dont turn this bit off! */
        typ &= bitSet;
    }

    /*
     *  Copy the expression
     */
    pzScan = pzCopy;
    pMac->ozText = (pzCopy - pT->pzTemplText);
    if (typ == FTYP_SELECT_EQUIVALENT) {
        do  {
            *(pzCopy++) = toupper( *(pzSrc++) );
        } while (--srcLen > 0);
    } else {
        do  {
            *(pzCopy++) = *(pzSrc++);
        } while (--srcLen > 0);
    }
    *(pzCopy++) = NUL;
    *(pzCopy++) = NUL;
    pT->pNext = pzCopy;

    if ((*pzScan == '"') || (*pzScan == '\''))
        spanQuote( (char*)pzScan );

 selection_done:
    pMac->funcCode = typ;

    current_case.pSelect->sibIndex = (pMac - pT->aMacros);
    current_case.pSelect = (tMacro*)pMac;

    return pMac + 1;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of funcCase.c */
