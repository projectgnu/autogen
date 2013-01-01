
/**
 *  \file cright-update.c
 *
 *  cright-update Copyright (c) 1992-2013 by Bruce Korb - all rights reserved
 *
 * cright-update is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * cright-update is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define  _GNU_SOURCE   1
#define  _XOPEN_SOURCE 600

#include <sys/stat.h>
#include <sys/types.h>

#include "opts.h"
#define  option_data cright_updateOptions

#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define  CRIGHT_UPDATE 1
#include "cright-cmap.h"
#include "collapse-fsm.h"

#ifndef NUL
#define NUL '\0'
#endif
#define RIDICULOUS_YEAR_LEN 8192

static char * scan_buffer  = NULL;
static char * compress_buf = NULL;

static char const report_fmt[] = "%-11s %s\n";

char * last_year = NULL;

static void
abort_bad_re(int reres, regex_t * preg)
{
    char z[1024];

    (void) regerror(reres, preg, z, sizeof(z));
    fprintf(stderr, "%s regcomp error %d (%s) compiling:\n%s\n",
            option_data.pzProgName, reres, z,
            OPT_ARG(COPYRIGHT_MARK));
    exit(CRIGHT_UPDATE_EXIT_REGCOMP);
}

static void
abort_bad_yr(void)
{
    fprintf(stderr, "%s invalid new year:  %s\n",
            option_data.pzProgName, OPT_ARG(NEW_YEAR));
    exit(CRIGHT_UPDATE_EXIT_REGCOMP);
}

static void
abort_re_srch_fail(int reres, regex_t * preg)
{
    char z[1024];

    (void) regerror(reres, preg, z, sizeof(z));
    fprintf(stderr, "%s regexec error %d (%s) searching for %s\n",
            option_data.pzProgName, reres, z,
            OPT_ARG(COPYRIGHT_MARK));
    exit(CRIGHT_UPDATE_EXIT_REGEXEC);
}

static void
abort_enomem(char const * typ, size_t len)
{
    fprintf(stderr, "%s failure: allocation of %d bytes for %s failed\n",
            option_data.pzProgName, (int)len, typ);
    exit(CRIGHT_UPDATE_EXIT_NOMEM);
}

static void
abort_fserr(char const * typ, char const * fname)
{
    fprintf(stderr, "%s failure: fs error %d (%s) %s-ing %s\n",
            option_data.pzProgName, errno, strerror(errno),
            typ, fname);
    exit(CRIGHT_UPDATE_EXIT_MKSTEMP);
}

/**
 * find the line prefix before the current copyright mark.
 */
static char *
find_prefix(char const * ftext, regoff_t off)
{
    char const * const cright_mark = ftext + off;
    char const * scan = cright_mark;
    char const * end  = scan;

    /*
     * Find the end of the prefix.  Back up over spaces and name
     * characters.
     */
    while ((end > ftext) && IS_SPACY_NAME_CHAR(end[-1])) end--;

    /*
     * Now go forward back over any initial white space.
     */
    while ((end < scan)  && IS_HORIZ_WHITE_CHAR(end[0])) end++;

    /*
     * Find the start of the line.  Make sure "end" follows any "dnl".
     */
    scan = end;
    while ((scan > ftext) && (scan[-1] != '\n'))    scan--;

    /*
     * "end" now points to the the end of the candidate prefix
     * and "scan" points to the start.  If they point to the same place
     * and if the copyright mark is beyond the beginning of the line,
     * then we have to decide if the string at the start of the line is
     * a name and the prefix is zero length, or if the name-like thing
     * is actually a comment indicator.  Presume a comment if the "name"
     * is either "rem" or "dnl" ("remark" or "Delete to New Line").
     */
    if (  (end == scan)
       && ((strncmp(scan, "dnl", 3) == 0) || (strncmp(scan, "rem", 3) == 0))
       && IS_HORIZ_WHITE_CHAR(scan[4])
       && (end < cright_mark)) {

        end += 4;
        while (IS_HORIZ_WHITE_CHAR(*end)) end++;
        if (end > cright_mark)
            end = cright_mark;
    }

    {
        size_t len = end - scan;
        char * pz  = malloc(len + 1);
        if (pz == NULL)
            abort_enomem("copyright prefix", len+1);
        if (len > 0)
            memcpy(pz, scan, len);
        pz[len] = NUL;

        return pz;
    }
}

