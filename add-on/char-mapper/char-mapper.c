
/**
 * \file char-mapper.c
 *
 *  Time-stamp:        "2012-02-12 09:55:29 bkorb"
 *
 *  This is the main routine for char-mapper.
 *
 *  This file is part of char-mapper.
 *  char-mapper Copyright (c) 1992-2012 by Bruce Korb - all rights reserved
 *
 * char-mapper is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * char-mapper is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "char-mapper.h"

int
main(int argc, char ** argv)
{
    if (argc > 1) {
        char * pz = argv[1];
        if (*pz == '-')
            parse_help(pz);
        if (freopen(argv[1], "r", stdin) != stdin) {
            fprintf(stderr, "fs error %d (%s) reopening '%s' as stdin\n",
                    errno, strerror(errno), pz);
            exit(EXIT_FAILURE);
        }

    } else if (isatty(STDIN_FILENO))
        parse_help(NULL);

    else argv[1] = "stdin";

    bit_count = read_data();

    {
        time_t tm = time(NULL);
        struct tm * tmp = localtime(&tm);

        strftime(buffer, BUF_SIZE, "%x %X", tmp);
        printf(leader_z, buffer, commentary, file_guard);
    }

    if (fseek(stdin, 0, SEEK_SET) == 0)
        copy_input_text(argv[1]);

    {
        int width = (bit_count+3)/4;
        if (width < 2) width = 2;
        sprintf(mask_fmt, mask_fmt_fmt, width);
        if (width > 8)
            strcat(mask_fmt, "ULL");
    }

    init_names();
    emit_macros(bit_count);
    emit_table(bit_count);
    emit_functions();
    return 0;
}

void
die(char const * fmt, ...)
{
    va_list ap;
    fputs("char-mapper error:  ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static void
init_names(void)
{
    static char const tbl[] = "_table";

    if (table_name == NULL) {

        if (file_guard == char_map_gd)
            table_name = "char_type_table";

        else {
            static char const grd[] = "_GUARD";
            char * p = NULL;
            char const * e = file_guard + strlen(file_guard)
                - (sizeof(grd) - 1);

            if (strcmp(e, grd) != 0)
                e += sizeof(grd) - 1;
            else if ((e[-2] == '_') && (e[-1] == 'H'))
                e -= 2;

            size_t copy_len = e - file_guard;
            table_name = p = malloc(copy_len + sizeof(tbl));
            memcpy(p, file_guard, copy_len);
            memcpy(p + copy_len, tbl, sizeof(tbl));
            for (; p < table_name + copy_len; p++)
                *p = tolower((int)*p);
        }
    }

    if (data_guard == NULL) {
        static char const guard_fmt[] = "DEFINE_%s";
        char * p = malloc(strlen(table_name) + sizeof(guard_fmt));
        sprintf(p, guard_fmt, table_name);
        make_define_name(p);
        data_guard = p;
    }

    {
        static char const msk[] = "_mask_t";
        size_t tn_len = strlen(table_name);
        if (strcmp(table_name + tn_len - (sizeof(tbl) - 1), tbl) == 0)
            tn_len -= sizeof(tbl) - 1;
        char * p = malloc(tn_len + sizeof(tbl));
        memcpy(p, table_name, tn_len);
        memcpy(p + tn_len, msk, sizeof(msk));
        mask_name = p;

        p = malloc(tn_len + 1);
        memcpy(p, table_name, tn_len);
        p[tn_len]  = NUL;
        base_fn_nm = p;

        p = malloc(tn_len + 1);
        memcpy(p, table_name, tn_len);
        p[tn_len]  = NUL;
        make_define_name(p);
        base_ucase = p;
    }
}

static void
copy_input_text(char const * name)
{
    printf(copy_input_start, name);
    while (fgets(buffer, BUF_SIZE, stdin) != NULL) {
        if (strstr(buffer, "%comment") != NULL) {
            skip_comment = 1;
            fputs("// %comment -- see above\n", stdout);
            continue;
        }
        if (strstr(buffer, "%emit") != NULL) {
            skip_comment = 1;
            fputs("// %emit code -- see below\n", stdout);
            continue;
        }
        if (skip_comment) {
            int bfix = 0;
            while (isspace(buffer[bfix]))  bfix++;
            if (buffer[bfix] != '%') continue;
            skip_comment = 0;
        }
        printf("// %s", buffer);
    }

    write_const(copy_input_end, stdout);
}

void
parse_help(char const * opt_pz)
{
#   include "cm-usage.c"

    static char const opterrmsg[] = "char-mapper error:  %s:  %s\n";
    int exit_code = EXIT_SUCCESS;
    FILE * fp = stdout;
    char const * pz = opt_pz;

    if (opt_pz == NULL) {
        fp = stderr;
        fprintf(stderr, opterrmsg, "input is from a TTY device",
                "must be file or pipe\n");
        write_const(usage_txt, fp);
        exit_code = EXIT_FAILURE;
    } else

    switch (pz[1]) {
    case '-':
        pz += 2;
        static char const help[] = "help";
        char const * p = help;
        for (;;) {
            if (*pz != *p)
                goto bad_opt;
            if (*(p++) == NUL)
                break;
            if (*++pz == NUL)
                break;
        }
        /* FALLTHROUGH */

    case 'h':
        write_const(usage_txt, fp);
        break;

    default:
    bad_opt:
        exit_code = EXIT_FAILURE;
        fp = stderr;
        fprintf(stderr, opterrmsg, "unknown option", opt_pz);
        pz = usage_txt;
        do putc(*pz, stderr);
        while (*(pz++) != '\n');
    }

    exit(exit_code);
}

