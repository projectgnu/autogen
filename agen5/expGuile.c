
/*
 *  $Id: expGuile.c,v 4.17 2009/01/01 16:49:26 bkorb Exp $
 *
 *  Time-stamp:        "2007-07-04 11:19:05 bkorb"
 *  Last Committed:    $Date: 2009/01/01 16:49:26 $
 *
 *  This module implements the expression functions that should
 *  be part of Guile.
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
 *
 * AutoGen is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AutoGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

LOCAL teGuileType
gh_type_e( SCM typ )
{
    if (AG_SCM_BOOL_P(   typ ))   return GH_TYPE_BOOLEAN;
    if (AG_SCM_SYM_P(    typ ))   return GH_TYPE_SYMBOL;
    if (AG_SCM_STRING_P( typ ))   return GH_TYPE_STRING;
    if (AG_SCM_IS_PROC(  typ ))   return GH_TYPE_PROCEDURE;
    if (AG_SCM_CHAR_P(   typ ))   return GH_TYPE_CHAR;
    if (AG_SCM_VEC_P(    typ ))   return GH_TYPE_VECTOR;
    if (AG_SCM_PAIR_P(   typ ))   return GH_TYPE_PAIR;
    if (AG_SCM_NUM_P(    typ ))   return GH_TYPE_NUMBER;
    if (AG_SCM_LIST_P(   typ ))   return GH_TYPE_LIST;

    return GH_TYPE_UNDEFINED;
}


LOCAL SCM
ag_scm_c_eval_string_from_file_line( tCC* pzExpr, tCC* pzFile, int line )
{
    SCM port;

    if (OPT_VALUE_TRACE >= TRACE_EVERYTHING) {
        fprintf( pfTrace, "eval from file %s line %d:\n%s\n", pzFile, line,
                 pzExpr );
    }

#if GUILE_VERSION < 106000
    {
        tSCC zEx[] = "eval-string-from-file-line";
        SCM  expr  = scm_makfrom0str( pzExpr );
        port = scm_mkstrport( SCM_INUM0, expr, SCM_OPN | SCM_RDNG, zEx );
    }
#else
    port = scm_open_input_string( AG_SCM_STR02SCM( pzExpr ));
#endif

#if GUILE_VERSION < 107000
    {
        static SCM   file = SCM_UNDEFINED;
        static char* pzFl = NULL;

        scm_t_port* pt;

        if (  (pzFl == NULL)
           || (strcmp( AG_SCM_CHARS( file ), pzFile ) != 0) )  {
            if (pzFl != NULL)
                AGFREE(pzFl);
            AGDUPSTR(pzFl, pzFile, "eval file name");
            file = AG_SCM_STR02SCM( pzFile );
        }

        pt = SCM_PTAB_ENTRY(port);
        pt->line_number = line - 1;
        pt->file_name   = file;
    }

#else
    {
        static SCM file = SCM_UNDEFINED;
        static char* pzOldFile = NULL;

        if ((pzOldFile == NULL) || (strcmp( pzOldFile, pzFile ) != 0)) {
            if (pzOldFile != NULL)
                AGFREE( pzOldFile );

            AGDUPSTR( pzOldFile, pzFile, "scheme file source" );
            file = scm_from_locale_string( pzFile );
        }

        scm_set_port_filename_x( port, file );
    }

    {
        SCM ln = scm_from_int( line );
        scm_set_port_line_x( port, ln );
    }
#endif

    {
        SCM ans = SCM_UNSPECIFIED;

        /* Read expressions from that port; ignore the values.  */
        for (;;) {
            SCM form = scm_read( port );
            if (SCM_EOF_OBJECT_P( form ))
                break;
            ans = scm_primitive_eval_x( form );
        }

        return ans;
    }
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
    long sum = 0;

    if (len <= 0)
        return AG_SCM_INT2SCM( 0 );

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
    tSCC  zFun[] = "ag_scm_string_to_c_name_x";
    tSCC  zMap[] = "cannot map unprintable chars to C name chars";
    int   len;
    char* pz;

    if (! AG_SCM_STRING_P( str ))
        scm_wrong_type_arg( zFun, 1, str );

    len = AG_SCM_STRLEN( str );
    pz  = (char*)(void*)AG_SCM_CHARS( str );
    while (--len >= 0) {
        char ch = *pz;
        if (! isalnum( ch )) {

            if (isspace( ch ))
                ;

            else if (isprint( ch ))
                *pz = '_';

            else
                scm_misc_error( zFun, zMap, str );
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

    if (! AG_SCM_STRING_P( str ))
        return SCM_UNDEFINED;

    len = AG_SCM_STRLEN( str );
    pz  = (char*)(void*)AG_SCM_CHARS( str );
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
    SCM res;
    if (! AG_SCM_STRING_P( str ))
        return SCM_UNDEFINED;

    res = AG_SCM_STR2SCM( AG_SCM_CHARS( str ), AG_SCM_STRLEN( str ));
    scm_string_upcase_x( res );
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

    if (! AG_SCM_STRING_P( str ))
        return SCM_UNDEFINED;

    len = AG_SCM_STRLEN( str );
    pz  = (char*)(void*)AG_SCM_CHARS( str );

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
    SCM res;
    if (! AG_SCM_STRING_P( str ))
        return SCM_UNDEFINED;

    res = AG_SCM_STR2SCM( AG_SCM_CHARS( str ), AG_SCM_STRLEN( str ));
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

    if (! AG_SCM_STRING_P( str ))
        return SCM_UNDEFINED;

    len = AG_SCM_STRLEN( str );
    pz  = (char*)(void*)AG_SCM_CHARS( str );
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
    SCM res;
    if (! AG_SCM_STRING_P( str ))
        return SCM_UNDEFINED;

    res = AG_SCM_STR2SCM( AG_SCM_CHARS( str ), AG_SCM_STRLEN( str ));
    ag_scm_string_downcase_x( res );
    return res;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expGuile.c */
