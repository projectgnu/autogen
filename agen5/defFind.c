/*
 *  $Id: defFind.c,v 4.18 2008/08/27 14:35:49 bkorb Exp $
 *
 *  Time-stamp:        "2008-08-10 12:06:47 bkorb"
 *  Last Committed:    $Date: 2008/08/27 14:35:49 $
 *
 *  This module locates definitions.
 *
 *  This file is part of AutoGen.
 *  AutoGen copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *  AutoGen copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *
 * AutoGen is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AutoGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

struct defEntryList {
    size_t         allocCt;
    size_t         usedCt;
    tDefEntry**    papDefEntry;
    int            nestLevel;
};
typedef struct defEntryList tDefEntryList;

tSCC zNameRef[]   = "Ill formed name ``%s'' in %s line %d\n";

static char zDefinitionName[ AG_PATH_MAX ];

static tDefEntry* findEntryByIndex( tDefEntry* pE, char* pzScan );

#define ILLFORMEDNAME() \
    AG_ABEND( aprf( zNameRef, zDefinitionName, \
              pCurTemplate->pzTplFile, pCurMacro->lineNo ));


/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by mk-fwd */
static tDefEntry*
findEntryByIndex( tDefEntry* pE, char* pzScan );

static void
addResult( tDefEntry* pDE, tDefEntryList* pDEL );

static size_t
badName( char* pzD, char const* pzS, size_t srcLen );

static tDefEntry*
defEntrySearch( char* pzName, tDefCtx* pDefCtx, ag_bool* pIsIndexed );

static tDefEntry**
entryListSearch( char* pzName, tDefCtx* pDefCtx );
/* = = = END-STATIC-FORWARD = = = */

static tDefEntry*
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
        while (IS_WHITESPACE_CHAR(*++pzScan )) ;
        if (*pzScan != ']')
            return NULL;

        if (pE->pEndTwin != NULL)
            return pE->pEndTwin;

        return pE;
    }

    /*
     *  '[nn]' means the specified index number
     */
    if (IS_DEC_DIGIT_CHAR(*pzScan)) {
        char* pz;
        idx = strtol( pzScan, &pz, 0 );

        /*
         *  Skip over any trailing space and make sure we have a closer
         */
        while (IS_WHITESPACE_CHAR(*pz )) pz++;
        if (*pz != ']')
            return NULL;
    }

    else {
        /*
         *  '[XX]' means get the index from our definitions
         */
        char* pzDef = pzScan;
        char const* pzVal;

        if (! IS_VAR_FIRST_CHAR(*pzScan))
            return NULL;

        while (IS_VALUE_NAME_CHAR(*pzScan)) pzScan++;

        /*
         *  Temporarily remove the character under *pzScan and
         *  find the corresponding defined value.
         */
        {
            char  svch = *pzScan;
            *pzScan = NUL;
            pzVal   = getDefine( pzDef, AG_TRUE );
            *pzScan = svch;
        }

        /*
         *  Skip over any trailing space and make sure we have a closer
         */
        while (IS_WHITESPACE_CHAR(*pzScan )) pzScan++;
        if (*pzScan != ']')
            return NULL;

        /*
         *  make sure we found a defined value
         */
        if ((pzVal == NULL) || (*pzVal == NUL))
            return NULL;

        idx = strtol( pzVal, &pzDef, 0 );

        /*
         *  Make sure we got a legal number
         */
        if (*pzDef != NUL)
            return NULL;
    }

    /*
     *  Search for the entry with the specified index.
     */
    do  {
        if (pE->index > idx)
            return NULL;
        if (pE->index == idx)
            break;
        pE = pE->pTwin;
    } while (pE != NULL);

    return pE;
}


/*
 *  find entry support routines:
 *
 *  addResult:  place a new definition entry on the end of the
 *              list of found definitions (reallocating list size as needed).
 */
static void
addResult( tDefEntry* pDE, tDefEntryList* pDEL )
{
    if (++(pDEL->usedCt) > pDEL->allocCt) {
        pDEL->allocCt += pDEL->allocCt + 8; /* 8, 24, 56, ... */
        pDEL->papDefEntry = (tDefEntry**)
            AGREALOC( (void*)pDEL->papDefEntry, pDEL->allocCt * sizeof( void* ),
                      "added find result" );
    }

    pDEL->papDefEntry[ pDEL->usedCt-1 ] = pDE;
}


static size_t
badName( char* pzD, char const* pzS, size_t srcLen )
{
    memcpy( (void*)pzD, (void*)pzS, srcLen );
    pzD[ srcLen ] = NUL;
    fprintf( pfTrace, zNameRef, pzD,
             pCurTemplate->pzTplFile, pCurMacro->lineNo );
    return srcLen + 1;
}