void
emit_macros(int bit_count)
{
    char fill[CLASS_NAME_LIMIT+3];

    {
        char const * mask_type;
        switch (bit_count) {
        case  1 ...  8: mask_type = "uint8_t";  break;
        case  9 ... 16: mask_type = "uint16_t"; break;
        case 17 ... 32: mask_type = "uint32_t"; break;
        case 33 ... 64: mask_type = "uint64_t"; break;

        default:
            die("too many char types (%u max)\n", sizeof(long long) * NBBY);
        }
        printf(typedef_mask, mask_type, mask_name);
    }

    if (max_name_len < CLASS_NAME_LIMIT-2)
        max_name_len += 2;
    memset(fill, ' ', max_name_len);
    fill[max_name_len - 1] = NUL;

    for (value_map_t * map = all_map.next; map != NULL; map = map->next) {

        char * pz = fill + strlen(map->vname);
        char z[24]; // 24 >= ceil(log10(1 << 63)) + 1
        snprintf(z, sizeof(z), mask_fmt, map->mask);

        printf(macro_def_fmt, map->vname, pz, base_fn_nm, z);
    }

    putc('\n', stdout);
}

static void
emit_test_code(void)
{
    int ix = 0;
    value_map_t * map;
    printf(testit_fmt, base_ucase);
    for (map = all_map.next; map != NULL; map = map->next) {
        printf(testit_class_names, ix++, map->vname);
    }
    fputs(testit_class_hdrs, stdout);
    for (map = all_map.next, ix=0;
         map != NULL;
         map = map->next) {
        printf(" %02X", ix++);
    }
    fputs(test_loop, stdout);
    for (map = all_map.next, ix=0;
         map != NULL;
         map = map->next) {
        char z[24]; // 24 >= ceil(log10(1 << 63)) + 1
        snprintf(z, sizeof(z), mask_fmt, map->mask);
        printf(each_test, base_fn_nm, z);
        ix++;
    }
    fputs("        putchar('\\n');\n", stdout);
    printf(endtest_fmt, base_ucase);
}

void
emit_functions(void)
{
    if (! table_is_static)
        printf(declare_tbl, mask_name, table_name, table_size);

    printf(inline_functions,
           base_fn_nm, mask_name, table_size, table_name);

    if (add_on_text != NULL)
        printf(emit_text_fmt, add_on_text);

    if (add_test_code)
        emit_test_code();

    printf(endif_fmt, file_guard);
}

