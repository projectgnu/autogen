
/*
 *  $Id: funcFor.c,v 1.16 2000/08/13 21:20:24 bkorb Exp $
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

STATIC ag_bool nextDefinition( ag_bool invert, tDefEntry** ppList );
STATIC int
doForByStep( tTemplate* pT,
             tMacro*    pMac,
             tDefEntry* pFoundDef,
             tDefEntry* pCurDef );
STATIC int
doForEach( tTemplate*   pT,
           tMacro*      pMac,
           tDefEntry*   pFoundDef,
           tDefEntry*   pCurDef );

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Operational Functions */

/*=gfunc first_for_p
 *
 * what: detect first iteration
 * doc:  Returns SCM_BOOL_T if the current, innermost FOR loop is
 *       on the first pass through the data.  @xref{FOR}.
=*/
    SCM
ag_scm_first_for_p( void )
{
    return ((forInfo.for_depth > 0) && forInfo.for_firstFor)
           ? SCM_BOOL_T : SCM_BOOL_F;
}


/*=gfunc last_for_p
 *
 * what: detect last iteration
 * doc:  Returns SCM_BOOL_T if the current, innermost FOR loop is
 *       on the last pass through the data.  @xref{FOR}.
=*/
    SCM
ag_scm_last_for_p( void )
{
    return ((forInfo.for_depth > 0) && forInfo.for_lastFor)
           ? SCM_BOOL_T : SCM_BOOL_F;
}


/*=gfunc for_index
 *
 * what:  get current loop index
 * doc:   Returns the current index for innermost FOR loop.
 *        Outside of any FOR loop, it returns SCM_UNDEFINED.
 *        @xref{FOR}.
=*/
    SCM
ag_scm_for_index( void )
{
    if (forInfo.for_depth <= 0)
        return SCM_UNDEFINED;
    return gh_int2scm( forInfo.for_index );
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
    if ((! forInfo.for_loading) || (! gh_number_p( from )))
        return SCM_UNDEFINED;
    forInfo.for_from = gh_scm2int( from );
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
    if ((! forInfo.for_loading) || (! gh_number_p( to )))
        return SCM_UNDEFINED;
    forInfo.for_to = gh_scm2int( to );
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
    if ((! forInfo.for_loading) || (! gh_number_p( by )))
        return SCM_UNDEFINED;
    forInfo.for_by = gh_scm2int( by );
    return SCM_BOOL_T;
}


/*=gfunc for_sep
 *
 * what:   set loop separation string
 * exparg: separator, the text to insert between each AutoGen FOR iteration
 *
 * doc:
 *  This function records the separation string that gets inserted between
 *  each iteration of an AutoGen FOR function.  @xref{FOR}.
=*/
    SCM
ag_scm_for_sep( SCM obj )
{
    if ((! forInfo.for_loading) || (! gh_string_p( obj )))
        return SCM_UNDEFINED;
    forInfo.for_pzSep = strdup( SCM_CHARS( obj ));
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
        if (pList->index == forInfo.for_index) {
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
            ? (pList->index < forInfo.for_index)
            : (pList->index > forInfo.for_index)) {

            /*
             *  When the "by" step is zero, force syncronization.
             */
            if (forInfo.for_by == 0) {
                forInfo.for_index = pList->index;
                haveMatch = AG_TRUE;
            }
            break;
        }

        /*
         *  The current index (forInfo.for_index) is past the current value
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
             tDefEntry* pFoundDef,
             tDefEntry* pCurDef )
{
    int         settings = 0;
    int         loopCt   = 0;
    tDefEntry   textDef;
    tDefEntry*  pPassDef;
    ag_bool     invert    = (forInfo.for_by < 0);
    t_word      loopLimit = OPT_VALUE_LOOP_LIMIT;

    if (forInfo.for_pzSep == (char*)NULL)
        forInfo.for_pzSep = "";

    if (pFoundDef->pEndTwin != (tDefEntry*)NULL)
         pPassDef = pFoundDef->pEndTwin;
    else pPassDef = pFoundDef;

    if (forInfo.for_from == 0x7BAD0BAD)
        forInfo.for_from = (invert) ? pPassDef->index : pFoundDef->index;

    if (forInfo.for_to == 0x7BAD0BAD)
        forInfo.for_to = (invert) ? pFoundDef->index : pPassDef->index;

    /*
     *  Make sure we have some work to do before we start.
     */
    if (invert) {
        if (forInfo.for_from < forInfo.for_to)
            return 0;
    } else {
        if (forInfo.for_from > forInfo.for_to)
            return 0;
    }

    forInfo.for_index = forInfo.for_from;
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
                     "by %d:\n\tmore than %d iterations\n",
                     pT->pzTemplText + pMac->ozText,
                     forInfo.for_from, forInfo.for_to, forInfo.for_by,
                     OPT_VALUE_LOOP_LIMIT );
            break;
        }

        if (forInfo.for_by != 0) {
            nextIdx = forInfo.for_index + forInfo.for_by;

        } else if (invert) {
            nextIdx = (pFoundDef->pPrevTwin == (tDefEntry*)NULL)
                ? forInfo.for_to - 1  /* last iteration !! */
                : pFoundDef->pPrevTwin->index;

        } else {
            nextIdx = (pFoundDef->pTwin == (tDefEntry*)NULL)
                ? forInfo.for_to + 1  /* last iteration !! */
                : pFoundDef->pTwin->index;
        }

        /*
         *  IF we have a non-base definition, ...
         */
        if (! gotNewDef)
            pPassDef = pCurDef;

        /*
         *  ELSE IF this macro is a text type
         *  THEN create an un-twinned version of it to be found first
         */
        else if (pFoundDef->valType == VALTYP_TEXT) {
            textDef = *pFoundDef;
            textDef.pNext = textDef.pTwin = (tDefEntry*)NULL;
            textDef.pDad  = pCurDef;
            pPassDef = &textDef;
        }

        /*
         *  ELSE the current definitions are based on the block
         *       macro's values
         */
        else
            pPassDef = (tDefEntry*)(void*)pFoundDef->pzValue;

        forInfo.for_lastFor = (invert)
            ? (nextIdx < forInfo.for_to)
            : (nextIdx > forInfo.for_to);
        generateBlock( pT, pMac+1, pT->aMacros + pMac->endIndex, pPassDef );
        loopCt++;

        if (forInfo.for_lastFor)
            break;

        fputs( forInfo.for_pzSep, pCurFp->pFile );
        forInfo.for_firstFor = AG_FALSE;
        forInfo.for_index = nextIdx;
    }
    return loopCt;
}

    STATIC int
