
/*
 *  $Id: funcFor.c,v 1.20 2001/06/06 04:19:57 uid24370 Exp $
 *
 *  This module implements the FOR text function.
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
#include "expr.h"

#define ENTRY_END  INT_MAX

STATIC tForState*  pFS;  /* Current "FOR" information (state) */


STATIC ag_bool nextDefinition( ag_bool invert, tDefEntry** ppList );
STATIC int
doForByStep( tTemplate* pT,
             tMacro*    pMac,
             tDefEntry* pFoundDef );
STATIC int
doForEach( tTemplate*   pT,
           tMacro*      pMac,
           tDefEntry*   pFoundDef );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Operational Functions */

/*=gfunc first_for_p
 *
 * what:    detect first iteration
 * exparg:  for_var, which for loop, opt
 * doc:     Returns SCM_BOOL_T if the named FOR loop (or, if not named, the
 *          current innermost loop) is on the first pass through the data.
 *          Outside of any FOR loop, it returns SCM_UNDEFINED.
 *          @xref{FOR}.
=*/
    SCM
ag_scm_first_for_p( SCM which )
{
    if (forInfo.fi_depth <= 0)
        return SCM_UNDEFINED;

    if (! gh_string_p( which ))
        return (pFS->for_firstFor) ? SCM_BOOL_T : SCM_BOOL_F;

    {
        int        ct;
        tForState* p   = forInfo.fi_data + (forInfo.fi_depth - 1);
        char*      pz  = gh_scm2newstr( which, &ct );
        SCM        res = SCM_UNDEFINED;
        ct = forInfo.fi_depth;

        do  {
            if (strcmp( p->for_pzName, pz ) == 0) {
                res = (p->for_firstFor ? SCM_BOOL_T : SCM_BOOL_F);
                break;
            }
            p--;
        } while (--ct > 0);

        free( (void*)pz );
        return res;
    }
}


/*=gfunc last_for_p
 *
 * what:    detect last iteration
 * exparg:  for_var, which for loop, opt
 * doc:     Returns SCM_BOOL_T if the named FOR loop (or, if not named, the
 *          current innermost loop) is on the last pass through the data.
 *          Outside of any FOR loop, it returns SCM_UNDEFINED.
 *          @xref{FOR}.
=*/
    SCM
ag_scm_last_for_p( SCM which )
{
    if (forInfo.fi_depth <= 0)
        return SCM_UNDEFINED;

    if (! gh_string_p( which ))
        return (pFS->for_lastFor ? SCM_BOOL_T : SCM_BOOL_F);

    {
        int        ct;
        tForState* p   = forInfo.fi_data + (forInfo.fi_depth - 1);
        char*      pz  = gh_scm2newstr( which, &ct );
        SCM        res = SCM_UNDEFINED;
        ct = forInfo.fi_depth;

        do  {
            if (strcmp( p->for_pzName, pz ) == 0) {
                res = (p->for_lastFor ? SCM_BOOL_T : SCM_BOOL_F);
                break;
            }
            p--;
        } while (--ct > 0);

        free( (void*)pz );
        return res;
    }
}


/*=gfunc for_index
 *
 * what:    get current loop index
 * exparg:  for_var, which for loop, opt
 * doc:     Returns the current index for the named FOR loop.
 *          If not named, then the index for the innermost loop.
 *          Outside of any FOR loop, it returns SCM_UNDEFINED.
 *          @xref{FOR}.
=*/
    SCM
ag_scm_for_index( SCM which )
{
    if (forInfo.fi_depth <= 0)
        return SCM_UNDEFINED;
    if (! gh_string_p( which ))
        return gh_int2scm( pFS->for_index );

    {
        int        ct;
        tForState* p   = forInfo.fi_data + (forInfo.fi_depth - 1);
        char*      pz  = gh_scm2newstr( which, &ct );
        SCM        res = SCM_UNDEFINED;
        ct = forInfo.fi_depth;

        do  {
            if (strcmp( p->for_pzName, pz ) == 0) {
                res = gh_int2scm( p->for_index );
                break;
            }
            p--;
        } while (--ct > 0);

        free( (void*)pz );
        return res;
    }
}


/*=gfunc for_from
 *
 * what:   set initial index
 * exparg: from, the initial index for the AutoGen FOR macro
 *
 * doc:  This function records the initial index information
 *       for an AutoGen FOR function.  @xref{FOR}.
=*/
    SCM