void
emit_table(int bit_count)
{
    char const * type_pz;
    char * fmt;

    int ix       = 0;
    int need_cm  = 0;
    int entry_ct = 0;
    int init_ct  = (bit_count > 32) ? 2 : 4;

    if (table_is_static)
        printf(start_static_table_fmt, mask_name, table_name, table_size);
    else
        printf(start_table_fmt, data_guard, mask_name, table_name, table_size);

    while (ix < TABLE_SIZE) {

        if (! need_cm)
            need_cm = 1;
        else putc(',', stdout);

        if (--entry_ct <= 0) {
            putc('\n', stdout);
            putc(' ', stdout);
            entry_ct = init_ct;
        }
        if (isprint(ix))
            printf(" /* %c */ ", (char)ix);
        else switch (ix) {
            case NUL:  fputs(" /*NUL*/ ", stdout); break;
            case '\a': fputs(" /*BEL*/ ", stdout); break;
            case '\b': fputs(" /* BS*/ ", stdout); break;
            case '\t': fputs(" /* HT*/ ", stdout); break;
            case '\n': fputs(" /* NL*/ ", stdout); break;
            case '\v': fputs(" /* VT*/ ", stdout); break;
            case '\f': fputs(" /* FF*/ ", stdout); break;
            case '\r': fputs(" /* CR*/ ", stdout); break;
            case 0x1B: fputs(" /*ESC*/ ", stdout); break;
            default:  printf(" /*x%02X*/ ", ix);    break;
            }

        printf(mask_fmt, all_map.values[ix++]);
    }
    fputs(end_table, stdout);
    if (! table_is_static)
        printf(endif_fmt, data_guard);
}

int
read_data(void)
{
    int bit_num = 0;

    while (fgets(buffer, BUF_SIZE, stdin) != NULL) {
        char nm_buf[CLASS_NAME_LIMIT+1];
        char * scan = trim(buffer);
        value_map_t * map = NULL;
        if (scan == NULL)  continue;

        scan = get_name(scan, nm_buf, sizeof(nm_buf));
        if (curr_name_len > max_name_len)
            max_name_len = curr_name_len;

        while (scan != NULL) {
            switch (*scan) {
            case NUL: goto end_while;
            case '"':
                if (map == NULL)
                    map  = new_map(nm_buf);
                if (map->bit_no == ~0) {
                    map->mask  |= 1 << bit_num;
                    map->bit_no = bit_num;
                    bit_num++;
                }
                scan = scan_quoted_value(scan+1, map, VAL_ADD);
                break;

            case ' ': case '\t':
                while (isspace(*scan))  scan++;
                break;

            case '+':
            case '-':
                if (map == NULL)
                    map  = new_map(nm_buf);
                if (scan[1] == '"')
                     scan = scan_quoted_value(scan+2, map, VAL_REMOVE);
                else scan = copy_another_value(scan, map);
                break;

            case '%':
                scan = parse_directive(scan);
                break;

            default:
                die("value must start with quote or '+' or '-'\n\t%s\n", scan);
            }
        } end_while:;
    }

    return bit_num;
}

