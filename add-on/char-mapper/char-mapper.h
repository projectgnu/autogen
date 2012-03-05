
/**
 * \file char-mapper.c
 *
 *  Time-stamp:        "2012-03-04 12:16:00 bkorb"
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
#include "map-text.h"

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

static char mask_fmt[sizeof(mask_fmt_fmt) + 9];
static char const * data_guard  = NULL;
static char const * mask_name   = NULL;
static char const * base_fn_nm  = NULL;
static char const * base_ucase  = NULL;
static char *       out_file_nm = NULL;
static char const * file_guard  = char_map_gd;
static char const * commentary  = " *  char-mapper Character Classifications\n";
static char const * add_on_text = NULL;
static char const * table_name  = NULL;
static int          table_is_static = 0;
static int const    table_size = TABLE_SIZE;
static char buffer[BUF_SIZE];
static int          add_test_code = 0;
static int          add_backup_code = 0;

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

static void
emit_leader(char * input);

#endif /* CHAR_MAPPER_H_GUARD */
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of char-mapper.h */
