
/*
 *  $Id: funcDef.c,v 1.28 2000/04/10 13:25:06 bkorb Exp $
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

#include <streqv.h>
#include <guile/gh.h>

#include "autogen.h"
#include "expr.h"

typedef int (tCmpProc)( const void*, const void* );

typedef struct def_list tDefList;
struct def_list {
    tDefEntry  de;
    char*      pzExpr;
};

tSCC  zNoResolution[] = "Could not resolve INVOKE macro name - %s";
tSCC zTplInvoked[] = "Template macro %s invoked with %d args\n";
#endif /* not defined DEFINE_LOAD_FUNCTIONS */

STATIC tDefList* linkTwins( tDefList* pDL, tDefList* pNext, int* pCt );

STATIC int   orderDefList( const void* p1, const void* p2 );
STATIC char* skipNestedExpression( char* pzScan, size_t len );
STATIC void  spanTemplate( char* pzQte, tDefList* pDefList );
STATIC void  prepInvokeArgs( tMacro* pMac );
STATIC char* anonymousTemplate( void* p, tDefEntry* pCurDef );


    STATIC int
orderDefList( const void* p1, const void* p2 )
{
    tDefEntry* pDL1 = (tDefEntry*)p1;
    tDefEntry* pDL2 = (tDefEntry*)p2;
    int cmp = streqvcmp( pDL1->pzName, pDL2->pzName );

    /*
     *  IF the names are the same, then we order them
     *  based on which name appears first.  We do _not_
     *  want the entries reordered within the same name!
     */
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
        pN = pNext + 1; /* We return this, valid or not */
        if (--ct <= 0)  /* count each successive twin   */
            break;
        if (streqvcmp( pNext->de.pzName, pN->de.pzName ) != 0)
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
 *  skipNestedExpression
 *
 *  This is a wrapper around "skipExpression" that has a special purpose:
 *  We are now allowing templates to appear inside of a quoted string
 *  passed as an argument to a user macro invocation.  That is a tricky
 *  thing to parse and not needed everywhere.  Not even doable everywhere.
 *  It can only be done during the initial parse of a template because
 *  that is the only time when the start/end macro markers are known.
 */
    STATIC char*
skipNestedExpression( char* pzScan, size_t len )
{
    char* pzStartMark   = pCurTemplate->zStartMac;

    tSCC zOnlyStrings[] = "Only strings may contain nested templates";
    char* pzStartMacro  = strstr( pzScan, pzStartMark );
    char* pzExpEnd      = (char*)skipExpression( pzScan, len );
    char q;

    if ((pzStartMacro == (char*)NULL) || (pzExpEnd < pzStartMacro))
        return pzExpEnd;

    switch (*pzScan) {
    case '"':
    case '\'':
        break;
    default:
        LOAD_ABORT( pCurTemplate, pCurMacro, zOnlyStrings );
    }

    q = *pzScan++;        /*  Save the quote character type */

    /*
     *  skipMacro() needs whatever the correct mark is for the
     *  macro when it was originally defined.
     */
    strcpy( zStartMac, pzStartMark );
    startMacLen = strlen( pzStartMark );
    strcpy( zEndMac, pCurTemplate->zEndMac );
    endMacLen = strlen( zEndMac );

    while (*pzScan != q) {
        /*
         *  IF we are at the start of a macro invocation,
         *  THEN it is time to skip over it.
         */
        if (pzScan == pzStartMacro) {
            pzScan = (char*)skipMacro( pzStartMacro ) + endMacLen;
            pzStartMacro = strstr( pzScan, pzStartMark );
            continue;
        }

        switch (*pzScan++) {
        case NUL:
            return pzScan-1;      /* Return address of terminating NUL */

        case '\\':
            if (q == '\'') {
                /*
                 *  Single quoted strings process the backquote specially
                 *  only in fron of these three characters:
                 */
                switch (*pzScan) {
                case '\\':
                case '\'':
                case '#':
                    pzScan++;
                }

            } else {
                char p[10];  /* provide a scratch pad for escape processing */
                unsigned int ct = doEscapeChar( pzScan, p );
                /*
                 *  IF the advance is zero,
                 *  THEN we either have end of string (caught above),
                 *       or we have an escaped new-line,
                 *       which is to be ignored.
                 *  ELSE advance the quote scanning pointer by ct
                 */
                if (ct == 0) {
                    pzScan++; /* skip over new-line character        */
                } else
                    pzScan += ct;
            } /* if (q == '\'')      */
        }     /* switch (*pzScan++)   */
    }         /* while (*pzScan != q) */

    return pzScan+1; /* Return addr of char after the terminating quote */
}


/*
 *  The following routine scans over quoted text, shifting
 *  it in the process and eliminating the starting quote,
 *  ending quote and any embedded backslashes.  They may
 *  be used to embed the quote character in the quoted text.
 *  The quote character is whatever character the argument
 *  is pointing at when this procedure is called.
 */
    STATIC void
spanTemplate( char* pzQte, tDefList* pDefList )
{
    char* pzStartMark   = pCurTemplate->zStartMac;

    char* pzStartMacro  = strstr( pzQte, pzStartMark );
    char* pzExpEnd = (char*)skipNestedExpression( pzQte, strlen( pzQte ));
    char* pzText = pzQte;
    char* p;
    char  q;
    int   mac_ct = 1;

    /*
     *  IF there are no more macros *OR* the current macro ends before
     *     the start of the next macro,
     *  THEN we don't have to worry about macros within quoted strings.
     */
    if (  (pzStartMacro == (char*)NULL)
       || (pzExpEnd < pzStartMacro)) {
        (void)spanQuote( pzQte );
	return;
    }

    q = *pzQte;          /*  Save the quote character type */
    p = pzQte++;         /*  Destination pointer           */

    /*
     *  skipMacro() needs whatever the correct mark is for the
     *  macro when it was originally defined.
     */
    strcpy( zStartMac, pzStartMark );
    startMacLen = strlen( pzStartMark );
    strcpy( zEndMac, pCurTemplate->zEndMac );
    endMacLen = strlen( zEndMac );

    while (*pzQte != q) {
        /*
         *  IF we are at the start of a macro invocation,
         *  THEN it is time to skip over it.
         */
        if (pzQte == pzStartMacro) {
            mac_ct += 2;
            pzQte = (char*)skipMacro( pzStartMacro ) + endMacLen;

            /*
             *  skipMacro has moved the macro text off elsewhere.
             *  Shift the remainder of the string down, find next start.
             */
            memcpy( (void*)p, pzStartMacro, (pzQte - pzStartMacro) );
            p += (pzQte - pzStartMacro);
            pzStartMacro = strstr( pzQte, pzStartMark );
            continue;
        }

        switch (*p++ = *pzQte++) {
        case NUL:
            pzQte--;      /* Return address of terminating NUL */
            goto NUL_termination;

        case '\\':
            if (q != '\'') {
                unsigned int ct = doEscapeChar( pzQte, p-1 );
                /*
                 *  IF the advance is zero,
                 *  THEN we either have end of string (caught above),
                 *       or we have an escaped new-line,
                 *       which is to be ignored.
                 *  ELSE advance the quote scanning pointer by ct
                 */
                if (ct == 0) {
                    p--;     /* move destination back one character */
                    pzQte++; /* skip over new-line character        */
                } else
                    pzQte += ct;

            } else {
                switch (*pzQte) {
                case '\\':
                case '\'':
                case '#':
                    p[-1] = *pzQte++;
                }
            }
            break;

        default:
            ;
        }
    }
    pzQte++; /* Return addr of char after the terminating quote */
    *p = NUL;

 NUL_termination:

    /*
     *  We found the end of the string before we found the end of the quote.
     */
    {
        tSCC zAnon[] = "* anonymous *";
        size_t alocSize = sizeof( tTemplate ) + sizeof( zAnon )
            + (mac_ct * sizeof( tMacro))
            + (pzQte - pzText) + strlen( pCurTemplate->pzFileName ) + 16;
        tTemplate* pNewT;
        alocSize &= ~0x0F;
        pNewT = (void*)AGALOC( alocSize );
        memset( (void*)pNewT, 0, alocSize );
        pNewT->descSize = alocSize;
        pNewT->macroCt  = mac_ct;
        pNewT->fd       = -1;
        strcpy( pNewT->zStartMac, pCurTemplate->zStartMac );
        strcpy( pNewT->zEndMac, pCurTemplate->zEndMac );
        loadMacros( pNewT, pCurTemplate->pzFileName, zAnon, pzText );
        pNewT = (tTemplate*)AGREALOC( (void*)pNewT, pNewT->descSize );
        pNewT = templateFixup( pNewT, pNewT->descSize );

        /*
         *  Replace the template text with a string containing its address.
         */
        sprintf( pzText, "[ 0x%08X ]", pNewT );
    }
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
        pzScan = (char*)skipNestedExpression( pzScan, strlen( pzScan ));
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

    /*
     *  Fill in the array of value assignments
     */
    for (;; pDL++ ) {
        pDL->de.pzName = pzScan;
        while (ISNAMECHAR( *pzScan ))  pzScan++;

        /*
         *  IF there is an immediate assignment character,
         *  THEN terminate the name
         *  ELSE ...
         */
        if (*pzScan == '=')
            *(pzScan++) = NUL;
        else {
            /*
             *  IF there is no value, then there is no value
             */
            if (*pzScan == NUL) {
                pDL->de.pzValue = "";
                break;
            }
            if (! isspace( *pzScan ))
                LOAD_ABORT( pT, pMac, "name not followed by '='" );

            *(pzScan++) = NUL;
            while (isspace( *pzScan )) pzScan++;

            /*
             *  The name was separated by space, but has no value
             */
            if (*pzScan != '=') {
                pDL->de.pzValue = "";
                if (*pzScan == NUL)
                    break;
                continue;
            }
            *(pzScan++) = NUL;  /* clobber the '=' */
        }
        while (isspace( *pzScan ))     pzScan++;
        pDL->pzExpr = pzScan;
        pDL->de.valType = VALTYP_TEXT;
        pzScan = (char*)skipNestedExpression( pzScan, strlen( pzScan ));

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
                char* pz = (char*)AGALOC( 24 );
                memcpy( (void*)pz, pDL->pzExpr, (pzScan - pDL->pzExpr) );
                pDL->pzExpr = pz;
            }
            spanTemplate( pDL->pzExpr, pDL );

            /*
             *  IF this is an anonymous template reference,
             *  THEN we must leave it as an expression!!
             */
            if (*pDL->pzExpr == '[')
                break;
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

    STATIC char*
anonymousTemplate( void* p, tDefEntry* pCurDef )
{
    tSCC zTFil[] = "/tmp/.AutoGenTemp";

    tTemplate*  pSavT = pCurTemplate;
    tMacro*     pCurM = pCurMacro;
    tTemplate*  pT    = (tTemplate*)p;
    tFpStack*   pfp;
    tFpStack*   psav  = pCurFp;

    long fpos;
    char*  res;

    pfp = (tFpStack*)AGALOC( sizeof( tFpStack ));
    pfp->pPrev = NULL;
    pfp->pzName = (char*)zTFil;
    pCurFp = pfp;

    unlink( zTFil );
    pfp->pFile = fopen( zTFil, "w+" FOPEN_BINARY_FLAG );
    if (pfp->pFile == (FILE*)NULL) {
        fprintf( stderr, "Error %d (%s) creating %s for read-write\n",
                 errno, strerror( errno ), zTFil );
        LOAD_ABORT( pSavT, pCurM, "Creating temp file" );
    }
    unlink( zTFil );

    if (OPT_VALUE_TRACE > TRACE_NOTHING) {
        fprintf( pfTrace, zTplInvoked, "anonymous", pCurM->sibIndex );
        if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
            fprintf( pfTrace, zFileLine, pSavT->pzFileName,
                     pCurM->lineNo );
    }

    generateBlock( pT, pT->aMacros, pT->aMacros + pT->macroCt, pCurDef );

    while (pCurFp != pfp)
        ag_scm_out_pop();

    fpos = ftell( pfp->pFile );
    res = (char*)AGALOC( fpos + 1 );
    rewind( pfp->pFile );
    fread( res, 1, fpos, pfp->pFile );
    fclose( pfp->pFile );
    pCurFp = psav;

    res[ fpos ] = NUL;

    pCurTemplate = pSavT;
    pCurMacro    = pCurM;
    return res;
}


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
 *      evaluated the same way simple expressions are evaluated.
 *      @xref{expression syntax}.
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

            case '[':
                if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS)
                    fprintf( pfTrace, "anonymus template arg %d\n",
                             pMac->sibIndex - defCt );

                pList->de.pzValue =
                    anonymousTemplate(
                        (void*)strtoul( pList->pzExpr + 2, (char**)NULL, 0 ),
                        pCurDef );
                break;

            case '(':
                /*
                 *  It is a scheme expression.  Accept only string
                 *  and number results.
                 */
                if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS) {
                    fprintf( pfTrace, "Scheme eval for arg %d:\n\t`%s'\n",
                             pMac->sibIndex - defCt, pList->pzExpr );
                }
                res = gh_eval_str( pList->pzExpr );

                if (gh_string_p( res )) {
                    pList->de.pzValue = strdup( SCM_CHARS( res ));
                }
                else if (gh_number_p( res )) {
                    pList->de.pzValue = (char*)AGALOC( 16 );
                    snprintf( pList->de.pzValue, 16, "%ld", gh_scm2long( res ));
                }
                else
                    pList->de.pzValue = strdup( "" );
                break;

            case '`':
                if (OPT_VALUE_TRACE >= TRACE_EXPRESSIONS) {
                    fprintf( pfTrace, "shell eval for arg %d:\n\t`%s'\n",
                             pMac->sibIndex - defCt, pList->pzExpr+1 );
                }
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
 *
 *  The string values may contain template macros that are parsed
 *  the first time the macro is processed and evaluated again every
 *  time the macro is evaluated.
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
 * what:   invoke AutoGen macro
 * exparg: macro, name of macro to invoke
 * exparg: args,  macro arguments, optional, ellipsis
 *
 * doc:  Invoke an AutoGen macro and put the results into a string.
 *       As of this writing, however, the macro arguments are not
 *       well implemented.  Each entry of the list must be a string
 *       that starts with a name, followed by an equal sign followed
 *       by the text value.  This is not very Schemey.  Don't rely on this.
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
    strcpy( pNewT->zStartMac, pT->zStartMac );
    strcpy( pNewT->zEndMac, pT->zEndMac );

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