char *
eat_a_byte(char * scan, int * byte_val)
{
    if (*scan != '\\') {
        *byte_val = (unsigned char)*(scan++);

    } else {
        int chval = '\\';

        switch (*++scan) {
        case NUL: break;
        case '\\':               scan++; break;
        case 'a':  chval = '\a'; scan++; break;
        case 'b':  chval = '\b'; scan++; break;
        case 't':  chval = '\t'; scan++; break;
        case 'n':  chval = '\n'; scan++; break;
        case 'v':  chval = '\v'; scan++; break;
        case 'f':  chval = '\f'; scan++; break;
        case 'r':  chval = '\r'; scan++; break;
        case '"':  chval = '"';  scan++; break;
        case '0' ... '7':
            {
                char octbuf[4];
                octbuf[0] = *(scan++);
                if ((*scan < '0') || (*scan > '7'))
                    octbuf[1] = NUL;
                else {
                    octbuf[1] = *(scan++);
                    if ((*scan < '0') || (*scan > '7'))
                        octbuf[2] = NUL;
                    else {
                        octbuf[2] = *(scan++);
                        octbuf[3] = NUL;
                    }
                }
                chval = (int)strtoul(octbuf, NULL, 8);
                if (chval > 0xFF) {
                    scan -= 2;
                    goto invalid_escape;
                }
                break;
            }

        case 'x': case 'X':
        {
            char hexbuf[4];
            if (! isxdigit(*++scan)) goto invalid_escape;
            hexbuf[0] = *(scan++);
            if (! isxdigit(*scan))
                hexbuf[1] = NUL;
            else {
                hexbuf[1] = *(scan++);
                hexbuf[2] = NUL;
            }
            chval = (int)strtoul(hexbuf, NULL, 16);
            break;
        }

        default:
        invalid_escape:
            die("invalid escape sequence:  %s\n", scan-1);
        }

        *byte_val = chval;
    }

    return scan;
}


char *
scan_quoted_value(char * scan, value_map_t * map, quoted_val_action_t act)
{
    value_bits_t mask = 1 << map->bit_no;
    if (act == VAL_REMOVE)
        mask = ~mask;

    for (;;) {
        int lo_ix, hi_ix;

        switch (*scan) {
            case NUL: return scan;
            case '"': return scan + 1;
            default:  break;
        }
        scan = eat_a_byte(scan, &lo_ix);
        if ((*scan == '-') && (scan[1] != NUL) && (scan[1] != '"')) {
            scan = eat_a_byte(scan + 1, &hi_ix);
        } else {
            hi_ix = lo_ix;
        }

        switch (act) {
        case VAL_ADD:
            do {
                all_map.values[lo_ix] |= (map->values[lo_ix] = mask);
            } while (++lo_ix <= hi_ix);
            break;

        case VAL_REMOVE:
            map->values[lo_ix] = 0;
            do {
                all_map.values[lo_ix] &= mask;
            } while (++lo_ix <= hi_ix);
        }
    }
}

char *
trim(char * buf)
{
    while (isspace(*buf))  buf++;

    {
        char * pe = buf + strlen(buf);
        while ((pe > buf) && isspace(pe[-1]))  pe--;
        *pe = NUL;
    }

    switch (*buf) {
    case NUL:
    case '#':
        return NULL;
    default:
        return buf;
    }
}

char *
get_name(char * scan, char * nm_buf, size_t buf_size)
{
    char * buf_end = nm_buf + buf_size - 2;
    size_t nm_len = 0;

    while (isalnum(*scan) || (*scan == '_') || (*scan == '-')) {
        int ch = *(scan++);
        if (ch == '-')
            ch = '_';
        else if (islower(ch))
            ch = toupper(ch);

        *(nm_buf++) = ch;
        nm_len++;

        if (nm_buf >= buf_end)
            nm_buf--;
    }
    if (scan == nm_buf)
        die("input line does not start with a name:\n\t%s\n", nm_buf);

    while (isspace(*scan)) scan++;

    *nm_buf = NUL;
    curr_name_len = nm_len;

    return scan;
}

value_map_t *
new_map(char * name)
{
    value_map_t * map = malloc(sizeof(*map));
    memset(map, 0, sizeof(*map));
    *end_map = map;
    end_map  = &(map->next);
    strcpy(map->vname, name);
    map->bit_no = ~0;
    return map;
}

