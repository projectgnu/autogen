
/*
 *  $Id: funcDef.c,v 3.0 2001/12/09 19:23:13 bkorb Exp $
 *
 *  This module implements the DEFINE text function.
 */

/*
 *  AutoGen copyright 1992-2001 Bruce Korb
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

#include <guile/gh.h>

#include "expr.h"
#include "autogen.h"

tSCC zNil[] = "";

typedef int (tCmpProc)( const void*, const void* );

typedef struct def_list tDefList;
struct def_list {
    tDefEntry  de;
    char*      pzExpr;
};

tSCC  zNoResolution[] = "Could not resolve macro name: ``%s''";
tSCC zTplInvoked[] = "Template macro %s invoked with %d args\n";
#endif /* not defined DEFINE_LOAD_FUNCTIONS */

STATIC tDefList* linkTwins( tDefList* pDL, tDefList* pNext, int* pCt );

STATIC int   orderDefList( const void* p1, const void* p2 );
STATIC void  prepInvokeArgs( tMacro* pMac );


    STATIC int
orderDefList( const void* p1, const void* p2 )
{
    tDefEntry* pDL1 = (tDefEntry*)p1;
    tDefEntry* pDL2 = (tDefEntry*)p2;
    int cmp = streqvcmp( pDL1->pzDefName, pDL2->pzDefName );

    /*
     *  IF the names are the same, then we order them
     *  based on which name appears first.  We do _not_
     *  want the entries reordered within the same name!
     */
    if (cmp == 0)
        cmp = (int)(pDL1->pzDefName - pDL2->pzDefName);
    return cmp;
}


    STATIC tDefList*
linkTwins( tDefList* pDL, tDefList* pNext, int* pCt )
{
    tDefList* pN;
    int  ct  = *pCt;
    int  idx = 1;

    pDL->de.pTwin = &(pNext->de);
    pNext->de.pPrevTwin = &(pDL->de);

    for (;;) {
        pNext->de.index = idx++;
        pN = pNext + 1; /* We return this, valid or not */
        if (--ct <= 0)  /* count each successive twin   */
            break;
        if (streqvcmp( pNext->de.pzDefName, pN->de.pzDefName ) != 0)
            break;

        /*
         *  We have found another twin.  Link it in and advance
         */
        pNext->de.pTwin  = &(pN->de);
        pN->de.pPrevTwin = &(pNext->de);
        pNext = pN;
    }

    pDL->de.pEndTwin  = &(pNext->de);
    pNext->de.pTwin   = (tDefEntry*)NULL; /* NULL terminated list */
    pDL->de.pPrevTwin = (tDefEntry*)NULL; /* NULL terminated list */
    *pCt = ct;
    pDL->de.pNext = (tDefEntry*)NULL;     /* in case ct == 0      */
    return pN; /* If ct is zero, then this is invalid */
}


/*
 *  parseMacroArgs
 *
 *  This routine is called just before the first call to mFunc_Define
 *  for a particular macro invocation.  It scans the text of the invocation
 *  for name-value assignments that are only to live for the duration
 *  of the processing of the user defined macro.
 */
    EXPORT void
