
/*
 *  tpParse.c
 *
 *  $Id: tpParse.c,v 3.4 2002/01/13 08:04:33 bkorb Exp $
 *
 *  This module will load a template and return a template structure.
 */

/*
 *  AutoGen copyright 1992-2002 Bruce Korb
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

#define LOAD_FUNCTIONS
#include "autogen.h"

STATIC teFuncType whichFunc( tCC** ppzScan );
STATIC tCC* findMacroEnd( tCC** ppzMark );


/*
 *  Return the enumerated function type corresponding
 *  to a name pointed to by the input argument.
 */
STATIC teFuncType
whichFunc( tCC** ppzScan )
{
    const char*   pzFuncName = *ppzScan;
    int           hi, lo, av;
    tNameType*    pNT;
    int           cmp;

    /*
     *  IF the name starts with a punctuation,
     *  THEN we will use strncmp instead of strneqvcmp to test values.
     */
    if (ispunct( *pzFuncName )) {
        hi = FUNC_ALIAS_HIGH_INDEX;
        lo = FUNC_ALIAS_LOW_INDEX;
        do  {
            av  = (hi + lo)/2;
            pNT = nameTypeTable + av;
            cmp = (int)(*(pNT->pName)) - (int)(*pzFuncName);

            /*
             *  For strings that start with a punctuation, we
             *  do not need to test for the end of token
             *  We will not strip off the marker and the load function
             *  will figure out what to do with the code.
             */
            if (cmp == 0)
                return pNT->fType;
            if (cmp > 0)
                 hi = av - 1;
            else lo = av + 1;
        } while (hi >= lo);
        return FTYP_BOGUS;
    }

    if (! isalpha( *pzFuncName ))
        return FTYP_BOGUS;

    hi = FUNC_NAMES_HIGH_INDEX;
    lo = FUNC_NAMES_LOW_INDEX;

    do  {
        av  = (hi + lo)/2;
        pNT = nameTypeTable + av;
        cmp = strneqvcmp( pNT->pName, pzFuncName, pNT->cmpLen );
        if (cmp == 0) {
            /*
             *  Make sure we matched to the end of the token.
             */
            if (ISNAMECHAR( pzFuncName[pNT->cmpLen] ))
                break;

            /*
             *  Advance the scanner past the macro name.
             *  The name is encoded in the "fType".
             */
            *ppzScan = pzFuncName + pNT->cmpLen;
            return pNT->fType;
        }
        if (cmp > 0)
             hi = av - 1;
        else lo = av + 1;
    } while (hi >= lo);

    /*
     *  Save the name for later lookup
     */
    pCurMacro->ozName = (pCurTemplate->pNext - pCurTemplate->pzTemplText);
    {
        char* pzCopy = pCurTemplate->pNext;
        while (ISNAMECHAR( *pzFuncName ))
            *(pzCopy++) = *(pzFuncName++);
        *(pzCopy++) = '\0';
        *ppzScan = pzFuncName;
        pCurTemplate->pNext = pzCopy;
    }

    /*
     *  "Unknown" means we have to check again before we
     *  know whether to assign it to "FTYP_INVOKE" or "FTYP_COND".
     *  That depends on whether or not we find a named template
     *  at template instantiation time.
     */
    return FTYP_UNKNOWN;
}


STATIC tCC*
findMacroEnd( tCC** ppzMark )
{
    tCC* pzMark = *ppzMark + startMacLen;
    tCC* pzNextMark;
    tCC* pzEndMark;
    tSCC zOop[] = "Error:  a %s macro contains a nested macro\n";
    char z[ 1024 ];

    /*
     *  Set our pointers to the start of the macro text
     */
    while (isspace( *pzMark )) {
        if (*(pzMark++) == '\n')
            templLineNo++;
    }

    pCurMacro->funcCode = whichFunc( &pzMark );
    pCurMacro->lineNo   = templLineNo;
    *ppzMark     = pzMark;

    pzEndMark = strstr( pzMark, zEndMac );
	if (pzEndMark == (char*)NULL)
		AG_ABEND( "macro has no end" );

	pzNextMark = strstr( pzMark, zStartMac );
	if (pzNextMark == NULL)
		return pzEndMark;

	if (pzEndMark > pzNextMark)
		AG_ABEND( "macros cannot nest" );

	return pzEndMark;
}


