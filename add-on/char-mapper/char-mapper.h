
/**
 * \file char-mapper.c
 *
 *  Time-stamp:        "2012-02-12 09:01:06 bkorb"
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
#ifndef  CHAR_MAPPER_H_GUARD
#define  CHAR_MAPPER_H_GUARD

#define  _GNU_SOURCE 1
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

#ifndef NBBY
#  define NBBY 8
#endif

#define BUF_SIZE            0x1000
#define CLASS_NAME_LIMIT    31
#define TABLE_SIZE          (1 << (NBBY - 1))

typedef unsigned int value_bits_t;
typedef struct value_map_s value_map_t;

struct value_map_s {
    value_map_t *   next;
    char            vname[CLASS_NAME_LIMIT+1];
    int             bit_no;
    value_bits_t    mask;
    value_bits_t    values[TABLE_SIZE];
};

typedef enum {
    VAL_ADD, VAL_REMOVE
} quoted_val_action_t;

static char const bad_directive[] = "invalid directive: %s\n";
static char const typedef_mask[]  = "typedef %s %s;\n\n";
static char const declare_tbl[]   = "extern %s const %s[%d];\n";

static char const leader_z[] = "/*\n\
 *   Character mapping generated %1$s\n *\n\
%2$s */\n\
#ifndef %3$s\n\
#define %3$s 1\n\n\
#ifdef HAVE_CONFIG_H\n\
# if defined(HAVE_INTTYPES_H)\n\
#   include <inttypes.h>\n\
# elif defined(HAVE_STDINT_H)\n\
#   include <stdint.h>\n\n\
# else\n\
#   ifndef HAVE_UINT8_T\n\
        typedef unsigned char   uint8_t;\n\
#   endif\n\
#   ifndef HAVE_UINT16_T\n\
        typedef unsigned short  uint16_t;\n\
#   endif\n\n\
#   ifndef HAVE_UINT32_T\n\
#     if SIZEOF_INT == 4\n\
        typedef unsigned int    uint32_t;\n\
#     elif SIZEOF_LONG == 4\n\
        typedef unsigned long   uint32_t;\n\
#     endif\n\
#   endif\n\n\
#   ifndef HAVE_UINT64_T\n\
#     if SIZEOF_LONG == 8\n\
        typedef unsigned long       uint64_t;\n\
#     elif SIZEOF_LONG_LONG == 8\n\
        typedef unsigned long long  uint64_t;\n\
#     endif\n\
#   endif\n\
# endif /* HAVE_*INT*_H header */\n\n\
#else /* not HAVE_CONFIG_H -- */\n\
# ifdef __sun\n\
#   include <inttypes.h>\n\
# else\n\
#   include <stdint.h>\n\
# endif\n\
#endif /* HAVE_CONFIG_H */\n";

static char const inline_functions[] = "\n"
"static inline int\n"
"is_%1$s_char(char ch, %2$s mask)\n"
"{\n"
"    unsigned int ix = (unsigned char)ch;\n"
"    return ((ix < %3$d) && ((%4$s[ix] & mask) != 0));\n"
"}\n\n"

"static inline char *\n"
"spn_%1$s_chars(char * p, %2$s mask)\n"
"{\n"
"    while ((*p != '\\0') && is_%1$s_char(*p, mask))  p++;\n"
"    return p;\n"
"}\n\n"

"static inline char *\n"
"brk_%1$s_chars(char * p, %2$s mask)\n"
"{\n"
"    while ((*p != '\\0') && (! is_%1$s_char(*p, mask)))  p++;\n"
"    return p;\n"
"}\n\n";

static char const emit_text_fmt[] =
"\n/* emit text from map file: */\n\n"
"%s\n";

static char const start_static_table_fmt[] =
"static %s const %s[%d] = {";

static char const start_table_fmt[] = "\
#ifdef %s\n%s const %s[%d] = {";

static char const copy_input_end[] =
"//\n#endif /* 0 -- mapping spec. source */\n\n";

static char const copy_input_start[] =
"\n#if 0 /* mapping specification source (from %s) */\n";

static char const macro_def_fmt[] =
"#define  IS_%1$s_CHAR( _c)%2$s  is_%3$s_char((char)( _c), %4$s)\n"
"#define SPN_%1$s_CHARS(_s)%2$s spn_%3$s_chars((char *)_s, %4$s)\n"
"#define BRK_%1$s_CHARS(_s)%2$s brk_%3$s_chars((char *)_s, %4$s)\n";

static char const mask_fmt_fmt[]= "0x%%0%dX";
static char mask_fmt[sizeof(mask_fmt_fmt) + 9];
static char const char_map_gd[] = "CHAR_MAPPER_H_GUARD";
static char const end_table[]   ="\n};\n";
static char const endif_fmt[]   = "#endif /* %s */\n";
static char const * data_guard  = NULL;
static char const * mask_name   = NULL;
static char const * base_fn_nm  = NULL;
static char const * base_ucase  = NULL;
static char const * file_guard  = char_map_gd;
static char const * commentary  = " *  char-mapper Character Classifications\n";
static char const * add_on_text = NULL;
static char const   tname_fmt[] = "%s_table";
static char const * table_name  = NULL;
static int          table_is_static = 0;
static int const    table_size = TABLE_SIZE;
static char buffer[BUF_SIZE];
static int          add_test_code = 0;

/* * * * * * * * * * * * * * * *
 *  testing formats
 */
static char const testit_fmt[] = "\n"
"#ifdef TEST_%1$s\n"
"int main (int argc, char ** argv) {\n"
"    int ix = 0;\n"
"    static char const header[] =";;

static char const testit_class_names[] = "\n"
"        \"%02X == %s\\n\"";

static char const testit_class_hdrs[] = "\n"
"        \"char is: ";

static char const test_loop[] =
"\\n\";\n"
"    fwrite(header, sizeof(header)-1, 1, stdout);\n\n"
"    for (; ix<128; ix++) {\n"
"        printf(\"0x%02X (%c) \", ix, ((ix >= 0x20) && (ix < 0x7F)) ? ix : '?');\n";

static char const each_test[] =
"        putchar(' '); putchar(' ');\n"
"        putchar(is_%s_char((char)ix, %s) ? 'X' : '.');\n";

static char const endtest_fmt[] =
"    }\n"
"    return 0;\n"
"}\n"
"#endif /* TEST_%1$s */\n";

value_map_t    all_map      = { NULL, "total", 0, 0, { 0 }};
value_map_t ** end_map      = &(all_map.next);
size_t         max_name_len = 0;
size_t         curr_name_len= 0;
int            need_comma   = 0;
int            bit_count    = 0;
int            skip_comment = 0;

void   die(char const *, ...);
char * trim(char * buf);
char * get_name(char * scan, char * nm_buf, size_t buf_size);
char * scan_quoted_value(char * scan, value_map_t *, quoted_val_action_t);
char * copy_another_value(char * scan, value_map_t * map);
char * parse_directive(char * scan);
value_map_t * new_map(char * name);
int  read_data(void);
void emit_macros(int bit_count);
void emit_table(int bit_count);
void emit_functions(void);
void parse_help(char const * pz);

static void
make_define_name(char * ptr);

static void
init_names(void);

static void
copy_input_text(char const * name);

#define write_const(_c, _fp)  fwrite(_c, sizeof(_c)-1, 1, _fp)

#endif /* CHAR_MAPPER_H_GUARD */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of char-mapper.h */