static void
emit_years(FILE * fp, char const * list, int cur_col, char const * pfx,
           size_t plen, char const * rest, char ** txt, size_t * tlen)
{
    cright_update_exit_code_t res =
        CRIGHT_UPDATE_EXIT_SUCCESS;

    char * pe;
    int    need_sp = 0;

    if (OPT_VALUE_WIDTH == 0) {
        fputs(list, fp);
        return;
    }

    while (pe = strchr(list, ' '), pe != NULL) {
        size_t len = pe - list;
        cur_col += len + 1;

        if (cur_col > OPT_VALUE_WIDTH) {
            fprintf(fp, "\n%s", pfx);
            cur_col = plen + len;
            need_sp = 0;
        }

        if (need_sp)
            putc(' ', fp);

        fwrite(list, len, 1, fp);
        need_sp = 1;
        list += len + 1;
        while (IS_WHITESPACE_CHAR(*list))  list++;
    }

    if (need_sp)
        putc(' ', fp);
    fputs(list, fp);

    pe = strchr(rest, '\n');
    if (pe != NULL) {
        size_t len = pe - rest;
        if (cur_col + len > OPT_VALUE_WIDTH)
            fprintf(fp, "\n%s", pfx);
    }

    *tlen -= rest - *txt;
    *txt   = (char *)rest;
}

static int
ownership_ok(char const * scan, char const * pfx, size_t p_len)
{
    char const * owner = OPT_ARG(OWNER);
    if ((owner == NULL) || (*owner == NUL))
        return 1;

    do  {
        /*
         * If there is a space character in the ownership, then there
         * must be one in the scan, too.  If not, then neither may have
         * a space character.  The number of space characters is ignored.
         */
        if (IS_WHITESPACE_CHAR(*owner)) {
            while (IS_WHITESPACE_CHAR(*owner)) owner++;
            if (*owner == NUL)
                break;
            if (! IS_WHITESPACE_CHAR(*scan))
                return 0;

            while (IS_WHITESPACE_CHAR(*scan)) {
                if ((*scan == '\n') && (strncmp(scan + 1, pfx, p_len) == 0)) {
                    scan += p_len + 1;
                    while (IS_HORIZ_WHITE_CHAR(*scan))  scan++;
                }
                while (IS_HORIZ_WHITE_CHAR(*scan))  scan++;
            }

        } else if (IS_WHITESPACE_CHAR(*scan))
            return 0;

        if (*(owner++) != *(scan++))
            return 0;
    } while (*owner != NUL);
    return 1;
}

te_cyr_event
get_next_token(char ** plist, unsigned long * val)
{
    char * p = *plist;
    while (IS_WHITESPACE_CHAR(*p))  p++;
    *plist = p + 1;

    switch (*p) {
    case ',':
    {
        unsigned long v;
        errno = 0;
        v = strtoul(*plist, NULL, 10);
        if ((errno != 0) || (v < 1900) || (v > 3000))
            return CYR_EV_INVALID;
        return (v == (*val + 1)) ? CYR_EV_COMMA_SEQ : CYR_EV_COMMA;
    }
    case '-': return CYR_EV_HYPHEN;
    case NUL: return CYR_EV_END;
    default:  return CYR_EV_INVALID;
    case '0' ... '9':
        break;
    }

    /*
     * We have a year, so parse it.
     */
    errno = 0;
    *val  = strtoul(p, &p, 10);
    if ((errno != 0) || (*val < 1900) || (*val > 3000))
        return CYR_EV_INVALID;
    while (IS_WHITESPACE_CHAR(*p))  p++;
    *plist = p;

    return CYR_EV_YEAR;
}