parseMacroArgs( tTemplate* pT, tMacro* pMac )
{
    char*        pzScan = pT->pzTemplText + pMac->ozText;
    int          ct     = 0;
    tDefList*    pDL;
    tDefList*    pN;

    /*
     *  If there is no argument text, then the arg count is zero.
     */
    if (pMac->ozText == 0) {
        pMac->res = 0;
        return;
    }

    /*
     *  Count the number of named values in the argument list
     */
    while (*pzScan != NUL) {
        ct++;
        if (! isalpha( *pzScan )) {
            fprintf( stderr, "On macro argument # %d:\n%s\n",
                     ct, pzScan );
            LOAD_ABORT( pT, pMac, "no macro arg name" );
        }

        while (ISNAMECHAR( *pzScan ))  pzScan++;
        while (isspace( *pzScan ))     pzScan++;
        if (*pzScan != '=')
            continue;
        while (isspace( *++pzScan ))   ;
        pzScan = (char*)skipExpression( pzScan, strlen( pzScan ));
        while (isspace( *pzScan ))     pzScan++;
    }

    /*
     *  The result is zero if we don't have any
     */
    pMac->sibIndex = ct;
    if (ct == 0) {
        pMac->ozText = 0;
        pMac->res = 0;
        return;
    }

    /*
     *  Allocate the array of definition descriptors
     */
    pzScan = pT->pzTemplText + pMac->ozText;
    pDL = (tDefList*)AGALOC( ct * sizeof( tDefList ), "array of def desc" );
    memset( (void*)pDL, 0, ct * sizeof( tDefList ));
    pMac->res = (long)pDL;

    /*
     *  Fill in the array of value assignments
     */
    for (;; pDL++ ) {
        pDL->de.pzDefName = pzScan;
        while (ISNAMECHAR( *pzScan ))  pzScan++;

        switch (*pzScan) {
        case NUL:
            pDL->de.pzValue = (char*)zNil; goto fill_in_array_done;

        default:
            LOAD_ABORT( pT, pMac, "name not followed by '='" );

        case ' ': case '\t': case '\n': case '\f':
            *(pzScan++) = NUL;
            while (isspace( *pzScan )) pzScan++;

            /*
             *  The name was separated by space, but has no value
             */
            if (*pzScan != '=') {
                pDL->de.pzValue = (char*)zNil;
                if (*pzScan == NUL)
                    goto fill_in_array_done;
                goto fill_in_array_continue;
            }
            /* FALLTHROUGH */
        case '=':
            *(pzScan++) = NUL;
        }

        /*
         *  When we arrive here, we have just clobbered a '=' char.
         *  Now we have gather up the assigned value.
         */
        while (isspace( *pzScan ))     pzScan++;
        strtransform( pDL->de.pzDefName, pDL->de.pzDefName );
        pDL->pzExpr = pzScan;
        pDL->de.valType = VALTYP_TEXT;
        pzScan = (char*)skipExpression( pzScan, strlen( pzScan ));

        /*
         *  Figure out what kind of expression we have
         */
        switch (*pDL->pzExpr) {
        case ';':
        case '(':
            /*
             *  These expressions will need evaluation
             */
            break;

        case '`':
        {
            char* pz;
            /*
             *  Process the quoted string, but leave a '`' marker, too
             */
            pz = strdup( pDL->pzExpr );
            spanQuote( pz );
            strcpy( pDL->pzExpr+1, pz );
            AGFREE( (void*)pz );
            break;
        }
        case '"':
        case '\'':
            /*
             *  Process the quoted strings now
             */
            if ((pzScan - pDL->pzExpr) < 24) {
                char* pz = (char*)AGALOC( 24, "quoted string" );
                memcpy( (void*)pz, pDL->pzExpr, (pzScan - pDL->pzExpr) );
                pDL->pzExpr = pz;
            }
            spanQuote( pDL->pzExpr );
            /* FALLTHROUGH */

        default:
            /*
             *  Default:  the raw sequence of characters is the value
             */
            pDL->de.pzValue = pDL->pzExpr;
            pDL->pzExpr     = (char*)NULL;
        }

        /*
         *  IF the next char is NUL, we are done.
         *  OTHERWISE, the next character must be a space
         */
        if (*pzScan == NUL)
            break;

        if (! isspace( *pzScan ))
            LOAD_ABORT( pT, pMac, "no space separating entries" );

        /*
         *  Terminate the string value and skip over any additional space
         */
        *(pzScan++) = NUL;
        while (isspace( *pzScan ))     pzScan++;
    fill_in_array_continue:;
    } fill_in_array_done:;

    if (ct > 1) {
        /*
         *  There was more than one value assignment.
         *  Sort them just so we know the siblings are together.
         *  Order is preserved by comparing string addresses,
         *  if the strings compare equal.
         */
        pDL = (tDefList*)pMac->res;
        qsort( (void*)pDL, ct, sizeof( tDefList ), orderDefList );

        /*
         *  Now, link them all together.  Singly linked list.
         */
        for (;;) {
            if (--ct == 0) {
                pDL->de.pNext = (tDefEntry*)NULL;
                break;
            }

            pN = pDL + 1;

            /*
             *  IF the next entry has the same name,
             *  THEN it is a "twin".  Link twins on the twin list.
             */
            if (streqvcmp( pDL->de.pzDefName, pN->de.pzDefName ) == 0) {
                pN = linkTwins( pDL, pN, &ct );
                if (ct <= 0)
                    break;  /* pN is now invalid */
            }

            pDL->de.pNext = &(pN->de);
            pDL = pN;
        }
    }
}


    STATIC void
