
/*
 *  $Id: functions.c,v 1.1 1999/10/14 00:33:53 bruce Exp $
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   Exported function procedures
 */
/*=macfunc ERROR
 *
 *  what:      Cease processing
 *  cindex:    error handling
 *  handler_proc:  Standard load.
 *
 *  desc:
 *  The arguments are evaluated and printed to the stderr,
 *  the current output file is removed and AutoGen exits
 *  with the EXIT_FAILURE error code.  IF, however, the argument
 *  is the number 0 (zero), then the output file is silently
 *  removed and processing continues with the next suffix.
 *
 *  @noindent
 *  For example, the following will print the quoted error
 *  message and AutoGen will exit:
 *
 *  @example
 *  [# ERROR "You did not have enough values to continue" #]
 *  @end example
=*/
MAKE_HANDLER_PROC( Error )
{
    tSCC    zErr[] = "DEFINITIONS ERROR for %s:  %s\n";
    tSCC    zEmptyError[] = "** EMPTY ERROR MESSAGE **";

    SCM   res = eval( pT, pMac, pCurDef );

    if (! gh_string_p( res )) {
        tSCC zNotStr[] = "\texpression yields non-string:  %s\n";

        if (gh_number_p( res ) && (gh_scm2int( res ) == 0))
            longjmp( fileAbort, PROBLEM );

        fprintf( stderr, zTplErr, pT->pzFileName,
                 pMac->lineNo, zCantInc );
        fprintf( stderr, zNotStr, pT->pzTemplText + pMac->ozText );
        longjmp( fileAbort, FAILURE );
    }

    fprintf( stderr, zErr, pT->pzFileName, SCM_CHARS( res ));
    longjmp( fileAbort, FAILURE );
    return (tMacro*)NULL;
}


/*=macfunc INCLUDE
 *
 *  what:   Read in and emit a template block
 *  handler_proc:  Standard load.
 *
 *  desc:
 *
 *  The entire contents of the named file is inserted at this point.
 *  The contents of the file are processed for macro expansion.  The
 *  arguments are eval-ed, so you may compute the name of the file to
 *  be included.  The included file must not contain any incomplete
 *  function blocks.  Function blocks are template text beginning with
 *  any of the macro functions @samp{_IF}, @samp{_FOR} and
 *  @samp{_CASE} and extending through the respective terminating
 *  macro functions.
=*/
MAKE_HANDLER_PROC( Include )
{
    SCM   res = eval( pT, pMac, pCurDef );

    if (! gh_string_p( res )) {
        tSCC zNotStr[] = "\texpression yields non-string:  %s\n";
        fprintf( stderr, zTplErr, pT->pzFileName,
                 pMac->lineNo, zCantInc );
        fprintf( stderr, zNotStr, pT->pzTemplText + pMac->ozText );
        longjmp( fileAbort, (abort) ? FAILURE : PROBLEM );
    }

    pT = loadTemplate( SCM_CHARS( res ));
    generateBlock( pT, pT->aMacros, pT->aMacros + pT->macroCt, pCurDef );
    unloadTemplate( pT );

    return pMac + 1;
}