static char *
extract_years(char ** ftext, size_t * fsize,
              int * col, char const * pfx, size_t p_len,
              char * const yrbuf, size_t yrbuf_len)
{
    char * scan = *ftext;
    char * buf  = yrbuf;
    int    was_sp;
    char * const bfend = yrbuf + yrbuf_len;

    {
        int cc = 0;
        char * p = scan;
        while (*--p != '\n')  cc++;
        *col = cc;
    }

    if (IS_WHITESPACE_CHAR(*scan)) {
        do  {
            (*fsize)--;
            scan++;
        } while (IS_WHITESPACE_CHAR(*scan));
        *ftext = scan;
    }

    /*
     *  Copy over the copyright years
     */
    was_sp = 0;
    while (IS_YEAR_LIST_CHAR(*scan)) {
        if (! IS_WHITESPACE_CHAR(*scan)) {
            *(buf++) = *(scan++);
            was_sp  = 0;

        } else if (*scan == '\n') {
            char * p = scan;
            while ((*p == '\n') && (strncmp(p + 1, pfx, p_len) == 0)) {
                p += p_len + 1;
                while (IS_HORIZ_WHITE_CHAR(*p))  p++;
            }
            if (*p == '\n')
                break;

            if (! was_sp) {
                *(buf++) = ' ';
                was_sp  = 1;
            }
            scan = p;

        } else {
            while (IS_HORIZ_WHITE_CHAR(*scan))  scan++;
            if (! was_sp) {
                *(buf++) = ' ';
                was_sp  = 1;
            }
        }

        if (buf >= bfend)
            return NULL;
    }

    /*
     * If we are worrying about ownership, it should be next in the
     * input stream.  Check that it matches.
     */
    if (! ownership_ok(scan, pfx, p_len))
        return NULL;

    /*
     * back up over any trailing year separation characters
     */
    while ((scan > *ftext) && IS_YEAR_SEP_CHAR(scan[-1])) scan--;

    while ((buf > yrbuf) && IS_YEAR_SEP_CHAR(buf[-1])) buf--;
    *buf = NUL;

    /*
     * IF --join-years was not specified or if --no-join-years was specified,
     * THEN do not compress the year list.
     */
    if ((! HAVE_OPT(JOIN_YEARS)) || (! ENABLED_OPT(JOIN_YEARS)))
        return scan;

    return (cyr_run_fsm(yrbuf) == CYR_ST_DONE) ? scan : NULL;
}

static cright_update_exit_code_t
fixup(char ** ftext, size_t * fsize, char const * pfx, size_t p_len,
      FILE* fp)
{
    char   z[RIDICULOUS_YEAR_LEN];
    int    col;
    char * scan = extract_years(ftext, fsize, &col, pfx, p_len, z, sizeof(z));
    if (scan == NULL)
        return CRIGHT_UPDATE_EXIT_NO_UPDATE;

    /*
     * We have a valid list of years.  If we have a range that ends with last
     * year, replace it with the current year.  Otherwise, append the new year.
     */
    do  {
        char * pz = z + strlen(z);

        /*
         * If we don't have any years, then just supply the current one.
         */
        if (pz < z + 4) {
            strcpy(z, OPT_ARG(NEW_YEAR));
            break;
        }

        /*
         * If the new year is already there, skip this one.
         */
        if (strcmp(pz - 4, OPT_ARG(NEW_YEAR)) == 0)
            return CRIGHT_UPDATE_EXIT_NO_UPDATE;

        /*
         * IF --no-join-years was specified, append year with a comma
         * If a year was missed, then append the year with a comma
         */
        if (  (! ENABLED_OPT(JOIN_YEARS))
           || (strcmp(pz - 4, last_year) != 0)) {

            if (pz >= z + sizeof(z) - 7)
                return CRIGHT_UPDATE_EXIT_NO_UPDATE;

            sprintf(pz, ", %s", OPT_ARG(NEW_YEAR));
            break;
        }

        /*
         * If last year was the end of a range, replace the year
         */
        if ((pz >= z + 5) && (pz[-5] == '-')) {
            strcpy(pz - 4, OPT_ARG(NEW_YEAR));
            break;
        }

        if (pz >= z + sizeof(z) - 6)
            return CRIGHT_UPDATE_EXIT_NO_UPDATE;

        /*
         * Last year was start of new year range
         */
        *(pz++) = '-';
        strcpy(pz, OPT_ARG(NEW_YEAR));
    } while (0);

    emit_years(fp, z, col, pfx, p_len, scan, ftext, fsize);
    return CRIGHT_UPDATE_EXIT_SUCCESS;
}

static int
is_c_file_name(char const * fname)
{
    fname = strrchr(fname, '.');
    if (fname == NULL)
        return 0;

    switch (fname[1]) {
    case 'h':
    case 'c':
        if (fname[2] == NUL)
            return 1;
        if (strcmp(fname + 2, "pp") == 0)
            return 1;
        if (strcmp(fname + 2, "xx") == 0)
            return 1;
        return strcmp(fname + 2, "++") == 0;

    case 'H':
    case 'C':
        if (fname[2] == NUL)
            return 1;
        if (strcmp(fname + 2, "XX") == 0)
            return 1;
        return strcmp(fname + 2, "++") == 0;

    default:
        return 0;
    }
}

