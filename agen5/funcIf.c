
/*
 *  $Id: funcIf.c,v 1.2 1999/10/14 17:05:55 bruce Exp $
 *
 *  This module implements the _IF text function.
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
#include "proto.h"

tSCC zNoIfEnd[]   = "%s ERROR:  cannot find ENDIF\n\t'%s'\n";


    STATIC ag_bool
eval_true( tTemplate* pT, tMacro* pMac, tDefEntry* pCurDef )
{
    SCM res = eval( pT, pMac, pCurDef );
    if (gh_boolean_p( res ))
        return SCM_NFALSEP( res );
    if (gh_number_p( res ))
        return (gh_scm2long( res ) != 0);
    if (gh_char_p( res ))
        return (gh_scm2char( res ) != '\0');
    if (gh_string_p( res ))
        return (*SCM_CHARS( res ) != '\0');
    return AG_FALSE;
}


/*=macfunc IF
 *
 *  what:    Conditionally Emit a Template Block
 *  cindex:  conditional emit
 *  cindex:  if test
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *  Conditional block.  Its arguments are evaluated (see _EVAL) and
 *  if the result is non-zero or a string with one or more bytes,
 *  then the condition is true and the text from that point
 *  until a matched "ELIF", "ELSE" or "ENDIF" is emitted.
 *  "ELIF" introduces a conditional alternative if the "_IF"
 *  clause evaluated FALSE and "ELSE" introduces an unconditional
 *  alternative.
 *
 *  @example
 *  [#IF [[condition]]#]
 *  emit things that are for the true condition[#
 *
 *  ELIF [[maybe]] #]
 *  emit things that are true maybe[#
 *
 *  ELSE "This may be a comment" #]
 *  emit this if all but else fails[#
 *
 *  ENDIF "This may *also* be a comment" #]
 *  @end example
=*/
/*=macfunc ENDIF
 *
 *  what:   Terminate the @code{IF} Template Block
 *  situational:
 *
 *  desc:
 *    This macro ends the @code{IF} function template block.
 *    For a complete description @xref{IF}.
=*/
MAKE_HANDLER_PROC( If )
{
    tMacro* pRet = pT->aMacros + pMac->endIndex;

    do  {
        /*
         *  The current macro becomes the 'ELIF' or 'ELSE' macro
         */
        pCurMacro = pMac;

        /*
         *  'ELSE' is equivalent to 'ELIF true'
         */
        if (  (pMac->funcCode == FTYP_ELSE)
           || eval_true( pT, pMac, pCurDef )) {

            generateBlock( pT, pMac+1, pT->aMacros + pMac->sibIndex, pCurDef );
            break;
        }
        pMac = pT->aMacros + pMac->sibIndex;
    } while (pMac < pRet);

    return pRet;
}


/*=macfunc WHILE
 *
 *  what:    Conditionally loop over a Template Block
 *  cindex:  conditional emit
 *  cindex:  while test
 *  handler_proc:
 *  load_proc:
 *
 *  desc:
 *  Conditionally repeated block.  Its arguments are evaluated (see _EVAL)
 *  and as long as the result is non-zero or a string with one or more bytes,
 *  then the condition is true and the text from that point
 *  until a matched "ENDWHILE" is emitted.
 *
 *  @example
 *  [#WHILE [[condition]]#]
 *  emit things that are for the true condition[#
 *
 *  ENDWHILE #]
 *  @end example
=*/
/*=macfunc ENDWHILE
 *
 *  what:   Terminate the @code{IF} Template Block
 *  situational:
 *
 *  desc:
 *    This macro ends the @code{IF} function template block.
 *    For a complete description @xref{IF}.
=*/
MAKE_HANDLER_PROC( While )
{
    tMacro* pRet = pT->aMacros + pMac->endIndex;

    for (;;) {
        if (! eval_true( pT, pMac, pCurDef ))
            break;

        generateBlock( pT, pMac+1, pT->aMacros + pMac->sibIndex, pCurDef );
    }

    return pRet;
}

#endif /* DEFINE_LOAD_FUNCTIONS defined */

typedef struct if_stack tIfStack;
struct if_stack {
    tMacro*  pIf;
    tMacro*  pElse;
};

STATIC tIfStack  current_if;
STATIC tLoadProc mLoad_Elif, mLoad_Else;


/*=macfunc ELIF
 *
 *  what:   Alternate Conditional Template Block
 *  situational:
 *
 *  desc:
 *    This macro must only appear after an @code{IF} function,
 *    and before any associated @code{ELSE} or @code{ENDIF} functions.
 *    It denotes the start of an alternate template block for
 *    the @code{IF} function.  For a complete description @xref{IF}.
=*/
MAKE_LOAD_PROC( Elif )
{
    const char*    pzScan = *ppzScan;  /* text after macro */
    char*          pzCopy = pT->pNext; /* next text dest   */
    const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
    size_t         srcLen = (size_t)pMac->res;         /* macro len  */

    if (srcLen == 0)
        LOAD_ABORT( pT, pMac, "No expression for `ELIF' function" );

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

    current_if.pElse->sibIndex = pMac - pT->aMacros;
    current_if.pElse = pMac;
    return pMac + 1;
}