ag_scm_for_from( SCM from )
{
    if ((! pFS->for_loading) || (! gh_number_p( from )))
        return SCM_UNDEFINED;
    pFS->for_from = gh_scm2int( from );
    return SCM_BOOL_T;
}


/*=gfunc for_to
 *
 * what:   set ending index
 * exparg: to, the final index for the AutoGen FOR macro
 *
 * doc:  This function records the terminating value information
 *       for an AutoGen FOR function.  @xref{FOR}.
=*/
    SCM
ag_scm_for_to( SCM to )
{
    if ((! pFS->for_loading) || (! gh_number_p( to )))
        return SCM_UNDEFINED;
    pFS->for_to = gh_scm2int( to );
    return SCM_BOOL_T;
}


/*=gfunc for_by
 *
 * what:   set iteration step
 * exparg: by, the iteration increment for the AutoGen FOR macro
 *
 * doc:
 *    This function records the "step by" information
 *    for an AutoGen FOR function.  @xref{FOR}.
=*/
    SCM
ag_scm_for_by( SCM by )
{
    if ((! pFS->for_loading) || (! gh_number_p( by )))
        return SCM_UNDEFINED;
    pFS->for_by = gh_scm2int( by );
    return SCM_BOOL_T;
}


/*=gfunc for_sep
 *
 * what:   set loop separation string
 * exparg: separator, the text to insert between the output of
 *         each FOR iteration
 *
 * doc:
 *  This function records the separation string that is to be inserted
 *  between each iteration of an AutoGen FOR function.  This is often
 *  nothing more than a comma.  @xref{FOR}.
=*/
    SCM
ag_scm_for_sep( SCM obj )
{
    if ((! pFS->for_loading) || (! gh_string_p( obj )))
        return SCM_UNDEFINED;
    pFS->for_pzSep = strdup( SCM_CHARS( obj ));
    return SCM_BOOL_T;
}


    STATIC ag_bool
nextDefinition( ag_bool invert, tDefEntry** ppList )
{
    ag_bool     haveMatch = AG_FALSE;
    tDefEntry*  pList     = *ppList;

    while (pList != (tDefEntry*)NULL) {
        /*
         *  Loop until we find or pass the current index value
         *
         *  IF we found an entry for the current index,
         *  THEN break out and use it
         */
        if (pList->index == pFS->for_index) {
            haveMatch = AG_TRUE;
            break;
        }

        /*
         *  IF the next definition is beyond our current index,
         *       (that is, the current index is inside of a gap),
         *  THEN we have no current definition and will use
         *       only the set passed in.
         */
        if ((invert)
            ? (pList->index < pFS->for_index)
            : (pList->index > pFS->for_index)) {

            /*
             *  When the "by" step is zero, force syncronization.
             */
            if (pFS->for_by == 0) {
                pFS->for_index = pList->index;
                haveMatch = AG_TRUE;
            }
            break;
        }

        /*
         *  The current index (pFS->for_index) is past the current value
         *  (pB->index), so advance to the next entry and test again.
         */
        pList = (invert) ? pList->pPrevTwin : pList->pTwin;
    }

    /*
     *  Save our restart point and return the find indication
     */
    *ppList = pList;
    return haveMatch;
}


    STATIC int