/*=macfunc COND
 *
 *  what:  Eval Expression Conditionally
 *  alias: -
 *  alias: ?
 *  alias: %
 *
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *      The macro text begins with an alphabetic character.  The first
 *      token may be the name of a user defined macro, or
 *      the name of a value, or be unknown to AutoGen.  If it is
 *      a user defined macro, then that macro is invoked and its
 *      output is inserted into the output stream.  If it is a named
 *      value, then the text value associated with the name is inserted
 *      into the output stream.  If the name is not known to AutoGen,
 *      then no output is emitted and processing continues.
 *
 *      For example, @code{[#var_name#]} and @code{[#var_name ...#]} are
 *      equivalent to the following:
 *
 *      @example
 *      [#IF (exist? var_name) #][#
 *          (get var_name) #][#
 *      ENDIF#][#
 *      IF (exist? var_name) #][#
 *      (...) #][#
 *      ENDIF#]
 *      @end example
=*/
MAKE_HANDLER_PROC( Cond )
{
    ag_bool     isIndexed;
    tDefEntry*  pDef;
    int         code = pMac->res;
    char*       pzText;

    /*
     *  Get the named definition entry, maybe
     */
    pDef = findDefEntry( pT->pzTemplText + pMac->ozName, pCurDef, &isIndexed );

    if (pDef == (tDefEntry*)NULL) {
        /*
         *  We do not have a definition.  Check to see if we _must_
         *  have a definition.
         */
        if ((code & (EMIT_IF_ABSENT | EMIT_ALWAYS)) == 0)
            return pMac + 1;
        pzText =  pT->pzTemplText + pMac->endIndex;
    }

    /*
     *  OTHERWISE, we found an entry.  Make sure we were supposed to.
     */
    else {
        if ((code & EMIT_IF_ABSENT) != 0)
            return pMac + 1;

        /*
         *  And make sure what we found is a text value
         */
        if (pDef->valType != VALTYP_TEXT) {
            fprintf( stderr, zTplWarn, pT->pzFileName, pMac->lineNo,
                     "attempted to use block macro in eval expression" );
            return pMac + 1;
        }

        /*
         *  IF this is a formatting macro,
         *  THEN use the value as the argument to the formatting string
         *  ELIF there is text associated with this macro
         *  THEN that text is the value of the expression
         *  OTHERWISE the value associated with the name becomes the
         *            expression.
         */
        if ((code & EMIT_FORMATTED) != 0)
            pzText = asprintf( pT->pzTemplText + pMac->ozText,
                               pDef->pzValue );
        else if (pMac->ozText != 0)
            pzText = pT->pzTemplText + pMac->ozText;
        else pzText = pDef->pzValue;
    }

    /*
     *  The "code" tells us how to handle the expression
     */
    switch (code & 0x000F) {
    case EMIT_VALUE:
        fputs( pDef->pzValue, pCurFp->pFile );
        break;

    case EMIT_EXPRESSION:
    {
        SCM res = scm_internal_stack_catch(
                      SCM_BOOL_T,
                      (scm_catch_body_t)gh_eval_str,
                      (void*)pzText,
                      ag_eval_handler,
                      (void*)pzText );
        if (gh_string_p( res ))
            fputs( SCM_CHARS( res ), pCurFp->pFile );

        else if (gh_char_p( res ))
            fputc( gh_scm2char( res ), pCurFp->pFile );

        else if (gh_number_p( res ))
            fprintf( pCurFp->pFile, "%ld", gh_scm2long( res ));

        break;
    }

    case EMIT_SHELL:
    {
        char* pz = runShell( pzText );
        if (pz != (char*)NULL)
            fputs( pz, pCurFp->pFile );
        AGFREE( (void*)pz );
        break;
    }

    case EMIT_STRING:
        fputs( pzText, pCurFp->pFile );
        break;
    }

    if ((code & EMIT_FORMATTED) != 0)
        AGFREE( (void*)pzText );

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

    pMac->funcCode = FTYP_COND;
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

    return mFunc_Cond( pT, pMac, pCurDef );
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
 *  handler_proc:  Internally loaded.
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
 *  load_proc:  Removed from macro list by loader.
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
    char*          pzCopy = pT->pNext; /* next text dest   */
    const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
    size_t         srcLen = (size_t)pMac->res;         /* macro len  */

    if (srcLen > 0) {
        pMac->res    = 0;
        pMac->ozText = (pzCopy - pT->pzTemplText);

        /*
         *  Copy the expression
         */
        do  {
            *(pzCopy++) = *(pzSrc++);
        } while (--srcLen > 0);
        *(pzCopy++) = '\0';
        *(pzCopy++) = '\0'; /* double terminate */

        pT->pNext = pzCopy;
    }

    return pMac + 1;
}


/*
 *  mLoad_Cond
 *
 *  Some functions are known to AutoGen, but invalid out of context.
 *  For example, ELIF, ELSE and ENDIF are all known to AutoGen.
 *  However, the load function pointer for those functions points
 *  here, until an "IF" function is encountered.
 */
MAKE_LOAD_PROC( Cond )
{
    char*          pzCopy = pT->pNext; /* next text dest   */
    const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
    long           srcLen = (long)pMac->res;           /* macro len  */
    const char*    pzSrcEnd = pzSrc + srcLen;

    pMac->res = EMIT_VALUE; /* zero */

    switch (*pzSrc) {
    case '-':
        pMac->res = EMIT_IF_ABSENT;
        pzSrc++;
        break;

    case '?':
        pMac->res = EMIT_ALWAYS;
        pzSrc++;
        if (*pzSrc != '%')
            break;
        /* FALLTHROUGH */
    case '%':
        pMac->res |= EMIT_FORMATTED;
        pzSrc++;
        break;
    }

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

        pMac->ozText = (pzCopy - pT->pzTemplText);
        /*
         *  Copy the expression
         */
        do  {
            *(pzCopy++) = *(pzSrc++);
        } while (--srcLen > 0);
        *(pzCopy++) = '\0';
        *(pzCopy++) = '\0'; /* double terminate */
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