prepInvokeArgs( tMacro* pMac )
{
    char*  pzText;
    tTemplate* pT = pCurTemplate;

    if (pMac->ozText == 0)
        LOAD_ABORT( pT, pMac, "The INVOKE macro requires a name" );
    pMac->ozName = pMac->ozText;
    pzText = pT->pzTemplText + pMac->ozText;
    pzText = (char*)skipExpression( pzText, strlen( pzText ));

    /*
     *  IF there is no more text,
     *  THEN there are no arguments
     */
    if (*pzText == NUL) {
        pMac->ozText = 0;
        pMac->res = 0;
    }

    /*
     *  OTHERWISE, skip to the start of the text and process
     *  the arguments to the macro
     */
    else {
        if (! isspace( *pzText ))
            LOAD_ABORT( pT, pMac,
                        "The INVOKE macro name not space separated" );
        *pzText = NUL;
        while (isspace( *++pzText ))  ;
        pMac->ozText = pzText - pT->pzTemplText;
        parseMacroArgs( pT, pMac );
        pCurTemplate = pT;
    }
}


#ifndef DEFINE_LOAD_FUNCTIONS
#ifdef DEBUG
/*=macfunc DEBUG, ifdef DEBUG
 *
 *  handler_proc:
 *  what:  Provide break point spots
 *  desc:
 *      By inserting [+DEBUG n+] into your template, you can set
 *      a breakpoint on the #n case element below and step through
 *      the processing of interesting parts of your template.
=*/
    tMacro*
mFunc_Debug( tTemplate* pT, tMacro* pMac )
{
    static int dummy = 0;

    if (OPT_VALUE_TRACE > TRACE_NOTHING)
        fprintf( pfTrace, "  --  DEBUG %s -- FOR index %d\n",
                 pT->pzTemplText + pMac->ozText, (forInfo.fi_depth <= 0) ? -1
                 : forInfo.fi_data[ forInfo.fi_depth - 1].for_index );
    /*
     *  The case element values were chosen to thwart most
     *  optimizers that might be too bright for its own good.
     *  (`dummy' is write-only and could be ignored)
     */
    switch (atoi( pT->pzTemplText + pMac->ozText )) {
    case 0:    dummy = 'A'; break;
    case 1:    dummy = 'u'; break;
    case 2:    dummy = 't'; break;
    case 4:    dummy = 'o'; break;
    case 8:    dummy = 'G'; break;
    case 16:   dummy = 'e'; break;
    case 32:   dummy = 'n'; break;
    case 64:   dummy = 'X'; break;
    case 128:  dummy = 'Y'; break;
    case 256:  dummy = 'Z'; break;
    case 512:  dummy = '.'; break;
    default:   dummy++;
    }
    return pMac+1;
}
#endif


/*
 *  build_defs
 *
 *  Build up a definition context created by passed-in macro arguments
 */
STATIC void
build_defs( int defCt, tDefList* pList )
{
    tDefEntry* pDefs = &(pList->de);
    currDefCtx.pDefs = pDefs;

    /*
     *  FOR each definition, evaluate the associated expression
     *      and set the text value to it.
     */
    do  {
        if (pList->pzExpr == (char*)NULL)
            continue;

    retryExpression:
        switch (*(pList->pzExpr)) {
        case ';':
        {
            char* pz = strchr( pList->pzExpr, '\n' );
            if (pz != (char*)NULL) {
                while (isspace( *++pz ))  ;
                pList->pzExpr = pz;
                goto retryExpression;
            }
            /* FALLTHROUGH */
        }
        case NUL:
            pList->pzExpr = (char*)NULL;
            pList->de.pzValue = (char*)zNil;
            break;

        case '(':
        {
            SCM res;

            /*
             *  It is a scheme expression.  Accept only string
             *  and number results.
             */
            if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS) {
                fprintf( pfTrace, "Scheme eval for arg %d:\n\t`%s'\n",
                         pCurMacro->sibIndex - defCt, pList->pzExpr );
            }
            res = gh_eval_str( pList->pzExpr );

            if (gh_string_p( res )) {
                pList->de.pzValue = strdup( ag_scm2zchars( res, "eval res" ));
            }
            else if (gh_number_p( res )) {
                pList->de.pzValue = (char*)AGALOC( 16, "number buf" );
                snprintf( pList->de.pzValue, 16, "%d", gh_scm2ulong( res ));
            }
            else
                pList->de.pzValue = strdup( zNil );
            break;
        }

        case '`':
            if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS) {
                fprintf( pfTrace, "shell eval for arg %d:\n\t`%s'\n",
                         pCurMacro->sibIndex - defCt, pList->pzExpr+1 );
            }
            pList->de.pzValue = runShell( pList->pzExpr+1 );
            break;
        }
    } while (pList++, --defCt > 0);
}