doForByStep( tTemplate* pT,
             tMacro*    pMac,
             tDefEntry* pFoundDef )
{
    int         loopCt   = 0;
    tDefEntry   textDef;
    ag_bool     invert    = (pFS->for_by < 0);
    t_word      loopLimit = OPT_VALUE_LOOP_LIMIT;
    tDefStack   stack     = currDefCtx;

    if (pFS->for_pzSep == (char*)NULL)
        pFS->for_pzSep = "";

    /*
     *  IF the for-from and for-to values have not been set,
     *  THEN we set them from the indices of the first and last
     *       entries of the twin set.
     */
    {
        tDefEntry* pLast = (pFoundDef->pEndTwin != (tDefEntry*)NULL)
                           ? pFoundDef->pEndTwin : pFoundDef;

        if (pFS->for_from == 0x7BAD0BAD)
            pFS->for_from = (invert) ? pLast->index : pFoundDef->index;

        if (pFS->for_to == 0x7BAD0BAD)
            pFS->for_to = (invert) ? pFoundDef->index : pLast->index;

        /*
         *  "loopLimit" is intended to catch runaway ending conditions.
         *  However, if you really have a gazillion entries, who am I
         *  to stop you?
         */
        if (loopLimit < pLast->index - pFoundDef->index)
            loopLimit = (pLast->index - pFoundDef->index) + 1;
    }

    /*
     *  Make sure we have some work to do before we start.
     */
    if (invert) {
        if (pFS->for_from < pFS->for_to)
            return 0;
    } else {
        if (pFS->for_from > pFS->for_to)
            return 0;
    }

    pFS->for_index = pFS->for_from;

    /*
     *  FROM `from' THROUGH `to' BY `by',
     *  DO...
     */
    for (;;) {
        int     nextIdx;
        ag_bool gotNewDef = nextDefinition( invert, &pFoundDef );

        if (loopLimit-- < 0) {
            fprintf( stderr, zTplErr, pT->pzTplName, pMac->lineNo,
                     "Too many FOR iterations" );
            fprintf( stderr, "\texiting FOR %s from %d to %d "
                     "by %d:\n\tmore than %ld iterations\n",
                     pT->pzTemplText + pMac->ozText,
                     pFS->for_from, pFS->for_to, pFS->for_by,
                     OPT_VALUE_LOOP_LIMIT );
            break;
        }

        if (pFS->for_by != 0) {
            nextIdx = pFS->for_index + pFS->for_by;

        } else if (invert) {
            nextIdx = (pFoundDef->pPrevTwin == (tDefEntry*)NULL)
                ? pFS->for_to - 1  /* last iteration !! */
                : pFoundDef->pPrevTwin->index;

        } else {
            nextIdx = (pFoundDef->pTwin == (tDefEntry*)NULL)
                ? pFS->for_to + 1  /* last iteration !! */
                : pFoundDef->pTwin->index;
        }

        /*
         *  IF we have a non-base definition, use the old def context
         */
        if (! gotNewDef)
            currDefCtx = stack;

        /*
         *  ELSE IF this macro is a text type
         *  THEN create an un-twinned version of it to be found first
         */
        else if (pFoundDef->valType == VALTYP_TEXT) {
            textDef = *pFoundDef;
            textDef.pNext = textDef.pTwin = (tDefEntry*)NULL;

            currDefCtx.pDefs = &textDef;
            currDefCtx.pPrev = &stack;
        }

        /*
         *  ELSE the current definitions are based on the block
         *       macro's values
         */
        else {
            currDefCtx.pDefs = (tDefEntry*)(void*)pFoundDef->pzValue;
            currDefCtx.pPrev = &stack;
        }

        pFS->for_lastFor = (invert)
            ? (nextIdx < pFS->for_to)
            : (nextIdx > pFS->for_to);

        generateBlock( pT, pMac+1, pT->aMacros + pMac->endIndex );
        loopCt++;
        pFS = forInfo.fi_data + (forInfo.fi_depth - 1);

        if (pFS->for_lastFor)
            break;

        fputs( pFS->for_pzSep, pCurFp->pFile );
        pFS->for_firstFor = AG_FALSE;
        pFS->for_index = nextIdx;
    }

    currDefCtx = stack;  /* Restore the def context */
    return loopCt;
}

    STATIC int
