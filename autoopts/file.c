
/*
 *  $Id: file.c,v 4.3 2008/01/23 00:35:27 bkorb Exp $
 *  Time-stamp:      "2007-10-30 11:36:13 bkorb"
 *
 *  This file is part of AutoOpts, a companion to AutoGen.
 *  AutoOpts is free software.
 *  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *  AutoOpts is copyright (c) 1992-2008 by Bruce Korb - all rights reserved
 *
 *  AutoOpts is available under any one of two licenses.  The license
 *  in use must be one of these two and the choice is under the control
 *  of the user of the license.
 *
 *   The GNU Lesser General Public License, version 3 or later
 *      See the files "COPYING.lgplv3" and "COPYING.gplv3"
 *
 *   The Modified Berkeley Software Distribution License
 *      See the file "COPYING.mbsd"
 *
 *  These files have the following md5sums:
 *
 *  239588c55c22c60ffe159946a760a33e pkg/libopts/COPYING.gplv3
 *  fa82ca978890795162346e661b47161a pkg/libopts/COPYING.lgplv3
 *  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd
 */

/*=export_func  optionFileCheck
 * private:
 *
 * what:  Decipher a boolean value
 * arg:   + tOptions*     + pOpts    + program options descriptor  +
 * arg:   + tOptDesc*     + pOptDesc + the descriptor for this arg +
 * arg:   + teOptFileType + ftype    + File handling type          +
 * arg:   + tuFileMode    + mode     + file open mode (if needed)  +
 *
 * doc:
 *   Make sure the named file conforms with the file type mode.
 *   The mode specifies if the file must exist, must not exist or may
 *   (or may not) exist.  The mode may also specify opening the
 *   file: don't, open just the descriptor (fd), or open as a stream
 *   (FILE* pointer).
=*/
void
optionFileCheck(tOptions* pOpts, tOptDesc* pOD,
                teOptFileType ftype, tuFileMode mode)
{
    char * fname;

    if (pOD == NULL) {
        switch (ftype & FTYPE_MODE_EXIST_MASK) {
        case FTYPE_MODE_MUST_NOT_EXIST:
            fputs(zFileCannotExist, option_usage_fp);
            break;

        case FTYPE_MODE_MUST_EXIST:
            fputs(zFileMustExist, option_usage_fp);
            break;
        }
        return;
    }

    {
        struct stat sb;
        AGDUPSTR(fname, pOD->optArg.argString, "copy file name");

        errno = 0;

        switch (ftype & FTYPE_MODE_EXIST_MASK) {
        case FTYPE_MODE_MUST_NOT_EXIST:
            if (  (stat(fname, &sb) == 0)
               || (errno != ENOENT) ){
                if (errno == 0)
                    errno = EINVAL;
                fprintf(stderr, zFSOptError, errno, strerror(errno),
                        "stat-ing for non-existant", pOD->pz_Name, fname);
                pOpts->pUsageProc(pOpts, EXIT_FAILURE);
                /* NOTREACHED */
            }
            /* FALLTHROUGH */

        default:
        case FTYPE_MODE_MAY_EXIST:
        {
            char * p = strrchr(fname, DIRCH);
            if (p != NULL)
                *p = NUL;
            if (  (stat(fname, &sb) != 0)
               || (errno = EINVAL, ! S_ISDIR(sb.st_mode)) ){
                fprintf(stderr, zFSOptError, errno, strerror(errno),
                        "stat-ing for directory", pOD->pz_Name, fname);
                pOpts->pUsageProc(pOpts, EXIT_FAILURE);
                /* NOTREACHED */
            }
            *p = '/';
            break;
        }

        case FTYPE_MODE_MUST_EXIST:
            if (  (stat(fname, &sb) != 0)
               || (errno = EINVAL, ! S_ISREG(sb.st_mode)) ){
                fprintf(stderr, zFSOptError, errno, strerror(errno),
                        "stat-ing for regular", pOD->pz_Name, fname);
                pOpts->pUsageProc(pOpts, EXIT_FAILURE);
                /* NOTREACHED */
            }
            break;
        }
    }

    switch (ftype & FTYPE_MODE_OPEN_MASK) {
    default:
    case FTYPE_MODE_NO_OPEN:
        break;

    case FTYPE_MODE_OPEN_FD:
    {
        int fd = open(fname, mode.file_flags);
        if (fd < 0) {
            fprintf(stderr, zFSOptError, errno, strerror(errno),
                    "open-ing", pOD->pz_Name, fname);
            pOpts->pUsageProc(pOpts, EXIT_FAILURE);
            /* NOTREACHED */
        }
        pOD->optArg.argFd = fd;
        break;
    }

    case FTYPE_MODE_FOPEN_FP:
    {
        FILE* fp = fopen(fname, mode.file_mode);
        if (fp == NULL) {
            fprintf(stderr, zFSOptError, errno, strerror(errno),
                    "fopen-ing", pOD->pz_Name, fname);
            pOpts->pUsageProc(pOpts, EXIT_FAILURE);
            /* NOTREACHED */
        }
        pOD->optArg.argFp = fp;
        break;
    }
    }

    AGFREE(fname);
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autoopts/file.c */
