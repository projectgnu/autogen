
/*
 *  expState.c
 *  $Id: expState.c,v 3.15 2004/02/01 21:09:52 bkorb Exp $
 *  This module implements expression functions that
 *  query and get state information from AutoGen data.
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

STATIC int     entry_length(   char* pzName );
STATIC int     count_entries(  char* pzName );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  EXPRESSION EVALUATION SUPPORT ROUTINES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

STATIC int
entry_length( char* pzName )
{
    tDefEntry**  papDefs = findEntryList( pzName );
    int          res     = 0;

    if (papDefs == NULL)
        return 0;

    for (;;) {
        tDefEntry*   pDE = *(papDefs++);
        if (pDE == NULL)
            break;
        if (pDE->valType == VALTYP_TEXT)
            res += strlen( pDE->val.pzText );
        else
            res++;
    }
    return res;
}


STATIC int
count_entries( char* pzName )
{
    tDefEntry**  papDefs = findEntryList( pzName );
    int          res     = 0;

    if (papDefs == NULL)
        return 0;

    for (;;) {
        tDefEntry*   pDE = *(papDefs++);
        if (pDE == NULL)
            break;
        res++;
    }
    return res;
}


STATIC SCM
find_entry_value( SCM op, SCM obj, SCM test )
{
    ag_bool     isIndexed;
    tDefEntry*  pE;
    tSCC        zFailed[] = "failed\n";
    tSCC        zSucc[]   = "SUCCESS\n";
    char*       pzField;

    char* pzName = ag_scm2zchars( obj, "find name" );

    if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
        fprintf( pfTrace, " in \"%s\" -- ", pzName );

    pzField = strchr( pzName, '.' );
    if (pzField != NULL)
        *(pzField++) = NUL;

    pE = findDefEntry( pzName, &isIndexed );

    /*
     *  No such entry?  return FALSE
     */
    if (pE == NULL) {
        if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
            fputs( zFailed, pfTrace );
        return SCM_BOOL_F;
    }

    /*
     *  No subfield?  Check the values
     */
    if (pzField == NULL) {
        SCM result;
        SCM field;
        if (pE->valType != VALTYP_TEXT) {
            if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
                fputs( zFailed, pfTrace );
            return SCM_BOOL_F; /* Cannot match string -- not a text value */
        }

        field  = gh_str02scm( pE->val.pzText );
        result = gh_call2( op, field, test );
        if (! isIndexed)
            while (result == SCM_BOOL_F) {

                pE = pE->pTwin;
                if (pE == NULL)
                    break;

                field = gh_str02scm( pE->val.pzText );
                result = gh_call2( op, field, test );
            }

        if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
            fputs( (result == SCM_BOOL_T) ? zSucc : zFailed, pfTrace );
        return result;
    }

    /*
     *  a subfield for a text macro?  return FALSE
     */
    if (pE->valType == VALTYP_TEXT) {
        if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
            fputs( zFailed, pfTrace );
        return SCM_BOOL_F;
    }

    /*
     *  Search the members for what we want.
     */
    pzField[-1] = '.';
    {
        SCM field   = gh_str02scm( pzField );
        SCM result;
        tDefCtx ctx = currDefCtx;

        currDefCtx.pPrev = &ctx;
        currDefCtx.pDefs = pE->val.pDefEntry;

        result = find_entry_value( op, field, test );

        if (! isIndexed)
            while (result == SCM_BOOL_F) {

                pE = pE->pTwin;
                if (pE == NULL)
                    break;

                currDefCtx.pDefs = pE->val.pDefEntry;
                result = find_entry_value( op, field, test );
            }

        currDefCtx = ctx;
        return result;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  EXPRESSION ROUTINES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=gfunc base_name
 *
 * what:   base output name
 *
 * doc:  Returns a string containing the base name of the output file(s).
 *       Generally, this is also the base name of the definitions file.
=*/
SCM
ag_scm_base_name( void )
{
    return gh_str02scm( (char*)(void*)OPT_ARG( BASE_NAME ));
}


/*=gfunc count
 *
 * what:   definition count
 *
 * exparg: ag-name, name of AutoGen value
 *
 * doc:  Count the number of entries for a definition.
 *      The input argument must be a string containing the name
 *      of the AutoGen values to be counted.  If there is no
 *      value associated with the name, the result is an SCM
 *      immediate integer value of zero.
=*/
SCM
ag_scm_count( SCM obj )
{
    int ent_len = count_entries( ag_scm2zchars( obj, "ag object" ));

    return gh_int2scm( ent_len );
}


/*=gfunc def_file
 *
 * what:   definitions file name
 *
 * doc:  Get the name of the definitions file.
 *       Returns the name of the source file containing the AutoGen
 *       definitions.
=*/
SCM
ag_scm_def_file( void )
{
    return gh_str02scm( (char*)(void*)pBaseCtx->pzFileName );
}


/*=gfunc exist_p
 *
 * what:   test for value name
 *
 * exparg: ag-name, name of AutoGen value
 *
 * doc:  return SCM_BOOL_T iff a specified name has an AutoGen value.
 *       The name may include indexes and/or member names.
 *       All but the last member name must be an aggregate definition.
 *       For example:
 *       @example
 *       (exist? "foo[3].bar.baz")
 *       @end example
 *       will yield true if all of the following is true:
 *       @*
 *       There is a member value of either group or string type
 *       named @code{baz} for some group value @code{bar} that
 *       is a member of the @code{foo} group with index @code{3}.
 *       There may be multiple entries of @code{bar} within
 *       @code{foo}, only one needs to contain a value for @code{baz}.
=*/
SCM
ag_scm_exist_p( SCM obj )
{
    ag_bool x;
    SCM     res;

    if (findDefEntry( ag_scm2zchars( obj, "ag object" ), &x ) == NULL)
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    return res;
}


/*=gfunc ag_function_p
 *
 * what:   test for function
 *
 * exparg: ag-name, name of AutoGen macro
 *
 * doc:  return SCM_BOOL_T if a specified name is a user-defined AutoGen
 *       macro, otherwise return SCM_BOOL_F.
=*/
SCM
ag_scm_ag_function_p( SCM obj )
{
    SCM     res;

    if (findTemplate( ag_scm2zchars( obj, "ag user macro" )) == NULL)
         res = SCM_BOOL_F;
    else res = SCM_BOOL_T;

    return res;
}


/*=gfunc match_value_p
 *
 * what:   test for matching value
 *
 * exparg: op,       boolean result operator
 * exparg: ag-name,  name of AutoGen value
 * exparg: test-str, string to test against
 *
 * doc:  This function answers the question, "Is there an AutoGen value named
 *       @code{ag-name} with a value that matches the pattern @code{test-str}
 *       using the match function @code{op}?"  Return SCM_BOOL_T iff at least
 *       one occurrence of the specified name has such a value.  The operator
 *       can be any function that takes two string arguments and yields a
 *       boolean.  It is expected that you will use one of the string matching
 *       functions provided by AutoGen.
 *       @*
 *       The value name must follow the same rules as the
 *       @code{ag-name} argument for @code{exist?} (@pxref{SCM exist?}).
=*/
SCM
ag_scm_match_value_p( SCM op, SCM obj, SCM test )
{
    if (  (! gh_procedure_p( op   ))
       || (! gh_string_p(    obj  )) )
        return SCM_UNDEFINED;

    if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
        fprintf( pfTrace, "searching for `%s'",
                 ag_scm2zchars( test, "test value" ));

    return find_entry_value( op, obj, test );
}


/*=gfunc get
 *
 * what:   get named value
 *
 * exparg: ag-name, name of AutoGen value
 * exparg: alt-val, value if not present, optional
 *
 * doc:
 *  Get the first string value associated with the name.
 *  It will either return the associated string value (if
 *  the name resolves), the alternate value (if one is provided),
 *  or else the empty string.
=*/
SCM
ag_scm_get( SCM agName, SCM altVal )
{
    tDefEntry*  pE;
    ag_bool     x;

    if (! gh_string_p( agName ))
         pE = NULL;
    else pE = findDefEntry( ag_scm2zchars( agName, "ag value" ), &x );

    if ((pE == NULL) || (pE->valType != VALTYP_TEXT)) {
        if (gh_string_p( altVal ))
            return altVal;
        return gh_str02scm( "" );
    }

    return gh_str02scm( pE->val.pzText );
}


/*=gfunc high_lim
 *
 * what:   get highest value index
 *
 * exparg: ag-name, name of AutoGen value
 *
 * doc:
 *
 *  Returns the highest index associated with an array of definitions.
 *  This is generally, but not necessarily, one less than the
 *  @code{count} value.  (The indexes may be specified, rendering a
 *  non-zero based or sparse array of values.)
 *
 *  This is very useful for specifying the size of a zero-based array
 *  of values where not all values are present.  For example:
 *
 *  @example
 *  tMyStruct myVals[ [+ (+ 1 (high-lim "my-val-list")) +] ];
 *  @end example
=*/
SCM
ag_scm_high_lim( SCM obj )
{
    tDefEntry*  pE;
    ag_bool     isIndexed;

    pE = findDefEntry( ag_scm2zchars( obj, "ag value" ), &isIndexed );

    /*
     *  IF we did not find the entry we are looking for
     *  THEN return zero
     *  ELSE search the twin list for the high entry
     */
    if (pE == NULL)
        return gh_int2scm( 0 );

    if (isIndexed)
        return gh_int2scm( pE->index );

    if (pE->pEndTwin != NULL)
        pE = pE->pEndTwin;

    return gh_int2scm( pE->index );
}


/*=gfunc len
 *
 * what:   get count of values
 *
 * exparg: ag-name, name of AutoGen value
 *
 * doc:  If the named object is a group definition, then "len" is
 *       the same as "count".  Otherwise, if it is one or more text
 *       definitions, then it is the sum of their string lengths.
 *       If it is a single text definition, then it is equivalent to
 *       @code{(string-length (get "ag-name"))}.
=*/
SCM
ag_scm_len( SCM obj )
{
    int         len;

    len = entry_length( ag_scm2zchars( obj, "ag value" ));

    return gh_int2scm( len );
}


/*=gfunc low_lim
 *
 * what:   get lowest value index
 *
 * exparg: ag-name, name of AutoGen value
 *
 * doc:  Returns the lowest index associated with an array of definitions.
=*/
SCM
ag_scm_low_lim( SCM obj )
{
    tDefEntry*  pE;
    ag_bool     x;

    pE = findDefEntry( ag_scm2zchars( obj, "ag value" ), &x );

    /*
     *  IF we did not find the entry we are looking for
     *  THEN return zero
     *  ELSE we have the low index.
     */
    if (pE == NULL)
        return gh_int2scm( 0 );

    return gh_int2scm( pE->index );
}


/*=gfunc set_option
 *
 * what:  Set a command line option
 *
 * exparg: opt, AutoGen option name + its argument
 *
 * doc:   The text argument must be an option name followed by any needed
 *        option argument.  Returns SCM_UNDEFINED.
=*/
SCM
ag_scm_set_option( SCM opt )
{
    optionLoadLine( &autogenOptions, ag_scm2zchars( opt, "opt + arg" ));
    return SCM_UNDEFINED;
}


/*=gfunc suffix
 *
 * what:   get the current suffix
 *
 * doc:
 *  Returns the current active suffix (@pxref{pseudo macro}).
=*/
SCM
ag_scm_suffix( void )
{
    return gh_str02scm( (char*)pzCurSfx );
}


/*=gfunc tpl_file
 *
 * what:   get the template file name
 *
 * doc:  Returns the name of the current template file.
=*/
SCM
ag_scm_tpl_file( void )
{
    return gh_str02scm( (char*)(void*)pzTemplFileName );
}


/*=gfunc tpl_file_line
 *
 * what:   get the template file and line number
 *
 * exparg: msg-fmt, formatting for line message, optional
 *
 * doc: Returns the file and line number of the current template macro
 *      using either the default format, "from %s line %d", or else
 *      the format you supply.  For example, if you want to insert a "C"
 *      language file-line directive, you would supply the format "# %2$d %1$s".
=*/
SCM
ag_scm_tpl_file_line( SCM fmt )
{
    char    zScribble[ 1024 ];
    tSCC    zFmt[] = "from %s line %d";
    tCC*    pzFmt = zFmt;
    char*   pz;

    if (gh_string_p( fmt ))
        pzFmt = ag_scm2zchars( fmt, "file/line format" );

    {
        size_t  maxlen = strlen( pCurTemplate->pzFileName )
                       + strlen( pzFmt ) + 4 * sizeof( int );
        if (maxlen >= sizeof( zScribble ))
            pz = (char*)AGALOC( maxlen, "file-line buffer" );
        else pz = zScribble;
    }

    sprintf( pz, pzFmt, pCurTemplate->pzFileName, pCurMacro->lineNo );

    {
        SCM res = gh_str02scm( pz );
        if (pz != zScribble)
            AGFREE( (void*)pz );

        return res;
    }
}

#include "expr.ini"
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/expState.c */
