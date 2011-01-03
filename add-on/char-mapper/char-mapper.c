
/**
 * \file char-mapper.c
 *
 *  Time-stamp:        "2010-07-16 13:52:39 bkorb"
 *
 *  This is the main routine for char-mapper.
 *
 *  This file is part of char-mapper.
 *  char-mapper Copyright (c) 1992-2011 by Bruce Korb - all rights reserved
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

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cm-opt.h"

#ifndef NUL
#  define NUL '\0'
#endif

static char const usage_txt[] =
    "USAGE:  char-mapper [ -h | --help | <input-file> ]\n"
    "If the '<input-file>' is not specified, it is read from standard input.\n"
    "Input may not be from a TTY device.  Various directives affecting the\n"
    "output are embedded in the input text.  These are:\n\n"

    "  %guard\n"
    "\tspecifies the name of a '#ifdef' guard protecting the compilation of\n"
    "\tthe bit mask table.  One compilation unit must '#define' this name.\n"
    "\tThe default is:  CHAR_MAPPER_DEFINE\n\n"

    "  %file\n"
    "\tspecifies the output file name.  The multi-inclusion guard is\n"
    "\tderived from this name.  That guard defaults to CHAR_MAPPER_H_GUARD.\n"
    "\tThe default output is to stdout.\n\n"

    "  %comment\n"
    "\tspecifies the text to insert in a comment block at the head of the\n"
    "\toutput file.  The comment text starts on the next line and ends with\n"
    "\tthe first input line with a percent ('%') in the first column.\n"
    "\tThe default is:  char-mapper Character Classifications\n\n"

    "  %table\n"
    "\tspecifies the name of the output table.\n"
    "\tThe default is:  char_type_table\n\n"

    "Otherwise, input lines that are blank or start with a hash ('#') are\n"
    "ignored.  The lines that are not ignored must begin with a name and be\n"
    "followed by space separated character membership specifications.\n"
    "A quoted string is \"cooked\" and the characters found added to the\n"
    "set of member characters for the given name.  If the string is preceded\n"
    "by a hyphen ('-'), then the characters are removed from the set.  Ranges\n"
    "of characters are specified by a hyphen between two characters.  If you\n"
    "want a hyphen in the set, use it at the start or end of the string.\n"
    "You may also add or remove members of previously defined sets by name,\n"
    "preceded with a plus ('+') or hyphen ('-'), respectively.\n\n"

    "If the input file can be rewound and re-read, then the input text will\n"
    "also be inserted into the output as a comment.\n";

#define BUF_SIZE 1024
#define CLASS_NAME_LIMIT 31

static char const bad_directive[] = "invalid directive: %s\n";

static char const leader_z[] = "/*\n\
 *   Character mapping generated %1$s\n *\n\
%2$s */\n\
#ifndef %3$s\n\
#define %3$s 1\n\n\
#ifdef HAVE_CONFIG_H\n\
# if defined(HAVE_INTTYPES_H)\n\
#  include <inttypes.h>\n\
# elif defined(HAVE_STDINT_H)\n\
#  include <stdint.h>\n\n\
# else\n\
#   ifndef HAVE_INT8_T\n\
        typedef signed char     int8_t;\n\
#   endif\n\
#   ifndef HAVE_UINT8_T\n\
        typedef unsigned char   uint8_t;\n\
#   endif\n\
#   ifndef HAVE_INT16_T\n\
        typedef signed short    int16_t;\n\
#   endif\n\
#   ifndef HAVE_UINT16_T\n\
        typedef unsigned short  uint16_t;\n\
#   endif\n\
#   ifndef HAVE_UINT_T\n\
        typedef unsigned int    uint_t;\n\
#   endif\n\n\
#   ifndef HAVE_INT32_T\n\
#    if SIZEOF_INT == 4\n\
        typedef signed int      int32_t;\n\
#    elif SIZEOF_LONG == 4\n\
        typedef signed long     int32_t;\n\
