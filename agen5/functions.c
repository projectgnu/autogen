
/*
 *  $Id: functions.c,v 1.11 2000/03/01 04:35:12 bruce Exp $
 *
 *  This module implements text functions.
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

#include <time.h>
#include <utime.h>

#include "autogen.h"

#include <streqv.h>
#include <guile/gh.h>


tSCC zCantInc[] = "cannot include file";


/*=macfunc INCLUDE
 *
 *  what:   Read in and emit a template block
 *  handler_proc:
 *  load_proc:     Expr
 *
 *  desc:
 *
 *  The entire contents of the named file is inserted at this point.
 *  The contents of the file are processed for macro expansion.  The
 *  arguments are eval-ed, so you may compute the name of the file to
 *  be included.  The included file must not contain any incomplete
 *  function blocks.  Function blocks are template text beginning with
 *  any of the macro functions @samp{IF}, @samp{FOR}, @samp{WHILE}, and
 *  @samp{CASE} and extending through the respective terminating
 *  macro functions.
=*/
MAKE_HANDLER_PROC( Include )
{
    tTemplate* pNewTpl;
    ag_bool needFree;
    char* pz = evalExpression( &needFree );

    if (*pz != NUL) {
        pNewTpl = loadTemplate( pz );

        if (OPT_VALUE_TRACE > TRACE_NOTHING) {
            tSCC zTplFmt[] = "Template %s included\n";
            tSCC zLinFmt[] = "\tfrom %s line %d\n";
            fprintf( pfTrace, zTplFmt, pNewTpl->pzFileName );
            if (OPT_VALUE_TRACE < TRACE_EVERYTHING)
                fprintf( pfTrace, zLinFmt, pCurTemplate->pzFileName,
                         pMac->lineNo );
        }

        generateBlock( pNewTpl, pNewTpl->aMacros,
                       pNewTpl->aMacros + pNewTpl->macroCt,
                       pCurDef );
        unloadTemplate( pNewTpl );
        pCurTemplate = pT;
    }

    if (needFree)
        AGFREE( (void*)pz );

    return pMac + 1;
}


/*=macfunc UNKNOWN
 *
 *  what:  Either a user macro or a value name.
 *  handler_proc:
 *  load_proc:
 *  unnamed:
 *
 *  desc:
 *  The macro text has started with a name not known to AutoGen.
 *  If, at template instantiation time, it turns out to be the
 *  name of a template block, then the macro invokes that template.
 *  If it is not, then it is a conditional expression that is
 *  evaluated only if the name is defined at the time the macro
 *  is invoked.
 *
 *  You may not specify @code{UNKNOWN} specifically.
=*/
MAKE_HANDLER_PROC( Unknown )
{
    tTemplate* pInv = findTemplate( pT->pzTemplText + pMac->ozName );
    if (pInv != (tTemplate*)NULL) {
        pMac->funcCode    = FTYP_DEFINE;
        pMac->funcPrivate = (void*)pInv;
        parseMacroArgs( pT, pMac );
        return mFunc_Define( pT, pMac, pCurDef );
    }

    pMac->funcCode = FTYP_EXPR;
    if (pMac->ozText == 0)
        pMac->res = EMIT_VALUE;

    else {
        char* pzExpr = pT->pzTemplText + pMac->ozText;
        switch (*pzExpr) {
        case ';':
        case '(':
            pMac->res = EMIT_EXPRESSION;
            break;

        case '`':
            pMac->res = EMIT_SHELL;
            spanQuote( pzExpr );
            break;

        case '"':
        case '\'':
            spanQuote( pzExpr );
            /* FALLTHROUGH */

        default:
            pMac->res = EMIT_STRING;
        }
    }

    return mFunc_Expr( pT, pMac, pCurDef );
}


/*=macfunc BOGUS
 *
 *  what:  Out-of-context or unknown functions are bogus.
 *  handler_proc:
 *  unnamed:
 *  load_proc:
=*/
MAKE_HANDLER_PROC( Bogus )
{
    tSCC z[] = "%d (%s) is an unknown macro function, or has no handler";
    char* pz = asprintf( z, pMac->funcCode, (pMac->funcCode < FUNC_CT)
                         ? apzFuncNames[ pMac->funcCode ] : "??" );
    fprintf( stderr, zTplErr, pT->pzFileName, pMac->lineNo, pz );
    longjmp( fileAbort, FAILURE );
    return pMac;
}



