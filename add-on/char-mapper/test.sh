#! /bin/sh

case "${VERBOSE}" in
[YytT1-9]* ) set -x ;;
esac
base_name=test-cmap
rm -f ${base_name}*

case "${1}" in
-c | --cl* )
    echo "rm -f ${base_name}*"
    exit 0
    ;;
esac

set -e

tbl_name=`echo ${base_name} | sed 's/[^a-zA-Z0-9_]/_/g'`

{
    cat <<- _EOF_
	%file ${base_name}.h
	%test
	%comment
	   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
	   Contributed by Zack Weinberg <zackw@stanford.edu>.

	This file is part of the libiberty library.
	Libiberty is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	Libiberty is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with libiberty; see the file COPYING.LIB.  If
	not, write to the Free Software Foundation, Inc.,
	51 Franklin Street - Fifth Floor,
	Boston, MA 02110-1301, USA.

	This is a compatible replacement of the standard C library's <ctype.h>
	with the following properties:

	   - Implements all isxxx() macros required by C99.
	   - Also implements some character classes useful when
	     parsing C-like languages.
	   - Does not change behavior depending on the current locale.
	   - Behaves properly for all values in the range of a signed or
	     unsigned char.

	   To avoid conflicts, this header defines the isxxx functions in upper
	   case, e.g. ISALPHA not isalpha.
	%

	# Basic types:
	#
	blank   " \t"
	cntrl   "\x00-\x1F\x7F"
	digit   "0-9"
	lower   "a-z"
	space   " \t\n\r\f\v"
	upper   "A-Z"
	vsp     "\r\n"

	# Compound char types:
	#
	xdigit  "a-fA-F"    +digit
	nvsp    "\x00\f\v"  +blank
	idst    "_"         +upper +lower
	punct   "\x20-\x7E" -upper -lower -blank

	# pure combined char types:
	#
	idnum   +idst       +digit
	alnum   +upper      +lower +digit
	alpha   +upper      +lower
	graph   +alnum      +punct
	print   +punct      +alnum +blank
	cppsp   +vsp        +nvsp
	basic   +print      +cppsp

	%emit
	#define HOST_CHARSET_UNKNOWN 0
	#define HOST_CHARSET_ASCII   1
	#define HOST_CHARSET_EBCDIC  2

	#if  '\n' == 0x0A && ' ' == 0x20 && '0' == 0x30 \\
	   && 'A' == 0x41 && 'a' == 0x61 && '!' == 0x21
	#  define HOST_CHARSET HOST_CHARSET_ASCII
	#else
	# if '\n' == 0x15 && ' ' == 0x40 && '0' == 0xF0 \\
	   && 'A' == 0xC1 && 'a' == 0x81 && '!' == 0x5A
	#  define HOST_CHARSET HOST_CHARSET_EBCDIC
	# else
	#  define HOST_CHARSET HOST_CHARSET_UNKNOWN
	# endif
	#endif

	// remapping from old names
	//
	#define ISALPHA(_c)             IS_ALPHA_CHAR(_c)
	#define ISALNUM(_c)             IS_ALNUM_CHAR(_c)
	#define ISBLANK(_c)             IS_BLANK_CHAR(_c)
	#define ISCNTRL(_c)             IS_CNTRL_CHAR(_c)
	#define ISDIGIT(_c)             IS_DIGIT_CHAR(_c)
	#define ISGRAPH(_c)             IS_GRAPH_CHAR(_c)
	#define ISLOWER(_c)             IS_LOWER_CHAR(_c)
	#define ISPRINT(_c)             IS_PRINT_CHAR(_c)
	#define ISPUNCT(_c)             IS_PUNCT_CHAR(_c)
	#define ISSPACE(_c)             IS_SPACE_CHAR(_c)
	#define ISUPPER(_c)             IS_UPPER_CHAR(_c)
	#define ISXDIGIT(_c)            IS_XDIGIT_CHAR(_c)
	#define ISIDNUM(_c)	        IS_IDNUM_CHAR(_c)
	#define ISIDST(_c)	        IS_IDST_CHAR(_c)
	#define IS_ISOBASIC(_c)	        IS_BASIC_CHAR(_c)
	#define IS_VSPACE(_c)	        IS_VSP_CHAR(_c)
	#define IS_NVSPACE(_c)	        IS_NVSP_CHAR(_c)
	#define IS_SPACE_OR_NUL(_c)	IS_CPPSP_CHAR(_c)
	%
	_EOF_
} > ${base_name}.map

./char-mapper ${base_name}.map
test -f ${base_name}.h

cat >${base_name}.c <<- _EOF_
	#include <stdio.h>
	#include "${base_name}.h"
	_EOF_

guard=$(echo ${base_name} | tr a-z- A-Z_)
defs="-DDEFINE_${guard}_TABLE -DTEST_${guard}"
cmd=$(echo ${CC:-cc} -o ${base_name} $defs ${base_name}.c)
echo $cmd
$cmd
case "$-" in
*x* ) ./${base_name}
      ls -l ${base_name}* ;;
*   ) ./${base_name} >/dev/null
      rm -f ${base_name}* ;;
esac
