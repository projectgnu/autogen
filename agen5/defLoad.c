/*
 *  agGetDef.c
 *  $Id: defLoad.c,v 1.3 1999/11/24 23:30:12 bruce Exp $
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


    STATIC int
compareIndex( const void* p1, const void* p2 )
{
    tDefEntry* pE1 = *((tDefEntry**)p1);
    tDefEntry* pE2 = *((tDefEntry**)p2);
    int  res = pE1->index - pE2->index;
    if (res == 0) {
        fprintf( stderr, "ERROR: two %s entries have index %ld\n",
                 pE1->pzName, pE1->index );
        AG_ABEND;
    }
    return res;
}


/*
 *  fixTwins
 *
 *  If the twins are unnumbered, then we assign numbers.
 *  Once they are all numbered, then we sort them.
 *  Once they are all sorted, then we link them together
 *  in that order.
 */
    STATIC void
fixTwins( tDefEntry** ppNode )
{
    long curMax = -1;
    int  twinCt = 0;
    void** p;
    int  idx;
    tDefEntry* pTw  = *ppNode;
    tDefEntry* pNxt = pTw->pNext;
    void*  ap[ 16 ];

    pTw->pNext = (tDefEntry*)NULL;

    /*
     *  Do the initial twin scan, assigning index numbers as needed
     */
    do  {
        if (pTw->index == NO_INDEX)
            pTw->index = ++curMax;

        else if (pTw->index > curMax)
            curMax = pTw->index;

        twinCt++;
        pTw = pTw->pTwin;
    } while (pTw != (tDefEntry*)NULL);

    /*
     *  Try to avoid allocating a pointer array,
     *  but do so if we have to to avoid arbitrary limits.
     */
    if (twinCt <= 16) {
        p = ap;
    } else {
        p = AGALOC( twinCt * sizeof( void* ));

        if (p == (void**)NULL) {
            fprintf( stderr, zAllocErr, pzProg,
                     twinCt * sizeof( void* ), "twin sort pointer array" );
            AG_ABEND;
        }
    }

    /*
     *  Sort all the twins by their index number.
     *  Reinsert the "next" pointer into the new first entry
     */
    for (pTw = *ppNode, idx = 0;
         pTw != (tDefEntry*)NULL;
         idx++, pTw = pTw->pTwin) {
        p[idx] = (void*)pTw;
    }

    qsort( (void*)p, twinCt, sizeof( void* ), compareIndex );
    ((tDefEntry*)(p[0]))->pNext = pNxt;

    /*
     *  Chain them all together in their sorted order,
     *  NULL terminating the list
     */
    for (idx = 0; idx < twinCt; idx++) {
        *ppNode = (tDefEntry*)p[idx];
        ppNode = &((tDefEntry*)(p[idx]))->pTwin;
    }
    *ppNode = (tDefEntry*)NULL;

    /*
     *  If we allocated the pointer array, dump it now
     */
    if (p != ap)
        AGFREE( (void*)p );
}


/*
 *  massageDefTree
 *
 *  The agReduce.c functions do not do everything.
 *  It stuffs all the definitins into the definition tree,
 *  but it is easier to come back at the end and link
 *  together the "twins" (definitions with the same name,
 *  but different indexes).  This routine does that.
 *  It is recursive, handling one level at a time.
 */
    STATIC void
