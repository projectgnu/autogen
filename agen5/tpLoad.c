
/*
 *  $Id: tpLoad.c,v 1.14 2000/09/29 03:19:13 bkorb Exp $
 *
 *  This module will load a template and return a template structure.
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h> 
#include <fcntl.h>

#include "autogen.h"

static tTlibMark magicMark = TEMPLATE_MAGIC_MARKER;

STATIC size_t countMacros( tCC* pz );


    EXPORT tTemplate*
findTemplate( tCC* pzTemplName )
{
    tTemplate* pT = pNamedTplList;
    while (pT != (tTemplate*)NULL) {
        if (streqvcmp( pzTemplName, pT->pzTplName ) == 0)
            break;
        pT = (tTemplate*)(pT->pNext);
    }
    return pT;
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

    /*
     *  Expand leading environment variables.
     *  We will not mess with embedded ones.
     */
    if (*pzFName == '$') {
        char* pzDef = (char*)(pzFName+1);
        char* pzEnd = strchr( pzDef, '/' );

        /*
         *  IF the string contains a directory separator,
         *  THEN everything up to that is the define name.
         */
        if (pzEnd != (char*)NULL)
            *(pzEnd++) = NUL;

        pzDef = (char*)getDefine( pzDef );

        /*
         *  IF we cannot find a define name,
         *  THEN print a warning and continue as normal.
         */
        if (pzDef == (char*)NULL) {
            if (pzEnd != (char*)NULL)
                pzEnd[-1] = '/';

            fprintf( stderr, "NOTE: cannot expand %s\n", pzFName );
            return FAILURE;
        }

        /*
         *  Add stuff after the expanded name IFF there is stuff
         */
        if (pzEnd != (char*)NULL) {
            snprintf( pzFullName, MAXPATHLEN, "%s/%s", pzDef, pzEnd );
            /*
             *  FIXME:  this is a memory leak
             */
            AGDUPSTR( pzFName, pzFullName, "find file name" );
            deallocAddr = (void*)pzFName;
            pzEnd[-1] = '/';
        } else {
            pzFName = pzDef;
        }
        /*
         *  pzFName now points to the name the file system can use.
         *  It must _not_ point to pzFullName because we will likely
         *  rewrite that value using this pointer!
         */
    }

    /*
     *  Check for an immediately accessible file
     */
    if (access( pzFName, R_OK ) == 0) {
        strcpy( pzFullName, pzFName );
        if (deallocAddr != NULL)
            AGFREE( deallocAddr );
        return SUCCESS;
    }

    /*
     *  Not a complete file name.  If there is not already
     *  a suffix for the file name, then append ".tpl".
     *  Check for immediate access once again.
     */
    pzRoot = strrchr( pzFName, '/' );
    pzSfx  = (pzRoot != (char*)NULL)
             ? strchr( ++pzRoot, '.' )
             : strchr( pzFName, '.' );

    /*
     *  IF the file does not already have a suffix,
     *  THEN try the suffixes that are okay for this file.
     */
    if ((pzSfx == (char*)NULL) && (papSuffixList != NULL)) {
        tCC** papSL = papSuffixList;

        char* pzEnd = pzFullName +
            snprintf( pzFullName, MAXPATHLEN-MAX_SUFFIX_LEN, "%s.", pzFName );

        do  {
            strcpy( pzEnd, *(papSL++) );
            if (access( pzFullName, R_OK ) == 0) {
                if (deallocAddr != NULL)
                    AGFREE( deallocAddr );
                return SUCCESS;
            }
        } while (*papSL != NULL);
    }

    if (*pzFName == '/')
        return FAILURE;

    /*
     *  Search each directory in our directory search list
     *  for the file.
     */
    if (HAVE_OPT( TEMPL_DIRS )) {
        int     ct     = STACKCT_OPT(  TEMPL_DIRS );
        char**  ppzDir = STACKLST_OPT( TEMPL_DIRS ) + ct - 1;

        do  {
            tSCC zDirFmt[] = "%s/%s";
            char*   pzDir  = *(ppzDir--);

            char*   pzEnd  = pzFullName
                + snprintf( pzFullName, MAXPATHLEN-MAX_SUFFIX_LEN,
                            zDirFmt, pzDir, pzFName );

            if (access( pzFullName, R_OK ) == 0)
                return SUCCESS;

            /*
             *  IF the file does not already have a suffix,
             *  THEN try the ones that are okay for this file.
             */
            if ((pzSfx == (char*)NULL) && (papSuffixList != NULL)) {
                tCC** papSL = papSuffixList;
                *(pzEnd++) = '.';

                do  {
                    strcpy( pzEnd, *(papSL++) );
                    if (access( pzFullName, R_OK ) == 0) {
                        if (deallocAddr != NULL)
                            AGFREE( deallocAddr );
                        return SUCCESS;
                    }
                } while (*papSL != NULL);
            }
        } while (--ct > 0);
    }

    /*
     *  IF there is a directory component to the template name,
     *  THEN we will not search for templates in the executable's
     *       directory.
     */
    if (pzRoot != (char*)NULL)
        return FAILURE;

    /*
     *  Now we try to find this file using our PATH environment variable.
     */
    {
        tSCC   zPath[] = "PATH";
        char*  pzPath  = getenv( zPath );
        char*  pzFound;

        /*
         *  IF we cannot find a PATH variable, bail
         */
        if (pzPath == (char*)NULL)
            return FAILURE;

        pzFound = pathfind( pzPath, pzFName, "rs" );

        /*
         *  IF we could not find the bare name on the path,
         *  THEN try all the suffixes we have available, too.
         */
        if (  (pzFound == (char*)NULL)
           && (pzSfx == (char*)NULL)
           && (papSuffixList != NULL)) {
            tCC**   papSL = papSuffixList;
            char*   pzEnd;

            /*
             *  IF our target name already has a suffix,
             *  THEN we won't try to supply our own.
             */
            if (pzSfx != (char*)NULL) {
                if (deallocAddr != NULL)
                    AGFREE( deallocAddr );
                return FAILURE;
            }

            pzEnd  = pzFullName
                + snprintf( pzFullName, MAXPATHLEN-MAX_SUFFIX_LEN,
                            "%s.", pzFName );

            for (;;) {
                strcpy( pzEnd, *papSL );

                if (access( pzFullName, R_OK ) == 0)
                    break;

                if (*++papSL == NULL)
                    return FAILURE;

            } while (*papSL != NULL);
        }

        if (deallocAddr != NULL)
            AGFREE( deallocAddr );

        if (pzFound == NULL)
            return FAILURE;

        strcpy( pzFullName, pzFound );
        return SUCCESS;
    }
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
        if (pz == (char*)NULL)
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
    EXPORT void