static void
mv_file(char const * fname, char const * tname)
{
    static mode_t const mode_mask =
        S_IRWXU | S_IRWXG | S_IRWXO;

    struct stat sb;
    struct utimbuf utbf = {
        .actime   = time(NULL) };

    if (stat(fname, &sb) != 0)
        abort_fserr("stat", fname);

    chmod(tname, sb.st_mode & mode_mask);
    utbf.modtime = sb.st_mtime;
    utime(tname, &utbf);

    rename(tname, fname);
}

static int
doit(char const * fname, char * ftext, size_t fsize,
     regex_t * preg, regmatch_t * match)
{
    char * tname  = malloc(strlen(fname) + 8);
    int    fd     = -1;
    FILE * fp     = NULL;
    int    fixct  = 0;
    int    is_c   = is_c_file_name(fname);
    char * prefix = NULL;
    size_t p_len;

    cright_update_exit_code_t res =
        CRIGHT_UPDATE_EXIT_SUCCESS;

    if (tname == NULL)
        abort_enomem("file name", strlen(fname) + 8);

    sprintf(tname, "%s-XXXXXX", fname);
    fd = mkstemp(tname);
    if (fd < 0)
        abort_fserr("mkstemp", fname);

    fp = fdopen(fd, "w");
    if (fp == NULL)
        abort_fserr("fdopen", tname);

    do  {
        regoff_t const so = match[1].rm_so;
        fwrite(ftext, so, 1, fp);
        if (prefix != NULL)
            free(prefix);
        prefix = find_prefix(ftext, match[0].rm_so);
        p_len  = strlen(prefix);
        ftext += so;
        fsize -= so;
        if (is_c && (prefix[0] == '/') && (prefix[1] == '*'))
            *prefix = ' ';

        if (fixup(&ftext, &fsize, prefix, p_len, fp)
            == CRIGHT_UPDATE_EXIT_SUCCESS) {
            fixct++;
            if (! HAVE_OPT(ALL))
                break;
        }
    } while (regexec(preg, ftext, 2, match, 0) == 0);

    fwrite(ftext, fsize, 1, fp);
    if (prefix != NULL)
        free(prefix);
    fclose(fp);
    fp = NULL;

    if (fixct > 0)
        mv_file(fname, tname);

    else {
        res = CRIGHT_UPDATE_EXIT_NO_UPDATE;
        unlink(tname);
    }

    free(tname);
    printf(report_fmt, res ? "NOT UPDATED" : "updated", fname);
    return res;
}

static regex_t *
initialize(void)
{
    static int const cr_flags =
        REG_EXTENDED | REG_ICASE | REG_NEWLINE;

    regex_t * preg = malloc(sizeof(* preg));
    int       reres;
    char *    re = OPT_ARG(COPYRIGHT_MARK);

    if (preg == NULL)
        abort_enomem("regex", sizeof(* preg));

    reres = regcomp(preg, re, cr_flags);
    if (reres != 0)
        abort_bad_re(reres, preg);

    if (OPT_ARG(NEW_YEAR) == NULL) {
        char *      pz;
        time_t      ctim = time(NULL);
        struct tm * tmp  = localtime(&ctim);

        if (asprintf(&pz, "%4d", tmp->tm_year + 1900) < 4)
            abort_enomem("asprintf of year", 5);
        SET_OPT_NEW_YEAR(pz);
    }

    {
        char * pz;
        int yr = strtoul(OPT_ARG(NEW_YEAR), &pz, 10);
        if ((yr < 1900) || (yr > 2200) || (*pz != NUL))
            abort_bad_yr();

        asprintf(&last_year, "%4d", yr - 1);
    }

    return preg;
}

int
fix_copyright(char const * fname, char * ftext, size_t fsize)
{
    static regex_t * preg = NULL;

    int        reres;
    regmatch_t match[2];

    if (preg == NULL)
        preg = initialize();

    reres = regexec(preg, ftext, 2, match, 0);
    switch (reres) {
    default:
        abort_re_srch_fail(reres, preg);

    case 0:
        if (match[1].rm_so > 0)
            return doit(fname, ftext, fsize, preg, match);
        /* FALLTHROUGH */

    case REG_NOMATCH:
        printf(report_fmt, "skipped - no match", fname);
        return CRIGHT_UPDATE_EXIT_SUCCESS;
    }
}