char *
copy_another_value(char * scan, value_map_t * map)
{
    int  add_in = (*scan == '+');
    char z[CLASS_NAME_LIMIT+1];
    value_map_t * mp = all_map.next;
    scan = get_name(scan+1, z, sizeof(z));

    for (;;mp = mp->next) {
        if (mp == NULL)
            die("No entry named %s\n", z);

        if (strcmp(z, mp->vname) == 0)
            break;
    }

    if (add_in) {
        map->mask |= mp->mask;
    }

    else if (map->bit_no == ~0)
        die("You cannot remove bits that are not there.\n");

    else {
        value_bits_t   mask = ~(1 << map->bit_no);
        value_bits_t   skip = mp->mask;
        value_bits_t * this = map->values;
        value_bits_t * all  = all_map.values;

        int ct = sizeof(mp->values) / sizeof(mp->values[0]);

        do  {
            /*
             *  We are negating, so if the named value has this bit set,
             *  turn off the current bit in the global mask and turn off
             *  all bits in the for-this-named-value array of masks.
             */
            if ((*all & skip) != 0) {
                *all &= mask;
                *this = 0;
            }

            this++; all++;
        } while (--ct > 0);
    }

    return scan;
}

static void
make_define_name(char * ptr)
{
    while (*ptr) {
        if (islower(*ptr))
            *ptr = toupper(*ptr);
        else if (! isalnum(*ptr))
            *ptr = '_';

        ptr++;
    }
}

char *
parse_directive(char * scan)
{
    size_t len = 0;

    while (isspace(*++scan))
        /* skip leading '%' and white space */;

    if (! isalpha(*scan))
        die("directives must begin with an alphabetic character:  %s\n", scan);

    for (;;) {
        char ch = scan[++len];
        switch (ch) {
        case ' ': case '\t':
        case NUL: goto have_name;
        case '-': scan[len] = '_'; break;
        case 'A' ... 'Z':
            scan[len] = _tolower((unsigned int)ch); break;
        }
    } have_name:;

    return disp_cm_opt(scan, len, scan + len);
}

static char *
add_text(char ** buff, size_t * sz, char * curr, char * add, int pfx)
{
    size_t len = (add == NULL) ? 0 : strlen(add);
    char * e = *buff + *sz - 4;

    if (*buff + len >= e) {
        size_t o = curr - *buff;
        *sz += BUF_SIZE;
        *buff = realloc(*buff, *sz);
        curr  = *buff + o;
    }

    if (pfx)
        *(curr++) = ' ', *(curr++) = '*';

    if (len == 0) {
        *(curr++) = '\n';

    } else {
        if (pfx)
            *(curr++) = ' ', *(curr++) = ' ';

        memcpy(curr, add, len);
        curr += len;
    }

    return curr;
}

static char *
load_text(int pfx)
{
    char * scan_in;
    char * com_buf      = malloc(BUF_SIZE);
    char * scan_out     = com_buf;
    size_t com_buf_size = BUF_SIZE;
    int    line_ct      = 0;
    int    blank_ct     = 0;

    for (;;) {
        if (fgets(buffer, BUF_SIZE, stdin) == NULL)
            die("incomplete comment section");

        scan_in = buffer;
        while (isspace(*scan_in)) scan_in++;

        /*
         *  if scan is NULL, we've got a comment
         */
        if (*scan_in == NUL) {
            blank_ct++;
            continue;
        }

        if (*scan_in == '%')
            break;

        if (line_ct++ == 0) {
            blank_ct = 0;

        } else while (blank_ct > 0) {
                scan_out =
                    add_text(&com_buf, &com_buf_size, scan_out, NULL, pfx);
                blank_ct--;
                line_ct++;
            }

        scan_out = add_text(&com_buf, &com_buf_size, scan_out, scan_in, pfx);
    }

    *scan_out  = NUL;
    return com_buf;
}

/* * * * * * * * * * * * * * * *

   The following handler functions document and define the embedded
   directives.  The first line of the comment must be exactly
   "handle" + one space + directive name + one space + "directive".
   Dispatch tables and documentation is derived from this.

 * * * * * * * * * * * * * * * */

/**
 * handle file directive.
 *
 * specifies the output file name.  The multi-inclusion guard is derived
 * from this name.  If %file is not specified, that guard defaults to
 * CHAR_MAPPER_H_GUARD.  The default output is to stdout.
 *
 * @param scan current scan point
 * @returns    end of guard scan
 */
