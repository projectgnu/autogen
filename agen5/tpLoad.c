
/*
 *  $Id: tpLoad.c,v 3.16 2003/04/19 02:40:33 bkorb Exp $
 *
 *  This module will load a template and return a template structure.
 */

/*
 *  AutoGen copyright 1992-2003 Bruce Korb
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

#include <sys/param.h>
#include <fcntl.h>

#include "autogen.h"

static tTlibMark magicMark = TEMPLATE_MAGIC_MARKER;

STATIC size_t countMacros( tCC* pz );


EXPORT tTemplate*
findTemplate( tCC* pzTemplName )
{
    tTemplate* pT = pNamedTplList;
    while (pT != NULL) {
        if (streqvcmp( pzTemplName, pT->pzTplName ) == 0)
            break;
        pT = (tTemplate*)(pT->pNext);
    }
    return pT;
}

STATIC ag_bool
canReadFile( tCC* pzFName )
{
    struct stat stbf;
    if (stat( pzFName, &stbf ) != 0)
        return AG_FALSE;
    if (! S_ISREG( stbf.st_mode ))
        return AG_FALSE;
    return (access( pzFName, R_OK) == 0);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
EXPORT tSuccess
findFile( tCC* pzFName, char* pzFullName, tCC** papSuffixList )
{
    char*   pzRoot;
    char*   pzSfx;
    void*   deallocAddr = NULL;

    tSuccess res = SUCCESS;

    /*
     *  Expand leading environment variables.
     *  We will not mess with embedded ones.
     */
    if (*pzFName == '$') {
        if (! optionMakePath( pzFullName, MAXPATHLEN, pzFName,
                              autogenOptions.pzProgPath ))
            return FAILURE;

        AGDUPSTR( pzFName, pzFullName, "find file name" );
        deallocAddr = (void*)pzFName;

        /*
         *  pzFName now points to the name the file system can use.
         *  It must _not_ point to pzFullName because we will likely
         *  rewrite that value using this pointer!
         */
    }

    /*
     *  Check for an immediately accessible file
     */
    if (canReadFile( pzFName )) {
        strlcpy( pzFullName, pzFName, MAXPATHLEN );
        goto findFileReturn;
    }

    /*
     *  Not a complete file name.  If there is not already
     *  a suffix for the file name, then append ".tpl".
     *  Check for immediate access once again.
     */
    pzRoot = strrchr( pzFName, '/' );
    pzSfx  = (pzRoot != NULL)
             ? strchr( ++pzRoot, '.' )
             : strchr( pzFName, '.' );

    /*
     *  IF the file does not already have a suffix,
     *  THEN try the suffixes that are okay for this file.
     */
    if ((pzSfx == NULL) && (papSuffixList != NULL)) {
        tCC** papSL = papSuffixList;

        char* pzEnd = pzFullName +
            snprintf( pzFullName, MAXPATHLEN-MAX_SUFFIX_LEN, "%s.", pzFName );

        do  {
            strcpy( pzEnd, *(papSL++) ); /* must fit */
            if (canReadFile( pzFullName )) {
                goto findFileReturn;
            }
        } while (*papSL != NULL);
    }

    /*
     *  IF the file name does not contain a directory separator,
     *  then we will use the TEMPL_DIR search list to keep hunting.
     */
    if (pzRoot == NULL) {

        /*
         *  Search each directory in our directory search list
         *  for the file.  We always force one copy of this option.
         */
        int     ct     = STACKCT_OPT(  TEMPL_DIRS );
        char**  ppzDir = STACKLST_OPT( TEMPL_DIRS ) + ct - 1;

        do  {
            tSCC    zDirFmt[] = "%s/%s";
            char*   pzDir  = *(ppzDir--);

            char*   pzEnd;

            /*
             *  IF one of our template paths starts with '$', then expand it.
             */
            if (*pzDir == '$') {
                if (! optionMakePath( pzFullName, MAXPATHLEN, pzDir,
                                      autogenOptions.pzProgPath ))
                    pzDir = ".";
                else
                    AGDUPSTR( pzDir, pzFullName, "find directory name" );

                ppzDir[1] = pzDir; /* save the computed name for later */
            }

            pzEnd  = pzFullName
                + snprintf( pzFullName, MAXPATHLEN-MAX_SUFFIX_LEN,
                            zDirFmt, pzDir, pzFName );

            if (canReadFile( pzFullName ))
                goto findFileReturn;

            /*
             *  IF the file does not already have a suffix,
             *  THEN try the ones that are okay for this file.
             */
            if ((pzSfx == NULL) && (papSuffixList != NULL)) {
                tCC** papSL = papSuffixList;
                *(pzEnd++) = '.';

                do  {
                    strcpy( pzEnd, *(papSL++) ); /* must fit */
                    if (canReadFile( pzFullName ))
                        goto findFileReturn;

                } while (*papSL != NULL);
            }
        } while (--ct > 0);
    }

 findFileReturnFailure:
    res = FAILURE;

 findFileReturn:
    if (deallocAddr != NULL)
        AGFREE( deallocAddr );
    return res;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  countMacros
 *
 *  Figure out how many macros there are in the template
 *  so that we can allocate the right number of pointers.
 */
