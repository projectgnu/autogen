
/*
 *  $Id: expGuile.c,v 1.14 2000/09/28 06:09:23 bkorb Exp $
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

#ifndef HAVE_STRFTIME
#  include "compat/strftime.c"
#endif



    teGuileType
gh_type_e( SCM typ )
{
    if (gh_boolean_p( typ ))
        return GH_TYPE_BOOLEAN;

    if (gh_symbol_p( typ ))
        return GH_TYPE_SYMBOL;

    if (gh_char_p( typ ))
        return GH_TYPE_CHAR;

    if (gh_vector_p( typ ))
        return GH_TYPE_VECTOR;

    if (gh_pair_p( typ ))
        return GH_TYPE_PAIR;

    if (gh_number_p( typ ))
        return GH_TYPE_NUMBER;

    if (gh_string_p( typ ))
        return GH_TYPE_STRING;

    if (gh_procedure_p( typ ))
        return GH_TYPE_PROCEDURE;

    if (gh_list_p( typ ))
        return GH_TYPE_LIST;

    if (gh_inexact_p( typ ))
        return GH_TYPE_INEXACT;

    if (gh_exact_p( typ ))
        return GH_TYPE_EXACT;

    return GH_TYPE_UNDEFINED;
}


/*=gfunc max
 *
 * what:   maximum value in list
 * general_use:
 *
 * exparg: list , list of values.  Strings are converted to numbers ,, list
 *
 * doc:  Return the maximum value in the list
=*/
    SCM
ag_scm_max( SCM list )
{
    int   len;
    SCM   car;
    long  max_val = LONG_MIN;
#   ifndef MAX
#     define MAX(m,v) ((v > m) ? v : m)
#   endif
    len = scm_ilength( list );
    if (len <= 0)
        return SCM_UNDEFINED;

    while (--len >= 0) {
        long val;

        car  = SCM_CAR( list );
        list = SCM_CDR( list );

        switch (gh_type_e( car )) {
        case GH_TYPE_BOOLEAN:
            if (car == SCM_BOOL_F) {
                val = 0;
            } else {
                val = 1;
            }
            break;

        case GH_TYPE_CHAR:
            val = (int)gh_scm2char( car );
            break;

        case GH_TYPE_NUMBER:
            val = gh_scm2long( car );
            break;

        case GH_TYPE_STRING:
	    val = strtol( SCM_CHARS( car ), (char**)NULL, 0 );
            break;

        default:
            continue;
        }
        max_val = MAX( max_val, val );
    }

#   undef MAX
    return gh_long2scm( max_val );
}


/*=gfunc min
 *
 * what:   minimum value in list
 * general_use:
 *
 * exparg: list , list of values.  Strings are converted to numbers ,, list
 *
 * doc:  Return the minimum value in the list
=*/
    SCM
ag_scm_min( SCM list )
{
    int   len;
    SCM   car;
    long  min_val = LONG_MAX;
#   ifndef MIN
#     define MIN(m,v) ((v < m) ? v : m)
#   endif
    len = scm_ilength( list );
    if (len <= 0)
        return SCM_UNDEFINED;

    while (--len >= 0) {
        long val;

        car  = SCM_CAR( list );
        list = SCM_CDR( list );

        switch (gh_type_e( car )) {
        case GH_TYPE_BOOLEAN:
            if (car == SCM_BOOL_F) {
                val = 0;
            } else {
                val = 1;
            }
            break;

        case GH_TYPE_CHAR:
            val = (int)gh_scm2char( car );
            break;

        case GH_TYPE_NUMBER:
            val = gh_scm2long( car );
            break;

        case GH_TYPE_STRING:
        {
            char* pz  = SCM_CHARS( car );
	    int   len = SCM_LENGTH( len );
	    while (isspace( *pz ) && (--len > 0))  pz++;
            if ((len > 0) && isdigit( *pz ))
                 val = strtol( pz, (char**)NULL, 0 );
            else val = LONG_MAX;
            break;
        }

        default:
            continue;
        }
        min_val = MIN( min_val, val );
    }

#   undef MIN
    return gh_long2scm( min_val );
}


/*=gfunc sum
 *
 * what:   sum of values in list
 * general_use:
 *
 * exparg: list , list of values.  Strings are converted to numbers ,, list
 *
 * doc:  Compute the sum of the list of expressions.
=*/
    SCM
