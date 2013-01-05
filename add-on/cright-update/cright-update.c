
/**
 *  \file cright-update.c
 *
 *  cright-update Copyright (C) 1992-2013 by Bruce Korb - all rights reserved
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
#include <stdbool.h>
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

/**
 * find the line prefix before the current copyright mark.
 */
static char *
find_prefix(char const * ftext, regoff_t off)
{
    char const * const cright_mark = ftext + off;
    char const * scan;
    char const * end = SPN_SPACY_NAME_BACK(ftext, cright_mark);

    /*
     * "end" has trimmed off any trailing names by backing up over
     * name-looking sequences, but then going back forward over
     * leading horizontal white space.
     */
    scan = end = SPN_HORIZ_WHITE_CHARS(end);

    /*
     * Find the start of the line.  Make sure "end" follows any "dnl".
     */
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
        end = SPN_HORIZ_WHITE_CHARS(end);
        if (end > cright_mark)
            end = cright_mark;
    }

    {
        size_t len = end - scan;
        char * pz  = malloc(len + 1);
        if (pz == NULL)
            fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "copyright prefix");
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
    cright_update_exit_code_t res = CRIGHT_UPDATE_EXIT_SUCCESS;

    char * pe;
    bool   need_sp = false;

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
            need_sp = false;
        }

        if (need_sp)
            putc(' ', fp);

        fwrite(list, len, 1, fp);
        need_sp = true;
        list    = SPN_WHITESPACE_CHARS(list + len + 1);
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

/**
 * check copyright ownership.  The --owner string must match, though
 * the "by " clause is skipped both in ownership and input text.
 *
 * @param[in] scan  current scan point
 * @param[in] pfx   the copyright statement prefix
 *                  (ownership may be on the next line)
 * @param[in] p_len prefix length
 *
 * @returns true (it is okay) or false (not).
 */
static bool
ownership_ok(char const * scan, char const * pfx, size_t p_len)
{
    char const * owner = OPT_ARG(OWNER);

    /*
     * IF we are ignoring ownership, ...
     */
    if ((owner == NULL) || (*owner == NUL)) {
        if (HAVE_OPT(VERBOSE))
            fputs("ownership ignored\n", stderr);
        return true;
    }

    {
        static char const by[] = "by ";
        static int  const byln = sizeof(by) - 1;
        if (strncmp(owner, by, byln) == 0)
            owner = SPN_HORIZ_WHITE_CHARS(owner + byln);

        if (strncmp(scan, by, byln) == 0)
            scan = SPN_HORIZ_WHITE_CHARS(scan + byln);
    }

    do  {
        /*
         * If there is a space character in the ownership, then there
         * must be one in the scan, too.  If not, then neither may have
         * a space character.  The number of space characters is ignored.
         */
        if (IS_WHITESPACE_CHAR(*owner)) {
            owner = SPN_HORIZ_WHITE_CHARS(owner);
            if (*owner == NUL)
                break;
            if (! IS_WHITESPACE_CHAR(*scan))
                return false;

            scan = SPN_HORIZ_WHITE_CHARS(scan);

            /*
             * If the input text has a newline, skip it and the required
             * following prefix and any white space after that.
             * At that point, the owner name must continue.  No newline.
             */
            if (*scan == '\n') {
               if (strncmp(scan + 1, pfx, p_len) != 0)
                   return false;
               scan = SPN_HORIZ_WHITE_CHARS(scan + p_len + 1);
               if (*scan == '\n')
                   return false;
            }
        }

        /*
         * The owner character is not white space and not NUL,
         * the input text character must match.
         */
        if (*(owner++) != *(scan++))
            return false;
    } while (*owner != NUL);
    return true;
}

/**
 * Find the next token in the year list string.  Used in the FSM.
 *
 * @param[in,out] plist  pointer to pointer scanning the year list
 * @param[in,out] val    previous year value we replace with current year
 *
 * @returns the token type found
 */
te_cyr_event
get_next_token(char ** plist, unsigned long * val)
{
    char * p = SPN_WHITESPACE_CHARS(*plist);
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
    *plist = SPN_WHITESPACE_CHARS(p);

    return CYR_EV_YEAR;
}

