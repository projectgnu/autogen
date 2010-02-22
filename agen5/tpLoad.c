
/*
 *  $Id$
 *
 * Time-stamp:        "2009-10-10 07:35:55 bkorb"
 *
 *  This module will load a template and return a template structure.
 *
 * This file is part of AutoGen.
 * AutoGen copyright (c) 1992-2009 by Bruce Korb - all rights reserved
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

static tTlibMark magicMark = TEMPLATE_MAGIC_MARKER;

/* = = = START-STATIC-FORWARD = = = */
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
        pT = (tTemplate*)(void*)(pT->pNext);
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
findFile(tCC* pzFName, char* pzFullName, tCC** papSuffixList, tCC * pzReferrer)
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
        if (! optionMakePath( pzFullName, (int)AG_PATH_MAX, pzFName,
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
     *  Not a complete file name.  If there is not already
     *  a suffix for the file name, then append ".tpl".
     *  Check for immediate access once again.
     */
    pzRoot = strrchr( pzFName, '/' );
    pzSfx  = (pzRoot != NULL)
             ? strchr( ++pzRoot, '.' )
             : strchr( pzFName, '.' );

    /*
     *  The referrer is useful only if it includes a directory name.
     */
    if (pzReferrer != NULL) {
        char * pz = strrchr(pzReferrer, '/');
        if (pz == NULL)
            pzReferrer = NULL;
        else {
            AGDUPSTR(pzReferrer, pzReferrer, "pzReferrer");
            pz = strrchr(pzReferrer, '/');
            *pz = NUL;
        }
    }

    /*
     *  IF the file name does not contain a directory separator,
     *  then we will use the TEMPL_DIR search list to keep hunting.
     */
    {
        static char const curdir[]  = ".";
        static char const zDirFmt[] = "%s/%s";

        /*
         *  Search each directory in our directory search list
         *  for the file.  We always force one copy of this option.
         */
        int     ct     = STACKCT_OPT(  TEMPL_DIRS );
        tCC**   ppzDir = STACKLST_OPT( TEMPL_DIRS ) + ct - 1;
        tCC*    pzDir  = curdir;

        if (*pzFName == '/')
            ct = 0;

        do  {
            char * pzEnd;
            void * coerce;

            /*
             *  IF one of our template paths starts with '$', then expand it.
             */
            if (*pzDir == '$') {
                if (! optionMakePath( pzFullName, (int)AG_PATH_MAX, pzDir,
                                      autogenOptions.pzProgPath )) {
                    coerce = (void *)pzDir;
                    strcpy(coerce, curdir);

                } else {
                    AGDUPSTR( pzDir, pzFullName, "find directory name" );
                    coerce = (void *)ppzDir[1];
                    free(coerce);
                    ppzDir[1] = pzDir; /* save the computed name for later */
                }
            }

            if ((*pzFName == '/') || (pzDir == curdir)) {
                strlcpy(pzFullName, pzFName, AG_PATH_MAX - MAX_SUFFIX_LEN);
                pzEnd = pzFullName + strlen(pzFullName);
            }
            else {
                pzEnd = pzFullName
                    + snprintf( pzFullName, AG_PATH_MAX - MAX_SUFFIX_LEN,
                                zDirFmt, pzDir, pzFName );
            }

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

            if (*pzFName == '/')
                break;

            /*
             *  IF there is a referrer, use it before the rest of the
             *  TEMPL_DIRS We detect first time through by *both* pzDir value
             *  and ct value.  "ct" will have this same value next time
             *  through and occasionally "pzDir" will be set to "curdir", but
             *  never again when ct is STACKCT_OPT(TEMPL_DIRS)
             */
            if (   (pzReferrer != NULL)
                && (pzDir == curdir)
                && (ct == STACKCT_OPT(TEMPL_DIRS))) {

                pzDir = pzReferrer;
                ct++;

            } else {
                pzDir = *(ppzDir--);
            }
        } while (--ct >= 0);
    }

    res = FAILURE;

 findFileReturn:
    AGFREE(deallocAddr);
    AGFREE(pzReferrer);
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
            int     delta = sizeof(tMacro) * (pT->macroCt - ct);
            void*   data  =
                (pT->pzTplName == NULL) ? pT->pzTemplText : pT->pzTplName;
            size_t  size  = pT->pNext - (char*)data;
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
            "\tBinary template size:  0x%zX\n\n";
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
loadTemplate(tCC* pzFileName, tCC * pzReferrer)
{
    static tmap_info_t  mapInfo;
    static char     zRealFile[ AG_PATH_MAX ];
    tSCC*           apzSfx[] = { "tpl", "agl", NULL };

    tTemplate*      pRes;
    tMacro*         pSaveMac = pCurMacro;

    /*
     *  Find the template file somewhere
     */
    if (! SUCCESSFUL( findFile(pzFileName, zRealFile, apzSfx, pzReferrer)))
        AG_ABEND( aprf( zCannot, ENOENT, "map data file", pzFileName,
                        strerror( ENOENT )));

    /*
     *  Make sure the specified file is a regular file.
     *  Make sure the output time stamp is at least as recent.
     */
    {
        struct stat stbf;
        if (stat( zRealFile, &stbf ) != 0)
            AG_ABEND( aprf( zCannot, errno, "stat template file", pzFileName,
                            strerror(errno)));

        if (! S_ISREG( stbf.st_mode ))
            AG_ABEND( aprf( zCannot, EINVAL, "not regular file", pzFileName,
                            strerror( EINVAL )));

        if (outTime <= stbf.st_mtime)
            outTime = stbf.st_mtime + 1;
    }

    text_mmap( zRealFile, PROT_READ|PROT_WRITE, MAP_PRIVATE, &mapInfo );
    if (TEXT_MMAP_FAILED_ADDR(mapInfo.txt_data))
        AG_ABEND( aprf( "Could not open template '%s'", zRealFile ));

    /*
     *  Process the leading pseudo-macro.  The template proper
     *  starts immediately after it.
     */
    pCurMacro = NULL;

    /*
     *  Count the number of macros in the template.  Compute
     *  the output data size as a function of the number of macros
     *  and the size of the template data.  These may get reduced
     *  by comments.
     */
    {
        tCC*   pzData   = loadPseudoMacro( (tCC*)mapInfo.txt_data, zRealFile );
        size_t macroCt  = countMacros( pzData );
        size_t alocSize = (sizeof( *pRes ) + (macroCt * sizeof( tMacro ))
            + mapInfo.txt_size - (pzData - (tCC*)mapInfo.txt_data)
            + strlen(zRealFile) + 0x10) & ~0x0F;

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
        loadMacros( pRes, zRealFile, "*template file*", pzData );
    }

    pRes->pzTplName   -= (long)pRes;
    pRes->pzTemplText -= (long)pRes;
    pRes = (tTemplate*)AGREALOC( (void*)pRes, pRes->descSize,
                                 "resize template" );
    pRes->pzTplName   += (long)pRes;
    pRes->pzTemplText += (long)pRes;

    text_munmap( &mapInfo );
    pCurMacro    = pSaveMac;

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
    if (HAVE_OPT(USED_DEFINES))
        print_used_defines();

    optionFree(&autogenOptions);

    for (;;) {
        tTemplate* pT = pNamedTplList;
        if (pT == NULL)
            break;
        pNamedTplList = (tTemplate*)(void*)(pT->pNext);
        unloadTemplate( pT );
    }

    AGFREE( forInfo.fi_data );
    unloadTemplate( pTF );
    unloadDefs();
    ag_scmStrings_deinit();

    manageAllocatedData( NULL );
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/tpLoad.c */
