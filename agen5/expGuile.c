
/*
 *  $Id: expGuile.c,v 3.9 2003/04/21 03:35:34 bkorb Exp $
 *  This module implements the expression functions that should
 *  be part of Guile.
 */

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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */

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
        unsigned long val;

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
            val = gh_scm2ulong( car );
            break;

        case GH_TYPE_STRING:
            val = strtol( ag_scm2zchars( car, "number-in-string" ),
                          NULL, 0 );
            break;

        default:
            continue;
        }
        max_val = MAX( max_val, val );
    }

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
        unsigned long val;

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
            val = gh_scm2ulong( car );
            break;

        case GH_TYPE_STRING:
            val = strtol( ag_scm2zchars( car, "number-in-string" ),
                          NULL, 0 );
            break;

        default:
            continue;
        }
        min_val = MIN( min_val, val );
    }

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
    unsigned long sum = 0;

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
            sum += gh_scm2ulong( car );
            break;

        case GH_TYPE_STRING:
            sum += strtol( ag_scm2zchars( car, "number-in-string" ),
                           NULL, 0 );
        }
    } while (--len > 0);

    return gh_long2scm( sum );
}


/*=gfunc string_to_c_name_x
 *
 * what:   map non-name chars to underscore
 * general_use:
 *
 * exparg: str , input/output string
 *
 * doc:  Change all the graphic characters that are invalid in a C name token
 *       into underscores.  Whitespace characters are ignored.  Any other
 *       character type (i.e. non-graphic and non-white) will cause a failure.
=*/
    SCM
ag_scm_string_to_c_name_x( SCM str )
{
    tSCC  zFun[] = "ag_scm_string_upcase_x";
    int   len;
    char* pz;

    if (! gh_string_p( str ))
        scm_wrong_type_arg( zFun, 1, str );

    len = SCM_LENGTH( str );
    pz  = SCM_CHARS( str );
    while (--len >= 0) {
        char ch = *pz;
        if (! isalnum( ch )) {

            if (isspace( ch ))
                ;

            else if (isprint( ch ))
                *pz = '_';

            else
                scm_misc_error( zFun,
                                "cannot map unprintable chars to C name chars",
                                str );
        }
        pz++;
    }

    return str;
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
 * what:   upper case a new string
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
    size_t  len;
    char*   pz;
    SCM     res;

    pz  = gh_scm2newstr( str, &len );
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
 * what:   capitalize a new string
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
    size_t  len;
    char*   pz;
    SCM     res;

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
 * what:   lower case a new string
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
    size_t  len;
    char*   pz;
    SCM     res;

    if (! gh_string_p( str ))
        return SCM_UNDEFINED;

    pz = gh_scm2newstr( str, &len );
    res = gh_str2scm( pz, len );
    free( (void*)pz );
    ag_scm_string_downcase_x( res );
    return res;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expGuile.c */
