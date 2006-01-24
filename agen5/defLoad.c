/*
 *  $Id: defLoad.c,v 4.9 2006/01/24 21:29:19 bkorb Exp $
 *  This module loads the definitions, calls yyparse to decipher them,
 *  and then makes a fixup pass to point all children definitions to
 *  their parent definition.
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

static tDefEntry* pFreeEntryList = NULL;
static void*      pAllocList     = NULL;

#define ENTRY_SPACE        (4096 - sizeof(void*))
#define ENTRY_ALLOC_CT     (ENTRY_SPACE / sizeof(tDefEntry))
#define ENTRY_ALLOC_SIZE   \
    ((ENTRY_ALLOC_CT * sizeof( tDefEntry )) + sizeof(void*))

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static tDefEntry*
insertDef( tDefEntry* pDef );
/* = = = END-STATIC-FORWARD = = = */

LOCAL void
freeEntry( tDefEntry* pDE )
{
    pDE->pNext = pFreeEntryList;
    pFreeEntryList = pDE;
}


LOCAL tDefEntry*
getEntry( void )
{
    tDefEntry*  pRes = pFreeEntryList;

    if (pRes != NULL) {
        pFreeEntryList = pRes->pNext;

    } else {
        int   ct = ENTRY_ALLOC_CT-1;
        void* p  = AGALOC( ENTRY_ALLOC_SIZE, "definition headers" );

        *((void**)p) = pAllocList;
        pAllocList   = p;
        pRes = pFreeEntryList = (tDefEntry*)((void**)p + 1);

        /*
         *  This is a post-loop test loop.  It will cycle one fewer times
         *  than there are 'tDefEntry' structs in the memory we just alloced.
         */
        do  {
            tDefEntry* pNxt = pRes+1;
            pRes->pNext = pNxt;

            /*
             *  When the loop ends, "pRes" will point to the last allocated
             *  structure instance.  That is the one we will return.
             */
            pRes = pNxt;
        } while (--ct > 0);

        /*
         *  Unlink the last entry from the chain.  The next time this
         *  routine is called, the *FIRST* structure in this list will
         *  be returned.
         */
        pRes[-1].pNext = NULL;
    }

    memset( (void*)pRes, 0, sizeof( *pRes ));
    return pRes;
}


/*
 *  Append a new entry at the end of a sibling (or twin) list.
 */
static tDefEntry*
insertDef( tDefEntry* pDef )
{
    tDefEntry* pList = ppParseStack[ stackDepth ];

    /*
     *  If the current level is empty, then just insert this one and quit.
     */
    if (pList->val.pDefEntry == NULL) {
        if (pDef->index == NO_INDEX)
            pDef->index = 0;
        pList->val.pDefEntry = pDef;
        return pDef;
    }
    pList = pList->val.pDefEntry;

    /*
     *  Scan the list looking for a "twin" (same-named entry).
     */
    while (strcmp( pDef->pzDefName, pList->pzDefName ) != 0) {
        /*
         *  IF we are at the end of the list,
         *  THEN put the new entry at the end of the list.
         *       This is a new name in the current context.
         *       The value type is forced to be the same type.
         */
        if (pList->pNext == NULL) {
            pList->pNext = pDef;

            if (pDef->index == NO_INDEX)
                pDef->index = 0;
            return pDef;
        }

        /*
         *  Check the next sibling for a twin value.
         */
        pList = pList->pNext;
    }

    /*  * * * * *  WE HAVE FOUND A TWIN
     *
     *  Link in the new twin chain entry into the list.
     */
    if (pDef->index == NO_INDEX) {
        tDefEntry* pT = pList->pEndTwin;
        if (pT == NULL)
            pT = pList;

        pDef->index = pT->index + 1;
        pT->pTwin = pDef;
        pDef->pPrevTwin = pT;
        pList->pEndTwin = pDef;

    } else if (pList->index > pDef->index) {

        /*
         *  Insert the new entry before any other in the list.
         *  We actually do this by leaving the pList pointer alone and swapping
         *  the contents of the definition entry.
         */
        tDefEntry def = *pDef;

        memcpy( &(pDef->pzDefName), &(pList->pzDefName),
                sizeof( def ) - ag_offsetof(tDefEntry, pzDefName) );

        memcpy( &(pList->pzDefName), &(def.pzDefName),
                sizeof( def ) - ag_offsetof(tDefEntry, pzDefName) );

        /*
         *  Contents are swapped.  Link "pDef" after "pList" and return "pList".
         */
        pDef->pTwin = pList->pTwin;
        if (pDef->pTwin != NULL)
            pDef->pTwin->pPrevTwin = pDef;

        pDef->pPrevTwin = pList;
        pList->pTwin  = pDef;

        /*
         *  IF this is the first twin, then the original list head is now
         *  the "end twin".
         */
        if (pList->pEndTwin == NULL)
            pList->pEndTwin = pDef;

        pDef = pList;  /* Return the replacement structure address */

    } else {
        tDefEntry* pL = pList;
        tDefEntry* pT = pL->pTwin;

        /*
         *  Insert someplace after the first entry.  Scan the list until
         *  we either find a larger index or we get to the end.
         */
        while ((pT != NULL) && (pT->index < pDef->index)) {
            pL = pT;
            pT = pT->pTwin;
        }

        pDef->pTwin = pT;

        pL->pTwin = pDef;
        pDef->pPrevTwin = pL;
        if (pT == NULL)
            pList->pEndTwin = pDef;
        else
            pT->pPrevTwin = pDef;
    }

    return pDef; /* sometimes will change */
}