/*=macfunc DEFINE
 *
 *  what:    Define a user AutoGen macro
 *  cindex:  define macro
 *  handler_proc:
 *
 *  desc:
 *
 *  This function will define a new macro.  You must provide a name for the
 *  macro.  You do not specify any arguments, though the invocation may
 *  specify a set of name/value pairs that are to be active during the
 *  processing of the macro.
 *
 *  @example
 *  [+ define foo +]
 *  ... macro body with macro functions ...
 *  [+ enddef +]
 *  ... [+ foo bar='raw text' baz=<<text expression>> +]
 *  @end example
 *
 *  Once the macro has been defined, this new macro can be invoked by
 *  specifying the macro name as the first token after the start macro
 *  marker.  Alternatively, you may make the invocation explicitly invoke a
 *  defined macro by specifying @code{INVOKE} in the macro invocation.  If
 *  you do that, the macro name can be computed with an expression that gets
 *  evaluated every time the INVOKE macro is encountered.  @xref{INVOKE}.
 *
 *  Any remaining text in the macro invocation will be used to create new
 *  name/value pairs that only persist for the duration of the processing of
 *  the macro.  The expressions are evaluated the same way basic
 *  expressions are evaluated.  @xref{expression syntax}.
 *
 *  The resulting definitions are handled much like regular
 *  definitions, except:
 *
 *  @enumerate
 *  @item
 *  The values may not be compound.  That is, they may not contain
 *  nested name/value pairs.
 *  @item
 *  The bindings go away when the macro is complete.
 *  @item
 *  The name/value pairs are separated by whitespace instead of
 *  semi-colons.
 *  @item
 *  Sequences of strings are not concatenated.
 *  @end enumerate
=*/
/*=macfunc ENDDEF
 *
 *  what:   Ends a macro definition.
 *
 *  desc:
 *    This macro ends the @code{DEFINE} function template block.
 *    For a complete description @xref{DEFINE}.
=*/
/*
 *  mFunc_Define
 *
 *  This routine runs the invocation.
 */
    tMacro*
mFunc_Define( tTemplate* pT, tMacro* pMac )
{
    tDefList*   pList  = (tDefList*)pMac->res;
    int         defCt  = pMac->sibIndex;
    SCM         res;
    tDefEntry*  pDefs;
    tDefStack*  pStack;
    tDefStack   stack;

    pT = (tTemplate*)pMac->funcPrivate;

    if (OPT_VALUE_TRACE > TRACE_NOTHING) {
        fprintf( pfTrace, zTplInvoked, pT->pzTplName, defCt );
        if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
            fprintf( pfTrace, zFileLine, pCurTemplate->pzFileName,
                     pMac->lineNo );
    }

    /*
     *  IF we have no special definitions, then do not nest definitions
     */
    if (defCt != 0) {
        stack  = currDefCtx;
        currDefCtx.pPrev = &stack;
        build_defs( defCt, pList );
    }

    {
        tTemplate*  pOldTpl = pCurTemplate;
        pCurTemplate = pT;

        generateBlock( pT, pT->aMacros, pT->aMacros + pT->macroCt );
        pCurTemplate = pOldTpl;
    }

    if (defCt != 0)
        currDefCtx = stack;

    if ((defCt = pMac->sibIndex) > 0) {
        pList = (tDefList*)pMac->res;
        while (defCt-- > 0) {
            if (pList->pzExpr != (char*)NULL) {
                AGFREE( (void*)pList->de.pzValue );
                pList->de.pzValue = (char*)NULL;
            }
            pList++;
        }
    }

    return pMac+1;
}