static char *
extract_years(char ** ftext, size_t * fsize,
              int * col, char const * pfx, size_t p_len,
              char * const yrbuf, size_t yrbuf_len)
{
    char * scan = *ftext;
    char * buf  = yrbuf;
    bool   was_sp;
    char * const bfend = yrbuf + yrbuf_len;

    {
        int cc = 0;
        char * p = scan;
        while (*--p != '\n')  cc++;
        *col = cc;
    }

    {
        char * next  = SPN_WHITESPACE_CHARS(scan);
        long   delta = next - scan;
        if (delta > 0) {
            *fsize -= delta;
            *ftext  = scan = next;
        }
    }

    /*
     *  Copy over the copyright years
     */
    was_sp = false;
    while (IS_YEAR_LIST_CHAR(*scan)) {
        if (! IS_WHITESPACE_CHAR(*scan)) {
            *(buf++) = *(scan++);
            was_sp   = false;

        } else if (*scan == '\n') {
            /*
             *  Newline.  Skip over the newline, the line prefix and
             *  any remaining white space on the line.  Another newline,
             *  and we bail out.  We must find something on this new line.
             */
            char * p = scan;
            while ((*p == '\n') && (strncmp(p + 1, pfx, p_len) == 0)) {
                p += p_len + 1;
                p = SPN_WHITESPACE_CHARS(p);
            }
            if (*p == '\n')
                break;

            if (! was_sp) {
                *(buf++) = ' ';
                was_sp   = true;
            }
            scan = p;

        } else {
            scan = SPN_HORIZ_WHITE_CHARS(scan);
            if (! was_sp) {
                *(buf++) = ' ';
                was_sp   = true;
            }
        }

        if (buf >= bfend)
            return NULL;
    }

    /*
     * If we are worrying about ownership, it should be next in the
     * input stream.  Check that it matches.
     */
    if (! ownership_ok(scan, pfx, p_len)) {
        fprintf(stderr, "ownership failed: %s\n", OPT_ARG(OWNER));
        return NULL;
    }

    /*
     * back up over any trailing year separation characters
     */
    scan = SPN_YEAR_SEP_BACK(*ftext, scan);
    buf  = SPN_YEAR_SEP_BACK(yrbuf, buf);
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
    if (scan == NULL) {
        if (HAVE_OPT(VERBOSE))
            fputs("no years found in copyright\n", stderr);
        return CRIGHT_UPDATE_EXIT_NO_UPDATE;
    }

    /*
     * We have a valid list of years.  If we have a range that ends with last
     * year, replace it with the current year.  Otherwise, append the new year.
     */
    do  {
        static char const no_room[] = "OOPS: no room for new year\n";
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
        if (strcmp(pz - 4, OPT_ARG(NEW_YEAR)) == 0) {
            if (HAVE_OPT(VERBOSE))
                fputs("already updated\n", stderr);
            return CRIGHT_UPDATE_EXIT_NO_UPDATE;
        }

        /*
         * IF --no-join-years was specified, append year with a comma
         * If a year was missed, then append the year with a comma
         */
        if (  (! ENABLED_OPT(JOIN_YEARS))
           || (strcmp(pz - 4, last_year) != 0)) {

            if (pz >= z + sizeof(z) - 7) {
                if (HAVE_OPT(VERBOSE))
                    fputs(no_room, stderr);
                return CRIGHT_UPDATE_EXIT_NO_UPDATE;
            }

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

        if (pz >= z + sizeof(z) - 6) {
            if (HAVE_OPT(VERBOSE))
                fputs(no_room, stderr);
            return CRIGHT_UPDATE_EXIT_NO_UPDATE;
        }

        /*
         * Last year was start of new year range
         */
        *(pz++) = '-';
        strcpy(pz, OPT_ARG(NEW_YEAR));
    } while (0);

    emit_years(fp, z, col, pfx, p_len, scan, ftext, fsize);
    return CRIGHT_UPDATE_EXIT_SUCCESS;
}

/**
 * See if a file is a C or C++ file, based only on the suffix.
 * "C" suffixes are ".c" and ".h", with or without "xx" or "++" following.
 * Upper and lower case versions are accepted, but all letters must be
 * all one case.
 *
 * @param[in] fname  the file name to test
 * @returns true or false, depending.
 */
static bool
is_c_file_name(char const * fname)
{
    /*
     * Find the "." introducing the suffix.  (The last period.)
     */
    fname = strrchr(fname, '.');
    if (fname == NULL)
        return false;

    switch (fname[1]) {
    case 'h':
    case 'c':
        if (fname[2] == NUL)
            return true;
        if (strcmp(fname + 2, "pp") == 0)
            return true;
        if (strcmp(fname + 2, "xx") == 0)
            return true;
        return strcmp(fname + 2, "++") == 0;

    case 'H':
    case 'C':
        if (fname[2] == NUL)
            return true;
        if (strcmp(fname + 2, "XX") == 0)
            return true;
        return strcmp(fname + 2, "++") == 0;

    default:
        return false;
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
        fserr(CRIGHT_UPDATE_EXIT_FSERR, "stat", fname);

    if (chmod(tname, sb.st_mode & mode_mask) != 0)
        fserr(CRIGHT_UPDATE_EXIT_FSERR, "chmod", fname);

    utbf.modtime = sb.st_mtime;
    utime(tname, &utbf);

    if (rename(tname, fname) != 0)
        fserr(CRIGHT_UPDATE_EXIT_FSERR, "renaming temp file", tname);
}

static int
doit(char const * fname, char * ftext, size_t fsize,
     regex_t * preg, regmatch_t * match)
{
    char * tname  = malloc(strlen(fname) + 8);
    FILE * fp     = NULL;
    int    fixct  = 0;
    int    is_c   = is_c_file_name(fname);
    char * prefix = NULL;
    size_t p_len;

    cright_update_exit_code_t res = CRIGHT_UPDATE_EXIT_SUCCESS;

    if (tname == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "file name");

    sprintf(tname, "%s-XXXXXX", fname);
    {
        int fd = mkstemp(tname);
        if (fd < 0)
            fserr(CRIGHT_UPDATE_EXIT_FSERR, "mkstemp", tname);

        fp = fdopen(fd, "w");
    }
    if (fp == NULL)
        fserr(CRIGHT_UPDATE_EXIT_FSERR, "fdopen", tname);

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
        if (HAVE_OPT(VERBOSE))
            fprintf(stderr, "no fixups applied to %s\n", fname);
        unlink(tname);
    }

    free(tname);
    printf(report_fmt,
           (res == CRIGHT_UPDATE_EXIT_SUCCESS) ? "updated" : "NOT UPDATED",
           fname);
    return res;
}

static regex_t *
initialize(void)
{
    static int const cr_flags =
        REG_EXTENDED | REG_ICASE | REG_NEWLINE;

    regex_t * preg = malloc(sizeof(* preg));
    int       reres;

    if (preg == NULL)
        fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "regex struct");
    reres = regcomp(preg, OPT_ARG(COPYRIGHT_MARK), cr_flags);

    if (reres != 0)
        die(CRIGHT_UPDATE_EXIT_REGCOMP, "regcomp failed on:  %s\n",
            OPT_ARG(COPYRIGHT_MARK));

    if (OPT_ARG(NEW_YEAR) == NULL) {
        char *      pz;
        time_t      ctim = time(NULL);
        struct tm * tmp  = localtime(&ctim);

        if (asprintf(&pz, "%4d", tmp->tm_year + 1900) < 4)
            fserr(CRIGHT_UPDATE_EXIT_NOMEM, "allocation", "year string");
        SET_OPT_NEW_YEAR(pz);
    }

    {
        char * pz;
        int yr = strtoul(OPT_ARG(NEW_YEAR), &pz, 10);
        if ((yr < 1900) || (yr > 2200) || (*pz != NUL))
            die(CRIGHT_UPDATE_EXIT_BAD_YEAR, "invalid year specified: %s\n",
                OPT_ARG(NEW_YEAR));

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
        die(CRIGHT_UPDATE_EXIT_REGEXEC, "regexec failed in %s:  %s\n",
            fname, OPT_ARG(COPYRIGHT_MARK));

    case 0:
        if (match[1].rm_so > 0)
            return doit(fname, ftext, fsize, preg, match);
        /* FALLTHROUGH */

    case REG_NOMATCH:
        printf(report_fmt, "no match", fname);
        return CRIGHT_UPDATE_EXIT_SUCCESS;
    }
}
