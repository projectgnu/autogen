
/*
 *  $Id: funcEval.c,v 1.12 1999/11/11 04:49:05 bruce Exp $
 *
 *  This module evaluates macro expressions.
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

#include <compat/compat.h>
#ifdef WITH_INCLUDED_REGEX
#  include "compat/gnu-regex.h"
#else
#  include <regex.h>
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   Exported function procedures
 *
 *  evalExpression
 *
 *  Evaluate an expression and return a string pointer.  Always.
 *  It may need to be deallocated, so a boolean pointer is used
 *  to tell the caller.
 */
    char*
evalExpression( ag_bool* pMustFree )
{
    tTemplate*  pT      = pCurTemplate;
    tMacro*     pMac    = pCurMacro;
    tDefEntry*  pCurDef = pDefContext;
    ag_bool     isIndexed;
    tDefEntry*  pDef;
    int         code = pMac->res;
    char*       pzText;
    tSCC        zNil[] = "";

    *pMustFree = AG_FALSE;

    if ((code & EMIT_NO_DEFINE) != 0) {
        pzText = pT->pzTemplText + pMac->ozText;
        code &= EMIT_PRIMARY_TYPE;
    }

    else {
        /*
         *  Get the named definition entry, maybe
         */
        pDef = findDefEntry( pT->pzTemplText + pMac->ozName,
                             pCurDef, &isIndexed );

        if (pDef == (tDefEntry*)NULL) {
            /*
             *  We do not have a definition.  Check to see if we _must_
             *  have a definition.
             */
            if ((code & (EMIT_IF_ABSENT | EMIT_ALWAYS)) == 0)
                return (char*)zNil;
            pzText =  pT->pzTemplText + pMac->endIndex;
            code = (code & EMIT_SECONDARY_TYPE) >> 4;
        }

        /*
         *  OTHERWISE, we found an entry.  Make sure we were supposed to.
         */
        else {
            if ((code & EMIT_IF_ABSENT) != 0)
                return (char*)zNil;

            /*
             *  And make sure what we found is a text value
             */
            if (pDef->valType != VALTYP_TEXT) {
                fprintf( stderr, zTplWarn, pT->pzFileName, pMac->lineNo,
                         "attempted to use block macro in eval expression" );
                return (char*)zNil;
            }

            /*
             *  IF this is a formatting macro,
             *  THEN use the value as the argument to the formatting string
             *  ELIF there is text associated with this macro
             *  THEN that text is the value of the expression
             *  OTHERWISE the value associated with the name becomes the
             *            expression.
             */
            if ((code & EMIT_FORMATTED) != 0){
                *pMustFree = AG_TRUE;
                pzText = asprintf( pT->pzTemplText + pMac->ozText,
                                   pDef->pzValue );
            } else if (pMac->ozText != 0)
                 pzText = pT->pzTemplText + pMac->ozText;
            else pzText = pDef->pzValue;
            code &= EMIT_PRIMARY_TYPE;
        }
    }

    /*
     *  The "code" tells us how to handle the expression
     */
    switch (code) {
    case EMIT_VALUE:
        if (*pMustFree) {
            AGFREE( (void*)pzText );
            *pMustFree = AG_FALSE;
        }

        pzText = pDef->pzValue;
        break;

    case EMIT_EXPRESSION:
    {
        static char z[ 32 ];

        SCM res = gh_eval_str( pzText );

        if (*pMustFree) {
            AGFREE( (void*)pzText );
            *pMustFree = AG_FALSE;
        }

        if (gh_string_p( res ))
            pzText = SCM_CHARS( res );

        else if (gh_char_p( res )) {
            z[0] = gh_scm2char( res );
            z[1] = NUL;
            pzText = z;
        }

        else if (gh_number_p( res )) {
            sprintf( z, "%ld", gh_scm2long( res ));
            pzText = z;
        }

        else if (gh_boolean_p( res )) {
            z[0] = SCM_NFALSEP( res ) ? '1' : '0';
            z[1] = NUL;
            pzText = z;
        }
        else
            pzText = (char*)zNil;

        break;
    }

    case EMIT_SHELL:
    {
        char* pz = runShell( pzText );

        if (*pMustFree)
            AGFREE( (void*)pzText );

        if (pz != (char*)NULL) {
            *pMustFree = AG_TRUE;
            pzText = pz;
        }
        else {
            *pMustFree = AG_FALSE;
            pzText = (char*)zNil;
        }
        break;
    }

    case EMIT_STRING:
        break;
    }

    return pzText;
}


/*=gfunc error_source_line
 *
 * what: display current file/line before SCM error
 * doc: This function should only be invoked just before
 *      Guile displays an error message.  It displays the
 *      file name and line number that triggered the evaluation error.
=*/
    SCM 
ag_scm_error_source_line( void )
{
    SCM port = scm_current_error_port();

    fprintf( stderr, "\nin %s line %d:  %s\n", pCurTemplate->pzTplName,
             pCurMacro->lineNo,
             pCurTemplate->pzTemplText + pCurMacro->ozText );

    return SCM_UNDEFINED;
}

/*
 *  eval
 *
 *  The global evaluation function.
 *  The string to "evaluate" may be a literal string, or may
 *  need Scheme interpretation.  So, we do one of three things:
 *  if the string starts with a Scheme comment character or
 *  evaluation character (';' or '('), then run a Scheme eval.
 *  If it starts with a quote character ('\'' or '"'), then
 *  digest the string and return that.  Otherwise, just return
 *  the string.
 */
    SCM
