
/*
 *  $Id: funcDef.c,v 1.18 1999/11/04 04:37:00 bruce Exp $
 *
 *  This module implements the DEFINE text function.
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

#include <streqv.h>
#include <guile/gh.h>

typedef int (tCmpProc)( const void*, const void* );

typedef struct def_list tDefList;
struct def_list {
    tDefEntry  de;
    char*      pzExpr;
};

tSCC  zNoResolution[] = "Could not resolve INVOKE macro name - %s";

#ifdef DEBUG
/*=macfunc DEBUG, ifdef DEBUG
 *
 *  handler_proc:
 *  load_proc:
 *  what:  Provide break point spots
 *  desc:
 *      By inserting [+DEBUG n+] into your template, you can set
 *      a breakpoint on the #n case element below and step through
 *      the processing of interesting parts of your template.
=*/
MAKE_HANDLER_PROC( Debug )
{
    static int dummy = 0;

    /*
     *  The case element values were chosen to thwart any
     *  optimizer that might be too bright for its own good.
     */
    switch (atoi( pT->pzTemplText + pMac->ozText )) {
    case 0:
        dummy = 'A'; break;
    case 1:
        dummy = 'u'; break;
    case 2:
        dummy = 't'; break;
    case 4:
        dummy = 'o'; break;
    case 8:
        dummy = 'G'; break;
    case 16:
        dummy = 'e'; break;
    case 32:
        dummy = 'n'; break;
    case 64:
        dummy = 'X'; break;
    case 128:
        dummy = 'Y'; break;
    case 256:
        dummy = 'Z'; break;
    case 512:
        dummy = '.'; break;
    default:
        dummy++;
    }
    return pMac+1;
}
#endif


/*=macfunc DEFINE
 *
 *  what:    Define a user AutoGen macro
 *  cindex:  define macro
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *      This function will define a new macro.  You must provide a
 *      name for the macro.  You do not specify any arguments,
 *      though the invocation may specify a set of name/value pairs
 *      that are to be active during the processing of the macro.
 *
 *      @example
 *      [+ define foo +]
 *      ... macro body with macro functions ...
 *      [+ enddef +]
 *      ... [+ foo bar='raw text' baz=<<text expression>> +]
 *      @end example
 *
 *      Once the macro has been defined, this new macro can be invoked by
 *      specifying the macro name as the first token after the start
 *      macro marker.  Any remaining text in the macro invocation will be
 *      used to create new name/value pairs that only persist for the
 *      duration of the processing of the macro.  The expressions are
 *      evaluated the same way expression clausess are evaluated in
 *      expression functions.  @xref{EXPR}.
 *
 *      The resulting definitions are handled much
 *      like regular definitions, except:
 *
 *      @enumerate
 *      @item
 *      The values may not be compound.  That is, they may not contain
 *      nested name/value pairs.
 *      @item
 *      The bindings go away when the macro is complete.
 *      @item
 *      The name/value pairs are separated by whitespace instead of
 *      semi-colons.
 *      @item
 *      Sequences of strings are not concatenated.
 *      @end enumerate
=*/
/*=macfunc ENDDEF
 *
 *  what:   Ends a macro definition.
 *  situational:
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
MAKE_HANDLER_PROC( Define )
{
    tSCC zTplInvoked[] = "Template macro %s invoked with %d args\n";
    tDefList*   pList  = (tDefList*)pMac->res;
    int         defCt  = pMac->sibIndex;
    SCM         res;
    tDefEntry*  pDefs;
    tTemplate*  pOldTpl = pCurTemplate;

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
    if (defCt == 0)
        pDefs = pCurDef;
    else {
        pDefs = &(pList->de);
        pList->de.pDad = pCurDef;

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
                pList->de.pzValue = "";
                break;

            case '(':
                /*
                 *  It is a scheme expression.  Accept only string
                 *  and number results.
                 */
                res = scm_internal_stack_catch(
                    SCM_BOOL_T,
                    (scm_catch_body_t)gh_eval_str,
                    (void*)pList->pzExpr,
                    ag_eval_handler,
                    (void*)pList->pzExpr );

                if (gh_string_p( res )) {
                    pList->de.pzValue = strdup( SCM_CHARS( res ));
                }
                else if (gh_number_p( res )) {
                    pList->de.pzValue = (char*)AGALOC( 16 );
                    sprintf( pList->de.pzValue, "%ld", gh_scm2long( res ));
                }
                else
                    pList->de.pzValue = strdup( "" );
                break;

            case '`':
                pList->de.pzValue = runShell( pList->pzExpr+1 );
                break;
            }
        } while (pList++, --defCt > 0);
    }

    pCurTemplate = pT;
    generateBlock( pT, pT->aMacros, pT->aMacros + pT->macroCt, pDefs );
    pCurTemplate = pOldTpl;

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

    /*
     *  The DEFINE function changes the global definition context.
     *  Restore it to the one passed in.
     */
    pDefContext  = pCurDef;

    return pMac+1;
}


    STATIC int
