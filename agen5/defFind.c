/*
 *  agGetDef.c
 *  $Id: defFind.c,v 1.5 2000/03/05 20:58:13 bruce Exp $
 *  This module loads the definitions, calls yyparse to decipher them,
 *  and then makes a fixup pass to point all children definitions to
 *  their parent definition (except the fixed "rootEntry" entry).
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

#include <streqv.h>

#include "autogen.h"

tSCC zInvalRef[] = "invalid reference";

typedef enum {
    CDF_INITIAL = 0,
    CDF_TOPLEVEL_NAME,
    CDF_INDEX_NAME
} teState;

STATIC tDefEntry* findEntryByIndex( tDefEntry* pE, char* pzScan );


/*
 *  Copy a reference to a definition.
 */
    EXPORT int
copyDefReference( tTemplate* pT, tMacro* pMac,
                  char** ppDest, const char** ppSrc, int maxLen )
{
    char*   pDest  = *ppDest;
    tCC*    pSrc   = *ppSrc;
    tCC*    pStart = pSrc;
    teState state  = CDF_INITIAL;
    char    ch, nextCh;

    /*
     *  After skipping any white space, we are allowed to stumble upon a top
     *  level name, an index or a period separating a nested value name.
     */
    do  {
        if (maxLen-- <= 0)
            goto leaveCopyDefReference;
        ch = *(pSrc++);
    } while (isspace( ch ));

    if (ISNAMECHAR( ch ))
        state = CDF_TOPLEVEL_NAME;

    else switch (ch) {
    case '[':
    case '.':
        *(pDest++) = ch;
        do  {
            if (maxLen-- <= 0)
                LOAD_ABORT( pT, pMac, zInvalRef );

            nextCh = *(pSrc++);
        } while (isspace( nextCh ));

        if (! ISNAMECHAR( nextCh )) {
            /*
             *  Not having a name character here is valid IFF
             *  we are using the "[$]" subscript syntax
             */
            if (ch != '[')
                LOAD_ABORT( pT, pMac, zInvalRef );

            switch (nextCh) {
            case '$':
                *(pDest++) = nextCh;

                /*
                 *  Skip trailing space
                 */
                do  {
                    if (maxLen-- <= 0)
                        LOAD_ABORT( pT, pMac, zInvalRef );
                    ch = *(pSrc++);
                } while (isspace( ch ));
                if (ch != ']')
                    LOAD_ABORT( pT, pMac, zInvalRef );
                goto closeBracket;
                /* FALLTHROUGH */

            case ']':
                ch = nextCh;
                goto closeBracket;

            default:
                LOAD_ABORT( pT, pMac, zInvalRef );
            }
        }

        state = (ch == '.') ? CDF_TOPLEVEL_NAME : CDF_INDEX_NAME;
        ch = nextCh;
        break;

    default:
        LOAD_ABORT( pT, pMac, zInvalRef );
    }

    for (;;) {
        /*
         *  FOR each name character, copy it.
         */
        if ((ch == '$') && (state == CDF_INDEX_NAME)) {
            *(pDest++) = ch;
            if (maxLen-- <= 0)
                LOAD_ABORT( pT, pMac, zInvalRef );
            ch = *(pSrc++);
            if ((! isspace( ch )) && (ch != ']'))
                LOAD_ABORT( pT, pMac, zInvalRef );
        }
        else do  {
            *(pDest++) = ch;
            if (maxLen-- <= 0) {
                if (state == CDF_INDEX_NAME)
                    LOAD_ABORT( pT, pMac, zInvalRef );
                goto leaveCopyDefReference;
            }
            ch = *(pSrc++);
        } while (ISNAMECHAR( ch ));

        /*
         *  skip any trailing space
         */
        while (isspace( ch )) {
            if (maxLen-- <= 0)
                goto leaveCopyDefReference;
            ch = *(pSrc++);
        }

        /*
         *  Only three special chars are used in definition references
         *  '[', ']' and '.'.  Stop on anything else.
         */
        switch (ch) {
        case '[':
        case '.':
            if (state == CDF_INDEX_NAME)
                LOAD_ABORT( pT, pMac, zInvalRef );

            /*
             *  The next state depends on the current char
             */
            *(pDest++) = ch;
            state = (ch == '.') ? CDF_TOPLEVEL_NAME : CDF_INDEX_NAME;

            /*
             *  Skip trailing space
             */
            do  {
                if (maxLen-- <= 0)
                    LOAD_ABORT( pT, pMac, zInvalRef );
                ch = *(pSrc++);
            } while (isspace( ch ));

            /*
             *  We *must* have a name at this point.
             */
            if (! ISNAMECHAR( ch )) {
                /*
                 *  Not having a name character here is valid IFF
                 *  we are using the "[$]" subscript syntax
                 */
                if (state != CDF_INDEX_NAME)
                    LOAD_ABORT( pT, pMac, zInvalRef );

                switch (ch) {
                case '$':
                    *(pDest++) = ch;

                    /*
                     *  Skip trailing space
                     */
                    do  {
                        if (maxLen-- <= 0)
                            LOAD_ABORT( pT, pMac, zInvalRef );
                        ch = *(pSrc++);
                    } while (isspace( ch ));
                    if (ch != ']')
                        LOAD_ABORT( pT, pMac, zInvalRef );
                    /* FALLTHROUGH */

                case ']':
                    goto closeBracket;

                default:
                    LOAD_ABORT( pT, pMac, zInvalRef );
                }
            }
            break;

        case ']':
            if (state == CDF_TOPLEVEL_NAME)
                LOAD_ABORT( pT, pMac, zInvalRef );

        closeBracket:
            *(pDest++) = ch;

            /*
             *  Skip trailing space
             */
            do  {
                if (maxLen-- <= 0)
                    goto leaveCopyDefReference;
                ch = *(pSrc++);
            } while (isspace( ch ));

            switch (ch) {
            case '.':
                *(pDest++) = ch;
                break;

            case '[':
            case ']':
                LOAD_ABORT( pT, pMac, zInvalRef );

            default:
                pSrc--;
                goto leaveCopyDefReference;
            }

            /*
             *  We found a '.' char.  Skip trailing space
             */
            do  {
                if (maxLen-- <= 0)
                    LOAD_ABORT( pT, pMac, zInvalRef );
                ch = *(pSrc++);
            } while (isspace( ch ));

            if (! ISNAMECHAR( ch ))
                LOAD_ABORT( pT, pMac, zInvalRef );
            state = CDF_TOPLEVEL_NAME;
            break;

        default:
            pSrc--;
            goto leaveCopyDefReference;
        }
    }

 leaveCopyDefReference:
    *ppSrc     = pSrc;
    *(pDest++) = NUL;
    *ppDest    = pDest;
    return (int)(pSrc - pStart);
}


    STATIC tDefEntry*
