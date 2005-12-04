
/*
 *  $Id: funcEval.c,v 4.6 2005/12/04 22:18:41 bkorb Exp $
 *
 *  This module evaluates macro expressions.
 */

/*
 *  AutoGen copyright 1992-2005 Bruce Korb
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
 *             51 Franklin Street, Fifth Floor,
 *             Boston, MA  02110-1301, USA.
 */

static int exprType( char* pz );

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static int
exprType( char* pz );
/* = = = END-STATIC-FORWARD = = = */

LOCAL char*
resolveSCM( SCM s )
{
    static char z[48];
    char*  pzRes = z;

    switch (gh_type_e( s )) {
    case GH_TYPE_BOOLEAN:         
        z[0] = SCM_NFALSEP(s) ? '1' : '0'; z[1] = NUL;
        break;

    case GH_TYPE_STRING:
    case GH_TYPE_SYMBOL:
        pzRes = ag_scm2zchars( s, "SCM Result" );
        break;

    case GH_TYPE_CHAR:
        z[0] = gh_scm2char(s); z[1] = NUL; break;

    case GH_TYPE_VECTOR:
        pzRes = "** Vector **"; break;

    case GH_TYPE_PAIR:
        pzRes = "** Pair **"; break;

    case GH_TYPE_NUMBER:
        snprintf( z, sizeof(z), "%ld", gh_scm2ulong(s) ); break;

    case GH_TYPE_PROCEDURE:
#ifdef SCM_SUBR_ENTRY
    {
        void* x = &SCM_SUBR_ENTRY(s);

        snprintf( z, sizeof(z), "** Procedure 0x%08lX **",
                  (unsigned long)x );
        break;
    }
#else
        pzRes = "** PROCEDURE **";
        break;
#endif

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
LOCAL char*
evalExpression( ag_bool* pMustFree )
{
    tTemplate*  pT      = pCurTemplate;
    tMacro*     pMac    = pCurMacro;
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
        pDef = findDefEntry( pT->pzTemplText + pMac->ozName, &isIndexed );

        if (pDef == NULL) {
            switch (code & (EMIT_IF_ABSENT | EMIT_ALWAYS)) {
            case EMIT_IF_ABSENT:
                /*
                 *  There is only one expression.  It applies because
                 *  we did not find a definition.
                 */
                pzText = pT->pzTemplText + pMac->ozText;
                code &= EMIT_PRIMARY_TYPE;
                break;

            case EMIT_ALWAYS:
                /*
                 *  There are two expressions.  Take the second one.
                 */
                pzText =  pT->pzTemplText + pMac->endIndex;
                code   = ((code & EMIT_SECONDARY_TYPE)
                          >> EMIT_SECONDARY_SHIFT);
                break;

            case 0:
                /*
                 *  Emit only if found
                 */
                return (char*)zDefaultNil;

            case (EMIT_IF_ABSENT | EMIT_ALWAYS):
                /*
                 *  Emit inconsistently :-}
                 */
                AG_ABEND_IN( pT, pMac, "PROGRAM ERROR:  ambiguous expr code" );
                /* NOTREACHED */
            }
        }

        /*
         *  OTHERWISE, we found an entry.  Make sure we were supposed to.
         */
        else {
            tSCC zBlock[] = "attempted to use block macro in eval expression";

            if ((code & EMIT_IF_ABSENT) != 0)
                return (char*)zDefaultNil;

            if (  (pDef->valType != VALTYP_TEXT)
               && ((code & EMIT_PRIMARY_TYPE) == EMIT_VALUE)  ) {
                fprintf( pfTrace, zTplWarn, pT->pzFileName, pMac->lineNo,
                         zBlock );
                return (char*)zDefaultNil;
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
                    fprintf( pfTrace, zTplWarn, pT->pzFileName, pMac->lineNo,
                             zBlock );
                    return (char*)zDefaultNil;
                }

                *pMustFree = AG_TRUE;
                pzText = aprf(pT->pzTemplText + pMac->ozText, pDef->val.pzText);
            }

            else if (pMac->ozText != 0)
                 pzText = pT->pzTemplText + pMac->ozText;

            else {
                /*
                 *  And make sure what we found is a text value
                 */
                if (pDef->valType != VALTYP_TEXT) {
                    fprintf( pfTrace, zTplWarn, pT->pzFileName, pMac->lineNo,
                             zBlock );
                    return (char*)zDefaultNil;
                }

                pzText = pDef->val.pzText;
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

        pzText = pDef->val.pzText;
        break;

    case EMIT_EXPRESSION:
    {
        SCM res = ag_eval( pzText );

        if (*pMustFree) {
            AGFREE( (void*)pzText );
            *pMustFree = AG_FALSE;
        }

        pzText = resolveSCM( res );
        break;
    }

    case EMIT_SHELL:
    {
        char* pz = runShell( pzText );

        if (*pMustFree)
            AGFREE( (void*)pzText );

        if (pz != NULL) {
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
 * general_use:
 * doc:  This function is only invoked just before Guile displays
 *       an error message.  It displays the file name and line number
 *       that triggered the evaluation error.  You should not need to
 *       invoke this routine directly.  Guile will do it automatically.
=*/
SCM 
ag_scm_error_source_line( void )
{
    tSCC zErr[] = "\nGuile/Scheme evaluation error in %s line %d:  %s\n";
    fprintf( stderr, zErr, pCurTemplate->pzTplName, pCurMacro->lineNo,
             pCurTemplate->pzTemplText + pCurMacro->ozText );
    fflush( stderr );
    guileFailure = 1;

    return SCM_UNDEFINED;
}


/*=gfunc emit
 *
 * what: emit the text for each argument
 * general_use:
 *
 * exparg: alist, list of arguments to stringify and emit, , list
 *
 * doc:  Walk the tree of arguments, displaying the values of displayable
 *       SCM types.
=*/
SCM 
ag_scm_emit( SCM val )
{
    for (;;) {
        if (val == SCM_UNDEFINED)
            break;

        if (SCM_NULLP( val ))
            break;

        if (AG_SCM_STRING_P( val )) {
            fputs( (char*)ag_scm2zchars( val, "emit val" ), pCurFp->pFile );
            fflush( pCurFp->pFile );
            break;
        }

        switch (gh_type_e( val )) {
        case GH_TYPE_LIST:
        case GH_TYPE_PAIR:
            ag_scm_emit( SCM_CAR( val ));
            val = SCM_CDR( val );
            continue;

        default:
            fputs( resolveSCM( val ), pCurFp->pFile );
            fflush( pCurFp->pFile );
            break;
        }

        break;
    }

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
LOCAL SCM
eval( const char* pzExpr )
{
    ag_bool allocated = AG_FALSE;
    char*   pzTemp;
    SCM   res;

    switch (*pzExpr) {
    case '(':
    case ';':
        res = ag_eval( (char*)pzExpr );
        break;

    case '`':
        AGDUPSTR( pzTemp, pzExpr, "shell script" );
        (void)spanQuote( pzTemp );
        pzExpr = runShell( pzTemp );
        AGFREE( (void*)pzTemp );
        res = AG_SCM_STR02SCM( (char*)pzExpr );
        AGFREE( (void*)pzExpr );
        break;

    case '"':
    case '\'':
        AGDUPSTR( pzTemp, pzExpr, "quoted string" );
        (void)spanQuote( pzTemp );
        allocated = AG_TRUE;
        pzExpr = pzTemp;
        /* FALLTHROUGH */

    default:
        res = AG_SCM_STR02SCM( (char*)pzExpr );
        if (allocated)
            AGFREE( (void*)pzExpr );
    }

    return res;
}


/*=macfunc EXPR
 *
 *  what:  Evaluate and emit an Expression
 *  alias: + - + ? + % + ; + ( + '`' + '"' + "'" + . +
 *
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *   This macro does not have a name to cause it to be invoked
 *   explicitly, though if a macro starts with one of the apply codes
 *   or one of the simple expression markers, then an expression
 *   macro is inferred.  The result of the expression evaluation
 *   (@pxref{expression syntax}) is written to the current output.
=*/
tMacro*
mFunc_Expr( tTemplate* pT, tMacro* pMac )
{
    ag_bool needFree;
    char* pz = evalExpression( &needFree );

    fputs( pz, pCurFp->pFile );
    fflush( pCurFp->pFile );
    if (needFree)
        AGFREE( (void*)pz );

    return pMac + 1;
}


static int
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
tMacro*
mLoad_Expr( tTemplate* pT, tMacro* pMac, tCC** ppzScan )
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
        size_t remLen = canonicalizeName( pzCopy, pzSrc, srcLen );
        if (remLen > srcLen)
            AG_ABEND_IN( pT, pMac, "Invalid definition name" );
        pzSrc  += srcLen - remLen;
        srcLen  = remLen;
        pzCopy += strlen( pzCopy )+1;
    }

    if (pzSrc >= pzSrcEnd) {
        if (pMac->res != EMIT_VALUE)
            AG_ABEND_IN( pT, pMac, "No text for unfound value" );

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
                AG_ABEND_IN( pT, pMac, "`?' needs two expressions" );

            if (! isspace( *pzNextExpr ))
                AG_ABEND_IN( pT, pMac, "No space between expressions" );

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
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/funcEval.c */