orderDefList( const void* p1, const void* p2 )
{
    tDefEntry* pDL1 = (tDefEntry*)p1;
    tDefEntry* pDL2 = (tDefEntry*)p2;
    int cmp = streqvcmp( pDL1->pzName, pDL2->pzName );
    if (cmp == 0)
        cmp = (int)(pDL1->pzName - pDL2->pzName);
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
        if (--ct <= 0)
            break;
        pN = pNext + 1;
        if (streqvcmp( pNext->de.pzName, pN->de.pzName ) != 0)
            break;
        pN->de.pPrevTwin = &(pNext->de);
        pNext->de.pTwin  = &(pN->de);
        pNext = pN;
    }

    pDL->de.pPrevTwin = \
    pDL->de.pEndTwin  = &(pNext->de);
    pNext->de.pTwin   = (tDefEntry*)NULL; /* NULL terminated list */
    pDL->de.pPrevTwin = (tDefEntry*)NULL; /* NULL terminated list */
    *pCt = ct;
}


/*
 *  parseMacroArgs
 *
 *  This routine is called just before the first call to mFunc_Define
 *  for a particular macro invocation.  It scans the text of the invocation
 *  for name-value assignments that are only to live for the duration
 *  of the processing of the user defined macro.
 */
    void
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
        pMac->ozText = 0;
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
    pDL = (tDefList*)AGALOC( ct * sizeof( tDefList ));
    memset( (void*)pDL, 0, ct * sizeof( tDefList ));
    pMac->res = (long)pDL;

    for (;; pDL++ ) {
        pDL->de.pzName = pzScan;
        while (ISNAMECHAR( *pzScan ))  pzScan++;
        if (*pzScan == '=')
            *(pzScan++) = NUL;
        else {
            if (*pzScan == NUL) {
                pDL->de.pzValue = "";
                break;
            }
            if (! isspace( *pzScan ))
                LOAD_ABORT( pT, pMac, "name not followed by '='" );

            *(pzScan++) = NUL;
            while (isspace( *pzScan )) pzScan++;
            if (*pzScan != '=') {
                pDL->de.pzValue = "";
                if (*pzScan == NUL)
                    break;
                continue;
            }
            *(pzScan++) = NUL;
        }
        while (isspace( *pzScan ))     pzScan++;
        pDL->pzExpr = pzScan;
        pDL->de.valType = VALTYP_TEXT;
        pzScan = (char*)skipExpression( pzScan, strlen( pzScan ));

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
        }
        case '"':
        case '\'':
            /*
             *  Process the quoted strings now
             */
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
    }

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
            if (streqvcmp( pDL->de.pzName, pN->de.pzName ) == 0) {
                pN = linkTwins( pDL, pN, &ct );
                if (ct <= 0)
                    break;
            }

            pDL->de.pNext = &(pN->de);
            pDL = pN;
        }
    }
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
 *  a @i{simple expression} that must yield a known user defined macro.
 *  A macro name _must_ be found, or AutoGen will issue a diagnostic
 *  and exit.
 *
 *  Arguments are passed to the invoked macro by name.
 *  The text following the macro name must consist of a series of
 *  names each of which is followed by an equal sign (@code{=}) and
 *  a @i{simple expression} that yields a string.
=*/
MAKE_HANDLER_PROC( Invoke )
{
    char* pzText;
    SCM   macName;
    tTemplate* pInv;

    /*
     *  IF this is the first time through,
     *  THEN separate the name from the rest of the arguments.
     */
    if (pMac->ozName == 0) {
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
        }

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

            return mFunc_Define( pT, pMac, pCurDef );
        }
    }

    /*
     *  Call `eval' to determine the macro name.  Every invocation
     *  may be different!!
     */
    macName = eval( pT->pzTemplText + pMac->ozName );

    if (! gh_string_p( macName )) {
        pzText = asprintf( zNoResolution, "??not string??" );
        LOAD_ABORT( pT, pMac, pzText );
    }
    pInv = findTemplate( SCM_CHARS( macName ));
    if (pInv == (tTemplate*)NULL) {
        pzText = asprintf( zNoResolution, SCM_CHARS( macName ) );
        LOAD_ABORT( pT, pMac, pzText );
    }

    pMac->funcPrivate = (void*)pInv;

    return mFunc_Define( pT, pMac, pCurDef );
}