findEntryByIndex( tDefEntry* pE, char* pzScan )
{
    int  idx;

    /*
     *  '[]' means the first entry of whatever index number
     */
    if (*pzScan == ']')
        return pE;

    /*
     *  '[$]' means the last entry of whatever index number
     */
    if (*pzScan == '$') {
        while (isspace( *++pzScan )) ;
        if (*pzScan != ']')
            return (tDefEntry*)NULL;

        if (pE->pEndTwin != (tDefEntry*)NULL)
            return pE->pEndTwin;

        return pE;
    }

    /*
     *  '[nn]' means the specified index number
     */
    if (isdigit( *pzScan )) {
        char* pz;
        idx = strtol( pzScan, &pz, 0 );

        /*
         *  Skip over any trailing space and make sure we have a closer
         */
        while (isspace( *pz )) pz++;
        if (*pz != ']')
            return (tDefEntry*)NULL;
    }

    else {
        /*
         *  '[XX]' means get the index from our definitions
         */
        char* pzDef = pzScan;
        const char* pzVal;

        if (! isalpha( *pzScan ))
            return (tDefEntry*)NULL;

        while (ISNAMECHAR( *pzScan )) pzScan++;

        /*
         *  Temporarily remove the character under *pzScan and
         *  find the corresponding defined value.
         */
        {
            char  svch = *pzScan;
            *pzScan = NUL;
            pzVal    = getDefine( pzDef );
            *pzScan = svch;
        }

        /*
         *  Skip over any trailing space and make sure we have a closer
         */
        while (isspace( *pzScan )) pzScan++;
        if (*pzScan != ']')
            return (tDefEntry*)NULL;

        /*
         *  make sure we found a defined value
         */
        if ((pzVal == (char*)NULL) || (*pzVal == NUL))
            return (tDefEntry*)NULL;

        idx = strtol( pzVal, &pzDef, 0 );

        /*
         *  Make sure we got a legal number
         */
        if (*pzDef != NUL)
            return (tDefEntry*)NULL;
    }

    /*
     *  Search for the entry with the specified index.
     */
    do  {
        if (pE->index > idx)
            return (tDefEntry*)NULL;
        if (pE->index == idx)
            break;
        pE = pE->pTwin;
    } while (pE != (tDefEntry*)NULL);

    return pE;
}

