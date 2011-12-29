
/**
 * @file tpLoad.c
 *
 * Time-stamp:        "2011-12-29 10:04:47 bkorb"
 *
 *  This module will load a template and return a template structure.
 *
 * This file is part of AutoGen.
 * Copyright (c) 1992-2011 Bruce Korb - all rights reserved
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
read_okay(char const * pzFName);

static size_t
cnt_macros(char const * pz);

static void
load_macs(tTemplate * pT, char const * pzF, char const * pzN,
          char const * pzData);

static tTemplate *
digest_pseudo_macro(tmap_info_t * minfo, char * real_file);
/* = = = END-STATIC-FORWARD = = = */

/**
 * Return the template structure matching the name passed in.
 */
LOCAL tTemplate *
findTemplate(char const * pzTemplName)
{
    tTemplate * pT = pNamedTplList;
    while (pT != NULL) {
        if (streqvcmp(pzTemplName, pT->pzTplName) == 0)
            break;
        pT = (tTemplate*)(void*)(pT->pNext);
    }
    return pT;
}

/**
 * the name is a regular file with read access
 */
static ag_bool
read_okay(char const * pzFName)
{
    struct stat stbf;
    if (stat(pzFName, &stbf) != 0)
        return AG_FALSE;
    if (! S_ISREG(stbf.st_mode))
        return AG_FALSE;
    return (access(pzFName, R_OK) == 0) ? AG_TRUE : AG_FALSE;
}