/*=macfunc TEXT
 *
 *  what:  A block of text to be emitted.
 *  handler_proc:
 *  unnamed:
=*/
MAKE_HANDLER_PROC( Text )
{
    fputs( pT->pzTemplText + pMac->ozText, pCurFp->pFile );
    fflush( pCurFp->pFile );
    return pMac + 1;
}

#endif /* DEFINE_LOAD_FUNCTIONS defined */

/*=macfunc COMMENT
 *
 *  what:  A block of comment to be ignored
 *  alias:  #
 *  load_proc:
 *
 *  situational:
 *    This function can be specified by the user, but there will
 *    never be a situation where it will be invoked at emit time.
 *    The macro is actually removed from the internal representation.
 *
 *  desc:
 *    If the specified function code is @code{#}, then the
 *    entire macro function is treated as a comment and ignored.
=*/
MAKE_LOAD_PROC( Comment )
{
    memset( (void*)pMac, 0, sizeof( *pMac ));
    return pMac;
}


/*
 *  mLoad_Unknown  --  the default (unknown) load function
 *
 *  Move any text into the text offset field.
 *  This is used as the default load mechanism.
 */
MAKE_LOAD_PROC( Unknown )
{
    char*          pzCopy = pT->pNext;
    const char*    pzSrc;
    size_t         srcLen = (size_t)pMac->res;         /* macro len  */

    if (srcLen <= 0)
        goto return_emtpy_expression;

    pzSrc = (const char*)pMac->ozText; /* macro text */

    switch (*pzSrc) {
    case ';':
        /*
         *  Strip off scheme comments
         */
        do  {
            while (--srcLen, (*++pzSrc != '\n')) {
                if (srcLen <= 0)
                    goto return_emtpy_expression;
            }

            while (--srcLen, isspace( *++pzSrc )) {
                if (srcLen <= 0)
                    goto return_emtpy_expression;
            }
        } while (*pzSrc == ';');
        break;

    case '[':
        pzCopy = pT->pzTemplText + pMac->ozName;
        pzCopy += strlen( pzCopy );
        srcLen -= copyDefReference( pT, pMac, &pzCopy, &pzSrc, srcLen );
        pT->pNext = pzCopy+1;
        if (srcLen <= 0)
            goto return_emtpy_expression;
        break;
    }

    /*
     *  Copy the expression
     */
    pzCopy = pT->pNext; /* next text dest   */
    pMac->ozText = (pzCopy - pT->pzTemplText);
    pMac->res    = 0;

    do  {
        *(pzCopy++) = *(pzSrc++);
    } while (--srcLen > 0);
    *(pzCopy++) = '\0';
    *(pzCopy++) = '\0'; /* double terminate */

    pT->pNext = pzCopy;

    return pMac + 1;

 return_emtpy_expression:
    pMac->ozText = 0;
    pMac->res    = 0;
    return pMac + 1;
}


/*
 *  mLoad_Bogus
 *
 *  Some functions are known to AutoGen, but invalid out of context.
 *  For example, ELIF, ELSE and ENDIF are all known to AutoGen.
 *  However, the load function pointer for those functions points
 *  here, until an "IF" function is encountered.
 */
MAKE_LOAD_PROC( Bogus )
{
    const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
    long           srcLen = (long)pMac->res;           /* macro len  */
    char z[ 128 ];
    sprintf( z, "%s function (%d) out of context",
             apzFuncNames[ pMac->funcCode ], pMac->funcCode );
    fprintf( stderr, zTplErr, pT->pzFileName,
             pMac->lineNo, z );

    if (srcLen > 0) {
        if (srcLen > 64)
            srcLen = 64;
        do  {
            fputc( *(pzSrc++), stderr );
        } while (--srcLen > 0);
        fputc( '\n', stderr );
    }

    LOAD_ABORT( pT, pMac, "bad context" );
    /* NOTREACHED */
    return (tMacro*)NULL;
}
/* end of functions.c */