char *
handle_file(char * scan)
{
    char * pz;

    if (! isspace(*scan)) die(bad_directive, scan-4);

    while (isspace(*scan))   scan++;

    if (freopen(scan, "w", stdout) != stdout)
        die("fs error %d (%s) reopening %s as stdout\n", errno,
            strerror(errno), scan);

    file_guard = pz = malloc(strlen(scan) + 7);
    sprintf(pz, "%s_GUARD", scan);
    make_define_name(pz);
    *scan = NUL;
    return scan;
}

/**
 * handle comment directive.
 *
 * specifies the text to insert in a comment block at the head of the output
 * file.  The comment text starts on the next line and ends with the first
 * input line with a percent ('%') in the first column.  The default is:
 * char-mapper Character Classifications
 *
 * @param scan current scan point
 * @returns    end of guard scan
 */
char *
handle_comment(char * scan)
{
    commentary = load_text(1);
    *scan = NUL;
    return scan;
}

/**
 * handle emit directive.
 *
 * collect text to be emitted at the end of the header
 *
 * @param scan current scan point
 * @returns    The '%' starting the next directive.
 */
char *
handle_emit(char * scan)
{
    add_on_text = load_text(0);
    *scan = NUL;
    return scan;
}

/**
 * handle map directive.
 *
 * Map a set of characters into another set.
 * There are three arguments to this directive:
 *
 * * the mapping name (e.g. "lower")
 * * the map-from list.  Specify a character class name.
 * * the map-to list. This may be a literal, a class name, or a mix.
 *
 * These three elements are separated by two commas (",").
 * If the "map-to" list is shorter than the map-from list, then the
 * last character is repeated until there are the same number of characters.
 * It is an error for it to be longer.  This will produce CPP macros
 * named TO_XXX_CHARS(), using the name above.  The input argument is also
 * the output argument.  It is an in-place replacement.
 *
 * @param scan current scan point
 * @returns    The '%' starting the next directive.
 */
char *
handle_map(char * scan)
{
    while (isspace(*scan))   scan++;
    if (*scan == NUL) die("%map requires a mapping specification");
    // mapping_text = strdup(scan);
    fprintf(stderr, "%map is not actually implemented yet.\n");
    *scan = NUL;
    return scan;
}

/**
 * handle table directive.
 *
 * specifies the name of the output table.  If not specified, it defaults to
 * the base file name, suffixed with "_table" and "char_type_table" if %file
 * is not specified.  Normally, this table is "extern" in scope, but may be
 * made static by specifying an empty %guard.
 *
 * @param scan current scan point
 * @returns    end of guard scan
 */
char *
handle_table(char * scan)
{
    char * pz;

    if (! isspace(*scan)) die(bad_directive, scan-5);

    while (isspace(*scan))   scan++;
    table_name = pz = strdup(scan);
    for (;*pz; pz++) {
        if (! isalnum((int)*pz))
            *pz = '_';
    }

    return scan + strlen(scan);
}

/**
 * handle guard directive.
 *
 * specifies the name of a '#ifdef' guard protecting the compilation of
 * the bit mask table.  One compilation unit must '#define' this name.
 * The default is the table name surrounded by "DEFINE_" and "_TABLE".
 * If empty, the output table is unguarded and made static in scope.
 *
 * @param scan current scan point
 * @returns    end of guard scan
 */
char *
handle_guard(char * scan)
{
    char * pz;

    if (! isspace(*scan)) {
        table_is_static = 1;
        return scan;
    }

    while (isspace(*scan))   scan++;
    data_guard = pz = strdup(scan);
    make_define_name(pz);
    return scan + strlen(scan);
}

/**
 * handle test directive.
 *
 * specifies that a main procedure is to be emitted under an ifdef guard.
 *
 * @param scan current scan point
 * @returns    end of guard scan
 */
char *
handle_test(char * scan)
{
    add_test_code = 1;
    return scan + strlen(scan);
}

/**
 * handle invalid directive.
 *
 * This function does not return.
 *
 * @param scan current scan point
 * @returns    not
 */
char *
handle_invalid(char * scan)
{
    die(bad_directive, scan);
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of char-mapper.c */
