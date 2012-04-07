
/**
 * @file tpLoad.c
 *
 * Time-stamp:        "2012-04-07 09:03:08 bkorb"
 *
 *  This module will load a template and return a template structure.
 *
 * This file is part of AutoGen.
 * Copyright (c) 1992-2012 Bruce Korb - all rights reserved
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

/* = = = START-STATIC-FORWARD = = = */
static bool
read_okay(char const * pzFName);

static size_t
cnt_macros(char const * pz);

static void
load_macs(templ_t * pT, char const * pzF, char const * pzN,
          char const * pzData);

static templ_t *
digest_tpl(tmap_info_t * minfo, char * fname);
/* = = = END-STATIC-FORWARD = = = */

/**
 * Return the template structure matching the name passed in.
 */
LOCAL templ_t *
find_tpl(char const * pzTemplName)
{
    templ_t * pT = named_tpls;
    while (pT != NULL) {
        if (streqvcmp(pzTemplName, pT->td_name) == 0)
            break;
        pT = (templ_t*)(void*)(pT->td_scan);
    }
    return pT;
}

/**
 * the name is a regular file with read access
 */
static bool
read_okay(char const * pzFName)
{
    struct stat stbf;
    if (stat(pzFName, &stbf) != 0)
        return false;
    if (! S_ISREG(stbf.st_mode))
        return false;
    return (access(pzFName, R_OK) == 0) ? true : false;
}


/**
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
LOCAL tSuccess
find_file(char const * pzFName,
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
        /*
         *  Search each directory in our directory search list for the file.
         *  We always force two copies of this option, so we know it exists.
         */
        int  ct = STACKCT_OPT(TEMPL_DIRS);
        char const ** ppzDir = STACKLST_OPT(TEMPL_DIRS) + ct - 1;
        char const *  pzDir  = FIND_FILE_CURDIR;

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
                    strcpy(coerce, FIND_FILE_CURDIR);

                } else {
                    AGDUPSTR(pzDir, pzFullName, "find directory name");
                    coerce = (void *)ppzDir[1];
                    free(coerce);
                    ppzDir[1] = pzDir; /* save the computed name for later */
                }
            }

            if ((*pzFName == '/') || (pzDir == FIND_FILE_CURDIR)) {
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
                               FIND_FILE_DIR_FMT, pzDir, pzFName);
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
               && (pzDir == FIND_FILE_CURDIR)
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
        pz = strstr(pz, st_mac_mark);
        if (pz == NULL)
            break;
        ct += 2;
        if (strncmp(pz - end_mac_len, end_mac_mark, end_mac_len) == 0)
            ct--;
        pz += st_mac_len;
    }
    return ct;
}


/**
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Load the macro array and file name.
 */
static void
load_macs(templ_t * pT, char const * pzF, char const * pzN,
          char const * pzData)
{
    macro_t* pMac   = pT->td_macros;

    {
        char*   pzText = (char*)(pMac + pT->td_mac_ct);
        size_t  len;

        AGDUPSTR(pT->td_file, pzF, "templ file");

        len = strlen(pzN) + 1;
        memcpy((void*)pzText, (void*)pzN, len);
        pT->td_name = pzText;
        pzText     += len;
        pT->td_text = pzText;
        pT->td_scan = pzText + 1;
    }

    current_tpl = pT;

    {
        macro_t* pMacEnd = parse_tpl(pMac, &pzData);
        int     ct;

        /*
         *  Make sure all of the input string was scanned.
         */
        if (pzData != NULL)
            AG_ABEND(LOAD_MACS_BAD_PARSE);

        ct = pMacEnd - pMac;

        /*
         *  IF there are empty macro slots,
         *  THEN pack the text
         */
        if (ct < pT->td_mac_ct) {
            int     delta = sizeof(macro_t) * (pT->td_mac_ct - ct);
            void*   data  =
                (pT->td_name == NULL) ? pT->td_text : pT->td_name;
            size_t  size  = pT->td_scan - (char*)data;
            memmove((void*)pMacEnd, data, size);

            pT->td_text  -= delta;
            pT->td_scan  -= delta;
            pT->td_name  -= delta;
            pT->td_mac_ct = ct;
        }
    }

    pT->td_size = pT->td_scan - (char*)pT;
    pT->td_scan = NULL;

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
        fprintf(trace_fp, zSum, pT->td_mac_ct, pzF, pT->td_size);
    }
#endif
}

/**
 * Load a template from mapped memory.  Load up the pseudo macro,
 * count the macros, allocate the data, and parse all the macros.
 *
 * @param[in] minfo  information about the mapped memory.
 * @param[in] fname  the full path input file name.
 *
 * @returns the digested data
 */
