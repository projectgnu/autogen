
/*
 *  $Id: funcEval.c,v 1.1.1.1 1999/10/14 00:33:53 bruce Exp $
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


    SCM 
ag_eval_handler( void *data, SCM tag, SCM throw_args )
{
    SCM port = scm_current_error_port();
    fprintf( stderr, "\nScheme evaluation error; tag is\n        ");
    scm_display( tag, port );

    fprintf( stderr, "\nin %s line %d:  %s\n", pCurTemplate->pzTplName,
             pCurMacro->lineNo,
             pCurTemplate->pzTemplText + pCurMacro->ozText );

#ifdef LATER
    if (scm_ilength( throw_args ) >= 3) {
        SCM stack = SCM_CDR(   scm_the_last_stack_var );
        SCM subr  = SCM_CAR(   throw_args );
        SCM msg   = SCM_CADR(  throw_args );
        SCM args  = SCM_CADDR( throw_args );
        scm_display_backtrace( stack, port, SCM_UNDEFINED, SCM_UNDEFINED );
        scm_newline( port );
        scm_display_error( stack, port, subr, msg, args, SCM_EOL );
    }
#endif /* LATER */

    AG_ABEND;
    /* NOTREACHED */
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
eval( tTemplate*  pT,
      tMacro*     pMac,
      tDefEntry*  pCurDef )
{
    char* pzExpr = pT->pzTemplText + pMac->ozText;
    SCM   res;

    if (pMac->ozText == 0)
        return gh_str02scm("");

    switch (*pzExpr) {
    case '(':
    case ';':
        res = scm_internal_stack_catch(
                  SCM_BOOL_T,
                  (scm_catch_body_t)gh_eval_str,
                  (void*)pzExpr,
                  ag_eval_handler,
                  (void*)pzExpr );
        break;

    case '"':
    case '\'':
        AGDUPSTR( pzExpr, pzExpr );
#ifdef LATER
        (void)spanQuote( pzExpr );
#endif
        /* FALLTHROUGH */

    default:
        res = gh_str02scm( pzExpr );
        if (pzExpr != pT->pzTemplText + pMac->ozText)
            AGFREE( (void*)pzExpr );
    }

    return res;
}

/*=macfunc EVAL
 *
 *  what:   Evaluate and Emit an Expression
 *  alias:  ;
 *  alias:  (
 *  alias:  `
 *  handler_proc:  Standard load.
 *
 *  desc:
 *       The argument is evaluated and the result printed to the output file.
 *       The evaluator is Guile, so any valid Guile expression that yields
 *       either a string, a character or a number may be used.
 *       This function is selected by specifying "(" as the first character
 *       in the macro.
 *
 *       There are a number of special AutoGen expression functions added
 *       to Guile.  They are:
 *
 *  table:   agexpr_func
=*/
MAKE_HANDLER_PROC( Eval )
{
    char* pz = pT->pzTemplText + pMac->ozText;
    SCM   res;

 try_again:  /* keep stripping off Scheme comments until done */
    switch ( *pz ) {
    case ';': /* scheme comment */
        pz = strchr( pz, '\n' );
        if (pz == (char*)NULL) {
            pT->pzTemplText[ pMac->ozText ] = NUL;
            return pMac + 1;
        }
        while (isspace( *pz )) pz++;
        pMac->ozText = (pz - pT->pzTemplText);
        goto try_again;

    case '(': /* scheme expression */
        res = eval( pT, pMac, pCurDef );

        if (gh_string_p( res ))
            fputs( SCM_CHARS( res ), pCurFp->pFile );

        else if (gh_char_p( res ))
            fputc( gh_scm2char( res ), pCurFp->pFile );

        else if (gh_number_p( res ))
            fprintf( pCurFp->pFile, "%ld", gh_scm2long( res ));
        break;

    case '`': /* unprocessed shell expression */
    {
        char* p;
        AGDUPSTR( p, pz );
        spanQuote( p );
        strcpy( pz+1, p );
        AGFREE( (void*)p );
        *pz = '#';
        /* FALLTHROUGH */
    }

    case '#': /* processed shell expression */
        pz = runShell( pz+1 );
        fputs( pz, pCurFp->pFile );
        AGFREE( (void*)pz );
        break;

    default:
        fprintf( stderr, zTplErr, pT->pzFileName,
                 pMac->lineNo, pz );
        *pz = NUL;
        /* FALLTHROUGH */

    case NUL:
        break;
    }
    return pMac + 1;
}
#endif /* DEFINE_LOAD_FUNCTIONS */
/* end of funcEval.c */