/*
 *  canonicalizeName:  remove white space and roughly verify the syntax.
 *  This procedure will consume everything from the source string that
 *  forms a valid AutoGen compound definition name.
 *  We leave legally when:
 *  1.  the state is "CN_NAME_ENDED", AND
 *  2.  We stumble into a character that is not either '[' or '.'
 *      (always skipping white space).
 *  We start in CN_START.
 */
LOCAL int
canonicalizeName( char* pzD, char const* pzS, int srcLen )
{
    typedef enum {
        CN_START_NAME = 0,   /* must find a name */
        CN_NAME_ENDED,       /* must find '[' or '.' or we end */
        CN_INDEX,            /* must find name, number, '$' or ']' */
        CN_INDEX_CLOSE,      /* must find ']' */
        CN_INDEX_ENDED       /* must find '.' or we end */
    } teConState;

    teConState state = CN_START_NAME;

    char const* pzOri = pzS;
    char*       pzDst = pzD;
    size_t      stLen = srcLen;

    /*
     *  Before anything, skip a leading '.' as a special hack to force
     *  a current context lookup.
     */
    while (IS_WHITESPACE_CHAR(*pzS )) {
        if (--srcLen <= 0) {
            pzS = zNil;
            break;
        }
        pzS++;
    }

    if (*pzS == '.') {
        *(pzD++) = '.';
        pzS++;
    }

 nextSegment:
    /*
     *  The next segment may always start with an alpha character,
     *  but an index may also start with a number.  The full number
     *  validation will happen in findEntryByIndex().
     */
    while (IS_WHITESPACE_CHAR(*pzS )) {
        if (--srcLen <= 0) {
            pzS = zNil;
            break;
        }
        pzS++;
    }

    switch (state) {
    case CN_START_NAME:
        if (! IS_VAR_FIRST_CHAR(*pzS))
            return badName( pzDst, pzOri, stLen );
        state = CN_NAME_ENDED;  /* we found the start of our first name */
        break;  /* fall through to name/number consumption code */

    case CN_NAME_ENDED:
        switch (*pzS++) {
        case '[':
            *(pzD++) = '[';
            state = CN_INDEX;
            break;

        case '.':
            *(pzD++) = '.';
            state = CN_START_NAME;
            break;

        default:
            /* legal exit -- we have a name already */
            *pzD = NUL;
            return srcLen;
        }

        if (--srcLen <= 0)
            return badName( pzDst, pzOri, stLen );
        goto nextSegment;

    case CN_INDEX:
        /*
         *  An index.  Valid syntaxes are:
         *
         *    '[' <#define-d name> ']'
         *    '[' <number> ']'
         *    '['  '$'  ']'
         *    '['       ']'
         *
         *  We will check for and handle the last case right here.
         *  The next cycle must find the index closer (']').
         */
        state = CN_INDEX_CLOSE;

        /*
         *  Numbers and #define-d names are handled at the end of the switch.
         *  '$' and ']' are handled immediately below.
         */
        if (IS_ALPHANUMERIC_CHAR(*pzS))
            break;

        /*
         *  A solitary '$' is the highest index, whatever that happens to be
         *  We process that right here because down below we only accept
         *  name-type characters and this is not VMS.
         */
        if (*pzS == '$') {
            if (--srcLen < 0)
                return badName( pzDst, pzOri, stLen );

            *(pzD++) = *(pzS++);
            goto nextSegment;
        }
        /* FALLTHROUGH */

    case CN_INDEX_CLOSE:
        /*
         *  Nothing else is okay.
         */
        if ((*(pzD++) = *(pzS++)) != ']')
            return badName( pzDst, pzOri, stLen );

        if (--srcLen <= 0) {
            *pzD = NUL;
            return srcLen;
        }
        state = CN_INDEX_ENDED;
        goto nextSegment;

    case CN_INDEX_ENDED:
        if ((*pzS != '.') || (--srcLen < 0)) {
            *pzD = NUL;
            return srcLen;
        }
        *(pzD++) = *(pzS++);

        state = CN_START_NAME;
        goto nextSegment;
    }

    /*
     *  The next state must be either looking for what comes after the
     *  end of a name, or for the close bracket after an index.
     *  Whatever, the next token must be a name or a number.
     */
    assert((state == CN_NAME_ENDED) || (state == CN_INDEX_CLOSE));
    assert( IS_ALPHANUMERIC_CHAR(*pzS));

    /*
     *  Copy the name/number.  We already know the first character is valid.
     *  However, we must *NOT* downcase #define names...
     */
    while (IS_VALUE_NAME_CHAR(*pzS)) {
        char ch = *(pzS++);
        if ((state != CN_INDEX_CLOSE) && IS_UPPER_CASE_CHAR(ch))
            *(pzD++) = tolower( ch );

        else switch ( ch ) { /* force the separator chars to be '_' */
        case '-':
        case '^':
            *(pzD++) = '_';
            break;

        default:
            *(pzD++) = ch;
        }

        if (--srcLen <= 0) {
            pzS = zNil;
            break;
        }
    }

    goto nextSegment;
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
static tDefEntry*
defEntrySearch( char* pzName, tDefCtx* pDefCtx, ag_bool* pIsIndexed )
{
    char*        pcBrace;
    char         breakCh;
    tDefEntry*   pE;
    ag_bool      dummy;
    ag_bool      noNesting    = AG_FALSE;

    static int   nestingDepth = 0;

    /*
     *  IF we are at the start of a search, then canonicalize the name
     *  we are hunting for, copying it to a modifiable buffer, and
     *  initialize the "indexed" boolean to false (we have not found
     *  an index yet).
     */
    if (nestingDepth == 0) {
        canonicalizeName( zDefinitionName, pzName, (int)strlen( pzName ));
        pzName = zDefinitionName;

        if (pIsIndexed != NULL)
             *pIsIndexed = AG_FALSE;
        else pIsIndexed  = &dummy;

        if (*pzName == '.') {
            noNesting = AG_TRUE;
            pzName++;
        }
    }

    pcBrace  = pzName + strcspn( pzName, "[." );
    breakCh  = *pcBrace;
    *pcBrace = NUL;

    if (breakCh == '[') *pIsIndexed = AG_TRUE;

    for (;;) {
        /*
         *  IF we are at the end of the definitions (reached ROOT),
         *  THEN it is time to bail out.
         */
        pE = pDefCtx->pDefs;
        if (pE == NULL)
            return NULL;

        do  {
            /*
             *  IF the name matches
             *  THEN break out of the double loop
             */
            if (strcmp( pE->pzDefName, pzName ) == 0)
                goto found_def_entry;

            pE = pE->pNext;
        } while (pE != NULL);

        /*
         *  IF we are nested, then we cannot change the definition level.
         *  So, we did not find anything.
         */
        if ((nestingDepth != 0) || noNesting)
            return NULL;

        /*
         *  Let's go try the definitions at the next higher level.
         */
        pDefCtx = pDefCtx->pPrev;
        if (pDefCtx == NULL)
            return NULL;
    } found_def_entry:;

    /*
     *  At this point, we have found the entry that matches the supplied
     *  name, up to the '[' or '.' or NUL character.  It *must* be one of
     *  those three characters.
     */
    *pcBrace = breakCh;

    switch (breakCh) {
    case NUL:
        return pE;

    case '[':
        /*
         *  We have to find a specific entry in a list.
         */
        while (IS_WHITESPACE_CHAR(*++pcBrace )) ;

        pE = findEntryByIndex( pE, pcBrace );
        if (pE == NULL)
            return pE;

        /*
         *  We must find the closing brace, or there is an error
         */
        pcBrace = strchr( pcBrace, ']' );
        if (pcBrace == NULL)
            ILLFORMEDNAME();

        /*
         *  IF we are at the end of the definition,
         *  THEN return what we found
         */
        switch (*++pcBrace) {
        case NUL:
            return pE;

        case '.':
            break;

        default:
            ILLFORMEDNAME();
        }
        /* FALLTHROUGH */

    case '.':
        /*
         *  It is a segmented value name.  Set the name pointer
         *  to the next segment and search starting from the newly
         *  available set of definitions.
         */
        pzName = pcBrace + 1;
        break;

    default:
        ILLFORMEDNAME();
    }

    /*
     *  We cannot find a member of a non-block type macro definition.
     */
    if (pE->valType != VALTYP_BLOCK)
        return NULL;

    /*
     *  Loop through all the twins of the entry we found until
     *  we find the entry we want.  We ignore twins if we just
     *  used a subscript.
     */
    nestingDepth++;
    {
        tDefCtx ctx = { NULL, &currDefCtx };

        ctx.pDefs = pE->val.pDefEntry;

        for (;;) {
            tDefEntry* res;

            res = defEntrySearch( pzName, &ctx, pIsIndexed );
            if ((res != NULL) || (breakCh == '[')) {
                nestingDepth--;
                return res;
            }
            pE = pE->pTwin;
            if (pE == NULL)
                break;
            ctx.pDefs = pE->val.pDefEntry;
        }
    }

    nestingDepth--;
    return NULL;
}


LOCAL tDefEntry*
findDefEntry( char* pzName, ag_bool* pIsIndexed )
{
    return defEntrySearch( pzName, &currDefCtx, pIsIndexed );
}


/*
 *  entryListSearch
 *
 *  Find the definition entry for the name passed in.  It is okay to find
 *  block entries IFF they are found on the current level.  Once you start
 *  traversing up the tree, the macro must be a text macro.  Return an
 *  indicator saying if the element has been indexed (so the caller will
 *  not try to traverse the list of twins).
 */
static tDefEntry**
entryListSearch( char* pzName, tDefCtx* pDefCtx )
{
    static tDefEntryList defList = { 0, 0, NULL, 0 };

    char*      pcBrace;
    char       breakCh;
    tDefEntry* pE;
    ag_bool    noNesting = AG_FALSE;

    /*
     *  IF we are at the start of a search, then canonicalize the name
     *  we are hunting for, copying it to a modifiable buffer, and
     *  initialize the "indexed" boolean to false (we have not found
     *  an index yet).
     */
    if (defList.nestLevel == 0) {
        if (! IS_ALPHANUMERIC_CHAR(*pzName)) {
            strncpy(zDefinitionName, pzName, sizeof(zDefinitionName) - 1);
            zDefinitionName[ sizeof(zDefinitionName) - 1] = NUL;
            ILLFORMEDNAME();
            return NULL;
        }

        canonicalizeName( zDefinitionName, pzName, (int)strlen( pzName ));
        pzName = zDefinitionName;
        defList.usedCt = 0;

        if (*pzName == '.') {
            noNesting = AG_TRUE;
            pzName++;
        }
    }

    pcBrace  = pzName + strcspn( pzName, "[." );
    breakCh  = *pcBrace;
    *pcBrace = NUL;

    for (;;) {
        /*
         *  IF we are at the end of the definitions (reached ROOT),
         *  THEN it is time to bail out.
         */
        pE = pDefCtx->pDefs;
        if (pE == NULL) {
            /*
             *  Make sure we are not nested.  Once we start to nest,
             *  then we cannot "change definition levels"
             */
        not_found:
            if (defList.nestLevel != 0)
                ILLFORMEDNAME();

            /*
             *  Don't bother returning zero entry list.  Just return NULL.
             */
            return NULL;
        }

        do  {
            /*
             *  IF the name matches
             *  THEN go add it, plus all its twins
             */
            if (strcmp( pE->pzDefName, pzName ) == 0)
                goto found_def_entry;

            pE = pE->pNext;
        } while (pE != NULL);

        /*
         *  IF we are nested, then we cannot change the definition level.
         *  Just go and return what we have found so far.
         */
        if ((defList.nestLevel != 0) || noNesting)
            goto returnResult;

        /*
         *  Let's go try the definitions at the next higher level.
         */
        pDefCtx = pDefCtx->pPrev;
        if (pDefCtx == NULL)
            goto not_found;
    } found_def_entry:;

    /*
     *  At this point, we have found the entry that matches the supplied
     *  name, up to the '[' or '.' or NUL character.  It *must* be one of
     *  those three characters.
     */
    *pcBrace = breakCh;

    switch (breakCh) {
    case NUL:
        do  {
            addResult( pE, &defList );
            pE = pE->pTwin;
        } while (pE != NULL);
        goto returnResult;

    case '[':
        /*
         *  We have to find a specific entry in a list.
         */
        while (IS_WHITESPACE_CHAR(*++pcBrace)) ;

        pE = findEntryByIndex( pE, pcBrace );
        if (pE == NULL)
            goto returnResult;

        /*
         *  We must find the closing brace, or there is an error
         */
        pcBrace = strchr( pcBrace, ']' );
        if (pcBrace == NULL)
            ILLFORMEDNAME();

        /*
         *  IF we are at the end of the definition,
         *  THEN return what we found
         */
        switch (*++pcBrace) {
        case NUL:
            goto returnResult;

        case '.':
            break;

        default:
            ILLFORMEDNAME();
        }
        /* FALLTHROUGH */

    case '.':
        /*
         *  It is a segmented value name.  Set the name pointer
         *  to the next segment and search starting from the newly
         *  available set of definitions.
         */
        pzName = pcBrace + 1;
        break;

    default:
        ILLFORMEDNAME();
    }

    /*
     *  We cannot find a member of a non-block type macro definition.
     */
    if (pE->valType != VALTYP_BLOCK)
        return NULL;

    /*
     *  Loop through all the twins of the entry.  We ignore twins if we just
     *  used a subscript.
     */
    defList.nestLevel++;
    {
        tDefCtx ctx = { NULL, &currDefCtx };

        ctx.pDefs = pE->val.pDefEntry;

        for (;;) {
            (void)entryListSearch( pzName, &ctx );
            if (breakCh == '[')
                break;
            pE = pE->pTwin;
            if (pE == NULL)
                break;
            ctx.pDefs = pE->val.pDefEntry;
        }
    }
    defList.nestLevel--;

 returnResult:
    if (defList.nestLevel == 0)
        addResult( NULL, &defList );

    return defList.papDefEntry;
}


LOCAL tDefEntry**
findEntryList( char* pzName )
{
    return entryListSearch( pzName, &currDefCtx );
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/defFind.c */