/*=macfunc INVOKE
 *
 *  handler_proc:
 *  what:  Invoke a User Defined Macro
 *
 *  desc:
 *
 *  User defined macros may be invoked explicitly or implicitly.
 *  If you invoke one implicitly, the macro must begin with the
 *  name of the defined macro.  Consequently, this may @strong{not}
 *  be a computed value.  If you explicitly invoke a user defined macro,
 *  the macro begins with the macro name @code{INVOKE} followed by
 *  a @i{basic expression} that must yield a known user defined macro.
 *  A macro name _must_ be found, or AutoGen will issue a diagnostic
 *  and exit.
 *
 *  Arguments are passed to the invoked macro by name.
 *  The text following the macro name must consist of a series of
 *  names each of which is followed by an equal sign (@code{=}) and
 *  a @i{basic expression} that yields a string.
 *
 *  The string values may contain template macros that are parsed
 *  the first time the macro is processed and evaluated again every
 *  time the macro is evaluated.
=*/
    tMacro*
mFunc_Invoke( tTemplate* pT, tMacro* pMac )
{
    char* pzText;
    SCM   macName;
    tTemplate* pInv;

    /*
     *  IF this is the first time through,
     *  THEN separate the name from the rest of the arguments.
     */
    if (pMac->ozName == 0) {
        prepInvokeArgs( pMac );

        /*
         *  IF the name is constant and not an expression,
         *  THEN go find the template now and bind the macro call
         *       to a particular template macro
         */
        if (isalpha( pT->pzTemplText[ pMac->ozName ])) {
            pMac->funcCode    = FTYP_DEFINE;
            pMac->funcPrivate = (void*)findTemplate(
                pT->pzTemplText + pMac->ozName );

            if (pMac->funcPrivate == (void*)NULL) {
                pzText = asprintf( zNoResolution,
                                   pT->pzTemplText + pMac->ozName );
                LOAD_ABORT( pT, pMac, pzText );
            }

            return mFunc_Define( pT, pMac );
        }
    }

    /*
     *  Call `eval' to determine the macro name.  Every invocation
     *  may be different!!
     */
    macName = eval( pT->pzTemplText + pMac->ozName );

    pInv = findTemplate( ag_scm2zchars( macName, "macro name" ));
    if (pInv == (tTemplate*)NULL) {
        pzText = asprintf( zNoResolution,
                           ag_scm2zchars( macName, "macro name" ));
        LOAD_ABORT( pT, pMac, pzText );
    }

    pMac->funcPrivate = (void*)pInv;

    return mFunc_Define( pT, pMac );
}
#endif /* DEFINE_LOAD_FUNCTIONS not defined */


/* Load Debug
 *
 *  what:   Loads the debug function so you can set breakpoints
 *          at load time, too :-)
=*/
    tMacro*
mLoad_Debug( tTemplate* pT, tMacro* pMac, tCC** ppzScan )
{
    return mLoad_Unknown( pT, pMac, ppzScan );
}


    tMacro*