/**
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
LOCAL tSuccess
findFile(char const * pzFName,
         char * pzFullName,
         char const * const * papSuffixList,
         char const * pzReferrer)
{
    char * pzRoot;
    char * pzSfx;
    void * deallocAddr = NULL;

    tSuccess res = SUCCESS;

    /*
     *  Expand leading environment variables.
     *  We will not mess with embedded ones.
     */
    if (*pzFName == '$') {
        if (! optionMakePath(pzFullName, (int)AG_PATH_MAX, pzFName,
                             autogenOptions.pzProgPath))
            return FAILURE;

        AGDUPSTR(pzFName, pzFullName, "find file name");
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
    pzRoot = strrchr(pzFName, '/');
    pzSfx  = (pzRoot != NULL)
             ? strchr(++pzRoot, '.')
             : strchr(pzFName,  '.');

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
         *  for the file.  We always force two copies of this option.
         */
        int  ct = STACKCT_OPT(TEMPL_DIRS);
        char const ** ppzDir = STACKLST_OPT(TEMPL_DIRS) + ct - 1;
        char const *  pzDir  = curdir;

        if (*pzFName == '/')
            ct = 0;

        do  {
            char * pzEnd;
            void * coerce;

            /*
             *  IF one of our template paths starts with '$', then expand it.
             */
            if (*pzDir == '$') {
                if (! optionMakePath(pzFullName, (int)AG_PATH_MAX, pzDir,
                                     autogenOptions.pzProgPath)) {
                    coerce = (void *)pzDir;
                    strcpy(coerce, curdir);

                } else {
                    AGDUPSTR(pzDir, pzFullName, "find directory name");
                    coerce = (void *)ppzDir[1];
                    free(coerce);
                    ppzDir[1] = pzDir; /* save the computed name for later */
                }
            }

            if ((*pzFName == '/') || (pzDir == curdir)) {
                size_t nln = strlen(pzFName);
                if (nln >= AG_PATH_MAX - MAX_SUFFIX_LEN)
                    break;

                memcpy(pzFullName, pzFName, nln);
                pzEnd  = pzFullName + nln;
                *pzEnd = NUL;
            }
            else {
                pzEnd = pzFullName
                    + snprintf(pzFullName, AG_PATH_MAX - MAX_SUFFIX_LEN,
                               zDirFmt, pzDir, pzFName);
            }

            if (read_okay(pzFullName))
                goto findFileReturn;

            /*
             *  IF the file does not already have a suffix,
             *  THEN try the ones that are okay for this file.
             */
            if ((pzSfx == NULL) && (papSuffixList != NULL)) {
                char const * const * papSL = papSuffixList;
                *(pzEnd++) = '.';

                do  {
                    strcpy(pzEnd, *(papSL++)); /* must fit */
                    if (read_okay(pzFullName))
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
            if (  (pzReferrer != NULL)
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


/**
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Figure out how many macros there are in the template
 *  so that we can allocate the right number of pointers.
 */
static size_t
cnt_macros(char const * pz)
{
    size_t  ct = 2;
    for (;;) {
        pz = strstr(pz, zStartMac);
        if (pz == NULL)
            break;
        ct += 2;
        if (strncmp(pz - endMacLen, zEndMac, endMacLen) == 0)
            ct--;
        pz += startMacLen;
    }
    return ct;
}


/**
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Load the macro array and file name.
 */
static void
load_macs(tTemplate * pT, char const * pzF, char const * pzN,
          char const * pzData)
{
    tMacro* pMac   = pT->aMacros;

    {
        char*   pzText = (char*)(pMac + pT->macroCt);
        size_t  len;

        AGDUPSTR(pT->pzTplFile, pzF, "templ file");

        len = strlen(pzN) + 1;
        memcpy((void*)pzText, (void*)pzN, len);
        pT->pzTplName   = pzText;
        pzText         += len;
        pT->pzTemplText = pzText;
        pT->pNext       = pzText + 1;
    }

    pCurTemplate = pT;

    {
        tMacro* pMacEnd = parseTemplate(pMac, &pzData);
        int     ct;

        /*
         *  Make sure all of the input string was scanned.
         */
        if (pzData != NULL)
            AG_ABEND("Template parse ended unexpectedly");

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
            memmove((void*)pMacEnd, data, size);

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
#if defined(DEBUG_ENABLED)
    if (HAVE_OPT(SHOW_DEFS)) {
        static char const zSum[] =
            "loaded %d macros from %s\n"
            "\tBinary template size:  0x%zX\n\n";
        fprintf(pfTrace, zSum, pT->macroCt, pzF, pT->descSize);
    }
#endif
}

/**
 * Process the stuff in the pseudo macro.
 */
static tTemplate *
digest_pseudo_macro(tmap_info_t * minfo, char * real_file)
{
    tTemplate * pRes;

    /*
     *  Count the number of macros in the template.  Compute
     *  the output data size as a function of the number of macros
     *  and the size of the template data.  These may get reduced
     *  by comments.
     */
    char const * pzData =
        loadPseudoMacro((char const *)minfo->txt_data, real_file);

    size_t macroCt  = cnt_macros(pzData);
    size_t alocSize = (sizeof(*pRes) + (macroCt * sizeof(tMacro))
                       + minfo->txt_size
                       - (pzData - (char const *)minfo->txt_data)
                       + strlen(real_file) + 0x10) & ~0x0F;

    pRes = (tTemplate*)AGALOC(alocSize, "main template");
    memset((void*)pRes, 0, alocSize);

    /*
     *  Initialize the values:
     */
    pRes->magic     = magicMark;
    pRes->descSize  = alocSize;
    pRes->macroCt   = macroCt;

    strcpy(pRes->zStartMac, zStartMac); /* must fit */
    strcpy(pRes->zEndMac, zEndMac);     /* must fit */
    load_macs(pRes, real_file, "*template file*", pzData);

    pRes->pzTplName   -= (long)pRes;
    pRes->pzTemplText -= (long)pRes;
    pRes = (tTemplate*)AGREALOC((void*)pRes, pRes->descSize,
                                "resize template");
    pRes->pzTplName   += (long)pRes;
    pRes->pzTemplText += (long)pRes;

    return pRes;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
LOCAL tTemplate *
loadTemplate(char const * pzFileName, char const * referrer)
{
    static tmap_info_t mapInfo;
    static char        zRealFile[ AG_PATH_MAX ];

    /*
     *  Find the template file somewhere
     */
    {
        static char const * const apzSfx[] = { "tpl", "agl", NULL };
        if (! SUCCESSFUL(findFile(pzFileName, zRealFile, apzSfx, referrer))) {
            errno = ENOENT;
            AG_CANT("map data file", pzFileName);
        }
    }

    /*
     *  Make sure the specified file is a regular file.
     *  Make sure the output time stamp is at least as recent.
     */
    {
        struct stat stbf;
        if (stat(zRealFile, &stbf) != 0)
            AG_CANT("stat file", pzFileName);

        if (! S_ISREG(stbf.st_mode)) {
            errno = EINVAL;
            AG_CANT("not regular file", pzFileName);
        }

        if (outTime <= stbf.st_mtime)
            outTime = stbf.st_mtime + 1;
    }

    text_mmap(zRealFile, PROT_READ|PROT_WRITE, MAP_PRIVATE, &mapInfo);
    if (TEXT_MMAP_FAILED_ADDR(mapInfo.txt_data))
        AG_ABEND(aprf("Could not open template '%s'", zRealFile));

    if (pfDepends != NULL)
        add_source_file(zRealFile);

    /*
     *  Process the leading pseudo-macro.  The template proper
     *  starts immediately after it.
     */
    {
        tMacro    * pSaveMac = pCurMacro;
        tTemplate * pRes;
        pCurMacro = NULL;

        pRes = digest_pseudo_macro(&mapInfo, zRealFile);
        pCurMacro = pSaveMac;
        text_munmap(&mapInfo);

        return pRes;
    }
}

/**
 * Deallocate anything we allocated related to a template,
 * including the pointer passed in.
 */
LOCAL void
unloadTemplate(tTemplate* pT)
{
    tMacro* pMac = pT->aMacros;
    int ct = pT->macroCt;

    while (--ct >= 0) {
        tpUnloadProc proc;
        unsigned int ix = pMac->funcCode;

        /*
         * "select" functions get remapped, depending on the alias used for
         * the selection.  See the "teFuncType" enumeration in functions.h.
         */
        if (ix >= FUNC_CT)
            ix = FTYP_SELECT;

        proc = apUnloadProc[ ix ];
        if (proc != NULL)
            (*proc)(pMac);

        pMac++;
    }

    AGFREE((void*)(pT->pzTplFile));
    AGFREE(pT);
}

/**
 *  This gets called when all is well at the end.
 */
LOCAL void
cleanup(tTemplate* pTF)
{
    if (HAVE_OPT(USED_DEFINES))
        print_used_defines();

    if (pfDepends != NULL)
        wrap_up_depends();

    optionFree(&autogenOptions);

    for (;;) {
        tTemplate* pT = pNamedTplList;
        if (pT == NULL)
            break;
        pNamedTplList = (tTemplate*)(void*)(pT->pNext);
        unloadTemplate(pT);
    }

    AGFREE(forInfo.fi_data);
    unloadTemplate(pTF);
    unloadDefs();
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/tpLoad.c */
