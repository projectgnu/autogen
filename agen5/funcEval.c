
/*
 *  $Id: funcEval.c,v 1.25 2000/09/28 03:12:27 bkorb Exp $
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
#include "expGuile.h"
#include "expr.h"

#include <compat/compat.h>
#ifdef WITH_INCLUDED_REGEX
#  include "compat/gnu-regex.h"
#else
#  include <regex.h>
#endif

tSCC        zNil[] = "";

STATIC int exprType( char* pz );


    EXPORT char*
resolveSCM( SCM s, ag_bool*  pMustFree )
{
    static char z[32];
    char*  pzRes = z;
    int    len;

    switch (gh_type_e( s )) {
    case GH_TYPE_BOOLEAN:         
        strcpy( z, SCM_NFALSEP(s) ? "1" : "0" ); break;

    case GH_TYPE_STRING:
    case GH_TYPE_SYMBOL:
        len = SCM_LENGTH(s);
        if (len >= sizeof( z )) {
            pzRes = AGALOC( len + 1, "SCM string extraction" );
            *pMustFree = AG_TRUE;
        }

        memcpy( pzRes, SCM_CHARS(s), len );
        pzRes[ len ] = NUL;
        break;

    case GH_TYPE_CHAR:
        z[0] = gh_scm2char(s); z[1] = NUL; break;

    case GH_TYPE_VECTOR:
        pzRes = "** Vector **"; break;

    case GH_TYPE_PAIR:
        pzRes = "** Pair **"; break;

    case GH_TYPE_NUMBER:
        snprintf( z, sizeof(z), "%ld", gh_scm2long(s) ); break;

    case GH_TYPE_PROCEDURE:
#ifdef SCM_SUBR_ENTRY
        snprintf( z, sizeof(z), "** Procedure 0x%08X **", SCM_SUBR_ENTRY(s) );
#else
        pzRes = "** PROCEDURE **";
#endif
        break;

    case GH_TYPE_LIST:
        pzRes = "** LIST **"; break;

    case GH_TYPE_INEXACT:
        pzRes = "** INEXACT **"; break;

    case GH_TYPE_EXACT:
        pzRes = "** EXACT **"; break;

    case GH_TYPE_UNDEFINED:
        pzRes = (char*)zNil; break;

    default:
        pzRes = "** UNKNOWN **"; break;
    }

    return pzRes;
}


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
    EXPORT char*
evalExpression( ag_bool* pMustFree )
{
    tTemplate*  pT      = pCurTemplate;
    tMacro*     pMac    = pCurMacro;
    tDefEntry*  pCurDef = pDefContext;
    ag_bool     isIndexed;
    tDefEntry*  pDef;
    int         code = pMac->res;
    char*       pzText;

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
            code   = ((code & EMIT_SECONDARY_TYPE) >> EMIT_SECONDARY_SHIFT);
        }

        /*
         *  OTHERWISE, we found an entry.  Make sure we were supposed to.
         */
        else {
            tSCC zBlock[] = "attempted to use block macro in eval expression";

            if ((code & EMIT_IF_ABSENT) != 0)
                return (char*)zNil;

            if (  (pDef->valType != VALTYP_TEXT)
               && ((code & EMIT_PRIMARY_TYPE) == EMIT_VALUE)  ) {
                fprintf( stderr, zTplWarn, pT->pzFileName, pMac->lineNo,
                         zBlock );
                return (char*)zNil;
            }

            /*
             *  Compute the expression string.  There are three possibilities:
             *  1.  There is an expression string in the macro, but it must
             *      be formatted with the text value.
             *      Make sure we have a value.
             *  2.  There is an expression string in the macro, but it is *NOT*
             *      to be formatted.  Use it as is.  Do *NOT* verify that
             *      the define value is text.
             *  3.  There is no expression with the macro invocation.
             *      The define value *must* be text.
             */
            if ((code & EMIT_FORMATTED) != 0) {
                /*
                 *  And make sure what we found is a text value
                 */
                if (pDef->valType != VALTYP_TEXT) {
                    fprintf( stderr, zTplWarn, pT->pzFileName, pMac->lineNo,
                             zBlock );
                    return (char*)zNil;
                }

                *pMustFree = AG_TRUE;
                pzText = asprintf( pT->pzTemplText + pMac->ozText,
                                   pDef->pzValue );
            }

            else if (pMac->ozText != 0)
                 pzText = pT->pzTemplText + pMac->ozText;

            else {
                /*
                 *  And make sure what we found is a text value
                 */
                if (pDef->valType != VALTYP_TEXT) {
                    fprintf( stderr, zTplWarn, pT->pzFileName, pMac->lineNo,
                             zBlock );
                    return (char*)zNil;
                }

                pzText = pDef->pzValue;
            }

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
        SCM res = gh_eval_str( pzText );

        if (*pMustFree) {
            AGFREE( (void*)pzText );
            *pMustFree = AG_FALSE;
        }

        pzText = resolveSCM( res, pMustFree );
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
 * what: display of file & line
 * doc:  This function is only invoked just before Guile displays
 *       an error message.  It displays the file name and line number
 *       that triggered the evaluation error.  You should not need to
 *       invoke this routine directly.  Guile will do it automatically.
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
    EXPORT SCM
eval( const char* pzExpr )
{
    ag_bool allocated = AG_FALSE;
    char*   pzTemp;
    SCM   res;

    switch (*pzExpr) {
    case '(':
    case ';':
        res = gh_eval_str( (char*)pzExpr );
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


    STATIC int
exprType( char* pz )
{
    switch (*pz) {
    case ';':
    case '(':
        return EMIT_EXPRESSION;

    case '`':
        spanQuote( pz );
        return EMIT_SHELL;

    case '"':
    case '\'':
        spanQuote( pz );
        /* FALLTHROUGH */

    default:
        return EMIT_STRING;
    }
}


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
    pMac->ozName = (pzCopy - pT->pzTemplText);
    {
        size_t remLen = cannonicalizeName( pzCopy, pzSrc, srcLen );
        if (remLen > srcLen)
            LOAD_ABORT( pT, pMac, "Invalid definition name" );
        pzSrc  += srcLen - remLen;
        srcLen  = remLen;
        pzCopy += strlen( pzCopy )+1;
    }

    if (pzSrc >= pzSrcEnd) {
        if (pMac->res != EMIT_VALUE) {
            tSCC zAb[] = "No replacement text for unfound value";
            tSCC zFm[] = "No formatting string for format expr";
            fprintf( stderr, zTplErr, pT->pzFileName, pMac->lineNo,
                     (pMac->res == EMIT_IF_ABSENT) ? zAb : zFm );
            LOAD_ABORT( pT, pMac, "definition expression" );
        }
        pMac->ozText = 0;

    } else {
        char* pz = pzCopy;
        long  ct = srcLen = (long)(pzSrcEnd - pzSrc);

        pMac->ozText = (pzCopy - pT->pzTemplText);
        /*
         *  Copy the expression
         */
        do { *(pzCopy++) = *(pzSrc++); } while (--ct > 0);
        *(pzCopy++) = NUL; *(pzCopy++) = NUL; /* double terminate */

        /*
         *  IF this expression has an "if-present" and "if-not-present"
         *  THEN find the ending expression...
         */
        if ((pMac->res & EMIT_ALWAYS) != 0) {
            char* pzNextExpr = (char*)skipExpression( pz, srcLen );

            /*
             *  The next expression must be within bounds and space separated
             */
            if (pzNextExpr >= pz + srcLen)
                LOAD_ABORT( pT, pMac, "`?' needs two expressions" );

            if (! isspace( *pzNextExpr ))
                LOAD_ABORT( pT, pMac, "No space between expressions" );

            /*
             *  NUL terminate the first expression, skip intervening
             *  white space and put the secondary expression's type
             *  into the macro type code as the "secondary type".
             */
            *(pzNextExpr++) = NUL;
            while (isspace( *pzNextExpr ))  pzNextExpr++;
            pMac->res |= (exprType( pzNextExpr ) << EMIT_SECONDARY_SHIFT);
            pMac->endIndex = pzNextExpr - pT->pzTemplText;
        }

        pMac->res |= exprType( pz );
    }

    pT->pNext = pzCopy;
    return pMac + 1;
}
/* end of funcEval.c */
