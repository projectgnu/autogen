
/*
 *  $Id: tpLoad.c,v 4.12 2006/03/25 19:23:28 bkorb Exp $
 *
 *  This module will load a template and return a template structure.
 */

/*
 *  AutoGen copyright 1992-2006 Bruce Korb
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

static tTlibMark magicMark = TEMPLATE_MAGIC_MARKER;

/* = = = START-STATIC-FORWARD = = = */
/* static forward declarations maintained by :mkfwd */
static ag_bool
canReadFile( tCC* pzFName );

static size_t
countMacros( tCC* pz );

static void
loadMacros( tTemplate* pT,
            tCC*       pzF,
            tCC*       pzN,
            tCC*       pzData );
/* = = = END-STATIC-FORWARD = = = */


LOCAL tTemplate*
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

static ag_bool
canReadFile( tCC* pzFName )
{
    struct stat stbf;
    if (stat( pzFName, &stbf ) != 0)
        return AG_FALSE;
    if (! S_ISREG( stbf.st_mode ))
        return AG_FALSE;
    return (access( pzFName, R_OK) == 0) ? AG_TRUE : AG_FALSE;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
LOCAL tSuccess
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
        tCC**   ppzDir = STACKLST_OPT( TEMPL_DIRS ) + ct - 1;

        do  {
            tSCC    zDirFmt[] = "%s/%s";
            tCC*    pzDir  = *(ppzDir--);
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
static size_t
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
static void
loadMacros( tTemplate* pT,
            tCC*       pzF,
            tCC*       pzN,
            tCC*       pzData )
{
    tMacro* pMac   = pT->aMacros;

    {
        char*   pzText = (char*)(pMac + pT->macroCt);
        size_t  len;

        pT->pzTplFile = strdup(pzF);

        len = strlen( pzN ) + 1;
        memcpy( (void*)pzText, (void*)pzN, len );
        pT->pzTplName   = pzText;
        pzText         += len;
        pT->pzTemplText = pzText;
        pT->pNext       = pzText + 1;
    }

    pCurTemplate = pT;

    {
        tMacro* pMacEnd = parseTemplate( pMac, &pzData );
        int     ct;

        /*
         *  Make sure all of the input string was scanned.
         */
        if (pzData != NULL)
            AG_ABEND( "Template parse ended unexpectedly" );

        ct = pMacEnd - pMac;

        /*
         *  IF there are empty macro slots,
         *  THEN pack the text
         */
        if (ct < pT->macroCt) {
            int   delta = sizeof(tMacro) * (pT->macroCt - ct);
            void* data  =
                (pT->pzTplName == NULL) ? pT->pzTemplText : pT->pzTplName;
            int   size  = pT->pNext - (char*)data;
            memmove( (void*)pMacEnd, data, size );

            pT->pzTemplText -= delta;
            pT->pNext       -= delta;
            pT->pzTplName   -= delta;
            pT->macroCt      = ct;
        }
    }

    pT->descSize = pT->pNext - (char*)pT;
    pT->pNext    = NULL;

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
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
LOCAL tTemplate*
loadTemplate( tCC* pzFileName )
{
    static tmap_info_t  mapInfo;
    tTemplate* pRes;

    {
        tSCC*       apzSfx[] = { "tpl", "agl", NULL };
        static char zRealFile[ MAXPATHLEN ];

        /*
         *  Find the template file somewhere
         */
        if (! SUCCESSFUL( findFile( pzFileName, zRealFile, apzSfx )))
            AG_ABEND( aprf( zCannot, ENOENT, "map data file", pzFileName,
                            strerror( ENOENT )));

        text_mmap( zRealFile, PROT_READ|PROT_WRITE, MAP_PRIVATE, &mapInfo );
        if (TEXT_MMAP_FAILED_ADDR(mapInfo.txt_data))
            AG_ABEND( aprf( "Could not open template '%s'", pzFileName ));
    }

    {
        size_t       macroCt;
        tMacro*      pSaveMac = pCurMacro;
        size_t       alocSize;
        tCC*         pzData;

        /*
         *  Process the leading pseudo-macro.  The template proper
         *  starts immediately after it.
         */
        pCurMacro    = NULL;
        pzData = loadPseudoMacro( (tCC*)mapInfo.txt_data, pzFileName );

        /*
         *  Count the number of macros in the template.  Compute
         *  the output data size as a function of the number of macros
         *  and the size of the template data.  These may get reduced
         *  by comments.
         */
        macroCt = countMacros( pzData );
        alocSize = sizeof( *pRes ) + (macroCt * sizeof( tMacro ))
                   + mapInfo.txt_size - (pzData - (tCC*)mapInfo.txt_data)
                   + strlen( pzFileName ) + 0x10;
        alocSize &= ~0x0F;
        pRes = (tTemplate*)AGALOC( alocSize, "main template" );
        memset( (void*)pRes, 0, alocSize );

        /*
         *  Initialize the values:
         */
        pRes->magic     = magicMark;
        pRes->descSize  = alocSize;
        pRes->macroCt   = macroCt;
        strcpy( pRes->zStartMac, zStartMac ); /* must fit */
        strcpy( pRes->zEndMac, zEndMac );     /* must fit */
        loadMacros( pRes, pzFileName, "*template file*", pzData );

        pRes->pzTplName   -= (long)pRes;
        pRes->pzTemplText -= (long)pRes;
        pRes = (tTemplate*)AGREALOC( (void*)pRes, pRes->descSize,
                                     "resize template" );
        pRes->pzTplName   += (long)pRes;
        pRes->pzTemplText += (long)pRes;

        text_munmap( &mapInfo );
        pCurMacro    = pSaveMac;
    }

    return pRes;
}

LOCAL void
unloadTemplate( tTemplate* pT )
{
    tMacro* pMac = pT->aMacros;
    int ct = pT->macroCt;

    while (--ct >= 0) {
        tpUnloadProc proc;
        unsigned int ix = pMac->funcCode;

        /*
         *  "select" functions get remapped, depending on the alias used
         *  for the selection.  See the "teFuncType" enumeration in functions.h.
         */
        if (ix >= FUNC_CT)
            ix = FTYP_SELECT;

        proc = apUnloadProc[ ix ];
        if (proc != NULL)
            (*proc)( pMac );

        pMac++;
    }

    AGFREE( (void*)(pT->pzTplFile) );
    AGFREE( pT );
}

LOCAL void
cleanup( tTemplate* pTF )
{
#if ! defined(DEBUG_ENABLED)
    for (;;) {
        tTemplate* pT = pNamedTplList;
        if (pT == NULL)
            break;
        pNamedTplList = (tTemplate*)pT->pNext;
        unloadTemplate( pT );
    }

    AGFREE( forInfo.fi_data );
    unloadTemplate( pTF );
    unloadDefs();
    ag_scmStrings_deinit();

    manageAllocatedData( NULL );
#endif
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * end of agen5/tpLoad.c */