massageDefTree( tDefEntry** ppNode, tDefEntry* pEldestUncle )
{
    static int lvl = 0;
    char zFmt[ 64 ];
    tDefEntry*  pNode;
    tDefEntry*  pEldestSib = *ppNode;

#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_DEFS )
    if (HAVE_OPT( SHOW_DEFS ))
        sprintf( zFmt, "%%%dd %%-%ds = ", 3 + (lvl * 2), 20 - (lvl * 2) );
#endif

    do  {
        tDefEntry** ppNextSib  = (tDefEntry**)NULL;

        pNode = *ppNode;

        /*
         *  IF this node has a twin (definition with same name)
         *  THEN ...
         */
        if (pNode->pTwin != (tDefEntry*)NULL) {
            fixTwins( ppNode );
            pNode = *ppNode;  /* new first entry */

            /*
             *  Now go through the twin list and put in the reverse
             *  pointers.  (We can go through _FOR loops in either
             *  direction.)
             */
            {
                tDefEntry* pNextTwin = pNode->pTwin;
                tDefEntry* pPrevTwin = pNode;
                do  {
                    pNextTwin->pPrevTwin = pPrevTwin;
                    pPrevTwin = pNextTwin;
                    pNextTwin = pNextTwin->pTwin;
                } while (pNextTwin != (tDefEntry*)NULL);

                /*
                 *  The eldest twin has a baby twin pointer
                 */
                pNode->pEndTwin = pPrevTwin;
            }

        } else if (pNode->index == NO_INDEX) {
            /*
             *  No twins and no index means an index of zero
             */
            pNode->index = 0;
        }

        ppNode = &(pNode->pNext);

    skipTwinFix:
#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_DEFS )
        if (HAVE_OPT(SHOW_DEFS))
            printf( zFmt, pNode->index, pNode->pzName );
#endif
        pNode->pDad = pEldestUncle;

        if (pNode->valType == VALTYP_BLOCK) {
#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_DEFS )
            if (HAVE_OPT(SHOW_DEFS))
                fputs( "{...}\n", stdout );
#endif
            /*
             *  Do this same function on all the children of this
             *  block definition.
             */
            lvl++;
            massageDefTree( (tDefEntry**)&(pNode->pzValue), pEldestSib );
            lvl--;
        }

#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_DEFS )
        /*
         *  IF we are displaying definitions,
         *  THEN show what we can on a single line for this text definition
         */
        else if (HAVE_OPT(SHOW_DEFS)) {
            char* pz = pNode->pzValue;
            int   ct = 32;
            while (isspace( *pz )) pz++;
            for (;;) {
                char ch = *(pz++);
                if (ch == NUL) {
                    fputc( '\n', stdout );
                    break;
                }
                fputc( ch, stdout );
                if (ch == '\n')
                    break;
                if (--ct == 0) {
                    fputc( '\n', stdout );
                    break;
                }
            }
        }
#endif

        /*
         *  IF we have to deal with a twin,
         *  THEN remember who the next sibling is and handle the twin
         */
        if (pNode->pTwin != (tDefEntry*)NULL) {
            /*
             *  IF we do not already have a next sibling pointer,
             *  THEN save it now.  We will clear this pointer when
             *       we run out of twins.
             */
            if (ppNextSib == (tDefEntry**)NULL)
                ppNextSib = ppNode;

            pNode = pNode->pTwin;
            goto skipTwinFix;
        }

        /*
         *  IF we stashed a next sibling pointer in order to handle
         *  twins, THEN resume the scan from that point
         */
        if (ppNextSib != (tDefEntry**)NULL)
            ppNode = ppNextSib;

    } while (*ppNode != (tDefEntry*)NULL);
}


/*
 *  readDefines
 *
 *  Suck in the entire definitions file and parse it.
 */
    void