ag_scm_sum( SCM list )
{
    int  len = scm_ilength( list );
    long sum = 0;

    if (len <= 0)
        return gh_int2scm( 0 );

    do  {
        SCM  car = SCM_CAR( list );
        list = SCM_CDR( list );
        switch (gh_type_e( car )) {
        default:
            return SCM_UNDEFINED;

        case GH_TYPE_CHAR:
            sum += (long)(unsigned char)gh_scm2char( car );
            break;

        case GH_TYPE_NUMBER:
            sum += gh_scm2long( car );
            break;

        case GH_TYPE_STRING:
            sum += strtol( SCM_CHARS( car ), (char**)NULL, 0 );
        }
    } while (--len > 0);

    return gh_long2scm( sum );
}


/*=gfunc string_upcase_x
 *
 * what:   make a string be upper case
 * general_use:
 *
 * exparg: str , input/output string
 *
 * doc:  Change to upper case all the characters in an SCM string.
=*/
    SCM
ag_scm_string_upcase_x( SCM str )
{
    int   len;
    char* pz;

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    len = SCM_LENGTH( str );
    pz  = SCM_CHARS( str );
    while (--len >= 0) {
        char ch = *pz;
        if (islower( ch ))
            *pz = toupper( ch );
        pz++;
    }

    return str;
}


/*=gfunc string_upcase
 *
 * what:   make a new string be upper case
 * general_use:
 *
 * exparg: str , input string
 *
 * doc:  Create a new SCM string containing the same text as the original,
 *       only all the lower case letters are changed to upper case.
=*/
    SCM
ag_scm_string_upcase( SCM str )
{
    int   len;
    char* pz;
    SCM   res;

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    pz = gh_scm2newstr( str, &len );
    res = gh_str2scm( pz, len );
    free( (void*)pz );
    ag_scm_string_upcase_x( res );
    return res;
}


/*=gfunc string_capitalize_x
 *
 * what:   capitalize a string
 * general_use:
 *
 * exparg: str , input/output string
 *
 * doc:  capitalize all the words in an SCM string.
=*/
    SCM
ag_scm_string_capitalize_x( SCM str )
{
    int     len;
    char*   pz;
    ag_bool word_start = AG_TRUE;

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    len = SCM_LENGTH( str );
    pz  = SCM_CHARS( str );

    while (--len >= 0) {
        char ch = *pz;

        if (! isalnum( ch )) {
            word_start = AG_TRUE;

        } else if (word_start) {
            word_start = AG_FALSE;
            if (islower( ch ))
                *pz = toupper( ch );

        } else if (isupper( ch ))
            *pz = tolower( ch );

        pz++;
    }

    return str;
}


/*=gfunc string_capitalize
 *
 * what:   make a new string be capitalized
 * general_use:
 *
 * exparg: str , input string
 *
 * doc:  Create a new SCM string containing the same text as the original,
 *       only all the first letter of each word is upper cased and all
 *       other letters are made lower case.
=*/
    SCM
ag_scm_string_capitalize( SCM str )
{
    int   len;
    char* pz;
    SCM   res;

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    pz = gh_scm2newstr( str, &len );
    res = gh_str2scm( pz, len );
    free( (void*)pz );
    ag_scm_string_capitalize_x( res );
    return res;
}


/*=gfunc string_downcase_x
 *
 * what:   make a string be lower case
 * general_use:
 *
 * exparg: str , input/output string
 *
 * doc:  Change to lower case all the characters in an SCM string.
=*/
    SCM
ag_scm_string_downcase_x( SCM str )
{
    int   len;
    char* pz;

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    len = SCM_LENGTH( str );
    pz  = SCM_CHARS( str );
    while (--len >= 0) {
        char ch = *pz;
        if (isupper( ch ))
            *pz = tolower( ch );
        pz++;
    }

    return str;
}


/*=gfunc string_downcase
 *
 * what:   make a new string be lower case
 * general_use:
 *
 * exparg: str , input string
 *
 * doc:  Create a new SCM string containing the same text as the original,
 *       only all the upper case letters are changed to lower case.
=*/
    SCM
ag_scm_string_downcase( SCM str )
{
    int   len;
    char* pz;
    SCM   res;

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    pz = gh_scm2newstr( str, &len );
    res = gh_str2scm( pz, len );
    free( (void*)pz );
    ag_scm_string_downcase_x( res );
    return res;
}
/* end of expGuile.c */