mLoad_Define( tTemplate* pT, tMacro* pMac, tCC** ppzScan )
{
    tSCC         zNameNeeded[] = "DEFINE requires a name";
    char*        pzCopy;             /* next text dest   */
    tTemplate*   pNewT;

    /*
     *  Save the global macro loading mode
     */
    tpLoadProc* papLP = papLoadProc;
    static tpLoadProc apDefineLoad[ FUNC_CT ] = { (tpLoadProc)NULL };

    if (pMac->ozText == 0)
        LOAD_ABORT( pT, pMac, zNameNeeded );

    /*
     *  IF this is the first time here,
     *  THEN set up the "DEFINE" block callout table.
     *  It is the standard table, except entries are inserted
     *  for functions that are enabled only while processing
     *  a DEFINE block (i.e. "ENDDEF")
     */
    if (apDefineLoad[0] == (tpLoadProc)NULL) {
        memcpy( (void*)apDefineLoad, apLoadProc, sizeof( apLoadProc ));
        apDefineLoad[ FTYP_ENDDEF ] = &mLoad_Ending;
    }
    papLoadProc = apDefineLoad;

    {
        const char*    pzScan = *ppzScan;  /* text after macro */
        const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
        int            macCt  = pT->macroCt - (pMac - pT->aMacros);
        int            fnameSize = strlen( pT->pzFileName ) + 1;

        size_t alocSize = sizeof( *pNewT ) + fnameSize
            + (macCt * sizeof( tMacro )) + strlen( pzScan ) + 0x100;
        alocSize &= ~0x0F;

        /*
         *  Allocate a new template block that is much larger than needed.
         */
        pNewT = (tTemplate*)AGALOC( alocSize, "AG macro definition" );
        memset( (void*)pNewT, 0, alocSize );
        memcpy( (void*)&(pNewT->magic), (void*)&(pT->magic),
                sizeof( pNewT->magic ));
        pNewT->fd       = 0;
        pNewT->descSize = alocSize;
        pNewT->macroCt  = macCt;
        pNewT->pzFileName = (char*)(pNewT->aMacros + macCt);
        memcpy( pNewT->pzFileName, pT->pzFileName, fnameSize );
        pzCopy = pNewT->pzTplName = pNewT->pzFileName + fnameSize;
        if (! isalpha( *pzSrc ))
            LOAD_ABORT( pT, pMac, zNameNeeded );

        while (ISNAMECHAR(*pzSrc))  *(pzCopy++) = *(pzSrc++);
    }

    *(pzCopy++) = NUL;
    pNewT->pzTemplText = pzCopy;
    pNewT->pNext = pzCopy+1;
    strcpy( pNewT->zStartMac, pT->zStartMac );
    strcpy( pNewT->zEndMac, pT->zEndMac );

    {
        tMacro* pMacEnd = parseTemplate( pNewT, pNewT->aMacros, ppzScan );

        /*
         *  Make sure all of the input string was *NOT* scanned.
         */
        if (*ppzScan == (char*)NULL)
            LOAD_ABORT( pT, pMac, "parse ended unexpectedly" );

        pNewT->macroCt = pMacEnd - &(pNewT->aMacros[0]);

        /*
         *  IF there are empty macro slots,
         *  THEN pack the text
         */
        if ((void*)pMacEnd < (void*)(pNewT->pzFileName)) {
            int  delta = pNewT->pzFileName - (char*)pMacEnd;
            int  size  = (pNewT->pNext - pNewT->pzFileName);
            memcpy( (void*)pMacEnd, (void*)(pNewT->pzFileName),
                    size );
            pNewT->pzFileName  -= delta;
            pNewT->pzTemplText -= delta;
            pNewT->pNext       -= delta;
            if (pNewT->pzTplName != 0)
                pNewT->pzTplName -= delta;
        }
    }

    /*
     *  Adjust the sizes.  Remove absolute pointers.
     *  Reallocate to the correct size.  Restore
     *  the offsets to pointer values.
     */
    pNewT->descSize     = pNewT->pNext - (char*)pNewT;
    pNewT->descSize    += 0x40;
    pNewT->descSize    &= ~0x3F;
    pNewT->pzFileName  -= (long)pNewT;
    pNewT->pzTplName   -= (long)pNewT;
    pNewT->pzTemplText -= (long)pNewT;
    pNewT = (tTemplate*)AGREALOC( (void*)pNewT, pNewT->descSize,
                                  "resize AG macro definition" );
    pNewT->pzFileName  += (long)pNewT;
    pNewT->pzTplName   += (long)pNewT;
    pNewT->pzTemplText += (long)pNewT;

    /*
     *  We cannot reallocate a smaller array because
     *  the entries are all linked together and
     *  realloc-ing it may cause it to move.
     */
#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_DEFS )
    if (HAVE_OPT( SHOW_DEFS )) {
        tSCC zSum[] = "loaded %d macros from %s\n"
            "\tBinary template size:  0x%X\n\n";
        printf( zSum, pNewT->macroCt, pNewT->pzFileName, pNewT->descSize );
    }
#endif

    pNewT->pNext  = (char*)pNamedTplList;
    pNamedTplList = pNewT;
    papLoadProc = papLP;
    memset( (void*)pMac, 0, sizeof(*pMac) );
    return pMac;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * End:
 * end of funcDef.c */