loadMacros( tTemplate*     pT,
            tCC*    pzF,
            tCC*    pzN,
            tCC*    pzData )
{
    tMacro* pMac   = pT->aMacros;

    {
        char*   pzText = (char*)(pMac + pT->macroCt);
        size_t  len;

        pT->pzFileName = pzText;
        len = strlen( pzF ) + 1;
        memcpy( (void*)pzText, (void*)pzF, len );
        pzText += len;
        if (pzN != (char*)NULL) {
            len = strlen( pzN ) + 1;
            memcpy( (void*)pzText, (void*)pzN, len );
            pzText += len;
        }

        pT->pzTemplText = pzText;
        pT->pNext = pzText + 1;
    }

    {
        tMacro* pMacEnd = parseTemplate( pT, pMac, &pzData );

        /*
         *  Make sure all of the input string was scanned.
         */
        if (pzData != (char*)NULL)  {
            fprintf( stderr, zTplErr, pzF, -1,
                     "parse ended unexpectedly" );
            AG_ABEND;
        }

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
#if defined( DEBUG ) && defined( VALUE_OPT_SHOW_DEFS )
    if (HAVE_OPT( SHOW_DEFS )) {
        tSCC zSum[] = "loaded %d macros from %s\n"
            "\tBinary template size:  0x%X\n\n";
        printf( zSum, pT->macroCt, pzF, pT->descSize );
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
        if (memcmp( (void*)pT, (void*)pTList, sizeof( tTlibMark )) != 0) {
            tSCC z[] = "The %s binary template library is inconsistent\n";
            fprintf( stderr, z, pTList->pzFileName );
            AG_ABEND;
        }
    }

    pT->pNext = (char*)NULL;
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
    static char  zRealFile[ MAXPATHLEN ];

    /*
     *  Find the template file somewhere
     */
    if (! SUCCESSFUL( findFile( pzFileName, zRealFile, papSuffixList ))) {
        tSCC zMapDataFile[] = "map data file";
        fprintf( stderr, zCannot, pzProg, ENOENT,
                 zMapDataFile, pzFileName, strerror( ENOENT ));
        AG_ABEND;
    }
    AGDUPSTR( pMapInfo->pzFileName, zRealFile, "map data file" );

    /*
     *  The template file must really be a file.
     */
    {
        struct stat stbf;
        if (stat( zRealFile, &stbf ) != 0) {
            tSCC zStat[] = "stat";
            fprintf( stderr, zCannot, pzProg, errno,
                     zStat, zRealFile, strerror( errno ));
            AG_ABEND;
        }

        if (! S_ISREG( stbf.st_mode )) {
            fprintf( stderr, "ERROR:  `%s' is not a regular file\n",
                     zRealFile );
            AG_ABEND;
        }

        if (outTime <= stbf.st_mtime)
            outTime = stbf.st_mtime + 1;
        pMapInfo->size = stbf.st_size+1;
    }

    /*
     *  Now open it and map it into memory
     */
    pMapInfo->fd = open( zRealFile, O_EXCL | O_RDONLY, 0 );
    if (pMapInfo->fd == -1) {
        tSCC zOpen[] = "open";
        fprintf( stderr, zCannot, pzProg, errno,
                 zOpen, zRealFile, strerror( errno ));
        AG_ABEND;
    }

    pMapInfo->pData =
        mmap( (void*)NULL, pMapInfo->size, PROT_READ | PROT_WRITE,
              MAP_PRIVATE, pMapInfo->fd, (off_t)0 );

    if (pMapInfo->pData == (void*)(-1)) {
        tSCC zMmap[] = "mmap";
        fprintf( stderr, zCannot, pzProg, errno,
                 zMmap, zRealFile, strerror( errno ));
        AG_ABEND;
    }
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

    /*
     *  Check for a binary template.  If it is, it must
     *  be the current revision, or we choke.
     */
    {
        tTemplate*  p = (tTemplate*)mapInfo.pData;

        if (p->magic.magic.i[0] == magicMark.magic.i[0]) {
            tSCC zOutOfDate[] = "out-of-date binary template";
            tSCC zTooNew[]    = "unknown binary template version";
            tSCC zChange[]    = "Functions changed - rebuild lib";
            tCC* pz;

            if (p->magic.magic.i[1] == magicMark.magic.i[1]) {
                p->fd = mapInfo.fd;
                return templateFixup( p, mapInfo.size-1 );
            }

            if (p->magic.revision < TEMPLATE_REVISION)
                pz = zOutOfDate;
            if (p->magic.revision > TEMPLATE_REVISION)
                pz = zTooNew;
            else
                pz = zChange;

            fprintf( stderr, zTplErr, mapInfo.pzFileName, 0, pz );
            AG_ABEND;
        }
    }

    {
        size_t       macroCt;
        tTemplate*   pRes;
        size_t       alocSize;
        tCC*         pzData;

        /*
         *  Process the leading pseudo-macro.  The template proper
         *  starts immediately after it.
         */
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
        memcpy( (void*)pRes, (void*)&magicMark, sizeof( magicMark ));
        pRes->descSize  = alocSize;
        pRes->macroCt   = macroCt;
        pRes->fd        = -1;
        strcpy( pRes->zStartMac, zStartMac );
        strcpy( pRes->zEndMac, zEndMac );
        loadMacros( pRes, mapInfo.pzFileName, (char*)NULL, pzData );
        pRes = (tTemplate*)AGREALOC( (void*)pRes, pRes->descSize,
                                     "resize main template" );

        munmap( mapInfo.pData, mapInfo.size );
        close( mapInfo.fd );
        return templateFixup( pRes, pRes->descSize );
    }
}

#ifdef MEMDEBUG

    void
unloadTemplate( tTemplate* pT )
{
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
/* end of tpLoad.c */