doForEach( tTemplate*   pT,
           tMacro*      pMac,
           tDefEntry*   pFoundDef )
{
    int loopCt = 0;
    tDefStack   stack = currDefCtx;

    currDefCtx.pPrev = &stack;

    for (;;) {
        tDefEntry  textDef;

        /*
         *  IF this loops over a text macro,
         *  THEN create a definition that will be found *before*
         *       the repeated (twinned) copy.  That way, when it
         *       is found as a macro invocation, the current value
         *       will be extracted, instead of the value list.
         */
        if (pFoundDef->valType == VALTYP_TEXT) {
            textDef = *pFoundDef;
            textDef.pNext = textDef.pTwin = (tDefEntry*)NULL;

            currDefCtx.pDefs = &textDef;
        } else {
            currDefCtx.pDefs = (tDefEntry*)(void*)pFoundDef->pzValue;
        }

        /*
         *  Set the global current index
         */
        pFS->for_index = pFoundDef->index;

        /*
         *  Advance to the next twin
         */
        pFoundDef = pFoundDef->pTwin;
        if (pFoundDef == (tDefEntry*)NULL)
            pFS->for_lastFor = AG_TRUE;

        generateBlock( pT, pMac+1, pT->aMacros + pMac->endIndex );

        loopCt++;
        pFS = forInfo.fi_data + (forInfo.fi_depth - 1);

        if (pFoundDef == (tDefEntry*)NULL)
            break;
        pFS->for_firstFor = AG_FALSE;

        /*
         *  Emit the iteration separation
         */
        fputs( pFS->for_pzSep, pCurFp->pFile );
    }

    currDefCtx = stack;  /* Restore the def context */
    return loopCt;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*=macfunc FOR
 *
 *  what:    Emit a template block multiple times
 *  cindex:  looping, for
 *  cindex:  for loop
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *  This macro has a slight variation on the standard syntax:
 *  @example
 *  FOR <value-name> [ <separator-string> ]
 *  @end example
 *  or
 *  @example
 *  FOR <value-name> (...Scheme expression list
 *  @end example
 *
 *  The first argument must be the name of an AutoGen value.  If there is
 *  no value associated with the name, the FOR loop block is skipped
 *  entirely.  The scope of the @code{FOR} function extends to the ENDFOR
 *  macro that contains this name.
 *
 *  If there are any further arguments, if the first character is either
 *  a semi-colon (@code{;}) or an opening parenthesis (@code{(}), then
 *  it is presumed to be a Scheme expression containing the FOR macro
 *  specific functions @code{for-from}, @code{for-by}, @code{for-to},
 *  and/or @code{for-sep}.  @xref{AutoGen Functions}.  Otherwise, the
 *  remaining text is presumed to be a string for inserting between
 *  each iteration of the loop.  This string will be emitted one time
 *  less than the number of iterations of the loop.  That is, it is
 *  emitted after each loop, excepting for the last iteration.
 *
 *  If the from/by/to functions are invoked, they will specify which
 *  copies of the named value are to be processed.  If there is no
 *  copy of the named value associated with a particular index,
 *  the @code{FOR} template block will be instantiated anyway.
 *  The template must use methods for detecting missing definitions and
 *  emitting default text.  In this fashion, you can insert entries
 *  from a sparse or non-zero based array into a dense, zero based array.
 *
 *  @strong{NB:} the @code{for-from}, @code{for-to}, @code{for-by} and
 *  @code{for-sep} functions are disabled outside of the context of the
 *  @code{FOR} macro.  Likewise, the @code{first-for}, @code{last-for}
 *  and @code{for-index} functions are disabled outside of the range
 *  of a @code{FOR} block.
 *
 *  @example
 *  [+FOR var (for-from 0) (for-to <number>) (for-sep ",") +]
 *  ... text with @code{var}ious substitutions ...[+
 *  ENDFOR var+]
 *  @end example
 *
 *  @noindent
 *  this will repeat the @code{... text with @code{var}ious
 *  substitutions ...} <number>+1 times.  Each repetition,
 *  except for the last, will have a comma @code{,} after it.
 *
 *  @example
 *  [+FOR var ",\n" +]
 *  ... text with @code{var}ious substitutions ...[+
 *  ENDFOR var +]
 *  @end example
 *
 *  @noindent
 *  This will do the same thing, but only for the index
 *  values of @code{var} that have actually been defined.
=*/
/*=macfunc ENDFOR
 *
 *  what:   Terminates the @code{FOR} function template block
 *  situational:
 *
 *  desc:
 *    This macro ends the @code{FOR} function template block.
 *    For a complete description @xref{FOR}.
=*/
MAKE_HANDLER_PROC( For )
{
    tMacro*     pMRet = pT->aMacros + pMac->endIndex;
    ag_bool     isIndexed;
    tDefEntry*  pDef;
    int         loopCt;

    pDef = findDefEntry( pT->pzTemplText + pMac->ozName, &isIndexed );
    if (pDef == (tDefEntry*)NULL) {
        if (OPT_VALUE_TRACE >= TRACE_BLOCK_MACROS) {
            fprintf( pfTrace, "FOR loop skipped - no definition for `%s'\n",
                     pT->pzTemplText + pMac->ozName );

            if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
                fprintf( pfTrace, zFileLine, pT->pzFileName, pMac->lineNo );
        }

        return pMRet;
    }

    if (++(forInfo.fi_depth) > forInfo.fi_alloc) {
        forInfo.fi_alloc += 5;
        if (forInfo.fi_data == NULL)
             forInfo.fi_data = (tForState*)
                 AGALOC( 5 * sizeof( tForState ), "Initial FOR sate");
        else forInfo.fi_data = (tForState*)
                 AGREALOC( (void*)forInfo.fi_data,
                           forInfo.fi_alloc * sizeof( tForState ),
                           "Expansion of FOR state" );
    }

    pFS = forInfo.fi_data + (forInfo.fi_depth - 1);
    memset( (void*)pFS, 0, sizeof( tForState ));
    pFS->for_firstFor = AG_TRUE;
    pFS->for_pzName   = pT->pzTemplText + pMac->ozName;

    if (OPT_VALUE_TRACE >= TRACE_BLOCK_MACROS)
        fprintf( pfTrace, "FOR %s loop in %s on line %d begins:\n",
                 pT->pzTemplText + pMac->ozName, pT->pzFileName,
                 pMac->lineNo );

    if (pT->pzTemplText[ pMac->ozText ] == '(') {
        pFS->for_from  = \
        pFS->for_to    = 0x7BAD0BAD;

        pFS->for_loading = AG_TRUE;
        (void) eval( pT->pzTemplText + pMac->ozText );
        pFS->for_loading = AG_FALSE;
        loopCt = doForByStep( pT, pMac, pDef );
    }
    else {
        pFS->for_pzSep = pT->pzTemplText + pMac->ozText;
        loopCt = doForEach( pT, pMac, pDef );
    }

    forInfo.fi_depth--;

    if (OPT_VALUE_TRACE >= TRACE_BLOCK_MACROS) {
        fprintf( pfTrace, "FOR %s repeated %d times\n",
                 pT->pzTemplText + pMac->ozName, loopCt );

        if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
            fprintf( pfTrace, zFileLine, pT->pzFileName, pMac->lineNo );
    }

    return pMRet;
}

#endif /* DEFINE_LOAD_FUNCTIONS defined */

tSCC zNoEnd[] = "%s ERROR:  FOR loop `%s' does not end\n";

MAKE_LOAD_PROC( For )
{
    char*        pzCopy = pT->pNext; /* next text dest   */
    const char*  pzSrc  = (const char*)pMac->ozText; /* macro text */
    int          srcLen = (int)pMac->res;            /* macro len  */
    tMacro*      pEndMac;

    /*
     *  Save the global macro loading mode
     */
    tpLoadProc* papLP = papLoadProc;

    static tpLoadProc apForLoad[ FUNC_CT ] = { (tpLoadProc)NULL };

    papLoadProc = apForLoad;

    /*
     *  IF this is the first time here,
     *  THEN set up the "FOR" mode callout table.
     *  It is the standard table, except entries are inserted
     *  for functions that are enabled only while processing
     *  a FOR macro
     */
    if (apForLoad[0] == (tpLoadProc)NULL) {
        memcpy( (void*)apForLoad, apLoadProc, sizeof( apLoadProc ));
        apForLoad[ FTYP_ENDFOR ] = &mLoad_Ending;
    }

    /*
     *  pzSrc points to the name of the iteration "variable"
     *  Special hack:  if the name is preceeded by a `.',
     *  then the lookup is local-only and we will accept it.
     */
    pMac->ozName = pT->pNext - pT->pzTemplText;
    if (*pzSrc == '.') {
        *(pzCopy++) = *(pzSrc++);
        if (! ISNAMECHAR( *pzSrc ))
            pzCopy--; /* force an error */
    }

    while (ISNAMECHAR( *pzSrc )) *(pzCopy++) = *(pzSrc++);
    *(pzCopy++) = '\0';

    if (pT->pzTemplText[ pMac->ozName ] == '\0')
        LOAD_ABORT( pT, pMac, "invalid FOR loop variable" );

    /*
     *  Copy the rest of the macro text into the "text" string
     */
    while (isspace( *pzSrc )) pzSrc++;
    srcLen -= pzSrc - (char*)pMac->ozText;
    if (srcLen <= 0)
        pMac->ozText = 0;
    else {
        char* pzCopied = pzCopy;
        pMac->ozText = pzCopy - pT->pzTemplText;
        do  {
            *(pzCopy++) = *(pzSrc++);
        } while (--srcLen > 0);
        *(pzCopy++) = '\0';
        if ((*pzCopied == '"') || (*pzCopied == '\''))
            spanQuote( pzCopied );
    }
    pT->pNext = pzCopy;

    pEndMac = parseTemplate( pT, pMac + 1, ppzScan );
    if (*ppzScan == (char*)NULL)
        LOAD_ABORT( pT, pMac, "ENDFOR not found" );

    pMac->endIndex = pMac->sibIndex = pEndMac - pT->aMacros;

    papLoadProc = papLP;
    return pEndMac;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * end of funcFor.c */