#    endif\n\
#   endif\n\n\
#   ifndef HAVE_UINT32_T\n\
#    if SIZEOF_INT == 4\n\
        typedef unsigned int    uint32_t;\n\
#    elif SIZEOF_LONG == 4\n\
        typedef unsigned long   uint32_t;\n\
#    endif\n\
#   endif\n\
# endif /* HAVE_*INT*_H header */\n\n\
#else /* not HAVE_CONFIG_H -- */\n\
# ifdef __sun\n\
#  include <inttypes.h>\n\
# else\n\
#  include <stdint.h>\n\
# endif\n\
#endif /* HAVE_CONFIG_H */\n";

static char const check_class_inline[] =
"typedef %3$s %1$s_mask_t;\n"
"extern %1$s_mask_t const %1$s[128];\n\n"
"static inline int is_%1$s_char(char ch, %1$s_mask_t mask) {\n"
"    unsigned int ix = (unsigned char)ch;\n"
"    return ((ix < 0x7F) && ((%1$s[ix] & mask) != 0)); }\n\n";

static char const endif_fmt[] = "#endif /* %s */\n";
static char const mask_fmt_fmt[] = "0x%%0%dX";
static char mask_fmt[sizeof(mask_fmt_fmt) + 9];

static char const * data_guard = "CHAR_MAPPER_DEFINE";
static char const * file_guard = "CHAR_MAPPER_H_GUARD";
static char const * commentary = " *  char-mapper Character Classifications\n";
static char const * table_name = "char_type_table";
char in_macro_name[CLASS_NAME_LIMIT+7] = "IN_CHAR_TYPE_TABLE";

static char buffer[BUF_SIZE];

typedef unsigned int value_bits_t;
typedef struct value_map_s value_map_t;

struct value_map_s {
    value_map_t *   next;
    char            vname[CLASS_NAME_LIMIT+1];
    int             bit_no;
    value_bits_t    mask;
    value_bits_t    values[128];
};

value_map_t all_map = { NULL, "total", 0, 0, { 0 }};
value_map_t ** end_map = &(all_map.next);
size_t max_name_len = 0;
size_t curr_name_len;

typedef enum {
    VAL_ADD, VAL_REMOVE
} quoted_val_action_t;

void   die(char const *, ...);
char * trim(char * buf);
char * get_name(char * scan, char * nm_buf, size_t buf_size);
char * scan_quoted_value(char * scan, value_map_t *, quoted_val_action_t);
char * copy_another_value(char * scan, value_map_t * map);
char * parse_directive(char * scan);
value_map_t * new_map(char * name);
int read_data(void);
void emit_macros(int bit_count);
void emit_table(int bit_count);
void parse_help(char const * pz);