doForEach( tTemplate*   pT,
           tMacro*      pMac,
           tDefEntry*   pFoundDef,
           tDefEntry*   pCurDef )
{
    int loopCt = 0;

    for (;;) {
        tDefEntry  textDef;
        tDefEntry* pIterate;

        /*
         *  IF this loops over a text macro,
         *  THEN create a definition that will be found *before*
         *       the repeated (twinned) copy.  That way, when it
         *       is found as a macro invocation, the current value
         *       will be extracted, instead of the value list.
         */
        if (pFoundDef->valType == VALTYP_TEXT) {
            textDef = *pFoundDef;
            textDef.pNext    = \
            textDef.pTwin    = \
            textDef.pEndTwin = (tDefEntry*)NULL;
            textDef.pDad     = pCurDef;
            pIterate = &textDef;
        } else {
            pIterate = (tDefEntry*)(void*)pFoundDef->pzValue;
        }

        /*
         *  Set the global current index
         */
        forInfo.for_index = pFoundDef->index;

        /*
         *  Advance to the next twin
         */
        pFoundDef = pFoundDef->pTwin;
        if (pFoundDef == (tDefEntry*)NULL)
            forInfo.for_lastFor = AG_TRUE;

        generateBlock( pT, pMac+1, pT->aMacros + pMac->endIndex, pIterate );
        loopCt++;

        if (pFoundDef == (tDefEntry*)NULL)
            break;
        forInfo.for_firstFor = AG_FALSE;

        /*
         *  Emit the iteration separation
         */
        fputs( forInfo.for_pzSep, pCurFp->pFile );
    }
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
 *  and/or @code{for-sep}.  @xref{Scheme Functions}.  Otherwise, the
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
    tForInfo    saveFor = forInfo;
    int         loopCt;

    pDef = findDefEntry( pT->pzTemplText + pMac->ozName, pCurDef,
                         &isIndexed );
    if (pDef == (tDefEntry*)NULL) {
        if (OPT_VALUE_TRACE >= TRACE_BLOCK_MACROS) {
            fprintf( pfTrace, "FOR loop skipped - no definition for `%s'\n",
                     pT->pzTemplText + pMac->ozName );

            if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
                fprintf( pfTrace, zFileLine, pT->pzFileName, pMac->lineNo );
        }

        return pMRet;
    }

    memset( (void*)&forInfo, 0, sizeof( forInfo ));
    forInfo.for_depth    = saveFor.for_depth + 1;
    forInfo.for_firstFor = AG_TRUE;

    if (OPT_VALUE_TRACE >= TRACE_BLOCK_MACROS)
        fprintf( pfTrace, "FOR %s loop in %s on line %d begins:\n",
                 pT->pzTemplText + pMac->ozName, pT->pzFileName,
                 pMac->lineNo );

    if (pT->pzTemplText[ pMac->ozText ] == '(') {
        forInfo.for_from  = \
        forInfo.for_to    = 0x7BAD0BAD;

        forInfo.for_loading = AG_TRUE;
        (void) eval( pT->pzTemplText + pMac->ozText );
        forInfo.for_loading = AG_FALSE;
        loopCt = doForByStep( pT, pMac, pDef, pCurDef );
    }
    else {
        forInfo.for_pzSep = pT->pzTemplText + pMac->ozText;
        loopCt = doForEach( pT, pMac, pDef, pCurDef );
    }

    forInfo = saveFor;

    /*
     *  The FOR function changes the global definition context.
     *  Restore it to the one passed in.
     */
    pDefContext  = pCurDef;

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
     */
    pMac->ozName = pT->pNext - pT->pzTemplText;
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
        LOAD_ABORT( pT, pMac, "parse error" );

    pMac->endIndex = pMac->sibIndex = pEndMac - pT->aMacros;

    papLoadProc = papLP;
    return pEndMac;
}
/* end of funcFor.c */