EXPORT tMacro*
parseTemplate( tMacro* pM, tCC** ppzText )
{
    tCC* pzScan = *ppzText;
    tTemplate* pT = pCurTemplate;

#if defined( DEBUG )
    tSCC zTDef[]   = "%-10s (%d) line %d end=%d, strlen=%d\n";
    tSCC zTUndef[] = "%-10s (%d) line %d - MARKER\n";

    static int level = 0;
    #define DEBUG_DEC(l)  l--

    if (  ((level++) > 0)
       && HAVE_OPT( SHOW_DEFS )) {
        int ct = level;
        tMacro* pPm = pM-1;

        fprintf( pfTrace, "%3d ", pPm - pT->aMacros );
        do { fputs( "  ", pfTrace ); } while (--ct > 0);

        fprintf( pfTrace, zTUndef, apzFuncNames[ pPm->funcCode ],
                 pPm->funcCode, pPm->lineNo );
    }
#else
    #define DEBUG_DEC(l)
#endif

    for (;;) {
        const char* pzMark = strstr( pzScan, zStartMac );

        /*
         *  IF there is any text, then make a text macro entry
         */
        if (pzMark != pzScan) {
            char* pzCopy = pT->pNext;
            const char* pzEnd = (pzMark != (char*)NULL)
                                ? pzMark : pzScan + strlen( pzScan );

            pM->ozText    = pzCopy - pT->pzTemplText;
            pM->funcCode  = FTYP_TEXT;
            pM->lineNo    = templLineNo;
#if defined( DEBUG )
            if (HAVE_OPT( SHOW_DEFS )) {
                int ct = level;
                fprintf( pfTrace, "%3d ", pM - pT->aMacros );
                do { fputs( "  ", pfTrace ); } while (--ct > 0);

                fprintf( pfTrace, zTDef, apzFuncNames[ FTYP_TEXT ], FTYP_TEXT,
                        pM->lineNo, pM->endIndex,
                        pzEnd - pzScan );
            }
#endif
            do  {
                if ((*(pzCopy++) = *(pzScan++)) == '\n')
                    templLineNo++;
            } while (pzScan < pzEnd);
            *(pzCopy++) = '\0';
            pM++;
            pT->pNext = pzCopy;
        }

        /*
         *  IF no more macro marks are found,
         *  THEN we are done...
         */
        if (pzMark == (char*)NULL)
            break;

        /*
         *  Find the macro code and the end of the macro invocation
         */
        pCurMacro = pM;
        pzScan = findMacroEnd( &pzMark );

        /*
         *  Count the lines in the macro text and advance the
         *  text pointer to after the marker.
         */
        {
            const char*  pzMacEnd = pzScan;
            const char*  pz       = pzMark;

            for (;;pz++) {
                pz = strchr( pz, '\n' );
                if ((pz == (char*)NULL) || (pz > pzMacEnd))
                    break;
                templLineNo++;
            }

            /*
             *  Strip white space from the macro
             */
            while ((pzMark < pzMacEnd) && isspace( *pzMark ))  pzMark++;
            while ((pzMacEnd > pzMark) && isspace( pzMacEnd[-1] )) pzMacEnd--;
            if (pzMark != pzMacEnd) {
                pM->ozText = (off_t)pzMark;
                pM->res    = (long)(pzMacEnd - pzMark);
            }
        }

        pzScan += endMacLen;

        /*
         *  IF the called function returns a NULL next macro pointer,
         *  THEN some block has completed.  The returned scanning pointer
         *       will be non-NULL.
         */
        {
#if ! defined( DEBUG )
            tMacro* pNM = (*(papLoadProc[ pM->funcCode ]))( pT, pM, &pzScan );
#else
            teFuncType ft = pM->funcCode;
            int        ln = pM->lineNo;

            tMacro* pNM = (*(papLoadProc[ ft ]))( pT, pM, &pzScan );
            if (HAVE_OPT( SHOW_DEFS )) {
                int ct = level;
                if (pM->funcCode == FTYP_BOGUS)
                     fputs( "    ", pfTrace );
                else fprintf( pfTrace, "%3d ", pM - pT->aMacros );

                do { fputs( "  ", pfTrace ); } while (--ct > 0);

                if (pM->funcCode == FTYP_BOGUS)
                     fprintf( pfTrace, zTUndef, apzFuncNames[ ft ], ft, ln );
                else
                    fprintf( pfTrace, zTDef, apzFuncNames[ ft ], pM->funcCode,
                             ln, pM->endIndex,
                             strlen( (pM->ozText == 0) ? ""
                                     : (pT->pzTemplText + pM->ozText) ));
            }
#endif

            if (pNM == (tMacro*)NULL) {
                *ppzText = pzScan;
                DEBUG_DEC(level);
                return pM;
            }
            pM = pNM;
        }
    }

    DEBUG_DEC(level);

    /*
     *  We reached the end of the input string.
     *  Return a NULL scanning pointer and a pointer to the end.
     */
    *ppzText = (const char*)NULL;
    return pM;
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of tpParse.c */