LOCAL tDefEntry*
findPlace( char* name, tCC* pzIndex )
{
    tDefEntry* pE = getEntry();

    pE->pzDefName = name;

    if (pzIndex == NULL)
        pE->index = NO_INDEX;

    else if (isdigit( *pzIndex ) || (*pzIndex == '-'))
        pE->index = strtol( pzIndex, NULL, 0 );

    else {
        pzIndex = getDefine( pzIndex, AG_TRUE );
        if (pzIndex != NULL)
             pE->index = strtol( pzIndex, NULL, 0 );
        else pE->index = NO_INDEX;
    }

    strtransform( pE->pzDefName, pE->pzDefName );
    pE->valType     = VALTYP_UNKNOWN;
    pE->pzSrcFile   = (char*)pCurCtx->pzCtxFname;
    pE->srcLineNum  = pCurCtx->lineNo;
    return (pCurrentEntry = insertDef( pE ));
}

/*
 *  readDefines
 *
 *  Suck in the entire definitions file and parse it.
 */
LOCAL void
readDefines( void )
{
    tCC*     pzDefFile;
    char*    pzData;
    size_t   dataSize;
    size_t   sizeLeft;
    ag_bool  useStdin;
    FILE*    fp;

    if (! ENABLED_OPT( DEFINITIONS )) {
        pBaseCtx = (tScanCtx*)AGALOC( sizeof( tScanCtx ), "scan context" );
        memset( (void*)pBaseCtx, 0, sizeof( tScanCtx ));
        pBaseCtx->lineNo     = 1;
        pBaseCtx->pzCtxFname = "@@ No-Definitions @@";
        return;
    }

    pzDefFile = OPT_ARG( DEFINITIONS );

    /*
     *  Check for stdin as the input file.  We use the current time
     *  as the modification time for stdin.  We also note it so we
     *  do not try to open it and we try to allocate more memory if
     *  the stdin input exceeds our initial allocation of 16K.
     */
    if (strcmp( pzDefFile, "-" ) == 0) {
        pzDefFile = OPT_ARG( DEFINITIONS ) = "stdin";
        if (getenv( "REQUEST_METHOD" ) != NULL) {
            loadCgi();
            pCurCtx = pBaseCtx;
            dp_run_fsm();
            return;
        }

        outTime  = time( NULL );
        dataSize = 0x4000 - (4+sizeof( *pBaseCtx ));
        useStdin = AG_TRUE;
    }

    /*
     *  This, then, must be a regular file.  Make sure of that and
     *  find out how big it was and when it was last modified.
     */
    else {
        struct stat stbf;

        if (stat( pzDefFile, &stbf ) != 0)
            AG_ABEND( aprf( zCannot, errno, "stat",
                            pzDefFile, strerror( errno )));

        if (! S_ISREG( stbf.st_mode )) {
            errno = EINVAL;
            AG_ABEND( aprf( zCannot, errno, "open non-regular file",
                            pzDefFile, strerror( errno )));
        }

        /*
         *  IF the source-time option has been enabled, then
         *  our output file mod time will start as one second after
         *  the mod time on this file.  If any of the template files
         *  are more recent, then it will be adjusted.
         */
        dataSize = stbf.st_size;

        if (ENABLED_OPT( SOURCE_TIME ))
             outTime = stbf.st_mtime + 1;
        else outTime = time( NULL );

        useStdin = AG_FALSE;
    }

    /*
     *  Allocate the space we need for our definitions.
     */
    sizeLeft = dataSize+4+sizeof( *pBaseCtx );
    pBaseCtx = (tScanCtx*)AGALOC( sizeLeft, "file buffer" );
    memset( (void*)pBaseCtx, 0, sizeLeft );
    pBaseCtx->lineNo     = 1;
    sizeLeft = dataSize;

    /*
     *  Our base context will have its currency pointer set to this
     *  input.  It is also a scanning pointer, but since this buffer
     *  is never deallocated, we do not have to remember the initial
     *  value.  (It may get reallocated here in this routine, tho...)
     */
    pzData =
        pBaseCtx->pzScan =
        pBaseCtx->pzData = (char*)(pBaseCtx+1);
    pBaseCtx->pCtx = NULL;

    /*
     *  Set the input file pointer, as needed
     */
    if (useStdin)
        fp = stdin;
    else {
        fp = fopen( pzDefFile, "r" FOPEN_TEXT_FLAG );
        if (fp == NULL)
            AG_ABEND( aprf( zCannot, errno, "open",
                            pzDefFile, strerror( errno )));
    }

    /*
     *  Read until done...
     */
    for (;;) {
        size_t rdct = fread( (void*)pzData, 1, sizeLeft, fp );

        /*
         *  IF we are done,
         */
        if (rdct == 0) {
            /*
             *  IF it is because we are at EOF, then break out
             *  ELSE abend.
             */
            if (feof( fp ) || useStdin)
                break;

            AG_ABEND( aprf( zCannot, errno, "read",
                            pzDefFile, strerror( errno )));
        }

        /*
         *  Advance input pointer, decrease remaining count
         */
        pzData   += rdct;
        sizeLeft -= rdct;

        /*
         *  See if there is any space left
         */
        if (sizeLeft == 0) {
            tScanCtx* p;

            /*
             *  IF it is a regular file, then we are done
             */
            if (! useStdin)
                break;

            /*
             *  We have more data and we are out of space.
             *  Try to reallocate our input buffer.
             */
            dataSize += (sizeLeft = 0x1000);
            p = (tScanCtx*)AGREALOC( (void*)pBaseCtx,
                                     dataSize+4+sizeof( *pBaseCtx ),
                                     "expanded file buffer" );

            /*
             *  The buffer may have moved.  Set the data pointer at an
             *  offset within the new buffer and make sure our base pointer
             *  has been corrected as well.
             */
            if (p != pBaseCtx) {
                p->pzScan = \
                p->pzData = (char*)(p+1);
                pzData = p->pzData + (pzData - pBaseCtx->pzData);
                pBaseCtx = p;
            }
        }
    }

    if (pzData == pBaseCtx->pzData)
        AG_ABEND( "No definition data were read" );

    *pzData = NUL;
    AGDUPSTR( pBaseCtx->pzCtxFname, pzDefFile, "def file name" );

    /*
     *  Close the input file, parse the data
     *  and alphabetically sort the definition tree contents.
     */
    if (! useStdin)
        fclose( fp );

    pCurCtx = pBaseCtx;
    dp_run_fsm();
}


LOCAL void
unloadDefs( void )
{
    return;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/defLoad.c */