STATIC size_t
countMacros( tCC* pz )
{
    size_t  ct = 2;
    for (;;) {
        pz = strstr( pz, zStartMac );
        if (pz == NULL)
            break;
        ct += 2;
        if (strncmp( pz - endMacLen, zEndMac, endMacLen ) == 0)
            ct--;
        pz += startMacLen;
    }
    return ct;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  fillTemplate
 *
 *  Load the macro array and file name.
 */
STATIC void
loadMacros( tTemplate* pT,
            tCC*       pzF,
            tCC*       pzN,
            tCC*       pzData )
{
    tMacro* pMac   = pT->aMacros;

    {
        char*   pzText = (char*)(pMac + pT->macroCt);
        size_t  len;

        pT->pzFileName = pzText;
        len = strlen( pzF ) + 1;
        memcpy( (void*)pzText, (void*)pzF, len );
        pzText += len;
        if (pzN != NULL) {
            len = strlen( pzN ) + 1;
            memcpy( (void*)pzText, (void*)pzN, len );
            pzText += len;
        }

        pT->pzTemplText = pzText;
        pT->pNext = pzText + 1;
    }

    pCurTemplate = pT;

    {
        tMacro* pMacEnd = parseTemplate( pMac, &pzData );

        /*
         *  Make sure all of the input string was scanned.
         */
        if (pzData != NULL)
            AG_ABEND( "Template parse ended unexpectedly" );

        pT->macroCt = pMacEnd - pMac;

        /*
         *  IF there are empty macro slots,
         *  THEN pack the text
         */
        if ((void*)pMacEnd < (void*)(pT->pzFileName)) {
            int  delta = pT->pzFileName - (char*)pMacEnd;
            int  size  = (pT->pNext - pT->pzFileName);
            memcpy( (void*)pMacEnd, (void*)(pT->pzFileName),
                    size );
            pT->pzFileName  -= delta;
            pT->pzTemplText -= delta;
            pT->pNext       -= delta;
            if (pT->pzTplName != 0)
                pT->pzTplName -= delta;
        }
    }

    pT->descSize     = pT->pNext - (char*)pT;
    pT->descSize    += 0x40;
    pT->descSize    &= ~0x3F;
    pT->pzFileName  -= (long)pT;
    pT->pzTplName    = 0;
    pT->pzTemplText -= (long)pT;
    pT->pNext        = NULL;

    /*
     *  We cannot reallocate a smaller array because
     *  the entries are all linked together and
     *  realloc-ing it may cause it to move.
     */
#if defined( DEBUG_ENABLED )
    if (HAVE_OPT( SHOW_DEFS )) {
        tSCC zSum[] = "loaded %d macros from %s\n"
            "\tBinary template size:  0x%X\n\n";
        fprintf( pfTrace, zSum, pT->macroCt, pzF, pT->descSize );
    }
#endif
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Convert a template from data file format to internal format.
 */
EXPORT tTemplate*
templateFixup( tTemplate* pTList, size_t ttlSize )
{
    tTemplate* pT = pTList;

    for (;;) {
        size_t  tplSize = pT->descSize;

        pT->pzFileName  += (long)pT;
        if (pT->pzTplName != (char*)0)
            pT->pzTplName = 0;
        pT->pzTemplText += (long)pT;

        ttlSize -= tplSize;
        if (ttlSize <= sizeof( tTemplate))
            break;

        pT->fd    = 0;
        pT->pNext = (char*)pT + tplSize;
        pT = (tTemplate*)(pT->pNext);
    }

    pT->pNext = NULL;
    return pTList;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
EXPORT void
mapDataFile( tCC* pzFileName, tMapInfo* pMapInfo, tCC** papSuffixList )
{
    tSCC zOpen[] = "open";

    static char  zRealFile[ MAXPATHLEN ];

    /*
     *  Find the template file somewhere
     */
    if (! SUCCESSFUL( findFile( pzFileName, zRealFile, papSuffixList )))
        AG_ABEND( aprf( zCannot, ENOENT, "map data file", pzFileName,
                        strerror( ENOENT )));

    AGDUPSTR( pMapInfo->pzFileName, zRealFile, "mmapped file name" );

    /*
     *  The template file must really be a file.
     */
    {
        char* pz;
        struct stat stbf;
        if (stat( zRealFile, &stbf ) != 0)
            AG_ABEND( aprf( zCannot, errno, zOpen, zRealFile,
                            strerror( errno )));

        if (! S_ISREG( stbf.st_mode ))
            AG_ABEND( aprf( zCannot, errno, zOpen, zRealFile,
                            "wrong file type" ));

        if (outTime <= stbf.st_mtime)
            outTime = stbf.st_mtime + 1;
        pMapInfo->size = stbf.st_size+1;
    }

    /*
     *  Now open it and map it into memory
     */
    pMapInfo->fd = open( zRealFile, O_EXCL | O_RDONLY, 0 );
    if (pMapInfo->fd == -1)
        AG_ABEND( aprf( zCannot, errno, zOpen, zRealFile, strerror( errno )));

    pMapInfo->pData =
        mmap( NULL, pMapInfo->size, PROT_READ | PROT_WRITE,
              MAP_PRIVATE, pMapInfo->fd, (off_t)0 );

    if (pMapInfo->pData == (void*)(-1))
        AG_ABEND( aprf( zCannot, errno, "mmap", zRealFile, strerror( errno )));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
EXPORT tTemplate*
loadTemplate( tCC* pzFileName )
{
    tSCC*        apzSfx[] = { "tpl", "agl", NULL };
    static tMapInfo  mapInfo;

    mapDataFile( pzFileName, &mapInfo, apzSfx );

    {
        size_t       macroCt;
        tTemplate    tmpTpl;
        tTemplate*   pRes;
        tTemplate*   pSaveTpl = pCurTemplate;
        tMacro*      pSaveMac = pCurMacro;
        size_t       alocSize;
        tCC*         pzData;

        /*
         *  Process the leading pseudo-macro.  The template proper
         *  starts immediately after it.
         */
        pCurMacro    = NULL;
        pCurTemplate = &tmpTpl;
        tmpTpl.pzFileName = mapInfo.pzFileName;
        pzData = loadPseudoMacro( (tCC*)mapInfo.pData, mapInfo.pzFileName );

        /*
         *  Count the number of macros in the template.  Compute
         *  the output data size as a function of the number of macros
         *  and the size of the template data.  These may get reduced
         *  by comments.
         */
        macroCt = countMacros( pzData );
        alocSize = sizeof( *pRes ) + (macroCt * sizeof( tMacro ))
                   + mapInfo.size - (pzData - (tCC*)mapInfo.pData)
                   + strlen( mapInfo.pzFileName ) + 0x10;
        alocSize &= ~0x0F;
        pRes = (tTemplate*)AGALOC( alocSize, "main template" );
        memset( (void*)pRes, 0, alocSize );

        /*
         *  Initialize the values:
         */
        pRes->magic     = magicMark;
        pRes->descSize  = alocSize;
        pRes->macroCt   = macroCt;
        pRes->fd        = -1;
        strcpy( pRes->zStartMac, zStartMac ); /* must fit */
        strcpy( pRes->zEndMac, zEndMac );     /* must fit */
        loadMacros( pRes, mapInfo.pzFileName, NULL, pzData );
        pRes = (tTemplate*)AGREALOC( (void*)pRes, pRes->descSize,
                                     "resize template" );

        munmap( mapInfo.pData, mapInfo.size );
        close( mapInfo.fd );
        pRes = templateFixup( pRes, pRes->descSize );
        pCurTemplate = pSaveTpl;
        pCurMacro    = pSaveMac;
        return pRes;
    }
}

#ifdef MEMDEBUG
STATIC void
mUnload_For( tMacro* pMac )
{
    tDefEntry* pDE = pMac->funcPrivate;
    if (pDE == NULL)
        return;
    AGFREE( (void*)pDE->pzDefName );

    do  {
        tDefEntry* pNext = pDE->pTwin;
        freeEntry( pDE );
        pDE = pNext;
    } while (pDE != NULL);
}

STATIC void
mUnload_Match( tMacro* pMac )
{
    if (pMac->funcPrivate != NULL)
        AGFREE( (void*)pMac->funcPrivate );
}

    void
unloadTemplate( tTemplate* pT )
{
    {
        int     ct = pT->macroCt;
        tMacro* pM = pT->aMacros;

        while (--ct >= 0) {
            if (   (pM->funcCode & 0xFFF8)
                == FTYP_SELECT_MATCH_FULL)

                mUnload_Match( pM );

            else switch (pM->funcCode) {

            case FTYP_FOR: mUnload_For( pM ); break;
            }
            pM++;
        }
    }

    if (pT->fd < 0)
        AGFREE( (void*)pT );
    else {
        int     fd = pT->fd;
        void*   addr = (void*)pT;
        size_t  size = pT->descSize;
        close( fd );
        munmap( addr, size );
    }
}
#endif
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/tpLoad.c */