eval( const char* pzExpr )
{
    ag_bool allocated = AG_FALSE;
    char*   pzTemp;
    SCM   res;

    switch (*pzExpr) {
    case '(':
    case ';':
        res = gh_eval_str( pzExpr );
        break;

    case '`':
        AGDUPSTR( pzTemp, pzExpr );
        (void)spanQuote( pzTemp );
        pzExpr = runShell( pzTemp );
        AGFREE( (void*)pzTemp );
        res = gh_str02scm( (char*)pzExpr );
        AGFREE( (void*)pzExpr );
        break;

    case '"':
    case '\'':
        AGDUPSTR( pzTemp, pzExpr );
        (void)spanQuote( pzTemp );
        allocated = AG_TRUE;
        pzExpr = pzTemp;
        /* FALLTHROUGH */

    default:
        res = gh_str02scm( (char*)pzExpr );
        if (allocated)
            AGFREE( (void*)pzExpr );
    }

    return res;
}


/*=macfunc EXPR
 *
 *  what:  Evaluate and emit an Expression
 *  alias:  -
 *  alias:  ?
 *  alias:  %
 *
 *  alias:  ;
 *  alias:  (
 *  alias:  `
 *
 *  alias:  '"'
 *  alias:  "'"
 *
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *   This macro does not have a name to cause it to be invoked
 *   explicitly, though if a macro starts with one of the apply codes
 *   or one of the simple expression markers, then an expression
 *   macro is inferred.  The result of the expression evaluation
 *   (@xref{expression syntax}) is written to the current output.
=*/
MAKE_HANDLER_PROC( Expr )
{
    ag_bool needFree;
    char* pz = evalExpression( &needFree );

    fputs( pz, pCurFp->pFile );
    if (needFree)
        AGFREE( (void*)pz );

    return pMac + 1;
}
#endif /* DEFINE_LOAD_FUNCTIONS */


/*
 *  mLoad_Expression
 */
MAKE_LOAD_PROC( Expr )
{
    char*    pzCopy; /* next text dest   */
    tCC*     pzSrc  = (const char*)pMac->ozText; /* macro text */
    long     srcLen = (long)pMac->res;           /* macro len  */
    tCC*     pzSrcEnd = pzSrc + srcLen;
    tMacro*  pNextMac;

    switch (*pzSrc) {
    case '-':
        pMac->res = EMIT_IF_ABSENT;
        pzSrc++;
        break;

    case '?':
        pMac->res = EMIT_ALWAYS;
        pzSrc++;
        if (*pzSrc == '%') {
            pMac->res |= EMIT_FORMATTED;
            pzSrc++;
        }
        break;

    case '%':
        pMac->res = EMIT_FORMATTED;
        pzSrc++;
        break;

    case '`':
        pNextMac  = mLoad_Unknown( pT, pMac, ppzScan );
        pMac->res = EMIT_NO_DEFINE | EMIT_SHELL;
        spanQuote( pT->pzTemplText + pMac->ozText );
        return pNextMac;

    case '"':
    case '\'':
        pNextMac  = mLoad_Unknown( pT, pMac, ppzScan );
        pMac->res = EMIT_NO_DEFINE | EMIT_STRING;
        spanQuote( pT->pzTemplText + pMac->ozText );
        return pNextMac;

    case '(':
    case ';':
        pNextMac  = mLoad_Unknown( pT, pMac, ppzScan );
        pMac->res = EMIT_NO_DEFINE | EMIT_EXPRESSION;
        return pNextMac;

    default:
        pMac->res = EMIT_VALUE; /* zero */
    }

    pzCopy = pT->pNext;

    while (isspace( *pzSrc ))  pzSrc++;
    if (! isalpha( *pzSrc )) {
        tSCC zMsg[] = "Conditional expression must start with a name";
        LOAD_ABORT( pT, pMac, zMsg );
    }
    pMac->ozName = (pzCopy - pT->pzTemplText);
    while (ISNAMECHAR( *pzSrc ))  *(pzCopy++) = *(pzSrc++);
    *(pzCopy++) = NUL;
    while (isspace( *pzSrc ))  pzSrc++;
    srcLen -= (size_t)(pzSrc - (const char*)pMac->ozText);
    if (srcLen <= 0) {
        if (pMac->res != EMIT_VALUE) {
            tSCC zAb[] = "No replacement text for unfound value";
            tSCC zFm[] = "No formatting string for format expr";
            fprintf( stderr, zTplErr, pT->pzFileName, pMac->lineNo,
                     (pMac->res == EMIT_IF_ABSENT) ? zAb : zFm );
        }
        pMac->ozText = 0;
    } else {
        char* pz = pzCopy;
        int   ct = srcLen;

        pMac->ozText = (pzCopy - pT->pzTemplText);
        /*
         *  Copy the expression
         */
        do  {
            *(pzCopy++) = *(pzSrc++);
        } while (--ct > 0);
        *(pzCopy++) = '\0';
        *(pzCopy++) = '\0'; /* double terminate */

        if ((pMac->res & EMIT_ALWAYS) != 0) {
            char* pzNextExpr = (char*)skipExpression( pz, srcLen );
            if (pzNextExpr < pz + srcLen) {
                if (! isspace( *pzNextExpr ))
                    LOAD_ABORT( pT, pMac, "No space between expressions" );

                *(pzNextExpr++) = NUL;
                while (isspace( *pzNextExpr ))  pzNextExpr++;
                pMac->endIndex = pzNextExpr - pT->pzTemplText;
            }
        }

        switch (*pz) {
        case ';':
        case '(':
            pMac->res |= EMIT_EXPRESSION;
            break;

        case '`':
            pMac->res |= EMIT_SHELL;
            spanQuote( pz );
            break;

        case '"':
        case '\'':
            spanQuote( pz );
            pMac->res |= EMIT_STRING;
        }
    }

    pT->pNext = pzCopy;
    return pMac + 1;
}
/* end of funcEval.c */