/*=macfunc ELSE
 *
 *  what:   Alternate Template Block
 *  situational:
 *
 *  desc:
 *    This macro must only appear after an @code{IF} function,
 *    and before the associated @code{ENDIF} function.
 *    It denotes the start of an alternate template block for
 *    the @code{IF} function.  For a complete description @xref{IF}.
=*/
MAKE_LOAD_PROC( Else )
{
    const char*    pzScan = *ppzScan;  /* text after macro */
    char*          pzCopy = pT->pNext; /* next text dest   */
    const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
    size_t         srcLen = (size_t)pMac->res;         /* macro len  */

    /*
     *  After processing an "ELSE" macro,
     *  we have a special handler function for 'ENDIF' only.
     */
    static tpLoadProc apElseLoad[ FUNC_CT ] = { (tpLoadProc)NULL };

    if (apElseLoad[0] == (tpLoadProc)NULL) {
        memcpy( (void*)apElseLoad, apLoadProc, sizeof( apLoadProc ));
        apElseLoad[ FTYP_ENDIF ] = &mLoad_Ending;
    }

    papLoadProc = apElseLoad;

    current_if.pElse->sibIndex = pMac - pT->aMacros;
    current_if.pElse = pMac;
    pMac->ozText = 0;

    return pMac+1;
}


MAKE_LOAD_PROC( Ending )
{
    memset( (void*)pMac, 0, sizeof( *pMac ));
    return (tMacro*)NULL;
}


MAKE_LOAD_PROC( If )
{
    const char*    pzScan = *ppzScan;  /* text after macro */
    char*          pzCopy = pT->pNext; /* next text dest   */
    const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
    size_t         srcLen = (size_t)pMac->res;         /* macro len  */

    tIfStack       save_stack = current_if;
    tpLoadProc*    papLP = papLoadProc;
    tMacro*        pEndifMac;

    /*
     *  While processing an "IF" macro,
     *  we have handler functions for 'ELIF', 'ELSE' and 'ENDIF'
     *  Otherwise, we do not.  Switch the callout function table.
     */
    static tpLoadProc apIfLoad[ FUNC_CT ] = { (tpLoadProc)NULL };

    if (apIfLoad[0] == (tpLoadProc)NULL) {
        memcpy( (void*)apIfLoad, apLoadProc, sizeof( apLoadProc ));
        apIfLoad[ FTYP_ELIF ]  = &mLoad_Elif;
        apIfLoad[ FTYP_ELSE ]  = &mLoad_Else;
        apIfLoad[ FTYP_ENDIF ] = &mLoad_Ending;
    }

    papLoadProc = apIfLoad;

    /*
     *  We will need to chain together the 'IF', 'ELIF', and 'ELSE'
     *  macros.  The 'ENDIF' gets absorbed.
     */
    current_if.pIf = current_if.pElse = pMac;

    if (srcLen == 0)
        LOAD_ABORT( pT, pMac, "No expression for `IF' function" );

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

    /*
     *  Now, do a nested parse of the template.
     *  When the matching 'ENDIF' macro is encountered,
     *  the handler routine will cause 'parseTemplate()'
     *  to return with the text scanning pointer pointing
     *  to the remaining text.
     */
    pEndifMac = parseTemplate( pT, pMac+1, ppzScan );
    if (*ppzScan == (char*)NULL) {
        fputs( "Unterminated IF block\n", stderr );
        LOAD_ABORT( pT, pMac, "parse err" );
    }

    current_if.pIf->endIndex   = \
    current_if.pElse->sibIndex = pEndifMac - pT->aMacros;

    /*
     *  Restore the context of any encompassing block macros
     */
    current_if  = save_stack;
    papLoadProc = papLP;
    return pEndifMac;
}


MAKE_LOAD_PROC( While )
{
    const char*    pzScan = *ppzScan;  /* text after macro */
    char*          pzCopy = pT->pNext; /* next text dest   */
    const char*    pzSrc  = (const char*)pMac->ozText; /* macro text */
    size_t         srcLen = (size_t)pMac->res;         /* macro len  */

    tpLoadProc*    papLP = papLoadProc;
    tMacro*        pEndMac;

    /*
     *  While processing an "IF" macro,
     *  we have handler functions for 'ELIF', 'ELSE' and 'ENDIF'
     *  Otherwise, we do not.  Switch the callout function table.
     */
    static tpLoadProc apWhileLoad[ FUNC_CT ] = { (tpLoadProc)NULL };

    if (apWhileLoad[0] == (tpLoadProc)NULL) {
        memcpy( (void*)apWhileLoad, apLoadProc, sizeof( apLoadProc ));
        apWhileLoad[ FTYP_ENDWHILE ] = &mLoad_Ending;
    }

    papLoadProc = apWhileLoad;

    if (srcLen == 0)
        LOAD_ABORT( pT, pMac, "No expression for `IF' function" );

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

    /*
     *  Now, do a nested parse of the template.
     *  When the matching 'ENDIF' macro is encountered,
     *  the handler routine will cause 'parseTemplate()'
     *  to return with the text scanning pointer pointing
     *  to the remaining text.
     */
    pEndMac = parseTemplate( pT, pMac+1, ppzScan );
    if (*ppzScan == (char*)NULL)
        LOAD_ABORT( pT, pMac, "parse err" );

    pMac->sibIndex = pMac->endIndex = pEndMac - pT->aMacros;

    /*
     *  Restore the context of any encompassing block macros
     */
    papLoadProc = papLP;
    return pEndMac;
}
/* end of funcIf.c */