int
main(int argc, char ** argv)
{
    int need_comma = 0;
    int bit_count = 0;

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

    if (fseek(stdin, 0, SEEK_SET) == 0) {
        static char const end_src[] =
            "//\n#endif /* 0 -- mapping spec. source */\n\n";
        static char const src_z[] =
            "\n#if 0 /* mapping specification source (from %s) */\n";
        printf(src_z, argv[1]);
        while (fgets(buffer, BUF_SIZE, stdin) != NULL)
            printf("// %s", buffer);

        fwrite(end_src, sizeof(end_src)-1, 1, stdout);
    }

    {
        int width = (bit_count+3)/4;
        if (width < 2) width = 2;
        sprintf(mask_fmt, mask_fmt_fmt, width);
        if (width > 8)
            strcat(mask_fmt, "ULL");
    }

    emit_macros(bit_count);
    emit_table(bit_count);
    printf(endif_fmt, file_guard);
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

void
parse_help(char const * opt_pz)
{
    static char const opterrmsg[] = "char-mapper error:  %s:  %s\n";
    int exit_code = EXIT_SUCCESS;
    FILE * fp = stdout;
    char const * pz = opt_pz;

    if (opt_pz == NULL) {
        fprintf(stderr, opterrmsg, "input is from a TTY device",
                "must be file or pipe\n");
        fwrite(usage_txt, sizeof(usage_txt)-1, 1, fp);
        exit_code = EXIT_FAILURE;
    } else

    switch (pz[1]) {
    case '-':
    {
        static char const help[] = "help";
        char const * p = help;
        pz += 2;
        for (;;) {
            if (*pz != *p)
                goto bad_opt;
            if (*(p++) == NUL)
                break;
            if (*++pz == NUL)
                break;
        }
    }
    case 'h':
        fwrite(usage_txt, sizeof(usage_txt)-1, 1, fp);
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
    static char const macro_fmt[] =
        "#define IS_%s_CHAR(_c)%s is_%s_char((_c), ";
    char fill[CLASS_NAME_LIMIT+3];

    
    value_map_t * map;

    {
        char * pzD = in_macro_name + 3;
        char const * pzS = table_name;
        for (;;) {
            char ch = *(pzS++);
            switch (ch) {
            case NUL:
                *pzD = NUL;
                goto print_class_mask;

            case 'a' ... 'z':
                *(pzD++) = toupper((unsigned int)ch);
                break;

            case 'A' ... 'Z': case '_':
                *(pzD++) = ch;
                break;

            default:
                *(pzD++) = '_';
            }
        }
    } print_class_mask:;

    {
        char const * type_pz;
        switch (bit_count) {
        case  1 ...  8: type_pz = "uint8_t";  break;
        case  9 ... 16: type_pz = "uint16_t"; break;
        case 17 ... 32: type_pz = "uint32_t"; break;
        case 33 ... 64: type_pz = "uint64_t"; break;

        default: die("too many char types (31 max)\n");
        }

        printf(check_class_inline, table_name, in_macro_name, type_pz);
    }

    if (max_name_len < CLASS_NAME_LIMIT-2)
        max_name_len += 2;
    memset(fill, ' ', max_name_len);
    fill[max_name_len - 1] = NUL;

    for (map = all_map.next; map != NULL; map = map->next) {
        char * pz = fill + strlen(map->vname);
        printf(macro_fmt, map->vname, pz, table_name);
        printf(mask_fmt, map->mask);
        putc(')', stdout);
        putc('\n', stdout);
    }

    putc('\n', stdout);
}


void
emit_table(int bit_count)
{
    static char const start_table_fmt[] = "\
#ifdef %1$s\n\
%2$s_mask_t const %2$s[128] = {\
";

    char const * type_pz;
    char * fmt;

    int ix       = 0;
    int need_cm  = 0;
    int entry_ct = 0;
    int init_ct  = (bit_count > 32) ? 2 : 4;

    printf(start_table_fmt, data_guard, table_name);

    while (ix < 128) {

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
            case '\a': fputs(" /*\\a */ ", stdout); break;
            case '\b': fputs(" /*\\b */ ", stdout); break;
            case '\t': fputs(" /*\\t */ ", stdout); break;
            case '\n': fputs(" /*\\n */ ", stdout); break;
            case '\v': fputs(" /*\\v */ ", stdout); break;
            case '\f': fputs(" /*\\f */ ", stdout); break;
            case '\r': fputs(" /*\\r */ ", stdout); break;
            default:   printf(" /*x%02X*/ ", ix); break;
            }

        printf(mask_fmt, all_map.values[ix++]);
    }
    fputs("\n};\n", stdout);

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
scan_quoted_value(char * scan, value_map_t * map, quoted_val_action_t act)
{
    value_bits_t mask = 1 << map->bit_no;
    if (act == VAL_REMOVE)
        mask = ~mask;

    for (;;) {
        int lo_ix = (unsigned)*(scan++);
        int hi_ix;

        if ((lo_ix == '"') || (lo_ix == NUL))
            return scan;

        if (lo_ix == '\\') {
            switch (*scan) {
            case NUL: break;
            case '\\':               scan++; break;
            case 'a':  lo_ix = '\a'; scan++; break;
            case 'b':  lo_ix = '\b'; scan++; break;
            case 't':  lo_ix = '\t'; scan++; break;
            case 'n':  lo_ix = '\n'; scan++; break;
            case 'v':  lo_ix = '\v'; scan++; break;
            case 'f':  lo_ix = '\f'; scan++; break;
            case 'r':  lo_ix = '\r'; scan++; break;
            case '"':  lo_ix = '"';  scan++; break;
            case '0' ... '7':
                {
                    char octbuf[4];
                    octbuf[0] = *(scan++);
                    if ((*scan < '0') || (*scan > '7'))
                        octbuf[1] = '\0';
                    else {
                        octbuf[1] = *(scan++);
                        if ((*scan < '0') || (*scan > '7'))
                            octbuf[2] = '\0';
                        else {
                            octbuf[2] = *(scan++);
                            octbuf[3] = '\0';
                        }
                    }
                    lo_ix = (int)strtoul(octbuf, NULL, 8);
                    if (lo_ix > 0xFF) {
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
                    hexbuf[1] = '\0';
                else {
                    hexbuf[1] = *(scan++);
                    hexbuf[2] = '\0';
                }
                lo_ix = (int)strtoul(hexbuf, NULL, 16);
                break;
            }

            default:
            invalid_escape:
                die("invalid escape sequence:  %s\n", scan-1);
            }
            hi_ix = lo_ix;
        }
        else if ((*scan == '-') && (scan[1] != NUL) && (scan[1] != '"')) {
            hi_ix = (unsigned)(scan[1]);
            scan += 2;

        } else
            hi_ix = lo_ix;

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
    char * pe = buf + strlen(buf);
    while ((pe > buf) && isspace(pe[-1]))  pe--;
    *pe = NUL;
    while (isspace(*buf))  buf++;
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

void
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
handle_guard(char * scan)
{
    char * pz;

    if (! isspace(*scan)) die(bad_directive, scan-5);

    while (isspace(*scan))   scan++;
    data_guard = pz = strdup(scan);
    make_define_name(pz);
    return scan + strlen(scan);
}

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
    return scan + strlen(scan);
}

char *
handle_comment(char * scan)
{
    char * com_buf = malloc(0x1000);
    char * com_scan = com_buf;
    size_t com_buf_size = 0x1000;

    for (;;) {
        size_t line_len;

        if (fgets(buffer, BUF_SIZE, stdin) == NULL)
            die("incomplete comment section");

        scan = trim(buffer);
        /*
         *  if scan is NULL, we've got a comment
         */
        if (scan == NULL)
            continue;

        if (*scan == '%')
            break;
        if ((*scan == NUL) && (com_scan + 3 < com_buf + com_buf_size)) {
            memcpy(com_scan, " *\n", 3);
            com_scan += 3;
        } else {
            line_len = strlen(scan);
            if (com_scan + line_len + 6 >= com_buf + com_buf_size) {
                com_buf_size += 0x1000;
                com_buf = realloc(com_buf, com_buf_size);
            }
            memcpy(com_scan, " *  ", 4);
            memcpy(com_scan + 4, scan, line_len);
            com_scan += line_len + 4;
            *(com_scan++) = '\n';
        }
        *com_scan = NUL;
    }
    commentary = com_buf;
    return scan + strlen(scan);
}

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

char *
handle_invalid(char * scan)
{
    die(bad_directive, scan);
}

char *
parse_directive(char * scan)
{
    size_t len = 0;

    while (isspace(*++scan))
        /* skip leading '%' and white space */;

    if (! isalpha(*scan))
        die("directives must begin with an alphabetic character:  %s\n", scan);

    while (! isspace(scan[++len]))
        if (scan[len] == NUL)
            break;

    return disp_cm_opt(scan, len, scan + len);
}