static templ_t *
digest_tpl(tmap_info_t * minfo, char * fname)
{
    templ_t * res;

    /*
     *  Count the number of macros in the template.  Compute
     *  the output data size as a function of the number of macros
     *  and the size of the template data.  These may get reduced
     *  by comments.
     */
    char const * dta =
        loadPseudoMacro((char const *)minfo->txt_data, fname);

    size_t mac_ct   = cnt_macros(dta);
    size_t alloc_sz = (sizeof(*res) + (mac_ct * sizeof(macro_t))
                       + minfo->txt_size
                       - (dta - (char const *)minfo->txt_data)
                       + strlen(fname) + 0x10) & ~0x0F;

    res = (templ_t*)AGALOC(alloc_sz, "main template");
    memset((void*)res, 0, alloc_sz);

    /*
     *  Initialize the values:
     */
    res->td_magic  = magic_marker;
    res->td_size   = alloc_sz;
    res->td_mac_ct = mac_ct;

    strcpy(res->td_start_mac, st_mac_mark); /* must fit */
    strcpy(res->td_end_mac,   end_mac_mark);   /* must fit */
    load_macs(res, fname, PSEUDO_MAC_TPL_FILE, dta);

    res->td_name -= (long)res;
    res->td_text -= (long)res;
    res = (templ_t*)AGREALOC((void*)res, res->td_size,
                                "resize template");
    res->td_name += (long)res;
    res->td_text += (long)res;

    return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
 *  Starting with the current directory, search the directory
 *  list trying to find the base template file name.
 */
LOCAL templ_t *
tpl_load(char const * fname, char const * referrer)
{
    static tmap_info_t map_info;
    static char        tpl_file[ AG_PATH_MAX ];

    /*
     *  Find the template file somewhere
     */
    {
        static char const * const sfx_list[] = {
            LOAD_TPL_SFX_TPL, LOAD_TPL_SFX_AGL, NULL };
        if (! SUCCESSFUL(find_file(fname, tpl_file, sfx_list, referrer))) {
            errno = ENOENT;
            AG_CANT(LOAD_TPL_CANNOT_MAP, fname);
        }
    }

    /*
     *  Make sure the specified file is a regular file.
     *  Make sure the output time stamp is at least as recent.
     */
    {
        struct stat stbf;
        if (stat(tpl_file, &stbf) != 0)
            AG_CANT(LOAD_TPL_CANNOT_STAT, fname);

        if (! S_ISREG(stbf.st_mode)) {
            errno = EINVAL;
            AG_CANT(LOAD_TPL_IRREGULAR, fname);
        }

        if (outfile_time < stbf.st_mtime)
            outfile_time = stbf.st_mtime;
    }

    text_mmap(tpl_file, PROT_READ|PROT_WRITE, MAP_PRIVATE, &map_info);
    if (TEXT_MMAP_FAILED_ADDR(map_info.txt_data))
        AG_ABEND(aprf(LOAD_TPL_CANNOT_OPEN, tpl_file));

    if (dep_fp != NULL)
        add_source_file(tpl_file);

    /*
     *  Process the leading pseudo-macro.  The template proper
     *  starts immediately after it.
     */
    {
        macro_t * sv_mac = cur_macro;
        templ_t * res;
        cur_macro = NULL;

        res = digest_tpl(&map_info, tpl_file);
        cur_macro = sv_mac;
        text_munmap(&map_info);

        return res;
    }
}

/**
 * Deallocate anything related to a template.
 * This includes the pointer passed in and any macros that have an
 * unload procedure associated with it.
 *
 *  @param[in] tpl  the template to unload
 */
LOCAL void
tpl_unload(templ_t * tpl)
{
    macro_t * mac = tpl->td_macros;
    int ct = tpl->td_mac_ct;

    while (--ct >= 0) {
        unload_proc_p_t proc;
        unsigned int ix = mac->md_code;

        /*
         * "select" functions get remapped, depending on the alias used for
         * the selection.  See the "mac_func_t" enumeration in functions.h.
         */
        if (ix >= FUNC_CT)
            ix = FTYP_SELECT;

        proc = unload_procs[ ix ];
        if (proc != NULL)
            (*proc)(mac);

        mac++;
    }

    AGFREE((void*)(tpl->td_file));
    AGFREE(tpl);
}

/**
 *  This gets called when all is well at the end.
 *  The supplied template and all named templates are unloaded.
 *
 *  @param[in] tpl  the last template standing
 */
LOCAL void
cleanup(templ_t * tpl)
{
    if (HAVE_OPT(USED_DEFINES))
        print_used_defines();

    if (dep_fp != NULL)
        wrap_up_depends();

    optionFree(&autogenOptions);

    for (;;) {
        tpl_unload(tpl);
        tpl = named_tpls;
        if (tpl == NULL)
            break;
        named_tpls = (templ_t*)(void*)(tpl->td_scan);
    }

    free_for_context(true);
    unload_defs();
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/tpLoad.c */