/*
 *  findDefEntry
 *
 *  Find the definition entry for the name passed in.
 *  It is okay to find block entries IFF they are found on the
 *  current level.  Once you start traversing up the tree,
 *  the macro must be a text macro.  Return an indicator saying if
 *  the element has been indexed (so the caller will not try
 *  to traverse the list of twins).
 */
    EXPORT tDefEntry*
findDefEntry( char* pzName, tDefEntry* pDefs, ag_bool* pIsIndexed )
{
    char*      pcBrace;
    char       breakCh;
    tDefEntry* pE;
    char       zDefName[ MAXPATHLEN ];

    if (pIsIndexed != (ag_bool*)NULL)
      *pIsIndexed = AG_FALSE;

 findDefEntryRecurse:
    pcBrace  = pzName + strcspn( pzName, "[." );
    breakCh  = *pcBrace;
    *pcBrace = NUL;

    if (pIsIndexed != (ag_bool*)NULL)
      *pIsIndexed |= (breakCh == '[');

    strtransform( zDefName, pzName );

    for (;;) {
        if (pDefs == (tDefEntry*)NULL) {
            *pcBrace = breakCh;
            return (tDefEntry*)NULL;
        }
        pE = pDefs;

        do  {
            /*
             *  IF the name matches
             *  THEN break out of the double loop
             */
            if (strcmp( pE->pzName, zDefName ) == 0)
                goto findTwin;

            pE = pE->pNext;
        } while (pE != (tDefEntry*)NULL);

        /*
         *  Advance and check for finish (not found in root defs)
         */
        pDefs = pDefs->pDad ;
    }

findTwin:
    *pcBrace = breakCh;
    switch (breakCh) {
    case NUL:
        return pE;

    case '[':
        /*
         *  We have to find a specific entry in a list.
         */
        while (isspace( *++pcBrace )) ;

        pE = findEntryByIndex( pE, pcBrace );
        if (pE == (tDefEntry*)NULL)
            return pE;

        /*
         *  We must find the closing brace, or there is an error
         */
        pcBrace = strchr( pcBrace, ']' );
        if (pcBrace == (char*)NULL)
            return (tDefEntry*)NULL;

        /*
         *  IF we are at the end of the definition,
         *  THEN return what we found
         */
        if (*++pcBrace != '.')
            return pE;

        /* FALLTHROUGH */

    case '.':
        /*
         *  It is a segmented value name.  Set the name pointer
         *  to the next segment and search starting from the newly
         *  available set of definitions.
         */
        pzName = pcBrace + 1;
        break;
    }

    /*
     *  We cannot find a member of a non-block type macro definition.
     */
    if (pE->valType != VALTYP_BLOCK)
        return (tDefEntry*)NULL;

    pDefs = (tDefEntry*)pE->pzValue;
    goto findDefEntryRecurse;
}
/* end of defFind.c */