/*=gfunc ag_invoke
 *
 * exparg: macro, name of macro to invoke
 * exparg: args,  macro arguments, optional, ellipsis
 *
 * doc:  Invoke an AutoGen macro and put the results into a string.
 *       As of this writing, however, the macro arguments are not
 *       well implemented.  Each entry of the list must be a string
 *       that starts with a name, followed by an equal sign followed
 *       by the text value.  This is not very Schemy.  Don't rely on this.
=*/
    SCM
ag_scm_ag_invoke( SCM macName, SCM list )
{
    SCM         res = SCM_UNDEFINED, car;
    tTemplate*  pT;
    tMacro*     pMac;
    char*       pz;

    int  len   = scm_ilength( list );
    int  defCt = 0;

    tDefEntry*  pDefList;
    tDefEntry*  pDE;

    if (! gh_string_p( macName ))
        LOAD_ABORT( pT, pMac, zNoResolution );

    pT = findTemplate( SCM_CHARS( macName ));
    if (pT == (tTemplate*)NULL)
        LOAD_ABORT( pCurTemplate, pCurMacro, zNoResolution );

    if (len <= 0) {
        pDE = pDefContext;
        pDefList = (tDefEntry*)NULL;
    }

    else {
        pDefList = pDE = (tDefEntry*)AGALOC( sizeof( *pDE ) * len );
        memset( (void*)pDE, 0, sizeof( *pDE ) * len );
        for (;;) {
            if (len-- <= 0)
                break;

            defCt++;
            car  = SCM_CAR( list );
            list = SCM_CDR( list );
            if (! gh_string_p( car ))
                LOAD_ABORT( pCurTemplate, pCurMacro,
                            "need name for macro arg" );
            pz = pDE->pzName = strdup( SCM_CHARS( car ));
            while (ISNAMECHAR( *pz ))  pz++;
            while (isspace( *pz ))     pz++;
            if (*pz != NUL) {
                pDE++;
                continue;
            }

            if (*pz != '=')
                LOAD_ABORT( pCurTemplate, pCurMacro,
                            "garbage in macro arg name" );

            *(pz++) = NUL;
            if (*pz != NUL) {
                while (isspace( *pz ))  pz++;
                pDE->pzValue = pz;
                continue;
            }

            if (len-- <= 0)
                LOAD_ABORT( pCurTemplate, pCurMacro,
                            "missing value for macro arg" );

            car  = SCM_CAR( list );
            list = SCM_CDR( list );
            if (! gh_string_p( car ))
                LOAD_ABORT( pCurTemplate, pCurMacro,
                            "need name for macro arg" );
            pDE->pzValue = strdup( SCM_CHARS( car ));
        }

        if (defCt > 1) {
            /*
             *  There was more than one value assignment.
             *  Sort them just so we know the siblings are together.
             *  Do not use qsort().  The first entries must be first.
             */
            qsort( (void*)pMac->res, defCt, sizeof( tDefList ), orderDefList );
#ifdef LATER
            /*
             *  The structures need to be made to be pDefLists, not defentries
             */
            /*
             *  Now, link them all together.  Singly linked list.
             */
            for (;;) {
                if (--defCt == 0) {
                    pDE->pNext = (tDefEntry*)NULL;
                    break;
                }
    
                pN = pDE + 1;
    
                /*
                 *  IF the next entry has the same name,
                 *  THEN it is a "twin".  Link twins on the twin list.
                 */
                if (streqvcmp( pDL->pzName, pN->pzName ) == 0)
                    pN = linkTwins( pDL, pN, &ct );
    
                pDE->pNext = pN;
                pDE = pN;
            }
#endif
        }
    }

    /*
     *  FIXME:  insert code that will
     *  1.  Process any invocation arguments
     *  2.  Divert to a _UNIQUE_ file name
     *  3.  read back the diversion and set the result SCM to it
     *  4.  delete the diversion file
     *  5.  deallocate anything allocated
     */
    return res;
}
#endif /* DEFINE_LOAD_FUNCTIONS not defined */

/* Load Debug
 *
 *  what:   Loads the debug function so you can set breakpoints
 *          at load time, too :-)
=*/
MAKE_LOAD_PROC( Debug )
{
    return mLoad_Unknown( pT, pMac, ppzScan );
}


MAKE_LOAD_PROC( Define )
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
        pNewT = (tTemplate*)AGALOC( alocSize );
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

    {
        tMacro* pMacEnd = parseTemplate( pNewT, pNewT->aMacros, ppzScan );

        /*
         *  Make sure all of the input string was *NOT* scanned.
         */
        if (*ppzScan == (char*)NULL)
            LOAD_ABORT( pT, pMac, "parse ended unexpectedly" );

        /*
         *  FIXME:  this expression does not work!!
         *
         *  pMacEnd - pNewT->aMacros
         */
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
    pNewT = (tTemplate*)AGREALOC( (void*)pNewT, pNewT->descSize );
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
/* end of funcDef.c */