readDefines( void )
{
    char*    pzData;
    size_t   dataSize;
    size_t   sizeLeft;
    ag_bool  useStdin;
    FILE*    fp;

    /*
     *  Start our definitions tree with a magical root
     */
    memset( (void*)&rootEntry, 0, sizeof( rootEntry ));
    rootEntry.pzName  = "@@ROOT@@";
    rootEntry.valType = VALTYP_BLOCK;
    if (! ENABLED_OPT( DEFINITIONS )) {
        pBaseCtx = (tScanCtx*)AGALOC( sizeof( tScanCtx ));
        memset( (void*)pBaseCtx, 0, sizeof( tScanCtx ));
        pBaseCtx->pzFileName = "@@ No-Definitions @@";
        return;
    }

    /*
     *  Check for stdin as the input file.  We use the current time
     *  as the modification time for stdin.  We also note it so we
     *  do not try to open it and we try to allocate more memory if
     *  the stdin input exceeds our initial allocation of 16K.
     */
    else if (strcmp( OPT_ARG( DEFINITIONS ), "-" ) == 0) {
        OPT_ARG( DEFINITIONS )  = "stdin";
        outTime  = time( (time_t*)NULL );
        sizeLeft = dataSize = 0x4000 - (4+sizeof( *pBaseCtx ));
        useStdin = AG_TRUE;
    }

    /*
     *  This, then, must be a regular file.  Make sure of that and
     *  find out how big it was and when it was last modified.
     */
    else {
        struct stat stbf;
        if (stat( OPT_ARG( DEFINITIONS ), &stbf ) != 0) {
            fprintf( stderr, "%s ERROR %d (%s) stat-ing %s\n",
                     pzProg, errno, strerror( errno ),
                     OPT_ARG( DEFINITIONS ) );
            AG_ABEND;
        }
        if (! S_ISREG( stbf.st_mode )) {
            fprintf( stderr, "%s ERROR:  %s is not a regular file\n",
                     pzProg, OPT_ARG( DEFINITIONS ) );
            AG_ABEND;
        }

        /*
         *  Our output file mod time will start as one second after
         *  the mod time on this file.  If any of the template files
         *  are more recent, then it will be adjusted.
         */
        sizeLeft = dataSize = stbf.st_size;
        outTime  = stbf.st_mtime + 1;
        useStdin = AG_FALSE;
    }

    /*
     *  Allocate the space we need for our definitions.
     */
    pBaseCtx = (tScanCtx*)AGALOC( dataSize+4+sizeof( *pBaseCtx ));
    if (pBaseCtx == (tScanCtx*)NULL) {
        fprintf( stderr, zAllocErr, pzProg,
                 dataSize+4+sizeof( *pBaseCtx ), "file buffer" );
        AG_ABEND;
    }

    /*
     *  Our base context will have its currency pointer set to this
     *  input.  It is also a scanning pointer, but since this buffer
     *  is never deallocated, we do not have to remember the initial
     *  value.  (It may get reallocated here in this routine, tho...)
     */
    pzData =
        pBaseCtx->pzScan =
        pBaseCtx->pzData = (char*)(pBaseCtx+1);
    pBaseCtx->pCtx = (tScanCtx*)NULL;

    /*
     *  Set the input file pointer, as needed
     */
    fp = useStdin ? stdin : fopen( OPT_ARG( DEFINITIONS ), "r" FOPEN_TEXT_FLAG );
    if (fp == (FILE*)NULL) {
        fprintf( stderr, "%s ERROR %d (%s):  cannot open %s\n",
                 pzProg, errno, strerror( errno ),
                 OPT_ARG( DEFINITIONS ) );
        AG_ABEND;
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
            if (feof( fp ))
                break;

            fprintf( stderr, "%s ERROR %d (%s):  cannot read %d bytes "
                     "from %s\n", pzProg, errno,
                     strerror( errno ), dataSize, OPT_ARG( DEFINITIONS ) );
            AG_ABEND;
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
                                     dataSize+4+sizeof( *pBaseCtx ));
            if (p == (tScanCtx*)NULL) {
                fprintf( stderr, zAllocErr, pzProg,
                         dataSize+4+sizeof( *pBaseCtx ),
                         "expanded file buffer" );
                AG_ABEND;
            }

            /*
             *  The buffer may have moved.  Set the data pointer at an
             *  offset within the new buffer and make sure our base pointer
             *  has been corrected as well.
             */
            if (p != pBaseCtx) {
                p->pzScan =
                    p->pzData = (char*)(p+1);
                pzData = p->pzData + (pzData - pBaseCtx->pzData);
                pBaseCtx = p;
            }
        }
    }

    *pzData = NUL;
    AGDUPSTR( pBaseCtx->pzFileName, OPT_ARG( DEFINITIONS ) );

    /*
     *  Close the input file, parse the data
     *  and alphabetically sort the definition tree contents.
     */
    if (fp != stdin)
        fclose( fp );
    pCurCtx = pBaseCtx;
    {
        extern int yyparse( void );
        (void)yyparse();
    }
    {
        tDefEntry* pRoot = &rootEntry;
        massageDefTree( &pRoot, (tDefEntry*)NULL );
    }
}
/* end of defLoad.c */
